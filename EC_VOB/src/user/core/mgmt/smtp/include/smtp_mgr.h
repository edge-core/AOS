/* Module Name: SMTP_MGR.H
 * Purpose: Initialize the resources and provide functions
 *          for the smtp module.
 *
 * Notes:
 *
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */

#ifndef SMTP_MGR_H
#define SMTP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "smtp_om.h"
#include "sysfun.h"


/* NAME CONSTANT DECLARATIONS
 */
typedef enum SMTP_MGR_RETURN_E
{
    SMTP_MGR_RETURN_FAIL         =   0,
    SMTP_MGR_RETURN_SUCCESS      =   1,
    SMTP_MGR_RETURN_QUEUE_EMPTY  =   2,
} SMTP_MGR_RETURN_T;

/* The key to get keygen mgr msgq.
 */
#define SMTP_MGR_IPCMSG_KEY    SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY

/* The commands for IPC message.
 */
enum {
    SMTP_MGR_IPC_CMD_ADD_SMTP_DST_EMAIL_ADDR,       /*  0 */
    SMTP_MGR_IPC_CMD_ADD_SMTP_SERVER_IP,            /*  1 */
    SMTP_MGR_IPC_CMD_DEL_SMTP_DST_EMAIL_ADDR,       /*  2 */
    SMTP_MGR_IPC_CMD_DEL_SMTP_SERVER_IP,            /*  3 */
    SMTP_MGR_IPC_CMD_DISABLE_ADMIN_STATUS,          /*  4 */
    SMTP_MGR_IPC_CMD_ENABLE_ADMIN_STATUS,           /*  5 */
    SMTP_MGR_IPC_CMD_GET_EMAIL_SER_LEVEL,           /*  6 */
    SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_DST_EML_ADR,/*  7 */
    SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_SVR_IP,     /*  8 */
    SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_DES_EMAIL_ADDR,  /*  9 */
    SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_SERVER_IP,       /* 10 */
    SMTP_MGR_IPC_CMD_GET_SMTP_ADMIN_STATUS,         /* 11 */
    SMTP_MGR_IPC_CMD_GET_SMTP_SRC_EMAIL_ADDR,       /* 12 */
    SMTP_MGR_IPC_CMD_GET_RUNN_EMAIL_SER_LVL,        /* 13 */
    SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_ADMIN_STATUS,    /* 14 */
    SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_SRC_EML_ADR,     /* 15 */
    SMTP_MGR_IPC_CMD_SET_EMAIL_SER_LEVEL,           /* 16 */
    SMTP_MGR_IPC_CMD_SET_SMTP_SRC_EMAIL_ADDR,       /* 17 */
    SMTP_MGR_IPC_CMD_GET_SMTP_SERVER_IP_ADDR,       /* 18 */
    SMTP_MGR_IPC_CMD_GET_SMTP_DST_EMAIL_ADDR,       /* 19 */
    SMTP_MGR_IPC_CMD_DEL_EMAIL_SER_LEVEL,           /* 20 */    //daniel
    SMTP_MGR_IPC_CMD_DEL_SMTP_SRC_EMAIL_ADDR,       /* 21 */    //daniel
};

/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - SMTP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SMTP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of SMTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SMTP_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((SMTP_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SMTP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of SMTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(SMTP_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - SMTP_MGR_MSG_CMD
 *              SMTP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the SMTP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The SMTP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SMTP_MGR_MSG_CMD(msg_p)    (((SMTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define SMTP_MGR_MSG_RETVAL(msg_p) (((SMTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - SMTP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SMTP_MGR_MSG_DATA(msg_p)   ((void *)&((SMTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* DATA TYPE DECLARATIONS
 */
/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} SMTP_MGR_IPCMsg_Type_T;

typedef struct
{
    UI8_T   email_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];
} SMTP_MGR_IPCMsg_EmailAddr_T;

typedef struct
{
    UI32_T  ip_addr;
} SMTP_MGR_IPCMsg_IpAddr_T;

typedef struct
{
    UI32_T  ser_level;
} SMTP_MGR_IPCMsg_SerLevel_T;

typedef struct
{
    UI32_T  status;
} SMTP_MGR_IPCMsg_Status_T;

typedef struct
{
    SMTP_OM_Record_T    smtp_rec;
} SMTP_MGR_IPCMsg_SmtpRecord_T;

typedef union
{
    SMTP_MGR_IPCMsg_EmailAddr_T     email; /* for add/get/set email address     */
    SMTP_MGR_IPCMsg_IpAddr_T        ipadr; /* for get/add server ip address     */
    SMTP_MGR_IPCMsg_SerLevel_T      srlvl; /* for get/set smtp severity level   */
    SMTP_MGR_IPCMsg_Status_T        stats; /* for get status                   */
    SMTP_MGR_IPCMsg_SmtpRecord_T    smtpr; /* for send mail                     */
} SMTP_MGR_IPCMsg_Data_T;

typedef struct
{
    SMTP_MGR_IPCMsg_Type_T    type;
    SMTP_MGR_IPCMsg_Data_T    data;
} SMTP_MGR_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: SMTP_MGR_GetOperationMode()
 * PURPOSE: This function will get current operation mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  Current operation mode.
 *           1. SYS_TYPE_STACKING_TRANSITION_MODE
 *           2. SYS_TYPE_STACKING_MASTER_MODE
 *           3. SYS_TYPE_STACKING_SLAVE_MODE
 * NOTES:   None.
 *
 */
SYS_TYPE_Stacking_Mode_T SMTP_MGR_GetOperationMode(void);

/* FUNCTION NAME: SMTP_MGR_EnterMasterMode()
 * PURPOSE: This function will enter master mode
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterMasterMode(void);

/* FUNCTION NAME: SMTP_MGR_EntertSlaveMode()
 * PURPOSE: This function will enter slave mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterSlaveMode(void);

/* FUNCTION NAME: SMTP_MGR_EntertTransitionMode()
 * PURPOSE: This function will enter transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterTransitionMode(void);

/* FUNCTION NAME: SMTP_MGR_SetTransitionMode()
 * PURPOSE: This function will set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_SetTransitionMode(void);

/* FUNCTION NAME: SMTP_MGR_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Create semphore.
 *          2. Initialize OM database of the smtp module.
 *          3. Called by SMTP_INIT_Initiate_System_Resources() only.
 *
 */
void SMTP_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SMTP_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SMTP_MGR_AddSmtpServerIPAddr
 * PURPOSE: This function is used to add the smtp server ip address.
 * INPUT:   ip_addr -- smtp server ip address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_EXIST -- input exist
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. add one server ip address one time.
 *          2. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *
 */
UI32_T SMTP_MGR_AddSmtpServerIPAddr(UI32_T ip_addr);

/* FUNCTION NAME: SMTP_MGR_GetSmtpServerIPAddr
 * PURPOSE: This function is used to get smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_MGR_GetSmtpServerIPAddr(UI32_T *ip_addr);

/* FUNCTION NAME: SMTP_MGR_GetNextSmtpServerIPAddr
 * PURPOSE: This function is used to get next smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_MGR_GetNextSmtpServerIPAddr(UI32_T *ip_addr);

/* FUNCTION NAME: SMTP_MGR_GetNextRunningSmtpServerIPAddr
 * PURPOSE: This function is used to get next running smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first running ip address
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetNextRunningSmtpServerIPAddr(UI32_T *ip_addr);

/* FUNCTION NAME: SMTP_MGR_DeleteSmtpServerIPAddr
 * PURPOSE: This function is used to delete the smtp server ip address.
 * INPUT:   ip_addr -- smtp server ip address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_NOT_EXIST -- input not exist
 * NOTES:   1. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. Delete one server ip address one time
 *
 */
UI32_T SMTP_MGR_DeleteSmtpServerIPAddr(UI32_T ip_addr);

/* FUNCTION NAME: SMTP_MGR_SetEmailSeverityLevel
 * PURPOSE: This function is used to set smtp severity level.
 * INPUT:   level -- smtp seveirity level.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. smtp level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_SetEmailSeverityLevel(UI32_T level);

//ADD daniel
/* FUNCTION NAME: SMTP_MGR_DeleteEmailSeverityLevel
 * PURPOSE: This function is used to delete smtp severity level to default value.
 * INPUT:   level -- smtp seveirity level.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_DeleteEmailSeverityLevel(UI32_T level);   //daniel


/* FUNCTION NAME: SMTP_MGR_GetEmailSeverityLevel
 * PURPOSE: This function is used to get smtp severity level.
 * INPUT:   *level -- output buffer of smtp severity level.
 * OUTPUT:  *level -- smtp severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. smtp level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_GetEmailSeverityLevel(UI32_T *level);

/* FUNCTION NAME: SMTP_MGR_GetRunningEmailSeverityLevel
 * PURPOSE: This function is used to get running smtp severity level.
 * INPUT:   *level -- output buffer of smtp severity level.
 * OUTPUT:  *level -- smtp severity level.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. smtp level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningEmailSeverityLevel(UI32_T *level);

/* FUNCTION NAME: SMTP_MGR_SetSmtpSourceEmailAddr
 * PURPOSE: This function is used to set the smtp source email address.
 * INPUT:   *email_addr -- smtp source email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. There is only one source email address
 *
 */
UI32_T SMTP_MGR_SetSmtpSourceEmailAddr(UI8_T *email_addr);

//ADD daniel
/* FUNCTION NAME: SMTP_MGR_DeleteSmtpSourceEmailAddr
 * PURPOSE: This function is used to delete the smtp source email address.
 * INPUT:   *email_addr -- smtp source email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_DeleteSmtpSourceEmailAddr(void);


/* FUNCTION NAME: SMTP_MGR_GetSmtpSourceEmailAddr
 * PURPOSE: This function is used to get the smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address.
 * OUTPUT:  *email_addr -- smtp source email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. There is only one source email address
 *
 */
UI32_T SMTP_MGR_GetSmtpSourceEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_GetRunningSmtpSourceEmailAddr
 * PURPOSE: This function is used to get the running smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address.
 * OUTPUT:  *email_addr -- smtp source email address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. There is only one source email address.
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningSmtpSourceEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_AddSmtpDestinationEmailAddr
 * PURPOSE: This function is used to add the smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_EXIST  -- input value already exist
 * NOTES:   1. Add one destination email address one time
 *          2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *
 */
UI32_T SMTP_MGR_AddSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_GetSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_GetSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_GetNextSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get next smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *
 */
UI32_T SMTP_MGR_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get next running smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_DeleteSmtpDestinationEmailAddr
 * PURPOSE: This function is used to delete the smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. delete one destination email address one time.
 *          2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *
 */
UI32_T SMTP_MGR_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_MGR_EnableSmtpAdminStatus
 * PURPOSE: This function is used to enable smtp admin status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_EnableSmtpAdminStatus(void);

/* FUNCTION NAME: SMTP_MGR_DisableSmtpAdminStatus
 * PURPOSE: This function is used to disable smtp admin status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_DisableSmtpAdminStatus(void);

/* FUNCTION NAME: SMTP_MGR_GetSmtpAdminStatus
 * PURPOSE: This function is used to get smtp admin status.
 * INPUT:   *status -- output buffer of smtp status.
 * OUTPUT:  *status -- smtp status.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_GetSmtpAdminStatus(UI32_T *status);

/* FUNCTION NAME: SMTP_MGR_GetRunningSmtpAdminStatus
 * PURPOSE: This function is used to get running smtp status.
 * INPUT:   *status -- output buffer of smtp status.
 * OUTPUT:  *status -- smtp status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *          2. default status is SYS_DFLT_SMTP_ADMIN_STATUS
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningSmtpAdminStatus(UI32_T *status);

/* FUNCTION NAME: SMTP_MGR_SendMail
 * PURPOSE: This function is used to send mail.
 * INPUT:   smtp_entry -- smtp entry.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. check smtp entey level
 *          2. if smtp entry level is higher than configured level,keep the entry into queue.
 *
 */
void SMTP_MGR_SendMail(SMTP_OM_Record_T *smtp_entry);

/* FUNCTION NAME: SMTP_MGR_HandleTrapQueue
 * PURPOSE: This function is used to send smtp mail when task recieve 
 *          SMTP_TASK_EVENT_SEND_MAIL or SMTP_TASK_EVENT_PERIODIC_TIMER event.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. if there is any entry in queue,
 *             then create connection,dequeue the entry and send out mail
 *          2. if send mail fail,enqueue the entry again.
 *          3. Before end of this function,close the connection.
 *          4. if send mail successfully,free the entry.
 *
 */
SMTP_MGR_RETURN_T
SMTP_MGR_HandleTrapQueue(
    );

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void SMTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void SMTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SMTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for smtp mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SMTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /* SMTP_MGR_H */

