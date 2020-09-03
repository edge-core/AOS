/* MODULE NAME: ets_mgr.c
 * PURPOSE:
 *   Definitions of MGR APIs for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
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

#if (SYS_CPNT_ETS == TRUE)

#include "sys_module.h"
#include "ets_type.h"
#include "ets_om.h"
#include "ets_mgr.h"
#include "ets_backdoor.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "l4_pmgr.h"
#include "backdoor_mgr.h"
#include "cos_vm.h"
#include "dev_swdrv.h"
#include "dev_swdrvl4.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_CN == TRUE)
	#include "cn_type.h"
	#include "cn_mgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#if (ETS_TYPE_BUILD_LINUX == TRUE)
    #define SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE()
    #define SYSFUN_USE_CSC(ret)
    #define SYSFUN_RELEASE_CSC()
#else
#endif

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #define ETS_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {                                        \
            SYSCTRL_XOR_MGR_GetSemaphore();         \
            if (!(expr))                            \
            {                                       \
                SYSCTRL_XOR_MGR_ReleaseSemaphore(); \
                return ret_val;                     \
            }                                       \
        } while (0)

    #define ETS_MGR_XOR_UNLOCK_AND_RETURN(ret_val)  \
        do {                                        \
            SYSCTRL_XOR_MGR_ReleaseSemaphore();     \
            return ret_val;                         \
        } while (0)

#else
    #define ETS_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {} while (0)

    #define ETS_MGR_XOR_UNLOCK_AND_RETURN(ret_val)             \
        do {return ret_val;} while (0)

#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

#define ETS_DEBUG_ENABLE FALSE

#if (ETS_DEBUG_ENABLE == TRUE)

#define ETS_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define ETS_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}

#define ETS_DEBUG_PORTMSG(port_entry)   \
{ \
  printf("\r\n index=(%lu) name=(%s) type=(%lu) speed_dpx_cfg=(%lu) flow_ctrl_cfg=(%lu) capabilities=(%lu) autonegotiation=(%lu) speed_dpx_status=(%lu) flow_ctrl_status=(%lu) trunk_index=(%lu) forced_mode=(%lu) forced_1000t_mode=(%lu)", \
  (port_entry)->port_index, \
  (port_entry)->port_name, \
  (port_entry)->port_type, \
  (port_entry)->port_speed_dpx_cfg, \
  (port_entry)->port_flow_ctrl_cfg, \
  (port_entry)->port_capabilities, \
  (port_entry)->port_autonegotiation, \
  (port_entry)->port_speed_dpx_status, \
  (port_entry)->port_flow_ctrl_status, \
  (port_entry)->port_trunk_index, \
  (port_entry)->port_forced_mode, \
  (port_entry)->port_forced_1000t_mode); \
  fflush(stdout); \
}
#else
#define ETS_DEBUG_LINE()
#define ETS_DEBUG_MSG(a,b...)
#define ETS_DEBUG_PORTMSG(port_entry)
#endif


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T ETS_MGR_SetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa, ETS_TYPE_DB_T oper_cfg);
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningTCTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T * tsa);
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningPrioAssign(UI32_T lport, UI32_T prio, UI32_T *tc);
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningWeight(UI32_T lport, UI32_T weight [ SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ]);
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningPortMode(UI32_T lport, ETS_TYPE_MODE_T *mode);
static BOOL_T ETS_MGR_SetAsicPrioAssign(UI32_T lport, UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]);
static BOOL_T ETS_MGR_SetAsicTCWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);
static BOOL_T ETS_MGR_GetMapping_QueuetoTC(UI32_T lport,
                        UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE], ETS_TYPE_DB_T oper_cfg);
static BOOL_T ETS_MGR_GetAllTCWeight(UI32_T lport,
                        UI32_T tc_weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg);
static BOOL_T ETS_MGR_SetAsicByOM(UI32_T lport);
static UI32_T ETS_MGR_SetWeightByUser(UI32_T lport, UI32_T weight [ SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ]);
static UI32_T ETS_MGR_SetPortPrioAssignByUser(UI32_T lport, UI32_T prio, UI32_T tc);
static UI32_T ETS_MGR_GetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg);
static UI32_T ETS_MGR_GetWeight(UI32_T lport, UI32_T weight [ SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ], ETS_TYPE_DB_T oper_cfg);
static UI32_T ETS_MGR_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode);
static UI32_T ETS_MGR_GetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T *tsa, ETS_TYPE_DB_T oper_cfg);
static UI32_T ETS_MGR_SetTSAByUser(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa);

static BOOL_T ETS_MGR_ChkCosMapping1to1(UI32_T lport);
static void ETS_MGR_LocalSetOperMode(
    UI32_T          lport,
    ETS_TYPE_MODE_T new_opr_mode);
static UI32_T ETS_MGR_SetOperMode(UI32_T lport, ETS_TYPE_MODE_T new_opr_mode);
static BOOL_T ETS_MGR_LocalSanityCheck(ETS_TYPE_PortEntry_T  *entry_p);

/* STATIC VARIABLE DECLARATIONS
 */
#if (ETS_TYPE_BUILD_LINUX == FALSE)
static UI32_T ets_mgr_sem_id; /* the semaphore id for OAM */
#endif

/* SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS */
static UI32_T ets_bw_default[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS][SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];


SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_AddFstTrkMbr_CallBack
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
void ETS_MGR_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    ETS_MGR_PortAddIntoTrunk_CallBack(trunk_ifindex, member_ifindex, TRUE);

    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_AddTrkMbr_CallBack
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
void ETS_MGR_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    ETS_MGR_PortAddIntoTrunk_CallBack(trunk_ifindex, member_ifindex, FALSE);

    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_DelLstTrkMbr_CallBack
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
void ETS_MGR_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    ETS_MGR_PortLeaveTrunk_CallBack(trunk_ifindex, member_ifindex, TRUE);

    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_DelTrkMbr_CallBack
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
void ETS_MGR_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    ETS_MGR_PortLeaveTrunk_CallBack(trunk_ifindex, member_ifindex, FALSE);

    SYSFUN_RELEASE_CSC();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_CosConfigChanged_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from COS when cos config is changed
 *          on the lport
 * INPUT  : lport             -- which lport event occurred on
 *          new_is_cos_global -- TRUE if cos is ok for ETS
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_MGR_CosConfigChanged_CallBack(
    UI32_T  lport,
    BOOL_T  new_is_cos_global)
{
    BOOL_T  old_is_cos_mapping_ok, new_is_cos_mapping_ok;
    BOOL_T  old_is_any_port_op_en = FALSE,
            new_is_any_port_op_en = FALSE;

    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    old_is_cos_mapping_ok = ETS_OM_GetIsCosConfigOk();

    new_is_cos_mapping_ok = (new_is_cos_global && ETS_MGR_ChkCosMapping1to1(lport));

    if (old_is_cos_mapping_ok != new_is_cos_mapping_ok)
    {
        old_is_any_port_op_en = ETS_OM_IsAnyPortOperEnabled();

        if (TRUE == new_is_cos_mapping_ok)
        {
            /* restore to user     if cos checking ok
             */
            for (lport =1; lport <=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                ETS_TYPE_MODE_T old_mode;

                if (  (TRUE == SWCTRL_LogicalPortExisting(lport))
                    &&(TRUE == ETS_OM_GetMode(lport, &old_mode))
                    &&(ETS_TYPE_MODE_OFF != old_mode)
                   )
                {
                    ETS_MGR_SetOperMode(lport, ETS_TYPE_MODE_USER);
                    ETS_OM_RestoreToConfig(lport);
                    ETS_MGR_SetAsicByOM(lport);
                }
            }
        }
        else
        {
            /* restore to disabled if cos checking failed
             */
            for (lport =1; lport <=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                if (TRUE == SWCTRL_LogicalPortExisting(lport))
                {
                    ETS_MGR_SetOperMode(lport, ETS_TYPE_MODE_OFF);
                }
            }
        }

        ETS_OM_SetIsCosConfigOk(new_is_cos_mapping_ok);

        new_is_any_port_op_en = ETS_OM_IsAnyPortOperEnabled();

#if (SYS_CPNT_CN == TRUE)
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

			ETS_DEBUG_MSG("notify CN %d", new_is_any_port_op_en);
			CN_MGR_SetGlobalOperStatus(cn_global_op_status);
        }
#endif
    }

    SYSFUN_RELEASE_CSC();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for ETS.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_InitiateSystemResources(void)
{
    ETS_OM_InitiateSystemResources();
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void ETS_MGR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
#if (ETS_TYPE_BUILD_LINUX == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ETS",
        SYS_BLD_DCB_GROUP_IPCMSGQ_KEY, ETS_BACKDOOR_Main);
#endif
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T ETS_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable ETS operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    ETS_OM_ResetToDefault();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the ETS operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    ETS_OM_ClearOM();

    /* init ets_bw_default
     */
    {
        int tc, ets_tc_nbr;
        int w, r;

        for (ets_tc_nbr = 1; ets_tc_nbr <= SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; ets_tc_nbr++)
        {
            w = SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT / ets_tc_nbr;
            r = SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT % ets_tc_nbr;

            for (tc = 0; tc < ets_tc_nbr-r; tc++)
            {
                ets_bw_default[ets_tc_nbr-1][tc] = w;
            }

            for (; tc < ets_tc_nbr; tc++)
            {
                ets_bw_default[ets_tc_nbr-1][tc] = w + 1;
            }

            for (; tc < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; tc++)
            {
                ets_bw_default[ets_tc_nbr-1][tc] = 0;
            }
        } 
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void ETS_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: To do provision complete.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
void ETS_MGR_ProvisionComplete(void)
{

}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleHotInsertion
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
void ETS_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default)
{

}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleHotRemoval
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
void ETS_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port)
{
}


#if (ETS_TYPE_BUILD_LINUX == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message for ETS MGR.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
BOOL_T ETS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    ETS_MGR_IpcMsg_T *msg_data_p;

    if(ipcmsg_p==NULL)
    {
        ETS_DEBUG_MSG("\r\n ETS_MGR_HandleIPCReqMsg ipcmsg_p==NULL");
        return FALSE;
    }

    msg_data_p = (ETS_MGR_IpcMsg_T*)ipcmsg_p->msg_buf;

    ETS_DEBUG_LINE();
    ETS_DEBUG_MSG("\r\n cmd %u", msg_data_p->type.cmd);


    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_data_p->type.ret_bool = FALSE;
        ipcmsg_p->msg_size = ETS_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding ETS_MGR function
     */
    switch (msg_data_p->type.cmd)
    {
        case ETS_MGR_IPCCMD_GET_OPER_MODE:
            ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_mode)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetOperMode(
                msg_data_p->data.ets_lport_mode.lport,
                &msg_data_p->data.ets_lport_mode.mode);
        	break;
        case ETS_MGR_IPCCMD_GET_MODE:
            ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_mode)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetMode(
                msg_data_p->data.ets_lport_mode.lport,
                &msg_data_p->data.ets_lport_mode.mode);
        	break;

        case ETS_MGR_IPCCMD_SET_MODE:
        	ipcmsg_p->msg_size=ETS_MGR_IPCMSG_TYPE_SIZE;
        	msg_data_p->type.result_ui32=ETS_MGR_SetMode(
                msg_data_p->data.ets_lport_mode.lport,
                msg_data_p->data.ets_lport_mode.mode);
        	break;
        case ETS_MGR_IPCCMD_GET_PRIO_ASSIGNMENT:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_prio_tc_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetPortPrioAssign(
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.lport,
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.prio,
                &msg_data_p->data.ets_lport_prio_tc_oper_cfg.tc,
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.oper_cfg);
        	break;

        case ETS_MGR_IPCCMD_SET_PRIO_ASSIGNMENT_BY_USER:
        	ipcmsg_p->msg_size=ETS_MGR_IPCMSG_TYPE_SIZE;
        	msg_data_p->type.result_ui32=ETS_MGR_SetPortPrioAssignByUser(
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.lport,
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.prio,
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.tc);
        	break;

        case ETS_MGR_IPCCMD_GET_TC_TSA:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_tc_tsa_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetTSA(
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.lport,
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tc,
                &msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tsa,
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.oper_cfg);
        	break;

        case ETS_MGR_IPCCMD_SET_TC_TSA_BY_USER:
        	ipcmsg_p->msg_size=ETS_MGR_IPCMSG_TYPE_SIZE;
        	msg_data_p->type.result_ui32=ETS_MGR_SetTSAByUser(
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.lport,
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tc,
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tsa);
        	break;

        case ETS_MGR_IPCCMD_GET_TC_WEIGHT:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_weight_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetWeight(
                msg_data_p->data.ets_lport_weight_oper_cfg.lport,
                msg_data_p->data.ets_lport_weight_oper_cfg.weight,
                msg_data_p->data.ets_lport_weight_oper_cfg.oper_cfg);
        	break;

        case ETS_MGR_IPCCMD_SET_TC_WEIGHT_BY_USER:
        	ipcmsg_p->msg_size=ETS_MGR_IPCMSG_TYPE_SIZE;
        	msg_data_p->type.result_ui32=ETS_MGR_SetWeightByUser(
                msg_data_p->data.ets_lport_weight_oper_cfg.lport,
                msg_data_p->data.ets_lport_weight_oper_cfg.weight);
        	break;

        case ETS_MGR_IPCCMD_GET_PORT_ENTRY:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_entry_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetPortEntry(
                msg_data_p->data.ets_lport_entry_oper_cfg.lport,
                &msg_data_p->data.ets_lport_entry_oper_cfg.entry,
                msg_data_p->data.ets_lport_entry_oper_cfg.oper_cfg);
        	break;

        case ETS_MGR_IPCCMD_GET_NEXT_PORT_ENTRY:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_entry_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetNextPortEntry(
                &msg_data_p->data.ets_lport_entry_oper_cfg.lport,
                &msg_data_p->data.ets_lport_entry_oper_cfg.entry,
                msg_data_p->data.ets_lport_entry_oper_cfg.oper_cfg);
        	break;

        case ETS_MGR_IPCCMD_GET_MAX_NUMBER_OF_TC:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ui8_data)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.result_ui32=ETS_MGR_GetMaxNumberOfTC(
                &msg_data_p->data.ui8_data);
        	break;

        case ETS_MGR_IPCCMD_GETRUNNING_PORT_MODE:
            ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_mode)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.ret_running_cfg=ETS_MGR_GetRunningPortMode(
                msg_data_p->data.ets_lport_mode.lport,
                &msg_data_p->data.ets_lport_mode.mode);
            break;
        case ETS_MGR_IPCCMD_GETRUNNING_TC_TSA:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_tc_tsa_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.ret_running_cfg=ETS_MGR_GetRunningTCTSA(
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.lport,
                msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tc,
                &msg_data_p->data.ets_lport_tc_tsa_oper_cfg.tsa);
            break;

        case ETS_MGR_IPCCMD_GETRUNNING_TC_WEIGHT:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_weight_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.ret_running_cfg=ETS_MGR_GetRunningWeight(
                msg_data_p->data.ets_lport_weight_oper_cfg.lport,
                msg_data_p->data.ets_lport_weight_oper_cfg.weight);
            break;

        case ETS_MGR_IPCCMD_GETRUNNING_PRIO_ASSIGNMENT:
        	ipcmsg_p->msg_size=(sizeof(((ETS_MGR_IpcMsg_T *)0)->data.ets_lport_prio_tc_oper_cfg)
                            + ETS_MGR_IPCMSG_TYPE_SIZE);
        	msg_data_p->type.ret_running_cfg=ETS_MGR_GetRunningPrioAssign(
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.lport,
                msg_data_p->data.ets_lport_prio_tc_oper_cfg.prio,
                &msg_data_p->data.ets_lport_prio_tc_oper_cfg.tc);
            break;

    default:
        SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
        msg_data_p->type.ret_bool = FALSE;
        ipcmsg_p->msg_size = ETS_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of ETS_MGR_HandleIPCReqMsg */
#endif /* #if (ETS_TYPE_BUILD_LINUX == TRUE) */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_PortAddIntoTrunk_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is added to a trunk
 * INPUT   : trunk_ifindex  - which trunk to add in
 *           member_ifindex - which port to be added
 *           is_firstmem    - Is this port the first member?
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void ETS_MGR_PortAddIntoTrunk_CallBack(
    UI32_T trunk_ifindex, UI32_T member_ifindex, BOOL_T is_firstmem)
{
    UI32_T              unit;
    UI32_T              u_port;
    UI32_T              trunk_id;
    SWCTRL_Lport_Type_T type;

    if(member_ifindex < 1 || member_ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
		return;

    type = SWCTRL_LogicalPortToUserPort(member_ifindex, &unit, &u_port, &trunk_id);
    if(type != SWCTRL_LPORT_TRUNK_PORT_MEMBER)
		return;

    if(is_firstmem==TRUE)
    {
        ETS_OM_CopyByLPort(trunk_ifindex, member_ifindex);
    }
    else
    {
        ETS_TYPE_MODE_T old_mode;

        if (  (TRUE == ETS_OM_GetMode(trunk_ifindex, &old_mode))
            &&(ETS_TYPE_MODE_OFF != old_mode)
           )
        {
            /* if cos checking failed do nothing
             * only copy om to member
             */
            if (TRUE == ETS_OM_GetIsCosConfigOk())
            {
                ETS_MGR_SetAsicByOM(trunk_ifindex);
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_PortLeaveTrunk_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is removed from a trunk
 * INPUT   : trunk_ifindex  - which trunk to add in
 *           member_ifindex - which port to be added
 *           is_last        - TRUE if it's the last member
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void ETS_MGR_PortLeaveTrunk_CallBack(
    UI32_T trunk_ifindex, UI32_T member_ifindex, BOOL_T is_last)
{
    ETS_TYPE_PortEntry_T    tmp_pentry;

    if(member_ifindex < 1 || member_ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
		return;

    memset (&tmp_pentry, 0, sizeof(tmp_pentry));

    /* copy setting from trunk to member
     */
    ETS_OM_CopyByLPort(member_ifindex, trunk_ifindex);

    /* 1. clear trunk's property when last member leaves
     */
    if (TRUE == is_last)
    {
        ETS_OM_SetLPortEntry(trunk_ifindex, &tmp_pentry, ETS_TYPE_DB_OPER);
        ETS_OM_SetLPortEntry(trunk_ifindex, &tmp_pentry, ETS_TYPE_DB_CONFIG);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetOperConfigEntryByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: copy ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           entry - input OM entry
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : only operational OM is written.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_SetOperConfigEntryByDCBx(UI32_T lport, ETS_TYPE_PortEntry_T  entry)
{
    ETS_TYPE_MODE_T mode;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }

    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);
    if(mode!=ETS_TYPE_MODE_AUTO)
    {
        return ETS_TYPE_RETURN_ILL_MODE;
    }

    if (FALSE == ETS_MGR_LocalSanityCheck(&entry))
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }

    /* if cos checking failed do nothing
     */
    if (TRUE == ETS_OM_GetIsCosConfigOk())
    {
        /*Update OM*/
        ETS_OM_SetLPortEntry(lport, &entry, ETS_TYPE_DB_OPER);
        if (FALSE == ETS_MGR_SetAsicByOM(lport))
        {
            return ETS_TYPE_RETURN_CHIP_ERROR;
        }
    }

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetPortEntry(UI32_T lport, ETS_TYPE_PortEntry_T  *entry, ETS_TYPE_DB_T oper_cfg)
{
    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    /*Update OM*/
    ETS_OM_GetLPortEntry(lport, entry, oper_cfg);
    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetNextPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport_p  - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry_p  - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetNextPortEntry(
    UI32_T *lport_p, ETS_TYPE_PortEntry_T  *entry_p, ETS_TYPE_DB_T oper_cfg)
{
    UI32_T  next_lport, ret = ETS_TYPE_RETURN_PORTNO_OOR;

    if ((NULL == lport_p) || (NULL == entry_p))
        return ETS_TYPE_RETURN_INPUT_ERR;

    for (next_lport = *lport_p+1; next_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; next_lport++)
    {
        if (TRUE == SWCTRL_LogicalPortExisting(next_lport))
        {
            ret = ETS_TYPE_RETURN_OK;

            /*Get OM*/
            ETS_OM_GetLPortEntry(next_lport, entry_p, oper_cfg);
            *lport_p = next_lport;
            break;
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetMaxNumberOfTC
 * -------------------------------------------------------------------------
 * FUNCTION: Get max number of ETS  on this machine
 * INPUT   : none
 * OUTPUT  : max_nbr_of_tc - max number of Traffic class (machine depends)
 * RETURN  : ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetMaxNumberOfTC(UI8_T *max_nbr_of_tc)
{

    *max_nbr_of_tc=SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS;

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_Is_Config_Match_Operation
 * -------------------------------------------------------------------------
 * FUNCTION: check if Config Matches Operation
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : TRUE : config = operation
 *           FALSE: config != operation or input lport error
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_Is_Config_Match_Operation(UI32_T lport)
{
    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return FALSE;
    }

    /*check OM
     */
    return ETS_OM_Is_Config_Match_Operation(lport);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_IsAnyPortOperEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: To check is any port operational enabled
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_IsAnyPortOperEnabled(void)
{
    return ETS_OM_IsAnyPortOperEnabled();
}

#if 0 /* not used */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetPortPrioAssignByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: Change mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           tc    - which traffic class of this Cos priority belong
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : only operational OM is written.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_SetPortPrioAssignByDCBx(UI32_T lport, UI32_T prio, UI32_T tc)
{
    ETS_TYPE_MODE_T mode;
    UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return FALSE;
    }
    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);
    if(mode!=ETS_TYPE_MODE_AUTO)
    {
        return ETS_TYPE_RETURN_ILL_MODE;
    }
    /*Update OM
     */
    ETS_OM_SetPortPrioAssign(lport, prio, tc, ETS_TYPE_DB_OPER);

    /*check mapping
     */
    ETS_MGR_GetMapping_QueuetoTC(lport, q2tc, ETS_TYPE_DB_OPER);

    /*set chip
     */
    return ETS_MGR_SetAsicPrioAssign(lport , q2tc);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetWeightByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: Set BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Only operational is updated.
 *           Weight is always 0 for non-ETS TC.
 *           Weight is 0~100 for ETS TC.
 *           For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by ETS_MGR_GetAllTCWeight().
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_SetWeightByDCBx(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    ETS_TYPE_MODE_T mode;
    UI32_T i, total_weight;
    ETS_TYPE_TSA_T tsa;
    BOOL_T any_ets_tc=FALSE;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return FALSE;
    }
    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);
    if(mode!=ETS_TYPE_MODE_AUTO)
    {
        return ETS_TYPE_RETURN_ILL_MODE;
    }

    /* Check if new weight will make total weight > SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT
     */
    total_weight=0;
    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ETS_OM_GetTSA(lport, i,  &tsa, ETS_TYPE_DB_OPER);
        if(tsa==ETS_TYPE_TSA_ETS)
        {
            any_ets_tc=TRUE;
        }
        else
        {
            if(weight[i]!=0)
            {
                return ETS_TYPE_RETURN_INPUT_ERR;
            }
        }
        total_weight+=weight[i];
    }
    if(any_ets_tc==TRUE && total_weight!=SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT)
    {
        ETS_DEBUG_MSG("%s:  total_weight have to be 100 @lport:%lu\r\n", __FUNCTION__, lport);
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    /*Update OM
     */
    if(ETS_OM_SetWeight(lport, weight, ETS_TYPE_DB_OPER)==FALSE)
    {
        return FALSE;
    }
    /*get all weight
     */
    if(ETS_MGR_GetAllTCWeight(lport, weight, ETS_TYPE_DB_OPER)==FALSE)
    {
        return FALSE;
    }
    /* set chip
     */
    return (ETS_MGR_SetAsicTCWeight(lport, weight));
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_RestoreToConfigByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: sync operational OM to configuration one.
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Restore OM and chip setting to be same as UI configuration.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_RestoreToConfigByDCBx(UI32_T lport)
{
    ETS_TYPE_MODE_T mode;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return FALSE;
    }

    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);
    if(mode!=ETS_TYPE_MODE_AUTO)
    {
        return FALSE;
    }

    /* if cos checking failed do nothing
     */
    if (TRUE == ETS_OM_GetIsCosConfigOk())
    {
        /*Update OM*/
        ETS_OM_RestoreToConfig(lport);
        if (FALSE == ETS_MGR_SetAsicByOM(lport))
        {
            return FALSE;
        }
    }

    return TRUE;
}


/* LOCAL SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningPrioAssign(UI32_T lport, UI32_T prio, UI32_T *tc)
{
    UI32_T trunk_ifindex;
    BOOL_T is_static;
    UI32_T local_lport=lport;

    if(tc == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*store trunk member setting instead of trunk itself*/
    if(SWCTRL_LogicalPortIsTrunkPort(lport)==TRUE) /*a TRUNK*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*for trunk member, get the trunk setting
     */
    if(SWCTRL_IsTrunkMember( local_lport, &trunk_ifindex, &is_static)==TRUE)
    {
        local_lport=trunk_ifindex;
    }

    ETS_OM_GetPortPrioAssign(local_lport, prio, tc, ETS_TYPE_DB_CONFIG);

    if(*tc!=SYS_DFLT_ETS_TRAFFIC_CLASS_OF_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningTCTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningTCTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T* tsa)
{
    UI32_T trunk_ifindex;
    BOOL_T is_static;
    UI32_T local_lport=lport;

    if(tsa == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /*Get trunk member setting instead of trunk itself*/
    if(SWCTRL_LogicalPortIsTrunkPort(lport)==TRUE) /*a TRUNK*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /*for trunk member, get the trunk setting
     */
    if(SWCTRL_IsTrunkMember( local_lport, &trunk_ifindex, &is_static)==TRUE)
    {
        local_lport=trunk_ifindex;
    }

    ETS_OM_GetTSA(local_lport, tc, tsa, ETS_TYPE_DB_CONFIG);
    if(*tsa!=SYS_DFLT_ETS_TC_SCHEDULE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningWeight(UI32_T lport,
                UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    SYS_TYPE_Get_Running_Cfg_T ret=SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    ETS_TYPE_TSA_T tsa_tmp;
    UI32_T i, ets_tc_no, ets_tc_total =0;
    UI32_T trunk_ifindex;
    BOOL_T is_static;
    UI32_T local_lport=lport;

    if(weight == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*Get trunk member setting instead of trunk itself
     */
    if(SWCTRL_LogicalPortIsTrunkPort(lport)==TRUE) /*a TRUNK*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*for trunk member, get the trunk setting
     */
    if(SWCTRL_IsTrunkMember( local_lport, &trunk_ifindex, &is_static)==TRUE) /*a member port*/
    {
        local_lport=trunk_ifindex;
    }
    ETS_OM_GetWeight(local_lport, weight, ETS_TYPE_DB_CONFIG);

    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ETS_OM_GetTSA(local_lport, i, &tsa_tmp, ETS_TYPE_DB_CONFIG);
        if(tsa_tmp==ETS_TYPE_TSA_ETS)
        {
            ets_tc_total++;
        }
    }

    /* compare default weight to ETS TCs. Non-ETS TCs get weight=0.
     */
    ets_tc_no=0;
    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ETS_OM_GetTSA(local_lport, i, &tsa_tmp, ETS_TYPE_DB_CONFIG);
        if(tsa_tmp==ETS_TYPE_TSA_ETS)
        {
            if(weight[i] != ets_bw_default[ets_tc_total-1][ets_tc_no])
            {
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
            ets_tc_no++;
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningPortMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to configure.
 * OUTPUT  : mode     -- which mode to set.
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    :
 * -------------------------------------------------------------------------*/
static SYS_TYPE_Get_Running_Cfg_T ETS_MGR_GetRunningPortMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    UI32_T trunk_ifindex;
    BOOL_T is_static;
    UI32_T local_lport=lport;

    if (mode == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*store trunk member setting instead of trunk itself*/
    if(SWCTRL_LogicalPortIsTrunkPort(lport)==TRUE) /*a TRUNK*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*for trunk member, get the trunk setting
     */
    if(SWCTRL_IsTrunkMember( local_lport, &trunk_ifindex, &is_static)==TRUE)
    {
        local_lport=trunk_ifindex;
    }

    ETS_OM_GetMode(local_lport, mode);
    if(*mode == SYS_DFLT_ETS_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_ChkCosMapping1to1
 * -------------------------------------------------------------------------
 * FUNCTION: check is Cos Mapping 1to1 for ETS to go.
 * INPUT   : lport - which port
 * OUTPUT  : None
 * RETURN  : FALSE: mapping invallid
 *           TRUE:  mapping is OK.
 * NOTE    : ETS goes with pri-pri-queue mapping have to be global(all ports shared same mapping)
 *            and 1-to-1 mapping.
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_ChkCosMapping1to1(UI32_T lport)
{
    UI32_T i, j;
    UI32_T color;
    UI8_T  ets_pri2queue[MAX_PRI_VAL+1];
    UI32_T ets_cos2pri[MAX_COS_VAL+1];

    /*Get mapping */
#if (SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT == TRUE)
    L4_PMGR_QOS_GetPortPriorityIngressDscp2Queue(lport, COS_TYPE_PRIORITY_USER, ets_pri2queue);
#else
    L4_PMGR_QOS_GetPriorityIngressDscp2Queue(COS_TYPE_PRIORITY_USER, ets_pri2queue);
#endif

    for(i=0; i<MAX_COS_VAL+1; i++)
    {
        L4_PMGR_QOS_GetIngressCos2Dscp(lport, i, 0, &ets_cos2pri[i], &color);
    }
    /*check if mapping 1-1*/
    for(i=0; i<MAX_COS_VAL; i++)
    {
        for(j=i+1; j<MAX_COS_VAL+1; j++){
            if(ets_cos2pri[i]==ets_cos2pri[j] || ets_pri2queue[i]==ets_pri2queue[j])
                return FALSE;
        }
    }
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetAsicByOM
 * -------------------------------------------------------------------------
 * FUNCTION: Get operational OM and set them to ASIC
 * INPUT   : lport - which port
 * OUTPUT  : None
 * RETURN  : FALSE: Error
 *           TRUE:  Successful
 * NOTE    :
 *
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_SetAsicByOM(UI32_T lport)
{
    UI32_T  q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE],
            tc_weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
    ETS_TYPE_MODE_T mode;

    /*check mapping & set chip for priority-assignment
     */
    if (  (FALSE == ETS_MGR_GetMapping_QueuetoTC(lport, q2tc, ETS_TYPE_DB_OPER))
        ||(FALSE == ETS_MGR_SetAsicPrioAssign(lport, q2tc))
       )
    {
        return FALSE;
    }

    /*check mapping & set chip for priority-assignment
     */
    if (  (FALSE == ETS_MGR_GetAllTCWeight( lport, tc_weight,  ETS_TYPE_DB_OPER))
        ||(FALSE == ETS_MGR_SetAsicTCWeight(lport, tc_weight))
       )
    {
        return FALSE;
    }

    /* reconfig asic when schedule topology is changed
     */
    if (  (FALSE == ETS_OM_GetOperMode(lport, &mode))
       )
    {
        return FALSE;
    }
    ETS_MGR_LocalSetOperMode(lport, mode);

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetAsicPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set Queue to TC mapping to ASIC
 * INPUT   : lport - which port
 *           q2tc  - all queue to TC mapping
 * OUTPUT  : None
 * RETURN  : Bypass result of SWCTRL_SetPortCosGroupMapping().
 * NOTE    :
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_SetAsicPrioAssign(UI32_T lport, UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE])
{
    return (SWCTRL_PMGR_SetPortCosGroupMapping(lport, q2tc));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetAsicTCWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Set weighting to ASIC
 * INPUT   : lport - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : Bypass result of SWCTRL_SetPortCosGroupSchedulingMethod().
 * NOTE    : The scheduling method is alwyas DEV_SWDRVL4_SpWithDrrScheduling.
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_SetAsicTCWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    return (SWCTRL_PMGR_SetPortCosGroupSchedulingMethod(lport, DEV_SWDRVL4_SpWithDrrScheduling,  weight));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetMapping_QueuetoTC
 * -------------------------------------------------------------------------
 * FUNCTION: Get all queue to TC mapping
 * INPUT   : lport - which port
 *           q2tc  - all queue to TC mapping
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE.
 * NOTE    : Both operational and configuration OM are copied.
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_GetMapping_QueuetoTC(UI32_T lport, UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE], ETS_TYPE_DB_T oper_cfg)
{
    UI32_T  i, tc_tmp, color, q_no, get_lport = lport;
    UI32_T  ets_cos2pri[MAX_COS_VAL+1];
    UI8_T   ets_pri2queue[MAX_PRI_VAL+1];
    BOOL_T  ret = FALSE;

    /* bcz it's not ok to get cos mapping on trunk port,
     * use 1st member's cos setting instead
     * NOTE: apply the same setting to all trunk member ports for current design
     */
    if (SWCTRL_LogicalPortIsTrunkPort(lport))
    {
        UI32_T                      i, unit, port, mbr_ifidx;
        SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;

        if (  (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
            &&(trunk_ext_p_info.member_number > 0)
           )
        {
            for (i=0; i<trunk_ext_p_info.member_number; i++)
            {
                unit = trunk_ext_p_info.member_list[i].unit;
                port = trunk_ext_p_info.member_list[i].port;

                SWCTRL_UserPortToIfindex(unit, port, &mbr_ifidx);
                get_lport = mbr_ifidx;
                break;
            }
        }
        else
        {
            return FALSE;
        }
    }

    /*Get mapping */
#if (SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT == TRUE)
    ret = L4_PMGR_QOS_GetPortPriorityIngressDscp2Queue(get_lport, COS_TYPE_PRIORITY_USER, ets_pri2queue);
#else
    ret = L4_PMGR_QOS_GetPriorityIngressDscp2Queue(COS_TYPE_PRIORITY_USER, ets_pri2queue);
#endif

    for (i=0; i<MAX_COS_VAL+1; i++)
    {
        ret = ret && L4_PMGR_QOS_GetPortPriorityIngressCos2Dscp(get_lport, COS_TYPE_PRIORITY_USER,
                        i, 0, &ets_cos2pri[i], &color);
    }

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
    {
        ret  = ret && ETS_OM_GetPortPrioAssign(lport, i, &tc_tmp, oper_cfg);
        if (TRUE == ret)
        {
            q_no = ets_pri2queue[ets_cos2pri[i]];
            q2tc[q_no]=tc_tmp;
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetAllTCWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by ETS_MGR_GetAllTCWeight().
 *           For function will setting chip, get weight by this api.
 *           For function get real OM setting, call ETS_OM_GetWeight().
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_GetAllTCWeight(UI32_T lport,
                        UI32_T tc_weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_TSA_T tsa;
    UI32_T  i;

    ETS_OM_GetWeight(lport, tc_weight, oper_cfg);
    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ETS_OM_GetTSA(lport, i, &tsa, oper_cfg);
        /*For chip, 0 means Strict Prioirity. So work-around here to be 1.
         */
        if(tsa==ETS_TYPE_TSA_ETS && tc_weight[i]==0)
        {
            tc_weight[i]=1;
        }
    }
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetPortPrioAssignByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Change mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           tc    - which traffic class of this Cos priority belong
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are copied.
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetPortPrioAssignByUser(UI32_T lport, UI32_T prio, UI32_T tc)
{
    ETS_TYPE_MODE_T mode;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }

    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);

    /* if cos checking failed do nothing
     */
    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];

        ETS_OM_SetPortPrioAssign(lport, prio, tc, ETS_TYPE_DB_OPER);

        /*check mapping
         */
        if (FALSE == ETS_MGR_GetMapping_QueuetoTC(lport, q2tc, ETS_TYPE_DB_OPER))
        {
            return ETS_TYPE_RETURN_OTHERS;
        }

        if(ETS_MGR_SetAsicPrioAssign(lport , q2tc)==FALSE)
        {
            return ETS_TYPE_RETURN_CHIP_ERROR;
        }
    }

    /*Update OM
     */
    ETS_OM_SetPortPrioAssign(lport, prio, tc, ETS_TYPE_DB_CONFIG);

    /* reconfig asic when schedule topology is changed
      */
    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        ETS_MGR_SetAsicByOM(lport);
    }

    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        /* notify DCBX to re-run port sm
         */
        SYS_CALLBACK_MGR_EtsConfigChangedCallback(SYS_MODULE_ETS, lport);
    }

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetPortPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_GetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg)
{
    if(tc == NULL)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if(prio>SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }

    ETS_OM_GetPortPrioAssign(lport, prio, tc, oper_cfg);

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetWeightByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 *           Weight is always 0 for non-ETS TC.
 *           Weight is 0~100 for ETS TC.
 *           If all weights are 0xffffffff, weights will be reset to default.
 *           For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by ETS_MGR_GetAllTCWeight().
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetWeightByUser(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    ETS_TYPE_MODE_T mode;
    UI32_T          i, total_weight, ets_tc_nbr =0;
    UI32_T          tmp_weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
    ETS_TYPE_TSA_T  tsa;
    BOOL_T          is_do_reset=FALSE;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }

    /* if all weights are  0xffffffff, reset weights to default
     */
    memset(tmp_weight, 0xff, sizeof(tmp_weight));
    if (0 == memcmp(tmp_weight, weight, sizeof(tmp_weight)))
        is_do_reset = TRUE;

    /* Check if new weight will make total weight > SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT
     */
    total_weight=0;
    for (i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ETS_OM_GetTSA(lport, i, &tsa, ETS_TYPE_DB_CONFIG);   /*check configuration only, for user mode confg=operation*/
        if (tsa==ETS_TYPE_TSA_ETS)
        {
            ets_tc_nbr++;
        }
        else
        {
            if ((weight[i] !=0) && (FALSE == is_do_reset))
            {
                return ETS_TYPE_RETURN_INPUT_ERR;
            }
        }
        total_weight+=weight[i];
    }

    if (FALSE == is_do_reset)
    {
        if(( 0 != ets_tc_nbr) && total_weight != SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT)
        {
            ETS_DEBUG_MSG("%s:  total_weight have to be 100 @lport:%lu\r\n", __FUNCTION__, lport);
            return ETS_TYPE_RETURN_INPUT_ERR;
        }
    }
    else
    {
        UI32_T  ets_tc_id =0;

        /* Set default weight to ETS TCs. Non-ETS TCs get weight=0.
         */
        memset(weight, 0, sizeof(ets_bw_default[0]));
        for (i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
        {
            ETS_OM_GetTSA(lport, i, &tsa, ETS_TYPE_DB_CONFIG);   /*check configuration only, for user mode confg=operation*/
            if (tsa == ETS_TYPE_TSA_ETS)
            {
                weight[i] = ets_bw_default[ets_tc_nbr-1][ets_tc_id++];
            }
        }
    }

    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);

    /* if cos checking failed do nothing
     */
    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        ETS_OM_SetWeight(lport, weight, ETS_TYPE_DB_OPER);

        /*get all weight
         */
        ETS_MGR_GetAllTCWeight(lport, tmp_weight, ETS_TYPE_DB_OPER);

        /* set chip
         */
        if(ETS_MGR_SetAsicTCWeight(lport, tmp_weight)==FALSE)
        {
            return ETS_TYPE_RETURN_CHIP_ERROR;
        }
    }

    /*Update OM
     */
    if(ETS_OM_SetWeight(lport, weight, ETS_TYPE_DB_CONFIG)==FALSE)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }

    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        /* notify DCBX to re-run port sm
         */
        SYS_CALLBACK_MGR_EtsConfigChangedCallback(SYS_MODULE_ETS, lport);
    }

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_GetWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg)
{
    if(weight == NULL)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }
    ETS_OM_GetWeight(lport, weight, oper_cfg);
    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Set TSA of a traffic class on the interface from UI.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           tsa      -- what kind of  TSA
 *           oper_cfg -- operation or configuration
 * OUTPUT  : NONE
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Only operational OM is updated.
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_SetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa, ETS_TYPE_DB_T oper_cfg)
{
    UI32_T i, ets_tc_no, ets_tc_total;
    UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
    ETS_TYPE_TSA_T tsa_tmp;

    if (ETS_TYPE_DB_CONFIG == oper_cfg)
    {
        /* update OM
         */
        ETS_OM_SetTSA(lport, tc, tsa, oper_cfg);

        /* Get number of ETS TCs
         */
        ets_tc_total=0;
        for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
        {
            ETS_OM_GetTSA(lport, i, &tsa_tmp, oper_cfg);
            if(tsa_tmp==ETS_TYPE_TSA_ETS)
            {
                ets_tc_total++;
            }
        }
        /* Set default weight to ETS TCs. Non-ETS TCs get weight=0.
         */
        ets_tc_no=0;
        memset(weight, 0, sizeof(weight));
        for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
        {
            ETS_OM_GetTSA(lport, i, &tsa_tmp, oper_cfg);
            if(tsa_tmp==ETS_TYPE_TSA_ETS)
            {
                weight[i] = ets_bw_default[ets_tc_total-1][ets_tc_no];
                ets_tc_no++;
            }
        }
        ETS_OM_SetWeight(lport,  weight, oper_cfg);
    }
    else
    {
        /* copy setting from config
         */
        ETS_OM_SetTSA(lport, tc, tsa, oper_cfg);
        ETS_OM_GetWeight(lport,  weight, ETS_TYPE_DB_CONFIG);
        ETS_OM_SetWeight(lport,  weight, ETS_TYPE_DB_OPER);

        /*get all weight
         */
        ETS_MGR_GetAllTCWeight(lport, weight, ETS_TYPE_DB_OPER);

        /* set chip
         */
        if(ETS_MGR_SetAsicTCWeight(lport, weight)==FALSE)
        {
            return FALSE;
        }
    }

    return TRUE;
}

#if 0 /* not used */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetTSAByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: Set TSA of a traffic class on the interface from UI.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           tsa      -- what kind of  TSA
 * OUTPUT  : NONE
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Only operational OM is updated.
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetTSAByDCBx(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa)
{
    ETS_TYPE_MODE_T mode;

    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }
    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);
    if(mode!=ETS_TYPE_MODE_AUTO)
    {
        return ETS_TYPE_RETURN_ILL_MODE;
    }
    ETS_MGR_SetTSA(lport, tc, tsa, ETS_TYPE_DB_OPER);

    return ETS_TYPE_RETURN_OK;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetTSAByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set TSA of a traffic class on the interface from UI.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           tsa      -- what kind of  TSA
 * OUTPUT  : NONE
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetTSAByUser(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa)
{
    ETS_TYPE_MODE_T mode;

    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }

    ETS_MGR_SetTSA(lport, tc, tsa, ETS_TYPE_DB_CONFIG);

    /* check mode
     */
    ETS_OM_GetMode(lport, &mode);

    /* if cos checking failed do nothing
     */
    if (  (ETS_TYPE_MODE_OFF != mode)
        &&(TRUE == ETS_OM_GetIsCosConfigOk())
       )
    {
        ETS_MGR_SetTSA(lport, tc, tsa, ETS_TYPE_DB_OPER);

        /* notify DCBX to re-run port sm
         */
        SYS_CALLBACK_MGR_EtsConfigChangedCallback(SYS_MODULE_ETS, lport);
    }

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           oper_cfg -- operation or configuration
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_GetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T *tsa, ETS_TYPE_DB_T oper_cfg)
{
    if(tsa == NULL)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if(tc>SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }
    ETS_OM_GetTSA(lport, tc, tsa, oper_cfg);

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to configure.
 * OUTPUT  : mode     -- which mode to set.
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    if(mode == NULL)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }
    ETS_OM_GetOperMode(lport, mode);
    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_LocalSetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS operation mode on the interface.
 * INPUT   : lport        -- which port to configure.
 *           new_opr_mode -- which mode to set.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void ETS_MGR_LocalSetOperMode(
    UI32_T          lport,
    ETS_TYPE_MODE_T new_opr_mode)
{
    /* operation mode:
     *   on -> off : disable l4, reset oper om, restore asic
     *  off -> on  : enable l4, caller need to re-confgure asic
     */
    switch(new_opr_mode)
    {
    case ETS_TYPE_MODE_USER:
        {
            UI32_T  q_id;

            L4_PMGR_QOS_EnablePortPriorityMode(lport, COS_TYPE_PRIORITY_ETS);
            for (q_id = 0; q_id < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; q_id++)
            {
                L4_PMGR_QOS_ResetPortPriorityEgressWrrQueueWeight(lport, COS_TYPE_PRIORITY_ETS, q_id);
            }
            L4_PMGR_QOS_ResetPortPriorityEgressQueueMode(lport, COS_TYPE_PRIORITY_ETS);
        }
        break;
    case ETS_TYPE_MODE_OFF:
        {
            ETS_TYPE_PortEntry_T    tmp_pentry = { {0} };

            UI32_T q2tc[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE] = {0};

            ETS_MGR_SetAsicPrioAssign(lport, q2tc);

            ETS_OM_SetLPortEntry(lport, &tmp_pentry, ETS_TYPE_DB_OPER);
            L4_PMGR_QOS_DisablePortPriorityMode(lport, COS_TYPE_PRIORITY_ETS);
        }
        break;
    default:
		break;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS operation mode on the interface.
 * INPUT   : lport        -- which port to configure.
 *           new_opr_mode -- which mode to set.
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_INPUT_ERR : input not valid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetOperMode(UI32_T lport, ETS_TYPE_MODE_T new_opr_mode)
{
    ETS_TYPE_MODE_T old_opr_mode;

    ETS_OM_GetOperMode(lport, &old_opr_mode);

    if (old_opr_mode != new_opr_mode)
    {
        switch(new_opr_mode)
        {
        case ETS_TYPE_MODE_USER:
        case ETS_TYPE_MODE_OFF:
            /* bcz qos not support trunk, need to apply to all trunk mbr
             */
            if (SWCTRL_LogicalPortIsTrunkPort(lport))
            {
                UI32_T                      i, unit, port, mbr_ifidx;
                SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;

                if (  (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
                    &&(trunk_ext_p_info.member_number > 0)
                   )
                {
                    for (i=0; i<trunk_ext_p_info.member_number; i++)
                    {
                        unit = trunk_ext_p_info.member_list[i].unit;
                        port = trunk_ext_p_info.member_list[i].port;

                        SWCTRL_UserPortToIfindex(unit, port, &mbr_ifidx);
                        ETS_MGR_LocalSetOperMode(mbr_ifidx, new_opr_mode);
                    }
                }
            }
            else
            {
                ETS_MGR_LocalSetOperMode(lport, new_opr_mode);
            }
            break;
        default:
			return ETS_TYPE_RETURN_INPUT_ERR;
        }

        ETS_OM_SetOperMode(lport, new_opr_mode);
    }

	return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to configure.
 * OUTPUT  : mode     -- which mode to set.
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    if(mode == NULL)
    {
        return ETS_TYPE_RETURN_INPUT_ERR;
    }
    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }
    ETS_OM_GetMode(lport, mode);
    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS mode on the interface.
 * INPUT   : lport -- which port to configure.
 *           mode  -- which mode to set.
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
static UI32_T ETS_MGR_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode)
{
    ETS_TYPE_MODE_T old_mode;
    BOOL_T          old_is_any_port_op_en = FALSE,
                    new_is_any_port_op_en = FALSE;

    if (  (FALSE == SWCTRL_LogicalPortExisting(lport))
        ||(FALSE == ETS_OM_GetMode(lport, &old_mode))
       )
    {
        return ETS_TYPE_RETURN_PORTNO_OOR;
    }

    if (old_mode != mode)
    {
        /* if cos is not global and 1-to-1 => operation off
         */
        if (TRUE == ETS_OM_GetIsCosConfigOk())
        {
            old_is_any_port_op_en = ETS_OM_IsAnyPortOperEnabled();

            switch (mode)
            {
            case ETS_TYPE_MODE_OFF:
                ETS_MGR_SetOperMode(lport, ETS_TYPE_MODE_OFF);
                break;
            case ETS_TYPE_MODE_USER:
            case ETS_TYPE_MODE_AUTO:
                /* off -> on  : apply user
                 * auto-> on  : apply user
                 *
                 * off -> auto: apply user
                 * on  -> auto: nothing
                 */
                ETS_MGR_SetOperMode(lport, ETS_TYPE_MODE_USER);

                if (  (ETS_TYPE_MODE_OFF == old_mode)
                    ||(ETS_TYPE_MODE_USER == mode)
                   )
                {
                    ETS_OM_RestoreToConfig(lport);
                    ETS_MGR_SetAsicByOM(lport);
                }
                break;
            }

            new_is_any_port_op_en = ETS_OM_IsAnyPortOperEnabled();

#if (SYS_CPNT_CN == TRUE)
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

				ETS_DEBUG_MSG("notify CN %d\n", new_is_any_port_op_en);
				CN_MGR_SetGlobalOperStatus(cn_global_op_status);
            }
#endif

            /* notify DCBX to re-run port sm
             */
            SYS_CALLBACK_MGR_EtsConfigChangedCallback(SYS_MODULE_ETS, lport);
        }

        ETS_OM_SetMode(lport, mode);
    }

    return ETS_TYPE_RETURN_OK;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_LocalSanityCheck
 * -------------------------------------------------------------------------
 * FUNCTION: To check if data is valid
 * INPUT   : entry_p - pointer to entry to check
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T ETS_MGR_LocalSanityCheck(ETS_TYPE_PortEntry_T  *entry_p)
{
    UI32_T  i, total_weight =0, tmp_tc, is_any_ets = FALSE;
    BOOL_T  ret = FALSE;

    if (NULL != entry_p)
    {
        ret = TRUE;

        for (i=0; i < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
        {
            switch (entry_p->tsa[i])
            {
            case ETS_TYPE_TSA_ETS:
                total_weight += entry_p->tc_weight[i];
                is_any_ets = TRUE;
                break;
            case ETS_TYPE_TSA_SP:
                if (0 != entry_p->tc_weight[i])
                    ret = FALSE;
                break;

            default:
                ret = FALSE;
                break;
            }
        }

        ret = ret && ((FALSE == is_any_ets) || (total_weight == 100));

        if (TRUE == ret)
        {
            for (i=0; i <SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
            {
                tmp_tc = entry_p->priority_assign[i];
                if (tmp_tc >= SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS)
                {
                    ret = FALSE;
                    break;
                }
            }
        }
    }

    return ret;
}

#endif /* #if (SYS_CPNT_ETS == TRUE) */
