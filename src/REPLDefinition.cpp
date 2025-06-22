#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "REPLDefinition.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <yaml-cpp/yaml.h>
#pragma GCC diagnostic pop

namespace replmk {

[[nodiscard]]
auto getRequiredString(const YAML::Node& node, std::string_view key) -> std::expected<std::string, DefinitionError> {
    std::string keyStr{key};
    if (not node[keyStr]) {
        return std::unexpected{DefinitionError::MissingRequiredField};
    }

    if (not node[keyStr].IsScalar()) {
        return std::unexpected{DefinitionError::InvalidFieldType};
    }
    return node[keyStr].as<std::string>();
}

[[nodiscard]]
auto getStringOrDefault(const YAML::Node& node, std::string_view key, std::string_view defaultValue) -> std::string { //NOLINT(bugprone-easily-swappable-parameters)
    std::string keyStr{key};
    if (node[keyStr] &&  node[keyStr].IsScalar()) {
        return node[keyStr].as<std::string>();
    }

    return std::string{defaultValue};
}

[[nodiscard]]
auto parseBasicFields(const YAML::Node& replDefNode) -> ReplDefinition {
    ReplDefinition replDef;
    replDef.prompt = getStringOrDefault(replDefNode, definition::PromptLabel, definition::DefaultPromptString);
    replDef.initialMessage = getStringOrDefault(replDefNode, definition::InitialMessageLabel, definition::InitialMessageIcon);
    replDef.inputNote = getStringOrDefault(replDefNode, definition::InputNoteLabel, definition::DefaultInputNote);


    // alternative values. Mostly used to replace name and descriptions in internal commands
    replDef.helpCommandName = getStringOrDefault(replDefNode, definition::AltHelpCmdNameLabel, "");
    replDef.helpCommandDescription = getStringOrDefault(replDefNode, definition::AltHelpCmdDescLabel, "");

    replDef.exitCommandName = getStringOrDefault(replDefNode, definition::AltExitCmdNameLabel, "");
    replDef.exitCommandDescription = getStringOrDefault(replDefNode, definition::AltExitCmdDescLabel, "");

    return replDef;
}

[[nodiscard]]
auto parseCommand(const YAML::Node& commandNode) -> std::expected<Command, DefinitionError> {
    Command cmd;

    auto nameResult = getRequiredString(commandNode, definition::CommandNameLabel);
    if (!nameResult) {
        return std::unexpected{nameResult.error()};
    }
    cmd.name = nameResult.value();

    auto descResult = getRequiredString(commandNode, definition::CommandDescLabel);
    if (!descResult) {
        return std::unexpected{descResult.error()};
    }
    cmd.description = descResult.value();

    auto typeStringResult = getRequiredString(commandNode, definition::CommandTypeLabel);
    if (!typeStringResult) {
        return std::unexpected{typeStringResult.error()};
    }

    cmd.cmdType = toCommandType(typeStringResult.value());
    if (cmd.cmdType == CommandType::Unknown) {
        return std::unexpected{DefinitionError::InvalidCommandType};
    }

    auto execResult = getRequiredString(commandNode, definition::CommandExecLabel);
    if (!execResult) {
        return std::unexpected{execResult.error()};
    }
    cmd.exec = execResult.value();

    return cmd;
}

[[nodiscard]]
auto parseCommands(const YAML::Node& replDefNode) -> std::expected<std::vector<Command>, DefinitionError> {
    if (not replDefNode[definition::CommandListLabel]) {
        return std::unexpected{DefinitionError::MissingCommandsList};
    }
    if (not replDefNode[definition::CommandListLabel].IsSequence()) {
        return std::unexpected{DefinitionError::InvalidCommandsList};
    }

    std::vector<Command> commands;
    for (const auto& commandNode : replDefNode[definition::CommandListLabel]) {
        auto cmdResult = parseCommand(commandNode);
        if (!cmdResult) {
            return std::unexpected{cmdResult.error()};
        };
        commands.push_back(cmdResult.value());
    }

    return commands;
}

[[nodiscard]]
auto loadDefinitionWithException(std::string_view filePath) -> std::expected<ReplDefinition, DefinitionError> {
    auto yamlRoot = YAML::LoadFile(std::string{filePath});

    if (!yamlRoot) {
        return std::unexpected{DefinitionError::YamlParseError};
    }

    auto replDef = parseBasicFields(yamlRoot);

    auto commandsResult = parseCommands(yamlRoot);
    if (!commandsResult) {
        return std::unexpected{commandsResult.error()};
    }

    replDef.commands = commandsResult.value();
    return replDef;
}

[[nodiscard]]
auto loadDefinition(std::string_view filePath) noexcept -> std::expected<ReplDefinition, DefinitionError> {
    try {
        return loadDefinitionWithException(filePath);
    } catch (const YAML::BadFile&) {
        return std::unexpected{DefinitionError::FileNotFound};
    } catch (const YAML::ParserException&) {
        return std::unexpected{DefinitionError::YamlParseError};
    } catch (const YAML::Exception&) {
        return std::unexpected{DefinitionError::YamlParseError};
    } catch (...) {
        return std::unexpected{DefinitionError::UnexpectedError};
    }
}

} // namespace replmk
