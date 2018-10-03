#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include "sockets.h"

void* serveLoop(void *s) {
    int sock = *((int*) s);
//int serveLoop(int sock) {
    char buffer[2048];
    while(1) {
        if(rcv(sock, buffer) < 1) {
	    close(sock);
            return 0;
	}
	printf(buffer);
    }
}

int main(int argc, char *argv[]) {
    int serve = 0;
    char *host = "127.0.0.1";
    int port = 8080;
    if(argc >= 2)
        port = atoi(argv[1]);
    int serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(serversock, (struct sockaddr*) &addr, sizeof(addr));
    listen(serversock, 1);
    printf("Accepting connections on port %d\n", port);
    while(1) {
        int sock = accept(serversock, NULL, NULL);
        printf("Connection established\n");
	pthread_t pth;
	pthread_create(&pth, NULL, serveLoop, &sock);
        //serveLoop(sock);
    }
    return 0;
}
