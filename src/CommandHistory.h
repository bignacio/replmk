#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <string_view>
#include <optional>

namespace replmk {

class CommandHistory final {
  private:
    size_t navPos{0};
    std::filesystem::path historyFilePath;
    std::vector<std::string> commands;

  public:
    explicit CommandHistory(std::filesystem::path filePath);

    CommandHistory(const CommandHistory&) = delete;
    auto operator=(const CommandHistory&) -> CommandHistory& = delete;
    CommandHistory(CommandHistory&&) = delete;
    auto operator=(CommandHistory&&) -> CommandHistory& = delete;

    auto Load() -> bool;

    auto Save() const -> bool;

    auto Add(std::string_view command) -> void;

    auto Next() -> std::optional<std::string>;

    auto Previous() -> std::optional<std::string>;

    ~CommandHistory() = default;
};

} // namespace replmk
