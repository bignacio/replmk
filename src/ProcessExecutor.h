#pragma once

#include <functional>
#include <string>
#include <vector>
#include <string_view>

namespace replmk {
using OnCommandOutput = std::function<void(std::string_view)>;

struct CommandOutputCallbacks {
    OnCommandOutput onStdOut;
    OnCommandOutput onStdErr;
};

auto executeAndCaptureOutputs(std::string_view cmd, const std::vector<std::string>& args,
                              const CommandOutputCallbacks& callbacks) -> bool;
} //namespace replmk