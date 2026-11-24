#pragma once

struct CaKiemThu {
    const char* ten;
    const char* ban_ro;
    const char* khoa;
    const char* ky_vong;
};

// Moi gia tri gom 32 chu so hex. Them mot dong cho moi ca moi.
constexpr CaKiemThu danh_sach_ca[] = {
    {"Vi du tren bang", "00000000000000000000000000000011",
     "00000000000000000000000000000000", "0410BEFCCDE6944B69DD007DEBE39A9D"},
};
