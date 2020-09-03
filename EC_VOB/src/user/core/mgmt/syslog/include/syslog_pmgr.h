/* MODULE NAME:  SYSLOG_pmgr.h
 * PURPOSE:
 *    PMGR implement for SYSLOG.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    07/30/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYSLOG_PMGR_H
#define SYSLOG_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "syslog_mgr.h"
#include "syslog_om.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_InitiateProcessResource(void);

/* FUNCTION NAME: SYSLOG_PMGR_AddEntrySync
 * PURPOSE: Add a log message to system log module synchrously.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the entry will be written to flash, it
 *          will be done after calling this function.
 */
BOOL_T SYSLOG_PMGR_AddEntrySync(SYSLOG_OM_Record_T *syslog_entry);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_AddEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_AddEntry through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_AddFormatMsgEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_AddFormatMsgEntry through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    *state --  current ssh status.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T 
SYSLOG_PMGR_AddFormatMsgEntry(
    const SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    const void    *arg_0,
    const void    *arg_1,
    const void    *arg_2
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_ClearAllFlashEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_ClearAllFlashEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_ClearAllFlashEntries(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_ClearAllRamEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_ClearAllRamEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T
SYSLOG_PMGR_ClearAllRamEntries(
    void
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetNextUcFlashEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetNextUcFlashEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_GetNextUcFlashEntry(SYSLOG_MGR_Record_T *mgr_record);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetNextUcNormalEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetNextUcNormalEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_GetNextUcNormalEntries(SYSLOG_MGR_Record_T *mgr_record);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningUcLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningUcLogLevel(
    UI32_T *uc_log_level
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningFlashLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningFlashLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningFlashLogLevel(UI32_T *flash_log_level);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetFlashLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetFlashLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetFlashLogLevel(
    UI32_T flash_log_level
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetSyslogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T 
SYSLOG_PMGR_SetSyslogStatus(
    UI32_T syslog_status
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetUcLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetUcLogLevel(
    UI32_T uc_log_level
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetNextUcNormalEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetNextUcNormalEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T 
SYSLOG_PMGR_SnmpGetNextUcNormalEntry(
    SYSLOG_MGR_Record_T *mgr_record
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetUcNormalEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetUcNormalEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T 
SYSLOG_PMGR_SnmpGetUcNormalEntry(
    SYSLOG_MGR_Record_T *mgr_record
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_NotifyStaTplgChanged
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_NotifyStaTplgChanged through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void 
SYSLOG_PMGR_NotifyStaTplgChanged(
    void
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_NotifyStaTplgStabled
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_NotifyStaTplgStabled through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void 
SYSLOG_PMGR_NotifyStaTplgStabled(
    void
);

BOOL_T 
SYSLOG_PMGR_GetSyslogStatus(
    UI32_T *syslog_status
);

BOOL_T 
SYSLOG_PMGR_GetUcLogLevel(
    UI32_T *uc_log_level
);

BOOL_T 
SYSLOG_PMGR_GetFlashLogLevel(
    UI32_T *flash_log_level
);

/*fuzhimin,20090414*/
#if(SYS_CPNT_REMOTELOG == TRUE)

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemoteLogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemoteLogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningRemoteLogStatus(
    UI32_T *status
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningSyslogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningSyslogStatus(
    UI32_T *syslog_status
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogStatus(
    UI32_T status
);

UI32_T 
SYSLOG_PMGR_GetServerIPAddr(
    UI8_T index, 
    L_INET_AddrIp_T *ip_address
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_DeleteAllRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_PMGR_DeleteAllHost through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void 
SYSLOG_PMGR_DeleteAllRemoteLogServer(
    void
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostPort through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.port
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogServerPort(
    L_INET_AddrIp_T*ip_address, 
    UI32_T port
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRemoteLogServerPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetHostPort through the IPC msgq.
 * INPUT:
 *    *ip_address -- the host ip to get the level
 *    *port_p -- the host port
 * OUTPUT:
 *    *port_p -- the host port
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRemoteLogServerPort(
    L_INET_AddrIp_T*ip_address, 
    UI32_T* port_p
);

UI32_T 
SYSLOG_PMGR_GetRemoteLogStatus(
    UI32_T *status
);

UI32_T 
SYSLOG_PMGR_GetNextRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config
);

UI32_T 
SYSLOG_PMGR_GetRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config
);

/*fuzhimin,20090414,end*/

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_CreateRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_CreateHostIpAddr through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_CreateRemoteLogServer(
    L_INET_AddrIp_T* ip_address
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_DeleteRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_DeleteHostIPAddr through the IPC msgq.
 * INPUT:
 *    ip_address
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_DeleteRemoteLogServer(
    L_INET_AddrIp_T *ip_address
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemoteLogServer through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config,
    UI8_T index
);

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerFacility
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostFacility through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogServerFacility(
    L_INET_AddrIp_T*ip_address, 
    UI32_T facility
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogServerLevel(
    L_INET_AddrIp_T *ip_address, 
    UI32_T level
);

#else

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningFacilityType
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningFacilityType through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningFacilityType(
    UI32_T *facility
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemotelogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemotelogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_GetRunningRemotelogLevel(
    UI32_T *level
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogFacility
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogFacility through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogFacility(
    UI32_T facility
);

UI32_T 
SYSLOG_PMGR_GetRemoteLogFacility(
    UI32_T *facility
);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T 
SYSLOG_PMGR_SetRemoteLogLevel(
    UI32_T level
);

UI32_T 
SYSLOG_PMGR_GetRemoteLogLevel(
    UI32_T *level
);
#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif /*endif (SYS_CPNT_REMOTELOG == TRUE)*/
#endif

