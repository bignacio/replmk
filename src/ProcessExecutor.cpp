#include "ProcessExecutor.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <algorithm>
#include <vector>
#include <array>
#include <cstdlib>

namespace replmk {

namespace {

struct ProcessExecutorStdPipes {
    std::array<int, 2> stdoutPipe;
    std::array<int, 2> stderrPipe;
};

auto childProcess(ProcessExecutorStdPipes pipes, std::string cmd, const std::vector<std::string>& args) -> void {
    // Child process
    dup2(pipes.stdoutPipe[1], STDOUT_FILENO);
    dup2(pipes.stderrPipe[1], STDERR_FILENO);
    close(pipes.stdoutPipe[0]);
    close(pipes.stdoutPipe[1]);
    close(pipes.stderrPipe[0]);
    close(pipes.stderrPipe[1]);

    // Build argv
    std::vector<char*> argv;
    std::string cmd_str(cmd);
    argv.push_back(cmd_str.data());
    std::vector<std::string> args_copy = args;
    for (auto& arg : args_copy) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    execvp(argv[0], argv.data());
    // If execvp fails
    _exit(EXIT_FAILURE);
}

auto handleProcessOutput(int fileDescriptor, const OnCommandOutput& callback) {
    constexpr size_t BufferSize = 4096;
    std::array<char, BufferSize> buf{};
    ssize_t bytesRead = read(fileDescriptor, buf.data(), BufferSize);
    if (bytesRead > 0) {
        callback(std::string_view(buf.data(), static_cast<size_t>(bytesRead)));
    } else {
        close(fileDescriptor);
        return false;
    }

    return true;
}

auto parentProcessEventLoop(int stdout_fd, int stdoutFd, const CommandOutputCallbacks& callbacks) {
    fd_set readfds;
    int maxfd = std::max(stdout_fd, stdoutFd);
    bool stdoutOpen = true;
    bool stderrOpen = true;

    while (stdoutOpen || stderrOpen) {
        FD_ZERO(&readfds);
        if (stdoutOpen) {
            FD_SET(stdout_fd, &readfds);
        }
        if (stderrOpen) {
            FD_SET(stdoutFd, &readfds);
        }

        int ret = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        if (ret < 0) {
            break;
        }

        if (stdoutOpen && FD_ISSET(stdout_fd, &readfds)) {
            stdoutOpen = handleProcessOutput(stdout_fd, callbacks.onStdOut);
        }
        if (stderrOpen && FD_ISSET(stdoutFd, &readfds)) {
            stderrOpen = handleProcessOutput(stdoutFd, callbacks.onStdErr);
        }
    }
}

auto parentProcess(pid_t pid, ProcessExecutorStdPipes pipes, const CommandOutputCallbacks& callbacks) -> bool {
    // Parent process
    close(pipes.stdoutPipe[1]);
    close(pipes.stderrPipe[1]);

    parentProcessEventLoop(pipes.stdoutPipe[0], pipes.stderrPipe[0], callbacks);

    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

} // namespace

auto executeAndCaptureOutputs(std::string_view cmd, const std::vector<std::string>& args,
                              const CommandOutputCallbacks& callbacks) -> bool {
    ProcessExecutorStdPipes pipes{};

    if (pipe(pipes.stdoutPipe.data()) != 0 || pipe(pipes.stderrPipe.data()) != 0) {
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        close(pipes.stdoutPipe[0]);
        close(pipes.stdoutPipe[1]);
        close(pipes.stderrPipe[0]);
        close(pipes.stderrPipe[1]);
        return false;
    }

    if (pid == 0) {
        childProcess(pipes, std::string(cmd), args);
    }

    return parentProcess(pid, pipes, callbacks);
}
} //namespace replmk