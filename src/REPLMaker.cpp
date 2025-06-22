#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include <cxxopts.hpp>
#pragma GCC diagnostic pop
#else
#include <cxxopts.hpp>
#endif


#include "REPLMaker.h"
#include "Core.h"
#include "TextUserInterface.h"

auto makeExternalCommandCatalog(const replmk::ReplDefinition& definition) -> replmk::CommandCatalog {
    replmk::CommandCatalog catalog;

    for(const auto& cmd: definition.commands) {
        catalog.emplace(cmd.name, cmd);
    }

    return catalog;
}

auto makeCommandModifiers(const replmk::ReplDefinition& definition) -> replmk::REPLModifiers {
    replmk::REPLModifiers modifiers;

    if(not definition.helpCommandName.empty()) {
        modifiers.emplace(replmk::definition::AltHelpCmdNameLabel, definition.helpCommandName);
    }

    if(not definition.helpCommandDescription.empty()) {
        modifiers.emplace(replmk::definition::AltHelpCmdDescLabel, definition.helpCommandDescription);
    }

    if(not definition.exitCommandName.empty()) {
        modifiers.emplace(replmk::definition::AltExitCmdNameLabel, definition.exitCommandName);
    }

    if(not definition.exitCommandDescription.empty()) {
        modifiers.emplace(replmk::definition::AltExitCmdDescLabel, definition.exitCommandDescription);
    }
    return modifiers;
}

auto runWithUserInterface(const replmk::ReplDefinition& definition, replmk::CommandHistory& cmdHistory, replmk::OutputHistory& outputHistory) -> void {
    const auto externalCatalog = makeExternalCommandCatalog(definition);
    const auto modifiers = makeCommandModifiers(definition);

    replmk::OutputBuffers outBuffers;
    if(not outputHistory.Load(outBuffers)) {
    }

    const auto cmdProcAction = replmk::makeCommandProcessingAction(externalCatalog, modifiers, outBuffers, cmdHistory, outputHistory);

    runTextUserInterface(outBuffers, cmdProcAction, definition, cmdHistory);
}

auto runMain(int argc, char* argv[]) -> int { //NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    cxxopts::Options options("replmk", "A generic REPL command-line interface tool.");

    options.add_options()
    ("c,config", "Path to the configuration YAML file", cxxopts::value<std::string>())
    ("s,command-history-file", "Optional path to a file where to save the command history", cxxopts::value<std::string>())
    ("o,output-history-file", "Optional path to a file where to save the output history", cxxopts::value<std::string>())
    ("h,help", "Print usage");

    options.allow_unrecognised_options();

    auto cmdOptionsParseResult = options.parse(argc, argv);

    if (cmdOptionsParseResult.contains("help") or not cmdOptionsParseResult.unmatched().empty()) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::string definitionFilePath{};
    if(cmdOptionsParseResult.contains("config") ) {
        definitionFilePath = cmdOptionsParseResult["config"].as<std::string>();
    }
    replmk::ReplDefinition definition{};

    if (not definitionFilePath.empty() and std::filesystem::exists(definitionFilePath)) {
        const auto maybeDefinition = replmk::loadDefinition(definitionFilePath);
        if(not maybeDefinition.has_value()) {
            std::cerr << "Error loading definition file: '" <<definitionFilePath <<
                      "'. Error code: '" << replmk::DefinitionErrorAsString(maybeDefinition.error()) << "'" << std::endl;
            return 1;
        }

        std::cout << "REPL Definition loaded from: " << definitionFilePath << "\n";

        definition = maybeDefinition.value();
    } else {
        std::cout << "Configuration file not found at '" << definitionFilePath << "', running default REPL.\n";
        definition.prompt = replmk::definition::DefaultPromptString;
        definition.initialMessage = replmk::definition::InitialMessageIcon;
    }

    std::string commandHistoryFile = cmdOptionsParseResult.count("command-history-file") > 0
                                     ? cmdOptionsParseResult["command-history-file"].as<std::string>()
                                     : std::string{std::getenv("HOME")} + "/.replmk_history";
    std::string outputHistoryFile = cmdOptionsParseResult.count("output-history-file") > 0
                                    ? cmdOptionsParseResult["output-history-file"].as<std::string>()
                                    : std::string{std::getenv("HOME")} + "/.replmk_output_history";

    replmk::CommandHistory cmdHistory{commandHistoryFile};
    replmk::OutputHistory outputHistory{outputHistoryFile};

    if (not cmdHistory.Load()) {
        // do nothing
    }

    runWithUserInterface(definition, cmdHistory, outputHistory);

    if (not cmdHistory.Save()) {
        // do nothing
    }

    return 0;
}
