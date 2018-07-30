#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    printf("HTTP/1.x 200 OK\nContent-Type:text\n\n");
    printf("pid:%ld\ntid:%ld\n\n",(long)getpid(),(long)pthread_self());
    return 0;
}
