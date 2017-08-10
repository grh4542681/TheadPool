#include "Socket.h"
#include "log.h"
#include "returnval.h"
#include "mem.h"
#include <string.h>

int InitTCPSocket(char* caIpAddr,char* caPort) 
{
	int iSockfd;
	struct sockaddr_in addr;

	memset(&addr,0,sizeof(addr));

	addr.sin_family=AF_INET;
	addr.sin_port=htons(atoi(caPort));
	addr.sin_addr.s_addr=inet_addr(caIpAddr);


	iSockfd=socket(AF_INET,SOCK_STREAM,0);
	if(iSockfd < 0)
	{   
		SYSLOG(ERROR,"InitTcpSocket error IP[%s] Port[%s]",caIpAddr,caPort);
		return SYSERROR; 
	}   
	if(bind(iSockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))==-1)
	{   
		SYSLOG(ERROR,"InitTcpSocket bind error IP[%s] Port[%s]",caIpAddr,caPort);
		return SYSERROR; 
	}   
	if(listen(iSockfd,CONMAXNUM)==-1)
	{   
		SYSLOG(ERROR,"InitTcpSocket listen error IP[%s] Port[%s]",caIpAddr,caPort);
		return SYSERROR; 
	}   
	return iSockfd;
}

int ConnTCPSocket(char* caIpAddr,char* caPort)
{
	int iSockfd;
	struct sockaddr_in addr;

	memset(&addr,0,sizeof(addr));

	addr.sin_family=AF_INET;
	addr.sin_port=htons(atoi(caPort));
	addr.sin_addr.s_addr=inet_addr(caIpAddr);


	iSockfd=socket(AF_INET,SOCK_STREAM,0);
	if(iSockfd < 0)
	{   
		SYSLOG(ERROR,"ConnTcpSocket error IP[%s] Port[%s]",caIpAddr,caPort);
		return SYSERROR; 
	}   
	
	if(connect(iSockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))==-1)
	{   
		SYSLOG(ERROR,"ConnTcpSocket connetc error IP[%s] Port[%s]",caIpAddr,caPort);
		return SYSERROR; 
	}   
	return iSockfd;
}

int CloseTCPSocket(int sockfd)
{
	return close(sockfd);
}

int RecvSocketWait(int sockfd,char* buf,int len)
{
	int ret=0;
	int nrecv=0;
	struct msghdr msg;
	struct iovec iov[1];

	memset(&msg,0x00,sizeof(struct msghdr));

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	do  
	{   
		iov[0].iov_base = buf+nrecv;
		iov[0].iov_len = len-nrecv;
		if((ret=recvmsg(sockfd,&msg,0))<0)
		{   
			if(errno==EINTR)
			{   
				continue;
			}   
			else
			{   
				SYSLOG(ERROR,"RecvSocketWait error - errno[%d]:%s",errno,strerror(errno));
				return SYSERROR; 
			}   
		}
		nrecv+=ret;
	}while(nrecv<len&&ret);
	return nrecv;
}

int RecvSocketNoWait(int sockfd,char* buf,int len)
{
	int ret=0;
	int nrecv=0;
	struct msghdr msg;
	struct iovec iov[1];

	memset(&msg,0x00,sizeof(struct msghdr));

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	do  
	{   
		iov[0].iov_base = buf+nrecv;
		iov[0].iov_len = len-nrecv;
		if((ret=recvmsg(sockfd,&msg,MSG_DONTWAIT))<0)
		{   
			if(errno==EINTR)
			{   
				continue;
			}   
			else
			{   
				SYSLOG(ERROR,"RecvSocketNoWait error - errno[%d]:%s",errno,strerror(errno));
				return SYSERROR; 
			}   
		}
		nrecv+=ret;
	}while(nrecv<len&&ret);
	return nrecv;
}

int SendSocketWait(int sockfd,char* buf,int len)
{
	int ret=0;
	int nsend=0;
	struct msghdr msg;
	struct iovec iov[1];

	memset(&msg,0x00,sizeof(struct msghdr));

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	do
	{
		iov[0].iov_base = buf+nsend;
		iov[0].iov_len = len - nsend;
		if((ret = sendmsg(sockfd,&msg,0))<0)
		{
			if(errno==EINTR)
			{   
				errno=0;
				continue;
			}   
			else
			{
				SYSLOG(ERROR,"SendSocketWait error - errno[%d]:%s",errno,strerror(errno));
				return SYSERROR;
			}
		}
		nsend+=ret;
	}while(nsend<len&&ret);
	return nsend;
}

int SendSocketNoWait(int sockfd,char* buf,int len)
{
	int ret=0;
	int nsend=0;
	struct msghdr msg;
	struct iovec iov[1];

	memset(&msg,0x00,sizeof(struct msghdr));

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	do
	{
		iov[0].iov_base = buf+nsend;
		iov[0].iov_len = len - nsend;
		if((ret = sendmsg(sockfd,&msg,MSG_DONTWAIT))<0)
		{
			if(errno==EINTR)
			{   
				continue;
			}   
			else
			{
				SYSLOG(ERROR,"SendSocketNoWait error - errno[%d]:%s",errno,strerror(errno));
				return SYSERROR;
			}
		}
		nsend+=ret;
	}while(nsend<len||ret);
	return nsend;
}

int PkgRecv(int sockfd,char* buf,int len,int flag)
{
	int ret=0;
	char msglen[MSGLENL+1];
	void* cache;

	cache = NULL;
	memset(msglen,0x00,sizeof(msglen));

	if(flag == BLOCK)
	{
		ret = RecvSocketWait(sockfd,msglen,MSGLENL);
		if(ret == SYSERROR || ret == 0)
		{
			return ret;
		}
		cache = (void*)Malloc(atoi(msglen)+1);
		memset(cache,0x00,atoi(msglen)+1);
		SYSLOG(INFO,"Recv Package len[%s]",msglen);

		ret = RecvSocketWait(sockfd,cache,atoi(msglen));
		if(ret == SYSERROR)
		{
			Free(cache);
			return ret;
		}
		SYSLOG(INFO,"Recv Package[%s]",(char*)cache);

		if(len < atoi(msglen))
		{
			SYSLOG(ERROR,"Recv cache too small, need[%s]",msglen);
			Free(cache);
			return SYSERROR;
		}
		memcpy(buf,cache,atoi(msglen));
		Free(cache);
	}
	else if(flag == NONBLOCK)
	{
		ret = RecvSocketNoWait(sockfd,msglen,MSGLENL);
		if(ret == SYSERROR || ret==0 )
		{
			return ret;
		}
		cache = (void*)Malloc(atoi(msglen)+1);
		memset(cache,0x00,atoi(cache)+1);
		SYSLOG(INFO,"Recv Package len[%s]",msglen);

		ret = RecvSocketNoWait(sockfd,cache,atoi(msglen));
		if(ret == SYSERROR)
		{
			Free(cache);
			return ret;
		}
		SYSLOG(INFO,"Recv Package[%s]",(char*)cache);

		if(len < atoi(msglen))
		{
			SYSLOG(ERROR,"Recv cache too small, need[%s]",msglen);
			Free(cache);
			return SYSERROR;
		}
		memcpy(buf,cache,atoi(msglen));
		Free(cache);
	}
	else
	{
		
	}
	return ret;
}
int PkgSend(int sockfd,char* buf,int len,int flag)
{
	int ret=0;
	void* cache;
	cache=NULL;
	char msglen[MSGLENL+1];
	memset(msglen,0x00,sizeof(msglen));	

	cache=Malloc(len+MSGLENL+1);
	memset(cache,0x00,len+MSGLENL+1);
	sprintf(msglen,"%08d",len);

	memcpy((char*)cache,msglen,MSGLENL);
	memcpy((char*)cache+MSGLENL,buf,len);

	SYSLOG(INFO,"Send Packge[%s]",(char*)cache);
	
	if(flag == BLOCK)
	{
		ret = SendSocketWait(sockfd,cache,len+8);
		if(ret == SYSERROR)
		{
			Free(cache);
			return ret;
		}
	}
	else if(flag == NONBLOCK)
	{
		ret = SendSocketNoWait(sockfd,cache,len+8);
		if(ret == SYSERROR)
		{
			Free(cache);
			return ret;
		}
	}
	else
	{

	}
	Free(cache);
	return ret;
}






