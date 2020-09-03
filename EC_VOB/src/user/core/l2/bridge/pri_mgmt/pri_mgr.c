/*---------------------------------------------------------------------------------
 * Module Name  : PRI_MGR.C
 *---------------------------------------------------------------------------------
 * Purpose: This package provides the sevices to manage the RFC2674 P-Bridge MIB.
 * Notes: 1. This package provides the services to comply the IEEE802.1D
 *           1998 standard, and BRIDGE MIB Extensions.
 *        2. The following table defined in RFC2674 P-Bridge MIB will not be supported
 *           in this package:
 *                   dot1dPortGmrpTable
 *        3. This package shall be a reusable package for all the L2/L3 switchs.
 *
 *
 *
 * Modification History:
 *
 *    Date          --  Modifier,       Reason
 * 06-13-2001           CPY             Create
 * 10-17-2001           AmyTu           Modify
 * 12-02-2002           Allen Cheng     Modify
 *---------------------------------------------------------------------------------
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002
 *---------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_2674p.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "pri_mgr.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_pmgr.h"
#include "syslog_type.h"
#endif
#include "swctrl.h"
#include "trk_mgr.h"


/* NAMING CONSTANT DECLARATION
 */

#define     PRI_MGR_NUM_OF_PRIORITY    MAX_dot1dTrafficClassPriority+1


/* LOCAL DATATYPE DECLARATION
 */
#define     PRI_MGR_FUNCTION_NUMBER     0
#define     PRI_MGR_ERROR_NUMBER        0

enum PRI_MGR_OPERATION_E
{
    PRI_MGR_USER_PRIORITY = 0,
    PRI_MGR_TRAFFIC_CLASS
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     PRI_MGR_InitDefault(void);
#if (SYS_CPNT_SYSLOG == TRUE)
static void     PRI_MGR_ReportSyslogMessage(UI32_T error_type, UI8_T error_msg, char *function_name);
#endif
static BOOL_T   PRI_MGR_CompareDefaultTrafficClass(UI32_T , UI32_T, UI32_T *running_cfg);
static BOOL_T   PRI_MGR_SetTrunkMemberInfo(UI32_T trunk_id, UI32_T type, UI32_T priority, UI32_T traffic_class);
static void     PRI_MGR_Notify_CosChanged(UI32_T lport_ifindex);

static BOOL_T   PRI_MGR_AddFirstTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static BOOL_T   PRI_MGR_AddTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static BOOL_T   PRI_MGR_DeleteTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static BOOL_T   PRI_MGR_DeleteLastTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);

static BOOL_T   PRI_MGR_InitPortPriorityTable(UI32_T lport_num, PRI_MGR_Port_Info_T *port_info, UI8_T *mapping_table);


/* STATIC VARIABLE DECLARATIONS
 */
static PRI_MGR_Port_Info_T     port_table[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static UI32_T   number_of_system_port;
/* Allen Cheng: Deleted
static UI32_T   pri_mgr_operation_mode;
*/
//Timon static SYS_TYPE_CallBack_T *cos_changed_list;
SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize priority manager table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_InitiateSystemResources(void)
{
    memset(port_table, 0, sizeof(PRI_MGR_Port_Info_T) * SYS_ADPT_TOTAL_NBR_OF_LPORT);

/* Allen Cheng: Deleted
    pri_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/

    return;
} /* end of PRI_MGR_InitiateSystemResources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_Create_InterCSC_Relation(void)
{
/*
    SWCTRL_Register_TrunkMemberAdd1st_CallBack( &PRI_MGR_AddFirstTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberAdd_CallBack( &PRI_MGR_AddTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberDelete_CallBack( &PRI_MGR_DeleteTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberDeleteLst_CallBack( &PRI_MGR_DeleteLastTrunkMember_CallBack );
*/
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  PRI_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

} /* end of PRI_MGR_SetTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the priority mapping operation while in transition
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
/* Allen Cheng: Deleted
    pri_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/
    memset(port_table, 0, sizeof(PRI_MGR_Port_Info_T) * SYS_ADPT_TOTAL_NBR_OF_LPORT);
    return;
} /* end of PRI_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the priority mapping operation while in master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_EnterMasterMode(void)
{

    /* Get the current number of port availabe on the system.
     */
/* Allen Cheng: Deleted
    pri_mgr_operation_mode = SYS_TYPE_STACKING_MASTER_MODE;
*/
    number_of_system_port = SWCTRL_GetSystemPortNumber();
    PRI_MGR_InitDefault();
    SYSFUN_ENTER_MASTER_MODE();

    return;
} /* end of PRI_MGR_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the priority mapping operation while in slave
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
/* Allen Cheng: Deleted
    pri_mgr_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
*/
    return;

} /* end of PRI_MGR_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T  PRI_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();

} /* end of PRI_MGR_GetOperationMode() */


/* Dot1dPortPriorityEntry
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dPortDefaultUserPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default user priority of the
 *            specific port can be set.  Otherwise, return false.
 * INPUT    : lport_ifindex -- the specified port number
 *            default_priority -- the default priority for the port
 * OUTPUT   : none
 * RETURN   : TRUE /FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *            3. "MIN_dot1dUserPriority = 0L", "MAX_dot1dUserPriority" = 7L
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dPortDefaultUserPriority(UI32_T lport_ifindex, UI32_T default_priority)
{
    UI32_T                  unit, port, trunk_id;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        return FALSE;
    } /* End of if */

    if (default_priority < MIN_dot1dUserPriority || default_priority > MAX_dot1dUserPriority)
    {
        return FALSE;
    } /* End of if */

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    if (TRUE == port_table[lport_ifindex].port_priority_entry.dot1d_port_dft_pri_is_dynamic_cfg)
    {
        port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority_user_cfg = default_priority;
    }
    else
#endif
    {
        port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority = default_priority;

        if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        {
            PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_USER_PRIORITY, default_priority, 0);
        } /* end of if */
        else
        {
            if (!SWCTRL_SetPortUserDefaultPriority(lport_ifindex, default_priority))
            {
#if (SYS_CPNT_SYSLOG == TRUE)
                PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dPortDefaultUserPriority");
#endif
                return FALSE;
            }
        }
    }

    return TRUE;
} /* end of PRI_MGR_SetDot1dPortDefaultUserPriority()*/

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDynamicDot1dPortDefaultUserPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function should set the default user priority on the
 *            specified port for dynamic QoS assignment.
 * INPUT    : lport_ifindex     -- the specified port number
 *            default_priority  -- the default priority for the port. If this
                                   value is 255, retore the default user priority
                                   to manual configured.
 * OUTPUT   : none
 * RETURN   : TRUE /FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *            3. The port changes by this function should not be save to configuration
 *               file.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDynamicDot1dPortDefaultUserPriority(UI32_T lport_ifindex, UI32_T default_priority)
{
    enum {RESTORE_TO_MANUAL_CONFIGURE = 255};

    UI32_T                  unit, port, trunk_id;
    BOOL_T                  is_changed = FALSE;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        return FALSE;
    } /* End of if */

    if (RESTORE_TO_MANUAL_CONFIGURE != default_priority)
    {
        if (default_priority < MIN_dot1dUserPriority || default_priority > MAX_dot1dUserPriority)
        {
            return FALSE;
        } /* End of if */
    }

    if (RESTORE_TO_MANUAL_CONFIGURE != default_priority)
    {
        if (FALSE == port_table[lport_ifindex].port_priority_entry.dot1d_port_dft_pri_is_dynamic_cfg)
        {
            port_table[lport_ifindex].port_priority_entry.dot1d_port_dft_pri_is_dynamic_cfg = TRUE;
            port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority_user_cfg =
                port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority;
        }

        port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority = default_priority;
        is_changed = TRUE;
    }
    else
    {
        if (TRUE == port_table[lport_ifindex].port_priority_entry.dot1d_port_dft_pri_is_dynamic_cfg)
        {
            port_table[lport_ifindex].port_priority_entry.dot1d_port_dft_pri_is_dynamic_cfg = FALSE;
            port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority =
                port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority_user_cfg;

            /* Get the real value
             */
            default_priority = port_table[lport_ifindex].port_priority_entry.dot1d_port_default_user_priority;
            is_changed = TRUE;
        }
    }

    if (TRUE == is_changed)
    {
        if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        {
            PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_USER_PRIORITY, default_priority, 0);
        } /* end of if */
        else
        {
            if (!SWCTRL_SetPortUserDefaultPriority(lport_ifindex, default_priority))
            {
                PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dPortDefaultUserPriority");
                return FALSE;
            }
        }
    }
    return TRUE;
}
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dPortNumTrafficClasses
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the number of traffic class
 *            can be set.  Otherwise, return false.
 * INPUT    : lport_ifindex -- the specified port number
 *            number_of_traffic_classes -- the number of traffic classes
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The port_id is the key to identify the number of traffic
 *               classes of a given port.
 *            2. The number_of_traffic_classes defined the number of egress
 *               traffic classes supported on this port.
 *            3. This field is READ-ONLY for current version of Mercury.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dPortNumTrafficClasses(UI32_T lport_ifindex, UI32_T number_of_traffic_classes)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    /* NOT SUPPORTED */
    return FALSE;
}/* end of PRI_MGR_SetDot1dPortNumTrafficClasses()*/


/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetDot1dPortPriorityEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the port priority entry can be
 *            retrieve from database.  Otherwise, return false.
 * INPUT    : lport_ifindex      - Key to identify specific entry
 * OUTPUT   : priority_entry    - the priority entry which contains information
 *                                of specific port.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority entry.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetDot1dPortPriorityEntry(UI32_T lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry)
{
    PRI_MGR_Port_Info_T     port_info;
    UI32_T                  unit, port, trunk_id;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        return FALSE;
    } /* End of if */

    memcpy(&port_info, &port_table[lport_ifindex], sizeof(PRI_MGR_Port_Info_T));

	memcpy(priority_entry, &port_info.port_priority_entry, sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));

    return TRUE;
} /* end of PRI_MGR_GetDot1dPortPriorityEntry () */



/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetNextDot1dPortPriorityEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available port priority
 *            can be retrieve from database.  Otherwise, return false.
 * INPUT    : lport_ifindex      - Key to identify specific entry
 * OUTPUT   : lport_ifindex      - Key to identify the next available entry
 *            priority_entry    - the priority entry which contains information
 *                                of specific port.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority entry.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetNextDot1dPortPriorityEntry(UI32_T *lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        return FALSE;

    return PRI_MGR_GetDot1dPortPriorityEntry(*lport_ifindex, priority_entry);

} /* end of PRI_MGR_GetNextDot1dPortPriorityEntry() */



/* Dot1dTrafficClassEntry
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dTrafficClass
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the mapped traffic class of the
 *            specific priority be set.  Otherwise, return false.
 * INPUT    : lport_ifindex -- PER PORT CONFIGURATION IS CURRENTLY NOT SUPPORTED.
 *            user_priority -- the user priority
 *            traffic_class -- the mapped traffic classs
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the vaule will be determined by the port
 *               default user priority.
 *            2. The traffic_class will the traffic class mapped to the user
 *               priority of received frame.
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dTrafficClass(UI32_T lport_ifindex, UI32_T priority, UI32_T traffic_class)
{
    SWCTRL_Lport_Type_T     port_type;
	UI32_T      unit, port, trunk_id;
	UI8_T		mapped_traffic_class[8];
	UI32_T		index;
    UI32_T      port_num;   /* This variable is necessary for per device setting.   */

    /* BODY */

#if (SYS_CPNT_1P_USER_CONFIGUREABLE != TRUE)
    return FALSE;
#endif
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (priority < MIN_dot1dTrafficClassPriority || priority > MAX_dot1dTrafficClassPriority)
    {
        return FALSE;
    } /* End of if */

    if (traffic_class >= SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE)
    {
        return FALSE;
    } /* End of if */

    if (port_table[lport_ifindex].traffic_class_entry[priority].dot1d_traffic_class == traffic_class)
    {
        return TRUE;
    }

    /* 1. The logic behind PER PORT setting is to apply user-specify traffic class to the specific
          lport_ifindex and apply set operation to SWCTRL individually.
       2. The logic behind PER SYSTEM setting is to apply user-specify traffic class to all logical
          ports on the device, with only one set operation to SWCTRL.
     */

#if (SYS_CPNT_PRI_MGMT_PER_PORT == TRUE) /* Per Port setting */
    port_table[lport_ifindex].traffic_class_entry[priority].dot1d_traffic_class = traffic_class;

    port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id);

    if (port_type != SWCTRL_LPORT_TRUNK_PORT)
    {
        for (index = MIN_dot1dTrafficClassPriority; index < MAX_dot1dTrafficClassPriority+1; index++)
            mapped_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[index].dot1d_traffic_class;

        if (!SWCTRL_SetPriorityMapping(lport_ifindex, mapped_traffic_class))
        {
#if (SYS_CPNT_SYSLOG == TRUE)
            PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
            return FALSE;
        }
    }

    if (port_type == SWCTRL_LPORT_TRUNK_PORT)
    {
        PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, priority, traffic_class);
    }


#else
    port_num = 0;

    while ((port_type = SWCTRL_GetNextLogicalPort(&port_num)) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        port_table[port_num].traffic_class_entry[priority].dot1d_traffic_class = traffic_class;

        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            SWCTRL_LogicalPortToUserPort(port_num, &unit, &port, &trunk_id);
            PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, priority, traffic_class);
        }

    } /* end of while */

    for (index = MIN_dot1dTrafficClassPriority; index < MAX_dot1dTrafficClassPriority+1; index++)
        mapped_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[index].dot1d_traffic_class;


    if (!SWCTRL_SetPriorityMappingPerSystem(mapped_traffic_class))
    {
#if (SYS_CPNT_SYSLOG == TRUE)
        PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
        return FALSE;
    }
#endif

    PRI_MGR_Notify_CosChanged(lport_ifindex);
    return TRUE;
} /* end of PRI_MGR_SetDot1dTrafficClass()*/


/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the mapped traffic class of the
 *            priority list.  Otherwise, return false.
 * INPUT    : lport_ifindex -- PER PORT CONFIGURATION IS CURRENTLY NOT SUPPORTED.
 *            traffic_class -- the mapped traffic classs
 *            priority      -- user priority bit list
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the vaule will be determined by the port
 *               default user priority.
 *            2. The traffic_class will the traffic class mapped to the user
 *               priority of received frame.
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *            4.This API is for CLI Only
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dPriority(UI32_T lport_ifindex, UI32_T traffic_class, UI8_T priority)
{
    SWCTRL_Lport_Type_T port_type;
    UI32_T              unit, port, trunk_id;
    UI8_T               mapped_traffic_class[8];
    UI8_T               check_list[MAX_dot1dTrafficClassPriority+1];
    UI32_T              index;
    UI32_T              port_num = 0;   /* This variable is necessary for per device setting.   */
    UI8_T               i = 0;
    BOOL_T              is_changed = FALSE;
    /* BODY */

#if (SYS_CPNT_1P_USER_CONFIGUREABLE != TRUE)
    return FALSE;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (traffic_class >= SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE)
    {
        return FALSE;
    } /* End of if */

    memset(check_list, 0, sizeof(UI8_T)*(MAX_dot1dTrafficClassPriority+1));
    for (i = 0; i<= MAX_dot1dTrafficClassPriority; i++)
    {
        if (port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class == traffic_class)
        {
            check_list[i] |= 0x1;/*old*/
        }

        if (priority & (1 << i))
        {
            check_list[i] |= 0x2;/*new*/
        }
    }/*priority for loop*/

    /* 1. The logic behind PER PORT setting is to apply user-specify traffic class to the specific
          lport_ifindex and apply set operation to SWCTRL individually.
       2. The logic behind PER SYSTEM setting is to apply user-specify traffic class to all logical
          ports on the device, with only one set operation to SWCTRL.
     */
    for (i = 0; i<= MAX_dot1dTrafficClassPriority; i++)
    {
#if (SYS_CPNT_PRI_MGMT_PER_PORT == TRUE) /* Per Port setting */
        if (check_list[i] == 1)/*set this priority to default queue*/
        {
            is_changed = TRUE;
            switch(i)
            {
            case 0:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS;
                break;
            case 1:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS;
                break;
            case 2:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS;
                break;
            case 3:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS;
                break;
            case 4:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS;
                break;
            case 5:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS;
                break;
            case 6:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS;
                break;
            case 7:
                port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS;
                break;
            default:
                break;
            }
        }
        if (check_list[i] == 2)/*change this priority to new queue*/
        {
            is_changed = TRUE;
            port_table[lport_ifindex].traffic_class_entry[i].dot1d_traffic_class = traffic_class;
        }
    }

    if (TRUE == is_changed)
    {
        port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id);

        if (port_type != SWCTRL_LPORT_TRUNK_PORT)
        {
            for (index = MIN_dot1dTrafficClassPriority; index < MAX_dot1dTrafficClassPriority+1; index++)
                mapped_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[index].dot1d_traffic_class;

            if (!SWCTRL_SetPriorityMapping(lport_ifindex, mapped_traffic_class))
            {
#if (SYS_CPNT_SYSLOG == TRUE)
                PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
                return FALSE;
            }
        }
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, priority, traffic_class);
        }
    }
#else
       port_num = 0;

       while ((port_type = SWCTRL_GetNextLogicalPort(&port_num)) != SWCTRL_LPORT_UNKNOWN_PORT)
       {
           for (i = 0; i<= MAX_dot1dTrafficClassPriority; i++)
           {
               if (check_list[i] == 1)/*set this priority to default queue*/
               {
                   is_changed = TRUE;
                   switch(i)
                   {
                   case 0:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 1:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 2:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 3:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 4:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 5:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 6:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS;
                       break;
                   case 7:
                       port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS;
                       break;
                   default:
                       break;
                   }

                   if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                   {
                       SWCTRL_LogicalPortToUserPort(port_num, &unit, &port, &trunk_id);
                       PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, i, port_table[port_num].traffic_class_entry[i].dot1d_traffic_class);
                   }
               }
               if (check_list[i] == 2)/*change this priority to new queue*/
               {
                   is_changed = TRUE;
                   port_table[port_num].traffic_class_entry[i].dot1d_traffic_class = traffic_class;

                   if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                   {
                       SWCTRL_LogicalPortToUserPort(port_num, &unit, &port, &trunk_id);
                       PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, i, port_table[port_num].traffic_class_entry[i].dot1d_traffic_class);
                   }
               }
           }/*end of for*/
       } /* end of while */

       if (TRUE == is_changed)
       {
           for (index = MIN_dot1dTrafficClassPriority; index < MAX_dot1dTrafficClassPriority+1; index++)
               mapped_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[index].dot1d_traffic_class;


           if (!SWCTRL_SetPriorityMappingPerSystem(mapped_traffic_class))
           {
#if (SYS_CPNT_SYSLOG == TRUE)
               PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
               return FALSE;
           }
       }
    }
#endif

    PRI_MGR_Notify_CosChanged(lport_ifindex);
    return TRUE;
} /* end of PRI_MGR_SetDot1dTrafficClass()*/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetDot1dTrafficClassEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the traffic class of the specific user
 *            priority and lport_ifindex can be retrieve successfully.  Otherwise,
 *            returns false.
 * INPUT    : traffic_class_entry.lport_ifindex  - the primary key to identify a port traffic class
 *            traffic_class_entry.dot1d_traffic_class_priority - the secondary key to identify a port traffic class
 * OUTPUT   : *traffic_class - the mapped traffic classs
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the value will be determined by the port
 *               default user priority.
 *            2. The traffic_class will be the traffic class mapped to the user
 *               priority of received frame.  Traffic class is a number in the
 *               range (0..(dot1dPortNumTrafficClasses-1)).
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetDot1dTrafficClassEntry(UI32_T lport_ifindex, UI32_T priority, PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry)
{
    PRI_MGR_Port_Info_T     port_info;
    UI32_T                  unit, port, trunk_id;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        return FALSE;
    } /* End of if */

    memcpy(&port_info, &port_table[lport_ifindex], sizeof(PRI_MGR_Port_Info_T));

    memcpy(traffic_class_entry, &port_info.traffic_class_entry[priority], sizeof(PRI_MGR_Dot1dTrafficClassEntry_T));

    return TRUE;
} /* end of PRI_MGR_GetDot1dTrafficClassEntry() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetNextDot1dTrafficClassEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available traffic class of
 *            the specific user priority and lport_ifindex can be retrieve
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex  - the primary key to identify a port traffic class
 *            priority - the secondary key to identify a port traffic class
 * OUTPUT   : lport_ifindex  - the primary key to identify a port traffic class
 *            priority - the secondary key to identify a port traffic class
 *            *traffic_class_entry - the specific traffic class
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the value will be determined by the port
 *               default user priority.
 *            2. The traffic_class will be the traffic class mapped to the user
 *               priority of received frame.  Traffic class is a number in the
 *               range (0..(dot1dPortNumTrafficClasses-1)).
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetNextDot1dTrafficClassEntry(UI32_T *lport_ifindex, UI32_T *priority, PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry)
{
    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            return FALSE;
        *priority = MIN_dot1dTrafficClassPriority;
    }
    else
    {
        if (*priority < MAX_dot1dTrafficClassPriority)
            *priority = *priority + 1;
        else
        {
            if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
                return FALSE;
            *priority = MIN_dot1dTrafficClassPriority;
        }
    }

    return PRI_MGR_GetDot1dTrafficClassEntry(*lport_ifindex, *priority, traffic_class_entry);


} /* end of PRI_MGR_GetNextDot1dTrafficClassEntry() */



/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the user regenrate priority of the
 *            specific user priority and lport_ifindex can be retrieve
 *            successfully. Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 *            user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                   be retrieve.
 * OUTPUT   : user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                         priority for the
 *                                                         designated port and priority.
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetDot1dUserPriorityRegenEntry(UI32_T lport_ifindex,
                                              PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry)
{
    PRI_MGR_Port_Info_T     port_info;
    UI32_T                  unit, port, trunk_id;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        return FALSE;
    } /* End of if */
    if (user_regen_entry->dot1d_user_priority > MAX_dot1dUserPriority)
    {
        return FALSE;
    }


    memcpy(&port_info, &port_table[lport_ifindex], sizeof(PRI_MGR_Port_Info_T));
    memcpy(user_regen_entry, &port_info.user_regen_entry[user_regen_entry->dot1d_user_priority], sizeof(PRI_MGR_Dot1dUserPriorityRegenEntry_T));
    
    return TRUE;

} /* end of PRI_MGR_GetDot1dUserPriorityRegenEntry() */



/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetNextDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next available user regenrate priority
 *            of the specific user priority and lport_ifindex can be retrieve
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 * OUTPUT   : user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                  be retrieve.
 *            user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                        priority for the
 *                                                        designated port and priority.
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_GetNextDot1dUserPriorityRegenEntry(UI32_T *lport_ifindex,
                                                  PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry)
{
    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;


    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            return FALSE;
        user_regen_entry->dot1d_user_priority = MIN_dot1dUserPriority;
    }
    else
    {
        if (user_regen_entry->dot1d_user_priority < MAX_dot1dUserPriority)
            user_regen_entry->dot1d_user_priority = user_regen_entry->dot1d_user_priority + 1;
        else
        {
            if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
                return FALSE;
            user_regen_entry->dot1d_user_priority = MIN_dot1dUserPriority;
        }
    }
    return PRI_MGR_GetDot1dUserPriorityRegenEntry(*lport_ifindex, user_regen_entry);


} /* end of PRI_MGR_GetNextDot1dUserPriorityRegenEntry() */



/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific user regenrated priority
 *            of the specific user priority and lport_ifindex can be set
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 *            user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                  be retrieve.
 *            user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                        priority for the
 *                                                        designated port and priority.
 * OUTPUT   : NONE
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dUserPriorityRegenEntry(UI32_T lport_ifindex,
                                              PRI_MGR_Dot1dUserPriorityRegenEntry_T user_regen_entry)
{
    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  unit, port, trunk_id;
    UI8_T	                  mapped_regen_traffic_class[8];
    UI32_T		              index;
    UI32_T                  port_num;   /* This variable is necessary for per device setting.   */

    /* BODY */
                                                 
#if (SYS_CPNT_1P_USER_CONFIGUREABLE != TRUE)
    return FALSE;
#endif
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (user_regen_entry.dot1d_user_priority < MIN_dot1dUserPriority || user_regen_entry.dot1d_user_priority > MAX_dot1dUserPriority)
    {
        return FALSE;
    } /* End of if */

    if (user_regen_entry.dot1d_regen_user_priority > MAX_dot1dUserPriority)
    {
        return FALSE;
    } /* End of if */

    if (port_table[lport_ifindex].user_regen_entry[user_regen_entry.dot1d_user_priority].dot1d_regen_user_priority == user_regen_entry.dot1d_regen_user_priority)
    {
        return TRUE;
    }

#if (SYS_CPNT_PRI_MGMT_PER_PORT == TRUE) /* Per Port setting */
    port_table[lport_ifindex].user_regen_entry[user_regen_entry.dot1d_user_priority].dot1d_regen_user_priority = user_regen_entry.dot1d_regen_user_priority;

    port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id);

    if (port_type != SWCTRL_LPORT_TRUNK_PORT)
    {
        /*final_queue = regen 1.p to original 802.1p to queue  */
        for (index = MIN_dot1dUserPriority; index < MAX_dot1dUserPriority+1; index++)
            mapped_regen_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[port_table[lport_ifindex].user_regen_entry[index].dot1d_regen_user_priority].dot1d_traffic_class;

        if (!SWCTRL_SetPriorityMapping(lport_ifindex, mapped_regen_traffic_class))
        {
#if (SYS_CPNT_SYSLOG == TRUE)
            PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
            return FALSE;
        }
    }

    if (port_type == SWCTRL_LPORT_TRUNK_PORT)
    {
        PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, user_regen_entry.dot1d_user_priority, port_table[lport_ifindex].traffic_class_entry[user_regen_entry.dot1d_user_priority].dot1d_traffic_class);
    }   

#else
    port_num = 0;

    while ((port_type = SWCTRL_GetNextLogicalPort(&port_num)) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        port_table[port_num].user_regen_entry[user_regen_entry.dot1d_user_priority].dot1d_regen_user_priority = user_regen_entry.dot1d_regen_user_priority;
        
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            SWCTRL_LogicalPortToUserPort(port_num, &unit, &port, &trunk_id);
            PRI_MGR_SetTrunkMemberInfo(trunk_id, PRI_MGR_TRAFFIC_CLASS, user_regen_entry.dot1d_user_priority, port_table[lport_ifindex].traffic_class_entry[user_regen_entry.dot1d_user_priority].dot1d_traffic_class);
        }
    } /* end of while */

    /*final_queue = regen 1.p to original 802.1p to queue  */
    for (index = MIN_dot1dUserPriority; index < MAX_dot1dUserPriority+1; index++)
        mapped_regen_traffic_class[index] =(UI8_T) port_table[lport_ifindex].traffic_class_entry[port_table[lport_ifindex].user_regen_entry[index].dot1d_regen_user_priority].dot1d_traffic_class;

    if (!SWCTRL_SetPriorityMappingPerSystem(mapped_regen_traffic_class))
    {
#if (SYS_CPNT_SYSLOG == TRUE)
        PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
        return FALSE;
    }
#endif
    PRI_MGR_Notify_CosChanged(lport_ifindex);
    return TRUE;

} /* end of PRI_MGR_SetDot1dUserPriorityRegenEntry() */




/* ---------------------------------------------------------------------
 * FUNCTION NAME  - PRI_MGR_GetRunningPortPriorityParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific non-default user priority associated with lport_ifindex
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - the specific port number
 * OUTPUT: user_priority - the user priority of the specific port
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user priority value
 * ----------------------------------------------------------------------------------*/
UI32_T PRI_MGR_GetRunningPortPriorityParameters(UI32_T lport_ifindex, PRI_MGR_PortPriority_RunningCfg_T *port_cfg)
{
    PRI_MGR_Dot1dPortPriorityEntry_T        priority_entry;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

    if (!PRI_MGR_GetDot1dPortPriorityEntry(lport_ifindex, &priority_entry))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

    memset(port_cfg, 0, sizeof(*port_cfg));

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    if (TRUE == priority_entry.dot1d_port_dft_pri_is_dynamic_cfg)
    {
        priority_entry.dot1d_port_default_user_priority = priority_entry.dot1d_port_default_user_priority_user_cfg;
    }
#endif

    if (priority_entry.dot1d_port_default_user_priority != MIN_dot1dPortDefaultUserPriority)
    {
        port_cfg->default_user_priority = priority_entry.dot1d_port_default_user_priority;
        port_cfg->user_priority_changed = TRUE;
    } /* end of if */

    if (priority_entry.dot1d_port_num_traffic_classes != SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE)
    {
        port_cfg->port_num_traffic_class = priority_entry.dot1d_port_num_traffic_classes;
        port_cfg->traffic_class_changed = TRUE;
    } /* end of if */

    if (port_cfg->user_priority_changed || port_cfg->traffic_class_changed)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
} /* end of PRI_MGR_GetRunningPortPriorityParameters() */


/* ---------------------------------------------------------------------
 * FUNCTION NAME  - PRI_MGR_GetNextRunningTrafficClassParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available traffic class associated with lport_ifindex
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - the specific port number
 *         traffic_class_cfg->dot1d_traffic_class_priority - specific prioriry mapped to
 *                                                           traffic class
 * OUTPUT: traffic_class_cfg  - structure which contains non-defulat traffic class value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user priority value
 * ----------------------------------------------------------------------------------*/
UI32_T PRI_MGR_GetNextRunningTrafficClassParameters(PRI_MGR_TrafficClass_RunningCfg_T *traffic_class_cfg)
{
    PRI_MGR_Dot1dTrafficClassEntry_T    traffic_class_entry;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

    if (traffic_class_cfg->lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        if (SWCTRL_GetNextLogicalPort(&traffic_class_cfg->lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
        traffic_class_cfg->dot1d_traffic_class_priority = MIN_dot1dTrafficClassPriority;
    }
    else
    {
        if (traffic_class_cfg->dot1d_traffic_class_priority < MAX_dot1dTrafficClassPriority)
            traffic_class_cfg->dot1d_traffic_class_priority = traffic_class_cfg->dot1d_traffic_class_priority + 1;
        else
        {
            if (SWCTRL_GetNextLogicalPort(&traffic_class_cfg->lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            {
                return SYS_TYPE_GET_RUNNING_CFG_FAIL;
            }
            traffic_class_cfg->dot1d_traffic_class_priority = MIN_dot1dTrafficClassPriority;
        }
    }

    if (!PRI_MGR_GetDot1dTrafficClassEntry(traffic_class_cfg->lport_ifindex,
                                           traffic_class_cfg->dot1d_traffic_class_priority,
                                           &traffic_class_entry))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */


    if (traffic_class_cfg->dot1d_traffic_class_priority == 0)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->dot1d_traffic_class_priority == 1)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->dot1d_traffic_class_priority == 2)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->dot1d_traffic_class_priority == 3)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->dot1d_traffic_class_priority == 4)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->dot1d_traffic_class_priority == 5)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */
    if (traffic_class_cfg->dot1d_traffic_class_priority == 6)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */
    if (traffic_class_cfg->dot1d_traffic_class_priority == 7)
    {
        traffic_class_cfg->traffic_class_changed = PRI_MGR_CompareDefaultTrafficClass(traffic_class_entry.dot1d_traffic_class,
                                                                                      SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS,
                                                                                      &traffic_class_cfg->dot1d_traffic_class);
    } /* end of if */

    if (traffic_class_cfg->traffic_class_changed)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

} /* end of PRI_MGR_GetRunningTrafficClassParameters() */

#if 0
/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_RegisterCosChanged_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : This is a register function which will signal all functions registered
 *            with PRI_MGR in the case of COS mapping changed, either PER SYSTEM
 *            or per PORT.
 * INPUT    : void (*fun()) -- CallBack function pointer.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : lport_ifindex indicates the specific port changed in the case of
 *            per port setting.
 *------------------------------------------------------------------------------*/
void PRI_MGR_RegisterCosChanged_CallBack(void (*fun)(UI32_T lport_ifindex))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(cos_changed_list);
	return;
} /* end of PRI_MGR_RegisterCosChanged_CallBack() */
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
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
 * ------------------------------------------------------------------------
 */
void PRI_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T                      ending_port_ifindex=0;
    UI8_T                       mapping_table[8];
    PRI_MGR_Port_Info_T         port_info;
    UI32_T                      next_lport;

    /* BODY */
    ending_port_ifindex = starting_port_ifindex + number_of_port - 1;
    for(next_lport = starting_port_ifindex; next_lport <= ending_port_ifindex; next_lport++)
    {
        memset(&port_info, 0, sizeof(PRI_MGR_Port_Info_T));
        memset(&mapping_table, 0, sizeof(mapping_table));
        if (PRI_MGR_InitPortPriorityTable(next_lport, &port_info, mapping_table))
        {
            memcpy(&port_table[next_lport], &port_info, sizeof(PRI_MGR_Port_Info_T));

            /* Initialize default value to hardware
             */
            if (   (!SWCTRL_SetPortUserDefaultPriority(next_lport, MIN_dot1dPortDefaultUserPriority))
                || (!SWCTRL_SetPriorityMapping(next_lport, mapping_table))
               )
            {
#if (SYS_CPNT_SYSLOG == TRUE)
                PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_HandleHotInsertion");
#endif
                return;
            }
        }
    }

    return;
}/* End of PRI_MGR_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void PRI_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T                      ending_port_ifindex=0;
    UI8_T                       mapping_table[8];
    PRI_MGR_Port_Info_T         port_info;
    UI32_T                      next_lport;

    /* BODY */
    ending_port_ifindex = starting_port_ifindex + number_of_port - 1;
    for(next_lport = starting_port_ifindex; next_lport <= ending_port_ifindex; next_lport++)
    {
        memset(&port_info, 0, sizeof(PRI_MGR_Port_Info_T));
        if (PRI_MGR_InitPortPriorityTable(next_lport, &port_info, mapping_table))
        {
            memcpy(&port_table[next_lport], &port_info, sizeof(PRI_MGR_Port_Info_T));
        }
    }

    return;
}/* End of PRI_MGR_HandleHotRemoval */

/* ===================================================================== */
/* Trunk callback function */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_AddFirstTrunkMember_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 1st member
 * INPUT    : trunk_ifindex   -- specify which trunk to join.
 *            member_ifindex  -- specify which member port being add to trunk.
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void PRI_MGR_AddFirstTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (!PRI_MGR_AddFirstTrunkMember(trunk_ifindex, member_ifindex) )
    {
        SYSFUN_Debug_Printf("\r\n %s: failed return from"
                            " PRI_MGR_AddFirstTrunkMember \r\n", __FUNCTION__);
    }

    return;
} /* End of PRI_MGR_AddFirstTrunkMember_CallBack */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_AddTrunkMember_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the 2nd or the following
 *            trunk member is removed from the trunk
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void PRI_MGR_AddTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (!PRI_MGR_AddTrunkMember(trunk_ifindex, member_ifindex) )
    {
        SYSFUN_Debug_Printf("\r\n %s: failed return from"
                            " PRI_MGR_AddTrunkMember \r\n", __FUNCTION__);
    }

    return;
} /* End of PRI_MGR_AddTrunkMember_CallBack */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_DeleteTrunkMember_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the 2nd or the following
 *            trunk member is removed from the trunk
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void PRI_MGR_DeleteTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (!PRI_MGR_DeleteTrunkMember(trunk_ifindex, member_ifindex) )
    {
        SYSFUN_Debug_Printf("\r\n %s: failed return from"
                            " PRI_MGR_DeleteTrunkMember \r\n", __FUNCTION__);
    }

    return;
} /* End of PRI_MGR_DeleteTrunkMember_CallBack */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_DeleteLastTrunkMember_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the last trunk member
 *            is removed from the trunk
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void PRI_MGR_DeleteLastTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (!PRI_MGR_DeleteLastTrunkMember(trunk_ifindex, member_ifindex) )
    {
        SYSFUN_Debug_Printf("\r\n %s: failed return from"
                            " PRI_MGR_DeleteLastTrunkMember \r\n", __FUNCTION__);
    }

    return;
} /* End of PRI_MGR_DeleteLastTrunkMember_CallBack */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: PRI_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for priority mgr.
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
BOOL_T PRI_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    PRI_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (PRI_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding PRI_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case PRI_MGR_IPC_SETDOT1DPORTDEFAULTUSERPRIORITY:
        	msg_p->type.ret_bool = PRI_MGR_SetDot1dPortDefaultUserPriority(
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
            break;

        case PRI_MGR_IPC_SETDOT1DPRIORITY:
        	msg_p->type.ret_bool = PRI_MGR_SetDot1dPriority(
        	    msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2,
        	    msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
            break;

        case PRI_MGR_IPC_SETDOT1DPORTNUMTRAFFICCLASSES:
        	msg_p->type.ret_bool = PRI_MGR_SetDot1dPortNumTrafficClasses(
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
            break;

        case PRI_MGR_IPC_GETDOT1DPORTPRIORITYENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetDot1dPortPriorityEntry(
        	    msg_p->data.arg_grp_ui32_dot1dportpriority.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_dot1dportpriority.arg_dot1d_port_pri_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1dportpriority);
            break;

        case PRI_MGR_IPC_GETNEXTDOT1DPORTPRIORITYENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetNextDot1dPortPriorityEntry(
        	    &msg_p->data.arg_grp_ui32_dot1dportpriority.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_dot1dportpriority.arg_dot1d_port_pri_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1dportpriority);
            break;

        case PRI_MGR_IPC_GETDOT1DTRAFFICCLASSENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetDot1dTrafficClassEntry(
        	    msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_ui32_2,
        	    &msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_dot1d_traffic_class_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_dot1dtrafficclass);
            break;

        case PRI_MGR_IPC_GETNEXTDOT1DTRAFFICCLASSENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetNextDot1dTrafficClassEntry(
        	    &msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_ui32_1,
        	    &msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_ui32_2,
        	    &msg_p->data.arg_grp_ui32_ui32_dot1dtrafficclass.arg_dot1d_traffic_class_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_dot1dtrafficclass);
            break;

        case PRI_MGR_IPC_SETDOT1DTRAFFICCLASS:
        	msg_p->type.ret_bool = PRI_MGR_SetDot1dTrafficClass(
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
            break;

        case PRI_MGR_IPC_GETDOT1DUSERPRIORITYREGENENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetDot1dUserPriorityRegenEntry(
        	    msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_dot1d_user_pri_regen_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1duserpriorityregen);
            break;

        case PRI_MGR_IPC_GETNEXTDOT1DUSERPRIORITYREGENENTRY:
        	msg_p->type.ret_bool = PRI_MGR_GetNextDot1dUserPriorityRegenEntry(
        	    &msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_dot1d_user_pri_regen_entry);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1duserpriorityregen);
            break;

        case PRI_MGR_IPC_SETDOT1DUSERPRIORITYREGENENTRY:
        	msg_p->type.ret_bool = PRI_MGR_SetDot1dUserPriorityRegenEntry(
        	    msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_ui32,
        	    msg_p->data.arg_grp_ui32_dot1duserpriorityregen.arg_dot1d_user_pri_regen_entry);
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
            break;

        case PRI_MGR_IPC_GETRUNNINGPORTPRIORITYPARAMETERS:
        	msg_p->type.ret_ui32 = PRI_MGR_GetRunningPortPriorityParameters(
        	    msg_p->data.arg_grp_ui32_portpriority.arg_ui32,
        	    &msg_p->data.arg_grp_ui32_portpriority.arg_port_pri_runningcfg);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_grp_ui32_portpriority);
            break;

        case PRI_MGR_IPC_GETNEXTRUNNINGTRAFFICCLASSPARAMETERS:
        	msg_p->type.ret_ui32 = PRI_MGR_GetNextRunningTrafficClassParameters(
        	    &msg_p->data.arg_traffic_class);
            msgbuf_p->msg_size = PRI_MGR_GET_MSG_SIZE(arg_traffic_class);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = PRI_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of PRI_MGR_HandleIPCReqMsg */


/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_InitDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initialize elements in priority table to its
 *            default value
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void PRI_MGR_InitDefault(void)
{
    UI32_T         index, lport_num=0;
    UI8_T          mapping_table[8];
    PRI_MGR_Port_Info_T     port_info;

    /* BODY */

    memset(&port_info, 0, sizeof(PRI_MGR_Port_Info_T));

    /* Allen Cheng, 12/02/2002
    while (SWCTRL_GetNextPortInfo(&port_num, &port_entry))
    */
    while (SWCTRL_GetNextLogicalPort(&lport_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        /* Initialize port priority table
         */
        port_info.lport_ifindex = lport_num;
        port_info.port_priority_entry.dot1d_port_default_user_priority = SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY;
        port_info.port_priority_entry.dot1d_port_num_traffic_classes = SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;


        /* Initialize traffic_class_table
         */
        for (index = MIN_dot1dTrafficClassPriority; index <= MAX_dot1dTrafficClassPriority; index++)
        {
            port_info.traffic_class_entry[index].dot1d_traffic_class_priority = index;

            if (index == 0)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS;
            if (index == 1)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS;
            if (index == 2)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS;
            if (index == 3)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS;
            if (index == 4)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS;
            if (index == 5)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS;
            if (index == 6)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS;
            if (index == 7)
                port_info.traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS;

            mapping_table[index] = (UI8_T)port_info.traffic_class_entry[index].dot1d_traffic_class;

        } /* end of for */
        /*regen priority table*/
        for (index = MIN_dot1dUserPriority; index <= MAX_dot1dUserPriority; index++)
        {
            port_info.user_regen_entry[index].dot1d_user_priority = index;
            port_info.user_regen_entry[index].dot1d_regen_user_priority = index;       
        }


        memcpy(&port_table[lport_num], &port_info, sizeof(PRI_MGR_Port_Info_T));

        /* Initialize default value to hardware
         */
        if (!SWCTRL_SetPortUserDefaultPriority(lport_num, MIN_dot1dPortDefaultUserPriority))
        {
#if (SYS_CPNT_SYSLOG == TRUE)
            PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_InitDefault");
#endif
            return;
        }
#if (SYS_CPNT_PRI_MGMT_PER_PORT == TRUE) /* per port */
        if (!SWCTRL_SetPriorityMapping(lport_num, mapping_table))
        {
    #if (SYS_CPNT_SYSLOG == TRUE)
            PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_InitDefault");
    #endif
            return;
        }
#endif
    } /* end of for */

#if (SYS_CPNT_PRI_MGMT_PER_PORT == FALSE) /* per port */
    if (!SWCTRL_SetPriorityMappingPerSystem(mapping_table))
    {
    #if (SYS_CPNT_SYSLOG == TRUE)
        PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_InitDefault");
    #endif
        return;
    }
#endif

    memset(&port_info, 0, sizeof(PRI_MGR_Port_Info_T));

    for (lport_num = SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER; lport_num < SYS_ADPT_RS232_1_IF_INDEX_NUMBER; lport_num++)
    {
        port_info.lport_ifindex = lport_num;
        /*regen priority table*/
        for (index = MIN_dot1dUserPriority; index <= MAX_dot1dUserPriority; index++)
        {
            port_info.user_regen_entry[index].dot1d_user_priority = index;
            port_info.user_regen_entry[index].dot1d_regen_user_priority = index;       
        }
        memcpy(&port_table[lport_num], &port_info, sizeof(PRI_MGR_Port_Info_T));
    }

    return;

} /* end of PRI_MGR_InitDefault()*/

#if (SYS_CPNT_SYSLOG == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_ReportSyslogMessage
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is another interface for each API to use syslog
 *            to report any error message.
 * INPUT    : error_type - the type of error.  Value defined in vlan_type.h
 *            error_msg - value defined in syslog_mgr.h
 *            function_name - the specific name of API to which error occured.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void PRI_MGR_ReportSyslogMessage(UI32_T error_type, UI8_T error_msg, char *function_name)
{
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;

    /* BODY */

    owner_info.level = (UI8_T)error_type;
    owner_info.module_no = SYS_MODULE_PRIMGMT;
    owner_info.function_no = PRI_MGR_FUNCTION_NUMBER;
    owner_info.error_no = PRI_MGR_ERROR_NUMBER;
    SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, error_msg, function_name, 0, 0);

    return;
} /* end of PRI_MGR_ReportSyslogMessage() */
#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_CompareDefaultTrafficClass
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the current traffic class is
 *            different from default traffic class value.  Otherwise, returns
 *            FALSE for no changed.
 * INPUT    : current_traffic_class - Current mapped traffic class
 *            default_traffic_class-  Default traffic class value.
 * OUTPUT   : running_cfg_value - This value contains the current traffic class
 *                                if different from default.
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_CompareDefaultTrafficClass(UI32_T current_traffic_class,
											   UI32_T default_traffic_class,
											   UI32_T *running_cfg_value)
{
    if (current_traffic_class != default_traffic_class)
    {
        *running_cfg_value = current_traffic_class;
        return TRUE;
    }

    return FALSE;
} /* end of PRI_MGR_CompareDefaultTrafficClass() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetTrunkMemberInfo
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if trunk member information has been
 *            successfully modified.  Otherwise, return FALSE.
 * INPUT    : trunk_id - the specific trunk information to be updated
 *            type of operation - PRI_MGR_TRAFFIC_CLASS \ PRI_MGR_USER_PRIORITY
 *            priority - user-specify prioirty
 *            traffic_class - user specify traffic class if necessary.
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_SetTrunkMemberInfo(UI32_T trunk_id, UI32_T type, UI32_T priority, UI32_T traffic_class)
{
    TRK_MGR_TrunkEntry_T    trunk_info;
    UI8_T       mapped_traffic_class[8];
    /*
    BOOL_T      setflag = FALSE;
    */
    UI32_T		trk_member, index;
    UI32_T      trunk_ifindex;

    /* BODY */


    trunk_info.trunk_index = trunk_id;

    if (!TRK_MGR_GetTrunkEntry(&trunk_info))
    {
        return FALSE;
    }
    for (trk_member = 1; trk_member <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST *8); trk_member++)
    {
        if (trunk_info.trunk_ports[(UI32_T)((trk_member-1)/8)]  & (1 << ( 7 - ((trk_member-1)%8))) )
        {
            if (type == PRI_MGR_TRAFFIC_CLASS)
            {
                port_table[trk_member].traffic_class_entry[priority].dot1d_traffic_class = traffic_class;
            }
            if (type == PRI_MGR_USER_PRIORITY)
            {
                port_table[trk_member].port_priority_entry.dot1d_port_default_user_priority = priority;
            }
        }
    }

/* Allen Cheng : Added, 12/02/2002 */
    if (SWCTRL_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex) )
    {
        if (type == PRI_MGR_TRAFFIC_CLASS)
        {
                port_table[trunk_ifindex].traffic_class_entry[priority].dot1d_traffic_class = traffic_class;
                /*modify to use regen priority*/
                for (index = MIN_dot1dTrafficClassPriority; index < MAX_dot1dTrafficClassPriority+1; index++)
                    mapped_traffic_class[index] = (UI8_T )port_table[trunk_ifindex].traffic_class_entry[port_table[trunk_ifindex].user_regen_entry[index].dot1d_regen_user_priority].dot1d_traffic_class;
                if (!SWCTRL_SetPriorityMapping(trunk_ifindex, mapped_traffic_class))
                {
#if (SYS_CPNT_SYSLOG == TRUE)
                    PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dTrafficClass");
#endif
                    return FALSE;
                }
        } /* End of if (type == PRI_MGR_TRAFFIC_CLASS) */
        if (type == PRI_MGR_USER_PRIORITY)
        {
            port_table[trunk_ifindex].port_priority_entry.dot1d_port_default_user_priority = priority;
            if (!SWCTRL_SetPortUserDefaultPriority(trunk_ifindex, priority))
            {
#if (SYS_CPNT_SYSLOG == TRUE)
                PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_SetDot1dPortDefaultUserPriority");
#endif
                return FALSE;
            }
        } /* End of if (type == PRI_MGR_USER_PRIORITY) */
    } /* End of if */

    return TRUE;
} /* end of PRI_MGR_SetTrunkMemberInfo() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_Notify_CosChanged
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when cos mapping changed.
 * INPUT    : lport_ifindex - specify which port the event occured.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void PRI_MGR_Notify_CosChanged(UI32_T lport_ifindex)
{
    SYS_CALLBACK_MGR_CosChangedCallback(SYS_MODULE_PRIMGMT, lport_ifindex);
    return;
} /* End of PRI_MGR_Notify_CosChanged()*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_InitPortPriorityTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize/Clear port OM.
 * INPUT    : lport_num             -- the ifindex of the port
 * OUTPUT   : port_info             -- the port info
 *            mapping_table         -- the mapping table
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T PRI_MGR_InitPortPriorityTable(UI32_T lport_num, PRI_MGR_Port_Info_T *port_info, UI8_T *mapping_table)
{
    UI32_T  index;

    if ((port_info == NULL) || (mapping_table == NULL))
    {
        return FALSE;
    }

    port_info->lport_ifindex = lport_num;
    port_info->port_priority_entry.dot1d_port_default_user_priority = SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY;
    port_info->port_priority_entry.dot1d_port_num_traffic_classes = SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;

    /* Initialize traffic_class_table
     */
    for (index = MIN_dot1dTrafficClassPriority; index <= MAX_dot1dTrafficClassPriority; index++)
    {
        port_info->traffic_class_entry[index].dot1d_traffic_class_priority = index;

        switch (index)
        {
            case 0:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS;
                break;

            case 1:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS;
                break;

            case 2:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS;
                break;

            case 3:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS;
                break;

            case 4:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS;
                break;

            case 5:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS;
                break;

            case 6:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS;
                break;

            case 7:
                port_info->traffic_class_entry[index].dot1d_traffic_class = SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS;
                break;
        }

        mapping_table[index] = (UI8_T)port_info->traffic_class_entry[index].dot1d_traffic_class;

    } /* end of for */

    /* Initialize regen_user_priority_table
     */
    for (index = MIN_dot1dUserPriority; index <= MAX_dot1dUserPriority; index++)
    {
        port_info->user_regen_entry[index].dot1d_user_priority = index;
        port_info->user_regen_entry[index].dot1d_regen_user_priority = index;       
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_AddFirstTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the first trunk member can be successfully
 *            added to the specific trunk index.  Otherwise, return FALSE
 * INPUT    : trunk_ifindex - the specific trunk_index
 *            member_ifindex - the specific port index
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Trunk_ifindex information is inherited from the first member.
 *------------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_AddFirstTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    PRI_MGR_Port_Info_T     trunk_info, member_info;
    UI32_T                  priority;

    /* BODY */

    memset(&trunk_info, 0, sizeof(PRI_MGR_Port_Info_T));
    memset(&member_info, 0, sizeof(PRI_MGR_Port_Info_T));

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    memcpy(&trunk_info, &port_table[trunk_ifindex], sizeof(PRI_MGR_Port_Info_T));
    memcpy(&member_info, &port_table[member_ifindex], sizeof(PRI_MGR_Port_Info_T));

    /* Trunk ifindex will inherit all priority information from its first member
     */
    trunk_info.port_priority_entry.dot1d_port_default_user_priority =
        member_info.port_priority_entry.dot1d_port_default_user_priority;

    trunk_info.port_priority_entry.dot1d_port_num_traffic_classes =
        member_info.port_priority_entry.dot1d_port_num_traffic_classes;

    /* trunk_ifindex will inherit COS information from its first member
     */
    for (priority = 0; priority < 8; priority++)
    {
        memcpy(&trunk_info.traffic_class_entry[priority], &member_info.traffic_class_entry[priority], sizeof(PRI_MGR_Dot1dTrafficClassEntry_T));
        memcpy(&trunk_info.user_regen_entry[priority], &member_info.user_regen_entry[priority], sizeof(PRI_MGR_Dot1dUserPriorityRegenEntry_T));
    }
    /* Update trunk_ifindex information to port_table
     */
    memcpy(&port_table[trunk_ifindex], &trunk_info, sizeof(PRI_MGR_Port_Info_T));

    return TRUE;
} /* end of PRI_MGR_AddFirstTrunkMember() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_AddTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific lport can be successfully
 *            added to the specific trunk index.  Otherwise, return FALSE
 * INPUT    : trunk_ifindex - the specific trunk_index
 *            member_ifindex - the specific port index
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Priority information of the member_ifindex will be inherited from
 *            trunk_ifindex
 *------------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_AddTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{

    PRI_MGR_Port_Info_T     trunk_info, member_info;

    /* BODY */

    memset(&trunk_info, 0, sizeof(PRI_MGR_Port_Info_T));
    memset(&member_info, 0, sizeof(PRI_MGR_Port_Info_T));

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    // member port will inherit information from trunk port.

    memcpy(&trunk_info, &port_table[trunk_ifindex], sizeof(PRI_MGR_Port_Info_T));
    memcpy(&member_info, &port_table[member_ifindex], sizeof(PRI_MGR_Port_Info_T));

    /* Trunk ifindex will inherit all priority information from its first member
     */
    member_info.port_priority_entry.dot1d_port_default_user_priority =
        trunk_info.port_priority_entry.dot1d_port_default_user_priority;

    member_info.port_priority_entry.dot1d_port_num_traffic_classes =
        trunk_info.port_priority_entry.dot1d_port_num_traffic_classes;

#if (SYS_CPNT_PRI_MGMT_PER_PORT == TRUE)  /* per port */

    /* In the case of Per Port COS mapping, member_ifindex inherits COS information from trunk_ifindex and
       the new COS mapping will need to be set to SWCTRL. However, for per system setting, this process can
       be eliminated since COS information is consistent accross all ports.
     */
    for (priority = 0; priority < 8; priority++)
    {
        memcpy(&member_info.traffic_class_entry[priority], &trunk_info.traffic_class_entry[priority], sizeof(PRI_MGR_Dot1dTrafficClassEntry_T));
        mapped_traffic_class[priority] =(UI8_T) member_info.traffic_class_entry[priority].dot1d_traffic_class;
    } /* end of for */

    /* Allen Cheng : 12/02/2002
    if (!SWCTRL_SetPriorityMapping(member_ifindex, mapped_traffic_class))
    */
    if (!SWCTRL_SetPriorityMapping(trunk_ifindex, mapped_traffic_class))
    {
    #if (SYS_CPNT_SYSLOG == TRUE)
        PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_AddTrunkMember");
    #endif
        return FALSE;
    } /* end of if */

#endif

    /* In the case of Per system setting, there is no need to set priority mapping for individual trunk member port
     */
    /* Allen Cheng : 12/02/2002
    if (!SWCTRL_SetPortUserDefaultPriority(member_ifindex, member_info.port_priority_entry.dot1d_port_default_user_priority))
    */
    if (!SWCTRL_SetPortUserDefaultPriority(trunk_ifindex, member_info.port_priority_entry.dot1d_port_default_user_priority))
    {
#if (SYS_CPNT_SYSLOG == TRUE)
        PRI_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, FUNCTION_RETURN_FAIL_INDEX, "PRI_MGR_AddTrunkMember");
#endif
        return FALSE;
    } /* end of if */

    /* Update member_ifindex information to port_table
     */
    memcpy(&port_table[member_ifindex], &member_info, sizeof(PRI_MGR_Port_Info_T));

    return TRUE;

} /* end of PRI_MGR_AddTrunkMember() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_DeleteTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific lport can be successfully
 *            remove from the trunk index.  Otherwise, return FALSE
 * INPUT    : trunk_ifindex - the specific trunk_index
 *            member_ifindex - the specific port index
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : member_ifindex removed from trunk_ifindex will carry on the information
 *            in trunk_ifindex.  However, these attribute value can now be individully
 *            modified, dependent from trunk_ifindex
 *------------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_DeleteTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */
    /* Loal port table does not keep information to distinguish trunk_member port from
       normal port.  trunk member port will carry on information from trunk port and
       become a normal port.
     */
    return TRUE;
} /* end of PRI_MGR_DeleteTrunkMember() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_DeleteLastTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the last trunk member lport can be successfully
 *            remove from the trunk index.  Otherwise, return FALSE
 * INPUT    : trunk_ifindex - the specific trunk_index
 *            member_ifindex - the specific port index
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : member_ifindex removed from trunk_ifindex will carry on the information
 *            in trunk_ifindex.  However, these attribute value can now be individully
 *            modified, dependent from trunk_ifindex.
 *            Priority information of trunk_ifindex will be reset to 0 until it has
 *            a new member.
 *------------------------------------------------------------------------------*/
static BOOL_T PRI_MGR_DeleteLastTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI32_T index=0;
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    memset(&port_table[trunk_ifindex], 0, sizeof(PRI_MGR_Port_Info_T));
    
    /*
       regarding to regen table, because the API have the sub key in entry, so it need to save back
    */
    for (index = MIN_dot1dUserPriority; index <= MAX_dot1dUserPriority; index++)
    {
        port_table[trunk_ifindex].user_regen_entry[index].dot1d_user_priority = index;
    }
    
    return TRUE;
} /* end of PRI_MGR_DeleteLastTrunkMember() */
