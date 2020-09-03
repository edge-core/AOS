/* Module Name: SYSLOG_ADPT.H
 * Purpose: Define constant symbol for system log module using.
 *
 * Notes:
 *
 * History:
 *    10/17/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SYSLOG_ADPT_H
#define SYSLOG_ADPT_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"


/* NAME CONSTANT DECLARATIONS
 */
#define SYSLOG_ADPT_MESSAGE_LENGTH                          128
#define SYSLOG_ADPT_LOG_ENTRY_LENGTH                        sizeof(SYSLOG_OM_Record_T)
#define SYSLOG_ADPT_LOGFILE_HEADER_LENGTH                   SYSLOG_ADPT_LOG_ENTRY_LENGTH
#define SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB              512
#define SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB             (2 * SYS_TYPE_1K_BYTES)

#define SYSLOG_ADPT_RESERVE_HEADER_SIZE_IN_LOGFILE          SYS_TYPE_1K_BYTES
#define SYSLOG_ADPT_LOGFILE_SIZE                            (SYS_ADPT_LOGFILE_SIZE_IN_FLASH - SYSLOG_ADPT_RESERVE_HEADER_SIZE_IN_LOGFILE + SYSLOG_ADPT_LOGFILE_HEADER_LENGTH)

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
#define SYSLOG_ADPT_LOGIN_OUT_LOGFILE_SIZE                  (SYS_ADPT_LOGIN_OUT_LOGFILE_SIZE_IN_FLASH - SYSLOG_ADPT_RESERVE_HEADER_SIZE_IN_LOGFILE + SYSLOG_ADPT_LOGFILE_HEADER_LENGTH)
#endif /*(SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE) */

#define SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE              (SYS_ADPT_LOGFILE_SIZE_IN_FLASH - SYSLOG_ADPT_RESERVE_HEADER_SIZE_IN_LOGFILE)/SYSLOG_ADPT_LOG_ENTRY_LENGTH

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
#define SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_LOGIN_OUT_FILE    (SYS_ADPT_LOGIN_OUT_LOGFILE_SIZE_IN_FLASH - SYSLOG_ADPT_RESERVE_HEADER_SIZE_IN_LOGFILE)/SYSLOG_ADPT_LOG_ENTRY_LENGTH
#endif /*(SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE) */

#define SYSLOG_ADPT_MAX_CAPABILITY_OF_TOTAL_UC_DB           (SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB + SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB)
#define SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_DB            (SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE + SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB)

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
#define SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_LOGIN_OUT_DB  (SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB + SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_LOGIN_OUT_FILE)
#endif /*(SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE) */

/* Filename of temporal files used to save current logs on UC memory before to
 * reload in order to restore logs to UC memory after warm start. It aims to
 * workaround the problem that UC memory always been cleared despite of cold
 * start or warm start by hardware.
 */
#define SYSLOG_ADPT_SYS_LOG_RAM_UC_TMP_FILE             ".sys_log_ram_uc_tmp_db"
#define SYSLOG_ADPT_SYS_LOG_FLASH_UC_TMP_FILE           ".sys_log_flash_uc_tmp_db"

#define SYSLOG_ADPT_EVENT_30_SEC                            0x0001L
#define SYSLOG_ADPT_EVENT_ENTER_TRANSITION                  0x0002L
#define SYSLOG_ADPT_EVENT_STA_STATE_CHANGED                 0x0004L
#define SYSLOG_ADPT_EVENT_RIF_UP                            0x0008L
#define SYSLOG_ADPT_EVENT_RIF_DOWN                          0x0010L
#define SYSLOG_ADPT_EVENT_TRAP_ARRIVAL                      0x0020L
#define SYSLOG_ADPT_EVENT_35_SEC       			    0x0040L

#define SYSLOG_ADPT_TIMER_TICKS_0_5_SEC                     50
#define SYSLOG_ADPT_STA_UNSTABLED_STATE                     0
#define SYSLOG_ADPT_STA_BECOME_STABLED_STATE                1
#define SYSLOG_ADPT_STA_STABLED_STATE                       2

#define SYSLOG_ADPT_MIN_REMOTE_HOST_UDP_PORT                MIN_remoteLogServerUdpPort
#define SYSLOG_ADPT_MAX_REMOTE_HOST_UDP_PORT                MAX_remoteLogServerUdpPort
#endif /* End of SYSLOG_ADPT_H */
