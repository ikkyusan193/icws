#define _GNU_SOURCE
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
#include <signal.h>
#include <sys/wait.h>

/* Rather arbitrary. In real life, be careful with buffer overflow */
#define MAXBUF 8192 

typedef struct sockaddr SA;


////////////////////////////////////////////////////////// TASK STUFF
int THREAD_NUM;
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
////////////////////////////////////////////////////////// Request stuff
char listenPort[MAXBUF];
typedef struct request_environment{
    char CONTENT_LENGTH[MAXBUF];
    char CONTENT_TYPE[MAXBUF];
    char HTTP_ACCEPT[MAXBUF];
    char HTTP_REFERER[MAXBUF];
    char HTTP_ACCEPT_ENCODING[MAXBUF];
    char HTTP_ACCEPT_LANGUAGE[MAXBUF];
    char HTTP_ACCEPT_CHARSET[MAXBUF];
    char HTTP_HOST[MAXBUF];
    char HTTP_COOKIE[MAXBUF];
    char HTTP_USER_AGENT[MAXBUF];
    char HTTP_CONNECTION[MAXBUF];
} request_environment;

char path[MAXBUF];
char hostBuf[MAXBUF];
char svcBuf[MAXBUF];
char wwwRoot[MAXBUF];
///////////////////////////////////////////////////////// SETTINGS
int timeout = 2000; //default timeout = 1min
char cgiProgram[MAXBUF];



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


void find_environment(Request* request,struct request_environment req_env){
    int i;
    for(i = 0; i < request->header_count; i++){
        if(strcasecmp(request->headers[i].header_name,"CONTENT-LENGTH") == 0){
            strcpy(req_env.CONTENT_LENGTH,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"CONTENT-TYPE") == 0){
            strcpy(req_env.CONTENT_TYPE,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"ACCEPT") == 0){
            strcpy(req_env.HTTP_ACCEPT,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"REFERER") == 0){
            strcpy(req_env.HTTP_REFERER,request->headers[i].header_value); 
        }                
        else if(strcasecmp(request->headers[i].header_name,"ACCEPT-ENCODING") == 0){
            strcpy(req_env.HTTP_ACCEPT_ENCODING,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"ACCEPT-LANGUAGE") == 0){
            strcpy(req_env.HTTP_ACCEPT_LANGUAGE,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"ACCEPT-CHARSET") == 0){
            strcpy(req_env.HTTP_ACCEPT_CHARSET,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"HOST") == 0){
            strcpy(req_env.HTTP_HOST,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"COOKIE") == 0){
            strcpy(req_env.HTTP_COOKIE,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"USER-AGENT") == 0){
            strcpy(req_env.HTTP_USER_AGENT,request->headers[i].header_value); 
        }
        else if(strcasecmp(request->headers[i].header_name,"CONNECTION") == 0){
            strcpy(req_env.HTTP_CONNECTION,request->headers[i].header_value); 
        }        
    }
}

void fail_exit(char *msg) { fprintf(stderr, "%s\n", msg); exit(-1); }

void set_environment(Request *request){
    struct request_environment info;
    find_environment(request, info);
    //set inviront ment from request_environtment info
    setenv("CONTENT_LENGTH",info.CONTENT_LENGTH,1);
    setenv("CONTENT_TYPE",info.CONTENT_TYPE,1);
    setenv("REQUEST_METHOD",request->http_method,1);
    setenv("REQUEST_URI",request->http_uri,1);
    setenv("HTTP_ACCEPT",info.HTTP_ACCEPT,1);
    setenv("HTTP_REFERER",info.HTTP_REFERER,1);
    setenv("HTTP_ACCEPT_ENCODING",info.HTTP_ACCEPT_ENCODING,1);
    setenv("HTTP_ACCEPT_LANGUAGE",info.HTTP_ACCEPT_LANGUAGE,1);
    setenv("HTTP_ACCEPT_CHARSET",info.HTTP_ACCEPT_CHARSET,1);
    setenv("HTTP_HOST",info.HTTP_HOST,1);
    setenv("HTTP_COOKIE",info.HTTP_COOKIE,1);
    setenv("HTTP_USER_AGENT",info.HTTP_USER_AGENT,1);
    setenv("HTTP_CONNECTION",info.HTTP_CONNECTION,1);
    setenv("GATEWAY_INTERFACE","CGI/1.1",1);
    setenv("SERVER_PORT",listenPort,1);
    setenv("SERVER_PROTOCOL","HTTP/1.1",1);
    setenv("SERVER_SOFTWARE","ICWS",1);
    setenv("PATH_INFO",path,1);
    setenv("REMOTE_ADDR",hostBuf,1);
    setenv("SCRIPT_NAME",cgiProgram,1);
    setenv("QUERY_STRING",strchr(request->http_uri,'?')+1,1);
}

void serve_cgi(int connFd,Request *request){
    set_environment(request);
    //todo after set environment do something idk
    int c2pFds[2]; /* Child to parent pipe */
    int p2cFds[2]; /* Parent to child pipe */
    if (pipe(c2pFds) < 0) fail_exit("c2p pipe failed.");
    if (pipe(p2cFds) < 0) fail_exit("p2c pipe failed.");
    int pid = fork();
    if (pid < 0) fail_exit("Fork failed.");
    if (pid == 0) { /* Child - set up the conduit & run inferior cmd */
        /* Wire pipe's incoming to child's stdin */
        /* First, close the unused direction. */
        if (close(p2cFds[1]) < 0) fail_exit("failed to close p2c[1]");
        if (p2cFds[0] != STDIN_FILENO) {
            if (dup2(p2cFds[0], STDIN_FILENO) < 0)
                fail_exit("dup2 stdin failed.");
            if (close(p2cFds[0]) < 0)
                fail_exit("close p2c[0] failed.");
        }
        /* Wire child's stdout to pipe's outgoing */
        /* But first, close the unused direction */
        if (close(c2pFds[0]) < 0) fail_exit("failed to close c2p[0]");
        if (c2pFds[1] != STDOUT_FILENO) {
            if (dup2(c2pFds[1], STDOUT_FILENO) < 0)
                fail_exit("dup2 stdin failed.");
            if (close(c2pFds[1]) < 0)
                fail_exit("close pipeFd[0] failed.");
        }
        char* inferiorArgv[] = {cgiProgram, NULL};
        if (execvpe(inferiorArgv[0], inferiorArgv, environ) < 0)
            fail_exit("exec failed.");
    }
    else { /* Parent - send a random message */
        /* Close the write direction in parent's incoming */
        if (close(c2pFds[1]) < 0) fail_exit("failed to close c2p[1]");
        /* Close the read direction in parent's outgoing */
        if (close(p2cFds[0]) < 0) fail_exit("failed to close p2c[0]");
        char *message = "OMGWTFBBQ\n";
        /* Write a message to the child - replace with write_all as necessary */
        write(p2cFds[1], message, strlen(message));
        /* Close this end, done writing. */
        if (close(p2cFds[1]) < 0) fail_exit("close p2c[01] failed.");

        char buf[MAXBUF+1];
        ssize_t numRead;
        /* Begin reading from the child */
        while ((numRead = read(c2pFds[0], buf, MAXBUF))>0) {
            printf("Parent saw %ld bytes from child...\n", numRead);
            buf[numRead] = '\x0'; /* Printing hack; won't work with binary data */
            write_all(connFd,buf,MAXBUF);
        }
        /* Close this end, done reading. */
        if (close(c2pFds[0]) < 0) fail_exit("close c2p[01] failed.");
        /* Wait for child termination & reap */
        int status;
        if (waitpid(pid, &status, 0) < 0) fail_exit("waitpid failed.");
    }
    return;
}

void sigint_hanlder(int signum){
    pthread_cond_destroy(&condQueue);
    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&parseQueue);
    exit(1);
}


void serve_http(int connFd, char *rootFolder) {
    char buf[MAXBUF];
    memset(buf,0,MAXBUF);
    char sbuf[MAXBUF];
    memset(sbuf,0,MAXBUF);
    int readRet;
    struct pollfd fds[1];
    int pret;
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
                    //check connection will return "close" or "keep-alive" only.
                    char checkConnection[20];
                    strcpy(checkConnection,keepConnection(request));
                    if (!readRet) return ;  
                    /* Check if request == NULL */
                    if(request == NULL){
                        mysprinf(400,"",0,"null",checkConnection,buf);
                        write_all(connFd,buf, strlen(buf));
                        return ;
                    }
                    //everything work now but need change to HTTP/1.0
                    if (strcasecmp(request->http_version, "HTTP/1.1")){
                        mysprinf(505,"",0,"null",checkConnection,buf);
                        write_all(connFd,buf, strlen(buf));
                        return ;
                    }
                    strcpy(path, rootFolder);
                    strcat(path, request->http_uri);
                    if(strncasecmp(request->http_uri,"/cgi/",5) == 0){
                        serve_cgi(connFd,request);
                    }
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
    signal(SIGINT,sigint_hanlder);
    int c = 0;
    static struct option long_options[] = 
    {
        {"port",      required_argument,       0,  'p' },
        {"root",      required_argument,       0,  'r' },
        {"numthreads",required_argument,       0,  'n'},
        {"timeout",   required_argument,       0,  't'},
        {"cgiHandler", required_argument,      0,   'c'},
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
             case 'c' : strcpy(cgiProgram,optarg);
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
    pthread_mutex_destroy(&parseQueue);
    pthread_cond_destroy(&condQueue);
    return 0;
}
