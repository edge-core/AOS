/* MODULE NAME - VXLAN_BACKDOOR.C
 * PURPOSE : Provides the definitions for VXLAN back door.
 * NOTES   : None.
 * HISTORY : 2015/05/14 -- Kelly Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_type.h"
#include "sys_dflt.h"
#include "backdoor_mgr.h"
#include "backdoor_lib.h"
#include "vxlan_backdoor.h"
#include "vxlan_om.h"
#include "vxlan_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define VXLAN_UNIT_TEST    TRUE

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (VXLAN_UNIT_TEST == TRUE)
static void VXLAN_BACKDOOR_UnitTest(void);
#endif

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - VXLAN_BACKDOOR_Main
 * PURPOSE : Provide main routine of VXLAN backdoor.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void VXLAN_BACKDOOR_Main(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    VXLAN_OM_VNI_T vni_entry;
    VXLAN_OM_RVtep_T rvtep_entry;
    UI32_T vid, lport, vxlan_port, l3_if, cur_vni;
    UI32_T i;
    UI16_T next_vid;
    /* LOCAL VARIABLE DECLARATIONS
     */

    int     choice;
    char    buffer[BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH+1];

    /* BODY
     */

    while (1)
    {
        BACKDOOR_MGR_Print("\r\n=============================================");
        BACKDOOR_MGR_Print("\r\n VXLAN Backdoor Menu");
        BACKDOOR_MGR_Print("\r\n=============================================");
        BACKDOOR_MGR_Print("\r\n a - Get access port info.");
        BACKDOOR_MGR_Print("\r\n d - Turn on/off showing database messages");
        BACKDOOR_MGR_Print("\r\n e - Turn on/off showing event messages");
        BACKDOOR_MGR_Print("\r\n f - Turn on/off showing vni messages");
        BACKDOOR_MGR_Print("\r\n g - Turn on/off showing vtep messages");
        BACKDOOR_MGR_Print("\r\n l - Get L3 interface");
        BACKDOOR_MGR_Print("\r\n m - Get multicast VTEP");
        BACKDOOR_MGR_Print("\r\n v - VLAN-VNI map");
        BACKDOOR_MGR_Print("\r\n r - Get unicast VTEP");
#if (VXLAN_UNIT_TEST == TRUE)
        BACKDOOR_MGR_Print("\r\n u - Run unit test");
#endif
        BACKDOOR_MGR_Print("\r\n x - Exit backdoor");
        BACKDOOR_MGR_Print("\r\n---------------------------------------------");
        BACKDOOR_MGR_Print("\r\nEnter your choice: ");

        choice = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", choice);

        switch (choice)
        {
        case 'a':
            BACKDOOR_MGR_Print("\r\n VXLAN access port");
            vid = BACKDOOR_LIB_RequestUI32("\r\n enter vid: ", 1);
            lport = BACKDOOR_LIB_RequestUI32("\r\n enter lport: ", 1);
            vxlan_port = VXLAN_OM_GetAccessVxlanPort((UI16_T)vid, lport);
            if (vxlan_port != 0)
            {
                BACKDOOR_MGR_Printf("\r\n vid=%lu, lport=%lu , uc_vxlan_port[0x%lx]", (unsigned long)vid, (unsigned long)lport, (unsigned long)vxlan_port);
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n vid=%lu, lport=%lu  not access port", (unsigned long)vid, (unsigned long)lport);
            }
            break;

        case 'd':
            BACKDOOR_MGR_Printf("\r\n current database debug value is %s",
                ((VXLAN_OM_GetDebug(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG) == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, TRUE);
            else if (choice == '2')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;
        case 'e':
            BACKDOOR_MGR_Printf("\r\n current event debug value is %s",
                ((VXLAN_OM_GetDebug(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG) == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, TRUE);
            else if (choice == '2')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;
        case 'f':
            BACKDOOR_MGR_Printf("\r\n current vni debug value is %s",
                ((VXLAN_OM_GetDebug(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG) == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, TRUE);
            else if (choice == '2')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;
        case 'g':
            BACKDOOR_MGR_Printf("\r\n current vtep  debug value is %s",
                ((VXLAN_OM_GetDebug(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG) == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, TRUE);
            else if (choice == '2')
                VXLAN_OM_SetDebug(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;
        case 'l':
            BACKDOOR_MGR_Print("\r\n VXLAN L3 interface ");
            vid = BACKDOOR_LIB_RequestUI32("\r\n enter vid: ", 1);
            l3_if = VXLAN_OM_GetL3If(vid);
            BACKDOOR_MGR_Printf("\r\n vid=%lu, l3_if=%lu", (unsigned long)vid, (unsigned long)l3_if);
            break;
        case 'm':
            BACKDOOR_MGR_Printf("\r\n === Multicast remote VTEP ===");
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            while (VXLAN_OM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
            {
                BACKDOOR_MGR_Printf("\r\n vni: %lu \r\n vfi: 0x%lx", (unsigned long)rvtep_entry.vni, (unsigned long)rvtep_entry.vfi);
                BACKDOOR_MGR_Printf("\r\n dest IP: %d.%d.%d.%d \r\n source IP: %d.%d.%d.%d \r\n nexthop_cnt: %lu",
                    rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                    rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                    rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
                    rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                    (unsigned long)rvtep_entry.nexthop_cnt);
                for(i=0;i<rvtep_entry.nexthop_cnt;i++)
                {
                    BACKDOOR_MGR_Printf("\r\n next_hop IP: %d.%d.%d.%d",
                        rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                        rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);
                }    
                BACKDOOR_MGR_Printf("\r\n vid: %u \r\n lport: %lu \r\n uc_vxlan_port: 0x%lx \r\n mc_vxlan_port: 0x%lx \r\n mac: %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                    rvtep_entry.vid, (unsigned long)rvtep_entry.lport, (unsigned long)rvtep_entry.uc_vxlan_port, (unsigned long)rvtep_entry.mc_vxlan_port,
                    rvtep_entry.mac_ar[0], rvtep_entry.mac_ar[1], rvtep_entry.mac_ar[2],
                    rvtep_entry.mac_ar[3], rvtep_entry.mac_ar[4], rvtep_entry.mac_ar[5]);
            }
            break;
        case 'r':
            BACKDOOR_MGR_Printf("\r\n=== Unicast remote VTEP ===");
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
            {
                BACKDOOR_MGR_Printf("\r\n vni: %lu \r\n vfi: 0x%lx", (unsigned long)rvtep_entry.vni, (unsigned long)rvtep_entry.vfi);
                BACKDOOR_MGR_Printf("\r\n dest IP: %d.%d.%d.%d \r\n source IP: %d.%d.%d.%d nexthop_cnt:%lu\r\n",
                    rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                    rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                    rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
                    rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                    (unsigned long)rvtep_entry.nexthop_cnt);
                for(i=0;i<rvtep_entry.nexthop_cnt;i++)
                {
                    BACKDOOR_MGR_Printf("\r\n next_hop IP: %d.%d.%d.%d",
                        rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                        rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);
                }

                BACKDOOR_MGR_Printf("\r\n vid: %u \r\n lport: %lu \r\n uc_vxlan_port: 0x%lx \r\n mc_vxlan_port: 0x%lx \r\n mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    rvtep_entry.vid, (unsigned long)rvtep_entry.lport, (unsigned long)rvtep_entry.uc_vxlan_port, (unsigned long)rvtep_entry.mc_vxlan_port,
                    rvtep_entry.mac_ar[0], rvtep_entry.mac_ar[1], rvtep_entry.mac_ar[2],
                    rvtep_entry.mac_ar[3], rvtep_entry.mac_ar[4], rvtep_entry.mac_ar[5]);
            }
            break;
        case 'v':
            BACKDOOR_MGR_Printf("\r\n=== VLAN-VNI mapping  ===");
            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            next_vid = 0;
            while(VXLAN_OM_GetNextVlanVniMapEntry(&next_vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
            {
                vni_entry.vni = cur_vni;
                VXLAN_OM_GetVniEntry(&vni_entry);
                BACKDOOR_MGR_Printf("\r\n vid: %u \r\n vni: %lu \r\n vfi: %lx \r\n bcast_group: 0x%lx",
                   next_vid, (unsigned long)vni_entry.vni, (unsigned long)vni_entry.vfi, (unsigned long)vni_entry.bcast_group);
            }
            break;
#if (VXLAN_UNIT_TEST == TRUE)
        case 'u':
            VXLAN_BACKDOOR_UnitTest();
            break;
#endif

        case 'x':
            return;

        default:
            BACKDOOR_MGR_Printf("\r\n Invalid choice!!");
        }

        BACKDOOR_MGR_Print("\r\n");
    } /* end while (1) */
} /* End of VXLAN_BACKDOOR_Main */

/* LOCAL SUBPROGRAM BODIES
 */

#if (VXLAN_UNIT_TEST == TRUE)
static void VXLAN_BACKDOOR_UnitTest(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */


    /* BODY
     */

    return;
} /* End of VXLAN_BACKDOOR_UnitTest */
#endif /* #if (VXLAN_UNIT_TEST == TRUE) */
