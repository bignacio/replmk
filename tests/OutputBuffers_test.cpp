#include <doctest/doctest.h>
#include "../src/OutputBuffers.h"

using namespace replmk;

//NOLINTBEGIN(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)

TEST_SUITE_BEGIN("OutputBuffers");


TEST_CASE("AddNewEntry triggers callback and stores entry") {
    bool callbackCalled = false;
    OutputBuffers buffers;
    buffers.SetOnOutputChangedEvent([&](const OutputBuffers&) {
        callbackCalled = true;
    });

    buffers.AddNewEntry({.prompt = "", .stdOutEntry="stdout text", .stdErrEntry="stderr text"});

    REQUIRE(callbackCalled);

    // Check buffer contents after AddNewEntry
    auto buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0].stdOutEntry == "stdout text");
    REQUIRE(buf[0].stdErrEntry == "stderr text");

    REQUIRE(buffers.AppendToLastStdOutEntry(" more"));
    REQUIRE(buffers.AppendToLastStdErrEntry(" error"));

    // Check buffer contents after appends
    buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0].stdOutEntry == "stdout text more");
    REQUIRE(buf[0].stdErrEntry == "stderr text error");

    callbackCalled = false;
    buffers.AppendToLastStdOutEntry(" again");
    REQUIRE(callbackCalled);

    // Check buffer contents after another append
    buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0].stdOutEntry == "stdout text more again");
    REQUIRE(buf[0].stdErrEntry == "stderr text error");
}

TEST_CASE("AppendToLastStdOutEntry and AppendToLastStdErrEntry on empty buffer") {
    OutputBuffers buffers;
    buffers.SetOnOutputChangedEvent([](const OutputBuffers&) {});

    REQUIRE_FALSE(buffers.AppendToLastStdOutEntry("should fail"));
    REQUIRE_FALSE(buffers.AppendToLastStdErrEntry("should fail"));

    // Buffer should still be empty
    auto buf = buffers.GetBuffer();
    REQUIRE(buf.empty());
}

TEST_CASE("AppendToLastEntry appends to correct field") {
    OutputBuffers buffers;
    buffers.SetOnOutputChangedEvent([](const OutputBuffers&) {});
    buffers.AddNewEntry({.prompt = "", .stdOutEntry="out", .stdErrEntry="err"});

    buffers.AppendToLastStdOutEntry("put");
    buffers.AppendToLastStdErrEntry("or");

    // Check buffer contents after first appends
    auto buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0].stdOutEntry == "output");
    REQUIRE(buf[0].stdErrEntry == "error");

    buffers.AddNewEntry({.prompt = "", .stdOutEntry="newout", .stdErrEntry="newerr"});
    REQUIRE(buffers.AppendToLastStdOutEntry("X"));
    REQUIRE(buffers.AppendToLastStdErrEntry("Y"));

    // Check buffer contents after second entry and appends
    buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 2);
    REQUIRE(buf[0].stdOutEntry == "output");
    REQUIRE(buf[0].stdErrEntry == "error");
    REQUIRE(buf[1].stdOutEntry == "newoutX");
    REQUIRE(buf[1].stdErrEntry == "newerrY");
}

TEST_CASE("Callback is not called if not set") {
    OutputBuffers buffers;
    buffers.AddNewEntry({.prompt = "", .stdOutEntry="a", .stdErrEntry="b"});
    buffers.AppendToLastStdOutEntry("c");
    buffers.AppendToLastStdErrEntry("d");

    // Check buffer contents
    auto buf = buffers.GetBuffer();
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0].stdOutEntry == "ac");
    REQUIRE(buf[0].stdErrEntry == "bd");
}

TEST_SUITE_END();

//NOLINTEND(readability-function-cognitive-complexity,cppcoreguidelines-avoid-do-while)