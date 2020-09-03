/* Module Name:NETCFG_OM_RIP.C
 * Purpose: To store RIP configuration information.
 *
 * Notes:
 *      
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2008/05/18     --- Lin.Li, Create
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
#include "netcfg_om_rip.h"
#include "netcfg_type.h"
#include "l_radix.h"
#include "l_stdlib.h"

#define MAX_NBR_OF_VR   1
#define MAX_NBR_OF_VRF_PER_VR   1
#define MAX_NBR_OF_INTERFACE 256

#define GETMINVALUE(v1, v2)             (v1 <= v2)? v1:v2
#define PREFIXLEN2MASK(len)             ~((1 << (32 - len)) - 1)


typedef    struct NETCFG_OM_RIP_Master_S
{
    UI32_T                          vr_id;
    L_SORT_LST_List_T               rip_ifs;
    NETCFG_OM_RIP_Instance_T        *rip_instances[MAX_NBR_OF_VRF_PER_VR];
    NETCFG_TYPE_RIP_Debug_Status_T  debug;
} NETCFG_OM_RIP_Master_T;

static int NETCFG_OM_RIP_InterfaceCompare(void *elm1,void *elm2);
static BOOL_T NETCFG_OM_RIP_GetMasterEntry(UI32_T vr_id,NETCFG_OM_RIP_Master_T **entry);
static BOOL_T NETCFG_OM_RIP_GetInstance(UI32_T vr_id,UI32_T instance,NETCFG_OM_RIP_Instance_T **entry);
static BOOL_T NETCFG_OM_RIP_GetInterface(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry);
static BOOL_T NETCFG_OM_RIP_UpdateInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry);
static BOOL_T NETCFG_OM_RIP_GetNextInterface(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry);
static void NETCFG_OM_RIP_DeleteInstance(UI32_T vr_id, UI32_T instance);

static UI32_T rip_om_sem_id;   
static NETCFG_OM_RIP_Master_T *rip_master[MAX_NBR_OF_VR];/*RIP database in om*/

/* FUNCTION NAME : NETCFG_OM_RIP_Init
 * PURPOSE:Init NETCFG_OM_RIP database, create semaphore
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
void NETCFG_OM_RIP_Init(void)
{
    UI32_T i;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &rip_om_sem_id) != SYSFUN_OK)
    {
        printf("NETCFG_OM_RIP_Init : Can't create semaphore\n");
    }
    
    for(i = 0; i < MAX_NBR_OF_VR; i++)
        rip_master[i] = NULL;
    if(NETCFG_OM_RIP_AddVr(SYS_DFLT_VR_ID) == FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_RIP_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_RIP_Init : Can't create RIP List.\n");
    }
}

/* FUNCTION NAME : NETCFG_OM_RIP_AddVr
 * PURPOSE:
 *      Add a RIP master entry when add a VR.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddVr(UI32_T vr_id)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_RIP_Master_T *entry = NULL;


    entry = (NETCFG_OM_RIP_Master_T *)malloc(sizeof(NETCFG_OM_RIP_Master_T));
    if(entry != NULL)
    {
        memset(entry, 0, sizeof(NETCFG_OM_RIP_Master_T));
        entry->vr_id = vr_id;

        if(L_SORT_LST_Create(&entry->rip_ifs,
                              MAX_NBR_OF_INTERFACE,
                              sizeof(NETCFG_TYPE_RIP_If_T),
                              NETCFG_OM_RIP_InterfaceCompare)==FALSE)
        {
            free(entry);
            entry = NULL;
            return FALSE;
        }
        original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
        rip_master[vr_id] = entry;
        result = TRUE;        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteVr
 * PURPOSE:
 *      Delete a RIP master entry when delete a VR.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteVr(UI32_T vr_id)
{
    UI32_T original_priority,i;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    for(i=0;i<MAX_NBR_OF_VRF_PER_VR;i++)
    {
        if(rip_master[vr_id]->rip_instances[i] != NULL) 
        {
            NETCFG_OM_RIP_DeleteInstance(vr_id, i);
        }
    }
    
    L_SORT_LST_Delete_All(&rip_master[vr_id]->rip_ifs);

    free(rip_master[vr_id]);
    rip_master[vr_id] = NULL;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteAllRipMasterEntry
 * PURPOSE:
 *          Remove all RIP master entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_RIP_DeleteAllRipMasterEntry(void)
{
    UI32_T original_priority,i,j;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    for(i=0;i<MAX_NBR_OF_VR;i++)
    {
        if(rip_master[i] != NULL)
        {
            for(j=0;j<MAX_NBR_OF_VRF_PER_VR;j++)
            {
                if(rip_master[i]->rip_instances[j] != NULL) 
                {
                    NETCFG_OM_RIP_DeleteInstance(i, j);
                }
            }
            L_SORT_LST_Delete_All(&(rip_master[i]->rip_ifs));
            free(rip_master[i]);
            rip_master[i] = NULL;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetMasterEntry
 * PURPOSE:
 *      Get a rip master entry with specific vr_id.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
static BOOL_T NETCFG_OM_RIP_GetMasterEntry(UI32_T vr_id,NETCFG_OM_RIP_Master_T **entry)
{
    BOOL_T     result = FALSE;

    if(rip_master[vr_id] != NULL)
    {
        *entry = rip_master[vr_id];
        result = TRUE;
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AddInstance
 * PURPOSE:
 *      Add a RIP instance entry when router rip enable in a instance.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddInstance(UI32_T vr_id, UI32_T instance)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_RIP_Master_T    *vr_entry = NULL;
    NETCFG_OM_RIP_Instance_T  *vrf_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }

    vrf_entry = (NETCFG_OM_RIP_Instance_T *)malloc(sizeof(NETCFG_OM_RIP_Instance_T));

    if(vrf_entry != NULL)
    {
        memset(vrf_entry,0,sizeof(NETCFG_OM_RIP_Instance_T));
        vrf_entry->instance_value.instance = instance;
        vrf_entry->instance_value.version = SYS_DFLT_RIP_GLOBAL_VERSION;
        vrf_entry->instance_value.default_information = SYS_DFLT_RIP_DEFAULT_ROUTE_ORIGINATE;
        vrf_entry->instance_value.default_metric = SYS_DFLT_RIP_DEFAULT_METRIC;
        vrf_entry->instance_value.timer.update = SYS_DFLT_RIP_UPDATE_TIME;
        vrf_entry->instance_value.timer.timeout = SYS_DFLT_RIP_TIMEOUT_TIME;
        vrf_entry->instance_value.timer.garbage = SYS_DFLT_RIP_GARBAGE_TIME;
        vrf_entry->instance_value.pmax = SYS_ADPT_MAX_NBR_OF_RIP_ROUTE_ENTRY;
        vrf_entry->instance_value.recv_buffer_size = SYS_DFLT_RIP_RECV_BUFF_SIZE;
        vrf_entry->instance_value.distance = SYS_DFLT_RIP_DISTANCE;
        L_RADIX_Create(&vrf_entry->distance_table);
        L_RADIX_Create(&vrf_entry->neignbor_table);
        L_RADIX_Create(&vrf_entry->network_table);
        vr_entry->rip_instances[instance] = vrf_entry;   
        result = TRUE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteInstance
 * PURPOSE:
 *      Delete a RIP instance entry when router rip disable in a instance.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static void NETCFG_OM_RIP_DeleteInstance(UI32_T vr_id, UI32_T instance)
{
    UI32_T i;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    NETCFG_TYPE_RIP_If_T   if_entry;
    L_RADIX_Node_T *node;
    L_RADIX_Node_T *next;

    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id,&vr_entry) == TRUE)
    {
        if(vr_entry->rip_instances[instance] != NULL)
        {   
            for(i = 0; i < NETCFG_TYPE_RIP_Redistribute_Max; i++)
            {
                if(vr_entry->rip_instances[instance]->instance_value.redistribute[i] != NULL)
                {
                    free(vr_entry->rip_instances[instance]->instance_value.redistribute[i]);
                    vr_entry->rip_instances[instance]->instance_value.redistribute[i] = NULL;
                }
            }

            /*delete all nodes of neighbor table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->neignbor_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            
            /*delete all nodes of network table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->network_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            
            /*delete all nodes of distance table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->distance_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        free(node->info);
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    free(node->info);
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            free((vr_entry->rip_instances[instance]));
            vr_entry->rip_instances[instance] = NULL;
        }
    }
    while(NETCFG_OM_RIP_GetNextInterface(vr_id, &if_entry) == TRUE)
    {
        if_entry.pass_if = FALSE;
        if_entry.network_if = FALSE;
        memset(&if_entry.distribute_table, 0, sizeof(NETCFG_TYPE_RIP_Distribute_T));
        if_entry.distribute_table.ifindex = if_entry.ifindex;
        NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
    }
}

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteInstanceEntry
 * PURPOSE:
 *      Delete a RIP instance entry when router rip disable in a instance.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteInstanceEntry(UI32_T vr_id, UI32_T instance)
{
    UI32_T original_priority, i;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    NETCFG_TYPE_RIP_If_T   if_entry;
    L_RADIX_Node_T *node;
    L_RADIX_Node_T *next;

    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id,&vr_entry) == TRUE)
    {
        if(vr_entry->rip_instances[instance] != NULL)
        {   
            for(i = 0; i < NETCFG_TYPE_RIP_Redistribute_Max; i++)
            {
                if(vr_entry->rip_instances[instance]->instance_value.redistribute[i] != NULL)
                {
                    free(vr_entry->rip_instances[instance]->instance_value.redistribute[i]);
                    vr_entry->rip_instances[instance]->instance_value.redistribute[i] = NULL;
                }
            }

            /*delete all nodes of neighbor table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->neignbor_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            
            /*delete all nodes of network table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->network_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            
            /*delete all nodes of distance table*/
            if (L_RADIX_Get_1st(&(vr_entry->rip_instances[instance]->distance_table), &node) == TRUE)
            {
                while (L_RADIX_GetNextNode(node, &next) == TRUE)
                {
                    if(node->info != NULL)
                    {
                        free(node->info);
                        node->info = NULL;
                        L_RADIX_UnlockNode(node);
                    }
                    node = next;
                }
                if(node->info != NULL)
                {
                    free(node->info);
                    node->info = NULL;
                    L_RADIX_UnlockNode(node);
                }
            }
            free((vr_entry->rip_instances[instance]));
            vr_entry->rip_instances[instance] = NULL;
        }
    }
    while(NETCFG_OM_RIP_GetNextInterface(vr_id, &if_entry) == TRUE)
    {
        if_entry.pass_if = FALSE;
        if_entry.network_if = FALSE;
        memset(&if_entry.distribute_table, 0, sizeof(NETCFG_TYPE_RIP_Distribute_T));
        if_entry.distribute_table.ifindex = if_entry.ifindex;
        NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetInstanceEntry
 * PURPOSE:
 *      Get a rip instance entry.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetInstanceEntry(UI32_T vr_id,UI32_T instance, NETCFG_OM_RIP_Instance_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    if(entry == NULL)
    {
        return FALSE;
    }
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    if(vr_entry->rip_instances[instance] != NULL)
    {
        memcpy(entry,vr_entry->rip_instances[instance], sizeof(NETCFG_OM_RIP_Instance_T));
        result = TRUE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetInstance
 * PURPOSE:
 *      Get a rip instance entry.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_RIP_GetInstance(UI32_T vr_id,UI32_T instance,NETCFG_OM_RIP_Instance_T **entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
        return FALSE;
    
    if(vr_entry->rip_instances[instance] != NULL)
    {
        *entry = vr_entry->rip_instances[instance];
        result = TRUE;
    }

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AddInterface
 * PURPOSE:
 *      Make a link-up l3 interface up in the RIP
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddInterface(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_RIP_If_T entry;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    memset(&entry, 0,sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    entry.recv_packet = TRUE;
    entry.send_packet = TRUE;
    entry.auth_mode = SYS_DFLT_RIP_AUTH_MODE;
    entry.recv_version_type = SYS_DFLT_RIP_RECEIVE_PACKET_VERSION;
    entry.send_version_type = SYS_DFLT_RIP_SEND_PACKET_VERSION;
    entry.split_horizon = SYS_DFLT_RIP_SPLIT_HORIZON_TYPE;
    entry.distribute_table.ifindex = ifindex;

    result = L_SORT_LST_Set(&vr_entry->rip_ifs, &entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteInterface
 * PURPOSE:
 *      Make a interface down in the RIP
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteInterface(UI32_T vr_id,UI32_T ifindex)
{
    UI32_T original_priority;
    NETCFG_TYPE_RIP_If_T entry;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    memset(&entry, 0,sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) == TRUE)
    {    
        L_SORT_LST_Delete(&vr_entry->rip_ifs, &entry);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_RIP_RifUp
 * PURPOSE:
 *      Singnal rif up
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_RifUp(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_RIP_If_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, entry) == TRUE)
    {
        result = TRUE;
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RifDown
 * PURPOSE:
 *      Singnal rif down
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_RifDown(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_RIP_If_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, entry) == TRUE)
    {
        result = TRUE;
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetInterfaceEntry
 * PURPOSE:
 *      Get a rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    result = L_SORT_LST_Get(&vr_entry->rip_ifs, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetInterface
 * PURPOSE:
 *      Get a rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_RIP_GetInterface(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        return FALSE;
    }

    result = L_SORT_LST_Get(&vr_entry->rip_ifs, entry);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextInterface
 * PURPOSE:
 *      Get next rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_RIP_GetNextInterface(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        return FALSE;
    }
    if(entry->ifindex == 0)
    {
        result = L_SORT_LST_Get_1st(&vr_entry->rip_ifs, entry);
    }
    else
    {
        result = L_SORT_LST_Get_Next(&vr_entry->rip_ifs, entry);
    }

    return  result;
}


/* FUNCTION NAME : NETCFG_OM_RIP_GetNextInterfaceEntry
 * PURPOSE:
 *      Get next rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetNextInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);

    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    if(entry->ifindex == 0)
        result = L_SORT_LST_Get_1st(&vr_entry->rip_ifs, entry);
    else
        result = L_SORT_LST_Get_Next(&vr_entry->rip_ifs, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_UpdateInterfaceEntry
 * PURPOSE:
 *      Update the rip interface entry.
 *
 * INPUT:
 *      vr_id
 *      entry.
 *
 * OUTPUT:
 *      None
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_RIP_UpdateInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    NETCFG_TYPE_RIP_If_T *mem_entry = NULL;
    NETCFG_TYPE_RIP_If_T local_entry;
    
    if(entry == NULL)
        return FALSE;
    
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        return FALSE;
    }
    
    memset(&local_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    local_entry.ifindex = entry->ifindex;

    mem_entry = L_SORT_LST_GetPtr(&vr_entry->rip_ifs, &local_entry);
    if(mem_entry == NULL)
    {
        result = FALSE;
    }
    else
    {
        memcpy(mem_entry, entry, sizeof(NETCFG_TYPE_RIP_If_T));
        result = TRUE;
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_InterfaceCompare
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
 * NOTES:  key:ifindex
 */
static int NETCFG_OM_RIP_InterfaceCompare(void *elm1,void *elm2)
{
    NETCFG_TYPE_RIP_If_T *element1, *element2;
    
    element1 = (NETCFG_TYPE_RIP_If_T *)elm1;
    element2 = (NETCFG_TYPE_RIP_If_T *)elm2;

    if (element1->ifindex > element2->ifindex)
    {
        return (1);
    }
    else if (element1->ifindex < element2->ifindex)
    {
        return (-1);
    }
    else
        return (0);
}   /* end of NETCFG_OM_RIP_InterfaceCompare  */

/* FUNCTION NAME : NETCFG_OM_RIP_RecvPacketSet
 * PURPOSE:
 *      Make RIP receive packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvPacketSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    
    entry.recv_packet = TRUE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RecvPacketUnset
 * PURPOSE:
 *      Make RIP not receive packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvPacketUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.recv_packet = FALSE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
        
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SendPacketSet
 * PURPOSE:
 *      Make RIP not send packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendPacketSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.send_packet = TRUE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
            
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SendPacketUnset
 * PURPOSE:
 *      Make RIP not send packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendPacketUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.send_packet = FALSE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RecvVersionTypeSet
 * PURPOSE:
 *      Set RIP receive version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvVersionTypeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                                       enum NETCFG_TYPE_RIP_Version_E type)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.recv_version_type = type;
    RIP_IF_PARAM_SET (&entry, RECV_VERSION);

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RecvVersionUnset
 * PURPOSE:
 *      Set RIP receive version type to default version type.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvVersionUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.recv_version_type = SYS_DFLT_RIP_RECEIVE_PACKET_VERSION;
    RIP_IF_PARAM_UNSET (&entry, RECV_VERSION);

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SendVersionTypeSet
 * PURPOSE:
 *      Set RIP send version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendVersionTypeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                                       enum NETCFG_TYPE_RIP_Version_E type)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.send_version_type = type;
    RIP_IF_PARAM_SET (&entry, SEND_VERSION);

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SendVersionUnset
 * PURPOSE:
 *      Set RIP send version type to default version type.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendVersionUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.send_version_type = SYS_DFLT_RIP_SEND_PACKET_VERSION;
    RIP_IF_PARAM_UNSET (&entry, SEND_VERSION);

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthModeSet
 * PURPOSE:
 *      Set RIP authentication mode.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      mode 
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthModeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                              enum NETCFG_TYPE_RIP_Auth_Mode_E mode)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.auth_mode = mode;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthModeUnset
 * PURPOSE:
 *      Set RIP authentication mode to default mode.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthModeUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.auth_mode = NETCFG_TYPE_RIP_NO_AUTH;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthStringSet
 * PURPOSE:
 *      Set RIP authentication string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      str
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthStringSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, char *str)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    if(str == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    memset(entry.auth_str,0, sizeof(entry.auth_str));
    strncpy(entry.auth_str, str, strlen(str));
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthStringUnset
 * PURPOSE:
 *      Set RIP authentication string to null string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthStringUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    memset(entry.auth_str,0, sizeof(entry.auth_str));

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthKeyChainSet
 * PURPOSE:
 *      Set RIP key-chain string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      str
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthKeyChainSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, char *str)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    if(str == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    memset(entry.key_chain,0, sizeof(entry.key_chain));
    strncpy(entry.key_chain, str, NETCFG_TYPE_RIP_AUTH_STRING_LENGTH);
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_AuthKeyChainUnset
 * PURPOSE:
 *      Set RIP Key-chain string to null string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthKeyChainUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    memset(entry.key_chain,0, sizeof(entry.key_chain));

    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SplitHorizonSet
 * PURPOSE:
 *      Set RIP split horizon.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SplitHorizonSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.split_horizon = type;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_SplitHorizonUnset
 * PURPOSE:
 *      Set RIP split horizon to default.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SplitHorizonUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority;
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.split_horizon = NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_VersionSet
 * PURPOSE:
 *      Set RIP global version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      version
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_VersionSet(UI32_T vr_id, UI32_T vrf_id,
                                           enum NETCFG_TYPE_RIP_Global_Version_E version)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.version = version;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_VersionUnset
 * PURPOSE:
 *      Set RIP global version to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_VersionUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id, &entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.version = SYS_DFLT_RIP_GLOBAL_VERSION;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultAdd
 * PURPOSE:
 *      Make RIP to orignate default route.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultAdd(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.default_information = TRUE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultDelete
 * PURPOSE:
 *      Not make RIP to orignate default route.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultDelete(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.default_information = SYS_DFLT_RIP_DEFAULT_ROUTE_ORIGINATE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultMetricSet
 * PURPOSE:
 *      Set RIP default metric value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      metric
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultMetricSet(UI32_T vr_id, UI32_T vrf_id, UI32_T metric)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.default_metric= metric;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultMetricUnset
 * PURPOSE:
 *      Set RIP default metric value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultMetricUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.default_metric= SYS_DFLT_RIP_DEFAULT_METRIC;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_TimerSet
 * PURPOSE:
 *      Set RIP timer value include update, timeout, garbage.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      timer
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_TimerSet(UI32_T vr_id, UI32_T vrf_id, NETCFG_TYPE_RIP_Timer_T *timer)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.timer.update = timer->update;
    entry->instance_value.timer.timeout= timer->timeout;
    entry->instance_value.timer.garbage= timer->garbage;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_TimerUnset
 * PURPOSE:
 *      Set RIP timer include update, timeout,garbage value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_TimerUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.timer.update = SYS_DFLT_RIP_UPDATE_TIME;
    entry->instance_value.timer.timeout= SYS_DFLT_RIP_TIMEOUT_TIME;
    entry->instance_value.timer.garbage= SYS_DFLT_RIP_GARBAGE_TIME;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceDefaultSet
 * PURPOSE:
 *      Set RIP default distance value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      distance
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DistanceDefaultSet(UI32_T vr_id, UI32_T vrf_id, UI32_T distance)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.distance = distance;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceDefaultUnset
 * PURPOSE:
 *      Set RIP default distance value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DistanceDefaultUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.distance = SYS_DFLT_RIP_DISTANCE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_MaxPrefixSet
 * PURPOSE:
 *      Set RIP max prefix number.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      pmax
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_MaxPrefixSet(UI32_T vr_id, UI32_T vrf_id, UI32_T pmax)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.pmax = pmax;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_MaxPrefixUnset
 * PURPOSE:
 *      Set RIP max prefix number to default number.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_MaxPrefixUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.pmax = SYS_DFLT_RIP_PREFIX_MAX;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RecvBuffSizeSet
 * PURPOSE:
 *      Set RIP receive buffer size.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      buff_size
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvBuffSizeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T buff_size)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.recv_buffer_size = buff_size;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RecvBuffSizeUnset
 * PURPOSE:
 *      Set RIP receive buffer size to default size.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvBuffSizeUnset(UI32_T vr_id, UI32_T vrf_id)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry->instance_value.recv_buffer_size = SYS_DFLT_RIP_RECV_BUFF_SIZE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RedistributeSet
* PURPOSE:
*     set redistribute.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string,
*      metric
*      rmap.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_RedistributeSet(UI32_T vr_id, UI32_T vrf_id,UI32_T pro_type, UI32_T metric, char *rmap)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_Redistribute_T *redist = NULL;

    if(pro_type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if(entry->instance_value.redistribute[pro_type] == NULL)
    {
        redist = (NETCFG_TYPE_RIP_Redistribute_T *)malloc(sizeof(NETCFG_TYPE_RIP_Redistribute_T));
        if(redist != NULL)
        {
            memset(redist, 0, sizeof(NETCFG_TYPE_RIP_Redistribute_T));
            if(metric != 0)
                redist->metric = metric;
        
            if(rmap != NULL)
                strcpy(redist->rmap_name, rmap);

            entry->instance_value.redistribute[pro_type] = redist;
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return NETCFG_TYPE_FAIL;
        }
    }
    else
    {
        if(metric != 0)
            entry->instance_value.redistribute[pro_type]->metric = metric;
        
        if(rmap != NULL)
        {
            memset(entry->instance_value.redistribute[pro_type]->rmap_name, 0 ,sizeof(entry->instance_value.redistribute[pro_type]->rmap_name));
            strcpy(entry->instance_value.redistribute[pro_type]->rmap_name, rmap);
        }
    }   
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_RedistributeUnset
* PURPOSE:
*     unset redistribute.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
void NETCFG_OM_RIP_RedistributeUnset(UI32_T vr_id, UI32_T vrf_id,UI32_T pro_type)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry);
    
    if(entry->instance_value.redistribute[pro_type] != NULL)
    {
        free(entry->instance_value.redistribute[pro_type]);
        entry->instance_value.redistribute[pro_type] = NULL;
    }   
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetDistanceTableEntry
 * PURPOSE:
 *      Get distance table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      distance_entry. 
 *
 * OUTPUT:
 *      distance_entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_GetDistanceTableEntry(UI32_T vr_id, UI32_T vrf_id,
                                                         NETCFG_OM_RIP_Distance_T *distance_entry)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if (L_RADIX_LookupNode(&entry->distance_table, &distance_entry->p, &node) == TRUE)
    {
        if(node->info)
        {
            memcpy(distance_entry, node->info, sizeof(NETCFG_OM_RIP_Distance_T));
            L_RADIX_UnlockNode(node);
        }
        else
        {
            L_RADIX_UnlockNode(node);
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return NETCFG_TYPE_FAIL;
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_DISTANCE_TABLE_NOT_EXIST;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceSet
* PURPOSE:
*     Set distance table.
*
* INPUT:
*      vr_id,
*      instance,
*      p,
*      distance,
*      alist.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_DistanceSet(UI32_T vr_id, UI32_T vrf_id, struct prefix *p,
                                            UI32_T distance, char *alist)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_OM_RIP_Distance_T   *distance_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if (L_RADIX_GetNode(&entry->distance_table, p, &node) == TRUE)
    {
        if(node->info != NULL)/*distance table exist*/
        {
            distance_entry = node->info;
            L_RADIX_UnlockNode(node);
        }
        else/*distance table not exist, add new router node*/
        {
            distance_entry = (NETCFG_OM_RIP_Distance_T *)malloc(sizeof(NETCFG_OM_RIP_Distance_T));
            if(distance_entry == NULL)
            {
                L_RADIX_UnlockNode(node);
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
                return NETCFG_TYPE_FAIL;
            }
                
            memset(distance_entry, 0, sizeof(NETCFG_OM_RIP_Distance_T));
            memcpy(&distance_entry->p, p, sizeof(struct prefix));
            node->info = distance_entry;
        }
        distance_entry->distance = distance;
        memset(distance_entry->alist_name, 0 , sizeof(distance_entry->alist_name));
        if(*alist != 0)
        {
            strcpy(distance_entry->alist_name, alist);
        }

    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_FAIL;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceUnset
* PURPOSE:
*     Unset distance table.
*
* INPUT:
*      vr_id,
*      instance,
*      p.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/

UI32_T NETCFG_OM_RIP_DistanceUnset(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)

{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if (L_RADIX_LookupNode(&entry->distance_table, p, &node) == TRUE)
    {
        L_RADIX_UnlockNode(node);
        if(node->info != NULL)
        {
            free(node->info);
            node->info = NULL;
        }
        L_RADIX_UnlockNode(node);
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeSet
* PURPOSE:
*     Set distribute table.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeSet(UI32_T vr_id, UI32_T vrf_id,
                                             UI32_T ifindex, char *list_name,
                                             enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                             enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_If_T       if_entry;

    if((list_name == NULL) || ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
        ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return FALSE;
    
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    if_entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }

    if(NETCFG_OM_RIP_GetInterface(vr_id, &if_entry) != TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
    {
        memset(if_entry.distribute_table.acl_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
        strcpy(if_entry.distribute_table.acl_list[type], list_name);
    }
    else
    {
        memset(if_entry.distribute_table.pre_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
        strcpy(if_entry.distribute_table.pre_list[type], list_name);
    }

    NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeSetAllInterface
* PURPOSE:
*     Set distribute table for all of l3 interface.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeSetAllInterface(UI32_T vr_id, UI32_T vrf_id, char *list_name,
                                                           enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                           enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_If_T       if_entry;

    if((list_name == NULL) || ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
        ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return FALSE;
    
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }

    while(NETCFG_OM_RIP_GetNextInterface(vr_id, &if_entry) == TRUE)
    {
        if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
        {
            memset(if_entry.distribute_table.acl_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
            strcpy(if_entry.distribute_table.acl_list[type], list_name);
        }
        else
        {
            memset(if_entry.distribute_table.pre_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
            strcpy(if_entry.distribute_table.pre_list[type], list_name);
        }
        
        NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}


/* FUNCTION NAME : NETCFG_OM_RIP_DistributeUnset
* PURPOSE:
*     Unset distribute table.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeUnset(UI32_T vr_id, UI32_T vrf_id,
                                                UI32_T ifindex, char *list_name,
                                                enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T original_priority; 
    NETCFG_TYPE_RIP_If_T       if_entry;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    if((list_name == NULL) || ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
        ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return FALSE;
    
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    if_entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    if(NETCFG_OM_RIP_GetInterface(vr_id, &if_entry) != TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
    {
        if(strcmp(if_entry.distribute_table.acl_list[type], list_name) != 0)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
        memset(if_entry.distribute_table.acl_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
    }
    else
    {
        if(strcmp(if_entry.distribute_table.pre_list[type], list_name) != 0)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
        memset(if_entry.distribute_table.pre_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
    }
    
    NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeUnsetAllInterface
* PURPOSE:
*     Unset distribute table for all of l3 interface.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeUnsetAllInterface(UI32_T vr_id, UI32_T vrf_id, char *list_name,
                                                              enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                              enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T original_priority; 
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_If_T       if_entry;

    if((list_name == NULL) || ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
        ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return FALSE;
    
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }

    while(NETCFG_OM_RIP_GetNextInterface(vr_id, &if_entry) == TRUE)
    {
        if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
        {
            if(strcmp(list_name, if_entry.distribute_table.acl_list[type]) == 0)/*if the list_name not match, cannot delete*/
            {    
                memset(if_entry.distribute_table.acl_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
                NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
            }
        }
        else
        {
            if(strcmp(list_name, if_entry.distribute_table.pre_list[type]) == 0)/*if the list_name not match, cannot delete*/
            {    
                memset(if_entry.distribute_table.pre_list[type], 0, sizeof(char) * (SYS_ADPT_ACL_MAX_NAME_LEN + 1));
                NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &if_entry);
            }
        }
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_PassiveIfSet
* PURPOSE:
*     Set passive inteface
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_PassiveIfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority; 
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry.pass_if = TRUE;
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_PassiveIfUnset
* PURPOSE:
*     Unset passive inteface
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_PassiveIfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority; 
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry.pass_if = FALSE;
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}


/* FUNCTION NAME : NETCFG_OM_RIP_CheckNeighbor
 * PURPOSE:
 *      Check if the neighbor exist.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_CheckNeighbor(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    node = L_RADIX_NodeLookupIPv4(&entry->neignbor_table, addr);
    if (node != NULL)
    {
        L_RADIX_UnlockNode(node);
        if(node->info)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return TRUE;
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
}

/* FUNCTION NAME : NETCFG_OM_RIP_NeighborSet
 * PURPOSE:
 *      Set neighbor table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NeighborSet(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    node = L_RADIX_NodeGetIPv4(&entry->neignbor_table, addr);
    if (node != NULL)
    {
        if(node->info != NULL)/*node exist*/
        {
            L_RADIX_UnlockNode(node);
        }
        else/*node not exist, but created*/
        {
            node->info = &entry->neignbor_table;
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_FAIL;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NeighborUnset
 * PURPOSE:
 *      Unset neighbor table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NeighborUnset(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if ((node = L_RADIX_NodeLookupIPv4(&entry->neignbor_table, addr)) != NULL)
    {
        L_RADIX_UnlockNode(node);
        if(node->info != NULL)
        {
            node->info = NULL;
        }
        L_RADIX_UnlockNode(node);
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkIfSet
 * PURPOSE:
 *      Set network ifname status.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkIfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority; 
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry)!= TRUE)
    {   
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    }
    
    entry.network_if = TRUE;
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
    {
        result = NETCFG_TYPE_OK;
    }
        
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkIfUnset
 * PURPOSE:
 *      Unset network ifname status.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkIfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    UI32_T original_priority; 
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_RIP_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInterface(vr_id, &entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    entry.network_if = FALSE;
    
    if(NETCFG_OM_RIP_UpdateInterfaceEntry(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkTableSet
 * PURPOSE:
 *      Set network table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkTableSet(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)
{
    UI32_T original_priority; 
    UI32_T mask;
    L_RADIX_Node_T *node = NULL;
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_If_T if_entry;

    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    if (L_RADIX_GetNode(&entry->network_table, p, &node) == TRUE)
    {
        if(node)
        {
            if(node->info)/*node exist*/
            {
                L_RADIX_UnlockNode(node);
            }
            else/*node not exist, add new router node*/
            {
                node->info = &entry->network_table;
            }
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return NETCFG_TYPE_FAIL;
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_FAIL;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkTableUnset
 * PURPOSE:
 *      Unset network table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkTableUnset(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)

{
    UI32_T original_priority; 
    UI32_T mask, node_mask;
    L_RADIX_Node_T *node;
    struct prefix tem_p;
    NETCFG_OM_RIP_Instance_T *entry = NULL;
    NETCFG_TYPE_RIP_If_T if_entry;

    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if (L_RADIX_LookupNode(&entry->network_table, p, &node) == TRUE)
    {
        L_RADIX_UnlockNode(node);
        if(node->info != NULL)
        {
            node->info = NULL;
        }
        L_RADIX_UnlockNode(node);

    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_RIP_CheckNetworkTable
 * PURPOSE:
 *      check if network table is exist.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_CheckNetworkTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)
{
    UI32_T original_priority; 
    L_RADIX_Node_T *node;
    NETCFG_OM_RIP_Instance_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id,vrf_id,&entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    if (L_RADIX_LookupNode(&entry->network_table, p, &node) == TRUE)
    {
        L_RADIX_UnlockNode(node);
        if(node->info)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return TRUE;
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextNetworkTableEntry
 * PURPOSE:
 *      Get next network table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      p.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (p.prefixlen, p.u.prefix4.s_addr).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextNetworkTableEntry(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)
{
    UI32_T original_priority; 
    BOOL_T result = FALSE;
    L_RADIX_Node_T *node = NULL;
    L_RADIX_Node_T *pre_node;
    NETCFG_OM_RIP_Instance_T *vrf_entry = NULL;

    if(p == NULL)
    {
        return FALSE;
    }
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id, vrf_id, &vrf_entry)!= TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    else
    {    
        for (node = L_RADIX_GetTableTop (&vrf_entry->network_table); node; node = L_RADIX_GetNext (pre_node))
        {
            if (node->info)
            {
                if (L_STDLIB_Ntoh32(node->p.u.prefix4.s_addr) >= L_STDLIB_Ntoh32(p->u.prefix4.s_addr))
                {
                    if(node->p.u.prefix4.s_addr == p->u.prefix4.s_addr)
                    {
                        if(node->p.prefixlen > p->prefixlen)
                        {
                            L_RADIX_UnlockNode (node);
                            result = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        L_RADIX_UnlockNode (node);
                        result = TRUE;
                        break;
                    }
                }
            }
            pre_node = node;
        }

        if(result == TRUE)
        {
            p->u.prefix4.s_addr = node->p.u.prefix4.s_addr;
            p->prefixlen = node->p.prefixlen;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextNetworkTable
 * PURPOSE:
 *      Get next network table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      p.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (p.prefixlen, p.u.prefix4.s_addr).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextNetworkTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)
{
    BOOL_T result = FALSE;
    L_RADIX_Node_T *node = NULL;
    L_RADIX_Node_T *pre_node;
    NETCFG_OM_RIP_Instance_T *vrf_entry = NULL;

    if(p == NULL)
    {
        return FALSE;
    }
    
    if(NETCFG_OM_RIP_GetInstance(vr_id, vrf_id, &vrf_entry)!= TRUE)
    {
        return FALSE;
    }
    else
    {    
        for (node = L_RADIX_GetTableTop (&vrf_entry->network_table); node; node = L_RADIX_GetNext (pre_node))
        {
            if (node->info)
            {
                if (L_STDLIB_Ntoh32(node->p.u.prefix4.s_addr) >= L_STDLIB_Ntoh32(p->u.prefix4.s_addr))
                {
                    if(node->p.u.prefix4.s_addr == p->u.prefix4.s_addr)
                    {
                        if(node->p.prefixlen > p->prefixlen)
                        {
                            L_RADIX_UnlockNode (node);
                            result = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        L_RADIX_UnlockNode (node);
                        result = TRUE;
                        break;
                    }
                }
            }
            pre_node = node;
        }

        if(result == TRUE)
        {
            p->u.prefix4.s_addr = node->p.u.prefix4.s_addr;
            p->prefixlen = node->p.prefixlen;
        }
    }
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextNeighborTable
 * PURPOSE:
 *      Get next neighbor table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id
 *      p
 *
 * OUTPUT:
 *      p.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (p.u.prefix4.s_addr).
 *      If key is (0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextNeighborTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p)
{
    UI32_T original_priority; 
    BOOL_T result = FALSE;
    L_RADIX_Node_T *node = NULL;
    L_RADIX_Node_T *pre_node = NULL;
    NETCFG_OM_RIP_Instance_T *vrf_entry = NULL;

    if(p == NULL)
    {
        return FALSE;
    }
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id, vrf_id, &vrf_entry)!= TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    else
    {    
        for (node = L_RADIX_GetTableTop (&vrf_entry->neignbor_table); node; node = L_RADIX_GetNext (pre_node))
        {
            if (node->info)
            {
                if (L_STDLIB_Ntoh32(node->p.u.prefix4.s_addr) > L_STDLIB_Ntoh32(p->u.prefix4.s_addr))
                {
                    L_RADIX_UnlockNode (node);
                    result = TRUE;
                    break;
                }
            }
            pre_node = node;
        }

        if(result == TRUE)
        {
            p->u.prefix4.s_addr = node->p.u.prefix4.s_addr;
            p->prefixlen = node->p.prefixlen;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextDistanceTable
 * PURPOSE:
 *      Get next distance table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.p.prefixlen,entry.p.u.prefix4.s_addr).
 *      If key is (0,0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextDistanceTable(UI32_T vr_id, UI32_T vrf_id, NETCFG_OM_RIP_Distance_T *entry)
{
    UI32_T original_priority; 
    BOOL_T result = FALSE;
    L_RADIX_Node_T *node = NULL;
    L_RADIX_Node_T *pre_node;
    NETCFG_OM_RIP_Instance_T *vrf_entry = NULL;

    if(entry == NULL)
    {
        return FALSE;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetInstance(vr_id, vrf_id, &vrf_entry)!= TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    else
    {    
        for (node = L_RADIX_GetTableTop (&vrf_entry->distance_table); node; node = L_RADIX_GetNext (pre_node))
        {
            if (node->info)
            {
                if (L_STDLIB_Ntoh32(node->p.u.prefix4.s_addr) >= L_STDLIB_Ntoh32(entry->p.u.prefix4.s_addr))
                {
                    if(node->p.u.prefix4.s_addr == entry->p.u.prefix4.s_addr)
                    {
                        if(node->p.prefixlen > entry->p.prefixlen)
                        {
                            L_RADIX_UnlockNode (node);
                            result = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        L_RADIX_UnlockNode (node);
                        result = TRUE;
                        break;
                    }
                }
            }
            pre_node = node;
        }

        if(result == TRUE)
        {
            memcpy(entry, node->info, sizeof(NETCFG_OM_RIP_Distance_T));
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DebugSet
* PURPOSE:
*     RIP debug on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      vr_id.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DebugSet(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.nsm_all_status =TRUE;
    vr_entry->debug.event_all_status = TRUE;
    vr_entry->debug.packet_all_status = TRUE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_DebugUnset
* PURPOSE:
*     RIP undebug on config mode.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DebugUnset(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.nsm_all_status =FALSE;
    vr_entry->debug.event_all_status = FALSE;
    vr_entry->debug.packet_all_status = FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_EventDebugSet
* PURPOSE:
*     RIP event debug on config mode.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_EventDebugSet(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.event_all_status = TRUE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_EventDebugUnset
* PURPOSE:
*     RIP event undebug on config mode.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_EventDebugUnset(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.event_all_status = FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NsmDebugSet
* PURPOSE:
*     RIP nsm debug on config mode.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_NsmDebugSet(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.nsm_all_status = TRUE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_NsmDebugUnset
* PURPOSE:
*     RIP nsm undebug on config mode.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_NsmDebugUnset(UI32_T vr_id)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    vr_entry->debug.nsm_all_status = FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_PacketDebugSet
* PURPOSE:
*     RIP packet debug on config mode.
*
* INPUT:
*      vr_id,
*      type.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_PacketDebugSet(UI32_T vr_id, NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }

    vr_entry->debug.packet_all_status = TRUE;
    switch(type)
    {
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE:
            vr_entry->debug.packet_send_status = TRUE;
            vr_entry->debug.packet_recv_status = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND: 
            vr_entry->debug.packet_send_status = TRUE;
        break;
        
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE: 
            vr_entry->debug.packet_recv_status = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL: 
            vr_entry->debug.packet_send_status = TRUE;
            vr_entry->debug.packet_recv_status = TRUE;
            vr_entry->debug.packet_detail_status = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL: 
            vr_entry->debug.packet_send_status = TRUE;
            vr_entry->debug.packet_detail_status = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL: 
            vr_entry->debug.packet_recv_status = TRUE;
            vr_entry->debug.packet_detail_status = TRUE;
        break;

        default:
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_PacketDebugUnset
* PURPOSE:
*     RIP packet undebug on config mode.
*
* INPUT:
*      vr_id,
*      type.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_PacketDebugUnset(UI32_T vr_id, NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
    
    switch(type)
    {
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE:
            vr_entry->debug.packet_all_status = FALSE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND:
            if(vr_entry->debug.packet_recv_status == TRUE)
            {
                vr_entry->debug.packet_send_status = FALSE;
                vr_entry->debug.packet_recv_status = FALSE;
            }
            else
                vr_entry->debug.packet_all_status = FALSE;
        break;
        
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE: 
            if(vr_entry->debug.packet_send_status == TRUE)
            {
                vr_entry->debug.packet_send_status = FALSE;
                vr_entry->debug.packet_recv_status = FALSE;
            }
            else
                vr_entry->debug.packet_all_status = FALSE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL: 
            vr_entry->debug.packet_detail_status = FALSE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL: 
            if(vr_entry->debug.packet_recv_status == TRUE)
            {
                vr_entry->debug.packet_send_status = FALSE;
                vr_entry->debug.packet_recv_status = FALSE;
            }
            else
                vr_entry->debug.packet_all_status = FALSE;
            vr_entry->debug.packet_detail_status = FALSE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL: 
            if(vr_entry->debug.packet_send_status == TRUE)
            {
                vr_entry->debug.packet_send_status = FALSE;
                vr_entry->debug.packet_recv_status = FALSE;
            }
            else
                vr_entry->debug.packet_all_status = FALSE;
            vr_entry->debug.packet_detail_status = FALSE;
        break;

        default:
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
            return FALSE;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_RIP_GetDebugStatus
* PURPOSE:
*     Get RIP debug status.
*
* INPUT:
*      vr_id.
*
* OUTPUT:
*      status.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_GetDebugStatus(UI32_T vr_id, NETCFG_TYPE_RIP_Debug_Status_T *status)
{
    NETCFG_OM_RIP_Master_T *vr_entry = NULL;
    UI32_T original_priority; 
    
    if(status == NULL)
        return FALSE;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(rip_om_sem_id);
    if(NETCFG_OM_RIP_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
        return FALSE;
    }
        
    memcpy(status, &vr_entry->debug, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(rip_om_sem_id, original_priority);
    return TRUE;
}

