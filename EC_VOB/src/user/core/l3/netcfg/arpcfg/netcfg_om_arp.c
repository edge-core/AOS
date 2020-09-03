/* Module Name:NETCFG_OM_ARP.C
 * Purpose: To store configuration static arp and arp aging timeout.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "l_pt.h"
#include "l_sort_lst.h"
#include "sysfun.h"
#include "netcfg_om_arp.h"
#include "netcfg_om_arp_private.h"
#include "netcfg_type.h"

typedef    struct NETCFG_OM_ARP_LCB_S
{
    UI32_T                        sem_id;
    SYS_TYPE_Stacking_Mode_T    stacking_mode;
} NETCFG_OM_ARP_LCB_T;

static int NETCFG_OM_ARP_Compare(void *elm1,void *elm2);
static int NETCFG_OM_ARP_CompareByAddress(void *elm1,void *elm2);

static NETCFG_OM_ARP_LCB_T arp_lcb;   /* semophore variable */
static L_SORT_LST_List_T arps;        /* a list to store ARP entry */
static UI32_T    arp_timeout;

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ARP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ARP OM.
 *
 * INPUT   : ipcmsg_p -- input request ipc message buffer
 *
 * OUTPUT  : ipcmsg_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ARP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_OM_ARP_IPCMsg_T *netcfg_om_arp_msg_p;

    if(ipcmsg_p==NULL)
        return FALSE;
    netcfg_om_arp_msg_p= (NETCFG_OM_ARP_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(netcfg_om_arp_msg_p->type.cmd)
    {            
        case     NETCFG_OM_ARP_IPC_GETTIMEOUT:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ARP_GetTimeout(&(netcfg_om_arp_msg_p->data.ui32_v));
            ipcmsg_p->msg_size=NETCFG_OM_ARP_GET_MSG_SIZE(ui32_v);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            netcfg_om_arp_msg_p->type.result_bool = FALSE;
            ipcmsg_p->msg_size = NETCFG_OM_ARP_MSGBUF_TYPE_SIZE;
            return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_ARP_Init
 * PURPOSE:create semaphore
 *
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_ARP_Init(void)
{
    arp_timeout = SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT;
    if(L_SORT_LST_Create(&arps,
                      SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY,
                      sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T),
                      NETCFG_OM_ARP_Compare)==FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_ARP_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_ARP_Init : Can't create ARP List.\n");
    }

    memset(&arp_lcb, 0, sizeof(NETCFG_OM_ARP_LCB_T));
    
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &arp_lcb.sem_id) != SYSFUN_OK)
    {
        DBG_PrintText("NETCFG_OM_ARP_Init : Can't create semaphore\n");
    }
}

/* FUNCTION NAME : NETCFG_OM_ARP_AddStaticEntry
 * PURPOSE:
 *      Add an static ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_AddStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    result = L_SORT_LST_Set(&arps, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteStaticEntry
 * PURPOSE:
 *      Remove a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T    NETCFG_OM_ARP_DeleteStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    result = L_SORT_LST_Delete(&arps, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_UpdateStaticEntry
 * PURPOSE:
 *      Update a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T    NETCFG_OM_ARP_UpdateStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T *mem_entry = NULL;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T local_entry;

    memset(&local_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    
    local_entry.ip_net_to_media_entry.ip_net_to_media_if_index = entry->ip_net_to_media_entry.ip_net_to_media_if_index;
    local_entry.ip_net_to_media_entry.ip_net_to_media_net_address = entry->ip_net_to_media_entry.ip_net_to_media_net_address;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    mem_entry = L_SORT_LST_GetPtr(&arps, &local_entry);
    if(mem_entry == NULL)
        result = FALSE;
    else
    {
        memcpy(mem_entry, entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
        result = TRUE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_GetStaticEntry
 * PURPOSE:
 *      Get a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_GetStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    result = L_SORT_LST_Get(&arps, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_GetNextStaticEntry
 * PURPOSE:
 *      Get next available static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_ARP_GetNextStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    if (entry == NULL)
        return FALSE;
        
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);
    
    if(entry->ip_net_to_media_entry.ip_net_to_media_if_index == 0 &&
       entry->ip_net_to_media_entry.ip_net_to_media_net_address == 0)
        result = L_SORT_LST_Get_1st(&arps, entry);
    else
        result = L_SORT_LST_Get_Next(&arps, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_GetNextStaticEntryByAddress
 * PURPOSE:
 *      Get next available static arp entry by IP address.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_net_address).
 *      If key is 0, get first one.
 */
BOOL_T NETCFG_OM_ARP_GetNextStaticEntryByAddress(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    if (entry == NULL)
        return FALSE;
        
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);
    arps.compare = NETCFG_OM_ARP_CompareByAddress;
    if(entry->ip_net_to_media_entry.ip_net_to_media_net_address == 0)
        result = L_SORT_LST_Get_1st(&arps, entry);
    else
        result = L_SORT_LST_Get_Next(&arps, entry);
    arps.compare = NETCFG_OM_ARP_Compare;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteAllStaticEntry
 * PURPOSE:
 *          Remove all static arp entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_ARP_DeleteAllStaticEntry(void)
{
    UI32_T   original_priority;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    L_SORT_LST_Delete_All(&arps);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_ARP_GetTimeout
 * PURPOSE:
 *      Get arp age timeout.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      age_time.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_GetTimeout(UI32_T *age_time)
{
    UI32_T   original_priority;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    *age_time = arp_timeout;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_ARP_SetTimeout
 * PURPOSE:
 *      Set arp age timeout.
 *
 * INPUT:
 *      age_time
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_SetTimeout(UI32_T age_time)
{
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(arp_lcb.sem_id);

    arp_timeout = age_time;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(arp_lcb.sem_id, original_priority);

    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_ARP_Compare
 * PURPOSE:
 *      Compare function of Sort-List.
 *
 * INPUT:
 *        elm1
 *          elm2
 *
 * OUTPUT: 
 *      None.
 *
 * RETURN: 
 *      =0 : equal
 *      >0 : elm1 > elm2
 *      <0 : elm1 < elm2
 *
 * NOTES:  Key:ip_net_to_media_if_index and ip_net_to_media_net_address.
 */
static int NETCFG_OM_ARP_Compare(void *elm1,void *elm2)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T *element1, *element2;
    
    element1 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm1;
    element2 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm2;

    if(element1->ip_net_to_media_entry.ip_net_to_media_if_index !=
       element2->ip_net_to_media_entry.ip_net_to_media_if_index)
    {
        if (element1->ip_net_to_media_entry.ip_net_to_media_if_index >
            element2->ip_net_to_media_entry.ip_net_to_media_if_index)
        {
            return (1);
        }
        else if (element1->ip_net_to_media_entry.ip_net_to_media_if_index <
                 element2->ip_net_to_media_entry.ip_net_to_media_if_index)
        {
            return (-1);
        }
    }
    else
    {
        if (element1->ip_net_to_media_entry.ip_net_to_media_net_address >
            element2->ip_net_to_media_entry.ip_net_to_media_net_address)
        {
            return (1);
        }
        else if (element1->ip_net_to_media_entry.ip_net_to_media_net_address <
                 element2->ip_net_to_media_entry.ip_net_to_media_net_address)
        {
            return (-1);
        }
        else
        {
            return (0);
        }
    }
    return (0);
}   /* end of NETCFG_OM_ARP_Compare  */

/* FUNCTION NAME : NETCFG_OM_ARP_CompareByAddress
 * PURPOSE:
 *      Compare function of Sort-List.
 *
 * INPUT:
 *        elm1
 *        elm2
 *
 * OUTPUT: 
 *      None.
 *
 * RETURN: 
 *      =0 : equal
 *      >0 : elm1 > elm2
 *      <0 : elm1 < elm2
 *
 * NOTES:  Key:ip_net_to_media_net_address.
 */
static int NETCFG_OM_ARP_CompareByAddress(void *elm1,void *elm2)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T *element1, *element2;
    
    element1 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm1;
    element2 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm2;

    if (element1->ip_net_to_media_entry.ip_net_to_media_net_address >
        element2->ip_net_to_media_entry.ip_net_to_media_net_address)
    {
        return (1);
    }
    else if (element1->ip_net_to_media_entry.ip_net_to_media_net_address <
             element2->ip_net_to_media_entry.ip_net_to_media_net_address)
    {
        return (-1);
    }
    else
    {
        return (0);
    }
    return (0);
}   /* end of NETCFG_OM_ARP_CompareByAddress  */

