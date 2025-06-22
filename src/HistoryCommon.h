#pragma once

#include <string>
#include <optional>
#include <fstream>

namespace replmk {

[[nodiscard]]
inline auto ReadHistoryFieldWithLengthAndLineBreak(std::ifstream& inFile) -> std::optional<std::string> {
    std::string lengthStr;
    if (not std::getline(inFile, lengthStr, ':')) {
        return std::nullopt;
    }
    std::streamsize length=0;
    try {
        length = std::stol(lengthStr);
    } catch(...) {
        return std::nullopt;
    }
    std::string content;
    if(length > 0) {
        content.resize(static_cast<std::size_t>(length), '\0');
        if (not inFile.read(content.data(), length)) {
            return std::nullopt;
        }
    }
    inFile.ignore(1);

    return content;
}

} // namespace replmk