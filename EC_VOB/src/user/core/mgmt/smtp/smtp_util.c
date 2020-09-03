#include <string.h>
#include <stdio.h>
#include "smtp_util.h"

void SMTP_UTIL_Print_SysName( UI8_T * s, UI8_T * buff )
{
	sprintf((char *)s,"HELO %s\r\n", buff);
    return;
}

void SMTP_UTIL_Print_IPAddr( UI8_T * s, UI32_T addr )
{
	sprintf((char *)s,"HELO %d.%d.%d.%d\r\n", (UI8_T)((addr>>24)&0xff), (UI8_T)((addr>>16)&0xff), (UI8_T)((addr>>8)&0xff), (UI8_T)(addr&0xff));
    return;
}

void SMTP_UTIL_Print_MailFrom( UI8_T * s, UI8_T * buff )
{
	sprintf((char *)s,"MAIL FROM:<%s>\r\n", buff);
    return;
}

void SMTP_UTIL_Print_RcptTo( UI8_T * s, UI8_T * buff )
{
	sprintf((char *)s,"RCPT TO:<%s>\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MailSubject( UI8_T * s, UI8_T * buff, UI8_T * buff1 )
{
	sprintf((char *)s,"From: <%s>\r\nTo: %s\r\nSubject: Alert Message!!\r\n\r\n", buff, buff1);
    return;
}

void SMTP_UTIL_Print_Date( UI8_T * s, const UI8_T * month, int day, int hour, int minute, int second )
{
	if (day < 10)
	{
	    sprintf((char *)s,"Date: %s 0%1d %d:%d:%d\r\n", month, day, hour, minute, second);
	}
	else
	{
	    sprintf((char *)s,"Date: %s %2d %d:%d:%d\r\n", month, day, hour, minute, second);
	}
    return;
}

void SMTP_UTIL_Print_MsgLevel( UI8_T * s, UI8_T buff )
{
	sprintf((char *)s,"Level: %d\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MsgModule( UI8_T * s, const UI8_T * buff )
{
	sprintf((char *)s,"Module: %s\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MsgFunction( UI8_T * s, UI8_T buff )
{
	sprintf((char *)s,"Function: %d\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MsgError( UI8_T * s, UI8_T buff )
{
	sprintf((char *)s,"Error: %d\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MsgTime( UI8_T * s, const UI8_T * month, int day, int hour, int minute, int second )
{
	if (day < 10)
	{
	    sprintf((char *)s,"Time: %s 0%1d %d:%d:%d\r\n", month, day, hour, minute, second);
	}
	else
	{
	    sprintf((char *)s,"Time: %s %2d %d:%d:%d\r\n", month, day, hour, minute, second);
	}
    return;
}

void SMTP_UTIL_Print_MsgSysName( UI8_T * s, UI8_T * buff )
{
	sprintf((char *)s,"Host: %s\r\n", buff);
    return;
}

void SMTP_UTIL_Print_MsgIPAddr( UI8_T * s, UI32_T addr )
{
	sprintf((char *)s,"Host: %d.%d.%d.%d\r\n", (UI8_T)((addr>>24)&0xff), (UI8_T)((addr>>16)&0xff), (UI8_T)((addr>>8)&0xff), (UI8_T)(addr&0xff));
    return;
}

void SMTP_UTIL_Print_MsgContent( UI8_T * s, UI8_T * buff )
{
	sprintf((char *)s,"Content: %s\r\n", buff);
    return;
}
