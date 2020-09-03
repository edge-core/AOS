/* Module Name: SMTP_OM.H
 * Purpose: Initialize the database resources and provide some Get/Set function
 *          for accessing the SMTP database.
 *
 * Notes:
 *
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *                   -- Separete parts from smtp_mgr.c, and generate the smtp_om.c
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */

#ifndef SMTP_OM_H
#define SMTP_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "syslog_adpt.h"
#include "leaf_es3626a.h"

/* NAME CONSTANT DECLARATIONS
 */
#define SMTP_ADPT_MESSAGE_LENGTH        SYSLOG_ADPT_MESSAGE_LENGTH

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SMTP_TASK_CreateSmtpTask_FUNC_NO               = 0,
    SMTP_MGR_ADD_SERVER_IP_FUNC_NO                 = 1,
    SMTP_MGR_GET_SERVER_IP_FUNC_NO                 = 2,
    SMTP_MGR_GET_NEXT_SERVER_IP_FUNC_NO	           = 3,
    SMTP_MGR_DELETE_SERVER_IP_FUNC_NO              = 4,
    SMTP_MGR_SET_SEVERITY_LEVEL_FUNC_NO	           = 5,
    SMTP_MGR_GET_SEVERITY_LEVEL_FUNC_NO	           = 6,
    SMTP_MGR_GET_RUNNING_SEVERITY_LEVEL_FUNC_NO    = 7,
    SMTP_MGR_SET_SOURCE_EMAIL_ADDR_FUNC_NO         = 8,
    SMTP_MGR_GET_SOURCE_EMAIL_ADDR_FUNC_NO         = 9,
    SMTP_MGR_GET_RUNNING_SOURCE_EMAIL_ADDR_FUNC_NO = 10,
    SMTP_MGR_SET_DES_EMAIL_ADDR_FUNC_NO	           = 11,
    SMTP_MGR_GET_DES_EMAIL_ADDR_FUNC_NO	           = 12,
    SMTP_MGR_GET_NEXT_DES_EMAIL_ADDR_FUNC_NO       = 13,
    SMTP_MGR_GET_ADMIN_STATUS_FUNC_NO	           = 14

} SMTP_UI_MESSAGE_FUNC_NO_T;

enum SYSLOG_SMTP_RETURN_VALUE_E
{
    SMTP_RETURN_SUCCESS = 0,     /* OK, Successful, Without any Error */
    SMTP_RETURN_FAIL,            /* fail */
    SMTP_RETURN_INVALID,         /* invalid input value */
    SMTP_RETURN_INVALID_BUFFER,  /* invalid input buffer */
    SMTP_RETURN_INPUT_EXIST,     /* input value already exist */
    SMTP_RETURN_CREATE_SOCKET_FAIL,  /* create socket fail */
    SMTP_RETURN_BIND_SOCKET_FAIL,    /* bind socket fail */
    SMTP_RETURN_DATABASE_FULL,    /* database full */
    SMTP_RETURN_INPUT_NOT_EXIST,     /* input value not exist */
};

typedef enum SMTP_STATUS_TYPE_E
{
    SMTP_STATUS_ENABLE  = VAL_smtpStatus_enabled,
    SMTP_STATUS_DISABLE = VAL_smtpStatus_disabled
} SMTP_STATUS_TYPE_T;

typedef struct SMTP_OM_RecordOwnerInfo_S
{
    UI8_T   level;          /* Reference SYSLOG_LEVEL_E         */
    UI8_T   module_no;      /* Reference SYS_MODULE_ID_E        */
    UI8_T   function_no;    /* Definition by each module itself */
    UI8_T   error_no;       /* Definition by each module itself */
} SMTP_OM_RecordOwnerInfo_T;

/*	SMTP_OM_Config_T is the structure of smtp config.
 *	Note:
 *      smtp_status:
 *		    -- SMTPLOG_STATUS_ENABLE  = 0,
 *		    -- SMTPLOG_STATUS_DISABLE = 1
 *	smtp_level: high or equal this level will belog to smtp server(when smtp_status is enabled).
 *   		-- SYSLOG_LEVEL_EMERG  = 0,    ( System unusable                  )
 *   		-- SYSLOG_LEVEL_ALERT  = 1,    ( Immediate action needed          )
 *   		-- SYSLOG_LEVEL_CRIT   = 2,    ( Critical conditions              )
 *   		-- SYSLOG_LEVEL_ERR    = 3,    ( Error conditions                 )
 *   		-- SYSLOG_LEVEL_WARNING= 4,    ( Warning conditions               )
 *   		-- SYSLOG_LEVEL_NOTICE = 5,    ( Normal but significant condition )
 *   		-- SYSLOG_LEVEL_INFO   = 6,    ( Informational messages only      )
 *   		-- SYSLOG_LEVEL_DEBUG  = 7     ( Debugging messages               )
 *	server_ipaddr: smtp server ip address.
 *
 */
typedef struct  SMTP_OM_Config_S
{
    UI32_T  smtp_admin_status;     /* Enable/Disable */
    UI32_T  smtp_level;      /* which level need to log to server */
    UI32_T  server_ipaddr[SYS_ADPT_MAX_NUM_OF_SMTP_SERVER];    /* server ipaddress */
    UI8_T   source_emailaddr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];    /* server ipaddress */
    UI8_T   destination_emailaddr[SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS][SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];    /* server ipaddress */
} SMTP_OM_Config_T;

/*  SMTP_OM_Record_T is structure of the smtp entry.
 *  Note:
 *      owner_info: owner information of this entry.
 *      log_time:   log time of this entry.
 *      message:    message information(0..1024)
 */
typedef struct  SMTP_OM_Record_S
{
    SMTP_OM_RecordOwnerInfo_T   owner_info;
    UI32_T  log_time;       /* seconds
                             * If the device have RTC, the log RTC time is equal with
                             * log_time plus lastest root RTC time.
                             */
    UI8_T   message[SMTP_ADPT_MESSAGE_LENGTH + 1];
}  SMTP_OM_Record_T;

/*  SMTP_OM_QueueRecord_T is structure of the smtp queue entry.
 *  Note:
 *      next:       pointer to next entry
 *      smtp_entry: data to send
 */
typedef struct	SMTP_OM_QueueRecord_S
{
	struct SMTP_OM_QueueRecord_S  *next;
	SMTP_OM_Record_T              smtp_entry;
} SMTP_OM_QueueRecord_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: SMTP_OM_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   initialize smtp_config.
 *
 */
void SMTP_OM_Initiate_System_Resources(void);

/* FUNCTION NAME: SMTP_OM_GetNextSmtpServerIPAddr
 * PURPOSE: This function is used to get next smtp server ip address.
 * INPUT:   *ip_addr -- buffer of server ip address.
 * OUTPUT:  *ip_addr -- value of server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_FAIL   --  fail
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_OM_GetNextSmtpServerIPAddr(UI32_T *ip_addr);

/* FUNCTION NAME: SMTP_OM_SetSmtpServerIPAddr
 * PURPOSE: This function is used to set the smtp server ip address.
 * INPUT:   ip_addr -- value of server ip.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *
 */
UI32_T SMTP_OM_SetSmtpServerIPAddr(UI32_T ip_addr);

/* FUNCTION NAME: SMTP_OM_DeleteSmtpServerIPAddr
 * PURPOSE: This function is used to delete smtp server ip address.
 * INPUT:   ip_addr -- deleted server ip.
 * OUTPUT:  NONE
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_NOT_EXIST   --  input not exist
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. delete one server ip address one time
 *
 */
UI32_T SMTP_OM_DeleteSmtpServerIPAddr(UI32_T ip_addr);

/* FUNCTION NAME: SMTP_OM_IsSmtpServerIPAddrExist
 * PURPOSE: This function is used to check if the smtp server ip address exist in database.
 * INPUT:   ip_addr -- value of server ip.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTPLOG_FAIL   --  set fail
 * NOTES:   None.
 *
 */
UI32_T SMTP_OM_IsSmtpServerIPAddrExist(UI32_T ip_addr);

/* FUNCTION NAME: SMTP_OM_SetSmtpAdminStatus
 * PURPOSE: This function is used to enable/disable smtp admin status.
 * INPUT:   status -- smtp admin status.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_OM_SetSmtpAdminStatus(UI32_T status);

/* FUNCTION NAME: SMTP_OM_GetSmtpAdminStatus
 * PURPOSE: This function is used to get smtp status.
 * INPUT:   *status -- output buffer of smtp admin status
 * OUTPUT:  *status -- smtp admin status.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_OM_GetSmtpAdminStatus(UI32_T *status);

/* FUNCTION NAME: SMTP_OM_GetSmtpSourceEmailAddr
 * PURPOSE: This function is used to get smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address
 * OUTPUT:  *email_addr -- value of smtp source email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_GetSmtpSourceEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_OM_SetSmtpSourceEmailAddr
 * PURPOSE: This function is used to set smtp source email address.
 * INPUT:   *email_addr -- value of smtp source email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_SetSmtpSourceEmailAddr(UI8_T *email_addr);

//ADD daniel 
/* FUNCTION NAME: SMTP_OM_DeleteSmtpSourceEmailAddr
 * PURPOSE: This function is used to delete smtp source email address.
 * INPUT:   *email_addr -- value of smtp source email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_DeleteSmtpSourceEmailAddr(UI8_T *email_addr);


/* FUNCTION NAME: SMTP_OM_GetNextSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_OM_SetSmtpDestinationEmailAddr
 * PURPOSE: This function is used to set smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_EXIST  -- input value already exist
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_SetSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_OM_DeleteSmtpDestinationEmailAddr
 * PURPOSE: This function is used to delete smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_NOT_EXIST   --  input not exist
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr);

/* FUNCTION NAME: SMTP_OM_SetEmailSeverityLevel
 * PURPOSE: This function is used to set smtp email severity level.
 * INPUT:   level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
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
UI32_T SMTP_OM_SetEmailSeverityLevel(UI32_T level);

//ADD daniel 
/* FUNCTION NAME: SMTP_OM_DeleteEmailSeverityLevel
 * PURPOSE: This function is used to delete smtp email severity level to default value.
 * INPUT:   level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
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
UI32_T SMTP_OM_DeleteEmailSeverityLevel(UI32_T level);


/* FUNCTION NAME: SMTP_OM_GetEmailSeverityLevel
 * PURPOSE: This function is used to get smtp email severity level.
 * INPUT:   *level -- output buffer of smtp email severity level.
 * OUTPUT:  *level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
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
UI32_T SMTP_OM_GetEmailSeverityLevel(UI32_T *level);

/* FUNCTION NAME: SMTP_MGR_QueueDequeue
 * PURPOSE: This function is used to get element number of smtp_queue
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  element number of smtp_queue
 * NOTES:   None.
 *
 */
UI32_T SMTP_OM_QueueGetElementNbr(void);

/* FUNCTION NAME: SMTP_MGR_QueueEnqueue
 * PURPOSE: This function is used to enqueue smtp entry.
 * INPUT:   *p -- smtp event data pointer.
 *          *q -- smtp queue pointer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Max queue number is SMTP_MGR_MAX_QUE_NBR
 *
 */
void SMTP_OM_QueueEnqueue(SMTP_OM_QueueRecord_T *qData);

/* FUNCTION NAME: SMTP_MGR_QueueDequeue
 * PURPOSE: This function is used to dequeue smtp entry.
 * INPUT:   *smtp_queue -- smtp queue.
 * OUTPUT:  None.
 * RETUEN:  smtp event data
 * NOTES:   None.
 *
 */
SMTP_OM_QueueRecord_T *SMTP_OM_QueueDequeue(void);

/* FUNCTION NAME: SMTP_OM_ClearQueue
 * PURPOSE: This function is used to clear the smtp entry queue.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_OM_ClearQueue(void);

/* FUNCTION NAME: SMTP_OM_CreatSem
 * PURPOSE: Initiate the semaphore for SMTP objects
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
BOOL_T SMTP_OM_CreatSem(void);

void
SMTP_OM_SetTaskId(
    UI32_T task_id
    );

UI32_T
SMTP_OM_GetTaskId(
    void);
#endif /* SMTP_OM_H */
