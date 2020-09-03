/*-----------------------------------------------------------------------------
 * MODULE NAME: DCBX_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of DCBX OM which are only used by DCBX.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */

#ifndef DCBX_OM_PRIVATE_H
#define DCBX_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_sort_lst.h"
#include "dcbx_om.h"
#include "dcbx_type.h"
#include "lldp_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T  sm_state;
}DCBX_OM_AsymSmEntry_T;

typedef struct
{
    UI8_T  sm_state;
}DCBX_OM_SymSmEntry_T;

/*
DCBX_OM_SystemOperEntry_T
*/
typedef struct
{
    BOOL_T  is_cfg_src_selected;
    BOOL_T  is_manual_cfg_src;
    UI32_T  cfg_src_ifindex;
}DCBX_OM_SystemOperEntry_T;

/*
DCBX_OM_PortConfigEntry_T
*/
typedef struct
{
    /* configuration status */
    UI32_T  lport_num;
    BOOL_T  port_status;          /* R/W */
    UI32_T  port_mode;            /* R/W */
    UI8_T   ets_sm_state;
    UI8_T   pfc_sm_state;
    BOOL_T  is_peer_detected;
}DCBX_OM_PortConfigEntry_T;

typedef struct
{
    UI32_T  lport;
    BOOL_T  is_delete;
    BOOL_T  rem_recommend_rcvd;
    BOOL_T  rem_willing;
    BOOL_T  rem_cbs;
    UI8_T   rem_max_tc;
    UI8_T   rem_con_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_con_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_con_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
    UI8_T   rem_recom_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_recom_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_recom_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
} DCBX_OM_RemEtsEntry_T;

typedef struct
{
    UI32_T   lport;
    BOOL_T  is_delete;
    UI8_T  rem_mac[6];
    BOOL_T  rem_willing;
    BOOL_T  rem_mbc;
    UI8_T  rem_pfc_cap;
    UI8_T  rem_pfc_enable;
} DCBX_OM_RemPfcEntry_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : DCBX_OM_PortConfigEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
DCBX_OM_PortConfigEntry_T* DCBX_OM_GetPortConfigEntryPtr(UI32_T lport) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetSysOper
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system operation
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : DCBX_OM_SystemOperEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
DCBX_OM_SystemOperEntry_T* DCBX_OM_GetSysOper(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_Init(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for DCBX objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_InitSemaphore(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is raised to
 *              SYS_BLD_RAISE_TO_HIGH_PRIORITY.
 *-------------------------------------------------------------------------
 */
void  DCBX_OM_EnterCriticalSection(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to the original
 *              task priority.
 *-------------------------------------------------------------------------
 */
void  DCBX_OM_LeaveCriticalSection(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_ResetAll
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_ResetAll() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_ResetPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_ResetPort(UI32_T   lport) ;


#endif /* DCBX_OM_PRIVATE_H */
/* End of DCBX_OM_PRIVATE_H */
