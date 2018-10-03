#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int rcv(int sock, char *buffer) {
    uint32_t in;
    read(sock, &in, sizeof(in));
    int messSize = ntohl(in);
    int retVal = read(sock, buffer, messSize);
    buffer[messSize] = '\0';
    return retVal;
}

void snd(int sock, char *message) {
    int in = htonl(strlen(message));
    write(sock, &in, sizeof(in));
    write(sock, message, strlen(message));
}
