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

#include "sock.h"
#include "define.h"
#include "server.h"

extern AcceptCallback acceptCallBack;

void OnAcceptCallback(int sock);
void DropRedundancyHead(int sock);
void HandleRequestHead(int sock);
void HandleResponse(int sock);
void ErrorPage(int sock);
void NotFoundPage(int sock);
void IStrCat(char *,char *);
void sendRawFile(int sock,char *path);
void execCgi(int sock,char *path);

int StartServer()
{
    acceptCallBack=OnAcceptCallback;
    printf("%d\n",getServerScok());
    printf("%d\n",runServerSock());
    return 0;
}

void OnAcceptCallback(int sock)
{
    printf("Acceptcallback actived!\n");
    HandleRequestHead(sock);
    HandleResponse(sock);
    close(sock);
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
    sscanf(requestHead,"%s %s %s",method,path,httpVersion);
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
        ErrorPage(sock);
    }
    if(0!=access(fullPath,X_OK))
    {
        sendRawFile(sock,fullPath);
    }
    else
    {
        execCgi(sock,fullPath);
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

void sendRawFile(int sock,char *path)
{
    char buf[128];
    if(NULL!=strstr(path,".html"))
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:image\n\n");
    }
    else
    {
        sprintf(buf,"%s","HTTP/1.x 200 OK\nContent-Type:text\n\n");
    }
    send(sock,buf,strlen(buf),0);
    int fp=open(path,O_RDWR);
    if(-1==fp)
    {
        ErrorPage(sock);
    }
    while(memset(buf,'\0',sizeof(buf)),read(fp,buf,sizeof(buf)))
    {
        send(sock,buf,strlen(buf),0);
    }
    close(fp);
}

void execCgi(int sock,char *path)
{
    char buf[128];
    int fp=popen(path,"r");
    if(NULL!=fp)
    {
        while(memset(buf,'\0',sizeof(buf)),read(fp,buf,sizeof(buf)))
        {
            send(sock,buf,sizeof(buf),0);
        }
    }
    pclose(fp); 
}