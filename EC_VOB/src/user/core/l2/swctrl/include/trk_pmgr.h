/*-----------------------------------------------------------------------------
 * MODULE NAME: TRK_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/29     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef TRK_PMGR_H
#define TRK_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "trk_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : TRK_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for TRK_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunking port to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_CreateTrunk(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DestroyTrunk(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_CreateDynamicTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allocate(create) a dynamic trunk
 * INPUT   : trunk_id -- The dynamic
 * OUTPUT  : None.
 * RETURN  : TRUE  -- 1. This trunk is dynamic already.
 *                    2. This trunk is created as dynamic trunk.
 *           FALSE -- 1. This trunk is static trunk already.
 *                    2. This trunk cannot be created.
 * NOTE    : for LACP.
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_CreateDynamicTrunk(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_FreeTrunkIdDestroyDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will free(destroy) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_FreeTrunkIdDestroyDynamic(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_AddTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           unit     -- which unit to add
 *           port     -- which port to add
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_AddTrunkMember(UI32_T trunk_id, UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_AddDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id   -- which trunking port to add member
 *           unit       -- which unit to add
 *           port       -- which port to add
 *           is_active_member  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value, for LACP
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_AddDynamicTrunkMember(UI32_T trunk_id,UI32_T ifindex, BOOL_T is_active_member);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DeleteTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DeleteTrunkMember(UI32_T trunk_id, UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DeleteDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DeleteDynamicTrunkMember(UI32_T trunk_id, UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           name     -- the name of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkName(UI32_T trunk_id, UI8_T *name);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           alias    -- the alias of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkAlias(UI32_T trunk_id, UI8_T *alias);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_IsDynamicTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the trunk is a dynamic trunk  or not
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Dynamic, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_IsDynamicTrunkId(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkMemberCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk member numbers
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total trunk member number
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetTrunkMemberCounts(UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk numbers which are created
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total created trunk number
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetTrunkCounts(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next available trunk ID
 * INPUT   : trunk_id -- the key to get
 * OUTPUT  : trunk_id -- from 0 to SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetNextTrunkId(UI32_T *trunk_id);

/*----------------------------------------------------------------------*/
/* (trunkMgt 1)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkMaxId
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the maximum number for a trunk identifier
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 1
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkMaxId(UI32_T *trunk_max_id);

/*----------------------------------------------------------------------*/
/* (trunkMgt 2)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkValidNumber
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the number of valid trunks
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 2
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkValidNumber(UI32_T *trunk_valid_numer);

/*----------------------------------------------------------------------*/
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
 * ROUTINE NAME - TRK_PMGR_GetTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetNextTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextRunningTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk entry of running config
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : trunk_id = 0 ==> get the first trunk (exclude dynamic trunk)
 *------------------------------------------------------------------------
 */
UI32_T  TRK_PMGR_GetNextRunningTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk port list
 * INPUT   : trunk_id                       - trunk id
 *           trunk_portlist                 - trunk port list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. ES3626A MIB/trunkMgt 3
 *           2. For trunk_portlist, only the bytes of in the range of user
 *              port will be handle.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkPorts(UI32_T trunk_id, UI8_T *trunk_portlist);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk status
 * INPUT   : trunk_id                       - trunk id
 *           trunk_status                   - VAL_trunkStatus_valid
 *                                            VAL_trunkStatus_invalid
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkStatus(UI32_T trunk_id, UI8_T trunk_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_IsTrunkExist
 *------------------------------------------------------------------------
 * FUNCTION: Does this trunk exist or not.
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : is_static -- TRUE/FASLE
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_IsTrunkExist(UI32_T trunk_id, BOOL_T *is_static);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_TrunkIdToIfindex
 *------------------------------------------------------------------------
 * FUNCTION: Convert trunk ID to ifindex
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : ifindex   -- trunk ifindex
 * RETURN  : TRUE/FALSE
 * NOTE    : Ignore trunk existing state.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_MGR_TrunkIdToIfindex(UI32_T trunk_id, UI32_T *ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_MGR_IfindexToTrunkId
 *------------------------------------------------------------------------
 * FUNCTION: Convert trunk ID to ifindex
 * INPUT   : ifindex   -- trunk ifindex
 * OUTPUT  : trunk_id  -- trunk id
 * RETURN  : TRUE/FALSE
 * NOTE    : Ignore trunk existing state.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_MGR_IfindexToTrunkId(UI32_T ifindex, UI32_T *trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetLastChangeTime(void);


#endif /* #ifndef TRK_PMGR_H */
