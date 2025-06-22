#pragma once

#include <string>
#include <vector>
#include <optional>

namespace replmk {


inline auto ParseCommandLine(const std::string& input) -> std::optional<std::vector<std::string>> {
    std::vector<std::string> cmdAndArgs;
    std::string current;
    bool inSingle = false;
    bool inDouble = false;
    bool escape = false;

    for (char inChar : input) {
        if (escape) {
            current += inChar;
            escape = false;
        } else if (inChar == '\\') {
            escape = true;
        } else if (inChar == '\'' && !inDouble) {
            inSingle = !inSingle;
        } else if (inChar == '"' && !inSingle) {
            inDouble = !inDouble;
        } else if ((isspace(inChar) != 0) && !inSingle && !inDouble) {
            if (!current.empty()) {
                cmdAndArgs.push_back(current);
                current.clear();
            }
        } else {
            current += inChar;
        }
    }
    if (escape || inSingle || inDouble) {
        return std::nullopt;
    }
    if (!current.empty()) {
        cmdAndArgs.push_back(current);
    }
    return cmdAndArgs;
}



} // namespace replmk
