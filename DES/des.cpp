#include <cstdint>
#include <iomanip>
#include <iostream>

#include "tables.h"
#include "test_cases.h"

using namespace std;

using Word = uint64_t;
constexpr Word mat_na_28_bit = 0xFFFFFFF;
constexpr Word mat_na_32_bit = 0xFFFFFFFF;

template <size_t N>
Word hoan_vi(Word gia_tri, int so_bit, const int (&vi_tri)[N]) {
    Word ket_qua = 0;
    // Vi tri trong bang bat dau tu 1, tinh tu bit cao nhat.
    for (int p : vi_tri) {
        int so_bit_dich = so_bit - p;
        Word bit_duoc_chon = (gia_tri >> so_bit_dich) & 1;
        ket_qua <<= 1;
        ket_qua |= bit_duoc_chon;
    }
    return ket_qua;
}

Word xoay_28_bit(Word gia_tri, int so_bit_xoay) {
    Word phan_dich_trai = (gia_tri << so_bit_xoay) & mat_na_28_bit;
    Word bit_quay_lai = gia_tri >> (28 - so_bit_xoay);
    return phan_dich_trai | bit_quay_lai;
}

void in_hex(const char* nhan, Word gia_tri, int so_chu_so) {
    cout << nhan << setw(so_chu_so) << gia_tri << '\n';
}

Word thay_the(Word dau_vao) {
    Word dau_ra = 0;
    for (int i = 0; i < 8; ++i) {
        int so_bit_dich = 6 * (7 - i);
        int nhom = (dau_vao >> so_bit_dich) & 0x3F;
        // Hai bit ngoai chon hang, bon bit giua chon cot.
        int bit_dau = nhom >> 5;
        int bit_cuoi = nhom & 1;
        int hang = 2 * bit_dau + bit_cuoi;
        int cot = (nhom >> 1) & 0xF;
        int vi_tri = 16 * hang + cot;
        int chu_so = S[i][vi_tri];
        cout << "  S" << i + 1 << " vao=" << setw(2) << nhom
             << " hang=" << hang << " cot=" << cot << " ra=" << chu_so << '\n';
        dau_ra <<= 4;
        dau_ra |= chu_so;
    }
    return dau_ra;
}

bool chay_ca(const CaKiemThu& ca) {
    const Word ban_ro = ca.ban_ro;
    const Word khoa = ca.khoa;
    const Word ky_vong = ca.ky_vong;
    constexpr int lich_xoay[] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    cout << "\nCa kiem thu: " << ca.ten << '\n';
    in_hex("Ban ro    = ", ban_ro, 16);
    in_hex("Khoa      = ", khoa, 16);

    Word khoa_56_bit = hoan_vi(khoa, 64, PC1);
    Word c = khoa_56_bit >> 28;
    Word d = khoa_56_bit & mat_na_28_bit;
    in_hex("PC-1 = ", khoa_56_bit, 14);
    in_hex("C0 = ", c, 7);
    in_hex("D0 = ", d, 7);
    Word sau_ip = hoan_vi(ban_ro, 64, IP);
    Word trai = sau_ip >> 32;
    Word phai = sau_ip & mat_na_32_bit;
    in_hex("IP = ", sau_ip, 16);
    in_hex("L0 = ", trai, 8);
    in_hex("R0 = ", phai, 8);

    for (int i = 0; i < 16; ++i) {
        cout << "\nVong " << dec << i+1 << '\n' << hex;
        c = xoay_28_bit(c, lich_xoay[i]);
        d = xoay_28_bit(d, lich_xoay[i]);
        Word ghep_c_d = (c << 28) | d;
        Word khoa_con = hoan_vi(ghep_c_d, 56, PC2);
        in_hex("C = ", c, 7);
        in_hex("D = ", d, 7);
        in_hex("K = ", khoa_con, 12);

        Word mo_rong = hoan_vi(phai, 32, E);
        Word sau_xor = mo_rong ^ khoa_con;
        in_hex("E(R)  = ", mo_rong, 12);
        in_hex("XOR K = ", sau_xor, 12);
        Word sau_s = thay_the(sau_xor);
        Word f = hoan_vi(sau_s, 32, P);
        in_hex("S    = ", sau_s, 8);
        in_hex("P(S) = ", f, 8);

        // Cap nhat Feistel theo cong thuc L[i] = R[i-1], R[i] = L[i-1] XOR f.
        Word phai_moi = trai ^ f;
        trai = phai;
        phai = phai_moi;
        in_hex("L = ", trai, 8);
        in_hex("R = ", phai, 8);
    }
    Word doi_cho = (phai << 32) | trai;
    Word ban_ma = hoan_vi(doi_cho, 64, IP_INV);
    in_hex("\nR16 || L16 = ", doi_cho, 16);
    in_hex("Ban ma     = ", ban_ma, 16);
    in_hex("Ky vong    = ", ky_vong, 16);
    bool dat = ban_ma == ky_vong;
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
