/* Module Name: STKCTRL_TASK.C
 * Purpose: This file contains the information of stack control:
 *          1. inform the other sub-systems to enter master mode or slaver mode.
 *          2. use callback function (list) to inform the other sub-systems.
 * Notes:
 * History:
 *    07/03/2001       -- Aaron Chuang, Create
 *    09/16/2003       -- Charles Cheng, Clean the code.
 *    09/19/2003       -- Aaron Chuang, Change stack size of the stkctrl task from
 *                        SYS_BLD_TASK_COMMON_STACK_SIZE to SYS_BLD_TASK_LARGE_STACK_SIZE
 *                        Solved the stack overflow issue in enter master mode.
 *    07/10/2007       -- Charlie Chen, port to linux
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#define STKCTRL_BACKDOOR_OPEN   1

/* INCLUDE FILE DECLARATIONS
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "sys_callback_mgr.h"

#include "stkmgmt_type.h"
#include "stkctrl_task.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"

#include "syslog_type.h"
#include "syslog_om.h"  /* syslog_pmgr.h will need this */
#include "syslog_mgr.h" /* syslog_pmgr.h will need this */
#include "syslog_pmgr.h"
#include "sys_time.h"
#include "syslog_init.h"
#include "uc_mgr.h"
#include "fs.h"
#include "cli_pmgr.h"
#include "leaf_sys.h"

#include "leddrv.h"

#if (SYS_CPNT_CFGDB == TRUE)
#include "cfgdb_mgr.h"
#endif

#if (SYS_CPNT_DBSYNC_TXT ==TRUE)
#include "dbsync_txt_mgr.h"
#endif

#if (SYS_CPNT_SYSDRV == TRUE)
#include "sysdrv.h"
#endif

#include "stkctrl_backdoor.h"

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
#include "userauth.h"
#endif

#include "sys_callback_init.h"
#include "driver_group_operation.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define STKCTRL_STATE_INIT              0
#define STKCTRL_STATE_TRANSITION        1
#define STKCTRL_STATE_MASTER            2
#define STKCTRL_STATE_SLAVE             3
#define STKCTRL_STATE_MAX               4

#define STKCTRL_TASK_INVALID_MSGQ_HANDLE ((UI32_T)(-1))

/* DATA TYPE DECLARATIONS
 */
typedef struct STKCTRL_TASK_Control_S
{
    /* keep task id of stack control
     */
    UI32_T task_id;
    /* once if stack topology found system state is chagned, it
     * will send event STKCTRL_TASK_EVENT_STATE_CHANGE to stack control
     * and keep final state in this variable.
     */
    UI32_T topology_change_state;
    /* indicates current state of the whole system (stack).
     */
    UI32_T system_state;
    /*  main boards provision complete or not
     */
    BOOL_T main_board_provision_complete;
    /* For the module oper state machine.
     */
    UI32_T module_operational_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    /* Because in our design module processing is module by module,
     * we need to know which module is in processing.
     */
    UI32_T processing_module_unit_id;
    /* Only after one module is procesing done then do another module,
     * we need a flag to know current busy module is done or not.
     * If CLI has not told me module provision done, then this flag is off,
     * and do another process is not allowable.
     */
    BOOL_T is_current_module_process_done;

} STKCTRL_TASK_Control_T;


typedef struct STKCTRL_TASK_StateTable_S
{
    /* current system state
     */
    UI32_T cur_state;
    /* final state notified from stack topology
     */
    UI32_T tplg_final_state;
    /* real action 1 that we need to do
     */
    void   (*action_1)(void);
    /* real action 2 that we need to do,
     * if no need, assign this one to be NULL
     */
    void   (*action_2)(void);

} STKCTRL_TASK_StateTable_T;

typedef enum STKCTRL_TASK_Module_Event_E
{
    /* Once stack topology notify module status dirty.
     * Triggered by STKTPLG and STKCTRL.
     */
    STKCTRL_TASK_MODULE_EVENT_TO_DIRTY      = 1,
    /* Before stack control process all CSCs hot swap insertion/removal.
     * Trigger by STKCTRL.
     */
    STKCTRL_TASK_MODULE_EVENT_PROCESS       = 2,
    /* After stack control process all CSCs hot swap insertion/removal and
     * CLI tell me module provision done.
     * Trigger by STKCTRL.
     */
    STKCTRL_TASK_MODULE_EVENT_DONE          = 3,
    /* System status back to transition
     * Trigger by STKCTRL.
     */
    STKCTRL_TASK_MODULE_EVENT_REGRESSION    = 4,

} STKCTRL_TASK_Module_Event_T;


typedef enum STKCTRL_TASK_Module_Status_E
{
    /* Nothing to do and to wait
     */
    STKCTRL_TASK_MODULE_STATUS_NONE   = 1,
    /* Stack control processing all CSCs hot swap insertion/removal.
     */
    STKCTRL_TASK_MODULE_STATUS_BUSY   = 2,
    /* Stack control wait to make all CSCs hot swap insertion/removal.
     */
    STKCTRL_TASK_MODULE_STATUS_DIRTY  = 3,

} STKCTRL_TASK_Module_Status_T;

typedef struct STKCTRL_TASK_CscGroupMsgQInfo_S
{
    UI32_T ipc_msgq_key;
    char*  csc_group_name;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T csc_subset_identifier; /* SYS_TYPE_CMD_CscSubset_T */
} STKCTRL_TASK_CscGroupMsgQInfo_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static  void    STKCTRL_TASK_ReportSyslogMessage(UI8_T err_msg_no, UI8_T fun_no, UI8_T err_no, UI8_T *arg_0);
static  void    STKCTRL_TASK_ControlSystemState(void);

#if (STKCTRL_BACKDOOR_OPEN == 1)
static  void    STKCTRL_TASK_BeforeCscToMasterMode(STKCTRL_BACKDOOR_CPNT_TIMER_T *ptime, char *csc_name);
#else
#define STKCTRL_TASK_BeforeCscToMasterMode(ptime, csc_name)
#endif
static  BOOL_T  STKCTRL_TASK_AfterCscToMasterMode(STKCTRL_BACKDOOR_CPNT_TIMER_T	*ptime);


static  void    STKCTRL_TASK_SetTransitionMode(void);
static  void    STKCTRL_TASK_EnterTransitionMode(void);
static  void    STKCTRL_TASK_EnterSlaveMode(void);
static  void    STKCTRL_TASK_EnterMasterMode(void);
static  void    STKCTRL_TASK_PreProvisionComplete(void);
static  void    STKCTRL_TASK_ProvisionComplete(UI32_T unit);
static  void    STKCTRL_TASK_EnterTransitionMasterMode(void);
#if 0 /* Do not support module with CPU on linux platform, need not to port these two functions */
static  void    STKCTRL_TASK_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port,  BOOL_T use_default);
static  void    STKCTRL_TASK_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
#endif
static  void    STKCTRL_TASK_ProcessModuleStateChange(void);
static  BOOL_T  STKCTRL_TASK_IsAnyModuleNeedToProcess(void);
static UI32_T   STKCTRL_TASK_ModuleStateMachine(UI32_T current_status, UI32_T event);
static  void    STKCTRL_TASK_ReloadSystemEvent(void);
static  void    STKCTRL_TASK_WarmStartSystemEvent(void);
static  void    STKCTRL_TASK_ColdStartSystemEvent(void);
static  void    STKCTRL_TASK_UnitIDReNumberingEvent(void);

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
static  BOOL_T  STKCTRL_TASK_WaitForDBSyncAndXferDone(void);
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static BOOL_T STKCTRL_TASK_ProcessUnitInsertRemove(UI32_T *unit);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* target state which is informed by STKTPLG
 */
static  UI32_T                  targetState;

/* control information for this module
 */
static  STKCTRL_TASK_Control_T  ctrl_info;

/* CFGDB session ID for boot reason
 */
static UI32_T stkctrl_boot_reason_session_handler = 0;

/* state transition table for system
 */
static STKCTRL_TASK_StateTable_T    state_table[] =
{
    {
    	STKCTRL_STATE_INIT, STKCTRL_STATE_TRANSITION,
    	STKCTRL_TASK_EnterTransitionMode, NULL
    },
    {
        STKCTRL_STATE_INIT, STKCTRL_STATE_MASTER,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterMasterMode,
    },
    {
        STKCTRL_STATE_INIT, STKCTRL_STATE_SLAVE,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterSlaveMode
    },
    {
        STKCTRL_STATE_TRANSITION, STKCTRL_STATE_MASTER,
        STKCTRL_TASK_EnterMasterMode, NULL
    },
    {
        STKCTRL_STATE_TRANSITION, STKCTRL_STATE_SLAVE,
        STKCTRL_TASK_EnterSlaveMode, NULL
    },
    {
        STKCTRL_STATE_MASTER, STKCTRL_STATE_TRANSITION,
        STKCTRL_TASK_EnterTransitionMode, NULL
    },
    {
        STKCTRL_STATE_MASTER, STKCTRL_STATE_MASTER,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterMasterMode
    },
    {
        STKCTRL_STATE_MASTER, STKCTRL_STATE_SLAVE,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterSlaveMode
    },
    {
        STKCTRL_STATE_SLAVE, STKCTRL_STATE_TRANSITION,
        STKCTRL_TASK_EnterTransitionMode, NULL
    },
    {
        STKCTRL_STATE_SLAVE, STKCTRL_STATE_SLAVE,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterSlaveMode
    },
    {
        STKCTRL_STATE_SLAVE, STKCTRL_STATE_MASTER,
        STKCTRL_TASK_EnterTransitionMode, STKCTRL_TASK_EnterMasterMode
    }
};

/* the order of csc group in csc_group_msgq_info is defined in the order of the
 * lower layer to higher layer
 */
static STKCTRL_TASK_CscGroupMsgQInfo_T csc_group_msgq_info[]=
{
/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, 2008-04-08, 11:34:07 */
//    {SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, "DRIVER_GROUP", STKCTRL_TASK_INVALID_MSGQ_HANDLE},
#else /* use driver process main thread to handle stacking mode change event */
    {SYS_BLD_DRIVER_PROC_OM_IPCMSGQ_KEY, "DRIVER_PROC", STKCTRL_TASK_INVALID_MSGQ_HANDLE, 0},
#endif
    {SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, "UTILITY_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if 0 /* comment out until cscgroup is ready */
    {SYS_BLD_EH_GROUP_IPCMSGQ_KEY, "EH_GROUP", STKCTRL_TASK_INVALID_MSGQ_HANDLE, 0},
#endif
#if (SYS_CPNT_CFGDB == TRUE)
    {SYS_BLD_CFGDB_GROUP_IPCMSGQ_KEY, "CFGDB_TD", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY, "SWCTRL_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_L2MCAST == TRUE)
    {SYS_BLD_CSC_MSL_TASK_MSGK_KEY, SYS_BLD_MSL_CSC_THREAD_NAME, L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY, "L2MUX_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_LLDP == TRUE)
    {SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY, "GVRP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    {SYS_BLD_OAM_GROUP_IPCMSGQ_KEY, "OAM_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    {SYS_BLD_LACP_GROUP_IPCMSGQ_KEY, "LACP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_XFER_GROUP_IPCMSGQ_KEY, "XFER_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
    {SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, "NETACCESS_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},

    {SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY, "AUTH_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
    {SYS_BLD_STA_GROUP_IPCMSGQ_KEY, "STA_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_L2MCAST == TRUE)
    {SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY, "L2MCAST_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_PPPOE_IA == TRUE)
    {SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY, "PPPOE_IA_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_L4_GROUP_IPCMSGQ_KEY, "L4_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
    {SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, "NETCFG_GROUP_L3IF", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), SYS_TYPE_CMD_CSC_SUBSET_IDENTIFIER_L3IF},
#if (SYS_CPNT_AMTRL3 == TRUE)
    {SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY, "AMTRL3_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_DHCPSNP == TRUE || SYS_CPNT_DHCPV6SNP == TRUE || SYS_CPNT_NDSNP == TRUE || SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
    {SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY, "DHCPSNP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY, "SYSMGMT_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_CLUSTER == TRUE)
    {SYS_BLD_CLUSTER_GROUP_IPCMSGQ_KEY, "CLUSTER_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, "NETCFG_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_RIP == TRUE)
    {SYS_BLD_RIP_GROUP_IPCMSGQ_KEY, "RIP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_OSPF == TRUE)
    {SYS_BLD_OSPF_GROUP_IPCMSGQ_KEY, "OSPF_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    {SYS_BLD_OSPF6_GROUP_IPCMSGQ_KEY, "OSPF6_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_BGP == TRUE)
    {SYS_BLD_BGP_GROUP_IPCMSGQ_KEY, "BGP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_VRRP == TRUE)
    {SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY, "VRRP_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_POE == TRUE)
    {SYS_BLD_POE_GROUP_IPCMSGQ_KEY, "POE_TASK", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_UDP_HELPER == TRUE)
    {SYS_BLD_UDPHELPER_GROUP_IPCMSGQ_KEY, "UDPHELPER_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if 0 /* comment out until cscgroup is ready */
    {SYS_BLD_UI_MGR_GROUP_IPCMSGQ_KEY, "UI_MGR_GROUP", STKCTRL_TASK_INVALID_MSGQ_HANDLE, 0},
#endif
#if (SYS_CPNT_DAI == TRUE)
    {SYS_BLD_DAI_GROUP_IPCMSGQ_KEY, "DAI_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
    {SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY, "IP_SERVICE_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
    {SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY, "APP_PROTOCOL_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#if (SYS_CPNT_VXLAN == TRUE)
    {SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY, "VXLAN_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_SSH2 == TRUE || SYS_CPNT_KEYGEN == TRUE)
    {SYS_BLD_SSH_GROUP_IPCMSGQ_KEY, "SSH_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
    {SYS_BLD_DCB_GROUP_IPCMSGQ_KEY, "DCB_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif

#if (SYS_CPNT_MLAG == TRUE)
    {SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY, "MLAG_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif

    {SYS_BLD_SNMP_GROUP_IPCMSGQ_KEY, "SNMP_PROC", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},

    {SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, "WEB_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},

#if (SYS_CPNT_TELNET == TRUE)
    {SYS_BLD_TELNET_GROUP_IPCMSGQ_KEY, "TELNET_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_SFLOW == TRUE)
    {SYS_BLD_SFLOW_GROUP_IPCMSGQ_KEY, "SFLOW_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif
#if (SYS_CPNT_CFM == TRUE)
    {SYS_BLD_CFM_GROUP_IPCMSGQ_KEY, "CFM_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
#endif

    /* CLI group must be the last for hot removal handling */
    {SYS_BLD_CLI_GROUP_IPCMSGQ_KEY, "CLI_GROUP", L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE), 0},
};

/* MACRO FUNCTIONS DECLARACTION
 */
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
enum
{
   TRANSITION_MODE,
   MASTER_MODE,
   SLAVE_MODE,
   HOTADD_MODE,
   HOTREMOVE_MODE,
   PROVISION_COMPLETED_MODE,
   MAX_NUM
};
 /*the last one for the driver group*/
 #define stkctrl_bd_driver_group_index sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0])
   UI32_T stkctrl_bd_max_time[MAX_NUM][sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0])+1];
#endif
#define DBGMSG(STRING, ...)                                  \
    if (TRUE == STKCTRL_BACKDOOR_IsStkCtrlDbgMsgOn())   \
        printf("\r\n"#STRING, ##__VA_ARGS__);

#define RUN_MODULE_STATE_MACHINE(UNIT, EVENT)                                           \
    ctrl_info.module_operational_status[(UNIT)-1] =                                       \
        STKCTRL_TASK_ModuleStateMachine(ctrl_info.module_operational_status[(UNIT)-1],    \
                                        EVENT);

#define STKCTRL_TASK_SET_TRANSITION_MODE_TO_CSC_GROUP(ipcmsgq_key, csc_group_name) \
    if(SYSFUN_OK!=(ret=SYSFUN_GetMsgQ(ipcmsgq_key, \
        SYSFUN_MSGQ_BIDIRECTIONAL,&msgq_handle))) \
    { \
        printf("\r\n%s():Get %s IPC MSGQ fail.(ret=%d)",__FUNCTION__, csc_group_name, (int)ret); \
    } \
    do{ \
        task_id=SYSFUN_GetMsgQOwner(msgq_handle); \
        /* when the owner of the msgq hasn't be created, take a snap and retry \
         * latter \
         */ \
        if(task_id==0) \
        { \
            SYSFUN_Sleep(5); \
        } \
    }while(task_id==0); \
    if(SYSFUN_OK!=(ret=SYSFUN_SendEvent(task_id, SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE))) \
    { \
        printf("\r\n%s():send event to %s fail.(ret=%d)",__FUNCTION__, csc_group_name, (int)ret); \
    }

/* EXPORTED SUBPROGRAM BODIES
 */
#if 0 /* remove */
/* FUNCTION NAME: STKCTRL_TASK_Create_Tasks
 * PURPOSE: This function is used to create the main task of the stack
 *          control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE - successful.
 *          FALSE - unspecified failure, system error.
 * NOTES:   None.
 */
BOOL_T STKCTRL_TASK_Create_Tasks(void)
{
    if ( SYSFUN_SpawnThread(SYS_BLD_STACK_CTRL_TASK_PRIORITY, SYSFUN_SCHED_DEFAULT,
                          SYS_BLD_STACK_CTRL_TASK,
                          SYS_BLD_TASK_LARGE_STACK_SIZE, SYSFUN_TASK_NO_FP,
                          STKCTRL_TASK_Main, 0,
                          &ctrl_info.task_id) != SYSFUN_OK)
    {
    	STKCTRL_TASK_ReportSyslogMessage(CREATE_TASK_FAIL_MESSAGE_INDEX,
    	                                 STKCTRL_TASK_Create_Tasks_FunNo,
    	                                 STKCTRL_TASK_Create_Tasks_ErrNo,
    	                                 "STKCTRL_TASK");
        return FALSE;
    }

    return TRUE;

} /* End of STKCTRL_TASK_Create_Tasks */
#endif

/* FUNCTION NAME: STKCTRL_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   None.
 */
BOOL_T STKCTRL_TASK_InitiateProcessResources(void)
{
    UI32_T rc, unit_id;
    UI16_T idx;
    BOOL_T ret;

    /* set LED pattern
     */
    /* Invoke PMGR API before in the initiate process phase will block the INIT
     * process. Move the following function call to the
     * beginning of STKCTRL_GROUP_Main_Thread_Function_Entry()
     */
    //STKTPLG_PMGR_SetStackRoleLed(LEDDRV_STACK_ARBITRATION);

    /* initialize control data structure
     */
    memset(&ctrl_info, 0, sizeof(STKCTRL_TASK_Control_T));

    for(unit_id=1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        ctrl_info.module_operational_status[unit_id-1] = STKCTRL_TASK_MODULE_STATUS_NONE;
    }

    /* initialize csc_group_msgq_info
     */
    ret=TRUE;
    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
    {
        if(SYSFUN_OK!=(rc=SYSFUN_GetMsgQ(csc_group_msgq_info[idx].ipc_msgq_key,
            SYSFUN_MSGQ_BIDIRECTIONAL,&(csc_group_msgq_info[idx].msgq_handle))))
        {
            printf("\r\n%s():Get %s IPC MSGQ fail.(ret=%d)",__FUNCTION__, csc_group_msgq_info[idx].csc_group_name, (int)rc);
            ret=FALSE;
        }
    }

    return ret;

} /* End of STKCTRL_TASK_InitiateProcessResources */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - STKCTRL_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void STKCTRL_TASK_Create_InterCSC_Relation(void)
{

} /* end of STKCTRL_TASK_Create_InterCSC_Relation */

/* FUNCTION NAME: STKCTRL_TASK_HandleEvents
 * PURPOSE: This procedure handles the events for the topology control task.
 * INPUT:   events_p  --  events to be handled
 * OUTPUT:  events_p  --  events remains after this function is called
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_HandleEvents(UI32_T *events_p,UI32_T *unit)
{
    UI32_T events;

    events = *events_p;

    if (events & STKCTRL_TASK_EVENT_STATE_CHANGE)
    {
        ctrl_info.topology_change_state = targetState;
        STKCTRL_TASK_ControlSystemState();

        /* if state is changed, there is no need for us to handle
         * other events.
         * we should come back immediately
         */
        *events_p=0;
        return;
    }

    if (events & STKCTRL_TASK_EVENT_RUNNING_CONFIG)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            /* suger, 09-22-2004,
             * set main_board_provision_complete flag to FALSE to skip
             * STKCTRL_TASK_ModuleStateChanged_CallBack() process before
             * provision complete.
             */
            ctrl_info.main_board_provision_complete = FALSE;
            /* currently it is handled by warm start. if we
             * want to use enter transition and enter master mode
             * to complete this function, we should consider this
             * kind of thing very carefully.
             */
            STKCTRL_TASK_EnterTransitionMasterMode();
        }
        events &= ~STKCTRL_TASK_EVENT_RUNNING_CONFIG;
    }

    if (events & STKCTRL_TASK_EVENT_PROVISION_COMPLETE)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            ctrl_info.main_board_provision_complete = TRUE;
            /* Before do module insertion/removal process, trun on this flag.
             */
            ctrl_info.is_current_module_process_done = TRUE;

            /* if no any module need to be processed, then notify all CSCs provision
             * complete.
             */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            if (FALSE == STKCTRL_TASK_IsAnyModuleNeedToProcess() && FALSE == STKTPLG_POM_IsAnyUnitNeedToProcess())
#else
            if (FALSE == STKCTRL_TASK_IsAnyModuleNeedToProcess())
#endif
            {
                *unit = 0 ;
                STKCTRL_TASK_PreProvisionComplete();
                STKCTRL_TASK_ProvisionComplete(*unit);
            }
        }
        events &= ~STKCTRL_TASK_EVENT_PROVISION_COMPLETE;
    }

    if (events & STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE)
    {
        /* 1) handle module status only when main board
         *    provision complete.
         * 2) If previous process has not been done, another
         *    processing is allowable.
         */
        if (ctrl_info.main_board_provision_complete  == TRUE &&
            ctrl_info.is_current_module_process_done == TRUE )
        {
            STKCTRL_TASK_ProcessModuleStateChange();

            if (FALSE == STKCTRL_TASK_IsAnyModuleNeedToProcess())
            {
                events &= ~STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE;

                /* no module need to process and current module is done;
                 * i.e. removal case, don't wait for module provison complete,
                 * just to notify all CSCs provision complete.
                 * This is overall provision completed.
                 */
                if (TRUE == ctrl_info.is_current_module_process_done)
                {
                    STKCTRL_TASK_PreProvisionComplete();
                    *unit = 0;
                    STKCTRL_TASK_ProvisionComplete(*unit);
                }
            }
        }
        else
        {
            /* Since provision for main board not completed, that means
             * still have some other task need to something, so sleep here
             * to force re-scheduling.
             */
            SYSFUN_Sleep(10);
        }
    }

    if (events & STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER &&
            ctrl_info.main_board_provision_complete == TRUE)
        {
            RUN_MODULE_STATE_MACHINE(ctrl_info.processing_module_unit_id, STKCTRL_TASK_MODULE_EVENT_DONE);
            ctrl_info.is_current_module_process_done = TRUE;

            if (TRUE == STKCTRL_TASK_IsAnyModuleNeedToProcess())
            {
                /* If still some module not process hot insertion
                 * do another module hot insertion process.
                 */
                events |= STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE;
            }
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            else if (TRUE == STKTPLG_POM_IsAnyUnitNeedToProcess())
            {
               events |= STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE;
            }
#endif
            else
            {
                /* In our design, module process is module by module,
                 * if all modules are in static state and got a provision done event
                 * that means overall provision done.
                 */
                STKCTRL_TASK_PreProvisionComplete();
                STKCTRL_TASK_ProvisionComplete(*unit);
            }
        }
        events &= ~STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE;
    }

    /* When receive reload event, stack ctrl task will wait for DBSync & CFGDB
     * complete sync operation, then do reload system.
     */
    if (events & STKCTRL_TASK_EVENT_RELOAD_SYSTEM)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            STKCTRL_TASK_ReloadSystemEvent();
        }
        events &= ~STKCTRL_TASK_EVENT_RELOAD_SYSTEM;
    }

    /* When receive reload event, stack ctrl task will wait for DBSync & CFGDB
     * complete sync operation, then do warm start system.
     */
    if (events & STKCTRL_TASK_EVENT_WARM_START_SYSTEM)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            STKCTRL_TASK_WarmStartSystemEvent();
        }
        events &= ~STKCTRL_TASK_EVENT_WARM_START_SYSTEM;
    }

    /* When receive reload event, stack ctrl task will wait for DBSync & CFGDB
     * complete sync operation, then do cold start system.
     */
    if (events & STKCTRL_TASK_EVENT_COLD_START_SYSTEM)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            STKCTRL_TASK_ColdStartSystemEvent();
        }
        events &= ~STKCTRL_TASK_EVENT_COLD_START_SYSTEM;
    }

    /* When receive renumber event, stack ctrl task will wait for DBSync & CFGDB
     * complete sync operation, then renumber unit id.
     */
    if (events & STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER)
    {
        if (ctrl_info.system_state == STKCTRL_STATE_MASTER)
        {
            STKCTRL_TASK_UnitIDReNumberingEvent();
        }
        events &= ~STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER;
    }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (events & STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE)
    {
        if (ctrl_info.main_board_provision_complete  == TRUE &&
             ctrl_info.is_current_module_process_done == TRUE )
        {
           if(TRUE == STKCTRL_TASK_ProcessUnitInsertRemove(unit))
           {
                    events &= ~STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE;
           }
           else
           {
               /* A Unit Hot Insert/Remove has been processed
                     */
               if(ctrl_info.is_current_module_process_done == TRUE)
               {
                        /* Not a "successful Hot Insert", so we can
                         * "try" to provision complete
                        */
                   events |= STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE;
               }
           }
        }
    }
#endif
    *events_p =events;

} /* end of STKCTRL_TASK_HandleEvents() */

/* FUNCTION NAME: STKCTRL_TASK_EnterTransitionMode_CallBack
 * PURPOSE: Hook function to receive enter transition event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_EnterTransitionMode_CallBack(UI32_T *events_p)
{
    UI32_T unit =0;
    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument.", __FUNCTION__);
        return;
    }

    /* notify stack control task that CLI wants us to enter transition mode.
     * this condition only happens that we need to re-initialize whole stack,
     * because user want to initialize running config.
     * it is all right for us to use event. it will not happen that events
     * are overwirtten.
     */

    *events_p = STKCTRL_TASK_EVENT_RUNNING_CONFIG;
    /* STKCTRL_TASK_EVENT_RUNNING_CONFIG will be handled in this function call
     * Need not to care about the output 'events'
     */
    STKCTRL_TASK_HandleEvents(events_p,&unit);

    return;

} /* End of STKCTRL_TASK_EnterTransitionMode_CallBack */

/* FUNCTION NAME: STKCTRL_TASK_ProvisionComplete_CallBack
 * PURPOSE: Hook function to receive provision complete event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ProvisionComplete_CallBack(UI32_T *events_p)
{
    UI32_T unit;

    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument.", __FUNCTION__);
        return;
    }

    /* 1) force STKCTRL itself to read all module status, and
     *    notify stack control task that configuration provision is completed.
     * 2) it is all right for us to use event. it will not happen that events
     *    are overwirtten.
     */
    unit = 0;
    while(STKTPLG_POM_GetNextUnit(&unit))
    {
        RUN_MODULE_STATE_MACHINE(unit, STKCTRL_TASK_MODULE_EVENT_TO_DIRTY);
    }

    *events_p = STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE | STKCTRL_TASK_EVENT_PROVISION_COMPLETE;
    unit = 0;
	STKCTRL_TASK_HandleEvents(events_p,&unit);
    return;

} /* End of STKCTRL_TASK_ProvisionComplete_CallBack */

/* FUNCTION NAME: STKCTRL_TASK_ModuleProvisionComplete_CallBack
 * PURPOSE: Hook function to receive module provision complete event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ModuleProvisionComplete_CallBack(UI32_T *events_p,UI32_T *unit)
{
    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument", __FUNCTION__);
        return;
    }

    *events_p=STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE;
    STKCTRL_TASK_HandleEvents(events_p,unit);

    printf("\r\nModule provision complete.\r\n");
}

/* FUNCTION NAME: STKCTRL_TASK_StackState_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   msg     -- topology change msg(TPOGY_CHANGE, TPOGY_LOSE)
 * OUTPUT:  events  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_StackState_CallBack(UI32_T stktplg_msg, UI32_T *events_p)
{
    UI32_T unit;
    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument", __FUNCTION__);
        return;
    }

    /* check message from stack topology and transfer it to state for stack control
     * (whole system).
     */
    switch (stktplg_msg)
    {
        case STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE:
            targetState = STKCTRL_STATE_TRANSITION;
            break;

        case STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE:
            targetState = STKCTRL_STATE_MASTER;
            break;

        case STKTPLG_MASTER_LOSE_MSG:
            targetState = STKCTRL_STATE_SLAVE;
            break;

        default:
            STKCTRL_TASK_ReportSyslogMessage(SWITCH_TO_DEFAULT_MESSAGE_INDEX,
    	                                     STKCTRL_TASK_StackState_CallBack_FunNo,
    	                                     STKCTRL_TASK_StackState_CallBack_SwitchToDefault_ErrNo,
    	                                     (UI8_T*)"STKCTRL_TASK_StackState_CallBack");
            return;
    }

    /* notify stack control task that topology state is changed.
     * with event, it is possible that we will overwrite some of them.
     * but it is fine for us, we only need the final state of stack topology.
     */
    *events_p=STKCTRL_TASK_EVENT_STATE_CHANGE;
    unit = 0;
    STKCTRL_TASK_HandleEvents(events_p,&unit);

    return;

} /* End of STKCTRL_TASK_StackState_CallBack */

/* FUNCTION NAME: STKCTRL_TASK_ModuleStateChanged_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   unit_id     -- The module state of which unit was changed.
 * OUTPUT:  events_p    -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ModuleStateChanged_CallBack(UI32_T *unit_id, UI32_T *events_p)
{

    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument.", __FUNCTION__);
        return;
    }

    if (ctrl_info.main_board_provision_complete != TRUE)
    {
        /* Before master mode, all module state change should be dropped,
         * because module state dirty will be cleared, and after all CSCs
         * entering master mode, STKCTRL will force itself to read all unit
         * module status.
         */
        return;
    }

    RUN_MODULE_STATE_MACHINE(*unit_id, STKCTRL_TASK_MODULE_EVENT_TO_DIRTY);

    *events_p=STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE;
    STKCTRL_TASK_HandleEvents(events_p,unit_id);

    printf("\r\nModule in %lu unit is changed.\r\n", unit_id);
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: STKCTRL_TASK_ReportSyslogMessage
 * PURPOSE: To log information in SYSLOG.
 * INPUT:   err_msg_no -- Message number.
 *          fun_no     -- Function number.
 *          err_no     -- Error number.
 *          arg_0      -- Argument for SYSLOG.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_ReportSyslogMessage(UI8_T err_msg_no, UI8_T fun_no, UI8_T err_no, UI8_T *arg_0)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    owner_info.module_no = SYS_MODULE_STKMGMT;

    /* define fun_no and err_no manually, what a foolish thing ???
     * unbelivable.
     */
    owner_info.function_no = fun_no;
    owner_info.error_no = err_no;

    switch(err_msg_no)
    {
        case CREATE_TASK_FAIL_MESSAGE_INDEX:
            owner_info.level = SYSLOG_LEVEL_CRIT;
            break;

        case ALLOCATE_MEMORY_FAIL_MESSAGE_INDEX:
            owner_info.level = SYSLOG_LEVEL_CRIT;
            break;

        case SWITCH_TO_DEFAULT_MESSAGE_INDEX:
            owner_info.level = SYSLOG_LEVEL_ERR;
            break;

        default:
            owner_info.level = SYSLOG_LEVEL_ERR;
            owner_info.function_no = STKCTRL_TASK_ReportSyslogMessage_FunNo;
            owner_info.error_no = STKCTRL_TASK_ReportSyslogMessage_SwitchToDefault_ErrNo;
            err_msg_no = SWITCH_TO_DEFAULT_MESSAGE_INDEX;
            SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, err_msg_no, "STKCTRL_TASK_ReportSyslogMessage", 0, 0);
            return;
    }

    SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, err_msg_no, (void *) arg_0, 0, 0);

    return;
}   /* End of STKCTRL_TASK_ReportSyslogMessage */


/* FUNCTION NAME: STKCTRL_TASK_ControlSystemState
 * PURPOSE: To call specific API by current state.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_ControlSystemState(void)
{
    STKCTRL_TASK_StateTable_T  *table_ptr;

    UI32_T table_idx;

    /* check what action that we should do via state table
     */
    for (table_idx = 0; table_idx < (sizeof(state_table)/sizeof(STKCTRL_TASK_StateTable_T)); table_idx++)
    {
    	table_ptr = &state_table[table_idx];

        if ((ctrl_info.system_state == table_ptr->cur_state) &&
            (ctrl_info.topology_change_state == table_ptr->tplg_final_state))
        {
            /* call corresponding actions
             */
            if (table_ptr->action_1 != NULL)
                table_ptr->action_1();

            if (table_ptr->action_2 != NULL)
                table_ptr->action_2();

            /* state change is completed, keep current state
             */
            ctrl_info.system_state = ctrl_info.topology_change_state;
            break;
        }
    }

    return;

} /* end of STKCTRL_TASK_ControlSystemState() */

#if (STKCTRL_BACKDOOR_OPEN == 1)
/* FUNCTION NAME: STKCTRL_TASK_BeforeCscToMasterMode
 * PURPOSE: To save the time before to enter master mode foe one CSC.
 * INPUT:   ptime    -- Time date storage.
 *          csc_name -- The CSC name to display.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_BeforeCscToMasterMode(STKCTRL_BACKDOOR_CPNT_TIMER_T *ptime, char *csc_name)
{
#if (STKCTRL_BACKDOOR_OPEN == 1)
    sprintf((char*)ptime->cpnt_name[ptime->cpnt_nbr], "%s", (csc_name));
    ptime->cpnt_time[ptime->cpnt_nbr] = SYS_TIME_GetSystemTicksBy10ms();
#endif
    DBGMSG("To %s", csc_name);
    return;
}
#endif

/* FUNCTION NAME: STKCTRL_TASK_AfterCscToMasterMode
 * PURPOSE: To save the time need to enter master mode foe one CSC
            and check if topology change.
 * INPUT:   ptime -> Time date storage.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- Topology change.
 *          FALSE -- Topology not change.
 * NOTES:   None.
 */
static BOOL_T STKCTRL_TASK_AfterCscToMasterMode(STKCTRL_BACKDOOR_CPNT_TIMER_T	*ptime)
{
#if (STKCTRL_BACKDOOR_OPEN == 1)
    ptime->cpnt_time[ptime->cpnt_nbr] = SYS_TIME_GetSystemTicksBy10ms() - ptime->cpnt_time[ptime->cpnt_nbr];
    ptime->cpnt_nbr++;
#endif

    /* for the sake of rapidly leaving entering master mode,
     * will check whether the state is changed while we are doing entering master mode.
     * If so, we will leave entering master mode right away to prevent doing something stupid.
     */
    if (STKTPLG_POM_IsStkTplgStateChanged(STKTPLG_OM_SIMPLE_STATE_MASTER))
    {
        printf("\r\nStop Entering Master mode due to topology change.\r\n");
        return TRUE;
    }
    return FALSE;
}


/* FUNCTION NAME: STKCTRL_TASK_SetTransitionMode
 * PURPOSE: To set the flag of transition mode for all CSCs.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_SetTransitionMode(void)
{
    UI32_T rc;
    UI32_T task_id=0,ret; /* init for avoid warning msg*/
    I16_T idx;
    UI16_T counter;

    printf("\r\nSetting transition mode ...");
    /* When enter transition mode, all CSCs will reset their database.
     *
     */

    /* handle message-sending part
     */
    for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
            counter =0;
            DBGMSG("Send event to %s", csc_group_msgq_info[idx].csc_group_name);
            do{
/*get task id by name*/
#if 0
                task_id=SYSFUN_GetMsgQOwner(csc_group_msgq_info[idx].msgq_handle);
#else
                ret=SYSFUN_TaskNameToID(csc_group_msgq_info[idx].csc_group_name,&task_id);
#endif
                /* when the owner of the msgq hasn't be created, take a snap and retry
                 * latter
                 */
                if(task_id==0 || ret !=SYSFUN_OK)
                {
                    SYSFUN_Sleep(5);
                }
                counter++;
                if(counter % 300 == 0)
                printf("\r\n system too slow,cannot get %s taskID",csc_group_msgq_info[idx].csc_group_name);
            }while(task_id==0 ||ret !=SYSFUN_OK);
            DBGMSG("Finish sending event to %s, task_id = %d", csc_group_msgq_info[idx].csc_group_name, task_id);
        }

        if(SYSFUN_OK!=(rc=SYSFUN_SendEvent(task_id, SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)))
        {
            printf("\r\n%s():send event to %s fail.(ret=%d).task_id=%lu\n",__FUNCTION__, csc_group_msgq_info[idx].csc_group_name, (int)rc, task_id);
        }
    }

    /* handle direct-call part
     */
    DBGMSG("Set transition mode to SYS_CALLBACK");
    SYS_CALLBACK_INIT_SetTransitionMode();

    DBGMSG("Set transition mode to DRIVER_GROUP shared lib");
/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-08, 11:34:39 */
    DRIVER_GROUP_SetTransitionMode();
#endif

    printf("\r\nFinished setting transition mode ...");

    return;

} /* End of STKCTRL_TASK_SetTransitionMode */


/* FUNCTION NAME: STKCTRL_TASK_EnterTransitionMode
 * PURPOSE: This function returns true if system database can be successfully
 *          initiated. Otherwise, false is returned.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_EnterTransitionMode(void)
{
    UI32_T unit_id;
    SYSFUN_Msg_T sysfun_msg;
    I16_T idx;

    /* To change main board provision complete state
     */
    ctrl_info.main_board_provision_complete = FALSE;

    /* To force module operation status as none.
     */
    for (unit_id = 1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit_id++)
    {
        RUN_MODULE_STATE_MACHINE(unit_id, STKCTRL_TASK_MODULE_EVENT_REGRESSION);
    }

    /* SLAVE is not ready when I'm going to be SLAVE
     */
    STKTPLG_PMGR_SlaveReady(FALSE);

    /* To indicate that provision has not been completed yet
     */
    STKTPLG_PMGR_ProvisionCompleted(FALSE);

    /* Set transition mode */
    STKCTRL_TASK_SetTransitionMode();

    printf("\r\nEntering transition mode ...");

    /* When enter transition mode, all CSCs will reset their database.
     */
    /* handle message-sending part
     */
    for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
		#endif
            sysfun_msg.cmd=SYS_TYPE_CMD_ENTER_TRANSITION_MODE;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s\n", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish sending message to %s\n", csc_group_msgq_info[idx].csc_group_name);
		#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()-time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(TRANSITION_MODE,idx,time2);
		#endif
        }
    }

    /* handle direct-call part
     */
    DBGMSG("Enter transition mode to SYS_CALLBACK\n");
    SYS_CALLBACK_INIT_EnterTransitionMode();

    DBGMSG("Enter transition mode to DRIVER_GROUP shared lib\n");
/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-08, 11:34:43 */
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
    UI32_T time1,time2;
    time1 = SYSFUN_GetSysTick();
#endif

    DRIVER_GROUP_EnterTransitionMode();
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
    time2 = SYSFUN_GetSysTick()-time1;
    STKCTRL_TASK_BD_SetPerformaceMaxTime(TRANSITION_MODE,stkctrl_bd_driver_group_index,time2);
#endif
#endif

    printf("\r\nFinished transition mode ...");

    return;

} /* End of STKCTRL_TASK_EnterTransitionMode */


/* FUNCTION NAME: STKCTRL_TASK_EnterSlaveMode
 * PURPOSE: To make all CSCs enter svale mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. The low level device driver shll be disabled first to prevent any
 *             device access operation.
 *          2. All of the object managers except E2prom and LED, will be
 *             disabled to prevent any database access operation.
 */
static void STKCTRL_TASK_EnterSlaveMode(void)
{
    SYSFUN_Msg_T sysfun_msg;
    I16_T idx;

    /* BODY */
    printf("\r\nEnter Slave mode ...");

    /* handle direct-call part
     */
    DBGMSG("Enter slave mode to SYS_CALLBACK");
    SYS_CALLBACK_INIT_EnterSlaveMode();

    DBGMSG("Enter slave mode to DRIVER_GROUP shared lib");
/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-08, 11:34:46 */
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)

        UI32_T time1,time2;
        time1 = SYSFUN_GetSysTick();
#endif

    DRIVER_GROUP_EnterSlaveMode();
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
    time2 = SYSFUN_GetSysTick()-time1;
    STKCTRL_TASK_BD_SetPerformaceMaxTime(SLAVE_MODE,stkctrl_bd_driver_group_index,time2);
#endif

#endif

    /* handle message-sending part
     */

    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
		#endif
            sysfun_msg.cmd=SYS_TYPE_CMD_ENTER_SLAVE_MODE;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
		#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()-time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(SLAVE_MODE,idx,time2);
		#endif
        }
    }

    STKTPLG_PMGR_SlaveReady(TRUE);

    /* set LED pattern
     */
#if 0 /*stktplg will light led,here pmgr to stktplg,it will delayed ,may be the role is changed*/
    STKTPLG_PMGR_SetStackRoleLed(LEDDRV_STACK_SLAVE);
#endif
    printf("\r\nFinish Slave mode ...");

    return;
}


/* FUNCTION NAME: STKCTRL_TASK_EnterMasterMode
 * PURPOSE: This function returns true if system database can be successfully
 *          initiated. Otherwise, false is returned.
 *          If topology changed during the system database initiation, the
 *          system database initiation will be aborted and false is returned.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Each subsystem shall purge its data base, and then reinitiates the
 *             database according the new topology.
 *          2. After database initiation completed, each subsystem shall initiate/
 *             configure the device if necessary.
 *          3. Re-enable the low level device drivers after system database initiated.
 */
static void STKCTRL_TASK_EnterMasterMode(void)
{
    SYSFUN_Msg_T sysfun_msg;
    I16_T idx;
    STKCTRL_BACKDOOR_CPNT_TIMER_T *ptime = NULL; /* init to avoid warning msg */
    BOOL_T need_to_sync = 0;
    UI32_T stkctrl_boot_reason = 0;
    UI32_T my_unit_id = 0;

#define AFTER_CSC_ENTER_MASTER_MODE(p)                          \
    if (TRUE == STKCTRL_TASK_AfterCscToMasterMode(p))   {       \
        return;                                                 \
    }

#if (STKCTRL_BACKDOOR_OPEN == 1)
	STKCTRL_BACKDOOR_GetTimeInfo(&ptime);
	memset(ptime, 0, sizeof(STKCTRL_BACKDOOR_CPNT_TIMER_T));
    ptime->is_master = TRUE;
#endif

    /* Slave not ready if I'm going to be SLAVE
     */
    STKTPLG_PMGR_SlaveReady(FALSE);

    printf("\r\nEntering master mode ...");

    /* Once CSCs have finished "enter master mode", SYS_CALLBACK will start
     * to process their callback event. (CSCs will send a de-queue event
     * to trigger SYS_CALLBACK to get callback event form CSCs' callback queue)
     * If SYS_CALLBACK don't enter master mode yet, de-queue event will be dropped.
     * So, SYS_CALLBACK should enter master mode first. To ensure that SYS_CALLBACK
     * will accept de-queue event.
     * When SYS_CALLBACK dispatch callback event to other CSCs, some CSCs possibly
     * still in transition mode. This callbck event will be dropped by these CSCs.
     * There is asynchornous until CSCs enter master mode. When CSCs enter master mode,
     * they will (must) update current status automatically.
     */

    /* handle direct-call part
     */
    DBGMSG("Enter master mode to SYS_CALLBACK");
    SYS_CALLBACK_INIT_EnterMasterMode();

    DBGMSG("Enter master mode to DRIVER_GROUP shared lib");
/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-08, 11:34:50 */
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
            UI32_T time1,time2;
            time1 = SYSFUN_GetSysTick();
#endif

    DRIVER_GROUP_EnterMasterMode();
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
    time2 = SYSFUN_GetSysTick()-time1;
    STKCTRL_TASK_BD_SetPerformaceMaxTime(MASTER_MODE,stkctrl_bd_driver_group_index,time2);
#endif

#endif

    /* handle message-sending part
     */

    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
		#endif

            sysfun_msg.cmd=SYS_TYPE_CMD_ENTER_MASTER_MODE;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            STKCTRL_TASK_BeforeCscToMasterMode(ptime, csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            AFTER_CSC_ENTER_MASTER_MODE(ptime);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
		#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()-time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(MASTER_MODE,idx, time2);
		#endif
        }
    }

#if (SYS_CPNT_CFGDB == TRUE)
    if (FALSE == CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_BOOT_REASON,
                                sizeof(UI32_T),
                                1,
                                &stkctrl_boot_reason_session_handler,
                                CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                                &need_to_sync))
    {
        printf("\r\nCFGDB failed to open boot reason session.\n");
    }
    else
    {
        if (need_to_sync)
        {
            if (FALSE == CFGDB_MGR_SyncSection(stkctrl_boot_reason_session_handler, &stkctrl_boot_reason))
            {
                printf("\r\nCFGDB failed to sync boot reason session.\n");
            }
        }
        else
        {
            if (FALSE == CFGDB_MGR_ReadSection(stkctrl_boot_reason_session_handler, &stkctrl_boot_reason))
            {
                printf("\r\nCFGDB failed to read boot reason session.\n");
            }

            if (TRUE == STKTPLG_OM_GetMyUnitID(&my_unit_id))
            {
                if (stkctrl_boot_reason == STKTPLG_BOOT_REASON_MAGIC_WORD)
                {
                    /* Warm restart because magic word has been identified.
                     */
                    STKTPLG_OM_SetUnitBootReason(my_unit_id, STKTPLG_OM_REBOOT_FROM_WARMSTART);
                }
                else
                {
                    /* Cold restart because there is no magic word matched.
                     */
                    STKTPLG_OM_SetUnitBootReason(my_unit_id, STKTPLG_OM_REBOOT_FROM_COLDSTART);
                }
            }
        } /* need_to_sync */
    } /* CFGDB_MGR_Open */

    CFGDB_MGR_EndOfOpen();
#endif

    /* set LED pattern
     */
#if 0 /*stktplg will light led,here pmgr to stktplg,it will delayed ,may be the role is changed,and in standalone ,it should off*/
    STKTPLG_PMGR_SetStackRoleLed(LEDDRV_STACK_MASTER);
#endif

    printf("\r\nFinished master mode ...\r\n");

    return;

} /* End of STKCTRL_TASK_EnterMasterMode */

/* FUNCTION NAME: STKCTRL_TASK_PreProvisionComplete
 * PURPOSE: To notify the CSCs that has pre-privision complete stage.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initially, this new phase "pre-provision complete" is added
 *          in order to speed up the vlan provision. In provision phase,
 *          vlan will only keep the configuration in OM and programing
 *          the configuration to ASIC in pre-provision complete phase.
 *          This mechanism can be used for other CSCs if required.
 */
static void STKCTRL_TASK_PreProvisionComplete(void)
{
    SYSFUN_Msg_T sysfun_msg;
    SYSFUN_MsgQ_T msgq_handle;

    if (STKTPLG_POM_IsStkTplgStateChanged(STKTPLG_OM_SIMPLE_STATE_MASTER))
    {
        printf("\r\nStop Entering Master mode due to Topology change.\r\n");
        return;
    }

    printf("\r\nPre-provision complete ...");

    /* handle direct-call part
     */

    /* handle message-sending part
     */
    DBGMSG("Send message to SWCTRL_GROUP");
    if(SYSFUN_GetMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
          SYSFUN_MSGQ_BIDIRECTIONAL,&msgq_handle)!=SYSFUN_OK)
    {
        printf("\r\nFailed to get SWCTRL_GROUP MsgQ.");
    }
    else
    {
        sysfun_msg.cmd=SYS_TYPE_CMD_PREPROVISION_COMPLETE;
        sysfun_msg.msg_size=0;
        SYSFUN_SendRequestMsg(msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
            SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
        DBGMSG("Finish sending message to SWCTRL_GROUP");
        SYSFUN_ReleaseMsgQ(msgq_handle);
    }
#if (SYS_CPNT_L2MCAST == TRUE)
    DBGMSG("Send message to MSL");
    if (SYSFUN_GetMsgQ(SYS_BLD_CSC_MSL_TASK_MSGK_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
                             &msgq_handle)!=SYSFUN_OK)
        printf("\r\nFailed to get MSL MsgQ.");
    else
    {
        SYSFUN_SendRequestMsg(msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
            SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
        DBGMSG("Finish sending message to MSL");
        SYSFUN_ReleaseMsgQ(msgq_handle);
    }
#endif
    printf("\r\nFinished pre-provision complete ...");

    return;
} /* End of STKCTRL_TASK_PreProvisionComplete */

/* FUNCTION NAME: STKCTRL_TASK_ProvisionComplete
 * PURPOSE: To notify all CSCs that provision has been completed.
 * INPUT:   unit   0 means all units
                    1-8 means unit id.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_ProvisionComplete(UI32_T unit)
{
    SYSFUN_Msg_T sysfun_msg;
    I16_T idx;
#if (SYS_CPNT_CFGDB == TRUE)
    UI32_T stkctrl_boot_reason = 0;
#endif

    /* question?
     * should CLI notify me the provision is completed even though he knows
     * he doesn't complete it
     *
     * My opinion, I shouldn't be called if provision is not completed due to any reason
     * "CLI provision has been completed" and "Notify provision complete" shouldn't appear neither.
     *
     * I can take care of it temporarily. Any decision, let PL to make it.
     */
    if (STKTPLG_POM_IsStkTplgStateChanged(STKTPLG_OM_SIMPLE_STATE_MASTER))
    {
        printf("\r\nStop Entering Master mode due to Topology change.\r\n");
        return;
    }

    printf("\r\nProvision complete ...");

#if (SYS_CPNT_CFGDB == TRUE)
    /* Clear boot reason area in CFGDB after restart.
     */
    stkctrl_boot_reason = 0;
    if (FALSE == CFGDB_MGR_WriteSection(stkctrl_boot_reason_session_handler, &stkctrl_boot_reason))
    {
        printf("\r\nCFGDB failed to reset boot reason session.\n");
    }
#endif

#if (SYS_CPNT_RUNTIME_D_CACHE == FALSE)
    SYSFUN_DisableDataCache();
#endif

    /* handle direct-call part
     */
    /* function call by stkctrl, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-08, 12:00:35 */
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
        UI32_T time1,time2;
        time1 = SYSFUN_GetSysTick();
#endif

    DRIVER_GROUP_ProvisionComplete(unit);
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
    time2 = SYSFUN_GetSysTick()-time1+1;
    STKCTRL_TASK_BD_SetPerformaceMaxTime(PROVISION_COMPLETED_MODE,stkctrl_bd_driver_group_index,time2);
#endif

#endif

    /* handle message-sending part
     */
    if(unit !=0)
     sysfun_msg.cmd=SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE;
    else
     sysfun_msg.cmd=SYS_TYPE_CMD_PROVISION_COMPLETE;

    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
#endif
            sysfun_msg.msg_size=0;

            DBGMSG("Send ProvisionComplete  message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish ProvisionComplete sending message to %s", csc_group_msgq_info[idx].csc_group_name);
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()-time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(PROVISION_COMPLETED_MODE,idx, time2);
#endif
        }
    }
    if(unit !=0)
     printf("\r\nFinished module %d provision complete ...",unit);
    else
     printf("\r\nFinished provision complete ...");

    /* For master to notify slave provision has been completed
     */
     STKTPLG_PMGR_ProvisionCompleted(TRUE);

    /* For CLI to know when to show login prompt.
     */
    CLI_PMGR_AllCscsKnowProvisionComplete();

    return;
} /* End of STKCTRL_TASK_ProvisionComplete */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) /* Do not support module with CPU on linux platform, need not to port STKCTRL_TASK_HandleHotInsertion() and STKCTRL_TASK_HandleHotRemoval() */
/* FUNCTION NAME: STKCTRL_TASK_HandleHotInsertion
 * PURPOSE: To handle the module hot insertion process.
 * INPUT:   starting_port_ifindex -- The starting port ifindex on the inserted module.
 *          number_of_port        -- The port number on the inserted module.
 *          use_default           -- Use default setting to apply to the inserted port.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_HandleHotInsertion( UI8_T unit_id,
                                            UI32_T starting_port_ifindex,
                                            UI32_T number_of_port,
                                            BOOL_T use_default)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_TYPE_HandleHotSwapArg_T *handle_hot_insertion_arg_p;
    I16_T idx;
    UI8_T ipcmsgbuf[SYSFUN_SIZE_OF_MSG(sizeof(SYS_TYPE_HandleHotSwapArg_T))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsgbuf;
    handle_hot_insertion_arg_p = (SYS_TYPE_HandleHotSwapArg_T*)sysfun_msg_p->msg_buf;

    printf("\r\nProcessing hot swap insertion: %lu-%lu, and use default [%s]\r\n", starting_port_ifindex, starting_port_ifindex+number_of_port-1, use_default? "TRUE":"FALSE");
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
            UI32_T time1,time2;
            time1 = SYSFUN_GetSysTick();
#endif
	/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
    DRIVER_GROUP_HandleHotInsertion(unit_id, starting_port_ifindex, number_of_port);

#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
        time2 = SYSFUN_GetSysTick()-time1;
        STKCTRL_TASK_BD_SetPerformaceMaxTime(HOTADD_MODE,stkctrl_bd_driver_group_index,time2);
#endif

    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
    {
        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
		#endif
            sysfun_msg_p->cmd=SYS_TYPE_CMD_HANDLE_HOT_INSERTION;
            sysfun_msg_p->msg_size=sizeof(SYS_TYPE_HandleHotSwapArg_T);
            handle_hot_insertion_arg_p->csc_subset_identifier = csc_group_msgq_info[idx].csc_subset_identifier;
            handle_hot_insertion_arg_p->starting_port_ifindex=starting_port_ifindex;
            handle_hot_insertion_arg_p->number_of_port=number_of_port;
            handle_hot_insertion_arg_p->unit_id=unit_id;
            handle_hot_insertion_arg_p->use_default=use_default;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, sysfun_msg_p);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()- time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(HOTADD_MODE,idx, time2);
		#endif
        }
    }

    printf("\r\nHot swap insertion done.\r\n");

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
    {
        TRAP_EVENT_TrapData_T     trap_data;

        trap_data.trap_type = TRAP_EVENT_MODULE_INSERTION;
        trap_data.community_specified = FALSE;
        trap_data.u.module_insertion.instance_swModuleOpCodeVer[0] = 1;
        trap_data.u.module_insertion.instance_swModuleOpCodeVer[1] = 1;
        trap_data.u.module_insertion.swModuleOpCodeVer[0] = 0;

        TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }
#endif
}

/* FUNCTION NAME: STKCTRL_TASK_HandleHotRemoval
 * PURPOSE: To handle the module hot removal process.
 *          starting_port_ifindex -- The starting port ifindex on the removed module.
 *          number_of_port        -- The port number on the removed module.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_HandleHotRemoval(UI8_T unit_id,
                                          UI32_T starting_port_ifindex,
                                          UI32_T number_of_port)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_TYPE_HandleHotSwapArg_T *handle_hot_removal_arg_p;
    I16_T idx;
    UI8_T ipcmsgbuf[SYSFUN_SIZE_OF_MSG(sizeof(SYS_TYPE_HandleHotSwapArg_T))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsgbuf;
    handle_hot_removal_arg_p = (SYS_TYPE_HandleHotSwapArg_T*)sysfun_msg_p->msg_buf;

    printf("\r\nProcessing hot swap removal: %lu-%lu\r\n", starting_port_ifindex, starting_port_ifindex+number_of_port-1);

    for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		UI32_T time1,time2;
		time1 = SYSFUN_GetSysTick();
		#endif
            sysfun_msg_p->cmd=SYS_TYPE_CMD_HANDLE_HOT_REMOVAL;
            sysfun_msg_p->msg_size=sizeof(SYS_TYPE_HandleHotSwapArg_T);
            handle_hot_removal_arg_p->starting_port_ifindex=starting_port_ifindex;
            handle_hot_removal_arg_p->unit_id=unit_id;
            handle_hot_removal_arg_p->number_of_port=number_of_port;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, sysfun_msg_p);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
        #if (STKCTRL_DEBUG_PERFORMACE == TRUE)
		time2 = SYSFUN_GetSysTick()-time1+1;
		STKCTRL_TASK_BD_SetPerformaceMaxTime(HOTREMOVE_MODE,idx, time2);
		#endif
        }
    }
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
            UI32_T time1,time2;
            time1 = SYSFUN_GetSysTick();
#endif
	/* function call by stkctrl, anzhen.zheng, 6/26/2008 */
    DRIVER_GROUP_HandleHotRemoval(unit_id, starting_port_ifindex, number_of_port);

#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
            time2 = SYSFUN_GetSysTick()-time1+1;
            STKCTRL_TASK_BD_SetPerformaceMaxTime(HOTREMOVE_MODE,stkctrl_bd_driver_group_index,time2);
#endif


    printf("Hot swap removal done.\r\n");

#if (SYS_CPNT_STKTPLG_WITH_HOT_SWAP == TRUE)
    {
        TRAP_EVENT_TrapData_T     trap_data;

        trap_data.trap_type = TRAP_EVENT_MODULE_REMOVAL;
        trap_data.community_specified = FALSE;
        trap_data.u.module_removal.instance_swModuleOpCodeVer[0] = 1;
        trap_data.u.module_removal.instance_swModuleOpCodeVer[1] = 1;
        trap_data.u.module_removal.swModuleOpCodeVer[0] = 0;

        TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }
#endif

}
#endif

/* FUNCTION NAME: STKCTRL_TASK_ProcessModuleStateChange
 * PURPOSE: The process the module state change.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Process one module per call.
 */
static void STKCTRL_TASK_ProcessModuleStateChange()
{
    UI32_T unit_id;
    UI32_T starting_port;
    UI32_T number_of_port;
    BOOL_T use_default;
    BOOL_T is_insertion;
    BOOL_T is_got;

    is_got = FALSE;
    for (unit_id=1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (ctrl_info.module_operational_status[unit_id-1] == STKCTRL_TASK_MODULE_STATUS_DIRTY)
        {
            ctrl_info.processing_module_unit_id = unit_id;
            is_got = TRUE;
            break;
        }
    }

    if (is_got == TRUE)
    {
        if (TRUE == STKTPLG_PMGR_SyncModuleProcess(unit_id, &is_insertion, &starting_port, &number_of_port, &use_default))
        {
            UI32_T starting_ifindex;

            starting_ifindex = ((unit_id-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + starting_port);

            if (TRUE == is_insertion)
            {
                RUN_MODULE_STATE_MACHINE(unit_id, STKCTRL_TASK_MODULE_EVENT_PROCESS);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#if 0 /* Do not support module with CPU on linux platform, need not to port STKCTRL_TASK_HandleHotInsertion() */
                STKCTRL_TASK_HandleHotInsertion(starting_ifindex, number_of_port, use_default);
#endif
#endif

                /* insertion need to wait for provision done.
                 */
                ctrl_info.is_current_module_process_done = FALSE;
            }
            else
            {
                RUN_MODULE_STATE_MACHINE(unit_id, STKCTRL_TASK_MODULE_EVENT_PROCESS);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#if 0 /* Do not support module with CPU on linux platform, need not to port STKCTRL_TASK_HandleHotRemoval() */
                STKCTRL_TASK_HandleHotRemoval(starting_ifindex, number_of_port);
#endif
#endif
                RUN_MODULE_STATE_MACHINE(unit_id, STKCTRL_TASK_MODULE_EVENT_DONE);

                /* removal not necessary to wait for provision.
                 */
                ctrl_info.is_current_module_process_done = TRUE;
            }
        }
        else
        {
            /* if dirty but, nothing to do, should regression to none.
             * and mark as not necessary to wait for provision.
             */
            RUN_MODULE_STATE_MACHINE(unit_id, STKCTRL_TASK_MODULE_EVENT_REGRESSION);

            ctrl_info.is_current_module_process_done = TRUE;
        }
    }

    return;
}


/* FUNCTION NAME: STKCTRL_TASK_IsAnyModuleNeedToProcess
 * PURPOSE: To check if there still any module state changed has not been processed yet.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static BOOL_T STKCTRL_TASK_IsAnyModuleNeedToProcess(void)
{
    UI32_T unit_id;

    for(unit_id=1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (ctrl_info.module_operational_status[unit_id-1] == STKCTRL_TASK_MODULE_STATUS_DIRTY)
        {
            return TRUE;
        }
    }
    return FALSE;
}



/* FUNCTION NAME: STKCTRL_TASK_EnterTransitionMasterMode
 * PURPOSE: To make system go to transition mode and then enter master mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void STKCTRL_TASK_EnterTransitionMasterMode(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    STKTPLG_PMGR_SetTransitionMode();
#else
    STKCTRL_TASK_EnterTransitionMode();
    STKCTRL_TASK_EnterMasterMode();
#endif
    return;
}

/* FUNCTION NAME: STKCTRL_TASK_ModuleStateMachine
 * PURPOSE: To run state machine of the module status.
 * INPUT:   current_status -- Current status of the module.
 *          event          -- Occured event.
 * OUTPUT:  None.
 * RETUEN:  The result of the state machine.
 * NOTES:   1) This is pure code.
 *          2) Two thread will trigger this state machine
 *             a. STKTPLG.
 *             b. STKCTRL.
 */
static UI32_T STKCTRL_TASK_ModuleStateMachine(UI32_T current_status, UI32_T event)
{
    UI32_T next_status;

    switch(current_status)
    {
    case STKCTRL_TASK_MODULE_STATUS_NONE:
        switch(event)
        {
        case STKCTRL_TASK_MODULE_EVENT_TO_DIRTY:
            next_status = STKCTRL_TASK_MODULE_STATUS_DIRTY;
            break;

        case STKCTRL_TASK_MODULE_EVENT_REGRESSION:
            next_status = STKCTRL_TASK_MODULE_STATUS_NONE;
            break;

        default:
            next_status = current_status;
            break;
        }
        break;

    case STKCTRL_TASK_MODULE_STATUS_BUSY:
        switch (event)
        {
        case STKCTRL_TASK_MODULE_EVENT_TO_DIRTY:
            next_status = STKCTRL_TASK_MODULE_STATUS_DIRTY;
            break;

        case STKCTRL_TASK_MODULE_EVENT_DONE:
            next_status = STKCTRL_TASK_MODULE_STATUS_NONE;
            break;

        case STKCTRL_TASK_MODULE_EVENT_REGRESSION:
            next_status = STKCTRL_TASK_MODULE_STATUS_NONE;
            break;

        default:
            next_status = current_status;
            break;
        }
        break;

    case STKCTRL_TASK_MODULE_STATUS_DIRTY:
        switch (event)
        {
        case STKCTRL_TASK_MODULE_EVENT_TO_DIRTY:
            next_status = STKCTRL_TASK_MODULE_STATUS_DIRTY;
            break;

        case STKCTRL_TASK_MODULE_EVENT_DONE:
            next_status = STKCTRL_TASK_MODULE_STATUS_DIRTY;
            break;

        case STKCTRL_TASK_MODULE_EVENT_PROCESS:
            next_status = STKCTRL_TASK_MODULE_STATUS_BUSY;
            break;

        case STKCTRL_TASK_MODULE_EVENT_REGRESSION:
            next_status = STKCTRL_TASK_MODULE_STATUS_NONE;
            break;

        default:
            next_status = current_status;
            break;
        }
        break;

    default:
        next_status = current_status;
        break;
    }

    return next_status;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_TASK_ReloadSystemEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will restart system from POST loader.
 *            And it will let the loader know this is warm start and
 *            need to run diagnostic.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static void STKCTRL_TASK_ReloadSystemEvent(void)
{
    I16_T idx;
    SYSFUN_Msg_T sysfun_msg;

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    /*  Extract Method for the procedure of waiting DBSync & XFER tasks done.
     */
    if (STKCTRL_TASK_WaitForDBSyncAndXferDone() == FALSE)
    {
        DBGMSG("Can't Reload and dead in STKCTRL_TASK.\r\n");
    }
#endif

#if (SYS_CPNT_CFGDB == TRUE)
    /* Charles move from sysdrv.c for CFGDB new design
     */
    CFGDB_MGR_Shutdown();
#endif
     /*add by fen.wang, notify every csc to prepare reload*/

    SYSDRV_PrepareStartSystem(SYS_VAL_WARM_START_FOR_RELOAD);

     for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {

            sysfun_msg.cmd=SYS_TYPE_CMD_RELOAD_SYSTEM;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
        }
    }

    SYS_CALLBACK_MGR_SavingConfigStatusCallBack(SYS_MODULE_STKCTRL, TRUE);

    /* Restart system, including all stacking units, if necessary.
     */
    SYSDRV_ReloadSystem();

    return ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_TASK_WarmStartSystemEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will restart system from POST loader.
 *            And it will let the loader know this is warm start.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static void STKCTRL_TASK_WarmStartSystemEvent(void)
{
    I16_T idx;
    SYSFUN_Msg_T sysfun_msg;
#if (SYS_CPNT_CFGDB == TRUE)
    UI32_T boot_reason = 0;
#endif


#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
    /* For software restart, turn off password recovery flag.
     */
    USERAUTH_SetPasswordRecoveryActive(FALSE);
#endif

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    /*  Extract Method for the procedure of waiting DBSync & XFER tasks done.
     */
    if (STKCTRL_TASK_WaitForDBSyncAndXferDone() == FALSE)
    {
        DBGMSG("Can't Reload and dead in STKCTRL_TASK.\r\n");
    }
#endif

#if (SYS_CPNT_CFGDB == TRUE)
    /* Write magic word to CFBDB for warm restart identification.
     */
    boot_reason = STKTPLG_BOOT_REASON_MAGIC_WORD;
    if (FALSE == CFGDB_MGR_WriteSection(stkctrl_boot_reason_session_handler, &boot_reason))
    {
        printf("\r\n%s: CFGDB failed to update boot reason session.\n");
    }

    /* Charles move from sysdrv.c for CFGDB new design
     */
    CFGDB_MGR_Shutdown();
#endif

     SYSDRV_PrepareStartSystem(SYS_VAL_WARM_START_FOR_RELOAD);
     /*add by fen.wang, notify every csc to prepare reload*/
     for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {

            sysfun_msg.cmd=SYS_TYPE_CMD_RELOAD_SYSTEM;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
        }
    }
    /* Restart system, including all stacking units, if necessary.
     */
    SYSDRV_WarmStartSystem();

    return ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_TASK_ColdStartSystemEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will restart system from POST loader.
 *            And it will let the loader know this is cold start and
 *            need to run diagnostic.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static void STKCTRL_TASK_ColdStartSystemEvent(void)
{
    I16_T idx;
    SYSFUN_Msg_T sysfun_msg;

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
    /* For software restart, turn off password recovery flag.
     */
    USERAUTH_SetPasswordRecoveryActive(FALSE);
#endif

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    /*  Extract Method for the procedure of waiting DBSync & XFER tasks done.
     */
    if (STKCTRL_TASK_WaitForDBSyncAndXferDone() == FALSE)
    {
        DBGMSG("Can't Reload and dead in STKCTRL_TASK.\r\n");
    }
#endif

#if (SYS_CPNT_CFGDB == TRUE)
    /* Charles move from sysdrv.c for CFGDB new design
     */
    CFGDB_MGR_Shutdown();
#endif
 /*add by fen.wang, notify every csc to prepare reload*/
     SYSDRV_PrepareStartSystem(SYS_VAL_COLD_START_FOR_RELOAD);

     for(idx=(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]))-1; idx>=0;idx--)
    {
        /* Currently, CSC subset identifier only used in Hot Insertion
         */
        if (csc_group_msgq_info[idx].csc_subset_identifier != 0)
            continue;

        if(csc_group_msgq_info[idx].msgq_handle!=L_CVRT_UINT_TO_PTR(STKCTRL_TASK_INVALID_MSGQ_HANDLE))
        {

            sysfun_msg.cmd=SYS_TYPE_CMD_RELOAD_SYSTEM;
            sysfun_msg.msg_size=0;

            DBGMSG("Send message to %s", csc_group_msgq_info[idx].csc_group_name);
            SYSFUN_SendRequestMsg(csc_group_msgq_info[idx].msgq_handle, &sysfun_msg, SYSFUN_TIMEOUT_WAIT_FOREVER,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, &sysfun_msg);
            DBGMSG("Finish sending message to %s", csc_group_msgq_info[idx].csc_group_name);
        }
    }

    /* Restart system, including all stacking units, if necessary.
     */
    SYSDRV_ColdStartSystem();

    return ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_TASK_UnitIDReNumberingEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is to result STKTPLG TCN and reassigned unit
 *            ID to all units based on the stacking spec.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : Renumbering must be not standalone or unit ID not equal
 *            to 1.
 * ---------------------------------------------------------------------
 */
static void STKCTRL_TASK_UnitIDReNumberingEvent(void)
{

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
    /* For software restart, turn off password recovery flag.
     */
    USERAUTH_SetPasswordRecoveryActive(FALSE);
#endif

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    /*  Extract Method for the procedure of waiting DBSync & XFER tasks done.
     */
    if (STKCTRL_TASK_WaitForDBSyncAndXferDone() == FALSE)
    {
        DBGMSG("Can't Reload and dead in STKCTRL_TASK.\r\n");
    }
#endif

#if (SYS_CPNT_CFGDB == TRUE)
    /* Charles move from sysdrv.c for CFGDB new design
     */
    CFGDB_MGR_Shutdown();
#endif

    SYS_CALLBACK_MGR_SavingConfigStatusCallBack(SYS_MODULE_STKCTRL, TRUE);

    STKTPLG_PMGR_UnitIDReNumbering();

    return ;
}

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_TASK_WaitForDBSyncAndXferDone
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will check had DBSync task and
 *            XFER task done.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  : Succeed
 *            FALSE : Failed
 * NOTES    : False return when we can't get the stack topology
 * ---------------------------------------------------------------------
 */
static BOOL_T STKCTRL_TASK_WaitForDBSyncAndXferDone(void)
{
    /* For software restart, force save running config.
     */
    DBSYNC_TXT_MGR_Set_FlushAndDisable(TRUE);

    while (TRUE)
    {

        /* Stop waiting if DIRTY CLEAN and AUTO SAVE DONE.
         */
        if (   DBSYNC_TXT_MGR_Get_IsAllDirtyClear() == TRUE
            && DBSYNC_TXT_MGR_Get_IsDoingAutosave() == FALSE)
        {
            break;
        }

        SYSFUN_Sleep(10);
    }

    return TRUE;
}
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKCTRL_TASK_UnitHotInsertRemove_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   msg     -- unit hot insert/remove
 * OUTPUT:  events  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_UnitHotInsertRemove_CallBack(UI32_T *events_p,UI32_T *unit)
{
    if(events_p==NULL)
    {
        printf("\r\n%s:invalid argument", __FUNCTION__);
        return;
    }

     /* notify stack control task that topology state is changed.
     * with event, it is possible that we will overwrite some of them.
     * but it is fine for us, we only need the final state of stack topology.
     */
    *events_p=STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE;
    STKCTRL_TASK_HandleEvents(events_p,unit);

    return;
}

/* FUNCTION NAME: STKCTRL_TASK_ProcessUnitInsertRemove
 * PURPOSE: Process one unit insertion/removal
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  - No more units to be processed
 *          FASLE - not done;
 * NOTES:
 */
static BOOL_T STKCTRL_TASK_ProcessUnitInsertRemove(UI32_T *unit)
{
    UI32_T starting_port;
    UI32_T number_of_port;
    UI8_T unit_id;
    BOOL_T use_default;
    BOOL_T is_insertion;
    BOOL_T ret;

       ret = STKTPLG_PMGR_ProcessUnitInsertRemove(&is_insertion, &starting_port, &number_of_port, &unit_id,&use_default);

        if (FALSE == ret) /*has unit to insert/remove*/
        {
            if(unit)
             *unit = (UI32_T)unit_id;

            if (TRUE == is_insertion) /*unit to insert*/
            {
               STKCTRL_TASK_HandleHotInsertion( unit_id,starting_port, number_of_port,use_default);
               ctrl_info.is_current_module_process_done = FALSE;
            }
            else/*unit to remove*/
            {
               STKCTRL_TASK_HandleHotRemoval(unit_id,starting_port,number_of_port);
            }
        }
        return ret;

}
#endif

#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
void STKCTRL_TASK_BD_ShowTranstionPerformace()
{
    UI8_T idx;
	printf("\r\n==========TRANSITION===========\n");
    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
      printf("\r\n%20s()   time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[TRANSITION_MODE][idx]);

     printf("\r\n%20s()   time %lu", "Driver_Group",stkctrl_bd_max_time[TRANSITION_MODE][idx]);
    printf("\r\n====================================================\n");

}

void STKCTRL_TASK_BD_ShowSlavePerformace()
{
    UI8_T idx;
	printf("\r\n==========SLAVE===========\n");
    for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
       printf("\r\n%20s() time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[SLAVE_MODE][idx]);

    printf("\r\n%20s() time %lu", "Driver_Group",stkctrl_bd_max_time[SLAVE_MODE][idx]);
    printf("\r\n====================================================\n");

}

 void STKCTRL_TASK_BD_ShowMasterPerformace()
{
	 UI8_T idx;
	printf("\r\n==========MASTER===========\n");
	for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
		printf("\r\n%20s()     time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[MASTER_MODE][idx]);

	printf("\r\n%20s()     time %lu","Driver_Group",stkctrl_bd_max_time[MASTER_MODE][idx]);
	printf("\r\n====================================================\n");


}

  void STKCTRL_TASK_BD_ShowProvisionPerformace()
 {
      UI8_T idx;
     printf("\r\n==========Provision===========\n");
     for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
         printf("\r\n%20s()     time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[PROVISION_COMPLETED_MODE][idx]);

     printf("\r\n%20s()     time %lu", "Driver_Group",stkctrl_bd_max_time[PROVISION_COMPLETED_MODE][idx]);
     printf("\r\n====================================================\n");


 }

  void STKCTRL_TASK_BD_ShowHotAddPerformace()
 {
      UI8_T idx;
     printf("\r\n==========HotAdd===========");
     for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
         printf("\r\n%20s()     time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[HOTADD_MODE][idx]);

     printf("\r\n%20s()     time %lu", "Driver_Group",stkctrl_bd_max_time[HOTADD_MODE][idx]);
     printf("\r\n====================================================\n");


 }
   void STKCTRL_TASK_BD_ShowHotRemovePerformace()
  {
       UI8_T idx;
      printf("\r\n=========HOT REMOVE===========\n");
      for(idx=0; idx<(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0]));idx++)
          printf("\r\n%20s()     time %lu", csc_group_msgq_info[idx].csc_group_name,stkctrl_bd_max_time[HOTREMOVE_MODE][idx]);

      printf("\r\n%20s()     time %lu", "Driver_Group",stkctrl_bd_max_time[HOTREMOVE_MODE][idx]);
      printf("\r\n====================================================\n");


  }

 void STKCTRL_TASK_BD_ClearPerformaceDB()
 {
   memset(stkctrl_bd_max_time,0,sizeof(stkctrl_bd_max_time));
 }
 void STKCTRL_TASK_BD_SetPerformaceMaxTime(UI8_T type,UI16_T index,UI32_T time)
{
     if(type >=MAX_NUM || index >(sizeof(csc_group_msgq_info)/sizeof(csc_group_msgq_info[0])))
      return;

	 if(stkctrl_bd_max_time[type][index]< time)
	  stkctrl_bd_max_time[type][index] = time;

}

#endif

