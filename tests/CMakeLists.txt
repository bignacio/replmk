set(replmk_tests_SRCS
    replmk_test.cpp

    Command_test.cpp
    Core_test.cpp
    REPLDefinition_test.cpp
    CommandLineParser_test.cpp
    OutputBuffers_test.cpp
    AutoCleanableScriptFile_test.cpp
    CommandHistory_test.cpp
    OutputHistory_test.cpp
    ProcessExecutor_test.cpp
    TextUserInterface_test.cpp
    REPLMaker_test.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Core.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/REPLDefinition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/OutputBuffers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ProcessExecutor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/TextUserInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/AutoCleanableScriptFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/CommandHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/OutputHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/REPLMaker.cpp
)



foreach (target replmk-tests replmk-tests-msan replmk-tests-asan)
    add_executable(${target} ${replmk_tests_SRCS})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_include_directories(${target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

    target_link_libraries(${target} PRIVATE
        doctest_with_main

        yaml-cpp
        cxxopts

        ftxui::dom
        ftxui::component
        ftxui::screen
    )

    target_compile_options(${target} PRIVATE ${CXX_PROJECT_FLAGS})
endforeach()

# handle special targets
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(replmk-tests PRIVATE "-Wno-#warnings")
    target_compile_options(replmk-tests-asan PRIVATE "-Wno-#warnings")

    target_compile_options(replmk-tests-msan PRIVATE -fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 "-Wno-#warnings")
    target_link_options(replmk-tests-msan PRIVATE -fsanitize=memory -fsanitize-memory-track-origins=2)
endif()

target_compile_options(replmk-tests-asan PRIVATE -fsanitize=address)
target_link_options(replmk-tests-asan PRIVATE -fsanitize=address)


if(CLANG_TIDY)
    set_target_properties(replmk-tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_PARAMS},-cppcoreguidelines-avoid-do-while")
endif()