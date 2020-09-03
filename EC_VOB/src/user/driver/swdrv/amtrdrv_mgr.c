/*-------------------------------------------------------------------------
 * MODULE NAME: AMTRDRV_MGR.C
 *-------------------------------------------------------------------------
 * PURPOSE: To manage AMTRDRV Hash Table.
 *
 * NOTES:
 *
 * Modification History:
 *      Date          Modifier,    Reason
 *      ------------------------------------------------------------------
 *      08-31-2004    MIKE_YEH     create
 *      04-29-2013    Charlie Chen Add new mac source type "security".
 *                                 The mac entry with mac source type
 *                                 "security" indicates the mac has been
 *                                 authenticated by a security protocol.
 *
 * COPYRIGHT(C)         Accton Corporation, 2004
 *------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "amtrdrv_mgr.h"
#include "amtrdrv_om.h"
#include "amtr_type.h"
#include "l_hash.h"
#include "l_mm.h"
#include "dev_amtrdrv.h"
#include "dev_amtrdrv_pmgr.h"
#include "swdrv.h"
#include "swdrv_type.h"
#include "hrdrv.h"
#include "lan.h"
#include "stktplg_pom.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_module.h"
#include "sysfun.h"
#include "xstp_type.h"
#if (SYS_CPNT_SYSCALLBACK == TRUE)
#include "sys_callback.h"
#endif
#include "backdoor_mgr.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_type.h"
#endif
/*-----------------
 * LOCAL CONSTANTS
 *-----------------*/

/* NAME CONSTANT DECLARATIONS
 */

#define CHECK_MAC_IS_NULL(mac) ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])==0)  /*added by Jinhua.Wei,to remove warning*/
#define SYSFUN_USE_CSC(a)
#define SYSFUN_RELEASE_CSC()

/* debug message format */
#define BACKDOOR_OPEN
#define AMTRDRV_MGR_DEBUG
#define AMTRDRV_MGR_DEBUG_DBGMSG                    0x01
#define AMTRDRV_MGR_DEBUG_ADDR_CONTENT              0x02

/* task event definition */
#define AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE     BIT_0    /* enter transition mode event*/
#define AMTRDRV_MGR_EVENT_TIMER                     BIT_1    /* timer event         */
#define AMTRDRV_MGR_EVENT_ADDRESS_OPERATION         BIT_2

/* Event ID definition -- This is for sync queue dequeue or callback queue using */
#define AMTRDRV_MGR_LIFETIME_OTHER_BIT              BIT_0
#define AMTRDRV_MGR_LIFETIME_INVALID_BIT            BIT_1
#define AMTRDRV_MGR_LIFETIME_PERMANENT_BIT          BIT_2
#define AMTRDRV_MGR_LIFETIME_DELETE_ON_RESET_BIT    BIT_3
#define AMTRDRV_MGR_LIFETIME_DELETE_ON_TIMEOUT_BIT  BIT_4
#define AMTRDRV_MGR_SOURCE_INTERNAL_BIT             BIT_5
#define AMTRDRV_MGR_SOURCE_INVALID_BIT              BIT_6
#define AMTRDRV_MGR_SOURCE_LEARN_BIT                BIT_7
#define AMTRDRV_MGR_SOURCE_SELF_BIT                 BIT_8
#define AMTRDRV_MGR_SOURCE_CONFIG_BIT               BIT_9
#define AMTRDRV_MGR_SOURCE_SECURITY_BIT             BIT_10
#define AMTRDRV_MGR_SOURCE_MLAG_BIT                 BIT_11

/*  #number of entries need to be processed one time when task is running*/
#define AMTRDRV_MGR_MAX_NUM_JOB_PROCESS             (AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET*AMTRDRV_TYPE_NUM_ISC_IN_PROCESS)
#define AMTRDRV_MGR_MAX_NUM_NA_PROCESS              (AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET*AMTRDRV_TYPE_NUM_ISC_IN_PROCESS)   /* master processes NA numbers per one time */
#define AMTRDRV_MGR_MAX_NUM_NA_SENDING              (AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET*AMTRDRV_TYPE_NUM_ISC_IN_PROCESS)   /* slave processes NA numbers per one time */
#define AMTRDRV_MGR_MAX_NUM_AGING_PROCESS           (AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET*AMTRDRV_TYPE_NUM_ISC_IN_PROCESS)
#define AMTRDRV_MGR_MAX_NUM_AGEOUT_PROCESS          (AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET*AMTRDRV_TYPE_NUM_ISC_IN_PROCESS)

#if (SYS_CPNT_STACKING == TRUE)
/* constants needed when using remote service calls */
#define AMTRDRV_MGR_ISC_TIMEOUT                     1000
#define AMTRDRV_MGR_ISC_TRY_COUNT                   5
#define AMTRDRV_MGR_ISC_TRY_FOREVER                 0
#define AMTRDRV_MGR_DIRECT_CALL_POLL_ID             1
#define AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID           2
#endif /*SYS_ADPT_INCLUDE_STACKING*/

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
#define AMTRDRV_MGR_ALL_DEVICE_ID                         -1          /* -1 -> all device */
#endif

#ifdef SYS_ADPT_AMTR_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN
#define AMTRDRV_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN SYS_ADPT_AMTR_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN
#else
#define AMTRDRV_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define IS_TRUNK(ifindex)                       (ifindex >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER &&   \
                                                 ifindex <  SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM )

#define AMTRDRV_CONVERT_TO_ARL_ENTRY_PRIORITY(pri)  ((pri) == AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY ? \
                                                        SWDRV_TYPE_L2_ADDRESS_WITHOUT_PRIORITY : (pri))

#define AMTRDRV_CONVERT_TO_ARL_ENTRY_ACTION(addr_entry_p, arl_entry_p) \
    switch ((addr_entry_p)->action)                                    \
    {                                                                  \
        case AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_SA_MATCH:             \
            (arl_entry_p)->discard_by_sa_match = TRUE;                 \
            (arl_entry_p)->discard_by_da_match = FALSE;                \
            break;                                                     \
        case AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_DA_MATCH:             \
            (arl_entry_p)->discard_by_sa_match = FALSE;                \
            (arl_entry_p)->discard_by_da_match = TRUE;                 \
            break;                                                     \
        default:                                                       \
            (arl_entry_p)->discard_by_sa_match = FALSE;                \
            (arl_entry_p)->discard_by_da_match = FALSE;                \
    }

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AMTRDRV_MGR_ACTION_ADD = 1,
    AMTRDRV_MGR_ACTION_DEL = 2
} AMTRDRV_MGR_ACTION_E; /* enum value for AMTRDRV_MGR_ACTION_T */
typedef UI8_T AMTRDRV_MGR_ACTION_T;

#if (SYS_CPNT_STACKING == TRUE)
enum
{
    AMTRDRV_MGR_SET_AGING_TIME = 0,                   /* master notify slave */
    AMTRDRV_MGR_AGING_OUT,                            /* slave notify master */
    AMTRDRV_MGR_SET_ADDR_LIST,                        /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_LIST,                     /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_DIRECTLY,                 /* master notify slave */
    AMTRDRV_MGR_DELETE_ALL_ADDR,                      /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME,              /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE,                /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_PORT,                  /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME_N_PORT,       /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE_N_PORT,         /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID,                   /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME_N_PORT, /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE_N_PORT,   /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME,        /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE,          /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT,            /* master notify slave */
    AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT_EXCEPT_CERTAIN_ADDR, /* master notify slave */
    AMTRDRV_MGR_CHANGE_PORT_LIFE_TIME,                /* master notify slave */
    AMTRDRV_MGR_PROCESS_NEW_ADDR,
    AMTRDRV_MGR_MAX_NUM_OF_DIRECT_CALL_SERVICE
}AMTRDRV_MGR_DirectCallServiceID_E; /* enum value for AMTRDRV_MGR_DirectCallServiceID_T */
typedef UI16_T AMTRDRV_MGR_DirectCallServiceID_T;

typedef enum
{
    AMTRDRV_MGR_SET_ADDR_TO_LOCAL_MODULE = 0,             /* Main board to notify its option module to set address */
    AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE,     /* Main board to notify its option module to check the address hit bit value */
    AMTRDRV_MGR_HANDLE_MODULE_HOT_INSERTION,          /* Main board to notify new insertion module */
    AMTRDRV_MGR_SET_ADDR_DIRECTLY,                    /* master notify slave */
    AMTRDRV_MGR_CREATE_MULTICAST_ADDR,                /* master notify slave */
    AMTRDRV_MGR_DESTROY_MULTICAST_ADDR,               /* master notify slave */
    AMTRDRV_MGR_SET_MULTICAST_PORT_MEMBER,            /* master notify slave */
    AMTRDRV_MGR_MAX_NUM_OF_CALL_BY_AGENT_SERVICE
}AMTRDRV_MGR_CallByAgentServiceID_E; /* enum value for AMTRDRV_MGR_CallByAgentServiceID_T */
typedef UI16_T AMTRDRV_MGR_CallByAgentServiceID_T;

struct mstp_amtr_callback{
    UI16_T ifindex;
    UI8_T  life_time;
    XSTP_MGR_MstpInstanceEntry_T mstp_entry;
};

/* If anyone want to change structure "AMTRDRV_MGR_ISCBuffer_T",
 * AMTRDRV_TYPE_ISC_HEADER(in amtr_type.h) must be updated.
 */
typedef struct
{
    AMTRDRV_MGR_DirectCallServiceID_T    service_id;
    UI16_T                               type;
    union
    {
        UI32_T                  aging_time;   /* master to slave to set aging time */

        struct
        {
            UI32_T              number_of_entries;
            AMTRDRV_TYPE_Record_T record[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
            BOOL_T              is_synchronous_op; /* TRUE if the operations is synchronous */
        }__attribute__((packed, aligned(1)))entries;

        struct
        {
            UI32_T              vid;
            UI8_T               mac[6];
            UI8_T               pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T               tbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

        }__attribute__((packed, aligned(1)))mclist;

        struct mstp_amtr_callback mstp_entry_callback;

        /* create this struct for Voice Vlan request.
         */
        struct
        {
            AMTRDRV_TYPE_Record_T record;
            UI32_T              num_of_mask_list;   /* how many records(mac) in mask_list[][].*/
            UI8_T               mask_mac_list[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN];
            UI8_T               mask_list[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN];
        }__attribute__((packed, aligned(1)))masklist;
    }__attribute__((packed, aligned(1)))data;
}__attribute__((packed, aligned(1)))AMTRDRV_MGR_ISCBuffer_T;

/* Why we have to add one more data structure ????
 * 1.This singleBuffer is for sending a single address information by ISC to remote unit.
 * 2.To have a better performance -- Currently it's only using in two threads
 *   "AMTRDRV_MGR_ProcessJobQueue" & "AMTRDRV_MGR_ProcessCheckingAgingList" for mother board
 *   to control module board to set/delete address to ASIC and also poll the hitbit value
 *   it the address is learnt on module board. If we still use the old ISCBuffer may
 *   gate by other tasks which use AMTR service to send out ISC to remote unit to do something
 */
typedef struct
{
    AMTRDRV_MGR_CallByAgentServiceID_T service_id;
    AMTRDRV_TYPE_Record_T                 record;
    BOOL_T                              return_value;
}__attribute__((packed, aligned(1)))AMTRDRV_MGR_ISCSingleBuffer_T;

typedef BOOL_T (*AMTRDRV_MGR_RemoteSvcFun_T)(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);

#endif/*SYS_CPNT_STACKING*/

/*In some situation, one cookis will include two condition. ex. vid+sourec or vid+lifetime*/
typedef struct
{
    BOOL_T   event_flag;
    UI32_T   record_index;
    UI32_T   event_id;
}AMTRDRV_MGR_EventInfo_T;

/******************
 LOCAL SUBROUTINES
 ******************/
static void AMTRDRV_MGR_Init(void);

/* LOCAL SUBPROGRAM DECLARATIONS for AMTRDRV_ADDRESS_TASK
 */
static void AMTRDRV_MGR_ConvertGatewayReturnValue(DEV_AMTRDRV_Ret_T gw_retval, AMTR_TYPE_Ret_T* amtr_type_ret_val);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
static void   AMTRDRV_ADDRESS_TASK_Main (void);
#if (SYS_CPNT_MAINBOARD == TRUE)
static UI32_T AMTRDRV_MGR_ProcessCheckingAgingList(void);
#endif/*#if (SYS_CPNT_MAINBOARD == TRUE)*/
#endif/*#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/

/* LOCAL SUBPROGRAM DECLARATIONS for AMTRDRV_ADDRESS_TASK
 */
static void   AMTRDRV_ASIC_COMMAND_TASK_Main(void);
#if (SYS_CPNT_MAINBOARD == TRUE)
static UI32_T AMTRDRV_MGR_ProcessJobQueue(void);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
static void   AMTRDRV_MGR_SyncASIC2OM(void);
static void   AMTRDRV_MGR_Notify_AgingOut(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
static void   AMTRDRV_MGR_Notify_NewAddress(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
#else/*#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/
#define AMTRDRV_MGR_SyncASIC2OM()
#endif/*End of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)*/
#endif
/* LOCAL SUBPROGRAM DECLARATIONS for Notify funciton
 */
BOOL_T AMTRDRV_MGR_AnnounceNA_N_SecurityCheck(UI8_T *dst_mac,UI8_T *src_mac,UI16_T vid,UI16_T ether_type,UI32_T src_unit,UI32_T src_port);

/* LOCAL SUBPROGRAM DECLARATIONS for AMTRDRV_MGR
 */
static void AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry(AMTR_TYPE_AddrEntry_T *amtrdrv_record_p, SWDRV_TYPE_L2_Address_Info_T *arl_record_p);
static UI32_T AMTRDRV_MGR_SetChip(AMTRDRV_TYPE_Record_T *address_record,UI8_T action, UI8_T reserved_state);
static BOOL_T AMTRDRV_MGR_ARLLookUp(UI32_T vid,UI8_T *mac,UI32_T unit,UI32_T *port,UI32_T *trunk_id,BOOL_T *is_trunk);

#if (SYS_CPNT_MAINBOARD == TRUE)
static BOOL_T AMTRDRV_MGR_IsInsert2AgingList(UI32_T ifindex,UI32_T life_time);

#if (SYS_CPNT_STACKING == TRUE)
static UI16_T AMTRDRV_MGR_GetValidUnitBmp(BOOL_T is_module_include);
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

/* LOCAL SUBPROGRAM DECLARATIONS for send event to sys_callback task to dequeue
 */
static void   AMTRDRV_MGR_CallBackQueueEnqueue(UI32_T index);
static void   AMTRDRV_MGR_ID2Event(AMTR_TYPE_AddrEntry_T *addr_entry,UI32_T index,UI32_T *id,AMTR_TYPE_Command_T *action);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
static UI32_T AMTRDRV_MGR_ProcessSlaveNABuffer(void);
#endif

/* LOCAL SUBPROGRAM DECLARATIONS for local operation process
 */
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrBySource(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPort(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByVID(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByVidAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByVidAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByVIDnPort(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndVidExceptCertainAddr(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
static BOOL_T AMTRDRV_MGR_LocalChangePortLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p);
#endif /* end of #if (SYS_CPNT_MAINBOARD == TRUE) */
static BOOL_T AMTRDRV_MGR_LocalDeleteAllAddr(AMTR_TYPE_BlockedCommand_T * blocked_command_p);

/* LOCAL SUBPROGRAM DECLARATIONS for AMTRDRV_MGR BACKDOOR
 */
#ifdef BACKDOOR_OPEN
static void   AMTRDRV_MGR_BD_GenerateAddress(AMTR_TYPE_AddrEntry_T* address_entry);
static BOOL_T AMTRDRV_MGR_BD_UnitExist(UI32_T unit);
static void   AMTRDRV_MGR_BD_MsgLevel(void);
static void   AMTRDRV_MGR_BD_SetAgingTime(void);
static void   AMTRDRV_MGR_BD_GetAgingTime(void);
static void   AMTRDRV_MGR_BD_SetAddr(void);
static void   AMTRDRV_MGR_BD_CreateMulticastAddr(void);
static void   AMTRDRV_MGR_BD_DestoryMulticastAddr(void);
static void   AMTRDRV_MGR_BD_ARLLookUp(void);
#if (SYS_CPNT_MAINBOARD == TRUE)
static void   AMTRDRV_MGR_BD_DeleteAddr(void);
static void   AMTRDRV_MGR_BD_GetExactRecord(void);
static void   AMTRDRV_MGR_BD_DumpAllRecords(void);
static void   AMTRDRV_MGR_BD_DumpRecordsInQueryGroup(void);
static void   AMTRDRV_MGR_BD_DumpRecordInLocalCheckingList();
static void   AMTRDRV_MGR_BD_DumpAgingOutBuffer();
static void   AMTRDRV_MGR_BD_Dump_NA_Hash_Counter(void);
static void   AMTRDRV_MGR_BD_ChecknClearHitBit(void);
static void   AMTRDRV_MGR_BD_AddressTaskPerformanceTest();
static void   AMTRDRV_MGR_BD_AsicTaskPerformanceTest();
static void   AMTRDRV_MGR_BD_PureProgramChipPerformanceTest();
static void   AMTRDRV_MGR_BD_ShowTotalCounters();
static void   AMTRDRV_MGR_BD_ShowByPortCounters();
static void   AMTRDRV_MGR_BD_ShowByVidCounter();
static void   AMTRDRV_MGR_BD_IntergrationPerformanceTesting();
#endif
static void   AMTRDRV_MGR_BackDoor_Menu(void);
#endif/*BACKDOOR_OPEN*/

/* LOCAL SUBPROGRAM DECLARATIONS for AMTRDRV_MGR ISC REMOTE SERVICE
 */
#if (SYS_CPNT_STACKING == TRUE)
/* One to One mapping to AMTR_MGR Functions */
static BOOL_T AMTRDRV_MGR_Service_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref, UI8_T svc_id);
static BOOL_T AMTRDRV_MGR_Remote_SetAgingTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_AgingOut(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_SetAddrEntryList(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
static BOOL_T AMTRDRV_MGR_Remote_SetAddrDirectly(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#endif
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrEntryList(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrDirectly(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAllAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrBySource(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByLifeTimeAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrBySoreceAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVID(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTimeAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndSourceAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndSource(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVIDnPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVIDnPortExceptCertainAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_ChangePortLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
static BOOL_T AMTRDRV_MGR_Remote_CreateMulticastAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_DestroyMulticastAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_SetMulticastPortMember(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#endif
static BOOL_T AMTRDRV_MGR_Remote_ProcessNewAddress(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
static BOOL_T AMTRDRV_MGR_Remote_SetAddrToLocalModule(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_PollAddrOfModuleHitBitValue(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
static BOOL_T AMTRDRV_MGR_Remote_HandleModuleHotInsertion(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p);
#endif
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

/***************
 LOCAL VARIABLES
 ***************/
const static UI8_T                         null_mac[] = { 0,0,0,0,0,0 };

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
//static UI32_T                              amtrdrv_address_task_tid;          /* amtrdrv_address_task id      */
//static void*                               amtrdrv_address_task_tmid;         /* amtrdrv_address_task periodic timer ID      */
//static BOOL_T                              amtrdrv_mgr_is_address_transition_done;
#endif

#if (SYS_CPNT_MAINBOARD == TRUE)
//static void*                               amtrdrv_asic_command_task_tmid;    /* amtrdrv_asic_command_task periodic timer ID */
#endif

//static BOOL_T                              amtrdrv_mgr_is_asic_command_transition_done;
static UI32_T                              amtrdrv_mgr_num_of_na_announce =0; /* This information can get from backdoor function */

/* CallBack function register -- only provide one CSC to register*/
static AMTRDRV_MGR_AddrCallbackFunction_T  AMTRDRV_MGR_NewAddressCallback = NULL;
static AMTRDRV_MGR_SecurityCheckFunction_T AMTRDRV_MGR_SecurityCheckCallback = NULL;
#if (SYS_CPNT_MAINBOARD == TRUE)
static AMTRDRV_MGR_AddrCallbackFunction_T  AMTRDRV_MGR_AgingOutCallback = NULL;
#endif

static UI32_T                            amtrdrv_local_finished_smid; /*local finish semaphore id */
static UI32_T                            amtrdrv_sync_op_smid; /* synchronous block command semaphore id */

// kh_shi the following two *_event variable is used as local variable
#if (SYS_CPNT_MAINBOARD == TRUE)
/* callback queue event information */
/* We just keep this variable for future feature but we don't use it in current design */
static AMTRDRV_MGR_EventInfo_T             amtrdrv_mgr_callback_event;
/* sync to hisam event information */
static AMTRDRV_MGR_EventInfo_T             amtrdrv_mgr_sync_event;
#endif

/* the buffer for notify core layer new addresses / aging out addresses */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    AMTR_TYPE_AddrEntry_T               addr_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
//static    AMTR_TYPE_AddrEntry_T    addr_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
/*the popose of adding this value is to modify the ticks dynamically*/

static UI32_T amtr_update_addr_table_ticks =
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-26, 10:32:19 */
                                             SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS_FOR_ES4308MA; /* SW learn */
#else
                                             SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS; /* SW learn */
#endif
#else
                                             52;                                   /* HW learn */
#endif

/*for backdoor display;water_huang*/
static BOOL_T                              amtrdrv_mgr_bd_display;
static BOOL_T                              amtrdrv_mgr_enable_hw_on_sw = FALSE;
/*#if (SYS_CPNT_MAINBOARD == TRUE)*/
/* These variables need to be delete when checking in codes */
static BOOL_T                              amtrdrv_mgr_bd_address_task_performance;
static BOOL_T                              amtrdrv_mgr_bd_asic_task_performance;
static UI32_T                              amtrdrv_mgr_address_task_testing_times;           /* total times */
static UI32_T                              amtrdrv_mgr_number_of_address_task_testing_times; /* decreasing counter */
static UI32_T                              amtrdrv_mgr_asic_task_testing_times;           /* total times */
static UI32_T                              amtrdrv_mgr_number_of_asic_task_testing_times; /* decreasing counter */
static UI32_T                              amtrdrv_mgr_testing_na_time;
static UI32_T                              amtrdrv_mgr_testing_na_counter;
static UI32_T                              amtrdrv_mgr_testing_agingcheck_time;
static UI32_T                              amtrdrv_mgr_testing_agingchecking_counter;
static UI32_T                              amtrdrv_mgr_testing_agingout_time;
static UI32_T                              amtrdrv_mgr_testing_agingout_counter;
static UI32_T                              amtrdrv_mgr_testing_program_chip_time;
static UI32_T                              amtrdrv_mgr_testing_program_chip_counter;
static UI32_T                              amtrdrv_mgr_integration_testing_num_of_entry=0;
static UI32_T                              amtrdrv_mgr_integration_testing_flag=FALSE;
static UI32_T                              amtrdrv_mgr_integration_tesing_na_start_tick=0;
static UI32_T                              amtrdrv_mgr_integration_testing_printf_first_age_out_entry = FALSE;
/*add by Tony.lei for testing the performance*/
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
static UI32_T                              amtrdrv_mgr_test_performance = 0;
static UI32_T                              amtrdrv_mgr_test_master_counter = 0;
static UI32_T                              amtrdrv_mgr_test_master_totol_counter =0;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
static UI32_T                              amtrdrv_mgr_test_start_tick = 0;
#endif
static UI32_T                              amtrdrv_mgr_test_slave_start_tick = 0;
static UI32_T                              amtrdrv_mgr_test_slave_counter = 0;
static UI32_T                              amtrdrv_mgr_test_slave_totol_counter =0;
static UI32_T                              amtrdrv_mgr_test_start_tick_asic = 0;
static UI32_T                              amtrdrv_mgr_test_master_counter_asic = 0;
static UI32_T                              amtrdrv_mgr_test_master_counter_asic_dequeue = 0;
//static UI32_T                              amtrdrv_mgr_test_master_counter_asic_dequeue_timer = 0;
static UI32_T                              amtrdrv_mgr_test_master_counter_asic_ok = 0;
static UI32_T                              amtrdrv_mgr_test_master_totol_counter_asic =0;
static void *                              amtrdrv_asic_command_task_tmid_test = 0;
static UI32_T                              amtrdrv_mgr_test_dequeue_method = 0 ;
static UI32_T                              amtrdrv_mgr_test_enqueue_failed_counter = 0;
//static UI32_T                              amtrdrv_mgr_test_queue_full_counter = 0;
static UI32_T                              amtrdrv_mgr_test_register_action = 0;
static UI32_T                              amtrdrv_mgr_narestore_counters = 0;
static UI32_T                              amtrdrv_mgr_isc_test   = 0;
static UI32_T                              amtrdrv_mgr_isc_test_returndonothing = 0;
static UI32_T                              amtrdrv_mgr_isc_test_returndonothing1 = 0;
#endif

#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
static UI32_T  memcpy_counter = 0;
static UI32_T  memset_counter = 0;
static UI32_T  memcmp_counter = 0;
static UI32_T  memcpy_time_spend = 0;
#define amtrdrv_memcpy(a,b,c)  do{  \
        UI32_T  t2;\
        UI32_T t1 = SYSFUN_GetSysTick();  \
        memcpy_counter ++;   \
        memcpy(a,b,c); \
        if((t2 =(SYSFUN_GetSysTick() -t1))!= 0) memcpy_time_spend+= t2;\
       }while(0)
#define amtrdrv_memset(a,b,c)  do{ \
           memset_counter ++;      \
           memset(a,b,c);          \
           }while(0)
#define amtrdrv_memcmp(a,b,c)  (memcmp_counter++,memcmp(a,b,c))
#else
#define amtrdrv_memcpy(a,b,c)  memcpy(a,b,c)
#define amtrdrv_memset(a,b,c)  memset(a,b,c)
#define amtrdrv_memcmp(a,b,c)  memcmp(a,b,c)
#endif
/*#endif*/
/* isc service
 */
#if (SYS_CPNT_STACKING == TRUE)
static AMTRDRV_MGR_RemoteSvcFun_T          amtrdrv_mgr_remote_service[AMTRDRV_MGR_MAX_NUM_OF_DIRECT_CALL_SERVICE] =
{
    AMTRDRV_MGR_Remote_SetAgingTime,
    AMTRDRV_MGR_Remote_AgingOut,
    AMTRDRV_MGR_Remote_SetAddrEntryList,
    AMTRDRV_MGR_Remote_DeleteAddrEntryList,
    AMTRDRV_MGR_Remote_DeleteAddrDirectly,
    AMTRDRV_MGR_Remote_DeleteAllAddr,
    AMTRDRV_MGR_Remote_DeleteAddrByLifeTime,
    AMTRDRV_MGR_Remote_DeleteAddrBySource,
    AMTRDRV_MGR_Remote_DeleteAddrByPort,
    AMTRDRV_MGR_Remote_DeleteAddrByLifeTimeAndPort,
    AMTRDRV_MGR_Remote_DeleteAddrBySoreceAndPort,
    AMTRDRV_MGR_Remote_DeleteAddrByVID,
    AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTimeAndPort,
    AMTRDRV_MGR_Remote_DeleteAddrByVidAndSourceAndPort,
    AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTime,
    AMTRDRV_MGR_Remote_DeleteAddrByVidAndSource,
    AMTRDRV_MGR_Remote_DeleteAddrByVIDnPort,
    AMTRDRV_MGR_Remote_DeleteAddrByVIDnPortExceptCertainAddr,
    AMTRDRV_MGR_Remote_ChangePortLifeTime,
    AMTRDRV_MGR_Remote_ProcessNewAddress,
};

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
static AMTRDRV_MGR_RemoteSvcFun_T          amtrdrv_mgr_handler_service[AMTRDRV_MGR_MAX_NUM_OF_CALL_BY_AGENT_SERVICE] =
{
    AMTRDRV_MGR_Remote_SetAddrToLocalModule,
    AMTRDRV_MGR_Remote_PollAddrOfModuleHitBitValue,
    AMTRDRV_MGR_Remote_HandleModuleHotInsertion,
    AMTRDRV_MGR_Remote_SetAddrDirectly,
    AMTRDRV_MGR_Remote_CreateMulticastAddr,
    AMTRDRV_MGR_Remote_DestroyMulticastAddr,
    AMTRDRV_MGR_Remote_SetMulticastPortMember,
};
#endif /* end of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE) */

#endif /*SYS_CPNT_STACKING*/

static UI32_T       amtrdrv_mgr_debug_flag;
static BOOL_T       amtrdrv_mgr_debug_show_collision_flag=FALSE;
#ifdef AMTRDRV_MGR_DEBUG
#define AMTRDRV_MGR_DBGMSG(x)                                           \
    if (amtrdrv_mgr_debug_flag & AMTRDRV_MGR_DEBUG_DBGMSG)  \
    {                                                       \
        BACKDOOR_MGR_Printf("%s(): %s\r\n", __FUNCTION__, (x));          \
    }

#define DBGADDRENTRY(x)                                     \
    if (amtrdrv_mgr_debug_flag & AMTRDRV_MGR_DEBUG_ADDR_CONTENT)    \
    {                                                       \
        BACKDOOR_MGR_Printf("VID:%d ", (x).vid);                        \
        BACKDOOR_MGR_Printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x ",        \
                (x).mac[0], (x).mac[1], (x).mac[2],         \
                (x).mac[3], (x).mac[4], (x).mac[5]);        \
        BACKDOOR_MGR_Printf("Ifindex:%d ", (x).ifindex);                     \
        BACKDOOR_MGR_Printf("life_time:%d ", (x).life_time);             \
        BACKDOOR_MGR_Printf("Source: %d\r\n", (x).source);             \
    }
#else
#define AMTRDRV_MGR_DBGMSG(x)
#define DBGADDRENTRY(x)
#endif

//SYSFUN_DECLARE_CSC

/*--------------------
 * EXPORTED ROUTINES
 *-------------------*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to create Tasks for AMTRDRV
 * INPUT  :
 * OUTPUT :
 * RETURN :
 * NOTES  : AMTRDRV this component has two tasks --
 *            1.AMTRDRV_ADDRESS_TASK:
 *                   a. to learn new address
 *                   b. to check address need to be aged out or not
 *                   c. to delete the address which is aged
 *            2.AMTRDRV_ASIC_COMMAND_TASK:
 *                   a. to program chip (set or delete)
 *                   b. to process command order
 *-----------------------------------------------------------------------------*/
void  AMTRDRV_TASK_CreateTask(void)
{
    UI32_T amtrdrv_asic_command_task_tid;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    UI32_T amtrdrv_address_task_tid;
#endif

/* When Hardware Learning, don't need create this task
 */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    if(SYSFUN_SpawnThread(SYS_BLD_AMTRDRV_ADDRESS_THREAD_PRIORITY,
                          SYS_BLD_AMTRDRV_ADDRESS_THREAD_SCHED_POLICY,
                          SYS_BLD_AMTRDRV_ADDRESS_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          AMTRDRV_ADDRESS_TASK_Main,
                          NULL,
                          &amtrdrv_address_task_tid)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf(" AMTRDRV_ADDRESS_TASK: create task fail !\n");
    }
    else
    {
        AMTRDRV_OM_SetAddressTaskId(amtrdrv_address_task_tid);
    }
#endif

    if (SYSFUN_SpawnThread(
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
                          SYS_BLD_AMTRDRV_TASK_PRIORITY,
#else /*SW Learning*/
                          SYS_BLD_AMTRDRV_ASIC_COMMAND_THREAD_PRIORITY,
#endif
                          SYS_BLD_AMTRDRV_ASIC_COMMAND_THREAD_SCHED_POLICY,
                          SYS_BLD_AMTRDRV_ASIC_COMMAND_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          AMTRDRV_ASIC_COMMAND_TASK_Main,
                          NULL,
                          &amtrdrv_asic_command_task_tid) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf(" AMTRDRV_ASIC_COMMAND_TASK: create task fail !\n");
    }
    else
    {
        AMTRDRV_OM_SetAsicComTaskId(amtrdrv_asic_command_task_tid);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_AMTRDRV, amtrdrv_asic_command_task_tid, SYS_ADPT_AMTRDRV_SW_WATCHDOG_TIMER);
#endif

} /* END OF AMTRDRV_TASK_CreateTask() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_NewAddress_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(New address)
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : callback function
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_NewAddress_CallBack(AMTRDRV_MGR_AddrCallbackFunction_T callbackfunction)
{
    AMTRDRV_MGR_NewAddressCallback = callbackfunction;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_SecurityCheck_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(security checking for NA)
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : callback function
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_SecurityCheck_CallBack(AMTRDRV_MGR_SecurityCheckFunction_T  callbackfunction)
{
    AMTRDRV_MGR_SecurityCheckCallback = callbackfunction;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_AgingOut_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(Aging out)
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : callback function
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_AgingOut_CallBack(AMTRDRV_MGR_AddrCallbackFunction_T callbackfunction)
{
    AMTRDRV_MGR_AgingOutCallback = callbackfunction;
}
#endif

void AMTRDRV_MGR_InitiateSystemResources(void)
{
    AMTRDRV_OM_InitiateSystemResources();
    AMTRDRV_MGR_Init();
}

void AMTRDRV_MGR_AttachSystemResources(void)
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTRDRV, &amtrdrv_local_finished_smid);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTRDRV_SYNC_OP, &amtrdrv_sync_op_smid);

    AMTRDRV_OM_AttachSystemResources();
}

void AMTRDRV_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    AMTRDRV_OM_GetShMemInfo(segid_p, seglen_p);
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to reserve the resources which will be used
 *          in whole system
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_MGR_Init(void)
{

#if (SYS_CPNT_MAINBOARD == TRUE)
    /* create database to store addresses info
     */
    if (!AMTRDRV_OM_L2TableInit())
    {
        BACKDOOR_MGR_Printf("\r\nAMTRDRV: Create L2 database failed.");
        while(1);
    }
#endif /* SYS_CPNT_MAINBOARD */

    /* create database to store addresses info
     */
    if (!AMTRDRV_OM_NABufferInit())
    {
        BACKDOOR_MGR_Printf("\r\nAMTRDRV: Create NA buffer failed.");
        while(1);
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Create_InterCSC_Relation()
{
    LAN_Register_NA_N_SecurityCheck_Handler(&AMTRDRV_MGR_AnnounceNA_N_SecurityCheck);

#if (SYS_CPNT_STACKING == TRUE)
{
    /* register as a ISC's client and set callback function
     */
    ISC_Register_Service_CallBack(ISC_AMTRDRV_DIRECTCALL_SID, AMTRDRV_MGR_Service_Callback);
}
#endif /*SYS_CPNT_STACKING*/

#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("AMTRDRV", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, AMTRDRV_MGR_BackDoor_Menu);
#endif  /* BACKDOOR_OPEN */

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set up the resources which are used in master mode
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterMasterMode(void)
{
    UI32_T  my_driver_unit_id;

    /* collect information from stacking topology
     */
    STKTPLG_POM_GetMyDriverUnit(&my_driver_unit_id);
    AMTRDRV_OM_SetMyDrvUnitId(my_driver_unit_id);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    DEV_AMTRDRV_PMGR_ResetToFirstAddrTblEntry(AMTRDRV_MGR_ALL_DEVICE_ID);
    DEV_AMTRDRV_PMGR_SetAgeingTime(SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME);
#endif
    /* set mgr in master mode
     */
    AMTRDRV_OM_EnterMasterMode();
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set the operation mode to transition mode
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_SetTransitionMode(void)
{
    /* 1. Setting operation_mode to transtition mode
     */
    AMTRDRV_OM_SetTransitionMode();
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    /* 2. Sending event to notify address_task transition mode isn't done
     */
    //amtrdrv_mgr_is_address_transition_done = FALSE;
    //SYSFUN_SendEvent(AMTRDRV_OM_GetAddressTaskId(),AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE);
#else
    AMTRDRV_OM_SetProvisionComplete(FALSE);
#endif
    /* 3. Sending event to notify asic_command task transition mode isn't done
     */
    //amtrdrv_mgr_is_asic_command_transition_done = FALSE;
    //SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(),AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE);

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to initialize the resources which are used in whole system
 *          no mater the unit is master or slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterTransitionMode(void)
{
    AMTRDRV_OM_EnterTransitionMode();

    /* 1. To leave CSC Task while transition done
     */
    //SYSFUN_TASK_ENTER_TRANSITION_MODE(amtrdrv_mgr_is_asic_command_transition_done);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    //SYSFUN_TASK_ENTER_TRANSITION_MODE(amtrdrv_mgr_is_address_transition_done);

#endif

    /* 2. Delete all address entries from ARL manually
     */
    /* When system re-boot and TCN, ASIC will be cleared by DEV_SWDRV.
     * AMTRDRV don't need to do again
     * DEV_AMTRDRV_PMGR_DeleteAllAddr();
     */
    /* Since AMTRDRV don't delete all address from ASIC ARL Table.
     * It doesn't need to reset multi-cast entry.
     * DEV_AMTRDRV_PMGR_SetMCastEntryForInternalPacket();
     */

#if (SYS_CPNT_MAINBOARD == TRUE)
    /* 3. Initialize OM database */
    AMTRDRV_OM_ClearDatabase();

    /* 4. Initialize callback_event information
     */
    amtrdrv_mgr_callback_event.event_flag = FALSE;
    amtrdrv_mgr_callback_event.event_id = 0;
    amtrdrv_mgr_callback_event.record_index = 0;
    /* 5. Initialize sync event information
     */
    amtrdrv_mgr_sync_event.event_flag = FALSE;
    amtrdrv_mgr_sync_event.event_id = 0;
    amtrdrv_mgr_sync_event.record_index = 0;
#endif /* SYS_CPNT_MAINBOARD */
    /* 6. Initialize NA buffer
     */
    AMTRDRV_OM_ClearNABuffer();

    /* 7. Setting default value
     */
    AMTRDRV_OM_SetMyDrvUnitId(0);
    amtrdrv_mgr_bd_display      = FALSE;
    /*the default value 0*/
    amtrdrv_mgr_debug_flag      = 0;  //kh_shi patch
    amtrdrv_mgr_debug_show_collision_flag   = FALSE;
#if (SYS_CPNT_MAINBOARD == TRUE)
    amtrdrv_mgr_bd_address_task_performance  = FALSE;
    amtrdrv_mgr_bd_asic_task_performance = FALSE;
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif

    /* 8. Setting aging time
     */
    AMTRDRV_OM_SetOperAgingTime(SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME);

    /* 9. Initialize blocked command info
     */
   /* set command null, set amtrdrv_local_finished_smid to 1
      */
    AMTRDRV_OM_SetBlockCommandNull();
    SYSFUN_GiveSem(amtrdrv_local_finished_smid);
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set up resources which are used in slave mode
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterSlaveMode(void)
{
    UI32_T  my_driver_unit_id;

    /* collect information from the stacking topology
     */
    STKTPLG_POM_GetMyDriverUnit(&my_driver_unit_id);
    AMTRDRV_OM_SetMyDrvUnitId(my_driver_unit_id);

    /* set mgr in slave mode
     */
    AMTRDRV_OM_EnterSlaveMode();

    return;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_ProvisionComplete
 *------------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_ProvisionComplete(void)
{
    //SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    AMTRDRV_OM_SetProvisionComplete(TRUE);
    SYSFUN_RELEASE_CSC();
}
#else

#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_CallByAgent_ISC_Handler
 *------------------------------------------------------------------------------
 * PURPOSE: This function will handle isc request from ISC_Agent.
 * INPUT  : ISC_Key_T *key              - key of isc
 *          L_MM_Mref_Handle_T *mem_ref
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTE   : callbacked by isc_agent
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_CallByAgent_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
    AMTRDRV_MGR_ISCBuffer_T     *buffer_p;
    UI32_T                      pdu_len;
    BOOL_T                      return_value;

    const static UI8_T   *func_names[]={
                 (UI8_T *)"AMTRDRV_MGR_SET_ADDR_TO_LOCAL_MODULE",
                 (UI8_T *)"AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE",
                 (UI8_T *)"AMTRDRV_MGR_HANDLE_MODULE_HOT_INSERTION",
                 (UI8_T *)"AMTRDRV_MGR_SET_ADDR_DIRECTLY",
                 (UI8_T *)"AMTRDRV_MGR_CREATE_MULTICAST_ADDR",
                 (UI8_T *)"AMTRDRV_MGR_DESTROY_MULTICAST_ADDR",
                 (UI8_T *)"AMTRDRV_MGR_SET_MULTICAST_PORT_MEMBER",
                 (UI8_T *)"AMTRDRV_MGR_MAX_NUM_OF_CALL_BY_AGENT_SERVICE"
            };
    buffer_p = L_MM_Mref_GetPdu(mem_ref, &pdu_len);

    if(buffer_p->service_id>=AMTRDRV_MGR_MAX_NUM_OF_CALL_BY_AGENT_SERVICE || amtrdrv_mgr_handler_service[buffer_p->service_id]==NULL)
    {
        L_MM_Mref_Release(&mem_ref);
        return TRUE;  /* We don't want the ISC re-send agian */
    }

    AMTRDRV_MGR_DBGMSG(func_names[buffer_p->service_id]);
    return_value = (amtrdrv_mgr_handler_service[buffer_p->service_id])(key, buffer_p);
    L_MM_Mref_Release(&mem_ref);
    return return_value;
}
#endif/* End of #if (SYS_CPNT_STACKING == TRUE) */
#endif

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_HandleHotInsertion
 *------------------------------------------------------------------------------
 * FURPOSE: This function is to set all addresses to new insertion unit
 * INPUT  : hot_insertion unit   - the new insertion unit id
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This API only call by AMTR_MGR in master mode
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_HandleHotInsertion(UI32_T hot_insertion_unit)
{
#if(SYS_CPNT_STACKING == TRUE)
    L_DLST_ShMem_Indexed_Dblist_T     *dblist;
    AMTRDRV_TYPE_Record_T         address_record;
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
    UI32_T                      pdu_len;
    UI32_T                      i;
    UI32_T                      service_id;
    UI32_T                      amtr_isc_service_id;
    UI32_T                      pool_id;
    UI16_T                      return_unit_bmp=0;
    UI16_T                      unit_bmp =0;
    UI32_T                      index;
    UI8_T                       action;
    BOOL_T                      ret = FALSE;
    /* set up ISC service information
     */

    /*
     * The reason why we have to set two different isc service for moudle and mainboard
     * 1.Due to module doesn't have OM,we have to program the new insertion module
     *   chip right away. To implement this we have to create a new isc service ID
     *   which can support to program a group of addresses.
     * 2.For a unit hot insertion, we can only write this to it's local OM and use
     *   job queue to program its chips.
     */
    if( hot_insertion_unit >  SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK )
    {
        service_id = AMTRDRV_MGR_HANDLE_MODULE_HOT_INSERTION;
        /* Remote unit will program chip.
         */
        amtr_isc_service_id = ISC_AMTRDRV_CALLBYAGENT_SID;
        pool_id             = AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID;
    }
    else
    {
        service_id = AMTRDRV_MGR_SET_ADDR_LIST;
        /* Remote unit only set NA to OM.
         */
        amtr_isc_service_id = ISC_AMTRDRV_DIRECTCALL_SID;
        pool_id             = AMTRDRV_MGR_DIRECT_CALL_POLL_ID;
    }

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,pool_id, service_id) /* user_id */);
    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_buffer_p==NULL)
    {
        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
        return FALSE;
    }

    isc_buffer_p->data.entries.number_of_entries = 0;
    /* Setup destination which will deliver to by ISC service */
    unit_bmp |= ((0x01) << (hot_insertion_unit-1));

    /* Getting all records info from OM via searching from life_time query group */
    for (i=AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;i<=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;i++)
    {
        ret = AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,i-1,&dblist,&index,(UI8_T *)&address_record,&action);

        while (ret)
        {   /* Only action = SET we will send to new insertion unit since the action = Delete
             * means this record will be killed sooner.
             */
            if (action == AMTR_TYPE_COMMAND_SET_ENTRY)
            {
                amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&address_record,sizeof(AMTRDRV_TYPE_Record_T));
                isc_buffer_p->data.entries.number_of_entries ++;
                if (isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
                {
                    isc_buffer_p->service_id = service_id;
                    return_unit_bmp = ISC_SendMcastReliable(unit_bmp,amtr_isc_service_id,
                                                     mref_handle_p,
                                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                     AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

                    if (return_unit_bmp !=0)
                    {
                        AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                        return FALSE;
                    }

                    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,pool_id, service_id) /* user_id */);
                    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                    if (isc_buffer_p==NULL)
                    {
                        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                        return FALSE;
                    }
                    isc_buffer_p->data.entries.number_of_entries = 0;
                }
            }
            ret = AMTRDRV_OM_GetNextEntryFromGivingQueryGroup(dblist, &index,(UI8_T *)&address_record,&action);
        }
    }

    if (isc_buffer_p->data.entries.number_of_entries !=0)
    {
        isc_buffer_p->service_id = service_id;
        return_unit_bmp = ISC_SendMcastReliable(unit_bmp,amtr_isc_service_id,
                                         mref_handle_p,
                                         SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                         AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        if (return_unit_bmp !=0)
        {
            AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
            return FALSE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }
#endif/*#if(SYS_CPNT_STACKING == TRUE)*/
    return TRUE;
}
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAgingTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set aging time for the whole system
 * INPUT  : UI32_T value  - Aging time to set
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : 1. aging time is in [10..1000000] seconds
 *          2. If you want to disable aging out function please assign value =0
 *          3. We only keep the value in local static variable since we set AGE=0
 *             to chip to let the software control aging out function.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAgingTime(UI32_T value)
{
    /* BODY
     */

    /* local unit setting
     * Due to aging out function disable we didn't update each record's timestamp.
     * When aging out functino is enable again we need to update the record's timestamp
     * as current system time one by one and also start to polling aging checking list.
     */
    SYSFUN_USE_CSC(FALSE);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    if(DEV_AMTRDRV_PMGR_SetAgeingTime(value) == FALSE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
#endif/*End of if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE) */

#if (SYS_CPNT_MAINBOARD == TRUE)
    if (AMTRDRV_OM_GetOperAgingTime() == 0 && value != 0)
    {
        AMTRDRV_OM_LocalAgingListUpdateTimeStamp();
    }

    #if(SYS_CPNT_STACKING == TRUE)
    if (AMTRDRV_OM_GetOperatingMode()== SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI16_T                      unit_bmp=0;

        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if ( unit_bmp != 0 )
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_SET_AGING_TIME) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            /* (operation_mode == MASTER) Set the ageing time of slave units to the
             * given value
             */
            isc_buffer_p->service_id  = AMTRDRV_MGR_SET_AGING_TIME;
            isc_buffer_p->data.aging_time = value;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                     mref_handle_p,
                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                     AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set up aging time in slave units.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
    #endif /* SYS_CPNT_STACKING */
#endif /* SYS_CPNT_MAINBOARD */
    AMTRDRV_OM_SetOperAgingTime(value);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry into Hash table & chip and
 *          aslo program remote units
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *          2. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrList(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T result = TRUE;
    SYSFUN_USE_CSC(FALSE);
#if (SYS_CPNT_MAINBOARD == TRUE)

    if(AMTRDRV_MGR_SetAddrList2LocalUnit(num_of_entries,addr_buf) != AMTR_TYPE_RET_SUCCESS)
    {
        result = FALSE;
    }
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    if(AMTRDRV_MGR_SetAddrList2RemoteUnit(num_of_entries,addr_buf) ==  FALSE)
    {
        result = FALSE;
    }
#endif
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return result;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetInterventionEntry
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set InterventionEntry address entry into Hash table
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTES  : 1. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *          2. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T AMTRDRV_MGR_SetInterventionEntry(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    AMTR_TYPE_Ret_T result = AMTR_TYPE_RET_SUCCESS;
    SYSFUN_USE_CSC(AMTR_TYPE_RET_ERROR_UNKNOWN);
    
#if (SYS_CPNT_MAINBOARD == TRUE)
    result = AMTRDRV_MGR_SetAddrList2LocalUnit(num_of_entries,addr_buf);
    if( result != AMTR_TYPE_RET_SUCCESS)
    {
        SYSFUN_RELEASE_CSC();
        return result;
    }    
    if(AMTRDRV_MGR_SetAddrList2RemoteUnit(num_of_entries,addr_buf) ==  FALSE)
    {
        result = AMTR_TYPE_RET_ERROR_UNKNOWN;
    }    
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return result;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry from Hash table
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 *          UI32_T num_of_entries               - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteInterventionEntry(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T                      result = TRUE;
#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T         address_record;
    UI32_T                      i;
    UI32_T                      index;

    SYSFUN_USE_CSC(FALSE);
    for(i=0;i<num_of_entries;i++)
    {
        /* if the mac = null mac it means this is an invalid address and we don't need to process it
         */
        if(CHECK_MAC_IS_NULL(addr_buf[i].mac)){
            continue;
        }

        amtrdrv_memcpy(&address_record.address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));

        if(!AMTRDRV_OM_DeleteAddr((UI8_T *)&address_record,&index)){

            amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
            result = FALSE ;
            continue;
        }

        AMTRDRV_OM_SyncQueueEnqueue(index);
    }
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList2LocalUnit
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry into Hash table & chip in local unit
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTES  : 1. If one of the entry in the buffer can't setup successfully we will
 *          return AMTR_TYPE_RET_ERROR_UNKNOWN. This is because if the num_of_entries = 1 the behavior
 *          can match that if setup failed then failed otherwise it will return AMTR_TYPE_RET_SUCCESS.
 *          2. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *-----------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T AMTRDRV_MGR_SetAddrList2LocalUnit(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    SWDRV_TYPE_L2_Address_Info_T   arl_entry;
    UI32_T                         i;
#if (SYS_CPNT_MAINBOARD == TRUE)
    SWDRV_Trunk_Info_T             trunk_port_info;
    AMTRDRV_TYPE_Record_T            address_record;
    UI32_T                         record_index =0;
    UI32_T                         j;
#endif /* SYS_CPNT_MAINBOARD */
    AMTR_TYPE_Ret_T                result = AMTR_TYPE_RET_SUCCESS;
    BOOL_T                         set_om_result;
    DEV_AMTRDRV_Ret_T              set_asic_retval= DEV_AMTRDRV_SUCCESS;
    if(amtrdrv_mgr_isc_test_returndonothing1 == 2)
        return AMTR_TYPE_RET_SUCCESS;

    SYSFUN_USE_CSC(AMTR_TYPE_RET_ERROR_UNKNOWN);
    for (i = 0;i < num_of_entries;i++){

        if(CHECK_MAC_IS_NULL(addr_buf[i].mac)) /*added by Jinhua Wei,to remvo warning*/
        {
            continue;
        }

        if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
        {
            /* Converting the record of AMTR_TYPE_AddrEntry_T format to
             * SWDRV_TYPE_L2_Address_Info_T format to program chip
             */
            AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry(&addr_buf[i], &arl_entry);


                /* CPU MAC is set to all chips, and skip Job-queue sync.
                 * If CPU MAC had not been set to ASIC when SWDRV is setting
                 * L3Bit of the CPU MAC, the operation of setting L3bit will fail.
                 * To avoid this error, the operation of writing CPU MAC to ASIC
                 * will be done directly.
                 */
            if (addr_buf[i].source == AMTR_TYPE_ADDRESS_SOURCE_SELF) /* CPU MAC */
            {
                set_asic_retval = DEV_AMTRDRV_PMGR_SetInterventionEntry(addr_buf[i].vid, 
                        addr_buf[i].mac, DEV_AMTRDRV_ALL_DEVICE, DEV_AMTRDRV_INTERV_MODE_DA);

                AMTRDRV_MGR_ConvertGatewayReturnValue(set_asic_retval, &result);              
            }
            else
            {
                /* Try to set to the first chip to check the chip has space for this entry or not
                 */
                set_asic_retval = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_FIRST_DEVICE,&arl_entry);
                AMTRDRV_MGR_ConvertGatewayReturnValue(set_asic_retval, &result);
                
                if (set_asic_retval!=DEV_AMTRDRV_SUCCESS)
                {
                    UI32_T total_cnt=0;
                    total_cnt = AMTRDRV_OM_GetTotalCounter();

                    if (amtrdrv_mgr_debug_show_collision_flag)
                    {
                        if(total_cnt < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
                        {
                            BACKDOOR_MGR_Printf("VID:%d ", addr_buf[i].vid);
                            BACKDOOR_MGR_Printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x ",
                                    addr_buf[i].mac[0], addr_buf[i].mac[1], addr_buf[i].mac[2],
                                    addr_buf[i].mac[3], addr_buf[i].mac[4], addr_buf[i].mac[5]);
                            BACKDOOR_MGR_Printf("Ifindex:%d ", addr_buf[i].ifindex);
                            BACKDOOR_MGR_Printf("life_time:%d ", addr_buf[i].life_time);
                            BACKDOOR_MGR_Printf("Source: %d\r\n", addr_buf[i].source);
                        }
                    }
                #if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
                    AMTRDRV_OM_LogCollisionVlanMac(addr_buf[i].vid, addr_buf[i].mac);
                #endif
                }
            }

            if (result==AMTR_TYPE_RET_SUCCESS)
            {
                AMTRDRV_MGR_DBGMSG("Address added to hardware ARL table successfully");
            }
            else
            {
                AMTRDRV_MGR_DBGMSG("Failed to add address to hardware ARL table");
                amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
                continue;
            }
        }else{

            /* Slave must set CPU MAC before SWDRV setting L3Bit. 
             * To make sure this happen,set CPU MAC to gateway directly.
             * Or the CPU MAC will be L2 entry. (L3Bit=FALSE.)
             */
            if (addr_buf[i].source == AMTR_TYPE_ADDRESS_SOURCE_SELF) /* CPU MAC */
            {
                set_asic_retval = DEV_AMTRDRV_PMGR_SetInterventionEntry(addr_buf[i].vid, addr_buf[i].mac,
                                           DEV_AMTRDRV_ALL_DEVICE, DEV_AMTRDRV_INTERV_MODE_DA);
            }        
            if (set_asic_retval==DEV_AMTRDRV_SUCCESS)
            {
                AMTRDRV_MGR_DBGMSG("Address added to hardware ARL table successfully");
            }
            else
            {
                AMTRDRV_MGR_DBGMSG("Failed to add address to hardware ARL table");
                amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
                continue;
            }
            
        /*add by Tony.Lei, add debug code*/
          if(amtrdrv_mgr_isc_test_returndonothing1 == 1)
            return AMTR_TYPE_RET_SUCCESS;
        }

#if (SYS_CPNT_MAINBOARD == TRUE)
        /* ADD or REPLACE the addr_entry to Hash table
         */
        amtrdrv_memcpy(&address_record.address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));
        address_record.hit_bit_value_on_local_unit = 1;
        address_record.trunk_hit_bit_value_for_each_unit = 0;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        address_record.mark = FALSE;
#endif
        if (IS_TRUNK(address_record.address.ifindex))
        {
            /* If the address is learnt on trunk port we need to set the trunk_hit_bit_value
             * if the unit is this trunk member ports.
             */
            SWDRV_GetTrunkInfo(address_record.address.ifindex, &trunk_port_info);
            for (j=0; j<trunk_port_info.member_number; j++)
            {
                address_record.trunk_hit_bit_value_for_each_unit |= ((0x1) << (trunk_port_info.member_list[j].unit-1));
            }
        }

        /* CPU MAC is set by direcly calling gateway API and skip Job-queue sync.
         */
        if (address_record.address.source == AMTR_TYPE_ADDRESS_SOURCE_SELF) /* CPU MAC */
        {
            set_om_result=AMTRDRV_OM_SetAddrHashOnly((UI8_T *)&address_record,&record_index);
        }
        else
        {
            set_om_result=AMTRDRV_OM_SetAddr((UI8_T *)&address_record,&record_index);
        }
        
        if (set_om_result==TRUE)
        {
            AMTRDRV_MGR_DBGMSG("Address added to hash table successfully");
            /* insert the entry into aging_list if this entry is learnt on local unit
             */
#if defined(AMTRDRV_SLAVE_REPEATING_TEST) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
            if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
            {
                AMTRDRV_OM_NABufferRestoreDeleteRecord((UI8_T *)&address_record);
            }
#endif
            if (AMTRDRV_MGR_IsInsert2AgingList(address_record.address.ifindex,address_record.address.life_time)){
                AMTRDRV_OM_LocalAgingListEnqueue(record_index);
            }else /*for config entries, delete them from aging list*/
                AMTRDRV_OM_LocalAginListDeleteEntry(record_index);

            if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
            {
                AMTRDRV_OM_SyncQueueEnqueue(record_index);
            }
        }
        else
        {
            AMTRDRV_MGR_DBGMSG("Failed to add address to hash table");
            /*Only Master had written first chip.
             */
            if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
            {
                DEV_AMTRDRV_PMGR_DeleteAddr(address_record.address.vid,address_record.address.mac);
            }
            amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
            result = AMTR_TYPE_RET_ERROR_UNKNOWN;
            continue;
        }
#endif  /* SYS_CPNT_MAINBOARD */
    }
    SYSFUN_RELEASE_CSC();
    return result;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList2RemoteUnit
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry to remote units
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrList2RemoteUnit(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
#if (SYS_CPNT_STACKING == TRUE)

    SYSFUN_USE_CSC(FALSE);
    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI32_T                      i;
        L_MM_Mref_Handle_T*         mref_handle_p;
        AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
        UI32_T                      pdu_len;
        UI16_T                      return_unit_bmp=0;
        UI16_T                      unit_bmp=0;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                  L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_SET_ADDR_LIST) /* user_id */);
        isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        isc_buffer_p->data.entries.number_of_entries= 0;
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);

        for (i=0;i< num_of_entries;i++)
        {
            if(CHECK_MAC_IS_NULL(addr_buf[i].mac)) /*added by Jinhua Wei,to remove warning*/
            {
                continue;
            }
            amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries].address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));
            isc_buffer_p->data.entries.number_of_entries ++;

            if ((unit_bmp!=0) && (isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET))
            {
                isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_LIST;
                return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

                if (return_unit_bmp !=0)
                {
                    AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }

                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_SET_ADDR_LIST) /* user_id */);
                isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                if (isc_buffer_p==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }
                isc_buffer_p->data.entries.number_of_entries= 0;
            }
        }

        if ((unit_bmp !=0) && (isc_buffer_p->data.entries.number_of_entries > 0))
        {
            isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_LIST;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
        else
        {
            L_MM_Mref_Release(&mref_handle_p);
        }
    }
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_STACKING */
    return TRUE;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList2RemoteUnittest
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry to remote units
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrList2RemoteUnittest(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
#if (SYS_CPNT_STACKING == TRUE)

    SYSFUN_USE_CSC(FALSE);
    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI32_T                      i;
        L_MM_Mref_Handle_T*         mref_handle_p;
        AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
        UI32_T                      pdu_len;
        UI16_T                      return_unit_bmp=0;
        UI16_T                      unit_bmp=0;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                  L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_SET_ADDR_LIST) /* user_id */);
        isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        isc_buffer_p->data.entries.number_of_entries= 0;
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);

        for (i=0;i< num_of_entries;i++)
        {
            isc_buffer_p->data.entries.number_of_entries ++;

            if ((unit_bmp!=0) && (isc_buffer_p->data.entries.number_of_entries != 0))
            {
                isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_LIST;
                return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

                if (return_unit_bmp !=0)
                {
                    AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }

                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_SET_ADDR_LIST) /* user_id */);
                isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                if (isc_buffer_p==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }
                isc_buffer_p->data.entries.number_of_entries= 0;
            }
        }

        if ((unit_bmp !=0) && (isc_buffer_p->data.entries.number_of_entries > 0))
        {
            isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_LIST;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
        else
        {
            L_MM_Mref_Release(&mref_handle_p);
        }
    }
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_STACKING */
    return TRUE;
}

#endif /* SYS_CPNT_MAINBOARD */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrDirectly
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry quickly without running hash FSM
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry          - Address entry
 *          BOOL_T                is_local_hash_only   - only update hash or need to update chip
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : 1.If we need to update chip we need to send by ISC to notify other units to update
 *            their table and chip as well. If only need to update hash it means this may the request
 *            from MIB to store the record with life_time = other in the master database only
 *          2.We support this function so the caller need to update his database by itself
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrDirectly(AMTR_TYPE_AddrEntry_T *addr_entry,BOOL_T is_local_hash_only)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T address_record;
    SWDRV_Trunk_Info_T  trunk_port_info;
    UI32_T              i;
    UI32_T              record_index;
    BOOL_T              result;
#endif /* SYS_CPNT_MAINBOARD */

    SYSFUN_USE_CSC(FALSE);
    if (!is_local_hash_only)
    {
        SWDRV_TYPE_L2_Address_Info_T   arl_entry;

        arl_entry.vid = addr_entry->vid;
        amtrdrv_memcpy(arl_entry.mac, addr_entry->mac,AMTR_TYPE_MAC_LEN);
        arl_entry.priority = AMTRDRV_CONVERT_TO_ARL_ENTRY_PRIORITY(addr_entry->priority);
        AMTRDRV_CONVERT_TO_ARL_ENTRY_ACTION(addr_entry, &arl_entry);
        if (!IS_TRUNK(addr_entry->ifindex))
        {
            /* normal port
             */
            arl_entry.unit = STKTPLG_POM_IFINDEX_TO_UNIT(addr_entry->ifindex);
            arl_entry.port = STKTPLG_POM_IFINDEX_TO_PORT(addr_entry->ifindex);
            arl_entry.trunk_id = 0;
            arl_entry.is_trunk = FALSE;
        }
        else
        {
            /* trunk port
             */
            arl_entry.unit = arl_entry.port = 0;
            arl_entry.trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(addr_entry->ifindex)-1;
            arl_entry.is_trunk = TRUE;
        }
        arl_entry.is_static = TRUE;
        if (DEV_AMTRDRV_FAIL == DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice(DEV_AMTRDRV_ALL_DEVICE,&arl_entry))
        {
            AMTRDRV_MGR_DBGMSG("Failed to set address to hardware ARL table");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    }
#if (SYS_CPNT_MAINBOARD == TRUE)
    amtrdrv_memcpy(&address_record.address,addr_entry,sizeof(AMTR_TYPE_AddrEntry_T));
    address_record.hit_bit_value_on_local_unit = 1;
    address_record.trunk_hit_bit_value_for_each_unit = 0;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    address_record.mark=FALSE;
#endif
    if (IS_TRUNK(address_record.address.ifindex))
    {
        /*
         * If the address is learnt on trunk port we need to set the trunk_hit_bit_value
         * if the unit is this trunk member ports.
         */
        SWDRV_GetTrunkInfo(address_record.address.ifindex, &trunk_port_info);
        for (i=0; i<trunk_port_info.member_number; i++)
            address_record.trunk_hit_bit_value_for_each_unit |= ((0x1) << (trunk_port_info.member_list[i].unit-1));

    }
    address_record.aging_timestamp = SYSFUN_GetSysTick();
    result = AMTRDRV_OM_SetAddrHashOnly((UI8_T *)&address_record ,&record_index);

    if (result)
    {
        AMTRDRV_MGR_DBGMSG("Address insert to hash table successfully");
        AMTRDRV_OM_SyncQueueEnqueue(record_index);
    }
    else if (!is_local_hash_only)
    {
        DEV_AMTRDRV_PMGR_DeleteAddr(address_record.address.vid,address_record.address.mac);
        AMTRDRV_MGR_DBGMSG("Failed to insert address to hash table");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if ((!is_local_hash_only) &&(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE))
    {
        UI16_T  unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if ( unit_bmp !=0 )
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_SET_ADDR_DIRECTLY) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_DIRECTLY;
            isc_buffer_p->data.entries.number_of_entries = 1;
            amtrdrv_memcpy(&isc_buffer_p->data.entries.record[0],&address_record,sizeof(AMTRDRV_TYPE_Record_T));

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_CALLBYAGENT_SID,
                                     mref_handle_p,
                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                     AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to delete all addree in slave units.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrEntryList
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry from Hash table & chip in whole system
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 *          UI32_T num_of_entries               - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will not execute in the module unit and it will return TRUE directly
 *             Due to isc callback function can't be null in module part we can't block
 *             the function by SYS_CPNT_MAINBOARD. We only can do it inside the function.
 *             In the module part it will return true directly. Actually this case
 *             won't happen in module unit.
 *          2. We support this function so the caller need to update his database as well since
 *             we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table, Job Queue, Sync Queue
 *             (learnt entry and security entry that will age out) and callback Queue.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrEntryList(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T                      result = TRUE;

#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T         address_record;
    UI32_T                      i;
    UI32_T                      index;
#if(SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
    UI32_T                      pdu_len;
    UI16_T                      return_unit_bmp=0;
    UI16_T                      unit_bmp=0;
#endif /* SYS_CPNT_STACKING */

    SYSFUN_USE_CSC(FALSE);
#if(SYS_CPNT_STACKING == TRUE)
    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
        L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_buffer_p==NULL)
    {
        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    /* initialize ISC service information if the system support this function
     */
    isc_buffer_p->data.entries.number_of_entries = 0;
    unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
#endif /* SYS_CPNT_STACKING */

    for(i=0;i<num_of_entries;i++)
    {
        /* if the mac = null mac it means this is an invalid address and we don't need to process it
         */
        if(CHECK_MAC_IS_NULL(addr_buf[i].mac)) /*added by Jinhua Wei,to remove warning*/
        {
            continue;
        }
        amtrdrv_memcpy(&address_record.address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));
        /* update local OM
         * Assume update OM should always return success so we didn't do any error handle
         */
        if(!AMTRDRV_OM_DeleteAddr((UI8_T *)&address_record,&index))
        {
            /* If we can't update record in OM as delete action we need to set this record = NULL
             * in case the remote unit does't have this record but master does
             */
            amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
            result = FALSE ;
            continue;
        }

        if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
        {
            AMTRDRV_OM_SyncQueueEnqueue(index);

            /* Only delete record event need to put in callback queue to let sys callback task can
             * notify AMTRL3 there is a record deleted and it needs to update its database too
             */
            AMTRDRV_MGR_CallBackQueueEnqueue(index);

#if(SYS_CPNT_STACKING == TRUE)

            /* Sending ISC packet to notify remote units to update it's local database & chip
             */
            amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&address_record,sizeof(AMTRDRV_TYPE_Record_T));
            isc_buffer_p->data.entries.number_of_entries ++;

            if((isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)&&(unit_bmp != 0))
            {
                isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_LIST;
                return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                     mref_handle_p,
                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                     AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

                if (return_unit_bmp !=0)
                {
                    AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
                    result = FALSE;
                }

                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
                isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                if (isc_buffer_p==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }
                /* initialize ISC service information if the system support this function
                 */
                isc_buffer_p->data.entries.number_of_entries = 0;
            }
#endif /* SYS_CPNT_STACKING */
        }
    }

#if(SYS_CPNT_STACKING == TRUE)
    if((AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)&& (unit_bmp != 0) && (isc_buffer_p->data.entries.number_of_entries > 0) )
    {
        isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_LIST;
        return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                         mref_handle_p,
                                         SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                         AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        if (return_unit_bmp !=0)
        {
            AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set all addree in slave units.");
            result = FALSE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrEntryListFromOm
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry from Hash table in whole system
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will not execute in the module unit and it will return TRUE directly
 *             Due to isc callback function can't be null in module part we can't block
 *             the function by SYS_CPNT_MAINBOARD. We only can do it inside the function.
 *             In the module part it will return true directly. Actually this case
 *             won't happen in module unit.
 *          2. We support this function so the caller need to update his database as well since
 *             we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table, Job Queue, Sync Queue
 *             (learnt entry and security entry that will age out) and callback Queue.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrEntryListFromOm(AMTR_TYPE_AddrEntry_T *addr_buf)
{

#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T   address_record;
    UI32_T  index;

    /* if the mac = null mac it means this is an invalid address and we don't need to process it
     */
    if(CHECK_MAC_IS_NULL(addr_buf->mac)) /*added by Jinhua Wei,to remove warning*/
       return FALSE ;

    amtrdrv_memcpy(&address_record.address,addr_buf,sizeof(AMTR_TYPE_AddrEntry_T));
    /* update local OM
     * Assume update OM should always return success so we didn't do any error handle
     */
    if(!AMTRDRV_OM_DeleteAddr((UI8_T *)&address_record,&index))
        return FALSE ;


     /*Set the flag for port move just delete the OM*/
    AMTRDRV_OM_SetReseverdState(index,AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS);

    AMTRDRV_OM_SyncQueueEnqueue(index);

    /* Only delete record event need to put in callback queue to let sys callback task can
     * notify AMTRL3 there is a record deleted and it needs to update its database too
     */
    AMTRDRV_MGR_CallBackQueueEnqueue(index);


#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrDirectly
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry quickly without running hash FSM
 * INPUT  :        addr_entry         - address info
 *          BOOL_T is_local_hash_only - only need to update hash table or need to update chip as well
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API shall only be called for deleting (life_time = Other) or (ifindex =0 but not cpu mac)
 *            to hash table since this kind of records only store in hash table only and not in chip.
 *            (When specify is_local_has_only as TRUE)
 *          2.We support this function so the caller need to update his database as well since
 *            we won't have any information for this entry after finishing.
 *          3.This function will update Hash Table and ASIC ARL Table.
 *            (When specify is_local_has_only as FALSE)
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrDirectly(AMTR_TYPE_AddrEntry_T *addr_entry, BOOL_T is_local_hash_only)
{

#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T  address_record;
    BOOL_T               result = FALSE;
    UI32_T               index;
#endif /* SYS_CPNT_MAINBOARD */

    SYSFUN_USE_CSC(FALSE);
    if(!is_local_hash_only)
    {
        if(!DEV_AMTRDRV_PMGR_DeleteAddr(addr_entry->vid,addr_entry->mac))
        {
            AMTRDRV_MGR_DBGMSG("Address delete from chip failed");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    }

#if (SYS_CPNT_MAINBOARD == TRUE)
    amtrdrv_memcpy(&address_record.address,addr_entry,sizeof(AMTR_TYPE_AddrEntry_T));

    result = AMTRDRV_OM_DeleteAddrFromHashOnly((UI8_T *)&address_record, &index);

    if (result)
    {
        AMTRDRV_MGR_DBGMSG("Address delete from hash table successfully");
        AMTRDRV_OM_SyncQueueEnqueue(index);
    }
    else
    {
        AMTRDRV_MGR_DBGMSG("Failed to delete address from hash table");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    /* Sending ISC packet to update Hash table in salve units */
#if (SYS_CPNT_STACKING == TRUE)
    if ( (!is_local_hash_only) && (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE))
    {
        UI16_T               unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_DIRECTLY) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_DIRECTLY;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = addr_entry->vid;
            amtrdrv_memcpy(isc_buffer_p->data.entries.record[0].address.mac, addr_entry->mac, AMTR_TYPE_MAC_LEN);

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if (return_unit_bmp !=0)
            {
               AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to delete all addree in slave units.");
               SYSFUN_RELEASE_CSC();
               return FALSE;
            }
        }
    }
#endif /*SYS_CPNT_STACKING*/
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrFromChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry quickly without running hash FSM
 * INPUT  :        addr_entry         - address info
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API is only support deleting (life_time = Other) or (ifindex =0 but not cpu mac)
 *            to hash table since this kind of records only store in hash table only not chip.
 *          2.We support this function so the caller need to update his database as well since
 *            we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table and ASIC ARL Table.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrFromChip(AMTR_TYPE_AddrEntry_T *addr_entry)
{

#if (SYS_CPNT_MAINBOARD == TRUE)
    UI16_T  unit_bmp ;

    SYSFUN_USE_CSC(FALSE);

    if(!DEV_AMTRDRV_PMGR_DeleteAddr(addr_entry->vid,addr_entry->mac))
    {
        AMTRDRV_MGR_DBGMSG("Address delete from chip failed");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
     unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

    if(unit_bmp != 0){

        L_MM_Mref_Handle_T*         mref_handle_p;
        AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
        UI32_T                      pdu_len;
        UI16_T                      return_unit_bmp=0;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                  L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_DIRECTLY) /* user_id */);

        isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p == NULL){

            AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_DIRECTLY;
        isc_buffer_p->data.entries.number_of_entries = 1;
        isc_buffer_p->data.entries.record[0].address.vid = addr_entry->vid;
        amtrdrv_memcpy(isc_buffer_p->data.entries.record[0].address.mac, addr_entry->mac, AMTR_TYPE_MAC_LEN);

        return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                         mref_handle_p,
                                         SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                         AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

        if (return_unit_bmp !=0){

           AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to delete all addree in slave units.");
           SYSFUN_RELEASE_CSC();
           return FALSE;
        }
    }
#endif /*SYS_CPNT_STACKING*/
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DelNumOfDynamicAddrs
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to handle to delete number of dynamic addresses when system ARL
 *          capacity is become smaller.
 * INPUT  : UI32_T num_of_entries   - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This API is called by AMTR when ARL capacity is become smaller and AMTR
 *          need to tell how many entries need to be deleted.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DelNumOfDynamicAddrs(UI32_T num_of_entries)
{
    L_DLST_ShMem_Indexed_Dblist_T *dblist;
    AMTRDRV_TYPE_Record_T     address_record;
    UI32_T                  i=0;
    UI32_T                  index;
    UI8_T                   action;
#if(SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
    UI32_T                      pdu_len;
    UI16_T                      return_unit_bmp=0;
    UI16_T                      unit_bmp=0;
#endif /* SYS_CPNT_STACKING */

    SYSFUN_USE_CSC(FALSE);

    /* Get the first dynamic address from query group -- AMTRDRV_OM_QUERY_GROUP_LIFE_TIME_DELETE_ON_TIMEOUT */
    if(!AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT-1,&dblist,&index,(UI8_T *)&address_record,&action))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
#if(SYS_CPNT_STACKING == TRUE)
    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_buffer_p==NULL)
    {
        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    /* initialize ISC service information if the system support this function */
    isc_buffer_p->data.entries.number_of_entries = 0;
    unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
#endif /* SYS_CPNT_STACKING */

    while(i < num_of_entries)
    {
        /* We only process the record who's action = "SET" since the record with action ="Delete"
         * means the record is killed. But why its still in the OM ? This is because it may wait
         * for sync to Hisam or callback other CSCs then we can really delete it
         */
        if(action ==AMTR_TYPE_COMMAND_SET_ENTRY)
        {
            AMTRDRV_MGR_DeleteAddrEntryList(1,&address_record.address);
            i++;
#if(SYS_CPNT_STACKING == TRUE)
            amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&address_record,sizeof(AMTRDRV_TYPE_Record_T));
            isc_buffer_p->data.entries.number_of_entries ++;
            /* We only can process max_entreis_in_one_packet one time
             */
            if((isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)&&(unit_bmp != 0))
            {
                isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_LIST;
                return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

                if (return_unit_bmp !=0)
                {
                    AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to delete number of addresses in slave units.");
                }
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
                isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                if (isc_buffer_p==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }
                /* initialize ISC service information if the system support this function
                 */
                isc_buffer_p->data.entries.number_of_entries = 0;
            }
#endif  /* SYS_CPNT_STACKING */
        }
        else /* If the action = "Delete" then we need to get next record */
        {
            if(!AMTRDRV_OM_GetNextEntryFromGivingQueryGroup(dblist, &index,(UI8_T *)&address_record,&action))
            {
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#if(SYS_CPNT_STACKING == TRUE)
    if( (isc_buffer_p->data.entries.number_of_entries > 0)&&(unit_bmp != 0))
    {
        isc_buffer_p->service_id  = AMTRDRV_MGR_DELETE_ADDR_LIST;
        return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                         mref_handle_p,
                                         SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                         AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        if (return_unit_bmp !=0)
        {
            AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to delete number of addresses in slave units.");
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}
#endif /* SYS_CPNT_MAINBOARD */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAllAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete all addresses from Hash table & chip
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAllAddr(void)
{
#if(SYS_CPNT_MAINBOARD == TRUE)

#if(SYS_CPNT_STACKING == TRUE)
    UI16_T                 unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_ALL;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op=FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ALL_ADDR) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ALL_ADDR;

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);
#ifdef BACKDOOR_OPEN
            if(amtrdrv_mgr_bd_display)
            {
                BACKDOOR_MGR_Printf("\n");
                BACKDOOR_MGR_Printf("==================AMTRDRV_MGR_DeleteAllAddr=============\n");
                BACKDOOR_MGR_Printf("*    Shown the result of Send ISC_MCastReliable( )                        *\n");
                BACKDOOR_MGR_Printf("=====================================================\n");
                BACKDOOR_MGR_Printf(" return unit bitmap: %d\r\n",return_unit_bmp);
            }
#endif /* BACKDOOR_OPEN */

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAllAddr failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */

    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which life_time is matced
 * INPUT  : AMTR_TYPE_AddressLifeTime_T life_time      -- specified life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.For example if we need to delete all dynamic addresses we need to
 *            set life_time = delete_on_time_out
 *          2.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T                 unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Take the token to run
        */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME;
    blocked_command_p->life_time = life_time;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op=FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.life_time = life_time;

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByLifeTime failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */

    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrBySource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which source is matched
 * INPUT  : AMTR_TYPE_AddressSource_T source      --source
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrBySource(AMTR_TYPE_AddressSource_T source)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T                unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_SOURCE
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_SOURCE;
    blocked_command_p->source = source;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if(unit_bmp != 0 )
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.source = source;

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if(return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrBySource failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */

    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex is matched
 * INPUT  : UI32_T ifindex      - which port/trunk
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPort(UI32_T ifindex)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T                unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_PORT
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT;
    blocked_command_p->ifindex = ifindex;

    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_PORT;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPort Failed to send ISC to slave.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex & life_time is matched
 * INPUT  : UI32_T                        ifindex     - which port/trunk
 *          AMTR_TYPE_AddressLifeTime_T   life_time   - which life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T               unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_PORT_N_LIFETIME
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->life_time = life_time;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;

#if(SYS_CPNT_VXLAN == TRUE)
    /* If ifindex is real VXLAN tunnel port, convert to logical ifindex.
     */
    if(VXLAN_TYPE_IS_R_PORT(ifindex))
    {
        UI32_T l_vxlan_port;

        if(VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(ifindex, l_vxlan_port)==0)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->ifindex = l_vxlan_port;
    }
#endif

    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME_N_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME_N_PORT;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            isc_buffer_p->data.entries.record[0].address.life_time = life_time;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPortAndLifeTime failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex & source is matched
 * INPUT  : UI32_T                        ifindex     - which port/trunk
 *        : AMTR_TYPE_AddressSource_T     source      - which source
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndSource(UI32_T ifindex,AMTR_TYPE_AddressSource_T source)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T              unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_PORT_N_SOURCE
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);

    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->source=source;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_VXLAN == TRUE)
    /* If ifindex is real VXLAN tunnel port, convert to logical ifindex.
     */
    if(VXLAN_TYPE_IS_R_PORT(ifindex))
    {
        UI32_T l_vxlan_port;

        if(VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(ifindex, l_vxlan_port)==0)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->ifindex = l_vxlan_port;
    }
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE_N_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE_N_PORT;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            isc_buffer_p->data.entries.record[0].address.source = source;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPortAndSource failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVID
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid is matched
 * INPUT  : UI32_T                   vid         - VLan number
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVID(UI32_T vid)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T               unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_VID;
    blocked_command_p->vid = vid;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_VXLAN == TRUE)
    /* If VID is Dot1q VLAN ID or logical VFI, don't convert.
     * If VID is real VFI, convert to logical VFI.
     */
    if(AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi;

        if(AMTRDRV_OM_ConvertRvfiToLvfi(vid, &l_vfi)==FALSE)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->vid = l_vfi;
    }
#endif

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp !=0 )
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByVID failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVidAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & life_time is matched
 * INPUT  : UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressLifeTime_T   life_time   - which life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVidAndLifeTime(UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T             unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_N_LIFETIME
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);

    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME;
    blocked_command_p->vid = vid;
    blocked_command_p->is_sync_op = FALSE;
#if(SYS_CPNT_VXLAN == TRUE)
    /* If VID is Dot1q VLAN ID or logical VFI, don't convert.
     * If VID is real VFI, convert to logical VFI.
     */
    if(AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi;

        if(AMTRDRV_OM_ConvertRvfiToLvfi(vid, &l_vfi)==FALSE)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->vid = l_vfi;
    }
#endif
    blocked_command_p->life_time = life_time;
    blocked_command_p->vlan_counter = 0;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if(unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            isc_buffer_p->data.entries.record[0].address.life_time= life_time;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByVidAndLifeTime failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & source is matched
 * INPUT  : UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressSource_T     source      - which source
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVidAndSource(UI32_T vid,AMTR_TYPE_AddressSource_T source)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T             unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_N_SOURCE
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);

    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE;
    blocked_command_p->vid=vid;
    blocked_command_p->source=source;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            isc_buffer_p->data.entries.record[0].address.source= source;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByVidAndSource failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVIDnPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & ifindex is matched
 * INPUT  : UI32_T                   vid         - VLan number
 *          UI32_T                   ifindex     - which port / trunk
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVIDnPort(UI32_T ifindex,UI32_T vid)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T               unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    
    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_N_PORT
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->vid = vid;
    blocked_command_p->vlan_counter = 0;
    
#if(SYS_CPNT_VXLAN == TRUE)
    /* If ifindex is real VXLAN tunnel port, convert to logical ifindex.
     */
    if(VXLAN_TYPE_IS_R_PORT(ifindex))
    {
        UI32_T l_vxlan_port;

        if(VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(ifindex, l_vxlan_port)==0)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->ifindex = l_vxlan_port;
    }

    /* If VID is Dot1q VLAN ID or logical VFI, don't convert.
     * If VID is real VFI, convert to logical VFI.
     */
    if(AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi;

        if(AMTRDRV_OM_ConvertRvfiToLvfi(vid, &l_vfi)==FALSE)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->vid = l_vfi;
    }
#endif
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    
#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByVIDnPort failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime
 *------------------------------------------------------------------------------
 * Purpose  : Delete all of the addresses with the specified life time of a
 *            specific port and vid from Hash table & chip
 * INPUT    : UI32_T ifindex              - which port/trunk
 *            UI32_T vid                  - vlan id
 *            AMTR_TYPE_AddressLifeTime_T - other, invalid, permanent, del on reset, del on timeout
 *            BOOL_T sync_op              - TRUE : The function will not
 *                                                 return until the
 *                                                 delete operation is
 *                                                 done.
 *                                          FALSE: The function is
 *                                                 returned when the
 *                                                 delete command had
 *                                                 been passed to the
 *                                                 AMTRDRV task.
 *
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *            2. For sync_op==TRUE, only support life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime(UI32_T ifindex, UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op)
{
    BOOL_T ret=TRUE;
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T              unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    /* For sync_op==TRUE, only support life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
     */
    if (sync_op==TRUE && life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
    {
        return FALSE;
    }

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_NPORT_N_LIFETIME
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->vlan_counter = 1;
    blocked_command_p->vlanlist[0] = vid;
    blocked_command_p->is_sync_op = sync_op;

#if(SYS_CPNT_VXLAN == TRUE)
    /* If VID is Dot1q VLAN ID or logical VFI, don't convert.
     * If VID is real VFI, convert to logical VFI.
     */
    if(AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi;

        if(AMTRDRV_OM_ConvertRvfiToLvfi(vid, &l_vfi)==FALSE)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->vlanlist[0] = l_vfi;
    }

    /* If ifindex is real VXLAN tunnel port, convert to logical ifindex.
     */
    if(VXLAN_TYPE_IS_R_PORT(ifindex))
    {
        UI32_T l_vxlan_port;

        if(VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(ifindex, l_vxlan_port)==0)
        {
            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertRvfiToLvfi returns error");
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        blocked_command_p->ifindex = l_vxlan_port;
    }
#endif
    blocked_command_p->vlanlist[1] = 0;
    blocked_command_p->life_time = life_time;

    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                ret=FALSE;
                goto exit;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME_N_PORT;
            isc_buffer_p->type = 0;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            isc_buffer_p->data.entries.record[0].address.life_time = life_time;
            isc_buffer_p->data.entries.is_synchronous_op=sync_op;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime failed to send ISC.");
                ret=FALSE;
                goto exit;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
exit:
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (sync_op==TRUE)
        {
            UI32_T rc;

            rc=SYSFUN_TakeSem(amtrdrv_sync_op_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
            if (rc!=SYSFUN_OK)
            {
                printf("%s(%d):Take amtrdrv_sync_op_smid sem error.(rc=%lu,amtrdrv_sync_op_smid=%lu)\r\n", __FUNCTION__, __LINE__,
                    (unsigned long)rc, (unsigned long)amtrdrv_sync_op_smid);
            }
            rc=SYSFUN_GiveSem(amtrdrv_local_finished_smid);
            if (rc!=SYSFUN_OK)
            {
                printf("%s(%d):Give amtrdrv_local_finished_smid sem error.(rc=%lu,amtrdrv_local_finished_smid=%lu)\r\n", __FUNCTION__, __LINE__,
                    (unsigned long)rc, (unsigned long)amtrdrv_local_finished_smid);
            }
        }
    }

#endif /* SYS_CPNT_MAINBOARD */

    SYSFUN_RELEASE_CSC();
    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime
 *------------------------------------------------------------------------------
 * Purpose  : Delete all dynamic addresses of a specific port from Hash table & chip
 * INPUT    : UI32_T ifindex              - which port/trunk
 *            UI32_T vlanbitmap           - vlan bit map
 *            AMTR_TYPE_AddressLifeTime_T - other, invalid, permanent, del on reset, del on timeout
 *
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime(UI32_T ifindex,
                                                             UI16_T *vlan_p,
                                                             UI32_T vlan_count,
                                                             AMTR_TYPE_AddressLifeTime_T life_time,
                                                             XSTP_MGR_MstpInstanceEntry_T * mstp_entry_p)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T              unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T * blocked_command_p ;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_NPORT_N_LIFETIME
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);


    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->life_time = life_time;
    blocked_command_p->vlan_counter = vlan_count;
    memcpy(blocked_command_p->vlanlist, vlan_p, vlan_count*2);
    blocked_command_p->vlanlist[vlan_count] = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME_N_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME_N_PORT;
            isc_buffer_p->type = 1;
            isc_buffer_p->data.mstp_entry_callback.ifindex = ifindex;
            isc_buffer_p->data.mstp_entry_callback.mstp_entry = *mstp_entry_p;
            isc_buffer_p->data.mstp_entry_callback.life_time = life_time;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddr_ByPortAndVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid , port & source is matched
 * INPUT  : UI32_T                        ifindex     - which port / trunk
            UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressSource_T     source      - which life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndVidAndSource(UI32_T ifindex, UI32_T vid, AMTR_TYPE_AddressSource_T source)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T               unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_N_PORT_N_SOURCE
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);

    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->vid = vid;
    blocked_command_p->source = source;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE_N_PORT) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE_N_PORT;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            isc_buffer_p->data.entries.record[0].address.source = source;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPortAndVidAndSource failed to send ISC.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;

}


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndVidExceptCertainAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & ifindex is matched
 * INPUT  : UI32_T                   vid         - VLan number
 *          UI32_T                   ifindex     - which port / trunk
 *          UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *          UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *          UI32_T number_of_entry_in_list
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndVidExceptCertainAddr(UI32_T ifindex,
                                                           UI32_T vid,
                                                           UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                           UI32_T number_of_entry_in_list)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T               unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    AMTRDRV_MsgCookis    *msg_cookie;

    SYSFUN_USE_CSC(FALSE);

    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- DELETE_BY_VID_N_PORT
     */
    msg_cookie = &blocked_command_p->cookie;

    memcpy(msg_cookie->mac_list_ar,mac_list_p,SYS_ADPT_MAC_ADDR_LEN*number_of_entry_in_list);
    memcpy(msg_cookie->mask_list_ar,mask_list_p,SYS_ADPT_MAC_ADDR_LEN*number_of_entry_in_list);
    msg_cookie->num_in_list  = number_of_entry_in_list;
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR;
    blocked_command_p->ifindex=ifindex;
    blocked_command_p->vid=vid;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT_EXCEPT_CERTAIN_ADDR) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT_EXCEPT_CERTAIN_ADDR;
            isc_buffer_p->data.masklist.record.address.ifindex = ifindex;
            isc_buffer_p->data.masklist.record.address.vid = vid;
            isc_buffer_p->data.masklist.num_of_mask_list = number_of_entry_in_list;
            memcpy(isc_buffer_p->data.masklist.mask_mac_list, mac_list_p,
                   SYS_ADPT_MAC_ADDR_LEN*number_of_entry_in_list);
            memcpy(isc_buffer_p->data.masklist.mask_list, mask_list_p,
                   SYS_ADPT_MAC_ADDR_LEN*number_of_entry_in_list);
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByVIDnPort failed to send ISC.");
                SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif /* SYS_CPNT_STACKING */

    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}


/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_GetExactAddrFromChipWithoutISC
 *------------------------------------------------------------------------------
 * Purpose  : This function will search port related to input mac&vid in ARL
 * INPUT    : addr_entry->mac - mac address
 *            addr_entry->vid - vlan id
 * OUTPUT   : addr_entry      - addresss entry info
 *                              if hw learn, only ifindex is valid
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr_mgr
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_GetExactAddrFromChipWithoutISC(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    AMTRDRV_TYPE_Record_T om_record;

    
    memset(&om_record, 0, sizeof(om_record));
    om_record.address.vid = addr_entry->vid;
    memcpy(om_record.address.mac, addr_entry->mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (FALSE == AMTRDRV_OM_GetExactRecord((UI8_T *)(&om_record)))
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        UI32_T unit,port,trunk_id;
        BOOL_T is_trunk;
        if(DEV_AMTRDRV_PMGR_ARLFirstChipLookUp(addr_entry->vid,
                                               addr_entry->mac,
                                               &unit, &port, &trunk_id, &is_trunk))
        {
            addr_entry->ifindex = (is_trunk)?
                            (trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER):/* arl_entry.trunk_id is 0 base.*/
                            (STKTPLG_POM_UPORT_TO_IFINDEX(AMTRDRV_OM_GetMyDrvUnitId(), port));
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#else
        return FALSE;
#endif
    }
    else
    {
        amtrdrv_memcpy(addr_entry, &(om_record.address),sizeof(AMTR_TYPE_AddrEntry_T));
        return TRUE;
    }
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ChangePortLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to update addresses life_time attribute by giving ifindex
 *          and life_time
 * INPUT  : ifindex    - port number
 *          life_time  - life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function is call by AMTR_MGR when the port_info is changed
 *          2.For this procedure we only need to update OM or local_aging list
 *            we don't need to care about the chip because the attribtue in chip
 *            side is always static and nothing changed.
 *          3.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_ChangePortLifeTime(UI32_T ifindex,UI32_T life_time)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    UI16_T                 unit_bmp=0;
#endif /* SYS_CPNT_STACKING */
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;

    SYSFUN_USE_CSC(FALSE);
    /* Send an event to wake up ASIC_COMMAND_TASK to process the order-- CHANGE_PORT_LIFE_TIME
     */
    /* Take the token to run
     */
    SYSFUN_TakeSem(amtrdrv_local_finished_smid, SYSFUN_TIMEOUT_WAIT_FOREVER);
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_CHANGE_PORT_LIFE_TIME;
    blocked_command_p->ifindex = ifindex;
    blocked_command_p->life_time = life_time;
    blocked_command_p->vlan_counter = 0;
    blocked_command_p->is_sync_op = FALSE;
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(FALSE);
        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_CHANGE_PORT_LIFE_TIME) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_CHANGE_PORT_LIFE_TIME;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.ifindex = ifindex;
            isc_buffer_p->data.entries.record[0].address.life_time = life_time;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, TRUE);

            if (return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_DeleteAddrByPort Failed to send ISC to slave.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }
#endif  /* SYS_CPNT_STACKING */
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_CreateMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to create a multicast address table for each device
 * INPUT  : UI32_T   vid     - vlan ID
 *          UI8_T    *mac    - multicast mac address
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This function is only called from AMTR_MGR.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_CreateMulticastAddr(UI32_T vid, UI8_T *mac)
{
    /* BODY
     */
    SYSFUN_USE_CSC(FALSE);
    /* Create multicast address entry on local machine
     */
    if(DEV_AMTRDRV_PMGR_CreateMulticastAddrTblEntry(vid, mac) == FALSE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
#if(SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* Master unit should tell slave units to create the same multicast entry */
        UI16_T             unit_bmp=0;

        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_CREATE_MULTICAST_ADDR) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id = AMTRDRV_MGR_CREATE_MULTICAST_ADDR;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            amtrdrv_memcpy(isc_buffer_p->data.entries.record[0].address.mac, mac, AMTR_TYPE_MAC_LEN);

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_CALLBYAGENT_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if(return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to create MulticastAddr");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }/* End of if (operation_mode != MASTER) */
#endif /*SYS_CPNT_STACKING*/
#endif /*SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DestroyMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to destroy a multicast address table for each device
 * INPUT  : UI32_T   vid     - vlan ID
 *          UI8_T    *mac    - multicast mac address
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DestroyMulticastAddr(UI32_T vid, UI8_T* mac)
{
    /* BODY
     */
    SYSFUN_USE_CSC(FALSE);
    if(DEV_AMTRDRV_PMGR_DestroyMulticastAddrTblEntry(vid, mac) == FALSE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
#if(SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI16_T             unit_bmp=0;

        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_DESTROY_MULTICAST_ADDR) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id   = AMTRDRV_MGR_DESTROY_MULTICAST_ADDR;
            isc_buffer_p->data.entries.number_of_entries = 1;
            isc_buffer_p->data.entries.record[0].address.vid = vid;
            amtrdrv_memcpy(isc_buffer_p->data.entries.record[0].address.mac, mac, AMTR_TYPE_MAC_LEN);

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_CALLBYAGENT_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if(return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to destroy MulticastAddr.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }/* End of if (operation_mode != MASTER) */
#endif /* End of SYS_CPNT_STACKING == TRUE */
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetMulticastPortMember
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set port members for a specified multicast group
 * INPUT  : UI32_T    unit    - Unit number
 *          UI32_T    vid     - VLAN ID
 *          UI8_T     *mac    - multicast MAC address
 *          UI8_T     *pbmp   - port member list
 *          UI8_T     *tbmp   - tagged port member list
 * OUTPUT: None
 * RETURN: TRUE / FALSE
 * NOTES : 1. per device dependent
 *         2. Caller doesn't need to pass "unit", this API will set all valid remote units via ISC.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetMulticastPortMember(UI32_T vid, UI8_T *mac, UI8_T *pbmp, UI8_T *tbmp)
{
    /* BODY
     */
    SYSFUN_USE_CSC(FALSE);
    /* Local Operation */
    if(FALSE == DEV_AMTRDRV_PMGR_SetMulticastPortList(vid, mac, pbmp, tbmp))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
#if(SYS_CPNT_MAINBOARD == TRUE)
#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI16_T             unit_bmp=0;

        unit_bmp = AMTRDRV_MGR_GetValidUnitBmp(TRUE);

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
            UI32_T                      pdu_len;
            UI16_T                      return_unit_bmp=0;


            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_SET_MULTICAST_PORT_MEMBER) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
            isc_buffer_p->service_id   = AMTRDRV_MGR_SET_MULTICAST_PORT_MEMBER;
            isc_buffer_p->data.mclist.vid  = vid;
            amtrdrv_memcpy(isc_buffer_p->data.mclist.mac, mac, AMTR_TYPE_MAC_LEN);
            amtrdrv_memcpy(isc_buffer_p->data.mclist.pbmp, pbmp, sizeof(isc_buffer_p->data.mclist.pbmp));
            amtrdrv_memcpy(isc_buffer_p->data.mclist.tbmp, tbmp, sizeof(isc_buffer_p->data.mclist.pbmp));

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_CALLBYAGENT_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);

            if(return_unit_bmp !=0)
            {
                AMTRDRV_MGR_DBGMSG("Failed to send ISC to slave to set Multicast port member.");
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }         }
    }
#endif /* SYS_CPNT_STACKING */
#endif /* SYS_CPNT_MAINBOARD */
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SyncToHisam
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get a record or event to sync to Hisam table
 * INPUT  : None
 * OUTPUT : *addr_entry           - user_record or event
 *          *action               - sync case : set_entry / delete_entry / delete_group .......
 * RETURN : TRUE(still has a record or event to sync) / FALSE (no record or evnet to sync)
 * NOTES  : This function is called by AMTR_task periodically to get record or event to sync.
 *          If the sync case is "set_entry" or "delete_entry" *addr_entry means a record or
 *          it contain evnet info
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SyncToHisam(AMTR_TYPE_AddrEntry_T *addr_entry,AMTR_TYPE_Command_T *action)
{
char* action_to_str_ary[AMTR_TYPE_COMMAND_MAX+1]=
{
    AMTR_TYPE_COMMAND_LIST(AMTR_TYPE_TO_STR)
};

    AMTRDRV_TYPE_Record_T    address_record;
    UI8_T                  local_action;

    SYSFUN_USE_CSC(FALSE);
    /* If there is no more event pending we will get the event from sync queue
     */
    if(amtrdrv_mgr_sync_event.event_flag == FALSE)
    {
        /* If there is any event in sync queue then return false to tell upper layer
         * to stop sync from Hash to Hisam
         */
        if(!AMTRDRV_OM_SyncQueueDequeue(&amtrdrv_mgr_sync_event.record_index,&amtrdrv_mgr_sync_event.event_id,(UI8_T *)&address_record,&local_action))
        {
            if(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo()&AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_TRACE_MSG)
            {
                BACKDOOR_MGR_Printf("SyncQ is empty.\r\n");
            }
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        if(amtrdrv_mgr_sync_event.record_index >= SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
        {
            /* This is an combination events we will store this event in local static variable
             */
            AMTRDRV_MGR_ID2Event(addr_entry,amtrdrv_mgr_sync_event.record_index,&amtrdrv_mgr_sync_event.event_id,action);

            if(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo() & AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_TRACE_MSG)
            {
                BACKDOOR_MGR_Printf("SynQ info(Event): record_index=%hu event_id=0x%08lX",
                    amtrdrv_mgr_sync_event.record_index,
                    (unsigned long)amtrdrv_mgr_sync_event.event_id);
                if(*action>=AMTR_TYPE_COMMAND_MAX)
                    BACKDOOR_MGR_Printf("action=Unknown(%d)\r\n", (int)(*action));
                else
                    BACKDOOR_MGR_Printf("action=%s\r\n", action_to_str_ary[*action]);
            }

            if(amtrdrv_mgr_sync_event.event_id != 0)
            {
                amtrdrv_mgr_sync_event.event_flag = TRUE;
            }
        }
        else
        {
            /* return the record information to upper layer */
            amtrdrv_memcpy(addr_entry,&address_record,sizeof(AMTR_TYPE_AddrEntry_T));
#if(SYS_CPNT_VXLAN == TRUE)
            if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(addr_entry->vid))
            {
                UI16_T r_vfi;

                if(AMTRDRV_OM_ConvertLvfiToRvfi(addr_entry->vid, &r_vfi)==FALSE)
                {
                    AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertLvfiToRvfi returns error");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }

                addr_entry->vid = r_vfi;
            }
#endif
            *action=local_action;
            if(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo() & AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_TRACE_MSG)
            {
                BACKDOOR_MGR_Printf("SynQ info(Record): record_index=%hu ", amtrdrv_mgr_sync_event.record_index);
                if(*action>=AMTR_TYPE_COMMAND_MAX)
                    BACKDOOR_MGR_Printf("action=Unknown(%d)\r\n", (int)(*action));
                else
                    BACKDOOR_MGR_Printf("action=%s\r\n", action_to_str_ary[*action]);
            }
        }
    }
    else/*(amtrdrv_mgr_sync_event.event_flag == TRUE)*/
    {
        AMTRDRV_MGR_ID2Event(addr_entry,amtrdrv_mgr_sync_event.record_index,&amtrdrv_mgr_sync_event.event_id,action);
        if(amtrdrv_mgr_sync_event.event_id == 0)
        {
            amtrdrv_mgr_sync_event.event_flag = FALSE;
        }

        if(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo() & AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_TRACE_MSG)
        {
            BACKDOOR_MGR_Printf("amtrdrv_mgr_sync_event.event_id=0x%08lX\r\n", (unsigned long)amtrdrv_mgr_sync_event.event_id);
        }

        if(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo() & AMTRDRV_OM_SYNCQ_DBG_FLAG_FORCE_CLEAR_EVENT)
        {
            if(amtrdrv_mgr_sync_event.event_id)
            {
                BACKDOOR_MGR_Printf("force amtrdrv_mgr_sync_event.event_id to be 0(0x%08lX\r\n", (unsigned long)amtrdrv_mgr_sync_event.event_id);
                amtrdrv_mgr_sync_event.event_id=0;
                amtrdrv_mgr_sync_event.event_flag=FALSE;
            }
        }

    }
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetCallBackMsg
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get a callback message for syscallbac task to notify other cscs.
 * INPUT  : None
 * OUTPUT : msg  -- include message type and entry information
 * RETURN : TRUE(still has a record or event to notify other cscs) / FALSE(no event)
 * NOTES  : This function is call by SYS_Callback task.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_GetCallBackMsg(SYS_TYPE_MSG_T *event)
{
    AMTRDRV_TYPE_Record_T            address_record;
    AMTRDRV_MGR_SysCallBackMsg_T   *callback_msg;
    UI8_T                          local_action;

    SYSFUN_USE_CSC(FALSE);
    /* If there is no more event pending we will get the event from sync queue
     */
    callback_msg =(AMTRDRV_MGR_SysCallBackMsg_T *) event->msg;

    /* In current design we don't support delete_group events to notify sys_callback task.
     * If you need to enable such features please modify to #if 0 */
#if 1
    if(!AMTRDRV_OM_CallBackQueueDequeue(&amtrdrv_mgr_callback_event.record_index,&amtrdrv_mgr_callback_event.event_id,(UI8_T *)&address_record,&local_action))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    /* we will copy the record information into the callback_msg buffer to return to sys_callback component*/
    amtrdrv_memcpy(callback_msg->mac,&address_record.address.mac,AMTR_TYPE_MAC_LEN);
    callback_msg->ifindex = address_record.address.ifindex;
    callback_msg->vid = address_record.address.vid;
    callback_msg->life_time = address_record.address.life_time;
    callback_msg->source = address_record.address.source;
    callback_msg->type = local_action;

#else
{
    AMTR_TYPE_AddrEntry_T          addr_entry;
    if(amtrdrv_mgr_callback_event.event_flag == FALSE)
    {
        if(!AMTRDRV_OM_CallBackQueueDequeue(&amtrdrv_mgr_callback_event.record_index,&amtrdrv_mgr_callback_event.event_id,(UI8_T *)&address_record,&local_action))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        if(amtrdrv_mgr_callback_event.record_index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
        {
            /* we will copy the record information into the callback_msg buffer to return to
             * sys_callback component
             */
            amtrdrv_memcpy(callback_msg->mac,&address_record.address.mac,AMTR_TYPE_MAC_LEN);
            callback_msg->ifindex = address_record.address.ifindex;
            callback_msg->vid = address_record.address.vid;
            callback_msg->life_time = address_record.address.life_time;
            callback_msg->source = address_record.address.source;
            callback_msg->type = local_action;
        }
        else
        {
            /* we will return an event to sys_callback component to provide delete group
             * event
             */
            amtrdrv_memset(&addr_entry,0,sizeof(AMTR_TYPE_AddrEntry_T));
            AMTRDRV_MGR_ID2Event(&addr_entry,amtrdrv_mgr_callback_event.record_index,&amtrdrv_mgr_callback_event.event_id,&callback_msg->type);
            amtrdrv_memcpy(callback_msg->mac,&addr_entry.mac,sizeof(AMTR_TYPE_MAC_LEN));
            callback_msg->ifindex = addr_entry.ifindex;
            callback_msg->vid = addr_entry.vid;
            callback_msg->life_time = addr_entry.life_time;
            callback_msg->source = addr_entry.source;
            if(amtrdrv_mgr_callback_event.event_id != 0)
            {
                amtrdrv_mgr_callback_event.event_flag = TRUE;
            }
        }
    }
    else
    {
        amtrdrv_memset(&addr_entry,0,sizeof(AMTR_TYPE_AddrEntry_T));
        AMTRDRV_MGR_ID2Event(&addr_entry,amtrdrv_mgr_callback_event.record_index,&amtrdrv_mgr_callback_event.event_id,&callback_msg->type);
        amtrdrv_memcpy(callback_msg->mac,&addr_entry.mac,sizeof(AMTR_TYPE_MAC_LEN));
        callback_msg->ifindex = addr_entry.ifindex;
        callback_msg->vid = addr_entry.vid;
        callback_msg->life_time = addr_entry.life_time;
        callback_msg->source = addr_entry.source;
        if(amtrdrv_mgr_callback_event.event_id == 0)
        {
            amtrdrv_mgr_callback_event.event_flag = FALSE;
        }
    }
}
#endif
    SYSFUN_RELEASE_CSC();
    return TRUE;
}
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_ClearCollisionVlanMacTable(void)
{

    SYSFUN_USE_CSC(FALSE);
    AMTRDRV_OM_ClearCollisionVlanMacTable();
    SYSFUN_RELEASE_CSC();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetNextEntryOfCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next entry from the collision vlan mac table.
 * INPUT    : idx_p - The entry next to the value of *idx_p will be output
 * OUTPUT   : idx_p - The index of the output entry
 *            vlan_id_p - The vlan id of the collision mac address
 *            mac       - the collision mac address
 *            count_p   - the count of hash collision occurence
 * RETURN   : TRUE  - An entry is output sucessfully
 *            FALSE - No more entry to output.
 * NOTE     : To get the first entry, set value of *idx_p as
 *            AMTRDRV_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_GetNextEntryOfCollisionVlanMacTable(UI8_T* idx_p,
    UI16_T* vlan_id_p, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T *count_p)
{
    BOOL_T retval;

    SYSFUN_USE_CSC(FALSE);
    retval = AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable(idx_p, vlan_id_p, mac, count_p);
    SYSFUN_RELEASE_CSC();

    return retval;
}
#endif

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetHashLookupDepthFromChip
 *------------------------------------------------------------------------------
 * PURPOSE  : Get hash lookup depth from chip
 * INPUT    : None
 * OUTPUT   : lookup_depth_p -- hash lookup depth
 * RETURN   : TRUE  - success
 *            FALSE - fail
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p)
{
    return DEV_AMTRDRV_PMGR_GetHashLookupDepth(lookup_depth_p);
}
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ReadAndClearLocalHitBit
 *------------------------------------------------------------------------------
 * PURPOSE  : Get and clear hit bit of specified {vid, mac}
 * INPUT    : mac_p - MAC address to check hit bit
 *            vid - vid to check hit bit
 *            ifindex - ifindex to check hit bit
 * OUTPUT   : hitbit_value_p -- hit or not
 * RETURN   : TRUE  - success
 *            FALSE - fail
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_ReadAndClearLocalHitBit(
    UI8_T *hitbit_value_p,
    UI8_T *mac_p,
    UI32_T vid,
    UI32_T ifindex)
{
    SWDRV_Trunk_Info_T trunk_member_info;
    UI32_T unit = 0;
    UI32_T checking_port_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK] = {0};
    UI32_T port_counter = 0, trunk_mem = 0;
    BOOL_T retval;

    if(IS_TRUNK(ifindex))
    {
        memset(&trunk_member_info, 0, sizeof(trunk_member_info));
        unit = STKTPLG_POM_IFINDEX_TO_UNIT(ifindex);
        SWDRV_GetTrunkInfo(ifindex, &trunk_member_info);

        for(trunk_mem=0; trunk_mem<trunk_member_info.member_number; trunk_mem++)
        {
            /* determine the mac is learnt on local unit or not
             */
            if(unit == trunk_member_info.member_list[trunk_mem].unit)
            {
                /* need to determine the mac is learnt on module unit or main board ???
                 * call STKTPLG_POM_IsModulePort
                 */
                checking_port_ar[port_counter] = trunk_member_info.member_list[trunk_mem].port;
                port_counter++;
            }
        }
    }
    else
    {
        unit = STKTPLG_POM_IFINDEX_TO_UNIT(ifindex);
        checking_port_ar[0] = STKTPLG_POM_IFINDEX_TO_PORT(ifindex);
        port_counter = 1;
    }

    retval = DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(hitbit_value_p, mac_p, vid, unit, checking_port_ar, port_counter);
    return retval;
}

/* Local SubRoutine ***************************************************************/

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ConvertGatewayReturnValue
 * -------------------------------------------------------------------------
 * PURPOSE: Convert gateway return value to AMTR_TYPE_Ret_T return value.
 * INPUT  : gw_retval           - the gateway return value
 * OUTPUT : amtr_type_ret_val   - the converted return value
 * RETURN : None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static void AMTRDRV_MGR_ConvertGatewayReturnValue(DEV_AMTRDRV_Ret_T gw_retval, AMTR_TYPE_Ret_T* amtr_type_ret_val)
{
    switch(gw_retval)
    {
        case DEV_AMTRDRV_SUCCESS:
            *amtr_type_ret_val = AMTR_TYPE_RET_SUCCESS;
            break;
        case DEV_AMTRDRV_FAIL:
            *amtr_type_ret_val = AMTR_TYPE_RET_ERROR_UNKNOWN;
            break;
        case DEV_AMTRDRV_COLLISION:
            *amtr_type_ret_val = AMTR_TYPE_RET_COLLISION;
            break;          
        default:
            printf("%s: Illegal retval: %u\r\n", __FUNCTION__, gw_retval);
            *amtr_type_ret_val = AMTR_TYPE_RET_ERROR_UNKNOWN;            
    }
}


#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_ADDRESS_TASK_Main
 * -------------------------------------------------------------------------
 * PURPOSE: This is the main function for AMTRDRV_ADDRESS_TASK.
 *          It has to process NA / AgingOut Cheecking /AgingOut delete
 *          periodically every 32 ticks
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 * -------------------------------------------------------------------------*/
static void AMTRDRV_ADDRESS_TASK_Main(void)
{
    UI32_T        pending_events,events;
#if (SYS_CPNT_MAINBOARD == TRUE)
    UI32_T        t1,t2,t3,t4,counter1 = 0,counter2 = 0,counter3 = 0;
#endif /* SYS_CPNT_MAINBOARD */
    void*         amtrdrv_address_task_tmid;
#if (AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
    UI32_T amtrdrv_address_task_ticks = amtr_update_addr_table_ticks;
#endif

    /* start periodic timer
     */
#if defined(AMTRDRV_SLAVE_REPEATING_TEST) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
     AMTRDRV_OM_InitNABufferRestore();
#endif

    amtrdrv_address_task_tmid = SYSFUN_PeriodicTimer_Create();

    SYSFUN_PeriodicTimer_Start(amtrdrv_address_task_tmid,amtr_update_addr_table_ticks, AMTRDRV_MGR_EVENT_TIMER);
    events = 0;

    while(TRUE)
    {
        /* wait event
         */
        SYSFUN_ReceiveEvent ((AMTRDRV_MGR_EVENT_TIMER |
                              AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE),
                              SYSFUN_EVENT_WAIT_ANY,
                              (events!=0)? SYSFUN_TIMEOUT_NOWAIT: SYSFUN_TIMEOUT_WAIT_FOREVER, /* timeout */
                              &pending_events);
        events |= pending_events;
        /* Transition event handler
         */
#if (AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_mgr_test_register_action == 1){
            LAN_Register_NA_N_SecurityCheck_Handler(0);
            amtrdrv_mgr_test_register_action = 3;
        }
        else if((amtrdrv_mgr_test_register_action == 2)){
            LAN_Register_NA_N_SecurityCheck_Handler(&AMTRDRV_MGR_AnnounceNA_N_SecurityCheck);
            amtrdrv_mgr_test_register_action = 3;
        }
#endif

        if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if(events & AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE)
            {
                //amtrdrv_mgr_is_address_transition_done = TRUE;
                AMTRDRV_OM_SetAddressTransitionDone();
            }
            events = 0;
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-26, 10:32:39 */
            SYSFUN_Sleep(SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS_FOR_ES4308MA);
#else
            SYSFUN_Sleep(SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS);
#endif /* ES3526MA_POE_7LF_LN */
            continue;
        }
#if (AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_address_task_ticks != amtr_update_addr_table_ticks){
            SYSFUN_PeriodicTimer_Restart (amtrdrv_address_task_tmid, amtr_update_addr_table_ticks);
            amtrdrv_address_task_ticks = amtr_update_addr_table_ticks;
        }
#endif
        /* Timer event handler */
        if(events & AMTRDRV_MGR_EVENT_TIMER)
        {
#if (SYS_CPNT_MAINBOARD == TRUE)
            if (amtrdrv_mgr_bd_address_task_performance == TRUE)
            {
                if(amtrdrv_mgr_number_of_address_task_testing_times != 0)
                {
                    t1 =SYSFUN_GetSysTick();
                    /* process NA */
                    t2 =SYSFUN_GetSysTick();
                    /* Process aging out checking */
                    counter2=AMTRDRV_MGR_ProcessCheckingAgingList();
                    t3 = SYSFUN_GetSysTick();

                    AMTRDRV_MGR_ProcessSlaveNABuffer();
                    /* process aging out delete */
                    t4 = SYSFUN_GetSysTick();
                    amtrdrv_mgr_number_of_address_task_testing_times --;
                    amtrdrv_mgr_testing_na_time = amtrdrv_mgr_testing_na_time + t2 - t1;
                    amtrdrv_mgr_testing_agingcheck_time = amtrdrv_mgr_testing_agingcheck_time + t3 - t2;
                    amtrdrv_mgr_testing_agingout_time = amtrdrv_mgr_testing_agingout_time + t4 - t3;
                    amtrdrv_mgr_testing_na_counter += counter1;
                    amtrdrv_mgr_testing_agingchecking_counter += counter2 ;
                    amtrdrv_mgr_testing_agingout_counter += counter3;
                }
                if(amtrdrv_mgr_number_of_address_task_testing_times == 0)
                {
                    BACKDOOR_MGR_Printf("The average spending time(tick) for process NA is : %ld\r\n",(long)amtrdrv_mgr_testing_na_time/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The average counter for process NA is : %ld\r\n",(long)amtrdrv_mgr_testing_na_counter/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The average spending time(tick) for process aging checking is : %ld\r\n",(long)amtrdrv_mgr_testing_agingcheck_time/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The average counter for process aging checking is : %ld\r\n",(long)amtrdrv_mgr_testing_agingchecking_counter/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The average spending time(tick) for process aging out is : %ld\r\n",(long)amtrdrv_mgr_testing_agingout_time/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The average counter for process aging out is : %ld\r\n",(long)amtrdrv_mgr_testing_agingout_counter/amtrdrv_mgr_address_task_testing_times);
                    BACKDOOR_MGR_Printf("The total average spending time for address task is : %ld\r\n",(long)(amtrdrv_mgr_testing_na_time + amtrdrv_mgr_testing_agingcheck_time + amtrdrv_mgr_testing_agingout_time)/amtrdrv_mgr_address_task_testing_times);
                    amtrdrv_mgr_bd_address_task_performance = FALSE;
                }
            }
            else
#endif /* SYS_CPNT_MAINBOARD */
            {
#if (SYS_CPNT_MAINBOARD == TRUE)
                AMTRDRV_MGR_ProcessCheckingAgingList();
                AMTRDRV_MGR_ProcessSlaveNABuffer();
#endif /* SYS_CPNT_MAINBOARD */
            }
            events ^= AMTRDRV_MGR_EVENT_TIMER;
//            SYSFUN_Sleep(4);
        }
    }
}

#if (AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
void AMTRDRV_MGR_ResendAgingOutEntry(void){
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p = NULL;
    UI32_T                      pdu_len;
    UI32_T                      index;
    UI16_T                      unit_bmp = 0;
    UI8_T                       master_unit_id;
    /*Get buf*/
    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
        L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
    if(!mref_handle_p)
        return ;
    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);
    if(!isc_buffer_p)
        goto RELEASE_MREF ;

    isc_buffer_p->data.entries.number_of_entries = 0;
    /*Get master ID*/
    if(FALSE == STKTPLG_POM_GetMasterUnitId(&master_unit_id))
        goto RELEASE_MREF ;

    unit_bmp |= ((0x01) << (master_unit_id-1));
    /*Get resending entry*/
    while((isc_buffer_p->data.entries.number_of_entries < AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
        &&(TRUE == AMTRDRV_OM_SlaveAgingOutResend(&index,(UI8_T *)&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries])))
        isc_buffer_p->data.entries.number_of_entries ++;
    /*no resending entry*/
    if(isc_buffer_p->data.entries.number_of_entries == 0)
        goto RELEASE_MREF ;

    /*resend entry */
    isc_buffer_p->service_id  = AMTRDRV_MGR_AGING_OUT;
    ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                          mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          AMTRDRV_MGR_ISC_TRY_FOREVER, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
     return ;
RELEASE_MREF:
     L_MM_Mref_Release(&mref_handle_p);
     return ;
}
#endif
#endif/*#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/

static void AMTRDRV_MGR_DeleteFromChip(AMTR_TYPE_BlockedCommand_T * blocked_command_p){

            UI32_T unit;
            UI32_T port;
            UI32_T trunk_id;
            UI32_T count;

            switch(blocked_command_p->blocked_command)
            {
                case AMTR_TYPE_COMMAND_DELETE_ALL:

                    break;

#if (SYS_CPNT_MAINBOARD == TRUE)
                case AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME:
                        if(blocked_command_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT){
                            DEV_AMTRDRV_PMGR_DeleteAllDynamicAddr();
                        }
                    break;

                case AMTR_TYPE_COMMAND_CHANGE_PORT_LIFE_TIME:

                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_SOURCE:
                    /* On original design, only Mac Entry with Source Learn
                     * belongs to dynamic Mac, and it can call SDK to remove
                     * all dynamic Mac on the specified source through
                     * ASIC register to speed up the operation.
                     * On new design, both Mac Entry with Source Learn and
                     * Security(with delete on timeout) belongs to dynamic
                     * Mac, wrong Mac Entry could be removed if calling SDK
                     * to remove all dynamic Mac on the mac table.
                     * So the operation shall be done by checking AMTRDRV OM.
                     */
                    #if 0
                    if(blocked_command_p->source == AMTR_TYPE_ADDRESS_SOURCE_LEARN){
                        DEV_AMTRDRV_PMGR_DeleteAllDynamicAddr();
                    }
                    #endif
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_PORT:
#if(SYS_CPNT_VXLAN == TRUE)
                    if(VXLAN_TYPE_IS_L_PORT(blocked_command_p->ifindex))
                    {
                        unit = 0;
                        VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(blocked_command_p->ifindex, port);
                        trunk_id = 0;
                    }
                    else
#endif
                    {
                        if(!IS_TRUNK(blocked_command_p->ifindex)){
                            unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                            port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                            trunk_id = 0;
                        }else{
                            unit = 0;
                            port = 0;
                            trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                        }
                    }
                    DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPort(unit,port,trunk_id);
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME:
                    if(blocked_command_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT){
#if(SYS_CPNT_VXLAN == TRUE)
                        if(VXLAN_TYPE_IS_L_PORT(blocked_command_p->ifindex))
                        {
                            unit = 0;
                            VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(blocked_command_p->ifindex, port);
                            trunk_id = 0;
                        }
                        else
#endif
                        {
                            if(!IS_TRUNK(blocked_command_p->ifindex)){
                                unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                                port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                                trunk_id = 0;
                            }else{
                                unit = 0;
                                port = 0;
                                trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                            }
                        }
                        DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPort(unit,port,trunk_id);
                    }
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE:
                        /* On original design, only Mac Entry with Source Learn
                         * belongs to dynamic Mac, and it can call SDK to remove
                         * all dynamic Mac on the specified port through ASIC
                         * register to speed up the operation.
                         * On new design, both Mac Entry with Source Learn and
                         * Security(with delete on timeout) belongs to dynamic
                         * Mac, wrong Mac Entry could be removed if calling SDK
                         * to remove all dynamic Mac on the specified port.
                         * So the operation shall be done by checking AMTRDRV OM.
                         */
                        #if 0
                        if(blocked_command_p->source == AMTR_TYPE_ADDRESS_SOURCE_LEARN){
                            if(!IS_TRUNK(blocked_command_p->ifindex)){
                                unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                                port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                                trunk_id = 0;
                            }else{
                                unit = 0;
                                port = 0;
                                trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                            }
                            DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPort(unit,port,trunk_id);
                        }
                        #endif
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME:
                {
                    UI32_T vlan=blocked_command_p->vid;
                    #if (SYS_CPNT_VXLAN == TRUE)
                    if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(vlan))
                    {
                        UI16_T r_vfi;

                        if(AMTRDRV_OM_ConvertLvfiToRvfi(vlan, &r_vfi)==FALSE)
                        {
                            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertLvfiToRvfi returns error");
                            break;
                        }

                        vlan=r_vfi;
                    }
                    #endif

                    if(blocked_command_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT){
                        DEV_AMTRDRV_PMGR_DeleteDynamicAddrByVlan(0,vlan);
                    }
                }
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE:
                    /* On original design, only Mac Entry with Source Learn
                     * belongs to dynamic Mac, and it can call SDK to remove
                     * all dynamic Mac on the specified vid and source through
                     * ASIC register to speed up the operation.
                     * On new design, both Mac Entry with Source Learn and
                     * Security(with delete on timeout) belongs to dynamic
                     * Mac, wrong Mac Entry could be removed if calling SDK
                     * to remove all dynamic Mac on the specified vid and source.
                     * So the operation shall be done by checking AMTRDRV OM.
                     */
                    #if 0
                    if(blocked_command_p->source == AMTR_TYPE_ADDRESS_SOURCE_LEARN){
                        DEV_AMTRDRV_PMGR_DeleteDynamicAddrByVlan(0,blocked_command_p->vid);
                    }
                    #endif

                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_VID:
                    {
                        UI32_T vlan=blocked_command_p->vid;

                        #if (SYS_CPNT_VXLAN == TRUE)
                        if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(vlan))
                        {
                            UI16_T r_vfi;

                            if(AMTRDRV_OM_ConvertLvfiToRvfi(vlan, &r_vfi)==FALSE)
                            {
                                AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertLvfiToRvfi returns error");
                                break;
                            }

                            vlan=r_vfi;
                        }
                        #endif
                        DEV_AMTRDRV_PMGR_DeleteDynamicAddrByVlan(0,vlan);
                    }

                    #if (SYS_CPNT_VXLAN == TRUE)
                    /* VxLan CSC will do delete by vfi when the vfi is destroyed
                     * the mapping between logical vfi and real vfi can be
                     * destroyed at this moment
                     */
                    if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(blocked_command_p->vid))
                    {
                        if(AMTRDRV_OM_RemoveLvfiMap(blocked_command_p->vid)==FALSE)
                            AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_RemoveLvfiMap returns error");
                    }
                    #endif

                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT:
#if(SYS_CPNT_VXLAN == TRUE)
                    if(VXLAN_TYPE_IS_L_PORT(blocked_command_p->ifindex))
                    {
                        unit = 0;
                        VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(blocked_command_p->ifindex, port);
                        trunk_id = 0;
                    }
                    else
#endif
                    {
                        if(!IS_TRUNK(blocked_command_p->ifindex)){
                            unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                            port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                            trunk_id = 0;
                        }else{
                            unit = 0;
                            port = 0;
                            trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                        }
                    }
                    DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPortAndVlan(unit,port,trunk_id,blocked_command_p->vid);

                    break;
	
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME:
                    if(blocked_command_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
                    {
#if(SYS_CPNT_VXLAN == TRUE)
                        if(VXLAN_TYPE_IS_L_PORT(blocked_command_p->ifindex))
                        {
                            unit = 0;
                            VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(blocked_command_p->ifindex, port);
                            trunk_id = 0;
                        }
                        else
#endif
                        {
                            if(!IS_TRUNK(blocked_command_p->ifindex))
                            {
                                 unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                                 port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                                 trunk_id = 0;
                            }
                            else
                            {
                                 unit = 0;
                                 port = 0;
                                 trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                            }
                        }
                         for(count = 0; count < blocked_command_p->vlan_counter; count++)
                         {
                            UI32_T vlan;

                            vlan=blocked_command_p->vlanlist[count];
                            #if (SYS_CPNT_VXLAN == TRUE)
                            if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(vlan))
                            {
                                UI16_T r_vfi;

                                if(AMTRDRV_OM_ConvertLvfiToRvfi(vlan, &r_vfi)==FALSE)
                                {
                                    AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertLvfiToRvfi returns error");
                                    break;
                                }
                                vlan=r_vfi;
                            }
                            #endif
                            DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPortAndVlan(unit,port,trunk_id,vlan);
                         }
                     }
                    break;
                    
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE:
                    /* On original design, only Mac Entry with Source Learn
                     * belongs to dynamic Mac, and it can call SDK to remove
                     * all dynamic Mac on the specified port and vlan through
                     * ASIC register to speed up the operation.
                     * On new design, both Mac Entry with Source Learn and
                     * Security(with delete on timeout) belongs to dynamic
                     * Mac, wrong Mac Entry could be removed if calling SDK
                     * to remove all dynamic Mac on the specified port and vlan.
                     * So the operation shall be done by checking AMTRDRV OM.
                     */
                    #if 0
                    if(blocked_command_p->source == AMTR_TYPE_ADDRESS_SOURCE_LEARN){
                        if(!IS_TRUNK(blocked_command_p->ifindex)){
                            unit = STKTPLG_POM_IFINDEX_TO_UNIT(blocked_command_p->ifindex);
                            port = STKTPLG_POM_IFINDEX_TO_PORT(blocked_command_p->ifindex);
                            trunk_id = 0;
                        }else{
                            unit = 0;
                            port = 0;
                            trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(blocked_command_p->ifindex);
                        }
                        DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPortAndVlan(unit,port,trunk_id,blocked_command_p->vid);
                    }
                    #endif
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR:
                    /* wakka: will add job queue in AMTRDRV_OM_DeleteVidExceptCertainAddrCallback(),
                     *        so do nothing here.
                     *        may exist performance issue.
                     */
                    break;
#endif /* SYS_CPNT_MAINBOARD */
               default:
                    break;
            }
}
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_ASIC_COMMAND_TASK_Main
 * -------------------------------------------------------------------------
 * PURPOSE: This is main function for ASIC_COMMAND_TASK.
 *          It has to program chip(set/delete)periodically every 32 ticks
 *          and it also need to handle the command from AMTRDRV_MGR to delete
 *          local addresses from chip & OM
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This task in module board only need to process transition event and address event
 * -------------------------------------------------------------------------*/
static void AMTRDRV_ASIC_COMMAND_TASK_Main(void)
{

    UI32_T                  pending_events,events;
#if (AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
    UI32_T amtrdrv_asic_commnand_task_ticks = amtr_update_addr_table_ticks;
#endif
#if (SYS_CPNT_MAINBOARD == TRUE)
    UI32_T                  t1,t2,counter1/*,counter2*/;

    void                    *amtrdrv_asic_command_task_tmid;
/*add by Tony.Lei*/
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE) && (AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
    static UI32_T  agingout_resend = 0;
#endif
    AMTR_TYPE_BlockedCommand_T temp_blocked_command;
    BOOL_T rc;

    /* reseved semaphore id for block command
     */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)    /*AMTR SW Learning*/
    /* Disable hardware learning and let software to control learning
     */
#ifndef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, this function is called by driver_proc, but amtr is initialized by l2_l4_proc, ???, 2009-06-15, 18:03:23 */
    if (FALSE == DEV_AMTRDRV_PMGR_DisableHardwareLearning())
    {
        BACKDOOR_MGR_Printf("%s: DEV_AMTRDRV_PMGR_DisableHardwareLearning return FALSE\n",__FUNCTION__);
    }
    /* Disable hardware aging out function and let software to control this function
     */
    if (FALSE==DEV_AMTRDRV_PMGR_SetAgeingTime(0))
    {
        BACKDOOR_MGR_Printf("%s: DEV_AMTRDRV_PMGR_SetAgeingTime return FALSE\n",__FUNCTION__);
    }
#endif /* ES3526MA_POE_7LF_LN */

#else
    AMTRDRV_OM_SetProvisionComplete(FALSE);
#endif

    /* start periodic timer
     */
    amtrdrv_asic_command_task_tmid = SYSFUN_PeriodicTimer_Create();
/*add by Tony.Lei */

#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
    /*add by Tony.Lei
     * to dynamicly modify the periodic timer
     */
    amtrdrv_asic_command_task_tmid_test = amtrdrv_asic_command_task_tmid;
#endif
    SYSFUN_PeriodicTimer_Start(amtrdrv_asic_command_task_tmid,amtr_update_addr_table_ticks, AMTRDRV_MGR_EVENT_TIMER);
#endif
    events = 0;
    while(TRUE)
    {
        /* wait event
         */
        SYSFUN_ReceiveEvent ((AMTRDRV_MGR_EVENT_TIMER |
                              AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                              SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                              AMTRDRV_MGR_EVENT_ADDRESS_OPERATION),
                              SYSFUN_EVENT_WAIT_ANY,
                              (events!=0)? SYSFUN_TIMEOUT_NOWAIT: SYSFUN_TIMEOUT_WAIT_FOREVER, /* timeout */
                              &pending_events);
        events |= pending_events;
        /*Transition event handler
         */
        if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE){

            if(events & AMTRDRV_MGR_EVENT_ENTER_TRANSITION_MODE)
                AMTRDRV_OM_SetAsicCommandTransitionDone();
            /*Init resource*/
            AMTRDRV_OM_SetBlockCommandNull();
            SYSFUN_GiveSem(amtrdrv_local_finished_smid);

            events =0;
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-26, 10:33:03 */
            SYSFUN_Sleep(SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS_FOR_ES4308MA);
#else
            SYSFUN_Sleep(SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS);
#endif /* ES3526MA_POE_7LF_LN */
            continue;

        }
#if (AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_asic_commnand_task_ticks != amtr_update_addr_table_ticks){
            SYSFUN_PeriodicTimer_Restart (amtrdrv_asic_command_task_tmid, amtr_update_addr_table_ticks);
            amtrdrv_asic_commnand_task_ticks = amtr_update_addr_table_ticks;
        }
#endif
        /* block command handler */
        if (events & AMTRDRV_MGR_EVENT_ADDRESS_OPERATION){

            AMTRDRV_OM_GetBlockCommand(&temp_blocked_command);
            AMTRDRV_OM_SetBlockCommandNull();

            switch(temp_blocked_command.blocked_command){

                case AMTR_TYPE_COMMAND_DELETE_ALL:
                    rc=AMTRDRV_MGR_LocalDeleteAllAddr(&temp_blocked_command);
                    break;
#if (SYS_CPNT_MAINBOARD == TRUE)
                case AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByLifeTime(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_SOURCE:
                    rc=AMTRDRV_MGR_LocalDeleteAddrBySource(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPort(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPortAndLifeTime(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPortAndSource(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByVID(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByVidAndLifeTime(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByVidAndSource(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByVIDnPort(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndLifeTime(&temp_blocked_command);
                    break;

                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndSource(&temp_blocked_command);
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR:
                    rc=AMTRDRV_MGR_LocalDeleteAddrByPortAndVidExceptCertainAddr(&temp_blocked_command);
                    break;
                 case AMTR_TYPE_COMMAND_CHANGE_PORT_LIFE_TIME:
                    rc=AMTRDRV_MGR_LocalChangePortLifeTime(&temp_blocked_command);
                    break;
#endif /* SYS_CPNT_MAINBOARD */
                 default:
                    rc=FALSE;
                    break;
            }
            /* EPR:ECS4210-52P-00278
             * Headline:PortSecurity: DUT learned dynamic security address,
             *          clear it while sending packets, DUT will not learn
             *          the address any more
             * Description:
             *   To ensure the operation to be enqueued into syncq is correct,
             *   need to perform deleting mac addresses from OM first.
             *   (So that the delete by xxx event will be enqueued to syncq
             *    first)
             *   And then remove the mac addresses on chip through the Chip
             *   register.
             *
             *   If remove mac addresses on the chip first, these removed mac
             *   addresses might be trapped to CPU and being set to OM and
             *   enqueue to syncq before delete by xxx event is enqueued to
             *   syncq. This incorrect order in syncq will result to some
             *   mac addresses are removed in AMTR_OM incorrectly.
             */
            AMTRDRV_MGR_DeleteFromChip(&temp_blocked_command);

            /* check whether the block command operation is synchronous
             */
            if (temp_blocked_command.is_sync_op==TRUE)
            {
                if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
                {
                    UI32_T rc;

                    rc=SYSFUN_GiveSem(amtrdrv_sync_op_smid);
                    if(rc!=SYSFUN_OK)
                    {
                        printf("%s(%d):Give sem amtrdrv_sync_op_smid error.(rc=%lu, amtrdrv_sync_op_smid=%lu)\r\n",
                            __FUNCTION__, __LINE__, (unsigned long)rc, (unsigned long)amtrdrv_sync_op_smid);
                    }
                }
                else
                    ISC_MCastReply(&(temp_blocked_command.isc_key), rc==TRUE?ISC_OP_ACK:ISC_OP_NAK);
            }
            else
            {
                UI32_T rc;

                rc=SYSFUN_GiveSem(amtrdrv_local_finished_smid);
                if(rc!=SYSFUN_OK)
                {
                    printf("%s(%d):Give sem amtrdrv_local_finished_smid error.(rc=%lu, amtrdrv_local_finished_smid=%lu)\r\n",
                        __FUNCTION__, __LINE__, (unsigned long)rc, (unsigned long)amtrdrv_local_finished_smid);
                }
            }

            events ^= AMTRDRV_MGR_EVENT_ADDRESS_OPERATION;
        }
#if (SYS_CPNT_MAINBOARD == TRUE)
        /* Timer event handler */
        if(events & AMTRDRV_MGR_EVENT_TIMER)
        {
            if(amtrdrv_mgr_bd_asic_task_performance)
            {
                if(amtrdrv_mgr_number_of_asic_task_testing_times !=0)
                {
                    /*counter2 = AMTRDRV_OM_GetJobQueueCounter();*/
                    /*BACKDOOR_MGR_Printf("\r\n There are still %ld entries left in jobqueue \r\n",counter2);*/
                    t1 = SYSFUN_GetSysTick();
                    counter1 = AMTRDRV_MGR_ProcessJobQueue();
                    t2 = SYSFUN_GetSysTick();
                    amtrdrv_mgr_testing_program_chip_time = amtrdrv_mgr_testing_program_chip_time + t2 - t1;
                    amtrdrv_mgr_testing_program_chip_counter += counter1;
                    amtrdrv_mgr_number_of_asic_task_testing_times --;
                }

                if(amtrdrv_mgr_number_of_asic_task_testing_times ==0)
                {
                    BACKDOOR_MGR_Printf("The average spending time(tick) for prgraming chip is : %ld\r\n",(long)amtrdrv_mgr_testing_program_chip_time/amtrdrv_mgr_asic_task_testing_times);
                    BACKDOOR_MGR_Printf("The average counter for prgraming chip is : %ld\r\n",(long)amtrdrv_mgr_testing_program_chip_counter/amtrdrv_mgr_asic_task_testing_times);
                    amtrdrv_mgr_bd_asic_task_performance = FALSE;
                }
            }
            else
            {
                /* When(Only) AMTR Hardware Learning:
                 * If job queue wasn't NULL, Task wouldn't synchronizes NA and Age out with chip.
                 * ERROR CASE:
                 * 1. port1 link down, delete dynamic by port1.
                 * 2. update OM, all deleted will be put in job queue. Caller leave critical section.
                 *    action of all port 1 entries are "del". If action == del, no one can get this entry from OM.
                 * 3. chip's port 1 entries aren't deleted yet. And they are still exist in OM with "action == del"
                 *    AMTRDRV Task will sync. these entries from chip. And can't get these entries form OM.
                 * 4. These entries will be looked as NA. And re-write to OM again.
                 * 5. These entries's action will be changed from "del" to "set".
                 * 6. Task process job queue, and set these entries to chip as static.
                 * 7. In final, some port 1 entries won't be deleted after del_dyn_by_port.
                 */
                if (AMTRDRV_MGR_ProcessJobQueue()!=AMTRDRV_MGR_MAX_NUM_JOB_PROCESS)
                {
                    AMTRDRV_MGR_SyncASIC2OM();

                    if (TRUE == amtrdrv_mgr_integration_testing_flag)
                    {
                        UI32_T temp = 0;
                        if ((amtrdrv_mgr_integration_testing_num_of_entry != 0)&&
                            (AMTRDRV_OM_GetTotalDynamicCounter() >=amtrdrv_mgr_integration_testing_num_of_entry))
                        {
                            UI32_T na_stop_tick;
                            na_stop_tick = SYSFUN_GetSysTick();
                            BACKDOOR_MGR_Printf("\r\n %ld entries is learnt.",(long)amtrdrv_mgr_integration_testing_num_of_entry);
                            BACKDOOR_MGR_Printf("\r\n Total spending time (ticks) = %ld ",(long)na_stop_tick-amtrdrv_mgr_integration_tesing_na_start_tick);
                            BACKDOOR_MGR_Printf("\r\n Average spending time (entry/tick) = %ld ",(long)amtrdrv_mgr_integration_testing_num_of_entry/(na_stop_tick-amtrdrv_mgr_integration_tesing_na_start_tick));
                            temp = amtrdrv_mgr_integration_testing_num_of_entry;
                            amtrdrv_mgr_integration_testing_num_of_entry = 0;
                        }

                        if ((amtrdrv_mgr_integration_testing_num_of_entry == 0)&&
                            (AMTRDRV_OM_GetTotalDynamicCounter() == 0))
                        {
                            UI32_T age_out_stop_tick;
                            age_out_stop_tick = SYSFUN_GetSysTick();
                            BACKDOOR_MGR_Printf("\r\n Entries is aged out.");
                            BACKDOOR_MGR_Printf("\r\n Total spending time (ticks)        = %ld ",(long)age_out_stop_tick-amtrdrv_mgr_integration_tesing_na_start_tick);
                            BACKDOOR_MGR_Printf("\r\n Average spending time (entry/tick) = %ld ",(long)temp/(age_out_stop_tick-amtrdrv_mgr_integration_tesing_na_start_tick));
                            amtrdrv_mgr_integration_testing_flag = FALSE;
                        }
                    }

                }
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE) && (AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
    /*in this thread ,the design has no  the concept of master and slave . this action is better located in the address_task . The reason of Locating  here is  burden.
          resend the aging out entry to master, the period is 5 -6s*/
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-26, 10:33:31 */
                if(!((++agingout_resend)%(5*100/SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS_FOR_ES4308MA + 1)))
#else
                if(!((++agingout_resend)%(5*100/SYS_BLD_AMTR_UPDATE_ADDR_TABLE_TICKS + 1)))
#endif /* ES3526MA_POE_7LF_LN */
                {
                    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE);
                    AMTRDRV_MGR_ResendAgingOutEntry();
                    agingout_resend = 0;
                }
#endif

            }
            events ^= AMTRDRV_MGR_EVENT_TIMER;
            /* sleep 4 ticks to aviod task which's priority is lower
             * than AMTRDRV_ASIC_COMMAND_TASK_Main starved.
             */
//            SYSFUN_Sleep(4);
        }
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_AMTRDRV);
            events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(TRUE) */
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessJobQueue
 *------------------------------------------------------------------------------
 * Purpose: This function is to get the record information and set to chip
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. action = add & in master mode => program the other chips except first
 *          2. action = add & in slave mode  => program all chips
 *          3. action = delelte & in master/slave unit => program all chips
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_MGR_ProcessJobQueue(void)
{
    AMTRDRV_TYPE_Record_T             address_record;
    UI32_T                          process_number   = 0;
    UI32_T                          event = 0;
    UI32_T                          index;
    UI8_T                           action;
#if (SYS_CPNT_STACKING == TRUE)
    UI16_T                          unit_bmp=0;
    BOOL_T                          is_option_module_exist=FALSE;
    UI32_T                          option_module_id       = 0;
    L_MM_Mref_Handle_T*             mref_handle_p;
    AMTRDRV_MGR_ISCSingleBuffer_T*  isc_buffer_p;
    UI32_T                          pdu_len;
    UI16_T                          return_unit_bmp=0;
    UI8_T                           state;
    /* BODY
     */
    is_option_module_exist = STKTPLG_POM_OptionModuleIsExist(AMTRDRV_OM_GetMyDrvUnitId(),&option_module_id);
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    while(process_number <AMTRDRV_MGR_MAX_NUM_JOB_PROCESS)
    {
        /* When the ASIC_COMMAND_TASK received the block command we need to
         * terminate to program chip this time since we need to finish
         * block command asap.
         */
        if(!AMTRDRV_OM_IsBlockCommandNull())
        {
            return process_number ;
        }
        /* if there is no more record need to set up to chip then break
         */
        if(!AMTRDRV_OM_DequeueJobList((UI8_T *)&address_record, &action,&index))
        {
            break;
        }
        process_number++;

        state = AMTRDRV_OM_GetReseverdState(index);
        /*When port move,make the flag AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS on reserverd state,
         *So now just clear the flag
         */
        if(AMTR_ISSET_DONOTDELETEFROMCHIP_FLAG(state)&&(action == AMTRDRV_MGR_ACTION_DEL)){
            event = L_HASH_DEL_SUCCESS_EV;
            AMTRDRV_OM_ClearReseverdState(index,AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS);

        }
        else
        {
            /* EPR ID: ASF4512MP-FLF-EC-00292
             *         After AMTRDRV_MGR_SetChip() is called, attribute between
             *         CHIP and OM must be the same.
             *         Clear AMTR_TYPE_OM_CHIP_NOT_ATTRIBUTE_NOT_SYNC_FLAGS here.
             *         charlie_chen 2011/06/22
             */
            AMTRDRV_OM_ClearReseverdState(index, AMTR_TYPE_OM_CHIP_ATTRIBUTE_NOT_SYNC_FLAGS);
            event = AMTRDRV_MGR_SetChip(&address_record,action,state);
        }

        /* Sending the result of programming chip to trigger HASH FSM
         * The FALSE case is only when the mac is replaced by attribute = other
         * we have to delete the record from hash directly. It means at that moment
         * we can't locate the reocrd and get the return value= FALSE
         * But this case is normal and legal.
         */

        AMTRDRV_OM_SetOperationResultEvent((UI8_T *)&address_record,event);
        /* Send ISC to program module's chips */
/*add by Tony.Lei
 * these are performance debug code
 */
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_mgr_test_performance == 4){
            if(!amtrdrv_mgr_test_master_counter_asic)
                amtrdrv_mgr_test_start_tick_asic = SYSFUN_GetSysTick();
            amtrdrv_mgr_test_master_counter_asic++;
            if(amtrdrv_mgr_test_master_counter_asic ==amtrdrv_mgr_test_master_totol_counter_asic){
                BACKDOOR_MGR_Printf("the tick %ld,the end%ld,counter %ld ok%ld\n",(long)amtrdrv_mgr_test_start_tick_asic,(long)SYSFUN_GetSysTick(),(long)amtrdrv_mgr_test_master_counter_asic,(long)amtrdrv_mgr_test_master_counter_asic_ok);
                amtrdrv_mgr_test_master_counter_asic = amtrdrv_mgr_test_master_totol_counter_asic =0;
            }

         }

        if(amtrdrv_mgr_test_performance == 5){
            if(event == L_HASH_SET_SUCCESS_EV){
                if(!amtrdrv_mgr_test_master_counter_asic_ok)
                    amtrdrv_mgr_test_start_tick_asic = SYSFUN_GetSysTick();
                amtrdrv_mgr_test_master_counter_asic_ok++;
                if(amtrdrv_mgr_test_master_counter_asic_ok ==amtrdrv_mgr_test_master_totol_counter_asic){
                    BACKDOOR_MGR_Printf("the tick %ld,the end%ld,counter %ld ok%ld\n",(long)amtrdrv_mgr_test_start_tick_asic,(long)SYSFUN_GetSysTick(),(long)amtrdrv_mgr_test_master_counter_asic,(long)amtrdrv_mgr_test_master_counter_asic_ok);
                    amtrdrv_mgr_test_master_counter_asic_ok = amtrdrv_mgr_test_master_totol_counter_asic =0;
                }
            }
        }
#endif
#if (SYS_CPNT_STACKING == TRUE)
        if(is_option_module_exist)
        {
            unit_bmp |= (0x01 << (option_module_id-1));
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCSingleBuffer_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_SET_ADDR_TO_LOCAL_MODULE) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                return process_number;
            }
            amtrdrv_memcpy(&isc_buffer_p->record,&address_record,sizeof(AMTRDRV_TYPE_Record_T));
            isc_buffer_p->record.address.action = action;
            isc_buffer_p->service_id = AMTRDRV_MGR_SET_ADDR_TO_LOCAL_MODULE;
            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_CALLBYAGENT_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_FOREVER, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

        if(AMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            return process_number;
        }
    } /* end of while */
    return process_number;
} /* end of AMTRDRV_MGR_ProcessJobQueue */

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_SyncASIC2OM
 *------------------------------------------------------------------------------
 * Purpose  : This function will synchronize entries from ASIC ARL Table to OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static void   AMTRDRV_MGR_SyncASIC2OM(void)
{
    UI32_T entry_count;
    UI32_T processed_number;
    UI32_T arl_entry_ifindex;
    AMTRDRV_TYPE_Record_T get_entry;
    SWDRV_TYPE_L2_Address_Info_T arl_entry={0};
    UI32_T ret;

    //kh_shi SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();

    if(AMTRDRV_OM_GetProvisionComplete() == FALSE)
    {
        SYSFUN_RELEASE_CSC();
        return;
    }

    if (AMTRDRV_OM_GetSynchronizing() == AMTRDRV_OM_SYNC_CEASING)
    {
        /* Mark all address in hisam table aged and reset hardware table pointer
         */
        AMTRDRV_OM_MarkAllDynamicRecord();
        DEV_AMTRDRV_PMGR_ResetToFirstAddrTblEntry(AMTRDRV_MGR_ALL_DEVICE_ID);
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_SYNCHRONIZING);
        amtrdrv_memset(&arl_entry,0,sizeof(SWDRV_TYPE_L2_Address_Info_T));
    }

    if (AMTRDRV_OM_GetSynchronizing() == AMTRDRV_OM_SYNC_SYNCHRONIZING)
    {
        entry_count = 0;

        for(processed_number=0; processed_number < SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN; processed_number++)
        {

            if(!AMTRDRV_OM_IsBlockCommandNull())
            {
                SYSFUN_RELEASE_CSC();
                return ;
            }

            ret = DEV_AMTRDRV_PMGR_GetNextAddrTblEntry(&arl_entry);

            if (ret == DEV_AMTRDRV_ADDRESS_ACCESS_SUCCESS)
            {
                /* copy ARL entry to address entry
                 */
                amtrdrv_memcpy(get_entry.address.mac,arl_entry.mac, AMTR_TYPE_MAC_LEN);

#if(SYS_CPNT_VXLAN == TRUE)
                if(AMTR_TYPE_IS_REAL_VFI(arl_entry.vid))
                {
                    UI16_T l_vfi;

                    if(AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(arl_entry.vid, &l_vfi)==TRUE)
                    {
                        arl_entry.vid=l_vfi;
                    }
                    else
                    {
                        AMTRDRV_MGR_DBGMSG("Failed to covert r_vfi to l_vfi");
                        continue;
                    }
                }
#endif
                get_entry.address.vid = arl_entry.vid;

            }
            else if (ret == DEV_AMTRDRV_ADDRESS_ACCESS_END_OF_TABLE)
            {
                AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_ANNOUNCING);
                break;
            }
            else /* DEV_AMTRDRV_ADDRESS_ACCESS_FAIL/DEV_AMTRDRV_ADDRESS_ACCESS_BREAK */
            {
                /* if error occurs or receive ACCESS_BREAK,
                 * stop to get entries.
                 */
                break;
            }
#if(SYS_CPNT_VXLAN == TRUE)
            if (arl_entry.vxlan_port > 0)
            {
                if(VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(arl_entry.vxlan_port, arl_entry_ifindex)==0)
                {
                    AMTRDRV_MGR_DBGMSG("Failed to covert real vxlan port in chip to logical vxlan port in om");
                    continue;
                }
                else
                {
                    AMTRDRV_MGR_DBGMSG("Covert real vxlan port in chip to logical vxlan port in om successfully");
                }
            }
            else
#endif
            {
                arl_entry_ifindex = (arl_entry.is_trunk)?
                                    (arl_entry.trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER):/* arl_entry.trunk_id is 0 base.*/
                                    (STKTPLG_POM_UPORT_TO_IFINDEX(AMTRDRV_OM_GetMyDrvUnitId(), arl_entry.port));
            }
            if ((AMTRDRV_OM_GetExactRecord((UI8_T *) &get_entry) == TRUE)&&(get_entry.address.ifindex == arl_entry_ifindex))
            {
                /* This new address is already exist in OM, clear mark
                 */
                AMTRDRV_OM_ClearMark((UI8_T *) &get_entry);
                continue;
            }
            else
            {
                /* port move or new address
                 */
                amtrdrv_memset(&addr_buf[entry_count], 0, sizeof(*addr_buf));
                amtrdrv_memcpy(addr_buf[entry_count].mac ,arl_entry.mac, AMTR_TYPE_MAC_LEN);
                addr_buf[entry_count].vid = arl_entry.vid;
                addr_buf[entry_count].ifindex = arl_entry_ifindex;
                /* don't need to clear mark.
                 * "mark field" has default value "FALSE" when setting to OM.
                 */
                entry_count++;
            }

        }/* End of for()*/

        if (entry_count > 0)
        {
            /* Notify new address to core layer
             */
            AMTRDRV_MGR_Notify_NewAddress(entry_count, addr_buf);
        }
    } /* AMTRDRV_OM_SYNC_SYNCHRONIZING */

    if (AMTRDRV_OM_GetSynchronizing() == AMTRDRV_OM_SYNC_ANNOUNCING)
    {
        UI32_T total_entry_announced;
        BOOL_T ret_get_entry;

        total_entry_announced = 0;
        entry_count = 0;

        while (total_entry_announced < AMTRDRV_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN)
        {
            ret_get_entry = AMTRDRV_OM_GetMarkedEntry((UI8_T *)&get_entry);

            /* store aged entry to buffer
             */
            if (ret_get_entry)
            {
                amtrdrv_memcpy(&addr_buf[entry_count],&(get_entry.address),sizeof(AMTR_TYPE_AddrEntry_T));
                entry_count++;
            }

            /* announce entries from buffer
             */
            if (entry_count > 0)
            {
                if (entry_count == SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN ||
                    !ret_get_entry)
                {
                    /* Notify age out entries to core layer
                     */
                    AMTRDRV_MGR_Notify_AgingOut(entry_count, addr_buf);
                    total_entry_announced += entry_count;
                    entry_count = 0;
                }
            }

            /* all entries announced
             */
            if (!ret_get_entry)
            {
                AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
                break;
            }
        }
    } /* AMTRDRV_OM_SYNC_ANNOUNCING */

    SYSFUN_RELEASE_CSC();
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Notify_AgingOut
 *------------------------------------------------------------------------------
 * PURPOSE: Notify AMTR to delete this entry who needs to be aged out
 * INPUT  : UI32_T                num_of_entries  -- how many records in the buffer
 *          AMTR_TYPE_AddrEntry_T *addr_entry     -- The aged addresses
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void AMTRDRV_MGR_Notify_AgingOut(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[])
{
    if(AMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (FALSE == SYS_CALLBACK_MGR_AgeOutMacAddress(SYS_MODULE_AMTRDRV,num_of_entries,(UI8_T*)addr_buf,num_of_entries*sizeof(AMTR_TYPE_AddrEntry_T)))
    {
        BACKDOOR_MGR_Printf("%s: SYS_CALLBACK_MGR_AgeOutMacAddress return false\n",__FUNCTION__) ;
    }
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Notify_NewAddress
 *------------------------------------------------------------------------------
 * PURPOSE: Notify AMTR_MGR for processing New Address
 * INPUT  : UI32_T                num_of_entries  -- how many records in the buffer
 *          AMTR_TYPE_AddrEntry_T *addr_entry     -- addresses buffer
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void AMTRDRV_MGR_Notify_NewAddress(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[])
{

    if(AMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (FALSE == SYS_CALLBACK_MGR_AnnounceNewMacAddress(SYS_MODULE_AMTRDRV,num_of_entries,(UI8_T*)addr_buf,num_of_entries*sizeof(AMTR_TYPE_AddrEntry_T)))
    {
        BACKDOOR_MGR_Printf("%s: SYS_CALLBACK_MGR_AnnounceNewMacAddress return false\n",__FUNCTION__) ;
    }
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddr2OMWithoutChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function only set entry to OM.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 *          UI32_T num_of_entries               - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will update Hash Table, and Callback Queue.
 *          2. This function should be used in HW Learning.
 *          3. This function doesn't notify remote unit.
 *          4. This function won't put entry in Job Queue, therefore AMTR won't
 *             program it to ASIC ARL Table.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddr2OMWithoutChip(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T address_record;
    UI32_T              i;
    UI32_T              record_index;
    BOOL_T              result = TRUE;

    SYSFUN_USE_CSC(FALSE);
    for (i=0;i<num_of_entries;i++)
    {
        if(CHECK_MAC_IS_NULL(addr_buf[i].mac)) /*added by Jinhua Wei,to remove warning*/
        {
            continue;
        }
        amtrdrv_memcpy(&address_record.address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        address_record.mark=FALSE;
#endif
        if ((result = AMTRDRV_OM_SetAddrHashOnly((UI8_T *)&address_record ,&record_index))==TRUE)
        {
            if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
            {
                AMTRDRV_OM_SyncQueueEnqueue(record_index);
            }
        }

    }
    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrFromOMWithoutChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function only delete entry from OM.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 *          UI32_T num_of_entries               - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will update Hash Table, Sync Queue(only learnt entry) and Callback Queue.
 *          2. This function should be used in HW Learning.
 *          3. This function doesn't notify remote unit.
 *          4. This function won't put entry in Job Queue, therefore AMTR won't
 *             program it to ASIC ARL Table.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrFromOMWithoutChip(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[])
{
    BOOL_T                  result = TRUE;

#if (SYS_CPNT_MAINBOARD == TRUE)
    AMTRDRV_TYPE_Record_T     address_record;
    UI32_T                  i;
    UI32_T                  index;

    SYSFUN_USE_CSC(FALSE);

    for(i=0;i<num_of_entries;i++)
    {
        /* if the mac = null mac it means this is an invalid address and we don't need to process it
         */
        if(CHECK_MAC_IS_NULL(addr_buf[i].mac)) /*added by Jinhua Wei,to remove warning*/
        {
            continue;
        }
        amtrdrv_memcpy(&address_record.address,&addr_buf[i],sizeof(AMTR_TYPE_AddrEntry_T));
#if(SYS_CPNT_VXLAN == TRUE)
        /* If VID is Dot1q VLAN ID or logical VFI, don't convert.
         * If VID is real VFI, convert to logical VFI.
         */
        if(AMTR_TYPE_IS_REAL_VFI(addr_buf[i].vid))
        {
            UI16_T l_vfi;

            if(AMTRDRV_OM_ConvertRvfiToLvfi(addr_buf[i].vid, &l_vfi)==TRUE)
            {
                address_record.address.vid=l_vfi;
            }
            else
                continue;
        }
#endif        
        /* update local OM
         * Assume update OM should always return success so we didn't do any error handle
         */
        if(!AMTRDRV_OM_DeleteAddrFromHashAndWaitSync((UI8_T *)&address_record, &index))
        {
            /* If we can't update record in OM as delete action we need to set this record = NULL
             * in case the remote unit does't have this record but master does
             */
            amtrdrv_memset(&addr_buf[i],0,sizeof(AMTR_TYPE_AddrEntry_T));
            result = FALSE ;
            continue;
        }

        if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
        {
            AMTRDRV_OM_SyncQueueEnqueue(index);

            /* Only delete record event need to put in callback queue to let sys callback task can
             * notify AMTRL3 there is a record deleted and it needs to update its database too
             */
            AMTRDRV_MGR_CallBackQueueEnqueue(index);

        }
    }

    SYSFUN_RELEASE_CSC();
#endif /* SYS_CPNT_MAINBOARD */
    return result;
}

#endif /* #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteChipOnly
 *------------------------------------------------------------------------------
 * PURPOSE: This function only delete entry from ASIC ARL Table.
 * INPUT  : UI16_T vid                       - vlan id
 *          UI8_T mac[SYS_ADPT_MAC_ADDR_LEN] - mac
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function only update ASIC ARL Table
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteChipOnly(UI16_T vid, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN])
{
    SYSFUN_USE_CSC(FALSE);
    if(FALSE==DEV_AMTRDRV_PMGR_DeleteAddr(vid,mac))
    {
        AMTRDRV_MGR_DBGMSG("Address delete from chip failed");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_ProcessCheckingAgingList
 *------------------------------------------------------------------------------
 * Purpose  : This function will get the address_entry from aging list and
 *            acheck the entry needs to be age out or not
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : UI32_T process_number -- check how many entry of aging list in this time.
 * NOTE     : 1. if trunk_port => polling master hitbit first. If local hitbit then
 *               polling all slave units. If all of them = 0 => delete the entry or
 *               update aging list with new timestamp.
 *            2. if not trunk_port => polling local hitbit. If hitbit !=0 => update
 *               aging list or delete the entry.
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_MGR_ProcessCheckingAgingList(void)
{
    UI32_T                      process_number = 0;
    AMTRDRV_TYPE_Record_T         address_record;
    SWDRV_Trunk_Info_T          trunk_port_info;
    UI32_T                      i,j=0;
    UI32_T                      checking_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI32_T                      record_index;
    BOOL_T                      is_needed_to_poll_module_board = FALSE;
    UI8_T                       hitbit_value = 0;
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p = NULL;
    UI32_T                      pdu_len;
    UI32_T                      option_module_id;
    UI16_T                      unit_bmp = 0;
    UI8_T                       master_unit_id;
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    /* When aging out function disable we don't need to check aging list and return TRUE directly
     */
    if(AMTRDRV_OM_GetOperAgingTime() == 0)
    {
        return 0;
    }

#if (SYS_CPNT_STACKING == TRUE)
    STKTPLG_POM_OptionModuleIsExist(AMTRDRV_OM_GetMyDrvUnitId(),&option_module_id);
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    STKTPLG_POM_GetMasterUnitId(&master_unit_id);
    unit_bmp |= ((0x01) << (master_unit_id-1));
    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                  L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_buffer_p==NULL)
    {
        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
        return process_number;
    }
    isc_buffer_p->data.entries.number_of_entries =0;
    }
#endif /* SYS_CPNT_STACKING */

    while(process_number < AMTRDRV_MGR_MAX_NUM_AGING_PROCESS)
    {
        if(!AMTRDRV_OM_LocalAgingListDequeue(AMTRDRV_OM_GetOperAgingTime(),&record_index,(UI8_T *)&address_record))
        {
            break;
        }

        process_number++;
        hitbit_value = 0;

        /* 1. Get the local entry's hitbit value if the aging time is reached */
        if(IS_TRUNK(address_record.address.ifindex))   /* the checking port is trunk port */
        {
            UI8_T   temp_hitbit_value;

            SWDRV_GetTrunkInfo(address_record.address.ifindex, &trunk_port_info);
            /*j means checking_port[] index and how many port need to be checked.
             */
            j=0;

            for(i=0; i<trunk_port_info.member_number; i++)
            {
                /* determine the mac is learnt on local unit or not */
                if(trunk_port_info.member_list[i].unit == AMTRDRV_OM_GetMyDrvUnitId())
                {
                    /* determine the mac is learnt on module unit or main board */
                    if(STKTPLG_POM_IsModulePort(trunk_port_info.member_list[i].unit,trunk_port_info.member_list[i].port))
                    {
                        is_needed_to_poll_module_board = TRUE;
                    }
                    else
                    {
                        checking_port[j] = trunk_port_info.member_list[i].port;
                        j++;
                    }
                }
            }

  #if (SYS_CPNT_STACKING == TRUE)
            if(is_needed_to_poll_module_board == TRUE)
            {
                L_MM_Mref_Handle_T*         mref_handle_p_module;
                AMTRDRV_MGR_ISCSingleBuffer_T*    isc_buffer_p_module;
                UI32_T                      pdu_len_module;
                mref_handle_p_module = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCSingleBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE) /* user_id */);
                isc_buffer_p_module = L_MM_Mref_GetPdu (mref_handle_p_module, &pdu_len_module);

                if (isc_buffer_p_module==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }

                isc_buffer_p_module->service_id = AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE;
                amtrdrv_memcpy(&isc_buffer_p_module->record,&address_record,sizeof(AMTRDRV_TYPE_Record_T));
                /*try forever
                 */
                ISC_RemoteCall ((UI8_T)option_module_id, ISC_AMTRDRV_CALLBYAGENT_SID,
                         mref_handle_p_module, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                         sizeof(temp_hitbit_value), &temp_hitbit_value,
                         AMTRDRV_MGR_ISC_TRY_FOREVER,AMTRDRV_MGR_ISC_TIMEOUT);
                if(temp_hitbit_value == 1)
                {
                    hitbit_value =1;
                }
            }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

            if (j > 0)
            {
                if(TRUE == DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(&temp_hitbit_value,address_record.address.mac,address_record.address.vid,AMTRDRV_OM_GetMyDrvUnitId(),checking_port,j))
                {
                    if(temp_hitbit_value == 1)
                    {
                        hitbit_value =1;
                    }
                }
            }
        }
        else  /* the checking port is normal port */
        {
#if (SYS_CPNT_STACKING == TRUE)
            checking_port[0] = STKTPLG_POM_IFINDEX_TO_PORT(address_record.address.ifindex);
            /* If the port is in the module we need to send ISC to poll the remote chip */
            option_module_id = STKTPLG_POM_IFINDEX_TO_DRIVERUNIT(address_record.address.ifindex);
            if(option_module_id > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
            {
                L_MM_Mref_Handle_T*         mref_handle_p_module;
                AMTRDRV_MGR_ISCSingleBuffer_T*    isc_buffer_p_module;
                UI32_T                      pdu_len_module;
                mref_handle_p_module = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCSingleBuffer_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE) /* user_id */);
                isc_buffer_p_module = L_MM_Mref_GetPdu (mref_handle_p_module, &pdu_len_module);

                if (isc_buffer_p_module==NULL)
                {
                    AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                    SYSFUN_RELEASE_CSC();
                    return FALSE;
                }
                isc_buffer_p_module->service_id = AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE;
                amtrdrv_memcpy(&isc_buffer_p_module->record,&address_record,sizeof(AMTRDRV_TYPE_Record_T));
                /*try forever
                 */
                ISC_RemoteCall ((UI8_T)option_module_id, ISC_AMTRDRV_CALLBYAGENT_SID,
                         mref_handle_p_module, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                         sizeof(hitbit_value), &hitbit_value,
                         AMTRDRV_MGR_ISC_TRY_FOREVER,AMTRDRV_MGR_ISC_TIMEOUT);
            }
            else /* polling local chip */
            {

                if(TRUE != DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(&hitbit_value,address_record.address.mac,address_record.address.vid,AMTRDRV_OM_GetMyDrvUnitId(),checking_port,1))
                    hitbit_value = 0;
            }
#else
            /* when the checking_port is a normal interface . the  trunk_member_count_in_localunit must be 1
                        */
            if (TRUE != DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(&hitbit_value,address_record.address.mac,address_record.address.vid,AMTRDRV_OM_GetMyDrvUnitId(),checking_port,1))
                hitbit_value = 0;
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */
        }

        /* 2. if the hitbit_value = 1 then keep this record back to the end of aging list */
        if(hitbit_value == 1)
        {
            /* no matter the address is learnt on trunk port or normal port
             * we need to put the node back to the end of local aging check list
             */
            AMTRDRV_OM_LocalAgingListEnqueue(record_index);
            if(IS_TRUNK(address_record.address.ifindex))
            {
                /* Only if the hit_bit_value which is got from chip this time != previous_hit_bit_value
                 * we have to check the trunk_hit_bit_value_for_each_unit = 0 or not to decide we
                 * need to put this record in aging out buffer or not in master mode.
                 * In slave mode we need to send out ISC packet to info master unit to change
                 * trunk hit bit value for that unit*/
                if(address_record.hit_bit_value_on_local_unit != hitbit_value)
                {
                    address_record.hit_bit_value_on_local_unit = hitbit_value;
                    AMTRDRV_OM_UpdateLocalRecordHitBitValue(record_index,hitbit_value);
                    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
                    {
                        address_record.trunk_hit_bit_value_for_each_unit |= ((0x1) << (AMTRDRV_OM_GetMyDrvUnitId()-1));
                        AMTRDRV_OM_UpdateSystemTrunkHitBit(record_index,address_record.trunk_hit_bit_value_for_each_unit);
                    }
#if (SYS_CPNT_STACKING == TRUE)
                    else if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
                    {
                        amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&address_record,sizeof(AMTRDRV_TYPE_Record_T));
                        isc_buffer_p->data.entries.number_of_entries ++;
                        /*add by tony.lei*/
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
                        /*enqueue before sending to master*/
                        AMTRDRV_OM_SlaveAgingOutEnqueue(record_index);
#endif
                        if(isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
                        {
                            isc_buffer_p->service_id  = AMTRDRV_MGR_AGING_OUT;
                            /* try forever
                             */
                            ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                             mref_handle_p,
                                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                             AMTRDRV_MGR_ISC_TRY_FOREVER, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
                            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                                      L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
                            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                            if (isc_buffer_p==NULL)
                            {
                                AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                                return process_number;
                            }
                            isc_buffer_p->data.entries.number_of_entries = 0;
                        }
                    }
                    else
                    {
                        return process_number;
                    }
#endif /* SYS_CPNT_STACKING */
                }
            }
        }
        else
        {
            if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
            {
                if(IS_TRUNK(address_record.address.ifindex))
                {
                    if(address_record.hit_bit_value_on_local_unit != hitbit_value)
                    {
                        if (amtrdrv_mgr_bd_display)
                        {
                            BACKDOOR_MGR_Printf("\r\n--Master:  AMTRDRV_MGR_ProcessCheckingAgingList( ), update hit bit value--");
                            BACKDOOR_MGR_Printf("\r\n[%02x-%02x-%02x-%02x-%02x-%02x] hit bit value: [%d];",
                            address_record.address.mac[0], address_record.address.mac[1],
                            address_record.address.mac[2], address_record.address.mac[3],
                            address_record.address.mac[4], address_record.address.mac[5],
                            address_record.trunk_hit_bit_value_for_each_unit);
                        }
                        address_record.hit_bit_value_on_local_unit = hitbit_value;
                        address_record.trunk_hit_bit_value_for_each_unit ^= ((0x1) << (AMTRDRV_OM_GetMyDrvUnitId()-1));
                        if (amtrdrv_mgr_bd_display)
                        {
                            BACKDOOR_MGR_Printf(" --> [%d]",address_record.trunk_hit_bit_value_for_each_unit);
                            BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------------");
                        }
                    }

                    if(address_record.trunk_hit_bit_value_for_each_unit ==0)
                    {
                        if((TRUE == AMTRDRV_OM_IsAgingOutBufferFull(1)||
                           (FALSE == AMTRDRV_OM_AgingOutBufferEnqueue(address_record.address.mac,address_record.address.vid,address_record.address.ifindex))))
                        {

                            SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION);
                            AMTRDRV_OM_UpdateLocalRecordHitBitValue(record_index,hitbit_value);
                            AMTRDRV_OM_UpdateSystemTrunkHitBit(record_index,address_record.trunk_hit_bit_value_for_each_unit);
                            AMTRDRV_OM_LocalAgingListEnqueue2Head(record_index);
                            return process_number;
                        }else
                            SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION);
                    }
                    else
                    {
                        AMTRDRV_OM_LocalAgingListEnqueue(record_index);
                        AMTRDRV_OM_UpdateLocalRecordHitBitValue(record_index,hitbit_value);
                        AMTRDRV_OM_UpdateSystemTrunkHitBit(record_index,address_record.trunk_hit_bit_value_for_each_unit);
                    }

                }
                else
                {
                    /* If buffer is full then stop doing checking procedure and insert the node back to the end
                     * local check list else insert the record into aging out buffer to wait for delete */
                    if(!AMTRDRV_OM_IsAgingOutBufferFull(1))
                    {
                            if ((TRUE == amtrdrv_mgr_integration_testing_flag)&&
                                (amtrdrv_mgr_integration_testing_printf_first_age_out_entry == TRUE))
                            {
                                if (AMTRDRV_OM_GetTotalDynamicCounter() >=amtrdrv_mgr_integration_testing_num_of_entry)
                                {
                                    UI32_T start_age_out_tick;
                                    start_age_out_tick = SYSFUN_GetSysTick();
                                    BACKDOOR_MGR_Printf("\r\n Pass %ld, AMTR start to age out entries", start_age_out_tick-amtrdrv_mgr_integration_tesing_na_start_tick);
                                    amtrdrv_mgr_integration_testing_printf_first_age_out_entry = FALSE;
                                }
                            }

                        if (FALSE == AMTRDRV_OM_AgingOutBufferEnqueue(address_record.address.mac,address_record.address.vid,address_record.address.ifindex))
                        {
                            AMTRDRV_OM_LocalAgingListEnqueue2Head(record_index);
                            return process_number;
                        }else{
                            SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION);
                        }

                    }
                    else
                    {
                        AMTRDRV_OM_LocalAgingListEnqueue2Head(record_index);
                        return process_number;
                    }
                }
            }
#if (SYS_CPNT_STACKING == TRUE)
            else if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
            {
                /* If the address is learnt on trunk port we need to update local previous_hitbit_value
                 * if the previous hit bit value != hitbit value which we got from chip */
                if(IS_TRUNK(address_record.address.ifindex))
                {
                    /* if the address is learnt on trunk port & its previous hitbit_value == hit_bit value
                     * we don't need to notify master unit but we have to insert the node back to the
                     * end of local aging check list */
                    AMTRDRV_OM_LocalAgingListEnqueue(record_index);
                    if(address_record.hit_bit_value_on_local_unit == hitbit_value)
                    {
                        if (amtrdrv_mgr_bd_display)
                        {
                            BACKDOOR_MGR_Printf("\r\n---------Slave:  AMTRDRV_MGR_ProcessCheckingAgingList( )----------------------");
                            BACKDOOR_MGR_Printf("\r\n[%02x-%02x-%02x-%02x-%02x-%02x] hit bit value are the same, do nothing!",
                            address_record.address.mac[0], address_record.address.mac[1],
                            address_record.address.mac[2], address_record.address.mac[3],
                            address_record.address.mac[4], address_record.address.mac[5]);
                        }
                        continue;
                    }
                    else
                    {
                        address_record.hit_bit_value_on_local_unit = hitbit_value;
                    }
                    AMTRDRV_OM_UpdateLocalRecordHitBitValue(record_index,hitbit_value);

                }
                /* in the case the address which is learnt on trunk port & its previous_hit_bit_value != current hit_bit_value
                 * and in another case the address which is learnt on normal port we need to put those information in the ISC packet
                 * to tell master to put addresses in the aging out buffer or just update system_trunk_hit_bit_value for that unit */
                amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&address_record,sizeof(AMTRDRV_TYPE_Record_T));
                isc_buffer_p->data.entries.number_of_entries++;
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
                /*enqueue before sending to master*/
             AMTRDRV_OM_SlaveAgingOutEnqueue(record_index);
#endif
                if(isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
                {
                    isc_buffer_p->service_id  = AMTRDRV_MGR_AGING_OUT;
                    /* try forever
                     */
                    ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                                     mref_handle_p,
                                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                     AMTRDRV_MGR_ISC_TRY_FOREVER, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
                    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
                    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                    if (isc_buffer_p==NULL)
                    {
                        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                        return process_number;
                    }
                    isc_buffer_p->data.entries.number_of_entries = 0;
                }
            }
#endif /*SYS_CPNT_STACKING*/
            else
            {
                return process_number;
            }
        }
    }
    /* So send out the remained records information by ISC */

#if(SYS_CPNT_STACKING == TRUE)

    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        if (isc_buffer_p->data.entries.number_of_entries != 0)
        {
            isc_buffer_p->service_id  = AMTRDRV_MGR_AGING_OUT;
            /* try forever
             */
            ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_FOREVER, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        }
        else
        {
            L_MM_Mref_Release(&mref_handle_p);
        }
    }
#endif /* SYS_CPNT_STACKING */
    return process_number;
} /* END OF AMTRDRV_MGR_ProcessAgingList */
#endif /*#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessMasterNABuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get NA information from NA buffer
 *          if master mode => dequeue to deal with NA
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *-----------------------------------------------------------------------------*/
UI32_T AMTRDRV_MGR_ProcessMasterNABuffer(AMTR_TYPE_AddrEntry_T *addr_buf, UI32_T buf_size)
{
    UI32_T  queue_counter;
    int i = 0;/*EPR: ES3628BT-FLF-ZZ-00735
Problem:stack:slave display exception when hot insert unit.
Rootcause:when the dut change from standalone to slave,it will enter transition mode first.And when the NETACCESS_NA is processMasterNaBuffer, "i" is rand value,and at this time task is switch ,and the AMTRDRV_OM_GetOperatingMode is changed from master to transtion,AMTRDRV_MGR_ProcessMasterNABuffer will return a rand value.
Solution:init the value i
Files:amtrdrv_mgr.c*/
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE){

#if (SYS_CPNT_MAINBOARD == TRUE)
        for(i = 0 ;(buf_size -- > 0);i++){
            if(!AMTRDRV_OM_NABufferDequeue((UI8_T *)&addr_buf[i],&queue_counter))
              break;
        }
#endif /* SYS_CPNT_MAINBOARD == TRUE */

    }

    return i;
 }
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessSlaveNABuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get NA information from NA buffer
 *          if master mode => notify upper layer to deal with NA
 *          if slave  mode => sending NA packets to master's NA buffer
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_MGR_ProcessSlaveNABuffer(void)
{
    AMTRDRV_TYPE_Record_T addr_record;
    UI32_T              process_number = 0;
#if(SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    AMTRDRV_MGR_ISCBuffer_T*    isc_buffer_p;
    UI32_T                      pdu_len;
    UI16_T                      unit_bmp=0;
    UI8_T                       master_unit_id;
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

#if(SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE){
        /* In slave mode we have to send NA information to Master and put in
         * master's NA buffer
         */

        /* Getting master unit id. This is because in slave mode we need to send those
         * information into master NA buffer
         */
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp |= ((0x01) << (master_unit_id-1));
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                  L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
        isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
            return process_number;
        }
        isc_buffer_p->data.entries.number_of_entries = 0;

        while(process_number < AMTRDRV_MGR_MAX_NUM_NA_PROCESS)
        {
            UI32_T  queue_counter;
            if(AMTRDRV_OM_NABufferDequeue((UI8_T *)&addr_record
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
            ,&queue_counter
#endif
                ))
            {
                if (TRUE == amtrdrv_mgr_integration_testing_flag)
                {
                    if (0 == amtrdrv_mgr_integration_tesing_na_start_tick)
                    {
                        amtrdrv_mgr_integration_tesing_na_start_tick = SYSFUN_GetSysTick();
                    }

                }
                amtrdrv_memcpy(&isc_buffer_p->data.entries.record[isc_buffer_p->data.entries.number_of_entries],&addr_record,sizeof(AMTRDRV_TYPE_Record_T));
                isc_buffer_p->data.entries.number_of_entries ++;
                process_number++ ;
                if(isc_buffer_p->data.entries.number_of_entries == AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
                {
                    isc_buffer_p->service_id = AMTRDRV_MGR_PROCESS_NEW_ADDR;
                    ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
                    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(AMTRDRV_MGR_ISCBuffer_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_DIRECT_CALL_POLL_ID, AMTRDRV_MGR_DELETE_ADDR_LIST) /* user_id */);
                    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                    if (isc_buffer_p==NULL)
                    {
                        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
                        return process_number;
                    }
                    isc_buffer_p->data.entries.number_of_entries = 0;
                }
            }
            else
            {
                break;
            }
        }
        if(isc_buffer_p->data.entries.number_of_entries != 0)
        {
            isc_buffer_p->service_id = AMTRDRV_MGR_PROCESS_NEW_ADDR;
            ISC_SendMcastReliable(unit_bmp,ISC_AMTRDRV_DIRECTCALL_SID,
                         mref_handle_p,
                         SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                         AMTRDRV_MGR_ISC_TRY_COUNT, AMTRDRV_MGR_ISC_TIMEOUT, FALSE);
        }
        else
        {
            L_MM_Mref_Release(&mref_handle_p);
        }
    }
#endif /* SYS_CPNT_STACKING */
    return process_number;
 }
#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessAgingOutEntries
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to process Aging out buffer
 *          if master mode => notify upper layer to delete agingout entries
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *-----------------------------------------------------------------------------*/
UI32_T AMTRDRV_MGR_ProcessAgingOutEntries(AMTR_TYPE_AddrEntry_T *addr_buf,UI32_T buf_size)
{
    UI32_T  number_of_get_entries=0;

    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE){

        number_of_get_entries = AMTRDRV_OM_AgingOutBufferDequeue(buf_size,addr_buf);

    }

    return number_of_get_entries;
}
#endif /* SYS_CPNT_MAINBOARD */
#endif /*#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_AnnounceNA_N_SecurityCheck
 *-------------------------------------------------------------------------
 * PURPOSE: Whenever Network Interface received a NA packet,it calls
 *          this function to handle the packet.
 * INPUT  :
 *          UI8_T*    dst_mac    -- the destination MAC[6] address of this packet.
 *          UI8_T*    src_mac    -- the source MAC[6] address of this packet.
 *          UI16_T    vid        -- Vlan ID
 *          UI16_T    ether_type -- what kind of protocol packet
 *          UI32_T    src_unit   -- unit
 *          UI32_T    src_port   -- port
 * OUTPUT : None
 * RETURN : TRUE  -   Drop packet.
 *          FALSE -   Not drop packet.
 * NOTES:
 *          In HW learning, this API just dispatch instruction mac to AMTR_MGR.
 *-------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_AnnounceNA_N_SecurityCheck(
         UI8_T *dst_mac,
         UI8_T *src_mac,
         UI16_T vid,
         UI16_T ether_type,
         UI32_T src_unit,
         UI32_T src_port)
{
    UI32_T               return_value = AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN; /* Learn & no Drop */
    UI32_T               src_lport;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    AMTRDRV_TYPE_Record_T  addr_record;

    src_lport = STKTPLG_POM_UPORT_TO_IFINDEX(src_unit,src_port);

    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE){
        /*Tony.Lei*/
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_mgr_test_performance == 1){
            if(!amtrdrv_mgr_test_master_counter)
                amtrdrv_mgr_test_start_tick = SYSFUN_GetSysTick();
            amtrdrv_mgr_test_master_counter++;
            if(amtrdrv_mgr_test_master_counter == amtrdrv_mgr_test_master_totol_counter){
                BACKDOOR_MGR_Printf("the tick %ld,the end%ld,counter %ld\n",amtrdrv_mgr_test_start_tick,SYSFUN_GetSysTick(),amtrdrv_mgr_test_master_counter);
                amtrdrv_mgr_test_master_counter = amtrdrv_mgr_test_master_totol_counter =0;
            }
            return TRUE;
        }
#endif
#if (SYS_CPNT_SECURITY == TRUE)
        //return_value = AMTRDRV_MGR_SecurityCheckCallback(src_lport,vid,src_mac,dst_mac,ether_type);
        /*delete by Tony for workaround*/
        return_value =  SYS_CALLBACK_MGR_SecurityCheckCallback(SYS_MODULE_AMTRDRV, src_lport,vid,src_mac,dst_mac,ether_type);
#endif /* SYS_CPNT_SECURITY */
        if(return_value & AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN)
        {
            /* put the NA information into NA buffer */
            amtrdrv_memcpy(addr_record.address.mac, src_mac, AMTR_TYPE_MAC_LEN);
            addr_record.address.vid = vid;
            addr_record.address.ifindex = src_lport;
            addr_record.address.priority = AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;
            addr_record.address.source = AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
            if(AMTRDRV_OM_NABufferEnqueue((UI8_T *)&addr_record))
            {
                /*
                 *    add by tony.lei  when enqueue success, set chip tem   UI16_T vid;
                 *
                 */
                if(amtrdrv_mgr_enable_hw_on_sw == TRUE)
                {
                    SWDRV_TYPE_L2_Address_Info_T arl_entry;
                    arl_entry.is_static =FALSE;
                    amtrdrv_memcpy(arl_entry.mac,addr_record.address.mac,SYS_ADPT_MAC_ADDR_LEN);
                    arl_entry.vid = addr_record.address.vid;
                    arl_entry.priority = AMTRDRV_CONVERT_TO_ARL_ENTRY_PRIORITY(addr_record.address.priority);
                    AMTRDRV_CONVERT_TO_ARL_ENTRY_ACTION(&addr_record.address, &arl_entry);
                    if (!IS_TRUNK(addr_record.address.ifindex)) /* if ifindex is normal port */
                    {
                        arl_entry.unit = STKTPLG_POM_IFINDEX_TO_UNIT(addr_record.address.ifindex);
                        arl_entry.port = STKTPLG_POM_IFINDEX_TO_PORT(addr_record.address.ifindex);
                        arl_entry.trunk_id = 0;
                        arl_entry.is_trunk = FALSE;
                    }
                    else  /* ifindex is trunk */
                    {
                        arl_entry.unit = 0;
                        arl_entry.port = 0;
                        arl_entry.trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(addr_record.address.ifindex)-1;
                        arl_entry.is_trunk = TRUE;
                    }
                    DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_ALL_DEVICE,&arl_entry);
                }
                amtrdrv_mgr_num_of_na_announce++;

                SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_OPERATION);

            }
            else
            {
                amtrdrv_mgr_test_enqueue_failed_counter ++;
            }
        }

        if(return_value & AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        /* In slave mode since this packet isn't belong to protocol packet
         * we can put in the slave NA buffer. After we send those information
         * to master then we will check at the moment
         */
        amtrdrv_memcpy(addr_record.address.mac, src_mac, AMTR_TYPE_MAC_LEN);
        addr_record.address.vid = vid;
        addr_record.address.ifindex = src_lport;
        addr_record.address.priority = AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;
        addr_record.address.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        if(amtrdrv_mgr_test_performance == 2){
            if(!amtrdrv_mgr_test_slave_counter)
                amtrdrv_mgr_test_slave_start_tick = SYSFUN_GetSysTick();
                amtrdrv_mgr_test_slave_counter++;
                if(amtrdrv_mgr_test_slave_counter ==amtrdrv_mgr_test_slave_totol_counter){
                    BACKDOOR_MGR_Printf("the tick %ld,the end%ld,counter %ld\n",amtrdrv_mgr_test_slave_start_tick,SYSFUN_GetSysTick(),amtrdrv_mgr_test_slave_counter);
                    amtrdrv_mgr_test_slave_counter = amtrdrv_mgr_test_slave_totol_counter =0;
                }
                return TRUE;
        }
#endif
        if(AMTRDRV_OM_NABufferEnqueue((UI8_T *)&addr_record))
        {
            amtrdrv_mgr_num_of_na_announce++;
        }
        return TRUE; /* Since this is a new address packet so we can drop it */
    }
    else
    {
        /* Transition mode, drop packets
         */
        return TRUE;
    }
#else /* SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE*/
    src_lport = STKTPLG_POM_UPORT_TO_IFINDEX(src_unit,src_port);

    if (AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
#if (SYS_CPNT_SECURITY == TRUE)
        //return_value = AMTRDRV_MGR_SecurityCheckCallback(src_lport,vid,src_mac,dst_mac,ether_type);
        return_value =  SYS_CALLBACK_MGR_SecurityCheckCallback(SYS_MODULE_AMTRDRV, src_lport,vid,src_mac,dst_mac,ether_type);
#endif /* SYS_CPNT_SECURITY */

        if(return_value & AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        /* Transition mode and Slave mode, drop packets
         */
        return TRUE;
    }
#endif
}

#if (SYS_CPNT_MAINBOARD == TRUE)

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_CallBackQueueEnqueue
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to insert the event or record information into the callback queue
 *          and also send an event to notify SYS_CallBack_Task to do something.
 * INPUT  : index   -- record_index(index < 16K) or event_index(index >16K)
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_MGR_CallBackQueueEnqueue(UI32_T index)
{
    /* if enqueue to callback queue success then send event to notify sys_callback task to dequeue */
#if (SYS_CPNT_SYSCALLBACK == TRUE)
    if(AMTRDRV_OM_CallBackQueueEnqueue(index))
        SYS_CALLBACK_SendEvent(SYS_MODULE_AMTRDRV);
#endif /* SYS_CPNT_SYSCALLBACK */
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_IsInsert2AgingList
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to check the recored need to be insert to local aging check list or not.
 *          if the entry is belong to dynamic address and is learnt on the local unit
 *          we will add into this aging list. But if the learnt address is learnt on
 *          trunk port we need to keep one record information in aging list too.
 * INPUT  : ifindex    - port
 *          lifetime   -
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_IsInsert2AgingList(UI32_T ifindex, UI32_T life_time)
{
/* Software Learning
 */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    SWDRV_Trunk_Info_T trunk_member_info;
    UI32_T             i,trunk_member_ifindex;

    /*
     * checking ifindex == trunk_id or not
     * if this mac is learnt on trunk port then doing nothing
     * else add this entry in AgingList if this entry is learnt on self unit.
     */
    if(life_time != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
    {
        return FALSE;
    }

    if(IS_TRUNK(ifindex))
    {
        if(!SWDRV_GetTrunkInfo(ifindex, &trunk_member_info))
        {
            return FALSE;
        }

        for(i=0; i<trunk_member_info.member_number; i++)
        {
            if(trunk_member_info.member_list[i].unit == AMTRDRV_OM_GetMyDrvUnitId())
            {
                return TRUE;
            }
        }
    }
    else
    {
        if(STKTPLG_POM_IFINDEX_TO_UNIT(ifindex) == AMTRDRV_OM_GetMyDrvUnitId() )
        {
            return TRUE;
        }
    }
#endif /* #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/
    return FALSE;
}

#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetValidUnitBmp
 *------------------------------------------------------------------------------
 * PURPOSE: Getting UnitBmp for all exist unit or module.
 * INPUT  : BOOL_T      --is_module_include
 *                     ( TRUE: module include / FALSE: module not include)
 * OUTPUT : None
 * RETURN : unit_bmp    -- which units are exist
 * NOTES  : This function is to search all exist unit id by stktplg
 *          then ISC can forward the information to all exist unit.
 *------------------------------------------------------------------------------*/
static UI16_T AMTRDRV_MGR_GetValidUnitBmp(BOOL_T is_module_include)
{
    UI32_T unit;
    UI16_T unit_bmp =0;

    if( is_module_include == FALSE)
    {
        for (unit = 0; STKTPLG_POM_GetNextUnit(&unit); )
        {
            if (unit == AMTRDRV_OM_GetMyDrvUnitId())
            {
                continue;
            }
            unit_bmp |= ((0x01) << (unit-1));
        }
    }
    else
    {
        for (unit = 0; STKTPLG_POM_GetNextDriverUnit(&unit); )
        {
            if (unit == AMTRDRV_OM_GetMyDrvUnitId())
            {
                continue;
            }
            unit_bmp |= ((0x01) << (unit-1));
        }
    }

return unit_bmp;
}
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */
#endif /* SYS_CPNT_MAINBOARD */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to convert AMTR_TYPE_AddrEntry_T record into
 *          SWDRV_TYPE_L2_Address_Info_T record.
 * INPUT  : amtrdrv_record_p --  AMTR_TYPE_AddrEntry_T record
 * OUTPUT : arl_record_p     --  converted SWDRV_TYPE_L2_Address_Info_T record
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry(AMTR_TYPE_AddrEntry_T *amtrdrv_record_p, SWDRV_TYPE_L2_Address_Info_T *arl_record_p)
{
    arl_record_p->vid = amtrdrv_record_p->vid;
    amtrdrv_memcpy(arl_record_p->mac, amtrdrv_record_p->mac , AMTR_TYPE_MAC_LEN);
    arl_record_p->priority = AMTRDRV_CONVERT_TO_ARL_ENTRY_PRIORITY(amtrdrv_record_p->priority);
    AMTRDRV_CONVERT_TO_ARL_ENTRY_ACTION(amtrdrv_record_p, arl_record_p);
    
    if (!IS_TRUNK(amtrdrv_record_p->ifindex)) /* if ifindex is normal/vxlan port */
    {
#if(SYS_CPNT_VXLAN == TRUE)        
        if (VXLAN_TYPE_IS_L_PORT(amtrdrv_record_p->ifindex)) /* ifindex is vxlan port */
        {
            STKTPLG_OM_GetMyUnitID((UI32_T *)&arl_record_p->unit);
            arl_record_p->port = amtrdrv_record_p->ifindex;
            arl_record_p->trunk_id = 0;
            arl_record_p->is_trunk = FALSE;
        }
        else /* ifindex is normal port */
#endif            
        {        
            arl_record_p->unit = STKTPLG_POM_IFINDEX_TO_UNIT(amtrdrv_record_p->ifindex);
            arl_record_p->port = STKTPLG_POM_IFINDEX_TO_PORT(amtrdrv_record_p->ifindex);
            arl_record_p->trunk_id = 0;
            arl_record_p->is_trunk = FALSE;
        }
    }
    else /* ifindex is trunk */
    {
        arl_record_p->unit = 0;
        arl_record_p->port = 0;
        arl_record_p->trunk_id = STKTPLG_POM_IFINDEX_TO_TRUNKID(amtrdrv_record_p->ifindex)-1;
        arl_record_p->is_trunk = TRUE;
    }
        
    if ((amtrdrv_record_p->source == AMTR_TYPE_ADDRESS_SOURCE_LEARN ||
         amtrdrv_record_p->source == AMTR_TYPE_ADDRESS_SOURCE_SECURITY ||
         amtrdrv_record_p->source == AMTR_TYPE_ADDRESS_SOURCE_MLAG) &&
        amtrdrv_record_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
        arl_record_p->is_static = FALSE;
    else
        arl_record_p->is_static = TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set/delete the address in chip side
 * INPUT  : AMTRDRV_TYPE_Record_T  address_record --  address info
 *          UI8_T                  action         --  set or delete
 *          UI8_T                  reserved_state --  reserved state of the entry
 * OUTPUT : None
 * RETURN : UI32_T  event ( set_success / delete_success / set fail)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_MGR_SetChip(AMTRDRV_TYPE_Record_T *address_record,UI8_T action, UI8_T reserved_state)
{
    SWDRV_TYPE_L2_Address_Info_T  arl_entry;
    UI32_T                        event=0;
    DEV_AMTRDRV_Ret_T             retval;
    BOOL_T                        ret;
    UI32_T                        current_mode = 0;

    switch(action)
    {
        case AMTRDRV_MGR_ACTION_ADD: /* Need to add the record information to chip */
            /* 1. Converting AMTRDRV_OM format to SWDRV_TYPE_L2_Address_Info_T format
             */
            AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry(&address_record->address, &arl_entry);
            /* 2. program chip */
            /* if the addr_entry is CPU mac since CPU mac use different API to program chip */
            if(address_record->address.source == AMTR_TYPE_ADDRESS_SOURCE_SELF)
            {
#if (SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD == SYS_CPNT_AMTR_CPU_INTERVENTION_FFP_TRAP)
                //kh_shi HRDRV_SetInterventionEntry(address_record->address.vid, address_record->address.mac);

#endif /* SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD */
               /* set address into local device  (CPU mac will only set in master unit)
                */
#if (SYS_CPNT_MAINBOARD == TRUE)
                if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
                {
                    retval = DEV_AMTRDRV_PMGR_SetInterventionEntry(address_record->address.vid, address_record->address.mac,DEV_AMTRDRV_EXCLUDE_1ST_DEVICES, DEV_AMTRDRV_INTERV_MODE_DA);
                    ret = (retval==DEV_AMTRDRV_SUCCESS)? TRUE:FALSE;                       
                    current_mode = 1;
                }
                else
#endif /* SYS_CPNT_MAINBOARD */
                {
                    retval = DEV_AMTRDRV_PMGR_SetInterventionEntry(address_record->address.vid, address_record->address.mac,DEV_AMTRDRV_ALL_DEVICE, DEV_AMTRDRV_INTERV_MODE_DA);
                    ret = (retval==DEV_AMTRDRV_SUCCESS)? TRUE:FALSE;      
                    current_mode = 2;
                }
            }
            else /* the mac is not CPU MAC */
            {
#if( SYS_CPNT_MAINBOARD == TRUE )
                if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
                {   /* In master mode we only need to program the other chips except first chip
                     * since we already did before I add record into OM
                     */
                    /* 1. program entry info to chip */
#if(SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT > 1)
                    if(AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(reserved_state))
                    {
                        /* program to all devices if attribute between CHIP and OM
                         * is not synchronized
                         */
                        retval = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_ALL_DEVICE,&arl_entry);
                    }
                    else
                        retval = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_EXCLUDE_1ST_DEVICES,&arl_entry);
#else
                    /* program to all devices if attribute between CHIP and OM
                     * is not synchronized
                     */
                    if(AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(reserved_state))
                        retval = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_ALL_DEVICE,&arl_entry);
                    else
                        retval = DEV_AMTRDRV_SUCCESS;
#endif
                    current_mode = 3;
                }
                else
#endif /* SYS_CPNT_MAINBOARD */
                {   /* in slave mode we need to program all chips */
                    /* 1. program entry info to chip */
                    retval = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_ALL_DEVICE,&arl_entry);
                    current_mode = 4;
                }

                if (retval==DEV_AMTRDRV_SUCCESS)
                {
                    ret = TRUE;
                }
                else
                {
                    ret = FALSE;
                }
            }
#if( SYS_CPNT_MAINBOARD == TRUE )
            if( ret == TRUE)
            {
                event = L_HASH_SET_SUCCESS_EV;
                AMTRDRV_MGR_DBGMSG("Address added to hardware ARL table successfully");
            }
            else
            {
                event = L_HASH_FAIL_EV;
                AMTRDRV_MGR_DBGMSG("Failed to add address to hardware ARL table");
                if (current_mode==0){
                    AMTRDRV_MGR_DBGMSG("current_mode=0\n");}
                else if (current_mode==1){
                    AMTRDRV_MGR_DBGMSG("current_mode=1\n");}
                else if (current_mode==2){
                    AMTRDRV_MGR_DBGMSG("current_mode=2\n");}
                else if (current_mode==3){
                    AMTRDRV_MGR_DBGMSG("current_mode=3\n");}
                else if (current_mode==4){
                    AMTRDRV_MGR_DBGMSG("current_mode=4\n");}
            }
#endif /* SYS_CPNT_MAINBOARD */
            break;

        case AMTRDRV_MGR_ACTION_DEL:
            /* If entry isn't CPU mac and (ifindex == 0 ||life_time == other), it is under create entry.
             * Under create entry is only wrote in OM without chip.
             */
            if((address_record->address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_OTHER) || ((address_record->address.ifindex == 0)&&(address_record->address.source!=AMTR_TYPE_ADDRESS_SOURCE_SELF)))
            {
                break;
            }

            if(address_record->address.source ==AMTR_TYPE_ADDRESS_SOURCE_SELF){
#if (SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD == SYS_CPNT_AMTR_CPU_INTERVENTION_FFP_TRAP)
                //kh_shi HRDRV_DeleteInterventionEntry(address_record->address.vid, address_record->address.mac);
#endif /* SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD */
                /* 1. delete the entry from chip */
                DEV_AMTRDRV_PMGR_DeleteInterventionEntry(address_record->address.vid, address_record->address.mac);
                ret = TRUE;
            }/*end sourec==self*/
            else
            {
                /* 1. delete the entry from chip */
                DEV_AMTRDRV_PMGR_DeleteAddr(address_record->address.vid,address_record->address.mac);
                /*if the chip has not the mac entry , delete the entry from om*/
                ret = TRUE;

#if ( SYS_CPNT_MAINBOARD == TRUE )
                if( ret == TRUE){

                    event = L_HASH_DEL_SUCCESS_EV;
                    AMTRDRV_MGR_DBGMSG("Address is deleted from hardware ARL table successfully");
                }else{

                    event = L_HASH_FAIL_EV;
                    AMTRDRV_MGR_DBGMSG("Failed to delete address from hardware ARL table");
                }
#endif /* SYS_CPNT_MAINBOARD */
            }/*end sourec!=self*/
            break;
        default:
            break;
    }
    return event;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ARLLookUp
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to look up the address in ARL or not
 * INPUT  : UI32_T vid                - VLAN ID
 *          UI8_T *mac                - MAC address
 *          UI32_T unit               - Unit number
 * OUTPUT : UI32_T *port              - Port number
 *          UI32_T *trunk_id          - Trunk ID
 *          BOOL_T *is_trunk          - is trunk or not
 * RETURN : TRUE(found in chip side) / FALSE (no exact address in the chip)
 * NOTES  : This function is only call backdoor for now
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_ARLLookUp( UI32_T vid,
                                                    UI8_T *mac,
                                                    UI32_T unit,
                                                    UI32_T *port,
                                                    UI32_T *trunk_id,
                                                    BOOL_T *is_trunk)
{
    /* BODY
     */

    /* Local operation */
    if(TRUE == DEV_AMTRDRV_PMGR_ARLLookUp(vid, mac, port, trunk_id, is_trunk)) /* look up an address entry within local unit*/
    {
        return TRUE;
    }
    else
    {
       return FALSE;
    }
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ID2Event
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to convert ID to Event type
 * INPUT  : UI32_T               index            --
 *          UI32_T               *id              --
 * OUTPUT : AMTR_TYPE_AddrEntry_T     *addr_entry      --
 *          AMTR_TYPE_Command_T       *action          --
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_MGR_ID2Event(AMTR_TYPE_AddrEntry_T *addr_entry,UI32_T index,UI32_T *id,AMTR_TYPE_Command_T *action)
{
    UI32_T position=0;
    enum
    {
        RANGE_TYPE_IFINDEX   = 1,
        RANGE_TYPE_VID       = 2,
        RANGE_TYPE_LIFE_TIME = 3,
        RANGE_TYPE_SOURCE    = 4,
    } range;

    /* index is 0 base.
     */
    if(index < (SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID ) )
    {
        range = RANGE_TYPE_IFINDEX;
        addr_entry->ifindex = index - SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + 1 ;
    }
    else if(index <(SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + AMTR_TYPE_MAX_LOGICAL_VLAN_ID))
    {
        range = RANGE_TYPE_VID;
        addr_entry->vid = index - SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY -AMTR_TYPE_MAX_LOGICAL_PORT_ID +1;
    }
    else if(index <(SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + AMTR_TYPE_MAX_LOGICAL_VLAN_ID +AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME))
    {
        range = RANGE_TYPE_LIFE_TIME;
        addr_entry->life_time= index - SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY -AMTR_TYPE_MAX_LOGICAL_PORT_ID -AMTR_TYPE_MAX_LOGICAL_VLAN_ID+1;
    }
    else
    {
        range = RANGE_TYPE_SOURCE;
        addr_entry->source= index - SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY -AMTR_TYPE_MAX_LOGICAL_PORT_ID -AMTR_TYPE_MAX_LOGICAL_VLAN_ID- AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME +1;
    }

    /* meaning of position(used with range to generate various combinations):
     *   0    ->  All
     *   1-5  ->  For one type of LIFETIME
     *   6-11 ->  For one type of SOURCE
     */

    if(*id == 0xFFFFFFFF)
    {
       position = 0;
       *id = 0 ;
    }
    else if( *id & AMTRDRV_MGR_LIFETIME_OTHER_BIT)
    {
        position = 1;
        addr_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_OTHER;
        *id ^=AMTRDRV_MGR_LIFETIME_OTHER_BIT;
    }
    else if( *id & AMTRDRV_MGR_LIFETIME_INVALID_BIT)
    {
        position = 2;
        addr_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_INVALID;
        *id ^= AMTRDRV_MGR_LIFETIME_INVALID_BIT;
    }
    else if( *id & AMTRDRV_MGR_LIFETIME_PERMANENT_BIT)
    {
        position = 3;
        addr_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
        *id ^= AMTRDRV_MGR_LIFETIME_PERMANENT_BIT;
    }
    else if( *id & AMTRDRV_MGR_LIFETIME_DELETE_ON_RESET_BIT)
    {
        position = 4;
        addr_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
        *id ^= AMTRDRV_MGR_LIFETIME_DELETE_ON_RESET_BIT;
    }
    else if( *id & AMTRDRV_MGR_LIFETIME_DELETE_ON_TIMEOUT_BIT)
    {
        position = 5;
        addr_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        *id ^= AMTRDRV_MGR_LIFETIME_DELETE_ON_TIMEOUT_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_INTERNAL_BIT)
    {
        position = 6;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_INTERNAL;
        *id ^= AMTRDRV_MGR_SOURCE_INTERNAL_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_INVALID_BIT)
    {
        position = 7;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_INVALID;
        *id ^= AMTRDRV_MGR_SOURCE_INVALID_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_LEARN_BIT)
    {
        position = 8;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        *id ^= AMTRDRV_MGR_SOURCE_LEARN_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_SELF_BIT)
    {
        position = 9;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_SELF;
        *id ^= AMTRDRV_MGR_SOURCE_SELF_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_CONFIG_BIT)
    {
        position = 10;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
        *id ^= AMTRDRV_MGR_SOURCE_CONFIG_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_SECURITY_BIT)
    {
        position = 11;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
        *id ^= AMTRDRV_MGR_SOURCE_SECURITY_BIT;
    }
    else if( *id & AMTRDRV_MGR_SOURCE_MLAG_BIT)
    {
        position = 12;
        addr_entry->source = AMTR_TYPE_ADDRESS_SOURCE_MLAG;
        *id ^= AMTRDRV_MGR_SOURCE_MLAG_BIT;
    }

    switch(range)
    {
        case RANGE_TYPE_IFINDEX:
            if(position == 0)
            {
                *action = AMTR_TYPE_COMMAND_DELETE_BY_PORT;
            }
            else
            {
                if(position > AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT/*=5*/)
                {
                    *action = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE;
                }
                else
                {
                    *action = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME;
                }
            }
            break;
        case RANGE_TYPE_VID:
            if(position == 0)
            {
                *action = AMTR_TYPE_COMMAND_DELETE_BY_VID;
            }
            else
            {
                if(position > AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT/*=5*/)
                {
                    *action = AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE;
                }
                else
                {
                    *action = AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME;
                }
            }
            break;
        case RANGE_TYPE_LIFE_TIME:
            if(position == 0)
            {
                *action = AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME;
            }
            break;
        case RANGE_TYPE_SOURCE:
            if(position == 0)
            {
                *action = AMTR_TYPE_COMMAND_DELETE_BY_SOURCE;
            }
            break;
        default:
            break;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Event2ID
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to convert Event type to ID
 * INPUT  : UI32_T               cookie         -- maybe be the information for life_time or source
 *          AMTR_TYPE_Command_T *action         --
 * OUTPUT : UI32_T              *event_id       --

 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Event2ID(UI32_T cookie,AMTR_TYPE_Command_T action,UI32_T *event_id)
{
    switch(action)
    {
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT:
        case AMTR_TYPE_COMMAND_DELETE_BY_VID:
        case AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME:
        case AMTR_TYPE_COMMAND_DELETE_BY_SOURCE:
            *event_id = 0xFFFFFFFF;
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME:
        case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME:
            *event_id |= 0x1 << ( cookie -1);
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE:
        case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE :
            *event_id |= 0x1 << (cookie-1 + AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME);
            break;
        default :
            break;
    }
    return ;
}
#endif /* SYS_CPNT_MAINBOARD */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAllAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete all addresses from Hash table & chip on local unit
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAllAddr(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;
    /* Delete addresses from chip
     */
    if(!DEV_AMTRDRV_PMGR_DeleteAllAddr())
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAllAddr failed to delete all address from chip");
        result = FALSE;
    }
    AMTRDRV_OM_ClearNABuffer();

#if (SYS_CPNT_MAINBOARD == TRUE)
    /* Delete addresses from database */
    if(!AMTRDRV_OM_ClearDatabase())
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAllAddr failed to delete all address from hash table");
        result = FALSE;
    }
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_STACKING == TRUE)
    if(AMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE){
        ISC_MCastReply(&(blocked_command_p->isc_key), result ? ISC_OP_ACK : ISC_OP_NAK);
    }
#endif

    return result;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delet specified addresses which life_time is match.
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;
    UI32_T event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + AMTR_TYPE_MAX_LOGICAL_VLAN_ID +blocked_command_p->life_time -1;

    result = AMTRDRV_OM_SearchNDelete(0,0,blocked_command_p->life_time,0,AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByLifeTime failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE){
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */
        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->life_time, AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else{
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrBySource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which source is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrBySource(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;
    UI32_T event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + AMTR_TYPE_MAX_LOGICAL_VLAN_ID +AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME + blocked_command_p->source;

    result = AMTRDRV_OM_SearchNDelete(0,0,0,blocked_command_p->source,AMTR_TYPE_COMMAND_DELETE_BY_SOURCE,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrBySource failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */
        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->source, AMTR_TYPE_COMMAND_DELETE_BY_SOURCE);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByPort(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T  result=TRUE;
    UI32_T  event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + blocked_command_p->ifindex -1;

    /*cookie = 0, it means the cookie will be ignored.*/
    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,0,0,0,AMTR_TYPE_COMMAND_DELETE_BY_PORT,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPort Fail");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event into
         * the sync queue
         * else we need to update event_id
         */
        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->ifindex, AMTR_TYPE_COMMAND_DELETE_BY_PORT);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByPortAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port & life_time is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByPortAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T                 result=TRUE;
    UI32_T                 event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + blocked_command_p->ifindex -1;

    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,0,blocked_command_p->life_time,0,AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPortAndLifeTime failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */
        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->life_time, AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByPortAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port & source is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByPortAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T  result=TRUE;
    UI32_T  event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + blocked_command_p->ifindex -1;

    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,0,0,blocked_command_p->source,AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPortAndSource failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */

        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->source, AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByVID
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which vid is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByVID(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;
    UI32_T event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + blocked_command_p->vid -1;

    result = AMTRDRV_OM_SearchNDelete(0,blocked_command_p->vid,0,0,AMTR_TYPE_COMMAND_DELETE_BY_VID,0,0);

    if(!result)
    {
       AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByVID failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the syn cqueue
         * else we need to update event_id
         */

        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->vid, AMTR_TYPE_COMMAND_DELETE_BY_VID);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK : ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByVidAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which vid and life_time is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByVidAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T  result=TRUE;
    UI32_T  event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + blocked_command_p->vid -1;

    result = AMTRDRV_OM_SearchNDelete(0,blocked_command_p->vid,blocked_command_p->life_time,0,AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByVidAndLifeTime failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */

        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->life_time, AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which vid & source is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByVidAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T  result=TRUE;
    UI32_T  event_index;

    event_index = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY + AMTR_TYPE_MAX_LOGICAL_PORT_ID + blocked_command_p->vid -1;

    result = AMTRDRV_OM_SearchNDelete(0,blocked_command_p->vid,0,blocked_command_p->source,AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE,0,0);

    if(result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByVidAndSource failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* If there is no event in the sync queue we just insert the event
         * into the sync queue
         * else we need to update event_id
         */

        AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(event_index, blocked_command_p->source, AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalDeleteAddrByVIDnPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which vid & port is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *          3.For this API since we don't have enought space to store such event in
 *            syncqueue or callback queue core_layer needs to handle by itself
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByVIDnPort(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;

    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,blocked_command_p->vid,0,0,AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT,0,0);
    
    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByVIDnPort failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        if (blocked_command_p->is_sync_op==FALSE)
        {
            ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
        }
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port & vid & life_time is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *          3.In this function since we don't have enough space to store this event
 *            in sync queue or callback queue core_layer needs to handle this by itself.
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;

    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,0,blocked_command_p->life_time,0,AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME,blocked_command_p->vlanlist,blocked_command_p->vlan_counter);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndLifeTime failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        if (blocked_command_p->is_sync_op==FALSE)
        {
            ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
        }
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port & vid & source is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *          3.In this function since we don't have enough space to store this event
 *            in sync queue or callback queue core_layer needs to handle this by itself.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndSource(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;

    result = AMTRDRV_OM_SearchNDelete(blocked_command_p->ifindex,blocked_command_p->vid,0,blocked_command_p->source,AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE,0,0);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPortAndVidAndSource failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;

}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_LocalDeleteAddrByPortAndVidExceptCertainAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete specified addresses which port & vid & source is match
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *          3.In this function since we don't have enough space to store this event
 *            in sync queue or callback queue core_layer needs to handle this by itself.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalDeleteAddrByPortAndVidExceptCertainAddr(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;

    result = AMTRDRV_OM_SearchNDeleteExceptCertainAddr(blocked_command_p->ifindex,blocked_command_p->vid,blocked_command_p->cookie.mac_list_ar,blocked_command_p->cookie.mask_list_ar,blocked_command_p->cookie.num_in_list);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalDeleteAddrByPortAndVidExceptCertainAddr failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_LocalChangePortLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to handle the port life time is changed.
 *          We need to update the OM recoreds and query group
 * INPUT  : blocked_command_p - the block command from AMTRDRV_MGR
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API has to release semaphore before return.
 *            Because AMTRDRV have gotten amtrdrv_local_finished_smid
 *            before command blocking.
 *          2.Caller thread can return if this semaphore has been released here.
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_LocalChangePortLifeTime(AMTR_TYPE_BlockedCommand_T * blocked_command_p)
{
    BOOL_T result=TRUE;
    BOOL_T is_update_aging_list=FALSE;

    is_update_aging_list = AMTRDRV_MGR_IsInsert2AgingList(blocked_command_p->ifindex,blocked_command_p->life_time);
    result = AMTRDRV_OM_UpdateRecordLifeTime(blocked_command_p->ifindex,blocked_command_p->life_time,is_update_aging_list);

    if(!result)
    {
        AMTRDRV_MGR_DBGMSG("AMTRDRV_MGR_LocalChangePortLifeTime failed");
    }

    if(AMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE){
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        /* restart synchronization from chip's first entry.
         */
        AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SYNC_CEASING);
#endif
    }
#if (SYS_CPNT_STACKING == TRUE)
    else
    {
        ISC_MCastReply(&(blocked_command_p->isc_key), result?ISC_OP_ACK:ISC_OP_NAK);
    }
#endif/*#if (SYS_CPNT_STACKING ==TRUE) */

    return result;
}
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_SetAgingTime
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to set aging time
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : TRUE / FALSE
 * RETURN : None
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_SetAgingTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
    /* local unit setting */
    /* Due to aging out function disable we didn't update each record's timestamp.
     * When aging out functino is enable again we need to update the record's timestamp
     * as current system time one by one and also start to polling aging checking list.
     */
    if((AMTRDRV_OM_GetOperAgingTime() == 0) && (request_p->data.aging_time != 0))
    {
        AMTRDRV_OM_LocalAgingListUpdateTimeStamp();
    }
#endif /* SYS_CPNT_MAINBOARD */

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    if(DEV_AMTRDRV_PMGR_SetAgeingTime(request_p->data.aging_time) == FALSE)
    {
        return FALSE;
    }
#endif/*End of if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE) */

    AMTRDRV_OM_SetOperAgingTime(request_p->data.aging_time);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_SetAddrEntryList
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote unit to set addresses
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_SetAddrEntryList(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    UI32_T  i;

    if(amtrdrv_mgr_isc_test_returndonothing)
        return TRUE;
    for(i = 0;i < request_p->data.entries.number_of_entries;i++)
    {
        AMTRDRV_MGR_SetAddrList2LocalUnit( 1,(AMTR_TYPE_AddrEntry_T *) &(request_p->data.entries.record[i].address));
    }
    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_SetAddrDriectly
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote unit to set address into OM with running FSM
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_SetAddrDirectly(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    /* since slave has received this command it means not only local hash table but also
     * needs to program chip it means you need to set FALSE for arg 2
     */
    AMTRDRV_MGR_SetAddrDirectly((AMTR_TYPE_AddrEntry_T *)&(request_p->data.entries.record[0].address),FALSE);
    return TRUE;
}
#endif /* end of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_AgingOut
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to handle the aging out information from slave units
 *          We need to check the master buffer still have enough space to store those info
 *          or not. If not then return fail to tell slave unit to resend.
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This remote function is only executed in master unit
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_AgingOut(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
#if (SYS_CPNT_MAINBOARD == TRUE)
    UI32_T     i;
    UI32_T     record_index;

    /* check master aging out buffer still have enough space to store information or not */
    /* Since ISC task is higher than AMTRDRV_ADDRESS_TASK, AMTRDRV_ADDRESS_TASK won't to
     * set any addresses into aging out buffer. It means after passing the buffer checking
     * the space is enough for handling remote addresses info.
     */
    if(AMTRDRV_OM_IsAgingOutBufferFull(request_p->data.entries.number_of_entries))
    {
        return FALSE;
    }
    else
    {

        for(i=0;i<request_p->data.entries.number_of_entries;i++)
        {
            if(IS_TRUNK(request_p->data.entries.record[i].address.ifindex))
            {
                /* if we can't find the record in OM then skip this address and process next address  */
                if(AMTRDRV_OM_GetRecordIndex((UI8_T *)&(request_p->data.entries.record[i]),&record_index))
                {
                    if (amtrdrv_mgr_bd_display)
                    {
                        BACKDOOR_MGR_Printf("\r\n---------Master:  AMTRDRV_MGR_Remote_AgingOut( )----------------------");
                        BACKDOOR_MGR_Printf("\r\nUnit[%d] send Trunk port hit bit value [%d]", key->unit,
                                  request_p->data.entries.record[i].hit_bit_value_on_local_unit);
                        BACKDOOR_MGR_Printf("\r\nAnd update [%02x-%02x-%02x-%02x-%02x-%02x] trunk hit bit value from [%d] to ",
                                  request_p->data.entries.record[i].address.mac[0], request_p->data.entries.record[i].address.mac[1],
                                  request_p->data.entries.record[i].address.mac[2], request_p->data.entries.record[i].address.mac[3],
                                  request_p->data.entries.record[i].address.mac[4], request_p->data.entries.record[i].address.mac[5],
                                  request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit);

                    }

                    if(request_p->data.entries.record[i].hit_bit_value_on_local_unit == 1)
                    {
                        request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit |= ((0x1) << ((key->unit)-1));
                    }
                    else
                    {
                        request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit ^= ((0x1) << ((key->unit)-1));
                    }

                    if (amtrdrv_mgr_bd_display)
                    {
                        BACKDOOR_MGR_Printf("[%d]",request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit);
                        BACKDOOR_MGR_Printf("\r\n-----------------------------------------------------------------");
                    }
                    if(request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit ==0)
                    {
                        if (FALSE == AMTRDRV_OM_AgingOutBufferEnqueue(request_p->data.entries.record[i].address.mac,
                            request_p->data.entries.record[i].address.vid,request_p->data.entries.record[i].address.ifindex))
                        {
                            return FALSE;
                        }else
                            SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION);
                    }
                    else
                    {
                        AMTRDRV_OM_UpdateSystemTrunkHitBit(record_index,request_p->data.entries.record[i].trunk_hit_bit_value_for_each_unit);
                    }
                }
            }
            else
            {
                if (FALSE == AMTRDRV_OM_AgingOutBufferEnqueue(request_p->data.entries.record[i].address.mac,
                    request_p->data.entries.record[i].address.vid,request_p->data.entries.record[i].address.ifindex))
                {
                    return FALSE;
                }else
                   SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION);
            }
        }
    }
#endif /* SYS_CPNT_MAINBOARD */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrEntryList
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote unit to delete addresses
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrEntryList(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    /* 1. Reply to master unit */
    UI32_T  i;
    for(i=0;i<request_p->data.entries.number_of_entries;i++)
        AMTRDRV_MGR_DeleteAddrEntryList(1,(AMTR_TYPE_AddrEntry_T *) &(request_p->data.entries.record[i].address));
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrDirectly
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete address without running FSM
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrDirectly(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{

    AMTRDRV_MGR_DeleteAddrDirectly(&request_p->data.entries.record[0].address,FALSE );
    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_Remote_DeleteAllAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete all addresses from chip & OM
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAllAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_ALL;
    blocked_command_p->is_sync_op = FALSE;

    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION Name:AMTRDRV_MGR_Remote_DeleteAddrByLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by life_time
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_Remote_DeleteAddrByLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->life_time = request_p->data.entries.record[0].address.life_time;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME;
    blocked_command_p->is_sync_op = FALSE;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrBySource
 *------------------------------------------------------------------------------
 * PURPOSE: This is remote function to let remote unit to delete addresses by source
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T  AMTRDRV_MGR_Remote_DeleteAddrBySource(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->source = request_p->data.entries.record[0].address.source;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_SOURCE;
    blocked_command_p->is_sync_op = FALSE;
    /* send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote unit to delete addresses by port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT;
    blocked_command_p->is_sync_op = FALSE;
    /* send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByLifeTimeAndPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by life_time & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByLifeTimeAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->life_time= request_p->data.entries.record[0].address.life_time;
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME;
    blocked_command_p->is_sync_op = FALSE;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrBySoreceAndPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by source & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrBySoreceAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->source= request_p->data.entries.record[0].address.source;
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE;
    blocked_command_p->is_sync_op = FALSE;
    /* send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVID
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVID(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->vid= request_p->data.entries.record[0].address.vid;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_VID;
    blocked_command_p->is_sync_op = FALSE;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & life_time
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/

static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->vid= request_p->data.entries.record[0].address.vid;
    blocked_command_p->life_time= request_p->data.entries.record[0].address.life_time;
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME;
    blocked_command_p->is_sync_op = FALSE;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & source
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndSource(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->vid = request_p->data.entries.record[0].address.vid;
    blocked_command_p->source = request_p->data.entries.record[0].address.source;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE;
    blocked_command_p->is_sync_op = FALSE;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVIDnPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVIDnPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T *blocked_command_p;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->vid= request_p->data.entries.record[0].address.vid;
    blocked_command_p->ifindex= request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT;
    blocked_command_p->is_sync_op=request_p->data.entries.is_synchronous_op;
    /* Send event to notify AMTRDRv_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTimeAndPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & life_time & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndLifeTimeAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T * blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    if(request_p->type == 0){
        blocked_command_p->vid = request_p->data.entries.record[0].address.vid;
        blocked_command_p->vlanlist[0] = request_p->data.entries.record[0].address.vid;
        blocked_command_p->vlanlist[1] = 0;
        blocked_command_p->vlan_counter = 1;
        blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
        blocked_command_p->life_time = request_p->data.entries.record[0].address.life_time;
    }else{
         UI16_T  vid,vlan_count = 0;
         for (vid = 1; vid <= SYS_DFLT_DOT1QMAXVLANID ; vid++){
             if ((0 < vid) && (vid <= 1023)){
                 if(request_p->data.mstp_entry_callback.mstp_entry.mstp_instance_vlans_mapped[vid>>3] & (0x01<<(vid%8))){
                     blocked_command_p->vlanlist[vlan_count] = vid;
                     vlan_count++;
                 }

             }else if ((1023 < vid) && (vid <= 2047)){
                 if(request_p->data.mstp_entry_callback.mstp_entry.mstp_instance_vlans_mapped2k[(vid>>3)-128] & (0x01<<(vid%8))){
                     blocked_command_p->vlanlist[vlan_count] = vid;
                     vlan_count++;
                 }
             }else if ((2047 < vid) && (vid <= 3071)){
                 if(request_p->data.mstp_entry_callback.mstp_entry.mstp_instance_vlans_mapped3k[(vid>>3)-256] & (0x01<<(vid%8))){
                     blocked_command_p->vlanlist[vlan_count] = vid;
                     vlan_count++;
                 }
             }else if ((3071 < vid) && (vid < 4096)){
                 if(request_p->data.mstp_entry_callback.mstp_entry.mstp_instance_vlans_mapped4k[(vid>>3)-384] & (0x01<<(vid%8))){
                     blocked_command_p->vlanlist[vlan_count] = vid;
                     vlan_count++;

                 }
             }
        }

        blocked_command_p->ifindex = request_p->data.mstp_entry_callback.ifindex;
        blocked_command_p->vlanlist[vlan_count] = 0;
        blocked_command_p->vlan_counter = vlan_count;
        blocked_command_p->life_time = request_p->data.mstp_entry_callback.life_time;
    }

    blocked_command_p->isc_key = *key;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME;
    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVidAndSourceAndPort
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & source & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVidAndSourceAndPort(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T * blocked_command_p ;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->vid = request_p->data.entries.record[0].address.vid;
    blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->source = request_p->data.entries.record[0].address.source;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE;
    /* Send Event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_DeleteAddrByVIDnPortExceptCertainAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let remote units to delete addresses by vid & source & port
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DeleteAddrByVIDnPortExceptCertainAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T * blocked_command_p ;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);
    blocked_command_p->isc_key = *key;
    blocked_command_p->vid= request_p->data.masklist.record.address.vid;
    blocked_command_p->ifindex= request_p->data.masklist.record.address.ifindex;
    blocked_command_p->cookie.num_in_list = request_p->data.masklist.num_of_mask_list;
    memcpy(blocked_command_p->cookie.mac_list_ar, request_p->data.masklist.mask_mac_list, AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK*SYS_ADPT_MAC_ADDR_LEN);
    memcpy(blocked_command_p->cookie.mask_list_ar, request_p->data.masklist.mask_list, AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK*SYS_ADPT_MAC_ADDR_LEN);
    blocked_command_p->blocked_command=AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR;
    /* Send Event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_ChangePortLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let romote units to update addresses & query group when port life_time is changed
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_ChangePortLifeTime(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTR_TYPE_BlockedCommand_T * blocked_command_p ;
    blocked_command_p = AMTRDRV_OM_GetBlockCommand(0);

    blocked_command_p->isc_key = *key;
    blocked_command_p->ifindex = request_p->data.entries.record[0].address.ifindex;
    blocked_command_p->life_time = request_p->data.entries.record[0].address.life_time;
    blocked_command_p->blocked_command = AMTR_TYPE_COMMAND_CHANGE_PORT_LIFE_TIME;

    /* Send event to notify AMTRDRV_ASIC_COMMAND_TASK to run */
    SYSFUN_SendEvent(AMTRDRV_OM_GetAsicComTaskId(), AMTRDRV_MGR_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_CreateMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will manipulte all of address in ARL via IMC
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_CreateMulticastAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    return DEV_AMTRDRV_PMGR_CreateMulticastAddrTblEntry(request_p->data.entries.record[0].address.vid,request_p->data.entries.record[0].address.mac );

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGr_Remote_DestroyMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will manipulte all of address in ARL via IMC
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_DestroyMulticastAddr(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    return DEV_AMTRDRV_PMGR_DestroyMulticastAddrTblEntry(request_p->data.entries.record[0].address.vid,
                                                    request_p->data.entries.record[0].address.mac);

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_SetMulticastPortMember
 *------------------------------------------------------------------------------
 * PURPOSE: This function will manipulte all of address in ARL via IMC
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : callbacked by isc
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_SetMulticastPortMember(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    return DEV_AMTRDRV_PMGR_SetMulticastPortList(request_p->data.mclist.vid,
                                            request_p->data.mclist.mac,
                                            request_p->data.mclist.pbmp,
                                            request_p->data.mclist.tbmp);
}
#endif /* end of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_ProcessNewAddress
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to let slave units to put NA information in master NA buffer
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This function is only executed in master unit to process the new address info
 *          from slave units
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_ProcessNewAddress(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    UI32_T    i=0;
    BOOL_T   send_ok = FALSE;
#ifdef AMTR_TYPE_DRV_SLAVE_ISC_TEST
     if(amtrdrv_mgr_isc_test){
        if(!amtrdrv_mgr_test_slave_counter)
        amtrdrv_mgr_test_slave_start_tick = SYSFUN_GetSysTick();

        amtrdrv_mgr_test_slave_counter += request_p->data.entries.number_of_entries;

        if(amtrdrv_mgr_test_slave_counter > amtrdrv_mgr_test_slave_totol_counter){
        BACKDOOR_MGR_Printf("the start tick %ld, the ending tick %ld,the counter %ld\n",(long)amtrdrv_mgr_test_slave_start_tick,(long)SYSFUN_GetSysTick(),(long)amtrdrv_mgr_test_slave_counter);
        amtrdrv_mgr_test_slave_counter = amtrdrv_mgr_test_slave_totol_counter = 0;
        }
        return TRUE;
      }
#endif
    while(request_p->data.entries.number_of_entries > 0)
    {
        if(AMTRDRV_OM_NABufferEnqueue((UI8_T *)&request_p->data.entries.record[i]))
        {
            i++;
            request_p->data.entries.number_of_entries -- ;
            if(send_ok == FALSE)
                send_ok = TRUE;
        }
        else
        {
            break; /* since the buffer is full we can't do anything for it then drop anyway */
        }
    }
    if(send_ok == TRUE)
        SYSFUN_SendEvent(AMTRDRV_MGR_GetAmtrID(),AMTR_TYPE_EVENT_ADDRESS_OPERATION);
    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_SetAddrToLocalModule
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to program local module chip
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  :
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_SetAddrToLocalModule(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    AMTRDRV_MGR_ISCSingleBuffer_T *addr;

    addr =( AMTRDRV_MGR_ISCSingleBuffer_T *)request_p;
    /* reserved_state is not required on slave/module device, so just
     * specify 0.
     */
    AMTRDRV_MGR_SetChip(&(addr->record),addr->record.address.action, 0);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_PollAddrOfModuleHitBitValue
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to poll entry's hit bit value in module chip
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  :
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_PollAddrOfModuleHitBitValue(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    UI32_T                          unit,port_counter,checking_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    AMTRDRV_MGR_ISCSingleBuffer_T   *addr;
    L_MM_Mref_Handle_T*             mref_handle_p;
    UI32_T                          pdu_len;
    UI8_T                           *hitbit_value;

    addr = (AMTRDRV_MGR_ISCSingleBuffer_T *)request_p;
    addr->return_value = TRUE;

    /* 1.unit & checking_port are useless here but we still need to initialize them for passing
     *   compiler.
     * 2.We put the port_counter = 0XFFFF. It means to poll all chips in module board
     */
    unit = 0;
    checking_port[0]=0;
    port_counter =  0xFFFF;
    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI8_T), /* tx_buffer_size */
                              L_MM_USER_ID(SYS_MODULE_AMTRDRV,AMTRDRV_MGR_CALL_BY_AGENT_POLL_ID, AMTRDRV_MGR_POLL_ADDR_OF_MODULE_HITBIT_VALUE) /* user_id */);
    hitbit_value = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (hitbit_value==NULL)
    {
        AMTRDRV_MGR_DBGMSG("Get Pdu fail.");
        return FALSE;
    }
    *hitbit_value = 0;
    DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(hitbit_value,addr->record.address.mac,addr->record.address.vid,unit,checking_port,port_counter);
    ISC_RemoteReply( mref_handle_p, key);
    return TRUE;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Remote_HandleModuleHotInsertion
 *------------------------------------------------------------------------------
 * PURPOSE: This is a remote function to program local module chip
 * INPUT  : ISC_Key_T *key    - key of isc
 *          AMTRDRV_MGR_ISCBuffer_T *request_p
 *                            - the buffer to hold request data from master
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : Module doesn't have om. When a module is insertion we have to
 *          program module chip right away.
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Remote_HandleModuleHotInsertion(ISC_Key_T *key, AMTRDRV_MGR_ISCBuffer_T *request_p)
{
    UI32_T  i;
    UI8_T   action;

    for(i=0;i<request_p->data.entries.number_of_entries;i++)
    {
        action = AMTRDRV_MGR_ACTION_ADD;
        /* reserved_state is not required on slave/module device, so just
         * specify 0.
         */
        AMTRDRV_MGR_SetChip(&request_p->data.entries.record[i],action, 0);
    }

    return TRUE;

}
#endif /* end of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE != TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_Service_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function will manipulte all of address in ARL via IMC
 * INPUT  : ISC_Key_T *key              - key of isc
 *          L_MM_Mref_Handle_T *mem_ref
 *          UI8_T svc_id                - serivce id
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTE   : callbacked by iscdrv
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_MGR_Service_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref, UI8_T svc_id)
{
    AMTRDRV_MGR_ISCBuffer_T     *buffer_p;
    UI32_T                      pdu_len;
    BOOL_T                      return_value;

    const static UI8_T   *func_names[]={
                 (UI8_T*)"AMTRDRV_MGR_SET_AGING_TIME",
                 (UI8_T*)"AMTRDRV_MGR_AGING_OUT",
                 (UI8_T*)"AMTRDRV_MGR_SET_ADDR_LIST",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_QUICKLY",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ALL_ADDR",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_LIFETIME_N_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_SOURCE_N_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME_N_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE_N_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_LIFETIME",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_SOURCE",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT",
                 (UI8_T*)"AMTRDRV_MGR_DELETE_ADDR_BY_VID_N_PORT_EXCEPT_CERTAIN_ADDR",
                 (UI8_T*)"AMTRDRV_MGR_CHANGE_PORT_LIFE_TIME",
                 (UI8_T*)"AMTRDRV_MGR_PROCESS_NEW_ADDR",
                 (UI8_T*)"AMTRDRV_MGR_MAX_NUM_OF_DIRECT_CALL_SERVICE"
            };
    buffer_p = L_MM_Mref_GetPdu(mem_ref, &pdu_len);

    if(buffer_p->service_id>=AMTRDRV_MGR_MAX_NUM_OF_DIRECT_CALL_SERVICE || amtrdrv_mgr_remote_service[buffer_p->service_id]==NULL)
    {
        L_MM_Mref_Release(&mem_ref);
        return TRUE;  /* We don't want the ISC re-send agian */
    }

    AMTRDRV_MGR_DBGMSG(func_names[buffer_p->service_id]);
    return_value = (amtrdrv_mgr_remote_service[buffer_p->service_id])(key, buffer_p);
    L_MM_Mref_Release(&mem_ref);
    return return_value;
}


BOOL_T AMTRDRV_MGR_SetInterfaceConfig(UI32_T unit,UI32_T port,UI32_T mode)
{
    return DEV_AMTRDRV_PMGR_SetInterfaceConfig(unit,port,mode);
}





UI32_T AMTRDRV_MGR_GetAmtrID()
{
     return AMTRDRV_OM_GetAmtrID();
}
void AMTRDRV_MGR_SetAmtrID(UI32_T tid)
{
     return AMTRDRV_OM_SetAmtrID(tid);
}

#endif /*SYS_CPNT_STACKING */


#ifdef BACKDOOR_OPEN

static void AMTRDRV_MGR_BD_GenerateAddress(AMTR_TYPE_AddrEntry_T *address_entry)
{
    UI8_T   ch;
    UI8_T   index;
    UI8_T   mac[12];

    amtrdrv_memset(address_entry, 0x0, sizeof(AMTR_TYPE_AddrEntry_T));

    /*vlan*/
    BACKDOOR_MGR_Printf("\n Set vlan[0~4095]:");
    address_entry->vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        address_entry->vid = address_entry->vid*10+ch;
    }
    BACKDOOR_MGR_Printf("\n");
    if(address_entry->vid > 4096)
    {
        BACKDOOR_MGR_Printf("vid too large. Do nothing!\n");
        return;
    }

    /*ifindex*/
    BACKDOOR_MGR_Printf("\n Set ifindex[0-300]:");
    address_entry->ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        address_entry->ifindex = address_entry->ifindex*10+ch;
    }
    BACKDOOR_MGR_Printf("\n");
    if(address_entry->ifindex > 300)
    {
        BACKDOOR_MGR_Printf("Ifindex too large. Do nothing!\n");
        return;
    }

    /*is static*/
    BACKDOOR_MGR_Printf("\n Static or not[Y]/[N]:");
    ch = BACKDOOR_MGR_GetChar();
    BACKDOOR_MGR_Printf("%c", ch);
    if ((ch != 'Y')&&(ch != 'y')&&(ch != 'N')&&(ch != 'n'))
    {
        BACKDOOR_MGR_Printf("\nYes or No?\n");
        return;
    }
    if((ch == 'Y')||(ch == 'y'))
    {
        address_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
    }
    else
    {
        address_entry->life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    }
    /*mac*/
    BACKDOOR_MGR_Printf("\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != '\r'))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        address_entry->mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
               address_entry->mac[0], address_entry->mac[1], address_entry->mac[2], address_entry->mac[3], address_entry->mac[4], address_entry->mac[5]);

    address_entry->source = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    address_entry->action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;

}

static BOOL_T AMTRDRV_MGR_BD_UnitExist(UI32_T unit)
{
    return STKTPLG_POM_UnitExist(unit);
}

static void AMTRDRV_MGR_BD_MsgLevel(void)
{
    char ch;
    for(;;)
    {
        BACKDOOR_MGR_Printf("\r\nAMTRDRV debug message level:\r\n");
        BACKDOOR_MGR_Printf("0. Exit\r\n");
        BACKDOOR_MGR_Printf("1. Debug Message: %s\r\n", (amtrdrv_mgr_debug_flag & AMTRDRV_MGR_DEBUG_DBGMSG)? "ON": "OFF");
        BACKDOOR_MGR_Printf("2. Show Address Content: %s\r\n", (amtrdrv_mgr_debug_flag & AMTRDRV_MGR_DEBUG_ADDR_CONTENT)? "ON": "OFF");
        BACKDOOR_MGR_Printf("Enter your choice: ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\r\n", ch);
        if ((ch < '0') || (ch > '2'))
        {
            BACKDOOR_MGR_Printf("Invalid selection, enter your choice agagin\r\n");
            continue;
        }

        if(ch == '0')
        {
            break;
        }

        amtrdrv_mgr_debug_flag ^= (0x01 << ((ch-'0') - 1));
    }
    return;
}

static void AMTRDRV_MGR_BD_SetAgingTime(void)
{
    UI32_T  aging_time;
    UI8_T   ch;

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("          Set Aging Time            \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\n How long you want to age the addr table entry out:");

    aging_time = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch>9)
        {
            continue;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        aging_time = aging_time*10+ch;
    }
    BACKDOOR_MGR_Printf("\n");
    AMTRDRV_MGR_SetAgingTime(aging_time);
}


static void AMTRDRV_MGR_BD_GetAgingTime(void)
{

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("          Get Aging Time            \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    BACKDOOR_MGR_Printf("The Aging out timer is : %ld\r\n",(long)AMTRDRV_OM_GetOperAgingTime());
    return;
}

static void AMTRDRV_MGR_BD_SetAddr(void)
{

    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("       Set addr table entry         \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    AMTRDRV_MGR_BD_GenerateAddress(&addr_entry);
    AMTRDRV_MGR_SetAddrList(1,&addr_entry);
    return;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
static void AMTRDRV_MGR_BD_DeleteAddr(void)
{
    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("     Delete addr table entry        \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    AMTRDRV_MGR_BD_GenerateAddress(&addr_entry);
    AMTRDRV_MGR_DeleteAddrEntryList(1,&addr_entry);
    return;
}
#endif /* SYS_CPNT_MAINBOARD */

static void AMTRDRV_MGR_BD_CreateMulticastAddr(void)
{
    UI8_T   ch;
    UI8_T   index;
    UI32_T  vid;
    UI8_T   mac[12];
    UI8_T   mac_addr[6];

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("    Create Multi addr table entry   \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\n Which unit you address:");

    /*vlan*/
    BACKDOOR_MGR_Printf("\n Set vlan[0~4095]:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }
    BACKDOOR_MGR_Printf("\n");
    if(vid > 4096)
    {
        BACKDOOR_MGR_Printf("vid too large. Do nothing!\n");
        return;
    }

    /*mac*/
    BACKDOOR_MGR_Printf("\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != '\r'))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac_addr[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    AMTRDRV_MGR_CreateMulticastAddr(vid, mac_addr);
    return;
}

static void AMTRDRV_MGR_BD_DestoryMulticastAddr(void)
{
    UI8_T   ch;
    UI8_T   index;
    UI32_T  vid;
    UI8_T   mac[12];
    UI8_T   mac_addr[6];

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("   Destory Multi addr table entry   \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /*vlan*/
    BACKDOOR_MGR_Printf("\n Set vlan[0~4095]:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }
    BACKDOOR_MGR_Printf("\n");
    if(vid > 4096)
    {
        BACKDOOR_MGR_Printf("vid too large. Do nothing!\n");
        return;
    }

    /*mac*/
    BACKDOOR_MGR_Printf("\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != '\r'))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac_addr[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    AMTRDRV_MGR_DestroyMulticastAddr(vid, mac_addr);
    return;
}

static void AMTRDRV_MGR_BD_ARLLookUp(void)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI8_T   ch;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;
    BOOL_T  is_trunk;


    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("        Address Look Up        \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\n Unit number:");

    /*unit*/
    ch = BACKDOOR_MGR_GetChar();
    ch -= '0';

    BACKDOOR_MGR_Printf ("%d\n",ch);
    if(!AMTRDRV_MGR_BD_UnitExist((UI32_T)ch))
    {
        BACKDOOR_MGR_Printf("The unit %d doesn't exist\n", ch);
        return;
    }
    unit = (UI32_T)ch;

    AMTRDRV_MGR_BD_GenerateAddress(&addr_entry);

    if(AMTRDRV_MGR_ARLLookUp(addr_entry.vid, addr_entry.mac, unit, &port, &trunk_id, &is_trunk))
    {
        BACKDOOR_MGR_Printf("Look up result: \n");
        BACKDOOR_MGR_Printf("port = %ld, trunk_id = %ld, is_trunk = %d\n", (long)port, (long)trunk_id, is_trunk);
    }
    else
    {
        BACKDOOR_MGR_Printf("Look up result: \n");
        BACKDOOR_MGR_Printf("Not found\n");
    }
        return;
}

#if (SYS_CPNT_MAINBOARD == TRUE)
static void AMTRDRV_MGR_BD_GetExactRecord(void)
{
    AMTRDRV_TYPE_Record_T  address_record;

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    BACKDOOR_MGR_Printf("   Get addr table entry from hash   \n");
    BACKDOOR_MGR_Printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");


    AMTRDRV_MGR_BD_GenerateAddress(&address_record.address);
    if(AMTRDRV_OM_GetExactRecord((UI8_T *)&address_record))
    {
        BACKDOOR_MGR_Printf("---The get entry---\r\n");
        BACKDOOR_MGR_Printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        address_record.address.mac[0], address_record.address.mac[1], address_record.address.mac[2], address_record.address.mac[3], address_record.address.mac[4], address_record.address.mac[5]);
        BACKDOOR_MGR_Printf("vid: %d\r\n",address_record.address.vid);
        BACKDOOR_MGR_Printf("ifindex: %d\r\n",address_record.address.ifindex);
        BACKDOOR_MGR_Printf("action: %d\r\n",address_record.address.action);
        BACKDOOR_MGR_Printf("source: %d\r\n",address_record.address.source);
        BACKDOOR_MGR_Printf("life_time: %d\r\n",address_record.address.life_time);
    }
    else
    {
        BACKDOOR_MGR_Printf("Address Entry not found!!\r\n");
    }
    return;

}


static void AMTRDRV_MGR_BD_ChecknClearHitBit(void)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T                checking_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI16_T                hitbit_value;

    AMTRDRV_MGR_BD_GenerateAddress(&addr_entry);

    if(IS_TRUNK(addr_entry.ifindex))
    {
        BACKDOOR_MGR_Printf(" Please try to test none trunk port \r\n");
        return;
    }

    checking_port[0] = STKTPLG_POM_IFINDEX_TO_PORT(addr_entry.ifindex);

    if(DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit((UI8_T *)&hitbit_value, addr_entry.mac,addr_entry.vid,1,checking_port,1))
    {
        BACKDOOR_MGR_Printf("OUTPUT:The hitbit value is %d\r\n",hitbit_value);
    }
    else
    {
        BACKDOOR_MGR_Printf("OUTPUT:Read Fail!\r\n");
    }
    return;
}

static void AMTRDRV_MGR_BD_DumpAllRecords(void)
{
    BACKDOOR_MGR_Printf("\r\n=====Address Record Entries in Addr Hash Table======\r\n");
    AMTRDRV_OM_ShowAddrInfo();
    return;
}

static void AMTRDRV_MGR_BD_DumpRecordsInQueryGroup(void)
{
    L_DLST_ShMem_Indexed_Dblist_T *dblist;
    AMTRDRV_TYPE_Record_T     address_record;
    UI32_T query_group=0;
    UI32_T element=0;
    UI32_T index;
    UI8_T  ch,ch1,action;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\r\n==========Guery Method================\r\n");
    BACKDOOR_MGR_Printf(" 1. QUERY_BY_LIFE_TIME\r\n");
    BACKDOOR_MGR_Printf(" 2. QUERY_BY_IFINDEX \r\n");
    BACKDOOR_MGR_Printf(" 3. QUERY_BY_VID\r\n");
    BACKDOOR_MGR_Printf(" 4. QUERY_BY_SOURCE\r\n");
    BACKDOOR_MGR_Printf("=================================================\r\n");
    BACKDOOR_MGR_Printf("     select =");
    ch = BACKDOOR_MGR_GetChar();

    BACKDOOR_MGR_Printf ("%c",ch);

    switch(ch)
    {
        case '1' :
            query_group = AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME;
            BACKDOOR_MGR_Printf("\r\n==========Life Time element================\r\n");
            BACKDOOR_MGR_Printf(" 1. OTHER \r\n");
            BACKDOOR_MGR_Printf(" 2. INVALID \r\n");
            BACKDOOR_MGR_Printf(" 3. PERMANENT \r\n");
            BACKDOOR_MGR_Printf(" 4. DELETE_ON_RESET ");
            BACKDOOR_MGR_Printf(" 5. DELETE_ON_TIMEOUT \r\n");
            BACKDOOR_MGR_Printf("=================================================\r\n");
            BACKDOOR_MGR_Printf("     select =");

            ch1 = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf ("%c",ch1);

            switch(ch1)
            {
                case '1':
                    element = AMTR_TYPE_ADDRESS_LIFETIME_OTHER;
                    break;
                case '2':
                    element = AMTR_TYPE_ADDRESS_LIFETIME_INVALID;
                    break;
                case '3':
                    element = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
                    break;
                case '4':
                    element = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
                    break;
                case '5':
                    element = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
                    break;
                default :
                    BACKDOOR_MGR_Printf(" Your choice is out of range. Please retry !!! \r\n");
                    return;
            }
            break;
        case '2' :
            query_group = AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX;
            BACKDOOR_MGR_Printf("\r\n Set ifindex:");
            while((ch=BACKDOOR_MGR_GetChar()) != '\r')
            {
                ch -= '0';
                if(ch>9)
                {
                    BACKDOOR_MGR_Printf("Erro in Input\n");
                    return;
                }
                BACKDOOR_MGR_Printf("%d", ch);
                element = element*10+ch;
            }
            BACKDOOR_MGR_Printf("\n");
            if(element >= AMTR_TYPE_MAX_LOGICAL_PORT_ID)
            {
                BACKDOOR_MGR_Printf("Ifindex too large. Do nothing!\n");
                return;
            }
            break;
        case '3' :
            query_group = AMTRDRV_OM_QUERY_GROUP_BY_VID;
            BACKDOOR_MGR_Printf("\r\n Set Vlan(1~4093):");
            while((ch=BACKDOOR_MGR_GetChar()) != '\r')
            {
                ch -= '0';

                if(ch>9)
                {
                    BACKDOOR_MGR_Printf("Erro in Input\n");
                    return;
                }
                BACKDOOR_MGR_Printf("%d", ch);
                element = element*10+ch;
            }
            BACKDOOR_MGR_Printf("\n");
            if(element > 4093)
            {
                BACKDOOR_MGR_Printf("Vlan too large. Do nothing!\n");
                return;
            }
            element = element -1;
            break;
        case '4' :
            query_group = AMTRDRV_OM_QUERY_GROUP_BY_SOURCE;
            BACKDOOR_MGR_Printf("\r\n==========Source element================\r\n");
            BACKDOOR_MGR_Printf(" 1. INTERNAL \r\n");
            BACKDOOR_MGR_Printf(" 2. INVALID \r\n");
            BACKDOOR_MGR_Printf(" 3. LEARN \r\n");
            BACKDOOR_MGR_Printf(" 4. SELF \r\n");
            BACKDOOR_MGR_Printf(" 5. CONFIG \r\n");
            BACKDOOR_MGR_Printf("=================================================\r\n");
            BACKDOOR_MGR_Printf("     select =");
            ch1 = BACKDOOR_MGR_GetChar();

            BACKDOOR_MGR_Printf ("%c",ch1);

            switch(ch1)
            {
                case '1':
                    element = AMTR_TYPE_ADDRESS_SOURCE_INTERNAL;
                    break;
                case '2':
                    element = AMTR_TYPE_ADDRESS_SOURCE_INVALID;
                    break;
                case '3':
                    element = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
                    break;
                case '4':
                    element = AMTR_TYPE_ADDRESS_SOURCE_SELF;
                    break;
                case '5':
                    element = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
                    break;
                default :
                    BACKDOOR_MGR_Printf(" Your choice is out of range. Please retry !!! \r\n");
                    return;
            }
            break;
        default  :
            BACKDOOR_MGR_Printf(" Your choice is out of range. Please retry !!! \r\n");
            return;
    }
    BACKDOOR_MGR_Printf("\r\n=====Address Record Entries in Specified Query Group======\r\n");
    if (query_group == AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX)
    {
        ret = AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(query_group,element,&dblist, &index,(UI8_T *)&address_record, &action);
    }
    else
    {
    ret = AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(query_group,element-1,&dblist, &index,(UI8_T *)&address_record, &action);
    }

    while(ret)
    {
        BACKDOOR_MGR_Printf("  Index:%lu", (unsigned long)index);
        BACKDOOR_MGR_Printf("  Mac:%02x-%02x-%02x-%02x-%02x-%02x",
            address_record.address.mac[0], address_record.address.mac[1], address_record.address.mac[2], address_record.address.mac[3], address_record.address.mac[4], address_record.address.mac[5]);
        BACKDOOR_MGR_Printf("  Vid:%d",address_record.address.vid);
        BACKDOOR_MGR_Printf("  Ifindex:%d",address_record.address.ifindex);
        BACKDOOR_MGR_Printf("  Source:%d",address_record.address.source);
        BACKDOOR_MGR_Printf("  Life_time:%d",address_record.address.life_time);
        BACKDOOR_MGR_Printf("  Record_action(set/delete):%d\r\n",action);
        ret = AMTRDRV_OM_GetNextEntryFromGivingQueryGroup(dblist, &index,(UI8_T *)&address_record,&action);
    }
    return;

}

static void AMTRDRV_MGR_BD_DumpAgingOutBuffer()
{
    AMTRDRV_OM_ShowRecordsInfoInAgingOutBuffer();
    return;
}

static void AMTRDRV_MGR_BD_DumpRecordInLocalCheckingList()
{
    AMTRDRV_OM_ShowRecordInfoInCheckingAgingList();
    return;
}

static void AMTRDRV_MGR_BD_Dump_NA_Hash_Counter(void)
{
    BACKDOOR_MGR_Printf("\r\n Number of NA Announce Times : %ld \r\n", (long)amtrdrv_mgr_num_of_na_announce);
    return;
}
static void AMTRDRV_MGR_BD_AddressTaskPerformanceTest()
{

    UI8_T         ch;

    if(amtrdrv_mgr_bd_address_task_performance != FALSE)
    {
        BACKDOOR_MGR_Printf("Please wait for the lated testing finished \r\n");
        return;
    }

    amtrdrv_mgr_address_task_testing_times = 0;
    amtrdrv_mgr_testing_na_time = 0;
    amtrdrv_mgr_testing_agingcheck_time = 0;
    amtrdrv_mgr_testing_agingout_time = 0;
    amtrdrv_mgr_testing_na_counter = 0;
    amtrdrv_mgr_testing_agingchecking_counter = 0;
    amtrdrv_mgr_testing_agingout_counter = 0;

    BACKDOOR_MGR_Printf("\r\nEnter how many times you want to testing :\r\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_address_task_testing_times = amtrdrv_mgr_address_task_testing_times*10+ch;
    }
    amtrdrv_mgr_number_of_address_task_testing_times = amtrdrv_mgr_address_task_testing_times;
    amtrdrv_mgr_bd_address_task_performance = TRUE;


}

static void AMTRDRV_MGR_BD_AsicTaskPerformanceTest()
{

    UI8_T         ch;

    if(amtrdrv_mgr_bd_asic_task_performance != FALSE)
    {
        BACKDOOR_MGR_Printf("Please wait for the lated testing finished \r\n");
        return;
    }

    amtrdrv_mgr_asic_task_testing_times = 0;
    amtrdrv_mgr_testing_program_chip_time = 0;
    amtrdrv_mgr_testing_program_chip_counter = 0;
    BACKDOOR_MGR_Printf("\r\nEnter how many times you want to testing :\r\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= '0';
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_asic_task_testing_times = amtrdrv_mgr_asic_task_testing_times*10+ch;
    }
    amtrdrv_mgr_number_of_asic_task_testing_times = amtrdrv_mgr_asic_task_testing_times;
    amtrdrv_mgr_bd_asic_task_performance = TRUE;

}

static void AMTRDRV_MGR_BD_PureProgramChipPerformanceTest()
{
    AMTRDRV_TYPE_Record_T           address_record;
    SWDRV_TYPE_L2_Address_Info_T  arl_entry;
    UI32_T                        i=0;
    UI32_T                        t1,t2,t3;
    UI32_T                        counter=0;
    BOOL_T                        result;

    address_record.address.vid = 1;
    address_record.address.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    address_record.address.ifindex = 1;
    address_record.address.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    address_record.address.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
    address_record.address.priority = AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;
    amtrdrv_memcpy(address_record.address.mac,null_mac,AMTR_TYPE_MAC_LEN);
    t1 = t2 = t3 =0;
    /*kh_shi SYSFUN_NonPreempty();*/
    for(i=0;i<AMTRDRV_MGR_MAX_NUM_JOB_PROCESS;i++)
    {
        address_record.address.mac[5]+=1;
        /* 1. Converting AMTRDRV_OM format to SWDRV_TYPE_L2_Address_Info_T format */
        AMTRDRV_MGR_ConvertToSWDRVL2AddrEntry(&(address_record.address), &arl_entry);

        t1 = SYSFUN_GetSysTick();
        result = DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (DEV_AMTRDRV_ALL_DEVICE,&arl_entry);
        t2 = SYSFUN_GetSysTick();
        t3 += t2 - t1;

        if(result == DEV_AMTRDRV_SUCCESS)
        {
            counter ++;
        }
    }
    BACKDOOR_MGR_Printf(" \r\n To program %ld macs into chip needs : %ld ticks \r\n",(long)counter,(long)t3);
    /* kh_shi SYSFUN_Preempty();*/
    return;
}

static void AMTRDRV_MGR_BD_ShowTotalCounters()
{
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf(" \r\n      Show Total Counters         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Total Counter        : %ld",(long)AMTRDRV_OM_GetTotalCounter());
    BACKDOOR_MGR_Printf("\r\n Total Static Counter : %ld",(long)AMTRDRV_OM_GetTotalStaticCounter());
    BACKDOOR_MGR_Printf("\r\n Total Dynamic Counter: %ld",(long)AMTRDRV_OM_GetTotalDynamicCounter());
    return;
}

static void AMTRDRV_MGR_BD_ShowByPortCounters()
{
    UI32_T ifindex;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Show By Port Counters         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }
    BACKDOOR_MGR_Printf("\r\n Static    Counter on [%ld]:  %ld",(long)ifindex,(long)AMTRDRV_OM_GetStaticCounterByPort(ifindex));
    BACKDOOR_MGR_Printf("\r\n Dynamic   Counter on [%ld]:  %ld",(long)ifindex,(long)AMTRDRV_OM_GetDynCounterByPort(ifindex));
    BACKDOOR_MGR_Printf("\r\n Learnt    Counter on [%ld]:  %ld",(long)ifindex,(long)AMTRDRV_OM_GetLearntCounterByport(ifindex));
    BACKDOOR_MGR_Printf("\r\n Config    Counter on [%ld]:  %ld",(long)ifindex,(long)AMTRDRV_OM_GetConfigCounterByPort(ifindex));
}

static void AMTRDRV_MGR_BD_ShowByVidCounter()
{
    UI32_T vid;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Show By Vid counter         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }
    BACKDOOR_MGR_Printf("\r\n Dynamic  Counter on [%ld]:  %ld",(long)vid,(long)AMTRDRV_OM_GetDynCounterByVid(vid));
}
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)

void AMTRDRV_MGR_BD_ENHW_performance()
{
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n~~~~~~(1:enable 2:disable)~~~~~~~~~~~~~~~~");
    /*init the value*/
    BACKDOOR_MGR_Printf("\r\n current amtrdrv_mgr_enable_hw_on_sw:%d\n",amtrdrv_mgr_enable_hw_on_sw);
    BACKDOOR_MGR_Printf("Input:");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch == 1 ||ch == 2)
            break;
        BACKDOOR_MGR_Printf("%i",ch);
    }
    amtrdrv_mgr_enable_hw_on_sw = (ch == 1)? TRUE : FALSE;
    return;
}

void AMTRDRV_MGR_BD_Dequeueperformance(){
        UI8_T ch;
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        BACKDOOR_MGR_Printf("\r\n~~~~~~(1:onebyon 2:battach)~~~~~~~~~~~~~~~~");
    /*init    the value*/
        BACKDOOR_MGR_Printf("\r\n current amtrdrv_mgr_test_dequeue_method:%ld\n", (long)amtrdrv_mgr_test_dequeue_method);
        BACKDOOR_MGR_Printf("Input:");
        while((ch=BACKDOOR_MGR_GetChar()) != '\r')
        {
            ch -= 0x30;
            if(ch == 1 ||ch == 2)
                break;
            BACKDOOR_MGR_Printf("%i",ch);
        }
        amtrdrv_mgr_test_dequeue_method = (ch == 1)? 0 : 1;
        return ;
}
void AMTRDRV_MGR_BD_RegisterNACallback()
{
        UI8_T ch;
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        BACKDOOR_MGR_Printf("\r\n~~~~~~(1:unregister 2:register)~~~~~~~~~~~~~~~~");
        BACKDOOR_MGR_Printf("Input:");
        while((ch=BACKDOOR_MGR_GetChar()) != '\r')
        {
            ch -= 0x30;
            if(ch == 1 ||ch == 2 || ch == 3)
                break;
            BACKDOOR_MGR_Printf("error input:%i\n:",ch);
        }
        BACKDOOR_MGR_Printf("%i",ch);
        if(ch == 1){
          BACKDOOR_MGR_Printf("unregister\n");
          amtrdrv_mgr_test_register_action = 1;
        }else if(ch == 2){
            BACKDOOR_MGR_Printf("register\n");
            amtrdrv_mgr_test_register_action = 2;
        }

        return ;
}

void AMTRDRV_MGR_BD_periodic_performance()
{
    UI32_T temp1 = 0;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       debug performance , this function is to modify the periodic timer\n     ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~(the range is (0-100))~~~~~~~~~~~~~~~~");
/*init  the value*/
    BACKDOOR_MGR_Printf("\r\n current amtr_update_addr_table_ticks:%ld\n",(long)amtr_update_addr_table_ticks);
    BACKDOOR_MGR_Printf("Input:");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 9){
            BACKDOOR_MGR_Printf("Erro in Input :%i\n",ch);
            break;
        }
        BACKDOOR_MGR_Printf("%i",ch);
        temp1 = temp1*10 + ch;
    }
    BACKDOOR_MGR_Printf("the ticks %ld\n", (long)temp1);
    if((temp1 <=100)&&(temp1 >0)){
      amtr_update_addr_table_ticks = temp1;
      return;
    }
    BACKDOOR_MGR_Printf("Erro in Input ticks :%ld\n", (long)temp1);
    return ;
}

void AMTRDRV_MGR_BD_asic_performanceCounter ()
{
    UI32_T counter = 0;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       debug asic performance\n     ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
/*init  the value*/
    amtrdrv_mgr_test_master_counter_asic = amtrdrv_mgr_test_master_counter_asic_ok = amtrdrv_mgr_test_master_counter_asic_dequeue =0;
    BACKDOOR_MGR_Printf("\r\n current amtrdrv_mgr_test_master_totol_counter_asic:%ld\n",(long)amtrdrv_mgr_test_master_totol_counter_asic);
    BACKDOOR_MGR_Printf("\r\n  set 1-9(1-9k),a:31K,b:15K,c:10k\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        switch(ch){
            case '1':
                counter =1;
                break;
            case '2':
                counter =2;
                break;
            case '3':
                counter =3;
                break;
            case '4':
                counter =4;
                break;
            case '5':
                counter =5;
                break;
            case '6':
                counter =6;
                break;
            case '7':
                counter =7;
                break;
            case '8':
                counter =8;
                break;
            case '9':
                counter =9;
                break;
            case 'a':
                counter =31;
                break;
            case 'b':
                counter =15;
                break;
            case 'c':
                counter =10;
                break;
            default:
                continue;
        }
        break;
    }
    amtrdrv_mgr_test_master_totol_counter_asic =counter*1024;
    BACKDOOR_MGR_Printf("AMTRDRV_OM_MAX_ENTRIES_IN_ONE_PACKET %d\n",(int)AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET);

reset_value:
    BACKDOOR_MGR_Printf("\r\n current value:%ld\n", (long)amtrdrv_mgr_test_performance);

    BACKDOOR_MGR_Printf("\r\nset 0: disable \n1 : master \n2: slave \n3: master and slave\n4:asic\n5:ok asic\n");
    BACKDOOR_MGR_Printf("Input:");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 5)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            goto reset_value;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_test_performance = ch;
        break;
    }

}

void AMTRDRV_MGR_BD_master_performanceCounter ()
{
    UI32_T counter = 0;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       debug master performance\n     ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
/*init  the value*/
    BACKDOOR_MGR_Printf("\r\n current amtrdrv_mgr_test_master_totol_counter:%ld,enqueu failed %ld\n",(long)amtrdrv_mgr_test_master_totol_counter,(long)amtrdrv_mgr_test_enqueue_failed_counter);
    amtrdrv_mgr_test_master_counter = amtrdrv_mgr_test_enqueue_failed_counter = 0;
    BACKDOOR_MGR_Printf("\r\n  set 1-9(1-9k),a:32K,b:15K,c:10k\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        switch(ch){
            case '1':
                counter =1;
                break;
            case '2':
                counter =2;
                break;
            case '3':
                counter =3;
                break;
            case '4':
                counter =4;
                break;
            case '5':
                counter =5;
                break;
            case '6':
                counter =6;
                break;
            case '7':
                counter =7;
                break;
            case '8':
                counter =8;
                break;
            case '9':
                counter =9;
                break;
            case 'a':
                counter =32;
                break;
            case 'b':
                counter =15;
                break;
            case 'c':
                counter =10;
                break;
            default:
                continue;
        }
        break;
    }
    amtrdrv_mgr_test_master_totol_counter =counter*1024;


reset_value:
    BACKDOOR_MGR_Printf("\r\n current value:%ld\n", (long)amtrdrv_mgr_test_performance);

    BACKDOOR_MGR_Printf("\r\nset 0: disable \n1 : master \n2: slave \n3: master and slave 6:test dequeue rate\n");
    BACKDOOR_MGR_Printf("Input:");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            goto reset_value;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_test_performance = ch;
        break;
    }

}
void AMTRDRV_MGR_BD_slave_performanceCounter ()
{
    UI32_T counter = 0;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       debug slave performance\n     ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
/*init  the value*/
    amtrdrv_mgr_test_slave_counter = 0;
    BACKDOOR_MGR_Printf("\r\n current amtrdrv_mgr_test_slave_totol_counter:%ld\n", (long)amtrdrv_mgr_test_slave_totol_counter);
    BACKDOOR_MGR_Printf("\r\n  set 1-9(1-9k),a:32K,b:15K,c:10k\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        switch(ch){
            case '1':
                counter =1;
                break;
            case '2':
                counter =2;
                break;
            case '3':
                counter =3;
                break;
            case '4':
                counter =4;
                break;
            case '5':
                counter =5;
                break;
            case '6':
                counter =6;
                break;
            case '7':
                counter =7;
                break;
            case '8':
                counter =8;
                break;
            case '9':
                counter =9;
                break;
            case 'a':
                counter =32;
                break;
            case 'b':
                counter =15;
                break;
            case 'c':
                counter =10;
                break;
            default:
                continue;
        }
        break;
    }
    amtrdrv_mgr_test_slave_totol_counter =counter*1024;


reset_value:
    BACKDOOR_MGR_Printf("\r\n current value:%ld\n", (long)amtrdrv_mgr_test_performance);

    BACKDOOR_MGR_Printf("\r\nset 0: disable \n1 : master \n2: slave \n3: master and slave\n");
    BACKDOOR_MGR_Printf("Input:");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 3)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            goto reset_value;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_test_performance = ch;
        break;
    }

}

#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
void AMTRDRV_MGR_GetMemActionCounter(){
     char p;
     if((p = BACKDOOR_MGR_GetChar())!='\0'){
        if(p- '0' > 9 || p-'0' <1){
            BACKDOOR_MGR_Printf("value %c is invalid\n",p);
            return ;
        }
        BACKDOOR_MGR_Printf("%c",p);
        if(p - '0' == 1){
            BACKDOOR_MGR_Printf("reset the counter to 0\n");
            memcpy_counter = 0;
            memset_counter = 0;
            memcmp_counter = 0;
        }
        if(p - '0' == 2){
            BACKDOOR_MGR_Printf("the memcpy_counter %ld,memset_counter %ld,memcmp_counter %ld ,memcpy_time_spend%ld\n",(long)memcpy_counter,(long)memset_counter,(long)memcmp_counter,(long)memcpy_time_spend);
        }

     }


}

#endif
static void AMTRDRV_MGR_BD_IntergrationPerformanceTesting()
{
    UI8_T ch;
    if (TRUE == amtrdrv_mgr_integration_testing_flag)
    {
        amtrdrv_mgr_integration_testing_flag = FALSE;
        return;
    }
    else
    {
        amtrdrv_mgr_integration_testing_num_of_entry = 0;
    }

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~ Intergrating performance testing~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\nUser input X to identify how many entries will be detected by AMTR.");
    BACKDOOR_MGR_Printf("\r\nThen user start to inject X NA by IXIA.");
    BACKDOOR_MGR_Printf("\r\nWhen AMTR have learnt X entries, it will BACKDOOR_MGR_Printf total and average spending time.");
    BACKDOOR_MGR_Printf("\r\nWhen learning is finished, please stop injecting.");
    BACKDOOR_MGR_Printf("\r\nAMTR will detect total and average spending age out time until all entries are aged.");
    BACKDOOR_MGR_Printf("\r\nnotes: 1. The DUT doesn't connect any device beside IXIA.");
    BACKDOOR_MGR_Printf("\r\n       2. AMTR will finish NA Learning then age out.");
    BACKDOOR_MGR_Printf("\r\n X = ");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        amtrdrv_mgr_integration_testing_num_of_entry = amtrdrv_mgr_integration_testing_num_of_entry*10+ch;
    }
    if (amtrdrv_mgr_integration_testing_num_of_entry == 0)
        BACKDOOR_MGR_Printf("\r\n Error Input");
    amtrdrv_mgr_integration_testing_flag = TRUE;
    amtrdrv_mgr_integration_tesing_na_start_tick=0;
    amtrdrv_mgr_integration_testing_printf_first_age_out_entry = TRUE;
}
static void AMTRDRV_MGR_BD_ShowSlaveRepeatCounter()
{
#if 0 /* JinhuaWei, 05 August, 2008 4:20:31 */
    UI32_T vid;
    UI8_T ch;
#endif /* #if 0 */
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Show By amtrdrv_mgr_narestore_counters        ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n amtrdrv_mgr_narestore_counters  %ld\n", (long)amtrdrv_mgr_narestore_counters);
    amtrdrv_mgr_narestore_counters =0 ;
}

#endif /* SYS_CPNT_MAINBOARD */
#ifdef  AMTR_TYPE_DRV_SLAVE_ISC_TEST
static void AMTRDRV_MGR_BD_IscTest()
{
#if 0 /* JinhuaWei, 05 August, 2008 4:21:03 */
    UI32_T vid;
#endif /* #if 0 */
    UI8_T ch;
    int counter = 0;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       change the value of amtrdrv_mgr_isc_test ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n amtrdrv_mgr_isc_test  %ld\n", (long)amtrdrv_mgr_isc_test);

    BACKDOOR_MGR_Printf("\r\n  set 1-9(1-9k),a:32K,b:15K,c:10k\n");
    while((ch=BACKDOOR_MGR_GetChar()) != '\r')
    {
        switch(ch){
            case '1':
                counter =1;
                break;
            case '2':
                counter =2;
                break;
            case '3':
                counter =3;
                break;
            case '4':
                counter =4;
                break;
            case '5':
                counter =5;
                break;
            case '6':
                counter =6;
                break;
            case '7':
                counter =7;
                break;
            case '8':
                counter =8;
                break;
            case '9':
                counter =9;
                break;
            case 'a':
                counter =32;
                break;
            case 'b':
                counter =15;
                break;
            case 'c':
                counter =10;
                break;
            default:
                continue;
        }
        break;
    }
    amtrdrv_mgr_test_slave_totol_counter =counter*1024;

    amtrdrv_mgr_test_slave_counter = 0;

    BACKDOOR_MGR_Printf("the total %ld\n",(long)amtrdrv_mgr_test_slave_totol_counter);

    if(amtrdrv_mgr_isc_test == 1)
        amtrdrv_mgr_isc_test = 0;
    else
        amtrdrv_mgr_isc_test = 1;

}
static void AMTRDRV_MGR_BD_IscTest_returndonothing()
{
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       change the value of amtrdrv_mgr_isc_test_returndonothing ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n amtrdrv_mgr_isc_test_returndonothing%ld\n",(long)amtrdrv_mgr_isc_test_returndonothing);

    if(amtrdrv_mgr_isc_test_returndonothing == 0)
        amtrdrv_mgr_isc_test_returndonothing = 1;
    else
        amtrdrv_mgr_isc_test_returndonothing = 0;
    BACKDOOR_MGR_Printf("the amtrdrv_mgr_isc_test_returndonothing %ld\n",(long)amtrdrv_mgr_isc_test_returndonothing);

}
static void AMTRDRV_MGR_BD_IscTest_returndonothing1()
{
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       return in slave at AMTRDRV_MGR_SetAddrList2LocalUnit ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n amtrdrv_mgr_isc_test_returndonothing1%ld\n",(long)amtrdrv_mgr_isc_test_returndonothing1);

    if(amtrdrv_mgr_isc_test_returndonothing1 == 1)
        amtrdrv_mgr_isc_test_returndonothing1 = 0;
    else
        amtrdrv_mgr_isc_test_returndonothing1 = 1;
    BACKDOOR_MGR_Printf("the amtrdrv_mgr_isc_test_returndonothing1 %ld\n",(long)amtrdrv_mgr_isc_test_returndonothing1);

}
static void AMTRDRV_MGR_BD_IscTest_returndonothing2()
{
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       return in slave at AMTRDRV_MGR_SetAddrList2LocalUnit() ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n amtrdrv_mgr_isc_test_returndonothing2%ld\n",(long)amtrdrv_mgr_isc_test_returndonothing1);

    if(amtrdrv_mgr_isc_test_returndonothing1 == 2)
        amtrdrv_mgr_isc_test_returndonothing1 = 0;
    else
        amtrdrv_mgr_isc_test_returndonothing1 = 2;
    BACKDOOR_MGR_Printf("the amtrdrv_mgr_isc_test_returndonothing2 %ld\n", (long)amtrdrv_mgr_isc_test_returndonothing1);

}

#endif

static void AMTRDRV_MGR_BD_SynQDebugBackdoor(void)
{
    int  ch;
    char buf[32];
    BOOL_T is_exit = FALSE;

    while(is_exit==FALSE)
    {
        BACKDOOR_MGR_Printf("1. Set synq debug info display(%02X)\r\n", (int)(AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo()));
        BACKDOOR_MGR_Printf("2. Dump synq\r\n");
        BACKDOOR_MGR_Printf("3. Dump synq lport event\r\n");
        BACKDOOR_MGR_Printf("q. exit\r\n");

        ch=BACKDOOR_MGR_GetChar();
        if(ch==EOF)
            continue;

        buf[0] = (char)ch;

        switch(buf[0])
        {
            case '1':
            {
                char  *end;
                UI8_T local_synq_debug_flag;

                BACKDOOR_MGR_Print("Debug flag(Hex)=\n");
                BACKDOOR_MGR_RequestKeyIn(buf, 2);
                local_synq_debug_flag = strtoul(buf, &end, 16);
                AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo(local_synq_debug_flag);
            }
                break;
            case '2':
                AMTRDRV_OM_BACKDOOR_DumpSynQ();
                break;
            case '3':
                AMTRDRV_OM_BACKDOOR_DumpSynqPortEventId();
                break;
            case 'q':
                is_exit=TRUE;
                break;
            default:
                break;
        }
    }
}

static void AMTRDRV_MGR_BackDoor_Menu (void)
{
    UI8_T   ch;
    BOOL_T  eof=FALSE;
    /*  BODY
     */
    while (!eof)
    {
        BACKDOOR_MGR_Printf("\r\n==========AMTRDRV BackDoor Menu================\r\n");
        BACKDOOR_MGR_Printf("\r\n 0. Exit\r\n");
        BACKDOOR_MGR_Printf(" 1. Set debug message level         2. Set aging time\r\n");
        BACKDOOR_MGR_Printf(" 3. Get aging time                  4. Set addr table entry\r\n");
        BACKDOOR_MGR_Printf(" 5. Create multicast addr           6. Destory multicast addr\r\n");
        BACKDOOR_MGR_Printf(" 7. Addr look up                    8. Toggle display");
        if(amtrdrv_mgr_bd_display) BACKDOOR_MGR_Printf("(ON):\r\n"); else BACKDOOR_MGR_Printf("(OFF):\r\n");
#if (SYS_CPNT_MAINBOARD == TRUE)
        BACKDOOR_MGR_Printf(" 9. Delete addr from table entry    a. Get Record from Hash table\r\n");
        BACKDOOR_MGR_Printf(" b. Check and Clear Hit Bit         c. Dump all records in hash table\r\n");
        BACKDOOR_MGR_Printf(" d. Dump records in specified query group \r\n");
        BACKDOOR_MGR_Printf(" e. Dump records in local checking aging list \r\n");
        BACKDOOR_MGR_Printf(" f. Dump records in Aging out Buffer \r\n");
        BACKDOOR_MGR_Printf(" g. Dump NA counter \r\n");
        BACKDOOR_MGR_Printf(" h. Clear & hit bit performance testing \r\n");
        BACKDOOR_MGR_Printf(" i. AMTRDRV_Addr_TASK performance testing \r\n");
        BACKDOOR_MGR_Printf(" j. AMTRDRV_Asic_TASK_performance testing \r\n");
        BACKDOOR_MGR_Printf(" k. AMTRDRV_PUREPROGRAMCHIP_PERFORMANCE TESTING \r\n");
        BACKDOOR_MGR_Printf(" l. Show Total Counters                  \r\n");
        BACKDOOR_MGR_Printf(" m. Show By port Counters                \r\n");
        BACKDOOR_MGR_Printf(" n. Show By Vid Counters                 \r\n");
        BACKDOOR_MGR_Printf(" o. Show collision entries");
        if(amtrdrv_mgr_debug_show_collision_flag) BACKDOOR_MGR_Printf("(YES):\r\n"); else BACKDOOR_MGR_Printf("(NO):\r\n");
        BACKDOOR_MGR_Printf(" p. Intergration performance testing ");
        if(amtrdrv_mgr_integration_testing_flag) BACKDOOR_MGR_Printf("(ON):\r\n"); else BACKDOOR_MGR_Printf("(OFF):\r\n");
        BACKDOOR_MGR_Printf(" q. set the chip for S/W learning \n\r");
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
/*Tony.Lei
 * add these for testing performance
 */
        BACKDOOR_MGR_Printf("r: test the rate of the MAC enqueuing on master(set 1: rate of enqueue 6: rate of dequeue,7.trace packet)\n\r");
        BACKDOOR_MGR_Printf("s: test the rate of the MAC enqueuing on slave\n\r");
        BACKDOOR_MGR_Printf("t: test the speed of the setting mac to asic\n\r");
        BACKDOOR_MGR_Printf("u: change the periodic of the MAC timer\n\r");
        BACKDOOR_MGR_Printf("v: set the enqueuing MAC to chip temply \n\r");
        BACKDOOR_MGR_Printf("w: choose the dequeue method \n\r");
        BACKDOOR_MGR_Printf("y: unregister AMTRDRV_MGR_AnnounceNA_N_SecurityCheck to test NIC performance\n");
/*end Tony.Lei*/
#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
        BACKDOOR_MGR_Printf("x: 1 : reset the memset memcpy counter. 2: show the counter \n\r");
#endif
#if defined(AMTRDRV_SLAVE_REPEATING_TEST) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        BACKDOOR_MGR_Printf("?: show the repeating counter in the slave \n\r");
#endif
#ifdef AMTR_TYPE_DRV_SLAVE_ISC_TEST
       BACKDOOR_MGR_Printf("':': Test the ISC rate.if value is 1, the callback is return imed \n\r");
       BACKDOOR_MGR_Printf("'!': when the value 1 : will return immediately \n\r");
       BACKDOOR_MGR_Printf("'#':  when set 1, in the function :AMTRDRV_MGR_SetAddrList2LocalUnit\n");
       BACKDOOR_MGR_Printf("'$':  when set 2, in the function :AMTRDRV_MGR_SetAddrList2LocalUnit(to test the stack frame)\n");
#endif
#endif  /* SYS_CPNT_MAINBOARD */
        BACKDOOR_MGR_Printf("'@': SynQ debug backdoor \r\n");

        BACKDOOR_MGR_Printf("=================================================\r\n");
        BACKDOOR_MGR_Printf("     select =");
        ch = BACKDOOR_MGR_GetChar();

        BACKDOOR_MGR_Printf ("%c",ch);
        switch (ch)
        {
            case '@' :
                AMTRDRV_MGR_BD_SynQDebugBackdoor();
                break;
            case '0' :
                eof = TRUE;
                break;
            case '1':
                AMTRDRV_MGR_BD_MsgLevel();
                break;
            case '2':
                AMTRDRV_MGR_BD_SetAgingTime();
                break;
            case '3':
                AMTRDRV_MGR_BD_GetAgingTime();
                break;
            case '4':
                AMTRDRV_MGR_BD_SetAddr();
                break;
            case '5':
                AMTRDRV_MGR_BD_CreateMulticastAddr();
                break;
            case '6':
                AMTRDRV_MGR_BD_DestoryMulticastAddr();
                break;
            case '7':
                AMTRDRV_MGR_BD_ARLLookUp();
                break;
            case '8':
                amtrdrv_mgr_bd_display = !amtrdrv_mgr_bd_display;
                break;
#if (SYS_CPNT_MAINBOARD == TRUE)
            case '9':
                AMTRDRV_MGR_BD_DeleteAddr();
                break;
            case 'a':
            case 'A':
                AMTRDRV_MGR_BD_GetExactRecord();
                break;
            case 'b':
            case 'B':
                AMTRDRV_MGR_BD_ChecknClearHitBit();
                break;
            case 'c':
            case 'C':
                AMTRDRV_MGR_BD_DumpAllRecords();
                break;
            case 'd':
            case 'D':
                AMTRDRV_MGR_BD_DumpRecordsInQueryGroup();
                break;
            case 'e':
            case 'E':
                AMTRDRV_MGR_BD_DumpRecordInLocalCheckingList();
                break;
            case 'f':
            case 'F':
                AMTRDRV_MGR_BD_DumpAgingOutBuffer();
                break;
            case 'g':
            case 'G':
                AMTRDRV_MGR_BD_Dump_NA_Hash_Counter();
                break;
            case 'h':
            case 'H':
                AMTRDRV_MGR_BD_ChecknClearHitBit();
                break;
            case 'i':
            case 'I':
                AMTRDRV_MGR_BD_AddressTaskPerformanceTest();
                break;
            case 'j':
            case 'J':
                AMTRDRV_MGR_BD_AsicTaskPerformanceTest();
                break;
            case 'k':
            case 'K':
                AMTRDRV_MGR_BD_PureProgramChipPerformanceTest();
                break;
            case 'l':
            case 'L':
                AMTRDRV_MGR_BD_ShowTotalCounters();
                break;
            case 'm':
            case 'M':
                AMTRDRV_MGR_BD_ShowByPortCounters();
                break;
            case 'n':
            case 'N':
                AMTRDRV_MGR_BD_ShowByVidCounter();
                break;
            case 'o':
            case 'O':
                amtrdrv_mgr_debug_show_collision_flag =!amtrdrv_mgr_debug_show_collision_flag;
                break;
            case 'p':
            case 'P':
                AMTRDRV_MGR_BD_IntergrationPerformanceTesting();
                break;
            case 'q':
            case 'Q':
                DEV_AMTRDRV_PMGR_DisableHardwareLearning();
                break;
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
            case 'r':
                AMTRDRV_MGR_BD_master_performanceCounter();
                break;
            case 's':
                AMTRDRV_MGR_BD_slave_performanceCounter();
                break;
            case 't':
                AMTRDRV_MGR_BD_asic_performanceCounter();
                break;
            case 'u':
                AMTRDRV_MGR_BD_periodic_performance();
                break;
            case 'v':
                AMTRDRV_MGR_BD_ENHW_performance();
                break;
            case 'w':
                AMTRDRV_MGR_BD_Dequeueperformance();
                break;
            case 'y':
                AMTRDRV_MGR_BD_RegisterNACallback();
                break;
#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
            case 'x':
                AMTRDRV_MGR_GetMemActionCounter();
                break;
#endif
#ifdef AMTRDRV_SLAVE_REPEATING_TEST
            case '?':
            AMTRDRV_MGR_BD_ShowSlaveRepeatCounter();
            break;
#endif
#ifdef AMTR_TYPE_DRV_SLAVE_ISC_TEST
            case ':':
            AMTRDRV_MGR_BD_IscTest();
            break;

            case '!':
            AMTRDRV_MGR_BD_IscTest_returndonothing();
            break;

            case '#':
            AMTRDRV_MGR_BD_IscTest_returndonothing1();
            break;
            case '$':
            AMTRDRV_MGR_BD_IscTest_returndonothing2();
            break;
#endif
#endif /* SYS_CPNT_MAINBOARD */
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */
}   /*  ISC_BackDoor_Menu   */
#endif/*BACKDOOR_OPEN*/

