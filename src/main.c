#include <stdio.h>

#include "server.h"

int main(int argc, char const *argv[])
{
    int serverSock=getServerScok();
    int runningCode=runServerSock(serverSock);
    printf("server runnig code:%d\n",runningCode);
    return 0;
}
