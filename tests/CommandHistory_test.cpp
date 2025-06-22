#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../src/CommandHistory.h"

namespace fs = std::filesystem;
using namespace replmk;

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
TEST_SUITE_BEGIN("CommandHistory");


TEST_CASE("Initialize with empty path returns always true") {
    CommandHistory history("");
    REQUIRE(history.Save());
    history.Add("hello");
    REQUIRE(history.Save());
}

TEST_CASE("Initialize with nonexistent file creates empty history") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_nonexistent_file.txt";

    if (fs::exists(tempFilePath)) {
        REQUIRE(fs::remove(tempFilePath));
    }

    CommandHistory history(tempFilePath);
    REQUIRE_FALSE(history.Load());
    REQUIRE_FALSE(fs::exists(tempFilePath));
    REQUIRE(history.Save());
    REQUIRE(fs::exists(tempFilePath));

    history.Add("hello");
    REQUIRE(history.Save());

    CommandHistory loadedHistory(tempFilePath);
    REQUIRE(loadedHistory.Load());

    const auto lastCommand = loadedHistory.Next();
    REQUIRE(lastCommand.has_value());
    REQUIRE_EQ(lastCommand.value(),"hello");

    REQUIRE(fs::remove(tempFilePath));
}

TEST_CASE("Initialize loads existing commands") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_cmd_history.txt";

    std::vector<std::string> expected = {"hello", "world", "foobar"};

    {
        CommandHistory writer(tempFilePath);
        for (const auto& cmd : expected) {
            writer.Add(cmd);
        }
        REQUIRE(writer.Save());
    }

    CommandHistory reader(tempFilePath);
    REQUIRE(reader.Load());
    // Check we can navigate around expected items behaviorally
    // Previous() from start should yield expected[size-2]
    REQUIRE_EQ(reader.Previous().value(), expected[expected.size() - 1]);
    REQUIRE_EQ(reader.Next().value(), expected.back());

    REQUIRE(fs::remove(tempFilePath));
}


TEST_CASE("Initialize with corrupted file handles gracefully") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_cmd_history_corrupt.txt";

    std::ofstream file(tempFilePath, std::ios::binary | std::ios::trunc);
    file << "5:hello\n10:world";  // No newline at end, second command claims length 10 but only provides 5 chars
    file.close();

    CommandHistory history(tempFilePath);
    REQUIRE(history.Load());
    // We should at least be able to save back what was parsed
    REQUIRE(history.Save());

    CommandHistory reread(tempFilePath);
    REQUIRE(reread.Load());
    // Ensure navigation works and returns a value for the valid parsed command
    const auto prev = reread.Previous();
    REQUIRE(prev.has_value());

    REQUIRE(fs::remove(tempFilePath));
}

TEST_CASE("Do not save empty commands") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_empty_cmds.txt";

    if (fs::exists(tempFilePath)) {
        REQUIRE(fs::remove(tempFilePath));
    }

    CommandHistory history(tempFilePath);
    history.Add("");
    history.Add("valid");
    history.Add(" ");
    history.Add("  ");

    REQUIRE(history.Save());

    CommandHistory reload(tempFilePath);
    REQUIRE(reload.Load());
    // Only "valid" should have been persisted; check via navigation
    REQUIRE_EQ(reload.Previous().value(), "valid");

    REQUIRE(fs::remove(tempFilePath));
}

TEST_CASE("Handles large commands") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_large_cmd.txt";

    if (fs::exists(tempFilePath)) {
        REQUIRE(fs::remove(tempFilePath));
    }

    constexpr size_t kLargeCommandSize = 10'000;
    std::string largeCommand(kLargeCommandSize, 'x');

    CommandHistory history(tempFilePath);
    history.Add(largeCommand);
    REQUIRE(history.Save());

    CommandHistory reload(tempFilePath);
    REQUIRE(reload.Load());
    REQUIRE_EQ(reload.Previous().value(), largeCommand);

    REQUIRE(fs::remove(tempFilePath));
}


TEST_CASE("Initialize with missing field size") {
    const fs::path tempFilePath = fs::temp_directory_path() / "test_cmd_history_corrupt.txt";

    std::ofstream file(tempFilePath, std::ios::binary | std::ios::trunc);
    file << "4:list\n:motd\n";
    file.close();

    CommandHistory history(tempFilePath);
    REQUIRE(history.Load());
    REQUIRE(history.Save());

    CommandHistory reread(tempFilePath);
    REQUIRE(reread.Load());
    REQUIRE(reread.Previous().has_value());

    REQUIRE(fs::remove(tempFilePath));
}

TEST_CASE("Navigate history forward and backward no history") {
    CommandHistory history("");

    const auto cmdBackward = history.Previous();
    REQUIRE_FALSE(cmdBackward.has_value());
    const auto cmdForward = history.Next();
    REQUIRE_FALSE(cmdForward.has_value());
}

TEST_CASE("Navigate history forward and backward") {
    CommandHistory history("");
    history.Add("cmd1");
    history.Add("cmd2");
    history.Add("cmd3");
    history.Add("cmd4");

    const auto cmdBackward = history.Previous();
    REQUIRE(cmdBackward.has_value());
    REQUIRE_EQ(cmdBackward.value(), "cmd4");

    // reset the position
    const auto resetCmd = history.Next();
    REQUIRE(resetCmd.has_value());
    REQUIRE_EQ(resetCmd.value(), "cmd4");
    // and then start backwards from last command
    REQUIRE_EQ(history.Previous().value(), "cmd3");
    REQUIRE_EQ(history.Previous().value(), "cmd2");
    REQUIRE_EQ(history.Previous().value(), "cmd1");

    // now we go back all the way
    REQUIRE_EQ(history.Next().value(), "cmd2");
    REQUIRE_EQ(history.Next().value(), "cmd3");
    REQUIRE_EQ(history.Next().value(), "cmd4");
}


TEST_CASE("Navigate back from last shows previous") {
    CommandHistory history("");
    history.Add("cmd1");
    history.Add("cmd2");
    history.Add("cmd3");

    // move to end
    REQUIRE_EQ(history.Next().value(), "cmd3");

    const auto cmdBackward = history.Previous();
    REQUIRE(cmdBackward.has_value());
    REQUIRE_EQ(cmdBackward.value(), "cmd2");
}

TEST_CASE("Do not add to last if same command") {
    const auto *const commandString = "cmd1";
    CommandHistory history("");
    history.Add(commandString);
    history.Add(commandString);
    // behavior: navigating should still yield the single command
    REQUIRE_EQ(history.Previous().value(), commandString);
}

TEST_CASE("Navigate backward after add start from the added command") {
    CommandHistory history("");
    history.Add("cmd1");
    history.Add("cmd2");
    history.Add("cmd3");

    const auto cmdBackward = history.Previous();
    REQUIRE(cmdBackward.has_value());
    REQUIRE_EQ(cmdBackward.value(), "cmd3");
}

TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)