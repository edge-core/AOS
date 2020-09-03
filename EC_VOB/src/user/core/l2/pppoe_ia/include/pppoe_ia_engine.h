/* MODULE NAME: pppoe_ia_engine.h
 * PURPOSE:
 *   Declarations of engine APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _PPPOE_IA_ENGINE_H_
#define _PPPOE_IA_ENGINE_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "pppoe_ia_type.h"
#include "pppoe_ia_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_AddFstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trk_ifidx - specify which trunk to join.
 *          mbr_ifidx - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : mbr_ifidx is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_AddFstTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_AddTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_AddTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_DelLstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_DelLstTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_DelTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_DelTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id.
 * INPUT  : None
 * OUTPUT : acc_nodeid_ar - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 *          nodeid_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetAccessNodeId(
    UI8_T   acc_nodeid_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1],
    UI8_T   *nodeid_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message.
 * INPUT  : None
 * OUTPUT : gen_ermsg_ar - pointer to output buffer
 *                         >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 *          ermsg_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetGenericErrMsg(
    UI8_T   gen_ermsg_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1],
    UI32_T  *ermsg_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. this api will fill the entry with default
 *             circuit-id/remote-id, while om api can not do this.
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetPortOprCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetPortStrDataByField
 *-------------------------------------------------------------------------
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
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *string_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE: To process the received PPPoE PDU. (PPPoE discover)
 * INPUT  : msg_p - pointer to the message get from the msg queue
 * OUTPUT : None
 * RETURN : None
 * NOTE   : called from the pppoe_ia_mgr.c
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_ProcessRcvdPDU(
    PPPOE_IA_TYPE_Msg_T     *msg_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetDefaultConfig
 * ------------------------------------------------------------------------
 * PURPOSE: To do extra work for default configuration.
 *          (e.g. setup the rules.)
 * INPUT  : start_ifidx - start ifidx for extra config (1-based)
 *          end_ifidx   - end   ifidx for extra config (1-based)
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_SetDefaultConfig(
    UI32_T  start_ifidx,
    UI32_T  end_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetGlobalEnable(
    BOOL_T  is_enable);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set string data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          str_p   - pointer to input string data
 *          str_len - length of input string data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len);

#endif /* End of _PPPOE_IA_ENGINE_H_*/

