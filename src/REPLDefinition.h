#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <expected>
#include <unordered_map>
#include <string_view>

#include "Command.h"

namespace replmk {

namespace definition {

constexpr std::string DefaultPromptString = "> ";
constexpr std::string InitialMessageIcon = "👋";
constexpr std::string DefaultInputNote = "Enter a command";
constexpr std::string ConsoleIcon = "\U0001F4BB"; // 🖥️
constexpr std::string DefaultHelpKeyword = "help";


// yaml fields labels

// modifiers
constexpr std::string AltHelpCmdNameLabel = "alt_help_cmd";
constexpr std::string AltHelpCmdDescLabel = "alt_help_desc";

constexpr std::string AltExitCmdNameLabel = "alt_exit_cmd";
constexpr std::string AltExitCmdDescLabel = "alt_exit_desc";
// all other labels
constexpr std::string PromptLabel = "prompt";
constexpr std::string InitialMessageLabel = "initial_message";
constexpr std::string InputNoteLabel = "input_note";
constexpr std::string CommandNameLabel = "name";
constexpr std::string CommandDescLabel = "description";
constexpr std::string CommandTypeLabel = "type";
constexpr std::string CommandExecLabel = "exec";
constexpr std::string CommandListLabel = "commands";


}// namespace definition

enum class DefinitionError: uint8_t {
    FileNotFound,
    FileCannotBeRead,
    YamlParseError,
    MissingRequiredField,
    InvalidFieldType,
    InvalidCommandType,
    MissingCommandsList,
    InvalidCommandsList,
    UnexpectedError
};

struct ReplDefinition {
    std::string prompt;
    std::string initialMessage;
    std::string helpCommandName;
    std::string helpCommandDescription;
    std::string exitCommandName;
    std::string exitCommandDescription;
    std::string inputNote;
    std::vector<Command> commands;
};


[[nodiscard]]
auto loadDefinition(std::string_view filePath) noexcept -> std::expected<ReplDefinition, DefinitionError>;

using REPLModifiers = std::unordered_map<std::string, std::string>;

[[nodiscard]]
constexpr auto DefinitionErrorAsString(DefinitionError err) -> std::string_view {
    switch (err) {
    case DefinitionError::FileNotFound:
        return "FileNotFound";
    case DefinitionError::FileCannotBeRead:
        return "FileCannotBeRead";
    case DefinitionError::YamlParseError:
        return "YamlParseError";
    case DefinitionError::MissingRequiredField:
        return "MissingRequiredField";
    case DefinitionError::InvalidFieldType:
        return "InvalidFieldType";
    case DefinitionError::InvalidCommandType:
        return "InvalidCommandType";
    case DefinitionError::MissingCommandsList:
        return "MissingCommandsList";
    case DefinitionError::InvalidCommandsList:
        return "InvalidCommandsList";
    case DefinitionError::UnexpectedError:
        return "UnexpectedError";
    default:
        return "Unknown";
    }
}


}//namespace replmk
