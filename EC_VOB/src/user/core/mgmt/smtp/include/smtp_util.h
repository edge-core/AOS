#ifndef SMTP_UTIL_H
#define SMTP_UTIL_H

#include "sys_type.h"

void SMTP_UTIL_Print_SysName( UI8_T * s, UI8_T * buff );
void SMTP_UTIL_Print_IPAddr(UI8_T *s, UI32_T addr );
void SMTP_UTIL_Print_MailFrom( UI8_T * s, UI8_T * buff );
void SMTP_UTIL_Print_RcptTo( UI8_T * s, UI8_T * buff );
void SMTP_UTIL_Print_MailSubject( UI8_T * s, UI8_T * buff, UI8_T * buff1 );
void SMTP_UTIL_Print_Date( UI8_T * s, const UI8_T * month, int day, int hour, int minute, int second );
void SMTP_UTIL_Print_MsgLevel( UI8_T * s, UI8_T buff );
void SMTP_UTIL_Print_MsgModule( UI8_T * s, const UI8_T * buff );
void SMTP_UTIL_Print_MsgFunction( UI8_T * s, UI8_T buff );
void SMTP_UTIL_Print_MsgError( UI8_T * s, UI8_T buff );
void SMTP_UTIL_Print_MsgTime( UI8_T * s, const UI8_T * month, int day, int hour, int minute, int second );
void SMTP_UTIL_Print_MsgSysName( UI8_T * s, UI8_T * buff );
void SMTP_UTIL_Print_MsgIPAddr( UI8_T * s, UI32_T addr );
void SMTP_UTIL_Print_MsgContent( UI8_T * s, UI8_T * buff );

#endif
