/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_MGR.h
 *
 * PURPOSE: This package provides the service routines to manage VRRP (RFC 2338)
 * NOTES:   The key functions of this module are to provide interfaces for the upper layer
 *          to configure VRRP, update VRRP database information based on the configuration.
 *
 *
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 * -------------------------------------------------------------------------------------*/


#ifndef __VRRP_MGR_H
#define __VRRP_MGR_H

#include "vrrp_type.h"
#include "backdoor_mgr.h"
/* TYPE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATION
 */

/* success value */
#define	VRRP_MGR_OK						  0x00000000
#define VRRP_MGR_REPLACE_DYNAMIC_ENTRY    0x00000001

/* fail value */

#define	VRRP_MGR_INVALID_ARG			  0x80000003
#define	VRRP_MGR_INVALID_NEXT_HOP		  0x80000004
#define	VRRP_MGR_CAN_NOT_ADD			  0x80000005
#define VRRP_MGR_CAN_NOT_DELETE           0x80000006
#define	VRRP_MGR_NO_MORE_INTERFACE		  0x80000007
#define VRRP_MGR_CAN_NOT_ADD_LOCAL_IP     0x80000008
#define VRRP_MGR_CAN_NOT_DELETE_LOCAL_IP  0x80000009
#define VRRP_MGR_TABLE_FULL               0x8000000A
#define VRRP_MGR_IP_ALREADY_EXIST         0x8000000B
#define VRRP_MGR_FAIL                     0x8000000C
#define VRRP_MGR_REJECT_SETTING_ENTRY     0x8000000D
#define VRRP_MGR_INTERFACE_NOT_EXISTED    0x8000000E
#define VRRP_MGR_NO_SUCH_SUBNET           0x8000000F
#define VRRP_MGR_NOT_IMPLEMENT            0x80000010
#define VRRP_MGR_NOT_FOUND                0x80000011
#define VRRP_MGR_NOT_MASTER_MODE		  0x80000012
#define VRRP_MGR_CAN_NOT_GET			  0x80000013
#define	VRRP_MGR_NOT_IN_SERVICE			  0x80000014
#define	VRRP_MGR_INVALID_IP				  0x80000015
#define VRRP_MGR_CAN_NOT_SET			  0x80000016
#define VRRP_MGR_DESTINATION_IS_LOCAL	  0x80000017
#define	VRRP_MGR_ALREADY_FORWARDING		  0x80000018
#define	VRRP_MGR_NO_SUCH_INTERFACE		  0x80000019
#define VRRP_MGR_NO_MORE_ENTRY            0x80000020

#define INET_ADDRESS_IPV4_SIZE 4
#define VRRP_MGR_AUTH_KEY_LENGTH          8


#define VRRP_MGR_IPCMSG_TYPE_SIZE sizeof(union VRRP_MGR_IPCMsg_Type_U)
#define VRRP_MGR_MSG_HEADER_SIZE ((uintptr_t) & ((VRRP_MGR_IPCMsg_T*)0)->data)

#define VRRP_MGR_GET_MSGBUF_SIZE(field_name) \
    ((uintptr_t) & ((VRRP_MGR_IPCMsg_T *)0)->data + sizeof(field_name))

#define VRRP_BD_FLAG_DBG      1
#define VRRP_BD_FLAG_INFO    (1<<1)
#define VRRP_BD_FLAG_NOTE    (1<<2)
#define VRRP_BD_FLAG_EVENT   (1<<3)
#define VRRP_BD_FLAG_TIMER   (1<<4)

#define VRRP_BD_MSG(__str, __arg...)  BACKDOOR_MGR_Printf(__str, ##__arg);
#define VRRP_BD_IS_FLAG_ON(__flag)	  VRRP_MGR_BACKDOOR_GetDebugFlag(VRRP_BD_FLAG_##__flag)
#define VRRP_BD_GetLine(buf, size) ({ BACKDOOR_MGR_RequestKeyIn((buf), (size)); strlen((char *)buf); })

#define VRRP_BD(__flag, format,...)         \
    do \
    {			\
        if (VRRP_BD_IS_FLAG_ON(__flag))				 \
        {											\
            VRRP_BD_MSG("%s[%d], ", __FUNCTION__, __LINE__); \
            VRRP_BD_MSG(format,##__VA_ARGS__); \
            VRRP_BD_MSG("\r\n"); \
        }											\
    }while(0)


/***************************************************
 **    vrrp_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    VRRP_MGR_IPC_IPADDRESSSET,
    VRRP_MGR_IPC_PRIMARYIPSET,
    VRRP_MGR_IPC_IPADDRESSGETNEXT,
    VRRP_MGR_IPC_OPERADMINSTATUSSET,
    VRRP_MGR_IPC_OPERADMINSTATUSGET,
    VRRP_MGR_IPC_OPERAUTHTYPESET,
    VRRP_MGR_IPC_OPERAUTHTYPEGET,
    VRRP_MGR_IPC_OPERAUTHKEYSET,
    VRRP_MGR_IPC_OPERAUTHKEYGET,
    VRRP_MGR_IPC_OPERAUTHSET,
    VRRP_MGR_IPC_PRIORITYSET,
    VRRP_MGR_IPC_PRIORITYGET,
    VRRP_MGR_IPC_PREEMPTIONMODESET,
    VRRP_MGR_IPC_PREEMPTMODESET,
    VRRP_MGR_IPC_PREEMPTMODEGET,
    VRRP_MGR_IPC_ADVERINTERVALSET,
    VRRP_MGR_IPC_ADVERINTERVALGET,
    VRRP_MGR_IPC_PINGSET,
    VRRP_MGR_IPC_OPERROWSTATUSSET,
    VRRP_MGR_IPC_OPERENTRYSET,
    VRRP_MGR_IPC_OPERENTRYDEL,
    VRRP_MGR_IPC_OPERENTRYGETNEXT,
    VRRP_MGR_IPC_DEFAULTOPERENTRYGET,
    VRRP_MGR_IPC_ASSOCIPADDRENTRYSET,
    VRRP_MGR_IPC_ASSOCIPADDRENTRYDEL,
    VRRP_MGR_IPC_ASSOCIPADDRENTRYGET,
    VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT,
    VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT_SNMP,
    VRRP_MGR_IPC_OPERIPADDRCOUNTGET,
    VRRP_MGR_IPC_OPERSTATEGET,
    VRRP_MGR_IPC_OPERPROTOCOLSET,
    VRRP_MGR_IPC_OPERPROTOCOLGET,
    VRRP_MGR_IPC_VERSIONNUMBERGET,
    VRRP_MGR_IPC_VRRPSYSSTATISTICSGET,
    VRRP_MGR_IPC_VRRPSYSSTATISTICSCLEAR,
    VRRP_MGR_IPC_VRRPGROUPSTATISTICSCLEAR,
    VRRP_MGR_IPC_VRRPGROUPSTATISTICSGET,
    VRRP_MGR_IPC_VRRPGROUPSTATISTICSGETNEXT,
    VRRP_MGR_IPC_RUNNINGPRIORITYGET,
    VRRP_MGR_IPC_RUNNINGPRIORITYGETNEXT,
    VRRP_MGR_IPC_RUNNINGAUTHTYPEGET,
    VRRP_MGR_IPC_RUNNINGAUTHTYPEGETNEXT,
    VRRP_MGR_IPC_RUNNINGADVERINTGET,
    VRRP_MGR_IPC_RUNNINGADVERINTGETNEXT,
    VRRP_MGR_IPC_RUNNINGPREEMPTGET,
    VRRP_MGR_IPC_RUNNINGPREEMPTGETNEXT,
    VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGET,
    VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGETNEXT,
    VRRP_MGR_IPC_RUNNINGPROTOCOLGET,
    VRRP_MGR_IPC_RUNNINGPROTOCOLGETNEXT,
    VRRP_MGR_IPC_RUNNINGASSOCIPGET,
    VRRP_MGR_IPC_RUNNINGASSOCIPGETNEXT,
    VRRP_MGR_IPC_RUNNINGPINGSTATUSGET
};

typedef struct VRRP_MGR_IPCMsg_S
{
    union VRRP_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;
    
    union
    {
        VRRP_OPER_ENTRY_T oper_entry;
        VRRP_ASSOC_IP_ENTRY_T assoc_ip_addr;
        UI32_T  version;
        UI32_T  ifindex;
        VRRP_OM_Router_Statistics_Info_T vrrp_router_statistics;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI8_T    arg3[INET_ADDRESS_IPV4_SIZE];
            UI32_T   arg4;
        } arg_grp1;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI8_T    arg3[INET_ADDRESS_IPV4_SIZE];
        } arg_grp2;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI32_T   arg3;
        } arg_grp3;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI8_T    arg3[VRRP_MGR_AUTH_KEY_LENGTH + 1];
        } arg_grp4;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI32_T   arg3;
            UI8_T    arg4[VRRP_MGR_AUTH_KEY_LENGTH + 1];
        } arg_grp5;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_grp6;
        
        struct
        {
            VRRP_OPER_ENTRY_T   arg1;
            UI32_T   arg2;
        } arg_grp7;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
        } arg_grp8;
        
        struct
        {
            UI32_T   arg1;
            UI8_T    arg2;
            VRRP_OM_Vrrp_Statistics_Info_T   arg3;
        } arg_grp9;
        
        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
        } arg_grp10;
        
    }data;
}VRRP_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM DECLARATION
 */

BOOL_T VRRP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *msgbuf_p);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for VRRP_om and mgr.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_RegisterCallbackFunction
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will register the callback functions
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This api registers the callback functions provided in the NETCFG
 *               file for being notified by Netcfg when rif transfers from down
 *               to up or from up to down.
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_RegisterCallbackFunction(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable VRRP operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VRRP_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the VRRP operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the VRRP operation while in transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set operation mode into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP operation mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : SYS_TYPE_STACKING_Master_MODE
 *            SYS_TYPE_STACKING_SLAVE_MODE
 *            SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTES    : none
 *-------------------------------------------------------------------------- */
SYS_TYPE_Stacking_Mode_T VRRP_MGR_GetOperationMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_Process_VrrpPacket
 *--------------------------------------------------------------------------
 * PURPOSE  : Process VRRP packets.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-------------------------------------------------------------------------- */
BOOL_T VRRP_MGR_Process_VrrpPacket(VRRP_TASK_MSG_QUEUE_ITEM_T *vrrp_msg_queue_item_ptr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ProcessTimerEvent
 *--------------------------------------------------------------------------
 * PURPOSE  : Do the process for timer event
 * INPUT    : pass_time         --  passed time interval in ticks
 * OUTPUT   : min_expire_time   --  minimum expire time interval in ticks
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-------------------------------------------------------------------------- 
 */
BOOL_T VRRP_MGR_ProcessTimerEvent(UI32_T pass_time, UI32_T *min_expire_time);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetTimerId
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Set VRRP_GROUP periodic timer id
 * INPUT    : timer_id      --  timer id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T VRRP_MGR_SetTimerId(void *timer_id);

/* VRRP MIB groups
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Set.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. Row status must be up before set enabled(1).
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T vrrp_admin_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : vrrp admin status for the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_admin_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ip address for the spicific vrrp on the interface (RFC2338, p.13)
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 *            row_status: VAL_vrrpAssoIpAddrRowStatus_notInService or
 *                        VAL_vrrpAssoIpAddrRowStatus_destroy to delete this ip address
 *                        VAL_vrrpAssoIpAddrRowStatus_createAndGo or
 *                        VAL_vrrpAssoIpAddrRowStatus_active to add this ip address
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/TRUE/FALSE/
 * NOTES    : modified by Aris to fix EPR:ES3628C-PoE-20-00181
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr, UI32_T row_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPrimaryIp
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the primary ip address for the spicific vrrp on the interface (RFC2338, p.13)
 *            for MIB
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPrimaryIp(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next ip address for vrrp (RFC2338, p.13)
 * INPUT    : if_index: the specific interface
 *            vrid:     the specific vrrp group id
 *            ip_addr:  the buffer to get the ip address
 * OUTPUT   : next associated ip address of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : MIN_vrrpOperPriority	0L
 *            MAX_vrrpOperPriority	255L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPriority(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : vrrp priority of the specific ifindex oand vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetPriority(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid, preempt mode and delay time for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode, UI32_T delay);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp (RFC2338, p.14) for MIB
 * INPUT    : ifindex, vrid, preempt mode for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPreemptMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the preemption mode for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : preempt mode and preempt delay time for this group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_preempt_mode, UI32_T *delay);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the advertisement interval for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid, advertisement interval for the group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : SYS_ADPT_MIN_VIRTUAL_ROUTER_ADVER_INT   1
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_ADVER_INT   255
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_advertise_interval);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the advertisement interval for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : advertisement interval of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_advertise_interval);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication type for each interface (RFC2338, p.13)
 * INPUT    : if_Index, vrid,
 *            auth_type -
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_auth_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication type for each interface (RFC2338, p.13)
 * INPUT    : if_Index, vrid
 * OUTPUT   : authentication type of the specific ifindex and vrid
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_auth_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication key for each interface (RFC2338, p.13)
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication key for each interface (RFC2338, p.13)
 * INPUT    : ifindex, vrid
 * OUTPUT   : authentication key of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthentication
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the authentication data is set successfully.
              Otherwise, return false.
 * INPUT    : ifIndex, vrid,
 *            auth_type -
              	VAL_vrrpOperAuthType_noAuthentication	1L \
              	VAL_vrrpOperAuthType_simpleTextPassword	2L \
              	VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
              auth_key
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetAuthentication(UI32_T if_index, UI8_T vrid, UI32_T auth_type, UI8_T *auth_key);

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_MGR_SetPingStatus(UI32_T ping_status);

#endif


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpOperRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry->if_index, vrrp_oper_entry->vrid,
 *            action    row_status to be set to the specific vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : action --
 *            VAL_vrrpOperRowStatus_active	      1L
 *            VAL_vrrpOperRowStatus_notInService    2L
 *            VAL_vrrpOperRowStatus_notReady        3L
 *            VAL_vrrpOperRowStatus_createAndGo	  4L
 *            VAL_vrrpOperRowStatus_createAndWait   5L
 *            VAL_vrrpOperRowStatus_destroy	      6L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpOperRowStatus(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T action);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP operation entry
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : next ifindex, next vrid, next vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetDefaultVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the default VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *		      VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetDefaultVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpAssocIpAddrEntryBySnmp
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : TRUE/FALSE
 * NOTES    :(1)use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *              vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *           (2)This API is used by snmp to getnext AssocIpAddrEntry through
 *              whole system
 *-------------------------------------------------------------------------- */
BOOL_T VRRP_MGR_GetNextVrrpAssocIpAddrEntryBySnmp(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the number of IP addresses for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : ip address count of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetIpAddrCount(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_ip_addr_count);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperState
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton gets the operation state of the specific vrid and ifindex
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper state of the specific vrrp group -
 *            VAL_vrrpOperState_initialize       1L
 *            VAL_vrrpOperState_backup           2L
 *            VAL_vrrpOperState_master           3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. This object affects all VRRP Applicant and Registrar
 *            state machines.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperState(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_state);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Set operation protocol for the specific ifindex and vrid
 * INPUT    : ifIndex  ,  vrid, operation protocol
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_protocol);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_protocol);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVersionNumber
 *------------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vrrp version of the system.
 * INPUT    : buffer to be put in the version number
 * OUTPUT   : The vrrp version number
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *            2. It's always return "2" right now.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVersionNumber(UI32_T *version);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetMasterPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the priority of the Master router.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The vrrp version number
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetMasterPriority(UI32_T *ifindex, UI8_T *vrid, UI32_T *master_priority);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetMasterAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the advertisement interval of the Master router.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The vrrp version number
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetMasterAdverInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *master_int);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetMasterDownInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the master down interval.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The vrrp version number
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetMasterDownInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *master_int);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetCurrentNumVrs
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the number of RFC 2338 Virtual Router are
 *            currently configured in the system.
 * INPUT    : buffer to be put in the current numbers of virtual routers configured
 * OUTPUT   : The current numbers of virtual routers configured
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetCurrentVrrpNum(UI32_T *vrrp_number);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ClearVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_ClearVrrpSysStatistics(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *vrrp_router_statistics);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ClearVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the specific vrrp group statistics.
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the next specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics);

/************************************************************************
 * The following APIs are for running config
 ************************************************************************/
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPriority(UI32_T ifindex, UI8_T vrid, UI32_T *priority);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPriority(UI32_T *ifindex, UI8_T *vrid, UI32_T *priority);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAuthType(UI32_T ifindex, UI8_T vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : ifindex   -- next ifindex
 *            vrid      -- next vrid
 *            auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAuthType(UI32_T *ifindex, UI8_T *vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_key -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            auth_key -- output vrrp authentication key
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAuthKey(UI32_T ifindex, UI8_T vrid, UI32_T *auth_type, UI8_T *auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_key -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            auth_key -- output vrrp authentication key
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAuthKey(UI32_T *ifindex, UI8_T *vrid, UI32_T *auth_type, UI8_T *auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAdverInt(UI32_T ifindex, UI8_T vrid, UI32_T *interval);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAdverInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *interval);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPreemptMode(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPreemptMode(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPreemptDelay(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_delay);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPreemptDelay(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_delay);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpProtocol(UI32_T ifindex, UI8_T vrid, UI32_T *protocol);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpProtocol(UI32_T *ifindex, UI8_T *vrid, UI32_T *protocol);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ip_addr_count  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAssocIp(UI32_T ifindex, UI8_T vrid, UI8_T *ip_addr_count);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            ip_addr  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAssocIp(UI32_T *ifindex, UI8_T *vrid, UI8_T *ip_addr);

#if (SYS_CPNT_VRRP_PING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningPingStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP ping status and check whether it is default value.
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningPingStatus(UI32_T *ping_status);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalRifActive
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that rifActive.
 * INPUT    : ip_addr  -- ip address of down rif
 *            ip_mask  -- ip mask of down rif
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalRifDown
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that rifActive.
 * INPUT    : ip_addr  -- ip address of down rif
 *            ip_mask  -- ip mask of down rif
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalVlanInterfaceDelete
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that vlan interface delete.
 * INPUT    : ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/

void VRRP_MGR_SignalVlanInterfaceDelete(UI32_T ifindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_GetDebugFlag
 *------------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *------------------------------------------------------------------------------
 */
BOOL_T  VRRP_MGR_BACKDOOR_GetDebugFlag(UI32_T flag);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_MainMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will display VRRP main menu,
 *           and select debug information
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void VRRP_MGR_BACKDOOR_MainMenu(void);

#endif

