#ifndef _SERVER_H_
#define _SERVER_H_

typedef void (*AcceptCallback)(int acceptSock);

AcceptCallback acceptCallBack;

int getServerScok();
int runServerSock();


#endif