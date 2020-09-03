/*-----------------------------------------------------------------------------
 * Module   : mflt_mgr.c
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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "l_cvrt.h"
#include "swctrl_pmgr.h"
#include "swctrl.h"
#include "amtr_pmgr.h"
#include "vlan_om.h"
#include "mflt_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define BIT2BYTE              8
#define MAC_ADDRESS_BYTES     6
#define TRUNKPORT_START       (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)


/* DATA TYPE DECLARATIONS
 */
typedef struct MFLT_MGR_Multicast_Filtering_S
{
   UI32_T   vid;
   UI8_T    mcast_mac[MAC_ADDRESS_BYTES];
   UI32_T   trunk_member_ifindex[SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM];
   UI8_T    port_ref_cnt[SYS_ADPT_TOTAL_NBR_OF_LPORT];                  /* logical port */
   struct   MFLT_MGR_Multicast_Filtering_S  *next;
} MFLT_MGR_Multicast_Filtering_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static MFLT_MGR_Multicast_Filtering_T *MFLT_MGR_GetFreeDbEntry (void);
static BOOL_T MFLT_MGR_IsLoadBalanceTrunkMember (UI32_T lport_ifindex, MFLT_MGR_Multicast_Filtering_T *ptr);
static BOOL_T MFLT_MGR_IsExistMulticastMemberSet (UI32_T vid, UI8_T *group_mac, MFLT_MGR_Multicast_Filtering_T **exist_entry_ptr);
static BOOL_T MFLT_MGR_IsEmptyMulticastMemberSet(MFLT_MGR_Multicast_Filtering_T *ptr);
static BOOL_T MFLT_MGR_AddMulticastMemberSet(UI32_T vid, UI8_T *group_mac, MFLT_MGR_Multicast_Filtering_T *insert);
static void MFLT_MGR_DelMulticastMemberSet(UI32_T vid, UI8_T *group_mac);
static BOOL_T MFLT_MGR_GetGroupMember(MFLT_MGR_Multicast_Filtering_T *ptr, UI8_T *linear_portlist, UI8_T *tagged_portlist);
static BOOL_T MFLT_MGR_LportIsTaggedPort(UI32_T vid, UI32_T lport_ifindex, BOOL_T *tagged);
static void MFLT_MGR_AddPortInPortList(UI32_T port_no, UI8_T *linear_portlist);
#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
static BOOL_T MFLT_MGR_SetUnknownIPMcastFwdPort(UI8_T *portmap);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static MFLT_MGR_Multicast_Filtering_T mflt_igmp_db[SYS_ADPT_MAX_NBR_OF_IGMP_SNOOPING_MCAST_GROUPS];
static MFLT_MGR_Multicast_Filtering_T *mflt_igmp_db_entry_ptr;
static MFLT_MGR_Multicast_Filtering_T *mflt_igmp_db_free_ptr;
#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
static UI8_T    mflt_unknown_mcast_fwd_port_ref_cnt[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
SYSFUN_DECLARE_CSC


/* MACRO DECLARATIONS
 */
#define MFLT_MGR_CHECK_ENABLE()                                             \
{                                                                           \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)   \
    {                                                                       \
        return FALSE;                                                       \
    }                                                                       \
}
#define MFLT_LOCK()
#define MFLT_UNLOCK()

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_InitiateProcessResource
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to initialize multicast filtering module.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Initialize two modules for mflt management
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_InitiateProcessResource(void)
{
    return TRUE;

}   /* MFLT_MGR_InitiateProcessResource() */

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
void MFLT_MGR_Create_InterCSC_Relation(void)
{
    return;
}

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
void MFLT_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}   /* End of MFLT_MGR_SetTransitionMode() */


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
void MFLT_MGR_EnterTransitionMode(void)
{
    UI32_T i;

    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();

    /* clear multicast filtering databese table */
    memset(mflt_igmp_db, 0, sizeof(mflt_igmp_db));

    /* make mflt_igmp_db_entry_ptr is NULL pointer when initialize */
    /* mflt_igmp_db_free_ptr is pointer to a free link-list entry */
    mflt_igmp_db_entry_ptr = (MFLT_MGR_Multicast_Filtering_T *) NULL;
    mflt_igmp_db_free_ptr  = (MFLT_MGR_Multicast_Filtering_T *) &mflt_igmp_db[0];

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_IGMP_SNOOPING_MCAST_GROUPS-1; i++)
        mflt_igmp_db[i].next = (MFLT_MGR_Multicast_Filtering_T *) &mflt_igmp_db[i+1];
    mflt_igmp_db[SYS_ADPT_MAX_NBR_OF_IGMP_SNOOPING_MCAST_GROUPS-1].next = (MFLT_MGR_Multicast_Filtering_T *) NULL;

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    memset(mflt_unknown_mcast_fwd_port_ref_cnt, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT * sizeof(UI8_T));
#endif
}   /* End of MFLT_MGR_EnterTransitionMode() */


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
void MFLT_MGR_EnterMasterMode(void)
{
#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    UI8_T   unknown_mcast_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
#endif

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    memset(unknown_mcast_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST * sizeof(UI8_T));

#if (SYS_CPNT_L2MCAST_SUPPORT_SYSTEM_UNKNOWN_MCAST_FORWARD_PORT == TRUE)
    SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(unknown_mcast_portlist);
#endif
#endif

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();
}   /* End of MFLT_MGR_EnterMasterMode() */


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
void MFLT_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();

}   /* End of MFLT_MGR_EnterSlaveMode() */


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
BOOL_T MFLT_MGR_AddGroupMember(UI32_T vid, UI8_T *group_mac, UI8_T *lportlist)
{
    MFLT_MGR_Multicast_Filtering_T *new_buffer;
    MFLT_MGR_Multicast_Filtering_T *ptr, *exist_entry_ptr;
    UI8_T   portMap[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI32_T  i;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(portMap, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);
    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    /* search current table to find this (vid, mac) pair,
     * if it's not existing, create a new one
     */
    if (MFLT_MGR_IsExistMulticastMemberSet(vid, group_mac, &exist_entry_ptr) == TRUE)
    {
        ptr = (MFLT_MGR_Multicast_Filtering_T *) exist_entry_ptr;
    }
    else
    {
        new_buffer = MFLT_MGR_GetFreeDbEntry();
        if (new_buffer == NULL)
        {
            MFLT_UNLOCK();
            return FALSE;
        }

        if (AMTR_PMGR_CreateMulticastAddrTblEntry(vid, group_mac) == FALSE)
        {
            new_buffer->next = mflt_igmp_db_free_ptr;
            mflt_igmp_db_free_ptr = new_buffer;
            MFLT_UNLOCK();
            return FALSE;
        }

        new_buffer->vid = vid;
        memcpy(new_buffer->mcast_mac, group_mac, MAC_ADDRESS_BYTES);
        memset(new_buffer->port_ref_cnt, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);

        if (MFLT_MGR_AddMulticastMemberSet(vid, group_mac, new_buffer) == TRUE)
        {
            ptr = new_buffer;
        }
        else
        {
            new_buffer->next = mflt_igmp_db_free_ptr;
            mflt_igmp_db_free_ptr = new_buffer;
            SYSFUN_Debug_Printf("\r\nMFLT_MGR_AddMulticastMemberSet 307 fail");
            MFLT_UNLOCK();
            return FALSE;
        }
    }

    /*conver bit list to byte list */
    L_CVRT_convert_portList_2_portMap(lportlist, portMap, SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if (portMap[i])
        {
            ptr->port_ref_cnt[i]++;
        }
    }

    if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
            {
                MFLT_UNLOCK();
                return FALSE;
            }

    if (AMTR_PMGR_SetMulticastPortMember(vid, group_mac, linear_portlist, tagged_portlist) == FALSE)
    {
        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if (portMap[i])
            {
                ptr->port_ref_cnt[i]--;
            }
        }

        MFLT_UNLOCK();
        return FALSE;
    }

    MFLT_UNLOCK();
    return TRUE;

}   /* End of MFLT_MGR_AddGroupMember() */


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
BOOL_T MFLT_MGR_DeleteGroupMember (UI32_T vid, UI8_T *group_mac, UI8_T *lportlist)
{
    MFLT_MGR_Multicast_Filtering_T *ptr, *exist_entry_ptr;
    UI32_T i;
    UI8_T   portMap[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(portMap, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);
    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    if (MFLT_MGR_IsExistMulticastMemberSet(vid, group_mac, &exist_entry_ptr) == FALSE)
    {
        MFLT_UNLOCK();
        return TRUE;
    }

    ptr = (MFLT_MGR_Multicast_Filtering_T *) exist_entry_ptr;

    /*conver bit list to byte list */
    L_CVRT_convert_portList_2_portMap(lportlist, portMap, SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if (portMap[i])
        {
            if (ptr->port_ref_cnt[i] > 0)
            {
                if (MFLT_MGR_IsLoadBalanceTrunkMember(i+1, ptr) == FALSE)
                ptr->port_ref_cnt[i]--;
            }
        }
    }

    if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
    {
        MFLT_UNLOCK();
        return FALSE;
    }

    /* for delete the group which no member in itself */
    if (MFLT_MGR_IsEmptyMulticastMemberSet(ptr) == TRUE)
    {
        /* destroy this multicast address table entry */
        if (AMTR_PMGR_DestroyMulticastAddrTblEntry(vid, group_mac) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
        /* remove this group entry from multicast filtering database */
        MFLT_MGR_DelMulticastMemberSet(vid, group_mac);

        MFLT_UNLOCK();
        return TRUE;
    }

    if (AMTR_PMGR_SetMulticastPortMember(vid, group_mac, linear_portlist, tagged_portlist) == FALSE)
    {
        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if (portMap[i])
            {
                ptr->port_ref_cnt[i]++;
            }
        }

        MFLT_UNLOCK();
        return FALSE;
    }

    if (MFLT_MGR_IsEmptyMulticastMemberSet(ptr) == TRUE)
    {
        /* destroy this multicast address table entry */
        if (AMTR_PMGR_DestroyMulticastAddrTblEntry(vid, group_mac) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
        /* remove this group entry from multicast filtering database */
        MFLT_MGR_DelMulticastMemberSet(vid, group_mac);
    }

    MFLT_UNLOCK();
    return TRUE;
}   /* End of MFLT_MGR_DeleteGroupMember() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_IsExistMulticastMemberSet
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to check if (vid, mac) entry existing in
 *            multicast filtering database.
 * INPUT    : vid             - vlan id
 *            group_mac       - multicast mac address
 *            exist_entry_ptr - output buffer of the exist entry
 * OUTPUT   : exist_entry_ptr - the exist entry
 * RETURN   : TRUE  - exist
 *            FALSE - not exist
 * NOTE     : 1. Aaron merge the following function together.
 *                 - MFLT_MGR_Mip_Exist_Igmp_Multicast_Member_Set
 *                 - MFLT_MGR_Mac_Exist_Igmp_Multicast_Member_Set
 *                 - MFLT_MGR_Mac_Exist_Gmrp_Multicast_Member_Set
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_IsExistMulticastMemberSet(UI32_T vid, UI8_T *group_mac, MFLT_MGR_Multicast_Filtering_T **exist_entry_ptr)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;
    for (; ptr != NULL; ptr = ptr->next)
    {
        if (ptr->vid == vid)
        {
            if (memcmp(ptr->mcast_mac, group_mac, MAC_ADDRESS_BYTES) == 0)
            {
                *exist_entry_ptr = (MFLT_MGR_Multicast_Filtering_T *) ptr;
                return TRUE;
            }
            if (memcmp(ptr->mcast_mac, group_mac, MAC_ADDRESS_BYTES) > 0)
            {
                return FALSE;
            }
        }
        if (ptr->vid > vid)
        {
            return FALSE;
        }
    }
    return FALSE;
}   /* End of MFLT_MGR_IsExistMulticastMemberSet() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_AddMulticastMemberSet
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to add new (vid, mac) multicast entry.
 * INPUT    : vid       - vlan id
 *            group_mac - multicast mac address
 *            insert
 * OUTPUT   : None
 * RETURN   : TRUE  - success
 *          : FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_AddMulticastMemberSet(UI32_T vid, UI8_T *group_mac, MFLT_MGR_Multicast_Filtering_T *insert)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;
    MFLT_MGR_Multicast_Filtering_T *pre_ptr;

    MFLT_MGR_CHECK_ENABLE();

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;
    pre_ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;
    if (ptr == NULL) /* insert first entry */
    {
        insert->next = ptr;
        mflt_igmp_db_entry_ptr = insert;
        return TRUE;
    }

    for (; ptr; pre_ptr = ptr, ptr = ptr->next)
    {
        if (vid > ptr->vid)
        {
            if (ptr->next)
                continue;
            else
            {
                ptr->next = insert;
                insert->next = NULL;
                return TRUE;
            }
        }
        else if (vid < ptr->vid)
        {
            if (ptr == pre_ptr) /* ptr == pre_ptr == mflt_igmp_db_entry_ptr */
            {
                insert->next = ptr;
                mflt_igmp_db_entry_ptr = insert;
            }
            else
            {
                insert->next = ptr;
                pre_ptr->next = insert;
            }
            return TRUE;
        }
        else if (memcmp(group_mac, ptr->mcast_mac, MAC_ADDRESS_BYTES) > 0)
        {
            if (ptr->next)
                continue;
            else
            {
                ptr->next = insert;
                insert->next = NULL;
                return TRUE;
            }
        }
        else
        {
            if (ptr == pre_ptr) /* ptr == pre_ptr == mflt_igmp_db_entry_ptr */
            {
                insert->next = ptr;
                mflt_igmp_db_entry_ptr = insert;
            }
            else
            {
                insert->next = ptr;
                pre_ptr->next = insert;
            }
            return TRUE;
        }
    }
    return FALSE;
}   /* End of MFLT_MGR_AddMulticastMemberSet() */



#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
/*-------------------------------------------------------------------------
 * FUNCTION : MFLT_MGR_ClearAllUnknownIPMcastFwdPort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to Clear unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_ClearAllUnknownIPMcastFwdPort(void)
{
    UI8_T   unknown_mcast_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    memset(mflt_unknown_mcast_fwd_port_ref_cnt, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT * sizeof(UI8_T));

    L_CVRT_convert_portMap_2_portList(mflt_unknown_mcast_fwd_port_ref_cnt,
                                        unknown_mcast_portlist,
                                        SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

#if (SYS_CPNT_L2MCAST_SUPPORT_SYSTEM_UNKNOWN_MCAST_FORWARD_PORT == TRUE)
    SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(unknown_mcast_portlist);
#endif

    MFLT_UNLOCK();

    return TRUE;
}   /* End of MFLT_MGR_ClearAllUnknownIPMcastFwdPort() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_FillAllUnknownIPMcastFwdPort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to fill unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_FillAllUnknownIPMcastFwdPort(void)
{
    UI8_T   unknown_mcast_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    memset(mflt_unknown_mcast_fwd_port_ref_cnt, 1, SYS_ADPT_TOTAL_NBR_OF_LPORT * sizeof(UI8_T));
    L_CVRT_convert_portMap_2_portList(mflt_unknown_mcast_fwd_port_ref_cnt,
                                        unknown_mcast_portlist,
                                        SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

#if (SYS_CPNT_L2MCAST_SUPPORT_SYSTEM_UNKNOWN_MCAST_FORWARD_PORT == TRUE)
    SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(unknown_mcast_portlist);
#endif

    MFLT_UNLOCK();

    return TRUE;
}   /* End of MFLT_MGR_FillAllUnknownIPMcastFwdPort() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_AddUnknownIPMcastFwdPort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to add a unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : lport  - port want to add unknown multicast forwarding port
 * OUTPUT   : None
 * RETURN   : TRUE   - if entry is exist and delete
 *            FALSE  - if not exist or error
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_AddUnknownIPMcastFwdPort(UI32_T lport)
{
    if (SWCTRL_LogicalPortExisting(lport) != TRUE)
    {
        return FALSE;
    }


    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    if (mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1] != 0)
    {   /* if reference count is non-zero, we don't need to do more */
        mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1]++;
        MFLT_UNLOCK();
        return TRUE;
    }

    mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1]++;

    if (MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt) != TRUE)
    {   /* if set fail, return to original status */
        mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1]--;
        MFLT_UNLOCK();
        return FALSE;
    }

    MFLT_UNLOCK();
    return TRUE;
}   /* End of MFLT_MGR_AddUnknownIPMcastFwdPort() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_DelUnknownIPMcastFwdPort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to add a unknown multicast forwarding port.
 *            For some customer's requirement and chip's limitation.
 * INPUT    : port  - port want to add unknown multicast forwarding port
 * OUTPUT   : None
 * RETURN   : TRUE  - if entry is exist and delete
 *            FALSE - if not exist or error
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T MFLT_MGR_DelUnknownIPMcastFwdPort(UI32_T lport)
{
    if (SWCTRL_LogicalPortExisting(lport) != TRUE)
    {
        return FALSE;
    }


    MFLT_MGR_CHECK_ENABLE();
    MFLT_LOCK();

    if (mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1] == 0)
    {   /* if original reference count is zero,
        there must have something wrong. we don't need to do more */
        MFLT_UNLOCK();
        return TRUE;
    }

    mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1]--;

    if (mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1] != 0)
    {   /* if reference count is non-zero, we don't need to do more */
        MFLT_UNLOCK();
        return TRUE;
    }

    /* if reference count is zero, we must remove this port from portlist */
    if (MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt) != TRUE)
    {   /* if set fail, return to original status */
        mflt_unknown_mcast_fwd_port_ref_cnt[lport - 1]++;
        MFLT_UNLOCK();
        return FALSE;
    }

    MFLT_UNLOCK();
    return TRUE;
}   /* End of MFLT_MGR_DelUnknownIPMcastFwdPort() */
#endif


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_GetFreeDbEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : Get a free db entry from multicast db pool.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : not 0 - get a free db entry pointer.
 *          : 0     - no free db entry
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static MFLT_MGR_Multicast_Filtering_T *MFLT_MGR_GetFreeDbEntry (void)
{
    MFLT_MGR_Multicast_Filtering_T *ret_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return 0;
    }

    if (mflt_igmp_db_free_ptr)
    {
        ret_entry = mflt_igmp_db_free_ptr;
        mflt_igmp_db_free_ptr = mflt_igmp_db_free_ptr->next;
        ret_entry->next = NULL;
        return ret_entry;
    }
    else
    {
        return 0; /* No free Db entry */
    }

}   /* End of MFLT_MGR_GetFreeDbEntry() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_LportIsTaggedPort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to check if the port is a tagged port.
 * INPUT    : vid           - vlan id
 *            lport_ifindex - the logical port being checked
 * OUTPUT   : TRUE  - tagged
 *            FALSE - untagged
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_LportIsTaggedPort(UI32_T vid, UI32_T lport_ifindex, BOOL_T *tagged)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T  bit;

    if (lport_ifindex < 1 || lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        return FALSE;
    }

    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info) == FALSE)
    {
        SYSFUN_Debug_Printf("\r\nMFLT_MGR VLAN_PMGR_GetDot1qVlanCurrentEntry 833 FALSE");
        return FALSE;
    }

    bit = BIT2BYTE - (lport_ifindex % BIT2BYTE);

    if (bit == BIT2BYTE)
        bit = 0;

    if ((vlan_info.dot1q_vlan_current_untagged_ports[lport_ifindex/BIT2BYTE - (lport_ifindex % BIT2BYTE == 0)]) & (0x01 << bit))
    {
        *tagged = FALSE;
    }
    else
    {
        *tagged = TRUE;
    }

    return TRUE;

}   /* End of MFLT_MGR_LportIsTaggedPort() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_AddPortInPortList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to add one port to linear port list.
 * INPUT    : port_no          - port number
 *            *linear_portlist - linear port list buffer
 * OUTPUT   : *linear_portlist - linear port list
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void MFLT_MGR_AddPortInPortList(UI32_T port_no, UI8_T *linear_portlist)
{
    linear_portlist[(port_no-1)/8] |= ( 0x80 >> (port_no-1)%8);

    return;

}   /* End of MFLT_MGR_AddPortInPortList() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_IsLoadBalanceTrunkMember
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to check if the port is a trunk member used
 *            to do load balance of specified IGMP/GMRP multicast filtering
 *            database entry on a given VLAN.
 * INPUT    : lport_ifindex - the logical port would be checked
 *            ptr
 * OUTPUT   : None
 * RETURN   : TRUE  - empty
 *            FALSE - not empty
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_IsLoadBalanceTrunkMember(UI32_T lport_ifindex, MFLT_MGR_Multicast_Filtering_T *ptr)
{
    UI32_T i;

    if (lport_ifindex < 1 || lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        SYSFUN_Debug_Printf("\r\nMFLT_MGR_IsLoadBalanceTrunkMember 897 lport_ifindex: %ld", lport_ifindex);
        return FALSE;
    }

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
    {
        if (ptr->trunk_member_ifindex[i] == lport_ifindex)
            return TRUE;
    }

    return FALSE;
}   /* End of MFLT_MGR_IsLoadBalanceTrunkMember() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_IsEmptyMulticastMemberSet
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns true if this vid/mac multicast entry
 *            has no member. Otherwise, false is returned.
 * INPUT    : ptr
 * OUTPUT   : None
 * RETURN   : TRUE  - empty
 *            FALSE - not empty
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_IsEmptyMulticastMemberSet(MFLT_MGR_Multicast_Filtering_T *ptr)
{
    UI32_T i;

    for (i = 1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if (ptr->port_ref_cnt[i-1] != 0)
        {
             return FALSE;
        }
    }
    return TRUE;
}   /* End of MFLT_MGR_IsEmptyMulticastMemberSet() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_DelMulticastMemberSet
 * ------------------------------------------------------------------------
 * PURPOSE  : This function removes this group entry from multicast
 *            filtering database.
 * INPUT    : vid       - vlan id
 *            group_mac - multicast address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void MFLT_MGR_DelMulticastMemberSet(UI32_T vid, UI8_T *group_mac)
{
    MFLT_MGR_Multicast_Filtering_T *pre_ptr, *ptr;

    pre_ptr = (MFLT_MGR_Multicast_Filtering_T *) NULL;
    ptr = mflt_igmp_db_entry_ptr;

    for (; ptr !=NULL ; pre_ptr = ptr, ptr = ptr->next)
    {
        if ((memcmp(group_mac, ptr->mcast_mac, 6) == 0) &&
           (ptr->vid == vid))
            break;
    }
    if (ptr == NULL)
    {
        return;
    }

    if (pre_ptr == NULL)
        mflt_igmp_db_entry_ptr = ptr->next;
    else
        pre_ptr->next = ptr->next;

    memset(ptr, 0, sizeof(MFLT_MGR_Multicast_Filtering_T));

    ptr->next = mflt_igmp_db_free_ptr;
    mflt_igmp_db_free_ptr = ptr;

    return;
}   /* End of MFLT_MGR_DelMulticastMemberSet() */


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
BOOL_T MFLT_MGR_AddTrunkMember(UI32_T trunk_ifindex, UI32_T trunk_member_ifindex)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_LOCK();

    if (trunk_ifindex < TRUNKPORT_START ||
        trunk_ifindex >= (TRUNKPORT_START + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        MFLT_UNLOCK();
        return FALSE;
    }

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;

    for (; ptr != NULL; ptr = ptr->next)
    {
        if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }

        if (AMTR_PMGR_SetMulticastPortMember(ptr->vid, ptr->mcast_mac, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
    }

    MFLT_UNLOCK();
    return TRUE;

}   /* End of MFLT_MGR_AddTrunkMember() */


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
BOOL_T MFLT_MGR_DeleteTrunkMember(UI32_T trunk_ifindex, UI32_T trunk_member_ifindex)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_LOCK();

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt);
#endif

    if (trunk_ifindex < TRUNKPORT_START ||
        trunk_ifindex >= (TRUNKPORT_START + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        MFLT_UNLOCK();
        return FALSE;
    }

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;

    for (; ptr != NULL; ptr = ptr->next)
    {
        if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }

        if (AMTR_PMGR_SetMulticastPortMember(ptr->vid, ptr->mcast_mac, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
    }

    MFLT_UNLOCK();
    return TRUE;

}   /* End of MFLT_MGR_DeleteTrunkMember() */


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
BOOL_T MFLT_MGR_TrunkMemberUp(UI32_T trunk_ifindex, UI32_T trunk_member_ifindex)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_LOCK();

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt);
#endif

    if (trunk_ifindex < TRUNKPORT_START ||
        trunk_ifindex >= (TRUNKPORT_START + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        MFLT_UNLOCK();
        return FALSE;
    }

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;

    for (; ptr != NULL; ptr = ptr->next)
    {
        if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }

        if (AMTR_PMGR_SetMulticastPortMember(ptr->vid, ptr->mcast_mac, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
    }

    MFLT_UNLOCK();
    return TRUE;

}   /* End of MFLT_MGR_TrunkMemberUp() */


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
BOOL_T MFLT_MGR_TrunkMemberDown(UI32_T trunk_ifindex, UI32_T trunk_member_ifindex)
{
    MFLT_MGR_Multicast_Filtering_T *ptr;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    MFLT_LOCK();

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt);
#endif

    if (trunk_ifindex < TRUNKPORT_START ||
        trunk_ifindex >= (TRUNKPORT_START + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
    {
        MFLT_UNLOCK();
        return FALSE;
    }

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;

    for (; ptr != NULL; ptr = ptr->next)
    {

        if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }

        if (AMTR_PMGR_SetMulticastPortMember(ptr->vid, ptr->mcast_mac, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return FALSE;
        }
    }

    MFLT_UNLOCK();
    return TRUE;
}   /* End of MFLT_MGR_TrunkMemberDown() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_GetGroupMember
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is to get multicast members.
 * INPUT    : *ptr              - multicast entry point
 *            *linear_portlist  - portlist buffer
 *            *tagged_portlist  - portlist buffer
 * OUTPUT   : linear_portlist   - existing portlist
 *            tagged_portlist   - tagged portlist
 * RETURN   : TRUE  - success
 *            FALSE - failure
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MFLT_MGR_GetGroupMember(MFLT_MGR_Multicast_Filtering_T *ptr, UI8_T *linear_portlist, UI8_T *tagged_portlist)
{
    UI32_T i;
    BOOL_T tag = TRUE;

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if (ptr->port_ref_cnt[i] > 0)
        {
            if (MFLT_MGR_LportIsTaggedPort(ptr->vid, i+1, &tag) == FALSE)
            {
                MFLT_UNLOCK();
                return FALSE;
            }

            if (tag == TRUE)
                MFLT_MGR_AddPortInPortList(i+1, tagged_portlist);

            MFLT_MGR_AddPortInPortList(i+1, linear_portlist);


            if (SWCTRL_LogicalPortIsTrunkPort(i+1) == TRUE)
            {
#if (SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM == SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ONE_TRUNK_MEMBER)

                UI32_T  trunk_member_ifindex, algorithm = 1;

                if (SWCTRL_L2LoadBalance(ptr->mcast_mac, i+1, algorithm, &trunk_member_ifindex) == TRUE)
                {
                    MFLT_MGR_AddPortInPortList(trunk_member_ifindex, linear_portlist);

                    if (tag == TRUE)
                        MFLT_MGR_AddPortInPortList(trunk_member_ifindex, tagged_portlist);
                }

#elif (SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM == SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ALL_TRUNK_MEMBER)
{
                UI32_T  k, port_count;
                UI32_T  active_lportlist[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];

                if (SWCTRL_GetActiveTrunkMember(i+1, active_lportlist, &port_count) == TRUE)
                {
                    for (k = 0; k < port_count; k++)
                    {
                        MFLT_MGR_AddPortInPortList(active_lportlist[k], linear_portlist);

                        if (tag == TRUE)
                            MFLT_MGR_AddPortInPortList(active_lportlist[k], tagged_portlist);
                    }
                }
}
#endif
            }
        }
    }

    return TRUE;

}   /* End of MFLT_MGR_GetGroupMember() */

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
void MFLT_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* will re-add all entry, we just re-add all entries. It will create null entries in hot-insert chip */
    MFLT_MGR_Multicast_Filtering_T *ptr;
    UI8_T   tagged_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   linear_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(linear_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(tagged_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return;
    }

    MFLT_LOCK();

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)
    MFLT_MGR_SetUnknownIPMcastFwdPort(mflt_unknown_mcast_fwd_port_ref_cnt);
#endif

    ptr = (MFLT_MGR_Multicast_Filtering_T *) mflt_igmp_db_entry_ptr;

    for (; ptr != NULL; ptr = ptr->next)
    {
        if (MFLT_MGR_GetGroupMember(ptr, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return;
        }

        if (AMTR_PMGR_SetMulticastPortMember(ptr->vid, ptr->mcast_mac, linear_portlist, tagged_portlist) == FALSE)
        {
            MFLT_UNLOCK();
            return;
        }
    }

    MFLT_UNLOCK();
    return;

} /* MFLT_MGR_HandleHotInsertion() */


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
void MFLT_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* hot remove is done by IGMPSNP, it will simulate port link down */

    return;
} /* MFLT_MGR_HandleHotRemoval() */

#if (SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET == SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_MGR_SetUnknownIPMcastFwdPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Use to set unknown ip mcast forwarding portlist
 * INPUT    : portmap - portmap of unknown multicast forwarding ports
 * OUTPUT   : None.
 * RETUEN   : TRUE  - success
 *          : FALSE - failure
 * NOTES    : None.
 */
static BOOL_T MFLT_MGR_SetUnknownIPMcastFwdPort(UI8_T *portmap_p)
{

    UI8_T   uport_portmap[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI8_T   uport_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  lport, trunk_member;
    SWCTRL_TrunkPortExtInfo_T   trunk_port_ext_info;
    UI32_T   member_arrary[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI32_T  active_count;

    memset(uport_portmap, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT * sizeof(UI8_T));

    for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (portmap_p[lport-1] != 0)
        {
#if 0
            if (SWCTRL_LogicalPortIsTrunkPort(lport) != TRUE)
            {   /* port is not trunk port, we just need to add it in uport_portmap*/
                uport_portmap[lport-1] = 1;
                continue;
            }

            memset(&trunk_port_ext_info, 0, sizeof(SWCTRL_TrunkPortExtInfo_T));
            /* port is trunk port, we will get all trunk member and add them in
               unknown IP multicast forwarding ports */
            if (SWCTRL_GetActiveTrunkMember(lport, member_arrary, &active_count) != TRUE)
            {   /* get trunk information fail */
                continue;
            }

            for (trunk_member = 0; trunk_member < active_count; trunk_member++)
            {
                uport_portmap[member_arrary[trunk_member] - 1] = 1;
            }
#endif
        }
    }

    memset(uport_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    L_CVRT_convert_portMap_2_portList(portmap_p, uport_portlist,
                                        SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

#if (SYS_CPNT_L2MCAST_SUPPORT_SYSTEM_UNKNOWN_MCAST_FORWARD_PORT == TRUE)
    if (SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(uport_portlist) != TRUE)
    {
        return FALSE;
    }
#endif

    return TRUE;
}   /* End of MFLT_MGR_SetUnknownIPMcastFwdPort() */
#endif
