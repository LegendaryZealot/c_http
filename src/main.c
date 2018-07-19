#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

int main(int argc, char const *argv[])
{
    if (!fork()) {
        int startCode=StartServer();
        printf("server start code:%d\n",startCode);
        if(0!=startCode)
        {
            exit(0);
        }
    }
    return 0;
}
