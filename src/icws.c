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
#include <pthread.h>
#include "time.h"
#include <poll.h>

/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 8192 

typedef struct sockaddr SA;


/* TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO */
typedef struct Task{
    struct sockaddr_storage clientAddr;
    int connFd;
    char wwwRoot[MAXBUF];
} Task;

Task taskQueue[256];


int taskCount = 0;
pthread_mutex_t mutexQueue;
pthread_mutex_t parseQueue;
pthread_cond_t condQueue;

int timeout = 600; //default timeout = 1min
struct pollfd fds[1];
int pret;
//////////////////////////////////////////////////////////



void mysprinf(int code, char *path, unsigned long stsize ,char* type,char* connectionType ,char* buf){  

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[100];
    strftime(date,sizeof(date),"%c",tm); 

    struct stat filestat;
    stat(path,&filestat);

    char time[100]; 
    strcpy(time,ctime(&filestat.st_mtime));
    time[strlen(time)-1] = '\0';

    switch(code){
        case 404:
                sprintf(buf,
                    "HTTP/1.1 %d Not Found\r\n"
                    "Date: %s\r\n"
                    "Server: Tiny\r\n"
                    "Connection: %s\r\n"
                    "Content-type: NULL\r\n\r\n"
                    , code, date, connectionType);
                    break;
        case 408:
                sprintf(buf,
                    "HTTP/1.1 %d Request Timeout\r\n"
                    "Date: %s\r\n"
                    "Server: Tiny\r\n"
                    "Connection: %s\r\n"
                    "Content-type: NULL\r\n\r\n"
                    , code, date, connectionType);
                    break;
        case 411:
                sprintf(buf,
                    "HTTP/1.1 %d Length Required\r\n"
                    "Date: %s\r\n"
                    "Server: Tiny\r\n"
                    "Connection: %s\r\n"
                    "Content-type: NULL\r\n\r\n"
                    , code, date, connectionType);
                    break;
        case 505:
                sprintf(buf,
                    "HTTP/1.1 %d HTTP Version Not Supported\r\n"
                    "Date: %s\r\n"
                    "Server: Tiny\r\n"
                    "Connection: %s\r\n"
                    "Content-type: NULL\r\n\r\n"
                    , code, date, connectionType);
                    break;
        case 200:
                sprintf(buf,
                    "HTTP/1.1 %d OK\r\n"
                    "Date: %s\r\n"
                    "Server: Tiny\r\n"
                    "Connection: %s\r\n"
                    "Content-length: %lu\r\n"
                    "Content-type: %s\r\n"
                    "Last-Modified: %s\r\n\r\n" , code, date, connectionType ,stsize, type, time);
                    break;
    }



}


void respond_server(int connFd, char *path, int get, char* connectionType) {
    char buf[MAXBUF];
    ssize_t numRead;
    
    struct stat s;
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
        //empty so give 404
        if(strcmp(type, "null")==0) {
            mysprinf(404,path,0,type,"close",buf);
            write_all(connFd, buf , strlen(buf) );
            return ;
        }
        // everything is ok
        mysprinf(200,path,s.st_size,type,connectionType,buf);
        write_all(connFd, buf, strlen(buf));
    }else{
        mysprinf(404,path,0,type,"close",buf);
        write_all(connFd,buf, strlen(buf));
        return ;
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


const char* keepConnection(Request* request){
    int i;
    for(i = 0; i < request->header_count;i++){
        if(strcasecmp(request->headers[i].header_name,"Connection") == 0){
            if(strcasecmp(request->headers[i].header_value,"keep-alive") == 0){
                return "keep-alive";
            }
            else
            {
                return "close";
            }
        }
    }
    return "close";
}



void serve_http(int connFd, char *rootFolder) {
    char buf[MAXBUF];
    memset(buf,0,MAXBUF);
    char sbuf[MAXBUF];
    memset(sbuf,0,MAXBUF);
    int readRet;
    while(1)
    {
        fds[0].fd = connFd;
        fds[0].events = 0;
        fds[0].events = POLLIN;
        pret = poll(fds,1,timeout*1000);
        if (pret == 0)
        {
            mysprinf(408,"",0,"null","close",buf);
            write_all(connFd,buf,strlen(buf));
            return;
        }
        else
        {
            if((readRet = read(connFd,sbuf,MAXBUF)) > 0){
                strcat(buf,sbuf);
                memset(sbuf,0,MAXBUF);
                if(strstr(buf, "\r\n\r\n") != NULL){
                    pthread_mutex_lock(&parseQueue);
                    Request *request = parse(buf,sizeof(buf),connFd);
                    pthread_mutex_unlock(&parseQueue);
                    if (!readRet) return ;  
                    /* [METHOD] [URI] [HTTPVER] */
                    if(request == NULL){
                        //TODO null request 
                        return ;
                    }
                    char checkConnection[20];
                    //check connection will return "close" or "keep-alive" only.
                    strcpy(checkConnection,keepConnection(request));
                    //everything work now but need change to HTTP/1.0
                    if (strcasecmp(request->http_version, "HTTP/1.1")){
                        mysprinf(505,"",0,"null",checkConnection,buf);
                        write_all(connFd,buf, strlen(buf));
                        return ;
                    }
                    char path[MAXBUF];
                    strcpy(path, rootFolder);
                    strcat(path, request->http_uri);
                    //add index.html to path if no have
                    if(strcmp(request->http_uri,"/") == 0){
                        strcat(path,"index.html");
                    } 
                    if(strcasecmp(request->http_method, "GET") == 0 && request->http_uri[0] == '/'){
                        printf("LOG: Sending %s\n", path);
                        respond_server(connFd, path, 1,checkConnection);
                    }
                    else if(strcasecmp(request->http_method, "HEAD") == 0 && request->http_uri[0] == '/'){
                        printf("LOG: Sending %s\n", path);
                        respond_server(connFd, path , 0,checkConnection);
                    }    
                    else 
                    {
                        mysprinf(505,"",0,"null",checkConnection,buf);
                        write_all(connFd,buf, strlen(buf));
                        return ;
                    }
                    free(request);
                    memset(buf,0,MAXBUF);
                    if (strcasecmp(checkConnection,"close") == 0) break;
                }
            }
        }
    }
}

void submitTask(Task task){
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}


void executeTask(Task* task){
    serve_http(task->connFd,task->wwwRoot);
    close(task->connFd);
}

void *startThread(void* args){
    while(1){
        Task task;
        pthread_mutex_lock(&mutexQueue);
        while(taskCount == 0){
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        task = taskQueue[0];
        int i;
        for(i = 0; i < taskCount - 1; i++){
            taskQueue[i] = taskQueue[i+1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);
        executeTask(&task);
    }
}


int main(int argc, char* argv[])
{
    int THREAD_NUM;

    char listenPort[MAXBUF];
    char wwwRoot[MAXBUF];



    int c = 0;
    static struct option long_options[] = 
    {
        {"port",      required_argument,       0,  'p' },
        {"root",      required_argument,       0,  'r' },
        {"numthreads",required_argument,       0,  'n'},
        {"timeout",   required_argument,       0,  't'},
        {0,0,0,0}
    };
    int long_index =0;
    while ((c = getopt_long(argc, argv,"p:r:n:t:", long_options, &long_index )) != -1) {
        switch (c) {
             case 'p' : strcpy(listenPort,optarg);
                 break;
             case 'r' : strcpy(wwwRoot,optarg);
                 break;
             case 'n' : THREAD_NUM = atoi(optarg);
                 break;
             case 't' : timeout = atoi(optarg);
                 break;    
             default:
                    printf("Invalid/Unknown option");
                    return 1;
        }
    }

    pthread_t threadPool[THREAD_NUM];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&parseQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    int i;
    for(i = 0; i < THREAD_NUM; i++){
        if (pthread_create(&threadPool[i], NULL, &startThread, NULL) != 0){
            perror("Failed to create thread");
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
       
        struct Task task;
        task.connFd = connFd;
        memcpy(&task.clientAddr, &clientAddr, sizeof(struct sockaddr_storage));
        strcpy(task.wwwRoot, wwwRoot);
        submitTask(task);
    }

    // for(int i = 0; i < THREAD_NUM; i++){
    //     if (pthread_join(&threadPool[i],NULL) != 0){
    //         perror("Failed to join thread\n");
    //     }
    // }
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

    return 0;
}
