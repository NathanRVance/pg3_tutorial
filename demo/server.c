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

void xfer(int sock, int isRcv, char *fname) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long before = 1000000 * tv.tv_sec + tv.tv_usec;
    unsigned long size;
    char *md5source, *md5dest;
    char buffer[BUFFER];
    if(isRcv) {
	char *tmp = rcv(sock, buffer);
	md5source = malloc(sizeof(char) * strlen(tmp));
	strcpy(md5source, tmp);
        size = rcvFile(sock, fname);
	md5dest = md5sum(fname);
    } else {
        md5source = md5sum(fname);
        size = sndFile(sock, fname);
	md5dest = rcv(sock, buffer);
    }
    gettimeofday(&tv,NULL);
    unsigned long duration = 1000000 * tv.tv_sec + tv.tv_usec - before;
    unsigned long rate = size * 1000000LU / duration;
    float mbps = (float) rate / 1000000;
    float secs = (float) duration / 1000000;
    char throughput[1024];
    //printf("md5 source/dest:\n\t\"%s\"\n\t\"%s\"\n", md5source, md5dest);
    sprintf(throughput, "%lu bytes transferred in %f seconds: %f Megabytes/sec\n\tMD5 hash: %s %s\n", size, secs, mbps, md5dest, (strcmp(md5source, md5dest) == 0)? "(matches)" : "ERROR: Does not match!");
    snd(sock, throughput);
}

//void* serveLoop(void *s) {
//    int sock = *((int*) s);
int serveLoop(int sock) {
    char buffer[BUFFER];
    while(1) {
        char *op = rcv(sock, buffer);
	//printf("RCVD: %s\n", op);
        if(strcmp(op, "LS") == 0) {
            char *ls = sysc("ls -l");
	    snd(sock, ls);
	    free(ls);
        }
	else if(strcmp(op, "EXIT") == 0) {
	    cleanup(sock);
	    return 0;
	}
	else if(strstr(op, "CD ") == op) {
	    int res = chdir(op+3);
	    if(errno == ENOTDIR) sndInt(sock, -2);
	    else sndInt(sock, res);
	}
	else if(strstr(op, "DL ") == op) {
	    if(!access(op+3, F_OK)) {
		sndInt(sock, 0);
		char fname[strlen(op)];
		strcpy(fname, op+3);
		xfer(sock, 0, fname);
	    } else {
                sndInt(sock, -1);
	    }
	}
	else if(strstr(op, "UP ") == op) {
	    op = (strrchr(op, '/') == NULL)? op+3 : strrchr(op, '/')+1;
            char fname[strlen(op)];
	    strcpy(fname, op);
	    xfer(sock, 1, fname);
	} else if(strstr(op, "RM ") == op) {
            char fname[strlen(op)];
	    strcpy(fname, op+3);
	    if(access(fname, F_OK)) {
	        sndInt(sock, -1);
		continue;
	    }
	    sndInt(sock, 0);
	    if(strcmp(rcv(sock, buffer), "Yes") == 0) {
	        sndInt(sock, remove(fname));
	    }
	} else if(strstr(op, "MKDIR ") == op) {
	    DIR *d = opendir(op+6);
	    if(d) {
                sndInt(sock, -2);
		closedir(d);
	    } else {
                sndInt(sock, mkdir(op+6, 0700));
	    }
	} else if(strstr(op, "RMDIR ") == op) {
            char dname[strlen(op)];
            strcpy(dname, op+6);
	    DIR *d = opendir(dname);
	    if(d == NULL) {
                sndInt(sock, -1);
		continue;
	    }
	    int n;
	    for(n=0; n <= 2 && readdir(d) != NULL; n++);
	    closedir(d);
            if(n == 3) {
                sndInt(sock, -2);
                continue;
            }
            sndInt(sock, 0);
            if(strcmp(rcv(sock, buffer), "Yes") == 0) {
                sndInt(sock, rmdir(dname));
            }
	}
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
    while(1) {
        printf("Accepting connections on port %d\n", port);
        int sock = accept(serversock, NULL, NULL);
        printf("Connection established\n");
	//pthread_t pth;
	//pthread_create(&pth, NULL, serveLoop, &sock);
        serveLoop(sock);
    }
    return 0;
}
