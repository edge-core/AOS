/* Project Name: New Feature
 * File_Name : netaccess_mgr.h
 * Purpose     : NETACCESS initiation and NETACCESS task creation
 *
 * 2006/01/27    : Ricky Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */

#ifndef NETACCESS_MGR_H
#define NETACCESS_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <string.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "security_backdoor.h"
#include "netaccess_om.h"
#include "1x_types.h"
#include "sys_callback_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_SUPPORT_ACCTON_BACKDOOR      (TRUE && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */
#define SYS_TYPE_FID_ALL 0

/* The key to get networkaccess mgr msgq.
 */
#define NETACCESS_MGR_IPCMSG_KEY    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY

#define NETACCESS_MGR_IPCMsg_AuthenticatePacket_T SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T

/* The commands for IPC message.
 */
enum {
    NETACCESS_MGR_IPC_CMD_CLR_SEC_ADR_ENT_BY_FTR,
    NETACCESS_MGR_IPC_CMD_GET_MAC_ADR_AGING_MODE,
    NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_ENT,
    NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_INTR_ACTION,
    NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_MAX_MAC_CNT,
    NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PORT_INTR_ACTN,
    NETACCESS_MGR_IPC_CMD_GET_NXT_MAC_AUTH_PORT_ENT,
    NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT,
    NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT_BY_FTR,

    NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC,
    NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC_BY_FTR_ID,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_MAC,
    NETACCESS_MGR_IPC_CMD_SET_FTR_MAC,
    NETACCESS_MGR_IPC_CMD_GET_FTR_ID_ON_PORT,
    NETACCESS_MGR_IPC_CMD_SET_FTR_ID_TO_PORT,

    NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_PORT_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_PORT_INTR_ACTN,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_ADR_AGING_MODE,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_INTR_ACTN,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_MAX_MAC,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_STAS,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_DYN_VLAN_STAS,

    NETACCESS_MGR_IPC_CMD_SET_DYN_QOS_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_DYN_QOS_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_DYN_QOS_STATUS,

    NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_GUEST_VLAN_ID,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_NBR_ADR,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_REAUTH_TIME,
    NETACCESS_MGR_IPC_CMD_GET_SEC_ADR_ENT,
    NETACCESS_MGR_IPC_CMD_GET_SEC_AUTH_AGE,
    NETACCESS_MGR_IPC_CMD_GET_SEC_DYN_VLAN_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_ID_ON_PORT,
    NETACCESS_MGR_IPC_CMD_GET_FTR_MAC,
    NETACCESS_MGR_IPC_CMD_GET_SEC_GUEST_VLAN_ID,
    NETACCESS_MGR_IPC_CMD_GET_SEC_NBR_ADR,
    NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_SEC_REAUTH_TIME,

    NETACCESS_MGR_IPC_CMD_SET_MAC_ADR_AGING_MODE,
    NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_INTR_ACTION,
    NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_MAX_MAC_CNT,
    NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_STATUS,
    NETACCESS_MGR_IPC_CMD_SET_SEC_AUTH_AGE,
    NETACCESS_MGR_IPC_CMD_SET_SEC_DYN_VLAN_STATUS,
    NETACCESS_MGR_IPC_CMD_SET_SEC_GUEST_VLAN_ID,
    NETACCESS_MGR_IPC_CMD_SET_SEC_NBR_ADR,
    NETACCESS_MGR_IPC_CMD_SET_SEC_REAUTH_TIME,

    /* begin for ieee_8021x.c/cli_api_1x.c
     */
    NETACCESS_MGR_IPC_CMD_DO_1X_REAUTHEN,
    NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_CFG_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_STS_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_DIAG_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_1X_PAE_PORT_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SEVR_TOUT,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SUPP_TOUT,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTHORIZED,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_CTRL_MODE,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_DETAILS,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_INTR_ACTN,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MAX_REQ,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_OPER_MODE,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_QUIET_PERIOD,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_ENABLED,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_MAX,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_PERIOD,
    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_TX_PERIOD,
    NETACCESS_MGR_IPC_CMD_GET_1X_SESS_STS_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_1X_SYS_AUTH_CTRL,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_CFG_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_DIAG_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_STS_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PAE_PORT_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_NXT_1X_SESS_STS_ENTRY,
    NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_MODE,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_ADMIN_CTRL_DIR,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SEVR_TOUT,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SUPP_TOUT,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_TX_ENABLED,
    NETACCESS_MGR_IPC_CMD_SET_1X_CFG_SETTING_TO_DFLT,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_CTRL_MODE,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_OPER_MODE,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_INTR_ACTN,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MAX_REQ,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MULHOST_MAC_CNT,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_PAE_PORT_INIT,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_QUIET_PERIOD,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_ENABLED,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_PERIOD,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_TX_PERIOD,
    NETACCESS_MGR_IPC_CMD_SET_1X_SYS_AUTH_CTRL,
    NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_MAX,
    NETACCESS_MGR_IPC_CMD_SET_1X_EAPOL_PASS_TRHOU,
    NETACCESS_MGR_IPC_CMD_GET_1X_EAPOL_PASS_TRHOU,
    NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_EAPOL_PASS_TRHOU,

    /* begin for cli_api_ethernet.c [port security]
     */
    NETACCESS_MGR_IPC_CMD_SET_PSEC_ACTN_STAS,
    NETACCESS_MGR_IPC_CMD_SET_PSEC_MAX_MAC_CNT,
    NETACCESS_MGR_IPC_CMD_SET_PSEC_STAS,
    NETACCESS_MGR_IPC_CMD_CONVERT_PSEC_SECURED_ADDRESS_INTO_MANUAL,

    NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MULHOST_MAC_CNT,
    NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_OPER_MODE,
    NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_MULHOST_MAC_CNT,

    NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_STATUS,
    NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_STATUS,
    NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_MODE,
    NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_MODE,
    NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_MODE,
    NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_ACTION,
    NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_ACTION,
    NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_ACTION,

    /* begin for asynchronous call
     */
    NETACCESS_MGR_IPC_CMD_ASYNC_CALL,
    NETACCESS_MGR_IPC_CMD_AUTHENTICATE_PACKET,
};


/* MACRO FUNCTION DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - NETACCESS_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of NETACCESS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of NETACCESS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NETACCESS_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((NETACCESS_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of NETACCESS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of NETACCESS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(NETACCESS_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - NETACCESS_MGR_MSG_CMD
 *              NETACCESS_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the NETACCESS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The NETACCESS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NETACCESS_MGR_MSG_CMD(msg_p)    (((NETACCESS_MGR_IPCMsg_T *)msg_p->msg_buf)->type.cmd)
#define NETACCESS_MGR_MSG_RETVAL(msg_p) (((NETACCESS_MGR_IPCMsg_T *)msg_p->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - NETACCESS_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NETACCESS_MGR_MSG_DATA(msg_p)   ((void *)&((NETACCESS_MGR_IPCMsg_T *)msg_p->msg_buf)->data)


/* DATA TYPE DECLARATIONS
 */
typedef struct NETACCESS_MGR_SecurePortEntry_S
{
    UI32_T  lport;
    UI32_T  port_mode;

    UI32_T  intrusion_action;
    UI32_T  number_addresses;
    UI32_T  number_addresses_stored;
    UI32_T  maximum_addresses;
    UI32_T  nbr_of_authorized_addresses;
    /* unauthorized mac = number_addresses_stored - nbr_of_authorized_addresses */
    UI32_T nbr_of_learn_authorized_addresses;
    UI32_T  filter_id;                   /* filter MAC table id which bind to this port */

    UI32_T  guest_vlan_id;               /* guest VLAN ID */

    UI32_T  link_detection_status;
    UI32_T  link_detection_mode;
    UI32_T  link_detection_action;

    UI32_T  dynamic_vlan_status;         /* dynamic VLAN status */
    UI32_T  dynamic_qos_status;          /* dynamic qos status */
} NETACCESS_MGR_SecurePortEntry_T;

typedef struct NETACCESS_MGR_MacAuthPortEntry_E
{
    UI32_T  lport;                       /* key, logical port nbr */
    UI32_T  intrusion_action;            /* action for unauthorized MAC */
    UI32_T  configured_number_addresses; /* allowed MAC nbr */
}NETACCESS_MGR_MacAuthPortEntry_T;

typedef enum NETACCESS_FidSecurePortEntry_E
{
    NETACCESS_FID_SECURE_PORT_ENTRY_IFINDEX = 1,
    NETACCESS_FID_SECURE_PORT_ENTRY_PORT_MODE,
    NETACCESS_FID_SECURE_PORT_ENTRY_INTRUSION_ACTION,
    NETACCESS_FID_SECURE_PORT_ENTRY_NUMBER_ADDRESSES,
    NETACCESS_FID_SECURE_PORT_ENTRY_NUMBER_ADDRESSES_STORED,
    NETACCESS_FID_SECURE_PORT_ENTRY_MAXIMUM_ADDRESSES,
    NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_AUTHORIZED_ADDRESSES,
    NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_LEARN_AUTHORIZED_ADDRESSES,
    NETACCESS_FID_SECURE_PORT_ENTRY_FILTER_ID,
    NETACCESS_FID_SECURE_PORT_ENTRY_DYNAMIC_VLAN_STATUS,
    NETACCESS_FID_SECURE_PORT_ENTRY_DYNAMIC_QOS_STATUS,
}NETACCESS_FidSecurePortEntry_T;

typedef enum NETACCESS_MGR_FidMacAuthPortEntry_E
{
    NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_IFINDEX = 1,
    NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_STATUS,
    NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_NUMBER_ADDRESSES,
    NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_INTRUSION_ACTION,
}NETACCESS_MGR_FidMacAuthPortEntry_T;

typedef struct NETACCESS_MGR_SecureAddressEntry_S
{
    UI32_T  addr_lport;
    UI32_T  addr_row_status;
    UI32_T  server_ip;
    UI32_T  record_time;
    BOOL_T  is_learnt;
    UI8_T   addr_MAC[SYS_ADPT_MAC_ADDR_LEN];
} NETACCESS_MGR_SecureAddressEntry_T;

typedef struct NETACCESS_MGR_SecureAddressEntryKey_S
{
    UI32_T  lport;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
} NETACCESS_MGR_SecureAddressEntryKey_T;

typedef enum NETACCESS_FidSecureAddressEntry_E
{
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_IFINDEX = 1,
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_ROW_STATUS,
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_SERVER_IP,
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_RECORD_TIME,
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_IS_LEARNT,
    NETACCESS_FID_SECURE_ADDRESS_ENTRY_MAC,
}NETACCESS_FidSecureAddressEntry_T;

typedef enum NETACCESS_MGR_FunctionFailure_E
{
    NETACCESS_FUNCTION_SUCCEEDED = 0,

    NETACCESS_COZ_NOT_NORMAL_PORT,                  /* port type != SWCTRL_LPORT_NORMAL_PORT */
    NETACCESS_COZ_INVLID_MAC_ADDRESS,               /* e.g. broadcast, multicast mac */
    NETACCESS_COZ_IS_TRUNK_PORT,
    NETACCESS_COZ_LACP_ENABLED,
    NETACCESS_COZ_NOT_ALLOW_CREATE_MAC_ADDRESS,     /* don't allow create mac in current port mode */
    NETACCESS_COZ_MAC_ALREADY_EXIST,                /* mac existed, please check the lport parameter */
    NETACCESS_COZ_NO_MORE_SPACE_TO_CREATE,          /* system capacity is full */

    NETACCESS_COZ_INTERNAL_ERROR,                   /* program internal error */

    NETACCESS_COZ_SUPPLICANT_ENABLED,
    NETACCESS_COZ_RSPAN_ENABLED,
    NETACCESS_COZ_PSEC_ENABLED,

    NETACCESS_COZ_UNKNOWN_REASON,
} NETACCESS_MGR_FunctionFailure_T;

typedef enum NETACCESS_MGR_Dot1XPortAuthorizedStatus_E
{
    NETACCESS_MGR_DOT1X_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED = DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED,
    NETACCESS_MGR_DOT1X_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED = DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED,
    NETACCESS_MGR_DOT1X_AUTH_CONTROLLED_PORT_STATUS_ERR = DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_ERR,
}NETACCESS_MGR_Dot1XAuthControlledPortStatus_T;

typedef struct NETACCESS_MGR_FailureReason_S
{
    NETACCESS_MGR_FunctionFailure_T reason;
    UI32_T                          lport;      /* reason caused by which port */
} NETACCESS_MGR_FailureReason_T;

typedef enum NETACCESS_MGR_AddressEntryType_E
{
    NETACCESS_ADDRESS_ENTRY_TYPE_ALL     = 1L,
    NETACCESS_ADDRESS_ENTRY_TYPE_STATIC,
    NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC,
} NETACCESS_MGR_AddressEntryType_T;

typedef enum NETACCESS_MGR_AddressEntrySort_E
{
    NETACCESS_ADDRESS_ENTRY_SORT_ADDRESS     = 1L,
    NETACCESS_ADDRESS_ENTRY_SORT_INTERFACE,
} NETACCESS_MGR_AddressEntrySort_T;

typedef struct NETACCESS_MGR_SecureAddressFilter_S
{
    UI32_T lport; /* not filter condition,fill zero */
    NETACCESS_MGR_AddressEntryType_T type; /* 1:all,2:static,3:dynamic */
    NETACCESS_MGR_AddressEntrySort_T sort; /* 1:address,2:port */
    UI8_T mac[SYS_ADPT_MAC_ADDR_LEN]; /* not filter condition,fill zero */
}NETACCESS_MGR_SecureAddressFilter_T;

typedef struct NETACCESS_MGR_SecureAddressEntryKeyFilter_S
{
    UI32_T  lport;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
    NETACCESS_MGR_SecureAddressFilter_T filter;
} NETACCESS_MGR_SecureAddressEntryKeyFilter_T;


/* Message declarations for IPC.
 */
typedef struct
{
    UI32_T  ifindex;
} NETACCESS_MGR_IPCMsg_Interface_T;

typedef struct
{
    UI32_T  data;
} NETACCESS_MGR_IPCMsg_U32Data_T;

typedef struct
{
    UI32_T  lport;
    UI32_T  data;
} NETACCESS_MGR_IPCMsg_LportData_T;

typedef struct
{
    UI32_T  lport;
    BOOL_T  bdata;
} NETACCESS_MGR_IPCMsg_LportBdata_T;

typedef struct
{
    UI32_T lport;
    NETACCESS_MGR_Dot1XAuthControlledPortStatus_T port_control;
} NETACCESS_MGR_IPCMsg_LportControlPort_T;

typedef struct
{
    UI32_T  filter_id;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   mask[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T  is_add;
} NETACCESS_MGR_IPCMsg_FidMacAct_T;

typedef struct
{
    UI32_T  filter_id;
    UI32_T  lport;
} NETACCESS_MGR_IPCMsg_FidPortAct_T;

typedef struct
{
    I32_T   mref_handle_offset;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  tag_info;
    UI16_T  type;
    UI32_T  pkt_length;
    UI32_T  src_unit;
    UI32_T  src_port;
    UI32_T  packet_class;
} NETACCESS_MGR_IPCMsg_AuthenticastePacket_T;

typedef struct
{
    UI32_T                                      field_id;
    UI32_T                                      buf_size;
    UI32_T                                      used_buf;

    union
    {
        NETACCESS_MGR_SecureAddressEntryKeyFilter_T key_ftr;
        NETACCESS_MGR_SecureAddressEntryKey_T       key;
    };

    NETACCESS_MGR_SecureAddressEntry_T          entry;
} NETACCESS_MGR_IPCMsg_SecAdrEnt_T;

typedef struct
{
    UI32_T                              field_id;
    UI32_T                              buf_size;
    UI32_T                              used_buf;
    UI32_T                              entry_key; /* lport */
    NETACCESS_OM_SecurePortEntry_T      prt_entry;
} NETACCESS_MGR_IPCMsg_SecPrtEnt_T;

typedef struct
{
    UI32_T                              field_id;
    UI32_T                              buf_size;
    UI32_T                              used_buf;
    UI32_T                              entry_key; /* lport */
    NETACCESS_MGR_MacAuthPortEntry_T    entry;
} NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T;

typedef struct
{
    UI32_T  filter_id;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   mask[SYS_ADPT_MAC_ADDR_LEN];
} NETACCESS_MGR_IPCMsg_FidMac_T;

typedef struct
{
    UI32_T                                  lport;
    DOT1X_AuthConfigEntry_T                 xacpe;
} NETACCESS_MGR_IPCMsg_LportXacpe_T;

typedef struct
{
    UI32_T                                  lport;
    DOT1X_AuthStatsEntry_T                  xaspe;
} NETACCESS_MGR_IPCMsg_LportXaspe_T;

typedef struct
{
    UI32_T                                  lport;
    DOT1X_AuthDiagEntry_T                   xadpe;
} NETACCESS_MGR_IPCMsg_LportXadpe_T;

typedef struct
{
    UI32_T                                  lport;
    DOT1X_PaePortEntry_T                    xpape;
} NETACCESS_MGR_IPCMsg_LportXpape_T;

typedef struct
{
    UI32_T                                  lport;
    DOT1X_AuthSessionStatsEntry_T           xasse;
} NETACCESS_MGR_IPCMsg_LportXasse_T;

typedef struct
{
    UI32_T                  lport;
    DOT1X_PortDetails_T     xpdtl;
} NETACCESS_MGR_IPCMsg_LportXpdtl_T;

typedef struct
{
    UI32_T                      lport;
    NETACCESS_PortMode_T        nacpm;
} NETACCESS_MGR_IPCMsg_LportNapm_T;

typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} NETACCESS_MGR_IPCMsg_Type_T;

typedef union
{
    NETACCESS_MGR_IPCMsg_Interface_T        interface_data;
    NETACCESS_MGR_IPCMsg_U32Data_T          ui32d;
    NETACCESS_MGR_IPCMsg_LportData_T        ldata;
    NETACCESS_MGR_IPCMsg_FidMacAct_T        fmact;
    NETACCESS_MGR_IPCMsg_FidPortAct_T       fpact;
    NETACCESS_MGR_IPCMsg_LportBdata_T       lbdat;
    NETACCESS_MGR_IPCMsg_SecAdrEnt_T        saent;
    NETACCESS_MGR_IPCMsg_SecPrtEnt_T        spent;
    NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T    mapen;
    NETACCESS_MGR_IPCMsg_FidMac_T           ftmac;
    NETACCESS_MGR_SecureAddressFilter_T     nmsaf;

    NETACCESS_MGR_IPCMsg_LportXacpe_T       lxacp;
    NETACCESS_MGR_IPCMsg_LportXaspe_T       lxasp;
    NETACCESS_MGR_IPCMsg_LportXadpe_T       lxadp;
    NETACCESS_MGR_IPCMsg_LportXpape_T       lxpap;
    NETACCESS_MGR_IPCMsg_LportXasse_T       lxass;
    NETACCESS_MGR_IPCMsg_LportXpdtl_T       lxdtl;
    NETACCESS_MGR_IPCMsg_LportNapm_T        lnapm;
    NETACCESS_MGR_IPCMsg_AuthenticatePacket_T  auth_pkt;
} NETACCESS_MGR_IPCMsg_Data_T;

typedef struct
{
    NETACCESS_MGR_IPCMsg_Type_T type;
    NETACCESS_MGR_IPCMsg_Data_T data;
} NETACCESS_MGR_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_InitiateSystemResources
 *---------------------------------------------------------------------------
 * Function : Initialize NETACCESS's MGR .
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_InitiateSystemResources(void);

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * Function : This function initializes all function pointer registration operations.
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterSlaveMode();

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterTransitionMode Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterTransitionMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetCurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of NETACCESS's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T NETACCESS_MGR_GetCurrentOperationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_ProcessNewMacMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from new mac message queue
 * INPUT:  new_mac_msg_p.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ProcessNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_ProcessTimeoutEvent
 * ---------------------------------------------------------------------
 * PURPOSE : Process timeout event
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ProcessTimeoutEvent(void);

/*************************
 * For UI (CLI/WEB/SNMP) *
 *************************
 */
/* port mode part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set security modes of the port
 * INPUT:  lport,secure_port_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get security modes of the port
 * INPUT:  lport
 * OUTPUT: secure_port_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_port_mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT:  secure_port_mode
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode);

/* learn count part
 */
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by lport.
 * INPUT    : lport : logical port.
 *            number:secureNumberAddresses
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureNumberAddresses(UI32_T lport,UI32_T number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get secureNumberAddresses by lport.
 * INPUT    : lport : logical port.
 * OUTPUT   : number:secureNumberAddresses.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureNumberAddresses(UI32_T lport, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default number is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT:  number:secureNumberAddresses
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureNumberAddresses(UI32_T lport, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureNumberAddressesStored
 * ---------------------------------------------------------------------
 * PURPOSE: The number of addresses that are currently in the
 *          AddressTable for this port. If this object has the same value as
 *          secureNumberAddresses, then no more addresses can be authorised on this
 *          port.
 * INPUT:  lport,secure_number_addresses_stored.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureNumberAddressesStored(UI32_T lport, UI32_T *secure_number_addresses_stored);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureMaximumAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maxinum value that secureNumberAddresses
 *          can be set to.
 * INPUT:  lport
 * OUTPUT: secure_maximum_addresses
 * RETURN: TRUE/FALSE.
 * NOTES:
 * This indicates the maximum value that secureNumberAddresses
 * can be set to. It is dependent on the resources available so may change,
 * eg. if resources are shared between ports, then this value can both
 * increase and decrease. This object must be read before setting
 * secureNumberAddresses.
 *
 * The following relationship must allows be preserved.
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureMaximumAddresses(UI32_T lport, UI32_T *secure_maximum_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the maximum value that secureNumberAddresses can be set to.
 * INPUT    : lport
 * OUTPUT   : usable_addresses
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr);

/* Timer part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT:  reauth_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureReauthTime(UI32_T reauth_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT:  None.
 * OUTPUT: reauth_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureReauthTime(UI32_T *reauth_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default reauth_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  reauth_time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureReauthTime(UI32_T *reauth_time);

/* dynamic VLAN part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT:  lport,
 *         dynamic_vlan_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureDynamicVlanStatus(UI32_T lport, UI32_T dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_vlan_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureDynamicVlanStatus(UI32_T lport, UI32_T *dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT: dynamic_vlan_status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureDynamicVlanStatus(UI32_T lport, UI32_T *status);

/* dynamic QoS part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic QoS configuration control
 * INPUT:  lport,
 *         dynamic_qos_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDynamicQosStatus(UI32_T lport, UI32_T dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_qos_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDynamicQosStatus(UI32_T lport, UI32_T *dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_qos_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT: dynamic_qos_status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDynamicQosStatus(UI32_T lport, UI32_T *status);

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Set secure filter table
 * INPUT:  filter_id,mac_address,mask,is_add
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetFilterMac(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask,BOOL_T is_add);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get secure filter table entry
 * INPUT:  filter_id,mac_address, mac_mask
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  TURE means entry exist,FALSE to not exist
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetFilterMac(UI32_T filter_id, UI8_T *mac_address, UI8_T *mac_mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: filter_id,mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextFilterMac(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextFilterMacByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry by filter id
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextFilterMacByFilterId(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next running secure filter table entry
 * INPUT:   filter_id
 * OUTPUT:  mac_address
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default value.
 *        3. Caller has to prepare buffer for storing return value
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningFilterMac(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_SetFilterIdToPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : bind secure filter id to port
 * INPUT    : lport,filter_id
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetFilterIdToPort(UI32_T lport,UI32_T filter_id);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get secure filter id which bind to port
 * INPUT    : lport
 * OUTPUT   : filter_id.
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetFilterIdOnPort(UI32_T lport,UI32_T *filter_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningFilterIdOnPort
 * ---------------------------------------------------------------------
 * PURPOSE: Get running secure filter id which bind to port
 * INPUT:   lport
 * OUTPUT:  filter_id
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default value.
 *        3. Caller has to prepare buffer for storing return value
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningFilterIdOnPort(UI32_T lport,UI32_T *filter_id);
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

/* clear secure MAC part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_ClearSecureAddressEntryByFilter
 * ---------------------------------------------------------------------
 * PURPOSE: clear secure mac address table entry by filter
 * INPUT:  in_filter.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ClearSecureAddressEntryByFilter(NETACCESS_MGR_SecureAddressFilter_T *in_filter);

/* port table part
 */
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetSecurePortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecurePortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/* mac table part
 */
#if 0
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT    : lport : logical port.
 *            mac_address  : secureAddrMAC.
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureAddressEntry(UI32_T lport, UI8_T *mac_address, NETACCESS_MGR_SecureAddressEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the unit and the port and the mac_address.
 * INPUT    : lport : logical port.
 *            mac_address  : secureAddrMAC.
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextSecureAddressEntry(UI32_T *lport, UI8_T *mac_address, NETACCESS_MGR_SecureAddressEntry_T *entry);
#endif
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetSecureAddressEntry(UI32_T field_id, NETACCESS_MGR_SecureAddressEntryKey_T *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the lport and the mac_address.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *            3.key must be NETACCESS_MGR_SecureAddressEntryKey_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecureAddressEntry(UI32_T field_id, NETACCESS_MGR_SecureAddressEntryKey_T *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextRunningSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next running secure address entry by the unit and the port and the mac_address.
 * INPUT    : lport, mac_address
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextRunningSecureAddressEntry(UI32_T *lport, UI8_T *mac_address, NETACCESS_MGR_SecureAddressEntry_T *entry);
#if 0
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntryByFilter
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next secure address entry by the filter.
 * INPUT    : in_filter
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextSecureAddressEntryByFilter(NETACCESS_MGR_SecureAddressFilter_T *in_filter, NETACCESS_MGR_SecureAddressEntry_T *entry);
#endif
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntryByFilter
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by filter.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *            3.key must be NETACCESS_MGR_SecureAddressEntryKeyFilter_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecureAddressEntryByFilter(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKeyFilter_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer);

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDebugFlag(UI32_T debug_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_ShowStateMachineStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : show state machine's status
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_ShowStateMachineStatus(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_ShowSecureAddressByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : show secure address by mac_index
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_ShowSecureAddressByIndex(UI32_T mac_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_ShowOtherDebugData
 *-------------------------------------------------------------------------
 * PURPOSE  : show other debug data
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_ShowOtherDebugData();

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_HandleHotInsertion
 *-----------------------------------------------------------------------------------
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
 *-----------------------------------------------------------------------------------
 */
void NETACCESS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_HandleHotRemoval
 *-----------------------------------------------------------------------------------
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
 *-----------------------------------------------------------------------------------
 */
void NETACCESS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 *            control_mode -- control mode
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortControlMode(UI32_T lport,UI32_T control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : control_mode -- control mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortControlMode(UI32_T lport,UI32_T *control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : control_mode -- control mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortControlMode(UI32_T lport,UI32_T *control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next port control mode of 1X configuration
 * INPUT    : lport -- logic port number, use 0 to get first
 * OUTPUT   : control_mode -- control mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortControlMode(UI32_T *lport,UI32_T *control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 *            mode -- operation mode
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortOperationMode(UI32_T lport,UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : mode -- operation mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortOperationMode(UI32_T lport,UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : mode -- operation mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortOperationMode(UI32_T lport,UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next port operation mode of 1X configuration
 * INPUT    : lport -- logic port number, use 0 to get first
 * OUTPUT   : mode -- operation mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortOperationMode(UI32_T *lport,UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : set the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 *            count -- max mac count for multi-host mode
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortMultiHostMacCount(UI32_T lport,UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortMultiHostMacCount(UI32_T lport,UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortMultiHostMacCount(UI32_T *lport,UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE  : set system auth control of 1X configuration
 * INPUT    : control_status
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_status (define in leaf_Ieee8021x.h):
 *         VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *         VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xSystemAuthControl(UI32_T control_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
 *          Control in a system.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : SystemAuthControl
 * NOTES  : VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xSystemAuthControl(UI32_T *control_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemAuthControl is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : None.
 * OUTPUT : SystemAuthControl
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xSystemAuthControl(UI32_T *control_status);

#if (SYS_CPNT_PORT_SECURITY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityStatus
 *-------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : lport                  - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE      : Port security doesn't support
 *           1) unknown port, 2) trunk member, and 3) trunk port
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityStatus(UI32_T lport, UI32_T portsec_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityMaxMacCount
 *-------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : lport          -- which port to
 *           max_mac_count  -- maximum mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Default : max_mac_count = 1
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityMaxMacCount(UI32_T lport, UI32_T max_mac_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityActionStatus
 *-------------------------------------------------------------------------
 * FUNCTION: This function will set port security action status
 * INPUT   : lport          -- which port to
 *           action_status  -- action status
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : if the port is not in portSecurity port mode,will return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityActionStatus (UI32_T lport, UI32_T action_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_ConvertPortSecuritySecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
NETACCESS_MGR_ConvertPortSecuritySecuredAddressIntoManual(
    UI32_T ifindex
);
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_SecureEntryDeleteFromAmtr_Callback
 *---------------------------------------------------------------------------
 * Function : This function will be notified when amtr delete a secure entry
 *            and only update vlan counter or delete mac address entry in database.
 * Input    : ifindex -- ifindex of deleted entry
 *            mac -- MAC address of deleted entry
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_SecureEntryDeleteFromAmtr_Callback(UI32_T ifindex, UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port MaxReq of 1X configuration
 * INPUT    : lport,times
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortMaxReq(UI32_T lport,UI32_T times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortMaxReq(UI32_T lport,UI32_T *times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortMaxReq(UI32_T lport,UI32_T *times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortMaxReq(UI32_T *lport,UI32_T *times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE  : enable/disable port period re-authentication of the 1X client,
 *            which is disabled by default
 * INPUT    : lport,control
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthEnabled(UI32_T lport ,UI32_T control);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Get dot1x period re-authentication status of the specified port
 * INPUT   : lport
 * OUTPUT  : value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthEnabled(UI32_T lport, UI32_T *value_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetRunningDot1xPortReAuthEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get port re-authentication status
 * INPUT   : lport
 * OUTPUT  : value_p
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortReAuthEnabled(UI32_T lport, UI32_T *value_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetNextDot1xPortReAuthEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : value_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_DoDot1xReAuthenticate
 *-------------------------------------------------------------------------
 * PURPOSE  : use the command to manually initiate a re-authentication of
 *            the specified 1X enabled port
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_DoDot1xReAuthenticate(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xPortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE  : set port QuietPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortQuietPeriod(UI32_T lport, UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortQuietPeriod(UI32_T *lport, UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : set port ReAuthPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthPeriod(UI32_T lport,UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortReAuthPeriod(UI32_T *lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port TxPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortTxPeriod(UI32_T lport,UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortTxPeriod(UI32_T *lport,UI32_T *seconds);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_SetDot1xPortPaePortInitialize
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of dot1xPaePortReauthenticate
 * INPUT   : lport - port number
 *           value - VAL_dot1xPaePortInitialize_true or
 *                   VAL_dot1xPaePortInitialize_false
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTES   : According to MIB description, setting this attribute FALSE
 *           has no effect. This attribute always returns FALSE when it
 *           is read.
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortPaePortInitialize(UI32_T lport, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport,direction.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
 *                        VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T dir);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : direction.
 * NOTES  : direction =  VAL_dot1xAuthAdminControlledDirections_both for Both
 *                       VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T *dir);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport,timeout.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T *timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: seconds.
 * NOTES:
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T *timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Set the port's re-auth max
 * INPUT:  lport, reauth_max
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthMax(UI32_T lport, UI32_T reauth_max);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthMax(UI32_T lport, UI32_T *reauth_max);

/*-------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_MGR_SetDot1xPortAuthTxEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Set the value of the keyTransmissionEnabled constant currently
 *           in use by the Authenticator PAE state machine.
 * INPUT   : lport - port number
 *           value - VAL_dot1xAuthKeyTxEnabled_true or
 *                   VAL_dot1xAuthKeyTxEnabled_false
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE.
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T value);

/* ------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_MGR_GetDot1xPortAuthTxEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get the value of the keyTransmissionEnabled constant currently
 *           in use by the Authenticator PAE state machine.
 * INPUT   : lport - port number
 * OUTPUT  : value_p - VAL_dot1xAuthKeyTxEnabled_true or
 *                     VAL_dot1xAuthKeyTxEnabled_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T *value_p);

/* for 802.1x MIB (IEEE8021-PAE-MIB)
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xPaePortTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthConfigTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthStatsTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthDiagTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xSessionStatsEntry(UI32_T lport, DOT1X_AuthSessionStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthSessionStatsTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xSessionStatsEntry(UI32_T *lport_p, DOT1X_AuthSessionStatsEntry_T *entry_p);

/*****************For CLI/CGI Show do1x **********************************/
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortAuthorized
 *-------------------------------------------------------------------------
 * PURPOSE:  Get dot1x port authorized status
 * INPUT  :  lport  - logic port number
 * OUTPUT :  status_p  - dot1x port authorized status
 * RETURN :  TRUE - succeeded / FALSE - failed
 * NOTE   :  None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortAuthorized(UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *status_p);

#if NotSupport
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xGlobalParameters
 *-------------------------------------------------------------------------
 * PURPOSE:  Get dot1x global parameters.
 * INPUT  :  GlobalParameters pointer.
 * OUTPUT :  None.
 * RETURN :  None.
 * NOTE   :  None.
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_GetDot1xGlobalParameters(NETACCESS_DOT1xGlobalParameters_T * GlobalParameters);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetDot1xPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortDetails(UI32_T lport, DOT1X_PortDetails_T *port_details_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the mac-authentication status for the specified port.
 * INPUT    : lport -- logic port number.
 *            mac_auth_status -- mac-authentication status.
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortStatus(UI32_T lport, UI32_T mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the mac-authentication status for the specified port.
 * INPUT    : lport -- logic port number.
 * OUTPUT   : mac_auth_status -- mac-authentication status.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the mac-authentication status for the specified port.
 * INPUT    : lport.
 * OUTPUT   : mac_auth_status -- mac-authentication status.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port status of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : mac_auth_status -- mac-authentication status
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortStatus(UI32_T *lport,UI32_T *mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 *            count -- max mac count
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortMaxMacCount(UI32_T lport,UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port max allowed MAC number of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortMaxMacCount(UI32_T *lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the intrustion action of mac-authentication for the specified port.
 * INPUT    : lport  -- logic port number
 *            action -- intrusion action
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port intrusion action of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortIntrusionAction(UI32_T *lport, UI32_T *action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the port entry of mac-authentication.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : FALSE : error, TRUE : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextMacAuthPortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get the next port entry of mac-authentication.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : FALSE : error, TRUE : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextMacAuthPortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureNumberAddressesStoredByRadius
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by RADIUS
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetSecureNumberAddressesStoredByRadius(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureNumberAddressesStoredByRada
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by RADA
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetSecureNumberAddressesStoredByRada(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureNumberAddressesStoredByDot1x
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by Dot1x
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetSecureNumberAddressesStoredByDot1x(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port guest VLAN ID.
 * INPUT  : lport,vid.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureGuestVlanId(UI32_T lport, UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get per-port guest VLAN ID.
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureGuestVlanId(UI32_T lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next port guest VLAN ID.
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextSecureGuestVlanId(UI32_T *lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Set the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport,
 *            action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT   : none.
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortIntrusionAction(UI32_T lport, UI32_T action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next port intrusion action that determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPortIntrusionAction(UI32_T *lport, UI32_T *action_status);

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Set status of EAPOL frames pass-through when the global
 *            status dot1x is disabled
 * INPUT    : status    - VAL_dot1xEapolPassThrough_enabled(1)
 *                        VAL_dot1xEapolPassThrough_disabled(2)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xEapolPassThrough(UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xEapolPassThrough(UI32_T *status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xEapolPassThrough(UI32_T *status);
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC address aging mode.
 * INPUT    : mode -- MAC address aging mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_MGR_SetMacAddressAgingMode(UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- MAC address aging mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_MGR_GetMacAddressAgingMode(UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningMacAddressAgingMode
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- VAL_networkAccessAging_enabled,
 *                      VAL_networkAccessAging_disabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAddressAgingMode(UI32_T *mode_p);
#endif/*#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)*/

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection status on this port.
 * INPUT:  lport, status
 * OUTPUT: None
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionStatus(UI32_T lport, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection status on this port.
 * INPUT:  lport
 * OUTPUT: status_p
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  status_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection status
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionStatus(UI32_T lport, UI32_T *status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection mode on this port.
 * INPUT:  lport, mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionMode(UI32_T lport, UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection mode on this port.
 * INPUT:  lport
 * OUTPUT: mode_p
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection mode
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionMode(UI32_T lport, UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection action on this port.
 * INPUT:  lport, action.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionAction(UI32_T lport, UI32_T action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection action on this port.
 * INPUT:  lport
 * OUTPUT: action_p
 * RETURN: TRUE/FALSE.
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection action is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection action
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionAction(UI32_T lport, UI32_T *action_p);
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NETACCESS_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for netaccess mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceEapPacket_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Announce EAP packets CallBack function
 * INPUT:  mem_ref,dst_mac,src_mac,tag_info,type,pkt_length,lport_no
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceEapPacket_CallBack(
    L_MM_Mref_Handle_T *mem_ref,
    UI8_T *dst_mac,     UI8_T *src_mac,
    UI16_T vid,         UI16_T type,
    UI32_T pkt_length,  UI32_T lport,
    void *cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceRadiusAuthorizedResult
 * ---------------------------------------------------------------------
 * PURPOSE: Announce RADA authorized result to network access task
 * INPUT:  lport, mac, authorized_result, authorized_vlan_list, authorized_qos_list, session_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceRadiusAuthorizedResult(
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    char    *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceDot1xAuthorizedResult
 * ---------------------------------------------------------------------
 * PURPOSE: Announce dot1x authorized result to network access task
 * INPUT:  lport, mac, authorized_result, authorized_vlan_list, authorized_qos_list, session_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceDot1xAuthorizedResult(
    UI32_T  lport,                  UI8_T   *mac,
    int     eap_identifier,         UI8_T   authorized_result,
    char    *authorized_vlan_list,  char    *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_PortLinkUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Port link up callback function
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_PortLinkUp_CallBack(UI32_T unit,UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_PortLinkDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Port link down callback function
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_PortLinkDown_CallBack(UI32_T unit,UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_UPortAdminEnable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin Enable CallBack function
 * INPUT:   unit,port.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_UPortAdminEnable_CallBack(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_UPortAdminDisable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin disable CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_UPortAdminDisable_CallBack(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_VlanMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : callback func for vlan member addition
 * INPUT    : vid, lport, status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_VlanMemberAdd_CallBack(UI32_T vid_ifindex, UI32_T lport_ifidx, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_VlanMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : callback func for vlan member deletion
 * INPUT    : vid, lport, status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_VlanMemberDelete_CallBack(UI32_T vid_ifindex, UI32_T lport_ifidx, UI32_T vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceNewMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Announce New MAC CallBack function
 * INPUT    : dst_mac, ...
 * OUTPUT   : None.
 * RETURN   : TRUE -- intrusion packet, drop packet / FALSE -- not intrusion, go ahead
 * NOTES    : ** no need to L_MM_Mref_Release() if return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceNewMac_CallBack(
    UI32_T   src_lport,     UI16_T   vid,
    UI8_T   *src_mac,       UI8_T   *dst_mac,
    UI16_T   ether_type,    UI32_T   reason,
    void *cookie);

#if ((SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC) || \
     (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE))
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceMacAgeOut_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Announce mac age out to network access task
 * INPUT:  vid, mac, ifindex, is_age
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceMacAgeOut_CallBack(
    UI32_T  vid,    UI8_T   *mac,   UI32_T  ifindex,    BOOL_T  is_age);

#endif /* #if ((SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC) || \
               (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_DelAcl_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for acl deletion.
 * INPUT    : acl_name          -- which acl be deleted.
 *            dynamic_port_list -- the port list that bind the deleted policy
 *                                 map with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_MGR_DelAcl_CallBack(const char *acl_name, UI32_T acl_type, UI8_T *dynamic_port_list);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_DelPolicyMap_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for policy map deletion.
 * INPUT    : acl_name          -- which acl be deleted.
 *            dynamic_port_list -- the port list that bind the deleted policy
 *                                 map with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_MGR_DelPolicyMap_CallBack(const char *policy_map_name, UI8_T *dynamic_port_list);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_CreatSem
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for NETACCESS objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_CreatSem(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xConfigSettingToDefault
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set default value of 1X configuration
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : [to replace DOT1X_MGR_SetConfigSettingToDefault]
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xConfigSettingToDefault(void);
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
void NETACCESS_MGR_VlanList_CallBack(UI8_T*msg,UI8_T event);
BOOL_T NETACCESS_MGR_ProcessVlanListModifiedMsg(NETACCESS_VlanListModified_MSGQ_T *vlan_modified_msg_p);

#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AuthenticatePacket
 * ---------------------------------------------------------------------
 * PURPOSE:Asynchronous call to authenticate packet
 * INPUT:  mref_handle_p  - MREF handle for packet
 *         pkt_length     - packet length
 *         dst_mac        - destination mac address
 *         src_mac        - source mac address
 *         tag_info       - vlan tag information
 *         ether_type     - ehternet type
 *         src_unit       - source unit
 *         src_port       - source port
 *         cookie         - shall be passed to next CSC via
 *                          SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_AuthenticatePacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T pkt_length,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI16_T tag_info,
    UI16_T ether_type,
    UI32_T src_unit,
    UI32_T src_port,
    void * cookie
);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /*NETACCESS_MGR_H*/
