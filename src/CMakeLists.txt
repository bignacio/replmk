set(replmk_SRCS
    replmk.cpp
    Core.cpp
    REPLDefinition.cpp
    TextUserInterface.cpp
    OutputBuffers.cpp
    ProcessExecutor.cpp
    AutoCleanableScriptFile.cpp
    OutputHistory.cpp
    CommandHistory.cpp
    REPLMaker.cpp
)

set(replmk_LIBS
     yaml-cpp
    cxxopts

    ftxui::dom
    ftxui::component
    ftxui::screen
)

foreach (target replmk)
    add_executable(${target} ${replmk_SRCS})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(${target} PRIVATE ${replmk_LIBS})
    target_compile_options(${target} PRIVATE ${CXX_PROJECT_FLAGS})
endforeach()

if(CLANG_TIDY)
    set_target_properties(replmk PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_PARAMS}")
endif()
