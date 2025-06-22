#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "ProcessExecutor.h"
//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)

TEST_SUITE_BEGIN("ProcessExecutor");

TEST_CASE("Execute simple command successfully") {
    std::string capturedStdout;
    bool executionSucceeded = replmk::executeAndCaptureOutputs(
    "echo", {"hello"}, {
        .onStdOut = [&capturedStdout](std::string_view data) {
            capturedStdout.append(data);
        },
        .onStdErr = [](std::string_view) {}
    });

    REQUIRE(executionSucceeded);
    REQUIRE(capturedStdout == "hello\n");
}

TEST_CASE("Capture both stdout and stderr") {
    std::string capturedStdout;
    std::string capturedStderr;

    bool executionSucceeded = replmk::executeAndCaptureOutputs(
    "ls", {"--nonexistent-option"}, {
        .onStdOut = [&capturedStdout](std::string_view data) {
            capturedStdout.append(data);
        },
        .onStdErr = [&capturedStderr](std::string_view data) {
            capturedStderr.append(data);
        }
    });

    REQUIRE_FALSE(executionSucceeded);
    REQUIRE(capturedStdout.empty());
    REQUIRE_FALSE(capturedStderr.empty());
}

TEST_CASE("Handle command with arguments") {
    std::string capturedStdout;
    bool executionSucceeded = replmk::executeAndCaptureOutputs(
    "echo", {"hello", "world"}, {
        .onStdOut = [&](std::string_view data) {
            capturedStdout.append(data);
        },
        .onStdErr = [](std::string_view) {}
    });

    REQUIRE(executionSucceeded);
    REQUIRE(capturedStdout == "hello world\n");
}

TEST_CASE("Handle command not found") {
    bool executionSucceeded = replmk::executeAndCaptureOutputs(
    "nonexistent_command_12345", {}, {
        .onStdOut = [](std::string_view) {},
        .onStdErr = [](std::string_view) {}
    });

    REQUIRE_FALSE(executionSucceeded);
}

TEST_CASE("Handle command that exits with non-zero status") {
    bool executionSucceeded = replmk::executeAndCaptureOutputs(
    "false", {}, {
        .onStdOut = [](std::string_view) {},
        .onStdErr = [](std::string_view) {}
    });

    REQUIRE_FALSE(executionSucceeded);
}

TEST_SUITE_END();
//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)