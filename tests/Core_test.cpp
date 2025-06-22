#include <doctest/doctest.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "../src/Core.h"
#include "../src/OutputBuffers.h"
#include "../src/Command.h"
#include "../src/CommandHistory.h"
#include "../src/OutputHistory.h"

using namespace replmk;

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
TEST_SUITE_BEGIN("Core");

namespace {
// Helper function to create a test command
auto CreateTestCommand(CommandType type, std::string name, std::string description, std::string exec = "") -> Command {
    return Command{
        .cmdType = type,
        .name = std::move(name),
        .description = std::move(description),
        .exec = std::move(exec)
    };
}
}

TEST_CASE("buildInternalCommandCatalog creates default commands") {
    REPLModifiers emptyModifiers;
    const auto catalog = buildInternalCommandCatalog(emptyModifiers);

    REQUIRE_NE(catalog.find("help"), catalog.end());
    REQUIRE_NE(catalog.find("exit"), catalog.end());
    REQUIRE_EQ(catalog.at("help").cmdType, CommandType::InternalHelp);
    REQUIRE_EQ(catalog.at("exit").cmdType, CommandType::InternalExit);
}

TEST_CASE("buildInternalCommandCatalog respects custom command names") {
    REPLModifiers customModifiers{
        {definition::AltHelpCmdNameLabel, "assist"},
        {definition::AltExitCmdNameLabel, "quit"}
    };

    const auto catalog = buildInternalCommandCatalog(customModifiers);

    REQUIRE_NE(catalog.find("assist"), catalog.end());
    REQUIRE_NE(catalog.find("quit"), catalog.end());
    REQUIRE_EQ(catalog.find("help"), catalog.end());
    REQUIRE_EQ(catalog.find("exit"), catalog.end());
}

TEST_CASE("resolveCommandLine finds external commands") {
    CommandCatalog external{
        {"echo", CreateTestCommand(CommandType::Single, "echo", "echo command", "echo")}
    };
    CommandCatalog internal{
        {"help", CreateTestCommand(CommandType::InternalHelp, "help", "help command")}
    };

    const auto result = resolveCommandLine(external, internal, "echo hello world");
    REQUIRE(result.has_value());
    const auto& [cmd, args] = result.value();
    REQUIRE_EQ(cmd.name, "echo");
    REQUIRE_EQ(args.size(), 2);
    REQUIRE_EQ(args[0], "hello");
    REQUIRE_EQ(args[1], "world");
}

TEST_CASE("resolveCommandLine finds internal commands") {
    CommandCatalog external{};
    CommandCatalog internal{
        {"help", CreateTestCommand(CommandType::InternalHelp, "help", "help command")}
    };

    const auto result = resolveCommandLine(external, internal, "help echo");
    REQUIRE(result.has_value());
    const auto& [cmd, args] = result.value();
    REQUIRE_EQ(cmd.name, "help");
    REQUIRE_EQ(args.size(), 1);
    REQUIRE_EQ(args[0], "echo");
}

TEST_CASE("executeSingleCommandLine executes command and captures output") {
    OutputBuffers outputBuffers;
    outputBuffers.AddNewEntry(OutputBufferEntry{"", "", ""});

    const auto cmd = CreateTestCommand(CommandType::Single, "echo", "echo command", "echo");
    const bool result = executeSingleCommandLine(cmd, {"hello", "world"}, outputBuffers);

    REQUIRE_EQ(result, true);
    const auto& lastOutput = outputBuffers.GetBuffer().back();
    REQUIRE_EQ(lastOutput.stdOutEntry, "hello world\n");
    REQUIRE_EQ(lastOutput.stdErrEntry, "");
}

TEST_CASE("executeShellScriptCommand executes shell script") {
    OutputBuffers outputBuffers;
    outputBuffers.AddNewEntry(OutputBufferEntry{"", "", ""});

    const auto cmd = CreateTestCommand(CommandType::Shell, "test", "test script", "echo $1 $2");
    const bool result = executeShellScriptCommand(cmd, {"hello", "world"}, outputBuffers);

    REQUIRE_EQ(result, true);
    const auto& lastOutput = outputBuffers.GetBuffer().back();
    REQUIRE_EQ(lastOutput.stdOutEntry, "hello world\n");
    REQUIRE_EQ(lastOutput.stdErrEntry, "");
}

TEST_CASE("executeCommandLine handles script command type") {
    CommandCatalog external{
        {"test", CreateTestCommand(CommandType::Script, "test", "test script", "echo")}
    };
    CommandCatalog internal{};
    OutputBuffers outputBuffers;
    outputBuffers.AddNewEntry(OutputBufferEntry{"", "", ""});

    bool eventHandlerCalled = false;
    const OnInternalCommandEvent eventHandler = [&](CommandType /*type*/) -> void { eventHandlerCalled = true; };

    const bool result = executeCommandLine(external, internal, outputBuffers, "test hello", eventHandler);

    // Script type is not yet implemented, should return false
    REQUIRE_EQ(result, false);
    REQUIRE_EQ(eventHandlerCalled, false);
}

TEST_CASE("handleHelpDisplay shows command specific help") {
    CommandCatalog external{
        {"echo", CreateTestCommand(CommandType::Single, "echo", "echo command", "echo")}
    };
    CommandCatalog internal{
        {"help", CreateTestCommand(CommandType::InternalHelp, "help", "help command")}
    };

    OutputBuffers outputBuffers;
    handleHelpDisplay({"echo"}, external, internal, outputBuffers);

    const auto& lastOutput = outputBuffers.GetBuffer().back();
    REQUIRE_NE(lastOutput.stdOutEntry.find("echo command"), std::string::npos);
    REQUIRE_EQ(lastOutput.stdErrEntry, "");
}

TEST_CASE("handleInternalCommands processes help command") {
    CommandCatalog external{
        {"echo", CreateTestCommand(CommandType::Single, "echo", "echo command", "echo")}
    };
    CommandCatalog internal{
        {"help", CreateTestCommand(CommandType::InternalHelp, "help", "help command")}
    };

    OutputBuffers outputBuffers;
    bool eventHandlerCalled = false;
    const OnInternalCommandEvent eventHandler = [&](CommandType type) -> void {
        eventHandlerCalled = true;
        REQUIRE_EQ(type, CommandType::InternalHelp);
    };

    const bool result = handleInternalCommands(
                            internal.at("help"), {"echo"}, external, internal, eventHandler, outputBuffers
                        );

    REQUIRE_EQ(result, true);
    REQUIRE_EQ(eventHandlerCalled, true);
    REQUIRE_NE(outputBuffers.GetBuffer().back().stdOutEntry.find("echo command"), std::string::npos);
}

TEST_CASE("handleInternalCommands processes exit command") {
    CommandCatalog external{};
    CommandCatalog internal{
        {"exit", CreateTestCommand(CommandType::InternalExit, "exit", "exit command")}
    };

    OutputBuffers outputBuffers;
    bool eventHandlerCalled = false;
    const OnInternalCommandEvent eventHandler = [&](CommandType type) -> void {
        eventHandlerCalled = true;
        REQUIRE_EQ(type, CommandType::InternalExit);
    };

    const bool result = handleInternalCommands(
                            internal.at("exit"), {}, external, internal, eventHandler, outputBuffers
                        );

    REQUIRE_EQ(result, true);
    REQUIRE_EQ(eventHandlerCalled, true);
}

TEST_CASE("MapGetOrDefault returns mapped or default with convertible key") {
    std::unordered_map<std::string, std::string> testMap{{"a","1"}};
    auto foundValue = MapGetOrDefault(testMap, "a", std::string("x"));
    auto defaultValue = MapGetOrDefault(testMap, "b", std::string("x"));
    REQUIRE_EQ(foundValue, std::string("1"));
    REQUIRE_EQ(defaultValue, std::string("x"));
}

TEST_CASE("Help command lists commands and internal help/exit") {
    CommandCatalog externalCommands{
        {"echo", Command{.cmdType=CommandType::Single, .name="echo", .description="print", .exec="echo"}}
    };
    REPLModifiers modifiers{};

    OutputBuffers outputBuffers;
    CommandHistory commandHistory("");
    OutputHistory outputHistory("");

    auto processCommand = makeCommandProcessingAction(externalCommands, modifiers, outputBuffers, commandHistory, outputHistory);

    bool wasEventHandlerCalled = false;
    CommandType receivedCommandType = CommandType::Unknown;
    const auto eventHandler = [&](CommandType commandType) {
        wasEventHandlerCalled = true;
        receivedCommandType = commandType;
    };

    const bool commandSucceeded = processCommand("help", eventHandler);
    REQUIRE_EQ(commandSucceeded, true);
    const auto& outputBuffer = outputBuffers.GetBuffer();
    REQUIRE_NE(outputBuffer.empty(), true);
    const auto& lastOutput = outputBuffer.back();
    REQUIRE_EQ(lastOutput.stdErrEntry.empty(), true);
    REQUIRE_NE(lastOutput.stdOutEntry.find("Available commands:"), std::string::npos);
    REQUIRE_NE(lastOutput.stdOutEntry.find("echo"), std::string::npos);
    REQUIRE_NE(lastOutput.stdOutEntry.find("help"), std::string::npos);
    REQUIRE_NE(lastOutput.stdOutEntry.find("exit"), std::string::npos);
    REQUIRE_EQ(wasEventHandlerCalled, true);
    REQUIRE_EQ(receivedCommandType, CommandType::InternalHelp);
}

TEST_CASE("Unknown command appends error to last stderr with help hint") {
    CommandCatalog externalCommands{};
    REPLModifiers modifiers{};

    OutputBuffers outputBuffers;
    outputBuffers.AddNewEntry(OutputBufferEntry{.prompt="> ", .stdOutEntry="", .stdErrEntry=""});

    CommandHistory commandHistory("");
    OutputHistory outputHistory("");

    auto processCommand = makeCommandProcessingAction(externalCommands, modifiers, outputBuffers, commandHistory, outputHistory);

    bool wasEventHandlerCalled = false;
    const auto eventHandler = [&](CommandType) {
        wasEventHandlerCalled = true;
    };

    const bool commandSucceeded = processCommand("does_not_exist", eventHandler);
    REQUIRE_EQ(commandSucceeded, false);
    const auto& outputBuffer = outputBuffers.GetBuffer();
    REQUIRE_NE(outputBuffer.empty(), true);
    const auto& lastOutput = outputBuffer.back();
    REQUIRE_EQ(lastOutput.stdOutEntry.empty(), true);
    REQUIRE_NE(lastOutput.stdErrEntry.find("Could not find the command"), std::string::npos);
    REQUIRE_NE(lastOutput.stdErrEntry.find("help"), std::string::npos);
    REQUIRE_EQ(wasEventHandlerCalled, false);
}

TEST_CASE("Execute external single command appends to stdout of last entry") {
    CommandCatalog externalCommands{
        {"echo", Command{.cmdType=CommandType::Single, .name="echo", .description="print", .exec="echo"}}
    };
    REPLModifiers modifiers{};

    OutputBuffers outputBuffers;
    outputBuffers.AddNewEntry(OutputBufferEntry{.prompt="> ", .stdOutEntry="", .stdErrEntry=""});

    CommandHistory commandHistory("");
    OutputHistory outputHistory("");

    auto processCommand = makeCommandProcessingAction(externalCommands, modifiers, outputBuffers, commandHistory, outputHistory);

    const bool commandSucceeded = processCommand("echo hello", [](CommandType) {});
    REQUIRE_EQ(commandSucceeded, true);
    const auto& lastOutput = outputBuffers.GetBuffer().back();
    REQUIRE_EQ(lastOutput.stdErrEntry.empty(), true);
    REQUIRE_EQ(lastOutput.stdOutEntry, "hello\n");
}

TEST_CASE("Internal exit triggers event and returns true without modifying buffer") {
    CommandCatalog externalCommands{};
    REPLModifiers modifiers{};
    OutputBuffers outputBuffers;

    const auto initialBufferSize = outputBuffers.GetBuffer().size();

    CommandHistory commandHistory("");
    OutputHistory outputHistory("");

    auto processCommand = makeCommandProcessingAction(externalCommands, modifiers, outputBuffers, commandHistory, outputHistory);

    bool wasEventHandlerCalled = false;
    CommandType receivedCommandType = CommandType::Unknown;
    const bool commandSucceeded = processCommand("exit", [&](CommandType commandType) {
        wasEventHandlerCalled = true;
        receivedCommandType = commandType;
    });
    REQUIRE(commandSucceeded);
    REQUIRE(wasEventHandlerCalled);
    REQUIRE_EQ(receivedCommandType, CommandType::InternalExit);
    REQUIRE_EQ(outputBuffers.GetBuffer().size(), initialBufferSize);
}

TEST_SUITE_END();
//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
