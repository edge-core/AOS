#ifndef _USERAUTH_PMGR_H_
#define _USERAUTH_PMGR_H_

#include "userauth.h"


void USERAUTH_PMGR_Init(void);

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
BOOL_T USERAUTH_PMGR_GetNextSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry);
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
UI32_T USERAUTH_PMGR_GetNextRunningSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry);
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
BOOL_T USERAUTH_PMGR_SetSnmpCommunityAccessRight(UI8_T *comm_string_name, UI32_T access_right);
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
BOOL_T USERAUTH_PMGR_SetSnmpCommunityStatus(UI8_T *comm_string_name, UI32_T status);
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
BOOL_T USERAUTH_PMGR_GetLoginLocalUser(USERAUTH_LoginLocal_T *login_user);
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
BOOL_T USERAUTH_PMGR_GetNextLoginLocalUser(USERAUTH_LoginLocal_T *login_user);
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
UI32_T USERAUTH_PMGR_GetNextRunningLoginLocalUser(USERAUTH_LoginLocal_T *login_user);

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
BOOL_T USERAUTH_PMGR_SetLoginLocalUserStatus(UI8_T *username, UI32_T status);
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
BOOL_T USERAUTH_PMGR_SetLoginLocalUserPassword(UI8_T *username, UI8_T *password);
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
BOOL_T USERAUTH_PMGR_SetLoginLocalUserPrivilege(UI8_T *username, UI32_T privilege);
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
BOOL_T USERAUTH_PMGR_GetPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password);
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
BOOL_T USERAUTH_PMGR_GetFirstRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password);
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
BOOL_T USERAUTH_PMGR_GetNextRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password);
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
BOOL_T USERAUTH_PMGR_SetPrivilegePasswordStatus(UI32_T privilege, UI32_T status);
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
BOOL_T USERAUTH_PMGR_SetPrivilegePassword(UI32_T privilege, UI8_T *password);
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

BOOL_T USERAUTH_PMGR_GetAuthMethod(USERAUTH_Auth_Method_T *auth_method);
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

BOOL_T USERAUTH_PMGR_SetAuthMethod(USERAUTH_Auth_Method_T *auth_method);
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
UI32_T USERAUTH_PMGR_GetRunningAuthMethod(USERAUTH_Auth_Method_T *auth_method);
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
BOOL_T USERAUTH_PMGR_GetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method);
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
BOOL_T USERAUTH_PMGR_SetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method);
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
UI32_T USERAUTH_PMGR_GetRunningEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method);
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
BOOL_T USERAUTH_PMGR_GetTelnetLoginMethod(USERAUTH_Login_Method_T *login_method);
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
BOOL_T USERAUTH_PMGR_SetTelnetLoginMethod(USERAUTH_Login_Method_T login_method);
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
UI32_T USERAUTH_PMGR_GetRunningTelnetLoginMethod(USERAUTH_Login_Method_T *login_method);
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
BOOL_T USERAUTH_PMGR_GetConsolLoginMethod(USERAUTH_Login_Method_T *login_method);
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
BOOL_T USERAUTH_PMGR_SetConsoleLoginMethod(USERAUTH_Login_Method_T login_method);
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
UI32_T USERAUTH_PMGR_GetRunningConsoleLoginMethod(USERAUTH_Login_Method_T *login_method);

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
BOOL_T USERAUTH_PMGR_GetConsoleLoginPassword(UI8_T *password);

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
BOOL_T USERAUTH_PMGR_SetConsoleLoginPassword(UI8_T *password);

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
UI32_T USERAUTH_PMGR_GetRunningConsoleLoginPassword(UI8_T *password);

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
BOOL_T USERAUTH_PMGR_GetTelnetLoginPassword(UI8_T *password);

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
BOOL_T USERAUTH_PMGR_SetTelnetLoginPassword(UI8_T *password);

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
UI32_T USERAUTH_PMGR_GetRunningTelnetLoginPassword(UI8_T *password);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_PMGR_LoginAuthPassByWhichMethod
 *-------------------------------------------------------------------------
 * PURPOSE  : to get the auth-method that USERAUTH_PMGR_LoginAuth() used
 * INPUT    : none
 * OUTPUT   : auth_method
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if the return value of USERAUTH_PMGR_LoginAuth() is FALSE (fail to auth),
 *            can't get auth_method (because it didn't pass authentication)
 *-------------------------------------------------------------------------*/
BOOL_T USERAUTH_PMGR_LoginAuthPassByWhichMethod(USERAUTH_Auth_Method_T *auth_method);

BOOL_T USERAUTH_PMGR_EnablePasswordAuth(
    const char *name,
    const char *password,
    USERAUTH_SessionType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    UI32_T *privilege
);

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
BOOL_T USERAUTH_PMGR_SetPasswordRecoveryActive(BOOL_T active);

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
BOOL_T USERAUTH_PMGR_GetPasswordRecoveryActive(BOOL_T *active);

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
BOOL_T USERAUTH_PMGR_DoPasswordRecovery(UI8_T *password);

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
    USERAUTH_AuthResult_T *auth_result_p
);

/* FUNCTION NAME:  USERAUTH_PMGR_SetAuthMethodBySessionType
 * PURPOSE:
 *          This function sets authentication method by session type.
 *
 * INPUT:
 *          USERAUTH_PMGR_Auth_Method_T  *auth_method    --  auth_method
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
BOOL_T USERAUTH_PMGR_SetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type);

/* FUNCTION NAME:  USERAUTH_PMGR_GetAuthMethodBySessionType
 * PURPOSE:
 *          This function gets authentication method by session type.
 *
 * INPUT:
 *          UI32_T                  sess_type       --  session type
 *
 * OUTPUT:
 *          USERAUTH_PMGR_Auth_Method_T  *auth_method    --  auth_method
 *
 * RETURN:
 *          BOOL_T  --  TRUE  -- successfully, or
 *                      FALSE -- fail
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_PMGR_GetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type);

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
    USERAUTH_LoginLocal_T login_user_ar[], UI32_T input_size, UI32_T *get_number_p);

#endif

