#include <stdio.h>

#include "server.h"

int main(int argc, char const *argv[])
{
    int startCode=StartServer();
    printf("server start code:%d\n",startCode);
    return 0;
}
