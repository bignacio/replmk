#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

#include "CommandHistory.h"
#include "Command.h"
#include "HistoryCommon.h"

namespace replmk {


CommandHistory::CommandHistory(std::filesystem::path filePath) : historyFilePath{std::move(filePath)} {}

auto CommandHistory::Load() -> bool {
    if (this->historyFilePath.empty()) {
        return true;
    }

    std::ifstream inFile(this->historyFilePath);
    if (inFile.is_open()) {
        while (const auto command = ReadHistoryFieldWithLengthAndLineBreak(inFile)) {
            this->Add(command.value());
        }
        return true;
    }
    return false;
}

auto CommandHistory::Save() const -> bool {
    if (this->historyFilePath.empty()) {
        return true;
    }

    std::ofstream outFile(this->historyFilePath);
    if (not outFile.is_open()) {
        return false;
    }

    for (const auto& command : this->commands) {
        const auto trimmedCommand = trimString(command);
        if(not trimmedCommand.empty()) {
            outFile << trimmedCommand.size() << ':' << trimmedCommand << '\n';
            outFile.flush();
        }
    }
    return true;
}

auto CommandHistory::Add(std::string_view command) -> void {
    if (this->commands.empty() || this->commands.back() != command) {
        this->commands.emplace_back(command);
        this->navPos = this->commands.size();
    }
}

auto CommandHistory::Next() -> std::optional<std::string> {
    if (this->commands.empty()) {
        return std::nullopt;
    }
    
    this->navPos++;
    if (this->navPos >= this->commands.size()) {
        this->navPos = this->commands.size()-1;
    }

    return this->commands[this->navPos];
}

auto CommandHistory::Previous() -> std::optional<std::string> {
    if (this->commands.empty()) {
        return std::nullopt;
    }

    if (this->navPos > 0) {
        this->navPos--;
    }

    return this->commands[this->navPos];
}

} // namespace replmk
