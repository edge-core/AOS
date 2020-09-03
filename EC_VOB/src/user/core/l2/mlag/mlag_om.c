/* =============================================================================
 * MODULE NAME : MLAG_OM.C
 * PURPOSE     : Provide definitions for MLAG data management.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_hisam.h"
#include "l_link_lst.h"
#include "l_sort_lst.h"
#include "l_cvrt.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "mlag_om.h"
#include "mlag_om_private.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

 /* for mlag entry */
enum
{
    MLAG_OM_KIDX_MLAG_ID,
    MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID,
};

/* for fdb entry */
enum
{
    MLAG_OM_KIDX_MAC_VID,
    MLAG_OM_KIDX_MLAG_ID_MAC_VID,
};

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_OM_ENTER_CRITICAL_SECTION() \
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(om_sem_id)

#define MLAG_OM_LEAVE_CRITICAL_SECTION() \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(om_sem_id, orig_priority)

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

static int MLAG_OM_CompareDomain(void *inlist_elem, void *input_elem);
static BOOL_T MLAG_OM_ComparePrivate(void *inlist_elem, void *input_elem);

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

static UI32_T   om_sem_id;
static UI32_T   orig_priority;

static UI32_T               global_status;
static L_SORT_LST_List_T    domain_list;
static L_HISAM_Desc_T       mlag_desc;
static L_HISAM_Desc_T       fdb_desc; /* for remote FDB */
static L_LINK_LST_List_T    private_list; /* for timer, remote digest */

static UI32_T   lport_to_mlag_id[SYS_ADPT_TOTAL_NBR_OF_LPORT];

static BOOL_T   debug_tx;
static BOOL_T   debug_rx;
static BOOL_T   debug_error;
static BOOL_T   debug_thread;

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_OM_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_OM_InitiateProcessResources()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    const L_HISAM_KeyType_T
            mlag_key_type_table[2][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
    {
        /* mlag_id */
        {L_HISAM_4BYTE_INTEGER},
        /* domain_id, mlag_id */
        {L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER},
    };

    const L_HISAM_KeyType_T
            fdb_key_type_table[2][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
    {
        /* mac, vid */
        {L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER},
        /* mlag_id, mac, vid */
        {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER},
    };

    /* LOCAL VARIABLE DECLARATIONS
     */

    L_HISAM_KeyDef_T    key_def_table[2];

    /* BODY
     */

    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &om_sem_id)
            != SYSFUN_OK)
    {
        printf("\r\n%s: failed to create semaphore.\r\n", __func__);
        return;
    }

    if (L_SORT_LST_Create(&domain_list, MLAG_TYPE_MAX_NBR_OF_DOMAIN,
            sizeof(MLAG_TYPE_DomainEntry_T), MLAG_OM_CompareDomain) == FALSE)
    {
        printf("\r\n%s: failed to create domain list\r\n", __func__);
        return;
    }

    memset(&mlag_desc, 0, sizeof(L_HISAM_Desc_T));
    mlag_desc.total_record_nbr = MLAG_TYPE_MAX_NBR_OF_MLAG;
    mlag_desc.total_index_nbr = 100;
    mlag_desc.total_hash_nbr = MLAG_TYPE_MAX_NBR_OF_MLAG;
    mlag_desc.record_length = sizeof(MLAG_TYPE_MlagEntry_T);
    mlag_desc.hash_depth = 9;
    mlag_desc.N1 = 3;
    mlag_desc.N2 = 13;
    key_def_table[MLAG_OM_KIDX_MLAG_ID].field_number = 1;
    key_def_table[MLAG_OM_KIDX_MLAG_ID].offset[0] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_TYPE_MlagEntry_T*)0)->mlag_id));
    key_def_table[MLAG_OM_KIDX_MLAG_ID].length[0] =
        sizeof(((MLAG_TYPE_MlagEntry_T*)0)->mlag_id);
    key_def_table[MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID].field_number = 2;
    key_def_table[MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID].offset[0] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_TYPE_MlagEntry_T*)0)->domain_id));
    key_def_table[MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID].offset[1] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_TYPE_MlagEntry_T*)0)->mlag_id));
    key_def_table[MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID].length[0] =
        sizeof(((MLAG_TYPE_MlagEntry_T*)0)->domain_id);
    key_def_table[MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID].length[1] =
        sizeof(((MLAG_TYPE_MlagEntry_T*)0)->mlag_id);
    if (L_HISAM_CreateV2(&mlag_desc, 2, key_def_table, mlag_key_type_table)
            == FALSE)
    {
        printf("\r\n%s: failed to create MLAG hisam\r\n", __func__);
        return;
    }

    memset(&fdb_desc, 0, sizeof(L_HISAM_Desc_T));
    fdb_desc.total_record_nbr = MLAG_TYPE_MAX_NBR_OF_REMOTE_FDB;
    fdb_desc.total_index_nbr = 200;
    fdb_desc.total_hash_nbr = MLAG_TYPE_MAX_NBR_OF_REMOTE_FDB;
    fdb_desc.record_length = sizeof(MLAG_OM_FdbEntry_T);
    fdb_desc.hash_depth = 10;
    fdb_desc.N1 = 4;
    fdb_desc.N2 = 14;
    key_def_table[MLAG_OM_KIDX_MAC_VID].field_number = 2;
    key_def_table[MLAG_OM_KIDX_MAC_VID].offset[0] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_OM_FdbEntry_T*)0)->mac));
    key_def_table[MLAG_OM_KIDX_MAC_VID].offset[1] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_OM_FdbEntry_T*)0)->vid));
    key_def_table[MLAG_OM_KIDX_MAC_VID].length[0] =
        sizeof(((MLAG_OM_FdbEntry_T*)0)->mac);
    key_def_table[MLAG_OM_KIDX_MAC_VID].length[1] =
        sizeof(((MLAG_OM_FdbEntry_T*)0)->vid);
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].field_number = 3;
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].offset[0] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_OM_FdbEntry_T*)0)->mlag_id));
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].offset[1] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_OM_FdbEntry_T*)0)->mac));
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].offset[2] =
        (UI32_T)L_CVRT_PTR_TO_UINT(&(((MLAG_OM_FdbEntry_T*)0)->vid));
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].length[0] =
        sizeof(((MLAG_OM_FdbEntry_T*)0)->mlag_id);
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].length[1] =
        sizeof(((MLAG_OM_FdbEntry_T*)0)->mac);
    key_def_table[MLAG_OM_KIDX_MLAG_ID_MAC_VID].length[2] =
        sizeof(((MLAG_OM_FdbEntry_T*)0)->vid);
    if (L_HISAM_CreateV2(&fdb_desc, 2, key_def_table, fdb_key_type_table)
            == FALSE)
    {
        printf("\r\n%s: failed to create FDB hisam\r\n", __func__);
        return;
    }

    if (L_LINK_LST_Create(&private_list, MLAG_TYPE_MAX_NBR_OF_DOMAIN,
            sizeof(MLAG_OM_PrivateEntry_T), MLAG_OM_ComparePrivate, FALSE)
            == FALSE)
    {
        printf("\r\n%s: failed to create internal list\r\n", __func__);
        return;
    }

    return;
} /* End of MLAG_OM_InitiateProcessResources */

/* FUNCTION NAME - MLAG_OM_ClearAll
 * PURPOSE : Clear all dynamic data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void MLAG_OM_ClearAll()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_SORT_LST_Delete_All(&domain_list) == FALSE)
    {
        printf("\r\n%s: failed to clear domain list\r\n", __func__);
        return;
    }

    L_HISAM_DeleteAllRecord(&mlag_desc);

    L_HISAM_DeleteAllRecord(&fdb_desc);

    if (L_LINK_LST_Delete_All(&private_list) == FALSE)
    {
        printf("\r\n%s: failed to clear private list\r\n", __func__);
        return;
    }

    memset(lport_to_mlag_id, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);

    MLAG_OM_LEAVE_CRITICAL_SECTION();

    debug_tx = FALSE;
    debug_rx = FALSE;
    debug_error = FALSE;
    debug_thread = FALSE;
    return;
} /* End of MLAG_OM_ClearAll */

/* FUNCTION NAME - MLAG_OM_DefaultAll
 * PURPOSE : Set all default values.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void MLAG_OM_DefaultAll()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_OM_ENTER_CRITICAL_SECTION();
    global_status = MLAG_TYPE_DEFAULT_GLOBAL_STATUS;
    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of MLAG_OM_DefaultAll */

/* FUNCTION NAME - MLAG_OM_GetGlobalStatus
 * PURPOSE : Get global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetGlobalStatus(UI32_T *status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (status_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();
    *status_p = global_status;
    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_GetGlobalStatus */

/* FUNCTION NAME - MLAG_OM_GetRunningGlobalStatus
 * PURPOSE : Get running global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : None
 */
SYS_TYPE_Get_Running_Cfg_T MLAG_OM_GetRunningGlobalStatus(UI32_T *status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (status_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();
    *status_p = global_status;
    MLAG_OM_LEAVE_CRITICAL_SECTION();

    if (*status_p == MLAG_TYPE_DEFAULT_GLOBAL_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of MLAG_OM_GetRunningGlobalStatus */

/* FUNCTION NAME - MLAG_OM_SetGlobalStatus
 * PURPOSE : Set global status of the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_SetGlobalStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_OM_ENTER_CRITICAL_SECTION();
    global_status = status;
    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_SetGlobalStatus */

/* FUNCTION NAME - MLAG_OM_GetDomainEntry
 * PURPOSE : Get a MLAG domain entry.
 * INPUT   : entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_SORT_LST_Get(&domain_list, entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetDomainEntry */

/* FUNCTION NAME - MLAG_OM_GetDomainEntryByPort
 * PURPOSE : Get a MLAG domain entry by given peer link.
 * INPUT   : entry_p->peer_link -- peer link
 * OUTPUT  : entry_p -- buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetDomainEntryByPort(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T local_entry;
    UI32_T                  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&local_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    while (L_SORT_LST_Get_Next(&domain_list, &local_entry) == TRUE)
    {
        if (local_entry.peer_link == entry_p->peer_link)
        {
            memcpy(entry_p, &local_entry, sizeof(MLAG_TYPE_DomainEntry_T));
            result = MLAG_TYPE_RETURN_OK;
            break;
        }
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetDomainEntry */

/* FUNCTION NAME - MLAG_OM_GetNextDomainEntry
 * PURPOSE : Get next MLAG domain entry.
 * INPUT   : entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Empty string for domain ID to get the first entry
 */
UI32_T MLAG_OM_GetNextDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_SORT_LST_Get_Next(&domain_list, entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetNextDomainEntry */

/* FUNCTION NAME - MLAG_OM_SetDomainEntry
 * PURPOSE : Set a MLAG domain entry.
 * INPUT   : entry_p -- buffer containing information for a domain
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_SetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_SORT_LST_Set(&domain_list, entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }
    else
    {
        result = MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_SetDomainEntry */

/* FUNCTION NAME - MLAG_OM_RemoveDomainEntry
 * PURPOSE : Set a MLAG domain entry.
 * INPUT   : entry_p->domain_id -- buffer containing information for a domain
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_RemoveDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_SORT_LST_Delete(&domain_list, entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }
    else
    {
        result = MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_RemoveDomainEntry */

/* FUNCTION NAME - MLAG_OM_GetMlagEntry
 * PURPOSE : Get a MLAG entry.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing information for a MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_HISAM_GetRecord(&mlag_desc, MLAG_OM_KIDX_MLAG_ID,
        (UI8_T*)&(entry_p->mlag_id), (UI8_T*)entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetMlagEntry */

/* FUNCTION NAME - MLAG_OM_GetMlagEntryByPort
 * PURPOSE : Get a MLAG entry by given local MLAG member.
 * INPUT   : entry_p->local_member -- local MLAG member
 * OUTPUT  : entry_p -- buffer containing the MLAG entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetMlagEntryByPort(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();

    entry_p->mlag_id = lport_to_mlag_id[entry_p->local_member - 1];
    if (entry_p->mlag_id == 0)
    {
        result = MLAG_TYPE_RETURN_ERROR;
    }
    else if (L_HISAM_GetRecord(&mlag_desc, MLAG_OM_KIDX_MLAG_ID,
                (UI8_T*)&(entry_p->mlag_id), (UI8_T*)entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }
    else
    {
        result = MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetMlagEntryByPort */

/* FUNCTION NAME - MLAG_OM_GetNextMlagEntry
 * PURPOSE : Get next MLAG entry.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_OM_GetNextMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_HISAM_GetNextRecord(&mlag_desc, MLAG_OM_KIDX_MLAG_ID,
        (UI8_T*)&entry_p->mlag_id, (UI8_T*)entry_p) == TRUE)
    {
        result = MLAG_TYPE_RETURN_OK;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetNextMlagEntry */

/* FUNCTION NAME - MLAG_OM_GetNextMlagEntryByDomain
 * PURPOSE : Get next MLAG entry in the given domain.
 * INPUT   : entry_p->mlag_id   -- MLAG ID
 *           entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_OM_GetNextMlagEntryByDomain(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   key_ar[MLAG_TYPE_MAX_DOMAIN_ID_LEN + 1 + sizeof(UI32_T)];
    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(key_ar, 0, sizeof(key_ar));
    memcpy(key_ar, entry_p->domain_id, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    memcpy(key_ar+MLAG_TYPE_MAX_DOMAIN_ID_LEN+1, &(entry_p->mlag_id),
        sizeof(UI32_T));

    result = MLAG_TYPE_RETURN_ERROR;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_HISAM_GetNextRecord(&mlag_desc, MLAG_OM_KIDX_DOMAIN_ID_MLAG_ID,
        key_ar, (UI8_T*)entry_p) == TRUE)
    {
        if (strncmp((char*)key_ar, entry_p->domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN) == 0)
        {
            result = MLAG_TYPE_RETURN_OK;
        }
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_GetNextMlagEntryByDomain */

/* FUNCTION NAME - MLAG_OM_SetMlagEntry
 * PURPOSE : Set a MLAG entry.
 * INPUT   : entry_p -- buffer containing information for a MLAG
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_OK / MLAG_TYPE_RETURN_ERROR
 * NOTE    : None
 */
UI32_T MLAG_OM_SetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   old_entry;
    UI32_T                  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&old_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    old_entry.mlag_id = entry_p->mlag_id;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_HISAM_GetRecord(&mlag_desc, MLAG_OM_KIDX_MLAG_ID,
            (UI8_T*)&(old_entry.mlag_id), (UI8_T*)&old_entry) == TRUE)
    {
        if (old_entry.local_member != entry_p->local_member)
        {
            lport_to_mlag_id[old_entry.local_member - 1] = 0;
        }
    }

    result = L_HISAM_SetRecord(&mlag_desc, (UI8_T*)entry_p, TRUE);
    switch (result)
    {
    case L_HISAM_INSERT:
    case L_HISAM_REPLACE:
        lport_to_mlag_id[entry_p->local_member - 1] = entry_p->mlag_id;
        result = MLAG_TYPE_RETURN_OK;
        break;

    default:
        result = MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_SetMlagEntry */

/* FUNCTION NAME - MLAG_OM_RemoveMlagEntry
 * PURPOSE : Remove a MLAG entry.
 * INPUT   : entry_p -- buffer containing information for a MLAG
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_RemoveMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (L_HISAM_DeleteRecord(&mlag_desc, (UI8_T*)&(entry_p->mlag_id)) == TRUE)
    {
        lport_to_mlag_id[entry_p->local_member - 1] = 0;
        result = MLAG_TYPE_RETURN_OK;
    }
    else
    {
        result = MLAG_TYPE_RETURN_ERROR;
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_RemoveMlagEntry */

/* FUNCTION NAME - MLAG_OM_IsMlagPort
 * PURPOSE : Check whether a logical port is peer link or MLAG member.
 * INPUT   : lport -- logical port to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- peer link or MLAG member
 *           FALSE -- otherwise
 * NOTE    : None
 */
BOOL_T MLAG_OM_IsMlagPort(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T domain_entry;
    BOOL_T                  result;

    /* BODY
     */

    result = FALSE;

    MLAG_OM_ENTER_CRITICAL_SECTION();

    if (lport_to_mlag_id[lport - 1] > 0)
    {
        result = TRUE;
    }
    else
    {
        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        while (L_SORT_LST_Get_Next(&domain_list, &domain_entry) == TRUE)
        {
            if (domain_entry.peer_link == lport)
            {
                result = TRUE;
                break;
            }
        }
    }

    MLAG_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of MLAG_OM_IsMlagPort */

/* FUNCTION NAME - MLAG_OM_GetRemoteFdbEntry
 * PURPOSE : Get a remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : entry_p -- buffer containing a remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   key_ar[SYS_ADPT_MAC_ADDR_LEN + sizeof(UI32_T)];

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(key_ar, 0, sizeof(key_ar));
    memcpy(key_ar, entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(key_ar+SYS_ADPT_MAC_ADDR_LEN, &(entry_p->vid), sizeof(UI32_T));

    if (L_HISAM_GetRecord(&fdb_desc, MLAG_OM_KIDX_MAC_VID, key_ar,
            (UI8_T*)entry_p) == TRUE)
    {
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_OM_GetRemoteFdbEntry */

/* FUNCTION NAME - MLAG_OM_GetNextRemoteFdbEntry
 * PURPOSE : Get next remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : entry_p -- buffer containing next remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetNextRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   key_ar[SYS_ADPT_MAC_ADDR_LEN + sizeof(UI32_T)];

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(key_ar, 0, sizeof(key_ar));
    memcpy(key_ar, entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(key_ar+SYS_ADPT_MAC_ADDR_LEN, &(entry_p->vid), sizeof(UI32_T));

    if (L_HISAM_GetNextRecord(&fdb_desc, MLAG_OM_KIDX_MAC_VID, key_ar,
            (UI8_T*)entry_p) == TRUE)
    {
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_OM_GetNextRemoteFdbEntry */

/* FUNCTION NAME - MLAG_OM_GetNextRemoteFdbEntryByMlag
 * PURPOSE : Get next remote FDB entry by given MLAG ID.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing next remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetNextRemoteFdbEntryByMlag(MLAG_OM_FdbEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   key_ar[SYS_ADPT_MAC_ADDR_LEN + sizeof(UI32_T) + sizeof(UI32_T)];

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(key_ar, 0, sizeof(key_ar));
    memcpy(key_ar, &(entry_p->mlag_id), sizeof(UI32_T));
    memcpy(key_ar+sizeof(UI32_T), entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(key_ar+sizeof(UI32_T)+SYS_ADPT_MAC_ADDR_LEN, &(entry_p->vid),
        sizeof(UI32_T));

    if (L_HISAM_GetNextRecord(&fdb_desc, MLAG_OM_KIDX_MLAG_ID_MAC_VID, key_ar,
            (UI8_T*)entry_p) == TRUE)
    {
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_OM_GetNextRemoteFdbEntry */

/* FUNCTION NAME - MLAG_OM_GetFreeRemoteFdbCount
 * PURPOSE : Get the free count for remote FDB.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : free count for remote FDB
 * NOTE    : None
 */
UI32_T MLAG_OM_GetFreeRemoteFdbCount()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return L_HISAM_GetFreeBufNo(&fdb_desc);
} /* End of MLAG_OM_GetFreeRemoteFdbCount */

/* FUNCTION NAME - MLAG_OM_SetRemoteFdbEntry
 * PURPOSE : Set a remote FDB entry.
 * INPUT   : entry_p -- buffer containing a remote FDB entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    switch (L_HISAM_SetRecord(&fdb_desc, (UI8_T*)entry_p, TRUE))
    {
    case L_HISAM_INSERT:
    case L_HISAM_REPLACE:
        return MLAG_TYPE_RETURN_OK;

    default:
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_OM_SetRemoteFdbEntry */

/* FUNCTION NAME - MLAG_OM_RemoveRemoteFdbEntry
 * PURPOSE : Remove a remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_RemoveRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   key_ar[SYS_ADPT_MAC_ADDR_LEN+sizeof(UI32_T)];

    /* BODY
     */

    if (entry_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(key_ar, 0, sizeof(key_ar));
    memcpy(key_ar, entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(key_ar+SYS_ADPT_MAC_ADDR_LEN, &(entry_p->vid), sizeof(UI32_T));

    if (L_HISAM_DeleteRecord(&fdb_desc, key_ar) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_RemoveRemoteFdbEntry */

/* FUNCTION NAME - MLAG_OM_AddPrivateEntry
 * PURPOSE : Add a private entry.
 * INPUT   : domain_id_p -- domain ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : All fields except key are cleaned
 */
UI32_T MLAG_OM_AddPrivateEntry(char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (L_LINK_LST_Set(&private_list, &entry, L_LINK_LST_APPEND) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_AddPrivateEntry */

/* FUNCTION NAME - MLAG_OM_DeletePrivateEntry
 * PURPOSE : Delete a private entry.
 * INPUT   : domain_id_p -- domain ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_DeletePrivateEntry(char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (L_LINK_LST_Delete(&private_list, &entry) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_DeletePrivateEntry */

/* FUNCTION NAME - MLAG_OM_GetTimer
 * PURPOSE : Get the value of a kind of timer for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_TIMER_E
 * OUTPUT  : None
 * RETURN  : timer value
 * NOTE    : None
 */
UI32_T MLAG_OM_GetTimer(char *domain_id_p, UI32_T kind)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return 0;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (L_LINK_LST_Get(&private_list, &entry) == TRUE)
    {
        switch (kind)
        {
        case MLAG_OM_TIMER_TX:
            return entry.tx_timer;

        case MLAG_OM_TIMER_FDB:
            return entry.fdb_timer;

        case MLAG_OM_TIMER_RX:
            return entry.rx_timer;
        }
    }

    return 0;
} /* End of MLAG_OM_GetTimer */

/* FUNCTION NAME - MLAG_OM_SetTimer
 * PURPOSE : Set the value of a kind of timer for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_TIMER_E
 *           value       -- timer value
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetTimer(char *domain_id_p, UI32_T kind, UI32_T value)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (L_LINK_LST_Get(&private_list, &entry) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    switch (kind)
    {
    case MLAG_OM_TIMER_TX:
        entry.tx_timer = value;
        break;

    case MLAG_OM_TIMER_FDB:
        entry.fdb_timer = value;
        break;

    case MLAG_OM_TIMER_RX:
        entry.rx_timer = value;
        break;

    default:
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (L_LINK_LST_Set(&private_list, &entry, L_LINK_LST_APPEND) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_SetTimer */


/* FUNCTION NAME - MLAG_OM_SameDigest
 * PURPOSE : Check whether the value of a kind of digest is the same as that in
 *           database for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_DIGEST_E
 *           digest_p    -- digest to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- same
 *           FALSE -- different
 * NOTE    : None
 */
BOOL_T MLAG_OM_SameDigest(char *domain_id_p, UI32_T kind, UI8_T *digest_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if ((domain_id_p == NULL) || (digest_p == NULL))
    {
        return FALSE;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (L_LINK_LST_Get(&private_list, &entry) == FALSE)
    {
        return FALSE;
    }

    switch (kind)
    {
    case MLAG_OM_DIGEST_LINK:
        if (memcmp(entry.remote_link_digest, digest_p, MLAG_OM_DIGEST_SIZE)
                == 0)
        {
            return TRUE;
        }
        break;

    case MLAG_OM_DIGEST_FDB:
        if (memcmp(entry.remote_fdb_digest, digest_p, MLAG_OM_DIGEST_SIZE)
                == 0)
        {
            return TRUE;
        }
        break;
    }

    return FALSE;
} /* End of MLAG_OM_SameDigest */

/* FUNCTION NAME - MLAG_OM_SetDigest
 * PURPOSE : Set the value of a kind of digest for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_DIGEST_E
 *           digest_p    -- digest to be set
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetDigest(char *domain_id_p, UI32_T kind, UI8_T *digest_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T  entry;

    /* BODY
     */

    if ((domain_id_p == NULL))
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&entry, 0, sizeof(MLAG_OM_PrivateEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (L_LINK_LST_Get(&private_list, &entry) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    switch (kind)
    {
    case MLAG_OM_DIGEST_LINK:
        if (digest_p == NULL)
        {
            memset(entry.remote_link_digest, 0, MLAG_OM_DIGEST_SIZE);
        }
        else
        {
            memcpy(entry.remote_link_digest, digest_p, MLAG_OM_DIGEST_SIZE);
        }
        break;

    case MLAG_OM_DIGEST_FDB:
        if (digest_p == NULL)
        {
            memset(entry.remote_fdb_digest, 0, MLAG_OM_DIGEST_SIZE);
        }
        else
        {
            memcpy(entry.remote_fdb_digest, digest_p, MLAG_OM_DIGEST_SIZE);
        }
        break;

    default:
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (L_LINK_LST_Set(&private_list, &entry, L_LINK_LST_APPEND) == FALSE)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_OM_SetDigest */

/* FUNCTION NAME - MLAG_OM_GetDebugFlag
 * PURPOSE : Get current value of specified kind of debug flag.
 * INPUT   : flag -- MLAG_OM_DEBUG_FLAG_E
 * OUTPUT  : None
 * RETURN  : current value of the debug flag
 * NOTE    : None
 */
BOOL_T MLAG_OM_GetDebugFlag(UI32_T flag)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    switch (flag)
    {
    case MLAG_OM_DEBUG_TX:
        return debug_tx;

    case MLAG_OM_DEBUG_RX:
        return debug_rx;

    case MLAG_OM_DEBUG_ERROR:
        return debug_error;

    case MLAG_OM_DEBUG_THREAD:
        return debug_thread;

    default:
        return FALSE;
    }
} /* End of MLAG_OM_GetDebugFlag */

/* FUNCTION NAME - MLAG_OM_SetDebugFlag
 * PURPOSE : Set the value of specified kind of debug flag.
 * INPUT   : flag  -- MLAG_OM_DEBUG_FLAG_E
 *           value -- the value to be set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_OM_SetDebugFlag(UI32_T flag, BOOL_T value)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    switch (flag)
    {
    case MLAG_OM_DEBUG_TX:
        debug_tx = value;
        break;

    case MLAG_OM_DEBUG_RX:
        debug_rx = value;
        break;

    case MLAG_OM_DEBUG_ERROR:
        debug_error = value;
        break;

    case MLAG_OM_DEBUG_THREAD:
        debug_thread = value;
        break;
    }

    return;
} /* End of MLAG_OM_SetDebugFlag */

/* FUNCTION NAME - MLAG_OM_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T MLAG_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_IpcMsg_T *msg_p;

    /* BODY
     */

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding OM function
     */
    switch (msg_p->type.cmd)
    {
    case MLAG_OM_IPC_GETGLOBALSTATUS:
        msg_p->type.ret_ui32 = MLAG_OM_GetGlobalStatus(&msg_p->data.arg_ui32);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_ui32);
        break;

    case MLAG_OM_IPC_GETRUNNINGGLOBALSTATUS:
        msg_p->type.ret_ui32 = MLAG_OM_GetRunningGlobalStatus(
                                &msg_p->data.arg_ui32);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_ui32);
        break;

    case MLAG_OM_IPC_GETDOMAINENTRY:
        msg_p->type.ret_ui32 = MLAG_OM_GetDomainEntry(&msg_p->data.arg_domain);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_domain);
        break;

    case MLAG_OM_IPC_GETNEXTDOMAINENTRY:
        msg_p->type.ret_ui32 = MLAG_OM_GetNextDomainEntry(
                                &msg_p->data.arg_domain);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_domain);
        break;

    case MLAG_OM_IPC_GETMLAGENTRY:
        msg_p->type.ret_ui32 = MLAG_OM_GetMlagEntry(&msg_p->data.arg_mlag);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);
        break;

    case MLAG_OM_IPC_GETNEXTMLAGENTRY:
        msg_p->type.ret_ui32 = MLAG_OM_GetNextMlagEntry(&msg_p->data.arg_mlag);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);
        break;

    case MLAG_OM_IPC_GETNEXTMLAGENTRYBYDOMAIN:
        msg_p->type.ret_ui32 = MLAG_OM_GetNextMlagEntryByDomain(
                                &msg_p->data.arg_mlag);
        msgbuf_p->msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);
        break;

    case MLAG_OM_IPC_ISMLAGPORT:
        msg_p->type.ret_bool = MLAG_OM_IsMlagPort(msg_p->data.arg_ui32);
        msgbuf_p->msg_size = MLAG_OM_IPCMSG_TYPE_SIZE;
        break;

    default:
        if (debug_error == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n%s: invalid IPC command\r\n", __func__);
        }
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = MLAG_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of MLAG_OM_HandleIPCReqMsg */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_OM_CompareDomain
 * PURPOSE : Compare with a domain element in the list.
 * INPUT   : *inlist_elem -- the element in the list
 *           *input_elem  -- the element to be compared with
 * OUTPUT  : None
 * RETURN  : >0, =0, <0
 * NOTE    : None
 */
static int MLAG_OM_CompareDomain(void *inlist_elem, void *input_elem)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T *inlist_p = inlist_elem;
    MLAG_TYPE_DomainEntry_T *input_p  = input_elem;

    /* BODY
     */

    return strncmp(inlist_p->domain_id, input_p->domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN);
} /* End of MLAG_OM_CompareDomain */

/* FUNCTION NAME - MLAG_OM_ComparePrivate
 * PURPOSE : Compare with a private element in the list.
 * INPUT   : *inlist_elem -- the element in the list
 *           *input_elem  -- the new element to be compared with
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 */
static BOOL_T MLAG_OM_ComparePrivate(void *inlist_elem, void *input_elem)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_OM_PrivateEntry_T *inlist_p = inlist_elem;
    MLAG_OM_PrivateEntry_T *input_p  = input_elem;

    /* BODY
     */

    if (strncmp(inlist_p->domain_id, input_p->domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* End of MLAG_OM_ComparePrivate */
