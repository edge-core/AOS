/* MODULE NAME:  netaccess_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of netaccess group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef NETACCESS_GROUP_H
#define NETACCESS_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for NETACCESS_Group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for NETACCESS_Group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in NETACCESS_Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    All threads in the same NETACCESS group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void NETACCESS_GROUP_Create_All_Threads(void);

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_IntrusionMacCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When detecting intrusion mac, AMTR will notify other CSC groups
 *           by this function.
 *
 * INPUT   : src_lport  -- which lport
 *           vid        -- which vlan id
 *           src_mac    -- source mac address
 *           dst_mac    -- destination mac address
 *           ether_type -- ether type
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE   -- Is intrusion
 *           FALSE  -- Is not intrusion
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETACCESS_GROUP_IntrusionMacCallbackHandler(
    UI32_T  src_lport,  UI16_T  vid,    UI8_T   *src_mac,
    UI8_T   *dst_mac,   UI16_T  ether_type);

#endif /* #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_SecurityPortMoveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When port move, AMTR will notify other CSC groups by this
 *           function.
 *
 * INPUT   : ifindex          -- port whcih the mac is learnt now
 *           vid              -- which vlan id
 *           mac              -- mac address
 *           original_ifindex -- original port which the mac was learnt before
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_SecurityPortMoveCallbackHandler(UI32_T ifindex,
                                                     UI32_T vid,
                                                     UI8_T  *mac,
                                                     UI32_T original_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_AutoLearnCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When learning arrive learn_with_count, AMTR will notify other CSCs
 *           by this function.
 *
 * INPUT   : ifindex        --
 *           portsec_status --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_AutoLearnCallbackHandler(UI32_T ifindex, UI32_T portsec_status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MacAgeOutCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when one mac is aged out
 *
 * INPUT   : vid, mac, ifindex
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MacAgeOutCallbackHandler(
    UI32_T vid,     UI8_T  *mac,    UI32_T ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by port.
 *
 * INPUT   : ifindex --
 *           reason  --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByPortCallbackHandler(UI32_T ifindex, UI32_T reason);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByVidCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by vid.
 *
 * INPUT   : vid -- which vlan id
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByVidCallbackHandler(UI32_T vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByVIDnPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by vid+port.
 *
 * INPUT   : vid     -- which vlan id
 *           ifindex --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByVIDnPortCallbackHandler(UI32_T vid, UI32_T ifindex);



BOOL_T NETACCESS_GROUP_SetStaticMacCheckCallbackHandler(UI32_T vid, UI8_T *mac, UI32_T lport);
void NETACCESS_GROUP_EditAddrEntryCallbackHandler(UI32_T vid, UI8_T *mac, UI32_T lport, BOOL_T is_age);



#endif    /* End of NETACCESS_GROUP_H */

