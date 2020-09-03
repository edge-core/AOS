/* Module Name: SW_WATCHDOG_MGR.H
 * Purpose: This moudle is responsible for (software) watchdog monitor procedure.
 * Notes:
 * History:
 *      31/May/2010    -- First Draft created by Aken Liu
 *       9/Feb/2011    -- Porting to EC platform by Charlie Chen
 *
 * Copyright(C)      Accton Corporation, 2001
 * Copyright(C)      Edge-Core Networks, 2011
 */
#ifndef _SW_WATCHDOG_MGR_H
#define _SW_WATCHDOG_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

typedef enum SW_WatchDogMonitorId_E
{
    /*CORE_UTIL_PROC*/
    SW_WATCHDOG_UTILITY_SYSLOG,             /*SYSLOG_TASK_Main*/
    SW_WATCHDOG_UTILITY_GROUP,              /*UTILITY_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_CFGDB_GROUP,                /*CFGDB_GROUP_Mgr_Thread_Function_Entry*/
    /* CLI_PROC */
    SW_WATCHDOG_CLI_GROUP,                  /*CLI_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_CLI,                        /*CLI_TASK_Main (UART Session)*/
    SW_WATCHDOG_TELNET_PARENT,              /*TNSHD_ParentTask*/
    SW_WATCHDOG_TELNET_SERVER,              /*tnpd_task*/
    SW_WATCHDOG_TELNET_GROUP,               /*TELNET_GROUP_Mgr_Thread_Function_Entry */
    SW_WATCHDOG_SSH_GROUP,                  /*SSH_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_SSH_PARENT,                 /*SSHD_TASK_Main*/
    /* SNMP_PROC */
    SW_WATCHDOG_SNMP,                       /*SNMP_TASK_Body*/
    SW_WATCHDOG_SNMP_GROUP,                 /*SNMP_GROUP_Mgr_Thread_Function_Entry*/
    /*IP_SERVICE_PROC*/
    SW_WATCHDOG_IP_SERVICE_GROUP,           /*IP_SERVICE_GROUP_Create_All_Threads*/
    /*AUTH_PROTOCOL_PROC*/
    SW_WATCHDOG_RADIUS,                     /*RADIUS_TASK_Body*/
    SW_WATCHDOG_TACACS,                     /*TACACS_TASK_Body*/
    SW_WATCHDOG_AUTH_PROTOCOL_GROUP,        /*AUTH_PROTOCOL_GROUP_Mgr_Thread_Function_Entry*/
    /* XFER_PROC */
    SW_WATCHDOG_XFER,                       /*XFER_MGR_Main*/
    SW_WATCHDOG_XFER_DNLD,                  /*XFER_DNLD_Tftp_Main*/
    SW_WATCHDOG_XFER_GROUP,                 /*XFER_GROUP_Mgr_Thread_Function_Entry*/
    /* L2_L4_PROC */
    SW_WATCHDOG_L2MUX_GROUP,                /*L2MUX_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_IML_RX,                     /*IML_MGR_RxTaskMain*/
    SW_WATCHDOG_SWCTRL,                     /*SWCTRL_TASK_Main*/
    SW_WATCHDOG_SWCTRL_GROUP,               /*SWCTRL_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_LACP_GROUP,                 /*LACP_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_NETACCESS_GROUP,            /*NETACCESS_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_NETACCESS_NMTR,             /*NETACCESS_NMTR_Thread_Function_Entry*/
    SW_WATCHDOG_NETACCESS_NMTR_HASH2HISAM,  /*NETACCESS_GROUP_Mgr_Thread_Hash2hisam_Function_Entry*/
    SW_WATCHDOG_NETACCESS_PSEC,             /*PSEC_TASK_Main*/
    SW_WATCHDOG_STA_GROUP,                  /*STA_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_L4_GROUP,                   /*L4_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_L2MCAST_GROUP ,             /*L2MCAST_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_CFM_GROUP,                  /*CFM_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_CMGR_GROUP,                 /*CMGR_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_DHCPSNP_GROUP,              /*DHCPSNP_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_DHCPSNP_CSC,                /*DHCPSNP_TASK_TaskMain*/
    SW_WATCHDOG_DHCPV6SNP_CSC,              /*DHCPV6SNP_TASK_TaskMain*/
    SW_WATCHDOG_DCB_GROUP,                  /*DCB_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_MLAG_GROUP,                 /*MLAG_GROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_VXLAN_GROUP,                /*VXLAN_GROUP_Mgr_Thread_Function_Entry*/
    /*STKCTRL_PROC*/
    SW_WATCHDOG_STKCTRL_GROUP,              /*STKCTRL_GROUP_Main_Thread_Function_Entry*/
    /*STKTPLG_PROC*/
    SW_WATCHDOG_STKTPLG_GROUP,              /*STKTPLG_GROUP_Mgr_Thread_Function_Entry*/
    /*SYS_MGMT_PROC*/
    SW_WATCHDOG_SYS_MGMT_GROUP,             /*SYS_MGMT_GROUP_Mgr_Thread_Function_Entry*/
    /*APP_PROTOCOL_PROC*/
    SW_WATCHDOG_APP_PROTOCOL_SNTP,          /*SNTP_TASK_TaskMain*/
    SW_WATCHDOG_PING,                       /*PING_TASK_TaskMain*/
    SW_WATCHDOG_APP_PROTOCOL_GROUP,         /*APP_PROTOCOL_GROUP_Mgr_Thread_Function_Entry*/
    /*NETCFG_PROC*/
    SW_WATCHDOG_NETCFG_GROUP,               /*NETCFG_GROUP_Mgr_Thread_Function_Entry*/
    /*NSM_PROC*/
    SW_WATCHDOG_NSM_GROUP,                  /*NSM_GROUP_Mgr_Thread_Function_Entry*/
    /* DRIVER_PROC */
    SW_WATCHDOG_DEV_NICDRV,                 /*DEV_NICDRV_Task*/
    SW_WATCHDOG_SWDRV,                      /*SWDRV_TASK_Main*/
    SW_WATCHDOG_SWDRV_SFP,                  /*SWDRV_TASK_SFP*/
    SW_WATCHDOG_AMTRDRV,                    /*AMTRDRV_ASIC_COMMAND_TASK_Main*/
    SW_WATCHDOG_SYSDRV,                     /*SYSDRV_TASK_TaskMain*/
    SW_WATCHDOG_NMTRDRV,                    /*NMTRDRV_TASK_Main*/
    SW_WATCHDOG_SYS_TIME,                   /*SYS_TIME_TASK_CreateTask*/
    SW_WATCHDOG_FLASHDRV,                   /*FS_TASK_Main*/
    SW_WATCHDOG_LEDDRV,                     /*LEDDRV_INIT_CreateTasks*/
    SW_WATCHDOG_DRIVER_GROUP,               /*DRIVER_GROUP_Mgr_Thread_Function_Entry*/
    /* WEB_PROC */
    SW_WATCHDOG_WEB_GROUP,                  /*WEBGROUP_Mgr_Thread_Function_Entry*/
    SW_WATCHDOG_HTTP,                       /*HTTP_TASK_Main*/
    /* SYS_CALLBACK_PROC */
    SW_WATCHDOG_SYS_CALLBACK_GROUP,         /*SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry */
    SW_WATCHDOG_MAX_MONITOR_ID
} SW_WatchDogMonitorId_T;

#define  SW_WATCHDOG_STATUS_ENABLE     1
#define  SW_WATCHDOG_STATUS_DISABLE    0

/* enum debug message level
 */
enum {
    SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG = 0,
    SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG,
    SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_IMPORTANT_MSG,
    SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME - SW_WATCHDOG_MGR_SetMonitorStatus
 * PURPOSE  : This function is for superuser to enable/disable software watchdog
 * INPUT    : monitor_status -- enable/disable
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
BOOL_T SW_WATCHDOG_MGR_SetMonitorStatus(BOOL_T monitor_status);

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetMonitorStatus
 * PURPOSE  : This function is for superuser get the status of software watchdog
 * INPUT    : monitor_status -- enable/disable
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
BOOL_T SW_WATCHDOG_MGR_GetMonitorStatus(BOOL_T *monitor_status);

/* FUNCTION NAME - SW_WATCHDOG_MGR_ResetTimer
 * PURPOSE  : This function is for each monitored thread to reset its monitor timer
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_ResetTimer(UI32_T monitor_id);

/* FUNCTION NAME - SW_WATCHDOG_MGR_RegisterMonitorThread
 * PURPOSE  : This function is for each monitored thread to register
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if the expired_time equal to 0, it means disable to monitor this thread
 */
void SW_WATCHDOG_MGR_RegisterMonitorThread(UI32_T monitor_id, UI32_T thread_id, UI32_T expired_timer);

/* FUNCTION NAME - SW_WATCHDOG_MGR_UnregisterMonitorThread
 * PURPOSE  : This function is for unregist the monitored thread
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 */
void SW_WATCHDOG_MGR_UnregisterMonitorThread(UI32_T monitor_id);


/* FUNCTION NAME - SW_WATCHDOG_MGR_HandleTimerEvents
 * PURPOSE  : This function is software watchdog main routine.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_HandleTimerEvents(void);

/* FUNCTION NAME - SW_WATCHDOG_MGR_Initiate_System_Resources
 * PURPOSE  : This function is used to initiate the system resources
 *            of software watchdog
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_InitiateSystemResources(void);

void SW_WATCHDOG_MGR_AttachSystemResources(void);

void SW_WATCHDOG_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME - SW_WATCHDOG_MGR_SetDebugMsgLevel
 * PURPOSE  : Set the status of software watchdog debug message level
 * INPUT    : debug_msg_level -- SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_XXX
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : When debug_msg_level == SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG
 *            no debug message will be output to console.
 */
void SW_WATCHDOG_MGR_SetDebugMsgLevel(UI8_T debug_msg_level);

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetDebugMsgLevel
 * PURPOSE  : Get the status of software watchdog debug message level
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug message level
 * NOTES    : When debug_msg_level == SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG
 *            no debug message will be output to console.
 */
UI8_T SW_WATCHDOG_MGR_GetDebugMsgLevel(void);

/* FUNCTION NAME - SW_WATCHDOG_MGR_SetStopRebootStatus
 * PURPOSE  : Set the status of stop reboot. If TRUE is set,
 *            software watchdog will not reboot when a software watchdog
 *            timeout occurs.
 * INPUT    : is_stop_reboot - the value of stop_reboot
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This is for debug only
 */
void SW_WATCHDOG_MGR_SetStopRebootStatus(BOOL_T is_stop_reboot);

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetStopRebootStatus
 * PURPOSE  : Get the status of stop reboot.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : stop_reboot status
 * NOTES    : This is for debug only
 */
BOOL_T SW_WATCHDOG_MGR_GetStopRebootStatus(void);

/* FUNCTION NAME - SW_WATCHDOG_MGR_DumpMonitoredThread
 * PURPOSE  : List the status of all monitored thread
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_DumpMonitoredThread(void);

#endif
