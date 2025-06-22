#include "Core.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <utility>
#include <optional>
#include <format>

#include "Command.h"
#include "CommandLineParser.h"
#include "OutputHistory.h"
#include "ProcessExecutor.h"
#include "AutoCleanableScriptFile.h"
#include "CommandHistory.h"

namespace replmk {

[[nodiscard]]
auto buildInternalCommandCatalog(const REPLModifiers& modifiers) -> CommandCatalog {
    const auto helpCmd = Command{
        .cmdType = CommandType::InternalHelp,
        .name = MapGetOrDefault(modifiers, definition::AltHelpCmdNameLabel, definition::DefaultHelpKeyword),
        .description= MapGetOrDefault(modifiers, definition::AltHelpCmdDescLabel, "Type 'help' to see a list of all commands or 'help <command>' for more details on a specific command."),
        .exec = "", // no exec for internal commands
    };

    const auto exitCmd = Command{
        .cmdType = CommandType::InternalExit,
        .name = MapGetOrDefault(modifiers, definition::AltExitCmdNameLabel, "exit"),
        .description= MapGetOrDefault(modifiers, definition::AltExitCmdDescLabel, "Exit the application"),
        .exec = "",
    };

    return {
        {helpCmd.name, helpCmd,},
        {exitCmd.name, exitCmd,}
    };
}

[[nodiscard]]
auto resolveCommandLine(const CommandCatalog& externalCommands, const CommandCatalog& internalCommands,
                        std::string_view fullCommandLine) -> std::optional<ResolvedCommand> {//NOLINT(bugprone-easily-swappable-parameters)

    const auto maybeParsed = ParseCommandLine(std::string(fullCommandLine));
    if(not maybeParsed.has_value()) {
        return std::nullopt;
    }

    const auto& cmdAndArgs = maybeParsed.value();
    if(cmdAndArgs.empty()) {
        return std::nullopt;
    }

    const auto& cmdName = cmdAndArgs.at(0);
    const auto argsOnly = std::vector<std::string>(std::next(cmdAndArgs.begin()), cmdAndArgs.end());

    const auto maybeExternalCmd = externalCommands.find(cmdName);
    if(maybeExternalCmd != externalCommands.end()) {
        return std::make_tuple(maybeExternalCmd->second, argsOnly);
    }

    const auto maybeInternalCmd = internalCommands.find(cmdName);
    if(maybeInternalCmd != internalCommands.end()) {
        return std::make_tuple(maybeInternalCmd->second, argsOnly);
    }

    return {};
}

auto handleHelpDisplay(const std::vector<std::string>& args,
                       const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, //NOLINT(bugprone-easily-swappable-parameters)
                       OutputBuffers& outBuffers) ->void {


    static constexpr std::string CommandFormatString = "- {}:  {}\n";
    std::string outStr;

    if(not args.empty()) {
        const auto& commandName = args.at(0);
        const auto maybeTargetCommand = externalCommands.find(commandName);
        if(maybeTargetCommand != externalCommands.end()) {
            outBuffers.AddNewEntry({
                .prompt = "",
                .stdOutEntry = std::format(CommandFormatString, maybeTargetCommand->second.name, maybeTargetCommand->second.description),
                .stdErrEntry = ""
            });
            return;
        }

        outStr = std::format("Command '{}' not found\n", commandName);
    }

    outStr.append("Available commands:\n");

    for(const auto& [name, cmd]: externalCommands) {
        outStr.append(std::format(CommandFormatString, name, cmd.description));
    }
    outStr.append("----\n");
    for(const auto& [name, cmd]: internalCommands) {
        outStr.append(std::format(CommandFormatString, name, cmd.description));
    }

    outBuffers.AddNewEntry({
        .prompt = "",
        .stdOutEntry = outStr,
        .stdErrEntry = ""
    });
}

[[nodiscard]]
auto handleInternalCommands(const Command& command, const std::vector<std::string>& args,
                            const CommandCatalog& externalCommands,
                            const CommandCatalog& internalCommands,
                            const OnInternalCommandEvent& onInternalCmd,
                            OutputBuffers& outBuffers) -> bool {


    if (command.cmdType == CommandType::InternalHelp) {
        handleHelpDisplay(args, externalCommands, internalCommands, outBuffers);
        onInternalCmd(command.cmdType);
        return true;
    }

    if (command.cmdType == CommandType::InternalExit) {
        onInternalCmd(command.cmdType);
        return true;
    }

    return false;
}

[[nodiscard]]
auto executeSingleCommandLine(const Command& command, const std::vector<std::string>& args, OutputBuffers& outBuffers) -> bool {
    CommandOutputCallbacks callbacks{
        .onStdOut = [&outBuffers](std::string_view chunk) {
            outBuffers.AppendToLastStdOutEntry(chunk);
        },
        .onStdErr = [&outBuffers](std::string_view chunk) {
            outBuffers.AppendToLastStdErrEntry(chunk);
        }
    };
    return executeAndCaptureOutputs(command.exec, args, callbacks);
}

[[nodiscard]]
auto executeShellScriptCommand(const Command& command, const std::vector<std::string>& args, OutputBuffers& outBuffers) -> bool {
    CommandOutputCallbacks callbacks{
        .onStdOut = [&outBuffers](std::string_view chunk) {
            outBuffers.AppendToLastStdOutEntry(chunk);
        },
        .onStdErr = [&outBuffers](std::string_view chunk) {
            outBuffers.AppendToLastStdErrEntry(chunk);
        }
    };

    const auto maybeScriptPath = io::MakeUniqueTempScriptFilePath();

    if(not maybeScriptPath.has_value()) {
        return false;
    }

    io::AutoCleanableScriptFile scriptFileGenerator;
    const auto& scriptPath = maybeScriptPath.value();
    if(scriptFileGenerator.WriteScript(scriptPath, command.exec)) {
        return executeAndCaptureOutputs(scriptPath.string(), args, callbacks);
    }

    return false;
}

auto executeCommandLine(const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, OutputBuffers& outBuffers,
                        std::string_view fullCommandLine, const OnInternalCommandEvent& onInternalCmd) -> bool {

    const auto maybeCmdAndArgs = resolveCommandLine(externalCommands, internalCommands, fullCommandLine);

    if(not maybeCmdAndArgs.has_value()) {
        const auto& foundIter = std::ranges::find_if(internalCommands, [](const auto& cmd) -> bool {
            return cmd.second.cmdType == CommandType::InternalHelp;
        });

        const auto helpCmdName = [&foundIter, &internalCommands]() -> std::string {
            if(foundIter == internalCommands.end()) {
                return definition::DefaultHelpKeyword;
            }
            return foundIter->second.name;
        }();

        outBuffers.AppendToLastStdErrEntry(
            std::format("Could not find the command '{}'. Type '{}' to see available commands",
                        fullCommandLine, helpCmdName));
        return false;
    }

    const auto& [command, args] = maybeCmdAndArgs.value();

    if (command.cmdType == CommandType::Single) {
        return executeSingleCommandLine(command, args, outBuffers);
    }

    if (command.cmdType == CommandType::Shell) {
        return executeShellScriptCommand(command, args, outBuffers);
    }

    if (command.cmdType == CommandType::Script) {
        // not yet supported
        return false;
    }

    // else, handle internal commands
    return handleInternalCommands(command, args, externalCommands, internalCommands, onInternalCmd, outBuffers);
}

auto makeCommandProcessingAction(const CommandCatalog& externalCommands, const REPLModifiers& modifiers, OutputBuffers& outBuffers,
                                 CommandHistory& cmdHistory, OutputHistory& outputHistory) -> CommandProcessingAction {
    const auto internalCommands = buildInternalCommandCatalog(modifiers);

    return [externalCommands, internalCommands, &outBuffers, &cmdHistory, &outputHistory](std::string_view fullCommandLine, const OnInternalCommandEvent& onInternalCmd) -> bool {

        cmdHistory.Add(fullCommandLine);
        cmdHistory.Save();
        const bool execResult = executeCommandLine(externalCommands, internalCommands, outBuffers, fullCommandLine, onInternalCmd);

        if(not outputHistory.Save(outBuffers)) {
            // do nothing
        }
        return execResult;
    };
}

}
