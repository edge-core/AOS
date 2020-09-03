/* Module Name: TRK_MGR.C
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the interface to create and destroy a
 *         trunk port.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes all the manipulation of port trunking.
 *        ( 3.  The domain would not be handled by this module. )
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2001/7/1    Jimmy Lin   Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "swctrl.h"
#include "swctrl_backdoor.h"
#include "sys_time.h"
#include "syslog_type.h"
#include "trk_mgr.h"
#include "vlan_mgr.h"
#include "swctrl_group.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif

#if (SYS_CPNT_ADD == TRUE)
#include "add_om.h"
#endif

#include "lacp_pmgr.h"
#include "lacp_om.h"

/* MACRO DEFINITIONS
 */
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #define TRK_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr) \
                do {                                        \
                    SYSCTRL_XOR_MGR_GetSemaphore();         \
                    if (!(expr))                            \
                    {                                       \
                        SYSCTRL_XOR_MGR_ReleaseSemaphore(); \
                        return ret_val;                     \
                    }                                       \
                } while (0)

    #define TRK_MGR_XOR_UNLOCK_AND_RETURN(ret_val)          \
                do {                                        \
                    SYSCTRL_XOR_MGR_ReleaseSemaphore();     \
                    return ret_val;                         \
                } while (0)

#else
    #define TRK_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr) \
                do {} while (0)

    #define TRK_MGR_XOR_UNLOCK_AND_RETURN(ret_val)          \
                do {return ret_val;} while (0)

#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */


/* LOCAL VARIABLES
 */
static UI32_T                       last_change_time;

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
static void TRK_MGR_Notify_AddStaticTrunkMember(UI32_T trunk_ifindex, UI32_T tm_ifindex);
static void TRK_MGR_Notify_DelStaticTrunkMember(UI32_T trunk_ifindex, UI32_T tm_ifindex);
#endif


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T TRK_MGR_AddPortIntoTrunk( UI32_T trunk_id,
                                        UI32_T unit,
                                        UI32_T port,
                                        BOOL_T is_static_member,
                                        BOOL_T is_active_member);


static BOOL_T TRK_MGR_DeletePortFromTrunk(UI32_T trunk_id,
                                          UI32_T unit,
                                          UI32_T port);

static BOOL_T TRK_MGR_IsStaticTrunk(UI32_T trunk_id);





enum
{
    TRK_MGR_CreateTrunk_FUN_NO,
    TRK_MGR_DestroyTrunk_FUN_NO,
    TRK_MGR_AllocateTrunkIdCreateDynamic_FUN_NO,
    TRK_MGR_FreeTrunkIdDestroyDynamic_FUN_NO,
    TRK_MGR_AddTrunkMember_FUN_NO,
    TRK_MGR_AddDynamicTrunkMember_FUN_NO,
    TRK_MGR_DeleteDynamicTrunkMember_FUN_NO,
    TRK_MGR_DeleteTrunkMember_FUN_NO,
    TRK_MGR_AddPortIntoTrunk_FUN_NO,
    TRK_MGR_DeletePortFromTrunk_FUN_NO,
    TRK_MGR_GetTrunkName_FUN_NO,
    TRK_MGR_SetTrunkName_FUN_NO,
    TRK_MGR_IsDynamicTrunkId_FUN_NO,
    TRK_MGR_GetTrunkMemberCounts_FUN_NO,
    TRK_MGR_GetNextTrunkId_FUN_NO,
    TRK_MGR_GetTrunkEntry_FUN_NO,
    TRK_MGR_GetNextTrunkEntry_FUN_NO,
    TRK_MGR_SetTrunkPorts_FUN_NO,
    TRK_MGR_SetTrunkStatus_FUN_NO,
    TRK_MGR_GetTrunkPorts_FUN_NO
};


#if (SYS_CPNT_EH == TRUE)
static UI8_T trk_mgr_static_trk_msg_p[]="trunk is a static trunk";
static UI8_T trk_mgr_port_is_a_trunk_mbr[]="port is already a member of trunk";
static UI8_T trk_mgr_out_of_range_ifidnex[]="ifindex";
static UI8_T trk_mgr_out_of_range_trunk_id[]="Trunk ID (1-6)";
static UI8_T trk_mgr_cfg_no_trk_member[]="trunk contains no members";
static UI8_T trk_mgr_out_of_range_trunk_name[]="trunk name out of range";
static UI8_T trk_mgr_invalid_status[]="invalid trunk status";
#endif


SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Port Trunk module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_Init(void)
{
    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_Create_InterCSC_Relation(void)
{
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: called by stkpkg to to set transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will initialize the Port Trunk module and
 *           free all resource to enter transition mode while stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Port Trunk module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the Poer Trunk services and
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();

}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: TRK_MGR_HandleHotInsertion
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

 * -------------------------------------------------------------------------*/
void TRK_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* do nothing, because knowledge in SWCTRL
     */
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: TRK_MGR_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void TRK_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* do nothing, because knowledge in SWCTRL
     */
}


#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_Notify_AddStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when UIs add static trunk member.
 * INPUT   : trunk_ifindex --- Trunk member is added to which trunk.
 *           tm_ifindex    --- Which trunk member is added to trunk.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void TRK_MGR_Notify_AddStaticTrunkMember(UI32_T trunk_ifindex, UI32_T tm_ifindex)
{

    UI32_T port_type;
    UI32_T unit, port, trunk_id, member_trunk_id,lacp_state;

    if (SWCTRL_BACKDOOR_IsDebugFlagOn (SWCTRL_BACKDOOR_DEBUG_FLAG_CALLBACK_NOTIFY))
        BACKDOOR_MGR_Printf("\r\n[trunk_ifindex = %ld, tm_ifindex = %ld][                 ][TRK_MGR_Notify_AddStaticTrunkMember]", trunk_ifindex, tm_ifindex);

    /* gordon_kao: Callback will deplay until next dequeue the IPC message,
     *             so we do the essential action first then callback to lacp for
     *             lacp state machine
     */

    SWCTRL_LogicalPortToUserPort(trunk_ifindex, &unit, &port, &trunk_id);
    port_type   = SWCTRL_LogicalPortToUserPort(tm_ifindex, &unit, &port, &member_trunk_id);

    if (!TRK_MGR_IsStaticTrunk(trunk_id) )
    {
        UI32_T tm_num;
        BOOL_T result;

        SYS_TYPE_Uport_T unit_ports[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];

        /* remove all members in this trunk */
        if (TRK_MGR_GetTrunkPorts(trunk_id, &tm_num, unit_ports))
        {
            int i;
            for(i = 0; i < tm_num; i++)
            {
                TRK_MGR_DeletePortFromTrunk(trunk_id, unit_ports[i].unit, unit_ports[i].port);
            }
        }
        result = SWCTRL_DestroyTrunk(trunk_id);
	SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    } /* End of if (_dynamic_trunk_id_) */

    if (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
    {
       TRK_MGR_DeletePortFromTrunk(member_trunk_id, unit, port);

        /* To join a LACP member to a static trunk,
         * if the member is last member of LACP,
         * should destroy the LACP trunk.
         */
        if (FALSE == TRK_MGR_IsStaticTrunk(member_trunk_id))
        {
            if (0 == TRK_MGR_GetTrunkMemberCounts(member_trunk_id))
            {
                SWCTRL_DestroyTrunk(member_trunk_id);
            }
        }
    } /* End of if (_trunk_member_) */
    /*EPR:ES3628BT-FLF-ZZ-00518
     *Problem:PortTrunk: add member to static trunk
                         cause DUT spanning tree not stable and print a lot
                         of error messages.
     *Root cause: When add the static trunk member,function set the lacp_oper_status
     *            to disable
     *Solution: When add the static trunk member,need check the lacp admin
     *          is enable or not!if enable make it oper to disable
     *Modify file:trk_mgr.c
     *Fixedby: DanXie
     */
#if 0 /* DanXie, Friday, January 09, 2009 4:00:16 */
    /* Set lacp oper status to disable */
    SWCTRL_SetPortLacpOperEnable(tm_ifindex, VAL_lacpPortStatus_disabled);
#else
    if(LACP_OM_GetRunningDot3adLacpPortEnabled(tm_ifindex,&lacp_state) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        if(lacp_state == LACP_ADMIN_ON)
            SWCTRL_SetPortLacpOperEnable(tm_ifindex, VAL_lacpPortStatus_disabled);
    }
#endif /* #if 0 */

    SWCTRL_GROUP_AddStaticTrunkMemberCallbackHandler(trunk_ifindex, tm_ifindex);
    SYS_CALLBACK_MGR_AddStaticTrunkMember_CallBack(SYS_MODULE_TRUNK, trunk_ifindex, tm_ifindex);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_Notify_DelStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when UIs delete static trunk member.
 * INPUT   : trunk_ifindex --- Trunk member is deleted from which trunk.
 *           tm_ifindex    --- Which trunk member is deleted from trunk.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void TRK_MGR_Notify_DelStaticTrunkMember(UI32_T trunk_ifindex, UI32_T tm_ifindex)
{
    UI32_T lacp_state;
    if (SWCTRL_BACKDOOR_IsDebugFlagOn (SWCTRL_BACKDOOR_DEBUG_FLAG_CALLBACK_NOTIFY))
        BACKDOOR_MGR_Printf("\r\n[trunk_ifindex = %ld, tm_ifindex = %ld][                 ][TRK_MGR_Notify_DelStaticTrunkMember]", trunk_ifindex, tm_ifindex);

    /* gordon_kao: Callback will deplay until next dequeue the IPC message,
     *             so we do the essential action first then callback to lacp for
     *             lacp state machine
     */
     /*EPR:ES3628BT-FLF-ZZ-00518
      *Problem:PortTrunk: Remove member from static trunk
                          cause DUT spanning tree not stable and print a lot
                          of error messages.
      *Root cause: When delete the static trunk member,function set the lacp_oper_status
      *            to enable,so make the port link status to lacp type,cause the STP
      *            could not run correct
      *Solution: When delete the static trunk member,need check admin-lacp is enable or not
      *          if enable set lacp_oper_state to enable
      *Modify file:trk_mgr.c
      *Fixedby: DanXie
      */
#if 0 /* DanXie, Friday, January 09, 2009 4:00:20 */
    SWCTRL_SetPortLacpOperEnable(tm_ifindex, VAL_lacpPortStatus_enabled);
#else
    if(LACP_OM_GetRunningDot3adLacpPortEnabled(tm_ifindex,&lacp_state) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        if(lacp_state == LACP_ADMIN_ON)
            SWCTRL_SetPortLacpOperEnable(tm_ifindex, VAL_lacpPortStatus_enabled);
    }
#endif /* #if 0 */
    SWCTRL_GROUP_DelStaticTrunkMemberCallbackHandler(trunk_ifindex, tm_ifindex);
    SYS_CALLBACK_MGR_DelStaticTrunkMember_CallBack(SYS_MODULE_TRUNK, trunk_ifindex, tm_ifindex);
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunking port to create
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_CreateTrunk(UI32_T trunk_id)
{
    BOOL_T retval;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_CreateTrunk_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    /* check the trunk have been created or not
     */
    if(TRK_MGR_IsTrunkExist (trunk_id, &is_static))
    {
        return TRUE;
    }

    retval = SWCTRL_CreateTrunk(trunk_id, TRUE /* static */);

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return retval;
} /* End of TRK_MGR_CreateTrunk() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_DestroyTrunk(UI32_T trunk_id)
{
    BOOL_T retval;
    BOOL_T is_static;
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    UI32_T trunk_ifindex;
    SWCTRL_TrunkPortExtInfo_T trunk_port_ext_info;
    UI32_T i;
    UI32_T tm_ifindex;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_DestroyTrunk_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);

        return FALSE;
    }

    /* check the trunk have been created or not
     */
    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_DestroyTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return TRUE;
    }

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    if (is_static == TRUE)
    {
        if (SWCTRL_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex) == FALSE)
        {
            return FALSE;
        }

        if (SWCTRL_GetTrunkPortExtInfo(trunk_ifindex, &trunk_port_ext_info) == FALSE)
        {
            return FALSE;
        }
    }
#endif /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    TRK_MGR_XOR_LOCK_AND_FAIL_RETURN(
    FALSE,
    (TRUE == SYSCTRL_XOR_MGR_PermitBeingDestroyTrunk(trunk_id)));
#endif

    retval = SWCTRL_DestroyTrunk(trunk_id);

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    if (  (TRUE == retval)
        &&(TRUE == is_static)
       )
    {
        for (i = 0; i < trunk_port_ext_info.member_number; i++)
        {
            SWCTRL_UserPortToIfindex(trunk_port_ext_info.member_list[i].unit,
                                     trunk_port_ext_info.member_list[i].port,
                                     &tm_ifindex);
            TRK_MGR_Notify_DelStaticTrunkMember(trunk_ifindex, tm_ifindex);
        }
    }
#endif /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    TRK_MGR_XOR_UNLOCK_AND_RETURN(retval);
#else
    return retval;
#endif
} /* End of TRK_MGR_DestroyTrunk() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AllocateTrunkIdCreateDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allocate(create) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_AllocateTrunkIdCreateDynamic(UI32_T *trunk_id)
{
    UI32_T i;
    BOOL_T retval;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    for(i=1; i<=SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
        if(!TRK_MGR_IsTrunkExist (i, &is_static))
            break;

    if (i > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_AllocateTrunkIdCreateDynamic_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    retval = SWCTRL_CreateTrunk(i, FALSE /* dynamic */);
    if (retval)
        retval = SWCTRL_SetTrunkStatus(i, VAL_trunkStatus_valid);

    *trunk_id = i;
    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return retval;
} /* End of TRK_MGR_AllocateTrunkIdCreateDynamic () */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_CreateDynamicTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allocate(create) a dynamic trunk
 * INPUT   : trunk_id -- The dynamic
 * OUTPUT  : None.
 * RETURN  : TRUE  -- 1. This trunk is dynamic already.
 *                    2. This trunk is created as dynamic trunk.
 *           FALSE -- 1. This trunk is static trunk already.
 *                    2. This trunk cannot be created.
 * NOTE    : for LACP.
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_CreateDynamicTrunk(UI32_T trunk_id)
{
    BOOL_T retval;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (TRUE == TRK_MGR_IsTrunkExist(trunk_id, &is_static))
    {
        if (TRUE == TRK_MGR_IsStaticTrunk(trunk_id))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    retval = SWCTRL_CreateTrunk(trunk_id, FALSE);

    if ( TRUE == retval)
    {
        retval = SWCTRL_SetTrunkStatus(trunk_id, VAL_trunkStatus_valid);
    }
    if (TRUE == retval)
    {
        SYS_TIME_GetSystemUpTimeByTick(&last_change_time);
    }

    return retval;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_FreeTrunkIdDestroyDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will free(destroy) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_FreeTrunkIdDestroyDynamic(UI32_T trunk_id)
{
    BOOL_T retval;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_FreeTrunkIdDestroyDynamic_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_FreeTrunkIdDestroyDynamic_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return TRUE;
    }

    /* return FALSE if the trunk is static trunk for user
     */
    if(TRK_MGR_IsStaticTrunk (trunk_id))
    {

        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_FreeTrunkIdDestroyDynamic_FUN_NO, EH_TYPE_MSG_TRUNK_CNFG_ERR, SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
        return FALSE;
    }

    retval = SWCTRL_DestroyTrunk(trunk_id);

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return retval;

} /* End of TRK_MGR_FreeTrunkIdDestroyDynamic() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AddTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           unit     -- which unit to add
 *           port     -- which port to add
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_AddTrunkMember(UI32_T trunk_id, UI32_T ifindex)
{
    BOOL_T retval;
    UI32_T unit;
    UI32_T port;
    BOOL_T is_static;

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    UI32_T to_join_trunk_ifindex = trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;
    BOOL_T is_used_to_be_static_trk;
    UI32_T current_trunk_ifindex;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return TRK_MGR_OTHER_WRONG;
    }

    if (trunk_id < 1 ||
        trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        /*EH_MGR_Handle_Exception(XXX); */
        return TRK_MGR_ERROR_TRUNK;
    }

    if (FALSE == TRK_MGR_IsTrunkExist(trunk_id, &is_static))
    {
        /*EH_MGR_Handle_Exception(XXX)*/
        return TRK_MGR_ERROR_TRUNK;
    }

    if (ifindex == 0  ||
        ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        /*EH_MGR_Handle_Exception(XXX)*/
        return TRK_MGR_ERROR_MEMBER;
    }

        unit =  ((UI32_T)((ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1;
        port =  ifindex - (unit-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;


#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    is_used_to_be_static_trk = TRK_MGR_IsStaticTrunk (trunk_id);

    if (TRUE == SWCTRL_IsTrunkMember(ifindex, &current_trunk_ifindex, &is_static))
    {
        if (TRUE == is_static)
        {
            /* this user port is currently static trunk member
             */
            if (current_trunk_ifindex == to_join_trunk_ifindex)
            {
                /* user want to make a user port:
                 * member of static trunk A become member of static trunk A
                 */
                return TRK_MGR_SUCCESS;
            }
            else
            {
                /* user want to make a user port:
                 * member of static trunk A become member of static/dynamic trunk B
                 */
                /*EH_MGR_Handle_Exception(XXX)*/
                return TRK_MGR_ERROR_MEMBER;
            }
        }
        else
        {
            /* this user port is currently dynamic trunk member:
             * 1) current_trunk_ifindex == to_join_trunk_ifindex
             * 2) to_join_trunk_ifindex != to_join_trunk_ifindex
             * then
             * 1) LACP will destroy whole trunk
             * 2) LACP will remove this user port from current trunk
             */
            ;
        }
    }
    else
    {
        /* This user port is NOT trunk member
         * 1) to_join_trunk_ifindex is dynamic trunk
         * 2) to_join_trunk_ifindex is static trunk
         * then
         * 1) LACP will destroy whole trunk
         * 2) LACP do nothing (except database)
         */
        ;
    }

    /* callback LACP to do somthing before all actions
     */
    TRK_MGR_Notify_AddStaticTrunkMember(trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1, ifindex);

    if (FALSE == is_used_to_be_static_trk)
    {
        /* It's dynamic trunk before.
         * S'pose this dynamic trunk was destroied by LACP when callback.
         * It's necessary to create a static trunk now.
         */
        if (FALSE == SWCTRL_CreateTrunk(trunk_id, TRUE))
        {
            return TRK_MGR_ERROR_MEMBER;
        }
    }
    else
    {
        /* It's static trunk before
         * S'pose LACP did not destroy the static trunk.
         * It's NOT necessary to create a trunk.
         */
         ;
    }
#else
    /* Trunk is created by LACP, then only LACP can add the trunk member
     */
    if (FALSE == TRK_MGR_IsStaticTrunk (trunk_id))
    {
        /*EH_MGR_Handle_Exception(XXX)*/
        return TRK_MGR_ERROR_TRUNK;
    }
#if (SYS_CPNT_LACP == TRUE)
    /*The lacp enabled port can't join the static trunk, macauley */
    {
        LACP_MGR_Dot3adLacpPortEntry_T  lacp_entry;
        lacp_entry.dot3ad_lacp_port_index = ifindex;
        if ((TRUE == LACP_PMGR_GetDot3adLacpPortEntry(&lacp_entry)) &&
            (VAL_lacpPortStatus_enabled == lacp_entry.dot3ad_lacp_port_status))
        {
            return TRK_MGR_ERROR_MEMBER;
        }
    }
#endif
#endif

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)

    SYSCTRL_XOR_MGR_GetSemaphore();

    if (TRUE == SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk(ifindex))
    {
        retval= TRK_MGR_AddPortIntoTrunk(trunk_id, unit, port, TRUE, TRUE);
    }
    else
    {
        retval = TRK_MGR_ERROR_MEMBER;
    }

    SYSCTRL_XOR_MGR_ReleaseSemaphore();

#else /* SYS_CPNT_SYSCTRL_XOR == FALSE */

    retval = TRK_MGR_AddPortIntoTrunk(trunk_id, unit, port, TRUE, TRUE);

#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    return retval;
} /* End of TRK_MGR_AddTrunkMember() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AddDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           unit     -- which unit to add
 *           port     -- which port to add
 *           is_active_member-- active or inactive member
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value, for LACP
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_AddDynamicTrunkMember(UI32_T trunk_id, UI32_T ifindex, BOOL_T is_active_member)
{
    BOOL_T retval;
    UI32_T unit;
    UI32_T port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return TRK_MGR_ERROR_TRUNK;
    }

    /* return FALSE if the trunk is static trunk for user
     */
    if(TRK_MGR_IsStaticTrunk (trunk_id))
    {
#if (SYS_CPNT_SUPPORT_NULL_TRUNK == TRUE)
        if (TRK_MGR_GetTrunkMemberCounts(trunk_id)>0)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_AddDynamicTrunkMember_FUN_NO, EH_TYPE_MSG_TRUNK_MEMBER_CNFG_ERR,
                                     SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
            return TRK_MGR_ERROR_TRUNK;
        }

#else
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_AddDynamicTrunkMember_FUN_NO, EH_TYPE_MSG_TRUNK_MEMBER_CNFG_ERR,
                                 SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
        return TRK_MGR_ERROR_TRUNK;
#endif
    }

    if (ifindex == 0  ||
        ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK, TRK_MGR_AddDynamicTrunkMember_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_ifidnex);
        return TRK_MGR_ERROR_TRUNK;
    }
    else
    {
        unit =  ((UI32_T)((ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1;
        port =  ifindex - (unit-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;
    }

    retval=TRK_MGR_AddPortIntoTrunk(trunk_id, unit, port, FALSE, is_active_member);
    return retval;

} /* End of TRK_MGR_AddDynamicTrunkMember() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_DeleteTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_DeleteTrunkMember(UI32_T trunk_id, UI32_T ifindex)
{
    BOOL_T retval;
    UI32_T unit;
    UI32_T port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* return FALSE if the trunk is dynamic trunk for LACP
     */
    if (!TRK_MGR_IsStaticTrunk (trunk_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_DeleteTrunkMember_FUN_NO,
                                 EH_TYPE_MSG_TRUNK_MEMBER_CNFG_ERR, SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
        return FALSE;
    }

    if (trunk_id < 1 ||
        trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        /*EH_MGR_Handle_Exception(XXX)*/
        return FALSE;
    }

    if (ifindex == 0  ||
        ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK, TRK_MGR_DeleteTrunkMember_FUN_NO,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_ifidnex);
        return FALSE;
    }
    else
    {
        unit =  ((UI32_T)((ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1;
        port =  ifindex - (unit-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;
    }

    retval=TRK_MGR_DeletePortFromTrunk(trunk_id, unit, port);

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    /* callback LACP after all actions are completed
     */
    if (TRUE == retval)
    {
        TRK_MGR_Notify_DelStaticTrunkMember(trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1, ifindex);
    }
#endif

    return retval;
} /* End of TRK_MGR_DeleteTrunkMember() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_DeleteDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_DeleteDynamicTrunkMember( UI32_T trunk_id, UI32_T ifindex)
{
    BOOL_T retval;
    UI32_T unit;
    UI32_T port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    /* return FALSE if the trunk is static trunk for user
     */
    if(TRK_MGR_IsStaticTrunk (trunk_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_DeleteDynamicTrunkMember_FUN_NO, EH_TYPE_MSG_TRUNK_MEMBER_CNFG_ERR,
                                 SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
        return FALSE;
    }

    if (ifindex == 0  ||
        ifindex > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK, TRK_MGR_DeleteDynamicTrunkMember_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_ifidnex);
        return TRK_MGR_ERROR_TRUNK;
    }
    else
    {
        unit =  ((UI32_T)((ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1;
        port =  ifindex - (unit-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;
    }


    retval=TRK_MGR_DeletePortFromTrunk(trunk_id, unit, port);
    return retval;
} /* End of TRK_MGR_DeleteDynamicTrunkMember() */




/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AddPortIntoTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           unit     -- which unit to add
 *           port     -- which port to add
 *           is_static-- static or dynamic trunk
 *           is_active-- active or inactive member
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value
 * -------------------------------------------------------------------------*/
static UI32_T TRK_MGR_AddPortIntoTrunk( UI32_T trunk_id,
                                        UI32_T unit,
                                        UI32_T port,
                                        BOOL_T is_static_member,
                                        BOOL_T is_active_member)
{
    SYS_TYPE_Uport_T unit_port;
    SWCTRL_Lport_Type_T port_type;

    UI32_T retval, new_l_port;
    UI32_T trunk_ifindex, tm_trunk_ifindex;
    BOOL_T is_static;

    unit_port.unit = unit;
    unit_port.port = port;
    if (trunk_id < 1 || trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK, TRK_MGR_AddPortIntoTrunk_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return TRK_MGR_ERROR_TRUNK;
    }

    if (!(TRK_MGR_IsTrunkExist(trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK, TRK_MGR_AddPortIntoTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return TRK_MGR_ERROR_TRUNK;
    }

    /* error if this port is already a trunk member or not existing
     */
    port_type = SWCTRL_UserPortToIfindex(unit, port, &new_l_port);

    trunk_ifindex = trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;

    if (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
    {
        SWCTRL_GetTrunkIfIndexByUport(new_l_port, &tm_trunk_ifindex);
        if (trunk_ifindex == tm_trunk_ifindex)
        {
            return TRK_MGR_SUCCESS; /* the trunk member is the member of trunk_id already */
        }
        else
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_AddPortIntoTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_CNFG_ERR, SYSLOG_LEVEL_INFO, trk_mgr_port_is_a_trunk_mbr);
            return TRK_MGR_ERROR_MEMBER; /* the trunk member it the member of other trunk */
        }
    }

#if (SYS_CPNT_ADD == TRUE)
    /* macauley add for voice vlan, the port must not be the voice vlan member*/
    {
        UI32_T mode = VAL_voiceVlanPortMode_none;

        if (TRUE == ADD_OM_IsVoiceVlanEnabled())
        {
            ADD_OM_GetVoiceVlanPortMode(new_l_port, &mode);
            if (VAL_voiceVlanPortMode_none != mode)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_AddPortIntoTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_CNFG_ERR, SYSLOG_LEVEL_INFO,"trunk member is the voice vlan member");
                return TRK_MGR_ERROR_MEMBER;
            }
        }

    }
#endif /*#if (SYS_CPNT_ADD == TRUE)*/

    retval = SWCTRL_AddTrunkMember(trunk_id, unit_port, is_static_member, is_active_member);

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    if (retval)
    {
        return TRK_MGR_SUCCESS;
    }
    else
    {
        return TRK_MGR_DEV_INTERNAL_ERROR;
    }
} /* End of TRK_MGR_AddPortIntoTrunk() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_DeletePortFromTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T TRK_MGR_DeletePortFromTrunk(UI32_T trunk_id,
                                          UI32_T unit,
                                          UI32_T port)
{
    SYS_TYPE_Uport_T unit_port;
    BOOL_T retval;
    BOOL_T is_static;

    if (trunk_id < 1 || trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_DeletePortFromTrunk_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if (!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_DeletePortFromTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }


    if (TRK_MGR_GetTrunkMemberCounts (trunk_id) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_DeletePortFromTrunk_FUN_NO, EH_TYPE_MSG_TRUNK_CNFG_ERR, SYSLOG_LEVEL_INFO, trk_mgr_cfg_no_trk_member);
        return FALSE;
    }

    unit_port.unit = (UI16_T)unit;
    unit_port.port = (UI16_T)port;

    retval = SWCTRL_DeleteTrunkMember(trunk_id, unit_port);

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    if (retval)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* End of TRK_MGR_DeletePortFromTrunk() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to get
 * OUTPUT  : name     -- the name of this trunk
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkName(UI32_T trunk_id, char *name)
{
    UI32_T      ifindex;
    Port_Info_T trunk_port_info;
    BOOL_T      is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkName_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {

        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK, TRK_MGR_GetTrunkName_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(name == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK, TRK_MGR_GetTrunkName_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (!SWCTRL_TrunkIDToLogicalPort (trunk_id, &ifindex))
    {
        return FALSE;
    }

    if (!SWCTRL_GetPortInfo (ifindex, &trunk_port_info))
    {
        return FALSE;
    }

    strcpy(name, (char *)trunk_port_info.port_name);

    return TRUE;
} /* End of TRK_MGR_GetTrunkName() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTrunkName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           name     -- the name of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_SetTrunkName(UI32_T trunk_id, char *name)
{
    UI32_T ifindex;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if ((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if (!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK, TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (name == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (strlen(name) > SYS_ADPT_MAX_TRUNK_NAME_LEN)
    {

        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_name);
        return FALSE;
    }

    if (!SWCTRL_TrunkIDToLogicalPort(trunk_id, &ifindex))
    {
        return FALSE;
    }

    SWCTRL_SetPortName(ifindex, (UI8_T *)name);
    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return TRUE;
} /* End of TRK_MGR_SetTrunkName() */
/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTrunkAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           alias     -- the alias of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_SetTrunkAlias(UI32_T trunk_id, char *alias)
{
    UI32_T ifindex;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if ((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if (!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK, TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (alias == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (strlen(alias) > SYS_ADPT_MAX_TRUNK_NAME_LEN)
    {

        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkName_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_name);
        return FALSE;
    }

    if (!SWCTRL_TrunkIDToLogicalPort(trunk_id, &ifindex))
    {
        return FALSE;
    }

    SWCTRL_SetPortAlias(ifindex, (UI8_T *)alias);
    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_IsDynamicTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the trunk is a dynamic trunk  or not
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Dynamic, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_IsDynamicTrunkId(UI32_T trunk_id)
{
    BOOL_T ret;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {

        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_IsDynamicTrunkId_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {

        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_IsDynamicTrunkId_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
     }

    ret=!TRK_MGR_IsStaticTrunk (trunk_id);
    return ret;
} /* End of TRK_MGR_IsDynamicTrunkId () */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkMemberCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk member numbers
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total trunk member number
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetTrunkMemberCounts(UI32_T trunk_id)
{
    UI32_T                          ifindex;
    SWCTRL_TrunkPortExtInfo_T    trunk_port_ext_info;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkMemberCounts_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkMemberCounts_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return 0;
    }

    if (!SWCTRL_TrunkIDToLogicalPort (trunk_id, &ifindex))
    {
        return FALSE;
    }

    if (!SWCTRL_GetTrunkPortExtInfo (ifindex, &trunk_port_ext_info))
    {
        return FALSE;
    }

    return trunk_port_ext_info.member_number;
} /* End of TRK_MGR_GetTrunkMemberCounts() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk numbers which are created
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total created trunk number
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetTrunkCounts(void)
{
    int i, count;
    BOOL_T is_static;

    count = 0;
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    for(i=1; i<=SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
        if(TRK_MGR_IsTrunkExist (i, &is_static))
            count++;

    return count;
} /* End of TRK_MGR_GetTrunkCounts() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next available trunk ID
 * INPUT   : trunk_id -- the key to get
 * OUTPUT  : trunk_id -- from 0 to SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetNextTrunkId(UI32_T *trunk_id)
{
    int i;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    if(trunk_id == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetNextTrunkId_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(*trunk_id >= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_GetNextTrunkId_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    for(i=(*trunk_id)+1; i<=SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
    {
        if(TRK_MGR_IsTrunkExist (i, &is_static))
        {
            *trunk_id = i;
            return TRUE;
        }
    }

    return FALSE;
} /* End of TRK_MGR_GetNextTrunkId() */



/*---------------------------------------------------------------------- */
/* (trunkMgt 1)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkMaxId
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the maximum number for a trunk identifier
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 1
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkMaxId(UI32_T *trunk_max_id)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    *trunk_max_id = SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM;
    return TRUE;
}


/*---------------------------------------------------------------------- */
/* (trunkMgt 2)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkValidNumber
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the number of valid trunks
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 2
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkValidNumber(UI32_T *trunk_valid_numer)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    *trunk_valid_numer = TRK_MGR_GetTrunkCounts();

    return TRUE;
}


/*---------------------------------------------------------------------- */
/* (trunkMgt 3)--ES3626A */
/*
 *      INDEX       { trunkIndex }
 *      TrunkEntry ::= SEQUENCE
 *      {
 *          trunkIndex                Integer32,
 *          trunkPorts                PortList,
 *          trunkCreation             INTEGER,
 *          trunkStatus               INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    UI32_T tm_num, i, tm_ifindex;
    UI32_T trunk_ifindex;
    SYS_TYPE_Uport_T unit_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    SWCTRL_TrunkPortExtInfo_T trunk_port_ext_info;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (trunk_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkEntry_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }
    if (!TRK_MGR_GetTrunkPorts(trunk_entry->trunk_index, &tm_num, unit_port))
    {
        return FALSE;
    }
    if (!SWCTRL_TrunkIDToLogicalPort(trunk_entry->trunk_index, &trunk_ifindex))
    {
        return FALSE;
    }
    if (!SWCTRL_GetTrunkPortExtInfo(trunk_ifindex, &trunk_port_ext_info))
    {
        return FALSE;
    }

    memset(trunk_entry->trunk_ports, 0, sizeof(trunk_entry->trunk_ports));

    for (i = 0; i < tm_num; i ++)
    {
        SWCTRL_UserPortToIfindex(unit_port[i].unit, unit_port[i].port, &tm_ifindex);
        (*trunk_entry).trunk_ports[(tm_ifindex - 1) / 8] |= ((0x01) << (7 - ((tm_ifindex - 1) % 8)));

    }

    memcpy(trunk_entry->active_trunk_ports, trunk_port_ext_info.oper_active_members, sizeof(trunk_entry->active_trunk_ports));

    if (TRK_MGR_IsStaticTrunk (trunk_entry->trunk_index))
        (*trunk_entry).trunk_creation = VAL_trunkCreation_static; /* static */
    else
        (*trunk_entry).trunk_creation = VAL_trunkCreation_lacp; /* lacp */

    /* always valid if the trunk is is exist */
    trunk_entry->trunk_status = VAL_trunkStatus_valid;

    if (!TRK_MGR_GetTrunkName(trunk_entry->trunk_index, trunk_entry->trunk_name))
    {
        return FALSE;
    }

    return TRUE;
} /* End of TRK_MGR_GetTrunkEntry () */


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetNextTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    UI32_T tm_num, i, tm_ifindex;
    UI32_T trunk_ifindex;
    SYS_TYPE_Uport_T unit_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    SWCTRL_TrunkPortExtInfo_T trunk_port_ext_info;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (trunk_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetNextTrunkEntry_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }
    if (!TRK_MGR_GetNextTrunkId(&((*trunk_entry).trunk_index)))
    {
        return FALSE;
    }
    if (!TRK_MGR_GetTrunkPorts((*trunk_entry).trunk_index, &tm_num, unit_port))
    {
        return FALSE;
    }
    if (!SWCTRL_TrunkIDToLogicalPort(trunk_entry->trunk_index, &trunk_ifindex))
    {
        return FALSE;
    }
    if (!SWCTRL_GetTrunkPortExtInfo(trunk_ifindex, &trunk_port_ext_info))
    {
        return FALSE;
    }

    memset (trunk_entry->trunk_ports, 0, sizeof(trunk_entry->trunk_ports));
    for(i=0; i<tm_num; i++)
    {
        SWCTRL_UserPortToIfindex(unit_port[i].unit, unit_port[i].port, &tm_ifindex);
        trunk_entry->trunk_ports[(tm_ifindex - 1) / 8] |= ((0x01) << (7 - ((tm_ifindex - 1) % 8)));
    }

    memcpy(trunk_entry->active_trunk_ports, trunk_port_ext_info.oper_active_members, sizeof(trunk_entry->active_trunk_ports));

    if (TRK_MGR_IsStaticTrunk (trunk_entry->trunk_index))
        trunk_entry->trunk_creation = VAL_trunkCreation_static; /* static */
    else
        trunk_entry->trunk_creation = VAL_trunkCreation_lacp; /* lacp */

    /* always valid if the trunk is is exist */
    trunk_entry->trunk_status = VAL_trunkStatus_valid;

    if (!TRK_MGR_GetTrunkName(trunk_entry->trunk_index, trunk_entry->trunk_name))
    {
        return FALSE;
    }

    return TRUE;
} /* End of TRK_MGR_GetNextTrunkEntry () */


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextRunningTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk entry of running config
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : trunk_id = 0 ==> get the first trunk (exclude dynamic trunk)
 *------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetNextRunningTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    UI32_T trunk_id = trunk_entry->trunk_index;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

	memset ( trunk_entry, 0, sizeof(TRK_MGR_TrunkEntry_T));
	trunk_entry->trunk_index = trunk_id;

    while (TRK_MGR_GetNextTrunkEntry(trunk_entry))
    {
        if( trunk_entry->trunk_creation == VAL_trunkCreation_lacp) /*dynamic*/
            continue;

		if(TRK_MGR_GetTrunkMemberCounts(trunk_entry->trunk_index) > 0)
			(*trunk_entry).trunk_ports_changed = TRUE;

        if( trunk_entry->trunk_status != VAL_trunkStatus_invalid)
            trunk_entry->trunk_status_changed = TRUE;

        if( trunk_entry->trunk_name[0] != '\0')
            trunk_entry->trunk_name_changed = TRUE;

        if( trunk_entry->trunk_ports_changed  ||
            trunk_entry->trunk_status_changed ||
            trunk_entry->trunk_name_changed   )
            {
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        else
            continue;
    }

    /* trunk not exist
     */

    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
} /* End of TRK_MGR_GetNextRunningTrunkEntry() */


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTrunkPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk port list
 * INPUT   : trunk_id                       - trunk id
 *           trunk_portlist                 - trunk port list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. ES3626A MIB/trunkMgt 3
 *           2. For trunk_portlist, only the bytes of in the range of user
 *              port will be handle.
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_SetTrunkPorts(UI32_T trunk_id, UI8_T *trunk_portlist)
{
    UI32_T exist_trunk_ifindex, tm_ifindex, unit, port, i, j, tm_num;
    SYS_TYPE_Uport_T unit_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkPorts_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }
    if (trunk_portlist == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkStatus_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    /* return FALSE if the trunk is dynamic trunk for LACP
     */
    if(!TRK_MGR_IsStaticTrunk (trunk_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkPorts_FUN_NO, EH_TYPE_MSG_TRUNK_CNFG_ERR, SYSLOG_LEVEL_INFO, trk_mgr_static_trk_msg_p);
        return FALSE ;
    }

    /* remove port from trunk
     */
    if (TRK_MGR_GetTrunkPorts(trunk_id, &tm_num, unit_port))
    {
        for(i=0; i<tm_num; i++)
        {
            SWCTRL_UserPortToIfindex(unit_port[i].unit, unit_port[i].port, &tm_ifindex);

            /* The current trunk member is the same as SNMP want to join
             */
            if (trunk_portlist[(tm_ifindex - 1) / 8] & ((0x01) << (7 - ((tm_ifindex - 1) % 8))))
            {
                continue;
            }

            if (!TRK_MGR_DeletePortFromTrunk(trunk_id, unit_port[i].unit, unit_port[i].port))
            {
                return FALSE;
            }
        }
    }

    /* add port into trunk
     */
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; i++)
    {
        if (trunk_portlist[i]==0)
        {
            continue;
        }

        for (j=0; j<8; j++)
        {
            if ((trunk_portlist[i]<<j) & 0x80)
            {
                tm_ifindex = 8*i+(j+1);

                /* continue if this port is already a trunk member
                 */
                if (TRUE == SWCTRL_GetTrunkIfIndexByUport( tm_ifindex, &exist_trunk_ifindex))
                {
                    if (exist_trunk_ifindex != trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1)
                    {
                        /* this is trunk member, but not in the trunk now SNMP want to join
                         */
                        return FALSE;
                    }
                    else
                    {
                        continue;
                    }
                }

                if (SWCTRL_LogicalPortToUserPort(tm_ifindex, &unit, &port, &exist_trunk_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    return FALSE;
                }

                if (TRK_MGR_AddPortIntoTrunk(trunk_id, unit, port, TRUE, TRUE) != TRK_MGR_SUCCESS)
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
} /* End of TRK_MGR_SetTrunkPorts() */


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTrunkStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk status
 * INPUT   : trunk_id                       - trunk id
 *           trunk_status                   - VAL_trunkStatus_valid
 *                                            VAL_trunkStatus_invalid
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_SetTrunkStatus(UI32_T trunk_id, UI8_T trunk_status)
{
    BOOL_T retval;
    BOOL_T is_static;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkStatus_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }
    if((trunk_status != VAL_trunkStatus_valid) && (trunk_status != VAL_trunkStatus_invalid))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkStatus_FUN_NO, EH_TYPE_MSG_INVALID_PARAMETER, SYSLOG_LEVEL_ERR, trk_mgr_invalid_status);
        return FALSE;
    }
    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_SetTrunkStatus_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    retval = SWCTRL_SetTrunkStatus(trunk_id, trunk_status);

    SYS_TIME_GetSystemUpTimeByTick(&last_change_time);

    return  retval;
} /* End of TRK_MGR_SetTrunkStatus() */


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_IsTrunkExist
 *------------------------------------------------------------------------
 * FUNCTION: Does this trunk exist or not.
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : is_static -- TRUE/FASLE
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_IsTrunkExist(UI32_T trunk_id, BOOL_T *is_static)
{
    UI32_T ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (!SWCTRL_TrunkIDToLogicalPort (trunk_id, &ifindex))
    {
        return FALSE;
    }

    if (FALSE == SWCTRL_LogicalPortExisting (ifindex))
    {
        return FALSE;
    }

    *is_static = TRK_MGR_IsStaticTrunk(trunk_id);

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetLastChangeTime()
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return 0;
    }

    return last_change_time;
} /* End of TRK_MGR_GetLastChangeTime() */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: TRK_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for TRK MGR.
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
BOOL_T TRK_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    TRK_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = TRK_MGR_OTHER_WRONG;
        msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding XSTP_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case TRK_MGR_IPC_CREATETRUNK:
        	msg_p->type.ret_bool =
        	    TRK_MGR_CreateTrunk(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_DESTROYTRUNK:
        	msg_p->type.ret_bool =
        	    TRK_MGR_DestroyTrunk(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_CREATEDYNAMICTRUNK:
        	msg_p->type.ret_bool =
        	    TRK_MGR_CreateDynamicTrunk(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_FREETRUNKIDDESTROYDYNAMIC:
        	msg_p->type.ret_bool =
        	    TRK_MGR_FreeTrunkIdDestroyDynamic(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_ADDTRUNKMEMBER:
        	msg_p->type.ret_ui32 = TRK_MGR_AddTrunkMember(
        	    msg_p->data.arg_grp_ui32_ui32.arg_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_2);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_ADDDYNAMICTRUNKMEMBER:
            msg_p->type.ret_ui32 = TRK_MGR_AddDynamicTrunkMember(
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_1,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_2,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_3);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
        	
            break;

        case TRK_MGR_IPC_DELETETRUNKMEMBER:
        	msg_p->type.ret_bool = TRK_MGR_DeleteTrunkMember(
        	    msg_p->data.arg_grp_ui32_ui32.arg_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_2);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_DELETEDYNAMICTRUNKMEMBER:
        	msg_p->type.ret_bool = TRK_MGR_DeleteDynamicTrunkMember(
        	    msg_p->data.arg_grp_ui32_ui32.arg_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_2);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_SETTRUNKNAME:
        	msg_p->type.ret_bool = TRK_MGR_SetTrunkName(
        	    msg_p->data.arg_grp_trunk_name.arg_id,
        	    msg_p->data.arg_grp_trunk_name.arg_name);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;
        case TRK_MGR_IPC_SETTRUNKALIAS:
        	msg_p->type.ret_bool = TRK_MGR_SetTrunkAlias(
        	    msg_p->data.arg_grp_trunk_name.arg_id,
        	    msg_p->data.arg_grp_trunk_name.arg_name);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_ISDYNAMICTRUNKID:
        	msg_p->type.ret_bool =
        	    TRK_MGR_IsDynamicTrunkId(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_GETTRUNKMEMBERCOUNTS:
        	msg_p->type.ret_ui32 =
        	    TRK_MGR_GetTrunkMemberCounts(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_GETTRUNKCOUNTS:
        	msg_p->type.ret_ui32 = TRK_MGR_GetTrunkCounts();
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_GETNEXTTRUNKID:
        	msg_p->type.ret_bool =
        	    TRK_MGR_GetNextTrunkId(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case TRK_MGR_IPC_GETTRUNKMAXID:
        	msg_p->type.ret_bool =
        	    TRK_MGR_GetTrunkMaxId(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case TRK_MGR_IPC_GETTRUNKVALIDNUMBER:
        	msg_p->type.ret_bool =
        	    TRK_MGR_GetTrunkValidNumber(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case TRK_MGR_IPC_GETTRUNKENTRY:
        	msg_p->type.ret_bool =
        	    TRK_MGR_GetTrunkEntry(&msg_p->data.arg_trunk_entry);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
            break;

        case TRK_MGR_IPC_GETNEXTTRUNKENTRY:
        	msg_p->type.ret_bool =
        	    TRK_MGR_GetNextTrunkEntry(&msg_p->data.arg_trunk_entry);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
            break;

        case TRK_MGR_IPC_GETNEXTRUNNINGTRUNKENTRY:
        	msg_p->type.ret_ui32 =
        	    TRK_MGR_GetNextRunningTrunkEntry(&msg_p->data.arg_trunk_entry);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
            break;

        case TRK_MGR_IPC_SETTRUNKPORTS:
        	msg_p->type.ret_bool = TRK_MGR_SetTrunkPorts(
        	    msg_p->data.arg_grp_trunk_portlist.arg_id,
        	    msg_p->data.arg_grp_trunk_portlist.arg_portlist);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_SETTRUNKSTATUS:
        	msg_p->type.ret_bool = TRK_MGR_SetTrunkStatus(
        	    msg_p->data.arg_grp_ui32_ui8.arg_ui32,
        	    msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        case TRK_MGR_IPC_ISTRUNKEXIST:
        	msg_p->type.ret_bool = TRK_MGR_IsTrunkExist(
        	    msg_p->data.arg_grp_ui32_bool.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;

        case TRK_MGR_IPC_GETLASTCHANGETIME:
        	msg_p->type.ret_ui32 = TRK_MGR_GetLastChangeTime();
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = TRK_MGR_OTHER_WRONG;
            msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of TRK_MGR_HandleIPCReqMsg */


/* Local Functions
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkPorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the member list of a specific trunk
 * INPUT   : trunk_id   -- which trunking port to get
 * OUTPUT  : port_count -- how many members
 *           unit_port  -- member list
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkPorts(UI32_T trunk_id,
                                    UI32_T *port_count,
                                    SYS_TYPE_Uport_T *unit_port)
{
    UI32_T                       ifindex, i;
    BOOL_T                       is_static;
    SWCTRL_TrunkPortExtInfo_T    trunk_port_ext_info;

    if((trunk_id < 1) || (trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkPorts_FUN_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, trk_mgr_out_of_range_trunk_id);
        return FALSE;
    }

    if(!(TRK_MGR_IsTrunkExist (trunk_id, &is_static)))
    {

        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkPorts_FUN_NO, EH_TYPE_MSG_TRUNK_NOT_PRESENT, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(port_count == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkPorts_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(unit_port == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_TRUNK,TRK_MGR_GetTrunkPorts_FUN_NO, EH_TYPE_MSG_NULL_POINTER,SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if (!SWCTRL_TrunkIDToLogicalPort (trunk_id, &ifindex))
        return FALSE;

    if (!SWCTRL_GetTrunkPortExtInfo (ifindex, &trunk_port_ext_info))
        return FALSE;

    *port_count = trunk_port_ext_info.member_number;
    for(i=0; i<*port_count; i++)
    {
        unit_port[i].unit = trunk_port_ext_info.member_list[i].unit;
        unit_port[i].port = trunk_port_ext_info.member_list[i].port;
    }
    return TRUE;
} /* End of TRK_MGR_GetTrunkPorts() */




static BOOL_T TRK_MGR_IsStaticTrunk(UI32_T trunk_id)
{
    UI32_T                          ifindex;
    SWCTRL_TrunkPortExtInfo_T    trunk_port_ext_info;

    if (!SWCTRL_TrunkIDToLogicalPort (trunk_id, &ifindex))
    {
        return FALSE;
    }

    if (!SWCTRL_GetTrunkPortExtInfo (ifindex, &trunk_port_ext_info))
    {
        return FALSE;
    }

    return trunk_port_ext_info.is_static;
}
