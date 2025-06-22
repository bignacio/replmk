#include <doctest/doctest.h>


// Include the header we're testing
#include "../src/REPLMaker.h"

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)

TEST_SUITE_BEGIN("REPLMaker");


// Test command catalog creation
TEST_CASE("Command catalog creation") {
    replmk::ReplDefinition cmdDef;
    auto catalog = makeExternalCommandCatalog(cmdDef);
    REQUIRE(catalog.empty());
}

// Test command modifiers
TEST_CASE("Command modifiers") {
    replmk::ReplDefinition cmdDef;
    auto modifiers = makeCommandModifiers(cmdDef);
    REQUIRE(modifiers.empty());
}
TEST_SUITE_END();
//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
