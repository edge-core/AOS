/* Project Name:  netaccess_om.c
 * Purpose:
 *      The database of NetAccess
 *
 * Reason:
 *      Description:
 *      Creator:      Ricky Lin
 *      Date         2006/01/27
 *
 * Copyright(C)      Accton Corporation, 2002
 */
/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netaccess_om.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_hisam.h"
#include "l_hash.h"
#include "l_mm.h"
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
#include "l_sort_lst.h"
#endif
#include "swctrl.h"
#include "security_backdoor.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_OM_ADPT_MIN_SECURE_NBR_ADDR_IN_LEARN_MODES           0
#define NETACCESS_OM_ADPT_MIN_SECURE_NBR_ADDR_IN_SECURE_MODES          0
#define NETACCESS_OM_ADPT_MIN_SECURE_NBR_ADDR_IN_USER_LOGIN            0
#define NETACCESS_OM_ADPT_MIN_SECURE_NBR_ADDR_IN_SECURE_USER_LOGIN     1
#define NETACCESS_OM_ADPT_MIN_SECURE_NBR_ADDR_IN_RADA_MODES            1

#define NETACCESS_OM_MAX_NBR_OF_DUP_EAP_PER_PORT        2 /* to avoid too many EAP packets */

#define NETACCESS_OM_SECURE_UNAUTH_TIME                         5L /* seconds */
#define NETACCESS_OM_MAX_NBR_OF_UNAUTHORIZED_MAC_CACHE_ENTRY    1024
#define NETACCESS_OM_UNAUTHORIZED_MAC_CACHE_HASH_BUCKET_SIZE    512/* change to use linear hash */


/* per port reserved one MAC space for userLogin authorized MAC because these MACs
   named "hidden secure MACs" are invisible for users and can't write Learned-PSEC to cip (AMTR) */
#define NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE       (SYS_ADPT_NETACCESS_TOTAL_NBR_OF_SECURE_ADDRESSES_PER_SYSTEM + \
                                                        SYS_ADPT_TOTAL_NBR_OF_LPORT * NETACCESS_OM_MAX_HIDDEN_MAC_PER_PORT)

/* key index for HISAM table
 */
#define NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS         0 /* slot + port + mac */
#define NETACCESS_KEY_INDEX_OF_OLDEST_ADDRESS         1 /* slot + port + authorized_status + record_time */
#define NETACCESS_KEY_INDEX_OF_EXPIRE_ADDRESS         2 /* session_expire_time + slot + port + mac */
#define NETACCESS_KEY_INDEX_OF_MACADR_ADDRESS         3 /* mac + slot + port */

/* constant definitions for HISAM
 */
#define NETACCESS_HISAM_NODE_NBR        NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE

#define NETACCESS_HISAM_INDEX_NBR       200 /* table is divided to 100 blocks  */
#define NETACCESS_HISAM_N1              2   /* table balance threshold */
#define NETACCESS_HISAM_N2              40  /* table balance threshold */
#define NETACCESS_HASH_DEPTH            4
#define NETACCESS_HASH_NBR              2000
#define NETACCESS_NUMBER_OF_KEYS        4

/* MACRO FUNCTION DECLARATIONS
 */
#define NETACCESS_OM_IS_BAD_LPORT_NO(lport)     ((0 >= lport) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
#define NETACCESS_OM_IS_VALID_LPORT_NO(lport)   ((0 < lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))

#define NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index)    ((0 >= mac_index) || (mac_index > NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE))
#define NETACCESS_OM_IS_VALID_MAC_INDEX(mac_index)  ((0 < mac_index) && (mac_index <= NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE))

#define NETACCESS_OM_IS_BAD_ROW_STATUS(rs)      ((NETACCESS_ROWSTATUS_ACTIVE != rs) && (NETACCESS_ROWSTATUS_NOT_READY != rs))

#define NETACCESS_OM_IS_DIGIT(c)                (('0' <= c) && (c <= '9'))

#define CHECK_MAC_IS_NULL(mac) ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])==0)  /*added by Jinhua.Wei,to remove warning*/

#define NETACCESS_OM_ENTER_CRITICAL_SECTION()  orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netaccess_om_sem_id);
#define NETACCESS_OM_LEAVE_CRITICAL_SECTION()  SYSFUN_OM_LEAVE_CRITICAL_SECTION(netaccess_om_sem_id,orig_priority);

#define NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(RET_VAL) { \
        NETACCESS_OM_LEAVE_CRITICAL_SECTION(); \
        return (RET_VAL); \
    }

#define NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE() { \
        NETACCESS_OM_LEAVE_CRITICAL_SECTION(); \
        return; \
    }

#define NETACCESS_OM_HISAM_LOCK()                                \
    orig_priority_hisam=SYSFUN_OM_ENTER_CRITICAL_SECTION(netaccess_om_sem_hisam);

#define NETACCESS_OM_HISAM_UNLOCK()                              \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netaccess_om_sem_hisam, orig_priority_hisam);

/* DATA TYPE DECLARATIONS
 */
typedef struct NETACCESS_OM_PortModeAttribute_S
{
    UI32_T     minimum_secure_address;                /* minimum value of max-mac-count */
    UI8_T      is_allow_shrink_upper_mac_count    :1; /* allow shrink upper max-mac-count or not */
    UI8_T      is_allow_create_secure_mac_address :1; /* allow create secure mac address manually */
    UI8_T      reserved                           :6; /* reserved */
} NETACCESS_OM_PortModeAttribute_T;

typedef struct NETACCESS_OM_PortDisableTimer_S
{
    UI32_T      lport;
    UI32_T      expire_time;
    BOOL_T      is_available;

    struct NETACCESS_OM_PortDisableTimer_S  *prev;
    struct NETACCESS_OM_PortDisableTimer_S  *next;

} NETACCESS_OM_PortDisableTimer_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static
NETACCESS_OM_SecurePortEntry_T* NETACCESS_OM_LocalGetSecurePortEntryPtr(
    UI32_T lport
    );

static
NETACCESS_OM_GuestVlan_T* NETACCESS_OM_GuestVlan(
    UI32_T lport
    );

static
NETACCESS_OM_DynamicVlan_T* NETACCESS_OM_DynamicVlan(
    UI32_T lport
    );

static BOOL_T NETACCESS_OM_LocalCreateSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *mac_entry);
static BOOL_T NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex(UI32_T mac_index);

static BOOL_T NETACCESS_OM_LocalIsMacEffectCounter(NETACCESS_OM_SecureMacEntry_T *mac_entry);

static BOOL_T NETACCESS_OM_LocalGetMinimumSecureAddresses(UI32_T port_mode, UI32_T *qty);
static BOOL_T NETACCESS_OM_LocalGetUsableSecureAddress(UI32_T lport, UI32_T *qty);

static void NETACCESS_OM_LocalDumpHisamEntry(NETACCESS_OM_HISAMentry_T *hisam_entry);
static void NETACCESS_OM_LocalPrintMacAddress(UI8_T *addr);
static void NETACCESS_OM_LocalInitPortModeAttribute(void);

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
static int NETACCESS_OM_LocalCompLocManAddrEntry(void* inlist_element, void* new_element);
static BOOL_T NETACCESS_OM_LocalIsMacFilterTableFull(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T   netaccess_om_sem_id;
static UI32_T   orig_priority;
static UI32_T   netaccess_om_sem_hisam;
static UI32_T   orig_priority_hisam;

const static L_HISAM_KeyDef_T key_def_table[NETACCESS_NUMBER_OF_KEYS] =
    {
        {   2,                                              /* fields number (lport + mac) */
            {0, 20,                    0, 0, 0, 0, 0, 0},   /* offset */
            {4, SYS_ADPT_MAC_ADDR_LEN, 0, 0, 0, 0, 0, 0}    /* len */
        },
        {   4,                                              /* fields number (lport + authorized_status + record_time + mac) */
            {0, 4, 8, 20,                    0, 0, 0, 0},  /* offset */
            {4, 4, 4, SYS_ADPT_MAC_ADDR_LEN, 0, 0, 0, 0}   /* len */
        },
        {   3,                                              /* fields number (session_expire_time + lport + mac) */
            {12, 0, 20,                    0, 0, 0, 0, 0},  /* offset */
            { 4, 4, SYS_ADPT_MAC_ADDR_LEN, 0, 0, 0, 0, 0}   /* len */
        },
        {   2,                                              /* fields number (mac + lport) */
            {20,                    0, 0, 0, 0, 0, 0, 0},   /* offset */
            {SYS_ADPT_MAC_ADDR_LEN, 4, 0, 0, 0, 0, 0, 0}    /* len */
        },
    };

static L_HISAM_Desc_T   netaccess_hisam_desc = { 0, 0, 0, 0, 0, 0, 0, 0 };

static BOOL_T   net_access_om_intialized = FALSE;

static UI32_T   new_mac_msgq_id,radius_msgq_id,dot1x_msgq_id, link_state_msgq_id, vlan_modified_msgq_id;
static UI32_T   macageout_msgq_id;
static UI32_T   network_access_task_id;
static UI32_T   authorized_result_cookie_for_dot1x  =0;
static UI32_T   authorized_result_cookie_for_radius =0;

/* secure port table which have relative information for authentication
 */
static NETACCESS_OM_SecurePortEntry_T      secure_port_table[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* mac-authentication port table
 */
static NETACCESS_OM_MacAuthPortEntry_T     mac_auth_port_table[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* secure MAC table which allow MAC switching
 */
static NETACCESS_OM_SecureMacEntry_T       secure_mac_table[NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE];

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* filter MAC table which pre-authenticated
 */
static L_SORT_LST_List_T                   filter_mac_table[SYS_ADPT_NETACCESS_MAX_FILTER_ID+1];
#endif

/* authorized MAC's life time,do reAuth
 */
static UI32_T secure_reauth_time;

/* authorized MAC's age time,age out
 */
static UI32_T secure_auth_age_time;

/* unauthorized MAC's keep time configuration
 */
static UI32_T secure_holdoff_time;
static UI32_T secure_auth_mode;

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/* MAC address aging mode
 */
static UI32_T mac_addr_aging_mode;
#endif

/* username and password's format when send to RADIUS server
 * papUsernameAsMacAddress(1) -- NETACCESS_AUTHMODE_MACADDRESS
 *                  Authentication uses the RADIUS server by
 *                  sending a PAP request with Username and
 *                  Password both equal to the MAC address being
 *                  authenticated. This is the default.
 * papUsernameFixed(2) -- NETACCESS_AUTHMODE_FIXED
 *                  Authentication uses the RADIUS server by
 *                  sending a PAP request with Username and
 *                  Password coming from the secureRadaAuthUsername and
 *                  secureRadaAuthPassword MIB objects.  In this mode
 *                  the RADIUS server would normally take into account
 *                  the request's calling-station-id attribute, which is
 *                  the MAC address of the host being authenticated.
 */
static NETACCESS_OM_StatisticData_T     per_system_statistic_data; /* statistic information */
static L_HASH_Desc_T unauth_mac_cache_desc = {0,0};
static UI32_T unauth_chache_expire_time;

static NETACCESS_OM_PortModeAttribute_T netaccess_om_port_mode_attribute_ar[NETACCESS_PORTMODE_MAX-1];
static UI32_T sum_of_max_nbr_addr_of_ports; /* the sum of secure_port_table entries' number_addresses */
static UI32_T netaccess_debug_flag = SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE;
static NETACCESS_OM_PortDisableTimer_T  port_disable_timer[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static NETACCESS_OM_PortDisableTimer_T  *head_of_diable_timer;
static NETACCESS_OM_PortDisableTimer_T  *tail_of_diable_timer;

static NETACCESS_PortEapDataList_T      port_eap_data_store_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* EXPORTED SUBPROGRAM BODIES */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_CreatSem
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for NETACCESS objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_CreatSem(void)
{
    /* create semaphore */
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &netaccess_om_sem_hisam) != SYSFUN_OK)
    {
        return FALSE;
    }

    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_NETACCESS_OM, &netaccess_om_sem_id)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of NETACCESS_OM_CreatSem */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for new mac
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetNewMacMsgQId(UI32_T msgq_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    new_mac_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for new mac
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNewMacMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == msgq_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *msgq_id = new_mac_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for dot1x
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDot1xMsgQId(UI32_T msgq_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    dot1x_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for dot1x
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDot1xMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == msgq_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *msgq_id = dot1x_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for RADIUS client
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetRadiusMsgQId(UI32_T msgq_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    radius_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for RADIUS client
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetRadiusMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == msgq_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *msgq_id = radius_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for link state change
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkStateChangeMsgQId(UI32_T msgq_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_state_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for link state change
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkStateChangeMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == msgq_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *msgq_id = link_state_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for vlan modified event
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetVlanModifiedMsgQId(UI32_T msgq_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    vlan_modified_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for vlan modified event
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetVlanModifiedMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == msgq_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *msgq_id = vlan_modified_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetMacAgeOutMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for mac age out
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetMacAgeOutMsgQId(UI32_T msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    macageout_msgq_id = msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetMacAgeOutMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for mac age out
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMacAgeOutMsgQId(UI32_T *msgq_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    *msgq_id = macageout_msgq_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Set task id of network access
 * INPUT:  task_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetTaskId(UI32_T task_id)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    network_access_task_id = task_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Get task id of network access
 * INPUT:  NONE.
 * OUTPUT: task_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetTaskId(UI32_T *task_id)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input buffer is valid
     */
    if(NULL == task_id)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *task_id = network_access_task_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_InitialSystemResource
 * ---------------------------------------------------------------------
 * PURPOSE: initial om resources (allocate memory, create HASH, HISAM etc)
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_InitialSystemResource()
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    {
        UI32_T filter_id;

        /* initialize filter mac table
         */
        for (filter_id = 1; filter_id<=SYS_ADPT_NETACCESS_MAX_FILTER_ID; filter_id++)
        {
            L_SORT_LST_Create(&filter_mac_table[filter_id],
                    SYS_ADPT_NETACCESS_TOTAL_NBR_OF_MAC_FILTER_ENTRY_PER_SYSTEM,
                    sizeof(NETACCESS_OM_MacFilterEntry_T),
                    NETACCESS_OM_LocalCompLocManAddrEntry);
        }
    }
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

    /* create hisam
     */
    netaccess_hisam_desc.hash_depth         = NETACCESS_HASH_DEPTH;
    netaccess_hisam_desc.N1                 = NETACCESS_HISAM_N1;
    netaccess_hisam_desc.N2                 = NETACCESS_HISAM_N2;
    netaccess_hisam_desc.record_length      = sizeof(NETACCESS_OM_HISAMentry_T);
    netaccess_hisam_desc.total_hash_nbr     = NETACCESS_HASH_NBR;
    netaccess_hisam_desc.total_index_nbr    = NETACCESS_HISAM_INDEX_NBR;
    netaccess_hisam_desc.total_record_nbr   = NETACCESS_HISAM_NODE_NBR;

    if (!L_HISAM_Create(&netaccess_hisam_desc, NETACCESS_NUMBER_OF_KEYS, key_def_table))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_Initialize] L_HISAM_Create() failed");

        return FALSE;
    }

    /* initialize unauthenticated mac table (create Hash table)
     */
    NETACCESS_OM_InitUnauthorizedMacCache();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize om
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_Initialize()
{
    UI32_T  index, unit, port, trunk_id;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* initialize secure port table
     */
    memset(&secure_port_table, 0, sizeof(secure_port_table));
    memset(&mac_auth_port_table, 0, sizeof(mac_auth_port_table));

    for (index = 0, sum_of_max_nbr_addr_of_ports = 0; SYS_ADPT_TOTAL_NBR_OF_LPORT > index; ++index)
    {
/*   ES4626F-SW-FLF-38-00975
 *   Make sure every port is properly initialized when it comes to stacking.
 *   So we commented the code snippet below.
 *   #if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
 *           if (SWCTRL_LPORT_NORMAL_PORT == SWCTRL_LogicalPortToUserPort(index + 1, &unit, &port, &trunk_id) ||
 *                SWCTRL_LPORT_TRUNK_PORT == SWCTRL_LogicalPortToUserPort(index + 1, &unit, &port, &trunk_id))
 *   #else
 *           if (SWCTRL_LPORT_NORMAL_PORT == SWCTRL_LogicalPortToUserPort(index + 1, &unit, &port, &trunk_id))
 *   #endif
*/
        {
            secure_port_table[index].lport = index + 1;
            secure_port_table[index].number_addresses = SYS_DFLT_NETACCESS_SECURE_ADDRESSES_PER_PORT;
            secure_port_table[index].configured_number_addresses = secure_port_table[index].number_addresses;

            mac_auth_port_table[index].lport = index + 1;
            mac_auth_port_table[index].configured_number_addresses = SYS_DFLT_NETACCESS_MACAUTH_SECURE_ADDRESSES_PER_PORT;
        }

        sum_of_max_nbr_addr_of_ports += secure_port_table[index].number_addresses;

        secure_port_table[index].port_mode = SYS_DFLT_NETACCESS_SECURE_PORT_MODE;
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
        secure_port_table[index].dynamic_vlan.dynamic_vlan_enabled = SYS_DFLT_NETACCESS_DYNAMIC_VLAN_ENABLE;
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
        secure_port_table[index].dynamic_qos.enabled = SYS_DFLT_NETACCESS_DYNAMIC_QOS_ENABLE;
#endif

        /* initialize state mcahine
         */
        secure_port_table[index].state_machine.running_port_mode = SYS_DFLT_NETACCESS_SECURE_PORT_MODE;
        secure_port_table[index].state_machine.new_port_mode = SYS_DFLT_NETACCESS_SECURE_PORT_MODE;

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        /* link-detection
         */
        if (SYS_DFLT_NETACCESS_LINK_DETECTION_STATUS == VAL_networkAccessPortLinkDetectionStatus_enabled)
        {
            secure_port_table[index].link_detection.enabled = 1;
        }

        if (SYS_DFLT_NETACCESS_LINK_DETECTION_MODE == VAL_networkAccessPortLinkDetectionMode_linkUp)
        {
            secure_port_table[index].link_detection.detect_linkup = 1;
        }
        else if (SYS_DFLT_NETACCESS_LINK_DETECTION_MODE == VAL_networkAccessPortLinkDetectionMode_linkDown)
        {
            secure_port_table[index].link_detection.detect_linkdown = 1;
        }
        else if (SYS_DFLT_NETACCESS_LINK_DETECTION_MODE == VAL_networkAccessPortLinkDetectionMode_linkUpDown)
        {
            secure_port_table[index].link_detection.detect_linkup = 1;
            secure_port_table[index].link_detection.detect_linkdown = 1;
        }

        if (SYS_DFLT_NETACCESS_LINK_DETECTION_ACTION == VAL_networkAccessPortLinkDetectionAciton_trap)
        {
            secure_port_table[index].link_detection.action_sendtrap = 1;
        }
        else if (SYS_DFLT_NETACCESS_LINK_DETECTION_ACTION == VAL_networkAccessPortLinkDetectionAciton_shutdown)
        {
            secure_port_table[index].link_detection.action_shutdown = 1;
        }
        else if (SYS_DFLT_NETACCESS_LINK_DETECTION_ACTION == VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown)
        {
            secure_port_table[index].link_detection.action_sendtrap = 1;
            secure_port_table[index].link_detection.action_shutdown = 1;
        }
#endif

        /* mac-authentication
         */
        mac_auth_port_table[index].intrusion_action = SYS_DFLT_NETACCESS_MACAUTH_INTRUSIONACTION_ACTION;

        /* intialize port disable timer */
        memset(&port_disable_timer[index], 0, sizeof(NETACCESS_OM_PortDisableTimer_T));
        port_disable_timer[index].lport = index + 1;
    }

    /* intialize statistic data
     */
    memset(&per_system_statistic_data, 0, sizeof(per_system_statistic_data));

    /* intialize port disable timer */
    head_of_diable_timer = NULL;
    tail_of_diable_timer = NULL;

    /* initialize mac table
     */
    for (index = 0; NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE > index; ++index)
    {
        memset(&secure_mac_table[index], 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
        secure_mac_table[index].mac_index = index + 1;
        secure_mac_table[index].addr_row_status = NETACCESS_ROWSTATUS_DESTROY;
    }

    /* should not initialize follwing function pointer
     * because MGR will initialize them in NETACCESS_MGR_InitiateSystemResources()
     * authorized_result_cookie_for_dot1x = NULL;
     * authorized_result_cookie_for_radius = NULL;
     */

    //secure_port_security_control_enabled = (SYS_DFLT_NETWORKACCESS_SECURE_PORT_SECURITY_CONTROL == VAL_securePortSecurityControl_enabled);
    secure_reauth_time = SYS_DFLT_NETACCESS_SECURE_REAUTH_TIME;
    secure_auth_age_time = NETACCESS_TYPE_DFLT_SECURE_MAC_ADDRESS_AUTH_AGE;
    secure_holdoff_time = NETACCESS_OM_SECURE_UNAUTH_TIME;
    secure_auth_mode = SYS_DFLT_NETACCESS_SECURE_AUTH_MODE;

    /* initialize unauthenticated mac table
     */
    unauth_chache_expire_time = NETACCESS_OM_SECURE_UNAUTH_TIME;

    /* initialize port mode attribute
     */
    NETACCESS_OM_LocalInitPortModeAttribute();

#ifdef SYS_CPNT_NETACCESS_AGING_MODE
#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    mac_addr_aging_mode = SYS_DFLT_NETACCESS_AGING_MODE;
#endif
#endif

    net_access_om_intialized = TRUE;

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[NETACCESS_OM_Initialize] done");

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ClearAll
 * ---------------------------------------------------------------------
 * PURPOSE: Clear om resource
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ClearAll()
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    {
        UI32_T filter_id;

        for (filter_id = 1; filter_id<=SYS_ADPT_NETACCESS_MAX_FILTER_ID; filter_id++)
        {
            L_SORT_LST_Delete_All(&filter_mac_table[filter_id]);
        }
    }
#endif

    /* Clear HISAM
     */
    L_HISAM_DeleteAllRecord(&netaccess_hisam_desc);

    /* initialize unauthenticated mac table
     */
    if (FALSE == L_HASH_DeleteAll(&unauth_mac_cache_desc))
    {
        printf("\r\n[%s]: L_HASH_DeleteAll() Error !", __FUNCTION__);
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for dot1x
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETACCESS_OM_SetDot1xAuthorizedResultCookie(UI32_T cookie)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    authorized_result_cookie_for_dot1x = cookie;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for dot1x
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDot1xAuthorizedResultCookie(UI32_T *cookie)
{
    BOOL_T  ret = FALSE;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid and database already have value
     */
    if ((NULL != cookie) && (0 != authorized_result_cookie_for_dot1x))
    {
        *cookie = authorized_result_cookie_for_dot1x;
        ret = TRUE;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetRadiusAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for rada
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETACCESS_OM_SetRadiusAuthorizedResultCookie(UI32_T cookie)
{
    /* set to database
     */
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    authorized_result_cookie_for_radius = cookie;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetRadaAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for rada
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetRadaAuthorizedResultCookie(UI32_T *cookie)
{
    BOOL_T  ret = FALSE;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid and database already have value
     */
    if ((NULL != cookie) && (0 != authorized_result_cookie_for_radius))
    {
        *cookie = authorized_result_cookie_for_radius;
        ret = TRUE;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set security modes of the port
 * INPUT:  lport,secure_port_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortMode(UI32_T lport,NETACCESS_PortMode_T secure_port_mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if port is reasonable
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecurePortMode] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if input is valid and set to database
     */
    if((NETACCESS_PORTMODE_NO_RESTRICTIONS > secure_port_mode) ||
        (NETACCESS_PORTMODE_MAX <= secure_port_mode))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    secure_port_table[lport - 1].port_mode = secure_port_mode;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the learning and security modes of the port
 * INPUT:  lport
 * OUTPUT: secure_port_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == secure_port_mode) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecurePortMode] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *secure_port_mode = secure_port_table[lport - 1].port_mode;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorised device
 *          transmits on this port.
 * INPUT:  lport,secure_intrusion_action.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_IntrusionAction_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureIntrusionAction(UI32_T lport,UI32_T secure_intrusion_action)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if bad intrusion action
     */
    if((NETACCESS_INTRUSIONACTION_NOT_AVAILABLE >= secure_intrusion_action) ||
        (NETACCESS_INTRUSIONACTION_MAX <= secure_intrusion_action))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad intrusion action(%lu)", __FUNCTION__, secure_intrusion_action);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* set to database
     */
    secure_port_table[lport - 1].intrusion_action = secure_intrusion_action;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorised device
 *          transmits on this port.
 * INPUT:  lport
 * OUTPUT: secure_intrusion_action.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_IntrusionAction_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureIntrusionAction(UI32_T lport, UI32_T *secure_intrusion_action)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* chcek if input is valid
     */
    if ((NULL == secure_intrusion_action) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureIntrusionAction] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *secure_intrusion_action = secure_port_table[lport - 1].intrusion_action;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by the unit and the port.
 * INPUT    : lport, number:secureNumberAddresses
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureNumberAddresses(UI32_T lport, UI32_T number)
{
    UI32_T      min, max, index;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport) ||
        (FALSE == NETACCESS_OM_LocalGetUsableSecureAddress(lport, &max)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecureNumberAddresses] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    index = lport - 1; /* to zero-based */

    /* get minimum nbr
     */
    if (FALSE == NETACCESS_OM_LocalGetMinimumSecureAddresses(secure_port_table[index].port_mode, &min))
        min = 0;

    /* range check
     * if stored # (%lu) > number (%lu), MGR MUST reduce stored numeber then set it
     */
    if ((min > number) ||
        (secure_port_table[index].number_addresses_stored > number) ||
        (max < number))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
        {
            printf("\r\n[NETACCESS_OM_SetSecureNumberAddresses] out of range: (%lu) <= (%lu) <= (%lu) <= (%lu)",
                    min, secure_port_table[index].number_addresses_stored, number, max);
        }

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* set to database
     */
    sum_of_max_nbr_addr_of_ports = sum_of_max_nbr_addr_of_ports - secure_port_table[index].number_addresses + number;
    secure_port_table[index].number_addresses = number;
    secure_port_table[index].configured_number_addresses = secure_port_table[index].number_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get secureNumberAddresses by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 * OUTPUT   : number:secureNumberAddresses.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddresses(UI32_T lport, UI32_T *number)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == number) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *number = secure_port_table[lport - 1].number_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesStored
 * ---------------------------------------------------------------------
 * PURPOSE: The number of addresses that are currently in the
 *          AddressTable for this port. If this object has the same value as
 *          secureNumberAddresses, then no more addresses can be authorised on this
 *          port.
 * INPUT:  lport,secure_number_addresses_stored.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesStored(UI32_T lport, UI32_T *secure_number_addresses_stored)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == secure_number_addresses_stored) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureNumberAddressesStored] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *secure_number_addresses_stored = secure_port_table[lport - 1].number_addresses_stored;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of authorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesAuthorized(UI32_T lport, UI32_T *addr_number)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == addr_number) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *addr_number = secure_port_table[lport - 1].nbr_of_authorized_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesUnauthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of unauthorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesUnauthorized(UI32_T lport, UI32_T *addr_number)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == addr_number) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureNumberAddressesUnauthorized] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *addr_number = secure_port_table[lport - 1].number_addresses_stored - secure_port_table[lport - 1].nbr_of_authorized_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureMaximumAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maxinum value that secureNumberAddresses
 *          can be set to.
 * INPUT:  lport,secure_maximum_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * This indicates the maximum value that secureNumberAddresses
 * can be set to. It is dependent on the resources available so may change,
 * eg. if resources are shared between ports, then this value can both
 * increase and decrease. This object must be read before setting
 * secureNumberAddresses.
 *
 * The following relationship must allows be preserved.
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureMaximumAddresses(UI32_T lport, UI32_T *secure_maximum_addresses)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == secure_maximum_addresses) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureMaximumAddresses] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_LocalGetUsableSecureAddress(lport, secure_maximum_addresses);
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesLearntAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of learnt authorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesLearntAuthorized(UI32_T lport, UI32_T *addr_number)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == addr_number) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *addr_number = secure_port_table[lport - 1].nbr_of_learn_authorized_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DecreaseSecureNumberAddressesLearntAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: set number of learnt authorized addresses
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecreaseSecureNumberAddressesLearntAuthorized(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if allow decrease counter
     */
    if(0 == secure_port_table[lport - 1].nbr_of_learn_authorized_addresses)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] counter is zero,cannot decrease", __FUNCTION__);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* decrease counter
     */
    secure_port_table[lport - 1].nbr_of_learn_authorized_addresses--;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetTotalReservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the total number that secureNumberAddresses had be set.
 * INPUT:  total_reserved_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetTotalReservedSecureAddresses(UI32_T *total_reserved_addresses)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == total_reserved_addresses)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *total_reserved_addresses = sum_of_max_nbr_addr_of_ports;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == min_addresses)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_LocalGetMinimumSecureAddresses(port_mode, min_addresses);
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maximum value that secureNumberAddresses can be set to.
 * INPUT:  lport, usable_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == usable_addresses)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    ret = NETACCESS_OM_LocalGetUsableSecureAddress(lport, usable_addresses);
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == unreserved_nbr)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* "hidden secure MACs" space is not configurable for user
     * so SHALL not use NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE to calculate unreserved number
     */
    *unreserved_nbr = SYS_ADPT_NETACCESS_TOTAL_NBR_OF_SECURE_ADDRESSES_PER_SYSTEM - sum_of_max_nbr_addr_of_ports;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ExtendUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to increase secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if there is no unreserved secure address, return FALSE;
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ExtendUpperBoundByPort(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if port is valid and could extend the number
     * "hidden secure MACs" space is not configurable for user
     * so SHALL not use NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE to check
     */
    if ((0 >= (SYS_ADPT_NETACCESS_TOTAL_NBR_OF_SECURE_ADDRESSES_PER_SYSTEM - sum_of_max_nbr_addr_of_ports)) ||
        (NETACCESS_OM_IS_BAD_LPORT_NO(lport)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ExtendUpperBoundByPort] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* extend the number
     */
    ++sum_of_max_nbr_addr_of_ports;
    ++(secure_port_table[lport - 1].number_addresses);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ShrinkUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to decrease secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if number_addresses <= configured_number_addresses, must check port mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ShrinkUpperBoundByPort(UI32_T lport)
{
    UI32_T index;
    UI32_T port_mode;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if port is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ShrinkUpperBoundByPort] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    index = lport - 1; /* to zero-based */
    port_mode = secure_port_table[index].port_mode;

    /* check if allow to decrease the number
     */
    if (secure_port_table[index].number_addresses <= secure_port_table[index].configured_number_addresses)
    {
        if(0 == netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_shrink_upper_mac_count)
        {
            /* not allow to shrink number_addresses smaller than configured_number_addresses */
            NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }
    }

    /* decrease the number
     */
    --sum_of_max_nbr_addr_of_ports;
    --(secure_port_table[index].number_addresses);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsSecureAddressesFull
 * ---------------------------------------------------------------------
 * PURPOSE: check whether secure address is full or not
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- full / FALSE -- not yet
 * NOTES:  full ==> secureNumberAddressesStored >= secureNumberAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressesFull(UI32_T lport)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if port is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_IsSecureAddressesFull] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if address full
     */
    ret = (secure_port_table[lport - 1].number_addresses_stored >= secure_port_table[lport - 1].number_addresses);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT:  reauth_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureReauthTime(UI32_T reauth_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* range check
     */
    if ((SYS_ADPT_NETACCESS_MIN_REAUTH_TIME > reauth_time) ||
        (SYS_ADPT_NETACCESS_MAX_REAUTH_TIME < reauth_time))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
        {
            printf("\r\n[NETACCESS_OM_SetSecureRadaDefaultSessionTime] out of range: (%lu) <= (%lu) <= (%lu)",
                    SYS_ADPT_NETACCESS_MIN_REAUTH_TIME, reauth_time, SYS_ADPT_NETACCESS_MAX_REAUTH_TIME);
        }

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* set to database
     */
    secure_reauth_time = reauth_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT:  None.
 * OUTPUT: reauth_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureReauthTime(UI32_T *reauth_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == reauth_time)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureRadaDefaultSessionTime] null pointer");

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *reauth_time = secure_reauth_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureAuthAgeTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the auth age time.
 * INPUT:  auth_age_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the auth age time in seconds when
 *         a forwarding MAC address will age out.
 *         The default time is 300 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAuthAgeTime(UI32_T auth_age_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* range check
     */
    if ((NETACCESS_TYPE_MIN_OF_SECURE_MAC_ADDRESS_AUTH_AGE > auth_age_time) ||
        (NETACCESS_TYPE_MAX_OF_SECURE_MAC_ADDRESS_AUTH_AGE < auth_age_time))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
        {
            printf("\r\n[%s] out of range: (%u) <= (%lu) <= (%u)",
                    __FUNCTION__,
                    NETACCESS_TYPE_MIN_OF_SECURE_MAC_ADDRESS_AUTH_AGE,
                    auth_age_time,
                    NETACCESS_TYPE_MAX_OF_SECURE_MAC_ADDRESS_AUTH_AGE);
        }

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* set to database
     */
    secure_auth_age_time = auth_age_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureAuthAgeTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the auth age time.
 * INPUT:  None.
 * OUTPUT: auth_age_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the auth age time in seconds when
 *         a forwarding MAC address will age out.
 *         The default time is 300 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAuthAgeTime(UI32_T *auth_age_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == auth_age_time)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer", __FUNCTION__);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *auth_age_time = secure_auth_age_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the holdoff time.
 * INPUT:  holdoff_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied)
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureHoldoffTime(UI32_T holdoff_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the holdoff time.
 * INPUT:  None.
 * OUTPUT: holdoff_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied)
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureHoldoffTime(UI32_T *holdoff_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == holdoff_time)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureRadaHoldoffTime] null pointer");

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from dtabase
     */
    *holdoff_time = secure_holdoff_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authentication mode.
 * INPUT:  auth_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAuthMode(UI32_T auth_mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* set to database
     */
    switch(auth_mode)
    {
        case NETACCESS_AUTHMODE_MACADDRESS:
        case NETACCESS_AUTHMODE_FIXED:
            secure_auth_mode = auth_mode;
            break;

        default:
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_SetSecureRadaAuthMode] bad rada auth mode(%lu)", auth_mode);

            NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authentication mode.
 * INPUT:  None.
 * OUTPUT: auth_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAuthMode(UI32_T *auth_mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NULL == auth_mode)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureRadaAuthMode] null pointer");

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    *auth_mode = secure_auth_mode;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port vlan configuration control
 * INPUT:  lport, dynamic_vlan_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureDynamicVlanStatus(UI32_T lport, BOOL_T enabled)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
    NETACCESS_OM_DynamicVlan_T *vlan_p = NETACCESS_OM_DynamicVlan(lport);

    if (vlan_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    vlan_p->dynamic_vlan_enabled = enabled;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return FALSE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port vlan configuration control
 * INPUT:  lport.
 * OUTPUT: enabled
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicVlanStatus(UI32_T lport,BOOL_T *enabled_p)
{
    if (enabled_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
    {
        NETACCESS_OM_DynamicVlan_T *vlan_p = NETACCESS_OM_DynamicVlan(lport);

    if (vlan_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
        *enabled_p = vlan_p->dynamic_vlan_enabled;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }
#else /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */
    *enabled_p = FALSE;
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Set MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport, md5
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDynamicVlanMd5(UI32_T lport, UI8_T md5[16])
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_DynamicVlan_T *vlan_p = NETACCESS_OM_DynamicVlan(lport);

    if (vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong lport(%lu)", lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    memcpy(vlan_p->applied_md5, md5, sizeof(vlan_p->applied_md5));
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ClearDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Clear MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ClearDynamicVlanMd5(UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_DynamicVlan_T *dynamic_vlan_p = NETACCESS_OM_DynamicVlan(lport);
    NETACCESS_OM_GuestVlan_T *guest_vlan_p = NETACCESS_OM_GuestVlan(lport);

    if (dynamic_vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong lport(%lu)", lport);
    return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    memset(dynamic_vlan_p->applied_md5, 0, sizeof(dynamic_vlan_p->applied_md5));

    if (guest_vlan_p)
    {
        guest_vlan_p->applied_on_port = FALSE;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return TRUE;
#endif
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Get MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport.
 * OUTPUT : md5
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicVlanMd5(UI32_T lport, UI8_T md5[16])
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_DynamicVlan_T *vlan_p = NETACCESS_OM_DynamicVlan(lport);

    if (vlan_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    memcpy(md5, vlan_p->applied_md5, sizeof(vlan_p->applied_md5));
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    memset(md5, 0, 16);
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port QoS configuration control
 * INPUT:  lport, dynamic_qos_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureDynamicQosStatus(UI32_T lport, BOOL_T yes_no)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_OM_SecurePortEntry_T *port_entry_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    if (port_entry_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    port_entry_p->dynamic_qos.enabled = yes_no;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return FALSE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port QoS configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_qos_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicQosStatus(UI32_T lport,BOOL_T *dynamic_qos_status)
{
    if (dynamic_qos_status == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    {
        NETACCESS_OM_SecurePortEntry_T *port_entry_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

        if (port_entry_p == NULL)
        {
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
            return FALSE;
        }

        NETACCESS_OM_ENTER_CRITICAL_SECTION();
        *dynamic_qos_status = port_entry_p->dynamic_qos.enabled;
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }
#else
    *dynamic_qos_status = FALSE;
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDynamicQosProfileMd5
 * ---------------------------------------------------------------------
 * PURPOSE  : Set md5 of qos profile string from RADIUS server
 * INPUT    : lport     -- lport index
 *            dqos_md5  -- md5 of qos profile string
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDynamicQosProfileMd5(UI32_T lport, UI8_T dqos_md5[16])
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_OM_SecurePortEntry_T *port_entry_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    if (port_entry_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    memcpy(port_entry_p->dynamic_qos.md5, dqos_md5, sizeof(port_entry_p->dynamic_qos.md5));
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return FALSE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicQosProfileMd5
 * ---------------------------------------------------------------------
 * PURPOSE  : Get md5 of qos profile string from RADIUS server
 * INPUT    : lport     -- lport index
 *            dqos_md5  -- md5 of qos profile string
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicQosProfileMd5(UI32_T lport, UI8_T dqos_md5[16])
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_OM_SecurePortEntry_T *port_entry_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    if (port_entry_p == NULL)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    memcpy(dqos_md5, port_entry_p->dynamic_qos.md5, sizeof(port_entry_p->dynamic_qos.md5));
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    memset(dqos_md5, 0, 16);
    return TRUE;
#endif
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will copy secure port entry by the lport.
 * INPUT    : lport
 * OUTPUT   : entry : The secure port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortEntry(UI32_T lport, NETACCESS_OM_SecurePortEntry_T *entry)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == entry) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETWORKACCESS_OM_GetSecurePortEntry] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, &secure_port_table[lport - 1], sizeof(NETACCESS_OM_SecurePortEntry_T));
    if (FALSE == NETACCESS_OM_LocalGetUsableSecureAddress(lport, &entry->maximum_addresses))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_CreateSecureAddressEntry
 * ---------------------------------------------------------------------
 * PURPOSE: create mac entry (om & hisam)
 * INPUT:  mac_entry (key: slot + port + mac)
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if existed, return FALSE
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_CreateSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *mac_entry)
{
    BOOL_T      ret;

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* create address entry
     */
    ret = NETACCESS_OM_LocalCreateSecureAddressEntry(mac_entry);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_DoesPortAllowCreateSecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : whether the specified port allow to create mac or not
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- yes / FALSE -- no
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DoesPortAllowCreateSecureAddrEntry(UI32_T lport)
{
    UI32_T port_mode;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check port_mode
     */
    port_mode = secure_port_table[lport - 1].port_mode;

    if(0 == netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_create_secure_mac_address)
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_UpdateSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: update mac entry by mac_index (om & hisam)
 * INPUT:  mac_entry
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_UpdateSecureAddressEntryByIndex(NETACCESS_OM_SecureMacEntry_T *mac_entry)
{
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecureMacEntry_T  *om_entry;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if ((NULL == mac_entry) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_entry->mac_index)) ||
        (NETACCESS_OM_IS_BAD_ROW_STATUS(mac_entry->addr_row_status)) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id)))
#else
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id)))
#endif
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_UpdateSecureAddressEntryByIndex] null pointer/bad mac index/bad row status/bad port");

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    om_entry = &secure_mac_table[mac_entry->mac_index - 1];

    /* lport, secure_mac are primary key
     * so they can't be changed.
     * If port move occurs, must delete the old entry and create a new one.
     */
    if ((om_entry->lport != mac_entry->lport) ||
        (0 != memcmp(om_entry->secure_mac, mac_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_UpdateSecureAddressEntryByIndex] bad slot or port or mac");

        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* update a hisam entry
     */
    hisam_entry.lport = mac_entry->lport;
    hisam_entry.authorized_status = mac_entry->authorized_status;
    hisam_entry.record_time = mac_entry->record_time;
    hisam_entry.session_expire_time = mac_entry->session_expire_time;
    hisam_entry.mac_index = mac_entry->mac_index;
    memcpy(hisam_entry.secure_mac, mac_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* L_HISAM_SetRecord(TRUE). replace the existed record
     */
    switch (L_HISAM_SetRecord(&netaccess_hisam_desc, (UI8_T *)&hisam_entry, TRUE))
    {
        case L_HISAM_REPLACE:
            break;

        case L_HISAM_INSERT:
            /* should not go here
             * it is impossible to update an non-existent mac entry
             */
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_UpdateSecureAddressEntryByIndex] L_HISAM_INSERT value was returned");

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;

        case L_HISAM_CONFLICT:
        case L_HISAM_NO_BUFFER: /* something wrong */
        default:
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_UpdateSecureAddressEntryByIndex] there is something wrong in hisam");

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;
    }

    /* update counter
     */
    if ((om_entry->authorized_status != mac_entry->authorized_status) &&
        (TRUE == NETACCESS_OM_LocalIsMacEffectCounter(mac_entry)))
    {
        if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry->authorized_status)
            ++secure_port_table[mac_entry->lport - 1].nbr_of_authorized_addresses;
        else
            --secure_port_table[mac_entry->lport - 1].nbr_of_authorized_addresses;
    }

    /* update the om entry
     */
    memcpy(om_entry, mac_entry, sizeof(NETACCESS_OM_SecureMacEntry_T));

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry)
{
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecureMacEntry_T  *om_entry;


    /* check if input is valid
     */
    if (NULL == entry)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureAddressEntry] null pointer");

        return FALSE;
    }

    /* prepare searching key
     */
    key.lport = entry->lport;
    memcpy(key.secure_mac, entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_HISAM_LOCK();
    /* get HISAM record
     */
    if ((FALSE == L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry)) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureAddressEntry] L_HISAM_GetRecord() failed or bad mac_index");

        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }
    NETACCESS_OM_HISAM_UNLOCK();

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    om_entry = &secure_mac_table[hisam_entry.mac_index - 1];

    /* check row status
     */
    if (NETACCESS_OM_IS_BAD_ROW_STATUS(om_entry->addr_row_status))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    memcpy(entry, om_entry, sizeof(NETACCESS_OM_SecureMacEntry_T));

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntryByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->mac_index
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntryByIndex(NETACCESS_OM_SecureMacEntry_T *entry)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == entry) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(entry->mac_index)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureAddressEntryByIndex] null pointer or bad mac index");

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[entry->mac_index - 1];

    /* check row status
     */
    if (NETACCESS_OM_IS_BAD_ROW_STATUS(om_entry->addr_row_status))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    memcpy(entry, om_entry, sizeof(NETACCESS_OM_SecureMacEntry_T));

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntryByMacFlag
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get secure address entry entry by lport and mac_flag
 * INPUT    : entry->lport, entry->mac_flag
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : return the first entry satisfied (entry->mac_flag & om_entry->mac_flag)
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntryByMacFlag(NETACCESS_OM_SecureMacEntry_T *entry)
{
    BOOL_T      ret;
    UI8_T       *searching_mac_flag, *om_mac_flag;

    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecureMacEntry_T  *om_entry;


    /* check if input is valid
     */
    if (NULL == entry)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetSecureAddressEntryByMacFlag] null pointer");

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    searching_mac_flag = (UI8_T*)&entry->mac_flag;

    /* prepare searching key
     */
    key.lport = entry->lport;
    memset(key.secure_mac, 0, SYS_ADPT_MAC_ADDR_LEN);

    ret = FALSE;

    /* get next HISAM record
     */
    while (TRUE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        /* let the key point to next record
         */
        key.lport = hisam_entry.lport;
        memcpy(key.secure_mac, hisam_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* check if mac_index is valid
         */
        if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
        {
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_GetSecureAddressEntryByMacFlag] bad mac_index(%lu)", hisam_entry.mac_index);

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;
        }

        om_entry = &secure_mac_table[hisam_entry.mac_index - 1];
        om_mac_flag = (UI8_T*)&om_entry->mac_flag; /* because sizeof(mac_flag) == 1 */

        /* check if same mac_flag
         */
        if (0 == (*om_mac_flag & *searching_mac_flag))
            continue;

        /* get from database
         */
        memcpy(entry, om_entry, sizeof(NETACCESS_OM_SecureMacEntry_T));

        ret = TRUE;

        if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        {
            printf("\r\n[NETACCESS_OM_GetSecureAddressEntryByMacFlag]\r\nfound lport:%lu, mac:", key.lport);
            NETACCESS_OM_LocalPrintMacAddress(key.secure_mac);
        }

        break;
    }

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the lport and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : get by (lport,MAC) key
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry)
{
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if (NULL == entry)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetNextSecureAddressEntry] null pointer");

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* prepare searching key
     */
    key.lport = entry->lport;
    memcpy(key.secure_mac, entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    if (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetNextSecureAddressEntry] Bad mac_index");

        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
    {
        printf("\r\n[NETACCESS_OM_GetNextSecureAddressEntry]\r\nkey(%lu, ", key.lport);
        NETACCESS_OM_LocalPrintMacAddress(key.secure_mac);
        printf(")\r\ndump hisam");
        NETACCESS_OM_LocalDumpHisamEntry(&hisam_entry);
    }

    /* get from database
     */
    memcpy(entry, &secure_mac_table[hisam_entry.mac_index - 1], sizeof(NETACCESS_OM_SecureMacEntry_T));

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextSecureAddressEntryByMacKey
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the mac_address and the lport.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : get by (MAC,lport) key
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextSecureAddressEntryByMacKey(NETACCESS_OM_SecureMacEntry_T *entry)
{
    NETACCESS_OM_MacAdrKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if (NULL == entry)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer", __FUNCTION__);

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* prepare searching key
     */
    key.lport = entry->lport;
    memcpy(key.secure_mac, entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    if (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_MACADR_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
            printf("\r\n[%s] L_HISAM_GetNextRecord() failed", __FUNCTION__);

        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* check if returned mac_index valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
            printf("\r\n[%s] bad mac_index", __FUNCTION__);

        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
    {
        printf("\r\n[%s]\r\nkey(%lu, ", __FUNCTION__, key.lport);
        NETACCESS_OM_LocalPrintMacAddress(key.secure_mac);
        printf(")\r\ndump hisam");
        NETACCESS_OM_LocalDumpHisamEntry(&hisam_entry);
    }

    /* get from database
     */
    memcpy(entry, &secure_mac_table[hisam_entry.mac_index - 1], sizeof(NETACCESS_OM_SecureMacEntry_T));

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextAdminConfiguredSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next manual configured secure address entry by the lport and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextAdminConfiguredSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry)
{
    BOOL_T      ret;

    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if (NULL == entry)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer", __FUNCTION__);

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* prepare searching key
     */
    key.lport = entry->lport;
    memcpy(key.secure_mac, entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    ret = FALSE;

    /* get next HISAM record
     */
    while (TRUE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        /* let the key point to next record
         */
        key.lport = hisam_entry.lport;
        memcpy(key.secure_mac, hisam_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* check if mac_index is valid
         */
        if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
        {
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[%s] bad mac_index(%lu)", __FUNCTION__, hisam_entry.mac_index);

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;
        }

        /* check if manual configured
         */
        if (0 == secure_mac_table[hisam_entry.mac_index - 1].mac_flag.admin_configured_mac)
        {
            /* only need manual configured mac
             */
            continue;
        }

        /* get from database
         */
        memcpy(entry, &secure_mac_table[hisam_entry.mac_index - 1], sizeof(NETACCESS_OM_SecureMacEntry_T));

        ret = TRUE;
        break;
    }

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_IsSecureAddressAdminConfigured
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check whether the secure address is administrative configured
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- admin configured / FALSE -- not admin configured
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressAdminConfigured(UI32_T mac_index)
{
    BOOL_T      ret;

    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_IsSecureAddressAdminConfigured] bad mac index(%lu)", mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* check row status
     */
    if (NETACCESS_OM_IS_BAD_ROW_STATUS(om_entry->addr_row_status))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if admin configured
     */
    ret = (1 == om_entry->mac_flag.admin_configured_mac);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_IsSecureAddressAuthByRadius
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check whether the secure address is authenticated by RADIUS
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- admin configured / FALSE -- not admin configured
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressAuthByRadius(UI32_T mac_index)
{
    BOOL_T      ret;

    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad mac index(%lu)", __FUNCTION__, mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* check row status
     */
    if (NETACCESS_OM_IS_BAD_ROW_STATUS(om_entry->addr_row_status))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if admin configured
     */
    ret = (1 == om_entry->mac_flag.auth_by_rada);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrRowStatus
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secure port entry status by the unit, port and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac, entry->addr_row_status
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : addr_row_status must be active or notInService
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrRowStatus(const NETACCESS_OM_SecureMacEntry_T *entry)
{
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if ((NULL == entry) ||
        (NETACCESS_OM_IS_BAD_ROW_STATUS(entry->addr_row_status)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecureAddrRowStatus] null pointer or bad row status");

        return FALSE;
    }

    /* prepare searching key
     */
    key.lport = entry->lport;
    memcpy(key.secure_mac, entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_HISAM_LOCK();
    /* get next HISAM record
     */
    if ((FALSE == L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry)) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecureAddrRowStatus] L_HISAM_GetRecord() failed or bad mac_index");

        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* nothing changed
     */
    if (secure_mac_table[hisam_entry.mac_index - 1].addr_row_status == entry->addr_row_status)
    {
        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* translate row status to authorized status
     * use 3Com's definition
     */
    if (NETACCESS_ROWSTATUS_ACTIVE == entry->addr_row_status)
        hisam_entry.authorized_status = NETACCESS_ROWSTATUS_ACTIVE;
    else
        hisam_entry.authorized_status = NETACCESS_ROWSTATUS_NOT_IN_SERVICE;

    /* L_HISAM_SetRecord(TRUE). replace the existed record
     */
    switch (L_HISAM_SetRecord(&netaccess_hisam_desc, (UI8_T *)&hisam_entry, TRUE))
    {
        case L_HISAM_REPLACE:
            break;

        case L_HISAM_INSERT:
            /* should not go here
             * it is impossible to update an non-existent mac entry
             */
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_SetSecureAddrRowStatus] L_HISAM_SetRecord() should not return L_HISAM_INSERT");

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;

        case L_HISAM_CONFLICT:
        case L_HISAM_NO_BUFFER:
        default:
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[NETACCESS_OM_GetSecurePortSecurityControl] something wrong in hisam");

            NETACCESS_OM_LEAVE_CRITICAL_SECTION();
            NETACCESS_OM_HISAM_UNLOCK();
            return FALSE;
    }

    /* update the om entry
     */
    secure_mac_table[hisam_entry.mac_index - 1].addr_row_status = entry->addr_row_status;

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrAppliedToChipByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry applied_to_chip by mac_index
 * INPUT    : mac_index, applied_to_chip
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrAppliedToChipByIndex(UI32_T mac_index, BOOL_T applied_to_chip)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if mac_index valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecureAddrAppliedToChipByIndex] mac_index(%lu)", mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* update the om entry
     */
    secure_mac_table[mac_index - 1].mac_flag.applied_to_chip = (TRUE == applied_to_chip) ? 1 : 0;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry write_to_amtr by mac_index
 * INPUT    : mac_index, write_to_amtr
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex(UI32_T mac_index, BOOL_T write_to_amtr)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if mac_index valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex] mac_index(%lu)", mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* update the om entry
     */
    secure_mac_table[mac_index - 1].mac_flag.write_to_amtr = (TRUE == write_to_amtr) ? 1 : 0;

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex] mac_index(%lu) write to amtr(%s)",
            mac_index, (TRUE == write_to_amtr) ? "true" : "false");

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by mac_index (om & hisam)
 * INPUT:  mac_index
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteSecureAddressEntryByIndex(UI32_T mac_index)
{
    BOOL_T      ret;

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* delete by mac_index
     */
    ret = NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex(mac_index);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteSecureAddressEntryBySecureKey
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by key (om & hisam)
 * INPUT:  key
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteSecureAddressEntryBySecureKey(const NETACCESS_OM_SecureKey_T *key)
{
    BOOL_T      ret;

    NETACCESS_OM_HISAMentry_T   hisam_entry;


    NETACCESS_OM_HISAM_LOCK();
    /* get HISAM record by key
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)key, (UI8_T *)&hisam_entry);

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* delete by mac_index
     */
    ret = NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex(hisam_entry.mac_index);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllSecureAddress()
{
    UI32_T      index;

    /* clear hisam
     */
    NETACCESS_OM_HISAM_LOCK();
    L_HISAM_DeleteAllRecord(&netaccess_hisam_desc);
    NETACCESS_OM_HISAM_UNLOCK();

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* clear om's mac table
     */
    for (index = 0; NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE > index; ++index)
    {
        memset(&secure_mac_table[index], 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
        secure_mac_table[index].mac_index = index + 1;
        secure_mac_table[index].addr_row_status = NETACCESS_ROWSTATUS_DESTROY;
        /* there is no need to reset other fields */
    }

    /* update counter
     */
    for (index = 0; SYS_ADPT_TOTAL_NBR_OF_LPORT > index; ++index)
    {
        secure_port_table[index].number_addresses_stored = 0;
        secure_port_table[index].nbr_of_authorized_addresses = 0;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllLearnedSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned mac from om & hisam except manually configured
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllLearnedSecureAddress()
{
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecurePortEntry_T *port_entry;
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecureMacEntry_T  *mac_entry;

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memset(&key, 0, sizeof(NETACCESS_OM_SecureKey_T));

    /* get next HISAM record
     */
    while (TRUE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        /* let the key point to next record
         */
        key.lport = hisam_entry.lport;
        memcpy(key.secure_mac, hisam_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* check if mac_index valid
         */
        if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
        {
            /* should not go here
             */
            continue;
        }

        mac_entry = &secure_mac_table[hisam_entry.mac_index - 1];

        /* check if leant MAC
         */
        if ((1 == mac_entry->mac_flag.admin_configured_mac) ||
            (1 == mac_entry->mac_flag.is_mac_filter_mac))
        {
            /* don't delete a manually configured mac
             */
            continue;
        }

        /* delete from hisam
         */
        if ((FALSE == L_HISAM_DeleteRecord(&netaccess_hisam_desc, (UI8_T *)&key)) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
            (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(key.lport, &unit, &port, &trunk_id) &&
              SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(key.lport, &unit, &port, &trunk_id)))
#else
            (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(key.lport, &unit, &port, &trunk_id)))
#endif
        {
            /* should not go here
             */
            continue;
        }

        port_entry = &secure_port_table[key.lport - 1];

        /* update counter
         */
        if (TRUE == NETACCESS_OM_LocalIsMacEffectCounter(mac_entry))
        {
            --(port_entry->number_addresses_stored);
            if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry->authorized_status)
                --(port_entry->nbr_of_authorized_addresses);
        }

        if ((1 == mac_entry->mac_flag.auth_by_rada) ||
            (1 == mac_entry->mac_flag.authorized_by_dot1x))
        {
            --(port_entry->nbr_of_learn_authorized_addresses);
        }

        /* delete om entry
         */
        memset(mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
        mac_entry->mac_index = hisam_entry.mac_index;
        mac_entry->addr_row_status = NETACCESS_ROWSTATUS_DESTROY;
    }

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllSecureAddressByPort(UI32_T lport)
{
    NETACCESS_OM_SecurePortEntry_T *port_entry;
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecureMacEntry_T  *mac_entry;


    /* check if port is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_DeleteAllSecureAddressByPort] bad lport(%lu)", lport);

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    port_entry = &secure_port_table[lport - 1];

    /* prepare searching key
     */
    key.lport = port_entry->lport;
    memset(key.secure_mac, 0, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    while (TRUE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        /* let the key point to next record
         */
        key.lport = hisam_entry.lport;
        memcpy(key.secure_mac, hisam_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* check if mac_index valid
         */
        if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
        {
            /* should not go here
             */
            continue;
        }

        /* check if same lport
         */
        if (key.lport != port_entry->lport)
        {
            break;
        }

        /* delete from hisam
         */
        if (FALSE == L_HISAM_DeleteRecord(&netaccess_hisam_desc, (UI8_T *)&key))
        {
            /* should not go here
             */
            continue;
        }

        /* delete om entry
         */
        mac_entry = &secure_mac_table[hisam_entry.mac_index - 1];

        memset(mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
        mac_entry->mac_index = hisam_entry.mac_index;
        mac_entry->addr_row_status = NETACCESS_ROWSTATUS_DESTROY;
    }

    /* update counter
     */
    port_entry->number_addresses_stored = 0;
    port_entry->nbr_of_authorized_addresses = 0;
    port_entry->nbr_of_learn_authorized_addresses = 0;

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllLearnedSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all mac addresses from om & hisam except manually configured mac
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllLearnedSecureAddressByPort(UI32_T lport)
{
    NETACCESS_OM_SecurePortEntry_T *port_entry;
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecureMacEntry_T  *mac_entry;

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_DeleteAllLearnedSecureAddressByPort] bad lport(%lu)", lport);

        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    port_entry = &secure_port_table[lport - 1];

    /* prepare searching key
     */
    key.lport = port_entry->lport;
    memset(key.secure_mac, 0, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    while (TRUE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry))
    {
        /* let the key point to next record
         */
        key.lport = hisam_entry.lport;
        memcpy(key.secure_mac, hisam_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* check if mac_index valid
         */
        if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
        {
            /* should not go here
             */
            continue;
        }

        /* check if same lport
         */
        if (key.lport != port_entry->lport)
        {
            break;
        }

        mac_entry = &secure_mac_table[hisam_entry.mac_index - 1];

        /* don't delete a manually configured mac
         */
        if ((1 == mac_entry->mac_flag.admin_configured_mac) ||
            (1 == mac_entry->mac_flag.is_mac_filter_mac))
        {
            continue;
        }

        /* delete from hisam
         */
        if (FALSE == L_HISAM_DeleteRecord(&netaccess_hisam_desc, (UI8_T *)&key))
        {
            /* should not go here
             */
            continue;
        }

        /* update counter
         */
        if (TRUE == NETACCESS_OM_LocalIsMacEffectCounter(mac_entry))
        {
            --(port_entry->number_addresses_stored);
            if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry->authorized_status)
                --(port_entry->nbr_of_authorized_addresses);
        }

        if ((1 == mac_entry->mac_flag.auth_by_rada) ||
            (1 == mac_entry->mac_flag.authorized_by_dot1x))
        {
            --(port_entry->nbr_of_learn_authorized_addresses);
        }

        /* delete om entry
         */
        memset(mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
        mac_entry->mac_index = hisam_entry.mac_index;
        mac_entry->addr_row_status = NETACCESS_ROWSTATUS_DESTROY;
    }

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_SecureKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordBySecureKey(const NETACCESS_OM_SecureKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    BOOL_T      ret;


    /* check if input is valid
     */
    if ((NULL == key) ||
        (NULL == hisam_entry))
    {
        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    /* get HISAM record
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry);
    NETACCESS_OM_HISAM_UNLOCK();

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordBySecureKey(NETACCESS_OM_SecureKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    NETACCESS_OM_HISAM_LOCK();
    /* check if input is valid and get next HISAM record
     */
    if ((NULL == key) ||
        (NULL == hisam_entry) ||
        (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry)))
    {
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }
    NETACCESS_OM_HISAM_UNLOCK();

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
    {
        printf("\r\n[NETACCESS_OM_GetNextHisamRecordBySecureKey]\r\nkey(%lu, ", key->lport);
        NETACCESS_OM_LocalPrintMacAddress(key->secure_mac);
        printf(")\r\ndump hisam");
        NETACCESS_OM_LocalDumpHisamEntry(hisam_entry);
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* let the key point to next record
     */
    key->lport = hisam_entry->lport;
    memcpy(key->secure_mac, hisam_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

   NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DoesRecordExistInHisamBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified record exist or not
 * INPUT    : key
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_DoesRecordExistInHisamBySecureKey(const NETACCESS_OM_SecureKey_T *key)
{
    BOOL_T      ret;

    NETACCESS_OM_HISAMentry_T   hisam_entry;

    NETACCESS_OM_HISAM_LOCK();

    /* get HISAM record
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)key, (UI8_T *)&hisam_entry);

    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_OldestKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordByOldestKey(const NETACCESS_OM_OldestKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    BOOL_T      ret;


    /* check if input is valid
     */
    if ((NULL == key) ||
        (NULL == hisam_entry))
    {
        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    /* get HISAM record
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_OLDEST_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry);

    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByOldestKey(NETACCESS_OM_OldestKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid and get next HISAM record
     */
    if ((NULL == key) ||
        (NULL == hisam_entry) ||
        (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_OLDEST_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry)))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* let the key point to next record
     */
    key->lport = hisam_entry->lport;
    key->authorized_status = hisam_entry->authorized_status;
    key->record_time = hisam_entry->record_time;
    memcpy(key->secure_mac, hisam_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_ExpireKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordByExpireKey(const NETACCESS_OM_ExpireKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    BOOL_T      ret;


    /* check if input is valid
     */
    if ((NULL == key) ||
        (NULL == hisam_entry))
    {
        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    /* get HISAM record
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_EXPIRE_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry);

    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByExpireKey(NETACCESS_OM_ExpireKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    NETACCESS_OM_HISAM_LOCK();
    /* check if input is valid and get next HISAM record
     */
    if ((NULL == key) ||
        (NULL == hisam_entry) ||
        (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_EXPIRE_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry)))
    {
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* let the key point to next record
     */
    key->session_expire_time = hisam_entry->session_expire_time;
    key->lport = hisam_entry->lport;
    memcpy(key->secure_mac, hisam_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByMacKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByMacKey(NETACCESS_OM_MacAdrKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    NETACCESS_OM_HISAM_LOCK();

    /* check if input is valid and get next HISAM record
     */
    if ((NULL == key) ||
        (NULL == hisam_entry) ||
        (FALSE == L_HISAM_GetNextRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_MACADR_ADDRESS, (UI8_T *)key, (UI8_T *)hisam_entry)))
    {
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* let the key point to next record
     */
    memcpy(key->secure_mac, hisam_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);
    key->lport = hisam_entry->lport;

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : set the specific port's state machine
 * INPUT    : lport, state_machine
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetPortStateMachine(UI32_T lport, const NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == state_machine) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_SetPortStateMachine] null pointer or bad lport");

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* set to database
     * no need any checking here
     */
    memcpy(&secure_port_table[lport - 1].state_machine, state_machine, sizeof(NETACCESS_OM_StateMachine_T));

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's state machine
 * INPUT    : lport
 * OUTPUT   : state_machine
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetPortStateMachine(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == state_machine) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetPortStateMachine] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     * no need any checking here
     */
    memcpy(state_machine, &secure_port_table[lport - 1].state_machine, sizeof(NETACCESS_OM_StateMachine_T));

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetPortStateMachineRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's running_port_mode
 * INPUT    : lport
 * OUTPUT   : running_port_mode
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetPortStateMachineRunningPortMode(UI32_T lport, UI32_T *running_port_mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == running_port_mode) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetPortStateMachineRunningPortMode] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     * no need any checking here
     */
    *running_port_mode = secure_port_table[lport - 1].state_machine.running_port_mode;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineNewMacMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's new_mac_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineNewMacMsg(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ClearStateMachineNewMacMsg] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    secure_port_table[lport - 1].state_machine.port_security_sm.new_mac_msg = NULL;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineRadiusMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's radius_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineRadiusMsg(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ClearStateMachineRadiusMsg] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    secure_port_table[lport - 1].state_machine.port_security_sm.radius_msg = NULL;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineDot1xMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineDot1xMsg(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ClearStateMachineDot1xMsg] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    secure_port_table[lport - 1].state_machine.port_security_sm.dot1x_msg = NULL;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineDot1xLogonFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_logon flag of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineDot1xLogonFlag(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_ClearStateMachineDot1xLogonFlag] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* get from database
     */
    secure_port_table[lport - 1].state_machine.port_security_sm.event_bitmap.dot1x_logon = 0;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineMacAgeOutMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's macageout_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineMacAgeOutMsg(UI32_T lport)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    secure_port_table[lport - 1].state_machine.port_security_sm.macageout_msg = NULL;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_StopStateMachineDoAuthentication
 *-------------------------------------------------------------------------
 * PURPOSE  : state become to idle and turn off is_authenticating flag
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_StopStateMachineDoAuthentication(UI32_T lport)
{
    UI32_T      index;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_StopStateMachineDoAuthentication] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    index = lport - 1; /* to zero-based */

    /* check if port state machine is under authenticating
     */
    if (1 == secure_port_table[index].state_machine.port_security_sm.event_bitmap.is_authenticating)
    {
        /* in any port mode, SHOULD have this state: idle */
        secure_port_table[index].state_machine.port_security_sm.running_state = NETACCESS_STATE_IDLE;

        secure_port_table[index].state_machine.port_security_sm.event_bitmap.new_mac = 0;
        secure_port_table[index].state_machine.port_security_sm.event_bitmap.eap_packet = 0;
        secure_port_table[index].state_machine.port_security_sm.event_bitmap.reauth = 0;
        secure_port_table[index].state_machine.port_security_sm.event_bitmap.is_authenticating = 0;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsStateMachineAuthenticating
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific port's is_authenticating flag of PSEC state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsStateMachineAuthenticating(UI32_T lport)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_IsStateMachineAuthenticating] bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* check if port state machine is under authenticating
     */
    ret = (1 == secure_port_table[lport - 1].state_machine.port_security_sm.event_bitmap.is_authenticating);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacAuthenticatingMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is authenticating mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacAuthenticatingMac(UI32_T lport, const UI8_T *mac)
{
    BOOL_T      ret;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if ((NULL == mac) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_IsThisMacAuthenticatingMac] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    ret = FALSE;

    /* check if this MAC is under authenticating
     */
    if (1 == secure_port_table[lport - 1].state_machine.port_security_sm.event_bitmap.is_authenticating)
    {
        ret = (0 == memcmp(mac, secure_port_table[lport - 1].state_machine.port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN));
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacAuthorizedByDot1x
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified (lport, mac) is authorized by dot1x
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacAuthorizedByDot1x(UI32_T lport, const UI8_T *mac)
{
    BOOL_T      ret;
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if ((NULL == mac) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        return FALSE;
    }

    /* prepare secure key
     */
    key.lport = lport;
    memcpy(key.secure_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* get HISAM record
     */
    if ((FALSE == L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry)) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index)))
    {
        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* check if this MAC authorized by dot1x
     */
    ret = (1 == secure_mac_table[hisam_entry.mac_index - 1].mac_flag.authorized_by_dot1x);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacLearnedFromEapPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is learned from an EAP packet or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacLearnedFromEapPacket(UI32_T lport, const UI8_T *mac)
{
    BOOL_T      ret;
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if ((NULL == mac) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        return FALSE;
    }

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* prepare secure key
     */
    key.lport = lport;
    memcpy(key.secure_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get HISAM record
     */
    if ((FALSE == L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry)) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index)))
    {
        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* check if this MAC is from eap packet
     */
    ret = (1 == secure_mac_table[hisam_entry.mac_index - 1].mac_flag.eap_packet);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacUnauthorizedMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is unauthorized mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacUnauthorizedMac(UI32_T lport, const UI8_T *mac)
{
    BOOL_T      ret;
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;


    /* check if input is valid
     */
    if ((NULL == mac) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        return FALSE;
    }

    /* prepare secure key
     */
    key.lport = lport;
    memcpy(key.secure_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_OM_HISAM_LOCK();
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    /* get HISAM record
     */
    if ((FALSE == L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)&key, (UI8_T *)&hisam_entry)) ||
        (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index)))
    {
        NETACCESS_OM_LEAVE_CRITICAL_SECTION();
        NETACCESS_OM_HISAM_UNLOCK();
        return FALSE;
    }

    /* check if this MAC is unauthorized
     */
    ret = (NETACCESS_ROWSTATUS_ACTIVE != secure_mac_table[hisam_entry.mac_index - 1].authorized_status);

    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
    NETACCESS_OM_HISAM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncAmtrDeleteAddrCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase amtr delete address counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncAmtrDeleteAddrCounter()
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* increase counter
     */
    ++(per_system_statistic_data.demand_amtr_delete_addr_counter);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetDebugFlag(UI32_T debug_flag)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* set to database
     */
    netaccess_debug_flag = debug_flag;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get backdoor debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_OM_GetDebugFlag()
{
    /* get from database
     */
    return netaccess_debug_flag;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetDebugPrompt
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug prompt
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
const char* NETACCESS_OM_GetDebugPrompt(UI32_T flag)
{
    return "NETACCESS";
}

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_LocalCompLocManAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            new_element           -- the element to compare
 * OUTPUT   : None
 * RETUEN   : -1 if inlist_element < new_element,
 *             1 if inlist_element > new_element
 *             0 if inlist_element == new_element
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int NETACCESS_OM_LocalCompLocManAddrEntry(void* inlist_element, void* new_element)
{
    NETACCESS_OM_MacFilterEntry_T *element1, *element2;
    int result;

    element1 = (NETACCESS_OM_MacFilterEntry_T*)(inlist_element);
    element2 = (NETACCESS_OM_MacFilterEntry_T*)(new_element);

    result = memcmp(element1->filter_mac, element2->filter_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (result < 0)
        return -1;
    else if (result > 0)
        return 1;

    result = memcmp(element1->filter_mask, element2->filter_mask, SYS_ADPT_MAC_ADDR_LEN);
    if (result < 0)
        return -1;
    else if (result > 0)
        return 1;

    return 0;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsMacFilterExist
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the MAC filter entry is exist in the MAC filter table.
 * INPUT    : filter_id, mac_address, mask
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsMacFilterExist(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask)
{
    BOOL_T ret;
    NETACCESS_OM_MacFilterEntry_T   element;

    if (    (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
        ||  (filter_id < SYS_ADPT_NETACCESS_MIN_FILTER_ID)
       )
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memcpy(element.filter_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(element.filter_mask, mask, SYS_ADPT_MAC_ADDR_LEN);

    ret = L_SORT_LST_Get((void*)&filter_mac_table[filter_id], &element);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsMacFilterMatched
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether for recognised device by mac address.
 * INPUT:   filter_mac, filter_mask, mac
 * OUTPUT:  None.
 * RETURN:  TRUE - match; FALSE - no match
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsMacFilterMatched(UI8_T *filter_mac, UI8_T *filter_mask, UI8_T *mac)
{
    if((filter_mac[0] & filter_mask[0]) != (mac[0] & filter_mask[0]))
        return FALSE;
    if((filter_mac[1] & filter_mask[1]) != (mac[1] & filter_mask[1]))
        return FALSE;
    if((filter_mac[2] & filter_mask[2]) != (mac[2] & filter_mask[2]))
        return FALSE;
    if((filter_mac[3] & filter_mask[3]) != (mac[3] & filter_mask[3]))
        return FALSE;
    if((filter_mac[4] & filter_mask[4]) != (mac[4] & filter_mask[4]))
        return FALSE;
    if((filter_mac[5] & filter_mask[5]) != (mac[5] & filter_mask[5]))
        return FALSE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE  : Add the new MAC filer entry into the MAC filter table.
 *            Remove the old MAC filter entry from the MAC filter table.
 * INPUT    : filter_id, mac_address, mask, is_add
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask, BOOL_T is_add)
{
    BOOL_T ret = FALSE;

    if (    (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
        ||  (filter_id < SYS_ADPT_NETACCESS_MIN_FILTER_ID)
       )
    {
        return FALSE;
    }

    /* when add new filter entry,check if exist and add
     * when delete filter entry,check if exist and delete
     */
    if(TRUE == is_add)
    {
        /* check if filter entry exist
         */
        if(TRUE == NETACCESS_OM_IsMacFilterExist(filter_id, mac_address, mask))
        {
            /* filter entry exist, just return TRUE
             */
            return TRUE;
        }

        ret = NETACCESS_OM_AddMacFilter(filter_id, mac_address, mask);
    }
    else if(FALSE == is_add)
    {
        ret = NETACCESS_OM_DeleteMacFilter(filter_id, mac_address, mask);
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_AddMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE  : Add the new MAC filer entry into the MAC filter table.
 * INPUT    : filter_id, mac_address, mask
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_AddMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask)
{
    NETACCESS_OM_MacFilterEntry_T   element;
    UI8_T                           invalid_mac[SYS_ADPT_MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    BOOL_T                          ret;

    if (    (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
        ||  (filter_id < SYS_ADPT_NETACCESS_MIN_FILTER_ID)
       )
    {
        return FALSE;
    }

    if (TRUE == NETACCESS_OM_IsMacFilterMatched(mac_address, mask, invalid_mac))
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (TRUE == NETACCESS_OM_LocalIsMacFilterTableFull())
    {
         NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(element.filter_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(element.filter_mask, mask, SYS_ADPT_MAC_ADDR_LEN);

    ret = L_SORT_LST_Set((void*)&filter_mac_table[filter_id], &element);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE  : Remove the old MAC filter entry from the MAC filter table.
 * INPUT    : filter_id,mac_address,mask
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask)
{
    NETACCESS_OM_MacFilterEntry_T   element;
    BOOL_T                          ret;

    if (    (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
        ||  (filter_id < SYS_ADPT_NETACCESS_MIN_FILTER_ID)
       )
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memcpy(element.filter_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(element.filter_mask, mask, SYS_ADPT_MAC_ADDR_LEN);

    ret = L_SORT_LST_Delete((void*)&filter_mac_table[filter_id], &element);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNextMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next MAC filter entry
 * INPUT    : filter_id,mac_address,mask
 * OUTPUT   : filter_id,mac_address,mask
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextMacFilter(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask)
{
    UI32_T                          i;
    NETACCESS_OM_MacFilterEntry_T   element;
    BOOL_T                          ret;

    if (NULL == filter_id)
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memcpy(element.filter_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(element.filter_mask, mask, SYS_ADPT_MAC_ADDR_LEN);

    ret = FALSE;
    for (i = (*filter_id==0)?1:*filter_id; i<= SYS_ADPT_NETACCESS_MAX_FILTER_ID; i++)
    {
        if (TRUE == L_SORT_LST_Get_Next((void*)&filter_mac_table[i], &element))
        {
            *filter_id = i;
            memcpy(mac_address, element.filter_mac, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(mask, element.filter_mask, SYS_ADPT_MAC_ADDR_LEN);
            ret = TRUE;
            break;
        }
        memset(&element.filter_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
        memset(&element.filter_mask, 0, SYS_ADPT_MAC_ADDR_LEN);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNextMacFilterByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the next MAC filter entry that have the same filter ID
 * INPUT    : filter_id,mac_address,mask
 * OUTPUT   : mac_address,mask
 * RETURN   : TRUE/FALSE.
 * NOTES    : Using zero mac_address to get the first MAC filter entry
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextMacFilterByFilterId(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask)
{
    NETACCESS_OM_MacFilterEntry_T   element;
    BOOL_T                          ret;

    if (    (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
        ||  (filter_id < SYS_ADPT_NETACCESS_MIN_FILTER_ID)
       )
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memcpy(element.filter_mac, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(element.filter_mask, mask, SYS_ADPT_MAC_ADDR_LEN);

    ret = L_SORT_LST_Get_Next((void*)&filter_mac_table[filter_id], &element);

    if (TRUE == ret)
    {
        memcpy(mac_address, element.filter_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(mask, element.filter_mask, SYS_ADPT_MAC_ADDR_LEN);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetApplyMacFilterByPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get mac filter id which bind to port
 * INPUT    : ifindex
 * OUTPUT   : filter_id.
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetApplyMacFilterByPort(UI32_T ifindex, UI32_T *filter_id)
{
    /* check if input is valid
     */
    if(NULL == filter_id)
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    *filter_id = secure_port_table[ifindex-1].filter_id;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetApplyMacFilterByPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : bind mac filter id to port
 * INPUT    : ifindex,filter_id
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetApplyMacFilterByPort(UI32_T ifindex, UI32_T filter_id)
{
    if (filter_id > SYS_ADPT_NETACCESS_MAX_FILTER_ID)
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    secure_port_table[ifindex-1].filter_id = filter_id;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - BOOL_T NETACCESS_OM_LocalIsMacFilterTableFull
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check total existed MAC filter pair number
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : Return TRUE, means the table full, otherwise return FALSE
 *-----------------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_OM_LocalIsMacFilterTableFull()
{
    UI32_T filter_id;
    UI32_T total_mac_pair_num = 0;

    for (filter_id = SYS_ADPT_NETACCESS_MIN_FILTER_ID; filter_id <= SYS_ADPT_NETACCESS_MAX_FILTER_ID; filter_id++)
    {
        total_mac_pair_num += filter_mac_table[filter_id].nbr_of_element;

        if (SYS_ADPT_NETACCESS_TOTAL_NBR_OF_MAC_FILTER_ENTRY_PER_SYSTEM <= total_mac_pair_num)
        {
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            {
                printf("\r\n[%s] MAC Filter Table is full", __FUNCTION__);
            }
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_InitUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Initial database for Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_InitUnauthorizedMacCache(void)
{
    memset(&unauth_mac_cache_desc, 0, sizeof(unauth_mac_cache_desc));

    /* 1. Initialize OM hash table */
    unauth_mac_cache_desc.nbr_of_rec        = NETACCESS_OM_MAX_NBR_OF_UNAUTHORIZED_MAC_CACHE_ENTRY;
    unauth_mac_cache_desc.nbr_of_hash_bucket= NETACCESS_OM_UNAUTHORIZED_MAC_CACHE_HASH_BUCKET_SIZE;
    unauth_mac_cache_desc.key_offset[0]     = 0;
    unauth_mac_cache_desc.key_size[0]       = SYS_ADPT_MAC_ADDR_LEN;
    unauth_mac_cache_desc.record_size       = SYS_ADPT_MAC_ADDR_LEN;
    unauth_mac_cache_desc.hash_method       = L_HASH_HASH_METHOD_WORD_XOR;

    /* 2. Create OM hash table */
    if (FALSE == L_HASH_Create(&unauth_mac_cache_desc))
    {
        printf("\r\n[%s]: Create unauthorized MAC Database Error !", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_ClearUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Clear Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ClearUnauthorizedMacCache (void)
{
    BOOL_T  rc;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    rc = L_HASH_DeleteAll(&unauth_mac_cache_desc);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(rc);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_AddOneMac2UnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Add One MAC to Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_AddOneMac2UnauthorizedMacCache (UI8_T *src_mac_p)
{
    /* Local Variable Declaration
      */
    L_HASH_Index_T  index;
    BOOL_T  rc=FALSE;

    NETACCESS_OM_HISAM_LOCK();

    /* add address entry to hash table
     */
    rc = L_HASH_SetRecord(&unauth_mac_cache_desc, src_mac_p, &index);

    NETACCESS_OM_HISAM_UNLOCK();
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_IsMacExistInUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Check if one MAC is exist in Unauthorized MAC cache or not.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsMacExistInUnauthorizedMacCache (UI8_T *src_mac_p)
{
    BOOL_T  rc;

    NETACCESS_OM_HISAM_LOCK();

    rc = L_HASH_GetExactRecord (&unauth_mac_cache_desc, src_mac_p);

    NETACCESS_OM_HISAM_UNLOCK();
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_GetUnauthorizedMacCacheExpireTime
 * -------------------------------------------------------------------------
 * PURPOSE: get Unauthorized MAC expire time.
 * INPUT  : expire_time_p.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetUnauthorizedMacCacheExpireTime(UI32_T *expire_time_p)
{
    if(NULL == expire_time_p)
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    *expire_time_p = unauth_chache_expire_time;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_SetUnauthorizedMacCacheExpireTime
 * -------------------------------------------------------------------------
 * PURPOSE: set Unauthorized MAC expire time.
 * INPUT  : expire_time.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
void NETACCESS_OM_SetUnauthorizedMacCacheExpireTime(UI32_T expire_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    unauth_chache_expire_time = expire_time;
    NETACCESS_OM_LEAVE_CRITICAL_SECTION();
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_SetSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: set authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrVidCounterByIndex(UI32_T mac_index, UI32_T vid_counter)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad mac index(%lu)", __FUNCTION__, mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* update counter
     */
    om_entry->vlan_counter = vid_counter;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_GetSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: get authenticated MAC address VLAN counter.
 * INPUT  : mac_index.
 * OUTPUT : vid_counter.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddrVidCounterByIndex(UI32_T mac_index, UI32_T *vid_counter)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad mac index(%lu)", __FUNCTION__, mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* get counter
     */
    *vid_counter = om_entry->vlan_counter;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_IncSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: increase authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IncSecureAddrVidCounterByIndex(UI32_T mac_index)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad mac index(%lu)", __FUNCTION__, mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* update counter
     */
    om_entry->vlan_counter++;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_DecSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: decrease authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecSecureAddrVidCounterByIndex(UI32_T mac_index)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad mac index(%lu)", __FUNCTION__, mac_index);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[mac_index - 1];

    /* update counter
     */
    if(om_entry->vlan_counter > 0)
    {
        om_entry->vlan_counter--;
    }
    else
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_DecreaseSecureAddress
 * -------------------------------------------------------------------------
 * PURPOSE: decrease authenticated MAC address.
 * INPUT  : key
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecreaseSecureAddress(NETACCESS_OM_SecureKey_T *key)
{
    NETACCESS_OM_SecureMacEntry_T  *om_entry;
    BOOL_T ret = FALSE;
    NETACCESS_OM_HISAMentry_T      hisam_entry;

    /* check if input is valid
     */
    if(NULL == key)
    {
        return FALSE;
    }

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(key->lport))
    {
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    memset(&hisam_entry, 0, sizeof(NETACCESS_OM_HISAMentry_T));

    /* get HISAM record
     */
    ret = L_HISAM_GetRecord(&netaccess_hisam_desc, NETACCESS_KEY_INDEX_OF_SECURE_ADDRESS, (UI8_T *)key, (UI8_T *)&hisam_entry);

    if(FALSE == ret)
    {
        /* treat as success when the MAC doesn't exist in HISAM
         */
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(hisam_entry.mac_index))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    om_entry = &secure_mac_table[hisam_entry.mac_index - 1];

    /* check if hisam_entry.mac_index is correct
     */
    if((om_entry->lport != key->lport) ||
        (0 != memcmp(om_entry->secure_mac, key->secure_mac, SYS_ADPT_MAC_ADDR_LEN)))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* update counter
     */
    om_entry->vlan_counter--;

    if(om_entry->vlan_counter > 0)
    {
        ret = TRUE;
    }
    else if(om_entry->vlan_counter == 0)
    {
        /* vlan counter is zero,so delete mac entry
         */
        ret = NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex(hisam_entry.mac_index);
    }
    else
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_CreatePortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : create disablePortTemporarily timer
 * INPUT    : lport, expire_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_CreatePortDisableTimer(UI32_T lport, UI32_T expire_time)
{
    NETACCESS_OM_PortDisableTimer_T     *iter, *disable_timer;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    disable_timer = &port_disable_timer[lport - 1];

    if (TRUE == disable_timer->is_available)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] timer already existed(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE); /* treat it as succeeded */
    }

    disable_timer->is_available = TRUE;
    disable_timer->expire_time = expire_time;

    for (iter = tail_of_diable_timer; NULL != iter; iter = iter->prev)
    {
        if (iter->expire_time < disable_timer->expire_time)
            break;
    }

    if (NULL == iter)
    {
        if ((NULL == tail_of_diable_timer) || (NULL == head_of_diable_timer))
        {
            /* first one timer */
            disable_timer->prev = NULL;
            disable_timer->next = NULL;
            head_of_diable_timer = disable_timer;
            tail_of_diable_timer = disable_timer;
        }
        else
        {
            /* insert before head_of_diable_timer */
            disable_timer->prev = NULL;
            disable_timer->next = head_of_diable_timer;
            head_of_diable_timer->prev = disable_timer;
        }
    }
    else
    {
        /* insert after iter */
        disable_timer->prev = iter;
        disable_timer->next = iter->next;
        iter->next->prev = disable_timer;
        iter->next = disable_timer;
    }

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[%s] (%lu) expire time(%lu) done", __FUNCTION__, lport, expire_time);

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyFirstPortDisableTimer()
{
    NETACCESS_OM_PortDisableTimer_T     *disable_timer;

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NULL == head_of_diable_timer) /* has no disable timer */
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
    }

    disable_timer = head_of_diable_timer; /* destroy head */

    head_of_diable_timer = disable_timer->next;

    if (NULL == disable_timer->next) /* destroy tail */
        tail_of_diable_timer = disable_timer->prev;
    else
        disable_timer->next->prev = disable_timer->prev;

    disable_timer->is_available = FALSE;
    disable_timer->prev = NULL;
    disable_timer->next = NULL;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : get first disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : lport, expire_time
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetFirstPortDisableTimer(UI32_T *lport, UI32_T *expire_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == lport) || (NULL == expire_time) ||
        (NULL == head_of_diable_timer) || (FALSE == head_of_diable_timer->is_available))
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *lport = head_of_diable_timer->lport;
    *expire_time = head_of_diable_timer->expire_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the violation trap hold off time on this port.
 * INPUT:  lport, holdoff_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T holdoff_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    secure_port_table[lport - 1].trap_of_violation_holdoff_time = holdoff_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: get the violation trap hold off time on this port.
 * INPUT:  lport
 * OUTPUT: holdoff_time
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T *holdoff_time)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == holdoff_time) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *holdoff_time = secure_port_table[lport - 1].trap_of_violation_holdoff_time;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Set guest VLAN ID of the port
 * INPUT    : lport,vid.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : The range of guest VLAN ID is from 0 to MAX_networkAccessPortGuestVlan,
 *            and 0 means guest VLAN feature is disabled.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureGuestVlanId(UI32_T lport, UI32_T vid)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_GuestVlan_T *vlan_p = NETACCESS_OM_GuestVlan(lport);

    if (vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong port(%lu)", lport);
        return FALSE;
    }

    if (MAX_networkAccessPortGuestVlan < vid)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Invalid guest VLAN ID(%lu)", vid);
        return FALSE;
    }


    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    vlan_p->guest_vlan_id = vid;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return FALSE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get guest VLAN ID of the port
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid_p)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_GuestVlan_T *vlan_p = NETACCESS_OM_GuestVlan(lport);

    if (vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong port(%lu)", lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    *vid_p = vlan_p->guest_vlan_id;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    *vid_p = 0; /* disabled VID */
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetJoinRestrictedVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : Set join restricted VLAN status of the port
 * INPUT    : lport  -- lport index
 *            status -- TRUE, the port join to restricted VLAN/FALSE, no.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetJoinRestrictedVlanStatus(UI32_T lport, BOOL_T yes_no)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_GuestVlan_T *vlan_p = NETACCESS_OM_GuestVlan(lport);

    if (vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong port(%lu)", lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    vlan_p->applied_on_port = yes_no;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    return FALSE;
#endif

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetJoinRestrictedVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : Get join restricted VLAN status of the port
 * INPUT    : lport  -- lport index
 * OUTPUT   : status -- TRUE, the port join to restricted VLAN/FALSE, no.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetJoinRestrictedVlanStatus(UI32_T lport, BOOL_T *yes_no_p)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_GuestVlan_T *vlan_p = NETACCESS_OM_GuestVlan(lport);

    if (vlan_p == NULL)
    {
        NETACCESS_DBG1(NETACCESS_OM_DEBUG_OM_ERR, "Wrong port(%lu)", lport);
        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    *yes_no_p = vlan_p->applied_on_port;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
#else
    *yes_no_p = FALSE;
    return TRUE;
#endif

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 *            count -- max mac count
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    mac_auth_port_table[lport - 1].configured_number_addresses = count;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == count) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *count = mac_auth_port_table[lport - 1].configured_number_addresses;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the intrustion action of mac-authentication for the specified port.
 * INPUT    : lport  -- logic port number
 *            action -- intrusion action
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    mac_auth_port_table[lport - 1].intrusion_action = action;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == action) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *action = mac_auth_port_table[lport - 1].intrusion_action;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetMacAuthenticationEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get the mac authentication entry by the lport.
 * INPUT    : lport
 * OUTPUT   : entry : The mac-authentication port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMacAuthPortEntry(UI32_T lport, NETACCESS_OM_MacAuthPortEntry_T *entry)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == entry) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetMacAuthPortEntry] null pointer or bad lport(%lu)", lport);

        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, &mac_auth_port_table[lport - 1], sizeof(NETACCESS_OM_MacAuthPortEntry_T));

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

#ifdef SYS_CPNT_NETACCESS_AGING_MODE
#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC address aging mode.
 * INPUT    : mode -- MAC address aging mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_OM_SetMacAddressAgingMode(UI32_T mode)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    mac_addr_aging_mode = mode;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- MAC address aging mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_OM_GetMacAddressAgingMode(UI32_T *mode_p)
{
    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    if (mode_p == NULL)
    {
        NETACCESS_DBG(NETACCESS_OM_DEBUG_OM_ERR, "null pointer");
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *mode_p = mac_addr_aging_mode;
    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}
#endif/*#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)*/
#endif /* #ifdef SYS_CPNT_NETACCESS_AGING_MODE */

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionPtr
 * ---------------------------------------------------------------------
 * PURPOSE: Get pointer of link detection entry.
 * INPUT  : lport -- logic port number
 * OUTPUT : None.
 * RETURN : pointer of link detection entry
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
static NETACCESS_OM_LinkDetection_T* NETACCESS_OM_GetLinkDetectionPtr(UI32_T lport)
{
    NETACCESS_OM_SecurePortEntry_T *port_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    return (port_p)?&port_p->link_detection : NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port link detection status.
 * INPUT  : None.
 * OUTPUT : status_p -- VAL_networkAccessPortLinkDetectionStatus_enabled
 *                    VAL_networkAccessPortLinkDetectionStatus_disabled
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if ((NULL == status_p) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetLinkDetectionStatus] null pointer or bad lport(%lu)", lport);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    *status_p = (link_detection_p->enabled)?
        VAL_networkAccessPortLinkDetectionStatus_enabled:
        VAL_networkAccessPortLinkDetectionStatus_disabled;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port link detection status.
 * INPUT  : status -- VAL_networkAccessPortLinkDetectionStatus_enabled
 *                    VAL_networkAccessPortLinkDetectionStatus_disable
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionStatus(UI32_T lport, UI32_T status)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        return FALSE;
    }

    if (    (status != VAL_networkAccessPortLinkDetectionStatus_enabled)
        &&  (status != VAL_networkAccessPortLinkDetectionStatus_disabled)
    )
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] unknow status(%lu)", __FUNCTION__, status);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    link_detection_p->enabled =
        (status == VAL_networkAccessPortLinkDetectionStatus_enabled)?1:0;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection mode.
 * INPUT  : None.
 * OUTPUT  : mode_p -- VAL_networkAccessPortLinkDetectionMode_linkUp
 *                    VAL_networkAccessPortLinkDetectionMode_linkDown
 *                    VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if ((NULL == mode_p) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetLinkDetectionMode] null pointer or bad lport(%lu)", lport);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    if (link_detection_p->detect_linkup
        && !link_detection_p->detect_linkdown)
    {
        *mode_p = VAL_networkAccessPortLinkDetectionMode_linkUp;
    }
    else if (!link_detection_p->detect_linkup
        && link_detection_p->detect_linkdown)
    {
        *mode_p = VAL_networkAccessPortLinkDetectionMode_linkDown;
    }
    else if (link_detection_p->detect_linkup
        && link_detection_p->detect_linkdown)
    {
        *mode_p = VAL_networkAccessPortLinkDetectionMode_linkUpDown;
    }
    else
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set port link detection mode.
 * INPUT  : mode -- VAL_networkAccessPortLinkDetectionMode_linkUp
 *                    VAL_networkAccessPortLinkDetectionMode_linkDown
 *                    VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionMode(UI32_T lport, UI32_T mode)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        return FALSE;
    }

    if (    (mode != VAL_networkAccessPortLinkDetectionMode_linkUp)
        &&  (mode != VAL_networkAccessPortLinkDetectionMode_linkDown)
        &&  (mode != VAL_networkAccessPortLinkDetectionMode_linkUpDown)
    )
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] unknow mode(%lu)", __FUNCTION__, mode);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    switch(mode)
    {
        case VAL_networkAccessPortLinkDetectionMode_linkUp:
        link_detection_p->detect_linkup = 1;
        link_detection_p->detect_linkdown = 0;
        break;

        case VAL_networkAccessPortLinkDetectionMode_linkDown:
        link_detection_p->detect_linkup = 0;
        link_detection_p->detect_linkdown = 1;
        break;

        case VAL_networkAccessPortLinkDetectionMode_linkUpDown:
        link_detection_p->detect_linkup = 1;
        link_detection_p->detect_linkdown = 1;
        break;
    }


    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection action.
 * INPUT  : None.
 * OUTPUT  : action_p -- VAL_networkAccessPortLinkDetectionAciton_trap
 *                     VAL_networkAccessPortLinkDetectionAciton_shutdown
 *                     VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if ((NULL == action_p) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetLinkDetectionAction] null pointer or bad lport(%lu)", lport);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    if (link_detection_p->action_sendtrap
        && !link_detection_p->action_shutdown)
    {
        *action_p = VAL_networkAccessPortLinkDetectionAciton_trap;
    }
    else if (!link_detection_p->action_sendtrap
        && link_detection_p->action_shutdown)
    {
        *action_p = VAL_networkAccessPortLinkDetectionAciton_shutdown;
    }
    else if (link_detection_p->action_sendtrap
        && link_detection_p->action_shutdown)
    {
        *action_p = VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown;
    }
    else
    {
        NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection action.
 * INPUT  : None.
 * OUTPUT  : action -- VAL_networkAccessPortLinkDetectionAciton_trap
 *                     VAL_networkAccessPortLinkDetectionAciton_shutdown
 *                     VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionAction(UI32_T lport, UI32_T action)
{
    NETACCESS_OM_LinkDetection_T *link_detection_p = NULL;

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] null pointer or bad lport(%lu)", __FUNCTION__, lport);

        return FALSE;
    }

    if (    (action != VAL_networkAccessPortLinkDetectionAciton_trap)
        &&  (action != VAL_networkAccessPortLinkDetectionAciton_shutdown)
        &&  (action != VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown)
    )
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] unknow action(%lu)", __FUNCTION__, action);

        return FALSE;
    }

    NETACCESS_OM_ENTER_CRITICAL_SECTION();
    link_detection_p = NETACCESS_OM_GetLinkDetectionPtr(lport);

    switch(action)
    {
        case VAL_networkAccessPortLinkDetectionAciton_trap:
        link_detection_p->action_sendtrap = 1;
        link_detection_p->action_shutdown = 0;
        break;

        case VAL_networkAccessPortLinkDetectionAciton_shutdown:
        link_detection_p->action_sendtrap = 0;
        link_detection_p->action_shutdown = 1;
        break;

        case VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown:
        link_detection_p->action_sendtrap = 1;
        link_detection_p->action_shutdown = 1;
        break;
    }

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */


/* LOCAL SUBPROGRAM BODIES */
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecurePortEntryPtr
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get entry pointer of secure port table
 * INPUT    : lport
 * OUTPUT   : None.
 * RETURN   : entry pointer of secure port table
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
static NETACCESS_OM_SecurePortEntry_T* NETACCESS_OM_LocalGetSecurePortEntryPtr(UI32_T lport)
{
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        return NULL;
    }

    return &secure_port_table[lport - 1];
}

static
NETACCESS_OM_GuestVlan_T* NETACCESS_OM_GuestVlan(
    UI32_T lport
    )
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_SecurePortEntry_T *port_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    return port_p?(&port_p->guest_vlan):NULL;
#else
    return NULL;
#endif
}

static
NETACCESS_OM_DynamicVlan_T* NETACCESS_OM_DynamicVlan(UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_SecurePortEntry_T *port_p = NETACCESS_OM_LocalGetSecurePortEntryPtr(lport);

    return port_p?(&port_p->dynamic_vlan):NULL;
#else
    return NULL;
#endif
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalCreateSecureAddressEntry
 * ---------------------------------------------------------------------
 * PURPOSE: create mac entry (om & hisam)
 * INPUT:  mac_entry (key: lport + mac)
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if existed, return FALSE
 *         mac_index, add_on_what_port_mode will be ignore
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_OM_LocalCreateSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *mac_entry)
{
    UI32_T      index, unit, port, trunk_id;

    NETACCESS_OM_SecureMacEntry_T  *new_mac_entry;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_SecurePortEntry_T *port_entry;

    /* check if input is valid
     */
    if ((NULL == mac_entry) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id)) ||
#else
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id)) ||
#endif
        (NETACCESS_OM_IS_BAD_ROW_STATUS(mac_entry->addr_row_status)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] input is invalid", __FUNCTION__);
        return FALSE;
    }

    /* find an un-used entry
     */
    new_mac_entry = NULL;
    for (index = 0; NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE > index; ++index)
    {
        /* check if entry un-used
         */
        if (NETACCESS_ROWSTATUS_DESTROY != secure_mac_table[index].addr_row_status)
            continue;

        new_mac_entry = &secure_mac_table[index];
        break;
    }

    /* there is no free entry so can't create
     */
    if (NETACCESS_OM_ADPT_MAX_ADDR_TABLE_SIZE <= index)
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] there is no free entry so can't create", __FUNCTION__);

        return FALSE;
    }

    /* try to create an entry in hisam
     */
    hisam_entry.lport = mac_entry->lport;
    hisam_entry.authorized_status = mac_entry->authorized_status;
    hisam_entry.record_time = mac_entry->record_time;
    hisam_entry.session_expire_time = mac_entry->session_expire_time;

    /* can't use mac_entry's mac_index
     */
    hisam_entry.mac_index = new_mac_entry->mac_index;
    memcpy(hisam_entry.secure_mac, mac_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* L_HISAM_SetRecord(FALSE). do not replace if record already existed
     */
    switch (L_HISAM_SetRecord(&netaccess_hisam_desc, (UI8_T *)&hisam_entry, FALSE))
    {
        case L_HISAM_CONFLICT: /* already existed */
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[%s] L_HISAM_SetRecord() return L_HISAM_CONFLICT", __FUNCTION__);

            return FALSE;

        case L_HISAM_INSERT:
            break;

        case L_HISAM_NO_BUFFER: /* something wrong */
        case L_HISAM_REPLACE:
        default:
            if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
                printf("\r\n[%s] L_HISAM_SetRecord() failed", __FUNCTION__);

            return FALSE;
    }

    port_entry = &secure_port_table[mac_entry->lport - 1];

    /* mac_index must be reserved
     */
    index = new_mac_entry->mac_index;
    memcpy(new_mac_entry, mac_entry, sizeof(NETACCESS_OM_SecureMacEntry_T));
    new_mac_entry->mac_index = index;

    /* update counter
     */
    if (TRUE == NETACCESS_OM_LocalIsMacEffectCounter(new_mac_entry))
    {
        ++(port_entry->number_addresses_stored);
        if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry->addr_row_status)
            ++(port_entry->nbr_of_authorized_addresses);
    }

    if ((1 == new_mac_entry->mac_flag.auth_by_rada) ||
        (1 == new_mac_entry->mac_flag.authorized_by_dot1x))
    {
        ++(port_entry->nbr_of_learn_authorized_addresses);
    }

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[NETACCESS_OM_LocalCreateSecureAddressEntry] done");

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry (om & hisam)
 * INPUT:  mac_index
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex(UI32_T mac_index)
{
    UI32_T      unit, port, trunk_id;

    NETACCESS_OM_SecureMacEntry_T  *mac_entry;
    NETACCESS_OM_SecureKey_T    key;

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_MAC_INDEX(mac_index))
        return FALSE;

    mac_entry = &secure_mac_table[mac_index - 1];

    /* check if lport is valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry->lport, &unit, &port, &trunk_id))
#endif
        return FALSE;

    /* prepare primary key for hisam deletion
     */
    key.lport = mac_entry->lport;
    memcpy(key.secure_mac, mac_entry->secure_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* delete HISAM record
     */
    if (FALSE == L_HISAM_DeleteRecord(&netaccess_hisam_desc, (UI8_T *)&key))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex] L_HISAM_DeleteRecord() failed");

        return FALSE;
    }

    /* update counter
     */
    if (TRUE == NETACCESS_OM_LocalIsMacEffectCounter(mac_entry))
    {
        --secure_port_table[mac_entry->lport - 1].number_addresses_stored;
        if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry->addr_row_status)
            --secure_port_table[mac_entry->lport - 1].nbr_of_authorized_addresses;
    }

    if ((1 == mac_entry->mac_flag.auth_by_rada) ||
        (1 == mac_entry->mac_flag.authorized_by_dot1x))
    {
        --(secure_port_table[mac_entry->lport - 1].nbr_of_learn_authorized_addresses);
    }

    /* delete om entry
     */
    memset(mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_entry->mac_index = mac_index;
    mac_entry->addr_row_status = NETACCESS_ROWSTATUS_DESTROY;

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[NETACCESS_OM_LocalDeleteSecureAddressEntryByIndex] mac_index(%lu) done", mac_index);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalIsMacEffectCounter
 * ---------------------------------------------------------------------
 * PURPOSE  : whether mac_entry effect counter or not
 * INPUT    : mac_entry
 * OUTPUT   : none
 * RETURN   : TRUE - yes / FALSE - no
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T NETACCESS_OM_LocalIsMacEffectCounter(NETACCESS_OM_SecureMacEntry_T *mac_entry)
{
    /* check if input is valid
     */
    if (NULL == mac_entry)
    {
        return FALSE;
    }

    /* check if hidden mac
     * "hidden MACs" are invisible for users and can't write Learned-PSEC to cip (AMTR).
     * As a result, they don't effect on counter.
     */
/*    if((NETACCESS_PORTMODE_USER_LOGIN == mac_entry->add_on_what_port_mode) &&
        (1 == mac_entry->mac_flag.is_hidden_mac))
    {
        return FALSE;
    }*/

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalGetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : qty
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_OM_LocalGetMinimumSecureAddresses(UI32_T port_mode, UI32_T *qty)
{
    /* check if input is valid
     */
    if (NULL == qty)
        return FALSE;

    /* check port mode to get value
     */
    *qty = netaccess_om_port_mode_attribute_ar[port_mode-1].minimum_secure_address;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalGetUsableSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maximum value that secureNumberAddresses can be set to.
 * INPUT:  usable_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_OM_LocalGetUsableSecureAddress(UI32_T lport, UI32_T *qty)
{
    /* check if input is valid
     */
    if ((NULL == qty) || NETACCESS_OM_IS_BAD_LPORT_NO(lport))
        return FALSE;

    *qty = SYS_ADPT_NETACCESS_MAX_NBR_OF_SECURE_ADDRESSES_PER_PORT;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalDumpHisamEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : dump hisam entry
 * INPUT    : hisam_entry
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void NETACCESS_OM_LocalDumpHisamEntry(NETACCESS_OM_HISAMentry_T *hisam_entry)
{
    /* check if input is valid
     */
    if (NULL == hisam_entry)
        return;

    printf("\r\n\t*lport(%lu) authorized_status(%lu)",
        hisam_entry->lport, hisam_entry->authorized_status);

    printf("\r\n\t*record_time(%lu) session_expire_time(%lu) mac_index(%lu)",
        hisam_entry->record_time, hisam_entry->session_expire_time, hisam_entry->mac_index);

    printf("\r\n\t*secure_mac(%02x-%02x-%02x-%02x-%02x-%02x)",
        hisam_entry->secure_mac[0], hisam_entry->secure_mac[1], hisam_entry->secure_mac[2],
        hisam_entry->secure_mac[3], hisam_entry->secure_mac[4], hisam_entry->secure_mac[5]);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_LocalPrintMacAddress
 * ---------------------------------------------------------------------
 * PURPOSE: printf mac address as string
 * INPUT:  addr.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_OM_LocalPrintMacAddress(UI8_T *addr)
{
    /* check if input is valid
     */
    if (NULL == addr)
        return;

    printf("%02x-%02x-%02x-%02x-%02x-%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DuplicatePortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : duplicate eap data in om by lport
 * INPUT    : lport, eap_data
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_DuplicatePortEapData(UI32_T lport, const NETACCESS_EAP_DATA_T *eap_data)
{
    UI32_T      index;

    NETACCESS_PortEapData_T      *port_eap_data_entry;

    if ((NULL == eap_data) ||
        (NULL == eap_data->pkt_data) ||
        (0 >= eap_data->pkt_length) ||
        (NETACCESS_OM_IS_BAD_LPORT_NO(lport)))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_DuplicatePortEapData] null pointer or bad lport(%lu)", lport);

        return FALSE;
    }

    index = lport - 1; /* to zero-based */

    if (NETACCESS_OM_MAX_NBR_OF_DUP_EAP_PER_PORT <= port_eap_data_store_ar[index].entry_counter)
    {
        /* buffer full but don't treat it as an error here
           because it is a kind of silently discarding
        */
        return TRUE;
    }

    /* allocate memory */
    port_eap_data_entry = (NETACCESS_PortEapData_T*)L_MM_Malloc(
        sizeof(NETACCESS_EAP_DATA_T),
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_DUPLICATEPORTEAPDATA));

    if (NULL == port_eap_data_entry)
    {
        return FALSE;
    }

    memset(port_eap_data_entry, 0, sizeof(NETACCESS_PortEapData_T));

    /* copy content of eap_data */
    memcpy(&port_eap_data_entry->eap_data, eap_data, sizeof(NETACCESS_EAP_DATA_T));

    port_eap_data_entry->eap_data.pkt_data = (UI8_T*)L_MM_Malloc(
        eap_data->pkt_length,
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_DUPLICATEPORTEAPDATA));

    if (NULL == port_eap_data_entry->eap_data.pkt_data)
    {
        L_MM_Free(port_eap_data_entry);
        return FALSE;
    }

    memcpy(port_eap_data_entry->eap_data.pkt_data, eap_data->pkt_data, eap_data->pkt_length);

    /* append to the list tail */
    if ((NULL == port_eap_data_store_ar[index].head_of_eap_data_list) ||
        (NULL == port_eap_data_store_ar[index].tail_of_eap_data_list))
    {
        /* first entry */
        port_eap_data_entry->prev = NULL;
        port_eap_data_entry->next = NULL;
        port_eap_data_store_ar[index].head_of_eap_data_list = port_eap_data_entry;
        port_eap_data_store_ar[index].tail_of_eap_data_list = port_eap_data_entry;
    }
    else
    {
        /* insert after tail */
        port_eap_data_entry->prev = port_eap_data_store_ar[index].tail_of_eap_data_list;
        port_eap_data_entry->next = NULL;
        port_eap_data_store_ar[index].tail_of_eap_data_list->next = port_eap_data_entry;
        port_eap_data_store_ar[index].tail_of_eap_data_list = port_eap_data_entry;
    }

    ++(port_eap_data_store_ar[index].entry_counter);

    if (NETACCESS_OM_DEBUG_OM_TRC & netaccess_debug_flag)
        printf("\r\n[NETACCESS_OM_DuplicatePortEapData] (%lu) done", lport);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy the first eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyFirstPortEapPacket(UI32_T lport)
{
    UI32_T      index;

    NETACCESS_PortEapData_T      *port_eap_data_entry;

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_DestroyFirstPortEapPacket] bad lport(%lu)", lport);

        return;
    }

    index = lport - 1; /* to zero-based */

    port_eap_data_entry = port_eap_data_store_ar[index].head_of_eap_data_list; /* destroy head */
    if (NULL == port_eap_data_entry) /* doesn't have any port eap data */
    {
        return;
    }

    port_eap_data_store_ar[index].head_of_eap_data_list = port_eap_data_entry->next;

    if (NULL == port_eap_data_entry->next) /* destroy tail */
        port_eap_data_store_ar[index].tail_of_eap_data_list = port_eap_data_entry->prev;
    else
        port_eap_data_store_ar[index].tail_of_eap_data_list->next->prev = port_eap_data_entry->prev;

    if (NULL != port_eap_data_entry->eap_data.pkt_data)
        L_MM_Free(port_eap_data_entry->eap_data.pkt_data);

    L_MM_Free(port_eap_data_entry);

    if (0 < port_eap_data_store_ar[index].entry_counter)
        --(port_eap_data_store_ar[index].entry_counter);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyAllPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy all eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyAllPortEapData(UI32_T lport)
{
    UI32_T      index;

    NETACCESS_PortEapData_T  *iter, *next_iter;


    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_DestroyAllPortEapData] bad lport(%lu)", lport);

        return;
    }

    index = lport - 1; /* to zero-based */

    for (iter = port_eap_data_store_ar[index].head_of_eap_data_list; NULL != iter; iter = next_iter)
    {
        next_iter = iter->next;

        if (NULL != iter->eap_data.pkt_data)
            L_MM_Free(iter->eap_data.pkt_data);

        L_MM_Free(iter);
    }

    memset(&port_eap_data_store_ar[index], 0, sizeof(NETACCESS_PortEapData_T));
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : get the first eap data by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : if succeeded, return a eap data; NULL - failed
 * NOTES    : MUST call NETACCESS_OM_FreeEapData() to free memory
 *-------------------------------------------------------------------------*/
NETACCESS_EAP_DATA_T *NETACCESS_OM_GetFirstPortEapData(UI32_T lport)
{
    UI32_T      index;

    NETACCESS_EAP_DATA_T    *eap_data;

    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[NETACCESS_OM_GetFirstPortEapData] bad lport(%lu)", lport);

        return NULL;
    }

    index = lport - 1; /* to zero-based */

    if (NULL == port_eap_data_store_ar[index].head_of_eap_data_list) /* doesn't have any port eap data */
    {
        return NULL;
    }

    /* allocate memory */
    eap_data = (NETACCESS_EAP_DATA_T*)L_MM_Malloc(
        sizeof(NETACCESS_EAP_DATA_T),
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_GETFIRSTPORTEAPDATA));

    if (NULL == eap_data)
    {
        return NULL;
    }

    memset(eap_data, 0, sizeof(NETACCESS_EAP_DATA_T));

    /* copy content of first entry */
    memcpy(eap_data, &port_eap_data_store_ar[index].head_of_eap_data_list->eap_data, sizeof(NETACCESS_EAP_DATA_T));

    eap_data->pkt_data = (UI8_T*)L_MM_Malloc(
        eap_data->pkt_length,
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_GETFIRSTPORTEAPDATA));
        /* use pkt_length, MUST memcpy() first */

    if (NULL == eap_data->pkt_data)
    {
        L_MM_Free(eap_data);
        return NULL;
    }

    memcpy(eap_data->pkt_data, port_eap_data_store_ar[index].head_of_eap_data_list->eap_data.pkt_data, eap_data->pkt_length); /* use pkt_length, MUST memcpy() first */

    return eap_data;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_FreeEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : free eap data
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_FreeEapData(NETACCESS_EAP_DATA_T **eap_data)
{
    if ((NULL == eap_data) || (NULL == *eap_data))
    {
        return;
    }

    if (NULL != (*eap_data)->pkt_data)
        L_MM_Free((*eap_data)->pkt_data);

    L_MM_Free(*eap_data);

    *eap_data = NULL;
}

static void NETACCESS_OM_LocalInitPortModeAttribute(void)
{
    UI32_T port_mode;

    memset(netaccess_om_port_mode_attribute_ar, 0, sizeof(NETACCESS_OM_PortModeAttribute_T)*(NETACCESS_PORTMODE_MAX-1));

    port_mode = NETACCESS_PORTMODE_NO_RESTRICTIONS;
    netaccess_om_port_mode_attribute_ar[port_mode-1].minimum_secure_address = 0;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_shrink_upper_mac_count = 1;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_create_secure_mac_address = 1;

    port_mode = NETACCESS_PORTMODE_MAC_AUTHENTICATION;
    netaccess_om_port_mode_attribute_ar[port_mode-1].minimum_secure_address = 1;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_shrink_upper_mac_count = 0;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_create_secure_mac_address = 0;

    port_mode = NETACCESS_PORTMODE_PORT_SECURITY;
    netaccess_om_port_mode_attribute_ar[port_mode-1].minimum_secure_address = 0;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_shrink_upper_mac_count = 0;
    netaccess_om_port_mode_attribute_ar[port_mode-1].is_allow_create_secure_mac_address = 0;

    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_UpdateCookie
 * ---------------------------------------------------------------------
 * PURPOSE: To transmit cookie to next CSC.
 * INPUT  : lport -- port number
 *          cookie_p -- callback data
 * OUTPUT : None.
 * RETURN : TRUE  -- succeeded
 *          FALSE -- failed
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_UpdateCookie(
    UI32_T lport,
    void *cookie_p)
{
    UI32_T      index = 0;

    /* check if input is valid
     */
    if (NETACCESS_OM_IS_BAD_LPORT_NO(lport))
    {
        if (NETACCESS_OM_DEBUG_OM_ERR & netaccess_debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);

        return FALSE;
    }

    index = lport - 1; /* to zero-based */

    NETACCESS_OM_ENTER_CRITICAL_SECTION();

    secure_port_table[index].state_machine.port_security_sm.cookie = cookie_p;

    NETACCESS_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
