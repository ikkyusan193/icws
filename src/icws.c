#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "pcsa_net.h"
#include <getopt.h>

/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 1024

typedef struct sockaddr SA;

struct survival_bag {
    struct sockaddr_storage clientAddr;
    int connFd;
    char rootFolder[MAXBUF];
};

void respond_server(int connFd, char *path) {
    char buf[MAXBUF];
    int inputFd = open(path, O_RDONLY);
    if(inputFd <= 0) {
        printf("inputFd was %d\n", inputFd);
    }
    ssize_t numRead;
    
    struct stat s;
    int slen;
    char* type = "null";
    char* ext = strrchr(path, '.');
    ext = ext+1;

    if(stat(path, &s)>=0) {
        if(strcmp(ext, "html")==0) type = "text/html";
        else if(strcmp(ext, "jpg")==0) type = "image/jpg";
        else if(strcmp(ext, "jpeg")==0) type = "image/jpeg";

        if(strcmp(type, "null")==0) {
            char * msg = "respond with 404\n";
            write_all(connFd, msg , strlen(msg) );
            close(inputFd);
            return ;
        }

        sprintf(buf,
            "HTTP/1.1 200 OK\r\n"
            "Server: Tiny\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n", s.st_size, type);
        write_all(connFd, buf, strlen(buf));
    }
    while ((numRead = read(inputFd, buf, MAXBUF)) > 0) {
        /* What about short counts? */
        write_all(connFd, buf, numRead);
    }
    close(inputFd);
}

void serve_http(int connFd, char *rootFolder) {
    char buf[MAXBUF];

    /* Drain the remaining of the request */
    while (read_line(connFd, buf, MAXBUF) > 0) {
        if (strcmp(buf, "\r\n") == 0) break;
        printf("LOG: %s\n", buf);
        /* [METHOD] [URI] [HTTPVER] */
        char method[MAXBUF], uri[MAXBUF], httpVer[MAXBUF];
        sscanf(buf, "%s %s %s", method, uri, httpVer);

        if (strcasecmp(method, "GET") == 0 && uri[0] == '/') {
            char path[MAXBUF];
            strcpy(path, rootFolder);
            strcat(path, uri);
            printf("LOG: Sending %s\n", path);
            respond_server(connFd, path);
        }
        else {
            printf("LOG: Unknown request\n");
        }     
    }    
}

void* conn_handler(void *args) {
    struct survival_bag *context = (struct survival_bag *) args;
    
    pthread_detach(pthread_self());
    serve_http(context->connFd, context->rootFolder);
    close(context->connFd);
    
    free(context); /* Done, get rid of our survival bag */
    return NULL; /* Nothing meaningful to return */
}

int main(int argc, char* argv[])
{

    int c = 0;
    int listenPort = -1;
    char wwwRoot[MAXBUF];

    static struct option long_options[] = 
    {
        {"port",      required_argument,       0,  'p' },
        {"root",      required_argument,       0,  'r' },
        {0,0,0,0}
    };


    int long_index =0;
    while ((c = getopt_long(argc, argv,"apl:b:", 
                   long_options, &long_index )) != -1) {
        switch (c) {
             case 'p' : listenPort = atoi(optarg);
                 break;
             case 'r' : strcpy(wwwRoot,optarg);
                 break;
             default: 
                 exit(EXIT_FAILURE);
        }
    }

    if (listenPort == -1){
        printf("Invalid Port no.")
        exit(EXIT_FAILURE);
    }



    //todo here maybe?
    int listenFd = open_listenfd(argv[1]);

    for (;;) {
        struct sockaddr_storage clientAddr;
        socklen_t clientLen = sizeof(struct sockaddr_storage);
        pthread_t threadInfo;

        int connFd = accept(listenFd, (SA *) &clientAddr, &clientLen);
        if (connFd < 0) { 
            fprintf(stderr, "Failed to accept\n"); 
            sleep(1);
            continue; 
        }

        char hostBuf[MAXBUF], svcBuf[MAXBUF];
        if (getnameinfo((SA *) &clientAddr, clientLen, 
                        hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
            printf("Connection from %s:%s\n", hostBuf, svcBuf);
        else
            printf("Connection from ?UNKNOWN?\n");
        
        struct survival_bag *context = 
            (struct survival_bag *) malloc(sizeof(struct survival_bag));

        context->connFd = connFd;
        memcpy(&context->clientAddr, &clientAddr, sizeof(struct sockaddr_storage));
        strcpy(context->rootFolder, argv[2]);

        pthread_create(&threadInfo, NULL, conn_handler, (void *) context);
        // serve_http(connFd, argv[2]);
    }
}
