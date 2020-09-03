/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_OM.c
 *
 * PURPOSE: This package serves as a database to store vrrp table by HISAM utility
 * NOTES:   API List # =
 *
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 * -------------------------------------------------------------------------------------*/



/* INCLUDE FILE DECLARATIONS
 */
/*#include "l_hisam.h"*/
#include "l_sort_lst.h"
#include "vrrp_om.h"
#include "string.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "vlan_mgr.h"
#include "vlan_lib.h"
#include "vlan_pom.h"
#include "vrrp_sys_adpt.h"


/*
 * NAMING CONSTANT
 */

/* LOCAL TYPE DECLARATION
 */
typedef struct
{
    UI32_T  if_index;    /* key 1*/
    UI32_T  vrid;        /* key 2*/
    VRRP_OPER_ENTRY_T vrrp_oper_entry; /* element */
}   SORTLST_ELM_T;

typedef struct
{
    UI8_T       assoc_ip_addr[VRRP_IP_ADDR_LENGTH]; /* key 1 */
    UI32_T      row_status;
}   ASSOCIPLST_ELM_T;

/* STATIC VARIABLE DECLARATIONS
 */
static VRRP_OM_Router_Statistics_Info_T       vrrp_router_static_info;
static VRRP_TYPE_GlobalEntry_T				  vrrp_global_config;
/* This list contains created vrrp in ascending order */
static L_SORT_LST_List_T                      sort_vrrp_list;

static UI32_T   vrrp_om_current_cfg_vrrp = 0; /* Total number of vrrp configured*/
static UI32_T   last_update_time;
static UI32_T   vrrp_om_sem_id;
static UI32_T	orig_priority;
static void   *vrrp_group_timer_id_p;
/*
 * MACRO DEFINITION
 */
#define VRRP_OM_LOCK()       orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(vrrp_om_sem_id)
#define VRRP_OM_UNLOCK()     SYSFUN_OM_LEAVE_CRITICAL_SECTION(vrrp_om_sem_id, orig_priority)


/* ----------------------------
   LOCAL SUBPROGRAM DECLARATION
 * ----------------------------*/
static void VRRP_OM_ClearAllVrrpTable(void);
static void VRRP_OM_ClearAllVrrpStatisticsTable(void);
static int VRRP_OM_Compare(void *elm1, void *elm2);
static int VRRP_OM_Assoc_Ip_Compare(void *elm1, void *elm2);



/*---------------------------
 * EXPORTED SUBPROGRAM BODIES
 *---------------------------*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_Init
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function initialize vrrp_om.  This will allocate memory for
 *            vrrp table and create link list to maintain vrrp table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------*/
void VRRP_OM_Init(void)
{
    L_SORT_LST_Create(&sort_vrrp_list, SYS_ADPT_MAX_VRRP_ID, sizeof(SORTLST_ELM_T), VRRP_OM_Compare);

    memset(&vrrp_router_static_info, 0, sizeof(VRRP_OM_Router_Statistics_Info_T));
    vrrp_om_current_cfg_vrrp = 0;

    memset(&vrrp_global_config, 0, sizeof(vrrp_global_config));

    vrrp_group_timer_id_p = 0;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_VRRP, &vrrp_om_sem_id) != SYSFUN_OK)
    {
        printf("VRRP_OM_Init : Can't create semaphore\n");
    }
    return;
} /* VRRP_OM_Init() */


/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearDatabase
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function clears all entry in VRRP table and VRRP associated ip table
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function shall be invoked when system enters transition mode.
 *            2. All the entries in database will be purged.
 *-----------------------------------------------------------------------------------*/
void VRRP_OM_ClearDatabase(void)
{
    SORTLST_ELM_T   list_elm;

    VRRP_OM_ClearAllVrrpTable();
    VRRP_OM_ClearAllVrrpStatisticsTable();

    memset(&list_elm, 0, sizeof(list_elm));
    VRRP_OM_LOCK();
    while (L_SORT_LST_Get_Next(&sort_vrrp_list, &list_elm))
    {
        L_SORT_LST_Delete_All(&(list_elm.vrrp_oper_entry.assoc_ip_list));
    }

    L_SORT_LST_Delete_All(&sort_vrrp_list);
    VRRP_OM_UNLOCK();
    return;
} /* VRRP_OM_ClearDatabase */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetTimerId
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Set VRRP_GROUP periodic timer id
 * INPUT    : timer_id      --  timer id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
void VRRP_OM_SetTimerId(void *timer_id_p)
{
    VRRP_OM_LOCK();
    vrrp_group_timer_id_p = timer_id_p;
    VRRP_OM_UNLOCK();
    return;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetTimerId
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP_GROUP periodic timer id
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : timer_id
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
void *VRRP_OM_GetTimerId(void)
{
    void *timer_id_p;
    VRRP_OM_LOCK();
    timer_id_p = vrrp_group_timer_id_p;
    VRRP_OM_UNLOCK();
    return timer_id_p;
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes the specific virtual router on some interface
 * INPUT    : vrrp_info.ifindex -- specify which ifindex the virtual router
 *            residents.
 *            vrrp_info.vrid -- specify which vrid to be deleted
 * OUTPUT   : TRUE if vrrp_info entry has been deleted. False, otherwise.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info)
{
    SORTLST_ELM_T  list_elm;

    VRRP_OM_LOCK();

    if (vrrp_info == 0)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }

    list_elm.if_index = vrrp_info->ifindex;
    list_elm.vrid = vrrp_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }
    else
    {
        /* Free memory allocated by assoc_ip_list*/
        L_SORT_LST_Delete_All(&(list_elm.vrrp_oper_entry.assoc_ip_list));

        /* Clear the specific vrrp_oper_entry structure */
        memset(&list_elm.vrrp_oper_entry, 0, sizeof(VRRP_OPER_ENTRY_T));

        /* Free memory allocated by the specific vrrp_oper_entry element of sort_vrrp_list */
        if (L_SORT_LST_Delete(&sort_vrrp_list, &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return FALSE;
        }
        vrrp_om_current_cfg_vrrp--;
        last_update_time = SYSFUN_GetSysTick();
        VRRP_OM_UNLOCK();
        return TRUE;
    }
} /* VRRP_OM_DeleteVrrpOperEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetFirstVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if virtual router on some interface
              is available.  Otherwise, return false.
 * INPUT    : vrrp_info->ifindex  -- specify which vrrp ifindex to be retrieved
 *            vrrp_info->vrid  -- specify which vrrp ifindex to be retrieved
 * OUTPUT   : returns the specific vrrp info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetFirstVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info)
{
    SORTLST_ELM_T           list_elm;

    VRRP_OM_LOCK();

    if (vrrp_info == 0)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }


    if (L_SORT_LST_Get_1st(&sort_vrrp_list, &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }
    else
    {
        memcpy(vrrp_info,
               &list_elm.vrrp_oper_entry,
               sizeof(VRRP_OPER_ENTRY_T));
        VRRP_OM_UNLOCK();
        return TRUE;
    }
} /* VRRP_OM_GetFirstVrrpOperEntry */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if virtual router on some interface
              is available.  Otherwise, return false.
 * INPUT    : vrrp_info->ifindex  -- specify which vrrp ifindex to be retrieved
 *            vrrp_info->vrid  -- specify which vrrp ifindex to be retrieved
 * OUTPUT   : returns the specific vrrp info
 * RETURN   : VRRP_TYPE_OK/VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info)
{
    SORTLST_ELM_T          list_elm;

    VRRP_OM_LOCK();

    list_elm.if_index = vrrp_info->ifindex;
    list_elm.vrid = vrrp_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
    }
    else
    {
        memcpy(vrrp_info,
               &list_elm.vrrp_oper_entry,
               sizeof(VRRP_OPER_ENTRY_T));
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_OK;
    }
} /* VRRP_OM_GetVrrpOperEntry */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_VrrpOperEntryCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Check is any VRRP entry exist
 * INPUT    : None
 * OUTPUT   :
 * RETURN   : TRUE - there is VRRP entry
 *            FALSE- there is no VRRP entry
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_VrrpOperEntryCount()
{
  return sort_vrrp_list.nbr_of_element;
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vrrp entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vrrp_info->ifindex  -- specify which interface information to be retrieved.
 *            vrrp_info->vrid  -- specify which vrrp to be retrieved.
 * OUTPUT   : return next available vrrp ifindex info
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_ACCESS_SUCCESS
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info)
{
    SORTLST_ELM_T   list_elm;

    VRRP_OM_LOCK();

    if (vrrp_info == 0)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    list_elm.if_index = vrrp_info->ifindex;
    list_elm.vrid = vrrp_info->vrid;
    if (L_SORT_LST_Get_Next(&sort_vrrp_list, &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
    }
    else
    {
        vrrp_info->ifindex = list_elm.if_index;
        vrrp_info->vrid = list_elm.vrid;
        memcpy(vrrp_info, &list_elm.vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ACCESS_SUCCESS;
    }
} /* VRRP_OM_GetNextVrrpOperEntry */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function modifies(creates) the specific vrrp entry.
 * INPUT    : vrrp_info->ifindex  -- which interface vrrp residents on
 *            vrrp_info->vrid  -- which vrrp to be modified.
 * OUTPUT   : vrrp info has been updated.
 * RETURN   : VRRP_TYPE_INTERNAL_ERROR/
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VRRP_MGR shall use this function to create a new VRRP ifindex entry.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_info)
{
    SORTLST_ELM_T   list_elm;

    VRRP_OM_LOCK();

    if (vrrp_info == NULL)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }
    {
        //VRID has to be global unique
        SORTLST_ELM_T   list_elm;
        memset(&list_elm,0,sizeof(list_elm));
        while (L_SORT_LST_Get_Next(&sort_vrrp_list, &list_elm) == TRUE)
        {
           if(list_elm.vrid == vrrp_info->vrid &&
                   list_elm.if_index!=vrrp_info->ifindex)
           {
               VRRP_OM_UNLOCK();
               return VRRP_TYPE_OPER_ENTRY_EXIST;
           }
        }
    }
    memset(&list_elm,0,sizeof(list_elm));
    list_elm.if_index = vrrp_info->ifindex;
    list_elm.vrid = vrrp_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &list_elm) == FALSE) /* If this entry does not exist */
    {
        if (vrrp_om_current_cfg_vrrp == SYS_ADPT_MAX_NBR_OF_VRRP_GROUP)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP;
        }

        memcpy(&list_elm.vrrp_oper_entry, vrrp_info, sizeof(VRRP_OPER_ENTRY_T));

        /* Initialize the assoc_ip_list for this vrrp_oper_entry
         */
        if (L_SORT_LST_Create(&list_elm.vrrp_oper_entry.assoc_ip_list,
                              SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP,
                              sizeof(ASSOCIPLST_ELM_T),
                              VRRP_OM_Assoc_Ip_Compare) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        if (L_SORT_LST_Set(&sort_vrrp_list, &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        /* copy created associated ip list to input oper entry
         */
        vrrp_info->assoc_ip_list = list_elm.vrrp_oper_entry.assoc_ip_list;
        vrrp_om_current_cfg_vrrp++;
    }
    else
    {
        memcpy(&list_elm.vrrp_oper_entry, vrrp_info, sizeof(VRRP_OPER_ENTRY_T));
        if (L_SORT_LST_Set(&sort_vrrp_list, &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    last_update_time = SYSFUN_GetSysTick();
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
} /* VRRP_OM_SetVrrpOperEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes the associated ip address on the specific
 *            vrrp and interface. The "delete" action just "disables" the
 *            specific entry, not really removes it.
 * INPUT    : associated_info.ifindex -- which ifindex vrrd residents on.
 *            associated_info.vrid -- which vrid associated ip address with
 *            associated_info.ip_addr -- which associated ip address
 * OUTPUT   : TRUE if associated_info entry has been deleted. False, otherwise.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    ASSOCIPLST_ELM_T list_elm;
    SORTLST_ELM_T    vrrp_list_elm;

    VRRP_OM_LOCK();

    if (assoc_ip_info == 0)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }

    vrrp_list_elm.if_index = assoc_ip_info->ifindex;
    vrrp_list_elm.vrid = assoc_ip_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &vrrp_list_elm) == FALSE) /* If this entry does not exist */
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }
    else
    {
        memcpy(list_elm.assoc_ip_addr, assoc_ip_info->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        if (L_SORT_LST_Get(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                           &list_elm) == FALSE)
        {
            /* No such associated Ip address exists
             */
            VRRP_OM_UNLOCK();
            return FALSE;
        }

        if ((list_elm.row_status != VAL_vrrpAssoIpAddrRowStatus_active) &&
                (list_elm.row_status != VAL_vrrpAssoIpAddrRowStatus_createAndGo))
        {
            /* found the associated Ip address but unavailable
             */
            VRRP_OM_UNLOCK();
            return FALSE;
        }

        /* Found the availabe specific associated Ip address and set row status to disable it
         */
        list_elm.row_status = VAL_vrrpAssoIpAddrRowStatus_notInService;
        if (L_SORT_LST_Set(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                           &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return FALSE;
        }

        /* Do not need to free memory allocated by list_elm in the assoc_ip_list because
            the specific associated IP address entry still exists
          */
    }
    last_update_time = SYSFUN_GetSysTick();
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_DeleteAssoIpEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpAssoIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the availabe associated IP address of the
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    ASSOCIPLST_ELM_T  list_elm;
    SORTLST_ELM_T     vrrp_list_elm;

    VRRP_OM_LOCK();

    if (assoc_ip_info == NULL)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    vrrp_list_elm.if_index = assoc_ip_info->ifindex;
    vrrp_list_elm.vrid = assoc_ip_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &vrrp_list_elm) == FALSE) /* If this entry does not exist */
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
    }
    memcpy(list_elm.assoc_ip_addr, assoc_ip_info->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
    if (L_SORT_LST_Get(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                       &list_elm) == FALSE)
    { /* No such available associated Ip address exists */
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
    }

    assoc_ip_info->row_status = list_elm.row_status;
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
} /* VRRP_OM_GetVrrpAssoIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpAssoIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the next availabe associated IP address of the
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpAssoIpAddress(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    ASSOCIPLST_ELM_T   list_elm;
    SORTLST_ELM_T      vrrp_list_elm;
    BOOL_T             found = FALSE;

    VRRP_OM_LOCK();

    if (assoc_ip_info == NULL)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if ((assoc_ip_info->ifindex == 0) && (assoc_ip_info->vrid == 0))
    {
        if (L_SORT_LST_Get_1st(&sort_vrrp_list, &vrrp_list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
        }

        if (L_SORT_LST_Get_1st(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list, &list_elm) != TRUE)
        {
            while (L_SORT_LST_Get_Next(&sort_vrrp_list, &vrrp_list_elm) == TRUE)
            {
                if (L_SORT_LST_Get_1st(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list, &list_elm) == TRUE)
                {
                    assoc_ip_info->ifindex = vrrp_list_elm.vrrp_oper_entry.ifindex;
                    assoc_ip_info->vrid = vrrp_list_elm.vrrp_oper_entry.vrid;
                    found = TRUE;
                    break;
                }
            }
        }
        else
        {
            assoc_ip_info->ifindex = vrrp_list_elm.vrrp_oper_entry.ifindex;
            assoc_ip_info->vrid = vrrp_list_elm.vrrp_oper_entry.vrid;
            found = TRUE;
        }
    }
    else
    {
        vrrp_list_elm.if_index = assoc_ip_info->ifindex;
        vrrp_list_elm.vrid = assoc_ip_info->vrid;
        if (L_SORT_LST_Get(&sort_vrrp_list, &vrrp_list_elm) == FALSE) /* If this entry does not exist */
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
        }

        memcpy(list_elm.assoc_ip_addr, assoc_ip_info->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);

        if (L_SORT_LST_Get_Next(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list, &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
        }
        assoc_ip_info->ifindex = vrrp_list_elm.vrrp_oper_entry.ifindex;
        assoc_ip_info->vrid = vrrp_list_elm.vrrp_oper_entry.vrid;
        found = TRUE;
    }

    if (found == TRUE)
    {

        memcpy(assoc_ip_info->assoc_ip_addr, list_elm.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        assoc_ip_info->row_status = list_elm.row_status;
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_OK;

    }
    else
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
    }
} /* VRRP_OM_GetNextVrrpAssoIpAddress() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpAssoIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if associated ip address on some
 *            interface has been modified successfully. otherwise, false is return.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VRRP_MGR shall use this function to create a new VRRP ifindex entry.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SetVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    ASSOCIPLST_ELM_T  list_elm;
    SORTLST_ELM_T     vrrp_list_elm;

    VRRP_OM_LOCK();

    if (assoc_ip_info == NULL)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    vrrp_list_elm.if_index = assoc_ip_info->ifindex;
    vrrp_list_elm.vrid = assoc_ip_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &vrrp_list_elm) == FALSE) /* If this entry does not exist */
    {
        if (assoc_ip_info->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_PARAMETER_ERROR;
        }
        else
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_PARAMETER_ERROR;
        }
    }
    memcpy(list_elm.assoc_ip_addr, assoc_ip_info->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
    list_elm.row_status = assoc_ip_info->row_status;
    if ((assoc_ip_info->row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
            (assoc_ip_info->row_status == VAL_vrrpAssoIpAddrRowStatus_active) ||
            (assoc_ip_info->row_status == VAL_vrrpAssoIpAddrRowStatus_notReady))
    {
        if (L_SORT_LST_Set(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                           &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    else if (assoc_ip_info->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService)
    {
        if (L_SORT_LST_Delete(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                              &list_elm) == FALSE)
        {
            VRRP_OM_UNLOCK();
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    if (L_SORT_LST_Set(&sort_vrrp_list, &vrrp_list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    last_update_time = SYSFUN_GetSysTick();
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
} /* VRRP_OM_SetVrrpAssoIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SearchVrrpAssoIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function searches the specific associated ip address.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : 1. If the specified entry does not exist, return FALSE.
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_SearchVrrpAssoIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    ASSOCIPLST_ELM_T  list_elm;
    SORTLST_ELM_T     vrrp_list_elm;

    VRRP_OM_LOCK();

    if (assoc_ip_info == NULL)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    vrrp_list_elm.if_index = assoc_ip_info->ifindex;
    vrrp_list_elm.vrid = assoc_ip_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &vrrp_list_elm) == FALSE) /* If this entry does not exist */
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
    }
    memcpy(list_elm.assoc_ip_addr, assoc_ip_info->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
    if (L_SORT_LST_Get(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list,
                       &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
    }
    if ((list_elm.row_status != VAL_vrrpAssoIpAddrRowStatus_active) &&
            (list_elm.row_status != VAL_vrrpAssoIpAddrRowStatus_createAndGo))
    {
        VRRP_OM_UNLOCK();
        return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
    }
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
} /* VRRP_OM_SearchVrrpAssoIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpAssoIpAddrEntryByIpaddr
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get specified vrrp associated IP entry by ifindex and ip address
 * INPUT    : associated_info.ip_addr -- specify which ip address to search
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : None
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpAssoIpAddrEntryByIpaddr(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    UI32_T ifindex;
    UI8_T ipaddr[SYS_ADPT_IPV4_ADDR_LEN];
    SORTLST_ELM_T vrrp_list_elm;
    ASSOCIPLST_ELM_T list_elm;
    UI32_T ret = VRRP_TYPE_INTERNAL_ERROR;

    if(assoc_ip_info == NULL)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_OM_LOCK();

    memset(&vrrp_list_elm, 0, sizeof(vrrp_list_elm));
    while(L_SORT_LST_Get_Next(&sort_vrrp_list, &vrrp_list_elm))
    {
        if(L_SORT_LST_Get_1st(&vrrp_list_elm.vrrp_oper_entry.assoc_ip_list, &list_elm) &&
            !memcmp(assoc_ip_info->assoc_ip_addr, list_elm.assoc_ip_addr, SYS_ADPT_IPV4_ADDR_LEN))
        {
            assoc_ip_info->vrid = vrrp_list_elm.vrrp_oper_entry.vrid;
            assoc_ip_info->ifindex = vrrp_list_elm.if_index;
            assoc_ip_info->row_status = list_elm.row_status;
            ret = VRRP_TYPE_OK;
            goto exit;
        }
    }

exit:
    VRRP_OM_UNLOCK();
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_VridExistOnIf
 *--------------------------------------------------------------------------
 * PURPOSE  : This function examines whether the specific vrid exists on the
 *            specific ifindex.
 * INPUT    : ifindex
 *            vrid
 * OUTPUT   : current_cfg_vrrp
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_VridExistOnIf(UI32_T ifindex, UI32_T vrid)
{
    SORTLST_ELM_T           list_elm;
    UI32_T                  result;

    VRRP_OM_LOCK();

    list_elm.if_index = ifindex;
    list_elm.vrid = vrid;
    result = L_SORT_LST_Get(&sort_vrrp_list, &list_elm);
    VRRP_OM_UNLOCK();
    return result;
} /* VRRP_OM_VridExistOnIf() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetCurrentNumbOfVRRPConfigured
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of VRRPs currently
 *            configured in the interface is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vrrp
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetCurrentNumOfVRRPConfigured(UI32_T *current_cfg_vrrp)
{
    if (current_cfg_vrrp == 0)
        return FALSE;

    VRRP_OM_LOCK();
    *current_cfg_vrrp = vrrp_om_current_cfg_vrrp;
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_GetCurrentNumOfVRRPConfigured */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetOperStat
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrieves the system operation info
 * INPUT    : none
 * OUTPUT   : The vrrp_oper_info opeation stat
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetOperStat(VRRP_OPER_ENTRY_T *vrrp_oper_info)
{
    SORTLST_ELM_T   list_elm;

    VRRP_OM_LOCK();

    if (vrrp_oper_info == 0)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }

    list_elm.if_index = vrrp_oper_info->ifindex;
    list_elm.vrid = vrrp_oper_info->vrid;
    if (L_SORT_LST_Get(&sort_vrrp_list, &list_elm) == FALSE)
    {
        VRRP_OM_UNLOCK();
        return FALSE;
    }
    vrrp_oper_info->oper_state = list_elm.vrrp_oper_entry.oper_state;
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_GetOperStat() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears the system statistics
 * INPUT    : none
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_ClearVrrpSysStatistics(void)
{
    VRRP_OM_LOCK();
    memset(&vrrp_router_static_info, 0, sizeof(VRRP_OM_Router_Statistics_Info_T));
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_ClearVrrpSysStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears the system statistics
 * INPUT    : none
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_SetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T router_statis_info)
{
    VRRP_OM_LOCK();
    memcpy(&vrrp_router_static_info, &router_statis_info, sizeof(VRRP_OM_Router_Statistics_Info_T));
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_SetVrrpSysStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpSysStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the system statistics
 * INPUT    : none
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *router_statis_info)
{
    if (router_statis_info == 0)
        return FALSE;

    VRRP_OM_LOCK();
    memcpy(router_statis_info, &vrrp_router_static_info, sizeof(VRRP_OM_Router_Statistics_Info_T));
    VRRP_OM_UNLOCK();
    return TRUE;
} /* VRRP_OM_GetVrrpSysStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears the vrrp group statistics
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return FALSE;
    }
    memset(&vrrp_oper_entry.vrrp_statistic, 0, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
        return FALSE;
    return TRUE;
} /* VRRP_OM_ClearVrrpGroupStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the vrrp group statistics
 * INPUT    : ifindex, vrid, statistics info to be set
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_OM_SetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
        return FALSE;
    memcpy(&vrrp_oper_entry.vrrp_statistic, vrrp_statis_info, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
        return FALSE;
    return TRUE;
} /* VRRP_OM_SetVrrpGroupStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the vrrp group statistics
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : statistics info of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T	result = 0;

    if (vrrp_statis_info == 0)
        return VRRP_TYPE_PARAMETER_ERROR;

    if ((vrid < SYS_ADPT_MIN_VRRP_ID) || (vrid > SYS_ADPT_MAX_VRRP_ID))
        return VRRP_TYPE_PARAMETER_ERROR;

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
        return result;

    memcpy(vrrp_statis_info, &vrrp_oper_entry.vrrp_statistic, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    return VRRP_TYPE_OK;
} /* VRRP_OM_GetVrrpGroupStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetNextVrrpGroupStatistics
 *--------------------------------------------------------------------------
 * PURPOSE  : This function retrives the next vrrp group statistics
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : statistics info of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_OM_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_statis_info)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T	result = 0;

    if ((vrrp_statis_info == NULL) || (if_index == NULL) || (vrid == NULL))
        return VRRP_TYPE_PARAMETER_ERROR;

    if ((*if_index != 0) || (*vrid != 0))
    {
        if ((*vrid < SYS_ADPT_MIN_VRRP_ID) || (*vrid > SYS_ADPT_MAX_VRRP_ID))
            return VRRP_TYPE_PARAMETER_ERROR;
    }

    vrrp_oper_entry.ifindex = *if_index;
    vrrp_oper_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_ACCESS_SUCCESS)
        return result;
    *if_index = vrrp_oper_entry.ifindex;
    *vrid = vrrp_oper_entry.vrid;
    memcpy(vrrp_statis_info, &vrrp_oper_entry.vrrp_statistic, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    return VRRP_TYPE_OK;
} /* VRRP_OM_GetNextVrrpGroupStatistics() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_SetVrrpGlobalConfig
 *--------------------------------------------------------------------------
 * PURPOSE  : This function set VRRP global configuration
 * INPUT    : config	--	global configuration structure
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_SetVrrpGlobalConfig(VRRP_TYPE_GlobalEntry_T *config)
{
    if (NULL == config)
        return VRRP_TYPE_PARAMETER_ERROR;

    VRRP_OM_LOCK();
    memcpy(&vrrp_global_config, config, sizeof(vrrp_global_config));
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetVrrpGlobalConfig
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get VRRP global configuration
 * INPUT    : config	--	global configuration structure
 * OUTPUT   : config	--	global configuration structure
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_GetVrrpGlobalConfig(VRRP_TYPE_GlobalEntry_T *config)
{
    if (NULL == config)
        return VRRP_TYPE_PARAMETER_ERROR;

    VRRP_OM_LOCK();
    memcpy(config, &vrrp_global_config, sizeof(VRRP_TYPE_GlobalEntry_T));
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_IncreaseRouterStatisticCounter
 *--------------------------------------------------------------------------
 * PURPOSE  : This function increase router statistic counter
 * INPUT    : flag	--	counter flag
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_IncreaseRouterStatisticCounter(VRRP_TYPE_ROUTER_STATISTICS_FLAG_T flag)
{
    VRRP_OM_LOCK();

    switch(flag)
    {
        case VRRP_ROUTER_CHECKSUM_ERROR:
            vrrp_router_static_info.vrrpRouterChecksumErrors++;
            break;
        case VRRP_ROUTER_VERSION_ERROR:
            vrrp_router_static_info.vrrpRouterVersionErrors++;
            break;
        case VRRP_ROUTER_VRID_ERROR:
            vrrp_router_static_info.vrrpRouterVrIdErrors++;
            break;
    }

    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
}

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_GetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_OM_GetPingStatus(UI32_T *ping_status)
{
    if(NULL == ping_status)
        return VRRP_TYPE_PARAMETER_ERROR;

    VRRP_OM_LOCK();
    *ping_status = vrrp_global_config.vrrpPingStatus;
    VRRP_OM_UNLOCK();
    return VRRP_TYPE_OK;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for VRRP om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VRRP_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    if (ipcmsg_p == NULL)
        return FALSE;

    switch (VRRP_OM_MSG_CMD(ipcmsg_p))
    {
        case VRRP_OM_IPC_GET_OPER_ENTRY:
        {
            VRRP_OM_IPCMsg_OperEntry_T *data_p = VRRP_OM_MSG_DATA(ipcmsg_p);
            VRRP_OM_MSG_RETVAL(ipcmsg_p) = VRRP_OM_GetVrrpOperEntry(&(data_p->entry));
            ipcmsg_p->msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_OperEntry_T);
        }
        break;

        case VRRP_OM_IPC_GET_NEXT_ASSOC_IP_ADDRESS:
        {
            VRRP_OM_IPCMsg_AssocIpEntry_T *data_p = VRRP_OM_MSG_DATA(ipcmsg_p);
            VRRP_OM_MSG_RETVAL(ipcmsg_p) = VRRP_OM_GetNextVrrpAssoIpAddress(&(data_p->entry));
            ipcmsg_p->msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_AssocIpEntry_T);
        }
        break;

        case VRRP_OM_IPC_GET_ASSOC_IP_ADDRESS_BY_IP:
        {
            VRRP_OM_IPCMsg_AssocIpEntry_T *data_p = VRRP_OM_MSG_DATA(ipcmsg_p);
            VRRP_OM_MSG_RETVAL(ipcmsg_p) = VRRP_OM_GetVrrpAssoIpAddrEntryByIpaddr(&(data_p->entry));
            ipcmsg_p->msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_AssocIpEntry_T);
        }
        break;
#if (SYS_CPNT_VRRP_PING == TRUE)
        case VRRP_OM_IPC_GET_PING_STATUS:
        {
            VRRP_OM_IPCMsg_UI32_T *data_p = VRRP_OM_MSG_DATA(ipcmsg_p);
            VRRP_OM_MSG_RETVAL(ipcmsg_p) = VRRP_OM_GetPingStatus(&(data_p->value));
            ipcmsg_p->msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_UI32_T);
        }
        break;
#endif

        default:
            printf("no message %lu\r\n", (unsigned long)VRRP_OM_MSG_CMD(ipcmsg_p));
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\r\n", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    return TRUE;
} /* VRRP_OM_HandleIPCReqMsg */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearAllVrrpTable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears all vrrp table
 * INPUT    : none
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void VRRP_OM_ClearAllVrrpTable(void)
{
    VRRP_OM_LOCK();
    memset(&vrrp_global_config, 0, sizeof(vrrp_global_config));
    VRRP_OM_UNLOCK();
    return;
} /* VRRP_OM_ClearAllVrrpTable */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_ClearAllVrrpStatisticsTable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears all vrrp table
 * INPUT    : none
 * OUTPUT   :
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void VRRP_OM_ClearAllVrrpStatisticsTable(void)
{
    VRRP_OM_LOCK();
    memset(&vrrp_router_static_info, 0, sizeof(VRRP_OM_Router_Statistics_Info_T));
    VRRP_OM_UNLOCK();
    return;
} /* VRRP_OM_ClearAllVrrpStatisticsTable */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_Compare
 *--------------------------------------------------------------------------
 * PURPOSE  : Compare function for sort list
 * INPUT    : elm1  -- element 1 with specific ifindx and vrid
              elm2  -- element 2 with specific ifindx and vrid
 * OUTPUT   : none
 * RETURN   : TRUE  -- the value for vid1 is greater then vid2
 *	          FALSE -- otherwise
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static int VRRP_OM_Compare(void *elm1, void *elm2)
{
    SORTLST_ELM_T    *element1, *element2;

    element1 = (SORTLST_ELM_T *)elm1;
    element2 = (SORTLST_ELM_T *)elm2;

    if ((element1->if_index == element2->if_index) && (element1->vrid == element2->vrid))
        return 0;  /* The two elements have the same key */
    if ((element1->if_index < element2->if_index) ||
            ((element1->if_index == element2->if_index) && (element1->vrid < element2->vrid)))
        return -1; /* The first element is smaller thatn the second element */
    if ((element1->if_index > element2->if_index) ||
            ((element1->if_index == element2->if_index) && (element1->vrid > element2->vrid)))
        return 1;  /* The first element is larger thatn the second element */
    return 0;
} /* end of VRRP_OM_Compare()*/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_OM_Assoc_Ip_Compare
 *--------------------------------------------------------------------------
 * PURPOSE  : Compare function for associated ip address list
 * INPUT    : elm1  -- element 1 with specific ifindx and vrid
              elm2  -- element 2 with specific ifindx and vrid
 * OUTPUT   : none
 * RETURN   : 0  -- the key of elm1 is the same as elm2
 *            >0 -- the key of elm1 is larger than elm2
 *	          <0 -- the key of elm1 is smaller than elm2
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static int VRRP_OM_Assoc_Ip_Compare(void *elm1, void *elm2)
{
    ASSOCIPLST_ELM_T    *element1, *element2;

    element1 = (ASSOCIPLST_ELM_T *)elm1;
    element2 = (ASSOCIPLST_ELM_T *)elm2;

    return memcmp(element1->assoc_ip_addr, element2->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
} /* VRRP_OM_Assoc_Ip_Compare */


