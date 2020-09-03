/*-------------------------------------------------------------------------
 * MODULE NAME: amtr_om_private.c
 *-------------------------------------------------------------------------
 * PURPOSE: To store and manage AMTR Hisam Table.
 *
 *
 * NOTES:
 *          Functions in this module only visible within the process
 *          where AMTR lies.
 *
 * Modification History:
 *      Date                Modifier        Reason
 *      ------------------------------------
 *      03-15-2005    water_huang    create
 *
 * COPYRIGHT(C)         Accton Corporation, 2005
 *------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "l_hisam.h"
#include "sysfun.h"
#include "amtr_om.h"
#include "amtr_om_private.h"
#include "amtr_type.h"
#include "amtrdrv_lib.h"
#include "swctrl.h"
#include "amtr_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
*/
#if (SYS_CPNT_ADD == TRUE || SYS_CPNT_AMTRL3 == TRUE)
#define AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY    TRUE
#endif

/* Field length definitions for HISAM table
 */
#define NODE_LEN                            sizeof(AMTR_TYPE_AddrEntry_T)

/* constant definitions for HISAM
 */
#define NODE_NBR                            SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
#define INDEX_NBR                           100                 /* table is divided to 100 blocks  */
#define HISAM_N1                            30                  /* table balance threshold */
#define HISAM_N2                            135                 /* table balance threshold */
#define HASH_DEPTH                          4
#define HASH_NBR                            (NODE_NBR / 10)
#define NUMBER_OF_KEYS                      3

/* TYPE DEFINITIONS
*/
typedef struct
{
    UI32_T ifindex;
    UI32_T vid;
    UI32_T attribute;    /*sourece or life time*/
    UI16_T *vlan_p;

    /* for DeleteRecordByLPortAndVidExceptCertainAddr
     */
    UI32_T number_of_entry_in_list;
    UI8_T (*mac_list_p)[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T (*mask_list_p)[SYS_ADPT_MAC_ADDR_LEN];
}AMTR_OM_Cookie_T;

/* MACRO DEFINITIONS
 */
#if(SYS_CPNT_VXLAN == TRUE)
#define IS_IFINDEX_INVALID(ifindex)    ((ifindex == 0)||(ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN))
#else
#define IS_IFINDEX_INVALID(ifindex)    ((ifindex == 0)||(ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT))
#endif
#define AMTR_OM_ENTER_CRITICAL_SECTION()                                \
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtr_om_sem_id);

#define AMTR_OM_LEAVE_CRITICAL_SECTION()                              \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtr_om_sem_id, original_priority);

/* LOCAL FUNCTIONS DECLARATIONS
 */
static UI32_T AMTR_OM_HisamDeleteByLifeTimeCallBack(void * record, void *life_time);
static UI32_T AMTR_OM_HisamDeleteBySourceCallBack(void * record, void *source);
static UI32_T AMTR_OM_HisamDeleteByPortCallBack(void * record, void *ifindex);
static UI32_T AMTR_OM_HisamDeleteByPortNLifeTimeCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNSourceCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByVidCallBack(void * record, void *vid);
static UI32_T AMTR_OM_HisamDeleteByVidNLifeTimeCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByVidNSourceCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNVidCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNVidNLifeTimeCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNVlanlistNLifeTimeCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNVidNSourceCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamDeleteByPortNVidExceptCertainAddrCallBack(void * record, void *cookie);
static UI32_T AMTR_OM_HisamUpdateRecordLifeTimeByLPortCallBack(void * record, void *cookie);
static BOOL_T AMTR_OM_UpdateCountForRemoveAddr(UI8_T mv_key[AMTR_MVKEY_LEN]);
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
static void   AMTR_OM_SetMacNotifyDefaultValue(void);
#endif
#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
static void   AMTR_OM_SetVlanLearningDefaultValue(void);
#endif
#if (SYS_CPNT_MLAG == TRUE)
static void AMTR_OM_SetMlagMacNotifyDefaultValue(void);
#endif

/* LOCAL VARIABLES DECLARATIONS
 */
const static UI8_T amtr_om_null_mac[] = { 0,0,0,0,0,0 };

/* Define key table for search Hisam Table
 * Hisam Key is mac+vid+ifindex.
 */
const static L_HISAM_KeyDef_T key_def_table[NUMBER_OF_KEYS] =
                {
                    /* MV Key = mac + vid */
                    {   2,                              /* field number */
                        {4, 0, 0, 0, 0, 0, 0, 0},       /* offset */
                        {6, 2, 0, 0, 0, 0, 0, 0}        /* len */
                    },
                    /* VM Key = vid + mac */
                    {   2,                              /* field number */
                        {0, 4, 0, 0, 0, 0, 0, 0},       /* offset */
                        {2, 6, 0, 0, 0, 0, 0, 0}        /* len */
                    },
                    /* IVM Key = ifindex + vid + mac */
                    {   3,                              /* field number */
                        { 2, 0, 4, 0, 0, 0, 0, 0},      /* offset */
                        { 2, 2, 6, 0, 0, 0, 0, 0}       /* len */
                    }

                };

const static L_HISAM_KeyType_T key_type_table[NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* MV Key = mac + vid */
                    {L_HISAM_NOT_INTEGER, L_HISAM_2BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                    /* VM Key = vid + mac */
                    {L_HISAM_2BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                    /* IVM Key = h_port + vid + mac */
                    {L_HISAM_2BYTE_INTEGER, L_HISAM_2BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                };

/* The description of Hisam table
 */
static L_HISAM_Desc_T   amtr_om_hisam_desc;

/* When AMTR delete by group on Hisam table,
 * AMTR will count the number of deletions.
 * It can avoid to spend too much time on hisam sync.
 */
static UI32_T           amtr_om_hisam_delete_group_counter;

static UI32_T amtr_om_sem_id;
static UI32_T original_priority;
/*Add By Tony.Lei*/
/*The define should be the num that a message can contain. but the code is not very good , just define a static num. if u change the ipc , u should check the num */
#define AMTR_OM_NA_BUF_NUM  80
#define AMTR_OM_NA_ENTRY_NUM  10

static UI32_T amtr_om_na_sem = 0;

struct amtr_om_na_strut{
    UI32_T  counter;
    AMTR_TYPE_AddrEntry_T amtr_om_na_buf[AMTR_OM_NA_BUF_NUM];
};
struct amtr_om_na_truct_roll{
    int get_index;
    int set_index;
    struct amtr_om_na_strut amtr_om_na_entry[AMTR_OM_NA_ENTRY_NUM];
};
struct amtr_om_na_truct_roll amtr_om_na_roll;

/*end by Tony.Lei*/

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
static AMTR_TYPE_EventEntry_T   *edit_entry_buf_p = NULL;
static UI32_T                   edit_entry_max_count;
#endif
/* shared memory variables
 */
static AMTR_MGR_Shmem_Data_T *amtr_om_shmem_data_p;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Transition mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function performs AMTR enter Transition mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Master mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterMasterMode(void)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    AMTR_OM_SetMacNotifyDefaultValue();
#endif
#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
    AMTR_OM_SetVlanLearningDefaultValue();
#endif
#if (SYS_CPNT_MLAG == TRUE)
    AMTR_OM_SetMlagMacNotifyDefaultValue();
#endif

    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Slave mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_Init
 * -------------------------------------------------------------------------
 * PURPOSE:  This function create and initialize AMTR_OM(Hisam table).
 * INPUT  :  none.
 * OUTPUT :  none.
 * RETURN :  none
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_Init(void)
{
    amtr_om_hisam_desc.hash_depth         = HASH_DEPTH;
    amtr_om_hisam_desc.N1                 = HISAM_N1;
    amtr_om_hisam_desc.N2                 = HISAM_N2;
    amtr_om_hisam_desc.record_length      = NODE_LEN;
    amtr_om_hisam_desc.total_hash_nbr     = HASH_NBR;
    amtr_om_hisam_desc.total_index_nbr    = INDEX_NBR;
    amtr_om_hisam_desc.total_record_nbr   = NODE_NBR;

    if (!L_HISAM_CreateV2(&amtr_om_hisam_desc, NUMBER_OF_KEYS /* number_of _key */, key_def_table, key_type_table))
    {
        printf(" AMTR_OM: Create HISAM Error !\n");
        while(1);
    }
    /*add by Tony.lei*/
    memset(&amtr_om_na_roll,0,sizeof(amtr_om_na_roll));

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTR_NA_OM, &amtr_om_na_sem)
        != SYSFUN_OK){
        printf("%s: get om sem id fail.\n", __FUNCTION__);
    }

    amtr_om_hisam_delete_group_counter = 0;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTR_OM, &amtr_om_sem_id)
            != SYSFUN_OK)
    {
        printf("%s: get om sem id fail.\n", __FUNCTION__);
    }

    amtr_om_shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_AMTR_MGR_SHMEM_SEGID);

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    AMTR_OM_SetMacNotifyDefaultValue();
#endif
#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
    AMTR_OM_SetVlanLearningDefaultValue();
#endif
#if (SYS_CPNT_MLAG == TRUE)
    AMTR_OM_SetMlagMacNotifyDefaultValue();
#endif
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetPortInfo
 *------------------------------------------------------------------------------
 * PURPOSE  : This function get the port Infomation
 * INPUT    : ifindex
 * OUTPUT   : port_info(learning mode, life time, count, protocol)
 * RETURN   : BOOL_T        - True : successs, False : failed
 * NOTE     : This function will reference SWCTRL OM so it cannot be
 *            shared library function until SWCTRL OM is changed to shared memory.
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_OM_GetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;

    SWCTRL_Lport_Type_T     port_type;

    if(!port_info)
        return FALSE;

    /* Validate the given parameter
     */
    if (IS_IFINDEX_INVALID(ifindex))
        return FALSE;

    /* SWCTRL_LogicalPortToUserPort() will read OM in SWCTRL
     * There is no problem in this function to call SWCTRL_LogicalPortToUserPort()
     * because they are in the same process(i.e. l2_l4_proc)
     */
    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT) && (port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        return FALSE;

    port_info ->learn_with_count = amtr_om_shmem_data_p->amtr_port_info[ifindex-1].learn_with_count;
    port_info ->life_time        = amtr_om_shmem_data_p->amtr_port_info[ifindex-1].life_time;
    port_info ->protocol         = amtr_om_shmem_data_p->amtr_port_info[ifindex-1].protocol;
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_OM_GetOperatingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return AMTR csc operating mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : SYS_TYPE_STACKING_MASTER_MODE
 *           SYS_TYPE_STACKING_SLAVE_MODE
 *           SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamSetRecord
 * -------------------------------------------------------------------------
 * PURPOSE: This function will set a record to Hisam Table
 * INPUT  : AMTR_TYPE_AddrEntry_T addr_entry -- address entry
 * OUTPUT : none.
 * RETURN : none
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamSetRecord(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    BOOL_T return_value;
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);

    AMTR_OM_ENTER_CRITICAL_SECTION();

    /* Need to decrease counter if the entry already exists
     */
    AMTR_OM_UpdateCountForRemoveAddr(mv_key);

    /* When AMTR insert entry to Hisam Table, dynamic entry can't replace static entry before.
     * But, AMTR is SW learning now. Hisam table is for UI display. It's not OM.
     * When entry is synchronized from OM, we don't need to determine replacement rule.
     * Just replace. So, AMTR_OM_HisamSetRecord(,,is_statice) always is TRUE
     */
    return_value = L_HISAM_SetRecord (&amtr_om_hisam_desc, (UI8_T *)addr_entry, TRUE);

    if ((return_value == L_HISAM_INSERT) || (return_value == L_HISAM_REPLACE))
    {
        AMTRDRV_LIB_UpdateCount(addr_entry, TRUE, &(amtr_om_shmem_data_p->counters));
    }

    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamGetRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will get next record from Hisam Table
 * INPUT  :  UI8_T *key                             -- search key
 * OUTPUT :  AMTR_TYPE_AddrEntry_T *hisam_entry     -- address entry
 * RETURN :  TRUE/FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_OM_HisamGetNextRecord(UI32_T kidx, UI8_T *key,  AMTR_TYPE_AddrEntry_T *hisam_entry)
{
    BOOL_T return_value;
    AMTR_OM_ENTER_CRITICAL_SECTION();
    return_value = L_HISAM_GetNextRecord(&amtr_om_hisam_desc, kidx, key, (UI8_T *)hisam_entry);
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return return_value;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamSearch
 * -------------------------------------------------------------------------
 * PURPOSE: Search next element(record) from HISAM-structure
 * INPUT  : desc     -- HISAM descriptor to operation
 *          kidx     -- key index
 *          key      -- according the "key" to search the next element.
 *                      if element is NULL(0) then search from 1st element.
 *          call_back-- The callback function to 'view' the element.
 *                      The return value of call_back:
 *                      L_HISAM_SEARCH_BREAK: tell this function to break searching
 *                      L_HISAM_SEARCH_CONTINUE: tell this function continue searching
 *                      L_HISAM_SEARCH_SKIP: tell this function to skip counting
 *          count    -- limited element count to be searched
 * OUTPUT : key      -- return the key back so that the user can do furthur search from the next record
 * RETURN : L_HISAM_SEARCH_BREAK: Searching is broken by (*fun)
 *          L_HISAM_SEARCH_END_OF_LIST: stop search because end of list
 *          L_HISAM_SEARCH_COMPLETED: stop search because reach the limited count
 *          L_HISAM_SEARCH_INVALID_KIDX: stop search because invalid key index
 * NOTE   : 1. If caller assign count=0, this function will search all hisam table.
 *          2. If callbaack function don't need cookies, caller can set this argument to be 0.
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_OM_HisamSearch(UI32_T kidx, UI8_T *key, UI32_T (*call_back) (void*rec, void *cookie),UI32_T count, void *cookie)
{
    UI32_T return_value;
    AMTR_OM_ENTER_CRITICAL_SECTION();
    return_value = L_HISAM_Search_WithCookie (&amtr_om_hisam_desc, kidx, key, call_back, count, cookie);
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return return_value;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecord
 * -------------------------------------------------------------------------
 * PURPOSE: This function will delete one record from Hisam Table
 * INPUT  : UI8_T *key     -- search key
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecord(UI32_T vid, UI8_T *mac)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    AMTR_OM_ENTER_CRITICAL_SECTION();
    AMTR_OM_SetMVKey(mv_key, vid, mac);
    if (AMTR_OM_UpdateCountForRemoveAddr(mv_key) == TRUE)
    {
        L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
    }

    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete all record from Hisam Table
 * INPUT  :  None
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :  Hisam TAble re-initial
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteAllRecord(void)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    L_HISAM_DeleteAllRecord(&amtr_om_hisam_desc);
    memset(&(amtr_om_shmem_data_p->counters), 0, sizeof(amtr_om_shmem_data_p->counters));
    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by life time from Hisam Table
 * INPUT  :  AMTR_TYPE_AddressLifeTime_T life_time    -- condition
 * OUTPUT :  UI32_T *action_counter                   -- number of deletion
 * RETURN :  None
 * NOTES  :  1. action_counter: This API count how many records are deleted from Hisam TAble.
 *           2. AMTR Task will count sync number by action_counter.
 *              This job (sync Hash to Hisam) can't be spent too much time.
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];
    /* (vid,mac)==(0,0)
     */
    AMTR_OM_SetMVKey(mv_key, 0, (UI8_T *)amtr_om_null_mac);
    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTR access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_MV_KIDX, mv_key, AMTR_OM_HisamDeleteByLifeTimeCallBack, 0, (void *)(uintptr_t)life_time);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordBySource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by source from Hisam Table
 * INPUT  :  AMTR_TYPE_AddressSource_T source    -- condition
 * OUTPUT :  UI32_T *action_counter              -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordBySource(AMTR_TYPE_AddressSource_T source,UI32_T *action_counter)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    AMTR_OM_SetMVKey(mv_key, 0, (UI8_T *)amtr_om_null_mac);

    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTR access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_MV_KIDX, mv_key, AMTR_OM_HisamDeleteBySourceCallBack, 0, (void *)(uintptr_t)source);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPort
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port from Hisam Table
 * INPUT  :  UI32_T ifindex           -- condition
 * OUTPUT :  UI32_T *action_counter   -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPort(UI32_T ifindex,UI32_T *action_counter)
{
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    AMTR_OM_SetIVMKey(ivm_key, 0, (UI8_T *)amtr_om_null_mac, ifindex);

    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTR access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortCallBack, 0,(void *)(uintptr_t)ifindex);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+life_time from Hisam Table
 * INPUT  :  UI32_T ifindex                            -- condition1
 *           AMTR_TYPE_AddressLifeTime_T life_time     -- condition2
 * OUTPUT :  UI32_T *action_counter                    -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndLifeTime(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];
    cookie.ifindex=ifindex;
    cookie.attribute=life_time;
    AMTR_OM_SetIVMKey(ivm_key, 0, (UI8_T *)amtr_om_null_mac, ifindex);
    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTR access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNLifeTimeCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+Source from Hisam Table
 * INPUT  :  UI32_T ifindex                          -- condition1
 *           AMTR_TYPE_AddressSource_T source        -- condition2
 * OUTPUT :  UI32_T *action_counter                  -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndSource(UI32_T ifindex,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.attribute=source;
    AMTR_OM_SetIVMKey(ivm_key, 0, (UI8_T *)amtr_om_null_mac, ifindex);

    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNSourceCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid from Hisam Table
 * INPUT  :  UI32_T vid               -- condition
 * OUTPUT :  UI32_T *action_counter   -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVid(UI32_T vid,UI32_T *action_counter)
{
    UI8_T  vm_key[AMTR_VMKEY_LEN];

    AMTR_OM_SetVMKey(vm_key, vid, (UI8_T *)amtr_om_null_mac);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_VM_KIDX, vm_key, AMTR_OM_HisamDeleteByVidCallBack,0, (void *)(uintptr_t)vid);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVidAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid+life_time from Hisam Table
 * INPUT  :  UI32_T vid                                -- condition1
 *           AMTR_TYPE_AddressLifeTime_T life_time     -- condition2
 * OUTPUT :  UI32_T *action_counter                    -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVidAndLifeTime(UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  vm_key[AMTR_VMKEY_LEN];

    cookie.vid=vid;
    cookie.attribute=life_time;
    AMTR_OM_SetVMKey(vm_key, vid, (UI8_T *)amtr_om_null_mac);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_VM_KIDX, vm_key, AMTR_OM_HisamDeleteByVidNLifeTimeCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVidAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid+Source from Hisam Table
 * INPUT  :  UI32_T vid                              -- condition1
 *           AMTR_TYPE_AddressSource_T source        -- condition2
 * OUTPUT :  UI32_T *action_counter                  -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVidAndSource(UI32_T vid,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  vm_key[AMTR_VMKEY_LEN];

    cookie.vid=vid;
    cookie.attribute=source;
    AMTR_OM_SetVMKey(vm_key, vid, (UI8_T *)amtr_om_null_mac);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_VM_KIDX, vm_key, AMTR_OM_HisamDeleteByVidNSourceCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVid
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid from Hisam Table
 * INPUT  :  UI32_T port                 -- condition1
 *           UI32_T vid                  -- condition2
 * OUTPUT :  UI32_T *action_counter      -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVid(UI32_T ifindex,UI32_T vid, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.vid=vid;
    AMTR_OM_SetIVMKey(ivm_key, vid, (UI8_T *)amtr_om_null_mac, ifindex);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNVidCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+life_time from Hisam Table
 * INPUT  :  UI32_T port                            -- condition1
 *           UI32_T vid                             -- condition2
 *           AMTR_TYPE_AddressLifeTime_T life_time  -- condition3
 * OUTPUT :  UI32_T *action_counter                 -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(UI32_T ifindex,UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.vid=vid;
    cookie.attribute=life_time;
    AMTR_OM_SetIVMKey(ivm_key, vid, (UI8_T *)amtr_om_null_mac, ifindex);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNVidNLifeTimeCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVlanlistNLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+life_time from Hisam Table
 * INPUT  :  UI32_T port                            -- condition1
 *           UI32_T vid                             -- condition2
 *           AMTR_TYPE_AddressLifeTime_T life_time  -- condition3
 * OUTPUT :  UI32_T *action_counter                 -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidlistAndLifeTime(UI32_T ifindex,UI16_T *vlan_p,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex = ifindex;
    cookie.vlan_p = vlan_p;
    cookie.attribute = life_time;
    AMTR_OM_SetIVMKey(ivm_key, 0, (UI8_T *)amtr_om_null_mac, ifindex);

    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNVlanlistNLifeTimeCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+source from Hisam Table
 * INPUT  :  UI32_T port                          -- condition1
 *           UI32_T vid                           -- condition2
 *           AMTR_TYPE_AddressSource_T source     -- condition3
 * OUTPUT :  UI32_T *action_counter               -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource(UI32_T ifindex,UI32_T vid,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.vid=vid;
    cookie.attribute=source;
    AMTR_OM_SetIVMKey(ivm_key, vid, (UI8_T *)amtr_om_null_mac, ifindex);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNVidNSourceCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidExceptCertainAddr
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid from Hisam Table
 * INPUT  :  UI32_T port                 -- condition1
 *           UI32_T vid                  -- condition2
 *           UI32_T number_of_entry_in_list
 *           UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *           UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 * OUTPUT :  UI32_T *action_counter      -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidExceptCertainAddr(UI32_T ifindex,
                                                             UI32_T vid,
                                                             UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                             UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                             UI32_T number_of_entry_in_list,
                                                             UI32_T *action_counter)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.vid=vid;
    cookie.number_of_entry_in_list = number_of_entry_in_list;
    cookie.mac_list_p = mac_list_p;
    cookie.mask_list_p = mask_list_p;
    AMTR_OM_SetIVMKey(ivm_key, vid, (UI8_T *)amtr_om_null_mac, ifindex);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamDeleteByPortNVidExceptCertainAddrCallBack, 0, &cookie);
    *action_counter=amtr_om_hisam_delete_group_counter;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamUpdateRecordLifeTimeByLPort
 * -------------------------------------------------------------------------
 * PURPOSE: This function will update record's life time by port in Hisam Table
 * INPUT  : UI32_T ifindex
 *          AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT : None
 * RETURN : None
 * NOTES  : Only learnt entry will be updated.
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamUpdateRecordLifeTimeByLPort(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time)
{
    AMTR_OM_Cookie_T cookie;
    UI8_T  ivm_key[AMTR_IVMKEY_LEN];

    cookie.ifindex=ifindex;
    cookie.attribute=life_time;
    AMTR_OM_SetIVMKey(ivm_key, 0, (UI8_T *)amtr_om_null_mac, ifindex);
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_hisam_delete_group_counter = 0;
    L_HISAM_Search_WithCookie(&amtr_om_hisam_desc, AMTR_IVM_KIDX, ivm_key, AMTR_OM_HisamUpdateRecordLifeTimeByLPortCallBack, 0, &cookie);
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMVKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T vid  - vlan id
 *          UI8_T  *mac - logical unit
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetMVKey (UI8_T key[AMTR_MVKEY_LEN], UI16_T vid, UI8_T *mac)
{
    memcpy(key, mac, AMTR_TYPE_MAC_LEN);
    memcpy(key+AMTR_TYPE_MAC_LEN, &vid, AMTR_VID_LEN);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetVMKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T vid  - vlan id
 *          UI8_T  *mac - logical unit
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetVMKey (UI8_T key[AMTR_VMKEY_LEN], UI16_T vid, UI8_T *mac)
{
    memcpy(key, &vid, AMTR_VID_LEN);
    memcpy(key+AMTR_VID_LEN, mac, AMTR_TYPE_MAC_LEN);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetIVMKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T  vid     - vlan id
 *          UI8_T   *mac    - mac address
 *          UI8_T   port    - u_port number
 *          UI8_T   trunk_id- trunk id
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetIVMKey(UI8_T key[AMTR_IVMKEY_LEN], UI16_T vid, UI8_T *mac, UI16_T ifindex)
{
    memcpy(key, &ifindex, AMTR_IFINDEX_LEN);
    memcpy(key+AMTR_IFINDEX_LEN, &vid, AMTR_VID_LEN);
    memcpy(key+AMTR_IFINDEX_LEN+AMTR_VID_LEN, mac, AMTR_TYPE_MAC_LEN);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AMTR_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTR OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    AMTR_OM_IpcMsg_T *msg_data_p;

    if(msgbuf_p==NULL)
    {
        printf("\r\n %s msgbuf_p==NULL", __FUNCTION__);
        return FALSE;
    }

    msg_data_p= (AMTR_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Get the current operation mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(amtr_om_shmem_data_p) == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msgbuf_p->msg_size=AMTR_OM_IPCMSG_TYPE_SIZE;
        msg_data_p->type.ret_bool=FALSE;
        return TRUE;
    }

    switch(msg_data_p->type.cmd)
    {
        case AMTR_OM_IPC_GETPORTINFO:
            msgbuf_p->msg_size=AMTR_OM_GET_MSG_SIZE(arg_grp_ui32_portinfo);
            msg_data_p->type.ret_bool=AMTR_OM_GetPortInfo(msg_data_p->data.arg_grp_ui32_portinfo.arg_ui32,
                                                          &(msg_data_p->data.arg_grp_ui32_portinfo.arg_portinfo));
            break;

        default:
            msgbuf_p->msg_size=AMTR_OM_IPCMSG_TYPE_SIZE;
            msg_data_p->type.ret_bool=FALSE;
            printf("\r\n%s: Invalid cmd(%lu)", __FUNCTION__, msg_data_p->type.cmd);
            break;
    }
    return TRUE;
}

#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To set the MLAG mac notify port status
 * INPUT    : ifidx
 *            is_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    BOOL_T  ret = FALSE;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    if ((0 < ifidx) && (ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        if (TRUE == is_enabled)
        {
            AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, ifidx);
        }
        else
        {
            AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, ifidx);
        }

        ret = TRUE;
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MlagMacNotifyAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx   - lport ifindex
 *            vid     - vlan id
 *            src_mac - source mac
 *            is_add  - add/remove operation
 *            need_sem - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_MlagMacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T                  new_idx;
    BOOL_T                  ret = FALSE;

    /* debug
       printf("\r\n %s-%d ifidx:%ld, vid:%d, is_add:%s",
             __FUNCTION__, __LINE__, ifidx, vid, (is_add==TRUE)?"TRUE":"FALSE");
     */
    if(need_sem)
        AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt < AMTR_MGR_MAC_NTFY_LST_MAX)
    {
        new_idx = (amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_head + amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt) % (AMTR_MGR_MAC_NTFY_LST_MAX);
        ++amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt;

        rec_p = &amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_rec_lst[new_idx];

        rec_p->ifidx    = ifidx;
        rec_p->act_vid  = (vid & 0xfff);

        if (TRUE == is_add)
            rec_p->act_vid  |= AMTR_MGR_MAC_NTFY_ACT_MASK;

        memcpy (rec_p->src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    }

    if(need_sem)
        AMTR_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MlagMacNotifyCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the MLAG mac notify entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_MlagMacNotifyCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt > 0)
    {
        *ntfy_cnt_p = amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt;
        *used_head_p = (amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_head) % AMTR_MGR_MAC_NTFY_LST_MAX;
        amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_cnt = 0;
        amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_used_head = (*used_head_p + *ntfy_cnt_p) % AMTR_MGR_MAC_NTFY_LST_MAX;
        memcpy(rec_p, &amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_rec_lst, sizeof(AMTR_MGR_MacNotifyRec_T)*AMTR_MGR_MAC_NTFY_LST_MAX);

    }

    amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_time_stamp = SYSFUN_GetSysTick();

    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return;
}
#endif

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE: This function will delete record from Hisam Table when condition matching.
 * INPUT  : void * record       -- hisam record(entry)
 *          UI32_T life_time    -- condition
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. delete this input record iff record->life_time == life_time.
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByLifeTimeCallBack(void * record, void *cookie)
{
    UI32_T life_time = (uintptr_t)cookie;
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
    AMTR_TYPE_AddrEntry_T  old_entry;
    BOOL_T need_sem = FALSE;
#endif
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T is_mlag_port_enabled = FALSE;
#endif
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    if(((AMTR_TYPE_AddrEntry_T *)record)->life_time==life_time)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
            if(FALSE == L_HISAM_DeleteRecordAndOutput(&amtr_om_hisam_desc, mv_key, (UI8_T *) &old_entry))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
            //BACKDOOR_MGR_Printf("%s(%d) delete mac of index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\n", __FUNCTION__, __LINE__, old_entry.ifindex, old_entry.vid, old_entry.mac[0], old_entry.mac[1], old_entry.mac[2], old_entry.mac[3], old_entry.mac[4], old_entry.mac[5]);

            /* For this function is call from AMTR_OM_HisamDeleteRecordByLifeTime(),
             * and is protect by sem already. Here we do not need to get sem again
             * for mac-notify.
             */
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
            is_mac_notify_enabled = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
            is_mac_notify_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;
            if((TRUE == is_mac_notify_enabled) && (TRUE == is_mac_notify_port_enabled) &&
               ((AMTR_TYPE_AddrEntry_T *)record)->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #if (SYS_CPNT_MLAG == TRUE)
            is_mlag_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;

            if((TRUE == is_mlag_port_enabled) &&
               ((AMTR_TYPE_AddrEntry_T *)record)->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MlagMacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #else
            if(FALSE == L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
        #endif
        }
        amtr_om_hisam_delete_group_counter++;
    }

    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteBySourceCallBack
 * -------------------------------------------------------------------------
 * PURPOSE: This function will delete record from Hisam Table when condition matching.
 * INPUT  : void * record              -- hisam record(entry)
 *          UI32_T source              -- condition
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. delete this input record iff record->source == source.
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteBySourceCallBack(void * record, void *cookie)
{
    UI32_T source = (uintptr_t)cookie;
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    if(((AMTR_TYPE_AddrEntry_T *)record)->source==source)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
            L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record            -- hisam record(entry)
 *           UI32_T ifindex           -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortCallBack(void * record, void *cookie)
{
    UI32_T ifindex = (uintptr_t)cookie;
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Port number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=ifindex)
    {
        return L_HISAM_SEARCH_BREAK;
    }

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    if (edit_entry_buf_p != NULL)
    {
        if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
            return L_HISAM_SEARCH_BREAK;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
        memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
    }
#endif

    AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
    if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
    {
        L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
    }
    amtr_om_hisam_delete_group_counter++;
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE: This function will delete record from Hisam Table when condition matching.
 * INPUT  : void * record        -- hisam record(entry)
 *          UI32_T cookie        -- condition
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNLifeTimeCallBack(void * record, void *cookie)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
    AMTR_TYPE_AddrEntry_T  old_entry;
    BOOL_T need_sem = FALSE;
#endif
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T is_mlag_port_enabled = FALSE;
#endif
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Port number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex)
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->life_time==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
            if(FALSE == L_HISAM_DeleteRecordAndOutput(&amtr_om_hisam_desc, mv_key, (UI8_T *) &old_entry))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
            //BACKDOOR_MGR_Printf("%s(%d) delete mac of index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\n", __FUNCTION__, __LINE__, old_entry.ifindex, old_entry.vid, old_entry.mac[0], old_entry.mac[1], old_entry.mac[2], old_entry.mac[3], old_entry.mac[4], old_entry.mac[5]);

            /* For this function is call from AMTR_OM_HisamDeleteRecordByLPortAndLifeTime(),
             * and is protect by sem already. Here we do not need to get sem again
             * for mac-notify.
             */
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
            is_mac_notify_enabled = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
            is_mac_notify_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;

            if((TRUE == is_mac_notify_enabled) && (TRUE == is_mac_notify_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #if (SYS_CPNT_MLAG == TRUE)
            is_mlag_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;

            if((TRUE == is_mlag_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MlagMacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #else
            if(FALSE == L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
        #endif
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNSourceCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record         -- hisam record(entry)
 *           UI32_T cookie         -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNSourceCallBack(void * record, void *cookie)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    /* Port number validation */
    if (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex)
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->source==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
            L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByVidCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record              -- hisam record(entry)
 *           UI32_T vid                 -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByVidCallBack(void * record, void *cookie)
{
    UI32_T vid = (uintptr_t)cookie;
    UI8_T  mv_key[AMTR_MVKEY_LEN];


    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    /* Vlan number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->vid!=vid)
    {
        return L_HISAM_SEARCH_BREAK;
    }

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    if (edit_entry_buf_p != NULL)
    {
        if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
            return L_HISAM_SEARCH_BREAK;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
        memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
    }
#endif

    AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
    if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
    {
        L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
    }
    amtr_om_hisam_delete_group_counter++;
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByVidNLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record          -- hisam record(entry)
 *           UI32_T cookie          -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByVidNLifeTimeCallBack(void * record, void *cookie)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
    AMTR_TYPE_AddrEntry_T  old_entry;
    BOOL_T need_sem = FALSE;
#endif
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T is_mlag_port_enabled = FALSE;
#endif
    UI8_T  mv_key[AMTR_MVKEY_LEN];


    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    /* Vlan number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->life_time==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
            if(FALSE == L_HISAM_DeleteRecordAndOutput(&amtr_om_hisam_desc, mv_key, (UI8_T *) &old_entry))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
            //BACKDOOR_MGR_Printf("%s(%d) delete mac of index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\n", __FUNCTION__, __LINE__, old_entry.ifindex, old_entry.vid, old_entry.mac[0], old_entry.mac[1], old_entry.mac[2], old_entry.mac[3], old_entry.mac[4], old_entry.mac[5]);

            /* For this function is call from AMTR_OM_HisamDeleteRecordByVidAndLifeTime(),
             * and is protect by sem already. Here we do not need to get sem again
             * for mac-notify.
             */
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
            is_mac_notify_enabled = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
            is_mac_notify_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;
            if((TRUE == is_mac_notify_enabled) && (TRUE == is_mac_notify_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #if (SYS_CPNT_MLAG == TRUE)
            is_mlag_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;

            if((TRUE == is_mlag_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MlagMacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #else
            if(FALSE == L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
        #endif
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByVidNSourceCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record         -- hisam record(entry)
 *           UI32_T cookie         -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByVidNSourceCallBack(void * record, void *cookie)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];


    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Vlan number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->source==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
            L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVidCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record           -- hisam record(entry)
 *           UI32_T cookie           -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNVidCallBack(void * record, void *cookie)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];


    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Port and Vlan number validation
     */
    if ((((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)||
        (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex))
    {
        return L_HISAM_SEARCH_BREAK;
    }

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    if (edit_entry_buf_p != NULL)
    {
        if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
            return L_HISAM_SEARCH_BREAK;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
        memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
    }
#endif

    AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
    if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
    {
        L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
    }
    amtr_om_hisam_delete_group_counter++;
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVidNLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record             -- hisam record(entry)
 *           UI32_T cookie             -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNVidNLifeTimeCallBack(void * record, void *cookie)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
    AMTR_TYPE_AddrEntry_T  old_entry;
    BOOL_T need_sem = FALSE;
#endif
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T is_mlag_port_enabled = FALSE;
#endif
    UI8_T  mv_key[AMTR_MVKEY_LEN];


    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    /* Port and Vlan number validation
     */
    if ((((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)||
        (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex))
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->life_time==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
            if(FALSE == L_HISAM_DeleteRecordAndOutput(&amtr_om_hisam_desc, mv_key, (UI8_T *) &old_entry))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
            //BACKDOOR_MGR_Printf("%s(%d) delete mac of index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\n", __FUNCTION__, __LINE__, old_entry.ifindex, old_entry.vid, old_entry.mac[0], old_entry.mac[1], old_entry.mac[2], old_entry.mac[3], old_entry.mac[4], old_entry.mac[5]);

            /* For this function is call from AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(),
             * and is protect by sem already. Here we do not need to get sem again
             * for mac-notify.
             */
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
            is_mac_notify_enabled = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
            is_mac_notify_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;
            if((TRUE == is_mac_notify_enabled) && (TRUE == is_mac_notify_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #if (SYS_CPNT_MLAG == TRUE)
            is_mlag_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, old_entry.ifindex)) ? TRUE : FALSE;

            if((TRUE == is_mlag_port_enabled) &&
               ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                AMTR_OM_MlagMacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
            }
        #endif
        #else
            if(FALSE == L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key))
            {
                return L_HISAM_SEARCH_CONTINUE;
            }
        #endif
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVlanlistNLifeTimeCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record             -- hisam record(entry)
 *           UI32_T cookie             -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNVlanlistNLifeTimeCallBack(void * record, void *cookie)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
    AMTR_TYPE_AddrEntry_T  old_entry;
    BOOL_T need_sem = FALSE;
#endif
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T is_mlag_port_enabled = FALSE;
#endif
    UI8_T   mv_key[AMTR_MVKEY_LEN];
    UI32_T  index=0;

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    is_mac_notify_enabled = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
    is_mac_notify_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, ((AMTR_TYPE_AddrEntry_T *)record)->ifindex)) ? TRUE : FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    is_mlag_port_enabled = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, ((AMTR_TYPE_AddrEntry_T *)record)->ifindex)) ? TRUE : FALSE;
#endif

    /* Port and Vlan number validation
     */
    while(((AMTR_OM_Cookie_T *)cookie)->vlan_p[index] != 0)
    {
        if (    (((AMTR_TYPE_AddrEntry_T *)record)->vid == ((AMTR_OM_Cookie_T *)cookie)->vlan_p[index])
             && (((AMTR_TYPE_AddrEntry_T *)record)->ifindex == ((AMTR_OM_Cookie_T *)cookie)->ifindex)
             && (((AMTR_TYPE_AddrEntry_T *)record)->life_time== ((AMTR_OM_Cookie_T *)cookie)->attribute)
           )
        {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
            if (edit_entry_buf_p != NULL)
            {
                if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                    return L_HISAM_SEARCH_BREAK;
                edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
                edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
                memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
                edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
            }
#endif
            AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
            if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
            {
            #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) || (SYS_CPNT_MLAG == TRUE)
                if(FALSE == L_HISAM_DeleteRecordAndOutput(&amtr_om_hisam_desc, mv_key, (UI8_T *) &old_entry))
                {
                    return L_HISAM_SEARCH_CONTINUE;
                }
                //BACKDOOR_MGR_Printf("%s(%d) delete mac of index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\n", __FUNCTION__, __LINE__, old_entry.ifindex, old_entry.vid, old_entry.mac[0], old_entry.mac[1], old_entry.mac[2], old_entry.mac[3], old_entry.mac[4], old_entry.mac[5]);

                /* For this function is call from AMTR_OM_HisamDeleteRecordByLPortAndVidlistAndLifeTime(),
                 * and is protect by sem already. Here we do not need to get sem again
                 * for mac-notify.
                 */
            #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
                if((TRUE == is_mac_notify_enabled && TRUE == is_mac_notify_port_enabled) &&
                    ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
                {
                    AMTR_OM_MacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
                }
            #endif
            #if (SYS_CPNT_MLAG == TRUE)
                if((TRUE == is_mlag_port_enabled) &&
                    ((AMTR_OM_Cookie_T *)cookie)->attribute == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
                {
                    AMTR_OM_MlagMacNotifyAddNewEntry(old_entry.ifindex, old_entry.vid, old_entry.mac, FALSE, need_sem);
                }
            #endif
            #else
                if(FALSE == L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key))
                {
                    return L_HISAM_SEARCH_CONTINUE;
                }
            #endif
            }
            amtr_om_hisam_delete_group_counter++;
            return L_HISAM_SEARCH_CONTINUE;
        }
        index++;
    }

    return L_HISAM_SEARCH_CONTINUE;
}
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVidNSourceCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record             -- hisam record(entry)
 *           UI32_T cookie             -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNVidNSourceCallBack(void * record, void *cookie)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }

    /* Port and Vlan number validation
     */
    if ((((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)||
        (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex))
    {
        return L_HISAM_SEARCH_BREAK;
    }

    if (((AMTR_TYPE_AddrEntry_T *)record)->source==((AMTR_OM_Cookie_T *)cookie)->attribute)
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        if (edit_entry_buf_p != NULL)
        {
            if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
                return L_HISAM_SEARCH_BREAK;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
            edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
        }
#endif

        AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
        if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
        {
            L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
        }
        amtr_om_hisam_delete_group_counter++;
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteByPortNVidExceptCertainAddrCallBack
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete record from Hisam Table when condition matching.
 * INPUT  :  void * record           -- hisam record(entry)
 *           UI32_T cookie           -- condition
 * OUTPUT :  None
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamDeleteByPortNVidExceptCertainAddrCallBack(void * record, void *cookie)
{
    UI8_T  mv_key[AMTR_MVKEY_LEN];

    UI32_T      mac_mask_index;
    UI32_T      mac_index;
    UI32_T      number_of_entry_in_list = ((AMTR_OM_Cookie_T *)cookie)->number_of_entry_in_list;
    UI8_T       (*mac_list_p)[SYS_ADPT_MAC_ADDR_LEN] = ((AMTR_OM_Cookie_T *)cookie)->mac_list_p;
    UI8_T       (*mask_list_p)[SYS_ADPT_MAC_ADDR_LEN] = ((AMTR_OM_Cookie_T *)cookie)->mask_list_p;

    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Port and Vlan number validation
     */
    if ((((AMTR_TYPE_AddrEntry_T *)record)->vid!=((AMTR_OM_Cookie_T *)cookie)->vid)||
        (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex))
    {
        return L_HISAM_SEARCH_BREAK;
    }

    /* check record is matched with mac_list or not.
     */
    for (mac_mask_index = 0; mac_mask_index < number_of_entry_in_list; mac_mask_index++)
    {
        for (mac_index = 0; mac_index < SYS_ADPT_MAC_ADDR_LEN; mac_index++)
        {
            if ((((AMTR_TYPE_AddrEntry_T *)record)->mac[mac_index] & mask_list_p[mac_mask_index][mac_index]) !=
                (mac_list_p[mac_mask_index][mac_index] & mask_list_p[mac_mask_index][mac_index]))
            {
                break;
            }
        }
        if (mac_index >= SYS_ADPT_MAC_ADDR_LEN)
        {
            return L_HISAM_SEARCH_SKIP;
        }
    }

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    if (edit_entry_buf_p != NULL)
    {
        if (amtr_om_hisam_delete_group_counter >= edit_entry_max_count)
            return L_HISAM_SEARCH_BREAK;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].vid = ((AMTR_TYPE_AddrEntry_T *)record)->vid;
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].ifindex = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
        memcpy(edit_entry_buf_p[amtr_om_hisam_delete_group_counter].mac, ((AMTR_TYPE_AddrEntry_T *)record)->mac, SYS_ADPT_MAC_ADDR_LEN);
        edit_entry_buf_p[amtr_om_hisam_delete_group_counter].source = ((AMTR_TYPE_AddrEntry_T *)record)->source;
    }
#endif

    AMTR_OM_SetMVKey(mv_key, ((AMTR_TYPE_AddrEntry_T *)record)->vid, ((AMTR_TYPE_AddrEntry_T *)record)->mac);
    if (AMTR_OM_UpdateCountForRemoveAddr(mv_key)==TRUE)
    {
        L_HISAM_DeleteRecord(&amtr_om_hisam_desc, mv_key);
    }
    amtr_om_hisam_delete_group_counter++;
    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamUpdateRecordLifeTimeByLPortCallBack
 * -------------------------------------------------------------------------
 * PURPOSE: This function will update record's life time in Hisam Table
 * INPUT  : void * record            -- hisam record(entry)
 *          UI32_T cookie            -- condition
 * OUTPUT : None
 * RETURN : None
 * NOTES  : Only learnt entry will be updated.
 * -------------------------------------------------------------------------*/
static UI32_T AMTR_OM_HisamUpdateRecordLifeTimeByLPortCallBack(void * record, void *cookie)
{
    if (record == NULL)
    {
        return L_HISAM_SEARCH_SKIP;
    }
    /* Port number validation
     */
    if (((AMTR_TYPE_AddrEntry_T *)record)->ifindex!=((AMTR_OM_Cookie_T *)cookie)->ifindex)
    {
        return L_HISAM_SEARCH_BREAK;
    }
    if (((AMTR_TYPE_AddrEntry_T *)record)->source==AMTR_TYPE_ADDRESS_SOURCE_LEARN)
    {
        AMTR_TYPE_AddrEntry_T *addr_entry_p = (AMTR_TYPE_AddrEntry_T*)record;

        /* LifeTime is going to be changed, update counters by removing the addr
         */
        AMTRDRV_LIB_UpdateCount(addr_entry_p, FALSE, &(amtr_om_shmem_data_p->counters));
        addr_entry_p->life_time=((AMTR_OM_Cookie_T *)cookie)->attribute;
        AMTRDRV_LIB_UpdateCount(addr_entry_p, TRUE,  &(amtr_om_shmem_data_p->counters));
    }
    return L_HISAM_SEARCH_CONTINUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_UpdateCountForRemoveAddr
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will update AMTR OM Mac counter by the given
 *          mv_key which will be removed from OM.
 * INPUT  : mv_key - the key of the Mac addr entry that will be removed from
 *                   OM
 * OUTPUT : None
 * RETURN : TRUE  - The given mv_key is able to get a mac entry from the hisam
 *                  table and the corresponding counters are decreased by one.
 *          FALSE - Unable to get a mac entry from by the given mv_key. No
 *                  counters are updated.
 * NOTE   :
 *   1. In this function, it will read the hisam table and update counters in OM
 *      accordingly. So caller need to do critical section protection.
 *   2. In this function, it will first check whether the given mac entry exists
 *      in the hisam table, and decrease the counters in OM if the entry exists.
 *-------------------------------------------------------------------------*/
static BOOL_T AMTR_OM_UpdateCountForRemoveAddr(UI8_T mv_key[AMTR_MVKEY_LEN])
{
    AMTR_TYPE_AddrEntry_T local_addr_entry;

    if (L_HISAM_GetRecord(&amtr_om_hisam_desc, AMTR_MV_KIDX, mv_key,
        (UI8_T*)&local_addr_entry)==TRUE)
    {
        AMTRDRV_LIB_UpdateCount(&local_addr_entry, FALSE, &(amtr_om_shmem_data_p->counters));
        return TRUE;
    }

    return FALSE;
}

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyDefaultValue
 *------------------------------------------------------------------------------
 * PURPOSE  : To set mac-notification-trap related OM to default value
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : No need to add semaphore for it's called in AMTR_MGR_EnterMasterMode
 *-----------------------------------------------------------------------------
 */
static void AMTR_OM_SetMacNotifyDefaultValue(void)
{
    amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_interval = SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL * SYS_BLD_TICKS_PER_SECOND;

    if (TRUE == SYS_DFLT_AMTR_MAC_NOTIFY_PORT_STATUS)
        memset(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, 0xff, sizeof (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports));
    else
        memset(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, 0x0, sizeof (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports));
    amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global = SYS_DFLT_AMTR_MAC_NOTIFY_GLOBAL_STATUS;
}
#endif

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetVlanLearningDefaultValue
 *------------------------------------------------------------------------------
 * PURPOSE  : To set vlan learning related OM to default value
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : No need to add semaphore for it's called in AMTR_MGR_EnterMasterMode
 *-----------------------------------------------------------------------------
 */
static void AMTR_OM_SetVlanLearningDefaultValue(void)
{
    if (FALSE == SYS_DFLT_AMTR_VLAN_MAC_LEARNING)
        memset(amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis, 0xff, sizeof (amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis));
    else
        memset(amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis, 0x0, sizeof (amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis));
}
#endif

#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMlagMacNotifyDefaultValue
 *------------------------------------------------------------------------------
 * PURPOSE  : To set MLAG mac notify related OM to default value
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
static void AMTR_OM_SetMlagMacNotifyDefaultValue(void)
{
    memset(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, 0, sizeof (amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports));
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferReset
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will reset na buffer
 * INPUT  : None
 * OUTPUT : None
 *-------------------------------------------------------------------------*/
void AMTR_OM_NABufferReset()
{
    memset(&amtr_om_na_roll,0,sizeof(amtr_om_na_roll));
}

/*  1.get_index and set_index will be inited to zero.
 *  if get_index == set_index, must be just in this case
 *  2. when enqeue , set_index ++, if set_index== max num goto zero
 *  if set_index+1 == get_index  , queue full
 *  3. when deqeue , get_index ++, if set_index== max num goto zero
 *  if get_index == set_index , queue empty
 *
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will set  NA entries to NA queue
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : TRUE / FALSE(queue empty)
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is empty, this function return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T AMTR_OM_NABufferEnqueue(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T     rc = FALSE;

    if(SYSFUN_OK == SYSFUN_TakeSem(amtr_om_na_sem,SYSFUN_TIMEOUT_WAIT_FOREVER)){
        if(amtr_om_na_roll.set_index == amtr_om_na_roll.get_index){
        /*this case just do when the beginning status */
            memcpy(&amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.set_index].amtr_om_na_buf,addr_buf,num_of_entries*sizeof(AMTR_TYPE_AddrEntry_T));
            amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.set_index].counter = num_of_entries;
            amtr_om_na_roll.set_index = (amtr_om_na_roll.set_index+1)%AMTR_OM_NA_ENTRY_NUM;
            rc = TRUE;

        } else if((amtr_om_na_roll.set_index+1)%AMTR_OM_NA_ENTRY_NUM != amtr_om_na_roll.get_index){

          memcpy(&amtr_om_na_roll.amtr_om_na_entry[(amtr_om_na_roll.set_index+1)%AMTR_OM_NA_ENTRY_NUM].amtr_om_na_buf,addr_buf,num_of_entries*sizeof(AMTR_TYPE_AddrEntry_T));
          amtr_om_na_roll.amtr_om_na_entry[(amtr_om_na_roll.set_index+1)%AMTR_OM_NA_ENTRY_NUM].counter = num_of_entries;
          amtr_om_na_roll.set_index = (amtr_om_na_roll.set_index+1)%AMTR_OM_NA_ENTRY_NUM;
          rc = TRUE;

        }

      SYSFUN_GiveSem(amtr_om_na_sem);

    }
    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will get an NA entry from NA queue
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : TRUE / FALSE(queue empty)
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is empty, this function return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T AMTR_OM_NABufferDequeue(UI32_T* num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T     rc = FALSE;

    if(SYSFUN_OK == SYSFUN_TakeSem(amtr_om_na_sem,SYSFUN_TIMEOUT_WAIT_FOREVER)){
        if(amtr_om_na_roll.get_index != amtr_om_na_roll.set_index){

          memcpy(addr_buf,&amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.get_index].amtr_om_na_buf,amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.get_index].counter*sizeof(AMTR_TYPE_AddrEntry_T));
          *num_of_entries = amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.get_index].counter;
          amtr_om_na_roll.amtr_om_na_entry[amtr_om_na_roll.get_index].counter = 0;
          amtr_om_na_roll.get_index = (amtr_om_na_roll.get_index+1)%AMTR_OM_NA_ENTRY_NUM;
          rc = TRUE;
        }

        SYSFUN_GiveSem(amtr_om_na_sem);
    }
    return rc;

}

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_SetEditAddrEntryBuf
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will specify the buffer of event entry for
 *          tracking which entries were deleted
 * INPUT  : edit_entry_buf -- the buffer to store event entry
 *          max_count      -- the number of event entry in the buffer
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-------------------------------------------------------------------------*/
BOOL_T AMTR_OM_SetEditAddrEntryBuf(AMTR_TYPE_EventEntry_T *edit_entry_buf, UI32_T max_count)
{
    edit_entry_buf_p = edit_entry_buf;
    edit_entry_max_count = max_count;

    return TRUE;
}
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the OVSVTEP mac notify entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_OvsCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt > 0)
    {
        *ntfy_cnt_p = amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt;
        *used_head_p = (amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_head) % AMTR_MGR_MAC_NTFY_LST_MAX;
        amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt = 0;
        amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_head = (*used_head_p + *ntfy_cnt_p) % AMTR_MGR_MAC_NTFY_LST_MAX;
        memcpy(rec_p, &amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_rec_lst, sizeof(AMTR_MGR_MacNotifyRec_T)*AMTR_MGR_MAC_NTFY_LST_MAX);
    }

    amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_time_stamp = SYSFUN_GetSysTick();

    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx     - lport ifindex
 *            vid       - vlan id
 *            src_mac_p - source mac
 *            is_add    - add/remove operation
 *            need_sem  - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_OvsAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T                  new_idx;
    BOOL_T                  ret = FALSE;

    if(need_sem)
        AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt < AMTR_MGR_MAC_NTFY_LST_MAX)
    {
        new_idx = (amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_head + amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt) % (AMTR_MGR_MAC_NTFY_LST_MAX);
        ++amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_used_cnt;

        rec_p = &amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_rec_lst[new_idx];

        rec_p->ifidx    = ifidx;
        rec_p->act_vid  = (vid & 0x7fff);

        if (TRUE == is_add)
            rec_p->act_vid  |= AMTR_MGR_MAC_NTFY_ACT_MASK;

        memcpy (rec_p->src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    }

    if(need_sem)
        AMTR_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MacNotifyCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the mac-notification entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt > 0)
    {
        *ntfy_cnt_p = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt;
        *used_head_p = (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_head) % AMTR_MGR_MAC_NTFY_LST_MAX;
        amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt = 0;
        amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_head = (*used_head_p + *ntfy_cnt_p) % AMTR_MGR_MAC_NTFY_LST_MAX;
        memcpy(rec_p, &amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_rec_lst, sizeof(AMTR_MGR_MacNotifyRec_T)*AMTR_MGR_MAC_NTFY_LST_MAX);

    }

    amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_time_stamp = SYSFUN_GetSysTick();

    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return;
}
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MacNotifyAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx   - lport ifindex
 *            vid     - vlan id
 *            src_mac - source mac
 *            is_add  - add/remove operation
 *            need_sem - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_MacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T                  new_idx;
    BOOL_T                  ret = FALSE;

    /* debug
       printf("\r\n %s-%d ifidx:%ld, vid:%d, is_add:%s",
             __FUNCTION__, __LINE__, ifidx, vid, (is_add==TRUE)?"TRUE":"FALSE");
     */
    if(need_sem)
        AMTR_OM_ENTER_CRITICAL_SECTION();

    if (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt < AMTR_MGR_MAC_NTFY_LST_MAX)
    {
        new_idx = (amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_head + amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt) % (AMTR_MGR_MAC_NTFY_LST_MAX);
        ++amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt;

        rec_p = &amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_rec_lst[new_idx];

        rec_p->ifidx    = ifidx;
        rec_p->act_vid  = (vid & 0xfff);

        if (TRUE == is_add)
            rec_p->act_vid  |= AMTR_MGR_MAC_NTFY_ACT_MASK;

        memcpy (rec_p->src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    }

    if(need_sem)
        AMTR_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To set mac-notification-trap global status
 * INPUT    : is_enabled - global status to set
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyGlobalStatus(BOOL_T  is_enabled)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    if (is_enabled != amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global)
    {
        amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global = is_enabled;

        /* clear the record queue
         */
        amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_used_cnt = 0;
    }
    amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_time_stamp = SYSFUN_GetSysTick();
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyInterval
 *------------------------------------------------------------------------------
 * PURPOSE  : To set mac-notification-trap interval
 * INPUT    : interval - interval to set (in ticks)
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyInterval(UI32_T  interval)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_interval = interval;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To get mac-notification-trap port status
 * INPUT    : ifidx
 *            is_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    BOOL_T  ret = FALSE;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    if ((0 < ifidx) && (ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        if (TRUE == is_enabled)
        {
            AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, ifidx);
        }
        else
        {
            AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, ifidx);
        }

        ret = TRUE;
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyAddFstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trk_ifidx - specify which trunk to join.
 *          mbr_ifidx - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : mbr_ifidx is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyAddFstTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx)
{
    if(trk_ifidx <= 0 || trk_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT ||
       mbr_ifidx <= 0 || mbr_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT )
        return;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    /* 1. apply the 1st member's property to trunk
     */
    if (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, mbr_ifidx))
    {
        AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx);
    }
    else
    {
        AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx);
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyDelTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the trunk member is
 *          removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to remove from
 *          mbr_ifidx - specify which member port being removed from trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyDelTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx)
{
    if(trk_ifidx <= 0 || trk_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT ||
       mbr_ifidx <= 0 || mbr_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT )
        return;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    /* 1. apply the trunk's property to member
     */
    if (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx))
    {
        AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, mbr_ifidx);
    }
    else
    {
        AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, mbr_ifidx);
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyDelLstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyDelLstTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx)
{
    if(trk_ifidx <= 0 || trk_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT ||
       mbr_ifidx <= 0 || mbr_ifidx > SYS_ADPT_TOTAL_NBR_OF_LPORT )
        return;

    AMTR_OM_ENTER_CRITICAL_SECTION();

    /* 1. apply the trunk's property to member
     */
    if (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx))
    {
        AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, mbr_ifidx);
    }
    else
    {
        AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, mbr_ifidx);
    }

    /* 2. apply default property to trunk
     */
    if (SYS_DFLT_AMTR_MAC_NOTIFY_PORT_STATUS == FALSE)
        AMTR_MGR_MAC_NTFY_CLR_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx);
    else
        AMTR_MGR_MAC_NTFY_SET_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, trk_ifidx);
    AMTR_OM_LEAVE_CRITICAL_SECTION();
}

#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetVlanLearningStatus(UI32_T vid, BOOL_T learning)
{
    BOOL_T  ret = FALSE;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    if (0 < vid && vid <= SYS_ADPT_MAX_VLAN_ID)
    {
        if (TRUE == !learning)
        {
            AMTR_MGR_VLAN_LEARNING_SET_BIT(amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis, vid);
        }
        else
        {
            AMTR_MGR_VLAN_LEARNING_CLR_BIT(amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis, vid);
        }

        ret = TRUE;
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}
#endif

