#include "sys_type.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "sysfun.h"
#include "l_inet.h"
#include "syslog_type.h"
#include "syslog_adpt.h"
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#include "syslog_om.h"
#include "fs_type.h"
#include "fs.h"
#include "uc_mgr.h"
#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_mgr.h"
#endif
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSLOG_FILE_1   ".logfile_1"
#define SYSLOG_FILE_2   ".logfile_2"


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     SYSLOG_BACKDOOR_ShowSyslogInfo(void);
static void     SYSLOG_BACKDOOR_ShowUcNormalHeaderInfo(void);
static void     SYSLOG_BACKDOOR_ShowUcFlashHeaderInfo(void);
static void     SYSLOG_BACKDOOR_ShowSyslogFileInfo(UI32_T value);
static void     SYSLOG_BACKDOOR_LogEntriesToUcFlashDb(UI32_T value);
static void     SYSLOG_BACKDOOR_ShowLast10EntriesOfSyslogFileInfo(UI32_T value);
static void     SYSLOG_BACKDOOR_AllDestoryLogfileInFileSystem(void);
static void     SYSLOG_BACKDOOR_DestoryLogfileInFileSystem(UI32_T value);
static BOOL_T   SYSLOG_BACKDOOR_RecoveryLogfileInFileSystem(void);
static BOOL_T   SYSLOG_BACKDOOR_GetAllUcNormalEntries(void);
static BOOL_T   SYSLOG_BACKDOOR_GetAllUcFlashEntries(void);
static void     SYSLOG_BACKDOOR_CreateSysInfoDbInUCMemory(void);
static void     SYSLOG_BACKDOOR_ShowUCSysInfoDb(void);
static void     SYSLOG_BACKDOOR_SetMagicNumber(UI32_T value);
static void     SYSLOG_BACKDOOR_SetWarmStartPointer(UI32_T value);
static void     SYSLOG_BACKDOOR_SetMacAddr(UI8_T *mac_addr);
static void     SYSLOG_BACKDOOR_SetSerialNumber(UI8_T *serial_no);
static void     SYSLOG_BACKDOOR_SetMainboardHwVer(UI8_T *mainboard_hw_ver);
static void     SYSLOG_BACKDOOR_SetManufactureDateString(UI8_T *manufacture_date_str);
static void     SYSLOG_BACKDOOR_SetLoaderVer(UI8_T *fw_string);
static void     SYSLOG_BACKDOOR_SetPostVer(UI8_T *fw_string);
static void     SYSLOG_BACKDOOR_SetRuntimeVer(UI8_T *fw_string);
static BOOL_T   SYSLOG_BACKDOOR_ShowUcMemDatabaseInfo();
static BOOL_T   SYSLOG_BACKDOOR_ShowFlashDatabaseInfo();

#if (SYS_CPNT_REMOTELOG == TRUE)
static void     SYSLOG_BACKDOOR_ShowRemotelogInfo(void);
static void     SYSLOG_BACKDOOR_ShowRemotelogMessage(void);
#endif

#if (SYS_CPNT_SMTP == TRUE)
static void SYSLOG_BACKDOOR_ShowSmtpInfo(void);
#endif

#if ((SYS_CPNT_REMOTELOG == TRUE) || (SYS_CPNT_SMTP == TRUE))
UI8_T   message[SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE];
#endif

/* FUNCTION NAME: SYSLOG_BACKDOOR_Main
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void SYSLOG_BACKDOOR_Main (void)
{
#define MAXLINE 255
#define MIN_DUMMY_FILE    1
#define MAX_DUMMY_FILE    2
char line_buffer[MAXLINE];
int  select_value = 0;
int  set_value = 0;
UI32_T  unit, port;

/* for add entry, 2001/10/18 by aaron Chuang */
SYSLOG_OM_Record_T add_entry;
SYSLOG_OM_Config_T syslog_config;
SYSLOG_OM_RecordOwnerInfo_T   owner_info;
UI8_T   temp_serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
UI8_T   temp_mainboard_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];
UI8_T   temp_manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN + 1];
UI8_T    temp_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
UI8_T   mac_0 = 0, mac_1 = 0, mac_2 = 0, mac_3 = 0, mac_4 = 0, mac_5 = 0;
UI8_T   temp_mac_addr[SYS_ADPT_MAC_ADDR_LEN];

#if ((SYS_CPNT_REMOTELOG == TRUE) || (SYS_CPNT_SMTP == TRUE))
UI8_T   ip_0 = 0,ip_1 = 0,ip_2 = 0,ip_3 = 0;
L_INET_AddrIp_T temp_ip_addr;
UI32_T  ret;
#endif
#if (SYS_CPNT_SMTP == TRUE)
UI8_T email_addr_buffer[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];
#endif


    while(1)
    {
        /* Get System Log Status */
        SYSLOG_OM_GetSyslogConfig(&syslog_config);

        BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
        BACKDOOR_MGR_GetChar();

        BACKDOOR_MGR_Printf("\r\n=======================================");
        BACKDOOR_MGR_Printf("\r\n  System Log Engineer Menu 2001/10/18  ");
        BACKDOOR_MGR_Printf("\r\n=======================================");
        BACKDOOR_MGR_Printf("\r\n [1] Show System Log Information.");
        BACKDOOR_MGR_Printf("\r\n [2] Show UC Normal Header Information.");
        BACKDOOR_MGR_Printf("\r\n [3] Show UC Flash Header Information.");
        BACKDOOR_MGR_Printf("\r\n [4] Show UC Normal Log Table.");
        BACKDOOR_MGR_Printf("\r\n [5] Show UC Flash Log Table.");
        BACKDOOR_MGR_Printf("\r\n [6] Enable System Log Status.");
        BACKDOOR_MGR_Printf("\r\n [7] Disable System Log Status.");
        BACKDOOR_MGR_Printf("\r\n [8] Set UC Normal Log Level.[%ld..%d]", (long)syslog_config.flash_log_level, SYSLOG_LEVEL_DEBUG);
        BACKDOOR_MGR_Printf("\r\n [9] Set UC Flash Log Level.[%d..%ld]", SYSLOG_LEVEL_EMERG, (long)syslog_config.uc_log_level);
        BACKDOOR_MGR_Printf("\r\n [10] Log a message entry.");
        BACKDOOR_MGR_Printf("\r\n [11] Clear all log from Ram.");
        BACKDOOR_MGR_Printf("\r\n [12] Clear all log from Flash.");
        BACKDOOR_MGR_Printf("\r\n [13] Show syslog file information.[%d..%d]", MIN_DUMMY_FILE, MAX_DUMMY_FILE);
        BACKDOOR_MGR_Printf("\r\n [14] Log Some Entries to UC Flash DB.");
        BACKDOOR_MGR_Printf("\r\n [15] Log UC Flash DB to Flash(File System).");
        BACKDOOR_MGR_Printf("\r\n [16] Show last 10 entries of syslog file information.[%d..%d]", MIN_DUMMY_FILE, MAX_DUMMY_FILE);
        BACKDOOR_MGR_Printf("\r\n [17] Destory All LogFile from File System.");
        BACKDOOR_MGR_Printf("\r\n [18] Destory LogFile from File System.[%d..%d]", MIN_DUMMY_FILE, MAX_DUMMY_FILE);
        BACKDOOR_MGR_Printf("\r\n [19] Recovery LogFile In File System.");
        BACKDOOR_MGR_Printf("\r\n [20] Log Format message 1 to syslog module.");
        BACKDOOR_MGR_Printf("\r\n [21] Log Format message 2 to syslog module.");

        BACKDOOR_MGR_Printf("\r\n [22] Create/Reset System Information Database in UC memory.");
        BACKDOOR_MGR_Printf("\r\n [23] Show UC System Information Database.");
        BACKDOOR_MGR_Printf("\r\n [24] Set Magic Number(1.ColdStart, 2.WarmStart).");
        BACKDOOR_MGR_Printf("\r\n [25] Set WarmStart Pointer(0x\?\?\?\?\?\?\?\?).");
        BACKDOOR_MGR_Printf("\r\n [26] Set Mac Address(xx-xx-xx-xx-xx-xx).");
        BACKDOOR_MGR_Printf("\r\n [27] Set Serial Number(123456789ab).");
        BACKDOOR_MGR_Printf("\r\n [28] Set Agent H/W Version(12345).");
        BACKDOOR_MGR_Printf("\r\n [29] Set Manufacture Date(yyyy-mm-dd).");
        BACKDOOR_MGR_Printf("\r\n [30] Set Loader Version(V0.00.00.01).");
        BACKDOOR_MGR_Printf("\r\n [31] Set POST Version(V0.00.00.01).");
        BACKDOOR_MGR_Printf("\r\n [32] Set Runtime Version(V0.00.00.01).");
        BACKDOOR_MGR_Printf("\r\n [33] Get UC Memory Database.");
        BACKDOOR_MGR_Printf("\r\n [34] Get Flash Database.");
#if (SYS_CPNT_REMOTELOG == TRUE)
        BACKDOOR_MGR_Printf("\r\n [35] Show Remote Log Information.");
        BACKDOOR_MGR_Printf("\r\n [36] Enable Remote Log Status.");
        BACKDOOR_MGR_Printf("\r\n [37] Disable Remote Log Status.");
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
        BACKDOOR_MGR_Printf("\r\n [38] Set Remote Log Facility.[%d..%d]", SYSLOG_REMOTE_FACILITY_LOCAL0, SYSLOG_REMOTE_FACILITY_LOCAL7);
        BACKDOOR_MGR_Printf("\r\n [39] Set Remote Log Level.[%d..%d]", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE) */
        BACKDOOR_MGR_Printf("\r\n [40] Set Remote Log Server IP Address.[xx.xx.xx.xx]");
        BACKDOOR_MGR_Printf("\r\n [41] Clear Remote Log Server IP Address.[xx.xx.xx.xx]");
        BACKDOOR_MGR_Printf("\r\n [42] Log a message entry.");
        BACKDOOR_MGR_Printf("\r\n [43] Log Format message 1 to syslog module.");
        BACKDOOR_MGR_Printf("\r\n [44] Log Format message 2 to syslog module.");
#endif
#if (SYS_CPNT_SMTP == TRUE)
        BACKDOOR_MGR_Printf("\r\n [45] Show SMTP Log Information.");
        BACKDOOR_MGR_Printf("\r\n [46] Enable SMTP Log Status.");
        BACKDOOR_MGR_Printf("\r\n [47] Disable SMTP Log Status.");
        BACKDOOR_MGR_Printf("\r\n [48] Set SMTP Log Server IP Address.[xx.xx.xx.xx]");
        BACKDOOR_MGR_Printf("\r\n [49] Clear SMTP Log Server IP Address.[xx.xx.xx.xx]");
        BACKDOOR_MGR_Printf("\r\n [50] Set SMTP Log Level.[%d..%d]", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
        BACKDOOR_MGR_Printf("\r\n [51] Set SMTP Log Source email address.");
        BACKDOOR_MGR_Printf("\r\n [52] Clear SMTP Log Source email address.");
        BACKDOOR_MGR_Printf("\r\n [53] Set SMTP Log Destination email address.");
        BACKDOOR_MGR_Printf("\r\n [54] Clear SMTP Log Destination email address.");
#endif
        BACKDOOR_MGR_Printf("\r\n [99] Exit System Log Engineer Menu!!");
        BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

        if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
        {
            select_value = atoi(line_buffer);
            BACKDOOR_MGR_Printf("\r\nSelect value is %d", select_value); /* Debug message */
        }

        if (select_value == 8 || select_value == 9 || select_value == 13 || select_value == 16 ||
            select_value == 24|| select_value == 25)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the setting value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
        }

        if (select_value == 14)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter log entry number: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nLog entry value is %d", set_value); /* Debug message */
            }
        }

        if (select_value == 18)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter log file number: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nLog file number is %d", set_value); /* Debug message */
            }
        }

        if (select_value == 10)
        {
            memset((UI8_T *)&add_entry, 0, sizeof(SYSLOG_OM_Record_T));
            BACKDOOR_MGR_Printf("\r\nPlease enter the level value(%d-%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.level = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the module no value(%d-%d): ", SYS_MODULE_CLI, SYS_MODULE_UNKNOWN);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.module_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the function no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.function_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the error no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.error_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the message string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(add_entry.message, line_buffer, sizeof(add_entry.message));
                add_entry.message[sizeof(add_entry.message) - 1] = '\0';
                BACKDOOR_MGR_Printf("\r\nmessage string is [%s]", add_entry.message); /* Debug message */
            }
        }

        if (select_value == 20 || select_value == 21)
        {
            memset((UI8_T *)&owner_info, 0, sizeof(SYSLOG_OM_RecordOwnerInfo_T));
            BACKDOOR_MGR_Printf("\r\nPlease enter the level value(%d-%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.level = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the module no value(%d-%d): ", SYS_MODULE_CLI, SYS_MODULE_UNKNOWN);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.module_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the function no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.function_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the error no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.error_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter arg_0 value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                unit = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\narg_0 is %ld", (long)unit); /* Debug message */
            }

            BACKDOOR_MGR_Printf("\r\nPlease enter arg_1 value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                port = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\narg_1 is %ld", (long)port); /* Debug message */
            }
        }

        if (select_value == 26)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[0]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_0 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[0] is [0x%02x]", mac_0); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[1]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_1 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[1] is [0x%02x]", mac_1); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[2]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_2 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[2] is [0x%02x]", mac_2); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[3]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_3 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[3] is [0x%02x]", mac_3); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[4]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_4 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[4] is [0x%02x]", mac_4); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the mac address[5]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                mac_5 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nmac address[5] is [0x%02x]", mac_5); /* Debug message */
            }

            temp_mac_addr[0] = mac_0;
            temp_mac_addr[1] = mac_1;
            temp_mac_addr[2] = mac_2;
            temp_mac_addr[3] = mac_3;
            temp_mac_addr[4] = mac_4;
            temp_mac_addr[5] = mac_5;
        }

        if (select_value == 27)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the serial number string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(temp_serial_no, line_buffer, SYS_ADPT_SERIAL_NO_STR_LEN);
                temp_serial_no[SYS_ADPT_SERIAL_NO_STR_LEN] = 0;
                BACKDOOR_MGR_Printf("\r\nserial number string is [%s]", temp_serial_no); /* Debug message */
            }
        }

        if (select_value == 28)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the mainborad H/W version string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(temp_mainboard_hw_ver, line_buffer, SYS_ADPT_HW_VER_STR_LEN);
                temp_mainboard_hw_ver[SYS_ADPT_HW_VER_STR_LEN] = 0;
                BACKDOOR_MGR_Printf("\r\nmainborad H/W version string is [%s]", temp_mainboard_hw_ver); /* Debug message */
            }
        }

        if (select_value == 29)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter manufacuture date string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(temp_manufacture_date, line_buffer, SYS_ADPT_MANUFACTURE_DATE_LEN);
                temp_manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN] = 0;
                BACKDOOR_MGR_Printf("\r\nserial number string is [%s]", temp_manufacture_date); /* Debug message */
            }
        }

        if (select_value == 30 || select_value == 31 || select_value == 32)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the version string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(temp_fw_ver, line_buffer, SYS_ADPT_FW_VER_STR_LEN);
                temp_fw_ver[SYS_ADPT_FW_VER_STR_LEN] = 0;
                BACKDOOR_MGR_Printf("\r\nversion string is [%s]", temp_fw_ver); /* Debug message */
            }
        }

#if (SYS_CPNT_REMOTELOG == TRUE)

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
        if (select_value == 38)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the setting value(%d..%d): ",SYSLOG_REMOTE_FACILITY_LOCAL0, SYSLOG_REMOTE_FACILITY_LOCAL7);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
        }

        if (select_value == 39)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the setting value(%d..%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
        }
#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE) */

        if ((select_value == 40) || (select_value == 41))
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[0]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_0 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[0] is [%d]", ip_0); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[1]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_1 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[1] is [%d]", ip_1); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[2]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_2 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[2] is [%d]", ip_2); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[3]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_3 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[3] is [%d]", ip_3); /* Debug message */
            }

            temp_ip_addr.type = L_INET_ADDR_TYPE_IPV4;
            temp_ip_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            temp_ip_addr.addr[0] = ip_0;
            temp_ip_addr.addr[1] = ip_1;
            temp_ip_addr.addr[2] = ip_2;
            temp_ip_addr.addr[3] = ip_3;
        }

        if (select_value == 42)
        {
            memset((UI8_T *)&add_entry, 0, sizeof(SYSLOG_OM_Remote_Record_T));
            BACKDOOR_MGR_Printf("\r\nPlease enter the level value(%d-%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.level = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the module no value(%d-%d): ", SYS_MODULE_CLI, SYS_MODULE_UNKNOWN);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.module_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the function no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.function_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the error no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            add_entry.owner_info.error_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the message string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(add_entry.message, line_buffer, sizeof(add_entry.message));
                add_entry.message[sizeof(add_entry.message) - 1] = '\0';
                BACKDOOR_MGR_Printf("\r\nmessage string is [%s]", add_entry.message); /* Debug message */
            }
        }

        if (select_value == 43 || select_value == 44)
        {
            memset((UI8_T *)&owner_info, 0, sizeof(SYSLOG_OM_RecordOwnerInfo_T));
            BACKDOOR_MGR_Printf("\r\nPlease enter the level value(%d-%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.level = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the module no value(%d-%d): ", SYS_MODULE_CLI, SYS_MODULE_UNKNOWN);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.module_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the function no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.function_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter the error no value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
            owner_info.error_no = (UI8_T)set_value;

            BACKDOOR_MGR_Printf("\r\nPlease enter arg_0 value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                unit = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\narg_0 is %ld", (long)unit); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter arg_1 value: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                port = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\narg_1 is %ld", (long)port); /* Debug message */
            }
        }
#endif
#if (SYS_CPNT_SMTP == TRUE)
        if ((select_value == 48) || (select_value == 49))
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[0]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_0 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[0] is [%d]", ip_0); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[1]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_1 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[1] is [%d]", ip_1); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[2]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_2 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[2] is [%d]", ip_2); /* Debug message */
            }
            BACKDOOR_MGR_Printf("\r\nPlease enter the IP address[3]: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                ip_3 = (UI8_T) atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nIP address[3] is [%d]", ip_3); /* Debug message */
            }

            temp_ip_addr.type = L_INET_ADDR_TYPE_IPV4;
            temp_ip_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            temp_ip_addr.addr[0] = ip_0;
            temp_ip_addr.addr[1] = ip_1;
            temp_ip_addr.addr[2] = ip_2;
            temp_ip_addr.addr[3] = ip_3;
        }
        if (select_value == 50)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the setting value(%d..%d): ", SYSLOG_LEVEL_EMERG, SYSLOG_LEVEL_DEBUG);
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                set_value = atoi(line_buffer);
                BACKDOOR_MGR_Printf("\r\nSet value is %d", set_value); /* Debug message */
            }
        }
        if ((select_value == 51) || (select_value == 52) ||
            (select_value == 53) || (select_value == 54))
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the email address string: ");
            if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
            {
                memcpy(email_addr_buffer, line_buffer, SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS);
                email_addr_buffer[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS-1] = 0;
                BACKDOOR_MGR_Printf("\r\nEmail address string is [%s]", email_addr_buffer); /* Debug message */
            }
        }
#endif
        switch(select_value)
        {
            case 1:
                SYSLOG_BACKDOOR_ShowSyslogInfo();
                break;
            case 2:
                SYSLOG_BACKDOOR_ShowUcNormalHeaderInfo();
                break;
            case 3:
                SYSLOG_BACKDOOR_ShowUcFlashHeaderInfo();
                break;
            case 4:
                BACKDOOR_MGR_Printf("\r\nShow UC Normal Log Table.");
                SYSLOG_BACKDOOR_GetAllUcNormalEntries();
                break;
            case 5:
                BACKDOOR_MGR_Printf("\r\nShow UC Flash Log Table.");
                SYSLOG_BACKDOOR_GetAllUcFlashEntries();
                break;
            case 6:
                SYSLOG_MGR_SetSyslogStatus(SYSLOG_STATUS_ENABLE);
                break;
            case 7:
                SYSLOG_MGR_SetSyslogStatus(SYSLOG_STATUS_DISABLE);
                break;
            case 8:
                SYSLOG_MGR_SetUcLogLevel(set_value);
                break;
            case 9:
                SYSLOG_MGR_SetFlashLogLevel(set_value);
                break;
            case 10:
                SYSLOG_MGR_AddEntry(&add_entry);
                break;
            case 11:
                SYSLOG_MGR_ClearAllRamEntries();
                break;
            case 12:
                SYSLOG_MGR_ClearAllFlashEntries();
                break;
            case 13:
                SYSLOG_BACKDOOR_ShowSyslogFileInfo(set_value);
                break;
            case 14:
                SYSLOG_BACKDOOR_LogEntriesToUcFlashDb(set_value);
                break;
            case 15:
                SYSLOG_MGR_LogUcFlashDbToLogFile();
                break;
            case 16:
                SYSLOG_BACKDOOR_ShowLast10EntriesOfSyslogFileInfo(set_value);
                break;
            case 17:
                SYSLOG_BACKDOOR_AllDestoryLogfileInFileSystem();
                break;
            case 18:
                SYSLOG_BACKDOOR_DestoryLogfileInFileSystem(set_value);
                break;
            case 19:
                SYSLOG_BACKDOOR_RecoveryLogfileInFileSystem();
                break;
            case 20:
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_UP_MESSAGE_INDEX, &unit, &port, 0);
                break;
            case 21:
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_DOWN_MESSAGE_INDEX, &unit, &port, 0);
                break;
            case 22:
                SYSLOG_BACKDOOR_CreateSysInfoDbInUCMemory();
                break;
            case 23:
                SYSLOG_BACKDOOR_ShowUCSysInfoDb();
                break;
            case 24:
                SYSLOG_BACKDOOR_SetMagicNumber(set_value);
                break;
            case 25:
                SYSLOG_BACKDOOR_SetWarmStartPointer(set_value);
                break;
            case 26:
                SYSLOG_BACKDOOR_SetMacAddr((UI8_T *)&temp_mac_addr);
                break;
            case 27:
                SYSLOG_BACKDOOR_SetSerialNumber((UI8_T *)&temp_serial_no);
                break;
            case 28:
                SYSLOG_BACKDOOR_SetMainboardHwVer((UI8_T *)&temp_mainboard_hw_ver);
                break;
            case 29:
                SYSLOG_BACKDOOR_SetManufactureDateString((UI8_T *)&temp_manufacture_date);
                break;
            case 30:
                SYSLOG_BACKDOOR_SetLoaderVer((UI8_T *)&temp_fw_ver);
                break;
            case 31:
                SYSLOG_BACKDOOR_SetPostVer((UI8_T *)&temp_fw_ver);
                break;
            case 32:
                SYSLOG_BACKDOOR_SetRuntimeVer((UI8_T *)&temp_fw_ver);
                break;
            case 33:
                SYSLOG_BACKDOOR_ShowUcMemDatabaseInfo();
                break;
            case 34:
                SYSLOG_BACKDOOR_ShowFlashDatabaseInfo();
                break;
#if (SYS_CPNT_REMOTELOG == TRUE)
            case 35:
                SYSLOG_BACKDOOR_ShowRemotelogInfo();
            case 36:
                ret = SYSLOG_MGR_SetRemoteLogStatus(SYSLOG_STATUS_ENABLE);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 37:
                ret = SYSLOG_MGR_SetRemoteLogStatus(SYSLOG_STATUS_DISABLE);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
            case 38:
                ret = SYSLOG_MGR_SetRemoteLogFacility(set_value);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 39:
                ret = SYSLOG_MGR_SetRemoteLogLevel(set_value);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE) */
            case 40:
                ret = SYSLOG_MGR_CreateRemoteLogServer(&temp_ip_addr);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 41:
                ret = SYSLOG_MGR_DeleteRemoteLogServer(&temp_ip_addr);
                if (ret != SYSLOG_REMOTE_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nRemote Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 42:
                ret = SYSLOG_MGR_AddEntry(&add_entry);
                if(ret == SYSLOG_REMOTE_SUCCESS)
                {
                    SYSLOG_BACKDOOR_ShowRemotelogMessage();
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nRemote Log send packet fail.Reutrn value[%ld]", (long)ret);
                }

                break;
            case 43:
                ret = SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_UP_MESSAGE_INDEX, &unit, &port, 0);
                if(ret == SYSLOG_REMOTE_SUCCESS)
                {
                    SYSLOG_BACKDOOR_ShowRemotelogMessage();
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nRemote Log send packet fail.Reutrn value[%ld]", (long)ret);
                }
                break;
            case 44:
                ret = SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_DOWN_MESSAGE_INDEX, &unit, &port, 0);
                if(ret == SYSLOG_REMOTE_SUCCESS)
                {
                    SYSLOG_BACKDOOR_ShowRemotelogMessage();
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nRemote Log send packet fail.Reutrn value[%ld]", (long)ret);
                }
                break;
#endif
#if (SYS_CPNT_SMTP == TRUE)
            case 45:
                SYSLOG_BACKDOOR_ShowSmtpInfo();
                break;
            case 46:
                ret = SMTP_MGR_EnableSmtpAdminStatus();
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 47:
                ret = SMTP_MGR_DisableSmtpAdminStatus();
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 48:
                ret = SMTP_MGR_AddSmtpServerIPAddr((UI32_T)&temp_ip_addr);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 49:
                ret = SMTP_MGR_DeleteSmtpServerIPAddr((UI32_T)&temp_ip_addr);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 50:
                ret = SMTP_MGR_SetEmailSeverityLevel(set_value);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 51:
                ret = SMTP_MGR_SetSmtpSourceEmailAddr(email_addr_buffer);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 52:
                ret = SMTP_MGR_SetSmtpSourceEmailAddr(email_addr_buffer);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 53:
                ret = SMTP_MGR_AddSmtpDestinationEmailAddr(email_addr_buffer);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
            case 54:
                ret = SMTP_MGR_DeleteSmtpDestinationEmailAddr(email_addr_buffer);
                if (ret != SMTP_RETURN_SUCCESS)
                    BACKDOOR_MGR_Printf("\r\nSMTP Log set fail.Reutrn value[%ld]", (long)ret);
                break;
#endif
            case 99:
                BACKDOOR_MGR_Printf("\r\n Exit System Log Engineer Menu");
                return;
        }
    }

} /* End of SYSLOG_BACKDOOR_Main */


static void SYSLOG_BACKDOOR_ShowSyslogInfo(void)
{
    SYSLOG_OM_Config_T syslog_config;

    /* Show System Log Status */
    SYSLOG_OM_GetSyslogConfig(&syslog_config);

    switch (syslog_config.syslog_status)
    {
        case SYSLOG_STATUS_ENABLE:
            BACKDOOR_MGR_Printf("\r\nSystem Log Status[ENABLE]");
            break;
        case SYSLOG_STATUS_DISABLE:
            BACKDOOR_MGR_Printf("\r\nSystem Log Status[DISABLE]");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nSystem Log Status[UNKNOWN]");
            break;
    }

    /* Show Un-cleared Memory Normal Log Level */
    switch(syslog_config.uc_log_level)
    {
        case SYSLOG_LEVEL_EMERG:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- System unusable]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_ALERT:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Immediate action needed]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_CRIT:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Critical conditions]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_ERR:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Error conditions]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_WARNING:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Warning conditions]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_NOTICE:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Normal but significant condition]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_INFO:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Informational messages only]", (long)syslog_config.uc_log_level);
            break;
        case SYSLOG_LEVEL_DEBUG:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Debugging messages]", (long)syslog_config.uc_log_level);
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nUC Normal Log Level[%ld -- Unknown level value]", (long)syslog_config.uc_log_level);
            break;
    }

    /* Show Un-cleared Memory Flash Log Level */
    switch(syslog_config.flash_log_level)
    {
        case SYSLOG_LEVEL_EMERG:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- System unusable]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_ALERT:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Immediate action needed]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_CRIT:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Critical conditions]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_ERR:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Error conditions]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_WARNING:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Warning conditions]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_NOTICE:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Normal but significant condition]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_INFO:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Informational messages only]", (long)syslog_config.flash_log_level);
            break;
        case SYSLOG_LEVEL_DEBUG:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Debugging messages]", (long)syslog_config.flash_log_level);
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nUC Flash Log Level[%ld -- Unknown level value]", (long)syslog_config.flash_log_level);
            break;
    }

    return;
} /* End of SYSLOG_MGR_ShowSyslogInfo */



static void SYSLOG_BACKDOOR_ShowUcNormalHeaderInfo(void)
{
    SYSLOG_OM_Header_T header;

    SYSLOG_OM_GetUcNormalHeader(&header);
    BACKDOOR_MGR_Printf("\r\nUC Normal Header front[%ld]", (long)header.front);
    BACKDOOR_MGR_Printf("\r\nUC Normal Header rear[%ld]", (long)header.rear);
    BACKDOOR_MGR_Printf("\r\nUC Normal Header count[%ld]", (long)header.count);

    return;
} /* End of SYSLOG_MGR_ShowUcNormalHeaderInfo */


static void SYSLOG_BACKDOOR_ShowUcFlashHeaderInfo(void)
{
    SYSLOG_OM_Header_T header;

    SYSLOG_OM_GetUcFlashHeader(&header);
    BACKDOOR_MGR_Printf("\r\nUC Flash Header front[%ld]", (long)header.front);
    BACKDOOR_MGR_Printf("\r\nUC Flash Header rear[%ld]", (long)header.rear);
    BACKDOOR_MGR_Printf("\r\nUC Flash Header count[%ld]", (long)header.count);

    return;
} /* End of SYSLOG_MGR_ShowUcFlashHeaderInfo */


static void SYSLOG_BACKDOOR_ShowSyslogFileInfo(UI32_T value)
{
    UI32_T  read_count, i;
    UI8_T   filename[20];
    UI32_T  dis_count = 0;
    char    line_buffer[255];
    SYSLOG_MGR_Prepare_T *temp_prepare_db;

    temp_prepare_db = (SYSLOG_MGR_Prepare_T *)SYSLOG_MGR_GetPrepareDbPointer();

    switch (value)
    {
        case 1:
            strcpy((char *)filename, (char *)SYSLOG_FILE_1);
            break;
        case 2:
            strcpy((char *)filename, (char *)SYSLOG_FILE_2);
            break;
    };

    if (FS_ReadFile(1, filename, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
        return;

    BACKDOOR_MGR_Printf("\r\n Show syslog file %ld", (long)value);
    BACKDOOR_MGR_Printf("\r\n Sequence no is [%ld]", (long)temp_prepare_db->header.sequence_no);
    BACKDOOR_MGR_Printf("\r\n Count is [%ld]", (long)temp_prepare_db->header.count);
    for (i=0; i<temp_prepare_db->header.count; i++)
    {
        BACKDOOR_MGR_Printf("\r\n Index(%ld) l(%d) m(%d) f(%d) e(%d) t(%ld) m(%s)", (long)i, temp_prepare_db->entry[i].owner_info.level,
                temp_prepare_db->entry[i].owner_info.module_no,temp_prepare_db->entry[i].owner_info.function_no,
                temp_prepare_db->entry[i].owner_info.error_no,(long)temp_prepare_db->entry[i].log_time,
                temp_prepare_db->entry[i].message);
        dis_count++;
        if (dis_count % 20 == 0)
        {
            BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
            BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
        }
        if (dis_count == 50)
            break;
    }

    BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
    return;
} /* End of SYSLOG_MGR_ShowSyslogFileInfo */


static void SYSLOG_BACKDOOR_LogEntriesToUcFlashDb(UI32_T value)
{
    UI32_T  i;
    SYSLOG_OM_Record_T syslog_entry;
    UI32_T  flash_log_level;

    SYSLOG_MGR_GetFlashLogLevel(&flash_log_level);
    for (i=0; i<value; i++)
    {
        memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
        syslog_entry.owner_info.level = flash_log_level;
        syslog_entry.owner_info.module_no = SYS_MODULE_IGMPSNP;
        syslog_entry.owner_info.function_no = 1;
        syslog_entry.owner_info.error_no = 1;
        sprintf((char *)syslog_entry.message, "Flash testing %ld", (long)i);

        SYSLOG_MGR_AddEntry(&syslog_entry);
    }
    return;
} /* End of SYSLOG_MGR_LogEntriesToUcFlashDb */


static void SYSLOG_BACKDOOR_ShowLast10EntriesOfSyslogFileInfo(UI32_T value)
{
    SYSLOG_MGR_Prepare_T *temp_prepare_db;
    UI32_T  read_count, i;
    UI8_T   filename[20];
    UI32_T  dis_count = 0;
    char    line_buffer[255];
    UI32_T  part_of_count;

    temp_prepare_db = (SYSLOG_MGR_Prepare_T *)SYSLOG_MGR_GetPrepareDbPointer();

    switch (value)
    {
        case 1:
            strcpy((char *)filename, (char *)SYSLOG_FILE_1);
            break;
        case 2:
            strcpy((char *)filename, (char *)SYSLOG_FILE_2);
            break;
    };

    if (FS_ReadFile(1, filename, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
        return;

    BACKDOOR_MGR_Printf("\r\n Show syslog file %ld", (long)value);
    BACKDOOR_MGR_Printf("\r\n Sequence no is [%ld]", (long)temp_prepare_db->header.sequence_no);
    BACKDOOR_MGR_Printf("\r\n Count is [%ld]", (long)temp_prepare_db->header.count);
    if (temp_prepare_db->header.count > SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE)
        part_of_count = SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE;
    else
        part_of_count = temp_prepare_db->header.count;
    if (part_of_count < 10)
        i = 0;
    else
        i = part_of_count-10;
    for (; i<part_of_count; i++)
    {
        BACKDOOR_MGR_Printf("\r\n Index(%ld) l(%d) m(%d) f(%d) e(%d) t(%ld) m(%s)", (long)i, temp_prepare_db->entry[i].owner_info.level,
                temp_prepare_db->entry[i].owner_info.module_no,temp_prepare_db->entry[i].owner_info.function_no,
                temp_prepare_db->entry[i].owner_info.error_no,(long)temp_prepare_db->entry[i].log_time,
                temp_prepare_db->entry[i].message);
        dis_count++;
        if (dis_count % 20 == 0)
        {
            BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
            BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
        }
        if (dis_count == 50)
            break;
    }

    BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
    return;
} /* End of SYSLOG_MGR_ShowLast10EntriesOfSyslogFileInfo */


static void SYSLOG_BACKDOOR_AllDestoryLogfileInFileSystem(void)
{

    if (FS_DeleteFile(1, (UI8_T *)SYSLOG_FILE_1) != FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n Destory syslog file 1 error!!");
        return;
    }
    if (FS_DeleteFile(1, (UI8_T *)SYSLOG_FILE_2) != FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n Destory syslog file 2 error!!");
        return;
    }
    BACKDOOR_MGR_Printf("\r\n Destory syslog file 1-2 successfully!!");
    return;
} /* End of SYSLOG_MGR_AllDestoryLogfileInFileSystem */


static void SYSLOG_BACKDOOR_DestoryLogfileInFileSystem(UI32_T value)
{
    UI8_T   filename[20];

    switch (value)
    {
        case 1:
            strcpy((char *)filename, (char *)SYSLOG_FILE_1);
            break;
        case 2:
            strcpy((char *)filename, (char *)SYSLOG_FILE_2);
            break;
    };

    if (FS_DeleteFile(1, filename) != FS_RETURN_OK)
    {
        if (value >= 1 && value <= 2)
            BACKDOOR_MGR_Printf("\r\n Destory syslog file %ld error!!", (long)value);
        return;
    }

    if (value >= 1 && value <= 2)
        BACKDOOR_MGR_Printf("\r\n Destory syslog file %ld successfully!!", (long)value);
    return;
} /* End of SYSLOG_MGR_DestoryLogfileInFileSystem */


static BOOL_T SYSLOG_BACKDOOR_RecoveryLogfileInFileSystem(void)
{
    UI32_T  read_count;
    SYSLOG_MGR_Prepare_T *temp_prepare_db;

    temp_prepare_db = (SYSLOG_MGR_Prepare_T *)SYSLOG_MGR_GetPrepareDbPointer();

    if (FS_ReadFile(1, (UI8_T *)SYSLOG_FILE_1, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        if (FS_ReadFile(1, (UI8_T *)SYSLOG_FILE_2, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
            return FALSE;
        temp_prepare_db->header.sequence_no = temp_prepare_db->header.sequence_no + 1;
        if (FS_WriteFile(1, (UI8_T *)SYSLOG_FILE_1,(UI8_T *) "SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *) &temp_prepare_db, (temp_prepare_db->header.count+1)*SYSLOG_ADPT_LOG_ENTRY_LENGTH, SYSLOG_ADPT_LOGFILE_SIZE) != FS_RETURN_OK)
        {
            return FALSE;
        }
        return TRUE;
    }

    if (FS_ReadFile(1, (UI8_T *)SYSLOG_FILE_2, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        if (FS_ReadFile(1, (UI8_T *)SYSLOG_FILE_1, (UI8_T *)temp_prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
            return FALSE;
        temp_prepare_db->header.sequence_no = temp_prepare_db->header.sequence_no + 1;
        if (FS_WriteFile(1, (UI8_T *)SYSLOG_FILE_2,(UI8_T *) "SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *) &temp_prepare_db, (temp_prepare_db->header.count+1)*SYSLOG_ADPT_LOG_ENTRY_LENGTH, SYSLOG_ADPT_LOGFILE_SIZE) != FS_RETURN_OK)
        {
            return FALSE;
        }
        return TRUE;
    }

    return TRUE;
} /* End of SYSLOG_MGR_RecoveryLogfileInFileSystem */


static void SYSLOG_BACKDOOR_CreateSysInfoDbInUCMemory(void)
{
    UC_MGR_Sys_Info_T  *uc_sys_info_ptr;
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    uc_sys_info_ptr = (UC_MGR_Sys_Info_T *) UC_MGR_Allocate(UC_MGR_SYS_INFO_INDEX, sizeof(UC_MGR_Sys_Info_T), 4);
    if (uc_sys_info_ptr == 0)
        BACKDOOR_MGR_Printf("\r\nCreate system information database fail.");

    memset((UI8_T *)&uc_sys_info, 0, sizeof(UC_MGR_Sys_Info_T));
    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet system information database fail.");
    return;
} /* End of SYSLOG_MGR_CreateSysInfoDbInUCMemory() */


static void SYSLOG_BACKDOOR_ShowUCSysInfoDb(void)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    UI8_T   tmp_manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN+1];
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    /* Magic number */
    if (uc_sys_info.magic_number == 0x1a2b3c4d)
        BACKDOOR_MGR_Printf("\r\nmagic_number is [ColdStart].");
    else if (uc_sys_info.magic_number == 0xd4c3b2a1)
        BACKDOOR_MGR_Printf("\r\nmagic_number is [WarmStart].");
    else
        BACKDOOR_MGR_Printf("\r\nmagic_number is [Unknown].");

    /* Warm start pointer entry */
    BACKDOOR_MGR_Printf("\r\nWarmStart Pointer is [%p]", uc_sys_info.warm_start_entry);

    /* mac address */
    BACKDOOR_MGR_Printf("\r\nMac Address is [%02x-%02x-%02x-%02x-%02x-%02x]",
            uc_sys_info.mac_addr[0], uc_sys_info.mac_addr[1], uc_sys_info.mac_addr[2],
            uc_sys_info.mac_addr[3], uc_sys_info.mac_addr[4], uc_sys_info.mac_addr[5]);

    /* serial number */
    BACKDOOR_MGR_Printf("\r\nSerial Number is [%s]", uc_sys_info.serial_no);

    /* mainboard H/W version */
    BACKDOOR_MGR_Printf("\r\nMainborad H/W Version is [%s]", uc_sys_info.mainboard_hw_ver);

    /* manufacture date */
    memcpy((UI8_T *)&tmp_manufacture_date, (UI8_T *)&uc_sys_info.manufacture_date, SYS_ADPT_MANUFACTURE_DATE_LEN);
    tmp_manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN] = 0;
    BACKDOOR_MGR_Printf("\r\nManufacture Date is [%s]", tmp_manufacture_date);

    /* loader version */
    BACKDOOR_MGR_Printf("\r\nLoader Version is [%s]", uc_sys_info.loader_ver);

    /* POST version */
    BACKDOOR_MGR_Printf("\r\nPOST Version is [%s]", uc_sys_info.post_ver);

    /* runtime version */
    BACKDOOR_MGR_Printf("\r\nRuntime Version is [%s]", uc_sys_info.runtime_fw_ver);

    /* epld version */
    BACKDOOR_MGR_Printf("\r\nEpld Version Reg Value is [0x%02X]", uc_sys_info.epld_version);

    /* check sum */
    BACKDOOR_MGR_Printf("\r\nCheck sum is [0x%04x]", uc_sys_info.check_sum);

    return;
} /* End of SYSLOG_MGR_ShowUCSysInfoDb() */


static void SYSLOG_BACKDOOR_SetMagicNumber(UI32_T value)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    switch (value)
    {
        case 1:
            uc_sys_info.magic_number = 0x1a2b3c4d;
            break;  /* Cold Start */
        case 2:
            uc_sys_info.magic_number = 0xd4c3b2a1;
            break;  /* Warm Start */
    }

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetMagicNumber() */


static void SYSLOG_BACKDOOR_SetMacAddr(UI8_T *mac_addr)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.mac_addr, mac_addr, SYS_ADPT_MAC_ADDR_LEN);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetMacAddr() */


static void SYSLOG_BACKDOOR_SetWarmStartPointer(UI32_T value)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    uc_sys_info.warm_start_entry = (void *)(uintptr_t)value;

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetWarmStartPointer() */


static void SYSLOG_BACKDOOR_SetSerialNumber(UI8_T *serial_no)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.serial_no, serial_no, SYS_ADPT_SERIAL_NO_STR_LEN + 1);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetSerialNumber() */


static void SYSLOG_BACKDOOR_SetMainboardHwVer(UI8_T *mainboard_hw_ver)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.mainboard_hw_ver, mainboard_hw_ver, SYS_ADPT_HW_VER_STR_LEN + 1);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetAgentHwVer() */


static void SYSLOG_BACKDOOR_SetManufactureDateString(UI8_T *manufacture_date_str)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.manufacture_date, manufacture_date_str, SYS_ADPT_MANUFACTURE_DATE_LEN);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetManufactureDateString() */


static void SYSLOG_BACKDOOR_SetLoaderVer(UI8_T *fw_string)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.loader_ver, fw_string, SYS_ADPT_FW_VER_STR_LEN + 1);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetLoaderVer() */


static void SYSLOG_BACKDOOR_SetPostVer(UI8_T *fw_string)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.post_ver, fw_string, SYS_ADPT_FW_VER_STR_LEN + 1);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetPostVer() */


static void SYSLOG_BACKDOOR_SetRuntimeVer(UI8_T *fw_string)
{
    UC_MGR_Sys_Info_T  uc_sys_info;
    BOOL_T  ret;

    ret = UC_MGR_GetSysInfo(&uc_sys_info);
    if (ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nGet UC system information fail.");

    memcpy((UI8_T *) &uc_sys_info.runtime_fw_ver, fw_string, SYS_ADPT_FW_VER_STR_LEN + 1);

    ret = UC_MGR_SetSysInfo(uc_sys_info);
    if(ret == FALSE)
        BACKDOOR_MGR_Printf("\r\nSet UC system information fail.");

    return;
} /* End of SYSLOG_MGR_SetRuntimeVer() */


static BOOL_T SYSLOG_BACKDOOR_ShowUcMemDatabaseInfo()
{
    SYSLOG_MGR_UcDatabase_T uc_database;
    SYSLOG_OM_Record_T  syslog_entry;
    SYSLOG_OM_Header_T  uc_normal_header;
    UI32_T  i,j;

    /* Aaron add, 2001/10/18 */
    UI8_T   level_name[8][8] = { "EMERG  ",
                                 "ALERT  ",
                                 "CRIT   ",
                                 "ERR    ",
                                 "WARNING",
                                 "NOTICE ",
                                 "INFO   ",
                                 "DEBUG  " };
    UI8_T   module_name[38][14] = {   "GLOBAL_SYSTEM",
                                      "APP_CLI      ",
                                      "APP_LEDMGMT  ",
                                      "APP_RMON     ",
                                      "APP_RMON2    ",
                                      "APP_SNMP     ",
                                      "APP_TRAPMGMT ",
                                      "APP_WEB      ",
                                      "APP_BOOTP    ",
                                      "APP_DHCP     ",
                                      "APP_XFER     ",
                                      "BSPS         ",
                                      "CORE_EXTBRG  ",
                                      "CORE_PRIMGMT ",
                                      "CORE_STA     ",
                                      "CORE_DATABASE",
                                      "CORE_IGMPSNP ",
                                      "CORE_MIB2MGMT",
                                      "CORE_NETWORK ",
                                      "CORE_ROOT    ",
                                      "CORE_SECURITY",
                                      "CORE_STKMGMT ",
                                      "CORE_SWCTRL  ",
                                      "CORE_SYSLOG  ",
                                      "CORE_SYSMGMT ",
                                      "CORE_USERAUTH",
                                      "CORE_VLAN    ",
                                      "CORE_GVRP    ",
                                      "CORE_L2MCAST ",
                                      "CORE_LACP    ",
                                      "DRIVER_FLASH ",
                                      "DRIVER_IMC   ",
                                      "DRIVER_LED   ",
                                      "DRIVER_NIC   ",
                                      "DRIVER_SEPM  ",
                                      "DRIVER_SWITCH",
                                      "DRIVER_FS    ",
                                      "UNKNOWN      "  };

    SYSLOG_OM_GetUcNormalHeader(&uc_normal_header);

    /* copy from uc normal to database
     */
    uc_database.count = uc_normal_header.count;

    for (i = uc_normal_header.front, j=0;
         j<uc_normal_header.count;
         i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
    {
        SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
        uc_database.entry[j] = syslog_entry; /* structure copy */
    }

    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    BACKDOOR_MGR_Printf("\r\n Index  Level            Module                 FunNo   ErrNo   RTC-Time(yyyy-mm-dd:hh-mm-ss)    Message        ");
    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    for (i=0; i<uc_database.count; i++)
    {
        int year, month, day, hour, minute, second;

        SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[i].log_time,
                                        &year, &month, &day, &hour, &minute, &second);
        /*year += 2000;*/ /* add base century, 2001/11/26 */
        BACKDOOR_MGR_Printf("\r\n  %3ld     %s(%1d)       %s(%2d)     %3d     %3d    %8ld(%4d-%2d-%2d:%2d-%2d-%2d)    %s",
                            (long)i,
                            level_name[uc_database.entry[i].owner_info.level],
                            uc_database.entry[i].owner_info.level,
                            module_name[uc_database.entry[i].owner_info.module_no],
                            uc_database.entry[i].owner_info.module_no,
                            uc_database.entry[i].owner_info.function_no,
                            uc_database.entry[i].owner_info.error_no,
                            (long)uc_database.entry[i].log_time,
                            year,
                            month,
                            day,
                            hour,
                            minute,
                            second,
                            uc_database.entry[i].message);
    }

    BACKDOOR_MGR_Printf("\r\n===============================================================================");

    return TRUE;
} /* End of SYSLOG_MGR_ShowUcMemDatabaseInfo */


static BOOL_T SYSLOG_BACKDOOR_ShowFlashDatabaseInfo()
{
    SYSLOG_MGR_UcDatabase_T uc_database;
    SYSLOG_OM_Header_T  uc_flash_header;
    SYSLOG_OM_Record_T  syslog_entry;
    UI32_T  i,j;

    /* Aaron add, 2001/10/18 */
    UI8_T   level_name[8][8] = { "EMERG  ",
                                 "ALERT  ",
                                 "CRIT   ",
                                 "ERR    ",
                                 "WARNING",
                                 "NOTICE ",
                                 "INFO   ",
                                 "DEBUG  " };
    UI8_T   module_name[38][14] = {   "GLOBAL_SYSTEM",
                                      "APP_CLI      ",
                                      "APP_LEDMGMT  ",
                                      "APP_RMON     ",
                                      "APP_RMON2    ",
                                      "APP_SNMP     ",
                                      "APP_TRAPMGMT ",
                                      "APP_WEB      ",
                                      "APP_BOOTP    ",
                                      "APP_DHCP     ",
                                      "APP_XFER     ",
                                      "BSPS         ",
                                      "CORE_EXTBRG  ",
                                      "CORE_PRIMGMT ",
                                      "CORE_STA     ",
                                      "CORE_DATABASE",
                                      "CORE_IGMPSNP ",
                                      "CORE_MIB2MGMT",
                                      "CORE_NETWORK ",
                                      "CORE_ROOT    ",
                                      "CORE_SECURITY",
                                      "CORE_STKMGMT ",
                                      "CORE_SWCTRL  ",
                                      "CORE_SYSLOG  ",
                                      "CORE_SYSMGMT ",
                                      "CORE_USERAUTH",
                                      "CORE_VLAN    ",
                                      "CORE_GVRP    ",
                                      "CORE_L2MCAST ",
                                      "CORE_LACP    ",
                                      "DRIVER_FLASH ",
                                      "DRIVER_IMC   ",
                                      "DRIVER_LED   ",
                                      "DRIVER_NIC   ",
                                      "DRIVER_SEPM  ",
                                      "DRIVER_SWITCH",
                                      "DRIVER_FS    ",
                                      "UNKNOWN      "  };

    SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);

    /* copy from uc normal to database
     */
    uc_database.count = uc_flash_header.count;

    for (i = uc_flash_header.front, j=0;
         j<uc_flash_header.count;
         i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
    {
        SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);
        uc_database.entry[j] = syslog_entry; /* structure copy */
    }

    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    BACKDOOR_MGR_Printf("\r\n Index  Level            Module                 FunNo   ErrNo   RTC-Time(yyyy-mm-dd:hh-mm-ss)    Message        ");
    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    for (i=0; i<uc_database.count; i++)
    {
        int year, month, day, hour, minute, second;

        SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[i].log_time,
                                        &year, &month, &day, &hour, &minute, &second);
        /*year += 2000;*/ /* add base century, 2001/11/26 */
        BACKDOOR_MGR_Printf("\r\n  %3ld     %s(%1d)       %s(%2d)     %3d     %3d    %8ld(%4d-%2d-%2d:%2d-%2d-%2d)    %s",
                            (long)i,
                            level_name[uc_database.entry[i].owner_info.level],
                            uc_database.entry[i].owner_info.level,
                            module_name[uc_database.entry[i].owner_info.module_no],
                            uc_database.entry[i].owner_info.module_no,
                            uc_database.entry[i].owner_info.function_no,
                            uc_database.entry[i].owner_info.error_no,
                            (long)uc_database.entry[i].log_time,
                            year,
                            month,
                            day,
                            hour,
                            minute,
                            second,
                            uc_database.entry[i].message);
    }

    BACKDOOR_MGR_Printf("\r\n===============================================================================");

    return TRUE;
} /* End of SYSLOG_MGR_ShowFlashDatabaseInfo */


/* FUNCTION NAME: SYSLOG_MGR_GetAllUcNormalEntries
 * PURPOSE: Get all log messages from un-cleared memory normal log DB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   This function only use for engineer debug.
 *
 */
static BOOL_T SYSLOG_BACKDOOR_GetAllUcNormalEntries(void)
{
    UI32_T  i, j;
    SYSLOG_OM_Record_T syslog_entry;
    SYSLOG_OM_Header_T uc_normal_header;

    /* Aaron add, 2001/10/18 */
    UI8_T   level_name[8][8] = { "EMERG  ",
                                 "ALERT  ",
                                 "CRIT   ",
                                 "ERR    ",
                                 "WARNING",
                                 "NOTICE ",
                                 "INFO   ",
                                 "DEBUG  " };
    UI8_T   module_name[38][14] = {   "GLOBAL_SYSTEM",
                                      "APP_CLI      ",
                                      "APP_LEDMGMT  ",
                                      "APP_RMON     ",
                                      "APP_RMON2    ",
                                      "APP_SNMP     ",
                                      "APP_TRAPMGMT ",
                                      "APP_WEB      ",
                                      "APP_BOOTP    ",
                                      "APP_DHCP     ",
                                      "APP_XFER     ",
                                      "BSPS         ",
                                      "CORE_EXTBRG  ",
                                      "CORE_PRIMGMT ",
                                      "CORE_STA     ",
                                      "CORE_DATABASE",
                                      "CORE_IGMPSNP ",
                                      "CORE_MIB2MGMT",
                                      "CORE_NETWORK ",
                                      "CORE_ROOT    ",
                                      "CORE_SECURITY",
                                      "CORE_STKMGMT ",
                                      "CORE_SWCTRL  ",
                                      "CORE_SYSLOG  ",
                                      "CORE_SYSMGMT ",
                                      "CORE_USERAUTH",
                                      "CORE_VLAN    ",
                                      "CORE_GVRP    ",
                                      "CORE_L2MCAST ",
                                      "CORE_LACP    ",
                                      "DRIVER_FLASH ",
                                      "DRIVER_IMC   ",
                                      "DRIVER_LED   ",
                                      "DRIVER_NIC   ",
                                      "DRIVER_SEPM  ",
                                      "DRIVER_SWITCH",
                                      "DRIVER_FS    ",
                                      "UNKNOWN      "  };

    SYSLOG_OM_GetUcNormalHeader(&uc_normal_header);
    if (uc_normal_header.count == 0)
        return FALSE;

    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    BACKDOOR_MGR_Printf("\r\n Index  Level            Module                 FunNo   ErrNo   RTC-Time(yyyy-mm-dd:hh-mm-ss)    Message        ");
    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    for (i = uc_normal_header.front, j=0; j<uc_normal_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
    {
        int year, month, day, hour, minute, second;

        SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
        SYS_TIME_ConvertSecondsToDateTime(syslog_entry.log_time,
                                        &year, &month, &day, &hour, &minute, &second);
        /*year += 2000;*/ /* add base century, 2001/11/26 */
        BACKDOOR_MGR_Printf("\r\n  %3ld     %s(%1d)       %s(%2d)     %3d     %3d    %8ld(%4d-%2d-%2d:%2d-%2d-%2d)    %s",
               (long)i, level_name[syslog_entry.owner_info.level], syslog_entry.owner_info.level,
               module_name[syslog_entry.owner_info.module_no], syslog_entry.owner_info.module_no,
               syslog_entry.owner_info.function_no, syslog_entry.owner_info.error_no,
               (long)syslog_entry.log_time, year, month, day, hour, minute, second, syslog_entry.message);
    }
    BACKDOOR_MGR_Printf("\r\n===============================================================================");

    return TRUE;
} /* End of SYSLOG_MGR_GetAllUcNormalEntries */


/* FUNCTION NAME: SYSLOG_MGR_GetAllUcFlashEntries
 * PURPOSE: Get all log messages from un-cleared memory flash log DB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   This function only use for engineer debug.
 *
 */
static BOOL_T SYSLOG_BACKDOOR_GetAllUcFlashEntries(void)
{
    UI32_T  i, j;
    SYSLOG_OM_Record_T syslog_entry;
    SYSLOG_OM_Header_T uc_flash_header;

    /* Aaron add, 2001/10/18 */
    UI8_T   level_name[8][8] = { "EMERG  ",
                                 "ALERT  ",
                                 "CRIT   ",
                                 "ERR    ",
                                 "WARNING",
                                 "NOTICE ",
                                 "INFO   ",
                                 "DEBUG  " };
    UI8_T   module_name[38][14] = {   "GLOBAL_SYSTEM",
                                      "APP_CLI      ",
                                      "APP_LEDMGMT  ",
                                      "APP_RMON     ",
                                      "APP_RMON2    ",
                                      "APP_SNMP     ",
                                      "APP_TRAPMGMT ",
                                      "APP_WEB      ",
                                      "APP_BOOTP    ",
                                      "APP_DHCP     ",
                                      "APP_XFER     ",
                                      "BSPS         ",
                                      "CORE_EXTBRG  ",
                                      "CORE_PRIMGMT ",
                                      "CORE_STA     ",
                                      "CORE_DATABASE",
                                      "CORE_IGMPSNP ",
                                      "CORE_MIB2MGMT",
                                      "CORE_NETWORK ",
                                      "CORE_ROOT    ",
                                      "CORE_SECURITY",
                                      "CORE_STKMGMT ",
                                      "CORE_SWCTRL  ",
                                      "CORE_SYSLOG  ",
                                      "CORE_SYSMGMT ",
                                      "CORE_USERAUTH",
                                      "CORE_VLAN    ",
                                      "CORE_GVRP    ",
                                      "CORE_L2MCAST ",
                                      "CORE_LACP    ",
                                      "DRIVER_FLASH ",
                                      "DRIVER_IMC   ",
                                      "DRIVER_LED   ",
                                      "DRIVER_NIC   ",
                                      "DRIVER_SEPM  ",
                                      "DRIVER_SWITCH",
                                      "DRIVER_FS    ",
                                      "UNKNOWN      "  };

    SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);
    if (uc_flash_header.count == 0)
        return FALSE;

    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    BACKDOOR_MGR_Printf("\r\n Index  Level            Module                 FunNo   ErrNo   RTC-Time(yyyy-mm-dd:hh-mm-ss)    Message        ");
    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");
    for (i = uc_flash_header.front, j=0; j<uc_flash_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB, j++)
    {
        int year, month, day, hour, minute, second;

        SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);
        SYS_TIME_ConvertSecondsToDateTime(syslog_entry.log_time,
                                        &year, &month, &day, &hour, &minute, &second);
        /*year += 2000;*/ /* add base century, 2001/11/26 */
        BACKDOOR_MGR_Printf("\r\n  %3ld     %s(%1d)       %s(%2d)     %3d     %3d    %8ld(%4d-%2d-%2d:%2d-%2d-%2d)    %s",
               (long)i, level_name[syslog_entry.owner_info.level], syslog_entry.owner_info.level,
               module_name[syslog_entry.owner_info.module_no], syslog_entry.owner_info.module_no,
               syslog_entry.owner_info.function_no, syslog_entry.owner_info.error_no,
               (long)syslog_entry.log_time, year, month, day, hour, minute, second, syslog_entry.message);
    }
    BACKDOOR_MGR_Printf("\r\n===============================================================================================================");

    return TRUE;
} /* End of SYSLOG_MGR_GetAllUcFlashEntries */


#if (SYS_CPNT_REMOTELOG == TRUE)
static void SYSLOG_BACKDOOR_ShowRemotelogInfo(void)
{
    SYSLOG_OM_Remote_Config_T remotelog_config;
    UI8_T   i;
    //UI32_T  ip_addr;

    /* Show System Log Status */
    SYSLOG_OM_GetRemoteLogConfiguration(&remotelog_config);

    switch (remotelog_config.status)
    {
        case SYSLOG_STATUS_ENABLE:
            BACKDOOR_MGR_Printf("\r\nRemote Log Status[ENABLE]");
            break;
        case SYSLOG_STATUS_DISABLE:
            BACKDOOR_MGR_Printf("\r\nRemote Log Status[DISABLE]");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nRemote Log Status[UNKNOWN]");
            break;
    }

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
    /* Show Facility */
    switch(remotelog_config.facility)
    {
        case SYSLOG_REMOTE_FACILITY_LOCAL0:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 0]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL1:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 1]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL2:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 2]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL3:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 3]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL4:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 4]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL5:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 5]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL6:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 6]", (long)remotelog_config.facility);
            break;
        case SYSLOG_REMOTE_FACILITY_LOCAL7:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Local 7]", (long)remotelog_config.facility);
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nRemote Log Facility[%ld -- Unknown Facility value]", (long)remotelog_config.facility);
            break;
    }

    /* Show Level */
    switch(remotelog_config.level)
    {
        case SYSLOG_LEVEL_EMERG:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- System unusable]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_ALERT:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Immediate action needed]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_CRIT:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Critical conditions]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_ERR:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Error conditions]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_WARNING:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Warning conditions]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_NOTICE:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Normal but significant condition]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_INFO:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Informational messages only]", (long)remotelog_config.level);
            break;
        case SYSLOG_LEVEL_DEBUG:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Debugging messages]", (long)remotelog_config.level);
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nRemote Log Level[%ld -- Unknown level value]", (long)remotelog_config.level);
            break;
    }
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE) */

    SYSLOG_MGR_Remote_Server_Config_T server_config;
    memset(&server_config, 0, sizeof(SYSLOG_MGR_Remote_Server_Config_T));

    for(i = 0;SYSLOG_PMGR_GetNextRemoteLogServer(&server_config) == SYSLOG_REMOTE_SUCCESS;i++)
    {
        char ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = "";
        L_INET_InaddrToString((L_INET_Addr_T *)&(server_config.ipaddr), ip_str, sizeof(ip_str));

        BACKDOOR_MGR_Printf("\r\nRemote Log IP address %d [%s]", ++i,ip_str);

    }

#if 0
    for(i=0;i<SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER;i++)
    {
        #define SYSLOG_BACKDOOR_MAX_LEN_IP_STRING       18
        UI8_T   ip_string[SYSLOG_BACKDOOR_MAX_LEN_IP_STRING + 1];

        SYSLOG_MGR_GetServerIPAddr(&ip_addr,i);
        /* ES3550MO-PoE-FLF-AA-00235
         * ip_addr is network byte order
         */
        BACKDOOR_MGR_Printf("\r\nRemote Log IP address %d [%s]", i,
            L_INET_Ntoa(ip_addr, ip_string));
    }
#endif
    return;
} /* End of SYSLOG_BACKDOOR_ShowRemotelogInfo */


static void SYSLOG_BACKDOOR_ShowRemotelogMessage(void)
{
    /*UI8_T   message[SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE];*/

    SYSLOG_OM_GetRemoteLogMessage(message);

    BACKDOOR_MGR_Printf("\r\nRemote Log Message : %s",message);

    return;
}
#endif
#if (SYS_CPNT_SMTP == TRUE)
static void SYSLOG_BACKDOOR_ShowSmtpInfo(void)
{
    UI32_T  smtp_status;
    UI32_T  smtp_level;
    UI8_T   emailaddr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];
    UI8_T   i;
    UI32_T  ip_addr = 0;
    UI32_T  ret;

    /* Show System Log Status */
    SMTP_MGR_GetSmtpAdminStatus(&smtp_status);

    switch (smtp_status)
    {
        case SMTP_STATUS_ENABLE:
            BACKDOOR_MGR_Printf("\r\nSMTP Status[ENABLE]");
            break;
        case SMTP_STATUS_DISABLE:
            BACKDOOR_MGR_Printf("\r\nSMTP Status[DISABLE]");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nSMTP Status[UNKNOWN]");
            break;
    }

    /* Show Level */
    SMTP_MGR_GetEmailSeverityLevel(&smtp_level);
    switch(smtp_level)
    {
        case SYSLOG_LEVEL_EMERG:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- System unusable]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_ALERT:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Immediate action needed]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_CRIT:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Critical conditions]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_ERR:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Error conditions]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_WARNING:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Warning conditions]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_NOTICE:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Normal but significant condition]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_INFO:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Informational messages only]", (long)smtp_level);
            break;
        case SYSLOG_LEVEL_DEBUG:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Debugging messages]", (long)smtp_level);
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nSMTP Level[%ld -- Unknown level value]", (long)smtp_level);
            break;
    }

    for(i=1;i<=SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
    {
        ret = SMTP_MGR_GetNextSmtpServerIPAddr(&ip_addr);
        if(ret == SMTP_RETURN_SUCCESS)
            BACKDOOR_MGR_Printf("\r\nSMTP IP address %d [%d.%d.%d.%d]",i,(UI8_T)((ip_addr>>24)&0xff),(UI8_T)((ip_addr>>16)&0xff),(UI8_T)((ip_addr>>8)&0xff),(UI8_T)(ip_addr&0xff));
        if(SMTP_MGR_GetSmtpServerIPAddr(&ip_addr) == SMTP_RETURN_SUCCESS)
        {
            BACKDOOR_MGR_Printf("\r\nSMTP IP address get success");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\nSMTP IP address get fail");
        }
    }

    memset(emailaddr,0,sizeof(emailaddr));
    SMTP_MGR_GetSmtpSourceEmailAddr(emailaddr);
    BACKDOOR_MGR_Printf("\r\nSMTP Source Email Address is %s",emailaddr);

    strcpy((char *)emailaddr,"");
    for(i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
    {
        ret = SMTP_MGR_GetNextSmtpDestinationEmailAddr(emailaddr);
        if(ret == SMTP_RETURN_SUCCESS)
            BACKDOOR_MGR_Printf("\r\nSMTP Destination Email Address is %s",emailaddr);
        if(SMTP_MGR_GetSmtpDestinationEmailAddr(emailaddr) == SMTP_RETURN_SUCCESS)
        {
            BACKDOOR_MGR_Printf("\r\nSMTP Destination Email address get success");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\nSMTP Destination Email address get fail");
        }
    }
    return;
}
#endif
