/*--------------------------------------------------------------------------+ 
 * FILE NAME - nmtr_mgr.h                                                       +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file defines Network Monitor APIs and Task                +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *   arthur         07/24/2001       creation                               +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * -------------------------------------------------------------------------*/
/* NOTES:
 * 1. RFC 1493 MIB contains the following group:
 *    a) the dot1dBase   group (dot1dBridge 1)  -- implemented in sta_mgr
 *    b) the dot1dStp    group (dot1dBridge 2)  -- implemented in sta_mgr
 *    c) the dot1dSr     group (dot1dBridge 3)  -- not implemented
 *    d) the dot1dTp     group (dot1dBridge 4)  -- implemented partial, partial in amtr_mgr
 *    e) the dot1dStatic group (dot1dBridge 5)  -- implemented here
 */ 

#ifndef NMTR_MGR_H
#define NMTR_MGR_H

/*--------------------------- 
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "sys_type.h"   
#include "swdrv_type.h"
#include "nmtr_type.h"


#define NMTR_MGR_IPCMSG_TYPE_SIZE   sizeof(union NMTR_MGR_IPCMsg_Type_U)

/* TYPE DEFINITIONS  
 */
typedef struct /* the struct for MIB1493 */
{
    /* key */
    UI32_T  dot1d_tp_port; /* 1..65535 */
    
    UI32_T  dot1d_tp_port_max_info;
    UI32_T  dot1d_tp_port_in_frames;
    UI32_T  dot1d_tp_port_out_frames;
    UI32_T  dot1d_tp_port_in_discards;
} NMTR_MGR_Dot1dTpPortEntry_T;

typedef struct NMTR_MGR_DiffCounter_S
{
   UI64_T ifInOctets;           /* from SWDRV_IfTableStats_T */
   UI64_T ifInUcastPkts;        /* from SWDRV_IfTableStats_T */
   UI64_T ifInNUcastPkts;       /* from SWDRV_IfTableStats_T */
   UI64_T ifInDiscards;         /* from SWDRV_IfTableStats_T */
   UI64_T ifInMulticastPkts;    /* from SWDRV_IfXTableStats_T*/
   UI64_T ifInBroadcastPkts;    /* from SWDRV_IfXTableStats_T*/
   UI64_T ifInErrors;           /* from SWDRV_IfTableStats_T */
   UI64_T ifOutOctets;          /* from SWDRV_IfTableStats_T */
   UI64_T ifOutUcastPkts;       /* from SWDRV_IfTableStats_T */
   UI64_T ifOutNUcastPkts;      /* from SWDRV_IfTableStats_T */
   UI64_T ifOutDiscards;        /* from SWDRV_IfTableStats_T */
   UI64_T ifOutMulticastPkts;   /* from SWDRV_IfXTableStats_T*/
   UI64_T ifOutBroadcastPkts;   /* from SWDRV_IfXTableStats_T*/
   UI64_T ifOutErrors;          /* from SWDRV_IfTableStats_T */
} NMTR_MGR_DiffCounter_T;


typedef struct NMTR_MGR_Utilization_S
{  /* This currently value is xx% */
   UI32_T ifInOctets_utilization;           /* from SWDRV_IfTableStats_T */
   UI32_T ifInUcastPkts_utilization;        /* from SWDRV_IfTableStats_T */
   UI32_T ifInMulticastPkts_utilization;    /* from SWDRV_IfXTableStats_T*/
   UI32_T ifInBroadcastPkts_utilization;    /* from SWDRV_IfXTableStats_T*/
   UI32_T ifInErrors_utilization;           /* from SWDRV_IfTableStats_T */
   UI32_T ifOutOctets_utilization;
} NMTR_MGR_Utilization_T;


typedef struct NMTR_MGR_Dot1dTpHCPortEntry_S/* the struct for MIB2674 */
{
    /* key */
    UI32_T  dot1d_tp_port; /* 1..65535 */

    UI64_T  dot1d_tp_hc_port_in_frames;
    UI64_T  dot1d_tp_hc_port_out_frames;
    UI64_T  dot1d_tp_hc_port_in_discards;
} NMTR_MGR_Dot1dTpHCPortEntry_T;

typedef struct  NMTR_MGR_Dot1dTpPortOverflowEntry_S/* the struct for MIB2674 */
{
    /* key */
    UI32_T  dot1d_tp_port; /* 1..65535 */

    UI32_T  dot1d_tp_port_in_overflow_frames;
    UI32_T  dot1d_tp_port_out_overflow_frames;
    UI32_T  dot1d_tp_port_in_overflow_discards;
} NMTR_MGR_Dot1dTpPortOverflowEntry_T;

typedef struct NMTR_MGR_EtherStatsHighCapacityEntry_S/* the struct for MIB3273 */
{
    /* key */
    UI32_T  ether_stats_index;  /* Integer32, 1..65535 */

    UI32_T  ether_stats_high_capacity_overflow_pkts;
    UI64_T  ether_stats_high_capacity_pkts;
    UI32_T  ether_stats_high_capacity_overflow_octets;
    UI64_T  ether_stats_high_capacity_octets;

    UI32_T  ether_stats_high_capacity_overflow_pkts64_octets;
    UI64_T  ether_stats_high_capacity_pkts64_octets;
    UI32_T  ether_stats_high_capacity_overflow_pkts65to127_octets;
    UI64_T  ether_stats_high_capacity_pkts65to127_octets;

    UI32_T  ether_stats_high_capacity_overflow_pkts128to255_octets;
    UI64_T  ether_stats_high_capacity_pkts128to255_octets;
    UI32_T  ether_stats_high_capacity_overflow_pkts256to511_octets;
    UI64_T  ether_stats_high_capacity_pkts256to511_octets;

    UI32_T  ether_stats_high_capacity_overflow_pkts512to1023_octets;
    UI64_T  ether_stats_high_capacity_pkts512to1023_octets;
    UI32_T  ether_stats_high_capacity_overflow_pkts1024to1518_octets;
    UI64_T  ether_stats_high_capacity_pkts1024to1518_octets;
} NMTR_MGR_EtherStatsHighCapacityEntry_T;

typedef struct  NMTR_MGR_EtherHistoryHighCapacityEntry_S/* the struct for MIB3273 */
{
    /* key */
    UI32_T  ether_history_index;           /* Integer32, 1..65535 */
    UI32_T  ether_history_sample_index;  /* Integer32, 1..2147483647 */

    UI32_T  ether_history_high_capacity_overflow_pkts;
    UI64_T  ether_history_high_capacity_pkts;
    UI32_T  ether_history_high_capacity_overflow_octets;
    UI64_T  ether_history_high_capacity_octets;
} NMTR_MGR_EtherHistoryHighCapacityEntry_T;

typedef struct NMTR_MGR_Utilization_300_SECS_S
{  /* This currently value is xx% */
   UI64_T ifInOctets;
   UI64_T ifOutOctets;
   UI64_T ifInPackets;
   UI64_T ifOutPackets;
   UI32_T ifInOctets_utilization;
   UI32_T ifOutOctets_utilization;

} NMTR_MGR_Utilization_300_SECS_T;

typedef struct NMTR_MGR_Utilization_300_SECS_LAST_TIME_S
{  /* This currently value is xx% */
   UI64_T ifInOctets;
   UI64_T ifOutOctets;
   UI64_T ifInPackets;
   UI64_T ifOutPackets;
} NMTR_MGR_Utilization_300_SECS_LAST_TIME_T;

/* definitions of command in L2MUX which will be used in ipc message
 */
enum
{
    NMTR_MGR_IPC_GET_IFTABLE_STATS = 0,
    NMTR_MGR_IPC_GET_IFXTABLE_STATS,
    NMTR_MGR_IPC_GET_ETHERLIKE_STATS,
    NMTR_MGR_IPC_SET_ETHERLIKE_PAUSE_ADMIN_MODE,
    NMTR_MGR_IPC_GET_ETHERLIKE_PAUSE_STATS,
    NMTR_MGR_IPC_GET_RMON_STATS,
    NMTR_MGR_IPC_GET_IFPERQ_STATS,                                          /* SYS_CPNT_NMTR_PERQ_COUNTER */
    NMTR_MGR_IPC_GET_PFC_STATS,                                             /* SYS_CPNT_PFC */
    NMTR_MGR_IPC_GET_QCN_STATS,                                             /* SYS_CPNT_CN */
    NMTR_MGR_IPC_GET_DOT1D_TP_PORT_ENTRY,
    NMTR_MGR_IPC_GET_DOT1D_TP_HC_PORT_ENTRY,
    NMTR_MGR_IPC_GET_DOT1D_TP_PORT_OVERFLOW_ENTRY,
    NMTR_MGR_IPC_GET_PORT_UTILIZATION,
    NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_STATS,
    NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_PFC_STATS,                                /* SYS_CPNT_PFC */
    NMTR_MGR_IPC_GET_SYSTEMWIDE_IFTABLE_STATS,
    NMTR_MGR_IPC_GET_SYSTEMWIDE_IFXTABLE_STATS,
    NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_STATS,
    NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_PAUSE_STATS,
    NMTR_MGR_IPC_GET_SYSTEMWIDE_RMON_STATS,
    NMTR_MGR_IPC_GET_SYSTEMWIDE_IFPERQ_STATS,                               /* SYS_CPNT_NMTR_PERQ_COUNTER */
    NMTR_MGR_IPC_GET_SYSTEMWIDE_PFC_STATS,                                  /* SYS_CPNT_PFC */
    NMTR_MGR_IPC_GET_SYSTEMWIDE_QCN_STATS,                                  /* SYS_CPNT_CN */
    NMTR_MGR_IPC_GET_PORT_UTILIZATION_300_SECS,
    NMTR_MGR_IPC_GET_ETHER_STATS_HIGH_CAPACITY_ENTRY,
    NMTR_MGR_IPC_SET_DEFAULT_HISTORY_CTRL_ENTRY,
    NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_FIELD,
    NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC,
    NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC,
    NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_IFINDEX,
    NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY,
    NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY_BY_DATASRC,
    NMTR_MGR_IPC_GET_HISTORY_CURRENT_ENTRY,
    NMTR_MGR_IPC_GET_HISTORY_PREVIOUS_ENTRY,
    NMTR_MGR_IPC_GET_NEXT_HISTORY_PREVIOUS_ENTRY_BY_CTRL_IDX,
    NMTR_MGR_IPC_GET_NEXT_REMOVED_DEFAULT_HISTORY_CTRL_ENTRY_BY_DATASRC,
    NMTR_MGR_IPC_GET_NEXT_USER_CFG_HISTORY_CTRL_ENTRY_BY_DATASRC,
};

typedef struct
{
    union NMTR_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;      /* for sending IPC request */
        BOOL_T ret_bool; /* for response bool value */
        UI32_T result;   /* for response            */
        UI64_T ret_ui64t;
    } type;
    
    union
    {
        struct NMTR_MGR_IPCMsg_GetIfStats_Data_S
        {
            UI32_T               ifindex;
            SWDRV_IfTableStats_T if_stats;
            BOOL_T               next;
        }GetIfStats_data;
        struct NMTR_MGR_IPCMsg_GetIfXStats_Data_S
        {
            UI32_T                ifindex;
            SWDRV_IfXTableStats_T ifx_stats;
            BOOL_T                next;
        }GetIfXStats_data;
        struct NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S
        {
            UI32_T                  ifindex;
            SWDRV_EtherlikeStats_T  etherlike_stats;
            BOOL_T                  next;
        }GetEtherlikeStats_data;
        struct NMTR_MGR_IPCMsg_SetEtherlikePauseAdminMode_Data_S
        {
            UI32_T                  ifindex;
            UI32_T                  mode;
        }SetEtherlikePauseAdminMode_data;
        struct NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S
        {
            UI32_T                  ifindex;
            SWDRV_EtherlikePause_T  etherlike_pause;
            BOOL_T                  next;
        }GetEtherlikePause_data;
        struct NMTR_MGR_IPCMsg_GetRmonStats_Data_S
        {
            UI32_T              ifindex;
            SWDRV_RmonStats_T   rmon_stats;
            BOOL_T              next;
        }GetRmonStats_data;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        struct NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S
        {
            UI32_T               ifindex;
            SWDRV_IfPerQStats_T     stats;
            BOOL_T               next;
        }GetIfPerQStats_data;
#endif
#if (SYS_CPNT_PFC == TRUE)
        struct NMTR_MGR_IPCMsg_GetPfcStats_Data_S
        {
            UI32_T               ifindex;
            SWDRV_PfcStats_T     stats;
            BOOL_T               next;
        }GetPfcStats_data;
#endif
#if (SYS_CPNT_CN == TRUE)
        struct NMTR_MGR_IPCMsg_GetQcnStats_Data_S
        {
            UI32_T               ifindex;
            SWDRV_QcnStats_T     stats;
            BOOL_T               next;
        }GetQcnStats_data;
#endif
        struct NMTR_MGR_IPCMsg_GetDot1dTpPortEntry_Data_S
        {
            NMTR_MGR_Dot1dTpPortEntry_T tp_port_entry;
            BOOL_T                      next;
        }GetDot1dTpPortEntry_data;
        struct NMTR_MGR_IPCMsg_GetDot1dTpHCPortEntry_Data_S
        {
            NMTR_MGR_Dot1dTpHCPortEntry_T tp_hc_port_entry;
            BOOL_T                      next;
        }GetDot1dTpHCPortEntry_data;
        struct NMTR_MGR_IPCMsg_GetDot1dTpPortOverflowEntry_Data_S
        {
            NMTR_MGR_Dot1dTpPortOverflowEntry_T tp_port_overflow_entry;
            BOOL_T                      next;
        }GetDot1dTpPortOverflowEntry_data;        
        struct NMTR_MGR_IPCMsg_GetPortUtilization_Data_S
        {
            UI32_T                 ifindex;
            NMTR_MGR_Utilization_T utilization_entry;
            BOOL_T                 next;
        }GetPortUtilization_data;        
        struct NMTR_MGR_IPCMsg_ClearSystemwideStats_Data_S
        {
            UI32_T                 ifindex;
        }ClearSystemwideStats_data;
        struct NMTR_MGR_IPCMsg_GetPortUtilization300secs_Data_S
        {
            UI32_T                          ifindex;        
            NMTR_MGR_Utilization_300_SECS_T utilization_entry;
            BOOL_T                 next;
        }GetPortUtilization300secs_data;
        struct NMTR_MGR_IPCMsg_GetEtherStatsHighCapacityEntry_Data_S
        {
            NMTR_MGR_EtherStatsHighCapacityEntry_T es_hc_entry;
        }GetEtherStatsHighCapacityEntry_data;
        struct NMTR_MGR_IPCMsg_HistoryCtrlEntryField_Data_S
        {
            UI32_T ctrl_idx;
            UI32_T field_idx;
            union
            {
                UI32_T i;
                char s[NMTR_TYPE_HIST_CTRL_NAME_LEN_MAX+1];
            } data;
        }HistoryCtrlEntryField_data;
        struct NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S
        {
            NMTR_TYPE_HistCtrlInfo_T entry;
            BOOL_T next;
        }HistoryCtrlEntry_data;
        struct NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S
        {
            NMTR_TYPE_HistSampleEntry_T entry;
            BOOL_T next;
        }HistorySampleEntry_data;
    } data;
} NMTR_MGR_IPCMsg_T;

/* NAMING CONSTANT
 */
#define NMTR_MGR_MSGBUF_TYPE_SIZE sizeof(union NMTR_MGR_IPCMsg_Type_U)
#define NMTR_MGR_GET_MSGBUFSIZE(struct_name) \
        (NMTR_MGR_MSGBUF_TYPE_SIZE + sizeof(struct struct_name))


/*---------------------------- 
 * EXPORTED SUBPROGRAM BODIES
 *----------------------------*/
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Init                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void NMTR_MGR_Init(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Create_InterCSC_Relation                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : Invoked by NMTR_INIT_Initiate_System_Resources()                                                        
 *------------------------------------------------------------------------*/
void NMTR_MGR_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_EnterTransitionMode                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize counters                
 * INPUT   : None                                                  
 * OUTPUT  : None                                                  
 * RETURN  : None                                                  
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : NMTR_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTR_MGR_SetTransitionMode(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_EnterMasterMode                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will enable network monitor services
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterMasterMode(void);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Enter_Slave_Mode                                   
 *------------------------------------------------------------------------|
 * FUNCTION: This function will disable network monitor services          
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_MGR_HandleHotInsertion
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

 * -------------------------------------------------------------------------*/
void NMTR_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_MGR_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void NMTR_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetOperationMode                                   
 *------------------------------------------------------------------------|
 * FUNCTION: This function will return present opertaion mode        
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
UI32_T NMTR_MGR_GetOperationMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Net monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ProvisionComplete(void);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_CreateTask                                        
 *------------------------------------------------------------------------|
 * FUNCTION: This function will create network monitor task               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTR_MGR_CreateTask(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearPortCounter                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter                 
 * INPUT   : UI32_T ifindex - interface index
 * OUTPUT  : None                                                  
 * RETURN  : Boolean        - TRUE : success , FALSE: failed
 * NOTE    : For clearing the port counter after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_ClearPortCounter(UI32_T ifindex);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the all counters in whole system
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_ClearAllCounters(void);


/*-----------------
 * MIB2 Statistics 
 *-----------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the iftable statistic of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : SWDRV_IfTableStats_T   *if_table_stats - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T   *if_table_stats - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats);



/*--------------------------
 * Extended MIB2 Statistics                                 
 *--------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfXTableStats                                 
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended iftable a interface           
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfXTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats);



/*---------------------------
 * Ether-like MIB Statistics 
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats);
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats);



/*---------------------------
 * Ether-like MIB Pause Stats
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SetEtherLikePauseAdminMode
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will set the ether-like pause function
 * INPUT   : UI32_T ifindex                            - interface index
 *           mode                                      - VAL_dot3PauseAdminMode_disabled
 *                                                       VAL_dot3PauseAdminMode_enabledXmit
 *                                                       VAL_dot3PauseAdminMode_enabledRcv
 *                                                       VAL_dot3PauseAdminMode_enabledXmitAndRcv
 * OUTPUT  : None
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetEtherLikePauseAdminMode(UI32_T ifindex, UI32_T mode);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like pause statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikePause                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause);



/*---------------------
 * RMON MIB Statistics 
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics of a interface                
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats);
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index 
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats);


#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */


#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetPfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */


#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats);
#endif /* (SYS_CPNT_CN == TRUE) */


/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 4 
 *         (Port Table for Transparent Bridges)                          */
/*
 *          dot1dTpPortTable OBJECT-TYPE
 *              SYNTAX  SEQUENCE OF Dot1dTpPortEntry
 *              ::= { dot1dTp 4 }
 *          dot1dTpPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortTable 1 }
 *          Dot1dTpPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPort             INTEGER,
 *                  dot1dTpPortMaxInfo      INTEGER,
 *                  dot1dTpPortInFrames     Counter,
 *                  dot1dTpPortOutFrames    Counter,
 *                  dot1dTpPortInDiscards   Counter
 *              }
 */              
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry); 



/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- the next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetNextDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry); 


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 5 ) -- dot1dTp 5
 *          dot1dTpHCPortTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpHCPortEntry
 *              ::= { dot1dTp 5 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpHCPortTable 1 }
 *          Dot1dTpHCPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpHCPortInFrames   Counter64,
 *                  dot1dTpHCPortOutFrames  Counter64,
 *                  dot1dTpHCPortInDiscards Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry);


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 6 ) -- dot1dTp 6
 *          dot1dTpPortOverflowTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpPortOverflowEntry
 *              ::= { dot1dTp 6 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortOverflowTable 1 }
 *          Dot1dTpPortOverflowEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPortInOverflowFrames     Counter32,
 *                  dot1dTpPortOutOverflowFrames    Counter32,
 *                  dot1dTpPortInOverflowDiscards   Counter32
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry);


/* the following APIs are only for NMTR_TASK use
 */
BOOL_T NMTR_MGR_IsProvisionComplete(void);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetPortUtilization(UI32_T ifindex, NMTR_MGR_Utilization_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get next utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPortUtilization(UI32_T *ifindex, NMTR_MGR_Utilization_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwideStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide counters of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ClearSystemwideStats(UI32_T ifindex);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide PFC counters of a interface
 * INPUT   : ifindex - interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ClearSystemwidePfcStats(UI32_T ifindex);
#endif /* (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide iftable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended systemwide iftable a interface
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide pause statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 * OUTPUT  : UI32_T *ifindex                           - the next interface index
 *           SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats);

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwidePfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwidePfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats);
#endif /* (SYS_CPNT_CN == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: get the utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetPortUtilization300secs(UI32_T ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization300secs                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 * OUTPUT  : ifindex            -- interface index
 *         : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPortUtilization300secs(UI32_T *ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_PrintPortAllUtilization300secs                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
#if 0
BOOL_T NMTR_MGR_PrintPortAllUtilization300secs(UI32_T ifindex, UI32_T select);
#endif

/*-------------------------------------------------------------------------
 * The etherStatsHighCapacityGroup group (statistics 7 ) -- statistics 7
 *          etherStatsHighCapacityTable OBJECT-TYPE
 *              SYNTAX     SEQUENCE OF EtherStatsHighCapacityEntry
 *              ::= { statistics 7 }
 *          etherStatsHighCapacityEntry OBJECT-TYPE
 *              INDEX   { etherStatsIndex }
 *              ::= { etherStatsHighCapacityTable 1 }
 *          EtherStatsHighCapacityEntry ::=
 *              SEQUENCE {
 *                  etherStatsHighCapacityOverflowPkts                 Counter32,
 *                  etherStatsHighCapacityPkts                         Counter64,
 *                  etherStatsHighCapacityOverflowOctets               Counter32,
 *                  etherStatsHighCapacityOctets                       Counter64,
 *                  etherStatsHighCapacityOverflowPkts64Octets         Counter32,
 *                  etherStatsHighCapacityPkts64Octets                 Counter64,
 *                  etherStatsHighCapacityOverflowPkts65to127Octets    Counter32,
 *                  etherStatsHighCapacityPkts65to127Octets            Counter64,
 *                  etherStatsHighCapacityOverflowPkts128to255Octets   Counter32,
 *                  etherStatsHighCapacityPkts128to255Octets           Counter64,
 *                  etherStatsHighCapacityOverflowPkts256to511Octets   Counter32,
 *                  etherStatsHighCapacityPkts256to511Octets           Counter64,
 *                  etherStatsHighCapacityOverflowPkts512to1023Octets  Counter32,
 *                  etherStatsHighCapacityPkts512to1023Octets          Counter64,
 *                  etherStatsHighCapacityOverflowPkts1024to1518Octets Counter32,
 *                  etherStatsHighCapacityPkts1024to1518Octets         Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetEtherStatsHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified ether statistics
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   es_hc_entry->ether_stats_index          -- (key)
 * OUTPUT   :   es_hc_entry         -- Ether stats high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherStatsHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetEtherStatsHighCapacityEntry(NMTR_MGR_EtherStatsHighCapacityEntry_T *es_hc_entry);

BOOL_T NMTR_MGR_HandleUpdateNmtrdrvStats(UI32_T update_type,UI32_T unit, UI32_T start_port, UI32_T port_amount);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_TrunkMemberAdd1st_Callback
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when the first port is added to a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberAdd1st_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_TrunkMemberAdd_Callback
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is added to a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberAdd_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is deleted from a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberDeleteLst_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetDefaultHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetDefaultHistoryCtrlEntry(UI32_T ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetHistoryCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : ctrl_p->ctrl_idx
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_DestroyHistoryCtrlEntryByIfindex(UI32_T ifindex);
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: NMTR_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for NMTR MGR.
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
BOOL_T NMTR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#endif /* NMTR_MGR_H */

