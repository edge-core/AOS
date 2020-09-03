/*-----------------------------------------------------------------------------
 * MODULE NAME: LLDP_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of LLDP OM which are only used by LLDP.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/07     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef LLDP_OM_PRIVATE_H
#define LLDP_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_link_lst.h"
#include "l_sort_lst.h"
#include "lldp_om.h"
#include "lldp_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

enum
{
#if (SYS_CPNT_CN == TRUE)
    LLDP_OM_DETECT_CHANGE_CN_TLV,
#endif
    LLDP_OM_MAX_DECTECT_NUM,
};

enum
{
    LLDP_OM_DELETE_REASON_GLOBALDISABLED,   /* global status disabled */
    LLDP_OM_DELETE_REASON_PERPORTDISABLED,  /* per port status disabled */
    LLDP_OM_DELETE_REASON_AGEOUT,
    LLDP_OM_DELETE_REASON_UPDATE,
    LLDP_OM_DELETE_REASON_RXSHUTDOWN,       /* rx shutdown LLDPDU */
};

/* DATA TYPE DECLARATIONS
 */

/* LLDP_OM_XMedLocationCivicAddr_T */
typedef struct
{
    UI8_T   ca_type;
    UI8_T   ca_length;
    UI8_T   ca_value[LLDP_TYPE_MAX_CA_VALUE_LEN];
}LLDP_OM_XMedLocationCivicAddrCaTlv_T;

typedef struct
{
    UI8_T   what;
    UI8_T   country_code[2];
    L_SORT_LST_List_T   ca_list;
}LLDP_OM_XMedLocationCivicAddr_T;

/* LLDP_OM_XMedLocationElin_T */
typedef struct
{
    UI8_T   elin_len;
    UI8_T   elin[LLDP_TYPE_MAX_ELIN_NUMBER_LEN];
} LLDP_OM_XMedLocationElin_T;

/* LLDP_OM_XMedLocationCoordinated_T */
typedef struct
{
    UI32_T  latitude;
    UI32_T  longitude;
    UI16_T  altitude;
    UI8_T   datum;
} LLDP_OM_XMedLocationCoordinated_T;

/* LLDP_OM_XMedLocLocationEntry_T */
typedef struct
{
    UI32_T  location_type_valid;
    LLDP_OM_XMedLocationCoordinated_T   coord_addr;
    LLDP_OM_XMedLocationCivicAddr_T     civic_addr;
    LLDP_OM_XMedLocationElin_T          elin_addr;
} LLDP_OM_XMedLocLocationEntry_T;

/*
LLDP_OM_SysConfigEntry_T                                                            ()
 */
typedef struct
{
     /* All R/W */
    UI32_T  msg_tx_interval;                /* 10.5.3.3 */
    UI32_T  msg_tx_hold_mul;                /* 10.5.3.3 */
    UI32_T  reinit_delay;                   /* 10.5.3.3 */
    UI32_T  tx_delay;                       /* 10.5.3.3 */
    UI32_T  notification_interval;
    UI32_T  global_admin_status;
    BOOL_T  something_changed_remote;
    UI32_T  fast_start_repeat_count;
    UI32_T  notification_timer;
}LLDP_OM_SysConfigEntry_T;

/*
LLDP_OM_PortConfigEntry_T                                                           ()
*/
typedef struct
{
    /* configuration status */
    UI32_T  lport_num;
    UI8_T   admin_status;                   /* R/W */   /* 10.5.1 */ /* get from oper UP/DOWN */
    UI32_T  notification_enable;            /* R/W */
    UI8_T   basic_tlvs_tx_flag;             /* R/W */   /* 1.2.2.1 */
    BOOL_T  man_addr_transfer_flag;

    BOOL_T  reinit_flag;
    BOOL_T  something_changed_local;
    BOOL_T  something_changed_remote;
    UI32_T  transfer_timer;
    UI32_T  tx_delay_timer;
    UI32_T  reinit_delay_timer;
    UI32_T  notify_timer;

    /* 802.1 extensions */
    UI8_T   xdot1_port_vlan_tx_enable;
    UI8_T   xdot1_vlan_name_tx_enable;
    UI8_T   xdot1_proto_vlan_tx_enable;
    UI8_T   xdot1_protocol_tx_enable;
    BOOL_T  xdot1_cn_tx_enable;

    /* 802.3 extensions */
    UI8_T   xdot3_tlvs_tx_flag;

    /* LLDP-MED */
    UI32_T  fast_start_count;
    BOOL_T  med_device_exist;
    UI16_T  lldp_med_tx_enable;
    UI8_T   lldp_med_notification;

    LLDP_OM_XMedLocLocationEntry_T  lldp_med_location;

    /* DCBX */
    UI8_T   xdcbx_tlvs_tx_flag;
    UI8_T   rx_dcbx_peer_num;
}LLDP_OM_PortConfigEntry_T;

/*
LLDP_OM_ConfigManAddrEntry_T                                                        ()
*/
typedef struct
{
    UI8_T   loc_man_addr_subtype;
    UI8_T   loc_man_addr[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];
    UI32_T  loc_man_addr_len;
    UI8_T   ports_tx_enable[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* R/W */   /* Port List */
}LLDP_OM_ConfigManAddrEntry_T;

/*
LLDP_OM_Statistics_T                                                                ()
*/
typedef struct
{
        /* All R/O */
    UI32_T  rem_table_last_change_time;
    UI32_T  rem_table_inserts;
    UI32_T  rem_table_deletes;
    UI32_T  rem_table_drops;
    UI32_T  rem_table_ageouts;
}LLDP_OM_Statistics_T;

/*
LLDP_OM_PortStatistics_T                                                            ()
*/
typedef struct
{
    /* Tx */
    UI32_T  tx_frames_total;                /* R/O */   /* 10.5.2.1 */
    /* Rx */
    UI32_T  rx_frames_discarded_total;      /* 10.5.2.2 */
    UI32_T  rx_frames_errors;               /* 10.5.2.2 */
    UI32_T  rx_frames_total;                /* 10.5.2.2 */
    UI32_T  rx_tlvs_discarded_total;        /* 10.5.2.2 */
    UI32_T  rx_tlvs_unrecognized_total;     /* 10.5.2.2 */
    UI32_T  rx_ageouts_total;               /* 10.5.2.2 */

    /* for telephone */
    UI32_T  rx_telephone_total;
}LLDP_OM_PortStatistics_T;

/* Note:
 *      No data structure for store local system MIB,
 *      and query the newest local-system-data at run time
 */

/*
LLDP_OM_RemManAddrEntry_T                                                           ()
*/
typedef struct LLDP_OM_RemManAddrEntry_S
{
    UI8_T   rem_man_addr_subtype;                                    /* key */   /* 9.5.9.3 */
    UI8_T   rem_man_addr[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];      /* key */   /* 9.5.9.4 */
    UI32_T  rem_man_addr_len;
    UI8_T   rem_man_addr_if_subtype;                                             /* 9.5.9.5 */
    UI32_T  rem_man_addr_if_id;                                                  /* 9.5.9.6 */
    UI8_T   rem_man_addr_oid[LLDP_TYPE_MAX_MANAGEMENT_ADDR_OID_LENGTH];                   /* 9.5.9.8 */
    UI32_T  rem_man_addr_oid_len;

    #if 0
    struct LLDP_OM_RemManAddrEntry_S      *next;
    #endif
}LLDP_OM_RemManAddrEntry_T;

/*
LLDP_OM_MSAP_ID_T                                                                      ()
*/
typedef struct
{
    UI8_T     rem_chassis_id[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH];
    UI8_T     rem_port_id[LLDP_TYPE_MAX_PORT_ID_LENGTH];
}LLDP_OM_MSAP_ID_T;

/*
LLDP_OM_RemSysEntry_T                                                               ()
*/
typedef struct LLDP_OM_RemSysEntry_S
{
    UI8_T   *rem_port_desc;         /* 9.5.5.2 */
    UI32_T  rem_port_desc_len;
    UI8_T   *rem_sys_name;          /* 9.5.6.2 */
    UI32_T  rem_sys_name_len;
    UI8_T   *rem_sys_desc;          /* 9.5.7.2 */
    UI32_T  rem_sys_desc_len;
    UI16_T  rem_sys_cap_supported;  /* 9.5.8.1 */
    UI16_T  rem_sys_cap_enabled;    /* 9.5.8.2 */

}LLDP_OM_RemSysEntry_T;


typedef struct
{
    UI32_T  rem_proto_vlan_id;
    UI8_T   rem_proto_vlan_supported;
    UI8_T   rem_proto_vlan_enabled;
} LLDP_OM_Xdot1RemProtoVlanEntry_T ;

typedef struct
{
    UI16_T  rem_vlan_id;
    UI8_T   rem_vlan_name[LLDP_TYPE_MAX_VLAN_NAME_LEN];
    UI32_T  rem_vlan_name_len;
} LLDP_OM_Xdot1RemVlanNameEntry_T;

typedef struct
{
    UI32_T  rem_protocol_index;
    UI8_T   rem_protocol_id[LLDP_TYPE_MAX_PROTOCOL_IDENT_LEN];
    UI32_T  rem_protocol_id_len;
} LLDP_OM_Xdot1ProtocolEntry_T;

typedef struct
{
    UI8_T   rem_auto_neg_support;
    UI8_T   rem_auto_neg_enable;
    UI16_T  rem_port_auto_neg_adv_cap;
    UI16_T  rem_port_oper_mau_type;
} LLDP_OM_Xdot3RemPortEntry_T;

typedef struct
{
    UI8_T   rem_power_port_class;
    UI8_T   rem_power_mdi_supported;
    UI8_T   rem_power_mdi_enabled;
    UI8_T   rem_power_pair_controlable;
    UI8_T   rem_power_pairs;
    UI8_T   rem_power_class;
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    UI8_T   rem_power_type;
    UI8_T   rem_power_source;
    UI8_T   rem_power_priority;
    UI16_T  rem_pd_requested_power;
    UI16_T  rem_pse_allocated_power;
#endif /* #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2) */
} LLDP_OM_Xdot3RemPowerEntry_T;

typedef struct
{
    UI8_T   rem_link_agg_status;
    UI32_T  rem_link_agg_port_id;
} LLDP_OM_Xdot3RemLinkAggEntry_T;


/* LLDP-MED */
typedef struct
{
    struct
    {
        BOOL_T  valid;
        BOOL_T  unknown;
        BOOL_T  tagged;
        BOOL_T  reserved;
        UI16_T  vid;
        UI8_T   priority;
        UI8_T   dscp;
    } app_type[LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE];
} LLDP_OM_XMedRemNetworkPolicyEntry_T;

typedef LLDP_OM_XMedLocLocationEntry_T LLDP_OM_XMedRemLocationEntry_T;

typedef struct
{
    UI8_T   power_type;
    UI8_T   power_source;
    UI8_T   power_priority;
    UI16_T  power_value;

} LLDP_OM_XMedRemExtPowerEntry_T;

typedef struct
{
    UI8_T   hardware_revision[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   hardware_revision_len;
    UI8_T   firmware_revision[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   firmware_revision_len;
    UI8_T   software_revision[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   software_revision_len;
    UI8_T   serial_num[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   serial_num_len;
    UI8_T   manufaturer_name[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   manufaturer_name_len;
    UI8_T   model_name[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   model_name_len;
    UI8_T   asset_id[LLDP_TYPE_MAX_INV_ITEM_LEN];
    UI8_T   asset_id_len;
} LLDP_OM_XMedRemInventoryEntry_T;

/* DCBX */
typedef struct
{
    BOOL_T  rem_willing;
    BOOL_T  rem_cbs;
    UI8_T   rem_max_tc;
    UI8_T   rem_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
} LLDP_OM_XDcbxRemEtsConfigEntry_T;

typedef struct
{
    UI8_T   rem_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
} LLDP_OM_XDcbxRemEtsRecommendEntry_T;

typedef struct
{
    UI8_T   rem_mac[6];
    BOOL_T  rem_willing;
    BOOL_T  rem_mbc;
    UI8_T   rem_cap;
    UI8_T   rem_enable;
} LLDP_OM_XDcbxRemPfcConfigEntry_T;

typedef struct
{
    UI8_T   rem_priority;
    UI8_T   rem_sel;
    UI16_T  rem_protocol_id;
} LLDP_OM_XDcbxRemAppPriorityEntry_T;

typedef struct LLDP_DCBX_TIMER_S
{
    BOOL_T  enabled;
    UI32_T  time;
    UI32_T  limit;
} LLDP_DCBX_TIMER_T;

typedef struct
{
    UI8_T   cnpv_indicators;
    UI8_T   ready_indicators;
} LLDP_OM_Xdot1RemCnEntry_T;

/*
LLDP_OM_RemData_T                                                                   ()
*/
typedef struct LLDP_OM_RemData_S
{
    UI8_T                       rem_chassis_id[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH];        /* hisam key */
    UI8_T                       rem_port_id[LLDP_TYPE_MAX_PORT_ID_LENGTH];              /* hisam key */

    UI16_T                      rx_info_ttl;            /* record TTL */
    UI32_T                      age_out_time;           /* expire time*/

    UI32_T                      time_mark;              /* key */
    UI32_T                      lport;                  /* key */
    UI32_T                      index;                  /* key */

    UI8_T                       rem_chassis_id_subtype;                                 /* 9.5.2.2 */
    UI32_T                      rem_chassis_id_len;
    UI8_T                       rem_port_id_subtype;                                    /* 9.5.3.2 */
    UI32_T                      rem_port_id_len;

    LLDP_OM_RemSysEntry_T       *rem_sys_entry;

    L_SORT_LST_List_T           rem_man_addr_list;


    /* 802.1 extensitons*/
    UI32_T                      xdot1_rem_port_vlan_id;
    L_SORT_LST_List_T           xdot1_rem_proto_vlan_list;
    L_SORT_LST_List_T           xdot1_rem_vlan_name_list;
    L_SORT_LST_List_T           xdot1_rem_protocol_list;
    LLDP_OM_Xdot1RemCnEntry_T   *xdot1_rem_cn_entry;

    /* 802.3 extensions */
    LLDP_OM_Xdot3RemPortEntry_T     *xdot3_rem_port_entry;
    LLDP_OM_Xdot3RemPowerEntry_T    *xdot3_rem_power_entry;
    LLDP_OM_Xdot3RemLinkAggEntry_T  *xdot3_rem_link_agg_entry;
    UI32_T                          xdot3_rem_max_frame_size;

    /* LLDP-MED */
    /* MED capabilities */
    UI16_T      lldp_med_cap_sup;
    UI16_T      lldp_med_cap_enabled;
    UI8_T       lldp_med_device_type;

    /* MED Network policy */
    LLDP_OM_XMedRemNetworkPolicyEntry_T *med_rem_network_policy;

    /* MED location identity */
    LLDP_OM_XMedRemLocationEntry_T      *med_rem_location;

    /* MED extended power via mdi */
    LLDP_OM_XMedRemExtPowerEntry_T      *med_rem_ext_power;

    /* MED Inventory */
    LLDP_OM_XMedRemInventoryEntry_T     *med_rem_inventory;

    /* DCBX ETS-Config */
    LLDP_OM_XDcbxRemEtsConfigEntry_T    *dcbx_rem_ets_config_entry;

    /* DCBX ETS-Recommend */
    LLDP_OM_XDcbxRemEtsRecommendEntry_T *dcbx_rem_ets_recommend_entry;

    /* DCBX PFC-Config */
    LLDP_OM_XDcbxRemPfcConfigEntry_T    *dcbx_rem_pfc_config_entry;

    /* DCBX App-Priority */
    L_SORT_LST_List_T                   dcbx_rem_app_pri_list;

    /* for age seq */
    struct LLDP_OM_RemData_S            *age_seq_prev;
    struct LLDP_OM_RemData_S            *age_seq_next;

    struct LLDP_OM_RemData_S            *self;

}LLDP_OM_RemData_T;


/*
LLDP_OM_ReinitDelayPort_T                                                           ()
*/
typedef struct LLDP_OM_ReinitDelayPort_S
{
    UI32_T                          lport;
    UI32_T                          reinit_time;

    struct LLDP_OM_ReinitDelayPort_S       *next;
    struct LLDP_OM_ReinitDelayPort_S       *prev;
}LLDP_OM_ReinitDelayPort_T;

/*
LLDP_OM_TxDelayPort_T                                                           ()
*/
typedef struct LLDP_OM_TxDelayPort_S
{
    UI32_T                          lport;
    UI32_T                          tx_delay_time;

    struct LLDP_OM_TxDelayPort_S           *next;
    struct LLDP_OM_TxDelayPort_S           *prev;
}LLDP_OM_TxDelayPort_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSysConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_SysConfigEntry_T *
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_SysConfigEntry_T* LLDP_OM_GetSysConfigEntryPtr() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_PortConfigEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_PortConfigEntry_T* LLDP_OM_GetPortConfigEntryPtr(UI32_T lport) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetStatisticsPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the global statistics information
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_Statistics_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_Statistics_T* LLDP_OM_GetStatisticsPtr() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_RemDataExist
 * ------------------------------------------------------------------------
 * PURPOSE  : Check whether the rem_data is in hisam table
 * INPUT    : rem_data          -- remote data to be checked (it will use the
 *                                 chassis_id and port_id field of the remote data)
 * OUTPUT   : void *ptr
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_RemDataExist(LLDP_OM_RemData_T *rem_data, LLDP_OM_RemData_T **ptr) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortStatisticsEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port statistics information
 * INPUT    : lport         -- a specified port
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_PortStatistics_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_PortStatistics_T* LLDP_OM_GetPortStatisticsEntryPtr(UI32_T lport) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_Init(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for LLDP objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_InitSemaphore(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to DB
 * INPUT    : remote data       -- remote data to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertRemData(LLDP_OM_RemData_T *rem_data) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from the DB
 * INPUT    : rem_data -- remote data to be deleted
 *            reason   -- why to delete remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemData(LLDP_OM_RemData_T *delete_rem_data, UI32_T reason) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemData_EX
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from the DB
 * INPUT    : new_rem_data_p -- new remote data
 *            del_rem_data_p -- old remote data to be deleted
 *            reason         -- why to delete remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemData_EX(
        LLDP_OM_RemData_T *new_rem_data_p,
        LLDP_OM_RemData_T *del_rem_data_p,
        UI32_T             reason);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemDataByPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from DB by a specified port
 * INPUT    : lport         -- a specified port
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemDataByPort(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is raised to
 *              SYS_BLD_RAISE_TO_HIGH_PRIORITY.
 *-------------------------------------------------------------------------
 */
void  LLDP_OM_EnterCriticalSection(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to the original
 *              task priority.
 *-------------------------------------------------------------------------
 */
void  LLDP_OM_LeaveCriticalSection(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AllocRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Allocate a remote data entry from the memory pool
 * INPUT    : None
 * OUTPUT   : void *rem_data
 * RETUEN   : LLDP_TYPE_RETURN_ERROR
 *            LLDP_TYPE_RETURN_OK
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_AllocRemData(LLDP_OM_RemData_T **rem_data) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Free a remote data entry to the memory pool
 * INPUT    : free_data        -- remote data entry to be freed
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_FreeRemData(LLDP_OM_RemData_T *free_data) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeOutRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Age out remote data when the timer expired
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeOutRemData(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetAll
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_ResetAll() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_ResetPort(UI32_T   lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetPortStatistics
 *-------------------------------------------------------------------------
 * PURPOSE  : Reset port statistics
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_ResetPortStatistics(UI32_T lport) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemManAddrList
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the remote management address sort list
 * INPUT    : list          -- the list to be initialized
 *            count         -- the max number of entry in the sort list
 * OUTPUT   : TRUE/FALSE
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemManAddrList(L_SORT_LST_List_T *list, UI32_T count) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data
 * INPUT    : time_mark         -- a specified timestamp
 *            lport             -- a specified port
 *            index             -- a specified index
 * OUTPUT   : get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetRemData(UI32_T lport, UI32_T index, LLDP_OM_RemData_T **get_data) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data on a port
 * INPUT    : lport                 -- a specified port (fix)
 *            (*rem_data_pp)->index -- a specified index
 * OUTPUT   : get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextRemDataByPort(UI32_T lport, LLDP_OM_RemData_T **rem_data_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemDataWithTimeMark
 * ------------------------------------------------------------------------
 * PURPOSE  : Find remote data with timemark, lport and index
 * INPUT    : (*rem_data_pp)->time_mark  -- a specified timestamp
 *            (*rem_data_pp)->lport      -- a specified port
 *            (*rem_data_pp)->index      -- a specified index
 * OUTPUT   : rem_data_pp
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetRemDataWithTimeMark(LLDP_OM_RemData_T **rem_data_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemDataWithTimeMark
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data
 * INPUT    : (*rem_data_pp)->time_mark  -- a specified timestamp
 *            (*rem_data_pp)->lport      -- a specified port
 *            (*rem_data_pp)->index      -- a specified index
 * OUTPUT   : rem_data_pp
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextRemDataWithTimeMark(LLDP_OM_RemData_T **rem_data_pp);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Insert local management address configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_SetLocManAddrConfig(LLDP_OM_ConfigManAddrEntry_T* input_config) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get local management address configuration entry pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetLocManAddrConfigPtr(LLDP_OM_ConfigManAddrEntry_T** loc_man_addr_config) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next local management address configuration entry pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextLocManAddrConfigPtr(LLDP_OM_ConfigManAddrEntry_T** loc_man_addr_config) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete local management address configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_DeleteLocManAddrConfig(LLDP_OM_ConfigManAddrEntry_T* del_config) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemProtoVlanList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol vlan list
 * INPUT    : rem_proto_vlan_list
 * OUTPUT   : rem_proto_vlan_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemProtoVlanList(L_SORT_LST_List_T *rem_proto_vlan_list) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemVlanNameList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote vlan name list
 * INPUT    : rem_vlan_name_list
 * OUTPUT   : rem_vlan_name_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemVlanNameList(L_SORT_LST_List_T *rem_vlan_name_list) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemProtocolList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol list
 * INPUT    : rem_proto_list
 * OUTPUT   : rem_proto_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemProtocolList(L_SORT_LST_List_T *rem_proto_list) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateCivicAddrCaList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol list
 * INPUT    : rem_proto_list
 * OUTPUT   : rem_proto_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateCivicAddrCaList(L_SORT_LST_List_T *civic_addr_ca_list) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeCivicAddrCaList
 * ------------------------------------------------------------------------
 * PURPOSE  : Free Civic addr ca list
 * INPUT    : civic_addr_ca_list
 * OUTPUT   : civic_addr_ca_list
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_FreeCivicAddrCaList(L_SORT_LST_List_T *civic_addr_ca_list) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeSeqInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Insert a remote data to the age sequence
 * INPUT    : new_rem_data      -- input remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeSeqInsert(LLDP_OM_RemData_T *new_rem_data) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeSeqDelete
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete a remote data from the age sequence
 * INPUT    : rem_data      -- remote data to be deleted
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeSeqDelete(LLDP_OM_RemData_T *del_rem_data) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyMedRemTableChangedList
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyMedRemTableChangedList() ;

#if (SYS_CPNT_ADD == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyTelephoneTypeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyTelephoneTypeListPtr();
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyTelephoneAddrChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyTelephoneAddrChangedListPtr() ;
#endif

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDot3atInfoReceivedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyDot3atInfoReceivedListPtr(void) ;
#endif /* #if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) */

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyCnRemoteChangeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T*
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T* LLDP_OM_GetNotifyCnRemoteChangeListPtr(void) ;
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyRemChangeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T*
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T* LLDP_OM_GetNotifyRemChangeListPtr(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortDeviceList
 *-------------------------------------------------------------------------
 * PURPOSE  : Get remote data list of port
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : SYS_TYPE_CallBack_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetPortDeviceList(UI32_T lport) ;

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSmallestTimeMarkOfRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get neighbor type call back list
 * INPUT    : None
 * OUTPUT   : smallest_time_mark
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_GetSmallestTimeMarkOfRemData(UI32_T *smallest_time_mark) ;
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ReleaseConvertMem
 *-------------------------------------------------------------------------
 * PURPOSE  : Release memory dynamically allocated when converting remote
 *            message
 * INPUT    : free_data - pointer to the memory to be released
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_ReleaseConvertMem(LLDP_OM_RemData_T *free_data);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemAppPriorityList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote vlan name list
 * INPUT    : rem_app_priority_list
 * OUTPUT   : rem_app_priority_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemAppPriorityList(L_SORT_LST_List_T *rem_app_priority_list);

#if(SYS_CPNT_DCBX == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertNotifyDcbxRemEtsChangedList
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to NotifyDcbxRemEtsChangedList
 * INPUT    : lport -- lport to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertNotifyDcbxRemEtsChangedList(UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertNotifyDcbxRemPfcChangedList
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to NotifyDcbxRemPfcChangedList
 * INPUT    : lport -- lport to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertNotifyDcbxRemPfcChangedList(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr(void);

BOOL_T LLDP_OM_IncreaseDcbxTimer(UI32_T lport);
#endif/* #if(SYS_CPNT_DCBX == TRUE)  */

BOOL_T LLDP_OM_GetTlvChangeDetect(UI32_T tlv);
void LLDP_OM_SetTlvChangeDetect(UI32_T tlv, BOOL_T flag);

#endif /* LLDP_OM_PRIVATE_H */
/* End of LLDP_OM_PRIVATE_H */
