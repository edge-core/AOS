#ifndef	SWDRV_CACHE_MGR_H
#define	SWDRV_CACHE_MGR_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "swdrv_cache_om.h"
#include "dev_swdrv.h"

/*#define SWDRV_CACHE_MGR_ACTION_ADD_MEMBER    SWDRV_CACHE_OM_ACTION_ADD_MEMBER         */
/*#define SWDRV_CACHE_MGR_ACTION_DELETE_MEMBER    SWDRV_CACHE_OM_ACTION_DELETE_MEMBER      */

#define SWDRV_CACHE_OM_NULL                         0
#define SWDRV_CACHE_MSG_EVENT                       BIT_0
#define SWDRV_CACHE_MGR_ACTION_TAG_MEMBER           SWDRV_CACHE_OM_ACTION_TAG_MEMBER        /* 1 */
#define SWDRV_CACHE_MGR_ACTION_UNTAG_MEMBER         SWDRV_CACHE_OM_ACTION_UNTAG_MEMBER      /* 2 */
                                                
#define SWDRV_CACHE_MGR_ACTION_NOTHING              SWDRV_CACHE_OM_ACTION_NOTHING           /* 0 */
#define SWDRV_CACHE_MGR_ACTION_CREATE_VLAN          SWDRV_CACHE_OM_ACTION_CREATE_VLAN       /* 1 */
#define SWDRV_CACHE_MGR_ACTION_DESTORY_VLAN         SWDRV_CACHE_OM_ACTION_DESTORY_VLAN      /* 2 */
#define SWDRV_CACHE_MGR_ACTION_MEMBER               SWDRV_CACHE_OM_ACTION_MEMBER            /* 3 */

typedef struct  
{
   UI16_T   mtype;
   UI32_T   vid;
   UI32_T   port;
   UI32_T   untagged;
} Msg_T;            /* 14 bytes in total */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Init this module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void SWDRV_CACHE_MGR_Init();

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Make this module enter master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void SWDRV_CACHE_MGR_EnterMasterMode();

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Make this module enter slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void SWDRV_CACHE_MGR_EnterSlaveMode();

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SWDRV_CACHE_MGR_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set flag of transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void SWDRV_CACHE_MGR_SetTransitionMode();

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Make this module enter transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void SWDRV_CACHE_MGR_EnterTransitionMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_MGR_GetOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T SWDRV_CACHE_MGR_GetOperationMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_MGR_SetQueueId
 *---------------------------------------------------------------------------
 * PURPOSE:  Call this rutine to let SWDRV_CACHE_MGR know SWDRV_CACHE_TASK
 *           messqge queue id.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_MGR_SetQueueId(UI32_T queue_id);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_MGR_SetTaskId
 *---------------------------------------------------------------------------
 * PURPOSE:  Call this rutine to let SWDRV_CACHE_MGR know SWDRV_CACHE_TASK
 *           task id.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_MGR_SetTaskId(UI32_T task_id);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_UpdateChip
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	: first_delay_time : enter this function, sleep time
 *            sleep_time : aftre one process, sleep time
              process_num : max num per process
 *  OUTPUT  : None
 *  RETURN  : TRUE -- success. FALSE -- failure
 *  NOTE    :
 * ---------------------------------------------------------------------*/
void SWDRV_CACHE_MGR_UpdateChip(UI32_T first_delay_time , UI32_T  sleep_time , UI32_T process_num);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_CreateVLAN
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	: vlanId : VLAN ID
 *  OUTPUT  : None
 *  RETURN  : TRUE -- success. FALSE -- failure
 *  NOTE    :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_CreateVLAN(UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_DestoryVLAN
 * ---------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT	: vlanId - indicate which VLAN.
 *  OUTPUT	: None
 *  RETURN	: TRUE -- success. FALSE -- failure
 *  NOTE    : None.
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_DestoryVLAN(UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_AddVlanMemberSet
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	 : vlanId : VLAN ID
 *             portNo : the port number that need to action.
 *  OUTPUT   : None
 *  RETURN   : TRUE -- success. FALSE -- failure
 *  NOTE     :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_AddVlanMemberSet(UI32_T portNo, UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_DeleteVlanMemberSet
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	 : vlanId : VLAN ID
 *             portNo : the port number that need to action.
 *  OUTPUT   : None
 *  RETURN   : TRUE -- success. FALSE -- failure
 *  NOTE     :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_DeleteVlanMemberSet(UI32_T portNo, UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_AddUntaggedSet
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	 : vlanId : VLAN ID
 *             portNo : the port number that need to action.
 *  OUTPUT   : None
 *  RETURN   : TRUE -- success. FALSE -- failure
 *  NOTE     :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_AddUntaggedSet(UI32_T portNo, UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_DeleteUntaggedSet
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	 : vlanId : VLAN ID
 *             portNo : the port number that need to action.
 *  OUTPUT   : None
 *  RETURN   : TRUE -- success. FALSE -- failure
 *  NOTE     :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_DeleteUntaggedSet(UI32_T portNo, UI32_T vlanId);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_SetPVID
 * ---------------------------------------------------------------------
 *  FUNCTION :
 *  INPUT	 : vlanpvid : VLAN ID of port
 *             portNo : the port number that need to action.
 *  OUTPUT   : None
 *  RETURN   : TRUE -- success. FALSE -- failure
 *  NOTE     :
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_SetPVID(UI32_T portNo, UI32_T pvid);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_SetPortXstpState
 * ---------------------------------------------------------------------
 *  FUNCTION :  This function sets the Port Spanning Tree State of the specified port.
 *  INPUT	 :  mstid              -- Multi-Spanning Tree ID
 *              vlan_count         -- total vlan count
 *              vlan_list          -- the vlan list (UI16_T array)
 *              unit_id            -- which unit
 *              port               -- which port to set
 *              state              -- the port spanning tree state
 *  OUTPUT   :  None
 *  RETURN   :  TRUE -- success. FALSE -- failure
 *  NOTE     : 1. In multiple spanning tree (MSTP) mode,
 *              a) In multiple spanning tree environment, each port will have a specific
 *                 state on a given spanning tree.
 *              b) Most of ASIC uses the per VLAN port state to implement the Port Spanning Tree State.
 *              c) Since a spanning tree is running over a certain VLANs, we need to configure the
 *                 all "per VLAN port states" to the same port state.
 *              d) The vlan_count and vlan_list shows the vlan groups that a given spanning tree
 *                 is running.
 *             2. In single spanning tree (SSTP) mode, the vlan_count and vlan_list shall be ignored.
 *                The calling route shall set vlan_count = 0, and vlan_list = NULL.
 *             3. The enumerated value for port spanning tree state is defined in file, "leaf_1493.h".
 *                  #define VAL_dot1dStpPortState_disabled
 *                  #define VAL_dot1dStpPortState_blocking
 *                  #define VAL_dot1dStpPortState_listening
 *                  #define VAL_dot1dStpPortState_learning
 *                  #define VAL_dot1dStpPortState_forwarding
 *             4. This function SHALL ONLY be supported/invoked for those ASICs which DO NOT have
 *                "Multiple Spanning Instance" concept in their design.
 *                This function is exclusive with the following service routines.
 *                  DEV_SWDRV_SetPortSTAStateWithMstid();
 *                  DEV_SWDRV_AddVlanToSTAWithMstid();
 *                  DEV_SWDRV_DeleteVlanFromSTAWithMstid();
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_SetPortXstpState( UI32_T mstid,
                                         UI32_T vlan_count,
                                         UI16_T *vlan_list,
                                         UI32_T unit_id,
                                         UI32_T port,
                                         UI32_T state);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_RegisterFinishUpdateChipProcess_CallBack                 
 *------------------------------------------------------------------------------
 * PURPOSE  : It is used to apply an interface for other modules to register    
 *            their call back functions.  The registered function would be called 
 *            when a logical port is added to a trunk  
 * INPUT    : void (*fun) () -- CallBack function pointer.       
 * OUTPUT   : none                                                              
 * RETURN   : none                                                              
 * NOTES    : none                                              
 *------------------------------------------------------------------------------*/  
void SWDRV_CACHE_MGR_RegisterFinishUpdateChipProcess_CallBack(void (*fun)(void));

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_MGR_Notify_FinishUpdateChipProcess
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when chip has been updated
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SWDRV_CACHE_MGR_Notify_FinishUpdateChipProcess(void);

/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_MGR_FlushVIDEntry                       |
 * ---------------------------------------------------------------------|
 *  FUNCTION :                                                          |
 *  INPUT	: vid    : VLAN ID which want to flush.                     |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- failure                         |
 *  NOTE    :                                                           |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_MGR_FlushVIDEntry(UI32_T vid);


#endif /* end of SWDRV_CACHE_MGR_H */
