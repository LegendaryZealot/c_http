#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "sock.h"
#include "define.h"
#include "server.h"

extern AcceptCallback acceptCallBack;

void OnAcceptCallback(int sock);
void DropRedundancyHead(int sock);
void *AcceptCallbackThread(void *sock);
void HandleRequestHead(int sock);
void HandleResponse(int sock);
void ErrorPage(int sock);
void NotFoundPage(int sock);
void IStrCat(char *,char *);
void SendRawFile(int sock,char *path);
void ExecCgi(int sock,char *path);

int StartServer()
{
    acceptCallBack=OnAcceptCallback;
    int initCode=getServerScok();
    printf("serverSock init code:%d\n",initCode);
    if(initCode<=0)
    {
        printf("serverSock init error!\n");
        return -1;
    }
    printf("%d\n",runServerSock());
    return 0;
}

void OnAcceptCallback(int sock)
{
    printf("callback:\npid:%ld\ntid:%ld\n\n",(long)getpid(),(long)pthread_self());
    pthread_t *my_thread;
    int err;
    if(0!=(err=pthread_create((pthread_t *)&my_thread,NULL,AcceptCallbackThread,(void *)&sock)))
    {
        printf("accept callback thread create error!\n");
    }
}

void *AcceptCallbackThread(void *sock)
{
    printf("callback thread:\npid:%ld\ntid:%ld\n\n",(long)getpid(),(long)pthread_self());
    int sockfd=*(int *)sock;
    HandleRequestHead(sockfd);
    HandleResponse(sockfd);
    close(sockfd);
    return NULL;
}

void HandleRequestHead(int sock)
{
    char requestHead[MAX_POST];
    char buf[128];
    int len=0;
    int bufSize=sizeof(buf);
    memset(requestHead,'\0',MAX_POST);    
    do
    {
        memset(buf,'0',sizeof(buf));
        len = recv(sock,buf,bufSize,0);
        if ((len+strlen(requestHead))>=MAX_POST) {
            DropRedundancyHead(sock);
            break;
        }
        strcat(requestHead,buf);
    } while (len==bufSize);
    char method[128];
    char path[128];
    char httpVersion[128];
    int count=sscanf(requestHead,"%s %s %s",method,path,httpVersion);
    if(3!=count)
    {
        ErrorPage(sock);
        return;
    }
    setenv("method",method,1);
    setenv("path",path,1);
    setenv("httpVersion",httpVersion,1);
    printf("%s %s %s\n",method,path,httpVersion);
}

void HandleResponse(int sock)
{
    char *path;
    path=getenv("path");
    if(NULL==path)
    {
        ErrorPage(sock);
        return;
    }
    char fullPath[256];
    memset(fullPath,'0',sizeof(fullPath));
    strcpy(fullPath,ROOT);
    strcat(fullPath,path);
    if(0!=access(fullPath,R_OK))
    {
        printf("access error!\n");
        NotFoundPage(sock);
    }
    if(0!=access(fullPath,X_OK))
    {
        printf("Raw start!\n");
        SendRawFile(sock,fullPath);
        printf("Raw end!\n");
    }
    else
    {
        printf("CGI start!\n");
        ExecCgi(sock,fullPath);
        printf("CGI end!\n");
    }
}

void DropRedundancyHead(int sock)
{
    char buf[128];
    int bufSize=sizeof(buf);
    int len = recv(sock,buf,bufSize,0);
    while(len==bufSize)
    {
        len=recv(sock,buf,bufSize,0);
    }
}

void ErrorPage(int sock)
{
    char buf[128];
    sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:text\n\n");
    send(sock,buf,strlen(buf),0);
    sprintf(buf,"%s","Error page!\n");
    send(sock,buf,strlen(buf),0);
}

void NotFoundPage(int sock)
{
    char buf[128];
    sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:text\n\n");
    send(sock,buf,strlen(buf),0);
    sprintf(buf,"%s","Page not found!\n");
    send(sock,buf,strlen(buf),0);
}

void SendRawFile(int sock,char *path)
{
    char buf[128];
    if(strstr(path,".html"))
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:text/html\n\n");
    }
    else if(strstr(path,".css"))
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:text/css\n\n");
    }
    else if(strstr(path,".js"))
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:application/x-javascript\n\n");
    }
    else
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:image/tiff\n\n");
    }
    send(sock,buf,strlen(buf),0);
    int fp=open(path,O_RDWR);
    if(-1!=fp)
    {
        while(memset(buf,'\0',sizeof(buf)),read(fp,buf,sizeof(buf)))
        {
            send(sock,buf,sizeof(buf),0);
        }
    }
    close(fp);
}

void ExecCgi(int sock,char *path)
{
    char buf[128];
    FILE *fp=popen(path,"r");
    if(fp)
    {
        while(memset(buf, '0', sizeof(buf)), fgets(buf, sizeof(buf) - 1, fp) != 0 ) 
        {
            send(sock,buf,strlen(buf),0);
        }
    }
    pclose(fp); 
}