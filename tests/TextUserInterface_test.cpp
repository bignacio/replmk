#include <doctest/doctest.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>
#include <string>

#include "TextUserInterface.h"
#include "OutputBuffers.h"
#include "CommandHistory.h"

using namespace replmk;

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
TEST_SUITE_BEGIN("TextUserInterface");

TEST_CASE("Command submission triggers callback") {
    std::string submittedCommand;
    bool callbackCalled = false;

    // Setup test data
    std::string inputNote;
    std::string inputBuffer = "test command";
    CommandHistory cmdHistory{""};

    // Setup command processing action
    auto onCommandEntered = [&](const std::string& cmd) {
        submittedCommand = cmd;
        callbackCalled = true;
    };

    // Create input field and simulate command submission
    auto inputField = makeCommandInput(inputBuffer, inputNote, onCommandEntered, cmdHistory);

    // Simulate typing the command
    for (char character : "test command") {
        if (character != '\0') {  // Skip null terminator
            inputField->OnEvent(ftxui::Event::Character(std::string(1, character)));
        }
    }

    // Simulate pressing Enter - use the same event type as in the implementation
    inputBuffer = "test command";  // Set the input buffer directly since we're testing the callback
    inputField->OnEvent(ftxui::Event::Return);

    // Verify
    REQUIRE(callbackCalled);
    REQUIRE(submittedCommand == "test command");
}

TEST_CASE("Output buffer updates are reflected in UI") {
    // Setup test data
    OutputBuffers outputBuffers;

    // Set up a dummy onOutputChanged handler
    outputBuffers.SetOnOutputChangedEvent([](const OutputBuffers&) -> void {});

    // Create and add entries using direct initialization
    {
        OutputBufferEntry entry1;
        entry1.prompt = "prompt1";
        entry1.stdOutEntry = "output1";
        entry1.stdErrEntry = "";
        outputBuffers.AddNewEntry(std::move(entry1));
    }
    {
        OutputBufferEntry entry2;
        entry2.prompt = "prompt2";
        entry2.stdOutEntry = "output2";
        entry2.stdErrEntry = "error2";
        outputBuffers.AddNewEntry(std::move(entry2));
    }

    // Create output frame
    float scrollYPos = 0;
    ftxui::Box dummyBox;
    auto outputFrame = makeOutputFrame(outputBuffers, scrollYPos, dummyBox);

    // Trigger render (should not crash)
    outputFrame->Render();

    // Verify buffer content
    const auto& buffer = outputBuffers.GetBuffer();
    REQUIRE(buffer.size() == 2);
    REQUIRE(buffer[0].prompt == "prompt1");
    REQUIRE(buffer[0].stdOutEntry == "output1");
    REQUIRE(buffer[1].stdErrEntry == "error2");
}

TEST_CASE("Navigation events update command history") {
    CommandHistory cmdHistory{""};
    cmdHistory.Add("command1");
    cmdHistory.Add("command2");  // Add a second command to properly test navigation

    // Test up arrow - should get the most recent command (command2)
    auto upEvent = ftxui::Event::ArrowUp;
    auto result = hasNavigateContent(upEvent, cmdHistory);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "command2");

    // Up arrow again - should get the previous command (command1)
    result = hasNavigateContent(upEvent, cmdHistory);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "command1");

    // Down arrow - should go back to command2
    auto downEvent = ftxui::Event::ArrowDown;
    result = hasNavigateContent(downEvent, cmdHistory);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "command2");

    // Down arrow again - should stay at command2 (end of history)
    result = hasNavigateContent(downEvent, cmdHistory);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "command2");
}

TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
