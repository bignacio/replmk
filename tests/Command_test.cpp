//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)

#include <doctest/doctest.h>
#include "Command.h"

using namespace replmk;

TEST_CASE("CommandType: toCommandType converts valid strings") {
    REQUIRE(toCommandType("shell") == CommandType::Shell);
    REQUIRE(toCommandType("script") == CommandType::Script);
    REQUIRE(toCommandType("not a type") == CommandType::Unknown);
}


//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
