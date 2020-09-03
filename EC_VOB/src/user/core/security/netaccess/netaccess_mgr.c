/* Project Name: New Feature
 * File_Name : netaccess_mgr.c
 * Purpose     : NETACCESS initiation and NETACCESS task creation
 *
 * 2006/01/27    : Ricky Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3)
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <stdio.h>
#include "sysfun.h"
#include "sys_time.h"
#include "sys_bld.h"
#include "sys_module.h"

#include "l_cvrt.h"
#include "l_mm.h"
#include "l_rstatus.h"
#include "1x_mgr.h"
#include "1x_om.h"
#include "dot1x_vm.h"
#include "dot1x_sm_auth.h"
#include "amtr_mgr.h"
#include "nmtr_mgr.h"
#include "l2mux_mgr.h"
#include "lacp_pmgr.h"
#include "netaccess_backdoor.h"
#include "netaccess_mgr.h"
#include "netaccess_om.h"
#include "netaccess_vm.h"
#include "l_ipcmem.h"

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_mgr.h"
#include "psec_om.h"
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
#include "portauthsrvc_mgr.h"
#endif

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_om.h"
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE) && (SYS_CPNT_NETWORK_ACCESS == FALSE)
#include "psec_task.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_TAGGED_ETHER_TYPE     0x8100

/* MACRO FUNCTION DECLARATIONS
 */
#define NETACCESS_MGR_IS_NULL_MAC(mac) ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])==0)  /*added by Jinhua.Wei,to remove warning*/
#define NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL) \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
       return (RET_VAL); \
    }

#define NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE() \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
        return; \
    }

#define NETACCESS_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) { \
        return (RET_VAL); \
    }

#define NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() { \
        return; \
    }

/* for linux platform, mgr thread does not need to lock
 */
#define NETACCESS_MGR_LOCK() { \
    }

#define NETACCESS_MGR_UNLOCK() { \
    }
#define SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG                       FALSE

/* enum for L_MM ext trace id for L_MM_Malloc
 */
enum
{
    EXT_TRACE_ID_AUTHENTICATEPACKET_ASYNCRETURN = 1,
};

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T (*NETACCESS_MGR_PreCheckFuncPtr_T)(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason);
typedef BOOL_T (*NETACCESS_MGR_MacCountFuncPtr_T)(UI32_T lport);
typedef struct NETACCESS_MGR_ProcessPortModeFuncPtr_S
{
    NETACCESS_MGR_PreCheckFuncPtr_T   pre_check;
} NETACCESS_MGR_ProcessPortModeFuncPtr_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T NETACCESS_MGR_LocalIsIntrusionPacket(const UI8_T *da, UI16_T ether_type);
static BOOL_T NETACCESS_MGR_LocalIsVisibleSecureMacForUser(const NETACCESS_OM_SecureMacEntry_T *mac_entry);
static BOOL_T NETACCESS_MGR_LocalPreCheckTrunkAndLacp(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason);
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
static BOOL_T NETACCESS_MGR_LocalPreCheckLacp(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason);
#endif
static BOOL_T NETACCESS_MGR_LocalIsRSPANEnabledPort(UI32_T lport);
static BOOL_T NETACCESS_MGR_LocalIsPSecEnabledPort(UI32_T lport);
static BOOL_T NETACCESS_MGR_LocalPreCheckForDot1X(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason);
static BOOL_T NETACCESS_MGR_LocalDoPortPreChecking(UI32_T lport, NETACCESS_PortMode_T port_mode, NETACCESS_MGR_FunctionFailure_T *reason);

static BOOL_T NETACCESS_MGR_LocalCopySecurePortEntry(NETACCESS_MGR_SecurePortEntry_T *dst, const NETACCESS_OM_SecurePortEntry_T *src);
static BOOL_T NETACCESS_MGR_LocalCopySecureAddressEntry(NETACCESS_MGR_SecureAddressEntry_T *dst, const NETACCESS_OM_SecureMacEntry_T *src);
static BOOL_T NETACCESS_MGR_LocalCopyMacAuthenticationPortEntry(NETACCESS_MGR_MacAuthPortEntry_T *dst, const NETACCESS_OM_MacAuthPortEntry_T *src);

#if (SYS_CPNT_DOT1X == TRUE)
static char* NETACCESS_MGR_LocalStrPortMode(UI32_T port_mode);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    static void NETACCESS_MGR_LocalPrintPortMode(UI32_T port_mode);
    static void NETACCESS_MGR_LocalPrintStateMachineState(UI32_T sm_state);
    static void NETACCESS_MGR_LocalPrintMacAddress(const UI8_T *addr);
    static void NETACCESS_MGR_LocalPrintEventBitmap(const NETACCESS_OM_StateMachineEvent_T *bm);
#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

static void NETACCESS_MGR_LocalInitializeFunctionPointer(void);
static BOOL_T NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortAddress(NETACCESS_MGR_SecureAddressFilter_T *in_filter, NETACCESS_MGR_SecureAddressEntry_T *entry);
static BOOL_T NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortPort(NETACCESS_MGR_SecureAddressFilter_T *in_filter, NETACCESS_MGR_SecureAddressEntry_T *entry);
static BOOL_T NETACCESS_MGR_LocalCheckPortModeChangeIssue(UI32_T lport,NETACCESS_PortMode_T new_port_mode);

static BOOL_T NETACCESS_MGR_LocalIsDot1xEnabled(NETACCESS_PortMode_T port_mode);
static NETACCESS_PortMode_T NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                                                        UI32_T dot1x_sys_control,
                                                        UI32_T dot1x_control_mode,
                                                        BOOL_T mac_auth_enabled);
static void NETACCESS_MGR_LocalFreeNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr);

/* STATIC VARIABLE DECLARATIONS
 */
static NETACCESS_MGR_ProcessPortModeFuncPtr_T netaccess_mgr_port_mode_fun_ar[NETACCESS_PORTMODE_MAX-1];

/*  declare variables used for Transition mode
 */
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_InitiateSystemResources
 *---------------------------------------------------------------------------
 * Function : Initialize NETACCESS's MGR .
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_InitiateSystemResources(void)
{
    NETACCESS_OM_CreatSem();
    NETACCESS_OM_InitialSystemResource();

    /*EPR:N/A
         Problem:when system brign up ,sometimes it will hang.
                        LLDP will call pom to radus to get data ,and radius  is taking sem of it.
          RootCause:LLDP will call pom to radus to get data ,and radius  is taking sem of it.
          Solution:when call pom,it doesn't need to sem protect
          File:1x_om.c
    */

    DOT1X_OM_InitSemaphore();
    /* set cookie to database
     */
    NETACCESS_OM_SetDot1xAuthorizedResultCookie (SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY);
    NETACCESS_OM_SetRadiusAuthorizedResultCookie(SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY);

    DOT1X_OM_SetTaskServiceFunPtr(SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, DOT1X_TASK_ASYNC_AUTH_CHECK);
    DOT1X_OM_Initialize();

    /* initialize function pointer
     */
    NETACCESS_MGR_LocalInitializeFunctionPointer();

    PORTAUTHSRVC_MGR_InitiateSystemResource();

    NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_IFO, "done");

    return TRUE;
}/* End of NETACCESS_MGR_InitiateSystemResources() */

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * Function : This function initializes all function pointer registration operations.
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_Create_InterCSC_Relation(void)
{
/* move to NETACCESS_GROUP_HandleSysCallbackIPCMsg
 */
#if 0
    /* link up/down and admin up/down registration
     */
    SWCTRL_Register_UPortLinkUp_CallBack(NETACCESS_MGR_PortLinkUp_CallBack);
    SWCTRL_Register_UPortLinkDown_CallBack(NETACCESS_MGR_PortLinkDown_CallBack);
    SWCTRL_Register_UPortAdminEnable_CallBack(NETACCESS_MGR_UPortAdminEnable_CallBack);
    SWCTRL_Register_UPortAdminDisable_CallBack(NETACCESS_MGR_UPortAdminDisable_CallBack);

    /* VLAN member add/delete registration
     */
    VLAN_MGR_RegisterVlanMemberAdd_CallBack(NETACCESS_MGR_VlanMemberAdd_CallBack);
    VLAN_MGR_RegisterVlanMemberDelete_CallBack(NETACCESS_MGR_VlanMemberDelete_CallBack);

    /* new MAC/eap registration
     */
#if 1
    L2MUX_MGR_Register_Intrusion_Handler(NETACCESS_MGR_AnnounceNewMac_CallBack);

#else
    AMTR_MGR_Register_SecureMac_CallBack(NETACCESS_MGR_AnnounceNewMac_CallBack);
#endif

    L2MUX_MGR_Register_DOT1XMAC_Handler(NETACCESS_MGR_AnnounceEapPacket_CallBack);
#endif

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    NETACCESS_Backdoor_Register_SubsysBackdoorFunc();
#endif

    return;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterMasterMode(void)
{
    UI32_T      debug_flag = 0;

    /* initialize database
     * should initial OM first because VM may reference OM's values
     */
    if (FALSE == NETACCESS_OM_Initialize())
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_Initialize() failed", __FUNCTION__);
    }

    if (FALSE == DOT1X_OM_Initialize())
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[%s] DOT1X_OM_Initialize() failed", __FUNCTION__);
    }

    /* initialize vm
     */
    if (FALSE == NETACCESS_VM_Initialize())
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_VM_Initialize() failed", __FUNCTION__);
    }

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_MG_IFO & debug_flag)
        printf("\r\n[%s] done", __FUNCTION__);

    return;
}/* End of NETACCESS_MGR_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterSlaveMode()
{
    /* enter slave mode
     */
    SYSFUN_ENTER_SLAVE_MODE();
}/* End of NETACCESS_MGR_EnterSlaveMode() */

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request
     */
    SYSFUN_SET_TRANSITION_MODE();
}/* End of NETACCESS_MGR_SetTransitionMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_EnterTransitionMode Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETACCESS enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_EnterTransitionMode()
{
    UI32_T      debug_flag = 0;

    /* enter transition mode,wait other callers leave
     */
    SYSFUN_ENTER_TRANSITION_MODE();

    NETACCESS_OM_ClearAll();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_MG_IFO & debug_flag)
        printf("\r\n[%s] done", __FUNCTION__);
}/* End of NETACCESS_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetCurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of NETACCESS's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   dot1x_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T NETACCESS_MGR_GetCurrentOperationMode()
{
   /* get operating mode
    */
   return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_ProcessNewMacMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from new mac message queue
 * INPUT:  new_mac_msg_p.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ProcessNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_p)
{
    BOOL_T  ret;
    UI32_T debug_flag = 0;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input valid
     */
    if (NULL == new_mac_msg_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[%s] start processing...", __FUNCTION__);
    }

    /* transfer to VM to process new MAC
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_VM_ProcessEventNewMac(new_mac_msg_p);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_ProcessNewMacMsg() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_ProcessTimeoutEvent
 * ---------------------------------------------------------------------
 * PURPOSE : Process timeout event
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ProcessTimeoutEvent(void)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* transfer to VM to process timer expiry
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_VM_ProcessEventTimerUp();
#if (SYS_CPNT_DOT1X == TRUE)
    ret |= DOT1X_VM_ProcessTimeoutEvent();
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_ProcessTimerEvent() */

/*************************
 * For UI (CLI/WEB/SNMP) *
 *************************/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set security modes of the port
 * INPUT:  lport,secure_port_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T secure_port_mode)
{
    BOOL_T  ret = FALSE;
    UI32_T  unit, port, trunk_id;
    NETACCESS_MGR_FunctionFailure_T reason;
    NETACCESS_PortMode_T ori_port_mode;

    NETACCESS_DBG2(NETACCESS_OM_DEBUG_MG_IFO, "start [lport-%ld/mode-%d]", lport, secure_port_mode);

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* FIX IT: SYS_CPNT_PORT_SECURITY_TRUNK means port security only not all security
     * feature.
     */
    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (secure_port_mode == NETACCESS_PORTMODE_PORT_SECURITY )
    {
        if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
            SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    else
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
#else
    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
#endif

    /* get original port mode
     */
    if(FALSE == NETACCESS_OM_GetSecurePortMode(lport, &ori_port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if anything change
     */
    if(ori_port_mode == secure_port_mode)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check if allow port mode change
     */
    if(FALSE == NETACCESS_MGR_LocalCheckPortModeChangeIssue(lport,secure_port_mode))
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "CheckPortModeChangeIssue failed");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if port can set to new port mode
     */
    if(FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, secure_port_mode, &reason))
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "DoPorePreChecking failed");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_LOCK();

    /* set to database
     */
    ret = NETACCESS_OM_SetSecurePortMode(lport, secure_port_mode);

#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d ret:%d \n",__FILE__,__FUNCTION__,__LINE__,ret);
#endif
    if (TRUE == ret)
    {
        /* process port mode change
         */
        ret = NETACCESS_VM_ProcessEventPortModeChange(lport, secure_port_mode);
    }

    NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_IFO, "exit");

    NETACCESS_MGR_UNLOCK();
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d ret:%d \n",__FILE__,__FUNCTION__,__LINE__,ret);
#endif
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetSecurePortMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get security modes of the port
 * INPUT:  lport
 * OUTPUT: secure_port_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    /* allow trunk port to be config */
    if ((NULL == secure_port_mode) ||
        ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))&&
        (SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
        )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
#else
    if ((NULL == secure_port_mode) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
#endif

    /* get from database
     */
    ret = NETACCESS_OM_GetSecurePortMode(lport, secure_port_mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecurePortMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_port_mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT:  secure_port_mode
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecurePortMode(UI32_T lport,NETACCESS_PortMode_T *secure_port_mode)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
        SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetSecurePortMode(lport, secure_port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_SECURE_PORT_MODE == *secure_port_mode) ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecurePortMode() */

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection status on this port.
 * INPUT:  lport, status
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionStatus(UI32_T lport, UI32_T status)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  ori_status;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get original link detection
     */
    if(FALSE == NETACCESS_OM_GetLinkDetectionStatus(lport, &ori_status))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if anything change
     */
    if(ori_status == status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    NETACCESS_MGR_LOCK();

    /* set to database
     */
    ret = NETACCESS_OM_SetLinkDetectionStatus(lport, status);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetLinkDetectionStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection status on this port.
 * INPUT:  lport
 * OUTPUT: status_p
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if (    (NULL == status_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetLinkDetectionStatus(lport, status_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetLinkDetectionStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  status_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection status
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionStatus(UI32_T lport, UI32_T *status_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if port is reasonable
     */
    if (    (NULL == status_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetLinkDetectionStatus(lport, status_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_LINK_DETECTION_STATUS == *status_p) ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningLinkDetectionStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection mode on this port.
 * INPUT:  lport, mode
 * OUTPUT: None
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionMode(UI32_T lport, UI32_T mode)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  ori_mode;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get original link detection
     */
    if(FALSE == NETACCESS_OM_GetLinkDetectionMode(lport, &ori_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if anything change
     */
    if(ori_mode == mode)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    NETACCESS_MGR_LOCK();

    /* set to database
     */
    ret = NETACCESS_OM_SetLinkDetectionMode(lport, mode);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetLinkDetectionMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection mode on this port.
 * INPUT:  lport
 * OUTPUT: mode_p
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if (    (NULL == mode_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetLinkDetectionMode(lport, mode_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetLinkDetectionStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection mode
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionMode(UI32_T lport, UI32_T *mode_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if port is reasonable
     */
    if (    (NULL == mode_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetLinkDetectionMode(lport, mode_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_LINK_DETECTION_MODE == *mode_p) ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningLinkDetectionMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection action on this port.
 * INPUT:  lport, action
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetLinkDetectionAction(UI32_T lport, UI32_T action)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  ori_action;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get original link detection
     */
    if(FALSE == NETACCESS_OM_GetLinkDetectionAction(lport, &ori_action))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if anything change
     */
    if(ori_action == action)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    NETACCESS_MGR_LOCK();

    /* set to database
     */
    ret = NETACCESS_OM_SetLinkDetectionAction(lport, action);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetLinkDetectionAction() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection action on this port.
 * INPUT:  lport
 * OUTPUT: action_p
 * RETURN: TRUE/FALSE.
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if (    (NULL == action_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetLinkDetectionAction(lport, action_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetLinkDetectionAction() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection action is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection action
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningLinkDetectionAction(UI32_T lport, UI32_T *action_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if port is reasonable
     */
    if (    (NULL == action_p)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetLinkDetectionAction(lport, action_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_LINK_DETECTION_ACTION == *action_p) ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningLinkDetectionAction() */
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by lport.
 * INPUT    : lport : logical port.
 *            number:secureNumberAddresses
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureNumberAddresses(UI32_T lport,UI32_T number)
{
    UI32_T  unit, port, trunk_id, number_stored, cur_qty, min_qty, max_qty;
    NETACCESS_PortMode_T port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    /* check if input value is valid and get information from database
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)) ||
#endif
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)) ||
        (FALSE == NETACCESS_OM_GetMinimumSecureAddresses(port_mode, &min_qty)) ||
        (FALSE == NETACCESS_OM_GetSecureMaximumAddresses(lport, &max_qty)) ||
        (number < min_qty) || (number > max_qty) ||
        (FALSE == NETACCESS_OM_GetSecureNumberAddresses(lport, &cur_qty)) ||
        (FALSE == NETACCESS_OM_GetSecureNumberAddressesStored(lport, &number_stored)))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if not change, do nothing
     */
    if (number == cur_qty)
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* if new value is less than current, delete all mac entry on this port
     */
    if (number < number_stored)
    {
        /* when new value is less than current,delete all mac entry on this port
         */
        if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
        {
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    /* set to database
     */
    if (FALSE == NETACCESS_OM_SetSecureNumberAddresses(lport, number))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_SetSecureNumberAddresses() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get secureNumberAddresses by lport.
 * INPUT    : lport : logical port.
 * OUTPUT   : number:secureNumberAddresses.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureNumberAddresses(UI32_T lport, UI32_T *number)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if ((NULL == number) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
#else
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
#endif
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetSecureNumberAddresses(lport, number);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureNumberAddresses() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default number is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT:  number:secureNumberAddresses
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureNumberAddresses(UI32_T lport,UI32_T *number)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input valid
     */
    if ((NULL == number) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetSecureNumberAddresses(lport, number))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_SECURE_ADDRESSES_PER_PORT == *number) ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecureNumberAddresses() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureNumberAddressesStored
 * ---------------------------------------------------------------------
 * PURPOSE: The number of addresses that are currently in the
 *          AddressTable for this port. If this object has the same value as
 *          secureNumberAddresses, then no more addresses can be authorized on this
 *          port.
 * INPUT:  unit, port,secure_number_addresses_stored.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureNumberAddressesStored(UI32_T lport, UI32_T *secure_number_addresses_stored)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if ((NULL == secure_number_addresses_stored) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetSecureNumberAddressesStored(lport, secure_number_addresses_stored);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureNumberAddressesStored() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureMaximumAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maxinum value that secureNumberAddresses
 *          can be set to.
 * INPUT:  lport
 * OUTPUT: secure_maximum_addresses
 * RETURN: TRUE/FALSE.
 * NOTES:
 * This indicates the maximum value that secureNumberAddresses
 * can be set to. It is dependent on the resources available so may change,
 * eg. if resources are shared between ports, then this value can both
 * increase and decrease. This object must be read before setting
 * secureNumberAddresses.
 *
 * The following relationship must allows be preserved.
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureMaximumAddresses(UI32_T lport, UI32_T *secure_maximum_addresses)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if ((NULL == secure_maximum_addresses) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetSecureMaximumAddresses(lport, secure_maximum_addresses);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureMaximumAddresses() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if (NULL == min_addresses)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetMinimumSecureAddresses(port_mode, min_addresses);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the maximum value that secureNumberAddresses can be set to.
 * INPUT    : lport
 * OUTPUT   : usable_addresses
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if ((NULL == usable_addresses) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetUsableSecureAddresses(lport, usable_addresses);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input valid
     */
    if (NULL == unreserved_nbr)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetUnreservedSecureAddresses(unreserved_nbr);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT:  reauth_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureReauthTime(UI32_T reauth_time)
{
    BOOL_T  ret;
    UI32_T ori_reauth_time;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input value is valid
     */
    if((SYS_ADPT_NETACCESS_MAX_REAUTH_TIME < reauth_time) ||
       (SYS_ADPT_NETACCESS_MIN_REAUTH_TIME > reauth_time))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get original reauth time
     */
    if(FALSE == NETACCESS_OM_GetSecureReauthTime(&ori_reauth_time))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if change
     */
    if(reauth_time == ori_reauth_time)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* set to database
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_OM_SetSecureReauthTime(reauth_time);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetSecureRadaDefaultSessionTime() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT:  None.
 * OUTPUT: reauth_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureReauthTime(UI32_T *reauth_time)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input pointer is valid
     */
    if(NULL == reauth_time)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetSecureReauthTime(reauth_time);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureRadaDefaultSessionTime() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default reauth_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  reauth_time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureReauthTime(UI32_T *reauth_time)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input pointer is valid
     */
    if(NULL == reauth_time)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetSecureReauthTime(reauth_time))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (SYS_DFLT_NETACCESS_SECURE_REAUTH_TIME == *reauth_time) ?
            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecureRadaDefaultSessionTime() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Set the auth age.
 * INPUT:  None.
 * OUTPUT: auth_age.
 * RETURN: TRUE/FALSE.
 * NOTES: none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureAuthAge(UI32_T auth_age)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input pointer is valid
     */
    if((NETACCESS_TYPE_MIN_OF_SECURE_MAC_ADDRESS_AUTH_AGE > auth_age) ||
        (NETACCESS_TYPE_MAX_OF_SECURE_MAC_ADDRESS_AUTH_AGE < auth_age))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* set to database
     */
    ret = NETACCESS_OM_SetSecureAuthAgeTime(auth_age);

#if NotSupport
    /* should set to amtr
     */
    ret &= AMTR_MGR_SetSecureAgingTime(auth_age);
#endif

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetSecureAuthAge() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Get the auth age.
 * INPUT:  None.
 * OUTPUT: auth_age.
 * RETURN: TRUE/FALSE.
 * NOTES: none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureAuthAge(UI32_T *auth_age)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input pointer is valid
     */
    if(NULL == auth_age)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetSecureAuthAgeTime(auth_age);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureAuthAge() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default auth_age is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  auth_age
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureAuthAge(UI32_T *auth_age)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input pointer is valid
     */
    if(NULL == auth_age)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetSecureAuthAgeTime(auth_age))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (NETACCESS_TYPE_DFLT_SECURE_MAC_ADDRESS_AUTH_AGE == *auth_age) ?
            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecureAuthAge() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT:  lport,
 *         dynamic_vlan_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureDynamicVlanStatus(UI32_T lport, UI32_T dynamic_vlan_status)
{
    BOOL_T new_value;
    BOOL_T orginal_value;
    NETACCESS_PortMode_T port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    if (dynamic_vlan_status != VAL_networkAccessPortDynamicVlan_enabled
        && dynamic_vlan_status != VAL_networkAccessPortDynamicVlan_disabled)
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    new_value = (dynamic_vlan_status == VAL_networkAccessPortDynamicVlan_enabled)?
                    TRUE : FALSE;

    /* check if port mode can enable dynamic vlan assignment and get original value
     */
    if ((FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)) ||
        (FALSE == NETACCESS_OM_GetDynamicVlanStatus(lport, &orginal_value)))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* if not change, do nothing
     */
    if (orginal_value == new_value)
    {
        /* do nothing
         */
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* set to database
     */
    if (FALSE == NETACCESS_OM_SetSecureDynamicVlanStatus(lport, new_value))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
        {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_SetSecureDynamicVlanStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_vlan_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureDynamicVlanStatus(UI32_T lport, UI32_T *dynamic_vlan_status)
{
    BOOL_T  ret;
    BOOL_T  enabled;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input poinnter is valid
     */
    if(NULL == dynamic_vlan_status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetDynamicVlanStatus(lport, &enabled);

    *dynamic_vlan_status = (enabled) ?
                            VAL_networkAccessPortDynamicVlan_enabled :
                            VAL_networkAccessPortDynamicVlan_disabled;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureDynamicVlanStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT: dynamic_vlan_status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureDynamicVlanStatus(UI32_T lport, UI32_T *status)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    BOOL_T enabled;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input poinnter is valid
     */
    if(NULL == status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetDynamicVlanStatus(lport, &enabled))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    *status = (enabled) ? VAL_networkAccessPortDynamicVlan_enabled :
        VAL_networkAccessPortDynamicVlan_disabled;

    ret = (SYS_DFLT_NETACCESS_DYNAMIC_VLAN_ENABLE == enabled) ?
            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecureDynamicVlanStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic QoS configuration control
 * INPUT:  lport,
 *         dynamic_qos_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDynamicQosStatus(UI32_T lport, UI32_T dynamic_qos_status)
{
    BOOL_T new_value;
    BOOL_T orginal_value;
    NETACCESS_PortMode_T port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    if (dynamic_qos_status != VAL_networkAccessPortLinkDynamicQos_enabled
        && dynamic_qos_status != VAL_networkAccessPortLinkDynamicQos_disabled)
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    new_value = (dynamic_qos_status == VAL_networkAccessPortDynamicVlan_enabled)?
                    TRUE : FALSE;

    /* check if port mode can enable dynamic vlan assignment and get original value
     */
    if ((FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)) ||
        (FALSE == NETACCESS_OM_GetDynamicQosStatus(lport, &orginal_value)))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* if not change, do nothing
     */
    if (orginal_value == new_value)
    {
        /* do nothing */
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* set to database
     */
    if (FALSE == NETACCESS_OM_SetSecureDynamicQosStatus(lport, new_value))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_SetSecureDynamicVlanStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_qos_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDynamicQosStatus(UI32_T lport, UI32_T *dynamic_qos_status)
{
    BOOL_T  enabled;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input poinnter is valid
     */
    if(NULL == dynamic_qos_status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetDynamicQosStatus(lport, &enabled);

    *dynamic_qos_status = (enabled) ? VAL_networkAccessPortLinkDynamicQos_enabled :
        VAL_networkAccessPortLinkDynamicQos_disabled;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetSecureDynamicVlanStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_qos_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT: dynamic_qos_status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDynamicQosStatus(UI32_T lport, UI32_T *status)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    BOOL_T enabled;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input pointer is valid
     */
    if(NULL == status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetDynamicQosStatus(lport, &enabled))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    *status = (enabled) ? VAL_networkAccessPortLinkDynamicQos_enabled :
        VAL_networkAccessPortLinkDynamicQos_disabled;

    ret = (SYS_DFLT_NETACCESS_DYNAMIC_QOS_ENABLE == enabled) ?
            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningSecureDynamicVlanStatus() */

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Set secure filter table
 * INPUT:  filter_id,mac_address,mask,is_add
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetFilterMac(UI32_T filter_id,UI8_T *mac_address,UI8_T *mask,BOOL_T is_add)
{
    BOOL_T  ret;
    UI32_T lport;
    UI32_T temp_filter_id;
    NETACCESS_OM_SecureMacEntry_T mac_entry;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    /* if the new add entry is exist, nothing to do
     */
    if (TRUE == is_add)
    {
        if (TRUE == NETACCESS_OM_IsMacFilterExist(filter_id, mac_address, mask))
        {
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
    }

    /* set to database
     */
    ret = NETACCESS_OM_SetMacFilter(filter_id, mac_address, mask, is_add);

    /* check if ever apply to port
     */
    for(lport = 1;lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        NETACCESS_OM_GetApplyMacFilterByPort(lport, &temp_filter_id);

        /* check if port apply filter_id
         */
        if(temp_filter_id == filter_id)
        {
            /* get secure mac entry by lport
             */
            memset(&mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
            mac_entry.lport = lport;
            while(TRUE == NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
            {
                /* if not same lport,means no record for this port
                 */
                if(lport != mac_entry.lport)
                {
                    break;
                }

                if (FALSE == NETACCESS_OM_IsMacFilterMatched(mac_address, mask, mac_entry.secure_mac))
                {
                    continue;
                }

                if (FALSE == is_add)
                {
                    /* since a MAC filter entry removed,delete all mac-filter mac entry
                     */
                    if (1 == mac_entry.mac_flag.is_mac_filter_mac)
                    {
                        NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index);
                    }
                }
                else
                {
                    /* since secure mac address exist in filter table, delete all authorized mac entry
                     */
                    if (0 == mac_entry.mac_flag.is_mac_filter_mac)
                    {
                        NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index);
                    }
                }
            }
        }
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetMacFilter() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get secure filter table entry
 * INPUT:  filter_id,mac_address,mac_mask
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  TURE means entry exist,FALSE to not exist
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetFilterMac(UI32_T filter_id, UI8_T *mac_address, UI8_T *mac_mask)
{
    BOOL_T  ret;
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* 1,check if input is valid
     */
    if(mac_address == NULL)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* 2,check if input all zero
     */
    if((0 == filter_id) &&
        (0 == memcmp(mac_address, null_mac, SYS_ADPT_MAC_ADDR_LEN)) &&
        (0 == memcmp(mac_mask, null_mac, SYS_ADPT_MAC_ADDR_LEN)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* 3,get from database
     */
    ret = NETACCESS_OM_IsMacFilterExist(filter_id, mac_address, mac_mask);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: filter_id,mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextFilterMac(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* 1,check if input is valid
     */
    if((filter_id == NULL) || (mac_address == NULL) || (mask == NULL))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* 2,get from database
     */
    ret = NETACCESS_OM_GetNextMacFilter(filter_id, mac_address, mask);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetNextMacFilter() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextFilterMacByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry by filter id
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextFilterMacByFilterId(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* 1,check if input is valid
     */
    if(mac_address == NULL)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* 2,get from database
     */
    ret = NETACCESS_OM_GetNextMacFilterByFilterId(filter_id, mac_address, mask);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetNextMacFilter() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next running secure filter table entry
 * INPUT:   filter_id
 * OUTPUT:  mac_address
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default value.
 *        3. Caller has to prepare buffer for storing return value
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningFilterMac(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask)
{
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* 1,check if input is valid
     */
    if((filter_id == NULL) || (mac_address == NULL) || (mask == NULL))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* 2,get from database
     */
    if (FALSE == NETACCESS_OM_GetNextMacFilter(filter_id, mac_address, mask))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
}/* End of NETACCESS_MGR_GetNextRunningMacFilter() */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_SetFilterIdToPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : bind secure filter id to port
 * INPUT    : lport,filter_id
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : Set filter_id to 0 to disable MAC filter on the specified port.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetFilterIdToPort(UI32_T lport,UI32_T filter_id)
{
    BOOL_T  ret;
    UI32_T  orginal_filter_id;
    UI32_T  unit, port, trunk_id;
    NETACCESS_OM_SecureMacEntry_T mac_entry;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_LOCK();

    /* get original setting
     */
    ret = NETACCESS_OM_GetApplyMacFilterByPort(lport, &orginal_filter_id);

    /* check if not change,do nothing
     */
    if (orginal_filter_id == filter_id)
    {
        /* do nothing
         */
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* set to database
     */
    if (FALSE == NETACCESS_OM_SetApplyMacFilterByPort(lport, filter_id))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_entry.lport = lport;

    /* check if any secure mac address entry on this port
     */
    while(TRUE == NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
    {
        /* check if same port
         */
        if(lport != mac_entry.lport)
        {
            break;
        }

        /* disable
         */
        if(0 == filter_id)
        {
            /* since filter-id unapplied to this port,delete all preauth mac entry
             */
            if(1 == mac_entry.mac_flag.is_mac_filter_mac)
            {
                NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index);
            }
        }
        else
        {
            /* check if secure mac address exist in filter table with filter-id
             */
            if (FALSE == NETACCESS_VM_IsMacFilterMatched(lport, mac_entry.secure_mac))
            {
                /* Pre-auth MAC add by old filter ID
                 */
                if (1 == mac_entry.mac_flag.is_mac_filter_mac)
                {
                    NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index);
                }
                continue;
            }

            /* since secure mac address exist in filter table,delete all
             */
            if (0 == mac_entry.mac_flag.is_mac_filter_mac)
            {
                NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index);
            }
        }
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_SetApplyMacFilterByPort() */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get secure filter id which bind to port
 * INPUT    : lport
 * OUTPUT   : filter_id.
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetFilterIdOnPort(UI32_T lport,UI32_T *filter_id)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input is valid
     */
    if((filter_id == NULL) ||
        (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_GetApplyMacFilterByPort(lport, filter_id);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetApplyMacFilterByPort() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningFilterIdOnPort
 * ---------------------------------------------------------------------
 * PURPOSE: Get running secure filter id which bind to port
 * INPUT:   lport
 * OUTPUT:  filter_id
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default value.
 *        3. Caller has to prepare buffer for storing return value
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningFilterIdOnPort(UI32_T lport,UI32_T *filter_id)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;
    UI32_T unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if port is reasonable
     */
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    if (FALSE == NETACCESS_OM_GetApplyMacFilterByPort(lport,filter_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (NETACCESS_TYPE_SECURE_PORT_APPLY_FILTER_ID_NONE == *filter_id) ?
            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningApplyMacFilterByPort() */

#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

/* clear secure MAC part
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_ClearSecureAddressEntryByFilter
 * ---------------------------------------------------------------------
 * PURPOSE: clear secure mac address table entry by filter
 * INPUT:  in_filter.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_ClearSecureAddressEntryByFilter(NETACCESS_MGR_SecureAddressFilter_T *in_filter)
{
    BOOL_T ret = TRUE;
    NETACCESS_OM_SecureMacEntry_T  mac_table;
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0,0,0,0,0,0};

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    /* check if input is valid
     */
    if(NULL == in_filter)
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    memset(&mac_table, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_table.lport = (in_filter->lport==0)?1:in_filter->lport;

    if (NETACCESS_OM_GetDebugFlag() & NETACCESS_OM_DEBUG_MG_TRC)
    {
        enum {LPORT_STR_LEN=10, MAC_STR_LEN = 17};

        char lport_str[LPORT_STR_LEN+1];
        char mac_str[MAC_STR_LEN+1];

        if (in_filter->lport == 0)
        {
            strcpy(lport_str, "any");
        }
        else
        {
            sprintf(lport_str, "%lu", in_filter->lport);
        }

        if (memcmp(in_filter->mac, null_mac, sizeof(in_filter->mac)) == 0)
        {
            strcpy(mac_str, "any");
        }
        else
        {
            sprintf(mac_str, "%02X-%02X-%02X-%02X-%02X-%02X",
                in_filter->mac[0], in_filter->mac[1], in_filter->mac[2],
                in_filter->mac[3], in_filter->mac[4], in_filter->mac[5]);
        }

        NETACCESS_DBG4(NETACCESS_OM_DEBUG_MG_TRC,
            "%s:\r\n"
            " type=%s, lport=%s, MAC=%s",
            __FUNCTION__,
            ((NETACCESS_ADDRESS_ENTRY_TYPE_STATIC == in_filter->type)?"static":
            (NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC== in_filter->type)?"dynamic":"all"),
            lport_str,
            mac_str
            );
    }

    /* get next mac entry to check if need to delete
     */
    while(NETACCESS_OM_GetNextSecureAddressEntry(&mac_table))
    {
        /* check if delete static mac address entry
         */
        if((NETACCESS_ADDRESS_ENTRY_TYPE_STATIC == in_filter->type) &&
            ((1 != mac_table.mac_flag.is_mac_filter_mac) &&
             (1 != mac_table.mac_flag.admin_configured_mac)))
        {
            continue;
        }

        /* check if delete dynamic mac address entry
         */
        if((NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC== in_filter->type) &&
            ((1 != mac_table.mac_flag.auth_by_rada) &&
             (1 != mac_table.mac_flag.authorized_by_dot1x)))
        {
            continue;
        }

        /* check if delete by correct lport
         */
        if((0 != in_filter->lport) &&
            (mac_table.lport != in_filter->lport))
        {
            continue;
        }

        /* check if delete by correct mac address
         */
        if((0 != memcmp(in_filter->mac, null_mac, SYS_ADPT_MAC_ADDR_LEN)) &&
            (0 != memcmp(in_filter->mac, mac_table.secure_mac, SYS_ADPT_MAC_ADDR_LEN)))
        {
            continue;
        }

        /* delete mac address entry
         */
        ret &= NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_table.mac_index);
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#define NETACCESS_MGR_SET_NON_STRING_BUFFER(field) {if(buffer_size < sizeof(field))\
        {\
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);\
        }\
        memcpy(buffer, &field, sizeof(field));\
        *used_buffer = sizeof(field);\
}

#define NETACCESS_MGR_SET_STRING_BUFFER(field) {if(buffer_size < sizeof(field))\
        {\
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);\
        }\
        memcpy(buffer, field, sizeof(field));\
        *used_buffer = sizeof(field);\
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetSecurePortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    BOOL_T  ret;
    UI32_T  *lport;
    UI32_T  unit, port, trunk_id;
    NETACCESS_OM_SecurePortEntry_T port_table;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(-1);

    lport = (UI32_T*)key;

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(*lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    /* check if input buffer is enough
     */
    if(buffer_size < sizeof(NETACCESS_MGR_SecurePortEntry_T))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    /* get secure port entryfrom database
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_OM_GetSecurePortEntry(*lport, &port_table);
    NETACCESS_MGR_UNLOCK();

    /* check if get information from database correctly
     */
    if(FALSE == ret)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    /* get information from secure port entry
     */
    switch(field_id)
    {
    case NETACCESS_FID_SECURE_PORT_ENTRY_IFINDEX:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.lport);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_PORT_MODE:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.port_mode);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_INTRUSION_ACTION:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.intrusion_action);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_NUMBER_ADDRESSES:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.number_addresses);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_NUMBER_ADDRESSES_STORED:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.number_addresses_stored);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_MAXIMUM_ADDRESSES:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.maximum_addresses);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_AUTHORIZED_ADDRESSES:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.nbr_of_authorized_addresses);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_LEARN_AUTHORIZED_ADDRESSES:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.nbr_of_learn_authorized_addresses);
        break;
    case NETACCESS_FID_SECURE_PORT_ENTRY_FILTER_ID:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.filter_id);
        break;

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
    case NETACCESS_FID_SECURE_PORT_ENTRY_DYNAMIC_VLAN_STATUS:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.dynamic_vlan.dynamic_vlan_enabled);
        break;
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    case NETACCESS_FID_SECURE_PORT_ENTRY_DYNAMIC_QOS_STATUS:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(port_table.dynamic_qos.enabled);
        break;
#endif

    case SYS_TYPE_FID_ALL:
        {
            NETACCESS_MGR_SecurePortEntry_T entry;

            ret = NETACCESS_MGR_LocalCopySecurePortEntry(&entry, &port_table);
            memcpy(buffer, &entry, sizeof(NETACCESS_MGR_SecurePortEntry_T));
        }
        *used_buffer = sizeof(NETACCESS_MGR_SecurePortEntry_T);
        break;
    default:
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(0);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecurePortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI32_T *lport;

    lport = (UI32_T*)key;

    /* get next port nbr
     */
    if (FALSE == SWCTRL_GetNextLogicalPort(lport))
    {
        return (-1);
    }

    /* get secure port entry
     */
    if(0 == NETACCESS_MGR_GetSecurePortEntry(field_id, (void*)lport, buffer, buffer_size, used_buffer))
    {
        memcpy(key, lport, sizeof(UI32_T));
        return 0;
    }

    return (-1);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextRunningSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next running secure address entry by the unit and the port and the mac_address.
 * INPUT    : lport, mac_address
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextRunningSecureAddressEntry(UI32_T *lport, UI8_T *mac_address, NETACCESS_MGR_SecureAddressEntry_T *entry)
{
    BOOL_T  ret;
    NETACCESS_OM_SecureMacEntry_T  mac_table;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input is valid
     */
    if ((NULL == lport) ||
        (NULL == mac_address) || (NULL == entry))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    mac_table.lport = *lport;
    memcpy(mac_table.secure_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

    /* get next admin configured secure mac entry
     */
    ret = NETACCESS_OM_GetNextAdminConfiguredSecureAddressEntry(&mac_table);

    /* get information from secure mac entry
     */
    if (TRUE == ret)
    {
        ret = NETACCESS_MGR_LocalCopySecureAddressEntry(entry, &mac_table);

        *lport = mac_table.lport;
        memcpy(mac_address, mac_table.secure_mac, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *entry_key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;
    NETACCESS_OM_SecureMacEntry_T  mac_table;
    BOOL_T is_learnt;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(-1);

    /* check if input is valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(entry_key->lport, &unit, &port, &trunk_id) &&
        SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(entry_key->lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(entry_key->lport, &unit, &port, &trunk_id))
#endif
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    mac_table.lport = entry_key->lport;
    memcpy(mac_table.secure_mac, entry_key->mac_address, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

    /* get secure mac entry from database
     */
    ret = NETACCESS_OM_GetSecureAddressEntry(&mac_table);

    /* check if get information from database correctly
     */
    if (FALSE == ret)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    /* get infromation from secure mac entry
     */
    switch(field_id)
    {
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IFINDEX:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.lport);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_ROW_STATUS:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.authorized_status);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_SERVER_IP:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.server_ip);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_RECORD_TIME:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.record_time);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IS_LEARNT:
        is_learnt = ((mac_table.mac_flag.auth_by_rada == 1) || (mac_table.mac_flag.authorized_by_dot1x== 1)) ? TRUE : FALSE;
        NETACCESS_MGR_SET_NON_STRING_BUFFER(is_learnt);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_MAC:
        NETACCESS_MGR_SET_STRING_BUFFER(mac_table.secure_mac);
        break;
    case SYS_TYPE_FID_ALL:
        {
            NETACCESS_MGR_SecureAddressEntry_T entry;

            ret = NETACCESS_MGR_LocalCopySecureAddressEntry(&entry, &mac_table);
            memcpy(buffer, &entry, sizeof(NETACCESS_MGR_SecureAddressEntry_T));
        }
        *used_buffer = sizeof(NETACCESS_MGR_SecureAddressEntry_T);
        break;
    default:
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(0);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the lport and the mac_address.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *            3.key must be NETACCESS_MGR_SecureAddressEntryKey_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *entry_key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    BOOL_T  ret;
    UI32_T  unit, port, trunk_id;
    NETACCESS_OM_SecureMacEntry_T  mac_table;
    BOOL_T is_learnt;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(-1);

    mac_table.lport = entry_key->lport;
    memcpy(mac_table.secure_mac, entry_key->mac_address, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

    /* get next secure mac entry
     */
    while (NETACCESS_OM_GetNextSecureAddressEntry(&mac_table))
    {
        /* check if not visible mac entry, continue get next
         */
        if (FALSE == NETACCESS_MGR_LocalIsVisibleSecureMacForUser(&mac_table))
        {
            continue;
        }

        /* check if port is reasonable
         */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        if(SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_table.lport, &unit, &port, &trunk_id) &&
           SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_table.lport, &unit, &port, &trunk_id))
#else
        if(SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_table.lport, &unit, &port, &trunk_id))
#endif
        {
            continue;
        }

        /* get information from secure mac entry
         */
        switch(field_id)
        {
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IFINDEX:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.lport);
            break;
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_ROW_STATUS:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.authorized_status);
            break;
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_SERVER_IP:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.server_ip);
            break;
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_RECORD_TIME:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(mac_table.record_time);
            break;
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IS_LEARNT:
            is_learnt = ((mac_table.mac_flag.auth_by_rada == 1) || (mac_table.mac_flag.authorized_by_dot1x== 1)) ? TRUE : FALSE;
            NETACCESS_MGR_SET_NON_STRING_BUFFER(is_learnt);
            break;
        case NETACCESS_FID_SECURE_ADDRESS_ENTRY_MAC:
            NETACCESS_MGR_SET_STRING_BUFFER(mac_table.secure_mac);
            break;
        case SYS_TYPE_FID_ALL:
            {
                NETACCESS_MGR_SecureAddressEntry_T entry;

                ret = NETACCESS_MGR_LocalCopySecureAddressEntry(&entry, &mac_table);
                memcpy(buffer, &entry, sizeof(NETACCESS_MGR_SecureAddressEntry_T));
            }
            *used_buffer = sizeof(NETACCESS_MGR_SecureAddressEntry_T);
            break;
        default:
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
        }

        entry_key->lport = mac_table.lport;
        memcpy(entry_key->mac_address, mac_table.secure_mac, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(0);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextSecureAddressEntryByFilter
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by filter.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : negative integers : error, 0 : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *            3.key must be NETACCESS_MGR_SecureAddressEntryKeyFilter_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_MGR_GetNextSecureAddressEntryByFilter(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKeyFilter_T *entry_key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    BOOL_T  ret = FALSE;
    NETACCESS_MGR_SecureAddressEntry_T entry;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(-1);

    entry.addr_lport = entry_key->lport;
    memcpy(entry.addr_MAC, entry_key->mac_address, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

    /* check in_filter->sort to decide which subroutine will be called
     */
    if(NETACCESS_ADDRESS_ENTRY_SORT_ADDRESS == entry_key->filter.sort)
    {
        /* sort by mac address
         */
        ret = NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortAddress(&entry_key->filter, &entry);
    }
    else
    {
        /* sort by interface
         */
        ret = NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortPort(&entry_key->filter, &entry);
    }

    /* check if get next successfully
     */
    if(FALSE == ret)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    /* get information from secure mac entry
     */
    switch(field_id)
    {
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IFINDEX:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(entry.addr_lport);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_ROW_STATUS:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(entry.addr_row_status);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_SERVER_IP:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(entry.server_ip);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_RECORD_TIME:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(entry.record_time);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_IS_LEARNT:
        NETACCESS_MGR_SET_NON_STRING_BUFFER(entry.is_learnt);
        break;
    case NETACCESS_FID_SECURE_ADDRESS_ENTRY_MAC:
        NETACCESS_MGR_SET_STRING_BUFFER(entry.addr_MAC);
        break;
    case SYS_TYPE_FID_ALL:
        memcpy(buffer, &entry, sizeof(NETACCESS_MGR_SecureAddressEntry_T));
        *used_buffer = sizeof(NETACCESS_MGR_SecureAddressEntry_T);
        break;
    default:
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(-1);
    }

    entry_key->lport = entry.addr_lport;
    memcpy(entry_key->mac_address, entry.addr_MAC, SYS_ADPT_MAC_ADDR_LEN); /* suppose mac_address has right size */

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(0);
}

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDebugFlag(UI32_T debug_flag)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* set to database
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_OM_SetDebugFlag(debug_flag);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_ShowStateMachineStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : show state machine's status
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_ShowStateMachineStatus(UI32_T lport)
{
    BOOL_T  ret;

    NETACCESS_OM_StateMachine_T     state_machine;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get from database
     */
    ret = NETACCESS_OM_GetPortStateMachine(lport, &state_machine);
    if (TRUE == ret)
    {
        printf("\r\nlport: %lu", lport);

        printf("\r\nrunning_port_mode: ");
        NETACCESS_MGR_LocalPrintPortMode(state_machine.running_port_mode);

        printf("\r\nnew_port_mode: ");
        NETACCESS_MGR_LocalPrintPortMode(state_machine.new_port_mode);

        printf("\r\n< port mode change state machine >");
        printf("\r\n\tstatus: ");
        NETACCESS_MGR_LocalPrintStateMachineState(state_machine.port_mode_change_sm.running_state);

        printf("\r\n< port security state machine >");
        printf("\r\n\tstatus: ");
        NETACCESS_MGR_LocalPrintStateMachineState(state_machine.port_security_sm.running_state);

        printf("\r\n\tnew_mac_msg(%d) radius_msg(%d) dot1x_msg(%d)",
            (NULL != state_machine.port_security_sm.new_mac_msg),
            (NULL != state_machine.port_security_sm.radius_msg),
            (NULL != state_machine.port_security_sm.dot1x_msg));

        printf("\r\n\tevent flags:");
        NETACCESS_MGR_LocalPrintEventBitmap(&state_machine.port_security_sm.event_bitmap);

        printf("\r\n\tauthenticating mac:");
        NETACCESS_MGR_LocalPrintMacAddress(state_machine.port_security_sm.authenticating_mac);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_ShowSecureAddressByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : show secure address by mac_index
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_ShowSecureAddressByIndex(UI32_T mac_index)
{
    BOOL_T  ret;

    NETACCESS_OM_SecureMacEntry_T     mac_entry;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get from database
     */
    mac_entry.mac_index = mac_index;
    ret = NETACCESS_OM_GetSecureAddressEntryByIndex(&mac_entry);
    if (TRUE == ret)
    {
        printf("\r\n          mac_index: %lu", mac_entry.mac_index);
        printf("\r\n              lport: %lu", mac_entry.lport);
        printf("\r\n         secure_mac: ");
        NETACCESS_MGR_LocalPrintMacAddress(mac_entry.secure_mac);

        printf("\r\n    addr_row_status: %lu", mac_entry.addr_row_status);

        printf("\r\n        record_time: %lu", mac_entry.record_time);
        printf("\r\n  authorized_status: %lu", mac_entry.authorized_status);
        printf("\r\n       session_time: %lu", mac_entry.session_time);
        printf("\r\n       holdoff_time: %lu", mac_entry.holdoff_time);
        printf("\r\nsession_expire_time: %lu", mac_entry.session_expire_time);

        printf("\r\n   add_on_port_mode: ");
        NETACCESS_MGR_LocalPrintPortMode(mac_entry.add_on_what_port_mode);

        printf("\r\n           mac_flag: ");
        printf("\r\n\t\t(%3s) eap_packet", (1 == mac_entry.mac_flag.eap_packet) ? "on" : "off");
        printf("\r\n\t\t(%3s) authorized_by_dot1x", (1 == mac_entry.mac_flag.authorized_by_dot1x) ? "on" : "off");
        printf("\r\n\t\t(%3s) admin_configured_mac", (1 == mac_entry.mac_flag.admin_configured_mac) ? "on" : "off");
        printf("\r\n\t\t(%3s) applied_to_chip", (1 == mac_entry.mac_flag.applied_to_chip) ? "on" : "off");
        printf("\r\n\t\t(%3s) write_to_amtr", (1 == mac_entry.mac_flag.write_to_amtr) ? "on" : "off");
        printf("\r\n\t\t(%3s) is_hidden_mac", (1 == mac_entry.mac_flag.is_hidden_mac) ? "on" : "off");
        printf("\r\n\t\t(%3s) is_mac_filter_mac", (1 == mac_entry.mac_flag.is_mac_filter_mac) ? "on" : "off");
        printf("\r\n\t\t(%3s) auth_by_rada", (1 == mac_entry.mac_flag.auth_by_rada) ? "on" : "off");
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_HandleHotInsertion
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 *-----------------------------------------------------------------------------------
 */
void NETACCESS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
//NotFinish
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_HandleHotRemoval
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 *-----------------------------------------------------------------------------------
 */
void NETACCESS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
//NotFinish
}

#if (SYS_CPNT_PORT_SECURITY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityStatus
 *-------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : lport                  - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Port security doesn't support
 *           1) unknown port, 2) trunk member, and 3) trunk port
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityStatus(UI32_T lport, UI32_T portsec_status)
{
    UI32_T  unit, port, trunk_id;
    NETACCESS_MGR_FunctionFailure_T reason;
    UI32_T ori_portsec_status;
    NETACCESS_PortMode_T secure_port_mode;
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d \n",__FILE__,__FUNCTION__,__LINE__);
#endif
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    /* allow trunk port to be config */
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT &&
         SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_TRUNK_PORT)
    {
        return FALSE;
    }
#else
    /* check if port is reasonable
     */
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
    {
        return FALSE;
    }
#endif

    /* get original port security status
     */
    if(FALSE == PSEC_OM_GetPortSecurityStatus(lport, &ori_portsec_status))
    {
        return FALSE;
    }

    /* check if change
     */
    if(ori_portsec_status == portsec_status)
    {
        return TRUE;
    }

    /* if global status is disable,enter noRestriction mode
     */
    if(VAL_portSecPortStatus_disabled == portsec_status)
    {
        /* get secure port mode
         */
        if((TRUE == NETACCESS_MGR_GetSecurePortMode(lport, &secure_port_mode)) &&
            (NETACCESS_PORTMODE_PORT_SECURITY != secure_port_mode) &&
            (NETACCESS_PORTMODE_NO_RESTRICTIONS != secure_port_mode))
        {
            return TRUE;
        }
        /* get secure port mode of current input */
        secure_port_mode = NETACCESS_PORTMODE_NO_RESTRICTIONS;
    }
    else
    {
        secure_port_mode = NETACCESS_PORTMODE_PORT_SECURITY;
    }

    /*
    EPR:       ES4827G-FLF-ZZ-00440
    Problem:   Display incorrect information when enable port security after enable LACP
    rootcasue: Set value to NETACCESS_MGR will fail when the interface is config with LACP.
               But set value to PSEC_MGR is succeed before it. It cause show running incorrect.
    sloution:  add check condition for LACP before both 'NETACCESS_MGR' and 'PSEC_MGR' with input secure mode
    File:      netaccess_mgr.c
    */
    /* check if allow port mode change
     */
    if(FALSE == NETACCESS_MGR_LocalCheckPortModeChangeIssue(lport,secure_port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if port can set to new port mode
     */
    if((FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, secure_port_mode, &reason))
        && (reason!=NETACCESS_COZ_PSEC_ENABLED) )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* set to database
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d lport:%d portsec_status:%d \n",__FILE__,__FUNCTION__,__LINE__,lport, portsec_status);
#endif
    if(TRUE == PSEC_MGR_SetPortSecurityStatus(lport, portsec_status))
    {
    // junying: need port review for adding port security to netaccess state machine ...
    //    return NETACCESS_MGR_SetSecurePortMode(lport, secure_port_mode);
        return TRUE;
    }
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityMaxMacCount
 *-------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : lport          -- which port to
 *           max_mac_count  -- maximum mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Default : max_mac_count = 1
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityMaxMacCount(UI32_T lport, UI32_T max_mac_count)
{
    UI32_T  unit, port, trunk_id;
    NETACCESS_MGR_FunctionFailure_T reason;
    UI32_T portsec_status;
    NETACCESS_PortMode_T secure_port_mode;
    UI32_T pre_mac_count;

    /* check if port reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT &&
         SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_TRUNK_PORT)
#else
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
#endif
    {
        return FALSE;
    }

    /* get original max mac count
     */
    PSEC_MGR_GetRunningPortSecurityMacCount(lport, &pre_mac_count);

    /* check if change
     */
    if(pre_mac_count == max_mac_count)
    {
        return TRUE;
    }

    /* get port security status
     */
    if(FALSE == PSEC_OM_GetPortSecurityStatus(lport, &portsec_status))
    {
        return FALSE;
    }

    /* if global status is disable,enter noRestriction mode
     */
    if((VAL_portSecPortStatus_disabled == portsec_status) &&
        (0 == max_mac_count))
    {
        /* get secure port mode
         */
        if((TRUE == NETACCESS_MGR_GetSecurePortMode(lport, &secure_port_mode)) &&
            (NETACCESS_PORTMODE_PORT_SECURITY != secure_port_mode) &&
            (NETACCESS_PORTMODE_NO_RESTRICTIONS != secure_port_mode))
        {
            return FALSE;
        }

        /* get secure port mode of current input */
        secure_port_mode = NETACCESS_PORTMODE_NO_RESTRICTIONS;
    }
    else
    {
        secure_port_mode = NETACCESS_PORTMODE_PORT_SECURITY;
    }


    /*
    EPR:       ES4827G-FLF-ZZ-00440
    Problem:   Display incorrect information when enable port security after enable LACP
    rootcasue: Set value to NETACCESS_MGR will fail when the interface is config with LACP.
               But set value to PSEC_MGR is succeed before it. It cause show running incorrect.
    sloution:  add check condition for LACP before both 'NETACCESS_MGR' and 'PSEC_MGR' with input secure mode
    File:      netaccess_mgr.c
    */
    /* check if allow port mode change
     */
    if(FALSE == NETACCESS_MGR_LocalCheckPortModeChangeIssue(lport,secure_port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if port can set to new port mode
     */
    if((FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, secure_port_mode, &reason))
        && (reason!=NETACCESS_COZ_PSEC_ENABLED))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }



    /* set to database
     */
    if(TRUE == PSEC_MGR_SetPortSecurityMacCount(lport, max_mac_count))
    {
        //return NETACCESS_MGR_SetSecurePortMode(lport, NETACCESS_PORTMODE_PORT_SECURITY);
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetPortSecurityActionStatus
 *-------------------------------------------------------------------------
 * FUNCTION: This function will set port security action status
 * INPUT   : lport          -- which port to
 *           action_status  -- action status
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : if the port is not in portSecurity port mode,will return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetPortSecurityActionStatus (UI32_T lport, UI32_T action_status)
{
    UI32_T unit, port, trunk_id;
    NETACCESS_PortMode_T secure_port_mode;

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT &&
         SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_TRUNK_PORT)
#else
    if (SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
#endif
    {
        return FALSE;
    }

    /* check if port is in portSecurity mode or noRestriction mode
     */
    if((TRUE == NETACCESS_MGR_GetSecurePortMode(lport, &secure_port_mode)) &&
        (NETACCESS_PORTMODE_PORT_SECURITY != secure_port_mode) &&
        (NETACCESS_PORTMODE_NO_RESTRICTIONS != secure_port_mode))
    {
        return FALSE;
    }

    /* set to database
     */
    return PSEC_MGR_SetPortSecurityActionStatus(lport, action_status);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_ConvertPortSecuritySecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
NETACCESS_MGR_ConvertPortSecuritySecuredAddressIntoManual(
    UI32_T ifindex)
{
    if (TRUE != PSEC_MGR_ConvertSecuredAddressIntoManual(ifindex))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_MGR_SecureEntryDeleteFromAmtr_Callback
 *---------------------------------------------------------------------------
 * Function : This function will be notified when amtr delete a secure entry
 *            and only update vlan counter or delete mac address entry in database.
 * Input    : ifindex -- ifindex of deleted entry
 *            mac -- MAC address of deleted entry
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_MGR_SecureEntryDeleteFromAmtr_Callback(UI32_T ifindex, UI8_T *mac)
{
    NETACCESS_OM_SecureMacEntry_T entry;
    UI32_T      debug_flag = 0;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
    NETACCESS_MGR_LOCK();

    debug_flag = NETACCESS_OM_GetDebugFlag();
    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[%s]:ifindex is %ld,MAC is", __FUNCTION__, ifindex);
        printf("%02X-%02X-%02X-%02X-%02X-%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }

    memset(&entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    entry.lport = ifindex;
    memcpy(entry.secure_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get MAC entry from database
     */
    if(FALSE == NETACCESS_OM_GetSecureAddressEntry(&entry))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /* decrease vlan counter and if vlan counter become zero,remove MAC entry
     */
    if(1 < entry.vlan_counter)
    {
        /* only decrease vlan counter
         */
        entry.vlan_counter--;
        NETACCESS_OM_UpdateSecureAddressEntryByIndex(&entry);
    }
    else
    {
        /* vlan counter will become zero, so delete this MAC entry
         */
        NETACCESS_OM_DeleteSecureAddressEntryByIndex(entry.mac_index);
    }

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NETACCESS_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for netaccess mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = NETACCESS_MGR_MSG_CMD(ipcmsg_p)))
    {
        case NETACCESS_MGR_IPC_CMD_CLR_SEC_ADR_ENT_BY_FTR:
        {
            NETACCESS_MGR_SecureAddressFilter_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_ClearSecureAddressEntryByFilter(
                data_p);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_ENT:
        {
            NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetMacAuthPortEntry(
                data_p->field_id, &data_p->entry_key, &data_p->entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_INTR_ACTION:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetMacAuthPortIntrusionAction(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_MAX_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetMacAuthPortMaxMacCount(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetMacAuthPortStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

#if (SYS_CPNT_DOT1X == TRUE)
        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PORT_INTR_ACTN:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xPortIntrusionAction(
                &data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

        case NETACCESS_MGR_IPC_CMD_GET_NXT_MAC_AUTH_PORT_ENT:
        {
            NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextMacAuthPortEntry(
                data_p->field_id, &data_p->entry_key, &data_p->entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT:
        {
            NETACCESS_MGR_IPCMsg_SecAdrEnt_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextSecureAddressEntry(
                data_p->field_id, &data_p->key, &data_p->entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT_BY_FTR:
        {
            NETACCESS_MGR_IPCMsg_SecAdrEnt_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextSecureAddressEntryByFilter(
                data_p->field_id, &data_p->key_ftr, &data_p->entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_PORT_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_SecPrtEnt_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextSecurePortEntry(
                data_p->field_id, &data_p->entry_key, &data_p->prt_entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T);
            break;
        }

#if (SYS_CPNT_DOT1X == TRUE)
        case NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_PORT_INTR_ACTN:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningDot1xPortIntrusionAction(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_INTR_ACTN:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningMacAuthPortIntrusionAction(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_MAX_MAC:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningMacAuthPortMaxMacCount(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_STAS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningMacAuthPortStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_GUEST_VLAN_ID:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningSecureGuestVlanId(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_NBR_ADR:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningSecureNumberAddresses(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_REAUTH_TIME:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningSecureReauthTime(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_ADR_ENT:
        {
            NETACCESS_MGR_IPCMsg_SecAdrEnt_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureAddressEntry(
                data_p->field_id, &data_p->key, &data_p->entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_AUTH_AGE:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureAuthAge(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
        case NETACCESS_MGR_IPC_CMD_SET_SEC_DYN_VLAN_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetSecureDynamicVlanStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_DYN_VLAN_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureDynamicVlanStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_DYN_VLAN_STAS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningSecureDynamicVlanStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
        case NETACCESS_MGR_IPC_CMD_SET_DYN_QOS_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDynamicQosStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_DYN_QOS_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDynamicQosStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_DYN_QOS_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningDynamicQosStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)

        case NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC:
        {
            NETACCESS_MGR_IPCMsg_FidMac_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextFilterMac(
                &data_p->filter_id, data_p->mac_address, data_p->mask);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC_BY_FTR_ID:
        {
            NETACCESS_MGR_IPCMsg_FidMac_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextFilterMacByFilterId(
                data_p->filter_id, data_p->mac_address, data_p->mask);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_MAC:
        {
            NETACCESS_MGR_IPCMsg_FidMac_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningFilterMac(
                &data_p->filter_id, data_p->mac_address, data_p->mask);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_FTR_MAC:
        {
            NETACCESS_MGR_IPCMsg_FidMac_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetFilterMac(
                data_p->filter_id, data_p->mac_address, data_p->mask);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_FTR_MAC:
        {
            NETACCESS_MGR_IPCMsg_FidMacAct_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetFilterMac(
                data_p->filter_id, data_p->mac_address, data_p->mask, data_p->is_add);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_FTR_ID_ON_PORT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetFilterIdOnPort(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_ID_ON_PORT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningFilterIdOnPort(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_FTR_ID_TO_PORT:
        {
            NETACCESS_MGR_IPCMsg_FidPortAct_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetFilterIdToPort(
                data_p->lport, data_p->filter_id);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

        case NETACCESS_MGR_IPC_CMD_GET_SEC_GUEST_VLAN_ID:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureGuestVlanId(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_NBR_ADR:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureNumberAddresses(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_SecPrtEnt_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecurePortEntry(
                data_p->field_id, &data_p->entry_key, &data_p->prt_entry, data_p->buf_size, &data_p->used_buf);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_REAUTH_TIME:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecureReauthTime(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
        case NETACCESS_MGR_IPC_CMD_GET_MAC_ADR_AGING_MODE:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetMacAddressAgingMode(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_ADR_AGING_MODE:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningMacAddressAgingMode(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_MAC_ADR_AGING_MODE:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetMacAddressAgingMode(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif /* (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */

        case NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_INTR_ACTION:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetMacAuthPortIntrusionAction(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_MAX_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetMacAuthPortMaxMacCount(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetMacAuthPortStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_SEC_AUTH_AGE:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetSecureAuthAge(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_SEC_GUEST_VLAN_ID:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetSecureGuestVlanId(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_SEC_NBR_ADR:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetSecureNumberAddresses(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_SEC_REAUTH_TIME:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetSecureReauthTime(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#if (SYS_CPNT_DOT1X == TRUE)
        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_MAX:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortReAuthMax(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        /* begin for ieee_8021x.c/cli_api_1x.c
         */
        case NETACCESS_MGR_IPC_CMD_DO_1X_REAUTHEN:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_DoDot1xReAuthenticate(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_CFG_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXacpe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xAuthConfigEntry(
                data_p->lport, &data_p->xacpe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_STS_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXaspe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xAuthStatsEntry(
                data_p->lport, &data_p->xaspe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_DIAG_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXadpe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xAuthDiagEntry(
                data_p->lport, &data_p->xadpe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PAE_PORT_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXpape_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPaePortEntry(
                data_p->lport, &data_p->xpape);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SEVR_TOUT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortAuthServerTimeout(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SUPP_TOUT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortAuthSuppTimeout(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTHORIZED:
        {
            NETACCESS_MGR_IPCMsg_LportControlPort_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortAuthorized(
                data_p->lport, &data_p->port_control);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportControlPort_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_CTRL_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortControlMode(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_DETAILS:
        {
            NETACCESS_MGR_IPCMsg_LportXpdtl_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortDetails(
                data_p->lport, &data_p->xpdtl);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpdtl_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_INTR_ACTN:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortIntrusionAction(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MAX_REQ:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortMaxReq(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_OPER_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortOperationMode(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_QUIET_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortQuietPeriod(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_ENABLED:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortReAuthEnabled(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_MAX:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortReAuthMax(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortReAuthPeriod(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_TX_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortTxPeriod(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_SESS_STS_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXasse_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xSessionStatsEntry(
                data_p->lport, &data_p->xasse);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportNapm_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetSecurePortMode(
                data_p->lport, &data_p->nacpm);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportNapm_T);
            break;
        }


        case NETACCESS_MGR_IPC_CMD_GET_1X_SYS_AUTH_CTRL:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xSystemAuthControl(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_CFG_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXacpe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xAuthConfigEntry(
                &data_p->lport, &data_p->xacpe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_DIAG_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXadpe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xAuthDiagEntry(
                &data_p->lport, &data_p->xadpe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_STS_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXaspe_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xAuthStatsEntry(
                &data_p->lport, &data_p->xaspe);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PAE_PORT_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXpape_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xPaePortEntry(
                &data_p->lport, &data_p->xpape);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NXT_1X_SESS_STS_ENTRY:
        {
            NETACCESS_MGR_IPCMsg_LportXasse_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xSessionStatsEntry(
                &data_p->lport, &data_p->xasse);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_ADMIN_CTRL_DIR:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortAdminCtrlDirections(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SEVR_TOUT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortAuthServerTimeout(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SUPP_TOUT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortAuthSuppTimeout(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_TX_ENABLED:
        {
            NETACCESS_MGR_IPCMsg_LportData_T   *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortAuthTxEnabled(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_CFG_SETTING_TO_DFLT:
        {
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xConfigSettingToDefault();
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_CTRL_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortControlMode(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_OPER_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortOperationMode(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_INTR_ACTN:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortIntrusionAction(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MAX_REQ:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortMaxReq(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MULHOST_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortMultiHostMacCount(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_PAE_PORT_INIT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortPaePortInitialize(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_QUIET_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortQuietPeriod(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_ENABLED:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortReAuthEnabled(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortReAuthPeriod(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_PORT_TX_PERIOD:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xPortTxPeriod(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_1X_SYS_AUTH_CTRL:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xSystemAuthControl(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
        case NETACCESS_MGR_IPC_CMD_SET_1X_EAPOL_PASS_TRHOU:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetDot1xEapolPassThrough(
                data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_1X_EAPOL_PASS_TRHOU:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xEapolPassThrough(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_EAPOL_PASS_TRHOU:
        {
            NETACCESS_MGR_IPCMsg_U32Data_T  *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningDot1xEapolPassThrough(
                &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T);
            break;
        }
#endif /* SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH */

#if (SYS_CPNT_PORT_SECURITY == TRUE)
        /* begin for cli_api_ethernet.c [port security]
         */
        case NETACCESS_MGR_IPC_CMD_SET_PSEC_ACTN_STAS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetPortSecurityActionStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_PSEC_MAX_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetPortSecurityMaxMacCount(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_PSEC_STAS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetPortSecurityStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_CONVERT_PSEC_SECURED_ADDRESS_INTO_MANUAL:
        {
            NETACCESS_MGR_IPCMsg_Interface_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_ConvertPortSecuritySecuredAddressIntoManual(data_p->ifindex);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
        case NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MULHOST_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetDot1xPortMultiHostMacCount(
            data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_OPER_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xPortOperationMode(
                &data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_MULHOST_MAC_CNT:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetNextDot1xPortMultiHostMacCount(
                &data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        case NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetLinkDetectionStatus(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetLinkDetectionStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_STATUS:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningLinkDetectionStatus(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetLinkDetectionMode(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetLinkDetectionMode(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_MODE:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningLinkDetectionMode(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_ACTION:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_SetLinkDetectionAction(
                data_p->lport, data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_ACTION:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetLinkDetectionAction(
                data_p->lport, &data_p->data);
            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }

        case NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_ACTION:
        {
            NETACCESS_MGR_IPCMsg_LportData_T    *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = NETACCESS_MGR_GetRunningLinkDetectionAction(
                data_p->lport, &data_p->data);

//printf("%s ret=%lu\n", __FUNCTION__, data_p->data);

            ipcmsg_p->msg_size = NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T);
            break;
        }
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

        /* begin for asynchronous call
         */
        case NETACCESS_MGR_IPC_CMD_AUTHENTICATE_PACKET:
        {
            NETACCESS_MGR_IPCMsg_AuthenticatePacket_T *data_p = NETACCESS_MGR_MSG_DATA(ipcmsg_p);
            NETACCESS_MGR_IPCMsg_AuthenticatePacket_T *cookie=NULL;

            /* Handler might enqueue this msg first and then dequeue by another thread,
             * so shouldn't pass syscallback_msg_p->callback_data to handler directly.
             * Allocate a buffer for it and free it when handler sent this packet
             * to next handler by calling SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
             */
            cookie = L_MM_Malloc(sizeof(NETACCESS_MGR_IPCMsg_AuthenticatePacket_T),
                L_MM_USER_ID2(SYS_MODULE_NETACCESS, EXT_TRACE_ID_AUTHENTICATEPACKET_ASYNCRETURN));

            if (cookie == NULL)
            {
                NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
                break;
            }

            memcpy(cookie, data_p, sizeof(NETACCESS_MGR_IPCMsg_AuthenticatePacket_T));

            NETACCESS_MGR_AuthenticatePacket(
                L_IPCMEM_GetPtr(data_p->lan_cbdata.mref_handle_offset),
                data_p->lan_cbdata.pkt_length,
                data_p->lan_cbdata.dst_mac,
                data_p->lan_cbdata.src_mac,
                data_p->lan_cbdata.tag_info,
                data_p->lan_cbdata.type,
                data_p->lan_cbdata.src_unit,
                data_p->lan_cbdata.src_port,
                cookie);

            NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
        }
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    if (NETACCESS_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    if(cmd < NETACCESS_MGR_IPC_CMD_ASYNC_CALL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* NETACCESS_MGR_HandleIPCReqMsg */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AuthenticatePacket
 * ---------------------------------------------------------------------
 * PURPOSE:Asynchronous call to authenticate packet
 * INPUT:  mref_handle_p  - MREF handle for packet
 *         pkt_length     - packet length
 *         dst_mac        - destination mac address
 *         src_mac        - source mac address
 *         tag_info       - vlan tag information
 *         ether_type     - ehternet type
 *         src_unit       - source unit
 *         src_port       - source port
 *         cookie         - shall be passed to next CSC via
 *                          SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_AuthenticatePacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T pkt_length,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI16_T tag_info,
    UI16_T ether_type,
    UI32_T src_unit,
    UI32_T src_port,
    void * cookie)
{
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *org_msg_p = cookie;
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T passed_auth_result = SYS_CALLBACK_MGR_AUTH_BYPASS;
    UI32_T  src_lport;
    UI32_T  sec_port_enabled_by_who;
    UI16_T  vid;

    SWCTRL_UserPortToLogicalPort(src_unit, src_port, &src_lport);
    vid = tag_info & 0xfff;

    if(0 == org_msg_p->flag)
    {
        org_msg_p->flag = 1;
    }

#if (SYS_CPNT_DOT1X == TRUE)
    if ((TRUE == SWCTRL_IsSecurityPort(src_lport, &sec_port_enabled_by_who)
        && (SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC != sec_port_enabled_by_who)))
    {
        passed_auth_result = SYS_CALLBACK_MGR_AUTH_PENDING;
    }
#endif /* SYS_CPNT_DOT1X */

#if (SYS_CPNT_PORT_SECURITY == TRUE) && (SYS_CPNT_NETWORK_ACCESS == FALSE)
    if(TRUE == PSEC_TASK_IntrusionMac_CallBack(src_lport, vid, src_mac, dst_mac, ether_type, cookie))
    {
        return;
    }
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) && (SYS_CPNT_NETWORK_ACCESS == FALSE) */

#if (SYS_CPNT_NETACCESS == TRUE)
    if (TRUE == NETACCESS_MGR_AnnounceNewMac_CallBack(
        src_lport, vid, src_mac,
        dst_mac, ether_type, L2MUX_MGR_RECV_REASON_INTRUDER, cookie))
    {
        return;
    }

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

    SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, passed_auth_result, cookie);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceEapPacket_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Announce EAP packets CallBack function
 * INPUT:  mem_ref,dst_mac,src_mac,tag_info,type,pkt_length,lport_no
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceEapPacket_CallBack(
    L_MM_Mref_Handle_T *mem_ref,
    UI8_T *dst_mac,     UI8_T *src_mac,
    UI16_T vid,         UI16_T type,
    UI32_T pkt_length,  UI32_T lport,
    void *cookie)
{
    UI32_T                      debug_flag;
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_NEWMAC_MSGQ_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T            *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    NETACCESS_EAP_DATA_T        *eap_data;
    UI8_T                       *pdu_p;

    /* can't use macro function because must call L_MM_Mref_Release()
     * NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
     */
    /* if not in master mode,release mem_ref and return
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

#if 0
    /* check if mem_ref is valid
     */
    if (NULL == L_MREF_GetPdu(mem_ref))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }
#endif

    /* check if other input is valid
     */
    if ((NULL == dst_mac) || (NULL == src_mac))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_AnnounceEapPacket_CallBack] src mac:");
        NETACCESS_MGR_LocalPrintMacAddress(src_mac);
    }

    /* check if MAC address is authenticating,no then drop it.
     */
    if (TRUE == NETACCESS_OM_IsStateMachineAuthenticating(lport))
    {
        /* EPR:ES4649-32-01139
         */
        if (FALSE == NETACCESS_OM_IsThisMacAuthenticatingMac(lport, src_mac))
        {
            if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
            {
                printf("\r\n[NETACCESS_MGR_AnnounceEapPacket_CallBack] (!= authenticating mac) drop eap packet src mac:");
                NETACCESS_MGR_LocalPrintMacAddress(src_mac);
            }

            /* src_mac is not authentication mac, just drop it
             */
            goto drop_pkt;
        }
    }
    else
    {
        /* allow EAP packet pass through even if the src_mac is unauthorized mac
         * because some port modes (e.g. NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE) allow a mac
         * failed on NewMAC but succeeded on EAP
         * besides, if a port mode don't allow this case, it just do nothing and supposedly has no side effect.
         */
    }

    /* prepare new mac msg
     */
    eap_data = (NETACCESS_EAP_DATA_T*)L_MM_Malloc(
        sizeof(NETACCESS_EAP_DATA_T),
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCEEAPPACKETCALLBACK));

    /* check if allocate buffer success
     */
    if(NULL == eap_data)
    {
        goto drop_pkt;
    }

    memcpy(eap_data->dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(eap_data->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    eap_data->tag_info = vid;
    eap_data->type = type;

    pdu_p = L_MM_Mref_GetPdu (mem_ref, &eap_data->pkt_length);

    if (NULL == pdu_p)
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[%s] L_MM_Mref_GetPdu failed", __FUNCTION__);

        L_MM_Free(eap_data);
        goto drop_pkt;
    }

    eap_data->pkt_data = (UI8_T*)L_MM_Malloc(
        eap_data->pkt_length,
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCERADIUSAUTHORIZEDRESULT));

    /* check if allocate buffer success
     */
    if (NULL == eap_data->pkt_data)
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[%s] Out of memory", __FUNCTION__);

        L_MM_Free(eap_data);
        goto drop_pkt;
    }

    memcpy(eap_data->pkt_data, pdu_p, eap_data->pkt_length);

    eap_data->lport_no = lport;
    eap_data->cookie = cookie;
    cookie = NULL;

    memset(&na_msg_p->new_mac_msg, 0, sizeof(na_msg_p->new_mac_msg));
    na_msg_p->new_mac_msg.m_eap_data = eap_data;

    NETACCESS_VM_ProcessEventNewMac(&na_msg_p->new_mac_msg);

    if (eap_data->cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, eap_data->cookie);
        eap_data->cookie = NULL;
    }

    NETACCESS_MGR_LocalFreeNewMacMsg(&na_msg_p->new_mac_msg);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);

drop_pkt:

    if (cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, cookie);
        cookie = NULL;
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_AnnounceEapPacket_CallBack() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceRadiusAuthorizedResult
 * ---------------------------------------------------------------------
 * PURPOSE: Announce RADA authorized result to network access task
 * INPUT:  lport, mac, authorized_result, authorized_vlan_list, authorized_qos_list, session_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceRadiusAuthorizedResult(
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    char    *authorized_vlan_list,  char    *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip)
{
    UI32_T debug_flag;
    UI8_T  msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_RADIUS_DATA_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    NETACCESS_RADIUS_DATA_T *radius_data;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[%s] start processing...result is %d", __FUNCTION__, authorized_result);
    }

    /* check if input is valid
     */
    if ((NULL == mac) || (NULL == authorized_vlan_list) || (NULL == authorized_qos_list))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* prepare msg
     */
    radius_data = (NETACCESS_RADIUS_DATA_T*)L_MM_Malloc(
        sizeof(NETACCESS_RADIUS_DATA_T),
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCERADIUSAUTHORIZEDRESULT));
    if (NULL == radius_data)
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
        {
            printf("\r\n[%s] Out of memory", __FUNCTION__);
        }

        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    radius_data->lport = lport;
    memcpy(radius_data->authorized_mac, mac, sizeof(radius_data->authorized_mac));

    strncpy(radius_data->authorized_vlan_list, authorized_vlan_list, SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST);
    radius_data->authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST] = '\0'; /* force to end a string */

    strncpy(radius_data->authorized_qos_list, authorized_qos_list, SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE);
    radius_data->authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE] = '\0'; /* force to end a string */

    radius_data->authorized_result = authorized_result;
    radius_data->session_time = session_time;
    radius_data->server_ip = server_ip;

    memset(&na_msg_p->radius_msg, 0, sizeof(na_msg_p->radius_msg));
    na_msg_p->radius_msg.m_radius_data = radius_data;

    NETACCESS_VM_ProcessRadiusMsg(&na_msg_p->radius_msg);

    if(radius_data)
    {
        L_MM_Free(radius_data);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_AnnounceRadiusAuthorizedResult() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceDot1xAuthorizedResult
 * ---------------------------------------------------------------------
 * PURPOSE: Announce dot1x authorized result to network access task
 * INPUT:  lport, mac, authorized_result, authorized_vlan_list, authorized_qos_list, session_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceDot1xAuthorizedResult(
    UI32_T  lport,                  UI8_T   *mac,
    int     eap_identifier,         UI8_T   authorized_result,
    char    *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip)
{
    UI32_T                  debug_flag;
    UI8_T                   msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_DOT1X_MSGQ_T))];
    SYSFUN_Msg_T            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T        *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    NETACCESS_DOT1X_DATA_T  *dot1x_data;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if (NULL == mac)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* prepare msg
     */
    dot1x_data = (NETACCESS_DOT1X_DATA_T*)L_MM_Malloc(
        sizeof(NETACCESS_DOT1X_DATA_T),
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCEDOT1XAUTHORIZEDRESULT));

    /* check if allocate buffer success
     */
    if (NULL == dot1x_data)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "out of memory");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    dot1x_data->lport = lport;
    memcpy(dot1x_data->authorized_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    dot1x_data->eap_identifier = eap_identifier;

    if (NULL != authorized_vlan_list)
    {
        strncpy(dot1x_data->authorized_vlan_list, authorized_vlan_list, SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST);
        dot1x_data->authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST] = '\0'; /* force to end a string */
    }
    else
    {
        memset(dot1x_data->authorized_vlan_list, 0, SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1);
    }

    if (NULL != authorized_qos_list)
    {
        strncpy(dot1x_data->authorized_qos_list, authorized_qos_list, SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE);
        dot1x_data->authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE] = '\0'; /* force to end a string */
    }
    else
    {
        memset(dot1x_data->authorized_qos_list, 0, SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1);
    }

    dot1x_data->authorized_result = authorized_result;
    dot1x_data->session_time = session_time;
    dot1x_data->server_ip    = server_ip;

    memset(&na_msg_p->dot1x_msg, 0, sizeof(na_msg_p->dot1x_msg));
    na_msg_p->dot1x_msg.m_dot1x_data = dot1x_data;

    NETACCESS_VM_ProcessDot1xMsg(&na_msg_p->dot1x_msg);

    if(dot1x_data)
    {
        L_MM_Free(dot1x_data);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_AnnounceDot1xAuthorizedResult() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_PortLinkUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Port link up callback function
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_PortLinkUp_CallBack(UI32_T unit,UI32_T port)
{
    UI32_T lport, debug_flag;
    NETACCESS_PortMode_T port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport) &&
           SWCTRL_LPORT_TRUNK_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#endif
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_PortLinkUp_CallBack] port(%lu)", lport);
    }

    NETACCESS_VM_ProcessEventPortLinkUp(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}/* End of NETACCESS_MGR_PortLinkUp_CallBack() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_PortLinkDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Port link down callback function
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_PortLinkDown_CallBack(UI32_T unit,UI32_T port)
{
    UI32_T lport, debug_flag;
    NETACCESS_PortMode_T port_mode;
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#endif
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_PortLinkDown_CallBack] port(%lu)", lport);
    }

    NETACCESS_VM_ProcessEventPortLinkDown(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}/* End of NETACCESS_MGR_PortLinkDown_CallBack() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_UPortAdminEnable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin Enable CallBack function
 * INPUT:   unit,port.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_UPortAdminEnable_CallBack(UI32_T unit, UI32_T port)
{
    UI32_T lport, debug_flag;
    NETACCESS_PortMode_T    port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#endif
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_UPortAdminEnable_CallBack] port(%lu)", lport);
    }

    NETACCESS_VM_ProcessEventPortAdminUp(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}/* End of NETACCESS_MGR_UPortAdminEnable_CallBack() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_UPortAdminDisable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin disable CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void NETACCESS_MGR_UPortAdminDisable_CallBack(UI32_T unit, UI32_T port)
{
    UI32_T                  lport, debug_flag;
    NETACCESS_PortMode_T    port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport)) ||
#endif
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_UPortAdminDisable_CallBack] port(%lu)", lport);
    }

    NETACCESS_VM_ProcessEventPortAdminDown(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}/* End of NETACCESS_MGR_UPortAdminDisable_CallBack() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_VlanMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : callback func for vlan member addition
 * INPUT    : vid, lport, status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_VlanMemberAdd_CallBack(UI32_T vid_ifindex, UI32_T lport_ifidx, UI32_T vlan_status)
{
    UI32_T debug_flag;
    UI32_T vid;
    NETACCESS_PortMode_T port_mode;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((VAL_dot1qVlanStatus_permanent != vlan_status) ||
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport_ifidx, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport_ifidx, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_VlanMemberAdd_CallBack] port(%lu) vid(%lu)", lport_ifidx, vid_ifindex);
    }

    /* convert vlan ifindex to vlan id
     */
    if (FALSE == VLAN_OM_ConvertFromIfindex(vid_ifindex, &vid))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    NETACCESS_VM_ProcessEventPortVlanChange(vid, lport_ifidx, vlan_status, NETACCESS_PORT_ADDED);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_VlanMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : callback func for vlan member deletion
 * INPUT    : vid, lport, status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_VlanMemberDelete_CallBack(UI32_T vid_ifindex, UI32_T lport_ifidx, UI32_T vlan_status)
{
    UI32_T debug_flag;
    UI32_T vid;
    NETACCESS_PortMode_T port_mode;
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((VAL_dot1qVlanStatus_permanent != vlan_status) ||
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport_ifidx, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport_ifidx, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[NETACCESS_MGR_VlanMemberDelete_CallBack] port(%lu) vid(%lu)", lport_ifidx, vid_ifindex);
    }

    /* convert vlan ifindex to vlan id
     */
    if (FALSE == VLAN_OM_ConvertFromIfindex(vid_ifindex, &vid))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    NETACCESS_VM_ProcessEventPortVlanChange(vid, lport_ifidx, vlan_status, NETACCESS_PORT_REMOVED);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_VlanMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : callback func for vlan member addition
 * INPUT    : vid, lport, status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------*/
void NETACCESS_MGR_VlanList_CallBack(UI8_T*msg,UI8_T event)
{
    UI32_T                  msgq_id, task_id, debug_flag;
    NETACCESS_PortMode_T    port_mode;
    UI8_T                   msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_VlanListModified_MSGQ_T))];
    SYSFUN_Msg_T            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T        *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    UI32_T lport_ifidx,vlan_status,vid;
    SYS_CALLBACK_MGR_REFINEList_CBData_T* rec_msg;
    NETACCESS_VLANList_MSGQ_T  vlanlist_arg;

     rec_msg = (SYS_CALLBACK_MGR_REFINEList_CBData_T*)msg;
     lport_ifidx = rec_msg->arg.arg2.value[0];
     vlan_status = rec_msg->arg.arg2.value[1];

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((VAL_dot1qVlanStatus_permanent != vlan_status) ||
        (FALSE == NETACCESS_OM_GetSecurePortMode(lport_ifidx, &port_mode)) ||
        (FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport_ifidx, port_mode, NULL)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }


    /* get message queue id and task id
     */
    if ((FALSE == NETACCESS_OM_GetVlanModifiedMsgQId(&msgq_id)) ||
        (FALSE == NETACCESS_OM_GetTaskId(&task_id)))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /* prepare msg
     */
    memset(&na_msg_p->vlan_modified_msg, 0, sizeof(na_msg_p->vlan_modified_msg));
    na_msg_p->vlanlist_modified_msg.event   = event;
    memcpy(vlanlist_arg.vlanlist,rec_msg->list.vlanlist,sizeof(vlanlist_arg.vlanlist));
    vlanlist_arg.lport_ifidx = lport_ifidx;
    vlanlist_arg.vlan_status= vlan_status;
    memcpy(&(na_msg_p->vlanlist_modified_msg.vlanlist_msg),&vlanlist_arg,sizeof(NETACCESS_VLANList_MSGQ_T));


    /* enqueue first
     */
    msg_p->msg_size = sizeof(NETACCESS_VlanListModified_MSGQ_T);
    msg_p->msg_type = 1;

    if (SYSFUN_SendRequestMsg(msgq_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) != SYSFUN_OK)
    {
        /* send event again to push the NETACCESS to clear up the queue
         * in case it lost last event flag
         */
        SYSFUN_SendEvent(task_id, NETACCESS_EVENT_VLAN_MODIFIED);

        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[NETACCESS_MGR_VlanMemberAdd_CallBack] SYSFUN_SendRequestMsg() failed");

        NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /* second, send event to task
     */
    if (SYSFUN_SendEvent(task_id, NETACCESS_EVENT_VLAN_MODIFIED) != SYSFUN_OK)
    {
        if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
            printf("\r\n[NETACCESS_MGR_VlanMemberAdd_CallBack] SYSFUN_SendEvent() failed");

        /* what action should take here ?
         */
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}
#define NETACCESS_MGR_IS_MEMBER(list, member)  (((list[((member) - 1) >> 3]) & (1 << (7 - (((member) - 1) & 7)))) != 0)

BOOL_T NETACCESS_MGR_ProcessVlanListModifiedMsg(NETACCESS_VlanListModified_MSGQ_T *vlan_modified_msg_p)
{
    BOOL_T      ret;
    UI32_T      debug_flag = 0;
    UI16_T      vid;
    UI32_T lport_ifidx,vlan_status;

     lport_ifidx = vlan_modified_msg_p->vlanlist_msg.lport_ifidx;
     vlan_status = vlan_modified_msg_p->vlanlist_msg.vlan_status;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input pointer valid
     */
    if (NULL == vlan_modified_msg_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_LOCK();
    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* transfer to VM to process VLAN member add/delete message
     */

     for(vid= 1; vid<=SYS_ADPT_MAX_NBR_OF_VLAN;vid++)
     {
         if(NETACCESS_MGR_IS_MEMBER(vlan_modified_msg_p->vlanlist_msg.vlanlist,vid))
              ret = NETACCESS_VM_ProcessEventPortVlanChange(vid,
                                                            lport_ifidx,
                                                            vlan_status,
                                                            vlan_modified_msg_p->event);
     }


    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_AnnounceNewMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Announce New MAC CallBack function
 * INPUT    : dst_mac, ...
 * OUTPUT   : None.
 * RETURN   : TRUE  -- intrusion packet, drop packet
 *            FALSE -- not intrusion, go ahead
 * NOTES    :
 *            this api may be called when
 *              1. AMTRDRV_MGR_AnnounceNA_N_SecurityCheck
 *                 upper CSC can decide if the packet is intrusion or not.
 *              2. AMTR_MGR_Notify_IntrusionMac
 *                 many packets is processed by AMTR_MGR,
 *                   then intrusion occurred.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_AnnounceNewMac_CallBack(
    UI32_T   src_lport,     UI16_T   vid,
    UI8_T   *src_mac,       UI8_T   *dst_mac,
    UI16_T   ether_type,    UI32_T   reason,
    void *cookie)
{
    UI32_T                  src_unit, src_port, trunk_id;
    UI32_T                  debug_flag;
    UI8_T                   msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_NEWMAC_MSGQ_T))];
    SYSFUN_Msg_T            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T        *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    NETACCESS_NEWMAC_DATA_T *new_mac_data = NULL;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[%s] start", __FUNCTION__);
    }

    /* check if input is valid
     */
    if (L2MUX_MGR_RECV_REASON_INTRUDER == reason)
    {
        if ((NULL == dst_mac) || (NULL == src_mac) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
            (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(src_lport, &src_unit, &src_port, &trunk_id) &&
             SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(src_lport, &src_unit, &src_port, &trunk_id)) ||
#else
            (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(src_lport, &src_unit, &src_port, &trunk_id)) ||
#endif
            (FALSE == NETACCESS_MGR_LocalIsIntrusionPacket(dst_mac, ether_type)))
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    {
        NETACCESS_PortMode_T secure_port_mode;

        if (TRUE == NETACCESS_OM_GetSecurePortMode(src_lport, &secure_port_mode))
        {
            if (secure_port_mode == NETACCESS_PORTMODE_NO_RESTRICTIONS)
            {
                if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
                {
                    printf("\r\n[%s] not secure port, by pass\r\n", __FUNCTION__);
                }
                NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
    }

    if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
    {
        printf("\r\n[%s] port: %ld,vid: %d,new mac:", __FUNCTION__, src_lport, vid/*tag_info, reason*/);
        NETACCESS_MGR_LocalPrintMacAddress(src_mac);
    }

    if (TRUE == NETACCESS_OM_IsStateMachineAuthenticating(src_lport))
    {
        if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
        {
            printf("\r\n[%s] (authenticating) drop new mac: ", __FUNCTION__);
            NETACCESS_MGR_LocalPrintMacAddress(src_mac);
        }

        /* 3Com-458, session 2.10.2.1.5, page 13
         * While authentication for a MAC address is in progress, an agent MAY ignore
         * further new hosts seen on the same port.
         * besides, EAP negociation will keep going on in NETACCESS_MGR_AnnounceEapPacket_CallBack()
         */
        goto authing_mac;
    }

    /* check if unauthorized now,black-list
     */
    if(TRUE == NETACCESS_OM_IsMacExistInUnauthorizedMacCache(src_mac))
    {
        if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
        {
            printf("\r\n[%s] (unauthenticated) drop new mac: ", __FUNCTION__);
            NETACCESS_MGR_LocalPrintMacAddress(src_mac);
        }

        goto un_auth_mac;
    }

    /* must determin tag/untag here, or
     *   apply all tagged vlan assignment returned from radius to port
     *   will fail in NETACCESS_VM_LocalCheckAutoVlanIssue
     */
    {
        VLAN_OM_Dot1qPortVlanEntry_T    port_vlan_entry;

        /* get PVID
         */
        if (VLAN_MGR_GetDot1qPortVlanEntry(src_lport ,&port_vlan_entry) == FALSE)
        {
            if (NETACCESS_OM_DEBUG_MG_ERR & debug_flag)
                printf("\r\n[%s] VLAN_MGR_GetDot1qPortVlanEntry(%lu) failed", __FUNCTION__, src_lport);

            //L_MM_Free(new_mac_data);
            if (cookie)
            {
                SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_FAILED, cookie);
            }
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        /*EPR:ES3628BT-FLF-ZZ-00951
         *Problem: Port Security: Enable port security on one-port
                   trunk will print exception on CLI.
         *Root cause: the ptr(new_mac_data) did not malloc the space,so cause exception
         *Solution: need malloc the space for use---DanXie
         *Modify file: netaccess_mgr.c,swctrl.c
         */
        new_mac_data = (NETACCESS_NEWMAC_DATA_T*)L_MM_Malloc(
            sizeof(NETACCESS_NEWMAC_DATA_T),
            L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCENEWMMACCALLBACK));

        if(new_mac_data == 0)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        memset(new_mac_data, 0, sizeof(*new_mac_data));

        /* In Marvell & BCM & Agere chip, always are tagged packets
         * and can not determine is_tag_packet by ether_type.
         */
        new_mac_data->is_tag_packet = (port_vlan_entry.dot1q_pvid_index == vid) ?
            FALSE : TRUE;

        if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
        {
            printf("\r\n[%s] %s packet, vid(%u)", __FUNCTION__,
            new_mac_data->is_tag_packet ? "tagged" : "untagged", vid);
        }
    }

    new_mac_data->lport = src_lport;
    memcpy(new_mac_data->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    new_mac_data->reason        = reason;
    new_mac_data->vid           = vid;

    /* The cookie ptr should be move not copy on NETACCESS_VM !!
     */
    new_mac_data->cookie        = cookie;
    cookie = NULL;

    memset(&na_msg_p->new_mac_msg, 0, sizeof(na_msg_p->new_mac_msg));
    na_msg_p->new_mac_msg.m_newmac_data = new_mac_data;

    NETACCESS_VM_ProcessEventNewMac(&na_msg_p->new_mac_msg);

    if (new_mac_data->cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, new_mac_data->cookie);
        new_mac_data->cookie = NULL;
    }

    NETACCESS_MGR_LocalFreeNewMacMsg(&na_msg_p->new_mac_msg);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);

authing_mac:
un_auth_mac:

    if (cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, cookie);
        cookie = NULL;
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_AnnounceNewMac_CallBack() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_DelAcl_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for acl deletion.
 * INPUT    : acl_name          -- which acl be deleted.
 *            dynamic_port_list -- the port list that bind the deleted policy
 *                                 map with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_MGR_DelAcl_CallBack(const char *acl_name, UI32_T acl_type, UI8_T *dynamic_port_list)
{
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    NETACCESS_MGR_LOCK();
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_VM_ProcessEventDelAcl(acl_name, acl_type, dynamic_port_list);
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_DelPolicyMap_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for policy map deletion.
 * INPUT    : acl_name          -- which acl be deleted.
 *            dynamic_port_list -- the port list that bind the deleted policy
 *                                 map with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_MGR_DelPolicyMap_CallBack(const char *policy_map_name, UI8_T *dynamic_port_list)
{
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    NETACCESS_MGR_LOCK();
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_VM_ProcessEventDelPolicyMap(policy_map_name, dynamic_port_list);
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalIsIntrusionPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : do intrusion packet checking
 * INPUT    : da, ether_type
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalIsIntrusionPacket(const UI8_T *da, UI16_T ether_type)
{
    #if 0
    UI8_T   null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
    #endif

    /* check if input is valid
     */
    if (NULL == da)
    {
        return FALSE;
    }

    {
        UI32_T  debug_flag = NETACCESS_OM_GetDebugFlag();

        if (NETACCESS_OM_DEBUG_MG_TRC & debug_flag)
        {
            printf("\r\n[%s] da:%02x%02x%02x-%02x%02x%02x, eth-%04x", __FUNCTION__,
                    da[0], da[1], da[2], da[3], da[4], da[5], ether_type);
        }
    }

    /* For AMTR refinement, DA={0,0,0,0,0,0} && ether_type = 0
     *   is meaning NA intruction (same as PSEC_TASK_IsIntrusionPacket)
     */
    if ((0 == ether_type) && NETACCESS_MGR_IS_NULL_MAC(da))
    {
        return TRUE;
    }

    /* the following packet should not be treat as an intrusion packet
     */

    /* BPDU, doesn's have ether type */
    #define NETACCESS_ADDR_BPDU1     0x01
    #define NETACCESS_ADDR_BPDU2     0x80
    #define NETACCESS_ADDR_BPDU3     0xC2
    #define NETACCESS_ADDR_BPDU4     0x00
    #define NETACCESS_ADDR_BPDU5     0x00
    #define NETACCESS_ADDR_BPDU6     0x00

    /* check if DA is BPDU, yes then not intrusion packet
     */
    if ((NETACCESS_ADDR_BPDU1 == da[0]) &&
        (NETACCESS_ADDR_BPDU2 == da[1]) &&
        (NETACCESS_ADDR_BPDU3 == da[2]) &&
        (NETACCESS_ADDR_BPDU4 == da[3]) &&
        (NETACCESS_ADDR_BPDU5 == da[4]) &&
        (NETACCESS_ADDR_BPDU6 == da[5]))
    {
        return FALSE;
    }

    /* EAP
     * RADA modes replace 802.1x multicast MAC with unicast one
     * so do not check da here
     */
    #define NETACCESS_EAP_FRAME_TYPE    0x888e

    /* check if ether type is 0x888e, yes then not intrusion packet
     */
    if (NETACCESS_EAP_FRAME_TYPE == ether_type)
    {
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalIsVisibleSecureMacForUser
 *-------------------------------------------------------------------------
 * PURPOSE  : whether mac can be seen or not
 * INPUT    : mac_entry
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalIsVisibleSecureMacForUser(const NETACCESS_OM_SecureMacEntry_T *mac_entry)
{
    if (NULL == mac_entry)
        return FALSE;

    /* "hidden MACs" are invisible for users and can't write Learned-PSEC to cip (AMTR).
     * As a result, they don't effect on counter.
     */
    if((NETACCESS_PORTMODE_USER_LOGIN == mac_entry->add_on_what_port_mode) &&
        (1 == mac_entry->mac_flag.is_hidden_mac))
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalPreCheckTrunkAndLacp
 *-------------------------------------------------------------------------
 * PURPOSE  : pre-check if port is not trunk and lacp port
 * INPUT    : lport
 * OUTPUT   : reason
 * RETURN   : TRUE - pass; FALSE - fail
 * NOTES    : reason is meaningful only when return FALSE
 *            allow to pass NULL pointer if don't care reason
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalPreCheckTrunkAndLacp(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason)
{
    LACP_MGR_Dot3adLacpPortEntry_T  lacp_entry;

    /* check trunk port
     */
    if (TRUE == SWCTRL_LogicalPortIsTrunkPort(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is Trunk port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_IS_TRUNK_PORT;
        }

        return FALSE;
    }

    /* check lacp enabled
     */
    lacp_entry.dot3ad_lacp_port_index = lport;
    if ((TRUE == LACP_PMGR_GetDot3adLacpPortEntry(&lacp_entry)) &&
        (VAL_lacpPortStatus_enabled == lacp_entry.dot3ad_lacp_port_status))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is LACP enabled port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_LACP_ENABLED;
        }
        return FALSE;
    }

    if (TRUE == NETACCESS_MGR_LocalIsRSPANEnabledPort(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is RSPSN enabled port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_RSPAN_ENABLED;
        }

        return FALSE;
    }

    if (TRUE == NETACCESS_MGR_LocalIsPSecEnabledPort(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is PSec enabled port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_PSEC_ENABLED;
        }

        return FALSE;
    }

    return TRUE;
}


#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
static BOOL_T NETACCESS_MGR_LocalPreCheckLacp(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason)
{
    LACP_MGR_Dot3adLacpPortEntry_T  lacp_entry;

    /* check lacp enabled
     */
    lacp_entry.dot3ad_lacp_port_index = lport;
    if ((TRUE == LACP_PMGR_GetDot3adLacpPortEntry(&lacp_entry)) &&
        (VAL_lacpPortStatus_enabled == lacp_entry.dot3ad_lacp_port_status))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is LACP enabled port",
                       lport);

        if (NULL != reason)
            *reason = NETACCESS_COZ_LACP_ENABLED;
        return FALSE;
    }

    if (TRUE == NETACCESS_MGR_LocalIsRSPANEnabledPort(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is RSAPN enabled port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_RSPAN_ENABLED;
        }

        return FALSE;
    }

    if (TRUE == NETACCESS_MGR_LocalIsPSecEnabledPort(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
                       "Port(%ld) is PSec enabled port",
                       lport);

        if (NULL != reason)
        {
            *reason = NETACCESS_COZ_PSEC_ENABLED;
        }

        return FALSE;
    }

    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalPreCheckForDot1X
 *-------------------------------------------------------------------------
 * PURPOSE  : check for enabling dot1x on the specified port
 * INPUT    : lport
 * OUTPUT   : reason
 * RETURN   : TRUE - pass; FALSE - fail
 * NOTES    : reason is meaningful only when return FALSE
 *            allow to pass NULL pointer if don't care reason
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalPreCheckForDot1X(UI32_T lport, NETACCESS_MGR_FunctionFailure_T *reason)
{
    if (FALSE == NETACCESS_MGR_LocalPreCheckTrunkAndLacp(lport, reason))
        return FALSE;

    return TRUE;
}

static BOOL_T NETACCESS_MGR_LocalIsRSPANEnabledPort(UI32_T lport)
{
#if (SYS_CPNT_RSPAN == TRUE)
    return RSPAN_OM_IsRspanUplinkPort(lport);
#else
    return FALSE;
#endif
}

static BOOL_T NETACCESS_MGR_LocalIsPSecEnabledPort(UI32_T lport)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)
    UI32_T status;
    UI32_T mac_count;

    if (PSEC_OM_GetPortSecurityStatus(lport, &status))
    {
        return (status == VAL_portSecPortStatus_enabled)
                ? TRUE
                : FALSE
                ;
    }
#endif

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalDoPortPreChecking
 *-------------------------------------------------------------------------
 * PURPOSE  : do port pre-checking
 * INPUT    : lport, port_mode
 * OUTPUT   : reason
 * RETURN   : TRUE - pass; FALSE - fail
 * NOTES    : reason is meaningful only when return FALSE
 *            allow to pass NULL pointer if don't care reason
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalDoPortPreChecking(UI32_T lport, NETACCESS_PortMode_T port_mode, NETACCESS_MGR_FunctionFailure_T *reason)
{
    if(NULL != netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check)
    {
        return netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check(lport, reason);
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalCopySecurePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to dst
 * INPUT    : src
 * OUTPUT   : dst
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalCopySecurePortEntry(NETACCESS_MGR_SecurePortEntry_T *dst, const NETACCESS_OM_SecurePortEntry_T *src)
{
    BOOL_T enabled;

    if ((NULL == dst) || (NULL == src))
        return FALSE;

    dst->lport = src->lport;
    dst->port_mode = src->port_mode;
    dst->intrusion_action = src->intrusion_action;
    dst->number_addresses = src->number_addresses;
    dst->number_addresses_stored = src->number_addresses_stored;
    dst->nbr_of_authorized_addresses = src->nbr_of_authorized_addresses;
    dst->maximum_addresses = src->maximum_addresses;
    dst->nbr_of_learn_authorized_addresses = src->nbr_of_learn_authorized_addresses;
    dst->filter_id= src->filter_id;

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    NETACCESS_OM_GetLinkDetectionStatus(src->lport, &dst->link_detection_status);
    NETACCESS_OM_GetLinkDetectionMode(src->lport, &dst->link_detection_mode);
    NETACCESS_OM_GetLinkDetectionAction(src->lport, &dst->link_detection_action);
#endif

    NETACCESS_OM_GetSecureGuestVlanId(src->lport, &dst->guest_vlan_id);

    NETACCESS_OM_GetDynamicVlanStatus(src->lport, &enabled);
    dst->dynamic_vlan_status = (enabled)
                                ? VAL_networkAccessPortDynamicVlan_enabled
                                : VAL_networkAccessPortDynamicVlan_disabled
                                ;

    NETACCESS_OM_GetDynamicQosStatus(src->lport, &enabled);
    dst->dynamic_qos_status = (enabled)
                                ? VAL_networkAccessPortLinkDynamicQos_enabled
                                : VAL_networkAccessPortLinkDynamicQos_disabled
                                ;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalCopySecureAddressEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to dst
 * INPUT    : src
 * OUTPUT   : dst
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalCopySecureAddressEntry(NETACCESS_MGR_SecureAddressEntry_T *dst, const NETACCESS_OM_SecureMacEntry_T *src)
{
    /* check if input is valid
     */
    if ((NULL == dst) || (NULL == src))
    {
        return FALSE;
    }

    dst->addr_lport = src->lport;
    memcpy(dst->addr_MAC, src->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* 3com use "notInService" but Accton use "notReady" in row status
     * so can't be dst->addr_row_status = src->addr_row_status;
     */
    dst->addr_row_status = src->authorized_status;
    dst->server_ip = src->server_ip;
    dst->record_time = src->record_time;
    dst->is_learnt = (src->mac_flag.is_mac_filter_mac == 1) ? FALSE : TRUE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalCopyMacAuthenticationPortEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to dst
 * INPUT    : src
 * OUTPUT   : dst
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalCopyMacAuthenticationPortEntry(NETACCESS_MGR_MacAuthPortEntry_T *dst, const NETACCESS_OM_MacAuthPortEntry_T *src)
{
    if ((NULL == dst) || (NULL == src))
        return FALSE;

    dst->lport = src->lport;
    dst->configured_number_addresses = src->configured_number_addresses;
    dst->intrusion_action = src->intrusion_action;
    return TRUE;
}

#if (SYS_CPNT_DOT1X == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalStrPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: covert port_mode to string
 * INPUT:  port_mode.
 * OUTPUT: the string of the port_mode.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static char* NETACCESS_MGR_LocalStrPortMode(UI32_T port_mode)
{
    /*enum{MAX_LENGTH_STR_PORT_MODE = 32};*/

    static char  *str_port_mode[] =  {"unknow",
                                      "noRestrictions", /*index=1*/
                                      "continuosLearning",
                                      "autoLearn",
                                      "secure",
                                      "userLogin",
                                      "userLoginSecure",
                                      "userLoginWithOUI",
                                      "macAddressWithRadius",
                                      "macAddressOrUserLoginSecure",
                                      "macAddressElseUserLoginSecure",
                                      "macAuthentication",
                                      "portSecurity",
                                      "dot1x"};
    if(port_mode >= NETACCESS_PORTMODE_MAX)
        port_mode = 0;

    return str_port_mode[port_mode];
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalPrintPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: printf port_mode as string
 * INPUT:  port_mode.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_MGR_LocalPrintPortMode(UI32_T port_mode)
{
    printf("%s",
        (NETACCESS_PORTMODE_NO_RESTRICTIONS == port_mode) ? "noRestrictions" :
        (NETACCESS_PORTMODE_CONTINUOS_LEARNING == port_mode) ? "continuousLearning" :
        (NETACCESS_PORTMODE_AUTO_LEARN == port_mode) ? "autoLearn" :
        (NETACCESS_PORTMODE_SECURE == port_mode) ? "secure" :
        (NETACCESS_PORTMODE_USER_LOGIN == port_mode) ? "userLogin" :
        (NETACCESS_PORTMODE_USER_LOGIN_SECURE == port_mode) ? "userLoginSecure" :
        (NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI == port_mode) ? "userLoginWithOUI" :
        (NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS == port_mode) ? "macAddressWithRadius" :
        (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == port_mode) ? "macAddressOrUserLoginSecure" :
        (NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE == port_mode) ? "macAddressElseUserLoginSecure" :
        (NETACCESS_PORTMODE_MAC_AUTHENTICATION == port_mode) ? "macAuthentication" :
        (NETACCESS_PORTMODE_PORT_SECURITY == port_mode) ? "portSecurity" :
        (NETACCESS_PORTMODE_DOT1X== port_mode) ? "dot1x" :
        "unknown");
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalPrintStateMachineState
 * ---------------------------------------------------------------------
 * PURPOSE: printf state as string
 * INPUT:  state.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_MGR_LocalPrintStateMachineState(UI32_T sm_state)
{
    printf("%s",
        (NETACCESS_STATE_SYSTEM_INIT == sm_state) ? "SystemInit" :

        (NETACCESS_STATE_ENTER_SECURE_PORT_MODE == sm_state) ? "EnterSecurePortMode" :
        (NETACCESS_STATE_SECURE_PORT_MODE == sm_state) ? "SecurePortMode" :
        (NETACCESS_STATE_EXIT_SECURE_PORT_MODE == sm_state) ? "ExitSecurePortMode" :

        (NETACCESS_STATE_INIT == sm_state) ? "init" :
        (NETACCESS_STATE_IDLE == sm_state) ? "idle" :
        (NETACCESS_STATE_LEARNING == sm_state) ? "learning" :
        (NETACCESS_STATE_INTRUSION_HANDLING == sm_state) ? "intrusionHandling" :
        (NETACCESS_STATE_AUTHENTICATING == sm_state) ? "authenticating" :
        (NETACCESS_STATE_SUCCEEDED == sm_state) ? "succeeded" :
        (NETACCESS_STATE_FAILED == sm_state) ? "failed" :
        (NETACCESS_STATE_DOT1X_AUTHENTICATING == sm_state) ? "dot1xAuthenticating" :
        (NETACCESS_STATE_RADA_AUTHENTICATING == sm_state) ? "radaAuthenticating" :
        (NETACCESS_STATE_DOT1X_FAILED == sm_state) ? "dot1xFailed" :
        (NETACCESS_STATE_RADA_FAILED == sm_state) ? "radaFailed" :
        "unknown state");
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalPrintMacAddress
 * ---------------------------------------------------------------------
 * PURPOSE: printf mac address as string
 * INPUT:  addr.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_MGR_LocalPrintMacAddress(const UI8_T *addr)
{
    if (NULL == addr)
        return;

    printf("%02x-%02x-%02x-%02x-%02x-%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_LocalPrintStateMachineState
 * ---------------------------------------------------------------------
 * PURPOSE: printf event bitmap as string
 * INPUT:  bm.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_MGR_LocalPrintEventBitmap(const NETACCESS_OM_StateMachineEvent_T *bm)
{
    if (NULL == bm)
        return;

    printf("\r\n\t\t(%3s) new_mac", (1 == bm->new_mac) ? "on" : "off");
    printf("\r\n\t\t(%3s) eap_packet", (1 == bm->eap_packet) ? "on" : "off");
    printf("\r\n\t\t(%3s) reauth", (1 == bm->reauth) ? "on" : "off");
    printf("\r\n\t\t(%3s) is_authenticating", (1 == bm->is_authenticating) ? "on" : "off");
    printf("\r\n\t\t(%3s) dot1x_logon", (1 == bm->dot1x_logon) ? "on" : "off");

    printf("\r\n\t\t(%3s) dot1x_success", (1 == bm->dot1x_success) ? "on" : "off");
    printf("\r\n\t\t(%3s) dot1x_fail", (1 == bm->dot1x_fail) ? "on" : "off");
    printf("\r\n\t\t(%3s) dot1x_logoff", (1 == bm->dot1x_logoff) ? "on" : "off");
    printf("\r\n\t\t(%3s) rada_success", (1 == bm->rada_success) ? "on" : "off");
    printf("\r\n\t\t(%3s) rada_fail", (1 == bm->rada_fail) ? "on" : "off");
    printf("\r\n\t\t(%3s) waiting_reauth_result", (1 == bm->waiting_reauth_result) ? "on" : "off");
    printf("\r\n\t\t(%3s) is_tagged", (1 == bm->is_tagged) ? "on" : "off");
    printf("\r\n\t\t(%3s) return_vlan_change", (1 == bm->return_vlan_change) ? "on" : "off");
}

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

static void NETACCESS_MGR_LocalInitializeFunctionPointer(void)
{
    UI32_T port_mode;

    memset(netaccess_mgr_port_mode_fun_ar, 0 ,sizeof(NETACCESS_MGR_ProcessPortModeFuncPtr_T)*(NETACCESS_PORTMODE_MAX-1));

    /* noRestrisions
     */
    port_mode = NETACCESS_PORTMODE_NO_RESTRICTIONS;
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = NULL;

    /* macAuthentication
     */
    port_mode = NETACCESS_PORTMODE_MAC_AUTHENTICATION;
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = (NETACCESS_MGR_PreCheckFuncPtr_T)NETACCESS_MGR_LocalPreCheckTrunkAndLacp;

    /* portSecurity
     */
    port_mode = NETACCESS_PORTMODE_PORT_SECURITY;
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = (NETACCESS_MGR_PreCheckFuncPtr_T)NETACCESS_MGR_LocalPreCheckLacp;
#else
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = (NETACCESS_MGR_PreCheckFuncPtr_T)NETACCESS_MGR_LocalPreCheckTrunkAndLacp;
#endif

    /* dot1x
     */
    port_mode = NETACCESS_PORTMODE_DOT1X;
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = (NETACCESS_MGR_PreCheckFuncPtr_T)NETACCESS_MGR_LocalPreCheckForDot1X;

    port_mode = NETACCESS_PORTMODE_SECURE;
    netaccess_mgr_port_mode_fun_ar[port_mode-1].pre_check = (NETACCESS_MGR_PreCheckFuncPtr_T)NETACCESS_MGR_LocalPreCheckForDot1X;

    return;
}

static BOOL_T NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortAddress(NETACCESS_MGR_SecureAddressFilter_T *in_filter, NETACCESS_MGR_SecureAddressEntry_T *entry)
{
    BOOL_T is_found = FALSE;
    #if 0
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0,0,0,0,0,0};
    #endif
    NETACCESS_OM_SecureMacEntry_T  mac_table;

    memset(&mac_table, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_table.lport = entry->addr_lport;
    memcpy(mac_table.secure_mac, entry->addr_MAC, SYS_ADPT_MAC_ADDR_LEN);

    /* get next mac entry to check if need to delete
     */
    while(NETACCESS_OM_GetNextSecureAddressEntryByMacKey(&mac_table))
    {
        /* check if static mac address entry
         */
        if((NETACCESS_ADDRESS_ENTRY_TYPE_STATIC == in_filter->type) &&
            ((1 != mac_table.mac_flag.is_mac_filter_mac) &&
             (1 != mac_table.mac_flag.admin_configured_mac)))
        {
            continue;
        }

        /* check if dynamic mac address entry
         */
        if((NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC== in_filter->type) &&
            ((1 != mac_table.mac_flag.auth_by_rada) &&
             (1 != mac_table.mac_flag.authorized_by_dot1x)))
        {
            continue;
        }

        /* check if correct lport
         */
        if((0 != in_filter->lport) &&
           (mac_table.lport != in_filter->lport))
        {
            continue;
        }

        /* check if delete by correct mac address
         */
        if(!(NETACCESS_MGR_IS_NULL_MAC(in_filter->mac)) &&
            (0 != memcmp(in_filter->mac, mac_table.secure_mac, SYS_ADPT_MAC_ADDR_LEN)))
        {
            continue;
        }

        /* found
         */
        is_found = TRUE;
        break;
    }

    /* if found,copy entry information
     */
    if(TRUE == is_found)
    {
        NETACCESS_MGR_LocalCopySecureAddressEntry(entry, &mac_table);
        return TRUE;
    }

    return FALSE;
}

static BOOL_T NETACCESS_MGR_LocalGetNextSecureAddressEntryByFilterSortPort(NETACCESS_MGR_SecureAddressFilter_T *in_filter, NETACCESS_MGR_SecureAddressEntry_T *entry)
{
    BOOL_T is_found = FALSE;
    #if 0
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0,0,0,0,0,0};
    #endif
    NETACCESS_OM_SecureMacEntry_T  mac_table;

    memset(&mac_table, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_table.lport = entry->addr_lport;
    memcpy(mac_table.secure_mac, entry->addr_MAC, SYS_ADPT_MAC_ADDR_LEN);

    /* get next mac entry to check if need to delete
     */
    while(NETACCESS_OM_GetNextSecureAddressEntry(&mac_table))
    {
        if((NETACCESS_ADDRESS_ENTRY_TYPE_STATIC == in_filter->type) &&
            ((1 != mac_table.mac_flag.is_mac_filter_mac) &&
             (1 != mac_table.mac_flag.admin_configured_mac)))
        {
            continue;
        }

        /* check if dynamic mac address entry
         */
        if((NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC== in_filter->type) &&
            ((1 != mac_table.mac_flag.auth_by_rada) &&
             (1 != mac_table.mac_flag.authorized_by_dot1x)))
        {
            continue;
        }

        /* check if correct lport
         */
        if((0 != in_filter->lport) &&
            (mac_table.lport != in_filter->lport))
        {
            continue;
        }

        /* check if delete by correct mac address
         */
        if(!(NETACCESS_MGR_IS_NULL_MAC(in_filter->mac)) &&
            (0 != memcmp(in_filter->mac, mac_table.secure_mac, SYS_ADPT_MAC_ADDR_LEN)))
        {
            continue;
        }

        /* found
         */
        is_found = TRUE;
        break;
    }

    /* if found,copy entry information
     */
    if(TRUE == is_found)
    {
        NETACCESS_MGR_LocalCopySecureAddressEntry(entry, &mac_table);
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalCheckPortModeChangeIssue
 *-------------------------------------------------------------------------
 * PURPOSE  : check port mode change issue
 * INPUT    : lport, new_port_mode
 * OUTPUT   : none
 * RETURN   : TRUE - can change; FALSE - don't change
 * NOTE     :
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalCheckPortModeChangeIssue(UI32_T lport,NETACCESS_PortMode_T new_port_mode)
{
    NETACCESS_PortMode_T original_port_mode;

    if(FALSE == NETACCESS_OM_GetSecurePortMode(lport, &original_port_mode))
    {
        return FALSE;
    }

    if(new_port_mode == original_port_mode)
    {
        return TRUE;
    }

    switch (new_port_mode)
    {
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
            return TRUE;
            break;

        /* active port mode */
        case NETACCESS_PORTMODE_PORT_SECURITY:
            if(NETACCESS_PORTMODE_NO_RESTRICTIONS == original_port_mode)
            {
                return TRUE;
            }
            break;

        case NETACCESS_PORTMODE_SECURE:
            if(    (NETACCESS_PORTMODE_NO_RESTRICTIONS == original_port_mode)
                || (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == original_port_mode)
                || (NETACCESS_PORTMODE_MAC_AUTHENTICATION == original_port_mode)
                || (NETACCESS_PORTMODE_DOT1X == original_port_mode) /*dot1x auto to force-unauthorized*/
              )
            {
                return TRUE;
            }
            break;

        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
            if(    (NETACCESS_PORTMODE_NO_RESTRICTIONS == original_port_mode)
                || (NETACCESS_PORTMODE_SECURE == original_port_mode)
                || (NETACCESS_PORTMODE_MAC_AUTHENTICATION == original_port_mode)
                || (NETACCESS_PORTMODE_DOT1X == original_port_mode) /*dot1x auto to force-unauthorized*/
              )
            {
                return TRUE;
            }
            break;

        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
            if(    (NETACCESS_PORTMODE_NO_RESTRICTIONS == original_port_mode)
                || (NETACCESS_PORTMODE_SECURE == original_port_mode)
                || (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == original_port_mode)
              )
            {
                return TRUE;
            }
            break;

        case NETACCESS_PORTMODE_DOT1X:
            if(    (NETACCESS_PORTMODE_NO_RESTRICTIONS == original_port_mode)
                || (NETACCESS_PORTMODE_SECURE == original_port_mode) /*dot1x force-unauthorized to auto*/
                || (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == original_port_mode)
              )
            {
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }

    return FALSE;
}

#if (SYS_CPNT_DOT1X == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 *            control_mode -- control mode
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortControlMode(UI32_T lport,UI32_T control_mode)
{
    DOT1X_SM_AUTH_Event_T   ev;
    UI32_T                  unit, port, trunk_id;
    NETACCESS_PortMode_T    current_port_mode, new_port_mode;
    BOOL_T                  ret;

    NETACCESS_DBG2(NETACCESS_OM_DEBUG_MG_IFO, "start [lport-%ld/mode-%ld]", lport, control_mode);

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input is valid
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* change control_mode from leaf to OM value
     */
    if (VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized == control_mode)
    {
        ev = DOT1X_SM_AUTH_FORCE_UNAUTH_EV;
    }
    else if (VAL_dot1xAuthAuthControlledPortControl_auto == control_mode)
    {
        ev = DOT1X_SM_AUTH_AUTO_EV;
    }
    else if (VAL_dot1xAuthAuthControlledPortControl_forceAuthorized == control_mode)
    {
        ev = DOT1X_SM_AUTH_FORCE_AUTH_EV;
    }
    else
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Unknow control mode(%lu)", control_mode);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* nothing change,just return
     */
    if (DOT1X_OM_Get_PortControlMode(lport) == control_mode)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    if (VAL_dot1xPaeSystemAuthControl_enabled == DOT1X_OM_Get_SystemAuthControl())
    {
        /* get the new secure port mode by dot1x control mode and mac-auth enabled status
         */
        if (FALSE == NETACCESS_MGR_GetSecurePortMode(lport, &current_port_mode))
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "cannot get the secure port(%ld) mode", lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        new_port_mode = NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                          DOT1X_OM_Get_SystemAuthControl(), control_mode, NETACCESS_VM_IsMacAuthEnabled(lport) );

        ret = NETACCESS_MGR_SetSecurePortMode(lport, new_port_mode);
        if (!ret)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        DOT1X_VM_SendEvent(lport, ev);
    }

    ret = DOT1X_OM_Set_PortControlMode(lport, control_mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : control_mode -- control mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortControlMode(UI32_T lport,UI32_T *control_mode)
{
    UI32_T  unit, port, trunk_id;

    if(control_mode == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "control_mode=NULL on port(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        if (SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        {
            *control_mode = VAL_dot1xAuthAuthControlledPortControl_forceAuthorized;
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *control_mode = DOT1X_OM_Get_PortControlMode(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port control mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : control_mode -- control mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortControlMode(UI32_T lport,UI32_T *control_mode)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T result;

    if(control_mode == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "control_mode=NULL on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    result = DOT1X_OM_GetRunning_PortControlMode(lport, control_mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(result);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next port control mode of 1X configuration
 * INPUT    : lport -- logic port number, use 0 to get first
 * OUTPUT   : control_mode -- control mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortControlMode(UI32_T *lport,UI32_T *control_mode)
{
    SWCTRL_Lport_Type_T type;

    if((lport == NULL) || (control_mode == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL&&control_mode==NULL)?"lport and control_mode":
                                                  (lport==NULL)?"lport": "control_mode");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port number
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    *control_mode = DOT1X_OM_Get_PortControlMode(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 *            mode -- operation mode
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortOperationMode(UI32_T lport,UI32_T mode)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  original_operation_mode;
    UI32_T  mac_count = 1;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get original dot1x port operation mode
     */
    if(FALSE == DOT1X_OM_Get_PortOperationMode(lport, &original_operation_mode))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Get the original operation mode fail on port(%lu)", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* nothing change,just return
     */
    if(original_operation_mode == mode)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check the input parameters
     */
    if (    (DOT1X_PORT_OPERATION_MODE_ONEPASS   != mode)
         && (DOT1X_PORT_OPERATION_MODE_MULTIPASS != mode)
#if (SYS_CPNT_DOT1X_MACBASED_AUTH == TRUE)
         && (DOT1X_PORT_OPERATION_MODE_MACBASED  != mode)
#endif
       )
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Unknow dot1x operation mode(%lu)", mode);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* if dot1x feature is disabled, just set to database and no need to change port mode
     */
    if (DOT1X_OM_Get_SystemAuthControl() != VAL_dot1xPaeSystemAuthControl_enabled)
    {
        NETACCESS_MGR_LOCK();
        ret = DOT1X_OM_Set_PortOperationMode(lport, mode);
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    /* lock
     */
    NETACCESS_MGR_LOCK();

    if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (DOT1X_PORT_OPERATION_MODE_MULTIPASS == mode)
    {
        if (FALSE == DOT1X_OM_Get_PortMultiHostMacCount(lport, &mac_count))
        {
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    /*
     * IMPORTANT: Make sure all state machine objects have be destoryed when
     *            changing the operation mode.
     */
    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_DISABLE_EV);

    if (FALSE == DOT1X_OM_Set_PortOperationMode(lport, mode))
    {
        NETACCESS_MGR_UNLOCK();
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_INIT_EV);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : mode -- operation mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortOperationMode(UI32_T lport,UI32_T *mode)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T ret;

    if(mode == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "mode=NULL on port(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_Get_PortOperationMode(lport, mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port operation mode of 1X configuration
 * INPUT    : lport -- logic port number
 * OUTPUT   : mode -- operation mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortOperationMode(UI32_T lport,UI32_T *mode)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(mode == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "mode=NULL on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = DOT1X_OM_GetRunning_PortOperationMode(lport,mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next port operation mode of 1X configuration
 * INPUT    : lport -- logic port number, use 0 to get first
 * OUTPUT   : mode -- operation mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *            DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortOperationMode(UI32_T *lport,UI32_T *mode)
{

    SWCTRL_Lport_Type_T type;
    BOOL_T result;

    if((lport == NULL) || (mode == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL&&mode==NULL)?"lport and mode":
                                                  (lport==NULL)?"lport": "mode");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    result = DOT1X_OM_Get_PortOperationMode(*lport,mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(result);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : set the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 *            count -- max mac count for multi-host mode
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T count)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T ret;
    UI32_T operation_mode, orig_mac_count;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == DOT1X_OM_Get_PortMultiHostMacCount(lport, &orig_mac_count))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Cannot get dot1x max mac count on port(%ld)", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* exit if no change
     */
    if (count == orig_mac_count)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check input parameter
     */
    if (count > SYS_ADPT_MAX_NBR_OF_AUTO_LEARN_MAC || count < 1)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "out of range of the max-mac-count(%lu)", count);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    DOT1X_OM_Get_PortOperationMode(lport, &operation_mode);

    /* lock
     */
    NETACCESS_MGR_LOCK();

    if (    (VAL_dot1xPaeSystemAuthControl_enabled == DOT1X_OM_Get_SystemAuthControl())
         && (VAL_dot1xAuthAuthControlledPortControl_auto == DOT1X_OM_Get_PortControlMode(lport))
         && (DOT1X_PORT_OPERATION_MODE_MULTIPASS == operation_mode)
       )
    {
        /* if new value is less than current, delete all mac entry on this port
         */
        if (count < orig_mac_count)
        {
            /* when new value is less than current,delete all mac entry on this port
             */
            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
            {
                NETACCESS_MGR_UNLOCK();
                NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
    }

    ret = DOT1X_OM_Set_PortMultiHostMacCount(lport, count);

    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortMultiHostMacCount(UI32_T lport,UI32_T *count)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T ret;

    if(count == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "count=NULL on port(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_Get_PortMultiHostMacCount(lport,count);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortMultiHostMacCount(UI32_T lport,UI32_T *count)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(count == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "count=NULL on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = DOT1X_OM_GetRunning_PortMultiHostMacCount(lport,count);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next max allowed MAC number in multi-host mode
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count for multi-host mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortMultiHostMacCount(UI32_T *lport,UI32_T *count)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    if((lport == NULL) || (count == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL&&count==NULL)?"lport and count":
                                                  (lport==NULL)?"lport": "count");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    ret = DOT1X_OM_Get_PortMultiHostMacCount(*lport, count);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xConfigSettingToDefault
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set default value of 1X configuration
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : [to replace DOT1X_MGR_SetConfigSettingToDefault]
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xConfigSettingToDefault(void)
{
    UI32_T                  lport;
    NETACCESS_PortMode_T    current_port_mode;
    BOOL_T                  ret = TRUE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (FALSE == NETACCESS_MGR_GetSecurePortMode(lport, &current_port_mode))
        {
            continue;
        }

        /* continue even one port failed
         */
        if (FALSE == NETACCESS_MGR_SetDot1xPortControlMode(lport, SYS_DFLT_DOT1X_AUTH_CONTROLLED_PORT_CONTROL))
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "reset to dflt port-control-mode failed (%ld)", lport);
            ret = FALSE;
        }
    }/*for(lport=1;*/

    if (TRUE == ret)
    {
        ret = NETACCESS_MGR_SetDot1xSystemAuthControl(SYS_DFLT_DOT1X_PAE_SYSTEM_AUTH_CONTROL);

        if (FALSE == ret)
        {
            NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "reset to dflt sys auth control failed");
        }
        else
        {
            DOT1X_OM_SetConfigSettingToDefault();
        }
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE  : set system auth control of 1X configuration
 * INPUT    : control_status
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_status (define in leaf_Ieee8021x.h):
 *         VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *         VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xSystemAuthControl(UI32_T control_status)
{
    UI32_T                          lport, unit, port, trunk_id;
    UI32_T                          dot1x_control_mode;
    NETACCESS_PortMode_T            current_port_mode, new_port_mode;
    NETACCESS_PortMode_T            original_port_mode;
    NETACCESS_MGR_FunctionFailure_T reason;
    BOOL_T  ret = TRUE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if change
     */
    if(DOT1X_OM_Get_SystemAuthControl() == control_status)
    {
        /* current status = new status, do nothing
         */
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    if (    (VAL_dot1xPaeSystemAuthControl_enabled != control_status)
        &&  (VAL_dot1xPaeSystemAuthControl_disabled != control_status)
       )
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Unknow dot1x system-auth control(%ld)", control_status);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }


    /* check if all port can be dot1x enable
     */
    for(lport=1;lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        /* check if port is reasonable
         */
        if (    (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
             || (FALSE == NETACCESS_MGR_GetSecurePortMode(lport, &current_port_mode))
           )
        {
            continue;
        }

        /* get the new secure port mode by dot1x control mode and mac-auth enabled status
         */
        dot1x_control_mode = DOT1X_OM_Get_PortControlMode(lport);
        new_port_mode = NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                          control_status, dot1x_control_mode, NETACCESS_VM_IsMacAuthEnabled(lport) );

        /* will become enable port
         * check if allow port mode change
         */
        if(FALSE == NETACCESS_MGR_LocalCheckPortModeChangeIssue(lport, new_port_mode))
        {
            NETACCESS_DBG3(NETACCESS_OM_DEBUG_MG_ERR, "Port(%ld) mode cannot change from (%s) to (%s) mode by port mode change issue",
                lport, NETACCESS_MGR_LocalStrPortMode(current_port_mode), NETACCESS_MGR_LocalStrPortMode(new_port_mode));
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        /* check if allow be dot1x port
         */
        if(FALSE == NETACCESS_MGR_LocalDoPortPreChecking(lport, new_port_mode, &reason))
        {
            NETACCESS_DBG3(NETACCESS_OM_DEBUG_MG_ERR, "Port(%ld) mode cannot change from (%s) to (%s) mode by port mode change prechecking",
                lport, NETACCESS_MGR_LocalStrPortMode(current_port_mode), NETACCESS_MGR_LocalStrPortMode(new_port_mode));
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

    }/*for(lport=1;*/

    if (FALSE == NETACCESS_VM_SetDriverForRecievingEapPacket(control_status, DOT1X_OM_GetEapolPassThrough()))
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "NETACCESS_VM_SetDriverForRecievingEapPacket() failed");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* set to database
     */
    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_SystemAuthControl(control_status);
    NETACCESS_MGR_UNLOCK();

    if(TRUE == ret)
    {
        for(lport=1;lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
        {
            /* check if port is reasonable
             */
            if (    (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
                 || (FALSE == NETACCESS_MGR_GetSecurePortMode(lport, &current_port_mode))
               )
            {
                continue;
            }

            /* need to improve...
             *   if port mode is not needed to change, should not change the port mode.
             */
            if (NETACCESS_PORTMODE_DOT1X_SUPPLICANT != current_port_mode)
            {
                if (VAL_dot1xPaeSystemAuthControl_enabled == control_status)
                {
                    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_INIT_EV);
                }
                else
                {
                    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_DISABLE_EV);
                }

                /* get the new secure port mode by dot1x control mode and mac-auth enabled status
                 */
                dot1x_control_mode = DOT1X_OM_Get_PortControlMode(lport);
                /*
                if dot1x_control_mode is force-auth(as default value) ,new_port_mode will be 1(no_restrictions)
                It will clear original secure value like port-sec of port and set dot1x on per port will be accept.
                */

                new_port_mode = NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                                  control_status, dot1x_control_mode, NETACCESS_VM_IsMacAuthEnabled(lport) );
                /*
                EPR:       ES3628BT-FLF-ZZ-00267
                Problem:   Port security: 802.1x and port security should not be enabled on one port at the same time.
                rootcasue: Command 'dot1x system-auth-control' will clear the port secure mode. When config port security first then config
                           dot1x system-auth-control and dot1x per port will be accept.
                sloution:  do not change the port mode which have been configured as 'port security' for Command 'dot1x system-auth-control'
                File:      netaccess_mgr.c
                */
                if(FALSE == NETACCESS_OM_GetSecurePortMode(lport, &original_port_mode))
                {
                    NETACCESS_MGR_RETURN_AND_RELEASE_CSC (FALSE);
                }

                if (original_port_mode!=NETACCESS_PORTMODE_PORT_SECURITY)
                ret &= NETACCESS_MGR_SetSecurePortMode(lport, new_port_mode);
            }
        }/*for(lport=1;*/
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
 *          Control in a system.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : SystemAuthControl
 * NOTES  : VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xSystemAuthControl(UI32_T *control_status)
{
    if(control_status == NULL)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "control_status=NULL");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    *control_status = DOT1X_OM_Get_SystemAuthControl();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemAuthControl is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : None.
 * OUTPUT : SystemAuthControl
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xSystemAuthControl(UI32_T *control_status)
{
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(control_status == NULL)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "control_status=NULL");
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = DOT1X_OM_GetRunning_SystemAuthControl(control_status);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port MaxReq of 1X configuration
 * INPUT    : lport,times
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortMaxReq(UI32_T lport,UI32_T times)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();

    ret = DOT1X_OM_Set_PortMaxReq(lport,times);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortMaxReq(UI32_T lport,UI32_T *times)
{
    UI32_T unit, port, trunk_id;
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if(times == NULL)
        return FALSE;

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *times = DOT1X_OM_Get_PortMaxReq(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortMaxReq(UI32_T lport,UI32_T *times)
{
    UI32_T unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(times == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = DOT1X_OM_GetRunning_MaxReq(times);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port MaxReq of 1X configuration
 * INPUT    : lport
 * OUTPUT   : times
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortMaxReq(UI32_T *lport,UI32_T *times)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || times == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    *times = DOT1X_OM_Get_PortMaxReq(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_SetDot1xPortReAuthEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : enable/disable port period re-authentication of the 1X client,
 *            which is disabled by default
 * INPUT   : lport - port number
 *           control - VAL_dot1xPaePortReauthenticate_true: enable
 *                     VAL_dot1xPaePortReauthenticate_false: disable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthEnabled(UI32_T lport, UI32_T control)
{
    UI32_T unit, port, trunk_id;
    UI32_T org_status;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    org_status = DOT1X_OM_Get_PortReAuthEnabled(lport);

    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_PortReAuthEnabled(lport, control);
    NETACCESS_MGR_UNLOCK();

    if (   (control != org_status)
        && (VAL_dot1xPaePortReauthenticate_true == control))
    {
        DOT1X_VM_EnableReauthTimer(lport);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Get dot1x period re-authentication status of the specified port
 * INPUT   : lport
 * OUTPUT  : value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthEnabled(UI32_T lport, UI32_T *value_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

    if (NULL == value_p)
    {
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *value_p = DOT1X_OM_Get_PortReAuthEnabled(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetRunningDot1xPortReAuthEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get port re-authentication status
 * INPUT   : lport
 * OUTPUT  : value_p
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortReAuthEnabled(UI32_T lport, UI32_T *value_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    SYS_TYPE_Get_Running_Cfg_T result;

    if (NULL == value_p)
    {
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    result = DOT1X_OM_GetRunning_PortReAuthEnabled(lport, value_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(result);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetNextDot1xPortReAuthEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : value_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p)
{
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == value_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    *value_p = DOT1X_OM_Get_PortReAuthEnabled(*lport_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_DoDot1xReAuthenticate
 *-------------------------------------------------------------------------
 * PURPOSE  : use the command to manually initiate a re-authentication of
 *            the specified 1X enabled port
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_DoDot1xReAuthenticate(UI32_T lport)
{
    UI32_T unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (VAL_dot1xPaeSystemAuthControl_disabled == DOT1X_OM_Get_SystemAuthControl())
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    SYSFUN_Sleep(10);

    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_REAUTH_EV);
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xPortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE  : set port QuietPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortQuietPeriod(UI32_T lport, UI32_T seconds)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();

    ret = DOT1X_OM_Set_PortQuietPeriod(lport,seconds);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds)
{
    UI32_T unit, port, trunk_id;

    if(seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *seconds = DOT1X_OM_Get_PortQuietPeriod(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds)
{
    SYS_TYPE_Get_Running_Cfg_T ret;
    UI32_T unit, port, trunk_id;

    if(seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret =  DOT1X_OM_GetRunning_PortQuietPeriod(lport,seconds);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port QuietPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortQuietPeriod(UI32_T *lport, UI32_T *seconds)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    *seconds = DOT1X_OM_Get_PortQuietPeriod(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : set port ReAuthPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthPeriod(UI32_T lport,UI32_T seconds)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();

    ret = DOT1X_OM_Set_PortReAuthPeriod(lport,seconds);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds)
{
    UI32_T unit, port, trunk_id;

    if(seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *seconds = DOT1X_OM_Get_PortReAuthPeriod(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds)
{
    UI32_T unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(seconds == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = DOT1X_OM_GetRunning_PortReAuthPeriod(lport,seconds);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port ReAuthPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortReAuthPeriod(UI32_T *lport,UI32_T *seconds)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    *seconds = DOT1X_OM_Get_PortReAuthPeriod(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port TxPeriod of 1X configuration
 * INPUT    : lport,seconds
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortTxPeriod(UI32_T lport,UI32_T seconds)
{
    BOOL_T ret;
    UI32_T unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();

    ret = DOT1X_OM_Set_PortTxPeriod(lport,seconds);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds)
{
    UI32_T unit, port, trunk_id;

    if(seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *seconds = DOT1X_OM_Get_PortTxPeriod(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds)
{
    UI32_T unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(seconds == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = DOT1X_OM_GetRunning_PortTxPeriod(lport,seconds);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next port TxPeriod of 1X configuration
 * INPUT    : lport
 * OUTPUT   : seconds
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextDot1xPortTxPeriod(UI32_T *lport,UI32_T *seconds)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || seconds == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    *seconds = DOT1X_OM_Get_PortTxPeriod(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_SetDot1xPortPaePortInitialize
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of dot1xPaePortReauthenticate
 * INPUT   : lport - port number
 *           value - VAL_dot1xPaePortInitialize_true or
 *                   VAL_dot1xPaePortInitialize_false
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTES   : According to MIB description, setting this attribute FALSE
 *           has no effect. This attribute always returns FALSE when it
 *           is read.
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortPaePortInitialize(UI32_T lport, UI32_T value)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (   (VAL_dot1xPaePortInitialize_true != value)
        && (VAL_dot1xPaePortInitialize_false != value))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (   (VAL_dot1xPaeSystemAuthControl_disabled == DOT1X_OM_Get_SystemAuthControl())
        || (VAL_dot1xPaePortInitialize_false == value))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_INIT_EV);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport,direction.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
 *                        VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T dir)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_AdminCtrlDirections(lport, dir);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : direction.
 * NOTES  : direction =  VAL_dot1xAuthAdminControlledDirections_both for Both
 *                       VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T *dir)
{
    UI32_T unit, port, trunk_id;

    if(dir == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *dir = DOT1X_OM_Get_AdminCtrlDirections(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport,timeout.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T timeout)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_AuthSuppTimeout(lport, timeout);
    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T *timeout)
{
    UI32_T unit, port, trunk_id;

    if(timeout == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *timeout = DOT1X_OM_Get_AuthSuppTimeout(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T timeout)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_AuthServerTimeout(lport, timeout);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: seconds.
 * NOTES:
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T *timeout)
{
    UI32_T unit, port, trunk_id;

    if(timeout == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *timeout = DOT1X_OM_Get_AuthServerTimeout(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_SetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Set the port's re-auth max
 * INPUT:  lport, reauth_max
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetDot1xPortReAuthMax(UI32_T lport, UI32_T reauth_max)
{
    UI32_T unit, port, trunk_id;
    BOOL_T ret;

    if(reauth_max == 0)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    ret = DOT1X_OM_Set_PortReAuthMax(lport, reauth_max);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_MGR_GetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetDot1xPortReAuthMax(UI32_T lport, UI32_T *reauth_max)
{
    UI32_T unit, port, trunk_id;

    if(reauth_max == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    *reauth_max = DOT1X_OM_Get_PortReAuthMax(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_MGR_SetDot1xPortAuthTxEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Set the value of the keyTransmissionEnabled constant currently
 *           in use by the Authenticator PAE state machine.
 * INPUT   : lport - port number
 *           value - VAL_dot1xAuthKeyTxEnabled_true or
 *                   VAL_dot1xAuthKeyTxEnabled_false
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE.
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T value)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_LOCK();
    ret = DOT1X_OM_Set_AuthTxEnabled(lport, value);
    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_MGR_GetDot1xPortAuthTxEnabled
 * ------------------------------------------------------------------------
 * PURPOSE : Get the value of the keyTransmissionEnabled constant currently
 *           in use by the Authenticator PAE state machine.
 * INPUT   : lport - port number
 * OUTPUT  : value_p - VAL_dot1xAuthKeyTxEnabled_true or
 *                     VAL_dot1xAuthKeyTxEnabled_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T *value_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == value_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *value_p = DOT1X_OM_Get_AuthTxEnabled(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* for 802.1x MIB (IEEE8021-PAE-MIB)
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p)
{
    UI32_T unit;
    UI32_T uport;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == entry_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &uport, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_GetPaePortEntry(lport, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xPaePortTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == entry_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    ret = NETACCESS_MGR_GetDot1xPaePortEntry(*lport_p, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == entry_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_GetAuthConfigEntry(lport, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthConfigTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == entry_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    ret = NETACCESS_MGR_GetDot1xAuthConfigEntry(*lport_p, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == entry_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_GetAuthStatsEntry(lport, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthStatsTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == entry_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    ret = DOT1X_OM_GetAuthStatsEntry(*lport_p, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T *entry_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == entry_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_GetAuthDiagEntry(lport, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthDiagTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T *entry_p)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == entry_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    ret = NETACCESS_MGR_GetDot1xAuthDiagEntry(*lport_p, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetDot1xSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xSessionStatsEntry(UI32_T lport, DOT1X_AuthSessionStatsEntry_T *entry_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == entry_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_GetAuthSessionStatsEntry(lport, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_MGR_GetNextDot1xSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get next entry of dot1xAuthSessionStatsTable
 * INPUT   : lport_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xSessionStatsEntry(UI32_T *lport_p, DOT1X_AuthSessionStatsEntry_T *entry_p)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ((NULL == lport_p) || (NULL == entry_p))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport_p);
        if (SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if (SWCTRL_LPORT_NORMAL_PORT == type)
        {
            break;
        }
    }

    ret = NETACCESS_MGR_GetDot1xSessionStatsEntry(*lport_p, entry_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*****************For CLI/CGI Show do1x **********************************/
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetDot1xPortAuthorized
 *-------------------------------------------------------------------------
 * PURPOSE:  Get dot1x port authorized status
 * INPUT  :  lport  - logic port number
 * OUTPUT :  status_p   - dot1x port authorized status
 * RETURN :  TRUE - succeeded / FALSE - failed
 * NOTE   :  None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortAuthorized(UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *status_p)
{
    if (NULL == status_p)
    {
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    {
        UI32_T unit;
        UI32_T port;
        UI32_T trunk_id;

        if (SWCTRL_LPORT_NORMAL_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        {
            *status_p = DOT1X_OM_GetPortAuthorized(lport);
        }
        else
        {
            *status_p = NETACCESS_MGR_DOT1X_AUTH_CONTROLLED_PORT_STATUS_ERR;
        }
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_MGR_GetDot1xPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortDetails(UI32_T lport, DOT1X_PortDetails_T *port_details_p)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    BOOL_T ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == port_details_p)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = DOT1X_OM_Get_Port_Details(lport, port_details_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the mac-authentication status for the specified port.
 * INPUT    : lport -- logic port number.
 *            mac_auth_status -- mac-authentication status.
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortStatus(UI32_T lport, UI32_T mac_auth_status)
{
    UI32_T  unit, port, trunk_id;
    NETACCESS_PortMode_T current_port_mode, new_port_mode;
    BOOL_T  ret = TRUE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (    (NETACCESS_TYPE_MACAUTH_ENABLED != mac_auth_status)
         && (NETACCESS_TYPE_MACAUTH_DISABLED != mac_auth_status)
       )
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Unknow mac-auth status(%ld)", mac_auth_status);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &current_port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* exit if no change
     */
    if (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
    {
        if (NETACCESS_TYPE_MACAUTH_ENABLED == mac_auth_status)
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }
    else
    {
        if (NETACCESS_TYPE_MACAUTH_DISABLED == mac_auth_status)
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* MacAuth and GuestVlan shall be mutual exclusive */
    {
        UI32_T  guest_vlan_id;

        NETACCESS_OM_GetSecureGuestVlanId(lport, &guest_vlan_id);

        if ((0 != guest_vlan_id) && (NETACCESS_TYPE_MACAUTH_ENABLED == mac_auth_status))
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get the new secure port mode by dot1x control mode and mac-auth enabled status
     */
#if (SYS_CPNT_DOT1X == TRUE)
    new_port_mode = NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                      DOT1X_OM_Get_SystemAuthControl(), DOT1X_OM_Get_PortControlMode(lport),
                      ((mac_auth_status==NETACCESS_TYPE_MACAUTH_ENABLED)?TRUE:FALSE));
#else

    new_port_mode = NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                      VAL_dot1xPaeSystemAuthControl_disabled, VAL_dot1xAuthAuthControlledPortControl_forceAuthorized,
                      ((mac_auth_status==NETACCESS_TYPE_MACAUTH_ENABLED)?TRUE:FALSE));
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    ret = NETACCESS_MGR_SetSecurePortMode(lport, new_port_mode);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the mac-authentication status for the specified port.
 * INPUT    : lport -- logic port number.
 * OUTPUT   : mac_auth_status -- mac-authentication status.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status)
{
    NETACCESS_PortMode_T port_mode;

    if(mac_auth_status == NULL)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "mac_auth_status=NULL");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *mac_auth_status = (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
        ? NETACCESS_TYPE_MACAUTH_ENABLED : NETACCESS_TYPE_MACAUTH_DISABLED;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the mac-authentication status for the specified port.
 * INPUT    : lport.
 * OUTPUT   : mac_auth_status -- mac-authentication status.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status)
{
    SYS_TYPE_Get_Running_Cfg_T ret;
    NETACCESS_PortMode_T port_mode;

    if(mac_auth_status == NULL)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "mac_auth_status=NULL");
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    *mac_auth_status = (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
        ? NETACCESS_TYPE_MACAUTH_ENABLED : NETACCESS_TYPE_MACAUTH_DISABLED;

    ret = (port_mode == SYS_DFLT_NETACCESS_SECURE_PORT_MODE)
            ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port status of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : mac_auth_status -- mac-authentication status
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortStatus(UI32_T *lport,UI32_T *mac_auth_status)
{
    SWCTRL_Lport_Type_T     type;
    NETACCESS_PortMode_T    port_mode;

    if((lport == NULL) || (mac_auth_status == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL&&mac_auth_status==NULL)?"lport and mac_auth_status":
                                                  (lport==NULL)?"lport": "mac_auth_status");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    if (FALSE == NETACCESS_OM_GetSecurePortMode(*lport, &port_mode))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *mac_auth_status = (TRUE == NETACCESS_VM_IsMacAuthEnabled(*lport))
        ? NETACCESS_TYPE_MACAUTH_ENABLED : NETACCESS_TYPE_MACAUTH_DISABLED;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 *            count -- max mac count
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  orig_count;
    UI32_T  number_stored;
    NETACCESS_PortMode_T port_mode;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (    (FALSE == NETACCESS_OM_GetMacAuthPortMaxMacCount(lport, &orig_count))
         || (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
         || (FALSE == NETACCESS_OM_GetSecureNumberAddressesStored(lport, &number_stored))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* exit if no change
     */
    if (orig_count == count)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check input parameter
     */
    if (   count > MAX_macAuthPortMaxMacCount
        || count < MIN_macAuthPortMaxMacCount)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "out of range of the max mac count(%lu)", count);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* lock
     */
    NETACCESS_MGR_LOCK();

    ret = TRUE;
    if (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
    {
        /* if new value is less than current, delete all mac entry on this port
         */
        if (count < number_stored)
        {
            /* when new value is less than current,delete all mac entry on this port
             */
            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
            {
                NETACCESS_MGR_UNLOCK();
                NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
    }

    ret |= ret && NETACCESS_OM_SetMacAuthPortMaxMacCount(lport,count);

    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T ret;

    if(count == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "count=NULL on port(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = NETACCESS_OM_GetMacAuthPortMaxMacCount(lport, count);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortMaxMacCount(UI32_T lport,UI32_T *count)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(count == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "count=NULL on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (TRUE == NETACCESS_OM_GetMacAuthPortMaxMacCount(lport, count))
    {
        if(SYS_DFLT_NETACCESS_MACAUTH_SECURE_ADDRESSES_PER_PORT == *count)
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

     NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port max allowed MAC number of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortMaxMacCount(UI32_T *lport, UI32_T *count)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    if((lport == NULL) || (count == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL&&count==NULL)?"lport and count":
                                                  (lport==NULL)?"lport": "count");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    ret = NETACCESS_OM_GetMacAuthPortMaxMacCount(*lport, count);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the intrustion action of mac-authentication for the specified port.
 * INPUT    : lport  -- logic port number
 *            action -- intrusion action
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action)
{
    UI32_T  unit, port, trunk_id;
    UI32_T  orig_action;
    NETACCESS_PortMode_T port_mode;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (    (FALSE == NETACCESS_OM_GetMacAuthPortIntrusionAction(lport, &orig_action))
         || (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* exit if no change
     */
    if (orig_action == action)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check input parameter
     */
    if (    (VAL_macAuthPortIntrusionAction_block_traffic!= action)
         && (VAL_macAuthPortIntrusionAction_pass_traffic!= action)
       )
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "invalid intrustion-action(%lu) of mac-auth", action);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* lock
     */
    NETACCESS_MGR_LOCK();

    ret = TRUE;
    if (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
    {
        /* when the intrusion action of mac-authentication changes,delete all mac entry on this port
         */
        if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
        {
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    ret |= ret && NETACCESS_OM_SetMacAuthPortIntrusionAction(lport, action);

    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T ret;

    if(action == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "action=NULL on port(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = NETACCESS_OM_GetMacAuthPortIntrusionAction(lport, action);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetRunningMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T ret;

    if(action == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "action=NULL on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "Logic port(%lu) is not NORMAL port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (TRUE == NETACCESS_OM_GetMacAuthPortIntrusionAction(lport, action))
    {
        if(SYS_DFLT_NETACCESS_MACAUTH_INTRUSIONACTION_ACTION == *action)
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetNextMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next port intrusion action of mac-authentication.
 * INPUT    : lport -- logic port number, use 0 to get first.
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetNextMacAuthPortIntrusionAction(UI32_T *lport, UI32_T *action)
{
    BOOL_T ret;
    SWCTRL_Lport_Type_T type;

    if((lport == NULL) || (action == NULL))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "%s=NULL",
                                                  (lport==NULL && action==NULL)?"lport and action":
                                                  (lport==NULL)?"lport": "action");
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    ret = NETACCESS_OM_GetMacAuthPortIntrusionAction(*lport, action);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAuthPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the port entry of mac-authentication.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : FALSE : error, TRUE : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_MGR_GetMacAuthPortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI32_T *lport;
    BOOL_T  ret;
    NETACCESS_OM_MacAuthPortEntry_T port_entry;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    lport = (UI32_T*)key;

    /* check if input buffer is enough
     */
    if(buffer_size < sizeof(NETACCESS_MGR_MacCountFuncPtr_T))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get mac authentication port entryfrom database
     */
    NETACCESS_MGR_LOCK();
    ret = NETACCESS_OM_GetMacAuthPortEntry(*lport, &port_entry);
    NETACCESS_MGR_UNLOCK();

    if (FALSE == ret)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get information from mac authentication port entry
     */
    switch(field_id)
    {
        case NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_IFINDEX:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(port_entry.lport);
            break;

        case NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_INTRUSION_ACTION:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(port_entry.intrusion_action);
            break;

        case NETACCESS_MGR_FID_MACAUTH_PORT_ENTRY_NUMBER_ADDRESSES:
            NETACCESS_MGR_SET_NON_STRING_BUFFER(port_entry.configured_number_addresses);
            break;

        case SYS_TYPE_FID_ALL:
            {
                NETACCESS_MGR_MacAuthPortEntry_T entry;

                ret = NETACCESS_MGR_LocalCopyMacAuthenticationPortEntry(&entry, &port_entry);
                memcpy(buffer, &entry, sizeof(NETACCESS_MGR_MacAuthPortEntry_T));
            }
            *used_buffer = sizeof(NETACCESS_MGR_MacAuthPortEntry_T);
            break;

        default:
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetNextMacAuthPortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get the next port entry of mac-authentication.
 * INPUT    : field_id     : field id.
 *            *key         : input key
 *            buffer_size  : buffer size
 * OUTPUT   : *buffer      : The secure port entry.
 *            *used_buffer : used buffer size for output
 * RETURN   : FALSE : error, TRUE : success
 * NOTES    : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *              so used_buffer = 4
 *            2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextMacAuthPortEntry(UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI32_T *lport;
    SWCTRL_Lport_Type_T type;

    lport = (UI32_T*)key;

    /* get next port nbr
     */
    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "port(%lu) type is not UNKNOWN", *lport);
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    /* get secure port entry
     */
    if(TRUE == NETACCESS_MGR_GetMacAuthPortEntry(field_id, (void*)lport, buffer, buffer_size, used_buffer))
    {
        memcpy(key, lport, sizeof(UI32_T));
        return TRUE;
    }

    return (FALSE);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureNumberAddressesStoredByRada
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by RADA
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetSecureNumberAddressesStoredByRada(UI32_T lport)
{
    return NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(lport);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_GetSecureNumberAddressesStoredByDot1x
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by Dot1x
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_MGR_GetSecureNumberAddressesStoredByDot1x(UI32_T lport)
{
    UI32_T number_stored;
    NETACCESS_OM_GetSecureNumberAddressesStored(lport, &number_stored);
    return (number_stored - NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(lport));
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port guest VLAN ID.
 * INPUT  : lport,vid.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetSecureGuestVlanId(UI32_T lport, UI32_T vid)
{
    UI32_T                  unit, port, trunk_id, orig_vid;
    NETACCESS_PortMode_T    port_mode;
    BOOL_T                  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "lport(%lu) is not normal port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (    (FALSE == NETACCESS_OM_GetSecureGuestVlanId(lport, &orig_vid))
         || (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* exit if no change
     */
    if (orig_vid == vid)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* check input parameter
     */
    if (vid > MAX_networkAccessPortGuestVlan)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "invalid guest vlan id(%lu) of secure port", vid);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check port mode
     * MacAuth and GuestVlan shall mutual exclusive
     */
    if (TRUE == NETACCESS_VM_IsMacAuthEnabled(lport))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "mac-auth is enabled on port(%ld)", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* lock
     */
    NETACCESS_MGR_LOCK();

    ret = TRUE;
    if (    (TRUE == NETACCESS_MGR_LocalIsDot1xEnabled(port_mode))
       )
    {
        if (FALSE == NETACCESS_VM_LeaveGuestVlan(lport))
        {
            NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "Leave guest VLAN failed");
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    ret = NETACCESS_OM_SetSecureGuestVlanId(lport, vid);

    NETACCESS_MGR_UNLOCK();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T  ret;

    if(NULL == vid)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "null pointer on lport(%ld)", lport);
        return FALSE;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "lport(%lu) is not normal port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = NETACCESS_OM_GetSecureGuestVlanId(lport, vid);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get per-port guest VLAN ID.
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningSecureGuestVlanId(UI32_T lport, UI32_T *vid)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T  ret;

    if(NULL == vid)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "null pointer on lport(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR, "lport(%lu) is not normal port", lport);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(lport, vid))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = (NETACCESS_TYPE_DFLT_GUEST_VLAN_ID == *vid) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next port guest VLAN ID.
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextSecureGuestVlanId(UI32_T *lport, UI32_T *vid)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || vid == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(*lport, vid))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

#if (SYS_CPNT_DOT1X == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Set the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport,
 *            action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT   : none.
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xPortIntrusionAction(UI32_T lport, UI32_T action_status)
{
    UI32_T  unit, port, trunk_id;
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if port is reasonable
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if anything change
     */
    if (action_status == DOT1X_OM_GetPortIntrusionActionStatus(lport))
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    NETACCESS_MGR_LOCK();

    /* Guest VLAN is working with dot1x single and multi host mode only.
     * If mac-based dot1x or mac authentication are enabled,
     * the is_restricted_vlan flag shall never be set.
     */
    /* leave restricted VLAN when changing the intrusion action to block-traffic
     */
    if (VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic == action_status)
    {
        if (FALSE == NETACCESS_VM_LeaveGuestVlan(lport))
        {
            NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "Leave guest VLAN failed");
            NETACCESS_MGR_UNLOCK();
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
    /* set to database
     */
    ret = DOT1X_OM_SetPortIntrusionActionStatus(lport, action_status);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetDot1xPortIntrusionAction() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status)
{
    UI32_T  unit, port, trunk_id;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check if input is valid and get port mode
     */
    if (    (NULL == action_status)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get from database
     */
    *action_status = DOT1X_OM_GetPortIntrusionActionStatus(lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC( ((0!=*action_status)?TRUE:FALSE) );
}/* End of NETACCESS_MGR_GetDot1xPortIntrusionAction() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status)
{
    UI32_T  unit, port, trunk_id;
    SYS_TYPE_Get_Running_Cfg_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input is valid and get port mode
     */
    if (    (NULL == action_status)
        ||  (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
       )
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
    *action_status = DOT1X_OM_GetPortIntrusionActionStatus(lport);

    ret = (DOT1X_DEFAULT_ACTION == *action_status) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningDot1xPortIntrusionAction() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetNextDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next port intrusion action that determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetNextDot1xPortIntrusionAction(UI32_T *lport, UI32_T *action_status)
{
    SWCTRL_Lport_Type_T type;

    if(lport == NULL || action_status == NULL)
        return FALSE;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* get next port nbr
     */
    while (1)
    {
        type = SWCTRL_GetNextLogicalPort(lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT == type)
        {
            NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else if(SWCTRL_LPORT_NORMAL_PORT == type)
            break;
        else
            continue;
    }

    /* get from database
     */
    *action_status = DOT1X_OM_GetPortIntrusionActionStatus(*lport);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC( ((0!=*action_status)?TRUE:FALSE) );
}/* End of NETACCESS_MGR_GetDot1xPortIntrusionAction() */
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_SetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Set status of EAPOL frames pass-through when the global
 *            status dot1x is disabled
 * INPUT    : status    - VAL_dot1xEapolPassThrough_enabled(1)
 *                        VAL_dot1xEapolPassThrough_disabled(2)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_SetDot1xEapolPassThrough(UI32_T status)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (    (VAL_dot1xEapolPassThrough_enabled != status)
        &&  (VAL_dot1xEapolPassThrough_disabled != status)
       )
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_MG_ERR,
            "Unknown status of EAPOL frames pass-through status(%ld)", status);
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

#if (SYS_CPNT_DOT1X == TRUE)
    if (FALSE == NETACCESS_VM_SetDriverForRecievingEapPacket(
        DOT1X_OM_Get_SystemAuthControl(), (DOT1X_OM_EapolPassThru_T)status))
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "NETACCESS_VM_SetDriverForRecievingEapPacket() failed");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    NETACCESS_MGR_LOCK();

    ret = DOT1X_OM_SetEapolPassThrough((DOT1X_OM_EapolPassThru_T)status);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_SetDot1xEapolPassThrough() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_MGR_GetDot1xEapolPassThrough(UI32_T *status)
{
    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *status = DOT1X_OM_GetEapolPassThrough();

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of NETACCESS_MGR_GetDot1xEapolPassThrough() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningDot1xEapolPassThrough(UI32_T *status)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (NULL == status)
    {
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    *status = (UI32_T)DOT1X_OM_GetEapolPassThrough();

    ret = (DOT1X_DEFAULT_EAPOL_PASS_THRU_STATUS == *status) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of NETACCESS_MGR_GetRunningDot1xPortIntrusionAction() */
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC address aging mode.
 * INPUT    : mode -- MAC address aging mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_MGR_SetMacAddressAgingMode(UI32_T mode)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    NETACCESS_MGR_LOCK();

    ret = NETACCESS_VM_SetMacAddressAgingMode(mode);

    NETACCESS_MGR_UNLOCK();
    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- MAC address aging mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_MGR_GetMacAddressAgingMode(UI32_T *mode_p)
{
    BOOL_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = NETACCESS_OM_GetMacAddressAgingMode(mode_p);

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningMacAddressAgingMode
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- VAL_networkAccessAging_enabled,
 *                      VAL_networkAccessAging_disabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_MGR_GetRunningMacAddressAgingMode(UI32_T *mode_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    NETACCESS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    /* check if input is valid
     */
    if (NULL == mode_p)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "null pointer");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* get from database
     */
     if (FALSE == NETACCESS_OM_GetMacAddressAgingMode(mode_p))
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_MG_ERR, "NETACCESS_OM_GetMacAddressAgingMode() failed");
        NETACCESS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = (SYS_DFLT_NETACCESS_AGING_MODE == *mode_p) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    NETACCESS_MGR_RETURN_AND_RELEASE_CSC(ret);
}
#endif /*#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_LocalIsDot1xEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Get the enabled status of 802.1X.
 * INPUT   : port_mode
 * OUTPUT  : None.
 * RETURN  : TRUE  -- enabled
 *           FALSE -- disabled
 * NOTE    :
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_MGR_LocalIsDot1xEnabled(NETACCESS_PortMode_T port_mode)
{
    if (    (port_mode == NETACCESS_PORTMODE_DOT1X)
         || (port_mode == NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE)
       )
    {
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get the secure port mode by dot1x and mac-authentication status.
 * INPUT   : dot1x_sys_control, dot1x_control_mode, mac_auth_enabled.
 * OUTPUT  : None.
 * RETURN  : The secure port mode.
 * NOTE    :
 *-------------------------------------------------------------------------*/
static NETACCESS_PortMode_T NETACCESS_MGR_LocalGetSecurePortModeByDot1xMacAuthStatus(
                                                        UI32_T dot1x_sys_control,
                                                        UI32_T dot1x_control_mode,
                                                        BOOL_T mac_auth_enabled)
{
    if (dot1x_sys_control == VAL_dot1xPaeSystemAuthControl_enabled)
    {
        if (dot1x_control_mode == VAL_dot1xAuthAuthControlledPortControl_auto)
        {
            if (mac_auth_enabled == TRUE)
            {
                return NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE;
            }
            else
            {
                return NETACCESS_PORTMODE_DOT1X;
            }
        }
        else if (dot1x_control_mode == VAL_dot1xAuthAuthControlledPortControl_forceAuthorized)
        {
            if (mac_auth_enabled == TRUE)
            {
                return NETACCESS_PORTMODE_MAC_AUTHENTICATION;
            }
            else
            {
                return NETACCESS_PORTMODE_NO_RESTRICTIONS;
            }
        }

        /* dot1x control mode = force-unauthentorized
         */
        return NETACCESS_PORTMODE_SECURE;
    }

    /* VAL_dot1xPaeSystemAuthControl_disabled */

    if (mac_auth_enabled == TRUE)
    {
        return NETACCESS_PORTMODE_MAC_AUTHENTICATION;
    }

    /* dot1x sys-auth-control = disabled
     * mac-authentication     = disabled
     */
    return NETACCESS_PORTMODE_NO_RESTRICTIONS;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME - NETACCESS_MGR_LocalFreeNewMacMsg
 * ------------------------------------------------------------------------
 *  FUNCTION : Free memory used by new mac msg
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
static void NETACCESS_MGR_LocalFreeNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr)
{
    if (NULL == new_mac_msg_ptr)
        return;

    if (NULL != new_mac_msg_ptr->m_newmac_data)
    {
        L_MM_Free(new_mac_msg_ptr->m_newmac_data);
        new_mac_msg_ptr->m_newmac_data = NULL;
    }
    if (NULL != new_mac_msg_ptr->m_eap_data)
    {
        if (NULL != new_mac_msg_ptr->m_eap_data->pkt_data)
        {
            L_MM_Free(new_mac_msg_ptr->m_eap_data->pkt_data);
            new_mac_msg_ptr->m_eap_data->pkt_data = NULL;
        }

        L_MM_Free(new_mac_msg_ptr->m_eap_data);
        new_mac_msg_ptr->m_eap_data = NULL;
    }
}
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
