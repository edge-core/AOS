/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_OM.h                                                                 
 *                                                                                      
 * PURPOSE: This package serves as a database to store VRRP MIB (RFC 2787) defined information.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009                               
 * -------------------------------------------------------------------------------------*/
 

#ifndef __VRRP_OM_H
#define __VRRP_OM_H

#include "sys_type.h"
#include "vrrp_type.h"
#include "sys_bld.h"
#include "sysfun.h"

/* TYPE DECLARATIONS 
 */

/* NAMING CONSTANT DECLARATION 
 */
 
#define VRRP_OM_NOT_IN_REAL_INTERFACE   0x90000001 

/* command used in IPC message */
enum
{
    VRRP_OM_IPC_GET_OPER_ENTRY = 1,
    VRRP_OM_IPC_GET_NEXT_ASSOC_IP_ADDRESS,
    VRRP_OM_IPC_GET_ASSOC_IP_ADDRESS_BY_IP,
#if (SYS_CPNT_VRRP_PING == TRUE)
    VRRP_OM_IPC_GET_PING_STATUS,
#endif
};

/* The key to get igmpsnp mgr msgq.
 */
#define VRRP_OM_IPCMSG_KEY    SYS_BLD_VRRP_PROC_OM_IPCMSGQ_KEY

/* These value will be use by mgr handler to set msg.type.result
 *   VRRP_OM_IPC_RESULT_OK   - only use when API has no return value
 *                                 and mgr deal this request.
 *   VRRP_OM_IPC_RESULT_FAIL - it denote that mgr handler can't deal
 *                                 the request. (ex. in transition mode)
 */
#define VRRP_OM_IPC_RESULT_OK    (0)
#define VRRP_OM_IPC_RESULT_FAIL  (-1)

/* MACRO FUNCTION DECLARATIONS 
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - VRRP_OM_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of VRRP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of VRRP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define VRRP_OM_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((VRRP_OM_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - VRRP_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of VRRP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of VRRP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define VRRP_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(VRRP_OM_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - VRRP_OM_MSG_CMD
 *              VRRP_OM_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the VRRP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The VRRP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define VRRP_OM_MSG_CMD(msg_p)    (((VRRP_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define VRRP_OM_MSG_RETVAL(msg_p) (((VRRP_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - VRRP_OM_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define VRRP_OM_MSG_DATA(msg_p)   ((void *)&((VRRP_OM_IPCMsg_T *)(msg_p)->msg_buf)->data)

typedef struct
{
    UI32_T value;
} VRRP_OM_IPCMsg_UI32_T;

typedef struct
{
    VRRP_OPER_ENTRY_T entry;
} VRRP_OM_IPCMsg_OperEntry_T;

typedef struct
{
    VRRP_ASSOC_IP_ENTRY_T entry;
} VRRP_OM_IPCMsg_AssocIpEntry_T;

typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} VRRP_OM_IPCMsg_Type_T;

typedef union
{
    VRRP_OM_IPCMsg_UI32_T             u32;
    VRRP_OM_IPCMsg_OperEntry_T        oper_entry;
    VRRP_OM_IPCMsg_AssocIpEntry_T     assoc_ip_entry;
} VRRP_OM_IPCMsg_Data_T;

typedef struct
{
    VRRP_OM_IPCMsg_Type_T type;
    VRRP_OM_IPCMsg_Data_T data;
} VRRP_OM_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_Init
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function initialize vrrp_om.  This will allocate memory for 
 *            vrrp table and create link list to maintain vrrp table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------*/
void VRRP_OM_Init(void);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetTimerId
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Set VRRP_GROUP periodic timer id
 * INPUT    : timer_id      --  timer id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
void VRRP_OM_SetTimerId(void *timer_id);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetTimerId
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP_GROUP periodic timer id
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : timer_id
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
void *VRRP_OM_GetTimerId(void);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearDatabase
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function clears all entry in VRRP table and VRRP associated ip table 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function shall be invoked when system enters transition mode.
 *            2. All the entries in database will be purged.
 *-----------------------------------------------------------------------------------*/
void VRRP_OM_ClearDatabase(void); 


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes the specific virtual router on some interface  
 * INPUT    : vrrp_info.ifindex -- specify which ifindex the virtual router
 *            residents.
 *            vrrp_info.vrid -- specify which vrid to be deleted
 * OUTPUT   : TRUE if vrrp_info entry has been deleted. False, otherwise.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetFirstVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if virtual router on some interface   
              is available.  Otherwise, return false.
 * INPUT    : vrrp_info->ifindex  -- specify which vrrp ifindex to be retrieved
 *            vrrp_info->vrid  -- specify which vrrp ifindex to be retrieved
 * OUTPUT   : returns the specific vrrp info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetFirstVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if virtual router on some interface   
              is available.  Otherwise, return false.
 * INPUT    : vrrp_info->ifindex  -- specify which vrrp ifindex to be retrieved
 *            vrrp_info->vrid  -- specify which vrrp ifindex to be retrieved
 * OUTPUT   : returns the specific vrrp info
 * RETURN   : VRRP_TYPE_OK/VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_VrrpOperEntryCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Check is any VRRP entry exist
 * INPUT    : None
 * OUTPUT   :
 * RETURN   : TRUE - there is VRRP entry
 *            FALSE- there is no VRRP entry
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_VrrpOperEntryCount();

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vrrp entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vrrp_info->ifindex  -- specify which interface information to be retrieved.
 *            vrrp_info->vrid  -- specify which vrrp to be retrieved.
 * OUTPUT   : return next available vrrp ifindex info
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_ACCESS_SUCCESS
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info); 


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function modifies(creates) the specific vrrp entry.
 * INPUT    : vrrp_info->ifindex  -- which interface vrrp residents on
 *            vrrp_info->vrid  -- which vrrp to be modified.
 * OUTPUT   : vrrp info has been updated.
 * RETURN   : VRRP_TYPE_INTERNAL_ERROR/ 
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VRRP_MGR shall use this function to create a new VRRP ifindex entry.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes the associated ip address on the specific 
 *            vrrp and interface. The "delete" action just "disables" the 
 *            specific entry, not really removes it.
 * INPUT    : associated_info.ifindex -- which ifindex vrrd residents on.
 *            associated_info.vrid -- which vrid associated ip address with
 *            associated_info.ip_addr -- which associated ip address 
 * OUTPUT   : TRUE if associated_info entry has been deleted. False, otherwise.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpAssoIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the availabe associated IP address of the 
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address 
 * OUTPUT   : returns the associated ip address info
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpAssoIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the next availabe associated IP address of the 
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address 
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpAssoIpAddress(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info); 


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if associated ip address on some 
 *            interface has been modified successfully. otherwise, false is return.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address 
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK / 
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VRRP_MGR shall use this function to create a new VRRP ifindex entry.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SetVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SearchVrrpAssoIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function searches the specific associated ip address.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address 
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK / 
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : 1. If the specified entry does not exist, return FALSE.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SearchVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpAssoIpAddrEntryByIpaddr
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get specified vrrp associated IP entry by ifindex and ip address
 * INPUT    : associated_info.ip_addr -- specify which ip address to search
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : None
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpAssoIpAddrEntryByIpaddr(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_VridExistOnIf
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of VRRPs currently 
 *            configured in the interface is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vrrp
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_VridExistOnIf(UI32_T ifindex, UI32_T vrid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetCurrentNumbOfVRRPConfigured
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of VRRPs currently 
 *            configured in the interface is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vrrp
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetCurrentNumOfVRRPConfigured(UI32_T *current_cfg_vrrp);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetOperStat
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the system operation info can be
 *            retrived
 * INPUT    : none            
 * OUTPUT   : The vrrp_oper_info opeation stat
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetOperStat(VRRP_OPER_ENTRY_T *vrrp_oper_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears the system statistics 
 * INPUT    : none            
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_ClearVrrpSysStatistics(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the system statistics 
 * INPUT    : none            
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_SetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T router_statis_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the system statistics 
 * INPUT    : none            
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *router_statis_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears the vrrp group statistics
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the vrrp group statistics
 * INPUT    : ifindex, vrid, statistics info to be set
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_SetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the vrrp group statistics
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : statistics info of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR  
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the next vrrp group statistics
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : statistics info of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpGlobalConfig
 *--------------------------------------------------------------------------
 * PURPOSE  : This function set VRRP global configuration
 * INPUT    : config	--	global configuration structure
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_SetVrrpGlobalConfig(VRRP_TYPE_GlobalEntry_T *config);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpGlobalConfig
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get VRRP global configuration
 * INPUT    : config	--	global configuration structure
 * OUTPUT   : config	--	global configuration structure
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_GetVrrpGlobalConfig(VRRP_TYPE_GlobalEntry_T *config);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_GetPingStatus(UI32_T *ping_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_IncreaseRouterStatisticCounter
 *--------------------------------------------------------------------------
 * PURPOSE  : This function increase router statistic counter
 * INPUT    : flag	--	counter flag
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_IncreaseRouterStatisticCounter(VRRP_TYPE_ROUTER_STATISTICS_FLAG_T flag);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for VRRP om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VRRP_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

#endif


