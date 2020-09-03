/* MODULE NAME: mldsnp_unknown.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*     this file is used to collect the function which process
*     the unknown multicast data.
*    {2. Related documents or hardware information}
*NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*                ++++++++++++
*                +mlsdnp_mgr+
*                ++++++++++++
*
*                 ++++++++++++++++
*                 + mldsnp_engine+
*                 ++++++++++++++++
*
*         ++++++++++++++++++        ++++++++++++++++++
*         + mldsnp_unknown +        +mldsnp_querier  +
*         ++++++++++++++++++        ++++++++++++++++++
*
* +++++++++++
* +mldsnp_om+
* +++++++++++
*                              +++++
*                              +msl+
*                              +++++
*
*    1. Unknown entry will register the receive data port to chip and other port accroding to flooding behavior.
*    2. Unknow entry will only register the receiver data port to om.
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_unknown.h"
#include "mldsnp_om.h"
#include "mldsnp_timer.h"
#include "mldsnp_backdoor.h"
#include "swctrl.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "msl_pmgr.h"
/* NAMING CONSTANT DECLARATIONS
*/
#define GROUP_LEN MLDSNP_TYPE_IPV6_DST_IP_LEN
#define SRC_IP_LEN MLDSNP_TYPE_IPV6_SRC_IP_LEN
/*if chip have ability to flood unknown multicast data, this define can be set or don't set this, let mldsnp to write flood entry*/
#if (SYS_CPNT_SWDRV_TRAP_UNKNOWN_IPV6MC_BY_RULE == FALSE)
#define MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
#endif
/* MACRO FUNCTION DECLARATIONS
*/
/* DATA TYPE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
/* STATIC VARIABLE DEFINITIONS
*/
extern UI8_T mldsnp_om_null_src_ip_a[SYS_ADPT_IPV6_ADDR_LEN];
extern UI8_T mldsnp_om_null_group_ip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];

/* LOCAL SUBPROGRAM BODIES
*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_ExitUnknownRegisterGroup
*------------------------------------------------------------------------------
* Purpose: this function check the hisam entry has unknown registered port or not
* INPUT  : entry_p - the hisam entry pointer for check this entry has unknown registered port
* OUTPUT : None
* RETURN : TRUE - exit port register unknown data
*          FALSE- doesn't exit port register unknown data
* NOTES  : search all port in (vid, gip, sip) to find a router port is registered as unknown
*------------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_ExitUnknownRegisterGroup(
    MLDSNP_OM_HisamEntry_T *entry_p)

{
    MLDSNP_OM_PortInfo_T   port_info;

    port_info.port_no = 0;
    while (TRUE == L_SORT_LST_Get_Next(&entry_p->register_port_list, &port_info))
    {
        if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type)
        {
            MLDSNP_BD(TRACE, "exit unknown multicast data registered group ");
            return TRUE;
        }
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_AddPortToChipEntry
*-------------------------------------------------------------------------
* PURPOSE : add one port into the chip enty
* INPUT   : vid        - the input vid
*           *gip_ap   - the group ip address
*           *sip_ap    - the sip address
*           lport      - the logical port
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_AddPortToChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport)
{
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
#endif

    if (0 == lport)
        msl_pmgr_mldsnp_entry_add(0,  group_ip, source_ip, vid, 0, lport);
    else
        msl_pmgr_mldsnp_entry_add(0,  group_ip, source_ip, vid, 1, lport);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port %d",
                             group_ip, source_ip, 1, vid, lport);

    return TRUE;
}/*MLDSNP_UNKNOWN_AddPortToChipEntry*/

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_AddPortBitmapToChipEntry
*-------------------------------------------------------------------------
* PURPOSE : add one port into the chip enty
* INPUT   : vid           - the input vid
*           *gip_ap       - the group ip address
*           *sip_ap       - the sip address
*           port_bitmap_a - the port bitmap
* OUTPUT  : None
* RETURN  : None
* NOTE    : replace port_bitmap
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_AddPortBitmapToChipEntry(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_bitmap_ap)
{
    UI16_T out_port_a[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI32_T b = 0, i = 0, j = 0;
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
#endif

    for (; b < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; b++)
    {
        if (0 != port_bitmap_ap[b])
        {
            for (i = 1;i <= 8; i++)
            {
                if (port_bitmap_ap[b]&(0x80 >> (i - 1) % 8))
                {
                    out_port_a[j] = b * 8 + i;
                    j++;
                }
            }
        }
    }

    msl_pmgr_mldsnp_entry_add_portlist(0, group_ip, source_ip, vid, j, out_port_a);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port bitmap: %02x%02x%02x",
                             group_ip, source_ip, 1, vid, port_bitmap_ap[0], port_bitmap_ap[1], port_bitmap_ap[2]);

    return TRUE;
}/*MLDSNP_UNKNOWN_AddPortBitmapToChipEntry*/

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_DeletePortFromChipEntry
*-------------------------------------------------------------------------
* PURPOSE : Delete one port from the chip enty
* INPUT   : vid        - the input vid
*           *gip_ap    - the group ip address
*           *sip_ap    - the sip address
*           lport      - the logical port
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_DeletePortFromChipEntry(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T  lport)
{
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
#endif

    if (0 == lport)
        msl_pmgr_mldsnp_entry_del(0,  group_ip, source_ip, vid, 0, lport);
    else
        msl_pmgr_mldsnp_entry_del(0,  group_ip, source_ip, vid, 1, lport);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port %d",
                             group_ip, source_ip, 1, vid, lport);
    return TRUE;
}/*end of MLDSNP_UNKNOWN_DeletePortFromChipEntry*/

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry
*-------------------------------------------------------------------------
* PURPOSE : Delete one ports from the chip enty
* INPUT   : vid            - the input vid
*           *gip_ap        - the group ip address
*           *port_bitmap_a - the port bitmap
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_bitmap_ap)
{
    UI16_T out_port_a[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI32_T b = 0, i = 0, j = 0;
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
#endif

    for (;b < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; b++)
    {
        if (0 != port_bitmap_ap[b])
        {
            for (i = 1;i <= 8; i++)
            {
                if (port_bitmap_ap[b]&(0x80 >> (i - 1) % 8))
                {
                    out_port_a[j] = b * 8 + i;
                    j++;
                }
            }
        }
    }

    msl_pmgr_mldsnp_entry_del_portlist(0, group_ip, source_ip, vid, j, out_port_a);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port bitmap: %02x%02x%02x",
                             group_ip, source_ip, 1, vid, port_bitmap_ap[0], port_bitmap_ap[1], port_bitmap_ap[2]);
    return TRUE;
}/*end of MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_JoinVidGipEntry
*------------------------------------------------------------------------------
* Purpose: this function add all (vid, gip, 0) joined port to (vid, gip, sip)
* INPUT  :  vid - vlan id to check
*           gip_ap - group address array pointer
*           sip_ap - source address array pointer
* OUTPUT : None
* RETURN : TRUE - have (vid, gip, 0) entry and add sucessfully
*          FALSE- don't have (vid, gip, 0)
* NOTES  :The problem is here, if chip won't support (*,G) lookup
*         1. the (vid, gip, 0) won't forward (vid, gip, sip) data
*         so, we robustly join all port in (vid, gip, 0) to (vid, gip, sip)
*         2. but when report (*,g) then data receive (S,G) then report (*,G) the (S,G) entry will be deleted.
*         although it won't have any problem, because (S,G) will be created again after receive (S,G) data.
*------------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_UNKNOWN_JoinVidGipEntry(
    UI16_T vid,
    UI16_T input_port,
    UI8_T *gip_ap,
    UI8_T *sip_ap)
{
#if 1 /*if this chip support (*,G) lookup, we only need to check the (0,G) exist or not*/
    MLDSNP_OM_HisamEntry_T entry_info;

    MLDSNP_BD(TRACE, " ");

    memset(&entry_info, 0, sizeof(MLDSNP_OM_HisamEntry_T));
    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        return TRUE;
    }

    return FALSE;
#else /*if this chip don't support, (*,G)loopk up, it shall enable below*/
    MLDSNP_OM_PortInfo_T port_info_p, new_port_info;
    UI32_T time_left = 0;
    UI16_T  next_lport       = 0;
    BOOL_T has_vid_gip_0 = FALSE;
    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    next_lport = 0;

    while (TRUE == MLDSNP_OM_GetNextPortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, &next_lport, &port_info_p))
    {
        MLDSNP_BD_SHOW_GROUP_SRC(RX, "exist port (vid, gip, 0), vid=%d, port=%d, type=%d, so robustly join (vid, gip, sip)",
                                 gip_ap, sip_ap, 1, vid, next_lport, port_info_p.list_type);
        /*use dymamic means list inlcude report join this (vid, gip, sip) ,use request list, because only exlude mode has (vid, gip, 0) entry*/
        MLDSNP_OM_InitialportInfo(next_lport, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &new_port_info);
        MLDSNP_TYPE_AddPortIntoPortBitMap(input_port, port_bitmap_a);
        /*we set this (vid, gip, sip) entry to be aged out with (*,G)*/
        MLDSNP_TIMER_QueryTimer(port_info_p.filter_timer_p, &time_left);

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                vid,
                gip_ap,
                sip_ap,
                1,
                next_lport,
                time_left,
                MLDSNP_TIMER_ONE_TIME,
                &new_port_info.src_timer_p))
        {
            return FALSE;
        }

        if (FALSE == MLDSNP_OM_AddPortInfo(vid, gip_ap, sip_ap, &new_port_info))
        {
            MLDSNP_BD_ARG(RX, "can't robust add port %d to join\r\n", next_lport);
            return FALSE;
        }

        has_vid_gip_0 = TRUE;
    }

    if (TRUE == has_vid_gip_0)
        MLDSNP_UNKNOWN_AddPortBitmapToChipEntry(vid, gip_ap, sip_ap, port_bitmap_a);

    return has_vid_gip_0;
#endif
}/*End of MLDSNP_UNKNOWN_JoinVidGipEntry*/

/* EXPORTED SUBPROGRAM BODIES
*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_HandleJoinReport
*------------------------------------------------------------------------------
* Purpose: This function process a port join a group but there is a unknown entry already.
* INPUT  : vid      - the vlan id
*         lport     - the logical port
*         *gip_ap   - the group ip array pointer
*         *sip_ap   - the source ip array pointer
*         list_type - the list type for this (vid, gip, sip)
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_HandleJoinReport(
    UI16_T                 vid,
    UI16_T                 lport,
    UI8_T                  *gip_ap,
    UI8_T                  *sip_ap,
    MLDSNP_TYPE_ListType_T list_type)
{
    MLDSNP_OM_HisamEntry_T        entry_info;
    MLDSNP_OM_PortInfo_T          port_info;
    UI16_T nxt_lport = 0;
    BOOL_T has_unknow_port = FALSE;

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, lport %d", gip_ap, sip_ap, 1, vid, lport);

    if (0 == memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
    {
        UI16_T nxt_vid = vid;
        UI8_T  nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

        MLDSNP_BD(TRACE, "Null src ip, clear (vid, gip, *)");

        /*src empty means deleting (vid, gip, *)*/
        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (nxt_vid != vid
                    || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            nxt_lport = 0;
            while (TRUE == MLDSNP_OM_GetNextPortInfo(nxt_vid, nxt_gip_a, nxt_sip_a, &nxt_lport, &port_info)) /*don't Use GetNextPortFromHisam, because here we will delete prot*/
            {
                if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type)
                {
                    has_unknow_port = TRUE;
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                    /*delete port from om*/
                    if (FALSE == MLDSNP_OM_DeletePortInfo(nxt_vid, nxt_gip_a, nxt_sip_a, port_info.port_no)) /*note: here the hisame is updated, but process entry_info is not updated*/
                    {
                        return FALSE;
                    }

                    /*delet this port from chip*/
                    if (FALSE == MLDSNP_UNKNOWN_DeletePortFromChipEntry(nxt_vid, nxt_gip_a, nxt_sip_a, port_info.port_no))
                    {
                        return FALSE;
                    }
                }
            }

            if (TRUE == has_unknow_port)
                MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(nxt_vid, nxt_gip_a, nxt_sip_a);

            has_unknow_port = FALSE;
        }
    }
    else
    {
        MLDSNP_BD(TRACE, "clear (vid, gip, sip)");

        /*src not empty means deleting (vid, gip, sip)*/
        if (FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            return TRUE;
        }

        while (TRUE == MLDSNP_OM_GetNextPortInfo(vid, gip_ap, sip_ap, &nxt_lport, &port_info))/*don't Use GetNextPortFromHisam, because here we will delete prot*/
        {
            if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type)
            {
                has_unknow_port = TRUE;
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                /*delete port from om */
                if (FALSE == MLDSNP_OM_DeletePortInfo(vid, gip_ap, sip_ap, port_info.port_no)) /*note: here the hisame is updated, but process entry_info is not updated*/
                {
                    return FALSE;
                }

                /*delet this port from chip*/
                if (FALSE == MLDSNP_UNKNOWN_DeletePortFromChipEntry(vid, gip_ap, sip_ap, port_info.port_no))
                {
                    return FALSE;
                }
            }
        }

        if (TRUE == has_unknow_port)
            MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(vid, gip_ap, sip_ap);
    }

    return TRUE;
}/*End of MLDSNP_UNKNOWN_HandleJoinReport*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_ProcessUnknownMcastData
*------------------------------------------------------------------------------
* Purpose: This function process the unknown multicast data
* INPUT  :*unknown_data_p - the packet content
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  : 1. this function use the ip header's dip and sip to as hisam key.
*         because this is multicast data not icmp's report
*         2. unknown multicast data only register at (vid, gip, sip)
*         3. it shall only one port register for one (vid, gip, sip)?? macauley
*             because two port recevie same group stream is a big problem
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_ProcessUnknownMcastData(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
    MLDSNP_OM_HisamEntry_T        entry_info;
    MLDSNP_OM_PortInfo_T          port_info;
    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, " ");

    /*get flood behavior*/
    MLDSNP_OM_GetUnknownFloodBehavior(msg_p->vid, &flood_behavior);

#ifdef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    /*add router ports*/
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == flood_behavior)
    {
        MLDSNP_BD(ERR, "flood mode won't trap packet to cpu");
        return TRUE;
    }
#endif

    /*check and join (vid, gip, 0)*/
    if (TRUE ==  MLDSNP_UNKNOWN_JoinVidGipEntry(msg_p->vid, msg_p->recevied_port, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a))
    {
        MLDSNP_BD_SHOW_GROUP(ERR, "(%ld, grp, 0) already joined",  msg_p->ipv6_header_p->dip_a, msg_p->vid);
        return TRUE;
    }

    /*
      1. (vid, gip, sip) has been registered
            i. unknown -> update timer
            ii. exluce list -> if some port join include_list then go to iii. else to router port
            iii. include list -> chip not forwarded? -> drop
       2. (vid, gip, sip) has not been registered
            i.  register router port, if there is no router port, only create entry, if this is to-router port behavior
            ii. register all port in this vlan, if this is flood behavior
     */

    /* 1.i*/
    if (TRUE == MLDSNP_OM_GetPortInfo(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, msg_p->recevied_port, &port_info))
    {
        if (MLDSNP_TYPE_JOIN_UNKNOWN != port_info.join_type)
        {
            MLDSNP_BD_SHOW_GROUP_SRC(RX, "already exist this vid=%d, port=%d, type=%d",
                                     msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, 1,
                                     msg_p->vid, msg_p->recevied_port, port_info.list_type);
            return TRUE;
        }

        if (NULL != port_info.src_timer_p)
        {
            MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, MLDSNP_TYPE_DFLT_UNKNOWN_TIMEOUT);
        }

        return TRUE;
    }

    /*make sure this is second port want to join this unknown entry, */
    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        MLDSNP_BD(ERR, "there is second port %d want to join already exist entry", msg_p->recevied_port);
        return FALSE;
    }

    /*1.ii*/

    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&entry_info.register_port_list, &port_info))
        {
            if (MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type)
            {
                /*write chip to forward this stream to router port*/
                MLDSNP_OM_RouterPortInfo_T r_port_info;
                MLDSNP_OM_VlanInfo_T       vlan_info;

                MLDSNP_BD_SHOW_GROUP_SRC(RX, "some port join group vid:%d, port:%d, type:%d",
                                         msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, 1, msg_p->vid, msg_p->recevied_port, port_info.list_type);

                if (FALSE == MLDSNP_OM_GetVlanInfo(msg_p->vid, &vlan_info))
                {
                    return FALSE;
                }

                /*add all router port and receiver port to port bitmap to write to chip
                   and doesn't record the router port into om
                  */
                r_port_info.port_no = 0;
                while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
                {
                    MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, port_bitmap_a);
                }

                if (FALSE == MLDSNP_UNKNOWN_AddPortBitmapToChipEntry(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, port_bitmap_a))
                {
                    return FALSE;
                }
            }
            else
            { /*1.iii*/
                MLDSNP_BD(ERR, "chip has write his entry but still trap to cpu");
                return TRUE;
            }
        }
        return TRUE;
    }

    /* 2. No port join (vid, gip, sip), join all router port or MLDSNP_TYPE_NULL_ROUTER_PORT if there is no router port exist
     */
    /*register the received port*/
    MLDSNP_OM_InitialportInfo(msg_p->recevied_port, MLDSNP_TYPE_JOIN_UNKNOWN, MLDSNP_TYPE_LIST_INCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);
    MLDSNP_TYPE_AddPortIntoPortBitMap(msg_p->recevied_port, port_bitmap_a);

    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout,
            msg_p->vid,
            msg_p->ipv6_header_p->dip_a,
            msg_p->ipv6_header_p->sip_a,
            1,
            msg_p->recevied_port,
            MLDSNP_TYPE_DFLT_UNKNOWN_TIMEOUT,
            MLDSNP_TIMER_ONE_TIME,
            &port_info.src_timer_p))
    {
        return FALSE;
    }

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (FALSE == MLDSNP_OM_AddPortInfo(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, &port_info))
#else
    if (FALSE == MLDSNP_OM_AddPortInfo(msg_p->vid, msg_p->ipv6_header_p->dip_a, NULL, &port_info))
#endif
    {
        MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
        MLDSNP_BD_ARG(RX, "can't register the unknwon data received port %d\r\n", msg_p->recevied_port);
        return FALSE;
    }

    /*add router ports*/
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == flood_behavior)
    {
        MLDSNP_OM_RouterPortInfo_T r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        MLDSNP_BD(RX, " flood to router port");

        if (FALSE == MLDSNP_OM_GetVlanInfo(msg_p->vid, &vlan_info))
        {
            MLDSNP_BD(RX, "no such vid %d exist ", msg_p->vid);
            return FALSE;
        }

        /*add all router port and receiver port to port bitmap to write to chip
           and doesn't record the router port into om
          */
        r_port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, port_bitmap_a);
        }
    }
#ifndef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    else/*MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD, add all ports*/
    {
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

        MLDSNP_BD(RX, " flood to all port");

        if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, msg_p->vid, &vlan_info))
        {
            return FALSE;
        }

        memcpy(port_bitmap_a, vlan_info.dot1q_vlan_current_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }
#endif
    /*
      write to chip
    */
    if (FALSE == MLDSNP_UNKNOWN_AddPortBitmapToChipEntry(msg_p->vid, msg_p->ipv6_header_p->dip_a, msg_p->ipv6_header_p->sip_a, port_bitmap_a))
    {
        return FALSE;
    }
    return TRUE;
}/*End of MLDSNP_UNKNOWN_ProcessUnknownMcastData*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_UNKNOWN_SetFloodBehavior
 *-------------------------------------------------------------------------
 * PURPOSE : This function isto set the unknown flood behavior
 * INPUT   : flood_behavior   - the flood behavior
 *           vlan_id          - which vlan
 * OUTPUT  : None
 * RETURN  :
 * NOTE    :  1. search all (vid, gip, sip) and its router port, if exsit unknown registed, delete the port
 *               if router port dosn't exist, we register port 0 to the group
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_UNKNOWN_SetFloodBehavior(
    UI32_T vlan_id,
    UI32_T new_flood_behavior)
{
    MLDSNP_TYPE_UnknownBehavior_T   ori_flood_behavior = 0;
    MLDSNP_OM_HisamEntry_T          entry_info;
    UI16_T nxt_vid = vlan_id;
    UI8_T nxt_sip_a[SRC_IP_LEN] = {0}, nxt_gip_a[GROUP_LEN] = {0};
#ifndef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
#endif

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetUnknownFloodBehavior(vlan_id, &ori_flood_behavior);
    /*check the same status*/
    if (new_flood_behavior == ori_flood_behavior)
    {
        return TRUE;
    }

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if(nxt_vid!=vlan_id)
          break;

        if (FALSE == MLDSNP_UNKNOWN_ExitUnknownRegisterGroup(&entry_info))
        {
            continue;
        }

#ifdef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
        /*remove all enetry*/
        MLDSNP_UNKNOWN_HandleJoinReport(nxt_vid, 0,  nxt_gip_a, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_LIST_INCLUDE);
#else
        if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, nxt_vid, &vlan_entry))
        {
            continue;
        }

        memcpy(port_bitmap_a, vlan_entry.dot1q_vlan_current_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

        if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == new_flood_behavior)
        {
            /*
              write to chip
            */
            if (FALSE == MLDSNP_UNKNOWN_AddPortBitmapToChipEntry(nxt_vid, nxt_gip_a, nxt_sip_a, port_bitmap_a))
            {
                return FALSE;
            }
        }
        else if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == new_flood_behavior)
        {
            MLDSNP_OM_RouterPortInfo_T r_port_info;
            MLDSNP_OM_VlanInfo_T       vlan_info;
            MLDSNP_OM_PortInfo_T       port_info;

            /*1. skip router in vlan port bitmap
               2. add  router port which in not vlan port bitmap, but I think won't happen
               3. remove the reduant port in vlan port bitmap*/
            if (FALSE == MLDSNP_OM_GetVlanInfo(nxt_vid, &vlan_info))
            {
                MLDSNP_BD(TRACE, "no such nxt_vid %d exist ", nxt_vid);
                return FALSE;
            }

            /*router port won't be clear from chip*/
            r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
            {
                MLDSNP_TYPE_DeletePortFromPorBitMap(r_port_info.port_no, port_bitmap_a);
            }

            /*registered port won't be clear from chip*/
            port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&entry_info.register_port_list, &port_info))
            {
                MLDSNP_TYPE_DeletePortFromPorBitMap(r_port_info.port_no, vlan_entry.dot1q_vlan_current_egress_ports);
            }

            MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry(nxt_vid, nxt_gip_a, nxt_sip_a, port_bitmap_a);

            /*when the same means all port is deleted, then delete the om
              this case shall not happend
             */
            if (0 == memcmp(vlan_entry.dot1q_vlan_current_egress_ports, port_bitmap_a, sizeof(port_bitmap_a)))
            {
                MLDSNP_BD(ERR, "all ports are deleted ");

                port_info.port_no = 0;
                while (TRUE == L_SORT_LST_Get_Next(&entry_info.register_port_list, &port_info))
                {
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.filter_timer_p);
                }
                MLDSNP_OM_DeleteHisamEntryInfo(&entry_info);
            }

        }
        else
        {
            return FALSE;
        }
#endif
    }

    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == TRUE)
    /*store to om*/
    MLDSNP_OM_SetUnknownFloodBehavior(vlan_id, new_flood_behavior);
    #endif

    return TRUE;
}/*End of MLDSNP_UNKNOWN_SetFloodBehavior*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_UNKNOWN_AddRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function process the new router port is added
 * INPUT   : flood_behavior   - the flood behavior
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : check if there is unknown data need flood to this port
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_UNKNOWN_AddRouterPort(
    UI16_T vid,
    UI16_T r_port)
{
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
    MLDSNP_OM_HisamEntry_T        entry_info;
    UI16_T next_vid = vid;
    UI8_T sip_a[SRC_IP_LEN] = {0}, gip_a[GROUP_LEN] = {0};

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetUnknownFloodBehavior(vid, &flood_behavior);

    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == flood_behavior)
    {
        return TRUE;
    }

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (next_vid != vid)
        {/*over the need take vid, then stop */
            break;
        }

        if (FALSE == MLDSNP_UNKNOWN_ExitUnknownRegisterGroup(&entry_info))
        {
            continue;
        }

        MLDSNP_UNKNOWN_AddPortToChipEntry(next_vid, gip_a, sip_a, r_port);
    }

    return TRUE;
}/*End of MLDSNP_UNKNOWN_AddRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the receiver port has a long timer
*          over timer without receiving the multicast data
* INPUT   : *timer_para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout(
    void * timer_para_p)
{
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
    MLDSNP_TIMER_TimerPara_T      *para_p = (MLDSNP_TIMER_TimerPara_T *)timer_para_p;
    MLDSNP_OM_PortInfo_T          port_info;

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, lport =%d", para_p->gip_a, para_p->sip_list_a, 1, para_p->vid, para_p->lport);

    MLDSNP_OM_GetUnknownFloodBehavior(para_p->vid, &flood_behavior);

    /*if the port info not exist*/
    #if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (FALSE == MLDSNP_OM_GetPortInfo(para_p->vid, para_p->gip_a, para_p->sip_list_a, para_p->lport, &port_info))
    #else
    if (FALSE == MLDSNP_OM_GetPortInfo(para_p->vid, para_p->gip_a, NULL, para_p->lport, &port_info))
    #endif
    {
        MLDSNP_BD_SHOW_GROUP_SRC(ERR, "unknown timeout but can't get this port join info, vid=%d, lport =%d",
                                 para_p->gip_a,  para_p->sip_list_a, 1, para_p->vid, para_p->lport);

        MLDSNP_TIMER_FreeTimer(&para_p->self_p);
        return TRUE;
    }

    /*if this port not join unknow skip it*/
    if (MLDSNP_TYPE_JOIN_UNKNOWN != port_info.join_type)
    {
        return TRUE;
    }

    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == flood_behavior)
    {
        MLDSNP_OM_RouterPortInfo_T r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;
        UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

        if (FALSE == MLDSNP_OM_GetVlanInfo(para_p->vid, &vlan_info))
        {
            MLDSNP_BD_ARG(RX, "no such vid %d exist\r\n", para_p->vid);
            return FALSE;
        }

        /*delete all router port*/
        r_port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, port_bitmap_a);
        }

        /*delet this registered port from chip
          */
        MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap_a);

        MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry(para_p->vid, para_p->gip_a, para_p->sip_list_a, port_bitmap_a);
    }
#ifndef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    else
    {
        /* delete this vlan's port
        */
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;

        if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, para_p->vid, &vlan_entry))
        {
            return FALSE;
        }

        /*delete all vlan's member port*/
        MLDSNP_UNKNOWN_DeletePortBitMapFromChipEntry(para_p->vid, para_p->gip_a, para_p->sip_list_a, vlan_entry.dot1q_vlan_current_egress_ports);
    }
#endif

    /*delete port from om
     */
    {
        MLDSNP_Timer_T *tmp_timer_p = port_info.src_timer_p;

        #if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
        if (FALSE == MLDSNP_OM_DeletePortInfo(para_p->vid, para_p->gip_a, para_p->sip_list_a, para_p->lport))
        #else
        if (FALSE == MLDSNP_OM_DeletePortInfo(para_p->vid, para_p->gip_a, NULL, para_p->lport))
        #endif
        {
            return FALSE;
        }

        MLDSNP_TIMER_FreeTimer(&tmp_timer_p);
    }
    return TRUE;
}/*End of MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout*/


/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_DeleteAllJoinedPort
*-------------------------------------------------------------------------
* PURPOSE : process port leave this group entry
* INPUT   : vid        - the input vid
*           *gip_ap    - the group ip address
*           *sip_ap    - the sip ip address
* OUTPUT  : None
* RETURN  : None
* NOTE    : because each (vid, gip, sip) entry will write more write port to chip according to flood or to-router port
*           behavior. This function get the extra write to chip port
*-------------------------------------------------------------------------
*/
void MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap)
{
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetUnknownFloodBehavior(vid, &flood_behavior);

    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == flood_behavior)
    {
        MLDSNP_OM_RouterPortInfo_T r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            MLDSNP_BD_ARG(RX, "no such vid %d exist\r\n", vid);
            return;
        }

        /*delete all router port*/
        r_port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
        {
            MLDSNP_UNKNOWN_DeletePortFromChipEntry(vid, gip_ap, sip_ap, r_port_info.port_no);
        }
    }
#ifndef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    else
    {
        /* delete this vlan's port
        */
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
        UI16_T i = 0;

        if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry))
            return;

        for (i = 1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if (MLDSNP_TYPE_IsPortInPortBitMap(i, vlan_entry.dot1q_vlan_current_egress_ports))
            {
                MLDSNP_UNKNOWN_DeletePortFromChipEntry(vid, gip_ap, sip_ap, i);
            }
        }
    }
#endif
    return;
}/*End of MLDSNP_UNKNOWN_DeleteAllJoinedPort*/


/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp
*-------------------------------------------------------------------------
* PURPOSE : To get the portbitmap this unknown forward ports
* INPUT   : vid            - the input vid
*               lport      - the port register this group
* OUTPUT  : *port_bitmap_a - the port bitmap
* RETURN  : None
* NOTE    : if flooding, return vlan's all member port bitmap, if to-router port, return all router port bitmap
*-------------------------------------------------------------------------
*/
void MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp(
    UI16_T vid,
    UI16_T lport,
    UI8_T *port_bimap_ap)
{
    MLDSNP_TYPE_UnknownBehavior_T unkonw_behavior = MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD;
    MLDSNP_OM_VlanInfo_T          vlan_info;

    MLDSNP_OM_GetUnknownFloodBehavior(vid, &unkonw_behavior);

    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == unkonw_behavior)
    {
        MLDSNP_BD(TRACE, "  to router port");
        /*get all router port*/
        if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;

            nxt_r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, port_bimap_ap);
            }
        }
    }
#ifndef MLDSNPP_UNKNONW_FLOOD_WILL_NOT_TRAP_DATA
    else
    {
        VLAN_OM_Dot1qVlanCurrentEntry_T    vlan_info;

        MLDSNP_BD(TRACE, " flood");

        if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info))
        {
            return;
        }

        memcpy(port_bimap_ap, vlan_info.dot1q_vlan_current_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }
#endif
    MLDSNP_TYPE_AddPortIntoPortBitMap(lport, port_bimap_ap);

    return;
}


