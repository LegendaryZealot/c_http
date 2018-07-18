#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sock.h"
#include "define.h"

int sockfd=0;
AcceptCallback acceptCallBack;

int getServerScok()
{
    if(0!=sockfd)
    {
        return sockfd;
    }
    if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0)))
    {
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(-1==bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)))
    {
        return -2;
    }
    if(-1==listen(sockfd,MAX_CONNECT))
    {
        return -3;
    }
    return sockfd;
}

int runServerSock()
{
    if(sockfd<=0)
    {
        return -1;
    }
    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        int client = accept(sockfd,(struct sockaddr*)&client_addr,&length);
        if(-1!=client)
        {
            printf("get a connect client:%d!\n",client);
            if(acceptCallBack)
            {
                acceptCallBack(client);
            }
        }
        close(client);
    }
}