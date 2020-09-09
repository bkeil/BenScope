#ifndef __BENSCOPE_C64_CBM_FLOATS_H__
#define __BENSCOPE_C64_CBM_FLOATS_H__

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace benscope::c64 {

using byte = std::uint8_t;
using DoubleBytes = std::array<byte, sizeof(double)>;

byte exponent(const DoubleBytes &bytes);
byte sign(const DoubleBytes &bytes);
byte mantissa(const DoubleBytes &bytes, int index);

DoubleBytes GetBytes(double val) {
  DoubleBytes bytes;
  std::memcpy(bytes.data(), &val, sizeof(double));
  std::reverse(std::begin(bytes), std::end(bytes));
  return bytes;
}

struct CBMPackedFloat {
  explicit CBMPackedFloat(double val) {
    DoubleBytes bytes = GetBytes(val);
    exponent_excess_128 = exponent(bytes);
    if (exponent_excess_128 == 0) return;

    sign_and_mantissa_0 = (sign(bytes) & 0x80) | mantissa(bytes, 0);
    mantissa_1 = mantissa(bytes, 1);
    mantissa_2 = mantissa(bytes, 2);
    mantissa_3 = mantissa(bytes, 3);
  }

  byte exponent_excess_128 = 0;
  byte sign_and_mantissa_0 = 0;
  byte mantissa_1 = 0;
  byte mantissa_2 = 0;
  byte mantissa_3 = 0;
};

struct CBMUnpackedFloat {
  explicit CBMUnpackedFloat(double val) {
    DoubleBytes bytes = GetBytes(val);
    exponent_excess_128 = exponent(bytes);
    if (exponent_excess_128 == 0) return;

    sign = ::benscope::c64::sign(bytes);
    mantissa_0 = (0x80) | mantissa(bytes, 0);
    mantissa_1 = mantissa(bytes, 1);
    mantissa_2 = mantissa(bytes, 2);
    mantissa_3 = mantissa(bytes, 3);
  }

  byte exponent_excess_128 = 0;
  byte mantissa_0 = 0;
  byte mantissa_1 = 0;
  byte mantissa_2 = 0;
  byte mantissa_3 = 0;
  byte sign = 0;
};

} // namespace benscope::c64

#endif // __BENSCOPE_C64_CBM_FLOATS_H__