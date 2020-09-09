#include "benscope/c64/cbm_floats.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace benscope::c64 {
namespace {

using ::testing::Eq;

TEST(CbmFloatsTest, Exponents) {
  {
    CBMPackedFloat f(2.0);
    EXPECT_THAT(f.exponent_excess_128, Eq(128 + 2)); // 2^2 * 0.5
  }
  {
    CBMPackedFloat f(1.0);
    EXPECT_THAT(f.exponent_excess_128, Eq(128 + 1)); // 2^1 * 0.5
  }
  {
    CBMPackedFloat f(0.5);
    EXPECT_THAT(f.exponent_excess_128, Eq(128 + 0)); // 2^0 * 0.5
  }
  {
    CBMPackedFloat f(0.25);
    EXPECT_THAT(f.exponent_excess_128, Eq(128 - 1)); // 2^-1 * 0.5
  }
  {
    CBMPackedFloat f(0);
    EXPECT_THAT(f.exponent_excess_128, Eq(0)); // Special case for 0.
  }
}

TEST(CbmFloatsTest, Mantissa) {
  {
    CBMPackedFloat f(0.5);
    CBMUnpackedFloat g(0.5);
    EXPECT_THAT(f.sign_and_mantissa_0, Eq(0b00000000)); // Implicit 1 for 1/2.
    EXPECT_THAT(g.mantissa_0, Eq(0b10000000)); // Unpacked has explicit 1.
  }
  {
    CBMPackedFloat f(0.75);
    CBMUnpackedFloat g(0.75);
    EXPECT_THAT(f.sign_and_mantissa_0, Eq(0b01000000)); // Explicit 1 for 1/4.
    EXPECT_THAT(g.mantissa_0, Eq(0b11000000));
  }
  {
    CBMPackedFloat f(0.875);
    EXPECT_THAT(f.sign_and_mantissa_0, Eq(0b01100000)); // Another 1 to add 1/8.
  }
  {
    CBMPackedFloat f(0.9375);
    EXPECT_THAT(f.sign_and_mantissa_0, Eq(0b01110000)); // Add 1/16.
  }
  {
    CBMPackedFloat f(0.96875);
    EXPECT_THAT(f.sign_and_mantissa_0, Eq(0b01111000)); // Add 1/32.
  }
}

TEST(CbmFloatsTest, Zeros) {
  CBMPackedFloat packed(0);
  EXPECT_THAT(packed.exponent_excess_128, Eq(0));
  EXPECT_THAT(packed.sign_and_mantissa_0, Eq(0));
  EXPECT_THAT(packed.mantissa_1, Eq(0));
  EXPECT_THAT(packed.mantissa_2, Eq(0));
  EXPECT_THAT(packed.mantissa_3, Eq(0));

  CBMUnpackedFloat unpacked(0);
  EXPECT_THAT(unpacked.exponent_excess_128, Eq(0));
  EXPECT_THAT(unpacked.mantissa_0, Eq(0));
  EXPECT_THAT(unpacked.mantissa_1, Eq(0));
  EXPECT_THAT(unpacked.mantissa_2, Eq(0));
  EXPECT_THAT(unpacked.mantissa_3, Eq(0));
  EXPECT_THAT(unpacked.sign, Eq(0));
}

} // namespace
} // namespace benscope::c64
