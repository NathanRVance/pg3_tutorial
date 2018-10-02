#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER 4096

int makeSock(int port, struct sockaddr_in *addr) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(port);
    return sock;
}

int makeConn(char *host, int port) {
    struct sockaddr_in addr;
    int sock = makeSock(port, &addr);
    struct hostent *he;
    if((he = gethostbyname(host)) != NULL) {
        struct in_addr *address = (struct in_addr*) he->h_addr_list[0];
        host = (char*) calloc(64, sizeof(char));
        strcpy(host, inet_ntoa(*address));
    }
    inet_pton(AF_INET, host, &addr.sin_addr);
    connect(sock, (struct sockaddr*) &addr, sizeof(addr));
    return sock;
}

int recvConn(int port, int *serversock) {
    if(! *serversock) {
        struct sockaddr_in addr;
        *serversock = makeSock(port, &addr);
        bind(*serversock, (struct sockaddr*) &addr, sizeof(addr));
    }
    listen(*serversock, 1);
    return accept(*serversock, NULL, NULL);
}

int rcvInt(int sock) {
    uint32_t in;
    read(sock, &in, sizeof(in));
    return ntohl(in);
}

char* rcv(int sock, char *buffer) {
    int messSize = rcvInt(sock);
    read(sock, buffer, messSize);
    buffer[messSize] = '\0';
    return buffer;
}

int rcvFile(int sock, char *name) {
    int size = rcvInt(sock);
    int toRet = size;
    FILE *f = fopen(name, "w");
    char buffer[BUFFER];
    while(size > 0) {
        int toRead = (size > BUFFER)? BUFFER : size;
        toRead = read(sock, buffer, toRead);
	if(fwrite(buffer, sizeof(char), toRead, f) != toRead) {
            printf("An error occurred!\n");
        }
	size -= toRead;
    }
    fclose(f);
    return toRet;
}

void sndInt(int sock, int in) {
    in = htonl(in);
    write(sock, &in, sizeof(in));
}

void snd(int sock, char *message) {
    sndInt(sock, strlen(message));
    write(sock, message, strlen(message));
}

int sndFile(int sock, char *name) {
    FILE *f = fopen(name, "r");
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    int toRet = size;
    rewind(f);
    sndInt(sock, size);
    char buffer[BUFFER];
    while(size > 0) {
        int toSend = (size > BUFFER)? BUFFER : size;
        if(fread(buffer, sizeof(char), toSend, f) != toSend) {
            printf("An error occurred!\n");
        }
	if(write(sock, buffer, toSend) != toSend) {
            printf("An error occurred!\n");
        }
	size -= toSend;
    }
    fclose(f);
    return toRet;
}

char* sysc(char *command) {
    FILE *f = popen(command, "r");
    if(f == NULL) printf("ERROR\n");
    int len = 1;
    char *input = calloc(len, sizeof(char));
    int buffsize = 512;
    char buffer[buffsize];
    while(fgets(buffer, buffsize, f) != NULL) {
        len += strlen(buffer);
        input = realloc(input, len);
        strcat(input, buffer);
    }
    pclose(f);
    return input;
}

char* md5sum(char *fname) {
    char md5[strlen(fname) + 8];
    strcpy(md5, "md5sum ");
    strcat(md5, fname);
    char *res = sysc(md5);
    *strchr(res, ' ') = '\0';
    return res;
}


void cleanup(int sock) {
    close(sock);
}
