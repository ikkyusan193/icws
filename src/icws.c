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
#include "time.h"

/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 1024

typedef struct sockaddr SA;

void respond_server(int connFd, char *path, int get) {
    char buf[MAXBUF];
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
        else if(strcmp(ext, "png")==0) type = "image/png";
        else if(strcmp(ext, "gif")==0) type = "image/gif";
        else if(strcmp(ext, "js")==0) type = "text/javascript";
        else if(strcmp(ext, "txt")==0) type = "text/plain";
        else if(strcmp(ext, "css")==0) type = "text/css";

        if(strcmp(type, "null")==0) {
            char * msg = "respond with 404\n";
            write_all(connFd, msg , strlen(msg) );
            return ;
        }

        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char date[50];
        strftime(date,sizeof(date),"%c",tm);   
        struct stat filestat;
        stat(path,&filestat);

        sprintf(buf,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s\r\n"
            "Server: Tiny\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n"
            "Last-Modified: %s\r\n" ,date,s.st_size, type, ctime(&filestat.st_mtime));
        write_all(connFd, buf, strlen(buf));

    }else{
        sprintf(buf,"Error 404, File not found\r\n");
        write_all(connFd,buf, strlen(buf));
    }

    if(get == 1){
        int inputFd = open(path, O_RDONLY);
        if(inputFd <= 0) {
            printf("inputFd was %d\n", inputFd);
        }
        while ((numRead = read(inputFd, buf, MAXBUF)) > 0) {
            /* What about short counts? */
            write_all(connFd, buf, numRead);
        }
        close(inputFd);
    }
}

void serve_http(int connFd, char *rootFolder) {
    char buf[MAXBUF];

    int readRet = read(connFd,buf,8192);
    if (!readRet) 
        return ;  /* Quit if we can't read the first line */
    // printf("LOG: %s\n", buf);
    /* [METHOD] [URI] [HTTPVER] */

    Request *request = parse(buf,readRet,connFd);
    if(request == NULL){
        return ;
    }

    if(strcmp(request->http_version,"HTTP/1.1") != 0){
        sprintf(buf,"Error 505, bad version numbers\r\n");
        write_all(connFd,buf, strlen(buf));
        return ;
    }

    if(strcasecmp(request->http_method, "GET") == 0 && request->http_uri[0] == '/'){
        char path[MAXBUF];
        strcpy(path, rootFolder);
        strcat(path, request->http_uri);
        printf("LOG: Sending %s\n", path);
        respond_server(connFd, path, 1);
    }
    else if(strcasecmp(request->http_method, "HEAD") == 0 && request->http_uri[0] == '/'){
        char path[MAXBUF];
        strcpy(path, rootFolder);
        strcat(path, request->http_uri);
        printf("LOG: Sending %s\n", path);
        respond_server(connFd, path , 0);
    }    
    else {
        sprintf(buf,"Error 501, unsupported methods\r\n");
        write_all(connFd,buf, strlen(buf));
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
