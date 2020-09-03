/* static char SccsId[] = "+-<>?!SNTP_MGR.H   22.1  22/04/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_MGR.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  04-22-2002  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */

#ifndef _SNTP_MGR_H
#define _SNTP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "sntp_type.h"
#include "sntp_txrx.h"

/* NAME CONSTANT DECLARATIONS
 */
/* The key to get keygen mgr msgq.
 */
#define SNTP_MGR_IPCMSG_KEY SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY

/* The commands for IPC message. */
enum
{
    SNTP_MGR_IPC_CMD_ADD_SVR_IP,                /* 0 */
    SNTP_MGR_IPC_CMD_ADD_SVR_IP_FOR_CLI,        /* 1 */
    SNTP_MGR_IPC_CMD_DEL_ALL_SVR_IP,            /* 2 */
    SNTP_MGR_IPC_CMD_DEL_SVR_IP,                /* 3 */
    SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_CLI,        /* 4 */
    SNTP_MGR_IPC_CMD_GET_CUR_SVR_IP,            /* 5 */
    SNTP_MGR_IPC_CMD_GET_POLL_TIME,             /* 6 */
    SNTP_MGR_IPC_CMD_GET_RUNN_POLL_TIME,        /* 7 */
    SNTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE,        /* 8 */
    SNTP_MGR_IPC_CMD_GET_RUNN_STATUS,           /* 9 */
    SNTP_MGR_IPC_CMD_GET_SVR_IP_BY_IDX,         /* 10 */
    SNTP_MGR_IPC_CMD_GET_SV_OPMODE,             /* 11 */
    SNTP_MGR_IPC_CMD_GET_STATUS,                /* 12 */
    SNTP_MGR_IPC_CMD_SET_POLL_TIME,             /* 13 */
    SNTP_MGR_IPC_CMD_SET_SV_OPMODE,             /* 14 */
    SNTP_MGR_IPC_CMD_SET_STATUS,                /* 15 */
    SNTP_MGR_IPC_CMD_GET_NEXT_SVR_IP_BY_IDX,    /* 16 */
    SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_SNMP,       /* 17 */
};


/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - SNTP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SNTP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of SMTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SNTP_MGR_GET_MSGBUFSIZE(type_name) \
        ((uintptr_t) & ((SNTP_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SNTP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of SNTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA()    sizeof(SNTP_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - SNTP_MGR_MSG_CMD
 *              SNTP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the SNTP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The SNTP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SNTP_MGR_MSG_CMD(msg_p)    (((SNTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define SNTP_MGR_MSG_RETVAL(msg_p) (((SNTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - SNTP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SNTP_MGR_MSG_DATA(msg_p)   ((void *)&((SNTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* TYPE DECLARATIONS
 */
/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} SNTP_MGR_IPCMsg_Type_T;

typedef struct
{
    UI32_T  status;
} SNTP_MGR_IPCMsg_Status_T;

typedef struct
{
    UI32_T  poll_time;
} SNTP_MGR_IPCMsg_PollTime_T;

typedef struct
{
    UI32_T  serv_mode;
} SNTP_MGR_IPCMsg_ServMode_T;

typedef struct
{
    L_INET_AddrIp_T ip_addr;
} SNTP_MGR_IPCMsg_IpAddr_T;

typedef struct
{
    UI32_T  index;
    L_INET_AddrIp_T ip_addr;
} SNTP_MGR_IPCMsg_IpAddrWithIndex_T;

typedef struct
{
    UI32_T  index;
} SNTP_MGR_IPCMsg_SvrIndex_T;

typedef union
{
    SNTP_MGR_IPCMsg_IpAddr_T            ipadr;  /* for add /del ip */
    SNTP_MGR_IPCMsg_IpAddrWithIndex_T   ipidx;  /* for get server ip */
    SNTP_MGR_IPCMsg_PollTime_T          ptime;  /* for get /set poll time */
    SNTP_MGR_IPCMsg_ServMode_T          srmod;  /* for get service mode */
    SNTP_MGR_IPCMsg_Status_T            stats;  /* for get /set status */
    SNTP_MGR_IPCMsg_SvrIndex_T          svrid;  /* for del server */
} SNTP_MGR_IPCMsg_Data_T;

typedef struct
{
    SNTP_MGR_IPCMsg_Type_T  type;
    SNTP_MGR_IPCMsg_Data_T  data;
} SNTP_MGR_IPCMsg_T;

typedef enum
{
    SNTP_TYPE_SYSTEM_STATE_TRANSITION       = SYS_TYPE_STACKING_TRANSITION_MODE,
    SNTP_TYPE_SYSTEM_STATE_MASTER           = SYS_TYPE_STACKING_MASTER_MODE,
    SNTP_TYPE_SYSTEM_STATE_SLAVE            = SYS_TYPE_STACKING_SLAVE_MODE,
} SNTP_TYPE_SYSTEM_STATE_T;


/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize SNTP_MGR used system resource, eg. protection semaphore.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_Init(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 void SNTP_MGR_EnterSlaveMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for sntp mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_GetOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return the SNTP_MGR  mode. (slave/master/transition)
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T SNTP_MGR_GetOperationMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_sntpStatus_enabled, 2 :VAL_sntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_SetStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current SNTP status.1 :VAL_sntpStatus_enabled, 2: VAL_sntpStatus_disabled
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_GetStatus(UI32_T *status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_sntpStatus_enabled/VAL_sntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: 	 default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_MGR_GetRunningStatus(UI32_T *stuats);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_SetServiceOperationMode(UI32_T mode);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_GetServiceOperationMode(UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_sntpServiceMode_unicast = 1
 *           VAL_sntpServiceMode_broadcast = 2
 *           VAL_sntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_MGR_GetRunningServiceMode(UI32_T *servicemode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_SetPollTime(UI32_T polltime);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_GetPollTime(UI32_T *polltime);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningPollTime
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   polling time
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: 	default value is 16 secconds
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_MGR_GetRunningPollTime(UI32_T *polltime);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : 1.index 2. ip address
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : 1.Delete one server ip will cause the ip behind deleted-ip was deleted.
 *            2.The ip is zero if it is deleted
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_DeleteServerIp(UI32_T index);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.index
 *            2.point to buffer that can contain information of a server
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *			  FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_GetServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - GetNextServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next server entry using ip address as index
 * INPUT    : 1.index
 *            2.point to buffer that can contain server ip address
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : if input index is 0xFFFFFFFF, then it will find first entry in table
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_GetNextServerIp(UI32_T *index, L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetLastUpdateUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of last update-time
 * INPUT    : buffer pointer stored time information
 * OUTPUT   : time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *			  FALSE: SNTP never get time from server.
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetLastUpdateUTCTime(UI32_T *time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetCurrentUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of GMT
 * INPUT    : Buffer of  UTC time
 * OUTPUT   : 1.time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *			  FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetCurrentUTCTime(UI32_T *time);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : TRUE : If success
 *			  FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetCurrentServer(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_InTimeServiceMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform time serice mode,e.g, unicast, broadcast mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by SNTP_TASK
 *------------------------------------------------------------------------------*/
void SNTP_MGR_InTimeServiceMode(void);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_AddServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_AddServerIpForCLI(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3,
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_DeleteServerIpForCLI(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_DeleteAllServerIp(void);

/* FUNCTION NAME - SNTP_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SNTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* FUNCTION NAME - SNTP_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SNTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIpForSNMP
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a set of server ip to prevent the hole problem
 * INPUT    : index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : Ex: index1_IP=1.1.1.1, index2_IP=1.1.1.2, index3_IP=1.1.1.3
 *            1.if delete index2_IP, index3_IP wiil be deleted.
 *			  2.if delete index1_IP, index2_IP & index3_IP wiil be deleted.
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_MGR_DeleteServerIpForSNMP(UI32_T index);

#endif /* SNTP_MGR_H */

