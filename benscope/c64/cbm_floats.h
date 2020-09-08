#ifndef __BENSCOPE_C64_CBM_FLOATS_H__
#define __BENSCOPE_C64_CBM_FLOATS_H__

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace benscope::c64 {

struct CBMPackedFloat {
  explicit CBMPackedFloat(double val) {
    uint8_t bytes[sizeof(double)];
    std::memcpy(bytes, &val, sizeof(double));
    std::reverse(std::begin(bytes), std::end(bytes));

    std::cerr << "double: " << val << " (@" << &val << ")\n";

    std::cerr << "bytes:  ";
    for (int i = 0; i < static_cast<int>(sizeof(double)); ++i) {
      std::cerr << static_cast<int>(bytes[i]) << " ";
    }
    std::cerr << "(@" << std::hex << reinterpret_cast<std::intptr_t>(bytes)
              << std::dec << ")\n";

    int exp_msb = bytes[0] & 0x7f;
    int exp_lsb = (bytes[1] & 0xF0) >> 4;
    int exp = ((exp_msb << 4) | exp_lsb) - 1023;

    if (exp == -1023) {
      exponent_excess_128 = 0;
    } else {
      ++exp; // Implied 1 comes before the decimal in IEEE, after the decimal in
             // CBM.
      if (exp < -127)
        exp = -127;
      if (exp > 127)
        exp = 127;
      exponent_excess_128 = exp + 128;
    }

    int sign = ((bytes[0] & 0x80) == 0 ? 0 : 1) << 7;
    sign_and_mantissa_0 =
        sign | ((bytes[1] & 0xF) << 3) | ((bytes[2] & 0xE0) >> 5);
  }

  std::uint8_t exponent_excess_128;
  std::uint8_t sign_and_mantissa_0;
  std::uint8_t mantissa_1;
  std::uint8_t mantissa_2;
  std::uint8_t mantissa_3;
};

struct CBMUnpackedFloat {
  std::uint8_t exponent_excess_128;
  std::uint8_t mantissa_0;
  std::uint8_t mantissa_1;
  std::uint8_t mantissa_2;
  std::uint8_t mantissa_3;
  std::uint8_t sign;
};

} // namespace benscope::c64

#endif // __BENSCOPE_C64_CBM_FLOATS_H__