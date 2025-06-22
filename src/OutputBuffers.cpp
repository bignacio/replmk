#include <string>
#include <string_view>


#include "OutputBuffers.h"

namespace replmk {

auto OutputBuffers::AddNewEntry(OutputBufferEntry&& entry) -> void {
    // I should really check for size before adding a new entry here

    this->bufferEntries.push_back(std::move(entry));
    this->SafeOnChange();
}

auto OutputBuffers::AppendToLastStdOutEntry(std::string_view text) -> bool {
    return AppendToLastEntry(text, [](OutputBufferEntry& entry) -> std::string& {
        return entry.stdOutEntry;
    });
}

auto OutputBuffers::AppendToLastStdErrEntry(std::string_view text) -> bool {
    return AppendToLastEntry(text, [](OutputBufferEntry& entry) -> std::string& {
        return entry.stdErrEntry;
    });
}

auto OutputBuffers::GetBuffer() const -> const std::vector<OutputBufferEntry>& {
    return this->bufferEntries;
}

auto OutputBuffers::SetOnOutputChangedEvent(OnOutputChangedEvent outputChangedCb) -> void {
    this->onOutputChanged = std::move(outputChangedCb);
}


// private methods
auto OutputBuffers::SafeOnChange() -> void {
    if(this->onOutputChanged) {
        this->onOutputChanged(*this);
    }
}

auto OutputBuffers::AppendToLastEntry(std::string_view text, const OutputEntryFieldSelector& fieldSelectorFn) -> bool {
    if(this->bufferEntries.empty()) {
        return false;
    }

    auto& lastEntry = *std::prev(this->bufferEntries.end());
    std::string& selectedField = fieldSelectorFn(lastEntry);
    selectedField.append(text);

    this->SafeOnChange();

    return true;
}


}//namespace replmk