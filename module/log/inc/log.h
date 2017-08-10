#ifndef __LOG_H__
#define __LOG_H__

#include "zlog.h"

#define FATAL 100
#define ERROR 101
#define WARN 102
#define NOTICE 103
#define INFO 104
#define DEBUG 105

void LOGINIT(char*);
void LOGFREE(void);
//void SYSLOG(int,char*,char*,...);
//void APPLOG(char*,int,char*,char*,...);

#define APPLOG(category,level,format,...)	\
    if(level == DEBUG)	\
    {   \
        zlog_debug(zlog_get_category(category), format, __VA_ARGS__);	\
    }   	\
    else if(level == INFO)	\
    {   \
        zlog_info(zlog_get_category(category), format, __VA_ARGS__);	\
    }   \
    else if(level == NOTICE)	\
    {   \
        zlog_notice(zlog_get_category(category), format, __VA_ARGS__);	\
    }   \
    else if(level == WARN)	\
    {   \
        zlog_warn(zlog_get_category(category), format, __VA_ARGS__);	\
    }   \
    else if(level == ERROR)	\
    {	\
        zlog_error(zlog_get_category(category), format, __VA_ARGS__);	\
    }	\
    else if(level == FATAL)	\
    {	\
        zlog_fatal(zlog_get_category(category), format, __VA_ARGS__);	\
    }	\
    else	\
    {	\
        dzlog_error("Unknow category");	\
    }	

#define SYSLOG(level,format,...)	\
	APPLOG("sys",level,format,__VA_ARGS__)

#endif


