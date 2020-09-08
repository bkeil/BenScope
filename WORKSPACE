workspace(name = "org_tensorflow")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

ABSL_COMMIT = "d0c433455801e1c1fb6f486f0b447e22f946ab52"

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-" + ABSL_COMMIT,
    urls = ["https://github.com/abseil/abseil-cpp/archive/{commit}.zip".format(commit = ABSL_COMMIT)],
)

# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/8567b09290fe402cf01923e2131c5635b8ed851b.zip"],  # 2020-06-12T22:24:28Z
    strip_prefix = "googletest-8567b09290fe402cf01923e2131c5635b8ed851b",
    sha256 = "9a8a166eb6a56c7b3d7b19dc2c946fe4778fd6f21c7a12368ad3b836d8f1be48",
)
