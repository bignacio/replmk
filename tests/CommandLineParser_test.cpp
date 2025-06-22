//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while,bugprone-unchecked-optional-access)

#include <doctest/doctest.h>

#include "../src/CommandLineParser.h"

using namespace replmk;

TEST_SUITE_BEGIN("ParseCommandLine");

TEST_CASE("simple unquoted words") {
    auto result = ParseCommandLine("cmd arg1 arg2");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1");
    REQUIRE(args[2] == "arg2");
}

TEST_CASE("single quoted argument") {
    auto result = ParseCommandLine("cmd 'arg1 arg2'");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1 arg2");
}

TEST_CASE("double quoted argument with spaces") {
    auto result = ParseCommandLine("cmd \"arg1 arg2\"");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1 arg2");
}

TEST_CASE("double quoted argument with escaped chars") {
    auto result = ParseCommandLine(R"(cmd "arg1 \"arg2\"")");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1 \"arg2\"");
}

TEST_CASE("single quote inside double quote") {
    auto result = ParseCommandLine(R"(cmd 'arg1' 'long arg2')");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1");
    REQUIRE(args[2] == "long arg2");
}

TEST_CASE("escaped space") {
    auto result = ParseCommandLine(R"(arg1\ arg2 arg3)");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "arg1 arg2");
    REQUIRE(args[1] == "arg3");
}

TEST_CASE("mixed quoting and escaping") {
    auto result = ParseCommandLine(R"(cmd "arg1 arg2" 'arg3\'s' arg4\ arg5)");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.size() == 4);
    REQUIRE(args[0] == "cmd");
    REQUIRE(args[1] == "arg1 arg2");
    REQUIRE(args[2] == "arg3's");
    REQUIRE(args[3] == "arg4 arg5");
}

TEST_CASE("empty input") {
    auto result = ParseCommandLine("");
    REQUIRE(result.has_value());
    const auto& args = result.value();
    REQUIRE(args.empty());
}

TEST_CASE("invalid input (unclosed quote)") {
    auto result = ParseCommandLine("cmd 'arg1");
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("invalid input (invalid escape)") {
    auto result = ParseCommandLine("cmd \\");
    REQUIRE_FALSE(result.has_value());
}

TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while,bugprone-unchecked-optional-access)
