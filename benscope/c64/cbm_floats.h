#ifndef __BENSCOPE_C64_CBM_FLOATS_H__
#define __BENSCOPE_C64_CBM_FLOATS_H__

#include <cstdint>

namespace benscope::c64 {

using byte = std::uint8_t;

struct CBMPackedFloat {
  explicit CBMPackedFloat(double val);
  byte exponent_excess_128 = 0;
  byte sign_and_mantissa_0 = 0;
  byte mantissa_1 = 0;
  byte mantissa_2 = 0;
  byte mantissa_3 = 0;
};

struct CBMUnpackedFloat {
  explicit CBMUnpackedFloat(double val);
  byte exponent_excess_128 = 0;
  byte mantissa_0 = 0;
  byte mantissa_1 = 0;
  byte mantissa_2 = 0;
  byte mantissa_3 = 0;
  byte sign = 0;
};

} // namespace benscope::c64

#endif // __BENSCOPE_C64_CBM_FLOATS_H__