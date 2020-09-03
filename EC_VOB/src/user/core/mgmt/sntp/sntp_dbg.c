/* static char SccsId[] = "+-<>?!SNTP_DBG.C   22.1  05/14/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_DBG.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  05-014-2002   Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "sntp_dbg.h"
 #include "sntp_txrx.h"
 #include "sntp_mgr.h"
 #include "sys_time.h"
 #include "sntp_type.h"
 #include "backdoor_mgr.h"
 #include "l_stdlib.h"
 #include "l_inet.h"
 #include "app_protocol_proc_comm.h"
 #include "l_threadgrp.h"

/* Global function */
void        SNTP_DBG_Init(void);

/* Global variable for printing debug message */
UI32_T      DBG_SNTP_TURN_MESSAGE_ON_OFF = 0;

/* STATIC PROGRAM for Debug use */
static void DBG_SNTP_BackdoorInfo_CallBack(void);
static void DBG_SNTP_Turn_Message_ON_OFF(void);
static void DBG_SNTP_Print_BackdoorHelp(void);
static void DBG_SNTP_Perform_Unicast(void);
static void DBG_SNTP_Perform_Brocast(void);
static void DBG_SNTP_GetServiceMode(void);
static void DBG_SNTP_SetServiceMode(void);
static void DBG_SNTP_GetPollingInteval(void);
static void DBG_SNTP_SetPollingInteval(void);
static void DBG_SNTP_GetSnptInfo(void);
static void DBG_SNTP_AddIpEntry(void);
static void DBG_SNTP_DeleteAllIpEntry(void);
static void DBG_SNTP_DeleteIpEntry(void);
static void DBG_SNTP_ShowAllServer(void);
static void DBG_SNTP_SetLocalPort(void);
static void DBG_SNTP_SetPacketVer(void);
static void DBG_SNTP_ShowLocalTime(void);
static void DBG_SNTP_SetTimeZone(void);
static void DBG_SNTP_GetTimeZone(void);

/*
 * static void DBG_SNTP_Printf(SNTP_MSG_E MSG);
 */
static void DBG_SNTP_EnableSNTP(void);
static void DBG_SNTP_DisableSNTP(void);
static void DBG_SNTP_ShowEnableOrDisable(void);
static void DBG_SNTP_GetNextIpEntry(void);

/*debug use */

/*extern void SNTP_OM_GetIpTable(UI32_T *server_entry,SNTP_SERVER_STATUS_E *server_status);
*/
/* EXPORTED SUBPROGRAM BODIES
 */


/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_DBG_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize SNTP_DBG used system resource,
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_DBG_Init(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_DBG_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SNTP_DBG_Create_InterCSC_Relation(void)
{
    /* Back door function : regerister to cli */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("SNTP",
                                                       SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
                                                       DBG_SNTP_BackdoorInfo_CallBack);
}

 /* Back door function
  */
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_BackdoorInfo_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Backdoor function for SNTP
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *-------------------------------------------------------------------------------*/
static void DBG_SNTP_BackdoorInfo_CallBack(void)
{
    L_THREADGRP_Handle_T    tg_handle;
    UI32_T                  backdoor_member_id;
    UI8_T                   ch;

    tg_handle = (L_THREADGRP_Handle_T) APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle();

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
                         &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("\nSNTP_MGR: SNTP Backdoor Selection");
    while (1)
    {
        DBG_SNTP_Print_BackdoorHelp();
        ch = BACKDOOR_MGR_GetChar();

        /*
         * Get execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

        switch (ch)
        {
            case '0':
                DBG_SNTP_Turn_Message_ON_OFF();
                break;

            case 'E':
                DBG_SNTP_EnableSNTP();
                break;

            case 'D':
                DBG_SNTP_DisableSNTP();
                break;

            case 'S':
                DBG_SNTP_ShowEnableOrDisable();
                break;

            case '1':
                DBG_SNTP_Perform_Unicast();
                break;

            case '2':
                DBG_SNTP_Perform_Brocast();
                break;

            case '3':
                DBG_SNTP_GetServiceMode();
                break;

            case '4':
                DBG_SNTP_SetServiceMode();
                break;

            case '5':
                DBG_SNTP_GetPollingInteval();
                break;

            case '6':
                DBG_SNTP_SetPollingInteval();
                break;

            case '7':
                DBG_SNTP_GetSnptInfo();
                break;

            case '8':
                DBG_SNTP_AddIpEntry();
                break;

            case '9':
                DBG_SNTP_DeleteIpEntry();
                break;

            case 'A':
                DBG_SNTP_DeleteAllIpEntry();
                break;

            case 'C':
                DBG_SNTP_ShowAllServer();
                break;

            case 'L':
                DBG_SNTP_SetLocalPort();
                break;

            case 'P':
                DBG_SNTP_SetPacketVer();
                break;

            case 'z':
                DBG_SNTP_GetTimeZone();
                break;

            case 't':
                DBG_SNTP_SetTimeZone();
                break;

            case 'U':
                DBG_SNTP_ShowLocalTime();
                break;

            case 'N':
                DBG_SNTP_GetNextIpEntry();
                break;

            case 'x':
            case 'X':
                /*
                 * Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                L_THREADGRP_Leave(tg_handle, backdoor_member_id);
                return;
        }

        /*
         * Release execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
    }
}

static void DBG_SNTP_Print_BackdoorHelp(void)
{
    BACKDOOR_MGR_Printf("\n\t0 : Turn ON/OFF debug message(0:off,1:0n)");
    BACKDOOR_MGR_Printf("\n\tE : Enable SNTP client");
    BACKDOOR_MGR_Printf("\n\tD : Disable SNTP client");
    BACKDOOR_MGR_Printf("\n\tS  :Show if SNTP is enable or disable ");
    BACKDOOR_MGR_Printf("\n\t1 : Perform Unicast mode");
    BACKDOOR_MGR_Printf("\n\t2 : Perform Broadcast mode");
    BACKDOOR_MGR_Printf("\n\t3 : Get Service Mode");
    BACKDOOR_MGR_Printf("\n\t4 : Set Service Mode(0:unicast,1:brocast,2:anycast,3:disable)");
    BACKDOOR_MGR_Printf("\n\t5 : Get Polling Inteval");
    BACKDOOR_MGR_Printf("\n\t6 : Set Polling Inteval");
    BACKDOOR_MGR_Printf("\n\t7 : Show SNTP Info");
    BACKDOOR_MGR_Printf("\n\t8 : Add an ip entry");
    BACKDOOR_MGR_Printf("\n\t9 : Delete an ip entry");
    BACKDOOR_MGR_Printf("\n\tA : Delete All Ip Entry");
    BACKDOOR_MGR_Printf("\n\tC : Get all the time server in database");
    BACKDOOR_MGR_Printf("\n\tL : Set Local port");
    BACKDOOR_MGR_Printf("\n\tP : Set Packet version");
    BACKDOOR_MGR_Printf("\n\tU : Get Local time");
    BACKDOOR_MGR_Printf("\n\tz : Get time zone");
    BACKDOOR_MGR_Printf("\n\tt : Set time zone");
    BACKDOOR_MGR_Printf("\n\tN : Get Next ip entry");

    BACKDOOR_MGR_Printf("\n-------------Press x or X to EXIT -------------------------");
    BACKDOOR_MGR_Printf("\nPlease Enter Your Choice :");
}

static void DBG_SNTP_Turn_Message_ON_OFF(void)
{
    UI8_T   index[2];
    UI32_T  mode;

    BACKDOOR_MGR_Printf("\nDo you want to print debug message?");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 2);
    mode = (UI32_T) atoi((char *)index);
    DBG_SNTP_TURN_MESSAGE_ON_OFF = mode;
	BACKDOOR_MGR_Printf("\n The Debug message is %lu", (unsigned long)DBG_SNTP_TURN_MESSAGE_ON_OFF);
}
/*
static void DBG_SNTP_Printf(SNTP_MSG_E MSG)
{
	switch (MSG)
	{
		case 0 :
			printf("SNTP_MSG_FAIL");
			break;
		case 1 :
			printf("SNTP_MSG_SUCCESS");
			break;
		case 2 :
			printf("SNTP_MSG_NoSupport");
			break;
		case 3 :
			printf("SNTP_MSG_NoBuffer");
			break;
		case 4 :
			printf("SNTP_MSG_NoData");
			break;
		case 5 :
			printf("SNTP_MSG_NoFound");
			break;
		case 6 :
			printf("SNTP_MSG_NoChange");
			break;
		case 7 :
			printf("SNTP_MSG_UnknowError");
			break;
		case 8 :
			printf("SNTP_MSG_ExceedRange");
			break;
		default :
			break;
	}
	return;
}
*/
static void DBG_SNTP_Perform_Unicast(void)
{
    SNTP_TXRX_STATUS_E  MSG;
    char                ServerIpaddress[16];
    UI8_T               delay[10];
    L_INET_AddrIp_T     ip;
    UI32_T              wait_reply;
    UI32_T              time;
    UI32_T              tick;

    BACKDOOR_MGR_Printf("\nPlease input time server ip address: ");
    BACKDOOR_MGR_RequestKeyIn((char *)ServerIpaddress, 16);
    BACKDOOR_MGR_Printf("\nPlease input wait reply time :");
    BACKDOOR_MGR_RequestKeyIn((char *)delay, 10);
    wait_reply = atoi((char *)delay);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_ADDR,
            ServerIpaddress,
            (L_INET_Addr_T *)&ip,
            sizeof(ip)))
    {
        BACKDOOR_MGR_Printf("\nSNTP DBG:invalid address");
        return;
    }

    if ((MSG = SNTP_TXRX_Client(&ip, wait_reply, &time, &tick)) != SNTP_TXRX_MSG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\nSNTP DBG:Receive error:%d", MSG);
        return;
    }

    BACKDOOR_MGR_Printf("\nSNTP DBG:Time is : %lu", (unsigned long)time);
    return;
}

static void DBG_SNTP_Perform_Brocast(void)
{
    SNTP_TXRX_STATUS_E  MSG;
    char                ServerIpaddress[16] = "";
    UI8_T               delay[10];
    L_INET_AddrIp_T     ip = {0};
    UI32_T              tick;
    UI32_T              wait_reply = 0;
    UI32_T              time = 0;

    BACKDOOR_MGR_Printf("\nPlease input wait reply time :");
    BACKDOOR_MGR_RequestKeyIn((char *)delay, 10);
    wait_reply = atoi((char *)delay);

    if ((MSG = SNTP_TXRX_Client(&ip, wait_reply, &time, &tick)) != SNTP_TXRX_MSG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\nSNTP DBG:Receive error:%d", MSG);
        return;
    }

    L_INET_InaddrToString((L_INET_Addr_T *)&ip, ServerIpaddress, sizeof(ServerIpaddress));
    BACKDOOR_MGR_Printf("\nSNTP DBG:Time is : %lu", (unsigned long)time);
    BACKDOOR_MGR_Printf("\nSNTP DBG:Broadcast server is %s", ServerIpaddress);
    return;
}

static void DBG_SNTP_GetServiceMode(void)
{
    UI32_T  mode;
    BOOL_T  MSG;

    MSG = SNTP_MGR_GetServiceOperationMode(&mode);
    BACKDOOR_MGR_Printf("\nSNTP DBG:Return Message is %d", MSG);
    BACKDOOR_MGR_Printf("\nCurrent service mode is :%lu", (unsigned long)mode);
}

static void DBG_SNTP_SetServiceMode(void)
{
    SNTP_MSG_E  MSG;
    UI8_T       index[2];
    UI32_T      mode;

    BACKDOOR_MGR_Printf("\nPlease enter the service mode:");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 2);
    mode = (UI32_T) atoi((char *)index);
    MSG = SNTP_MGR_SetServiceOperationMode(mode);
    BACKDOOR_MGR_Printf("\nSNTP DBG:Return Message is %d", MSG);
}

static void DBG_SNTP_GetPollingInteval(void)
{
    BOOL_T  MSG;
    UI32_T  time;

    MSG = SNTP_MGR_GetPollTime(&time);
    BACKDOOR_MGR_Printf("\nSNTP DBG: Current Poll Time is %lu", (unsigned long)time);
    BACKDOOR_MGR_Printf("\nSNTP DBG:Return Message is %d ", MSG);
}

static void DBG_SNTP_SetPollingInteval(void)
{
    SNTP_MSG_E  MSG;
    UI32_T      time;
    UI8_T       index[2];

    BACKDOOR_MGR_Printf("\nPlease enter polling inteval in seconds :");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 2);
    time = (UI32_T) atoi((char *)index);
    MSG = SNTP_MGR_SetPollTime(time);
    BACKDOOR_MGR_Printf("\nSNTP DBG: Current Poll Time is %lu", (unsigned long)time);
    BACKDOOR_MGR_Printf("\nSNTP DBG:Return Message is %d ", MSG);
}

static void DBG_SNTP_GetSnptInfo(void)
{
    /*
     * SNTP_MSG_E MSG;
     * SNTP_INFO_T Info;
     * MSG = SNTP_MGR_GetSNTPInfo(&Info);
     * printf("\nSNTP DBG:Current time is :%x",Info.Current_time);
     * printf("\nSNTP DBG:Last update time is :%x",Info.Last_SNTP_Update);
     * printf("\nSNTP DBG:Polling inteval is :%d",Info.Poll_inteval);
     * printf("\nSNTP DBG:From server :%x",Info.From_server);
     * printf("\nSNTP DBG:Current mode :is %d",Info.Current_mode);
     * printf("\nSNTP DBG:Return Message is ");
     * DBG_SNTP_Printf(MSG);
     */
}

static void DBG_SNTP_AddIpEntry(void)
{
    SNTP_MSG_E          MSG;
    SNTP_SERVER_ENTRY_T server_entry;
    char                ServerIpaddress[L_INET_MAX_IPADDR_STR_LEN+1];
    L_INET_AddrIp_T     ip;

    BACKDOOR_MGR_Printf("\nPlease input time server ip address: ");
    BACKDOOR_MGR_RequestKeyIn((char *)ServerIpaddress, L_INET_MAX_IPADDR_STR_LEN);

    memset(&ip, 0 , sizeof(ip));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       ServerIpaddress,
                                                       (L_INET_Addr_T *)&ip,
                                                       sizeof(ip)))
    {
        BACKDOOR_MGR_Printf("\nSNTP DBG:invalid address");
        return;
    }

    memcpy(&server_entry.ipaddress, &ip, sizeof(ip));

    /*
     * printf("\nPlease enter priority :");
     * BACKDOOR_MGR_RequestKeyIn(index, 2);
     * server_entry.priority = (UI32_T)atoi(index);
     * printf("\nPlease enter version :");
     * BACKDOOR_MGR_RequestKeyIn(index, 2);
     * server_entry.version = (UI32_T)atoi(index);
     * server_entry.rowStatus = 1;
     */
    MSG = SNTP_MGR_AddServerIp(0, &server_entry.ipaddress);

    BACKDOOR_MGR_Printf("\nSNTP DBG:Return Message is %d ", MSG);
}

static void DBG_SNTP_DeleteIpEntry(void)
{
    SNTP_MSG_E  MSG;

    /*
     * printf("\nPlease input ip address you want to delete: ");
     * BACKDOOR_MGR_RequestKeyIn(ServerIpaddress, 16);
     * if (L_INET_Aton(ServerIpaddress, &ip)!= TRUE) { printf("\nSNTP
     * DBG:invalid address");
     * return;
     * } ip = ntohl(ip);
     */
    MSG = SNTP_MGR_DeleteServerIp(0);

    BACKDOOR_MGR_Printf("\nSNTP DBG: Delete message is %d", MSG);
}

static void DBG_SNTP_DeleteAllIpEntry(void)
{
    /*
     * SNTP_MSG_E MSG;
     * MSG = SNTP_MGR_DeleteAllServers();
     * printf("\nSNTP DBG:Return Message is ");
     * DBG_SNTP_Printf(MSG);
     */
}

static void DBG_SNTP_ShowAllServer(void)
{
	/*SNTP_MSG_E  MSG  ;
	UI32_T ip,index;
	SNTP_SERVER_ENTRY_T record;
	UI32_T server_entry[SNTP_MAX_SERVER];
	SNTP_SERVER_STATUS_E server_status[SNTP_MAX_SERVER];*/
	/* Find the first entry */
	/*
	if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
	{
		printf("\nSNTP DBG : Begin to print Server entry");
	}
	printf("\n SNTP DBG : IP address table");

	MSG = SNTP_MGR_GetNextServerEntry((UI32_T)0, &record);
	if (MSG == SNTP_MSG_NoFound)
	{
		printf("\n SNTP DBG : No invalid ip entry");
		return;
	}
	printf("\nEntry 0 : %x",record.ipaddress);

	for (index=1;MSG != SNTP_MSG_NoFound;index++)
	{
		MSG = SNTP_MGR_GetNextServerEntry(record.ipaddress, &record);
		if (MSG == SNTP_MSG_NoFound)
		{
			break;
		}
		printf("\nEntry %d : %x",index,record.ipaddress);
	}
	printf("\n SNTP_DBG:Current Ip table :");

	SNTP_OM_GetIpTable(server_entry,server_status);

	for (index =0;index<SNTP_MAX_SERVER;index++)
	{
		printf("\n %d. IP = %x, Status = %d",index, server_entry[index],server_status[index]);
	}
	*/
	return;
}

static void DBG_SNTP_SetLocalPort(void)
{
    unsigned int    LocalPort;
    UI8_T           index[4];

    BACKDOOR_MGR_Printf("\nPlease enter local port:");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 4);
    LocalPort = (UI32_T) atoi((char *)index);
    SNTP_TXRX_SetLocalPort(LocalPort);
}

static void DBG_SNTP_SetPacketVer(void)
{
    UI8_T   version;
    UI8_T   index[2];

    BACKDOOR_MGR_Printf("\nPlease enter packet version:");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 2);
    version = (UI32_T) atoi((char *)index);
    SNTP_TXRX_SetVersion(version);
}

static void DBG_SNTP_ShowLocalTime(void)
{
    char    UTC[22];
    UI32_T  time;
    int     year;
    int     month;
    int     day;
    int     hour;
    int     minute;
    int     second;

    SYS_TIME_GetRealTimeBySec(&time);
    SYS_TIME_ConvertTime(time, UTC);

    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);
	BACKDOOR_MGR_Printf("\nSNTP_DBG: The Local time from GetRealTimeBySec is %s",UTC);
	BACKDOOR_MGR_Printf("\nSNTP_DBG: The Local time from GetRealTime is %d year %d month %d day %d hour %d minute %d second",year,month,day,hour,minute,second);
}

static void DBG_SNTP_GetTimeZone(void)
{
    SYS_TIME_Timezone_T timezone;

    SYS_TIME_GetTimeZone(&timezone);
    BACKDOOR_MGR_Printf("\nSNPT_DBG: The zone name is %s,the time zone is %s%lu",
        timezone.timezone.custom.name,
        (timezone.timezone.custom.sign == SYS_TIME_TIMEZONE_MINUS) ? "-" : "",
        (unsigned long)timezone.timezone.custom.hour*60 + timezone.timezone.custom.minute);
}

static void DBG_SNTP_SetTimeZone(void)
{
    SYS_TIME_TIMEZONE_SIGN_T  sign;
    UI32_T  hour = 0, minute = 0;
    int     offset = 0;
    UI8_T   index[4];
    char    zone_name[256];

    BACKDOOR_MGR_Printf("\nPlease enter time zone name:");
    BACKDOOR_MGR_RequestKeyIn((char *)zone_name, 256);
    BACKDOOR_MGR_Printf("\nPlease enter time zone offset:");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 4);

    offset = (UI32_T) atoi((char *)index);

    if (offset < 0)
    {
        sign = SYS_TIME_TIMEZONE_MINUS;
        hour = -(offset/60);
        minute = -(offset%60);
    }
    else
    {
        sign = SYS_TIME_TIMEZONE_PLUS;
        hour = offset/60;
        minute = offset%60;
    }

    SYS_TIME_SetTimeZone(zone_name, sign, hour, minute);
}

static void DBG_SNTP_EnableSNTP(void)
{
    SNTP_MSG_E  MSG;

    MSG = SNTP_MGR_SetStatus(0);
    BACKDOOR_MGR_Printf("\nSNTP DBG: Delete message is %d", MSG);
}

static void DBG_SNTP_DisableSNTP(void)
{
    SNTP_MSG_E  MSG;

    MSG = SNTP_MGR_SetStatus(1);
    BACKDOOR_MGR_Printf("\nSNTP DBG: Delete message is %d", MSG);
}

static void DBG_SNTP_ShowEnableOrDisable(void)
{
    UI32_T  service;

    SNTP_MGR_GetStatus(&service);
    BACKDOOR_MGR_Printf("\nSNTP DBG : The service is %lu", (unsigned long)service);
}

static void DBG_SNTP_GetNextIpEntry(void)
{
    UI32_T  key;
    UI8_T   index[2];
    L_INET_AddrIp_T  ipaddress;
    char    ServerIpaddress[16];

    BACKDOOR_MGR_Printf("\nPlease enter index :");
    BACKDOOR_MGR_RequestKeyIn((char *)index, 2);
    key = (UI32_T) atoi((char *)index);

    SNTP_MGR_GetNextServerIp(&key, &ipaddress);
    L_INET_InaddrToString((L_INET_Addr_T *)&ipaddress, ServerIpaddress, sizeof(ServerIpaddress));
	BACKDOOR_MGR_Printf("\nSNTP DBG: Next IP entry is %s & next key is %lu",ServerIpaddress,(unsigned long)key);
}
