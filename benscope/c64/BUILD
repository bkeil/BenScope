load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "cbm_floats",
    hdrs = ["cbm_floats.h"],
)

cc_test(
    name = "cbm_floats_test",
    srcs = ["cbm_floats_test.cc"],
    deps = [
        ":cbm_floats",
        "@com_google_googletest//:gtest_main",
    ],
)
