/*-----------------------------------------------------------------------------
 * Module Name: lacp_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Header files for lacp_mgr.h
 * NOTES:

 * MODIFICATION HISTORY:
 * Modifier         Date                Description
 * -------------------------------------------------------------------------------------
 * Lewis Kang          12-05-2001       First Created
 * Amy Tu              06-29-2002       Move callback functions from Task to MGR
 *                     07-29-2002       Each dynamic trunk per speed duplex should
 *                                      have different LAG ID
 * Allen Cheng         11-07-2002       Add functions for setting MIB entry
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */

#ifndef _LACP_MGR_H
#define _LACP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "lacp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define LACP_MGR_IPCMSG_TYPE_SIZE sizeof(union LACP_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
	LACP_MGR_IPC_GETDOT3ADAGGENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADAGGENTRY,
	LACP_MGR_IPC_GETDOT3ADAGGPORTLISTENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTLISTENTRY,
	LACP_MGR_IPC_GETDOT3ADAGGPORTENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTENTRY,
	LACP_MGR_IPC_GETDOT3ADAGGPORTSTATSENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTSTATSENTRY,
	LACP_MGR_IPC_GETDOT3ADAGGPORTDEBUGENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTDEBUGENTRY,
	LACP_MGR_IPC_GETDOT3ADLAGMIBOBJECTS,
	LACP_MGR_IPC_SETDOT3ADLACPPORTENABLED,
	LACP_MGR_IPC_GETDOT3ADLACPPORTENTRY,
	LACP_MGR_IPC_GETNEXTDOT3ADLACPPORTENTRY,
	LACP_MGR_IPC_SETDOT3ADAGGACTORSYSTEMPRIORITY,
	LACP_MGR_IPC_SETDOT3ADAGGACTORADMINKEY,
        LACP_MGR_IPC_SETDEFAULTDOT3ADAGGACTORADMINKEY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORADMINKEY,
	LACP_MGR_IPC_SETDOT3ADAGGCOLLECTORMAXDELAY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTORSYSTEMPRIORITY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORSYSTEMPRIORITY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINKEY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORADMINKEY,
	LACP_MGR_IPC_SETDEFAULTDOT3ADAGGPORTACTORADMINKEY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTOROPERKEY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMID,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINKEY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINKEY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTORPORTPRIORITY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORPORTPRIORITY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORT,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORTPRIORITY,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINPORTPRIORITY,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINSTATE,
	LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSTATE,
	LACP_MGR_IPC_SETDOT3ADAGGPORTACTORLACP_TIMEOUT,
	LACP_MGR_IPC_GETDOT3ADAGGPORTACTORLACP_TIMEOUT,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORLACP_TIMEOUT,
	LACP_MGR_IPC_SETDOT3ADAGGACTORLACP_TIMEOUT,
	LACP_MGR_IPC_GETDOT3ADAGGACTORLACP_TIMEOUT,
	LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORLACP_TIMEOUT
};

/* For Exceptional Handler */
enum LACP_MGR_FUN_NO_E
{
    LACP_MGR_IsLeagalifIndex_Fun_No         = 1,
    LACP_MGR_GetNextDot3adAggEntry__Fun_No,
    LACP_MGR_GetDot3adAggEntry__Fun_No,
    LACP_MGR_GetNextDot3adAggPortListEntry__Fun_No,
    LACP_MGR_GetDot3adAggPortListEntry__Fun_No,
    LACP_MGR_GetNextDot3adAggPortEntry__Fun_No,
    LACP_MGR_GetDot3adAggPortEntry__Fun_No,
    LACP_MGR_GetNextDot3adAggPortStatsEntry__Fun_No,
    LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No,
    LACP_MGR_GetNextDot3adAggPortDebugEntry__Fun_No,
    LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No,
    LACP_MGR_GetDot3adLagMibObjects__Fun_No,
    LACP_MGR_SetLACPState_Fun_No,
    LACP_MGR_GetLACPState_Fun_No,
    LACP_MGR_SetDot3adLacpPortEnabled__Fun_No,
    LACP_MGR_GetNextDot3adLacpPortEntry__Fun_No,
    LACP_MGR_GetDot3adLacpPortEntry__Fun_No
};

#define LACP_PORT_LIST_OCTETS   ((((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)*(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+7)/8)
#define MUX_REASON_LENGTH       64


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in LACP_MGR_IpcMsg_T.data
 */
#define LACP_MGR_GET_MSG_SIZE(field_name)                       \
            (LACP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((LACP_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T          dot3ad_tables_last_changed;
} LACP_MGR_LagMibObjects_T;

typedef struct
{
    UI32_T          dot3ad_agg_index;
    UI8_T           dot3ad_agg_mac_address[6];
    UI16_T          dot3ad_agg_actor_system_priority;
    UI8_T           dot3ad_agg_actor_system_id[6];
    UI8_T           dot3ad_agg_aggregate_or_individual;
    UI32_T          dot3ad_agg_actor_admin_key;
    UI32_T          dot3ad_agg_actor_oper_key;
    UI8_T           dot3ad_agg_partner_system_id[6];
    UI16_T          dot3ad_agg_partner_system_priority;
    UI32_T          dot3ad_agg_partner_oper_key;
    UI32_T          dot3ad_agg_collector_max_delay;
    UI32_T          dot3ad_agg_actor_timeout;
} LACP_MGR_Dot3adAggEntry_T;

typedef struct
{
    UI32_T          dot3ad_agg_index;       /* Index of the aggregator id.                   */
                                            /* Augment of previous LACP_MGR_Dot3adAggEntry_T */
    UI8_T           dot3ad_agg_port_list_ports[LACP_PORT_LIST_OCTETS];
} LACP_MGR_Dot3adAggPortListEntry_T;

typedef struct
{
    UI32_T          dot3ad_agg_port_index;
    UI16_T          dot3ad_agg_port_actor_system_priority;
    UI8_T           dot3ad_agg_port_actor_system_id[6];
    UI16_T          dot3ad_agg_port_actor_admin_key;
    UI16_T          dot3ad_agg_port_actor_oper_key;
    UI16_T          dot3ad_agg_port_partner_admin_system_priority;
    UI16_T          dot3ad_agg_port_partner_oper_system_priority;
    UI8_T           dot3ad_agg_port_partner_admin_system_id[6];
    UI8_T           dot3ad_agg_port_partner_oper_system_id[6];
    UI16_T          dot3ad_agg_port_partner_admin_key;
    UI16_T          dot3ad_agg_port_partner_oper_key;
    UI32_T          dot3ad_agg_port_selected_agg_id;
    UI32_T          dot3ad_agg_port_attached_agg_id;
    UI16_T          dot3ad_agg_port_actor_port;
    UI16_T          dot3ad_agg_port_actor_port_priority;
    UI16_T          dot3ad_agg_port_partner_admin_port;
    UI16_T          dot3ad_agg_port_partner_oper_port;
    UI16_T          dot3ad_agg_port_partner_admin_port_priority;
    UI16_T          dot3ad_agg_port_partner_oper_port_priority;
    UI8_T           dot3ad_agg_port_actor_admin_state;
    UI8_T           dot3ad_agg_port_actor_oper_state;
    UI8_T           dot3ad_agg_port_partner_admin_state;
    UI8_T           dot3ad_agg_port_partner_oper_state;
    UI8_T           dot3ad_agg_port_aggregate_or_individual;
} LACP_MGR_Dot3adAggPortEntry_T;

typedef struct
{
    UI32_T          dot3ad_agg_port_index; /* Augments of LACP_MGR_Dot3adAggPortEntry_T */
    UI32_T          dot3ad_agg_port_stats_lac_pdus_rx;
    UI32_T          dot3ad_agg_port_stats_marker_pdus_rx;
    UI32_T          dot3ad_agg_port_stats_marker_response_pdus_rx;
    UI32_T          dot3ad_agg_port_stats_unknown_rx;
    UI32_T          dot3ad_agg_port_stats_illegal_rx;
    UI32_T          dot3ad_agg_port_stats_lac_pdus_tx;
    UI32_T          dot3ad_agg_port_stats_marker_pdus_tx;
    UI32_T          dot3ad_agg_port_stats_marker_response_pdus_tx;
} LACP_MGR_Dot3adAggPortStatsEntry_T;

typedef struct
{
    UI32_T          dot3ad_agg_port_index; /* Augments of LACP_MGR_Dot3adAggPortEntry_T */
    UI8_T           dot3ad_agg_port_debug_rx_state;
    UI32_T          dot3ad_agg_port_debug_last_rx_time;
    UI8_T           dot3ad_agg_port_debug_mux_state;
    UI8_T           dot3ad_agg_port_debug_mux_reason[MUX_REASON_LENGTH];
    UI8_T           dot3ad_agg_port_debug_actor_churn_state;
    UI8_T           dot3ad_agg_port_debug_partner_churn_state;
    UI32_T          dot3ad_agg_port_debug_actor_churn_count;
    UI32_T          dot3ad_agg_port_debug_partner_churn_count;
    UI32_T          dot3ad_agg_port_debug_actor_sync_transition_count;
    UI32_T          dot3ad_agg_port_debug_partner_sync_transition_count;
    UI32_T          dot3ad_agg_port_debug_actor_change_count;
    UI32_T          dot3ad_agg_port_debug_partner_change_count;
} LACP_MGR_Dot3adAggPortDebugEntry_T;


typedef struct
{
    UI32_T          dot3ad_lacp_port_index;
    UI16_T          dot3ad_lacp_port_status;
} LACP_MGR_Dot3adLacpPortEntry_T;

/* IPC message structure
 */
typedef struct
{
	union LACP_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
        UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
	    UI16_T                             arg_ui16;
	    LACP_MGR_Dot3adAggEntry_T          arg_agg_entry;
	    LACP_MGR_Dot3adAggPortListEntry_T  arg_agg_port_list_entry;
	    LACP_MGR_Dot3adAggPortEntry_T      arg_agg_port_entry;
	    LACP_MGR_Dot3adAggPortStatsEntry_T arg_agg_port_stats_entry;
	    LACP_MGR_Dot3adAggPortDebugEntry_T arg_agg_port_debug_entry;
	    LACP_MGR_LagMibObjects_T           arg_lag_mib_objects;
	    LACP_MGR_Dot3adLacpPortEntry_T     arg_lacp_port_entry;
	    struct
	    {
	        UI32_T arg1;
	        UI32_T arg2;
	    } arg_grp1;
	    struct
	    {
	        UI16_T arg1;
	        UI16_T arg2;
	    } arg_grp2;
	    struct
	    {
	        UI16_T arg1;
	        UI32_T arg2;
	    } arg_grp3;
	    struct
	    {
	        UI16_T arg1;
	        UI8_T  arg2[6]; /* dot3ad_agg_port_partner_admin_system_id */
	    } arg_grp4;
	    struct
	    {
	        UI16_T arg1;
	        UI8_T  arg2;
	    } arg_grp5;
	    struct
	    {
	        UI32_T arg1;
	        UI32_T arg2;
	        UI32_T arg3;
	    } arg_grp6;
	} data; /* the argument(s) for the function corresponding to cmd */
} LACP_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for LACP.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable lacp operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the lacp operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void LACP_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  LACP_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T  LACP_MGR_GetOperationMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Information
 *------------------------------------------------------------------------
 * FUNCTION: This function shows LACP information on a port
 * INPUT   : unit    : unit number
 *           port    : port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
/* void LACP_MGR_ShowInformation( UI32_T unit, UI32_T port);*/

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function will Enable/Disable LACP debug information show
 *           on screen or not.
 * INPUT   : enable  : flag to enable/disable debug messages
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_MGR_EnableDebugMessage( BOOL_T enable);


void    LACP_MGR_BackDoorInfo(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified Agg entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port list entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   agg_port_entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLagMibObjects
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified lag mib objects
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adLagMibObjects(LACP_MGR_LagMibObjects_T *lag_mib_objects);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set LACP on port to be enable or disable.
 * INPUT    :   UI32_T lacp_state         -- VAL_lacpPortStatus_enabled or VAL_lacpPortStatus_disabled (defined in Leaf_es3626a.h)
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_OK           -- set successfully
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adLacpPortEnabled( UI32_T ifindex, UI32_T lacp_state);

/* The following is a system-wised operation, not a configure value */
/*UI32_T  LACP_MGR_SetDot3adLacpEnabled(UI32_T bEnabled); */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextRunningDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next port lacp information.
 * INPUT    :   UI32_T *lport_ptr   -- the pointer of the lport number
 * OUTPUT   :   UI32_T *lacp_state     -- pointer of the value
 *                                     VAL_LacpStuts_enable or VAL_LacpStuts_disable (defined in swctrl.h)
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- Successfully
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL    -- End of the port list
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. If the next available port is available, the *lport_ptr
 *                 will be updated.
 *-------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetNextRunningDot3adLacpPortEnabled(UI32_T *ifindex, UI32_T *lacp_state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next lacp port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetNextDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);



/* ======================================================================== */
/* Functions for setting MIB entry */

/* Dot3adAggEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  priority    -- the dot3ad_agg_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  admin_key   -- the dot3ad_agg_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggActorAdminKey(UI16_T agg_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDefaultDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information to default.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDefaultDot3adAggActorAdminKey(UI16_T agg_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggActorAdminKey(UI16_T agg_index, UI16_T *admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  max_delay   -- the dot3ad_agg_collector_max_delay value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T max_delay);

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* Dot3adAggPortEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T *admin_key);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDefaultDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the dot3ad_agg_port_actor_admin_key to default value.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDefaultDot3adAggPortActorAdminKey(UI16_T port_index);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  oper_key    -- the dot3ad_agg_port_actor_oper_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorOperKey(UI16_T port_index, UI16_T oper_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *oper_key   -- the dot3ad_agg_port_actor_oper_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorOperKey(UI16_T port_index, UI16_T *oper_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_partner_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T *admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_port  -- the dot3ad_agg_port_partner_admin_port value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T admin_port);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_actor_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorAdminState(UI16_T port_index, UI8_T admin_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_partner_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T admin_state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetSystemPtr
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : lport -- lport number
 * OUTPUT   : None
 * RETUEN   : Pointer to the system variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_SYSTEM_T*   LACP_MGR_GetSystemPtr(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_SetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_GetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_lacp_timeout information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout(UI16_T port_index, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the lacp port channel with long term or short term timeout.
 * INPUT    :   UI16_T agg_index -- the dot3ad_agg_index
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_SetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the lacp port channel with long term or short term timeout
 * INPUT    :   UI16_T agg_index  -- the dot3ad_agg_index
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_GetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_lacp_timeout information.
 * INPUT    :   UI16_T agg_index  -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_HandleHotInsertion
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
void LACP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_HandleHotRemoval
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
void LACP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: LACP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LACP MGR.
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
BOOL_T LACP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

void    LACP_MGR_PortDuplexChange_CallBack(UI32_T unit, UI32_T port, UI32_T speed_duplex);
void    LACP_MGR_PortLinkup_CallBack(UI32_T unit, UI32_T port);
void    LACP_MGR_PortLinkdown_CallBack(UI32_T unit, UI32_T port);
void    LACP_MGR_PortAdminEnable_CallBack(UI32_T unit, UI32_T port);
void    LACP_MGR_PortAdminDisable_CallBack(UI32_T unit, UI32_T port);

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
void    LACP_MGR_AddStaticTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);
void    LACP_MGR_DelStaticTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TASK_TimerTick
 *------------------------------------------------------------------------
 * FUNCTION: This function is what have to do in timer tick.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_TASK_TimerTick(void);

/* created for replacing lacp_task */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_LacpduRcvd_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : Whenever Network Interface received a LACPDU packet,it calls
 *            this function to handle the packet.
 * INPUT    :
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * RETURN   :   none
 * NOTE:
 *      This function is called at interrupt time, so it need to be fast.
 *      To reduce the processing time, this function just create a message
 *      that contain the buffer pointer and other packet information. Then
 *      it sends the message to the packet queue. And it sends an event to
 *      the LACP task to notify the arrival of the LACP packet. The LACP task
 *      will handle the packet after it get out the message from the packet
 *      message queue. It is the receiving routine's responsibility to call
 *      the L_MM_Mref_Release() function to free the receiving buffer.
 *-------------------------------------------------------------------------
 */
void LACP_MGR_LacpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T              dst_mac[6],
                                  UI8_T              src_mac[6],
                                  UI16_T             tag_info,
                                  UI16_T             type,
                                  UI32_T             pkt_length,
                                  UI32_T             unit_no,
                                  UI32_T             port_no);


#endif /* _LACP_MGR_H */
