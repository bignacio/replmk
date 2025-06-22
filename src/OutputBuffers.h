#pragma once

#include <string>
#include <vector>
#include <functional>
#include <string_view>

namespace replmk {
class OutputBuffers;

using OnOutputChangedEvent = std::function<void(const OutputBuffers&)>;
struct OutputBufferEntry {
    std::string prompt;
    std::string stdOutEntry;
    std::string stdErrEntry;
};

using OutputEntryFieldSelector = std::function<std::string&(OutputBufferEntry&)>;

class OutputBuffers final {
  private:
    OnOutputChangedEvent onOutputChanged;

    std::vector<OutputBufferEntry> bufferEntries;

    auto SafeOnChange() -> void;
    auto AppendToLastEntry(std::string_view text, const OutputEntryFieldSelector& fieldSelectorFn) -> bool;
  public:
    OutputBuffers() = default;
    OutputBuffers(const OutputBuffers&)=delete;
    OutputBuffers(OutputBuffers&&)=delete;

    auto operator=(const OutputBuffers&) -> OutputBuffers& = delete;
    auto operator=(OutputBuffers&&) -> OutputBuffers& = delete;

    auto SetOnOutputChangedEvent(OnOutputChangedEvent outputChangedCb) -> void;
    auto AddNewEntry(OutputBufferEntry&& entry) -> void;
    auto AppendToLastStdOutEntry(std::string_view text) -> bool;
    auto AppendToLastStdErrEntry(std::string_view text) -> bool;

    auto GetBuffer() const -> const std::vector<OutputBufferEntry>&;
    ~OutputBuffers() = default;
};

} //namespace replmk