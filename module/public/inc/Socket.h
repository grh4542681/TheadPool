#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>

#define CONMAXNUM 100
#define MSGLENL 8

#define TCP 1
#define UDP 2

#define BLOCK 1
#define NONBLOCK 2

int InitTCPSocket(char*,char*);
int ConnTCPSocket(char*,char*);
int CloseTCPSocket(int);

int RecvSocketWait(int,char*,int);
int RecvSocketNoWait(int,char*,int);
int SendSocketWait(int,char*,int);
int SendSockeatNoWait(int,char*,int);

int PkgRecv(int ,char* ,int ,int );
int PkgSend(int ,char* ,int ,int );

#endif
