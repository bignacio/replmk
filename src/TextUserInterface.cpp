#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <optional>
#include <string>


#include "TextUserInterface.h"
#include "CommandHistory.h"
#include "OutputBuffers.h"
#include "Command.h"

namespace replmk {
using OnCommandEnterEvent = std::function<void(const std::string&)>;

auto hasNavigateContent(const ftxui::Event& event, CommandHistory& cmdHistory)->std::optional<std::string> {
    if(event == ftxui::Event::ArrowUp) {
        return cmdHistory.Previous();
    }
    if(event == ftxui::Event::ArrowDown) {
        return cmdHistory.Next();
    }
    return std::nullopt;
}
auto makeCommandInput(std::string& inputBuffer, const std::string& inputNote, const OnCommandEnterEvent& onCommandEntered, CommandHistory& cmdHistory)  -> ftxui::Component {
    auto inputField = ftxui::Input(&inputBuffer, inputNote);

    auto inputFieldWithEvents = ftxui::CatchEvent(inputField, [&inputBuffer, onCommandEntered, &cmdHistory](const ftxui::Event& event) {

        if(const auto navigateContent = hasNavigateContent(event, cmdHistory); navigateContent.has_value()) {
            const auto& historyContent = navigateContent.value();
            if(not historyContent.empty()) {
                inputBuffer.clear();
                inputBuffer.append(historyContent);
            }
            return false;
        }

        if(not event.is_character() and event.character() == "\n") {
            std::string fullCommandLine = inputBuffer;
            inputBuffer.clear();
            onCommandEntered(fullCommandLine);
            return true;
        }

        return false;
    });

    return inputFieldWithEvents;
}

auto makeOutputFrame(const OutputBuffers& outBuffers, const float& scrollYPos, ftxui::Box& firstItemBox)  -> ftxui::Component {
    auto outputRenderer = ftxui::Renderer([&outBuffers, &scrollYPos, &firstItemBox] {
        ftxui::Elements lines;

        for (const auto& entry : outBuffers.GetBuffer()) {

            if(not entry.prompt.empty()) {
                lines.push_back(ftxui::bold(ftxui::paragraph(entry.prompt)));
            }

            auto pline = ftxui::paragraph(entry.stdOutEntry);
            if(lines.empty()) {
                pline = pline | ftxui::reflect(firstItemBox);
            }
            lines.push_back(pline);

            if(not entry.stdErrEntry.empty()) {
                lines.push_back(ftxui::separator()|ftxui::color(ftxui::Color::OrangeRed1));
                lines.push_back(ftxui::hflow(ftxui::paragraph(entry.stdErrEntry)));
                lines.push_back(ftxui::separator()|ftxui::color(ftxui::Color::OrangeRed1));
            }
        }

        const auto linesVbox = ftxui::vbox(lines);

        return linesVbox | ftxui::focusPositionRelative(0, scrollYPos)| ftxui::vscroll_indicator  |ftxui::yframe;
    });

    return outputRenderer;
}

auto makeTopBarRenderer(const std::string& initialMessage) {
    return ftxui::Renderer([&initialMessage]->ftxui::Element {
        return ftxui::hbox({
            ftxui::bold(ftxui::text(definition::InitialMessageIcon+" "+initialMessage))
        }) | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Blue);
    });
}


auto makeStatusBarRenderer() {
    return ftxui::Renderer([] ->ftxui::Element{
        return ftxui::hbox({
            ftxui::text(" Status: idle")
        }) | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::DarkGreen);
    });

}

auto calculateScrollPosition ( int minFirstItemYValue, const ftxui::Box& outputFrameRenderedBox, float scrollYPos, const ftxui::Event& event) {

    // hack to calculate position. For some reason, y_max has the correct value when the page goes off screen
    const int totalHeight = outputFrameRenderedBox.y_max - minFirstItemYValue;
    const int pageHeight = outputFrameRenderedBox.y_max - outputFrameRenderedBox.y_min;


    float pageStep = 0;
    if(totalHeight > 0) {
        pageStep = static_cast<float>(pageHeight)/static_cast<float>(totalHeight);
    }

    if(event == ftxui::Event::PageUp) {
        scrollYPos = scrollYPos-pageStep;
    }
    if(event == ftxui::Event::PageDown) {
        scrollYPos = scrollYPos+pageStep;
    }

    scrollYPos = std::min<float>(scrollYPos, 1);
    scrollYPos = std::max<float>(scrollYPos, 0);

    if(event == ftxui::Event::Home) {
        scrollYPos = 0;
    }
    if(event == ftxui::Event::End) {
        scrollYPos = 1;
    }

    return scrollYPos;
}

auto createAndRunTextUserInterface(const std::string& inputNote, OutputBuffers& outBuffers, const std::string& prompt,
                                   const CommandProcessingAction& cmdProcAction, const std::string& initialMessage, CommandHistory& cmdHistory) {
    auto screen = ftxui::ScreenInteractive::FullscreenAlternateScreen();

    std::string inputBuffer;
    const auto onInternalSpecialCmd = [&screen](CommandType command) {
        if(command == CommandType::InternalExit) {
            screen.Exit();
        }
    };

    const auto onCommandEntered = [&outBuffers, &prompt, cmdProcAction, onInternalSpecialCmd](const std::string& fullCommandLine) {

        const auto trimmedFullCmdLine = trimString(fullCommandLine);
        if (trimmedFullCmdLine.empty()) {
            return;
        }

        outBuffers.AddNewEntry({
            .prompt= prompt+trimmedFullCmdLine+"\n",
            .stdOutEntry = "",
            .stdErrEntry = ""});

        // I should do something if this returns false. Probably. Maybe
        cmdProcAction(trimmedFullCmdLine, onInternalSpecialCmd);
    };

    float scrollYPos = 1;
    ftxui::Box firstItemBox;

    const auto inputField = makeCommandInput(inputBuffer, inputNote, onCommandEntered, cmdHistory);
    const auto outputFrame = makeOutputFrame(outBuffers, scrollYPos, firstItemBox);
    const auto topBarRenderer = makeTopBarRenderer(initialMessage);
    const auto statusBarRenderer= makeStatusBarRenderer();

    constexpr int MaxInputFieldHeight = 6;
    constexpr int StartInputFieldHeight = 4;

    ftxui::Box outputFrameRenderedBox;
    const auto outputFrameFlexBox = outputFrame | ftxui::border |  ftxui::flex |ftxui::reflect(outputFrameRenderedBox);


    auto mainContainer = ftxui::Container::Vertical({
        topBarRenderer,
        outputFrameFlexBox,
        inputField | ftxui::frame | ftxui::border |
        ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, StartInputFieldHeight) |
        ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, MaxInputFieldHeight),
        statusBarRenderer
    });

    mainContainer->SetActiveChild(mainContainer->ChildAt(2));

    int minFirstItemYValue = 0;
    auto mainContainerEventCather = ftxui::CatchEvent(mainContainer, [&scrollYPos, &outputFrameRenderedBox, &firstItemBox, &minFirstItemYValue]([[maybe_unused]] const ftxui::Event& event) {
        minFirstItemYValue = std::min(minFirstItemYValue, firstItemBox.y_max);

        scrollYPos = calculateScrollPosition(minFirstItemYValue, outputFrameRenderedBox, scrollYPos, event);

        return false;

    });

    ftxui::Loop looper(&screen, mainContainerEventCather);

    outBuffers.SetOnOutputChangedEvent([&looper, &screen]([[maybe_unused]] const OutputBuffers& unused) {
        screen.PostEvent(ftxui::Event::Custom);
        looper.RunOnceBlocking();
    });

    looper.Run();
}

auto runTextUserInterface(OutputBuffers& outBuffers, const CommandProcessingAction& cmdProcessingAction,
                          const ReplDefinition& definition, CommandHistory& cmdHistory) -> void {

    createAndRunTextUserInterface(definition.inputNote, outBuffers, definition.prompt, cmdProcessingAction,
                                  definition.initialMessage, cmdHistory);
}

} // namespace replmk