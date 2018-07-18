#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "sock.h"
#include "define.h"
#include "server.h"

extern AcceptCallback acceptCallBack;

void OnAcceptCallback(int sock);
void DropRedundancyHead(int sock);
void HandleRequestHead(int sock);
void ErrorPage(int sock);
void NotFoundPage(int sock);
void IStrCat(char *,char *);

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
    char html[]="HTTP/1.x 200 OK\nContent-Type:text\n\nhi";
    int len = send(sock,html,strlen(html),0);
    printf("send:%d\n",len);
    close(sock);
    printf("sock closed!\n");
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
    printf("%s %s %s\n",method,path,httpVersion);
    // printf("%s\n",requestHead);
    // printf("Accepted!\n%s\n",requestHead);
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