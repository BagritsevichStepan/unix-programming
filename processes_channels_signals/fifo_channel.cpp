#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

namespace {

    const char* kInFifoName = "in.fifo";
    const char* kOutFifoName = "out.fifo";

    constexpr size_t kBufferSize = 512;

    int MkFifoFailed() {
        std::cerr << "Failed during mkfifo: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int ReadFailed() {
        std::cerr << "Failed during read: " << errno << std::endl;
        return EXIT_FAILURE;
    }
}

int main() {
    if (mkfifo(kInFifoName, 0666) == -1) {
        return MkFifoFailed();
    }
    if (mkfifo(kOutFifoName, 0666) == -1) {
        return MkFifoFailed();
    }

    int input = open(kInFifoName, O_RDONLY);
    int output = open(kOutFifoName, O_WRONLY);
    char buffer[kBufferSize];

    while (true) {
        ssize_t bytes_read = read(input, &buffer, kBufferSize);

        if (bytes_read < 0) {
            close(input);
            close(output);
            unlink(kInFifoName);
            unlink(kOutFifoName);
            return ReadFailed();
        }

        if (!bytes_read) {
            break;
        }

        write(output, &buffer, bytes_read);
    }

    close(input);
    close(output);
    unlink(kInFifoName);
    unlink(kOutFifoName);
    return 0;
}
