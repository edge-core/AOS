/*-----------------------------------------------------------------------------
 * FILE NAME: PPPOE_IA_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for PPPOE_IA MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2009/11/26     --- Squid Ro, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef _PPPOE_IA_PMGR_H
#define _PPPOE_IA_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "pppoe_ia_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : PPPOE_IA_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for PPPOE_IA_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_ClearPortStatistics(
    UI32_T  lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id. (operation string)
 * INPUT  : None
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : a. access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *          b. set API - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetAccessNodeId(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message. (operation string)
 * INPUT  : None
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : 1. set API - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetGenericErrMsg(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1]);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To get global enable status
 * INPUT  : None
 * OUTPUT : is_enable_p - pointer to output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetGlobalEnable(
    BOOL_T  *is_enable_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port config entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          pcfg_p  - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetNextPortConfigEntry(
    UI32_T                          *lport_p,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetNextPortStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port statistic entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          psts_p  - pointer to statistic entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetNextPortStatisticsEntry(
    UI32_T                      *lport_p,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get boolean data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  *val_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get ui32 data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  *val_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port oper config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. will get the operation string for the port
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortOprCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get string data for specified ifindex and field id.
 * INPUT  : lport     - 1-based ifindex to get
 *          fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to get
 *          is_oper   - TRUE to get operation string
 *          str_len_p - length of input buffer
 *                      (including null terminator)
 * OUTPUT : str_p     - pointer to output string data
 *          str_len_p - length of output buffer used
 *                      (not including null terminator)
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *str_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running bool data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running ui32 data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running string data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 *          str_len_max - maximum length of buffer to receive the string
 *                        (including null terminator)
 * OUTPUT : string_p    - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetGlobalEnable(
    BOOL_T  is_enable);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set global string data for specified field id.
 * INPUT  : fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to set
 *          str_p     - pointer to input string data
 *          str_len   - length of input string data
 *                      (not including null terminator, 0 to reset to default)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetAccessNodeId/
 *                       PPPOE_IA_PMGR_GetGenericErrMsg
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetGlobalAdmStrDataByField(
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set string data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          str_p   - pointer to input string data
 *          str_len - length of input string data
 *                    (not including null terminator)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. use engine API to apply setting on trunk.
 *          2. get API - PPPOE_IA_PMGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len);

#endif /* #ifndef _PPPOE_IA_PMGR_H */

