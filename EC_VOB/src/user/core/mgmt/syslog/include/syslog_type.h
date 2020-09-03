/* Module Name: SYSLOG_TYPE.H
 * Purpose:
 *
 * Notes:
 *
 * History:
 *    10/29/01       -- Aaron Chuang, Create
 *                   -- Separete parts from syslog_mgr.c, and generate the syslog_om.c
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SYSLOG_TYPE_H
#define SYSLOG_TYPE_H

#include "sys_cpnt.h"
#include "leaf_es3626a.h"
#include "l_inet.h"

#define SYSLOG_TYPE_MAX_QUE_NBR              (SYS_ADPT_TOTAL_NBR_OF_LPORT * 2)

#define SYSLOG_TYPE_MAX_FACILITY_STR_LEN      60
#define SYSLOG_TYPE_MAX_LEVEL_STR_LEN         60

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    SYSLOG_TYPE_TRACE_ID_SYSLOG_MGR_QUEUEENQUEUE = 0,
    SYSLOG_TYPE_TRACE_ID_SYSLOG_MGR_HANDLETRAPQUEUE
};

enum SYSLOG_ERROR_E
{
    SYSLOG_RETURN_OK = 0,   /* OK, Successful, Without any Error */
    SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL,    /* Normal level is smaller than flash level */
    SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL,     /* Flash level is bigger than normal level */
    SYSLOG_LEVEL_VLAUE_INVALID, /* Syslog level is invalid */
    SYSLOG_UNKNOWN_ERROR
};

enum SYSLOG_OM_PROFILE_E
{
    SYSLOG_OM_DEFAULT_PROFILE = 0,

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
    SYSLOG_OM_LOGIN_OUT_PROFILE,
#endif /* SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT */

    SYSLOG_OM_MAX_NBR_OF_PROFILE
};

typedef struct{
    L_INET_AddrIp_T  ip_address;
    UI32_T           udp_port;
} SYSLOG_Host_T;

typedef enum SYSLOG_STATUS_TYPE_E
{
    SYSLOG_STATUS_ENABLE  = VAL_sysLogStatus_enabled,
    SYSLOG_STATUS_DISABLE = VAL_sysLogStatus_disabled
} SYSLOG_STATUS_TYPE_T;


typedef enum SYSLOG_LEVEL_TYPE_E     /* Reference CLI Specification      */
{
    SYSLOG_LEVEL_EMERG  = 0,    /* System unusable                  */
    SYSLOG_LEVEL_ALERT  = 1,    /* Immediate action needed          */
    SYSLOG_LEVEL_CRIT   = 2,    /* Critical conditions              */
    SYSLOG_LEVEL_ERR    = 3,    /* Error conditions                 */
    SYSLOG_LEVEL_WARNING= 4,    /* Warning conditions               */
    SYSLOG_LEVEL_NOTICE = 5,    /* Normal but significant condition */
    SYSLOG_LEVEL_INFO   = 6,    /* Informational messages only      */
    SYSLOG_LEVEL_DEBUG  = 7     /* Debugging messages               */
} SYSLOG_LEVEL_TYPE_T;


typedef enum
{
    SYSLOG_NONE_FUNC_NO                             = 0,
    SYSLOG_MGR_SET_SYSLOG_STATUS_FUNC_NO            = 1,
    SYSLOG_MGR_GET_UC_LOG_LEVEL_FUNC_NO             = 2,
    SYSLOG_MGR_SET_UC_LOG_LEVEL_FUNC_NO             = 3,
    SYSLOG_MGR_SET_FLASH_LOG_LEVEL_FUNC_NO          = 4,
    SYSLOG_MGR_ADD_FORMAT_MSG_ENTRY_LOCAL_FUNC_NO   = 5,
    SYSLOG_MGR_ADD_ENTRY_LOCAL_FUNC_NO              = 6,
    SYSLOG_MGR_GET_NEXT_UC_NORMAL_ENTRIES_FUNC_NO   = 7,
    SYSLOG_MGR_GET_ALL_FLASH_TO_XFER_BUFFER_FUNC_NO = 8,
    SYSLOG_MGR_ADD_FORMAT_MSG_ENTRY_REMOTE_FUNC_NO  = 9,
    SYSLOG_MGR_ADD_ENTRY_REMOTE_FUNC_NO             = 10,
    SYSLOG_MGR_GET_SERVER_IP_ADDR_FUNC_NO           = 11,
    SYSLOG_MGR_SET_FACILITY_TYPE_FUNC_NO            = 12,
    SYSLOG_MGR_SET_REMOTELOG_STATUS_FUNC_NO         = 13,
    SYSLOG_MGR_SET_REMOTELOG_LEVEL_FUNC_NO          = 14,
    SYSLOG_MGR_VALID_IP_FUNC_NO                     = 15,
    SYSLOG_MGR_SEND_PACKET_FUNC_NO                  = 16,
    SYSLOG_OM_GET_SERVER_IP_ADDR_FUNC_NO            = 17,
    SYSLOG_OM_DELETE_SERVER_IP_ADDR_FUNC_NO         = 18,
    SYSLOG_OM_SET_FACILITY_TYPE_FUNC_NO             = 19,
    SYSLOG_OM_GET_FACILITY_TYPE_FUNC_NO             = 20,
    SYSLOG_OM_SET_REMOTELOG_STATUS_FUNC_NO          = 21,
    SYSLOG_OM_GET_REMOTELOG_STATUS_FUNC_NO          = 22,
    SYSLOG_OM_SET_REMOTELOG_LEVEL_FUNC_NO           = 23,
    SYSLOG_OM_GET_REMOTELOG_LEVEL_FUNC_NO           = 24,
    SYSLOG_OM_SET_SERVER_IP_ADDR_FUNC_NO            = 25,
    SYSLOG_ENTER_MASTER_MODE_FUNC_NO                = 26

} SYSLOG_UI_MESSAGE_FUNC_NO_T;

#if (SYS_CPNT_EH == FALSE)
typedef enum SYSLOG_MODULE_TYPE_E
{
    SYSLOG_MODULE_GLOBAL_SYSTEM = 0,
    SYSLOG_MODULE_APP_CLI,
    SYSLOG_MODULE_APP_LEDMGMT,
    SYSLOG_MODULE_APP_RMON,
    SYSLOG_MODULE_APP_RMON2,
    SYSLOG_MODULE_APP_SNMP,
    SYSLOG_MODULE_APP_TRAPMGMT,
    SYSLOG_MODULE_APP_WEB,
    SYSLOG_MODULE_APP_BOOTP,    /* add */
    SYSLOG_MODULE_APP_DHCP,     /* add */
    SYSLOG_MODULE_APP_XFER,     /* add */
    SYSLOG_MODULE_BSPS,
    SYSLOG_MODULE_CORE_EXTBRG,
    SYSLOG_MODULE_CORE_PRIMGMT,
    SYSLOG_MODULE_CORE_STA,
    SYSLOG_MODULE_CORE_DATABASE,
    SYSLOG_MODULE_CORE_IGMPSNP,
    SYSLOG_MODULE_CORE_MIB2MGMT,
    SYSLOG_MODULE_CORE_NETWORK,
    SYSLOG_MODULE_CORE_ROOT,
    SYSLOG_MODULE_CORE_SECURITY,
    SYSLOG_MODULE_CORE_STKMGMT,
    SYSLOG_MODULE_CORE_SWCTRL,
    SYSLOG_MODULE_CORE_SYSLOG,
    SYSLOG_MODULE_CORE_SYSMGMT,
    SYSLOG_MODULE_CORE_USERAUTH,
    SYSLOG_MODULE_CORE_VLAN,
    SYSLOG_MODULE_CORE_GVRP,    /* add */
    SYSLOG_MODULE_CORE_L2MCAST, /* add */
    SYSLOG_MODULE_CORE_LACP,    /* add */
    SYSLOG_MODULE_DRIVER_FLASH,
    SYSLOG_MODULE_DRIVER_IMC,
    SYSLOG_MODULE_DRIVER_LED,
    SYSLOG_MODULE_DRIVER_NIC,
    SYSLOG_MODULE_DRIVER_SEPM,
    SYSLOG_MODULE_DRIVER_SWITCH,
    SYSLOG_MODULE_DRIVER_FS,    /* add */
    SYSLOG_MODULE_CORE_COS,     /* add */
    SYSLOG_MODULE_APP_CGI,
    SYSLOG_MODULE_APP_HTTP,
    SYSLOG_MODULE_DRIVER_VDSL,    /* add for VDSL project */
    SYSLOG_MODULE_CORE_XSTP,
    SYSLOG_MODULE_UNKNOWN = 0xff
} SYSLOG_MODULE_TYPE_T;
#endif

enum SYSLOG_REMOTE_RETURN_VALUE_E
{
    SYSLOG_REMOTE_SUCCESS = 0,     /* OK, Successful, Without any Error */
    SYSLOG_REMOTE_FAIL,            /* fail */
    SYSLOG_REMOTE_INVALID,         /* invalid input value */
    SYSLOG_REMOTE_INVALID_BUFFER,  /* invalid input buffer */
    SYSLOG_REMOTE_INPUT_EXIST,     /* input value already exist */
    SYSLOG_REMOTE_CREATE_SOCKET_FAIL,  /* create socket fail */
    SYSLOG_REMOTE_BIND_SOCKET_FAIL,    /* bind socket fail */
    SYSLOG_REMOTE_NOT_REMOTE_IP_ADDR,  /* this ip address is not the remote log server ip */
    SYSLOG_REMOTE_DUPCLIATE,
};

typedef enum SYSLOG_REMOTE_FACILITY_TYPE_E
{
    SYSLOG_REMOTE_FACILITY_KERN        = 0, /* kernel messages */
    SYSLOG_REMOTE_FACILITY_USER        = 1, /* random user-level messages */
    SYSLOG_REMOTE_FACILITY_MAIL        = 2, /* mail system */
    SYSLOG_REMOTE_FACILITY_DAEMON      = 3, /* system daemons */
    SYSLOG_REMOTE_FACILITY_AUTH        = 4, /* security/authorization messages */
    SYSLOG_REMOTE_FACILITY_SYSLOG      = 5, /* messages generated internally by syslogd */
    SYSLOG_REMOTE_FACILITY_LPR         = 6, /* line printer subsystem */
    SYSLOG_REMOTE_FACILITY_NEWS        = 7, /* network news subsystem */
    SYSLOG_REMOTE_FACILITY_UUCP        = 8, /* UUCP subsystem */
    SYSLOG_REMOTE_FACILITY_CRON        = 9, /* clock daemon */
    SYSLOG_REMOTE_FACILITY_AUTH1       = 10, /* security/authorization messages */
    SYSLOG_REMOTE_FACILITY_FTP         = 11, /* FTP daemon */
    SYSLOG_REMOTE_FACILITY_NTP         = 12, /* NTP subsystem */
    SYSLOG_REMOTE_FACILITY_AUDIT       = 13, /* log audit */
    SYSLOG_REMOTE_FACILITY_ALERT       = 14, /* log alert */
    SYSLOG_REMOTE_FACILITY_CRON1       = 15, /* clock daemon */
    SYSLOG_REMOTE_FACILITY_LOCAL0      = 16, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL1      = 17, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL2      = 18, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL3      = 19, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL4      = 20, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL5      = 21, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL6      = 22, /* reserved for local use */
    SYSLOG_REMOTE_FACILITY_LOCAL7      = 23, /* reserved for local use */
}SYSLOG_REMOTE_FACILITY_TYPE_T;

#endif /* End of SYSLOG_TYPE_H */
