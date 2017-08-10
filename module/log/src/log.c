#include <stdarg.h>
#include "log.h"
#include "zlog.h"

void LOGINIT(char* config)
{
	zlog_init(config);
}

void LOGFREE()
{
	zlog_fini();
}

//void SYSLOG(int level, char* format, char* message,...)
//{	
//	va_list args;
//	va_start(args,message);
//	printf(format,message,args);
//	APPLOG("sys", level, format,message, args);
//	va_end(args);
//}
//
//void APPLOG(char* category, int level, char* format, char* message,...)
//{
//	va_list args;
//	va_start(args,message);
//	zlog_category_t* sys=zlog_get_category(category);
//	if(level == DEBUG)
//	{
//		zlog_debug(sys, format, args);
//	}
//	else if(level == INFO)
//	{
//		zlog_info(sys, format, args);
//	}
//	else if(level == NOTICE)
//	{
//		zlog_notice(sys, format, args);
//	}
//	else if(level == WARN)
//	{
//		zlog_warn(sys, format, args);
//	}
//	else if(level == ERROR)
//	{
//		zlog_error(sys, format, args);
//	}
//	else if(level == FATAL)
//	{
//		zlog_fatal(sys, format, args);
//	}
//	else
//	{
//		dzlog_error("Unknow category");
//	}
//	va_end(args);
//}
//

