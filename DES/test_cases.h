#pragma once

#include <cstdint>

struct CaKiemThu {
    const char* ten;
    std::uint64_t ban_ro;
    std::uint64_t khoa;
    std::uint64_t ky_vong;
};

// Moi ca gom ten, ban ro, khoa va ban ma ky vong.
constexpr CaKiemThu danh_sach_ca[] = {
    {"Vi du tu chon", 0x1122334455667788, 0x0E329232EA6D0D73, 0x80A43903BFAA2E7C},
};
