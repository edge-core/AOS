/* Module Name:NETCFG_OM_PIM.C
 * Purpose: To store PIM configuration information.
 *
 * Notes:
 *      
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2008/05/18     --- Hongliang, Create
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
#include "netcfg_om_pim.h"
#include "netcfg_type.h"
#include "l_radix.h"
#include "table.h"

#define MAX_NBR_OF_VR                     1
#define MAX_NBR_OF_VRF_PER_VR             1
#define MAX_NBR_OF_INTERFACE              256
#define NETCFG_OM_PIM_SYS_DFLT_VR_ID      0
#define NETCFG_OM_PIM_SYS_DFLT_VRF_ID     0

#define GETMAXVALUE(v1, v2)  ((v1 <= v2)? v1:v2)
#define PREFIXLEN2MASK(len)  (~((1 << (32 - len)) - 1))
#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

typedef    struct NETCFG_OM_PIM_Master_S
{
    UI32_T                          vr_id;
    L_SORT_LST_List_T               pim_ifs;
    NETCFG_OM_PIM_Instance_T       *pim_instances[MAX_NBR_OF_VRF_PER_VR];
} NETCFG_OM_PIM_Master_T;

static int    NETCFG_OM_PIM_InterfaceCompare(void *elm1,void *elm2);
static BOOL_T NETCFG_OM_PIM_GetMasterEntry(UI32_T vr_id, NETCFG_OM_PIM_Master_T **entry);
static BOOL_T NETCFG_OM_PIM_GetInstance(UI32_T vr_id,UI32_T instance,NETCFG_OM_PIM_Instance_T *entry);
static BOOL_T NETCFG_OM_PIM_GetInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry);
static BOOL_T NETCFG_OM_PIM_GetNextInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry);
static void   NETCFG_OM_PIM_DeleteInstance(UI32_T vr_id, UI32_T instance);
static BOOL_T NETCFG_OM_PIM_AddVr(UI32_T vr_id);
static BOOL_T NETCFG_OM_PIM_AddInstance(UI32_T vr_id, UI32_T instance);
static BOOL_T NETCFG_OM_PIM_UpdateInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry);
static struct netcfg_om_pim_crp *
  NETCFG_OM_PIM_RpCandidateLookup (struct netcfg_om_pim_bsr *bsr, char *ifname);

static UI32_T pim_om_sem_id;   
static NETCFG_OM_PIM_Master_T *pim_master[MAX_NBR_OF_VR];/*PIM database in om*/

/* FUNCTION NAME : NETCFG_OM_PIM_Init
 * PURPOSE:Init NETCFG_OM_PIM database, create semaphore
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
void NETCFG_OM_PIM_Init(void)
{
    UI32_T i;

    for(i = 0; i < MAX_NBR_OF_VR; i++)
        pim_master[i] = NULL;  
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &pim_om_sem_id) != SYSFUN_OK)
    {
        printf("NETCFG_OM_PIM_Init : Can't create semaphore\n");
    }
    if(NETCFG_OM_PIM_AddVr(NETCFG_OM_PIM_SYS_DFLT_VR_ID) == FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_PIM_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_PIM_Init : Can't add vr.\n");
    }
    if(NETCFG_OM_PIM_AddInstance(NETCFG_OM_PIM_SYS_DFLT_VR_ID, NETCFG_OM_PIM_SYS_DFLT_VRF_ID) == FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_PIM_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_PIM_Init : Can't add vrf.\n");
    }

    return;
}



/* FUNCTION NAME : NETCFG_OM_PIM_RpCandidateLookup
 * PURPOSE:
 *      Find the crp.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
struct netcfg_om_pim_crp *
NETCFG_OM_PIM_RpCandidateLookup (struct netcfg_om_pim_bsr *bsr, char *ifname)
{
  struct netcfg_om_pim_crp *crp;

  for (crp = bsr->crp_head; crp; crp = crp->next)
    {
      if (! strcmp (crp->crp_ifname, ifname))
        return crp;
    }
  return NULL;
}
/* FUNCTION NAME : NETCFG_OM_PIM_RpCandidateRemove
 * PURPOSE:
 *      remove the crp.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
BOOL_T
NETCFG_OM_PIM_RpCandidateRemove (struct netcfg_om_pim_bsr *bsr, char *ifname)
{
    struct netcfg_om_pim_crp *crp = NULL;

    crp = NETCFG_OM_PIM_RpCandidateLookup(bsr, ifname);
    if ( NULL == crp )
        return FALSE;
    /* Remove this crp */
    if (crp->prev)
        crp->prev->next = crp->next;
    else
        bsr->crp_head = crp->next;
  
    if (crp->next)
        crp->next->prev = crp->prev;
    else
        bsr->crp_tail = crp->prev;
  
    crp->next = crp->prev = NULL;

    if (crp->crp_ifname)
        free(crp->crp_ifname);

    /* Free the Group Range table */
    if (crp->grp_range)
        route_table_finish (crp->grp_range);    
    free(crp);

    return TRUE;
}
/* FUNCTION NAME : NETCFG_OM_PIM_RpCandidateInsert
 * PURPOSE:
 *      insert the crp.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
BOOL_T
NETCFG_OM_PIM_RpCandidateInsert (struct netcfg_om_pim_bsr *bsr, char *ifname)
{
    struct netcfg_om_pim_crp *crp = NULL;

    crp = malloc(sizeof(struct netcfg_om_pim_crp));
    if ( NULL == crp )
        return FALSE;
    crp->crp_ifname = malloc(strlen(ifname)+1);
    if ( NULL == crp->crp_ifname )
    {
        free(crp);
        return FALSE;
    }
    crp->grp_range = route_table_init (); 
    if (crp->grp_range == NULL)
    {
      free(crp->crp_ifname);
      free(crp);
      return FALSE;
    }
    crp->num_grp_range = 0;

    strcpy(crp->crp_ifname, ifname);
    /* insert this CRP to the list */
    crp->next = NULL;
    crp->prev = bsr->crp_tail;
    
    if (bsr->crp_tail)
      bsr->crp_tail->next = crp;
    else
      bsr->crp_head = crp;
    
    bsr->crp_tail = crp;
    
    return TRUE;

}
/* FUNCTION NAME : NETCFG_OM_PIM_RpAddressLookup
 * PURPOSE:
 *      Find the rp with some address.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
struct netcfg_om_pim_rp_conf *
NETCFG_OM_PIM_RpAddressLookup (NETCFG_OM_PIM_Instance_T *instance, UI32_T address)
{
  struct netcfg_om_pim_rp_conf *rp;

  for (rp = instance->st_rp_head; rp; rp = rp->next)
    {
      if ( rp->rp_addr == address )
        return rp;
    }
  return NULL;
}

/* FUNCTION NAME : NETCFG_OM_PIM_RpAddressRemove
 * PURPOSE:
 *      remove the rp.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
BOOL_T
NETCFG_OM_PIM_RpAddressRemove (NETCFG_OM_PIM_Instance_T *instance, UI32_T address)
{
    struct netcfg_om_pim_rp_conf *rp = NULL;

    rp = NETCFG_OM_PIM_RpAddressLookup(instance, address);
    if ( NULL == rp )
        return FALSE;
    /* Remove this rp */
    if (rp->prev)
        rp->prev->next = rp->next;
    else
        instance->st_rp_head = rp->next;
  
    if (rp->next)
        rp->next->prev = rp->prev;
    else
        instance->st_rp_tail = rp->prev;
  
    rp->next = rp->prev = NULL;
    /*  free this rp memory */
    free(rp);

    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_PIM_RpAddressInsert
 * PURPOSE:
 *      insert an RP address.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NULL/!NULL
 *
 * NOTES:
 *      
 */
BOOL_T
NETCFG_OM_PIM_RpAddressInsert (NETCFG_OM_PIM_Instance_T *instance, UI32_T address)
{
    struct netcfg_om_pim_rp_conf *rp = NULL;

    rp = malloc(sizeof(struct netcfg_om_pim_rp_conf));
    if ( NULL == rp )
        return FALSE;
    rp->rp_addr = address;

    /* insert this rp to the list */
    rp->next = NULL;
    rp->prev = instance->st_rp_tail;
    
    if (instance->st_rp_tail)
      instance->st_rp_tail->next = rp;
    else
      instance->st_rp_head = rp;
    
    instance->st_rp_tail = rp;
    
    return TRUE;

}
/* FUNCTION NAME : NETCFG_OM_PIM_AddVr
 * PURPOSE:
 *      Add a PIM master entry when add a VR.
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
BOOL_T NETCFG_OM_PIM_AddVr(UI32_T vr_id)
{
    UI32_T i;
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_PIM_Master_T *entry = NULL;


    entry = (NETCFG_OM_PIM_Master_T *)malloc(sizeof(NETCFG_OM_PIM_Master_T));
    if(entry != NULL)
    {
        memset(entry, 0, sizeof(NETCFG_OM_PIM_Master_T));
        entry->vr_id = vr_id;

        if(L_SORT_LST_Create(&entry->pim_ifs,
                              MAX_NBR_OF_INTERFACE,
                              sizeof(NETCFG_TYPE_PIM_If_T),
                              NETCFG_OM_PIM_InterfaceCompare)==FALSE)
        {
            free(entry);
            entry = NULL;
            return FALSE;
        }
        original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
        pim_master[vr_id] = entry;
        result = TRUE;        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_DeleteVr
 * PURPOSE:
 *      Delete a PIM master entry when delete a VR.
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
void NETCFG_OM_PIM_DeleteVr(UI32_T vr_id)
{
    UI32_T original_priority,i;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    for(i=0;i<MAX_NBR_OF_VRF_PER_VR;i++)
    {
        if(pim_master[vr_id]->pim_instances[i] != NULL) 
        {
            NETCFG_OM_PIM_DeleteInstance(vr_id, i);
        }
    }
    
    L_SORT_LST_Delete_All(&pim_master[vr_id]->pim_ifs);

    free(pim_master[vr_id]);
    pim_master[vr_id] = NULL;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_PIM_DeleteAllRipMasterEntry
 * PURPOSE:
 *          Remove all PIM master entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_PIM_DeleteAllRipMasterEntry(void)
{
    UI32_T original_priority,i,j;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    for(i=0;i<MAX_NBR_OF_VR;i++)
    {
        for(j=0;j<MAX_NBR_OF_VRF_PER_VR;j++)
        {
            if(pim_master[i]->pim_instances[j] != NULL) 
            {
                NETCFG_OM_PIM_DeleteInstance(i, j);
            }
        }
        
        L_SORT_LST_Delete_All(&pim_master[i]->pim_ifs);

        free(pim_master[i]);
        pim_master[i] = NULL;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetMasterEntry
 * PURPOSE:
 *      Get a pim master entry with specific vr_id.
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
static BOOL_T NETCFG_OM_PIM_GetMasterEntry(UI32_T vr_id,NETCFG_OM_PIM_Master_T **entry)
{
    BOOL_T     result = FALSE;

    if(pim_master[vr_id] != NULL)
    {
        *entry = pim_master[vr_id];
        result = TRUE;
    }

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_AddInstance
 * PURPOSE:
 *      Add a PIM instance entry when router pim enable in a instance.
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
BOOL_T NETCFG_OM_PIM_AddInstance(UI32_T vr_id, UI32_T instance)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_PIM_Master_T    *vr_entry = NULL;
    NETCFG_OM_PIM_Instance_T  *vrf_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }

    vrf_entry = (NETCFG_OM_PIM_Instance_T *)malloc(sizeof(NETCFG_OM_PIM_Instance_T));

    if(vrf_entry != NULL)
    {
        memset(vrf_entry,0,sizeof(NETCFG_OM_PIM_Instance_T));
        vrf_entry->st_rp_head = NULL;
        vrf_entry->st_rp_tail = NULL;
        vrf_entry->bsr.crp_head = NULL;
        vrf_entry->bsr.crp_tail = NULL;
        vrf_entry->bsr.my_hash_masklen = SYS_DFLT_PIM_BSR_HASH_MASKLEN;
        vrf_entry->bsr.my_priority = SYS_DFLT_PIM_BSR_HASH_MASKLEN;
        vrf_entry->bsr.ifname = NULL;
        vrf_entry->reg_checksum_filter = NULL;
        vrf_entry->rp_reg_filter = NULL;
        vrf_entry->reg_suppression = SYS_DFLT_PIM_REGISTER_SUPPRESSION_TIME;
        vrf_entry->rp_reg_kat = SYS_DFLT_PIM_REGISTER_KAT;
        vrf_entry->reg_rate_limit = SYS_DFLT_PIM_REGISTER_RATE_LIMIT;
        vrf_entry->spt_switch = NULL;
        vrf_entry->jp_periodic = SYS_DFLT_PIM_JOINE_PRUNE_PERIODIC;
        vr_entry->pim_instances[instance] = vrf_entry;   
        result = TRUE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_DeleteInstance
 * PURPOSE:
 *      Delete a PIM instance entry when router pim disable in a instance.
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
static void NETCFG_OM_PIM_DeleteInstance(UI32_T vr_id, UI32_T instance)
{
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    NETCFG_TYPE_PIM_If_T   if_entry;


    memset(&if_entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id,&vr_entry) == TRUE)
    {
        if(vr_entry->pim_instances[instance] != NULL)
        {             
            free(vr_entry->pim_instances[instance]);
            vr_entry->pim_instances[instance] = NULL;
        }
    }
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetInstanceEntry
 * PURPOSE:
 *      Get a pim instance entry.
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
BOOL_T NETCFG_OM_PIM_GetInstance(UI32_T vr_id,UI32_T instance, NETCFG_OM_PIM_Instance_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    if(entry == NULL)
    {
        return FALSE;
    }
    
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        return FALSE;
    }

    if(vr_entry->pim_instances[instance] != NULL)
    {
        memcpy(entry,vr_entry->pim_instances[instance], sizeof(NETCFG_OM_PIM_Instance_T));
        result = TRUE;
    }

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetInstance
 * PURPOSE:
 *      Get a pim instance entry.
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
BOOL_T NETCFG_OM_PIM_GetInstanceEntry(UI32_T vr_id,UI32_T instance,NETCFG_OM_PIM_Instance_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        memcpy(entry, vr_entry->pim_instances[instance], sizeof(NETCFG_OM_PIM_Instance_T));
        result = TRUE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_AddInterface
 * PURPOSE:
 *      Make a link-up l3 interface up in the PIM
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
BOOL_T NETCFG_OM_PIM_AddInterface(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_PIM_If_T entry;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);    
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id,&vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    memset(&entry, 0,sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    entry.mode = NETCFG_TYPE_PIM_PIM_MODE_NONE;
    entry.hello_period = SYS_DFLT_PIM_HELLO_INTERVAL;
    entry.holdtime = SYS_DFLT_PIM_HELLO_HOLDTIME;
    entry.dr_priority = SYS_DFLT_PIM_DR_PRIORITY;
    entry.exclude_genid_yes = FALSE;
    entry.nbr_flt = NULL;
    result = L_SORT_LST_Set(&vr_entry->pim_ifs, &entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_DeleteInterface
 * PURPOSE:
 *      Make a interface down in the PIM
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
void NETCFG_OM_PIM_DeleteInterface(UI32_T vr_id,UI32_T ifindex)
{
    UI32_T original_priority;
    NETCFG_TYPE_PIM_If_T entry;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    memset(&entry, 0,sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) == TRUE)
    {    
        L_SORT_LST_Delete(&vr_entry->pim_ifs, &entry);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_PIM_RifUp
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
BOOL_T NETCFG_OM_PIM_RifUp(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_PIM_If_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetInterface(vr_id, entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    
    entry->if_status = TRUE;
    
    if(NETCFG_OM_PIM_UpdateInterface(vr_id, &entry) == TRUE)
    {
        result = TRUE;
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_RifDown
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
BOOL_T NETCFG_OM_PIM_RifDown(UI32_T vr_id,UI32_T ifindex)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_TYPE_PIM_If_T *entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetInterface(vr_id, entry)!= TRUE)
    {    
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    
    entry->if_status = FALSE;
    
    if(NETCFG_OM_PIM_UpdateInterface(vr_id, &entry) == TRUE)
    {
        result = TRUE;
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetInterfaceEntry
 * PURPOSE:
 *      Get a pim interface entry.
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
BOOL_T NETCFG_OM_PIM_GetInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);

    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    
    result = L_SORT_LST_Get(&vr_entry->pim_ifs, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetInterface
 * PURPOSE:
 *      Get a pim interface entry.
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
static BOOL_T NETCFG_OM_PIM_GetInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        return FALSE;
    }

    result = L_SORT_LST_Get(&vr_entry->pim_ifs, entry);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_GetNextInterface
 * PURPOSE:
 *      Get next pim interface entry.
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
static BOOL_T NETCFG_OM_PIM_GetNextInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        return FALSE;
    }
    if(entry->ifindex == 0)
    {
        result = L_SORT_LST_Get_1st(&vr_entry->pim_ifs, entry);
    }
    else
    {
        result = L_SORT_LST_Get_Next(&vr_entry->pim_ifs, entry);
    }

    return  result;
}


/* FUNCTION NAME : NETCFG_OM_PIM_GetNextInterfaceEntry
 * PURPOSE:
 *      Get next pim interface entry.
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
BOOL_T NETCFG_OM_PIM_GetNextInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);

    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(entry->ifindex == 0)
        result = L_SORT_LST_Get_1st(&vr_entry->pim_ifs, entry);
    else
        result = L_SORT_LST_Get_Next(&vr_entry->pim_ifs, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_UpdateInterface
 * PURPOSE:
 *      Update the pim interface.
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
static BOOL_T NETCFG_OM_PIM_UpdateInterface(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    NETCFG_TYPE_PIM_If_T *mem_entry = NULL;
    NETCFG_TYPE_PIM_If_T local_entry;

    if(entry == NULL)
        return FALSE;
    
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {       
        result = FALSE;
        goto exit;
    }
    
    memset(&local_entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    local_entry.ifindex = entry->ifindex;

    mem_entry = L_SORT_LST_GetPtr(&vr_entry->pim_ifs, &local_entry);
    if(mem_entry == NULL)
    {
        result = FALSE;
    }
    else
    {
        memcpy(mem_entry, entry, sizeof(NETCFG_TYPE_PIM_If_T));
        result = TRUE;
    }
exit:
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_UpdateInterfaceEntry
 * PURPOSE:
 *      Update the pim interface entry.
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
BOOL_T NETCFG_OM_PIM_UpdateInterfaceEntry(UI32_T vr_id, NETCFG_TYPE_PIM_If_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    NETCFG_TYPE_PIM_If_T *mem_entry = NULL;
    NETCFG_TYPE_PIM_If_T local_entry;
    UI32_T     original_priority;

    if(entry == NULL)
        return FALSE;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {       
        result = FALSE;
        goto exit;
    }
    
    memset(&local_entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    local_entry.ifindex = entry->ifindex;

    mem_entry = L_SORT_LST_GetPtr(&vr_entry->pim_ifs, &local_entry);
    if(mem_entry == NULL)
    {
        result = FALSE;
    }
    else
    {
        memcpy(mem_entry, entry, sizeof(NETCFG_TYPE_PIM_If_T));
        result = TRUE;
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_UpdateInstanceEntry
 * PURPOSE:
 *      Update the pim instance entry.
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
BOOL_T NETCFG_OM_PIM_UpdateInstanceEntry(UI32_T vr_id, UI32_T instance, NETCFG_OM_PIM_Instance_T *entry)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    NETCFG_OM_PIM_Instance_T *mem_entry = NULL;
    NETCFG_OM_PIM_Instance_T local_entry;
    UI32_T     original_priority;

    if(entry == NULL)
        return FALSE;
    /* get semaphore */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);    
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        result = FALSE;
        goto exit;
    }    
    memset(&local_entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    mem_entry = vr_entry->pim_instances[instance];
    if(mem_entry == NULL)
    {
        result = FALSE;
    }
    else
    {
        memcpy(mem_entry, entry, sizeof(NETCFG_OM_PIM_Instance_T));
        result = TRUE;
    }
exit:
    /* give semaphore */
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_InterfaceCompare
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
static int NETCFG_OM_PIM_InterfaceCompare(void *elm1,void *elm2)
{
    NETCFG_TYPE_PIM_If_T *element1, *element2;
    
    element1 = (NETCFG_TYPE_PIM_If_T *)elm1;
    element2 = (NETCFG_TYPE_PIM_If_T *)elm2;

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
}   /* end of NETCFG_OM_PIM_InterfaceCompare  */

/* FUNCTION NAME : NETCFG_OM_PIM_SignalIpAddrAdd
 * PURPOSE:
 *      When add a primary IP address signal PIM.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      ip_addr,
 *      ip_mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_PIM_SignalIpAddrAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T original_priority; 
    UI32_T result, node_mask;
    NETCFG_TYPE_PIM_If_T entry;
    struct prefix   p;
    
    memset(&p, 0 ,sizeof(struct prefix));
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.ip_addr = ip_addr;
    entry.ip_mask = ip_mask;
    entry.if_status = TRUE;   
    if(NETCFG_OM_PIM_UpdateInterface(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_SignalIpAddrDelete
 * PURPOSE:
 *      When delete a primary IP address signal PIM.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      ip_addr,
 *      ip_mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_PIM_SignalIpAddrDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T original_priority; 
    UI32_T result, node_mask;
    NETCFG_TYPE_PIM_If_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);
    if(NETCFG_OM_PIM_GetInterface(vr_id, &entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return NETCFG_TYPE_INTERFACE_NOT_UP;
    }
    entry.ip_addr = 0;
    entry.ip_mask = 0;
    entry.if_status = FALSE;    
    if(NETCFG_OM_PIM_UpdateInterface(vr_id, &entry) == TRUE)
        result = NETCFG_TYPE_OK;
                
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return result;
}
/* FUNCTION NAME : NETCFG_OM_PIM_GetRpCandidate
 * PURPOSE:
 *      Get the RP candidate.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      RP.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_GetRpCandidate(UI32_T vr_id,
                                                  UI32_T instance,
                                                  UI8_T* ifname,
                                                  struct netcfg_om_pim_crp *candidate_rp)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        crp = NETCFG_OM_PIM_RpCandidateLookup(&vr_entry->pim_instances[instance]->bsr,ifname);
        if ( crp == NULL )
            goto exit;
        if ( !FLAG_ISSET(crp->config, NETCFG_OM_PIM_CRP_CONFIG_DEFAULT_RANGE) )
            goto exit;
        memcpy(candidate_rp, crp, sizeof(crp));
        result = TRUE;
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}
/* FUNCTION NAME : NETCFG_OM_PIM_GetRpCandidateGroupAddr
 * PURPOSE:
 *      Get the RP candidate group address
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      RP.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_GetRpCandidateGroupAddr(UI32_T vr_id,
                                                              UI32_T instance,
                                                              UI8_T* ifname,
                                                              UI32_T groupAddr,
                                                              UI32_T maskAddr)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    struct netcfg_om_pim_prefix p;
    struct route_node *rn;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        crp = NETCFG_OM_PIM_RpCandidateLookup(&vr_entry->pim_instances[instance]->bsr,ifname);
        if ( crp == NULL )
            goto exit;
        p.family = AF_LOCAL;
        p.group = (groupAddr & maskAddr);
        p.src = maskAddr;
        p.prefixlen = IPV4_MAX_BITLEN * 2;
        /* search this route node */
        rn = route_node_lookup (crp->grp_range, (struct prefix*)&p);
        if (rn)
        {
            route_unlock_node (rn);
            result = TRUE;
            goto exit;
        }
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}

/* FUNCTION NAME : NETCFG_OM_PIM_InsertNewRpCandidate
 * PURPOSE:
 *      Insert new RP candidate.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      .
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_SetPimRpCandidate( UI32_T vr_id,
                                                       UI32_T instance,
                                                       UI8_T* ifname )
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        result = NETCFG_OM_PIM_RpCandidateInsert(&vr_entry->pim_instances[instance]->bsr,ifname);
        if ( result == FALSE )
            goto exit;
        crp = NETCFG_OM_PIM_RpCandidateLookup(&vr_entry->pim_instances[instance]->bsr, ifname);
        if ( crp == NULL )
            goto exit;
        /* This flag will be used in rp candidate group funcitons */
        SET_FLAG(crp->config, NETCFG_OM_PIM_CRP_CONFIG_DEFAULT_RANGE);
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}

/* FUNCTION NAME : NETCFG_OM_PIM_RemoveRpCandidate
 * PURPOSE:
 *      Remove a RP candidate.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      .
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_UnSetPimRpCandidate( UI32_T vr_id,
                                                           UI32_T instance,
                                                           UI8_T* ifname )
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        crp = NETCFG_OM_PIM_RpCandidateLookup(&vr_entry->pim_instances[instance]->bsr, ifname);
        if ( crp == NULL )
        {
            result = TRUE;
            goto exit;
        }
        /* This flag will be used in rp candidate group funcitons */
        UNSET_FLAG(crp->config, NETCFG_OM_PIM_CRP_CONFIG_DEFAULT_RANGE);
        if ( FLAG_ISSET(crp->config, NETCFG_OM_PIM_CRP_CONFIG_GROUP) )
        {
            result = TRUE;
            goto exit;
        }
        /* remove this rp */
        result = NETCFG_OM_PIM_RpCandidateRemove(&vr_entry->pim_instances[instance]->bsr, ifname);
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}
/* FUNCTION NAME : NETCFG_OM_PIM_SetPimRpCandidateGroupAddr
 * PURPOSE:
 *      Set a RP candidate group address
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      .
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */

BOOL_T NETCFG_OM_PIM_SetPimRpCandidateGroupAddr(UI32_T vr_id,
                                                                   UI32_T instance,
                                                                   UI8_T *ifname,
                                                                   UI32_T groupAddr, 
                                                                   UI32_T maskAddr)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    struct netcfg_om_pim_bsr *bsr;
    struct netcfg_om_pim_prefix p;
    struct route_node *rn;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        bsr = &vr_entry->pim_instances[instance]->bsr;
        /* find this crp */
        crp = NETCFG_OM_PIM_RpCandidateLookup(bsr,ifname);   
        if ( NULL == crp )
        {
            /* create new crp */
            result = NETCFG_OM_PIM_RpCandidateInsert(bsr, ifname);
            if ( result == FALSE )
                goto exit;
            /* find again */
            crp = NETCFG_OM_PIM_RpCandidateLookup(bsr,ifname);
            if ( NULL == crp )
            {
                result == FALSE;
                goto exit;
            }
        }
        p.family = AF_LOCAL;
        p.group = (groupAddr & maskAddr);
        p.src = maskAddr;
        p.prefixlen = IPV4_MAX_BITLEN * 2;
        /* search this route node */
        rn = route_node_lookup (crp->grp_range, (struct prefix*)&p);
        if (rn)
        {
            route_unlock_node (rn);
            result = TRUE;
            goto exit;
        }
        /* create this node */
        rn = route_node_get (crp->grp_range, (struct prefix*)&p);
        if (rn == NULL)
        {
            result = FALSE;
            goto exit;
        }
        rn->info = crp;
        ++crp->num_grp_range;
        SET_FLAG(crp->config, NETCFG_OM_PIM_CRP_CONFIG_GROUP);
        result = TRUE;
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}
/* FUNCTION NAME : NETCFG_OM_PIM_UnSetPimRpCandidateGroupAddr
 * PURPOSE:
 *      Unset a RP candidate group address
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      .
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */

BOOL_T NETCFG_OM_PIM_UnSetPimRpCandidateGroupAddr(UI32_T vr_id,
                                                                   UI32_T instance,
                                                                   UI8_T *ifname,
                                                                   UI32_T groupAddr, 
                                                                   UI32_T maskAddr)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_crp *crp = NULL;
    struct netcfg_om_pim_bsr *bsr;
    struct netcfg_om_pim_prefix p;
    struct route_node *rn;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        bsr = &vr_entry->pim_instances[instance]->bsr;
        /* find this crp */
        crp = NETCFG_OM_PIM_RpCandidateLookup(bsr,ifname);   
        if ( NULL == crp || crp->grp_range == NULL )
        {
            result = TRUE;
            goto exit;
        }
        p.family = AF_LOCAL;
        p.group = (groupAddr & maskAddr);
        p.src = maskAddr;
        p.prefixlen = IPV4_MAX_BITLEN * 2;
        /* search this route node */
        rn = route_node_lookup (crp->grp_range, (struct prefix*)&p);
        if (!rn)
        {
            result = TRUE;
            goto exit;
        }
        /* delete this node */
        route_unlock_node(rn);
        if ( --crp->num_grp_range == 0 )
        {
            /* unset this flag. It means we have no any group on this crp */
            UNSET_FLAG(crp->config, NETCFG_OM_PIM_CRP_CONFIG_GROUP);
            /* NETCFG_OM_PIM_InsertNewRpCandidate will set this flag.
                        NETCFG_OM_PIM_RemoveRpCandidate will unset this flag if this crp still have group address
                        So, if we have no any group address, we'll delete this crp really. */
            if( !FLAG_ISSET(crp->config, NETCFG_OM_PIM_CRP_CONFIG_DEFAULT_RANGE) )
                result = NETCFG_OM_PIM_RpCandidateRemove(bsr,ifname);
        }
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_PIM_SetPimRpAddress
 * PURPOSE:
 *      set new RP address.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      address
 *
 * OUTPUT:
 *      .
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_SetPimRpAddress( UI32_T vr_id,
                                                     UI32_T instance,
                                                     UI32_T address )
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_rp_conf *rp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        
        rp = NETCFG_OM_PIM_RpAddressLookup(vr_entry->pim_instances[instance], address);
        if ( rp )
        {
            result = TRUE;
            goto exit;
        }
        result = NETCFG_OM_PIM_RpAddressInsert(vr_entry->pim_instances[instance],address);
        if ( result == FALSE )
            goto exit;
        rp = NETCFG_OM_PIM_RpAddressLookup(vr_entry->pim_instances[instance], address);
        if ( rp == NULL )
        {
            result = FALSE;
            goto exit;
        }
        result = TRUE;
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}

/* FUNCTION NAME : NETCFG_OM_PIM_UnSetPimRpAddress
 * PURPOSE:
 *      Remove a RP address.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      ifname
 *
 * OUTPUT:
 *      .
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_UnSetPimRpAddress( UI32_T vr_id,
                                                        UI32_T instance,
                                                        UI32_T address )
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_rp_conf *rp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        rp = NETCFG_OM_PIM_RpAddressLookup(vr_entry->pim_instances[instance], address);
        if ( rp == NULL )
        {
            result = TRUE;
            goto exit;
        }
        /* remove this rp */
        result = NETCFG_OM_PIM_RpAddressRemove(vr_entry->pim_instances[instance], address);
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}
/* FUNCTION NAME : NETCFG_OM_PIM_GetRpAddress
 * PURPOSE:
 *      Get the RP address.
 *
 * INPUT:
 *      vr_id
 *      instance,
 *      address
 *
 * OUTPUT:
 *      RP.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_PIM_GetRpAddress(UI32_T vr_id,
                                                UI32_T instance,
                                                UI32_T address)
{
    BOOL_T result = FALSE;
    NETCFG_OM_PIM_Master_T *vr_entry = NULL;
    struct netcfg_om_pim_rp_conf *rp = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pim_om_sem_id);       
    if(NETCFG_OM_PIM_GetMasterEntry(vr_id, &vr_entry) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
        return FALSE;
    }
    if(vr_entry->pim_instances[instance] != NULL)
    {
        rp = NETCFG_OM_PIM_RpAddressLookup(vr_entry->pim_instances[instance], address);
        if ( rp == NULL )
            goto exit;
        result = TRUE;
    }
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(pim_om_sem_id, original_priority);
    return  result;
    
}

