//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)

#include <doctest/doctest.h>       // Doctest testing framework
#include "REPLDefinition.h"
#include "Command.h"

#include <fstream>                  // For creating temporary test files
#include <string>                   // For std::string
#include <filesystem>               // For path manipulation and file cleanup (C++17)

using namespace replmk;

class TempYamlFile {
  public:
    TempYamlFile(const TempYamlFile &) = default;
    TempYamlFile(TempYamlFile &&) = delete;
    auto operator=(const TempYamlFile &) -> TempYamlFile& = default;
    auto operator=(TempYamlFile &&) -> TempYamlFile& = delete;

    explicit TempYamlFile(const std::string &content) {
        filePath = std::filesystem::temp_directory_path() / "temp_config.yaml";
        std::ofstream file(filePath);
        REQUIRE(file.is_open()); // Ensure the file was opened successfully
        file << content;
        file.close();
    }

    ~TempYamlFile() {
        std::filesystem::remove(filePath);
    }

    auto path() const -> const std::string& {
        return filePath;
    }

  private:
    std::string filePath;
};

TEST_SUITE_BEGIN("DefinitionLoader");

TEST_CASE("Load valid minimal definition data") {
    const std::string yamlContent = R"(
prompt: "test-repl> "
commands:
  - name: testcmd
    description: A test command.
    type: shell
    exec: "echo hello"
)";

  TempYamlFile tempFile(yamlContent);
  const std::string& filePath = tempFile.path();

  const auto maybeDefinition = loadDefinition(filePath);
  REQUIRE(maybeDefinition.has_value());

  const ReplDefinition& definition = maybeDefinition.value();

  REQUIRE(definition.prompt == "test-repl> ");
  REQUIRE_FALSE(definition.initialMessage.empty());
  REQUIRE(definition.commands.size() == 1);

  const auto& cmd = definition.commands[0];
  REQUIRE(cmd.name == "testcmd");
  REQUIRE(cmd.description == "A test command.");
  REQUIRE(cmd.cmdType == CommandType::Shell);
  REQUIRE(cmd.exec == "echo hello");

}

TEST_CASE("Load definition with initial message and multiple commands") {
    const std::string yamlContent = R"(
prompt: "$ "
initial_message: "Hello World!"
commands:
    - name: greet
      description: Says hello.
      type: script
      exec: "echo hello"
    - name: run
      description: Run a program.
      type: shell
      exec: "{1} {args}"
      no_args_exec: "echo No program specified"
)";
  TempYamlFile tempFile(yamlContent);
  const std::string& filePath = tempFile.path();

  const auto maybeDefinition = loadDefinition(filePath);

  REQUIRE(maybeDefinition.has_value());
  const ReplDefinition& definition = maybeDefinition.value();

  REQUIRE_EQ(definition.prompt, "$ ");
  REQUIRE_EQ(definition.initialMessage, "Hello World!");
  REQUIRE(definition.commands.size() == 2);

  const auto& greetCmd = definition.commands[0];
  REQUIRE_EQ(greetCmd.name, "greet");
  REQUIRE_EQ(greetCmd.description, "Says hello.");
  REQUIRE_EQ(greetCmd.cmdType, CommandType::Script);
  REQUIRE_EQ(greetCmd.exec, "echo hello");

  const auto& runCmd = definition.commands[1];
  REQUIRE_EQ(runCmd.name, "run");
  REQUIRE_EQ(runCmd.description, "Run a program.");
  REQUIRE_EQ(runCmd.cmdType, CommandType::Shell);
  REQUIRE_EQ(runCmd.exec, "{1} {args}");
}

TEST_CASE("File not found returns FileNotFound error") {
  const std::string filePath = "/tmp/nonexistent_file.yaml";
  const auto result = loadDefinition(filePath);
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == DefinitionError::FileNotFound);
}

void VerifyLoadDefinitionError(const std::string& yamlContent, DefinitionError expectedError) {
  TempYamlFile tempFile(yamlContent);
  const auto result = loadDefinition(tempFile.path());
  REQUIRE_FALSE(result.has_value());
  REQUIRE_EQ(result.error(), expectedError);
}

TEST_CASE("Invalid YAML returns YamlParseError") {
  VerifyLoadDefinitionError("prompt: [ this is: not: valid: yaml", DefinitionError::YamlParseError);
}

TEST_CASE("Missing commands list returns MissingCommandsList error") {
    const std::string yamlContent = R"(
prompt: ">"
)";

  VerifyLoadDefinitionError(yamlContent, DefinitionError::MissingCommandsList);
}

TEST_CASE("Missing command name returns MissingRequiredField error") {
    const std::string yamlContent = R"(
prompt: ">"
commands:
  - type: shell
)";

  VerifyLoadDefinitionError(yamlContent, DefinitionError::MissingRequiredField);
}

TEST_CASE("Missing command description returns MissingRequiredField error") {
    const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: testcmd
    type: shell
    exec: "echo hello"
)";
  VerifyLoadDefinitionError(yamlContent, DefinitionError::MissingRequiredField);
}

TEST_CASE("Missing command type returns MissingRequiredField error") {
    const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: testcmd
    description: A test command
    exec: "echo hello"
)";
  VerifyLoadDefinitionError(yamlContent, DefinitionError::MissingRequiredField);
}

TEST_CASE("Missing command exec for shell type returns MissingRequiredField error") {
    const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: testcmd
    description: A test command
    type: shell
)";
  VerifyLoadDefinitionError(yamlContent, DefinitionError::MissingRequiredField);
}


TEST_CASE("Invalid commands list (not a sequence) returns InvalidCommandsList error") {
    const std::string yamlContent = R"(
prompt: ">"
commands: not_an_array
)";

  TempYamlFile tempFile(yamlContent);
  const auto result = loadDefinition(tempFile.path());
  REQUIRE_EQ(result.error(), DefinitionError::InvalidCommandsList);
}

TEST_CASE("Array instead of string field returns InvalidFieldType error") {
  const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: test
    description: [ "not", "a", "string" ]
    type: shell
    exec: "echo hi"
)";

  VerifyLoadDefinitionError(yamlContent, DefinitionError::InvalidFieldType);
}


TEST_CASE("Command with non-string name returns InvalidFieldType error") {
  const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: [ "not", "a", "string" ]
)";

  VerifyLoadDefinitionError(yamlContent, DefinitionError::InvalidFieldType);
}

TEST_CASE("Command with invalid type string returns InvalidCommandType error") {
  const std::string yamlContent = R"(
prompt: ">"
commands:
  - name: test
    description: desc
    type: not_a_valid_type
)";

  VerifyLoadDefinitionError(yamlContent, DefinitionError::InvalidCommandType);
}


TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)
