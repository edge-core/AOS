/*-----------------------------------------------------------------------------
 * Module   : smtp_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access smtp control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 11/07/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef	SMTP_PMGR_H
#define	SMTP_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "smtp_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SMTP_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_AddSmtpDestinationEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to add the smtp destination email address.
 * INPUT   : *email_addr -- smtp destination email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS        -- success
 *           SMTP_RETURN_INVALID        -- invalid input value
 *           SMTP_RETURN_INPUT_EXIST    -- input value already exist
 *           SMTP_RETURN_DATABASE_FULL  -- database full
 * NOTE    : 1. Add one destination email address one time
 *           2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *           3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_AddSmtpDestinationEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_AddSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to add the smtp server ip address.
 * INPUT    : ip_addr -- smtp server ip address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS    -- success
 *            SMTP_RETURN_FAIL    -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 *            SMTP_RETURN_INPUT_EXIST -- input exist
 *            SMTP_RETURN_DATABASE_FULL -- database full
 * NOTE		: 1. add one server ip address one time.
 *            2. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_AddSmtpServerIPAddr(UI32_T ip_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DisableSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to disable smtp admin status.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS -- success
 *            SMTP_RETURN_FAIL    -- fail
 * NOTE		: 1. status is defined as following:
 *                  SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DisableSmtpAdminStatus(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete the smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTE		: 1. delete one destination email address one time.
 *            2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *            3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete the smtp server ip address.
 * INPUT    : ip_addr -- smtp server ip address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS    -- success
 *            SMTP_RETURN_FAIL    -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 *            SMTP_RETURN_INPUT_NOT_EXIST -- input not exist
 * NOTE		: 1. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. Delete one server ip address one time
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteSmtpServerIPAddr(UI32_T ip_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_EnableSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to enable smtp admin status.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS -- success
 *            SMTP_RETURN_FAIL    -- fail
 * NOTE		: 1. status is defined as following:
 *                  SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_EnableSmtpAdminStatus(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp severity level.
 * INPUT    : None
 * OUTPUT	: *level -- smtp severity level.
 * RETURN   : SMTP_RETURN_SUCCESS        -- success
 *            SMTP_RETURN_FAIL           -- fail
 *            SMTP_RETURN_INVALID_BUFFER -- invalid input buffer
 * NOTE		: 1. smtp severity level symbol is defined as following:
 *              SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *              SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *              SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *              SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *              SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *              SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *              SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *              SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *           2. SYSLOG_LEVEL_EMERG is highest priority.
 *              SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetEmailSeverityLevel(UI32_T *level);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextRunningSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	:  1. Max number of destination email address is
                  SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *             2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetNextRunningSmtpDestinationEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextRunningSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next running smtp server ip address.
 * INPUT    : None
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	: 1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. get one server ip address one time
 *            3. use 0 to get first running ip address
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetNextRunningSmtpServerIPAddr(UI32_T *ip_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SMTP_RETURN_SUCCESS          -- success
 *            SMTP_RETURN_FAIL             -- fail
 *            SMTP_RETURN_INVALID          -- invalid input value
 *            SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTE		: 1. Max number of destination email address is
 *                   SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp server ip address.
 * INPUT    : *ip_addr -- smtp server ip address.
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SMTP_RETURN_SUCCESS       -- success
 *            SMTP_RETURN_FAIL          -- fail
 *            SMTP_RETURN_INVALID       -- invalid input value
 *            SMTP_RETURN_INVALID_BUFFER-- invalid input buffer
 * NOTE		: 1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. get one server ip address one time
 *            3. use 0 to get first ip address
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetNextSmtpServerIPAddr(UI32_T *ip_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get running smtp severity level.
 * INPUT    : None
 * OUTPUT	: *level -- smtp severity level.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	:  1. smtp severity level symbol is defined as following:
 *              SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *              SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *              SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *              SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *              SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *              SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *              SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *              SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *           2. SYSLOG_LEVEL_EMERG is highest priority.
 *              SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningEmailSeverityLevel(UI32_T *level);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get running smtp status.
 * INPUT    : None
 * OUTPUT	: *status -- smtp status.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:     1. status is defined as following:
 *               SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 *            2. default status is SYS_DFLT_SMTP_ADMIN_STATUS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningSmtpAdminStatus(UI32_T *status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get the running smtp source email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp source email address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	: 1. There is only one source email address.
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningSmtpSourceEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp admin status.
 * INPUT    : None
 * OUTPUT	: *status -- smtp status.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTE		: 1. status is defined as following:
 *                  SMTP_STATUS_ENABLE
 *                  SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpAdminStatus(UI32_T *status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get the smtp source email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp source email address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTE		: 1. There is only one source email address
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpSourceEmailAddr(UI8_T *email_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_SetEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to set smtp severity level.
 * INPUT    : level -- smtp seveirity level.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTE		: 1. smtp severity level symbol is defined as following:
 *               SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *               SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *               SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *               SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *               SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *               SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *               SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *               SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *            2. SYSLOG_LEVEL_EMERG is highest priority.
 *               SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_SetEmailSeverityLevel(UI32_T level);

//ADD: daniel
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete smtp severity level to default value.
 * INPUT    : level -- smtp seveirity level.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTES	: 1. smtp severity level symbol is defined as following:
 *               SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *               SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *               SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *               SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *               SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *               SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *               SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *               SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *            2. SYSLOG_LEVEL_EMERG is highest priority.
 *               SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteEmailSeverityLevel(UI32_T level);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_SetSmtpSourceEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to set the smtp source email address.
 * INPUT   : *email_addr -- smtp source email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS  -- success
 *           SMTP_RETURN_FAIL     -- fail
 *           SMTP_RETURN_INVALID  -- invalid input value
 * NOTE    : 1. There is only one source email address
 *           2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_SetSmtpSourceEmailAddr(UI8_T *email_addr);


//ADD daniel 
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_DeleteSmtpSourceEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to delete the smtp source email address.
 * INPUT   : *email_addr -- smtp source email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS  -- success
 *           SMTP_RETURN_FAIL     -- fail
 *           SMTP_RETURN_INVALID  -- invalid input value
 * NOTES   : 1. There is only one source email address
 *           2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_DeleteSmtpSourceEmailAddr(void);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp server ip address.
 * INPUT    : None
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. status is defined as following:
 *               SMTP_STATUS_ENABLE/ SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpServerIPAddr(UI32_T *ip_addr);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp destination email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. There is only one source email address
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpDestinationEmailAddr(UI8_T *email_addr);

#endif /* SMTP_PMGR_H */
