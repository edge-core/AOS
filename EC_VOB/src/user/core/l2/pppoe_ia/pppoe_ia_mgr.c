/* MODULE NAME: pppoe_ia_mgr.c
 * PURPOSE:
 *   Definitions of MGR APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"

#include "pppoe_ia_type.h"
#include "pppoe_ia_om.h"
#include "pppoe_ia_mgr.h"
#include "pppoe_ia_engine.h"
#include "pppoe_ia_backdoor.h"
#include "swctrl.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "stktplg_mgr.h"
#include "backdoor_mgr.h"
#include "vlan_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#define PPPOE_IA_MGR_LOCK()
#define PPPOE_IA_MGR_UNLOCK()

#define SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE()
#define SYSFUN_USE_CSC(ret)
#define SYSFUN_RELEASE_CSC()

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */


SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_AddFstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trunk_ifindex  - specify which trunk to join.
 *          member_ifindex - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_ENGINE_AddFstTrkMbr(trunk_ifindex, member_ifindex);
    PPPOE_IA_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_AddTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex  - specify which trunk to join to
 *          member_ifindex - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_ENGINE_AddTrkMbr(trunk_ifindex, member_ifindex);
    PPPOE_IA_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_DelLstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_ENGINE_DelLstTrkMbr(trunk_ifindex, member_ifindex);
    PPPOE_IA_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_DelTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_ENGINE_DelTrkMbr(trunk_ifindex, member_ifindex);
    PPPOE_IA_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PPPoE IA operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_OM_ResetToDefault();
    PPPOE_IA_ENGINE_SetDefaultConfig(1, SYS_ADPT_TOTAL_NBR_OF_LPORT);
    PPPOE_IA_MGR_UNLOCK();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PPPOE IA operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    PPPOE_IA_MGR_LOCK();
    PPPOE_IA_OM_ClearOM();
    PPPOE_IA_MGR_UNLOCK();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PPPOE_IA_MGR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("PPPOE_IA",
        SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY, PPPOE_IA_BACKDOOR_Main);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_ClearPortStatistics(
    UI32_T  lport)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_ClearPortStatistics(lport);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id. (operation string)
 * INPUT  : is_oper   - TRUE to get operation access node id
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : a. access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *          b. set API - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetAccessNodeId(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1])
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        if (TRUE == is_oper)
        {
            UI8_T   buf_len;

            ret = PPPOE_IA_ENGINE_GetAccessNodeId(outbuf_ar, &buf_len);
        }
        else
        {
            I32_T   buf_len = PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1;

            ret = PPPOE_IA_OM_GetAdmStrDataByField(
                        0,           PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID,
                        &buf_len, outbuf_ar);
        }
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message. (operation string)
 * INPUT  : is_oper   - TRUE to get operation generic error message
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : 1. set API - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetGenericErrMsg(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1])
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();

        if (TRUE == is_oper)
        {
            UI32_T  buf_len = PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1;

            ret = PPPOE_IA_ENGINE_GetGenericErrMsg(outbuf_ar, &buf_len);
        }
        else
        {
            I32_T   buf_len = PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1;

            ret = PPPOE_IA_OM_GetAdmStrDataByField(
                        0,           PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG,
                        &buf_len, outbuf_ar);
        }
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To get global enable status
 * INPUT  : None
 * OUTPUT : is_enable_p - pointer to output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetGlobalEnable(
    BOOL_T  *is_enable_p)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_GetBoolDataByField(
                    0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, is_enable_p);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetNextPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port config entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          pcfg_p  - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetNextPortOprCfgEntry(
    UI32_T                          *lport_p,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p)
{
    UI32_T  nxt_lport;
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "");

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(NULL != lport_p) && (NULL != pcfg_p)
       )
    {
        SYSFUN_USE_CSC(ret);
        nxt_lport = *lport_p;
        while (nxt_lport < SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            nxt_lport++;
            PPPOE_IA_MGR_LOCK();
            /* use engine api to fill the default circuit-id/remote-id
             */
            ret = PPPOE_IA_ENGINE_GetPortOprCfgEntry(nxt_lport, pcfg_p);
            PPPOE_IA_MGR_UNLOCK();

            if (TRUE == ret)
            {
                *lport_p = nxt_lport;
                break;
            }
        }
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetNextPortStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port statistic entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          psts_p  - pointer to statistic entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetNextPortStatisticsEntry(
    UI32_T                      *lport_p,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p)
{
    UI32_T  nxt_lport;
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(NULL != lport_p) && (NULL != psts_p)
       )
    {
        SYSFUN_USE_CSC(ret);
        nxt_lport = *lport_p;
        while (nxt_lport < SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            nxt_lport++;

            if (TRUE == SWCTRL_LogicalPortExisting(nxt_lport))
            {
                PPPOE_IA_MGR_LOCK();
                ret = PPPOE_IA_OM_GetPortStatisticsEntry(nxt_lport, psts_p);
                PPPOE_IA_MGR_UNLOCK();

                if (TRUE == ret)
                {
                    *lport_p = nxt_lport;
                    break;
                }
            }
        }
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T PPPOE_IA_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortStrDataByField
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
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *str_p)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(0 < lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_ENGINE_GetPortStrDataByField(
                    lport, fld_id, is_oper, str_len_p, str_p);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get boolean data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  *val_p)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        if (SWCTRL_LogicalPortExisting(lport))
        {
            SYSFUN_USE_CSC(ret);
            PPPOE_IA_MGR_LOCK();
            ret = PPPOE_IA_OM_GetBoolDataByField(lport, fld_id, val_p);
            PPPOE_IA_MGR_UNLOCK();
            SYSFUN_RELEASE_CSC();
        }
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get ui32 data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  *val_p)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        if (SWCTRL_LogicalPortExisting(lport))
        {
            SYSFUN_USE_CSC(ret);
            PPPOE_IA_MGR_LOCK();
            ret = PPPOE_IA_OM_GetUi32DataByField(lport, fld_id, val_p);
            PPPOE_IA_MGR_UNLOCK();
            SYSFUN_RELEASE_CSC();
        }
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. will get the operation string for the port
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortOprCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_ENGINE_GetPortOprCfgEntry(lport, pcfg_p);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(TRUE == SWCTRL_LogicalPortExisting(lport))
       )
    {
        if (SWCTRL_LogicalPortExisting(lport))
        {
            SYSFUN_USE_CSC(ret);
            PPPOE_IA_MGR_LOCK();
            ret = PPPOE_IA_OM_GetPortStatisticsEntry(lport, psts_p);
            PPPOE_IA_MGR_UNLOCK();
            SYSFUN_RELEASE_CSC();
        }
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningBoolDataByField
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
UI32_T PPPOE_IA_MGR_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_GetRunningBoolDataByField(
                lport, field_id, bool_flag_p);
        PPPOE_IA_MGR_UNLOCK();
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningUi32DataByField
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
UI32_T PPPOE_IA_MGR_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_GetRunningUi32DataByField(
                lport, field_id, ui32_data_p);
        PPPOE_IA_MGR_UNLOCK();
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningStrDataByField
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
UI32_T PPPOE_IA_MGR_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_GetRunningStrDataByField(
                lport, field_id, str_len_max, string_p);
        PPPOE_IA_MGR_UNLOCK();
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      inserted
 *          in_number_of_port        -- the number of ports on the inserted
 *                                      module
 *          in_use_default           -- the flag indicating the default
 *                                      configuration is used without further
 *                                      provision applied; TRUE if a new module
 *                                      different from the original one is
 *                                      inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_HandleHotInsertion(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port,
    BOOL_T  in_use_default)
{
    UI32_T  ifindex;
    UI32_T  end_ifindex = in_starting_port_ifindex+in_number_of_port-1;

    /* 1. for global setting,   need to re-config the driver setting
     *
     * 2. for per port setting, cli will re-provision the setting
     *                          when inserting back
     * 3. is_use_default is used for cli provision,
     *      core layer should always reset the config to default
     */
    PPPOE_IA_MGR_LOCK();

    for (ifindex = in_starting_port_ifindex;
         ifindex<= end_ifindex;
         ifindex++)
    {
        PPPOE_IA_OM_ResetOnePortToDefault(ifindex);
    }

    PPPOE_IA_ENGINE_SetDefaultConfig(in_starting_port_ifindex, end_ifindex);
    PPPOE_IA_MGR_UNLOCK();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      removed
 *          in_number_of_port        -- the number of ports on the removed
 *                                      module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_HandleHotRemoval(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port)
{
    UI32_T  ifindex;

    /* 1. for global setting,   do nothing
     *                          (bcz cli will not record it)
     * 2. for per port setting, clear the om
     */
    PPPOE_IA_MGR_LOCK();
    for (ifindex = in_starting_port_ifindex;
         ifindex<= in_starting_port_ifindex+in_number_of_port-1;
         ifindex++)
    {
        PPPOE_IA_OM_DelPortFromPorts(ifindex);
        PPPOE_IA_OM_ClearPortConfig(ifindex);
        PPPOE_IA_OM_ClearPortStatistics(ifindex);
    }
    PPPOE_IA_MGR_UNLOCK();
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: PPPOE_IA_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for PPPOE_IA MGR.
 * INPUT   : msgbuf_p -- input request ipc message buffer
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 * RETURN  : TRUE  - there is a  response required to be sent
 *           FALSE - there is no response required to be sent
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    PPPOE_IA_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *) msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "cmd-%ld", msg_p->type.cmd);

    /* dispatch IPC message and call the corresponding PPPOE_IA_MGR function
     */
    switch (msg_p->type.cmd)
    {
    case PPPOE_IA_MGR_IPC_CLEAR_PORT_STATISTICS:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_ClearPortStatistics(
            msg_p->data.ui32_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_ACCESS_NODE_ID:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetAccessNodeId(
            msg_p->data.bool_str_data.bool_data,
            msg_p->data.bool_str_data.str_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_GENERIC_ERR_MSG:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetGenericErrMsg(
            msg_p->data.bool_str_data.bool_data,
            msg_p->data.bool_str_data.str_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_GLOBAL_ENABLE:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetGlobalEnable(
            msg_p->data.str_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_NEXT_PORT_OPRCFG_ENTRY:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetNextPortOprCfgEntry(
            &msg_p->data.lport_cfg_data.lport,
            &msg_p->data.lport_cfg_data.pcfg_entry);
        break;
    case PPPOE_IA_MGR_IPC_GET_NEXT_PORT_STATS_ENTRY:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetNextPortStatisticsEntry(
            &msg_p->data.lport_sts_data.lport,
            &msg_p->data.lport_sts_data.psts_entry);
        break;
    case PPPOE_IA_MGR_IPC_GET_PORT_BOOL_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetPortBoolDataByField(
            msg_p->data.lport_fld_bool_data.lport,
            msg_p->data.lport_fld_bool_data.fld_id,
            &msg_p->data.lport_fld_bool_data.bool_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_PORT_UI32_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetPortUi32DataByField(
            msg_p->data.lport_fld_ui32_data.lport,
            msg_p->data.lport_fld_ui32_data.fld_id,
            &msg_p->data.lport_fld_ui32_data.ui32_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_PORT_OPRCFG_ENTRY:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetPortOprCfgEntry(
            msg_p->data.lport_cfg_data.lport,
            &msg_p->data.lport_cfg_data.pcfg_entry);
        break;
    case PPPOE_IA_MGR_IPC_GET_PORT_STATS_ENTRY:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetPortStatisticsEntry(
            msg_p->data.lport_sts_data.lport,
            &msg_p->data.lport_sts_data.psts_entry);
        break;
    case PPPOE_IA_MGR_IPC_GET_PORT_STR_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
        msg_p->type.ret_bool = PPPOE_IA_MGR_GetPortStrDataByField(
            msg_p->data.lport_fld_str_data.lport,
            msg_p->data.lport_fld_str_data.fld_id,
            msg_p->data.lport_fld_str_data.is_oper,
            &msg_p->data.lport_fld_str_data.str_len_max,
            msg_p->data.lport_fld_str_data.str_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_RUNNING_BOOL_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
        msg_p->type.ret_ui32 = PPPOE_IA_MGR_GetRunningBoolDataByField(
            msg_p->data.lport_fld_bool_data.lport,
            msg_p->data.lport_fld_bool_data.fld_id,
            &msg_p->data.lport_fld_bool_data.bool_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_RUNNING_UI32_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
        msg_p->type.ret_ui32 = PPPOE_IA_MGR_GetRunningUi32DataByField(
            msg_p->data.lport_fld_ui32_data.lport,
            msg_p->data.lport_fld_ui32_data.fld_id,
            &msg_p->data.lport_fld_ui32_data.ui32_data);
        break;
    case PPPOE_IA_MGR_IPC_GET_RUNNING_STR_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
        msg_p->type.ret_ui32 = PPPOE_IA_MGR_GetRunningStrDataByField(
            msg_p->data.lport_fld_str_data.lport,
            msg_p->data.lport_fld_str_data.fld_id,
            (UI32_T) msg_p->data.lport_fld_str_data.str_len_max,
            msg_p->data.lport_fld_str_data.str_data);
        break;
    case PPPOE_IA_MGR_IPC_SET_GLOBAL_ENABLE:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_SetGlobalEnable(
            msg_p->data.bool_data);
        break;
    case PPPOE_IA_MGR_IPC_SET_GLOBAL_ADM_STR_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_SetGlobalAdmStrDataByField(
            msg_p->data.lport_fld_str_data.fld_id,
            msg_p->data.lport_fld_str_data.str_data,
            (UI32_T) msg_p->data.lport_fld_str_data.str_len_max);
        break;
    case PPPOE_IA_MGR_IPC_SET_PORT_BOOL_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_SetPortBoolDataByField(
            msg_p->data.lport_fld_bool_data.lport,
            msg_p->data.lport_fld_bool_data.fld_id,
            msg_p->data.lport_fld_bool_data.bool_data);
        break;
    case PPPOE_IA_MGR_IPC_SET_PORT_UI32_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_SetPortUi32DataByField(
            msg_p->data.lport_fld_ui32_data.lport,
            msg_p->data.lport_fld_ui32_data.fld_id,
            msg_p->data.lport_fld_ui32_data.ui32_data);
        break;
    case PPPOE_IA_MGR_IPC_SET_PORT_ADM_STR_DATA_BY_FLD:
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
        msg_p->type.ret_bool = PPPOE_IA_MGR_SetPortAdmStrDataByField(
            msg_p->data.lport_fld_str_data.lport,
            msg_p->data.lport_fld_str_data.fld_id,
            msg_p->data.lport_fld_str_data.str_data,
            (UI32_T) msg_p->data.lport_fld_str_data.str_len_max);
        break;

    default:
        SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of PPPOE_IA_MGR_HandleIPCReqMsg */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PPPOE_IA.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_InitiateSystemResources(void)
{
#if 0
    if( SYSFUN_CreateSem (1, SYSFUN_SEM_FIFO, &pppoe_ia_mgr_sem_id) != SYSFUN_OK)
    {
        printf("%s : Create Semaphore Failed.\r\n", __FUNCTION__);
    }
#endif

    PPPOE_IA_OM_InitSemaphore();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE: To process the received PPPoE PDU. (PPPoE discover)
 * INPUT  : msg_p - pointer to the message get from the msg queue
 * OUTPUT : None
 * RETURN : None
 * NOTE   : call from pppoe_ia_task
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_ProcessRcvdPDU(
    PPPOE_IA_TYPE_Msg_T  *pppoe_ia_msg_p)
{
    BOOL_T  global_status;
#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
    BOOL_T  port_status;
#endif

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_RX_PKT_FLOW, "");

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        ||(NULL == pppoe_ia_msg_p)
        ||(NULL == pppoe_ia_msg_p->mem_ref_p)
        ||(NULL == pppoe_ia_msg_p->pkt_hdr_p))
    {
        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_RX_PKT_FLOW, "");
        return;
    }

    /*check the port exist or trunk member*/
    if (  (FALSE == SWCTRL_LogicalPortExisting(pppoe_ia_msg_p->pkt_hdr_p->lport))
#if (SYS_CPNT_MGMT_PORT == TRUE)
        || (TRUE == SWCTRL_IsManagementPort(pppoe_ia_msg_p->pkt_hdr_p->lport))
#endif
       )
    {
        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_RX_PKT_FLOW,
            "logical port %ld doesn't exist", pppoe_ia_msg_p->pkt_hdr_p->lport);
        return;
    }

    /*check the vlan exit, not implement*/
    if(FALSE == VLAN_OM_IsVlanExisted(pppoe_ia_msg_p->pkt_hdr_p->tag_info & 0x0fff))
    {
        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_RX_PKT_FLOW,
            "vlan %d does not exist", pppoe_ia_msg_p->pkt_hdr_p->tag_info&0x0fff);
        return;
    }

    /*process the PDU*/
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PPPOE_IA_MGR_LOCK();

    if (  (TRUE == PPPOE_IA_OM_GetBoolDataByField(0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, &global_status))
        &&(TRUE == global_status)
#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
        &&(TRUE == PPPOE_IA_OM_GetBoolDataByField(pppoe_ia_msg_p->pkt_hdr_p->lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &port_status))
        &&(TRUE == port_status)
#endif
       )
    {
        PPPOE_IA_ENGINE_ProcessRcvdPDU(pppoe_ia_msg_p);
    }
    else
    {
        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_RX_PKT_FLOW,
            "global or port not enabled %ld", pppoe_ia_msg_p->pkt_hdr_p->lport);
    }

    PPPOE_IA_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetGlobalEnable(
    BOOL_T  is_enable)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "is_enable: %d", is_enable);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_ENGINE_SetGlobalEnable(is_enable);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set global string data for specified field id.
 * INPUT  : fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to set
 *          str_p     - pointer to input string data
 *          str_len   - length of input string data
 *                      (not including null terminator, 0 to reset to default)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetAccessNodeId/
 *                       PPPOE_IA_MGR_GetGenericErrMsg
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetGlobalAdmStrDataByField(
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "fld/str-len: %ld/%ld", fld_id, str_len);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_OM_SetAdmStrDataByField(0, fld_id, str_p, str_len);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_ENGINE_SetPortBoolDataByField(
                    lport, fld_id, new_val);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val)
{
    BOOL_T  ret = FALSE;

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        switch (fld_id)
        {
        case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
            if (  (PPPOE_IA_TYPE_MIN_DELIM_ASCII <= new_val)
                &&(new_val <= PPPOE_IA_TYPE_MAX_DELIM_ASCII)
               )
               ret = TRUE;
        default:
            break;
        }

        if (TRUE == ret)
        {
            SYSFUN_USE_CSC(ret);
            PPPOE_IA_MGR_LOCK();
            ret = PPPOE_IA_ENGINE_SetPortUi32DataByField(
                        lport, fld_id, new_val);
            PPPOE_IA_MGR_UNLOCK();
            SYSFUN_RELEASE_CSC();
        }
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortAdmStrDataByField
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
 *          2. get API - PPPOE_IA_MGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_UI, "lport/fld/str-len: %ld/%ld/%ld",
        lport, fld_id, str_len);

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        SYSFUN_USE_CSC(ret);
        PPPOE_IA_MGR_LOCK();
        ret = PPPOE_IA_ENGINE_SetPortAdmStrDataByField(
                    lport, fld_id, str_p, str_len);
        PPPOE_IA_MGR_UNLOCK();
        SYSFUN_RELEASE_CSC();
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* LOCAL SUBPROGRAM BODIES
 */


