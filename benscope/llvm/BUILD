load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "KaleidoscopeJIT",
    srcs = ["KaleidoscopeJIT.cc"],
    hdrs = ["KaleidoscopeJIT.h"],
    deps = [
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:ExecutionEngine",
        "@llvm-project//llvm:OrcJIT",
        "@llvm-project//llvm:Support",
        "@llvm-project//llvm:Target",
    ],
)

cc_library(
    name = "codegen",
    hdrs = ["codegen.h"],
    deps = ["@llvm-project//llvm:Core"],
)

cc_library(
    name = "environment",
    srcs = ["environment.cc"],
    hdrs = ["environment.h"],
    deps = [
        "//benscope/parsing:ast",
        "@com_google_absl//absl/container:flat_hash_map",
        "@llvm-project//llvm:Core",
    ],
)

cc_binary(
    name = "driver",
    srcs = ["driver.cc"],
    linkopts = [
        "-ldl",
        "-lm",
        "-pthread",
    ],
    deps = [
        ":KaleidoscopeJIT",
        ":codegen",
        ":environment",
        "//benscope/parsing:lexer",
        "//benscope/parsing:parser",
        "@com_google_absl//absl/strings",
        "@llvm-project//llvm:InstCombine",
        "@llvm-project//llvm:Scalar",
        "@llvm-project//llvm:X86AsmParser",
        "@llvm-project//llvm:X86CodeGen",
    ],
)
