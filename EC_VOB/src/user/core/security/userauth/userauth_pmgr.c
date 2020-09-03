/* ------------------------------------------------------------------------
 *  FILE NAME  -  USERAUTH.C
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the authentication
 *          security stuff which include
 *          1) SNMP Community
 *          2) SNMP Trap Receiver
 *          3) User name, password and Privilege password checking
 *
 * Note:
 *
 *  History
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
 #include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sys_module.h"
#include "sys_bld.h"
#include "userauth.h"
#if (SYS_CPNT_AUTHORIZATION == TRUE)
#include "aaa_mgr.h"
#endif
#include "userauth_pmgr.h"
#include "radius_mgr.h"
#include "radius_type.h"
#include "tacacs_mgr.h"

#ifndef NEWLINE
#define NEWLINE                                             "\r\n"
#endif

#define USERAUTH_PMGR_IS_DEBUG_ERROR_ON(flag)               (flag)

#define USERAUTH_PMGR_PRINT_HEADER()                        \
    {                                                       \
        printf("[%s:%d]" NEWLINE,                           \
            __FUNCTION__, __LINE__);                        \
    }

#define USERAUTH_PMGR_PRINT(fmt, ...)                       \
    {                                                       \
        printf(fmt, ##__VA_ARGS__);                         \
    }

#define USERAUTH_PMGR_LOG(fmt,...)                          \
    {                                                       \
        if (USERAUTH_PMGR_IS_DEBUG_ERROR_ON(userauth_pmgr_dbg_flag))  \
        {                                                   \
            USERAUTH_PMGR_PRINT_HEADER();                   \
            USERAUTH_PMGR_PRINT(fmt NEWLINE, ##__VA_ARGS__);\
        }                                                   \
    }

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;
static BOOL_T userauth_pmgr_dbg_flag = FALSE;

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for L4_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void USERAUTH_PMGR_Init(void)
{
    /* Given that USERAUTH PMGR requests are handled.
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_PMGR_GetNextSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available SNMP community
 *          string can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community string
 * OUTPUT: comm_entry                  - next available SNMP community info
 * RETURN: TRUE/FALSE
 * NOTES: 1. Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *        2. Any invalid(0) community string will be skip duing the GetNext operation.
 */
BOOL_T USERAUTH_PMGR_GetNextSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_entry)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETNEXTSNMPCOMMUNITY;

    /*assign input*/
    msg_data_p->data.comm_entry=*comm_entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *comm_entry=msg_data_p->data.comm_entry;

    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_PMGR_GetNextRunningSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default SNMP community can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community
 * OUTPUT: comm_entry                  - next available non-default SNMP community
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default SNMP community.
 *        3. Community string name is an "ASCII Zero" string (char array ending with '\0').
 *        4. Any invalid(0) SNMP community will be skip during the GetNext operation.
 * ---------------------------------------------------------------------
 */
UI32_T USERAUTH_PMGR_GetNextRunningSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_entry)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETNEXTRUNNINGSNMPCOMMUNITY;

    /*assign input*/
    msg_data_p->data.comm_entry=*comm_entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *comm_entry=msg_data_p->data.comm_entry;

    return msg_data_p->type.result_ui32;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_PMGR_SetSnmpCommunityAccessRight
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the access right can be successfully
 *          set to the specified community string. Otherwise, false is returned.
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *        access_right      - the access level for this SNMP community
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new community string to the system
 *           if the specified comm_string_name does not exist, and total number
 *           of community string configured is less than
 *           SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *           When a new community string is created by this function, the
 *           status of this new community string will be set to disabled(2)
 *           by default.
 *        2. This function will update the access right an existed community
 *           string if the specified comm_string_name existed already.
 *        3. False is returned if total number of community string configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING
 * ---------------------------------------------------------------------
 */
BOOL_T USERAUTH_PMGR_SetSnmpCommunityAccessRight(UI8_T *comm_string_name, UI32_T access_right)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_access)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETSNMPCOMMUNITYACCESSRIGHT;

    /*assign input*/
    memcpy(msg_data_p->data.comm_access.comm_string_name,
        comm_string_name,sizeof(msg_data_p->data.comm_access.comm_string_name));
    msg_data_p->data.comm_access.access_right=access_right;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_PMGR_SetSnmpCommunityStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the status can be successfully
 *          set to the specified community string. Otherwise, false is returned.
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *        status            - the status for this SNMP community(enabled/disabled)
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new community string to the system if
 *           the specified comm_string_name does not exist, and total number
 *           of community string configured is less than
 *           SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *           When a new community string is created by this function, the
 *           access right of this new community string will be set to READ_ONLY(1)
 *           by default.
 *        2. This function will update an existed community string if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of community string configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING
 * ---------------------------------------------------------------------
 */
BOOL_T USERAUTH_PMGR_SetSnmpCommunityStatus(UI8_T *comm_string_name, UI32_T status)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_status)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETSNMPCOMMUNITYSTATUS;

    /*assign input*/
    memcpy(msg_data_p->data.comm_status.comm_string_name,
        comm_string_name,sizeof(msg_data_p->data.comm_status.comm_string_name));
    msg_data_p->data.comm_status.status=status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetLoginLocalUser
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets login user's entry by specified user name
 *  INPUT   : login_user->username (key) in the record
 *  OUTPUT  : login_user a whole record on specified user name
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : If user use the key of super user's username, this function
 *            will return the inhibited super user record.
 */
BOOL_T USERAUTH_PMGR_GetLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.login_user.username)+USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETLOGINLOCALUSER;

    /*assign input*/
    memcpy(msg_data_p->data.login_user.username,
        login_user->username,sizeof(msg_data_p->data.login_user.username));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *login_user=msg_data_p->data.login_user;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetNextLoginLocalUser
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets next legal login user's entry
 *  INPUT   : login_user->username (key) in the record
 *  OUTPUT  : login_user a whole record on specified user name
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : Login user record doesn't sort and for GetNext we just try
 *            to get one more.  So the rule here is,
 *            1) If caller wants to get first user, use null user name
 *               to get first one.
 *            2) If caller wants to get next one, he needs to use the
 *               username which is really in the entry as key to get
 *               next valid entry which index is bigger than the key.
 *            3) Otherwise, if the key can not be matched, then this function
 *               will always return FALSE.
 *            4) This function is not able to get super user
 */
BOOL_T USERAUTH_PMGR_GetNextLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.login_user)+USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETNEXTLOGINLOCALUSER;

    /*assign input*/
    memcpy(msg_data_p->data.login_user.username,
        login_user->username,sizeof(msg_data_p->data.login_user.username));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *login_user=msg_data_p->data.login_user;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_PMGR_GetNextRunningLoginLocalUser
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default user/password can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: login_user->privilege  - (key) to specify a unique user name
 * OUTPUT: user_password            - next available non-default user/password info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user/password.
 * ---------------------------------------------------------------------
 */
UI32_T USERAUTH_PMGR_GetNextRunningLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.login_user)+USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETNEXTRUNNINGLOGINLOCALUSER;

    /*assign input*/
    memcpy(msg_data_p->data.login_user.username,
        login_user->username,sizeof(msg_data_p->data.login_user.username));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *login_user=msg_data_p->data.login_user;

    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
* ROUTINE NAME  - USERAUTH_PMGR_GetRunningAllLoginLocalUser
* ---------------------------------------------------------------------
* PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
*          next available non-default user/password can be retrieved
*          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
*          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
* INPUT:
*         input_size    - input number of user account array
* OUTPUT: login_user_ar - vaild entry array
*         get_number_p  - get existed user number
* RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
*         SYS_TYPE_GET_RUNNING_CFG_FAIL
* NOTES: 1. This function shall only be invoked by CLI to save the
*           "running configuration" to local or remote files.
*        2. Since only non-default configuration will be saved, this
*           function shall return non-default user/password.
* ---------------------------------------------------------------------
*/
SYS_TYPE_Get_Running_Cfg_T USERAUTH_PMGR_GetRunningAllLoginLocalUser(
    USERAUTH_LoginLocal_T login_user_ar[], UI32_T input_size, UI32_T *get_number_p)
{
    const UI32_T msg_buf_size = (sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_account_data)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    /*assign size*/
    req_size = sizeof(msg_data_p->data.auth_account_data) + USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *)&space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p = (USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGALLLOGINLOCALUSER;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (input_size < msg_data_p->data.auth_account_data.user_count)
    {
        *get_number_p = 0;
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(login_user_ar, msg_data_p->data.auth_account_data.alluser_data_ar,
        sizeof(*login_user_ar) * msg_data_p->data.auth_account_data.user_count);

    *get_number_p = msg_data_p->data.auth_account_data.user_count;

    return (SYS_TYPE_Get_Running_Cfg_T)msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetLoginLocalUserStatus
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets username password entry status based on
 *            specified privilege.  If the username is not in the database,
 *            we will create a new user if the user's number is not exceed
 *            the limit.
 *  INPUT   : username (key)
 *          : username_length
 *          : status(USERAUTH_ENTRY_VALID/USERAUTH_ENTRY_INVALID)
 *  OUTPUT  : username
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetLoginLocalUserStatus(UI8_T *username, UI32_T status)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.user_status)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETLOGINLOCALUSERSTATUS;

    /*assign input*/
    memcpy(msg_data_p->data.user_status.username,
        username,sizeof(msg_data_p->data.user_status.username));
    msg_data_p->data.user_status.status=status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetLoginLocalUserPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets password based on specified privilege
 *  INPUT   : username (key)
 *          : password
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetLoginLocalUserPassword(UI8_T *username, UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.user_password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETLOGINLOCALUSERPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.user_password.username,
        username,sizeof(msg_data_p->data.user_password.username));
    memcpy(msg_data_p->data.user_password.password,
        password,sizeof(msg_data_p->data.user_password.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetLoginLocalUserPrivilege
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets password based on specified privilege
 *  INPUT   : username (key)
 *          : username_length
 *          : password
 *          : privilege
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetLoginLocalUserPrivilege(UI8_T *username, UI32_T privilege)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.user_privilege)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETLOGINLOCALUSERPRIVILEGE;

    /*assign input*/
    memcpy(msg_data_p->data.user_privilege.username,
        username,sizeof(msg_data_p->data.user_privilege.username));
    msg_data_p->data.user_privilege.privilege=privilege;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetPrivilegePassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *  OUTPUT  : privilege_password in record
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(privilege_password->privilege)+USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETPRIVILEGEPASSWORD;

    /*assign input*/
    msg_data_p->data.privilege_password.privilege=privilege_password->privilege;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *privilege_password=msg_data_p->data.privilege_password;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetFirstRunningPrivilegePassword
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default privilege password can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : privilege_password->privilege  - (key) to specify a unique user/password
 * OUTPUT: password            - next available non-default password info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 *        3. This function is a very particular API.  Since the privilege
 *           is count from 0.  But if we only support GetNext, then we can
 *           not get the first one.  So this is the Get First Running Config.
 */
BOOL_T USERAUTH_PMGR_GetFirstRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size = resp_size = msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETFIRSTRUNNINGPRIVILEGEPASSWORD;

    /*assign input*/
    msg_data_p->data.privilege_password.privilege=privilege_password->privilege;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *privilege_password=msg_data_p->data.privilege_password;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetNextRunningPrivilegePassword
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default privilege password can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : privilege_password->privilege  - (key) to specify a unique user/password
 * OUTPUT: password            - next available non-default password info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
BOOL_T USERAUTH_PMGR_GetNextRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(privilege_password->privilege)+USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETNEXTRUNNINGPRIVILEGEPASSWORD;

    /*assign input*/
    msg_data_p->data.privilege_password.privilege=privilege_password->privilege;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *privilege_password=msg_data_p->data.privilege_password;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetPrivilegePasswordStatus
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *            privilege_password
 *  OUTPUT  : status(USERAUTH_ENTRY_VALID/USERAUTH_ENTRY_INVALID)
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : User needs to be careful that he needs to set privilege password
 *            first and then set the status valid.  Otherwise, the password
 *            could be anything(previous setting).
 */
BOOL_T USERAUTH_PMGR_SetPrivilegePasswordStatus(UI32_T privilege, UI32_T status)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.ui32a1_ui32a2)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETPRIVILEGEPASSWORDSTATUS;

    /*assign input*/
    msg_data_p->data.ui32a1_ui32a2.ui32_a1=privilege;
    msg_data_p->data.ui32a1_ui32a2.ui32_a2=status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetPrivilegePassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *            password
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetPrivilegePassword(UI32_T privilege, UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETPRIVILEGEPASSWORD;

    /*assign input*/
    msg_data_p->data.privilege_password.privilege=privilege;
    memcpy(msg_data_p->data.privilege_password.password,
        password,sizeof(msg_data_p->data.privilege_password.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets authentication method
 *  INPUT   : auth_method
 *  OUTPUT  : auth_method
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */

/* design changed for TACACS by JJ, 2002-05-24 */

BOOL_T USERAUTH_PMGR_GetAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETAUTHMETHOD;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(auth_method,msg_data_p->data.auth_method_numbers,sizeof(msg_data_p->data.auth_method_numbers));

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets authentication method
 *  INPUT   : auth_method
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */

/* design changed for TACACS by JJ, 2002-05-24 */

BOOL_T USERAUTH_PMGR_SetAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETAUTHMETHOD;

    /*assign input*/
    memcpy(msg_data_p->data.auth_method_numbers,auth_method,sizeof(msg_data_p->data.auth_method_numbers));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningAuthMethod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          authentication method is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : auth_method
 * OUTPUT: auth_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGAUTHMETHOD;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(auth_method,msg_data_p->data.auth_method_numbers,sizeof(msg_data_p->data.auth_method_numbers));

    return msg_data_p->type.result_ui32;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetEnableAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets enable authentication method
 *  INPUT   : enable_auth_method
 *  OUTPUT  : enable_auth_method
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETENABLEAUTHMETHOD;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(enable_auth_method,msg_data_p->data.auth_method_numbers,sizeof(msg_data_p->data.auth_method_numbers));

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetEnableAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets enable authentication method
 *  INPUT   : enable_auth_method
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETENABLEAUTHMETHOD;

    /*assign input*/
    memcpy(msg_data_p->data.auth_method_numbers,enable_auth_method,sizeof(msg_data_p->data.auth_method_numbers));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningEnableAuthMethod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          authentication method is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : enable_auth_method
 * OUTPUT: enable_auth_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGENABLEAUTHMETHOD;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(enable_auth_method,msg_data_p->data.auth_method_numbers,sizeof(msg_data_p->data.auth_method_numbers));

    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetTelnetLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets telnet login method
 *  INPUT   : login_method
 *  OUTPUT  : login_method
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetTelnetLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETTELNETLOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=*login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *login_method=msg_data_p->data.login_method;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetTelnetLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets telnet login method
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetTelnetLoginMethod(USERAUTH_Login_Method_T login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETTELNETLOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningTelnetLoginMethod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          telnet login method is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : login_method
 * OUTPUT: login_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningTelnetLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGTELNETLOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=*login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *login_method=msg_data_p->data.login_method;

    return msg_data_p->type.result_ui32;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetConsolLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets console login method
 *  INPUT   : login_method
 *  OUTPUT  : login_method
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetConsolLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETCONSOLLOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=*login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *login_method=msg_data_p->data.login_method;

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetConsoleLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets console login method
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetConsoleLoginMethod(USERAUTH_Login_Method_T login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETCONSOLELOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningConsoleLoginMethod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          console login method is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : login_method
 * OUTPUT: login_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningConsoleLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGCONSOLELOGINMETHOD;

    /*assign input*/
    msg_data_p->data.login_method=*login_method;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *login_method=msg_data_p->data.login_method;

    return msg_data_p->type.result_ui32;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetConsoleLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets console login password
 *  INPUT   : password
 *  OUTPUT  : password
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetConsoleLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETCONSOLELOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(password,msg_data_p->data.password
        ,sizeof(msg_data_p->data.password));

    return msg_data_p->type.result_bool;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetConsoleLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets console login password
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetConsoleLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETCONSOLELOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningConsoleLoginPassword
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          console login password is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : login_method
 * OUTPUT: login_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningConsoleLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGCONSOLELOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(password,msg_data_p->data.password
        ,sizeof(msg_data_p->data.password));

    return msg_data_p->type.result_ui32;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetTelnetLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets telnet login password
 *  INPUT   : password
 *  OUTPUT  : password
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_GetTelnetLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETTELNETLOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(password,msg_data_p->data.password
        ,sizeof(msg_data_p->data.password));

    return msg_data_p->type.result_bool;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_SetTelnetLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets telnet login password
 *  INPUT   : password
 *  OUTPUT  : None
 *  RETURN	: BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_SetTelnetLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETTELNETLOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_GetRunningTelnetLoginPassword
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          telnet login password is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT : login_method
 * OUTPUT: login_method
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default privilege password.
 */
UI32_T USERAUTH_PMGR_GetRunningTelnetLoginPassword(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETRUNNINGTELNETLOGINPASSWORD;

    /*assign input*/
    memcpy(msg_data_p->data.password,
        password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(password,msg_data_p->data.password
        ,sizeof(msg_data_p->data.password));

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_PMGR_LoginAuthPassByWhichMethod
 *-------------------------------------------------------------------------
 * PURPOSE  : to get the auth-method that USERAUTH_LoginAuth() used
 * INPUT    : none
 * OUTPUT   : auth_method
 * RETURN   : TRUE - success
{
}

 FALSE - fail
 * NOTES    : if the return value of USERAUTH_LoginAuth() is FALSE (fail to auth),
 *            can't get auth_method (because it didn't pass authentication)
 *-------------------------------------------------------------------------*/
BOOL_T USERAUTH_PMGR_LoginAuthPassByWhichMethod(USERAUTH_Auth_Method_T *auth_method)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_LOGINAUTHPASSBYWHICHMETHOD;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *auth_method=msg_data_p->data.auth_method;

    return msg_data_p->type.result_bool;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_PMGR_EnablePasswordAuth
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets privilege password based on specified
 *            privilege
 * INPUT : password -- password of the user (SYS_DFLT_ENABLE_PASSWORD_USERNAME)
 * OUTPUT: priv -- priviledge if return TRUE, otherwise error number is output
 * RETURN: TRUE -- authentication successfully, or
 *         FALSE -- authentication fail
 * NOTES: For RADIUS and TACACS, in addition to authentication,
 *        authorization is also done in outputed priviledge
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_PMGR_EnablePasswordAuth(
    const char *name,
    const char *password,
    USERAUTH_SessionType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    UI32_T *privilege)
{
#define req     data.enable_auth.req
#define resp    data.enable_auth.resp

    const UI32_T req_msg_size = USERAUTH_MSGBUF_TYPE_SIZE +
                                sizeof(((USERAUTH_IPCMsg_T *)0)->req);
    const UI32_T resp_msg_size = USERAUTH_MSGBUF_TYPE_SIZE +
                                sizeof(((USERAUTH_IPCMsg_T *)0)->resp);
    const UI32_T msg_size = (req_msg_size > resp_msg_size) ? req_msg_size : resp_msg_size;

    UI8_T ipc_msg_buf[SYSFUN_SIZE_OF_MSG(msg_size)];

    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipc_msg_buf;
    UI32_T rv = 0;

    USERAUTH_IPCMsg_T *msg_data_p;

    memset(ipc_msg_buf, 0, sizeof(ipc_msg_buf));

    sysfun_msg_p->cmd = SYS_MODULE_USERAUTH;
    sysfun_msg_p->msg_size = req_msg_size;

    msg_data_p = (USERAUTH_IPCMsg_T *)sysfun_msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_ENABLE_PASSWORD_AUTH;

    memcpy(msg_data_p->req.name, name, sizeof(msg_data_p->req.name));
    memcpy(msg_data_p->req.password, password, sizeof(msg_data_p->req.password));
    msg_data_p->req.sess_type = sess_type;
    msg_data_p->req.sess_id = sess_id;

    if (NULL != rem_ip_addr)
    {
        msg_data_p->req.rem_ip_addr = *rem_ip_addr;
    }

    msg_data_p->req.privilege = *privilege;

    USERAUTH_PMGR_LOG("Request> name=%s, password=***, sess_type=%d, sess_id=%lu, privilege=%lu",
        name,
        sess_type,
        (unsigned long)sess_id,
        (unsigned long)*privilege);

    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_msg_size, sysfun_msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, (long)rv);
        return FALSE;
    }

    USERAUTH_PMGR_LOG("Result> bool=%s, privilege=%lu, auth_by_whom=%d",
        (TRUE == msg_data_p->type.result_bool) ? "TRUE" : "FALSE",
        (unsigned long)msg_data_p->resp.result.privilege,
        msg_data_p->resp.result.authen_by_whom);

    if (TRUE == msg_data_p->type.result_bool)
    {
        *privilege = msg_data_p->resp.result.privilege;
    }

    return msg_data_p->type.result_bool;

#undef req
#undef resp
}



/* FUNCTION NAME:  USERAUTH_PMGR_SetPasswordRecoveryActive
 * PURPOSE:
 *          This function set status of PasswordRecoveryActive.
 *
 * INPUT:
 *          BOOL_T  active  --  status of password recovery active.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          BOOL_T --  TRUE or FALSE.
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_SetPasswordRecoveryActive(BOOL_T active)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.bool_v)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETPASSWORDRECOVERYACTIVE;

    /*assign input*/
    msg_data_p->data.bool_v=active;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}



/* FUNCTION NAME:  USERAUTH_PMGR_GetPasswordRecoveryActive
 * PURPOSE:
 *          This function get status of PasswordRecoveryActive.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *active --  status of password recovery active.
 *
 * RETURN:
 *          BOOL_T --  TRUE or FALSE.
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_GetPasswordRecoveryActive(BOOL_T *active)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.bool_v)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=USERAUTH_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETPASSWORDRECOVERYACTIVE;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *active=msg_data_p->data.bool_v;

    return msg_data_p->type.result_bool;
}



/* FUNCTION NAME:  USERAUTH_PMGR_DoPasswordRecovery
 * PURPOSE:
 *          This function to reset admin's password or set new admin's password.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *active --  status of password recovery active.
 *
 * RETURN:
 *          BOOL_T --  TRUE or FALSE.
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_DoPasswordRecovery(UI8_T *password)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_DOPASSWORDRECOVERY;

    /*assign input*/
    memcpy(msg_data_p->data.password
        ,password,sizeof(msg_data_p->data.password));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}



/* FUNCTION NAME:  USERAUTH_PMGR_LoginAuthBySessionType
 * PURPOSE:
 *          This function authenticate the user by session type who is trying
 *          to login and output the priviledge of this user if he/she is
 *          authenticated otherwise returns FALSE to indicate the failure of
 *          authentication
 *
 * INPUT:
 *          name             --  name of user trying to login
 *          password         --  password of the user
 *          sess_type        --  session type of user trying to login
 *          sess_id          --  (optional) CLI session ID. This parameter is
 *                               required for authenticating user who login via
 *                               CLI, telnet, or SSH only
 *          rem_ip_addr      --  (optional) caller IP address. This parameter is
 *                               required for user login via telnet, SSH, or WEB
 *
 * OUTPUT:
 *          auth_result_p    --  authentication result
 *
 * RETURN:
 *          TRUE  -- authentication successfully, or
 *          FALSE -- authentication fail
 * NOTES:
 *          None
 */
BOOL_T USERAUTH_PMGR_LoginAuthBySessionType(
    const char *name,
    const char *password,
    USERAUTH_SessionType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_ip_addr,
    USERAUTH_AuthResult_T *auth_result_p)
{
#define req     data.login_auth.req
#define resp    data.login_auth.resp

    const UI32_T req_msg_size = USERAUTH_MSGBUF_TYPE_SIZE +
                                sizeof(((USERAUTH_IPCMsg_T *)0)->req);
    const UI32_T resp_msg_size = USERAUTH_MSGBUF_TYPE_SIZE +
                                sizeof(((USERAUTH_IPCMsg_T *)0)->resp);
    const UI32_T msg_size = (req_msg_size > resp_msg_size) ? req_msg_size : resp_msg_size;

    UI8_T ipc_msg_buf[SYSFUN_SIZE_OF_MSG(msg_size)];

    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipc_msg_buf;
    UI32_T rv = 0;

    USERAUTH_IPCMsg_T *msg_data_p;

    memset(ipc_msg_buf, 0, sizeof(ipc_msg_buf));

    sysfun_msg_p->cmd = SYS_MODULE_USERAUTH;
    sysfun_msg_p->msg_size = req_msg_size;

    msg_data_p = (USERAUTH_IPCMsg_T *)sysfun_msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_LOGIN_AUTH_BY_SESSION_TYPE;

    memcpy(msg_data_p->req.name, name, sizeof(msg_data_p->req.name));
    memcpy(msg_data_p->req.password, password, sizeof(msg_data_p->req.password));
    msg_data_p->req.sess_type = sess_type;
    msg_data_p->req.sess_id = sess_id;

    if (NULL != rem_ip_addr)
    {
        msg_data_p->req.rem_ip_addr = *rem_ip_addr;
    }

    USERAUTH_PMGR_LOG("Request> name=%s, password=%s, sess_type=%d, sess_id=%lu",
        name,
        password,
        sess_type,
        (unsigned long)sess_id);

    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_msg_size, sysfun_msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, (long)rv);
        return FALSE;
    }

    USERAUTH_PMGR_LOG("Result> bool=%s, privilege=%lu, auth_by_whom=%d",
        (TRUE == msg_data_p->type.result_bool) ? "TRUE" : "FALSE",
        (unsigned long)msg_data_p->resp.result.privilege,
        msg_data_p->resp.result.authen_by_whom);

    if (TRUE == msg_data_p->type.result_bool)
    {
        *auth_result_p = msg_data_p->resp.result;
    }

    return msg_data_p->type.result_bool;

#undef req
#undef resp
}


/* FUNCTION NAME:  USERAUTH_PMGR_SetAuthMethodBySessionType
 * PURPOSE:
 *          This function sets authentication method by session type.
 *
 * INPUT:
 *          USERAUTH_Auth_Method_T  *auth_method    --  auth_method
 *          UI32_T                  sess_type       --  session type
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          BOOL_T  --  TRUE  -- successfully, or
 *                      FALSE -- fail
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_SetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.method_type)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=USERAUTH_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_SETAUTHMETHODBYSESSIONTYPE;

    /*assign input*/
    memcpy(msg_data_p->data.method_type.auth_method_numbers,
        auth_method,sizeof(msg_data_p->data.method_type.auth_method_numbers));
    msg_data_p->data.method_type.sess_type=sess_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/

    return msg_data_p->type.result_bool;
}

/* FUNCTION NAME:  USERAUTH_PMGR_GetAuthMethodBySessionType
 * PURPOSE:
 *          This function gets authentication method by session type.
 *
 * INPUT:
 *          UI32_T                  sess_type       --  session type
 *
 * OUTPUT:
 *          USERAUTH_Auth_Method_T  *auth_method    --  auth_method
 *
 * RETURN:
 *          BOOL_T  --  TRUE  -- successfully, or
 *                      FALSE -- fail
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_GetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type)
{
    const UI32_T msg_buf_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.method_type)
        + USERAUTH_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    USERAUTH_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_USERAUTH;
    msg_p->msg_size = req_size;

    msg_data_p=(USERAUTH_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = USERAUTH_IPCCMD_GETAUTHMETHODBYSESSIONTYPE;

    /*assign input*/
    msg_data_p->data.method_type.sess_type=sess_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(auth_method,msg_data_p->data.method_type.auth_method_numbers,
        sizeof(msg_data_p->data.method_type.auth_method_numbers));

    return msg_data_p->type.result_bool;
}



