/* Module Name: DHCPSNP_MGR.H
 * Purpose:
 *      DHCPSNP_MGR provides all DHCP Snooping accessing functions, include Provision Start/Complete. 
 *      All task access-protections are kept in this module.
 *
 * Notes:
 *      1. CLI command includes :
 *          ?????????????????????????????????????????
 *      2. Execution function includes :
 *          ?????????????????????????????????????????
 *
 *
 * History:
 *       Date       -- Modifier,  Reason
 *  0.1 2006.1.16  --  Andrew Huang, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001.
 */

#ifndef     _DHCPSNP_MGR_H
#define     _DHCPSNP_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include "sysfun.h"
#include "dhcpsnp_type.h"

/* NAME CONSTANT DECLARATIONS
 */
#define DHCPSNP_MGR_IPC_RESULT_OK    (0)
#define DHCPSNP_MGR_IPC_RESULT_FAIL  (-1)

/* The commands for IPC message.
 */
enum {
    DHCPSNP_MGR_IPC_CMD_CLEAR_BINDING_ENTRIES_IN_FLASH,
    DHCPSNP_MGR_IPC_CMD_DELETE_STATIC_IPSRCGUARD_BINDING_ENTRY,
    DHCPSNP_MGR_IPC_CMD_DISABLE_DHCPSNOOPING_BY_VLAN,
    DHCPSNP_MGR_IPC_CMD_ENABLE_DHCPSNOOPING_BY_VLAN,
    DHCPSNP_MGR_IPC_CMD_GET_DHCPSNOOPING_BINDING_ENTRY,
    DHCPSNP_MGR_IPC_CMD_GET_DHCPSNOOPING_STATUS_BY_VLAN,
    DHCPSNP_MGR_IPC_CMD_GET_GLOBAL_DHCPSNOOPING_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_POLICY,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_RID_MODE,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_RID_VALUE,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_PORT_CID_MODE,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_PORT_CID_VALUE,
    DHCPSNP_MGR_IPC_CMD_GET_INFORMATION_OPTION_FORMAT,
    DHCPSNP_MGR_IPC_CMD_GET_NEXT_ACTIVE_DHCPSNOOPING_VLAN,
    DHCPSNP_MGR_IPC_CMD_GET_NEXT_DHCPSNOOPING_BINDING_ENTRY,
    DHCPSNP_MGR_IPC_CMD_GET_NEXT_DHCPSNOOPING_STATUS_BY_VLAN,
    DHCPSNP_MGR_IPC_CMD_GET_NEXT_PORT_INFO,
    DHCPSNP_MGR_IPC_CMD_GET_NEXT_RUNNING_DHCPSNOOPING_STATUS_BY_VLAN,
    DHCPSNP_MGR_IPC_CMD_GET_PORT_INFO,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_GLOBAL_DHCPSNOOPING_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_INFORMATION_OPTION_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_INFORMATION_POLICY,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_INFORMATION_OPTION_RID_MODE,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_INFORMATION_OPTION_FORAMT,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_PORT_TRUST_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_VERIFY_MAC_ADDRESS_STATUS,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_INFORMATION_OPTION_PORT_CID_MODE,
    DHCPSNP_MGR_IPC_CMD_GET_TOTAL_FORWARDED_PKTCOUNTER,
    DHCPSNP_MGR_IPC_CMD_GET_UNTRUSTED_DROPPED_PKTCOUNTER,
    DHCPSNP_MGR_IPC_CMD_GET_VERIFY_MAC_ADDRESS_STATUS,
    DHCPSNP_MGR_IPC_CMD_SET_GLOBAL_DHCP_SNOOPING_STATUS,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_STATUS,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_POLICY,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_RID_MODE,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_RID_VALUE,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_PORT_CID_MODE,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_PORT_CID_VALUE,
    DHCPSNP_MGR_IPC_CMD_SET_INFORMATION_OPTION_SUBTYPE_FORMAT,
    DHCPSNP_MGR_IPC_CMD_SET_PORT_TRUST_STATUS,
    DHCPSNP_MGR_IPC_CMD_SET_VERIFY_MAC_ADDRESS_STATUS,
    DHCPSNP_MGR_IPC_CMD_WRITE_BINDING_ENTRIES_TO_FLASH,
    DHCPSNP_MGR_IPC_CMD_SET_BIND_LEASE_TIME,
    DHCPSNP_MGR_IPC_CMD_GET_PORT_TRUST_STATUS,
    DHCPSNP_MGR_IPC_CMD_IS_VALID_DAI,
    DHCPSNP_MGR_IPC_CMD_DELETE_DYNAMIC_DHCPSNP_BINDING_ENTRY,
    DHCPSNP_MGR_IPC_CMD_DELETE_ALL_DYNAMIC_DHCPSNP_BINDING_ENTRY,
    DHCPSNP_MGR_IPC_CMD_SET_SYSTEM_RATELIMIT,
    DHCPSNP_MGR_IPC_CMD_GET_SYSTEM_RATELIMIT,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_SYSTEM_RATELIMIT,
    DHCPSNP_MGR_IPC_CMD_RX_PROCESS_PACKET,
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
    DHCPSNP_MGR_IPC_CMD_SET_BOARD_ID,
    DHCPSNP_MGR_IPC_CMD_GET_BOARD_ID,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_BOARD_ID,
#endif
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
    DHCPSNP_MGR_IPC_CMD_SET_OPTION_DELIMITER,
    DHCPSNP_MGR_IPC_CMD_GET_OPTION_DELIMITER,
    DHCPSNP_MGR_IPC_CMD_GET_RUNNING_DELIMITER,
#endif
#if (SYS_CPNT_DHCP_RELAY == TRUE)
    DHCPSNP_MGR_IPC_CMD_SET_L3_RELAY_FORWARDING,
#endif  //SYS_CPNT_DHCP_RELAY
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCPSNP_MGR_IPC_CMD_SET_L2_RELAY_FORWARDING,
#endif
};    
    
    
/* MACRO FUNCTION DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - DHCPSNP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DHCPSNP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of DHCPSNP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DHCPSNP_MGR_GET_MSGBUFSIZE(type_name) \
    (offsetof(DHCPSNP_MGR_IPCMsg_T, data) + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - DHCPSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DHCPSNP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of DHCPSNP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DHCPSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(DHCPSNP_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - DHCPSNP_MGR_MSG_CMD
 *              DHCPSNP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the DHCPSNP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The DHCPSNP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DHCPSNP_MGR_MSG_CMD(msg_p)    (((DHCPSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define DHCPSNP_MGR_MSG_RETVAL(msg_p) (((DHCPSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - DHCPSNP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DHCPSNP_MGR_MSG_DATA(msg_p)   ((void *)&((DHCPSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)



/* DATA TYPE DECLARATIONS
 *
 *  Message declarations for IPC.
 */
typedef struct
{
    UI32_T  vid;
    UI8_T   mac[6];
    UI32_T  ip_addr;
    UI32_T  ip_addr_type;
    UI32_T  lport_ifindex;
	BOOL_T  allow_zero_sip;
}DHCPSNP_MGR_IPCMsg_data1_T;

typedef struct
{
    DHCPSNP_TYPE_BindingEntry_T    bind_entry;
}DHCPSNP_MGR_IPCMsg_Bind_Entry_T;

typedef struct
{
    UI32_T  vid;
    UI8_T   status;
}DHCPSNP_MGR_IPCMsg_Snooping_Status_T;

typedef struct
{
    UI32_T      lport;
    UI8_T       filter_mode;
}DHCPSNP_MGR_IPCMsg_Ip_Source_Guard_T;

typedef struct
{
    UI8_T option82_status;
    UI8_T option82_policy;
}DHCPSNP_MGR_IPCMsg_Option82_Data_T;

typedef struct
{
    DHCPSNP_TYPE_PortInfo_T   portinfo;
    UI32_T                    lport;
}DHCPSNP_MGR_IPCMsg_Port_Data_T;   

typedef struct
{
    UI32_T vid;
    UI8_T   mac_addr[6];
    UI32_T lease_time;
}DHCPSNP_MGR_IPCMsg_BIND_LEASE_TIME;

typedef struct
{
	UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
	UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
	UI32_T ing_vid;
	UI32_T ing_lport;
	UI32_T pkt_len;
	I32_T  mref_offset;
	I32_T  new_mref_offset;
}DHCPSNP_MGR_IPCMsg_RxProcessPacket_T;

typedef struct DHCPSNP_OM_LCB_S
{
    UI8_T       global_dhcp_snooping_status;
    UI8_T       verify_mac_address_status;
    UI8_T       option82_status;        /* 2006-06, Joseph *//* DHCPSNP Option82 ON or OFF */
    UI8_T       option82_policy;        /* 2006-06, Joseph *//* how to process DHCP Request packet which Option82 existed */
    UI8_T       dhcp_snooping_configured_vlan_bitmap_ar[DHCPSNP_TYPE_VLAN_BITMAP_SIZE_IN_BYTE];
} DHCPSNP_MGR_IPCMsg_LCB_T;

typedef struct
{
    DHCPSNP_MGR_IPCMsg_LCB_T       gdata;
}DHCPSNP_MGR_IPCMsg_LCB_Data_T;   

typedef struct
{
    UI32_T  lport_ifindex;
    UI8_T   status;    
}DHCPSNP_MGR_IPCMsg_PORT_TRUST_STATUS;

typedef struct
{
    UI32_T          total_fwd_pkt_counter;
    UI32_T          untrusted_dropped_pkt_counter;
}DHCPSNP_MGR_IPCMsg_Counter_Data_T;   


typedef struct
{
    UI32_T lport_ifindex;
    UI32_T limit_count;
    UI8_T  mode;
}DHCPSNP_MGR_IPCMsg_PortBindingEntryLimit_T;

typedef struct 
{
    UI8_T dhcpsnp_rid_mode;
    UI8_T  dhcpsnp_rid_value[SYS_ADPT_MAX_LENGTH_OF_RID + 1];
}DHCPSNP_MGR_IPCMsg_Rid_Data_T;


typedef struct
{
    UI32_T lport_ifindex;
    UI8_T  dhcpsnp_cid_mode;
    UI8_T  dhcpsnp_cid_value[SYS_ADPT_MAX_LENGTH_OF_CID + 1];
}DHCPSNP_MGR_IPCMsg_Cid_Data_T;

typedef struct 
{
    UI32_T ifindex;
    UI8_T  mode;
}DHCPSNP_MGR_IPCMsg_FilterTableMode_T;

typedef struct 
{
	UI32_T limit;
}DHCPSNP_MGR_IPCMsg_SystemRate_T;

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
typedef struct 
{
    UI8_T board_id;
}DHCPSNP_MGR_IPCMsg_BoardId_T;
#endif

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
typedef struct 
{
    UI8_T delimiter;
}DHCPSNP_MGR_IPCMsg_Delimiter_T;
#endif
#if (SYS_CPNT_DHCP_RELAY == TRUE)
typedef struct
{
    UI32_T vid;
    BOOL_T forwarding;
}DHCPSNP_MGR_IPCMsg_L3Forwarding_T;
#endif  //SYS_CPNT_DHCP_RELAY

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
typedef struct
{
    BOOL_T forwarding;
}DHCPSNP_MGR_IPCMsg_L2Forwarding_T;
#endif  //SYS_CPNT_DHCP_RELAY_OPTION82
typedef union
{
    DHCPSNP_MGR_IPCMsg_data1_T                  data1;    
    DHCPSNP_MGR_IPCMsg_Bind_Entry_T             bindentry;
    DHCPSNP_MGR_IPCMsg_Snooping_Status_T        status;
    DHCPSNP_MGR_IPCMsg_Option82_Data_T          option82;
    DHCPSNP_MGR_IPCMsg_Port_Data_T              port_data;
    DHCPSNP_MGR_IPCMsg_Ip_Source_Guard_T        ipsrcguard;
    DHCPSNP_MGR_IPCMsg_LCB_Data_T               lcbdata;
    DHCPSNP_MGR_IPCMsg_BIND_LEASE_TIME          leasetime;
    DHCPSNP_MGR_IPCMsg_PORT_TRUST_STATUS        truststatus;
    DHCPSNP_MGR_IPCMsg_PortBindingEntryLimit_T  limit;
    DHCPSNP_MGR_IPCMsg_Rid_Data_T               option82_rid;
    DHCPSNP_MGR_IPCMsg_Cid_Data_T               option82_cid;
    BOOL_T                                      subtype_format;    
    DHCPSNP_MGR_IPCMsg_FilterTableMode_T        table_mode;
    DHCPSNP_MGR_IPCMsg_RxProcessPacket_T        rx_pkt;
#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
    DHCPSNP_MGR_IPCMsg_SystemRate_T             rate_limit;
#endif
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
    DHCPSNP_MGR_IPCMsg_BoardId_T                board_id;
#endif
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
    DHCPSNP_MGR_IPCMsg_Delimiter_T              delimiter;
#endif
#if (SYS_CPNT_DHCP_RELAY == TRUE)
    DHCPSNP_MGR_IPCMsg_L3Forwarding_T           l3_fwd;
#endif  //SYS_CPNT_DHCP_RELAY    
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCPSNP_MGR_IPCMsg_L2Forwarding_T           l2_fwd;
#endif  //SYS_CPNT_DHCP_RELAY_OPTION82
} DHCPSNP_MGR_IPCMsg_Data_T;

typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} DHCPSNP_MGR_IPCMsg_Type_T;

    
typedef struct
{
    DHCPSNP_MGR_IPCMsg_Type_T    type;
    DHCPSNP_MGR_IPCMsg_Data_T    data;
} DHCPSNP_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : DHCPSNP_MGR_Init
 * PURPOSE:
 *        Initialize DHCPSNP_MGR used system resource.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DHCPSNP_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : DHCPSNP_MGR_SetTransitionMode
 * PURPOSE:
 *        Set transition mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_MGR_SetTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_MGR_EnterTransitionMode
 * PURPOSE:
 *        Enter transition mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_MGR_EnterTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_MGR_EnterMasterMode
 * PURPOSE:
 *        Enter master mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_MGR_EnterMasterMode(void);

/* FUNCTION NAME : DHCPSNP_MGR_EnterSlaveMode
 * PURPOSE:
 *        Enter slave mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_MGR_EnterSlaveMode(void);

/* FUNCTION NAME : DHCPSNP_MGR_ProvisionComplete
 * PURPOSE:
 *        Be signaled by STKCTRL, CLI had provision all configuration commands, all 
 *        CSC should begin do his job.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        1. This function is called by STKCTRL.
 */
void DHCPSNP_MGR_ProvisionComplete(void);

/* FUNCTION NAME : DHCPSNP_MGR_GetOperationMode
 * PURPOSE:
 *        Get current dhcpsnp operation mode (master / slave / transition).
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        operation_mode -- DHCPSNP_TYPE_SYSTEM_STATE_TRANSITION | DHCPSNP_TYPE_SYSTEM_STATE_MASTER |
 *                          DHCPSNP_TYPE_SYSTEM_STATE_SLAVE | DHCPSNP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE.
 *
 * NOTES:
 *        None.
 */
UI32_T DHCPSNP_MGR_GetOperationMode(void);

/*-----------------------------------
 *  CLI provision command function.
 *-----------------------------------
 */

/*-----------------------------------
 * The APIs for Register Callback Function 
 *-----------------------------------
 */

/*-----------------------------------
 * The APIs for DHCP Snooping 
 *-----------------------------------
 */

/* FUNCTION NAME : DHCPSNP_MGR_SetGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Enable/Diasble the global DHCP Snooping function.
 *
 * INPUT:
 *      status  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1. In Global CLI: "ip dhcp snooping" or "no ip dhcp snooping"
 *      2. default is disabled.
 *      3. If set the global status to disabled, we have to clear all dynamic dhcp snooping binding entry.
 */
UI32_T DHCPSNP_MGR_SetGlobalDhcpSnoopingStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_MGR_GetGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Get the global DHCP Snooping function status.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1. In Global CLI: show ip dhcp snooping
 */
UI32_T DHCPSNP_MGR_GetGlobalDhcpSnoopingStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_EnableDhcpSnoopingByVlan
 * PURPOSE:
 *      Enable dhcp snooping on specified vlan.
 *
 * INPUT:
 *      vid --  the vlan that will be added.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.In Global CLI: ip dhcp snooping/vlan [a range]
 *      2.CLI call it by a "for loop" to handle the range. ex: ip dhcp snooping/vlan 100-200 
 */
UI32_T DHCPSNP_MGR_EnableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_MGR_DisableDhcpSnoopingByVlan
 * PURPOSE:
 *      Disable dhcp snooping on specified vlan.
 *
 * INPUT:
 *      vid --  the vlan that will be deleted.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.In Global CLI: no ip dhcp snooping/vlan [a range]
 *      2.CLI call it by a "for loop" to handle the range. ex: no ip dhcp snooping/vlan 100-200 
 */
UI32_T DHCPSNP_MGR_DisableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_MGR_GetDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the DHCP Snooping status by the vlan id.
 *
 * INPUT:
 *      vid     --  the vlan id.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetDhcpSnoopingStatusByVlan(UI32_T vid, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetNextDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the next existed vlan id with DHCP Snooping status.
 *
 * INPUT:
 *      vid_p --  current vlan id.
 *
 * OUTPUT:
 *      vid_p     --  the next existed vlan id with DHCP Snooping status.
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping 
 */
UI32_T DHCPSNP_MGR_GetNextDhcpSnoopingStatusByVlan(UI32_T *vid_p, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetNextActiveDhcpSnoopingVlan
 * PURPOSE:
 *      Get the next active vlan id with DHCP Snooping Enabled.
 *
 * INPUT:
 *      vid_p --  current vlan id.
 *
 * OUTPUT:
 *      vid_p --  the next active vlan id with DHCP Snooping Enabled.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping 
 */
UI32_T DHCPSNP_MGR_GetNextActiveDhcpSnoopingVlan(UI32_T *vid_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetIpSourceGuardMode
 * PURPOSE:
 *      Get the filtering mode of the ip source guard in a port.
 *
 * INPUT:
 *      lport_ifindex   --  the logic port to get the the filtering mode
 *
 * OUTPUT:
 *      filter_mode_p     --  the filtering mode of the ip source guard shall be DHCPSNP_TYPE_IP_SRC_GRD_SIP_FILTER,
 *                          DHCPSNP_TYPE_IP_SRC_GRD_SIP_AND_MAC_FILTER or DHCPSNP_TYPE_IP_SRC_GRD_DISABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetIpSourceGuardMode(UI32_T lport_ifindex, UI8_T *filter_mode_p);

/* FUNCTION NAME : DHCPSNP_MGR_SetVerifyMacAddressStatus
 * PURPOSE:
 *      Enable/Disable verify MAC address funtion.
 *
 * INPUT:
 *      status  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1.Default value is enabled.
 *      2.In Global CLI: "ip dhcp snooping/verify/mac-address" or "no ip dhcp snooping/verify/mac-address"
 */
UI32_T DHCPSNP_MGR_SetVerifyMacAddressStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_MGR_GetVerifyMacAddressStatus
 * PURPOSE:
 *      Get the status of verify MAC address function.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.Default value is enabled.
 */
UI32_T DHCPSNP_MGR_GetVerifyMacAddressStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_SetPortTrustStatus
 * PURPOSE:
 *      Set the trsut status of the port.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be set the trust status.
 *      status          --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1.In per port CLI: "ip dhcp snooping/trust" or "no ip dhcp snooping/trust";
 */
UI32_T DHCPSNP_MGR_SetPortTrustStatus(UI32_T lport_ifindex, UI8_T status);

/* FUNCTION NAME : DHCPSNP_MGR_GetPortTrustStatus
 * PURPOSE:
 *      Get the trsut status of the port.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be get the trust status.
 *
 * OUTPUT:
 *      status_p        --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetPortTrustStatus(UI32_T lport_ifindex, UI8_T *status_p);

#if (SYS_CPNT_CFGDB == TRUE)
/* FUNCTION NAME : DHCPSNP_MGR_WriteBindingEntriesToFlash
 * PURPOSE:
 *      Write only dynamic dhcp snooping binding into the flash.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1. In Global CLI: ip dhcp snooping/database/flash
 *      2. There is only 1 entry type will be written to the flash: DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 */
UI32_T DHCPSNP_MGR_WriteBindingEntriesToFlash(void);

/* FUNCTION NAME : DHCPSNP_MGR_ReadBindingEntriesFromFlash
 * PURPOSE:
 *      Read only dynamic dhcp snooping binding entry from the flash to OM if the dhcp snooping 
 *      status is correct.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_ReadBindingEntriesFromFlash(void);

/* begin 2007-05, Joseph */
/* FUNCTION NAME : DHCPSNP_MGR_ClearBindingEntriesInFlash
 * PURPOSE:
 *      To clear DHCP snooping binding entries written in flash memory. 
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1. In Global CLI: clear ip dhcp snooping database flash
 *      2. There is only 1 entry type will be written to the flash: DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 *      3. Read the flash when provision complete.
 */
UI32_T DHCPSNP_MGR_ClearBindingEntriesInFlash(void);

/* end 2007-05 */
#endif

/* FUNCTION NAME : DHCPSNP_MGR_GetDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Get the dhcp snooping binding entry that the type is DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY;
 *
 * INPUT:
 *      binding_entry_p->vid_ifindex  --  the vlan ifindex
 *      binding_entry_p->mac_p        --  the MAC of this binding entry
 *
 * OUTPUT:
 *      binding_entry_p               --  the dynamic snooping binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_BINDING_ENTRY_NOT_EXISTED
 *
 * NOTES:
 *      1.There're 3 entry types
 *          a.DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY       
 *          b.DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY.  
 *        - ip soure guard binding entry includes type a,b.
 *        - dhcp snooping binding entry includes type a.
 */
UI32_T DHCPSNP_MGR_GetDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetNextDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Get the next dhcp snooping binding entry that the type is DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 *
 * INPUT:
 *      binding_entry_p->vid_ifindex  --  the vlan ifindex
 *      binding_entry_p->mac_p        --  the MAC of this binding entry
 *
 * OUTPUT:
 *      binding_entry_p   --  the next dhcp snooping binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping binding
 *      2.There're 3 entry types
 *          a.DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY        
 *          b.DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY.  
 *        - ip soure guard binding entry includes type a,b.
 *        - dhcp snooping binding entry includes type a.
 */
UI32_T DHCPSNP_MGR_GetNextDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetPortInfo
 * PURPOSE:
 *      Get the port information.
 *
 * INPUT:
 *      lport_ifindex    --  the logic port to get the port info.
 *
 * OUTPUT:
 *      port_info_p      --  the port information.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetPortInfo(UI32_T lport_ifindex, DHCPSNP_TYPE_PortInfo_T *port_info_p); 
 
/* FUNCTION NAME : DHCPSNP_MGR_GetNextPortInfo
 * PURPOSE:
 *      Get the next port information.
 *
 * INPUT:
 *      lport_ifindex_p    --  the logic port to get the next port info.
 *
 * OUTPUT:
 *      lport_ifindex    --  the next logic port.
 *      port_info_p      --  the next port information.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.This API will get all port information.
 */
UI32_T DHCPSNP_MGR_GetNextPortInfo(UI32_T *lport_ifindex_p, DHCPSNP_TYPE_PortInfo_T *port_info_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetTotalForwardedPktCounter
 * PURPOSE:
 *      Get the total forwarded packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      total_fwd_counter_p   --  the total forwarded packet counter
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.When the DHCP packets pass the filtering criteria, then we will increase the counter and forward it.
 */
/* 2007-09, Joseph, for ES3528M-SFP-FLF-38-00112 */
UI32_T DHCPSNP_MGR_GetTotalForwardedPktCounter(UI32_T *total_fwd_counter_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetUntrustedDroppedPktCounter
 * PURPOSE:
 *      Get the untrusted dropped packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      untrust_drp_counter_p   --  the dropped packet counter for the untrusted port
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.When the DHCP packets don't pass the filtering criteria, then we will increase the counter and drop it.
 *      2.The counter also equals to the number of the DHCP packets dropped by DHCP Snooping filtering criteria.
 */
/* 2007-09, Joseph, for ES3528M-SFP-FLF-38-00112 */
UI32_T DHCPSNP_MGR_GetUntrustedDroppedPktCounter(UI32_T *untrust_drp_counter_p);

/* FUNCTION NAME : DHCPSNP_MGR_SendDhcpClientPkt
 * PURPOSE:
 *      Send the DHCP client packet from the DUT when it's also a DHCP Client.
 *
 * INPUT:
 *      mem_ref_p         --  L_MREF descriptor.
 *      packet_length   --  the length of the packet.
 *      txRifNum        --  the tx rif number (always zero)
 *      dst_mac_p         --  the dstination MAC address
 *      src_mac_p         --  the source MAC address
 *      vid             --  the vlan id
 *      
 * OUTPUT:
 *      None
 *      
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.This API is called by DHCP_TXRX when DHCPSNP is supported.
 */
UI32_T DHCPSNP_MGR_SendDhcpClientPkt(L_MM_Mref_Handle_T    *mem_ref_p, 
                                     UI32_T      packet_length, 
                                     UI32_T      txRifNum, 
                                     UI8_T       *dst_mac_p, 
                                     UI8_T       *src_mac_p, 
                                     UI32_T      vid);



/* FUNCTION NAME : DHCPSNP_MGR_CheckBindingEntriesLeaseTime
 * PURPOSE:
 *      Check if the entry is expired every X seconds. 
 *
 * INPUT:
 *      None
 *      
 * OUTPUT:
 *      None
 *      
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.This API is called by DHCPSNP_TASK every X seconds.
 */
UI32_T DHCPSNP_MGR_CheckBindingEntriesLeaseTime(void);

/* FUNCTION NAME : DHCPSNP_MGR_GetRunningGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Get the global DHCP Snooping function status. And check is it the default value.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetRunningGlobalDhcpSnoopingStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetRunningDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the DHCP Snooping status by the vlan id. And check is it the default value.
 *
 * INPUT:
 *      vid     --  the vlan id.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetRunningDhcpSnoopingStatusByVlan(UI32_T vid, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetNextRunningDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the next user configured vlan id. And check is it the default value.
 *
 * INPUT:
 *      vid_p       --  current vlan id.
 *
 * OUTPUT:
 *      vid_p       --  the next vlan id configured by the user.
 *      status_p    --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      1. The DHCP Snooping of the vlan may be enabled/disabled. \
 *         This API only return the vlan id configured by user. 
 */
UI32_T DHCPSNP_MGR_GetNextRunningDhcpSnoopingStatusByVlan(UI32_T *vid_p, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetRunningVerifyMacAddressStatus
 * PURPOSE:
 *      Get the status of verify MAC address function. And check is it the default value.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetRunningVerifyMacAddressStatus(UI8_T *status_p);


/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetRunningPortFilterTableMode
 *------------------------------------------------------------------------------
 * PURPOSE: Get running port IPSG filter table mode
 * INPUT :  lport  -  logical port ifindex
 * OUTPUT:  mode_p -  port filter mode 
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetRunningPortFilterTableMode(UI32_T lport, UI8_T *mode_p);


/* begin 2006-07, Joseph */
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE)	
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationOptionStatus                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 status : enable/disable
 * INPUT    : status you want to set. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : 1. In Global CLI: "[no] ip dhcp snooping information option"
 *            2. default is disabled.                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationOptionStatus(UI8_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationOptionStatus                                
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current status. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : 1. In Global CLI: "show ip dhcp snooping"                                                            
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_MGR_GetInformationOptionStatus(UI8_T *status_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationPolicy                         
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 policy : drop/replace/keep
 * INPUT    : policy you want to set. 1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : 1. In Global CLI: "ip dhcp snooping information policy {drop|keep|replace}"
 *            2. default is replace.                                                               
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationPolicy(UI8_T policy);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationPolicy                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 Policy  : 
 * INPUT    : none
 * OUTPUT   : current option82 policy.1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : 1. In Global CLI: "show ip dhcp snooping"                                                                 
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_MGR_GetInformationPolicy(UI8_T *policy_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationRidMode                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id mode 
 * INPUT    : mode   -- DHCP_OPTION82_RID_MAC_HEX 
 *                      DHCP_OPTION82_RID_MAC_ASCII
 *                      DHCP_OPTION82_RID_IP_HEX
 *                      DHCP_OPTION82_RID_IP_ASCII
 *                      DHCP_OPTION82_RID_CONFIGURED_STRING
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationRidMode(UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationRidMode                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 remote id mode 
 * INPUT    : none
 * OUTPUT   : mode   -- DHCP_OPTION82_RID_MAC_HEX 
 *                      DHCP_OPTION82_RID_MAC_ASCII
 *                      DHCP_OPTION82_RID_IP_HEX
 *                      DHCP_OPTION82_RID_IP_ASCII
 *                      DHCP_OPTION82_RID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_GetInformationRidMode(UI8_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationRidValue                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value 
 * INPUT    : rid_value  -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationRidValue(UI8_T *rid_value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationRidValue                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value 
 * INPUT    : rid_value  -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_GetInformationRidValue(UI8_T *rid_value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationPortCidMode                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id mode : vlan-unit-port / configured string
 * INPUT    : lport_ifindex  -- port ifindex
 *            mode           -- DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT
 *                              DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationPortCidMode(UI32_T lport_ifindex,UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationPortCidMode                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id mode  : 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : current option82 circuit id mode. DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT / DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_MGR_GetInformationPortCidMode(UI32_T lport_ifindex, UI8_T *mode_p);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationPortCidValue                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id value 
 * INPUT    : lport_ifindex  -- port ifindex
 *            cid_value_p    -- configured string 
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : max number of characters of cid_value is 32                                                               
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationPortCidValue(UI32_T lport_ifindex,UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationPortCidValue                             
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id value 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : cid_value_p    -- configured string 
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_MGR_GetInformationPortCidValue(UI32_T lport_ifindex, UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetInformationFormat                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 subytpe format : TRUE / FALSE
 * INPUT    : subtype_format  -- use subtype format or not  (TRUE/FALSE)
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_SetInformationFormat(BOOL_T subtype_format);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetInformationFormat                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 subytpe format : TRUE / FALSE
 * INPUT    : subtype_format_p  
 * OUTPUT   : subtype_format_p  -- use subtype format or not  (TRUE/FALSE)
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_MGR_GetInformationFormat(BOOL_T *subtype_format_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_GetRunningInformationOptionStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- DHCPSNP_TYPE_OPTION82_ENABLED/DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     default value is (2)  DHCPSNP_TYPE_OPTION82_DISABLED
 *---------------------------------------------------------------------------*/
 UI32_T  DHCPSNP_MGR_GetRunningInformationOptionStatus(UI8_T *status_p);
 
/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_GetRunningInformationPolicy
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of drop/replace/keep mapping of system
 * INPUT:    None
 * OUTPUT:   status -- drop/replace/keep
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is (2) DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *---------------------------------------------------------------------------*/
 UI32_T  DHCPSNP_MGR_GetRunningInformationPolicy(UI8_T *policy_p);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_GetRunningInformationRidMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the mode of mac/ip address in remote id sub-option
 * INPUT:    None
 * OUTPUT:   mode -- DHCPSNP_TYPE_OPTION82_RID_MAC/DHCPSNP_TYPE_OPTION82_RID_IP
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is DHCPSNP_TYPE_OPTION82_RID_MAC
 *---------------------------------------------------------------------------*/
 UI32_T  DHCPSNP_MGR_GetRunningInformationRidMode(UI8_T *mode_p);

/* FUNCTION NAME : DHCPSNP_MGR_GetRunningInformationFormat
 * PURPOSE:
 *      Get the option82 subtype format. And check is it the default value.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      subtype_format      --  TRUE or FALSE
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_MGR_GetRunningInformationFormat(BOOL_T *subtype_format);

#endif
/* end 2006-07 */

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_SetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Set TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG/
 *          DHCPSNP_TYPE_FAIL/
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_SetBoardId(UI8_T board_id);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Set TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  board_id    --  board identifier
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG/
 *          DHCPSNP_TYPE_FAIL/
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetBoardId(UI8_T *board_id);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetRunningBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Get running config of TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  board_id    --  board identifier
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetRunningBoardId(UI8_T *board_id);
#endif

#if(SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_SetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE  : Set option82 delimiter
 * INPUT    : delimiter  --  option delimiter
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK/
 *            DHCPSNP_TYPE_FAIL
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCPSNP_MGR_SetOptionDelimiter(UI8_T delimiter);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_GetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE  : Get option82 delimiter
 * INPUT    : delimiter  --  option delimiter
 * OUTPUT   : delimiter  --  option delimiter
 * RETURN   : DHCPSNP_TYPE_OK/
 *            DHCPSNP_TYPE_INVALID_ARG/
 *            DHCPSNP_TYPE_FAIL
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCPSNP_MGR_GetOptionDelimiter(UI8_T *delimiter);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_GetRunningOptionDelimiter
 *---------------------------------------------------------------------------
 * PURPOSE:  Get running config of remote-id option delimiter
 * INPUT:    delimiter	--	option delimiter
 * OUTPUT:   delimiter 	--	option delimiter
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     if rate = 0, that means no rate limit.
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_MGR_GetRunningOptionDelimiter(UI8_T *delimiter);
#endif

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_IsMyPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will compare the client hardware address in mem_ref_p
 *           with unit base address.
 * INPUT:    mem_ref_p
 * OUTPUT:   none
 * RETURN:   TRUE -- client hardware address is the same as unit base address.
 *           FALSE -- else.
 *
 *---------------------------------------------------------------------------*/
BOOL_T DHCPSNP_MGR_IsMyPacket(L_MM_Mref_Handle_T *mem_ref_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_NotifyDhcpClient
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will notify DHCP client to a DHCP packet come.
 * INPUT:
 *      mem_ref_p       --  L_MREF descriptor.
 *      packet_length   --  the length of the packet.
 *      dst_mac_p       --  the dstination MAC address
 *      src_mac_p       --  the source MAC address
 *      vid             --  the vlan id
 *      src_lport_ifindex -- the source port index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 *---------------------------------------------------------------------------*/
UI32_T DHCPSNP_MGR_NotifyDhcpClient(L_MM_Mref_Handle_T *mem_ref_p, UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,
                                    UI32_T vid, UI32_T src_lport_ifindex);

#if ( SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT_CONFIGURABLE == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_SetPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Configure port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode_idx       -  filter table mode index
 *          limit          -  entry limit
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_SetPortBindingEntryLimit(UI32_T lport_ifindex, UI8_T mode_idx, UI32_T limit);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode           -  filter table mode
 * OUTPUT:  limit_p        -  port entry limit
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetPortBindingEntryLimit(UI32_T lport_ifindex, UI8_T mode, UI32_T *limit_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetRunningPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get the next per-port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode           -  filter table mode
 * OUTPUT:  limit_p        -  entry limit
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetRunningPortBindingEntryLimit(UI32_T  lport_ifindex, 
                                                   UI8_T   mode,
                                                   UI32_T *limit_p);

#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void DHCPSNP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void DHCPSNP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_AddFirstTrunkMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This is the callback function that will be registed to trunk manager
 * INPUT:    trunk_ifindex      
 *           member_ifindex
 * OUTPUT:   None
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_MGR_AddFirstTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_AddTrunkMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This is the callback function that will be registed to trunk manager
 * INPUT:    trunk_ifindex
 *           member_ifindex
 * OUTPUT:   None
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_MGR_AddTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_DelTrunkMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This is the callback function that will be registed to trunk manager
 * INPUT:    trunk_ifindex
 *           member_ifindex
 * OUTPUT:   None
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_MGR_DelTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_DelLastTrunkMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This is the callback function that will be registed to trunk manager
 * INPUT:    trunk_ifindex 
 *           member_ifindex
 * OUTPUT:   None
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_MGR_DelLastTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);


/* FUNCTION NAME : DHCPSNP_MGR_DeleteDynamicDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Delete dynamic DHCP snooping binding entry from the the binding table.
 *
 * INPUT :  binding_entry_p  --  binding entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_BINDING_TABLE_FULL
 *
 * NOTES:
 *      It can only delete dynamic learnt DHCP binding entry 
 */
UI32_T DHCPSNP_MGR_DeleteDynamicDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_MGR_DeleteAllDynamicDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Delete all dynamic DHCP snooping binding entry from the the binding table.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_BINDING_TABLE_FULL
 *
 * NOTES:
 *      It can only clear dynamic learnt DHCP binding entry 
 */
UI32_T DHCPSNP_MGR_DeleteAllDynamicDhcpSnoopingBindingEntry(void);

#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_SetGlobalRateLimit
 *------------------------------------------------------------------------------
 * PURPOSE: configure system-wise dhcp packet rate limit
 * INPUT :  rate	--	the limited rate
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  if rate = 0, that means no rate limit.
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_SetGlobalRateLimit(UI32_T rate);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_GetGlobalRateLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get system-wise dhcp packet rate limit
 * INPUT :  rate	--	the limited rate
 * OUTPUT:  rate	--	the limited rate
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  if rate = 0, that means no rate limit.
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_GetGlobalRateLimit(UI32_T *rate);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_MGR_GetRunningGlobalRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  Get running config of system-wise rate limit
 * INPUT:    rate	--	rate limit	
 * OUTPUT:   rate 	--	rate limit
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     if rate = 0, that means no rate limit.
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_MGR_GetRunningGlobalRateLimit(UI32_T *rate);
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_TxSnoopDhcpPacket
 *------------------------------------------------------------------------------
 * PURPOSE: Process Tx dhcp packet
 * INPUT  : mem_ref_p      --  L_MM_Mref descriptor
 *          dst_mac        --  destincation mac address
 *          src_mac        --  source mac address
 *          pkt_len        --  packet length(not including ethernet header)
 *          egr_vidifindex --  egress vlan id ifindex
 *          egr_lport       -- egress lport(if specified, only send to this port)
 *          ing_lport      --  ingress lport(0 means sent by DUT)
 * OUTPUT:  None
 * RETURN:  None
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
void DHCPSNP_MGR_TxSnoopDhcpPacket(
  L_MM_Mref_Handle_T *mem_ref_p,
  UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
  UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
  UI32_T              pkt_len,
  UI32_T              egr_vidifindex,
  UI32_T              egr_lport,
  UI32_T              ing_lport);
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_SetL2RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set L2 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  forwarding  -   needs forwarding or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_SetL2RelayForwarding(BOOL_T forwarding);
#endif  //SYS_CPNT_DHCP_RELAY_OPTION82

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_MGR_SetL3RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set L3 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  vid         -   vlan id
 *          forwarding  -   needs forward or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_MGR_SetL3RelayForwarding(UI32_T vid, BOOL_T forwarding);
#endif  //SYS_CPNT_DHCP_RELAY
#endif
