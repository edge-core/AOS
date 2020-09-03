/* Module Name: TRK_MGR.H
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
/* NOTES:
 * 1. ES3626A MIB contains the following group:
 *    trunkMgt       { es3626aMIBObjects 3 } -- implemented here
 */

#ifndef _TRK_MGR_H_
#define _TRK_MGR_H_


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define TRK_MGR_IPCMSG_TYPE_SIZE sizeof(union TRK_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    TRK_MGR_IPC_CREATETRUNK,
    TRK_MGR_IPC_DESTROYTRUNK,
    TRK_MGR_IPC_CREATEDYNAMICTRUNK,
    TRK_MGR_IPC_FREETRUNKIDDESTROYDYNAMIC,
    TRK_MGR_IPC_ADDTRUNKMEMBER,
    TRK_MGR_IPC_ADDDYNAMICTRUNKMEMBER,
    TRK_MGR_IPC_DELETETRUNKMEMBER,
    TRK_MGR_IPC_DELETEDYNAMICTRUNKMEMBER,
    TRK_MGR_IPC_SETTRUNKNAME,
    TRK_MGR_IPC_SETTRUNKALIAS,
    TRK_MGR_IPC_ISDYNAMICTRUNKID,
    TRK_MGR_IPC_GETTRUNKMEMBERCOUNTS,
    TRK_MGR_IPC_GETTRUNKCOUNTS,
    TRK_MGR_IPC_GETNEXTTRUNKID,
    TRK_MGR_IPC_GETTRUNKMAXID,
    TRK_MGR_IPC_GETTRUNKVALIDNUMBER,
    TRK_MGR_IPC_GETTRUNKENTRY,
    TRK_MGR_IPC_GETNEXTTRUNKENTRY,
    TRK_MGR_IPC_GETNEXTRUNNINGTRUNKENTRY,
    TRK_MGR_IPC_SETTRUNKPORTS,
    TRK_MGR_IPC_SETTRUNKSTATUS,
    TRK_MGR_IPC_ISTRUNKEXIST,
    TRK_MGR_IPC_GETLASTCHANGETIME
};

enum TRK_MGR_Error_E
{
    TRK_MGR_SUCCESS = 1,
    TRK_MGR_ERROR_TRUNK,
    TRK_MGR_ERROR_MEMBER,
    TRK_MGR_MEMBER_TOO_MANY,
    TRK_MGR_DIFFERENT_PORT_TYPE,
    TRK_MGR_NOT_IN_THE_SAME_DEVICE,
    TRK_MGR_ADMIN_INCONSIST,
    TRK_MGR_STATE_INCONSIST,
    TRK_MGR_NOT_ON_THE_SAME_VLAN,
    TRK_MGR_NOT_ON_THE_SAME_PRIVATE_VLAN,
    TRK_MGR_PRIVATE_VLAN_DOWN_LINK_PORT,
    TRK_MGR_ANALYZER_PORT_ERROR,
    TRK_MGR_DEV_INTERNAL_ERROR,
    TRK_MGR_OVERLAPPED_TRUNK,
    TRK_MGR_MEMBER_NOT_NEIGHBORED,
    TRK_MGR_OTHER_WRONG
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in TRK_MGR_IpcMsg_T.data
 */
#define TRK_MGR_GET_MSG_SIZE(field_name)                        \
            (TRK_MGR_IPCMSG_TYPE_SIZE +                         \
            sizeof(((TRK_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T trunk_index;
    UI8_T  trunk_ports[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    UI8_T  active_trunk_ports[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    UI32_T trunk_creation;
    UI32_T trunk_status;

    /* not for MIB */
    char   trunk_name[SYS_ADPT_MAX_TRUNK_NAME_LEN];
    /* for GetNextRunning */
    BOOL_T trunk_status_changed;        /* TRUE: changed,  FALSE: no changed  */
    BOOL_T trunk_ports_changed;
    BOOL_T trunk_name_changed;
} TRK_MGR_TrunkEntry_T;

/* IPC message structure
 */
typedef struct
{
	union TRK_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
        UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
	    UI32_T               arg_ui32;
	    TRK_MGR_TrunkEntry_T arg_trunk_entry;

	    struct
	    {
	        UI32_T arg_1;
	        UI32_T arg_2;
	    } arg_grp_ui32_ui32;

        struct
	    {
	        UI32_T arg_1;
	        UI32_T arg_2;
	        UI32_T arg_3;
	    } arg_grp_ui32_ui32_ui32;

	    struct
	    {
	        UI32_T arg_id;
	        char   arg_name[SYS_ADPT_MAX_TRUNK_NAME_LEN+1];
	    } arg_grp_trunk_name;

	    struct
	    {
	        UI32_T arg_id;
	        UI8_T  arg_portlist[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
	    } arg_grp_trunk_portlist;

	    struct
	    {
	        UI32_T arg_ui32;
	        UI8_T  arg_ui8;
	    } arg_grp_ui32_ui8;

	    struct
	    {
	        UI32_T arg_ui32;
	        BOOL_T arg_bool;
	    } arg_grp_ui32_bool;
	} data;
} TRK_MGR_IpcMsg_T;


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
void TRK_MGR_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_Create_InterCSC_Relation(void);

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
void TRK_MGR_EnterTransitionMode(void);


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
void TRK_MGR_EnterMasterMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: called by stkpkg to to set transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void TRK_MGR_SetTransitionMode(void);



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
void TRK_MGR_EnterSlaveMode(void);

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
void TRK_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


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
void TRK_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunking port to create
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_CreateTrunk(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_DestroyTrunk(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AllocateTrunkIdCreateDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allocate(create) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_AllocateTrunkIdCreateDynamic(UI32_T *trunk_id);


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
BOOL_T TRK_MGR_CreateDynamicTrunk(UI32_T trunk_id);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_FreeTrunkIdDestroyDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will free(destroy) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_FreeTrunkIdDestroyDynamic(UI32_T trunk_id);


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
UI32_T TRK_MGR_AddTrunkMember(UI32_T trunk_id, UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_AddDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id   -- which trunking port to add member
 *           unit       -- which unit to add
 *           port       -- which port to add
 *           is_active  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value, for LACP
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_AddDynamicTrunkMember(UI32_T trunk_id, UI32_T ifindex, BOOL_T is_active_member);

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
BOOL_T TRK_MGR_DeleteTrunkMember(UI32_T trunk_id, UI32_T ifindex);


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
BOOL_T TRK_MGR_DeleteDynamicTrunkMember( UI32_T trunk_id, UI32_T ifindex);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to get
 * OUTPUT  : name     -- the name of this trunk
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetTrunkName(UI32_T trunk_id, char *name);


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
BOOL_T TRK_MGR_SetTrunkName(UI32_T trunk_id, char *name);

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
BOOL_T TRK_MGR_SetTrunkAlias(UI32_T trunk_id, char *alias);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_IsDynamicTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the trunk is a dynamic trunk  or not
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Dynamic, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_IsDynamicTrunkId(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkMemberCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk member numbers
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total trunk member number
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetTrunkMemberCounts(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetTrunkCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk numbers which are created
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total created trunk number
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetTrunkCounts(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next available trunk ID
 * INPUT   : trunk_id -- the key to get
 * OUTPUT  : trunk_id -- from 0 to SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetNextTrunkId(UI32_T *trunk_id);


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
BOOL_T TRK_MGR_GetTrunkMaxId(UI32_T *trunk_max_id);


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
BOOL_T TRK_MGR_GetTrunkValidNumber(UI32_T *trunk_valid_numer);


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
BOOL_T TRK_MGR_GetTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_GetNextTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetNextRunningTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk entry of running config
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : trunk_id = 0 ==> get the first trunk (exclude dynamic trunk)
 *------------------------------------------------------------------------*/
UI32_T  TRK_MGR_GetNextRunningTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);


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
BOOL_T TRK_MGR_SetTrunkPorts(UI32_T trunk_id, UI8_T *trunk_portlist);


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
BOOL_T TRK_MGR_SetTrunkStatus(UI32_T trunk_id, UI8_T trunk_status);


/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_IsTrunkExist
 *------------------------------------------------------------------------
 * FUNCTION: Does this trunk exist or not.
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : is_static -- TRUE/FASLE
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T TRK_MGR_IsTrunkExist(UI32_T trunk_id, BOOL_T *is_static);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T TRK_MGR_GetLastChangeTime();

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
BOOL_T TRK_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

BOOL_T TRK_MGR_GetTrunkPorts(UI32_T trunk_id,
                                    UI32_T *port_count,
                                    SYS_TYPE_Uport_T *unit_port);

#endif /* _TRK_MGR_H_ */
