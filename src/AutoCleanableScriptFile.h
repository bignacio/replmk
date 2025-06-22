#pragma once

#include <filesystem>
#include <string_view>
#include <optional>

namespace replmk::io {

/**
 * A class that generates a shell script file and makes it executable
 */
class AutoCleanableScriptFile final {
  private:
    std::filesystem::path filePath;


    [[nodiscard]]
    auto MakeExecutable() -> bool;
  public:
    AutoCleanableScriptFile() = default;

    [[nodiscard]]
    auto WriteScript(const std::filesystem::path& scriptFilePath, std::string_view content) -> bool;


    AutoCleanableScriptFile(const AutoCleanableScriptFile&) = delete;
    AutoCleanableScriptFile(AutoCleanableScriptFile&& other) = delete;
    auto operator=(const AutoCleanableScriptFile&) -> AutoCleanableScriptFile& = delete;
    auto operator=(AutoCleanableScriptFile&& other) = delete;

    ~AutoCleanableScriptFile();
}; // class AutoCleanableScriptFile

[[nodiscard]]
auto MakeUniqueTempScriptFilePath() -> std::optional<std::filesystem::path>;

} // namespace replmk::io