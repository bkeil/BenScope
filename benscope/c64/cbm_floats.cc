#include "benscope/c64/cbm_floats.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace benscope::c64 {
namespace {
using DoubleBytes = std::array<byte, sizeof(double)>;

DoubleBytes GetBytes(double val);
byte exponent(const DoubleBytes &bytes);
byte sign(const DoubleBytes &bytes);
byte mantissa(const DoubleBytes &bytes, int index);

DoubleBytes GetBytes(double val) {
  DoubleBytes bytes;
  std::memcpy(bytes.data(), &val, sizeof(double));
  std::reverse(std::begin(bytes), std::end(bytes));
  return bytes;
}

byte exponent(const DoubleBytes &bytes) {
  int exp_msb = bytes[0] & 0x7f;
  int exp_lsb = (bytes[1] & 0xF0) >> 4;
  int exp = ((exp_msb << 4) | exp_lsb) - 1023;

  if (exp == -1023) {
    return 0;
  } else {
    ++exp; // Implied 1 comes before the decimal in IEEE, after the decimal in
           // CBM.
    if (exp < -127)
      exp = -127;
    if (exp > 127)
      exp = 127;

    return static_cast<byte>(exp + 128);
  }
}

byte sign(const DoubleBytes &bytes) {
  return (bytes[0] & 0x80) == 0 ? 0 : 0xFF;
}

byte mantissa(const DoubleBytes &bytes, int index) {
  const byte high_mask = index == 0 ? 0x0F : 0x1F;
  return ((bytes[index + 1] & high_mask) << 3) |
         ((bytes[index + 2] & 0xE0) >> 5);
}

} // namespace

CBMPackedFloat::CBMPackedFloat(double val) {
  DoubleBytes bytes = GetBytes(val);
  exponent_excess_128 = exponent(bytes);
  if (exponent_excess_128 == 0)
    return;

  sign_and_mantissa_0 = (sign(bytes) & 0x80) | mantissa(bytes, 0);
  mantissa_1 = mantissa(bytes, 1);
  mantissa_2 = mantissa(bytes, 2);
  mantissa_3 = mantissa(bytes, 3);
}

CBMUnpackedFloat::CBMUnpackedFloat(double val) {
  DoubleBytes bytes = GetBytes(val);
  exponent_excess_128 = exponent(bytes);
  if (exponent_excess_128 == 0)
    return;

  sign = ::benscope::c64::sign(bytes);
  mantissa_0 = (0x80) | mantissa(bytes, 0);
  mantissa_1 = mantissa(bytes, 1);
  mantissa_2 = mantissa(bytes, 2);
  mantissa_3 = mantissa(bytes, 3);
}

} // namespace benscope::c64