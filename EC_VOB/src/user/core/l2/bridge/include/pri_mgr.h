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
 *    Date          -- Modifier,        Reason
 * 06-13-2001          CPY              Create
 * 10-17-2001          AmyTu            Modify
 *---------------------------------------------------------------------------------
 * Copyright(C)      Accton Corporation, 1999, 2000
 *---------------------------------------------------------------------------------
 */

#ifndef     PRI_MGR_H
#define     PRI_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define PRI_MGR_IPCMSG_TYPE_SIZE sizeof(union PRI_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
	PRI_MGR_IPC_SETDOT1DPORTDEFAULTUSERPRIORITY,
	PRI_MGR_IPC_SETDOT1DPRIORITY,
	PRI_MGR_IPC_SETDOT1DPORTNUMTRAFFICCLASSES,
	PRI_MGR_IPC_GETDOT1DPORTPRIORITYENTRY,
	PRI_MGR_IPC_GETNEXTDOT1DPORTPRIORITYENTRY,
	PRI_MGR_IPC_GETDOT1DTRAFFICCLASSENTRY,
	PRI_MGR_IPC_GETNEXTDOT1DTRAFFICCLASSENTRY,
	PRI_MGR_IPC_SETDOT1DTRAFFICCLASS,
	PRI_MGR_IPC_GETDOT1DUSERPRIORITYREGENENTRY,
	PRI_MGR_IPC_GETNEXTDOT1DUSERPRIORITYREGENENTRY,
	PRI_MGR_IPC_SETDOT1DUSERPRIORITYREGENENTRY,
	PRI_MGR_IPC_GETRUNNINGPORTPRIORITYPARAMETERS,
	PRI_MGR_IPC_GETNEXTRUNNINGTRAFFICCLASSPARAMETERS
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in XSTP_MGR_IpcMsg_T.data
 */
#define PRI_MGR_GET_MSG_SIZE(field_name)                       \
            (PRI_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((PRI_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T      dot1d_port_num_traffic_classes;
    UI32_T      dot1d_port_default_user_priority;
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    UI32_T      dot1d_port_default_user_priority_user_cfg;      /* If dot1d_port_dft_pri_applied_dynamic_cfg is set,
                                                                 * this value keep the priority value from user
                                                                 * interface.
                                                                 */
    BOOL_T      dot1d_port_dft_pri_is_dynamic_cfg;              /* If TRUE, the port default user priority is applied
                                                                 * by dynamic qos.
                                                                 */
#endif
} PRI_MGR_Dot1dPortPriorityEntry_T;

typedef struct
{
    UI32_T      dot1d_traffic_class_priority;   /* Second KEY for Dot1dTrafficClassEntry*/
    UI32_T      dot1d_traffic_class;
} PRI_MGR_Dot1dTrafficClassEntry_T;

typedef struct
{
    UI32_T      dot1d_user_priority;   /* Second KEY for Dot1dTrafficClassEntry*/
    UI32_T      dot1d_regen_user_priority;
} PRI_MGR_Dot1dUserPriorityRegenEntry_T;

typedef struct
{
    UI32_T      lport_ifindex;          /* KEY for Dot1dPortPriorityEntry */
                                        /* First KEY for Dot1dTrafficClassEntry */
    PRI_MGR_Dot1dPortPriorityEntry_T      port_priority_entry;
    PRI_MGR_Dot1dTrafficClassEntry_T      traffic_class_entry[8];
    PRI_MGR_Dot1dUserPriorityRegenEntry_T user_regen_entry[8];
} PRI_MGR_Port_Info_T;

typedef struct
{
    UI32_T  lport_ifindex;              /* KEY */

    UI32_T  default_user_priority;
    UI32_T  port_num_traffic_class;

    BOOL_T  user_priority_changed;
    BOOL_T  traffic_class_changed;
} PRI_MGR_PortPriority_RunningCfg_T;

typedef struct
{
    UI32_T  lport_ifindex;                  /* Primary Key */

    UI32_T  dot1d_traffic_class_priority;   /* Secondary Key */
    UI32_T  dot1d_traffic_class;

    BOOL_T  traffic_class_changed;
} PRI_MGR_TrafficClass_RunningCfg_T;

/* IPC message structure
 */
typedef struct
{
	union PRI_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
		UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
        PRI_MGR_TrafficClass_RunningCfg_T arg_traffic_class;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ui32;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI8_T  arg_ui8;
        } arg_grp_ui32_ui32_ui8;

        struct
        {
            UI32_T                           arg_ui32;
            PRI_MGR_Dot1dPortPriorityEntry_T arg_dot1d_port_pri_entry;
        } arg_grp_ui32_dot1dportpriority;

        struct
        {
            UI32_T                            arg_ui32_1;
            UI32_T                            arg_ui32_2;
            PRI_MGR_Dot1dTrafficClassEntry_T  arg_dot1d_traffic_class_entry;
        } arg_grp_ui32_ui32_dot1dtrafficclass;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
        } arg_grp_ui32_ui32_ui32;

        struct
        {
            UI32_T                                arg_ui32;
            PRI_MGR_Dot1dUserPriorityRegenEntry_T arg_dot1d_user_pri_regen_entry;
        } arg_grp_ui32_dot1duserpriorityregen;

        struct
        {
            UI32_T                            arg_ui32;
            PRI_MGR_PortPriority_RunningCfg_T arg_port_pri_runningcfg;
        } arg_grp_ui32_portpriority;
	} data; /* the argument(s) for the function corresponding to cmd */
} PRI_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM DECLARATION
 */

/* Initialization API
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
void PRI_MGR_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_Create_InterCSC_Relation(void);

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
void  PRI_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_EnterTransitionMode(void);

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
void PRI_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to disable the priority operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PRI_MGR_EnterSlaveMode(void);

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
SYS_TYPE_Stacking_Mode_T  PRI_MGR_GetOperationMode(void);


/* Dot1dPortPriorityEntry
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dPortDefaultUserPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default user priority of the
 *            specific port can be set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex        -- the specified port number
 *            default_priority     -- the default priority for the port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. lport_ifindex is the key to identify a port's default user
 *               priority.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dPortDefaultUserPriority(UI32_T lport_ifindex, UI32_T default_priority);

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
BOOL_T PRI_MGR_SetDynamicDot1dPortDefaultUserPriority(UI32_T lport_ifindex, UI32_T default_priority);
#endif

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
BOOL_T PRI_MGR_SetDot1dPriority(UI32_T lport_ifindex, UI32_T traffic_class, UI8_T priority);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dPortNumTrafficClasses
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the number of priority supported
 *            by the specified port can be set.  otherwise, return false.
 * INPUT    : lport_ifindex            - the specified port number
 *            number_of_traffic_class - the number of priority queue
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The port_id is the key to identify the number of traffic
 *               classes of a given port.
 *--------------------------------------------------------------------------*/
BOOL_T PRI_MGR_SetDot1dPortNumTrafficClasses(UI32_T lport_ifindex, UI32_T number_of_traffic_class);


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
BOOL_T PRI_MGR_GetDot1dPortPriorityEntry(UI32_T lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry);


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
BOOL_T PRI_MGR_GetNextDot1dPortPriorityEntry(UI32_T *lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry);



/* Dot1dTrafficClassEntry
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_GetDot1dTrafficClassEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the traffic class of the specific user
 *            priority and lport_ifindex can be retrieve successfully.  Otherwise,
 *            returns false.
 * INPUT    : traffic_class_entry.lport_ifindex  - the primary key to identify a
 *                                                 port traffic class
 *            traffic_class_entry.dot1d_traffic_class_priority - the secondary key
 *                                                               to identify a port
 *                                                               traffic class
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
BOOL_T PRI_MGR_GetDot1dTrafficClassEntry(UI32_T lport_ifindex,
                                         UI32_T priority,
                                         PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry);


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
BOOL_T PRI_MGR_GetNextDot1dTrafficClassEntry(UI32_T *lport_ifindex,
                                             UI32_T *priority,
                                             PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry);



/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_SetDot1dTrafficClass
 *--------------------------------------------------------------------------
 * PURPOSE  : Using to set the mapped traffic class of the user priority by
 *            the specified port.
 * INPUT    : lport_ifindex        -- the specified port number
 *            user_priority -- the user priority
 *            traffic_class -- the mapped traffic classs
 * OUTPUT   : none
 * RETURN   : TRUE  -- the value can set successfully
 *            FLASE -- the value can't set successfully
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
BOOL_T PRI_MGR_SetDot1dTrafficClass(UI32_T lport_ifindex, UI32_T user_priority, UI32_T traffic_class);


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
                                              PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry);


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
                                                  PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry);


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
                                              PRI_MGR_Dot1dUserPriorityRegenEntry_T user_regen_entry);



/* GetRunningConfig API
 */
/* ---------------------------------------------------------------------
 * FUNCTION NAME  - PRI_MGR_GetRunningPortPriorityParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific non-default user priority associated with lport_ifindex
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - the specific port number
 * OUTPUT: port_cfg  - structure which contains non-defulat priority value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user priority value
 * ----------------------------------------------------------------------------------*/
UI32_T PRI_MGR_GetRunningPortPriorityParameters(UI32_T lport_ifindex, PRI_MGR_PortPriority_RunningCfg_T *port_cfg);


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
UI32_T PRI_MGR_GetNextRunningTrafficClassParameters(PRI_MGR_TrafficClass_RunningCfg_T *traffic_class_cfg);

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
void PRI_MGR_RegisterCosChanged_CallBack(void (*fun)(UI32_T lport_ifindex));
#endif
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
void PRI_MGR_AddFirstTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void PRI_MGR_AddTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void PRI_MGR_DeleteTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void PRI_MGR_DeleteLastTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);


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
void PRI_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void PRI_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

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
BOOL_T PRI_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif /* #ifndef PRI_MGR_H */
