#ifndef _NETSNMP_PORT_H
#define _NETSNMP_PORT_H
#define VXWORKS

#define SSL_LINUX


#include <sys/types.h>
#include <sys/socket.h>
#if 0
#include "socket.h"
#endif

#if 0
struct timeval {
               long tv_sec;        /* seconds */
               long tv_usec;  /* microseconds */
       };
#endif
struct timezone {
               int  tz_minuteswest; /* minutes W of Greenwich */
               int  tz_dsttime;     /* type of dst correction */
       };

#if 0
char* strdup(char* str);
#endif


struct vacm_accessEntry *vacm_get_accessList(void); /*in vacm.c*/

int gettimeofday(struct timeval *tv, struct timezone *tz);
void init_mib_modules();  /*in mib_modules.c*/
void netsnmp_udp_com2SecList_free(void); /* in snmpUDPDomain.c*/
                   
struct header_complex_index* SNMP_NOTIFY_FILTER_PROFILE_TABLE_GetNotifyFilterProfileList();
/* in snmpNotifyFilterProfileTable.c*/ 

struct header_complex_index** SNMP_NOTIFY_FILTER_PROFILE_TABLE_GetNotifyFilterProfileListAddr();
/* in snmpNotifyFilterProfileTable.c*/ 

struct header_complex_index* SNMP_NOTIFY_FILTER_TABLE_GetNotifyFilterList();/* in snmpNotifyFilterTable.c*/
struct header_complex_index** SNMP_NOTIFY_FILTER_TABLE_GetNotifyFilterListAddr(); /* in snmpNotifyFilterTable.c*/

void STATISTICS_DeleteAllRow();
void STATISTICS_CreateDefaultEntry();
void HISTORY_DeleteAllRow();
void HISTORY_CreateDefaultEntry();
void ALARM_DeleteAllRow();
void ALARM_CreateDefaultEntry();
void EVENT_DeleteAllRow();
void EVENT_CreateDefaultEntry();
void init_mib_modules();
#endif
