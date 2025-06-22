#pragma once

#include <filesystem>

#include "OutputBuffers.h"


namespace replmk {

constexpr std::string_view OutputHistoryPromptPrefix = "PROMPT";
constexpr std::string_view OutputHistoryStdOutPrefix = "STDOUT";
constexpr std::string_view OutputHistoryStdErrPrefix = "STDERR";


using OutputBufferEntry = replmk::OutputBufferEntry;

class OutputHistory final {
  private:
    std::filesystem::path historyFilePath;
  public:
    explicit OutputHistory(std::filesystem::path filePath);

    OutputHistory(const OutputHistory&) = delete;
    auto operator=(const OutputHistory&) -> OutputHistory& = delete;
    OutputHistory(OutputHistory&&) = delete;
    auto operator=(OutputHistory&&) -> OutputHistory& = delete;

    [[nodiscard]] auto Load(OutputBuffers& outBuffers) -> bool;

    [[nodiscard]] auto Save(const OutputBuffers& outBuffers) -> bool;

    [[nodiscard]] auto GetFilePath() const -> std::filesystem::path;

    ~OutputHistory() = default;
}; // class OutputHistory

}
