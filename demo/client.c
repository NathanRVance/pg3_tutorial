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
#include "sockets.h"

void clientLoop(int sock) {
    char buffer[BUFFER];
    char input[2048];
    printf("> ");
    while(fgets(input, 2048, stdin)) {
        // Remove newline
        input[strlen(input)-1] = '\0';
        // Handle input
        if(strcmp(input, "LS") == 0) {
            snd(sock, input);
            printf("%s", rcv(sock, buffer));
        } else if(strcmp(input, "EXIT") == 0) {
            snd(sock, input);
            exit(0);
        } else if(strstr(input, "CD ") == input) {
            snd(sock, input);
            int ret = rcvInt(sock);
            if(ret == -2) printf("The directory does not exist on server\n");
            else if(ret == -1) printf("Error in changing directory\n");
            else printf("Changed current directory\n");
        } else if(strstr(input, "DL ") == input) {
            snd(sock, input);
            if(! rcvInt(sock)) {
	        char *md5 = md5sum(input+3);
		char *in = (strrchr(input, '/') == NULL)? input+3 : strrchr(input, '/')+1;
		rcvFile(sock, in);
	        snd(sock, md5);
	        free(md5);
                printf("%s", rcv(sock, buffer));
	    }
            else printf("File does not exist\n");
        } else if(strstr(input, "UP ") == input) {
            snd(sock, input);
	    char *md5 = md5sum(input+3);
	    //printf("Calculated: %s\n", md5);
	    snd(sock, md5);
	    free(md5);
            sndFile(sock, input+3);
            printf("%s", rcv(sock, buffer));
        } else if(strstr(input, "RM ") == input) {
            snd(sock, input);
            if(rcvInt(sock)) {
                printf("ERROR: File does not exist\n> ");
                continue;
            }
            printf("Are you sure? ");
            fgets(input, 2048, stdin);
            input[strlen(input)-1] = '\0';
            snd(sock, input);
            if(strcmp(input, "Yes") != 0) {
                printf("Delete abandoned by the user!\n");
            } else {
                if(rcvInt(sock)) printf("ERROR: Incorrect permissions\n");
                else printf("File deleted.\n");
            }
        } else if(strstr(input, "MKDIR ") == input) {
            snd(sock, input);
            int retval = rcvInt(sock);
            if(retval == -2) printf("The directory already exists on server\n");
            else if(retval == -1) printf("Error in making directory\n");
            else printf("The directory was successfully made\n");
        } else if(strstr(input, "RMDIR ") == input) {
            snd(sock, input);
            int retval = rcvInt(sock);
            if(retval == -2) printf("The directory is not empty\n");
            else if(retval == -1) printf("The directory does not exist on server\n");
            else {
                printf("Are you sure? ");
                fgets(input, 2048, stdin);
                input[strlen(input)-1] = '\0';
                snd(sock, input);
		if(strcmp(input, "Yes") != 0) {
                    printf("Delete abandoned by the user!\n");
                } else {
                    if(rcvInt(sock)) printf("Failed to delete directory\n");
                    else printf("Directory deleted.\n");
                }
            }
        }
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
    int sock = makeConn(host, port);
    printf("Connection established\n");
    clientLoop(sock);
    return 0;
}

