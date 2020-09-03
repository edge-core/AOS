/*-----------------------------------------------------------------------------
 * Module   : mflt_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : This file is the multicast filtering manager, to provide get/set
 *            function for managemented objects.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 05/03/2000 - Lico,  for cabletron
 *                                if dynamic IP, add no port to multicast member
 *                                member set
 *            06/12/2000          because 1.1 something error bug fixed.
 *                     		  If we add a router port first then send a IP
 *                                multicast packet the router port will not join
 *                                to the multicast member set.
 *            10/09/2001 - Aaron, Mask the group index (vidx), the new platform
 *                                have multicast table, so don't need the vidx,
 *                                but the Galileo maybe need this field,
 *                                do I mask the code, but don't remove it.
 *            10/11/2001 - Aaron  Remove router port database for new platform.
 *            11/20/2001 - Kelly  Do load balance and mark some code written by
 *                                Arron.
 *            02/19/2002 - Kelly  Delete some code written by Aaron.
 *            02/19/2003 - Lyn    add compilier option for trunk
 *                                change the way to access AMTR by port list
 *            03/27/2003 - Biker  Unknown IP Multicast forward port maintain
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef MFLT_MGR_H
#define MFLT_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* NAME CONSTANT DECLARATIONS
 */
enum MFLT_MGR_PROTOCOL_TYPE_E
{
    MFLT_MGR_LEARN_BY_IGMP = 0,	/* Learn by IGMP */
    MFLT_MGR_LEARN_BY_IPMC,		/* Learn by IP multicast */
    MFLT_MGR_LEARN_BY_GMRP		/* Learn by GMRP */
};

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_InitiateProcessResource
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to initialize multicast filtering module.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_InitiateProcessResource(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void MFLT_MGR_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_SetTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to set MFLT to the transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void MFLT_MGR_SetTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_EnterTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : The function is to set MFLT to transition mode and free all
 *            MFLT resources and reset database to factory default.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void MFLT_MGR_EnterTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_EnterMasterMode
 * ------------------------------------------------------------------------
 * PURPOSE  : The function is to set MFLT to master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void MFLT_MGR_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_EnterSlaveMode
 * ------------------------------------------------------------------------
 * PURPOSE  : The function is to set MFLT to slave mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
*-------------------------------------------------------------------------
 */
void MFLT_MGR_EnterSlaveMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_AddGroupMember
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to add multicast members.
 * INPUT    : vid       - vlan id
 *            group_mac - multicast mac address
 *            lportlist - the logical ports would be added
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_AddGroupMember (UI32_T    vid,
                                UI8_T     *group_mac,
                                UI8_T     *lportlist);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_DeleteGroupMember
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to delete multicast members from the
 *            specified multicast filter entry.
 * INPUT    : vid       - vlan id
 *            group_mac - multicast mac address
 *            lportlist - the logical ports would be removed
 * OUTPUT   : None
 * RETURN   : TRUE  - if entry is exist and delete
 *          : FALSE - if not exist or error
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_DeleteGroupMember (UI32_T   vid,
                                   UI8_T    *group_mac,
                                   UI8_T    *lportlist);

/*-------------------------------------------------------------------------
 * FUNCTION : MFLT_MGR_ClearAllUnknownIPMcastFwdPort
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is used to Clear unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_ClearAllUnknownIPMcastFwdPort (void);


/*-------------------------------------------------------------------------
 * FUNCTION : MFLT_MGR_FillAllUnknownIPMcastFwdPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to fill unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_FillAllUnknownIPMcastFwdPort (void);


/*-------------------------------------------------------------------------
 * FUNCTION : MFLT_MGR_AddUnknownIPMcastFwdPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to add a unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : port  - port want to add unknown multicast forwarding port
 * OUTPUT   : None
 * RETURN   : TRUE  - if entry is exist and delete
 *            FALSE - if not exist or error
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_AddUnknownIPMcastFwdPort (UI32_T   port);


/*-------------------------------------------------------------------------
 * FUNCTION : MFLT_MGR_DelUnknownIPMcastFwdPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to add a unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : port  - port want to add unknown multicast forwarding port
 * OUTPUT   : None
 * RETURN   : TRUE  - if entry is exist and delete
 *            FALSE - if not exist or error
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_DelUnknownIPMcastFwdPort (UI32_T   port);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_AddTrunkMember
 * ------------------------------------------------------------------------
 * PURPOSE  : SWCTRL uses this funtion to notify MFLT that this trunk
 *            member is added to this trunk.
 * INPUT    : trunk_ifindex   	   - trunk interface index
 *            trunk_member_ifindex - lport interface index
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_AddTrunkMember(UI32_T trunk_ifindex,
                               UI32_T trunk_member_ifindex);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_DeleteTrunkMember
 * ------------------------------------------------------------------------
 * PURPOSE  : SWCTRL uses this funtion to notify MFLT that this trunk
 *            member is deleted from this trunk.
 * INPUT    : trunk_ifindex 	   - trunk interface index
 *            trunk_member_ifindex - lport interface index
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_DeleteTrunkMember(UI32_T trunk_ifindex,
                                  UI32_T trunk_member_ifindex);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_TrunkMemberUp
 * ------------------------------------------------------------------------
 * PURPOSE  : SWCTRL uses this funtion to notify MFLT that trunk member
 *            oper link from down becomes up.
 * INPUT    : trunk_ifindex 	   - trunk interface index
 *            trunk_member_ifindex - lport interface index
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_TrunkMemberUp(UI32_T trunk_ifindex,
                              UI32_T trunk_member_ifindex);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_TrunkMemberDown
 * ------------------------------------------------------------------------
 * PURPOSE  : SWCTRL uses this funtion to notify MFLT that trunk member
 *            oper link from up becomes down.
 * INPUT    : trunk_ifindex        - trunk interface index
 *            trunk_member_ifindex - lport interface index
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_TrunkMemberDown(UI32_T trunk_ifindex,
                                UI32_T trunk_member_ifindex);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_HandleHotInsertion
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
void MFLT_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_HandleHotRemoval
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
void MFLT_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/* MACRO FUNCTION DECLARATIONS
 */
#endif/* End of MFLT_MGR_H */
