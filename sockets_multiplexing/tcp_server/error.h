#ifndef BEEJ_SOCKETS_ERROR_H
#define BEEJ_SOCKETS_ERROR_H

#include <stdio.h>
#include <netdb.h>

void print_error(const char* message, int status) {
    fprintf(stderr, "%s %s\n", message, gai_strerror(status));
}

#endif //BEEJ_SOCKETS_ERROR_H
