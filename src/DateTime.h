#pragma once
#include <chrono>
#include <ctime>

namespace replmk::time {

[[nodiscard]] inline auto getLocalDateTimeString() -> std::string {
    std::time_t timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::locale defaultLocale("");

    std::ostringstream oss;
    oss.imbue(defaultLocale);

    std::tm tm_buf{};
    localtime_r(&timeNow, &tm_buf);

    oss << std::put_time(&tm_buf, "%c");
    return oss.str();
}
} // namespace  replmk::time