#ifndef _SERVER_H_
#define _SERVER_H_

typedef void (*AcceptCallback)(int acceptSock);

int getServerScok();
int runServerSock();

#endif