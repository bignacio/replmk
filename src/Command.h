#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>

namespace replmk {

enum class CommandType: uint8_t {
    Unknown,
    Single,
    Shell,
    Script,
    InternalHelp,
    InternalExit
};

[[nodiscard]] inline auto toCommandType(const std::string& typeString) {
    if (typeString == "single") {
        return CommandType::Single;
    }
    if (typeString == "shell") {
        return CommandType::Shell;
    }
    if (typeString == "script") {
        return CommandType::Script;
    }
    // these shouldn't really be used in definitions
    if (typeString == "internal_help") {
        return CommandType::InternalHelp;
    }
    if (typeString == "internal_exit") {
        return CommandType::InternalExit;
    }
    return CommandType::Unknown;
}

inline auto trimString(const std::string& str) -> std::string {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

struct Command {
    CommandType cmdType{CommandType::Unknown};

    std::string name;
    std::string description;
    std::string exec;
};

using CommandCatalog = std::map<std::string, Command>;
using ResolvedCommand = std::tuple<Command, std::vector<std::string>> ;
}
