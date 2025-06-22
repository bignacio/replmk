
#include <cstring>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <unistd.h>

#include "AutoCleanableScriptFile.h"

namespace replmk::io {


auto AutoCleanableScriptFile::WriteScript(const std::filesystem::path& scriptFilePath, std::string_view content) -> bool {
    this->filePath = scriptFilePath;

    std::ofstream shFile(this->filePath, std::ios::binary | std::ios::trunc);
    if (shFile.good()) {
        const auto& writeResult = shFile.write(content.data(), static_cast<std::streamsize>(content.size()));

        shFile.close();

        if(writeResult.good()) {
            return this->MakeExecutable();
        }
    }

    return false;

}

AutoCleanableScriptFile::~AutoCleanableScriptFile() {
    if (not this->filePath.empty()) {
        std::error_code removeErrorCode;
        std::filesystem::remove(this->filePath, removeErrorCode);
    }
}

// private methods
auto AutoCleanableScriptFile::MakeExecutable() -> bool {
    std::error_code permErrorCode;
    const auto perms = std::filesystem::perms::owner_read |
                       std::filesystem::perms::owner_write |
                       std::filesystem::perms::owner_exec;

    std::filesystem::permissions(this->filePath, perms, permErrorCode);
    return permErrorCode.value() == 0;
}

// free functions
auto MakeUniqueTempScriptFilePath() -> std::optional<std::filesystem::path> {
    const std::string fileNameTemplate("exec_XXXXXX.replmkscript");
    const auto SuffixLength = 13;
    const auto fullFilePath = std::filesystem::temp_directory_path() / fileNameTemplate;
    auto fullFilePathStr = fullFilePath.string();
    const auto fileDescriptor = mkstemps(fullFilePathStr.data(), SuffixLength);
    if (fileDescriptor != -1) {
        close(fileDescriptor);
        return {fullFilePathStr};
    }

    return {};
}


} // namespace replmk::io