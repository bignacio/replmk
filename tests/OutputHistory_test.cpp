#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "OutputHistory.h"
#include "OutputBuffers.h"

using namespace replmk;

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
TEST_SUITE_BEGIN("OutputHistory");

TEST_CASE("Load and save with empty path") {
    OutputBuffers buffers;
    auto outHistory = OutputHistory("");

    REQUIRE_FALSE(outHistory.Load(buffers));

    REQUIRE_FALSE(outHistory.Save(buffers));
}


TEST_CASE("Load and save with valid path") {
    OutputBuffers buffers;
    const std::filesystem::path tempFilePath(std::filesystem::temp_directory_path() / "output_history_test.txt");
    std::filesystem::remove(tempFilePath);

    auto saverOutHistory = OutputHistory(tempFilePath);
    // Load should fail since the file does not exist
    REQUIRE_FALSE(saverOutHistory.Load(buffers));

    // Add entries to buffers
    buffers.AddNewEntry({.prompt = "prompt1", .stdOutEntry = "stdout1", .stdErrEntry = "stderr1"});

    // Save to file
    REQUIRE(saverOutHistory.Save(buffers));

    // Verify file exists and contains correct data
    REQUIRE(std::filesystem::exists(tempFilePath));

    OutputBuffers loadedBuffers;
    auto loaderOutHistory = OutputHistory(tempFilePath);
    REQUIRE(loaderOutHistory.Load(loadedBuffers));

    REQUIRE_FALSE(loadedBuffers.GetBuffer().empty());

    const auto entry = loadedBuffers.GetBuffer().at(0);

    REQUIRE(entry.prompt == "prompt1");
    REQUIRE(entry.stdOutEntry == "stdout1");
    REQUIRE(entry.stdErrEntry == "stderr1");

    // Clean up
    REQUIRE(std::filesystem::remove(tempFilePath));
}

auto verifyLoadAndSaveWithEntries(const std::vector<OutputBufferEntry>& entries) -> void {
    OutputBuffers buffers;
    const std::filesystem::path tempFilePath(std::filesystem::temp_directory_path() / "output_history_test.txt");
    std::filesystem::remove(tempFilePath);


    // Initialize with temp file path
    auto saverOutHistory = OutputHistory(tempFilePath);
    // Load should fail since the file does not exist
    REQUIRE_FALSE(saverOutHistory.Load(buffers));

    // Add entries to buffers
    for(auto entry: entries) {
        buffers.AddNewEntry(std::move(entry));
    }

    // Save to file
    REQUIRE(saverOutHistory.Save(buffers));

    // Verify file exists and contains correct data
    REQUIRE(std::filesystem::exists(tempFilePath));

    OutputBuffers loadedBuffers;
    auto loaderOutHistory = OutputHistory(tempFilePath);
    REQUIRE(loaderOutHistory.Load(loadedBuffers));

    REQUIRE_FALSE(loadedBuffers.GetBuffer().empty());
    for(size_t lbi = 0; lbi < loadedBuffers.GetBuffer().size(); lbi++) {
        const auto& loadedEntry = loadedBuffers.GetBuffer().at(lbi);
        const auto& expectedEntry = entries.at(lbi);
        REQUIRE(loadedEntry.prompt == expectedEntry.prompt);
        REQUIRE(loadedEntry.stdOutEntry == expectedEntry.stdOutEntry);
        REQUIRE(loadedEntry.stdErrEntry == expectedEntry.stdErrEntry);
    }

    // Clean up
    REQUIRE(std::filesystem::remove(tempFilePath));
}

TEST_CASE("Load and save without stdout stderr") {
    const std::vector<OutputBufferEntry> entries = {
        {.prompt = "prompt1", .stdOutEntry = "", .stdErrEntry = ""},
        {.prompt = "prompt2", .stdOutEntry = "stdout", .stdErrEntry = ""},
        {.prompt = "prompt3", .stdOutEntry = "", .stdErrEntry = "stderr"},
        {.prompt = "prompt4", .stdOutEntry = "stdout", .stdErrEntry = "stderr"},
    };
    verifyLoadAndSaveWithEntries(entries);
}

TEST_CASE("Load and save with invalid path") {
    OutputBuffers buffers;
    const std::filesystem::path tempFilePath(std::filesystem::temp_directory_path() / "/not/a/valid/path/output_history_test.txt");

    auto outHistory = OutputHistory(tempFilePath);
    REQUIRE_FALSE(outHistory.Save(buffers));

    // Add entries to buffers
    buffers.AddNewEntry({.prompt = "prompt1", .stdOutEntry = "stdout1", .stdErrEntry = "stderr1"});

    // Save function should not crash
    REQUIRE_FALSE(outHistory.Save(buffers));

    REQUIRE_FALSE(std::filesystem::exists(tempFilePath));
}

TEST_CASE("Read incomplete entry") {
    const std::filesystem::path tempFilePath(std::filesystem::temp_directory_path() / "output_history_test.txt");

    std::filesystem::remove(tempFilePath);

    std::ofstream outFile(tempFilePath);
    REQUIRE(outFile.is_open());

    // Write only PROMPT field
    constexpr size_t kTestStringLength = 7;  // Length of test strings ("prompt1", "stdout1", "stderr1")

    outFile << OutputHistoryPromptPrefix << ":" << kTestStringLength << ":prompt1\n";
    outFile.close();

    OutputBuffers loadedBuffers;
    auto outHistory = OutputHistory(tempFilePath);
    REQUIRE_FALSE(outHistory.Load(loadedBuffers));

    REQUIRE(loadedBuffers.GetBuffer().empty());
    // Clean up
    REQUIRE(std::filesystem::remove(tempFilePath));
}

TEST_CASE("Read entry fail when size is missing") {
    const std::filesystem::path tempFilePath(std::filesystem::temp_directory_path() / "output_history_test.txt");

    std::filesystem::remove(tempFilePath);

    std::ofstream outFile(tempFilePath);
    REQUIRE(outFile.is_open());

    outFile << OutputHistoryPromptPrefix << ":" << ":prompt1\n";
    outFile << OutputHistoryStdOutPrefix << ":" << ":stdout1\n";
    outFile << OutputHistoryStdErrPrefix << ":" << ":stderr1\n";
    outFile.close();


    OutputBuffers loadedBuffers;
    auto outHistory = OutputHistory(tempFilePath);
    REQUIRE_FALSE(outHistory.Load(loadedBuffers));

    REQUIRE(loadedBuffers.GetBuffer().empty());

    REQUIRE(std::filesystem::remove(tempFilePath));
}

TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
