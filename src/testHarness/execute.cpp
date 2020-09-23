#include <string>
#include <cassert>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "execute.hpp"

static const int READ_END = 0;
static const int WRITE_END = 1;

static const int STDIN_FD = 0;
static const int STDOUT_FD = 1;
static const int STDERR_FD = 2;

static void nonblocking(int fd, bool enabled);

static int needsRetry(int rtn);

static void waitUntilOneReady(int wrFd, bool wrDone,
                              int rd1Fd, bool rd1Done,
                              int rd2Fd, bool rd2Done);

static void pumpTo(std::string &str, int fd, bool &done);

static void pumpFrom(int fd, std::string &str, bool &done);

static void waitChild(pid_t pid, int &exitCode);

// Run the program in command[0], where `command` must be a NULL-terminated
// array (like `execv` expects). Supply the given string as stdin to the
// program, wait until it complete, and report its exit status, stdout
// as a string, and stderr s a string. The exit status is set to a signal
// number if the program exits with a signal.
ExecResult execProgram(const char *const *command, std::string input) {
    signal(SIGPIPE, SIG_IGN);

    int in[2];
    if (pipe(in) != 0)
        throw std::runtime_error("stdin pipe failed");

    int out[2];
    if (pipe(out) != 0)
        throw std::runtime_error("stdout pipe failed");

    int err[2];
    if (pipe(err) != 0)
        throw std::runtime_error("stdout pipe failed");

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("fork failed");
    else if (pid == 0) {
        /* child */
        dup2(in[READ_END], STDIN_FD);
        dup2(out[WRITE_END], STDOUT_FD);
        dup2(err[WRITE_END], STDERR_FD);

        close(in[READ_END]);
        close(in[WRITE_END]);
        close(out[READ_END]);
        close(out[WRITE_END]);
        close(err[READ_END]);
        close(err[WRITE_END]);

        execv(command[0], (char *const *) command);

        /* Getting here means that the execve failed */
        {
            const char *msg = "exec failed";
            write(STDERR_FD, msg, strlen(msg));
            exit(1);
        }
    } else {
        /* parent */
        bool inDone = false, outDone = false, errDone = false;
        ExecResult r;

        close(in[READ_END]);
        close(out[WRITE_END]);
        close(err[WRITE_END]);

        do {
            waitUntilOneReady(in[WRITE_END], inDone,
                              out[READ_END], outDone,
                              err[READ_END], errDone);
            pumpTo(input, in[WRITE_END], inDone);
            pumpFrom(out[READ_END], r.out, outDone);
            pumpFrom(err[READ_END], r.err, errDone);
        } while (!inDone || !outDone || !errDone);

        waitChild(pid, r.exitCode);

        return r;
    }
}

// Enable/disable nonblocking mode for a file descriptor
static void nonblocking(int fd, bool enabled) {
    int oldFlags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, (enabled
                        ? oldFlags | O_NONBLOCK
                        : oldFlags - (oldFlags & O_NONBLOCK)));
}

// Check whether a system call result means "retry"
static int needsRetry(int rtn) {
    return (rtn == -1) && (errno == EINTR);
}

// Block until reading or writing is possible on one of the
// given file descriptors
static void waitUntilOneReady(int wrFd, bool wrDone,
                              int rd1Fd, bool rd1Done,
                              int rd2Fd, bool rd2Done) {
    struct pollfd pollInfo[3];
    int count = 0;

    if (!wrDone) {
        pollInfo[count].fd = wrFd;
        pollInfo[count++].events = POLLOUT;
    }
    if (!rd1Done) {
        pollInfo[count].fd = rd1Fd;
        pollInfo[count++].events = POLLIN;
    }
    if (!rd2Done) {
        pollInfo[count].fd = rd2Fd;
        pollInfo[count++].events = POLLIN;
    }

    if (count == 0)
        return;

    int rtn;
    do {
        rtn = poll(pollInfo, static_cast<nfds_t>(count), -1);
    } while (needsRetry(rtn));

    if (rtn == -1)
        throw std::runtime_error("poll failed");
}

// Move characters from the given string to the given file descriptor,
// closing the file descriptor if the string is empty. The `done` flag
// is consulted and possibly set to indicate whether the file descriptor
// is still open.
static void pumpTo(std::string &str, int fd, bool &done) {
    if (!done) {
        ssize_t len;
        nonblocking(fd, true);
        do {
            len = write(fd, str.c_str(), str.length());
        } while (needsRetry((int) len));
        nonblocking(fd, false);
        if ((len < 0) && (errno == EAGAIN)) {
            // not ready to write
        } else {
            if (len < 0)
                str.erase(0, str.length()); // treat error like writing all
            else
                str.erase(0, static_cast<unsigned long>(len));
            if (str.length() == 0) {
                done = true;
                close(fd);
            }
        }
    }
}

// Move characters from the given file descriptor to the given string
// closing the file descriptor on EOF. The `done` flag is consulted
// and possibly set to indicate whether the file descriptor is still
// open.
static void pumpFrom(int fd, std::string &str, bool &done) {
    if (!done) {
        char buffer[256];
        ssize_t len;
        nonblocking(fd, true);
        do {
            len = read(fd, buffer, sizeof(buffer) - 1);
        } while (needsRetry((int) len));
        nonblocking(fd, false);
        if ((len < 0) && (errno == EAGAIN)) {
            // nothing ready to read
        } else {
            if (len < 1) {
                // error or EOF
                done = true;
                close(fd);
            } else {
                ssize_t oldLen = str.length();
                str.insert(static_cast<unsigned long>(oldLen), buffer, static_cast<unsigned long>(len));
                assert(str.length() == oldLen + len);
            }
        }
    }
}

// Wait until a process has terminated
static void waitChild(pid_t pid, int &exitCode) {
    int status;
    pid_t rtn;

    do {
        rtn = waitpid(pid, &status, 0);
    } while (needsRetry(rtn));
    if (rtn == -1)
        throw std::runtime_error("waitpid failed");
    if (WIFEXITED(status))
        exitCode = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        exitCode = WTERMSIG(status);
    else
        throw std::runtime_error("unrecognized status from waitpid");
}
