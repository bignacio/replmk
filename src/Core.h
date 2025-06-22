#pragma once

#include <functional>
#include <vector>
#include <string_view>
#include <optional>

#include "OutputHistory.h"
#include "REPLDefinition.h"
#include "OutputBuffers.h"
#include "Command.h"
#include "CommandHistory.h"

namespace replmk {

using OnInternalCommandEvent = std::function<void(CommandType)>;
using CommandProcessingAction = std::function<bool(std::string_view, const OnInternalCommandEvent&)>;

// Internal command catalog and processing
[[nodiscard]] auto buildInternalCommandCatalog(const REPLModifiers& modifiers) -> CommandCatalog;

[[nodiscard]] auto resolveCommandLine(const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, std::string_view fullCommandLine) -> std::optional<ResolvedCommand>;

auto handleHelpDisplay(const std::vector<std::string>& args, const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, OutputBuffers& outBuffers) -> void;

auto handleInternalCommands(const Command& command, const std::vector<std::string>& args, const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, const OnInternalCommandEvent& onInternalCmd, OutputBuffers& outBuffers) -> bool;

auto executeSingleCommandLine(const Command& command, const std::vector<std::string>& args, OutputBuffers& outBuffers) -> bool;

auto executeShellScriptCommand(const Command& command, const std::vector<std::string>& args, OutputBuffers& outBuffers) -> bool;

auto executeCommandLine(const CommandCatalog& externalCommands, const CommandCatalog& internalCommands, OutputBuffers& outBuffers, std::string_view fullCommandLine, const OnInternalCommandEvent& onInternalCmd) -> bool;

auto makeCommandProcessingAction(const CommandCatalog& externalCommands, const REPLModifiers& modifiers, OutputBuffers& outBuffers, CommandHistory& cmdHistory, OutputHistory& outputHistory) -> CommandProcessingAction;

template<typename Map, typename Key, typename Default>
[[nodiscard]]
auto MapGetOrDefault(const Map& map, Key&& key, Default&& defaultValue) -> typename Map::mapped_type {
    using MapKeyType = typename Map::key_type;
    auto convertedKey = static_cast<MapKeyType>(std::forward<Key>(key));
    if (auto mapITer = map.find(convertedKey); mapITer != map.end()) {
        return mapITer->second;
    }
    return std::forward<Default>(defaultValue);
}

}// namespace replmk
