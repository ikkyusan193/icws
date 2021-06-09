#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "pcsa_net.h"
#include <getopt.h>
#include "parse.h"


/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 1024

typedef struct sockaddr SA;

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

    if (!read_line(connFd, buf, MAXBUF)) 
        return ;  /* Quit if we can't read the first line */

    printf("LOG: %s\n", buf);
    /* [METHOD] [URI] [HTTPVER] */

    //todo
    int fd_in = open(rootFolder,O_RDONLY);
    int readRet = read(fd_in,buf,8192);
    Request *request = parse(MAXBUF,readRet,fd_in);
    if(strcasecmp(request->http_method, "GET") == 0 && request->http_uri[0] == '/'){
        char path[MAXBUF];
        strcpy(path, rootFolder);
        strcat(path, request->http_uri);
        printf("LOG: Sending %s\n", path);
        respond_server(connFd, path);
    }else if(strcasecmp(request->http_method, "HEAD") == 0 && request->http_uri[0] == '/'){
        char path[MAXBUF];
        strcpy(path, rootFolder);
        strcat(path, request->http_uri);
        printf("LOG: Sending %s\n", path);
        respond_server(connFd, path);
    }    
    else {
        printf("LOG: Unknown request\n");
    }            
}




int main(int argc, char* argv[])
{

    int c = 0;
    char listenPort[MAXBUF];
    char wwwRoot[MAXBUF];

    static struct option long_options[] = 
    {
        {"port",      required_argument,       0,  'p' },
        {"root",      required_argument,       0,  'r' },
        {0,0,0,0}
    };

    int long_index =0;
    while ((c = getopt_long(argc, argv,"p:r:", long_options, &long_index )) != -1) {
        switch (c) {
             case 'p' : strcpy(listenPort,optarg);
                 break;
             case 'r' : strcpy(wwwRoot,optarg);
                 break;
        }
    }

    if (strlen(listenPort) == 0 || strlen(wwwRoot) == 0){
        printf("Invalid input");
        exit(-1);
    }

    // printf("%s\n",listenPort);
    // printf("%s\n",wwwRoot);

    int listenFd = open_listenfd(listenPort);

    for (;;) {
        struct sockaddr_storage clientAddr;
        socklen_t clientLen = sizeof(struct sockaddr_storage);

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
        
        serve_http(connFd, wwwRoot);
        close(connFd);
    }
}
