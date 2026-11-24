#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include "test_cases.h"

using namespace std;

using Byte = uint8_t;
using Khoi = array<Byte, 16>;

// Nhan voi x trong truong AES, lay du theo x^8 + x^4 + x^3 + x + 1.
Byte xtime(Byte x) {
    int ket_qua = x << 1;
    if (ket_qua & 0x100) {
        ket_qua ^= 0x11B;
    }
    return static_cast<Byte>(ket_qua);
}

void in_hex(const char* nhan, const Khoi& khoi) {
    cout << nhan << ' ';
    for (Byte byte : khoi) {
        cout << setw(2) << static_cast<unsigned>(byte);
    }
    cout << '\n';
}

Khoi xor_khoa(Khoi trang_thai, const Khoi& khoa) {
    for (int i = 0; i < 16; ++i) {
        trang_thai[i] = trang_thai[i] ^ khoa[i];
    }
    return trang_thai;
}

Khoi dich_hang(const Khoi& trang_thai) {
    Khoi ket_qua{};
    // Byte o hang r, cot c nam tai vi tri 4*c+r.
    for (int hang = 0; hang < 4; ++hang) {
        for (int cot = 0; cot < 4; ++cot) {
            int cot_nguon = (cot + hang) % 4;
            ket_qua[4 * cot + hang] = trang_thai[4 * cot_nguon + hang];
        }
    }
    return ket_qua;
}

void tron_cot(Khoi& trang_thai) {
    for (int i = 0; i < 16; i += 4) {
        Byte a = trang_thai[i];
        Byte b = trang_thai[i + 1];
        Byte c = trang_thai[i + 2];
        Byte d = trang_thai[i + 3];
        Byte a2 = xtime(a);
        Byte b2 = xtime(b);
        Byte c2 = xtime(c);
        Byte d2 = xtime(d);

        // Trong truong AES, 03 × x = (02 × x) XOR x.
        Byte a3 = a2 ^ a;
        Byte b3 = b2 ^ b;
        Byte c3 = c2 ^ c;
        Byte d3 = d2 ^ d;

        // Moi dong tuong ung voi mot hang cua ma tran MixColumns.
        trang_thai[i]     = a2 ^ b3 ^ c ^ d;
        trang_thai[i + 1] = a ^ b2 ^ c3 ^ d;
        trang_thai[i + 2] = a ^ b ^ c2 ^ d3;
        trang_thai[i + 3] = a3 ^ b ^ c ^ d2;
    }
}

Khoi sinh_khoa_tiep(Khoi khoa, Byte rcon);
// Nhan hai byte trong GF(2^8) bang cach cong cac tich theo tung bit.
Byte nhan_trong_truong(Byte a, Byte b) {
    Byte ket_qua = 0;
    for (int i = 0; i < 8; ++i) {
        if ((b & 1) != 0) {
            ket_qua ^= a;
        }
        a = xtime(a);
        b >>= 1;
    }
    return ket_qua;
}

Byte nghich_dao(Byte byte) {
    if (byte == 0) {
        return 0;
    }

    // Voi byte khac 0, nghich dao la byte^254 trong GF(2^8).
    // Tinh luy thua bang binh phuong va nhan theo tung bit cua so mu.
    Byte ket_qua = 1;
    Byte co_so = byte;
    int so_mu = 254;
    while (so_mu > 0) {
        if ((so_mu & 1) != 0) {
            ket_qua = nhan_trong_truong(ket_qua, co_so);
        }
        co_so = nhan_trong_truong(co_so, co_so);
        so_mu >>= 1;
    }
    return ket_qua;
}

Byte xoay_trai_byte(Byte byte, int so_bit) {
    int phan_dich_trai = (byte << so_bit) & 0xFF;
    int bit_quay_lai = byte >> (8 - so_bit);
    return static_cast<Byte>(phan_dich_trai | bit_quay_lai);
}

Byte thay_byte(Byte byte) {
    Byte nghich_dao_byte = nghich_dao(byte);

    // Bien doi affine dung bon ban xoay cua cung mot byte nghich dao.
    Byte ket_qua = nghich_dao_byte ^ 0x63;
    for (int so_bit = 1; so_bit <= 4; ++so_bit) {
        ket_qua ^= xoay_trai_byte(nghich_dao_byte, so_bit);
    }
    return ket_qua;
}

Khoi doc_khoi_hex(const string& chuoi_hex) {
    Khoi khoi{};
    for (int i = 0; i < 16; ++i) {
        string cap_hex = chuoi_hex.substr(2 * i, 2);
        khoi[i] = static_cast<Byte>(stoul(cap_hex, nullptr, 16));
    }
    return khoi;
}

bool chay_ca(const CaKiemThu& ca) {
    Khoi ban_ro = doc_khoi_hex(ca.ban_ro);
    Khoi khoa = doc_khoi_hex(ca.khoa);
    const Khoi ky_vong = doc_khoi_hex(ca.ky_vong);
    cout << "\nCa kiem thu: " << ca.ten << '\n';
    in_hex("Ban ro     ", ban_ro);
    in_hex("Khoa vong 0", khoa);
    Khoi trang_thai = xor_khoa(ban_ro, khoa);
    in_hex("AddRoundKey", trang_thai);

    Byte rcon = 1;
    for (int vong = 1; vong <= 10; ++vong) {
        cout << "\nVong " << dec << vong << '\n' << hex;
        for (Byte& byte : trang_thai) {
            byte = thay_byte(byte);
        }
        in_hex("SubBytes   ", trang_thai);
        trang_thai = dich_hang(trang_thai);
        in_hex("ShiftRows  ", trang_thai);
        if (vong < 10) {
            tron_cot(trang_thai);
            in_hex("MixColumns ", trang_thai);
        }
        khoa = sinh_khoa_tiep(khoa, rcon);
        rcon = xtime(rcon);
        in_hex("Khoa vong  ", khoa);
        trang_thai = xor_khoa(trang_thai, khoa);
        in_hex("AddRoundKey", trang_thai);
    }
    in_hex("\nBan ma     ", trang_thai);
    in_hex("Ky vong    ", ky_vong);
    bool dat = trang_thai == ky_vong;
    cout << (dat ? "DAT\n" : "KHONG DAT\n");
    return dat;
}

int main() {
    cout << hex << uppercase << setfill('0');
    bool tat_ca_dat = true;
    for (const CaKiemThu& ca : danh_sach_ca) {
        if (!chay_ca(ca)) {
            tat_ca_dat = false;
        }
    }
    return tat_ca_dat ? 0 : 1;
}

Khoi sinh_khoa_tiep(Khoi khoa, Byte rcon) {
    // RotWord xoay tu cuoi cua khoa cu sang trai mot byte.
    array<Byte, 4> tu_tam = {khoa[13], khoa[14], khoa[15], khoa[12]};
    // SubWord ap dung phep thay the AES cho tung byte.
    for (Byte& byte : tu_tam) {
        byte = thay_byte(byte);
    }
    tu_tam[0] ^= rcon;

    Khoi khoa_moi{};
    for (int i = 0; i < 4; ++i) {
        khoa_moi[i] = khoa[i] ^ tu_tam[i];
    }
    // Voi ba tu con lai, XOR tu cu voi tu moi lien truoc.
    for (int i = 4; i < 16; ++i) {
        khoa_moi[i] = khoa[i] ^ khoa_moi[i - 4];
    }
    return khoa_moi;
}
