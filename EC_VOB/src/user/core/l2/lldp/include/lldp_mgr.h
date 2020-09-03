/*-----------------------------------------------------------------------------
 * Module Name: lldp_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP API
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/17/2005 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */

#ifndef LLDP_MGR_H
#define LLDP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "lldp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define LLDP_MGR_IPCMSG_TYPE_SIZE sizeof(union LLDP_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    LLDP_MGR_IPC_SETSYSADMINSTATUS,
    LLDP_MGR_IPC_SETMSGTXINTERVAL,
    LLDP_MGR_IPC_SETMSGTXHOLDMUL,
    LLDP_MGR_IPC_SETREINITDELAY,
    LLDP_MGR_IPC_SETTXDELAY,
    LLDP_MGR_IPC_SETNOTIFICATIONINTERVAL,
    LLDP_MGR_IPC_SETPORTCONFIGADMINSTATUS,
    LLDP_MGR_IPC_SETPORTCONFIGNOTIFICATIONENABLE,
    LLDP_MGR_IPC_SETPORTOPTIONALTLVSTATUS,
    LLDP_MGR_IPC_SETCONFIGMANADDRTLV,
    LLDP_MGR_IPC_SETCONFIGALLPORTMANADDRTLV,
    LLDP_MGR_IPC_GETPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETNEXTPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETCONFIGMANADDRTLV,
    LLDP_MGR_IPC_GETSYSSTATISTICSENTRY,
    LLDP_MGR_IPC_GETPORTTXSTATISTICSENTRY,
    LLDP_MGR_IPC_GETNEXTPORTTXSTATISTICSENTRY,
    LLDP_MGR_IPC_GETPORTRXSTATISTICSENTRY,
    LLDP_MGR_IPC_GETNEXTPORTRXSTATISTICSENTRY,
    LLDP_MGR_IPC_GETLOCALSYSTEMDATA,
    LLDP_MGR_IPC_GETLOCALPORTDATA,
    LLDP_MGR_IPC_GETNEXTLOCALPORTDATA,
    LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESSTLVENTRY,
    LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESS,
    LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESS,
    LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESSTLVENTRY,
    LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATA,
    LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYPORT,
    LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYINDEX,
    LLDP_MGR_IPC_GETREMOTEMANAGEMENTADDRESSTLVENTRY,
    LLDP_MGR_IPC_GETNEXTREMOTEMANAGEMENTADDRESSTLVENTRY,
    LLDP_MGR_IPC_GETXDOT1CONFIGENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1CONFIGENTRY,
    LLDP_MGR_IPC_SETXDOT1CONFIGENTRY,
    LLDP_MGR_IPC_SETXDOT1CONFIGPORTVLANTXENABLE,
    LLDP_MGR_IPC_SETXDOT1CONFIGPROTOVLANTXENABLE,
    LLDP_MGR_IPC_SETXDOT1CONFIGVLANNAMETXENABLE,
    LLDP_MGR_IPC_SETXDOT1CONFIGPROTOCOLTXENABLE,
    LLDP_MGR_IPC_GETXDOT1LOCENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1LOCENTRY,
    LLDP_MGR_IPC_GETXDOT1LOCPROTOVLANENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOVLANENTRY,
    LLDP_MGR_IPC_GETXDOT1LOCVLANNAMEENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1LOCVLANNAMEENTRY,
    LLDP_MGR_IPC_GETXDOT1LOCPROTOCOLENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOCOLENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1REMENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1REMENTRYBYINDEX,
    LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOVLANENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1REMVLANNAMEENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOCOLENTRY,
    LLDP_MGR_IPC_GETXDOT3PORTCONFIGENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3PORTCONFIGENTRY,
    LLDP_MGR_IPC_SETXDOT3PORTCONFIGENTRY,
    LLDP_MGR_IPC_SETXDOT3PORTCONFIG,
    LLDP_MGR_IPC_GETXDOT3LOCPORTENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3LOCPORTENTRY,
    LLDP_MGR_IPC_GETXDOT3LOCPOWERENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3LOCPOWERENTRY,
    LLDP_MGR_IPC_GETXDOT3LOCLINKAGGENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3LOCLINKAGGENTRY,
    LLDP_MGR_IPC_GETXDOT3LOCMAXFRAMESIZEENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3LOCMAXFRAMESIZEENTRY,
    LLDP_MGR_IPC_GETXDOT3REMPORTENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3REMPORTENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3REMPORTENTRYBYINDEX,
    LLDP_MGR_IPC_GETNEXTXDOT3REMPOWERENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3REMPOWERENTRYBYINDEX,
    LLDP_MGR_IPC_GETXDOT3REMLINKAGGENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3REMLINKAGGENTRY,
    LLDP_MGR_IPC_GETXDOT3REMMAXFRAMESIZEENTRY,
    LLDP_MGR_IPC_GETNEXTXDOT3REMMAXFRAMESIZEENTRY,
    LLDP_MGR_IPC_ISTELEPHONEMAC,
    LLDP_MGR_IPC_ISTELEPHONENETWORKADDR,
    LLDP_MGR_IPC_NOTIFYSYSNAMECHANGED,
    LLDP_MGR_IPC_NOTIFYRIFCHANGED,
    LLDP_MGR_IPC_NOTIFYROUTINGCHANGED,
    LLDP_MGR_IPC_NOTIFYPSETABLECHANGED,
    LLDP_MGR_IPC_SETPORTADMINDISABLE,

    LLDP_MGR_IPC_SETXMEDFASTSTARTREPEATCOUNT,
    LLDP_MGR_IPC_GETXMEDPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDPORTCONFIGENTRY,
    LLDP_MGR_IPC_SETXMEDPORTCONFIGTLVSTX,
    LLDP_MGR_IPC_GETRUNNINGXMEDPORTCONFIGTLVSTX,
    LLDP_MGR_IPC_SETXMEDPORTCONFIGNOTIFENABLED,
    LLDP_MGR_IPC_GETRUNNINGXMEDPORTNOTIFICATION,
    LLDP_MGR_IPC_GETXMEDLOCMEDIAPOLICYENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDLOCMEDIAPOLICYENTRY,
    LLDP_MGR_IPC_GETXMEDLOCLOCATIONENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONENTRY,
    LLDP_MGR_IPC_SETXMEDLOCLOCATIONENTRY,
    LLDP_MGR_IPC_GETXMEDLOCLOCATIONSTATUS,
    LLDP_MGR_IPC_SETXMEDLOCLOCATIONSTATUS,
    LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONSTATUS,
    LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE,
    LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE,
    LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRCOUTRYCODE,
    LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRWHAT,
    LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRWHAT,
    LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRWHAT,
    LLDP_MGR_IPC_GET1STXMEDLOCLOCATIONCIVICADDRCAENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONCIVICADDRCAENTRY,
    LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCAENTRY,
    LLDP_MGR_IPC_GETXMEDLOCXPOEPSEPORTENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDLOCXPOEPSEPORTENTRY,
    LLDP_MGR_IPC_GETXMEDLOCXPOEENTRY,
    LLDP_MGR_IPC_GETXMEDLOCINVENTORYENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMCAPENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMMEDIAPOLICYENTRY,
    LLDP_MGR_IPC_GETXMEDREMMEDIAPOLICYENTRYBYINDEX,
    LLDP_MGR_IPC_GETNEXTXMEDREMINVENTORYENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRYBYINDEX,
    LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRCOUTRYCODE,
    LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRWHAT,
    LLDP_MGR_IPC_GET1STXMEDREMLOCATIONCIVICADDRCAENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONCIVICADDRCAENTRY,
    LLDP_MGR_IPC_GETXMEDREMLOCATIONELIN,
    LLDP_MGR_IPC_GETNEXTXMEDREMPOEENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMPOEPSEENTRY,
    LLDP_MGR_IPC_GETNEXTXMEDREMPOEPDENTRY,
    LLDP_MGR_IPC_SETXDCBXPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETXDCBXPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETRUNNINGXDCBXPORTCONFIGENTRY,
    LLDP_MGR_IPC_GETXDCBXETSREMOTEENTRY,
    LLDP_MGR_IPC_GETXDCBXPFCREMOTEENTRY,
    LLDP_MGR_IPC_GETNEXTXDCBXAPPPRIREMOTEENTRY,
    LLDP_MGR_IPC_NOTIFYETSPFCCFGCHANGED,
};

enum
{
    LLDP_MGR_FID_REM_INVENTORY_HARDWARE_REV = 1,
    LLDP_MGR_FID_REM_INVENTORY_FRIMWARE_REV,
    LLDP_MGR_FID_REM_INVENTORY_SOFTWARE_REV,
    LLDP_MGR_FID_REM_INVENTORY_SERIAL_NUM,
    LLDP_MGR_FID_REM_INVENTORT_MFG_NAME,
    LLDP_MGR_FID_REM_INVENTORT_MODEL_NAME,
    LLDP_MGR_FID_REM_INVENTORT_ASSET_ID
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_MGR_IpcMsg_T.data
 */
#define LLDP_MGR_GET_MSG_SIZE(field_name)                       \
            (LLDP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((LLDP_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/*
LLDP_MGR_PortConfigEntry_T                                                          ()
*/
typedef struct
{
    UI32_T  port_num;                       /* key lport ifindex */
    UI32_T  admin_status;                   /* R/W */   /* 10.5.1 */
    UI32_T  notification_enable;            /* R/W */
    UI8_T   basic_tlvs_tx_flag;             /* R/W */   /* 1.2.2.1 */
} LLDP_MGR_PortConfigEntry_T;

/*
LLDP_MGR_ConfigManAddrEntry_T                                                       ()
*/
typedef struct
{
    UI8_T   ports_tx_enable[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* R/W */   /* Port List */
} LLDP_MGR_ConfigManAddrEntry_T;

/*
LLDP_MGR_Statistics_T                                                               ()
*/
typedef struct
{
    /* All R/O */
    UI32_T  rem_tables_last_change_time;
    UI32_T  rem_tables_inserts;
    UI32_T  rem_tables_deletes;
    UI32_T  rem_tables_drops;
    UI32_T  rem_tables_ageouts;
} LLDP_MGR_Statistics_T;

/*
LLDP_MGR_PortTxStatistics_T                                                         ()
*/
typedef struct
{
    UI32_T  port_num;                       /* key lport */
    UI32_T  tx_frames_total;                /* R/O */   /* 10.5.2.1 */
} LLDP_MGR_PortTxStatistics_T;

/*
LLDP_MGR_PortRxStatistics_T                                                         ()
*/
typedef struct
{
    /* All R/O */
    UI32_T  port_num;                       /* key lport */
    UI32_T  rx_frames_discarded_total;      /* 10.5.2.2 */
    UI32_T  rx_frames_errors;               /* 10.5.2.2 */
    UI32_T  rx_frames_total;                /* 10.5.2.2 */
    UI32_T  rx_tlvs_discarded_total;        /* 10.5.2.2 */
    UI32_T  rx_tlvs_unrecognized_total;     /* 10.5.2.2 */
    UI32_T  rx_ageouts_total;               /* 10.5.2.2 */

#if (SYS_CPNT_ADD == TRUE)
    UI32_T  rx_telephone_total;
#endif

} LLDP_MGR_PortRxStatistics_T;

/*
LLDP_MGR_LocalSystemData_T                                                          ()
*/
typedef struct
{
    /* All R/O */
    UI8_T  loc_chassis_id_subtype;                                  /* 9.5.2.2 MAC address(4) */
    UI8_T  loc_chassis_id[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH];            /* 9.5.2.3 CPU MAC */
    UI32_T loc_chassis_id_len;
    char   loc_sys_name[LLDP_TYPE_MAX_SYS_NAME_LENGTH];             /* 9.5.6.2 */
    UI32_T loc_sys_name_len;
    char   loc_sys_desc[LLDP_TYPE_MAX_SYS_DESC_LENGTH];             /* 9.5.7.2 */
    UI32_T loc_sys_desc_len;
    UI16_T loc_sys_cap_supported;                                   /* 9.5.8.1 bit2:bridge, bit4:router */
    UI16_T loc_sys_cap_enabled;                                     /* 9.5.8.2 bit2:bridge, bit4:router */
} LLDP_MGR_LocalSystemData_T;

/*
LLDP_MGR_LocalPortData_T                                                            ()
*/
typedef struct
{
    /* All R/O */
    UI32_T  port_num;                                               /* key lport */
    UI8_T   loc_port_id_subtype;                                       /* 9.5.3.2 Port MAC address(3)*/
    UI8_T   loc_port_id[LLDP_TYPE_MAX_PORT_ID_LENGTH];                 /* 9.5.3.3 Port MAC address */
    UI32_T  loc_port_id_len;
    char    loc_port_desc[LLDP_TYPE_MAX_PORT_DESC_LENGTH];             /* 9.5.5.2 RFC2863, ifDescr */
    UI32_T  loc_port_desc_len;

} LLDP_MGR_LocalPortData_T;

/*
LLDP_MGR_LocalManagementAddrEntry_T                                                 ()
*/
typedef struct
{
    /* All R/O */
    UI8_T  loc_man_addr_subtype;                                        /* key */   /* 9.5.9.3 IPv4(1)IPv6(2) */
    UI8_T  loc_man_addr[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];          /* key */   /* 9.5.9.4 */
    UI32_T loc_man_addr_len;
    UI8_T  ports_tx_enable[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* R/W */   /* Port List *//* 9.5.9.2 */
    UI8_T  loc_man_addr_if_subtype;                                             /* 9.5.9.5 unknown(1) */
    UI32_T loc_man_addr_if_id;                                                  /* 9.5.9.6 null:0 ??? */
    /*UI8_T  loc_man_addr_oid[LLDP_MGR_MANAGEMENT_ADDR_OID_LENGTH+1];*/             /* 9.5.9.8 OID string length:0 ??? */
} LLDP_MGR_LocalManagementAddrEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;
    UI32_T  rem_local_port_num;
    UI32_T  rem_index;
} LLDP_MGR_RemDataIndex_T ;

typedef struct
{
    UI32_T  lport;          /* key */
    UI32_T  port_vlan_id;   /* read-only */
} LLDP_MGR_Xdot1LocEntry_T;

typedef struct
{
    UI32_T  lport;                  /* key */
    UI32_T  proto_vlan_id;          /* key */
    BOOL_T  proto_vlan_supported;   /* read-only */
    BOOL_T  proto_vlan_enabled;     /* read-only */
} LLDP_MGR_Xdot1LocProtoVlanEntry_T;

typedef struct
{
    UI32_T  lport;          /* key */
    UI32_T  vlan_id;        /* key */
    UI8_T   vlan_name[LLDP_TYPE_MAX_VLAN_NAME_LEN];
    UI32_T  vlan_name_len;
} LLDP_MGR_Xdot1LocVlanNameEntry_T;

typedef struct
{
    UI32_T  lport;          /* key */
    UI32_T  protocol_index; /* key */
    UI8_T   protocol_id[LLDP_TYPE_MAX_PROTOCOL_IDENT_LEN];
    UI32_T  protocol_id_len;
} LLDP_MGR_Xdot1LocProtocolEntry_T;

typedef struct
{
    UI32_T  lport;      /* key */
    UI8_T   loc_port_auto_neg_supported;
    UI8_T   loc_port_auto_neg_enabled;
    UI16_T  loc_port_auto_neg_adv_cap;
    UI16_T  loc_port_oper_mau_type;
} LLDP_MGR_Xdot3LocPortEntry_T;

typedef struct
{
    UI32_T  lport;      /* key */
    UI8_T   loc_power_port_class;
    UI8_T   loc_power_mdi_supported;
    UI8_T   loc_power_mdi_enabled;
    UI8_T   loc_power_pair_controlable;
    UI8_T   loc_power_pairs;
    UI8_T   loc_power_class;
} LLDP_MGR_Xdot3LocPowerEntry_T;

typedef struct
{
    UI32_T  lport;      /* key */
    UI8_T   loc_link_agg_status;
    UI32_T  loc_link_agg_port_id;
} LLDP_MGR_Xdot3LocLinkAggEntry_T;

/* LLDP-MED */
typedef struct
{
    UI32_T  lport;                              /* key */
    UI16_T  lldp_xmed_port_cap_supported;
    UI16_T  lldp_xmed_port_tlvs_tx_enabled;     /* r/w */
    UI8_T   lldp_xmed_port_notif_enabled;       /* r/w */
} LLDP_MGR_XMedPortConfigEntry_T;

typedef struct
{
    UI32_T  lport;          /* key */
    UI8_T   app_type;       /* key */
    UI32_T  vid;
    UI32_T  priority;
    UI32_T  dscp;
    UI8_T   unknown;
    UI8_T   tagged;
} LLDP_MGR_XMedLocMediaPolicyEntry_T;

typedef struct
{
    UI8_T   ca_type;
    UI8_T   ca_length;
    UI8_T   ca_value[LLDP_TYPE_MAX_CA_VALUE_LEN];
} LLDP_MGR_XMedLocationCivicAddrCaEntry_T;

typedef struct
{
    UI8_T   elin_len;
    UI8_T   elin[LLDP_TYPE_MAX_ELIN_NUMBER_LEN];
} LLDP_MGR_XMedLocationElin_T;

#if 0
typedef struct
{
    UI32_T  latitude;
    UI32_T  longitude;
    UI16_T  altitude;
    UI8_T   datum;
} LLDP_MGR_XMedLocationCoordinated_T;
#endif

typedef struct
{
    UI32_T  lport;                          /* key */
    UI8_T   location_subtype;                /* key */
    UI32_T  location_info_len;
    UI8_T   location_info[LLDP_TYPE_MAX_LOCATION_ID_LEN];

} LLDP_MGR_XMedLocLocationEntry_T;

typedef struct
{
    UI32_T  lport;              /* key */
    UI32_T  power_av;
    UI8_T   pd_priority;
} LLDP_MGR_XMedLocXPoePsePortEntry_T;

typedef struct
{
    UI8_T   loc_hardware_rev[LLDP_TYPE_MAX_HARDWARE_REV_LEN];
    UI32_T  loc_hardware_rev_len;
    UI8_T   loc_firmware_rev[LLDP_TYPE_MAX_FIRMWARE_REV_LEN];
    UI32_T  loc_firmware_rev_len;
    UI8_T   loc_software_rev[LLDP_TYPE_MAX_SOFTWARE_REV_LEN];
    UI32_T  loc_software_rev_len;
    UI8_T   loc_serial_num[LLDP_TYPE_MAX_SERIAL_NUM_LEN];
    UI32_T  loc_serial_num_len;
    UI8_T   loc_mfg_name[LLDP_TYPE_MAX_MFG_NAME_LEN];
    UI32_T  loc_mfg_name_len;
    UI8_T   loc_model_name[LLDP_TYPE_MAX_MODEL_NAME_LEN];
    UI32_T  loc_model_name_len;
    UI8_T   loc_asset_id[LLDP_TYPE_MAX_ASSET_ID_LEN];
    UI32_T  loc_asset_id_len;
} LLDP_MGR_XMedLocInventory_T;

typedef struct
{
    UI8_T   device_type;
    UI8_T   pse_power_source;
    UI32_T  pd_power_req;
    UI8_T   pd_power_source;
    UI8_T   pd_power_priority;

} LLDP_MGR_XMedLocXPoeEntry_T;

typedef struct
{
    UI8_T   rem_hardware_rev[LLDP_TYPE_MAX_HARDWARE_REV_LEN];
    UI32_T  rem_hardware_rev_len;
    UI8_T   rem_firmware_rev[LLDP_TYPE_MAX_FIRMWARE_REV_LEN];
    UI32_T  rem_firmware_rev_len;
    UI8_T   rem_software_rev[LLDP_TYPE_MAX_SOFTWARE_REV_LEN];
    UI32_T  rem_software_rev_len;
    UI8_T   rem_serial_num[LLDP_TYPE_MAX_SERIAL_NUM_LEN];
    UI32_T  rem_serial_num_len;
    UI8_T   rem_mfg_name[LLDP_TYPE_MAX_MFG_NAME_LEN];
    UI32_T  rem_mfg_name_len;
    UI8_T   rem_model_name[LLDP_TYPE_MAX_MODEL_NAME_LEN];
    UI32_T  rem_model_name_len;
    UI8_T   rem_asset_id[LLDP_TYPE_MAX_ASSET_ID_LEN];
    UI32_T  rem_asset_id_len;

} LLDP_MGR_XMedRemInventoryEntry_T2;

/* IPC message structure
 */
typedef struct
{
	union LLDP_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
        UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
        UI32_T                               arg_ui32;
        LLDP_MGR_PortConfigEntry_T           arg_port_config_entry;
        LLDP_MGR_Statistics_T                arg_statistics;
        LLDP_MGR_PortTxStatistics_T          arg_port_tx_statistics;
        LLDP_MGR_PortRxStatistics_T          arg_port_rx_statistics;
        LLDP_MGR_LocalSystemData_T           arg_local_system_data;
        LLDP_MGR_LocalPortData_T             arg_local_port_data;
        LLDP_MGR_LocalManagementAddrEntry_T  arg_local_mgmt_addr_entry;
        LLDP_MGR_RemoteSystemData_T          arg_remote_system_data;
        LLDP_MGR_RemoteManagementAddrEntry_T arg_remote_mgmt_addr_entry;
        LLDP_MGR_Xdot1ConfigEntry_T          arg_xdot1_config_entry;
        LLDP_MGR_Xdot1LocEntry_T             arg_xdot1_loc_entry;
        LLDP_MGR_Xdot1LocProtoVlanEntry_T    arg_xdot1_loc_proto_vlan_entry;
        LLDP_MGR_Xdot1LocVlanNameEntry_T     arg_xdot1_loc_vlan_name_entry;
        LLDP_MGR_Xdot1LocProtocolEntry_T     arg_xdot1_loc_protocol_entry;
        LLDP_MGR_Xdot1RemEntry_T             arg_xdot1_rem_entry;
        LLDP_MGR_Xdot1RemProtoVlanEntry_T    arg_xdot1_rem_proto_vlan_entry;
        LLDP_MGR_Xdot1RemVlanNameEntry_T     arg_xdot1_rem_vlan_name_entry;
        LLDP_MGR_Xdot1RemProtocolEntry_T     arg_xdot1_rem_protocol_entry;
        LLDP_MGR_Xdot3PortConfigEntry_T      arg_xdot3_port_config_entry;
        LLDP_MGR_Xdot3LocPortEntry_T         arg_xdot3_local_port_entry;
        LLDP_MGR_Xdot3LocPowerEntry_T        arg_xdot3_loc_power_entry;
        LLDP_MGR_Xdot3LocLinkAggEntry_T      arg_xdot3_loc_link_agg_entry;
        LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T arg_xdot3_loc_maxframe_entry;
        LLDP_MGR_Xdot3RemPortEntry_T         arg_xdot3_rem_port_entry;
        LLDP_MGR_Xdot3RemPowerEntry_T        arg_xdot3_rem_power_entry;
        LLDP_MGR_Xdot3RemLinkAggEntry_T      arg_xdot3_rem_link_agg_entry;
        LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T arg_xdot3_rem_max_frame_size_entry;

#if (LLDP_TYPE_MED == TRUE)
        LLDP_MGR_XMedPortConfigEntry_T       arg_xmed_port_config_entry;
        LLDP_MGR_XMedLocMediaPolicyEntry_T   arg_xmed_loc_med_policy_entry;
        LLDP_MGR_XMedLocLocationEntry_T      arg_xmed_loc_location_entry;
        LLDP_MGR_XMedLocXPoePsePortEntry_T   arg_xmed_loc_poe_pse_port_entry;
        LLDP_MGR_XMedLocXPoeEntry_T          arg_xmed_loc_poe_entry;
        LLDP_MGR_XMedLocInventory_T          arg_xmed_loc_inventory_entry;
        LLDP_MGR_XMedRemCapEntry_T           arg_xmed_rem_cap_entry;
        LLDP_MGR_XMedRemMediaPolicyEntry_T   arg_xmed_rem_med_policy_entry;
        LLDP_MGR_XMedRemInventoryEntry_T     arg_xmed_rem_inventory_entry;
        LLDP_MGR_XMedRemLocationEntry_T      arg_xmed_rem_location_entry;
        LLDP_MGR_XMedRemPoeEntry_T           arg_xmed_rem_poe_entry;
        LLDP_MGR_XMedRemPoePseEntry_T        arg_xmed_rem_pse_entry;
        LLDP_MGR_XMedRemPoePdEntry_T         arg_xmed_rem_pd_entry;

        struct
        {
            UI32_T arg_lport;
            UI32_T arg_location_type;
            BOOL_T arg_status;
        } arg_grp_xmed_loc_status;

        struct
        {
            UI32_T arg_lport;
            UI8_T  arg_country_code[2];
        } arg_grp_xmed_loc_country_code;

        struct
        {
            UI32_T arg_lport;
            LLDP_MGR_XMedLocationCivicAddrCaEntry_T arg_ca_entry;
            BOOL_T arg_set_or_unset;
        } arg_grp_xmed_ca_entry;

        struct
        {
            UI32_T arg_rem_loc_port_num;
            UI32_T arg_rem_index;
            UI8_T  arg_country_code[2];
        } arg_grp_xmed_rem_country_code;

        struct
        {
            UI32_T arg_rem_loc_port_num;
            UI32_T arg_rem_index;
            UI8_T  arg_what;
        } arg_grp_xmed_rem_what;

        struct
        {
            UI32_T arg_rem_loc_port_num;
            UI32_T arg_rem_index;
            LLDP_MGR_XMedLocationCivicAddrCaEntry_T arg_ca_entry;
        } arg_grp_xmed_rem_ca_entry;

        struct
        {
            UI32_T arg_rem_loc_port_num;
            UI32_T arg_rem_index;
            LLDP_MGR_XMedLocationElin_T arg_rem_loc_elin_entry;
        } arg_grp_xmed_rem_loc_elin_entry;
#endif
#if (LLDP_TYPE_DCBX == TRUE)
        LLDP_TYPE_XdcbxPortConfigEntry_T        arg_xdcbx_port_config_entry;
        LLDP_TYPE_DcbxRemEtsEntry_T             arg_xdcbx_ets_rem_entry;
        LLDP_TYPE_DcbxRemPfcEntry_T             arg_xdcbx_pfc_rem_entry;
        LLDP_TYPE_DcbxRemAppPriEntry_T          arg_xdcbx_app_pri_rem_entry;
#endif
	    struct
	    {
	        UI32_T arg_ui32;
	        BOOL_T arg_bool_1;
	        BOOL_T arg_bool_2;
	    } arg_grp_ui32_bool_bool;

	    struct
	    {
	        UI32_T arg_ui32_1;
	        UI32_T arg_ui32_2;
	    } arg_grp_ui32_ui32;
	    struct
	    {
	        UI32_T arg_ui32;
	        BOOL_T arg_bool;
	    } arg_grp_ui32_bool;
	    struct
	    {
	        UI32_T arg_ui32;
	        UI8_T  arg_ui8;
	    } arg_grp_ui32_ui8;
	    struct
	    {
            UI8_T arg_ui8[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
	    } arg_grp_ui8;
	    struct
	    {
	        UI32_T arg_ui32_1;
	        UI8_T  arg_ui8[SYS_ADPT_MAC_ADDR_LEN];
	        UI32_T arg_ui32_2;
	    } arg_grp_ui32_ui8_ui32;
	    struct
	    {
	        UI32_T arg_ui32_1;
	        UI8_T  arg_ui8_1;
	        UI8_T  arg_ui8_2[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];
	        UI32_T arg_ui32_2;
	    } arg_grp_ui32_ui8_ui8_ui32;
            struct
            {
                UI32_T arg_ui32;
                UI16_T arg_ui16;
            } arg_grp_ui32_ui16;
	} data; /* the argument(s) for the function corresponding to cmd */
} LLDP_MGR_IpcMsg_T;

/*
___________________________________________________________________________         ()
*/
void
lldp_bitstring_setbit(char *bstring, int number);
void
lldp_bitstring_clearbit(char *bstring, int number);
int
lldp_bitstring_testbit(char *bstring, int number);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP global admin status
 * INPUT    : admin_status
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetSysAdminStatus(UI32_T admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_tx_interval to determine interval at which LLDP frames are transmitted
 * INPUT    : UI32_T msg_tx_interval    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 5~32768
 *            Default value: 30 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetMsgTxInterval(UI32_T msg_tx_interval) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_hold time multiplier to determine the actual TTL value used in an LLDPDU.
 * INPUT    : UI32_T msg_tx_hold_multiplier      --  a multiplier on the msgTxInterval
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 2~10
 *            Default value: 4.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetMsgTxHoldMul(UI32_T msg_tx_hold_mul) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the amount of delay from when adminStatus becomes ¡§disabled¡¦
 *            until re-initialization will be attempted.
 * INPUT    : UI32_T reinit_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~10
 *            Default value: 2 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetReinitDelay(UI32_T reinit_delay) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the minimum delay between successive LLDP frame transmissions.
 * INPUT    : UI32_T tx_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~8192
 *            Default value: 2 seconds.
 *            The recommended value is set by the following formula:
 *              1 <= lldpTxDelay <= (0.25 * lldpMessageTxInterval)
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetTxDelay(UI32_T tx_delay) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetNotificationInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set this value to control the transmission of LLDP notifications.
 * INPUT    : UI32_T interval_time    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Range: 5~3600
 *            Default value: 5 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetNotificationInterval(UI32_T notify_interval_time) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortConfigAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set admin_status to control whether or not a local LLDP agent is
 *             enabled(transmit and receive, transmit only, or receive only)
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.1, IEEE Std 802.1AB-2005.
 *            Default value: txAndRx(3).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortConfigAdminStatus(UI32_T lport, UI32_T admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortConfigNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : The value true(1) means that notifications are enabled;
 *            The value false(2) means that they are not.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T status           -- status vaule:
 *                                       true(1),
 *                                       false(2),
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Default value: false(2).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortConfigNotificationEnable(UI32_T lport, BOOL_T status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortOptionalTlvStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : UI32_T lport            -- lport number
 *            UI8_T tlv_status        -- bitmap:
 *                                       BIT_0: Port Description TLV,
 *                                       BIT_1: System Name TLV,
 *                                       BIT_2: System Description TLV,
 *                                       BIT_3: System Capabilities TLV,
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.2.1.1, IEEE Std 802.1AB-2005.
 *            Default value: no bit on (empty set).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortOptionalTlvStatus(UI32_T port, UI8_T tlv_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport, transfer_enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetConfigManAddrTlv(UI32_T lport, BOOL_T transfer_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetConfigAllPortManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : port_list       -- specified port list with size equal to
 *                               SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 *            transfer_enable -- flag to enable/disable transfer
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetConfigAllPortManAddrTlv(UI8_T *port_list) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for specified port.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next configuration entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocManAddrConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_ConfigManAddrEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocManAddrConfigEntry(LLDP_MGR_LocalManagementAddrEntry_T *) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocManAddrConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_ConfigManAddrEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocManAddrConfigEntry(LLDP_MGR_LocalManagementAddrEntry_T *) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetConfigManAddrEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : enable
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetConfigManAddrTlv(UI32_T lport, UI8_T *enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetSysStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info.
 * INPUT    : LLDP_MGR_Statistics_T *statistics_entry
 * OUTPUT   : LLDP_MGR_Statistics_T *statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetSysStatisticsEntry(LLDP_MGR_Statistics_T *statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get tx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get rx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_LocalSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2/9.5.6/9.5.7/9.5.8, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalSystemData(LLDP_MGR_LocalSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info. in the local system for specified port.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalPortData(LLDP_MGR_LocalPortData_T *port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalPortData(LLDP_MGR_LocalPortData_T *port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WE
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalNextManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemDataByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : If system_entry->index = 0, the return system entry will be
 *            the 1st data of the port
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteSystemDataByPort(LLDP_MGR_RemoteSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemDataByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteSystemDataByIndex(LLDP_MGR_RemoteSystemData_T *system_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry);

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRemoteUnknownTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get unknown TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteUnknownTlvEntry_T *unknow_tlv_entry
 * OUTPUT   : LLDP_MGR_RemoteUnknownTlvEntry_T *unknow_tlv_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.3.2/10.3.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetRemoteUnknownTlvEntry(LLDP_MGR_RemoteUnknownTlvEntry_T *unknow_tlv_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRemoteOrgDefInfoEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get organizationaly defined info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteOrgDefInfoEntry_T *org_def_info_entry
 * OUTPUT   : LLDP_MGR_RemoteOrgDefInfoEntry_T *org_def_info_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.6.1.3/9.6.1.4/9.6.1.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetRemoteOrgDefInfoEntry(LLDP_MGR_RemoteOrgDefInfoEntry_T *org_def_info_entry) ;
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_EnterMasterMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_EnterSlaveMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_SetTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_EnterTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberAdd1st_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 1st member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberDeleteLst_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PvidChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : When Pvid Changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PvidChanged_CallBack(UI32_T lport, UI32_T old_vid, UI32_T new_vid) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_VlanMemberChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : When vlan member changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_VlanMemberChanged_CallBack(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : None
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetOperationMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully set. Otherwise, false.
 * INPUT    : lport
 *            man_addr_subtype
 *            man_addr
 *            man_addr_len
 *            enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetLocManAddrConfig(UI32_T  lport,
                                    UI8_T   man_addr_subtype,
                                    UI8_T   *man_addr,
                                    UI32_T  man_addr_len,
                                    BOOL_T  enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will be invoke to do action when timer event come
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProcessTimerEvent() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : When there comes an lldpdu, this function will be invoked to
 *            handle the lldpdu
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProcessRcvdPDU(LLDP_TYPE_MSG_T *msg) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_RegisterTelephoneDetect
 *-------------------------------------------------------------------------
 * PURPOSE  : For registration of detecting a new neighbor
 * INPUT    : fun       -- function to be call back
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void  LLDP_MGR_RegisterTelephoneDetect(void (*fun)(UI32_T lport,
                                                   UI8_T *mac_addr,
                                                   UI8_T network_addr_subtype,
                                                   UI8_T *network_addr,
                                                   UI8_T network_addr_len,
                                                   UI32_T network_addr_ifindex,
                                                   BOOL_T tel_exist)) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PortAdminEnable_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for port link up
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PortAdminEnable_CallBack(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PortAdminDisable_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for port admin disable
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PortAdminDisable_CallBack(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleHotInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_HandleHotInsertion(UI32_T beg_ifindex, UI32_T end_of_index, BOOL_T use_default) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot remove.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port) ;

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigPortVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigPortVlanTxEnable(UI32_T lport, UI32_T port_vlan_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigProtoVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigProtoVlanTxEnable(UI32_T lport, UI32_T proto_vlan_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigVlanNameTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigVlanNameTxEnable(UI32_T lport, UI32_T vlan_name_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigProtocolTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigProtocolTxEnable(UI32_T lport, UI32_T protocol_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemEntryByIndex(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry) ;
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot3PortConfigEntry(UI32_T lport, UI8_T status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot3PortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot3PortConfig(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local port entry
 * INPUT    : xdot3_loc_port_entry
 * OUTPUT   : xdot3_loc_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_loc_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local port entry
 * INPUT    : xdot3_loc_port_entry
 * OUTPUT   : xdot3_loc_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */

UI32_T  LLDP_MGR_GetNextXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_loc_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPortEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPortEntryByIndex(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPowerEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPowerEntryByIndex(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemLinkAggEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemLinkAggEntryByIndex(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry) ;


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntryByIndex(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry) ;
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedFastStartRepeatCount(UI32_T  repeat_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port transfer tlvs
 * INPUT    : lport, tlvs_tx_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedPortConfigTlvsTx(UI32_T lport, UI16_T tlvs_tx_enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : tlvs_tx_enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXMedPortConfigTlvsTx(UI32_T lport, UI16_T *tlvs_tx_enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedPortConfigNotifEnabled
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port notification
 * INPUT    : notif_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedPortConfigNotifEnabled(UI32_T lport, UI8_T notif_enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedPortNotification
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXMedPortNotification(UI32_T lport, BOOL_T *enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location entry
 * INPUT    : loc_location_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 *            status
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr country code.
 * INPUT    : lport, country_code
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T what) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport
 * OUTPUT   : what
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr tlv.
 * INPUT    : lport, ca_tlv, set_or_unset
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry, BOOL_T set_or_unset) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocXPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local poe entry
 * INPUT    : None
 * OUTPUT   : loc_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocXPoeEntry(LLDP_MGR_XMedLocXPoeEntry_T *loc_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local inventory entry
 * INPUT    : None
 * OUTPUT   : loc_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocInventoryEntry(LLDP_MGR_XMedLocInventory_T *loc_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry by lport, index and app_type
 *            (i.e. without timemark)
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemLocationEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used onlu for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemLocationEntryByIndex(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCountryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationCivicAddrCountryCode(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       UI8_T *country_code);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationCivicAddrWhat(UI32_T rem_loc_port_num,
                                                UI32_T rem_index,
                                                UI8_T *what);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_Get1stXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                      UI32_T rem_index,
                                                      LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetNextXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationElin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location elin entry
 * INPUT    : None
 * OUTPUT   : rem_loc_elin_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationElin(UI32_T rem_loc_port_num,
                                       UI32_T rem_index,
                                       LLDP_MGR_XMedLocationElin_T *rem_loc_elin_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pd entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry) ;
#endif /* #if (LLDP_TYPE_MED == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsTelephoneMac
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the mac address is belonged to a telephone.
 * INPUT    : lport, device_mac, device_mac_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_IsTelephoneMac(UI32_T lport, UI8_T  *device_mac, UI32_T device_mac_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsTelephoneNetworkAddr
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the netowrk address is belonged to a telephone.
 * INPUT    : lport, device_network_addr_subtype, device_network_addr, device_network_addr_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_IsTelephoneNetworkAddr(UI32_T lport, UI8_T device_network_addr_subtype, UI8_T  *device_network_addr, UI32_T device_network_addr_len) ;
#endif /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_DCBX == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyEtsPfcCfgChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When ets/pfc cfg is changed, this function will be called.
 * INPUT    : lport       -- the specified port
 *            is_ets_chgd -- TRUE if ets is changed
 *            is_pfc_chgd -- TRUE if pfc is changed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyEtsPfcCfgChanged(
    UI32_T  lport,
    BOOL_T  is_ets_chgd,
    BOOL_T  is_pfc_chgd);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Set DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port__config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p->lport
 * OUTPUT   : xdcbx_port__config_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port__config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p->lport
 * OUTPUT   : xdcbx_port__config_entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port__config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetDcbxEtsRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX remote ETS entry
 * INPUT    : lport
 * OUTPUT   : rem_ets_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetDcbxEtsRemoteData(LLDP_TYPE_DcbxRemEtsEntry_T *rem_ets_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetDcbxPfcRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX remote PFC entry
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetDcbxPfcRemoteData(LLDP_TYPE_DcbxRemPfcEntry_T *rem_pfc_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextDcbxAppPriRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get Next DCBX remote Application Priority entry
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextDcbxAppPriRemoteData(LLDP_TYPE_DcbxRemAppPriEntry_T *rem_app_pri_entry_p);

#endif/* #if (SYS_CPNT_DCBX == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


/*=============================================================================
 * Moved from lldp_mgr_notify.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifySysNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifySysNameChanged(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyVlanNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyVlanNameChanged(UI32_T vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyRifChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : vid_ifindex -- the specified vlan;
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyRifChanged(UI32_T vid_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyIfMauChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyIfMauChanged(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyRoutingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyRoutingChanged(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyProtoVlanGroupIdBingingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyProtoVlanGroupIdBindingChanged(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyPseTableChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : lport can be 0, which means changes for all lports
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyPseTableChanged(UI32_T lport) ;


/*=============================================================================
 * Moved from lldp_task.h and changed name
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the LLDP CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_Init(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell LLDP that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by LLDP mgr will tell if LLDP provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_IsProvisionComplete(void);

/* created for replacing lldp_task */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_LldpduRcvd_Callback
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event when receiving a LLDPDU packet.
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           lport         -- source lport
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------------------
 */
#if 0
void LLDP_MGR_LldpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T *dst_mac,
                                  UI8_T *src_mac,
                                  UI16_T tag_info,
                                  UI16_T type,
                                  UI32_T length,
                                  UI32_T lport);
#else
void LLDP_MGR_LldpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T *dst_mac,
                                  UI8_T *src_mac,
                                  UI16_T tag_info,
                                  UI16_T type,
                                  UI32_T pkt_length,
                                  UI32_T  src_unit,
                                  UI32_T  src_port,
                                  UI32_T  packet_class);

#endif

#endif /* End of LLDP_MGR_H */
