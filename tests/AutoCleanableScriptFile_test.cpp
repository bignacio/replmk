//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while,bugprone-unchecked-optional-access)

#include <doctest/doctest.h>

#include <fstream>
#include <sstream>
#include <string>

#include "../src/AutoCleanableScriptFile.h"
#include "../src/ProcessExecutor.h"


using namespace replmk::io;


auto ReadAllFromFile(const std::filesystem::path& filePath) -> std::string {
    std::ifstream fileStrm(filePath);
    std::ostringstream outStringStream;
    outStringStream << fileStrm.rdbuf();
    return outStringStream.str();
}

TEST_SUITE("AutoCleanableScriptFile") {


    TEST_CASE("WriteScript successfully") {
        const auto tmpDir =std::filesystem::temp_directory_path();
        const std::string expectedStdout = "oh hello stdout!";
        const std::string expectedStderr = "oh hello stderr!";

        std::string actualStdout;
        std::string actualStderr;

        const auto content = std::format("#!/bin/bash\necho \"{}\"\necho \"{}\" 1>&2", expectedStdout, expectedStderr);
        const auto scriptFilePath = tmpDir / "test.sh";

        {
            AutoCleanableScriptFile fileGenerator;
            REQUIRE(fileGenerator.WriteScript( scriptFilePath, content));

            const auto exitStatus = replmk::executeAndCaptureOutputs(scriptFilePath.string(), {}, {
                .onStdOut = [&actualStdout](std::string_view data) {
                    actualStdout += data;
                },
                .onStdErr = [&actualStderr](std::string_view data) {
                    actualStderr += data;
                }
            });
            REQUIRE(exitStatus);
        }

        actualStdout.erase(actualStdout.find_last_not_of(" \t\n\r") + 1);
        actualStderr.erase(actualStderr.find_last_not_of(" \t\n\r") + 1);

        REQUIRE_EQ(actualStdout, expectedStdout);
        REQUIRE_EQ(actualStderr, expectedStderr);
        REQUIRE_FALSE(std::filesystem::exists(scriptFilePath));

    }

}

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while,bugprone-unchecked-optional-access)
