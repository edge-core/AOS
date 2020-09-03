/*-----------------------------------------------------------------------------
 * Module Name: dcbx_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the DCBX API
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */

#ifndef DCBX_MGR_H
#define DCBX_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "dcbx_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define DCBX_MGR_IPCMSG_TYPE_SIZE sizeof(union DCBX_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    DCBX_MGR_IPC_SETPORTSTATUS,
    DCBX_MGR_IPC_GETPORTSTATUS,
    DCBX_MGR_IPC_GETRUNNINGPORTSTATUS,
    DCBX_MGR_IPC_SETPORTMODE,
    DCBX_MGR_IPC_GETPORTMODE,
    DCBX_MGR_IPC_GETRUNNINGPORTMODE
};



/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_MGR_IpcMsg_T.data
 */
#define DCBX_MGR_GET_MSG_SIZE(field_name)                       \
            (DCBX_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((DCBX_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */




/* IPC message structure
 */
typedef struct
{
	union DCBX_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
        UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
	    struct
	    {
	        UI32_T arg_ui32_1;
	        UI32_T arg_ui32_2;
	    } arg_grp_ui32_ui32;
	    struct
	    {
	        UI32_T arg_ui32;
	        BOOL_T arg_bool;
	    } arg_grp_ui32_bool;
	} data; /* the argument(s) for the function corresponding to cmd */
} DCBX_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set port_status to control DCBX is enabled
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            BOOL_T port_status      -- Enable(TRUE)
 *                                       Disable(FALSE)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_SetPortStatus(UI32_T lport, BOOL_T port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p      -- Enable(TRUE)
 *                                                       Disable(FALSE)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetPortStatus(UI32_T lport, BOOL_T *port_status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p      -- Enable(TRUE)
 *                                                       Disable(FALSE)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T port_mode        -- manual(1),
 *                                       configuration_source(2),
 *                                       auto-upstream(3),
 *                                       auto-downstream(4)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_SetPortMode(UI32_T lport, UI32_T port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p   --   manual(1),
 *                                                      configuration_source(2),
 *                                                      auto-upstream(3),
 *                                                      auto-downstream(4)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetPortMode(UI32_T lport, UI32_T *port_mode_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p   --   manual(1),
 *                                                      configuration_source(2),
 *                                                      auto-upstream(3),
 *                                                      auto-downstream(4)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetRunningPortMode(UI32_T lport, UI32_T *port_mode_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EnterMasterMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EnterSlaveMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_SetTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_EnterTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberAdd1st_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberDeleteLst_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EtsRcvd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from LLDP when ETS TLV changed for the port
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EtsRcvd_CallBack( UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                    BOOL_T  rem_recommend_rcvd,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_cbs,
                                                    UI8_T  rem_max_tc,
                                                    UI8_T  *rem_config_pri_assign_table,
                                                    UI8_T   *rem_config_tc_bandwidth_table,
                                                    UI8_T   *rem_config_tsa_assign_table,
                                                    UI8_T  *rem_recommend_pri_assign_table,
                                                    UI8_T   *rem_recommend_tc_bandwidth_table,
                                                    UI8_T   *rem_recommend_tsa_assign_table);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_PfcRcvd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from LLDP when PFC TLV changed for the port
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_PfcRcvd_CallBack(UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                     UI8_T   *rem_mac,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_mbc,
                                                    UI8_T  rem_pfc_cap,
                                                    UI8_T  rem_pfc_enable);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : None
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetOperationMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleHotInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_HandleHotInsertion(UI32_T beg_ifindex, UI32_T end_of_index, BOOL_T use_default) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot remove.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port) ;

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for DCBX MGR.
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
BOOL_T DCBX_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the DCBX CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_Init(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell DCBX that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_IsProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by DCBX mgr will tell if DCBX provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T DCBX_MGR_IsProvisionComplete(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_ReRunPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : To re-run port sm when ETS/PFC config is changed.
 * INPUT    : lport       -- lport number
 *            is_ets_chgd -- TRUE if ETS changed/FALSE if PFC changed
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_ReRunPortStateMachine(
    UI32_T  lport,
    BOOL_T  is_ets_chgd);

#endif /* End of DCBX_MGR_H */

