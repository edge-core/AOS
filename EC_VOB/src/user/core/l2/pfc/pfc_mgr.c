/* MODULE NAME: pfc_mgr.c
 * PURPOSE:
 *   Definitions of MGR APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"

#if (SYS_CPNT_PFC == TRUE)

#include "sys_module.h"
#include "pfc_type.h"
#include "pfc_om.h"
#include "pfc_mgr.h"
#include "pfc_backdoor.h"
#include "swctrl.h"
#include "backdoor_mgr.h"
#include "l4_mgr.h"
#include "sys_callback_mgr.h"

#include "swctrl_pmgr.h"

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #include "sysctrl_xor_mgr.h"
#endif

#if (SYS_CPNT_CN == TRUE)
	#include "cn_type.h"
	#include "cn_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#if (PFC_TYPE_BUILD_LINUX == TRUE)
    #define PFC_MGR_LOCK()
    #define PFC_MGR_UNLOCK()

    #define SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE()
    #define SYSFUN_USE_CSC(ret)
    #define SYSFUN_RELEASE_CSC()
#else
    #define PFC_MGR_LOCK()     if (SYSFUN_GetSem(pfc_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER) != SYSFUN_OK)\
                                    printf("%s MGR_LOCK failed\n", __FUNCTION__);

    #define PFC_MGR_UNLOCK()   if (SYSFUN_SetSem(pfc_mgr_sem_id) != SYSFUN_OK)\
                                    printf("%s MGR_UNLOCK failed\n", __FUNCTION__);

#endif

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #define PFC_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {                                        \
            SYSCTRL_XOR_MGR_GetSemaphore();         \
            if (!(expr))                            \
            {                                       \
                SYSCTRL_XOR_MGR_ReleaseSemaphore(); \
                return ret_val;                     \
            }                                       \
        } while (0)

    #define PFC_MGR_XOR_UNLOCK_AND_RETURN(ret_val)  \
        do {                                        \
            SYSCTRL_XOR_MGR_ReleaseSemaphore();     \
            return ret_val;                         \
        } while (0)

#else
    #define PFC_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {} while (0)

    #define PFC_MGR_XOR_UNLOCK_AND_RETURN(ret_val)             \
        do {return ret_val;} while (0)

#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

#define PFC_MGR_IS_OPR_MODE_ON(lport)                                   \
    ({                                                                  \
        UI32_T  opr_mode;                                               \
        BOOL_T  tmp_ret = FALSE;                                        \
                                                                        \
        if (  (TRUE == PFC_OM_GetDataByField(                           \
                        lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &opr_mode)) \
            &&(PFC_TYPE_PMODE_ON == opr_mode)                           \
           )                                                            \
            tmp_ret = TRUE;                                             \
        tmp_ret;                                                        \
     })

#define PFC_MGR_IS_ADM_MODE_OFF(lport)                                  \
    ({                                                                  \
        UI32_T  adm_mode;                                               \
        BOOL_T  tmp_ret = FALSE;                                        \
                                                                        \
        if (  (TRUE == PFC_OM_GetDataByField(                           \
                        lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &adm_mode)) \
            &&(PFC_TYPE_PMODE_OFF == adm_mode)                          \
           )                                                            \
            tmp_ret = TRUE;                                             \
        tmp_ret;                                                        \
     })

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    SWCTRL_TrunkPortExtInfo_T   trunk_ext_info;
    BOOL_T                      is_trunk;
} PFC_MGR_TrunkInfo_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void PFC_MGR_LocalClearOnePortConfig(
    UI32_T  lport);
static void PFC_MGR_LocalSetDefaultConfigPort(
    UI32_T  start_lport,
    UI32_T  end_lport);
static UI32_T PFC_MGR_LocalPreCheckForGet(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);
static UI32_T PFC_MGR_LocalPreCheckForSet(
    UI32_T                      lport,
    UI32_T                      field_id,
    void                        *data_p,
    BOOL_T                      *is_trk_p,
    SWCTRL_TrunkPortExtInfo_T   *trk_ext_info_p);
static UI32_T PFC_MGR_LocalSetDataByField(
    UI32_T                      lport,
    UI32_T                      field_id,
    void                        *data_p,
    SWCTRL_TrunkPortExtInfo_T   *trk_ext_info_p);

/* STATIC VARIABLE DECLARATIONS
 */
#if (PFC_TYPE_BUILD_LINUX == FALSE)
static UI32_T pfc_mgr_sem_id; /* the semaphore id for OAM */
#endif

/* for backdoor info display */
char *pfc_rce_str[] = { PFC_TYPE_RCE_LST(PFC_TYPE_SINGAL_NAME) };

SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_AddFstTrkMbr_CallBack
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
void PFC_MGR_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PFC_MGR_LOCK();

    /* 1. copy the 1st member's property to trunk
     */
    PFC_OM_CopyPortConfigTo(member_ifindex, trunk_ifindex);

    PFC_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_AddTrkMbr_CallBack
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
void PFC_MGR_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PFC_MGR_LOCK();

    /* 1. apply the trunk's property to member
     */
    {
#define _FLD_SET_AR_MAX  (sizeof(fld_set_ar)/sizeof(fld_set_ar[0]))

        UI32_T  fld_set_ar[] =
                    {
                        PFC_TYPE_FLDE_PORT_MODE_ADM,
                        PFC_TYPE_FLDE_PORT_MODE_OPR,
                        PFC_TYPE_FLDE_PORT_PRI_EN_ADM,
                        PFC_TYPE_FLDE_PORT_PRI_EN_OPR
                    };
        UI32_T  tmp_data, idx, fld_id;

        for (idx =0; idx < _FLD_SET_AR_MAX; idx++)
        {
            fld_id = fld_set_ar[idx];
            PFC_OM_GetDataByField(trunk_ifindex, fld_id, &tmp_data);
            PFC_MGR_LocalSetDataByField(member_ifindex, fld_id, &tmp_data, NULL);
        }
    }

    PFC_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_DelLstTrkMbr_CallBack
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
void PFC_MGR_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PFC_MGR_LOCK();

    /* 1. clear trunk's property
     */
    PFC_MGR_LocalClearOnePortConfig(trunk_ifindex);

    PFC_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_DelTrkMbr_CallBack
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
void PFC_MGR_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    /* nothing to do in current design */
#if 0
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PFC_MGR_LOCK();
    PFC_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_CosConfigChanged_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from COS when cos config is changed
 *          on the lport
 * INPUT  : lport         -- which lport event occurred on
 *          new_is_cos_ok -- TRUE if cos is ok for PFC
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PFC_MGR_CosConfigChanged_CallBack(
    UI32_T  lport,
    BOOL_T  new_is_cos_ok)
{
    UI32_T  old_is_cos_ok = FALSE;
    UI32_T  old_is_any_port_op_en = FALSE,
            new_is_any_port_op_en = FALSE;

    PFC_BDR_MSG(PFC_DBG_MGR, "");

    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    PFC_MGR_LOCK();

    PFC_OM_GetDataByField(0, PFC_TYPE_FLDE_GLOBAL_IS_COS_OK, &old_is_cos_ok);
    PFC_OM_GetDataByField(
        0, PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN, &old_is_any_port_op_en);

    if (old_is_cos_ok != new_is_cos_ok)
    {
        if (FALSE == new_is_cos_ok)
        {
            /* disable operation mode
             */
            UI32_T                      lport, tmp_opr_mode = PFC_TYPE_PMODE_OFF;
            SWCTRL_TrunkPortExtInfo_T   trk_ext_info;

            for (lport =1; lport <=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                if (TRUE == SWCTRL_LogicalPortExisting(lport))
                {
                    if (lport >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)
                    {
                        if (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trk_ext_info))
                        {
                            PFC_MGR_LocalSetDataByField(
                                lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &tmp_opr_mode, &trk_ext_info);
                        }
                    }
                    else
                    {
                        PFC_MGR_LocalSetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &tmp_opr_mode, NULL);
                    }
                }
            }

            PFC_OM_SetDataByField(0, PFC_TYPE_FLDE_GLOBAL_IS_COS_OK, &new_is_cos_ok);
        }
        else
        {
            /* enable operation mode
             * auto mode setting will be updated by next dcbx call
             */
            UI32_T                      lport, old_adm_mode,
                                        tmp_adm_mode = PFC_TYPE_PMODE_OFF;
            SWCTRL_TrunkPortExtInfo_T   trk_ext_info;

            PFC_OM_SetDataByField(0, PFC_TYPE_FLDE_GLOBAL_IS_COS_OK, &new_is_cos_ok);

            for (lport =1; lport <=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                if (TRUE == SWCTRL_LogicalPortExisting(lport))
                {
                    if (  (FALSE == PFC_OM_GetDataByField(
                                        lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &old_adm_mode))
                        ||(PFC_TYPE_PMODE_ON != old_adm_mode)
                       )
                        continue;

                    if (lport >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)
                    {
                        if (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trk_ext_info))
                        {
                            PFC_OM_SetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_adm_mode);
                            PFC_MGR_LocalSetDataByField(
                                lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &old_adm_mode, &trk_ext_info);
                        }
                    }
                    else
                    {
                        PFC_OM_SetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_adm_mode);
                        PFC_MGR_LocalSetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &old_adm_mode, NULL);
                    }
                }
            }
        }

#if (SYS_CPNT_CN == TRUE)
        PFC_OM_GetDataByField(
            0, PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN, &new_is_any_port_op_en);

        /* notify CN to enable  if all port is operational disabled
         * notify CN to disable if one port is operational enabled
         */
        if (old_is_any_port_op_en != new_is_any_port_op_en)
        {
            UI32_T	cn_global_op_status = CN_TYPE_GLOBAL_STATUS_DISABLE;

            if (FALSE == new_is_any_port_op_en)
            {
				cn_global_op_status = CN_TYPE_GLOBAL_STATUS_ENABLE;
            }
            else
            {
                SWCTRL_PMGR_UpdatePfcPriMap();
            }

			PFC_BDR_MSG(PFC_DBG_MGR, "notify CN %d", new_is_any_port_op_en);
			CN_MGR_SetGlobalOperStatus(cn_global_op_status);
        }
#endif /* (SYS_CPNT_CN == TRUE) */
    }

    PFC_MGR_UNLOCK();
    SYSFUN_RELEASE_CSC();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PFC.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_InitiateSystemResources(void)
{
#if (PFC_TYPE_BUILD_LINUX == FALSE)
    if( SYSFUN_CreateSem (1, SYSFUN_SEM_FIFO, &pfc_mgr_sem_id) != SYSFUN_OK)
    {
        printf("%s : Create Semaphore Failed.\r\n", __FUNCTION__);
    }
#endif

    PFC_OM_InitOM();
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void PFC_MGR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
#if (PFC_TYPE_BUILD_LINUX == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("PFC",
        SYS_BLD_DCB_GROUP_IPCMSGQ_KEY, PFC_BACKDOOR_Main);  //!!! SQUID
#endif
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T PFC_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PFC operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    PFC_MGR_LOCK();
    PFC_MGR_LocalSetDefaultConfigPort(1, PFC_TYPE_MAX_NBR_OF_UPORT);
    PFC_MGR_UNLOCK();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PFC operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    PFC_MGR_LOCK();
    PFC_OM_ClearOM();
    PFC_MGR_UNLOCK();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void PFC_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: To do provision complete.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
void PFC_MGR_ProvisionComplete(void)
{
    PFC_BDR_MSG(PFC_DBG_UI, "");

    SWCTRL_PMGR_UpdatePfcPriMap();

#if 0 /* seems nothing is needed */
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        PFC_MGR_LOCK();
        PFC_MGR_UNLOCK();
    }
    SYSFUN_RELEASE_CSC();
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   inserted
 *          number_of_port        -- the number of ports on the inserted
 *                                   module
 *          use_default           -- the flag indicating the default
 *                                   configuration is used without further
 *                                   provision applied; TRUE if a new module
 *                                   different from the original one is
 *                                   inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void PFC_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default)
{
#if 0 /* seems nothing is needed */
    UI32_T  end_ifindex = in_starting_port_ifindex+in_number_of_port-1;

    /* 1. for global setting,   need to re-config the driver setting
     *
     * 2. for per port setting, cli will re-provision the setting
     *                          when inserting back
     */
    PFC_MGR_LOCK();
    if (TRUE == in_use_default)
    {
        PFC_ENGINE_SetDefaultConfigPort(in_starting_port_ifindex, end_ifindex);
    }
    PFC_MGR_UNLOCK();
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   removed
 *          number_of_port        -- the number of ports on the removed
 *                                   module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void PFC_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port)
{
    UI32_T  ifindex;

    /* 1. for global setting,   do nothing
     *                          (bcz cli will not record it)
     * 2. for per port setting, clear the om
     */
    PFC_MGR_LOCK();
    for (ifindex = in_starting_port_ifindex;
         ifindex<= in_starting_port_ifindex+in_number_of_port-1;
         ifindex++)
    {
        PFC_MGR_LocalClearOnePortConfig(ifindex);
    }
    PFC_MGR_UNLOCK();
}

#if (PFC_TYPE_BUILD_LINUX == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message for PFC MGR.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    PFC_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (PFC_MGR_IpcMsg_T *) msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = PFC_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    PFC_BDR_MSG(PFC_DBG_UI, "cmd-%ld", (long)msg_p->type.cmd);

    /* dispatch IPC message and call the corresponding PFC_MGR function
     */
    switch (msg_p->type.cmd)
    {
    case PFC_MGR_IPC_GET_DATA_BY_FLD:
        msgbuf_p->msg_size = PFC_MGR_GET_MSG_SIZE(lport_fld_data);
        msg_p->type.ret_bool = PFC_MGR_GetDataByField(
        msg_p->data.lport_fld_data.lport,
        msg_p->data.lport_fld_data.fld_id,
        (UI8_T *) &msg_p->data.lport_fld_data.ui32_data);
        break;
    case PFC_MGR_IPC_GET_RUNNING_DATA_BY_FLD:
        msgbuf_p->msg_size = PFC_MGR_GET_MSG_SIZE(lport_fld_data);
        msg_p->type.ret_ui32 = PFC_MGR_GetRunningDataByField(
        msg_p->data.lport_fld_data.lport,
        msg_p->data.lport_fld_data.fld_id,
        (UI8_T *) &msg_p->data.lport_fld_data.ui32_data);
        break;
    case PFC_MGR_IPC_SET_DATA_BY_FLD:
        msgbuf_p->msg_size = PFC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        msg_p->type.ret_ui32 = PFC_MGR_SetDataByField(
        msg_p->data.lport_fld_data.lport,
        msg_p->data.lport_fld_data.fld_id,
        (UI8_T *) &msg_p->data.lport_fld_data.ui32_data);
        break;
    default:
        SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = PFC_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of PFC_MGR_HandleIPCReqMsg */
#endif /* #if (PFC_TYPE_BUILD_LINUX == TRUE) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetRunningDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : 1. all output data are represented in UI32_T
 *--------------------------------------------------------------------------
 */
UI32_T PFC_MGR_GetRunningDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    BOOL_T  tmp_bool;

    PFC_BDR_MSG(PFC_DBG_MGR, "");

    SYSFUN_USE_CSC(ret);
    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(NULL != data_p)
       )
    {
        PFC_MGR_LOCK();

        tmp_bool = PFC_OM_GetDataByField(lport, field_id, data_p);

        PFC_MGR_UNLOCK();

        if (TRUE == tmp_bool)
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

            switch (field_id)
            {
            case PFC_TYPE_FLDE_PORT_MODE_ADM:
                if (*((UI32_T *) data_p) != SYS_DFLT_PFC_PORT_MODE)
                    ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM:
                if (*((UI32_T *) data_p) != PFC_TYPE_DFT_PORT_PRI_EN_BMP)
                    ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                break;
            }
        }
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to output data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    BOOL_T  ret = FALSE;

    PFC_BDR_MSG(PFC_DBG_UI, "");

    SYSFUN_USE_CSC(ret);
    if (  (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        &&(NULL != data_p)
       )
    {
        if (PFC_TYPE_RCE_OK == PFC_MGR_LocalPreCheckForGet(
                                    lport, field_id, data_p))
        {
            if (PFC_TYPE_FLDE_PORT_NXT_PORT == field_id) /* for SNMP */
            {
                UI32_T  tmp_lport;

                ret = FALSE;
                for (tmp_lport = lport+1; tmp_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; tmp_lport++)
                {
                    if (TRUE == SWCTRL_LogicalPortExisting(tmp_lport))
                    {
                        *((UI32_T *) data_p) = tmp_lport;
                        ret = TRUE;
                        break;
                    }
                }
            }
            else
            {
                PFC_MGR_LOCK();
                ret = PFC_OM_GetDataByField(lport, field_id, data_p);
                PFC_MGR_UNLOCK();
            }
        }
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByFieldByDCBX
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport for DCBX.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByFieldByDCBX(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    UI32_T  ret = PFC_TYPE_RCE_UNKNOWN;
    UI32_T  tmp_adm_mode = PFC_TYPE_PMODE_OFF;

    PFC_BDR_MSG(PFC_DBG_UI, "");

    if (NULL != data_p)
    {
        switch (field_id)
        {
        case PFC_TYPE_FLDE_PORT_MODE_OPR:
        case PFC_TYPE_FLDE_PORT_PRI_EN_OPR:
            /* only these field are allowed to be set by DCBX
             */
            PFC_OM_GetDataByField(
                lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_adm_mode);

            if (PFC_TYPE_PMODE_AUTO == tmp_adm_mode)
            {
                ret = PFC_MGR_SetDataByField(lport, field_id, data_p);
            }
            else
            {
                ret = PFC_TYPE_RCE_NOT_ADMIN_AUTO;
            }
            break;
        default:
            break;
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    UI32_T  ret = PFC_TYPE_RCE_XOR_CHK_FAILED;
    BOOL_T  is_xor_chk = FALSE;

    PFC_BDR_MSG(PFC_DBG_UI, "");

    if (NULL != data_p)
    {
        if (PFC_TYPE_FLDE_PORT_MODE_ADM == field_id)
        {
            if (PFC_TYPE_PMODE_OFF != *((UI32_T *) data_p)) /* for no command */
            {
                is_xor_chk = TRUE;
                PFC_MGR_XOR_LOCK_AND_FAIL_RETURN(
                    ret,
                    (TRUE == SYSCTRL_XOR_MGR_PermitBeingSetToPfcPort(lport)));
            }
        }

        ret = PFC_MGR_SetDataByFieldWithoutXorCheck(lport, field_id, data_p);

        if (TRUE == is_xor_chk)
        {
            PFC_MGR_XOR_UNLOCK_AND_RETURN(ret);
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByFieldWithoutXorCheck
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *          2. no xor checking for lport
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByFieldWithoutXorCheck(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    UI32_T  ret = PFC_TYPE_RCE_NOT_IN_MASTER_MODE;

    PFC_BDR_MSG(PFC_DBG_UI, "");

    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (NULL != data_p)
        {
            UI32_T                      old_is_any_port_op_en = FALSE,
                                        new_is_any_port_op_en = FALSE;
            SWCTRL_TrunkPortExtInfo_T   trk_ext_info, *trk_ext_info_p = NULL;
            BOOL_T                      is_trk = FALSE;

            ret = PFC_MGR_LocalPreCheckForSet(lport, field_id, data_p, &is_trk, &trk_ext_info);

            if (PFC_TYPE_RCE_OK == ret)
            {
                PFC_MGR_LOCK();

                if (  (PFC_TYPE_FLDE_PORT_MODE_ADM == field_id)
                    ||(PFC_TYPE_FLDE_PORT_MODE_OPR == field_id)
                   )
                {
                    PFC_OM_GetDataByField(
                        0, PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN,
                        &old_is_any_port_op_en);
                }

                if (TRUE == is_trk)
                {
                    trk_ext_info_p = &trk_ext_info;
                }
                ret = PFC_MGR_LocalSetDataByField(lport, field_id, data_p, trk_ext_info_p);

                if (  (PFC_TYPE_FLDE_PORT_MODE_ADM == field_id)
                    ||(PFC_TYPE_FLDE_PORT_MODE_OPR == field_id)
                   )
                {
#if (SYS_CPNT_CN == TRUE)
                    PFC_OM_GetDataByField(
                        0, PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN,
                        &new_is_any_port_op_en);

                    /* notify CN to enable  if all port is operational disabled
                     * notify CN to disable if one port is operational enabled
                     */
                    if (old_is_any_port_op_en != new_is_any_port_op_en)
                    {
						UI32_T	cn_global_op_status = CN_TYPE_GLOBAL_STATUS_DISABLE;

						if (FALSE == new_is_any_port_op_en)
						{
							cn_global_op_status = CN_TYPE_GLOBAL_STATUS_ENABLE;
						}

						PFC_BDR_MSG(PFC_DBG_MGR, "notify CN %d", new_is_any_port_op_en);
						CN_MGR_SetGlobalOperStatus(cn_global_op_status);
                    }
#endif /* (SYS_CPNT_CN == TRUE) */
                }

                PFC_MGR_UNLOCK();
            }
        }
    }
    SYSFUN_RELEASE_CSC();

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_RestoreToConfigByDCBX
 *-------------------------------------------------------------------------
 * PURPOSE: Restore operational OM to configuration one.
 * INPUT  : lport - which lport to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : Restore OM and chip setting to be same as UI configuration.
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_RestoreToConfigByDCBX(UI32_T lport)
{
    UI32_T  tmp_adm_mode = PFC_TYPE_PMODE_OFF,
            tmp_pri_en = PFC_TYPE_DFT_PORT_PRI_EN_BMP,
            tmp_opr_mode = PFC_TYPE_PMODE_OFF;
    BOOL_T  ret = FALSE;

    PFC_BDR_MSG(PFC_DBG_UI, "");

    if (TRUE == PFC_OM_GetDataByField(
            lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_adm_mode))
    {
        if (PFC_TYPE_PMODE_AUTO == tmp_adm_mode)
        {
            PFC_OM_GetDataByField(
                lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &tmp_pri_en);

            PFC_OM_GetDataByField(
                lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &tmp_opr_mode);

            if (PFC_TYPE_RCE_OK == PFC_MGR_SetDataByField(
                                        lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &tmp_pri_en))
            {
                ret = TRUE;

                if (PFC_TYPE_PMODE_ON != tmp_opr_mode)
                {
                    tmp_opr_mode = PFC_TYPE_PMODE_ON;
                    if (PFC_TYPE_RCE_OK != PFC_MGR_SetDataByField(
                                                lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &tmp_opr_mode))
                    {
                        ret = FALSE;
                    }
                }
            }
        }
    }

    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalClearOnePortConfig
 * ------------------------------------------------------------------------
 * PURPOSE: To clear configuration for one port.
 * INPUT  : lport - which lport to clear (1-based)
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static void PFC_MGR_LocalClearOnePortConfig(
    UI32_T  lport)
{
    PFC_TYPE_PortCtrlRec_T *pctrl_p;

    pctrl_p = PFC_OM_GetPortCtrlPtrByLport(lport);
    if (NULL != pctrl_p)
    {
        memset(pctrl_p, 0, sizeof(*pctrl_p));
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetDefaultConfigPort
 * ------------------------------------------------------------------------
 * PURPOSE: To do extra work for default configuration per port.
 *          (e.g. setup the rules.)
 * INPUT  : start_lport - start lport for extra config (1-based)
 *          end_lport   - end   lport for extra config (1-based)
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static void PFC_MGR_LocalSetDefaultConfigPort(
    UI32_T  start_lport,
    UI32_T  end_lport)
{
    UI32_T  lport,
            tmp_pri_ebmp = PFC_TYPE_DFT_PORT_PRI_EN_BMP,
            tmp_adm_mode = SYS_DFLT_PFC_PORT_MODE;

    for (lport =start_lport; lport <=end_lport; lport++)
    {
        PFC_MGR_LocalSetDataByField(
                lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &tmp_pri_ebmp, NULL);
        PFC_MGR_LocalSetDataByField(
                lport, PFC_TYPE_FLDE_PORT_MODE_ADM,   &tmp_adm_mode, NULL);
    }

#if (SYS_CPNT_CN == TRUE)
    /* notify CN to disable if one port is operational enabled
     */
    if (PFC_TYPE_PMODE_ON == tmp_adm_mode)
    {
        PFC_BDR_MSG(PFC_DBG_MGR, "notify CN %d", 0);
		CN_MGR_SetGlobalOperStatus(CN_TYPE_GLOBAL_STATUS_DISABLE);
    }
#endif /* (SYS_CPNT_CN == TRUE) */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalPreCheckForGet
 *-------------------------------------------------------------------------
 * PURPOSE: To do pre-checking for getting data by field.
 * INPUT  : lport    - which lport to set
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
static UI32_T PFC_MGR_LocalPreCheckForGet(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    UI32_T  tmp_value;
    BOOL_T  ret = PFC_TYPE_RCE_VALUE_OUT_OF_RANGE;

    tmp_value = *((UI32_T *) data_p);

    if (field_id < PFC_TYPE_FLDE_MAX)
    {
        if (0 == lport)
        {
            switch (field_id)
            {
            case PFC_TYPE_FLDE_GLOBAL_IS_COS_OK:
            case PFC_TYPE_FLDE_GLOBAL_MAX_TC_CAP:
            case PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN:
            case PFC_TYPE_FLDE_PORT_NXT_PORT:
                ret = PFC_TYPE_RCE_OK;
                break;
            default:
                break;
            }
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            UI32_T                      unit, port, trunk;
            SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;

            switch (field_id)
            {
            default:
                switch (SWCTRL_LogicalPortToUserPort(
                        lport, &unit, &port, &trunk))
                {
                case SWCTRL_LPORT_TRUNK_PORT:
                    /* trunk can be configured when it has at least one member port
                     */
                    if (  (FALSE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
                        ||(0 == trunk_ext_p_info.member_number)
                       )
                    {
                        ret = PFC_TYPE_RCE_TRUNK_NO_MBR;
                        break;
                    }

                case SWCTRL_LPORT_NORMAL_PORT:

#if 0 //not support trunk member now
                case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
#endif
                    ret = PFC_TYPE_RCE_OK;
                    break;
                default:
                    break;
                }

                break;
            }
        }
    }

    PFC_BDR_MSG(PFC_DBG_MGR,
        "port/field/ret-%ld/%ld/%s",
         (long)lport, (long)field_id, &pfc_rce_str[ret][PFC_TYPE_RCE_STR_PREFIX]);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalPreCheckForSet
 *-------------------------------------------------------------------------
 * PURPOSE: To do pre-checking for setting data by field.
 * INPUT  : lport          - which lport to set
 *                           0 - global data to set
 *                          >0 - lport  data to set
 *          field_id       - field id to set (PFC_TYPE_FieldId_E)
 *          data_p         - pointer to input data
 * OUTPUT : is_trk_p       - pointer to output flag (TRUE if lport is a trunk)
 *          trk_ext_info_p - pointer to output trunk ext info
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
static UI32_T PFC_MGR_LocalPreCheckForSet(
    UI32_T                      lport,
    UI32_T                      field_id,
    void                        *data_p,
    BOOL_T                      *is_trk_p,
    SWCTRL_TrunkPortExtInfo_T   *trk_ext_info_p)
{
    UI32_T  tmp_value;
    UI32_T  ret = PFC_TYPE_RCE_FIELD_OUT_OF_RANGE;

    tmp_value = *((UI32_T *) data_p);

    if (field_id < PFC_TYPE_FLDE_MAX)
    {
        ret = PFC_TYPE_RCE_VALUE_OUT_OF_RANGE;
        if (0 == lport)
        {
            switch (field_id)
            {
            default:    /* get only */
                break;
            }
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            UI32_T  unit, port, trunk;

            switch (SWCTRL_LogicalPortToUserPort(
                    lport, &unit, &port, &trunk))
            {
            case SWCTRL_LPORT_TRUNK_PORT:
                /* trunk can be configured when it has at least one member port
                 */
                if (NULL != trk_ext_info_p)
                {
                    if (  (FALSE == SWCTRL_GetTrunkPortExtInfo(lport, trk_ext_info_p))
                        ||(0 == trk_ext_info_p->member_number)
                       )
                    {
                        ret = PFC_TYPE_RCE_TRUNK_NO_MBR;
                        break;
                    }

                    if (NULL != is_trk_p)
                        *is_trk_p = TRUE;
                }
            case SWCTRL_LPORT_NORMAL_PORT:

#if 0 //not support trunk member now
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
#endif

                switch (field_id)
                {
                case PFC_TYPE_FLDE_PORT_MODE_ADM:
                    if (PFC_TYPE_PMODE_MAX > tmp_value)
                        ret = PFC_TYPE_RCE_OK;
                    break;
                case PFC_TYPE_FLDE_PORT_MODE_OPR:
                    if (PFC_TYPE_PMODE_AUTO > tmp_value)
                        ret = PFC_TYPE_RCE_OK;
                    break;
                case PFC_TYPE_FLDE_PORT_PRI_EN_ADM:
                case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD:
                case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM:
                case PFC_TYPE_FLDE_PORT_PRI_EN_OPR:
                    if (0xffff >= tmp_value)
                        ret = PFC_TYPE_RCE_OK;
                    break;
                default:    /* get only */
                    ret = PFC_TYPE_RCE_GET_ONLY;
                    break;
                }
                break;

            default:
                break;
            }
        }
    }

    PFC_BDR_MSG(PFC_DBG_MGR,
        "port/field/ret-%ld/%ld/%s",
         (long)lport, (long)field_id, &pfc_rce_str[ret][PFC_TYPE_RCE_STR_PREFIX]);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetPortModeOpr
 * -------------------------------------------------------------------------
 * PURPOSE: To set the Operation mode by specified ifidx.
 * INPUT  : lport       - which lport to set
 *          new_mode    - which mode to set
 *          new_pri_bmp - which priority bitmap to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : admin mode checking done by caller
 * -------------------------------------------------------------------------*/
static BOOL_T PFC_MGR_LocalSetPortModeOpr(
    UI32_T  lport,
    UI32_T  new_mode,
    UI32_T  new_pri_bmp)
{
    BOOL_T  ret, drv_en;

    /* config new opr pri to driver and om
     */
    drv_en = (PFC_TYPE_PMODE_ON == new_mode) ? TRUE : FALSE;

    ret = SWCTRL_SetPortPfcStatus(lport, drv_en, drv_en, new_pri_bmp);

    ret = ret && PFC_OM_SetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &new_pri_bmp);

    ret = ret && PFC_OM_SetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &new_mode);

    PFC_BDR_MSG(PFC_DBG_MGR,"lport/new_mode/new_pri_bmp/ret-%ld/%ld/%04x/%d\n",
                (long)lport, (long)new_mode, new_pri_bmp, ret);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetPortModeAdm
 * -------------------------------------------------------------------------
 * PURPOSE: To set the admin mode by specified ifidx.
 * INPUT  : lport      - which lport to set
 *          new_mode   - which mode to set
 *          is_cfg_opr - TRUE to config operation mode
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : only for PFC_MGR_LocalSetDataByField
 * -------------------------------------------------------------------------*/
static BOOL_T PFC_MGR_LocalSetPortModeAdm(
    UI32_T  lport,
    UI32_T  new_mode,
    BOOL_T  is_cfg_opr)
{
    UI32_T  new_opr_mode, new_pri_bmp =0;
    BOOL_T  ret = TRUE;

    if (TRUE == is_cfg_opr)
    {
        switch (new_mode)
        {
        case PFC_TYPE_PMODE_OFF:
            new_opr_mode = PFC_TYPE_PMODE_OFF; /* clear opr pri */
            ret = TRUE;
            break;
        case PFC_TYPE_PMODE_ON:
        case PFC_TYPE_PMODE_AUTO:
            new_opr_mode = PFC_TYPE_PMODE_ON;  /* set opr pri   */
            ret = PFC_OM_GetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &new_pri_bmp);
            break;
        }

        ret = ret && PFC_MGR_LocalSetPortModeOpr(lport, new_opr_mode, new_pri_bmp);
    }

    ret = ret && PFC_OM_SetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &new_mode);

    if (TRUE == ret)
    {
        /* notify DCBX to re-run port sm
         */
        SYS_CALLBACK_MGR_PfcConfigChangedCallback(SYS_MODULE_PFC, lport);
    }

    PFC_BDR_MSG(PFC_DBG_MGR,"lport/new_mode/is_cfg_opr/ret-%ld/%d/%d/%d\n",
                (long)lport, new_mode, is_cfg_opr, ret);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetPortPriEnOpr
 * -------------------------------------------------------------------------
 * PURPOSE: To set the operation priority enable list by specified ifidx.
 * INPUT  : lport       - which lport to set
 *          new_pri_bmp - which priority bitmap to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : operation mode checking done by caller
 * -------------------------------------------------------------------------*/
static BOOL_T PFC_MGR_LocalSetPortPriEnOpr(
    UI32_T  lport,
    UI32_T  new_pri_bmp)
{
    BOOL_T  ret = FALSE;

    /* set opr mode to driver */
    ret = SWCTRL_SetPortPfcStatus(lport, TRUE, TRUE, new_pri_bmp);

    ret = ret && PFC_OM_SetDataByField(
                lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &new_pri_bmp);

    PFC_BDR_MSG(PFC_DBG_MGR,"lport/new_pri_bmp/ret-%ld/%04x/%d\n",
                (long)lport, new_pri_bmp, ret);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetPortPriEnAdm
 * -------------------------------------------------------------------------
 * PURPOSE: To set the admin priority enable list by specified ifidx.
 * INPUT  : lport       - which lport to set
 *          new_pri_bmp - which priority bitmap to set
 *          is_cfg_opr - TRUE to config operation mode
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : only for PFC_MGR_LocalSetDataByField
 * -------------------------------------------------------------------------*/
static BOOL_T PFC_MGR_LocalSetPortPriEnAdm(
    UI32_T  lport,
    UI32_T  new_pri_bmp,
    BOOL_T  is_cfg_opr)
{
    BOOL_T  ret = TRUE;

    if (  (TRUE == is_cfg_opr)
        &&(TRUE == PFC_MGR_IS_OPR_MODE_ON(lport))
       )
    {
        ret = PFC_MGR_LocalSetPortPriEnOpr(lport, new_pri_bmp);
    }

    ret = ret && PFC_OM_SetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &new_pri_bmp);

    if (  (TRUE == is_cfg_opr)
        &&(TRUE == PFC_MGR_IS_OPR_MODE_ON(lport))
        &&(TRUE == ret)
       )
    {
        /* notify DCBX to re-run port sm
         */
        SYS_CALLBACK_MGR_PfcConfigChangedCallback(SYS_MODULE_PFC, lport);
    }

    PFC_BDR_MSG(PFC_DBG_MGR,"lport/new_pri_bmp/ret-%ld/%04x/%d\n",
                (long)lport, new_pri_bmp, ret);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalSetDataByField
 * -------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and ifindex.
 * INPUT  : lport          - lport to set (1-based)
 *          field_id       - field id to set
 *                           refer to PFC_TYPE_FLDE_LST
 *          data_p         - pointer to data for set
 *          trk_ext_info_p - pointer to trunk ext info
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
static UI32_T PFC_MGR_LocalSetDataByField(
    UI32_T                      lport,
    UI32_T                      field_id,
    void                        *data_p,
    SWCTRL_TrunkPortExtInfo_T   *trk_ext_info_p)
{
    UI32_T  new_data, old_data;
    UI32_T  ret = PFC_TYPE_RCE_UNKNOWN;

    new_data = *((UI32_T *) data_p);

    /* check if config changed
     */
    if (TRUE == PFC_OM_GetDataByField(lport, field_id, &old_data))
    {
        ret = PFC_TYPE_RCE_OK;

        switch (field_id)
        {
        case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD:
            new_data |= old_data;
            field_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM;
            break;
        case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM:
            field_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM;
            new_data = (old_data & (~new_data));
            break;
        default:
            break;
        }

        if (old_data != new_data)
        {
            UI32_T  old_is_cos_ok = FALSE;
            BOOL_T  tmp_ret = FALSE, is_cfg_opr = TRUE;

            PFC_OM_GetDataByField(0, PFC_TYPE_FLDE_GLOBAL_IS_COS_OK, &old_is_cos_ok);
            if (FALSE == old_is_cos_ok)
            {
                is_cfg_opr = FALSE;
            }

            switch (field_id)
            {
            /* Adm Mode  | Opr Mode    | Adm Pri | Opr Pri
             * --------------------------------------------------
             * X -> ON   | ON        v | Z       | Z        v   notify
             * X -> OFF  | OFF       v | Z       | EMPTY        notify
             * X -> AUTO | ON          | Z       | Z     (DCBX will nego with current setting)
             * AUTO      | ON  => OFF  | Z       | EMPTY
             * AUTO      | OFF => ON   | Z       | EMPTY (set by next call from DCBX)
             * ON        | ON          | Z -> Y  | Y        v   notify
             * OFF       | OFF         | Z -> Y  | EMPTY
             * AUTO      | ON          | Z -> Y  | Y     (DCBX will nego with current setting)
             * AUTO      | OFF         | Z -> Y  | EMPTY (DCBX will nego with current setting)
             * AUTO      | ON          | Z       | A => B
             *
             * v  : changed follow admin
             * -> : changed by user
             * => : changed by DCBX
             *
             * assume DCBX need to set opr_mode to on first b4 change the opr_pri
             */
            case PFC_TYPE_FLDE_PORT_MODE_ADM:
                tmp_ret = PFC_MGR_LocalSetPortModeAdm(lport, new_data, is_cfg_opr);
                break;
            case PFC_TYPE_FLDE_PORT_MODE_OPR:
                if (TRUE == is_cfg_opr)
                {
                    if (FALSE == PFC_MGR_IS_ADM_MODE_OFF(lport))
                        tmp_ret = PFC_MGR_LocalSetPortModeOpr(lport, new_data, 0);
                }
                else
                    ret = PFC_TYPE_RCE_OPR_DISB_BY_COS;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM:
                tmp_ret = PFC_MGR_LocalSetPortPriEnAdm(lport, new_data, is_cfg_opr);
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_OPR:
                if (TRUE == is_cfg_opr)
                {
                    if (TRUE == PFC_MGR_IS_OPR_MODE_ON(lport))
                        tmp_ret = PFC_MGR_LocalSetPortPriEnOpr(lport, new_data);
                }
                else
                    ret = PFC_TYPE_RCE_OPR_DISB_BY_COS;
                break;
            }

            if (TRUE == tmp_ret)
            {
                if (NULL != trk_ext_info_p)
                {
                    UI32_T  i, mbr_ifidx;

                    for (i=0; i<trk_ext_info_p->member_number; i++)
                    {
                        SWCTRL_UserPortToIfindex(
                            trk_ext_info_p->member_list[i].unit,
                            trk_ext_info_p->member_list[i].port,
                            &mbr_ifidx);

                        PFC_OM_CopyPortConfigTo(lport, mbr_ifidx);
                    }
                }
            }
            else
            {
                if (ret == PFC_TYPE_RCE_OK)
                    ret = PFC_TYPE_RCE_UNKNOWN;
            }
        }
    }

    PFC_BDR_MSG(PFC_DBG_MGR,
        "port/field/ret-%ld/%ld/%s",
         (long)lport, (long)field_id, &pfc_rce_str[ret][PFC_TYPE_RCE_STR_PREFIX]);

    return ret;
}
#endif /* #if (SYS_CPNT_PFC == TRUE) */

