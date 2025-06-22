#pragma once

#include <string>
#include <functional>
#include <optional>

// FTXUI includes
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "Core.h"
#include "OutputBuffers.h"
#include "CommandHistory.h"

namespace replmk {

using OnCommandEnterEvent = std::function<void(const std::string&)>;

auto runTextUserInterface(OutputBuffers& outBuffers, const CommandProcessingAction& cmdProcessingAction,
                         const ReplDefinition& definition, CommandHistory& cmdHistory) -> void;

auto makeCommandInput(std::string& inputBuffer, const std::string& inputNote, const OnCommandEnterEvent& onCommandEntered, CommandHistory& cmdHistory) -> ftxui::Component;

auto makeOutputFrame(const OutputBuffers& outBuffers, const float& scrollYPos, ftxui::Box& firstItemBox) -> ftxui::Component;

auto hasNavigateContent(const ftxui::Event& event, CommandHistory& cmdHistory) -> std::optional<std::string>;

} // namespace replmk