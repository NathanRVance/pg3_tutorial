#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "sockets.h"

void clientLoop(int sock) {
    char input[2048];
    printf("> ");
    while(fgets(input, 2048, stdin)) {
        snd(sock, input);
        printf("> ");
    }
}

int main(int argc, char *argv[]) {
    int serve = 0;
    char *host = "127.0.0.1";
    int port = 8080;
    if(argc >= 2)
            host = argv[1];
    if(argc >= 3)
            port = atoi(argv[2]);
    printf("Connecting to %s on port %d\n", host, port);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    struct hostent *he;
    if((he = gethostbyname(host)) != NULL) {
        struct in_addr *address = (struct in_addr*) he->h_addr_list[0];
        host = (char*) calloc(64, sizeof(char));
        strcpy(host, inet_ntoa(*address));
    }
    inet_pton(AF_INET, host, &addr.sin_addr);
    if(connect(sock, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        printf("Connection failed\n");
	exit(1);
    }
    printf("Connection established\n");
    clientLoop(sock);
    return 0;
}

