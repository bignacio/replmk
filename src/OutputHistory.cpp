#include <filesystem>
#include <fstream>
#include <string_view>

#include "OutputHistory.h"
#include "HistoryCommon.h"

namespace replmk {
// Helper function to read a field with prefix:length:content format
auto ReadField(std::ifstream& inFile, const std::string_view expectedPrefix) -> std::optional<std::string> {
    std::string prefix;
    if (not  std::getline(inFile, prefix, ':')) {
        return std::nullopt;
    }
    if (prefix != expectedPrefix) {
        return std::nullopt;
    }

    return ReadHistoryFieldWithLengthAndLineBreak(inFile).value_or("");
}

// Helper function to write a field with prefix:length:content format
auto WriteField(std::ofstream& outFile, const std::string_view prefix, const std::string& content) -> void {
    outFile << prefix << ':' << content.size() << ':' << content << '\n';
}

// Helper function to read a complete entry (PROMPT, STDOUT, STDERR) from file
auto ReadCompleteEntry(std::ifstream& inFile) -> std::optional<replmk::OutputBufferEntry> {
    replmk::OutputBufferEntry entry;

    // Read PROMPT
    const auto prompt = ReadField(inFile, OutputHistoryPromptPrefix);
    if (not prompt.has_value()) {
        return std::nullopt;
    }
    entry.prompt = prompt.value();

    // Read STDOUT. The presence of a value for stdout is optional but the tag must exist
    const auto stdout = ReadField(inFile, OutputHistoryStdOutPrefix);
    if (not stdout.has_value()) {
        return std::nullopt;
    }
    entry.stdOutEntry = stdout.value();

    // Read STDERR. The presence of stderr is optional
    const auto stderr = ReadField(inFile, OutputHistoryStdErrPrefix);
    if (not stderr.has_value()) {
        return std::nullopt;
    }
    entry.stdErrEntry = stderr.value();

    return entry;
}

// Helper function to write a complete entry to file
auto WriteCompleteEntry(std::ofstream& outFile, const OutputBufferEntry& entry) -> void {
    WriteField(outFile, OutputHistoryPromptPrefix, entry.prompt);
    WriteField(outFile, OutputHistoryStdOutPrefix, entry.stdOutEntry);
    WriteField(outFile, OutputHistoryStdErrPrefix, entry.stdErrEntry);
    outFile.flush();
}

// class implementation
OutputHistory::OutputHistory(std::filesystem::path filePath) : historyFilePath{std::move(filePath)} {}

auto OutputHistory::Load(OutputBuffers& outBuffers) -> bool {
    if (this->historyFilePath.empty()) {
        return false;
    }

    std::ifstream inFile(this->historyFilePath);
    if (inFile.is_open()) {
        while (not inFile.eof()) {
            auto entry = ReadCompleteEntry(inFile);
            if(not entry.has_value()) {
                return false;
            }
            outBuffers.AddNewEntry(std::move(entry.value()));

            if(inFile.peek() == EOF) {
                return true;
            }
        }
    }
    return false;
}

auto OutputHistory::Save(const OutputBuffers& outBuffers) -> bool {
    if(this->historyFilePath.empty()) {
        return false;
    }
    std::ofstream outFile(this->historyFilePath, std::ios::trunc);
    if (outFile.is_open()) {
        for (const auto& entry : outBuffers.GetBuffer()) {
            WriteCompleteEntry(outFile, entry);
        }
        outFile.close();
        return true;
    }

    return false;
}

auto OutputHistory::GetFilePath() const -> std::filesystem::path {
    return this->historyFilePath;
}

} // namespace replmk
