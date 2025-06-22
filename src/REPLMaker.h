#pragma once

#include "OutputHistory.h"
#include "REPLDefinition.h"
#include "CommandHistory.h"

auto makeExternalCommandCatalog(const replmk::ReplDefinition& definition) -> replmk::CommandCatalog;
auto makeCommandModifiers(const replmk::ReplDefinition& definition) -> replmk::REPLModifiers;
auto runWithUserInterface(const replmk::ReplDefinition& definition, replmk::CommandHistory& cmdHistory, replmk::OutputHistory& outputHistory) -> void;
auto runMain(int argc, char* argv[]) -> int; //NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
