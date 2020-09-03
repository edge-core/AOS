/* MODULE NAME: NETCFG_OM_PBR.c
 * PURPOSE:
 *   Definitions of OM APIs for PBR
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   2015/07/09     --- KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_PBR == TRUE)
#include "sys_bld.h"
#include "sys_dflt.h"
#include "l_inet.h"
#include "l_hisam.h"
#include "netcfg_om_pbr.h"

static  NETCFG_OM_PBR_BindingEntry_T                  *_NETCFG_OM_PBR_BindingEntry_T;

#define NETCFG_OM_PBR_SIZEOF(type, field)               sizeof (_##type->field)
#define NETCFG_OM_PBR_OFFSET(offset, type, field)       { type v; offset=(uintptr_t)&v.field - (uintptr_t)&v; }

#define SIZE_OF_VID                             NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, vid)
#define SIZE_OF_RMAP_NAME                       NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, rmap_name)
#define SIZE_OF_SEQ_NUM                         NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, seq_num)
#define SIZE_OF_ACL_NAME                        NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, acl_name)
#define SIZE_OF_NEXTHOP                         NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, nexthop)

#define NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN          (SIZE_OF_VID + SIZE_OF_RMAP_NAME + SIZE_OF_SEQ_NUM)
#define NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KEY_LEN          (SIZE_OF_RMAP_NAME + SIZE_OF_VID + SIZE_OF_SEQ_NUM)
#define NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KEY_LEN      (SIZE_OF_ACL_NAME + SIZE_OF_VID + SIZE_OF_RMAP_NAME + SIZE_OF_SEQ_NUM)
#define NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KEY_LEN  (SIZE_OF_NEXTHOP + SIZE_OF_VID + SIZE_OF_RMAP_NAME + SIZE_OF_SEQ_NUM)

#define NETCFG_OM_PBR_BINDING_ENTRY_NBR	    (SYS_ADPT_MAX_NBR_OF_L3_INTERFACE * SYS_ADPT_MAX_SEQUENCE_PER_ROUTE_MAP)
#define NETCFG_OM_PBR_BINDING_INDEX_NBR	    ((NETCFG_OM_PBR_BINDING_ENTRY_NBR * 2 / NETCFG_OM_PBR_BINDING_HISAM_N2) + 1)
#define NETCFG_OM_PBR_BINDING_HISAM_N1      4
#define NETCFG_OM_PBR_BINDING_HISAM_N2      64
#define NETCFG_OM_PBR_BINDING_HASH_DEPTH    4
#define NETCFG_OM_PBR_BINDING_HASH_NBR      (SYS_ADPT_MAX_NBR_OF_VLAN / 2)
#define NETCFG_OM_PBR_BINDING_ENTRY_LEN     (sizeof(NETCFG_OM_PBR_BindingEntry_T))

enum NETCFG_OM_PBR_BINDING_KEY_E
{
    NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX = 0,        /* key: vid + rmap_name + seq_num */
    NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX,            /* key: rmap_name + vid + seq_num */
    NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX,        /* key: acl_name + vid + rmap_name + seq_num */
    NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX     /* key: nexthop + vid + rmap_name + seq_num */
};

#define PBR_OM_ENTER_CRITICAL_SECTION()   SYSFUN_TakeSem(pbr_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define PBR_OM_LEAVE_CRITICAL_SECTION()   SYSFUN_GiveSem(pbr_om_sem_id)

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T pbr_om_sem_id = 0;
static L_HISAM_KeyDef_T     pbr_om_binding_key_def_table[4];
static L_HISAM_Desc_T       pbr_om_binding_table;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NETCFG_OM_PBR_SetBindingKey(UI8_T *key, UI32_T key_index, NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);
static SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_PBR_GetRunningBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1]);


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Init the OM resouces.
 * INPUT:    none
 * OUTPUT:   none.
 * RETURN:
 * NOTES:
 * -------------------------------------------------------------------------*/
void NETCFG_OM_PBR_Initiate_System_Resources(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &pbr_om_sem_id) != SYSFUN_OK)
    {
        printf("\r\n%s: get om sem id fail!\r\n", __FUNCTION__);
    }

    NETCFG_OM_PBR_Init();

    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_Init
 *--------------------------------------------------------------------------
 * PURPOSE: To create the binding table
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_OM_PBR_Init()
{
    /* Key: vid + rmap_name + seq_num
     */
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].field_number = 3;
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].offset[0], NETCFG_OM_PBR_BindingEntry_T, vid);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].offset[1], NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].offset[2], NETCFG_OM_PBR_BindingEntry_T, seq_num);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].length[0] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, vid);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].length[1] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX].length[2] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, seq_num);

    /* Key: rmap_name + vid + seq_num
     */
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].field_number = 3;
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].offset[0], NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].offset[1], NETCFG_OM_PBR_BindingEntry_T, vid);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].offset[2], NETCFG_OM_PBR_BindingEntry_T, seq_num);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].length[0] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].length[1] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, vid);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX].length[2] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, seq_num);

    /* Key: acl_name + vid + rmap_name + seq_num
     */
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].field_number = 4;
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].offset[0], NETCFG_OM_PBR_BindingEntry_T, acl_name);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].offset[1], NETCFG_OM_PBR_BindingEntry_T, vid);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].offset[2], NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].offset[3], NETCFG_OM_PBR_BindingEntry_T, seq_num);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].length[0] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, acl_name);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].length[1] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, vid);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].length[2] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX].length[3] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, seq_num);


    /* Key: nexthop + vid + rmap_name + seq_num
     */
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].field_number = 4;
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].offset[0], NETCFG_OM_PBR_BindingEntry_T, nexthop);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].offset[1], NETCFG_OM_PBR_BindingEntry_T, vid);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].offset[2], NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    NETCFG_OM_PBR_OFFSET(pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].offset[3], NETCFG_OM_PBR_BindingEntry_T, seq_num);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].length[0] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, nexthop);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].length[1] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, vid);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].length[2] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, rmap_name);
    pbr_om_binding_key_def_table[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX].length[3] = NETCFG_OM_PBR_SIZEOF(NETCFG_OM_PBR_BindingEntry_T, seq_num);

    pbr_om_binding_table.hash_depth         = NETCFG_OM_PBR_BINDING_HASH_DEPTH;
    pbr_om_binding_table.N1                 = NETCFG_OM_PBR_BINDING_HISAM_N1;
    pbr_om_binding_table.N2                 = NETCFG_OM_PBR_BINDING_HISAM_N2;
    pbr_om_binding_table.record_length      = NETCFG_OM_PBR_BINDING_ENTRY_LEN;
    pbr_om_binding_table.total_hash_nbr     = NETCFG_OM_PBR_BINDING_HASH_NBR;
    pbr_om_binding_table.total_index_nbr    = NETCFG_OM_PBR_BINDING_INDEX_NBR;
    pbr_om_binding_table.total_record_nbr   = NETCFG_OM_PBR_BINDING_ENTRY_NBR;

    if (FALSE == L_HISAM_Create(&pbr_om_binding_table, 4 ,pbr_om_binding_key_def_table))
    {
        printf("NETCFG_OM_PBR create binding table failed.\n");
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_ClearAll
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_OM_PBR_ClearAll(void)
{
    PBR_OM_ENTER_CRITICAL_SECTION();

    L_HISAM_DeleteAllRecord(&pbr_om_binding_table);

    PBR_OM_LEAVE_CRITICAL_SECTION();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_AddBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Add a binding entry to OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_AddBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI32_T ret;

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_SetRecord(&pbr_om_binding_table, (UI8_T *)binding_entry_p, TRUE);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (ret == L_HISAM_INSERT || ret == L_HISAM_REPLACE)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get a binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    BOOL_T ret;

    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                            key, (UI8_T *)binding_entry_p);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    BOOL_T ret;

    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_DeleteRecord(&pbr_om_binding_table, key);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    BOOL_T ret;

    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)binding_entry_p);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by nexthop
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByNextHop(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KEY_LEN];
    BOOL_T ret;

    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)binding_entry_p);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByExactNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by nexthop
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES: 1. if the nexthop retrieved is not the same as input, return false
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByExactNextHop(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KEY_LEN];
    L_INET_AddrIp_T nexthop = binding_entry_p->nexthop;
    BOOL_T ret;

    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)binding_entry_p);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (L_INET_CompareInetAddr((L_INET_Addr_T *)&nexthop, (L_INET_Addr_T *)&binding_entry_p->nexthop, 0))
    {
        return FALSE;
    }

    return ret;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByAclName
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by ACL name
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByAclName(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KEY_LEN];
    char acl_name[SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1] = {0};
    BOOL_T ret;

    strncpy(acl_name, binding_entry_p->acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1);
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX, binding_entry_p);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)binding_entry_p);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (strncmp(acl_name, binding_entry_p->acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH) != 0)
        return FALSE;

    return ret;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetBindingRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Get binding route-map name by vlan id
 * INPUT:    vid        -- VLAN ID
 * OUTPUT:   rmap_name  -- route-map name
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    NETCFG_OM_PBR_BindingEntry_T binding_entry;
    BOOL_T ret;

    memset(&binding_entry, 0, sizeof(binding_entry));
    binding_entry.vid = vid;
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &binding_entry);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)&binding_entry);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (ret == TRUE && binding_entry.vid == vid)
    {
        strncpy(rmap_name, binding_entry.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
        return TRUE;
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingVlan
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding vlan id by route-map name
 * INPUT:    rmap_name  -- route-map name
 * OUTPUT:   vid        -- VLAN ID
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingVlan(char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1], UI32_T *vid)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KEY_LEN];
    NETCFG_OM_PBR_BindingEntry_T binding_entry;
    BOOL_T ret;

    memset(&binding_entry, 0, sizeof(binding_entry));
    strncpy(binding_entry.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    binding_entry.vid = *vid;
    binding_entry.seq_num = 0xFFFFFFFF;
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX, &binding_entry);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX,
                                key, (UI8_T *)&binding_entry);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (ret == TRUE && 0 == strncmp(binding_entry.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
    {
        *vid = binding_entry.vid;
        return TRUE;
    }

    return FALSE;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteEntriesByVidAndRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete binding entries by vid and route-map name
 * INPUT:    vid        -- VLAN ID
 *           rmap_name  -- route-map name
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTES: 1. won't delete the seq_num = 0 entry
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteEntriesByVidAndRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    NETCFG_OM_PBR_BindingEntry_T binding_entry;

    memset(&binding_entry, 0, sizeof(binding_entry));
    binding_entry.vid = vid;
    strncpy(binding_entry.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &binding_entry);

    PBR_OM_ENTER_CRITICAL_SECTION();

    while (L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                                 key, (UI8_T *)&binding_entry))
    {
        if (binding_entry.vid != vid || 0 != strncmp(rmap_name, binding_entry.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
            break;

        NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &binding_entry);
        L_HISAM_DeleteRecord(&pbr_om_binding_table, key);
    }

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteEntriesByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete binding entries by vid
 * INPUT:    vid        -- VLAN ID
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteEntriesByVid(UI32_T vid)
{
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    NETCFG_OM_PBR_BindingEntry_T binding_entry;

    memset(&binding_entry, 0, sizeof(binding_entry));
    binding_entry.vid = vid;
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &binding_entry);

    PBR_OM_ENTER_CRITICAL_SECTION();

    while (L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                                 key, (UI8_T *)&binding_entry))
    {
        if (binding_entry.vid != vid)
            break;

        NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &binding_entry);
        L_HISAM_DeleteRecord(&pbr_om_binding_table, key);
    }

    PBR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for PBR OM.
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
BOOL_T NETCFG_OM_PBR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_OM_PBR_IPCMsg_T *msg_p;
    UI32_T                 ret;

    if (ipcmsg_p == NULL)
        return FALSE;

    msg_p= (NETCFG_OM_PBR_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(msg_p->type.cmd)
    {
        case  NETCFG_OM_PBR_IPCCMD_GETNEXTBINDINGENTRY:
            msg_p->type.result_bool= NETCFG_OM_PBR_GetNextBindingEntry(&msg_p->data.binding_entry);
            ipcmsg_p->msg_size = NETCFG_OM_PBR_GET_MSG_SIZE(binding_entry);
            break;
        case NETCFG_OM_PBR_IPCCMD_GETRUNNINGBINDINGROUTEMAP:
            msg_p->type.result_running_cfg = NETCFG_OM_PBR_GetRunningBindingRouteMap(msg_p->data.binding_rmap.vid,
                                                   msg_p->data.binding_rmap.rmap_name);
            ipcmsg_p->msg_size = NETCFG_OM_PBR_GET_MSG_SIZE(binding_rmap);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            ipcmsg_p->msg_size = NETCFG_OM_PBR_MSGBUF_TYPE_SIZE;
            return FALSE;
    }
    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_SetBindingKey
 * -------------------------------------------------------------------------
 * PURPOSE:  Setting the key to the hisam
 * INPUT:    binding_entry_p: NETCFG_OM_PBR_BindingEntry_T
 *           key_index:  to decide which field should be set in key.
 * OUTPUT:   key.
 * RETURN:   none.
 * NOTES:    none.
 * -------------------------------------------------------------------------*/
static void NETCFG_OM_PBR_SetBindingKey(UI8_T *key, UI32_T key_index, NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    switch (key_index)
    {
        case NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX:
            memcpy(key, (UI8_T *)&(binding_entry_p->vid), SIZE_OF_VID);
            memcpy(key + SIZE_OF_VID, (UI8_T *)&(binding_entry_p->rmap_name), SIZE_OF_RMAP_NAME);
            memcpy(key + SIZE_OF_VID + SIZE_OF_RMAP_NAME, (UI8_T *)&(binding_entry_p->seq_num), SIZE_OF_SEQ_NUM);
            break;

        case NETCFG_OM_PBR_BINDING_RMAP_VID_SEQ_KIDX:
            memcpy(key, (UI8_T *)&(binding_entry_p->rmap_name), SIZE_OF_RMAP_NAME);
            memcpy(key + SIZE_OF_RMAP_NAME, (UI8_T *)&(binding_entry_p->vid), SIZE_OF_VID);
            memcpy(key + SIZE_OF_RMAP_NAME + SIZE_OF_VID, (UI8_T *)&(binding_entry_p->seq_num), SIZE_OF_SEQ_NUM);
            break;

        case NETCFG_OM_PBR_BINDING_ACL_VID_RMAP_SEQ_KIDX:
            memcpy(key, (UI8_T *)&(binding_entry_p->acl_name), SIZE_OF_ACL_NAME);
            memcpy(key + SIZE_OF_ACL_NAME, (UI8_T *)&(binding_entry_p->vid), SIZE_OF_VID);
            memcpy(key + SIZE_OF_ACL_NAME + SIZE_OF_VID, (UI8_T *)&(binding_entry_p->rmap_name), SIZE_OF_RMAP_NAME);
            memcpy(key + SIZE_OF_ACL_NAME + SIZE_OF_VID + SIZE_OF_RMAP_NAME, (UI8_T *)&(binding_entry_p->seq_num), SIZE_OF_SEQ_NUM);
            break;

        case NETCFG_OM_PBR_BINDING_NEXTHOP_VID_RMAP_SEQ_KIDX:
            memcpy(key, (UI8_T *)&(binding_entry_p->nexthop), SIZE_OF_NEXTHOP);
            memcpy(key + SIZE_OF_NEXTHOP, (UI8_T *)&(binding_entry_p->vid), SIZE_OF_VID);
            memcpy(key + SIZE_OF_NEXTHOP + SIZE_OF_VID, (UI8_T *)&(binding_entry_p->rmap_name), SIZE_OF_RMAP_NAME);
            memcpy(key + SIZE_OF_NEXTHOP + SIZE_OF_VID + SIZE_OF_RMAP_NAME, (UI8_T *)&(binding_entry_p->seq_num), SIZE_OF_SEQ_NUM);
            break;

        default:
            break;
    }

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetRunningBindingRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Get the binding route-map for a vlan
 * INPUT:    vid       -- vlan id
 * OUTPUT:   rmap_name -- bing route-map name
 * RETURN:
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:
 * -------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_PBR_GetRunningBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    NETCFG_OM_PBR_BindingEntry_T  pbr_binding;
    UI8_T key[NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KEY_LEN];
    BOOL_T ret;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    NETCFG_OM_PBR_SetBindingKey(key, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX, &pbr_binding);

    PBR_OM_ENTER_CRITICAL_SECTION();

    ret = L_HISAM_GetNextRecord(&pbr_om_binding_table, NETCFG_OM_PBR_BINDING_VID_RMAP_SEQ_KIDX,
                                key, (UI8_T *)&pbr_binding);

    PBR_OM_LEAVE_CRITICAL_SECTION();

    if (ret == TRUE && pbr_binding.vid == vid)
    {
        strncpy(rmap_name, pbr_binding.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
#endif /* #if (SYS_CPNT_PBR == TRUE) */
