load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "cbm_floats",
    srcs = ["cbm_floats.cc"],
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

cc_library(
    name = "codegen",
    srcs = ["codegen.cc"],
    hdrs = ["codegen.h"],
    deps = [
        ":cbm_floats",
        "//benscope/parsing:ast",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "bs64",
    srcs = ["bs64.cc"],
    deps = [
        ":codegen",
        "//benscope/parsing:ast",
        "//benscope/parsing:lexer",
        "//benscope/parsing:parser",
    ],
)
