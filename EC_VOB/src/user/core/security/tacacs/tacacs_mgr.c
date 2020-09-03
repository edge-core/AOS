/* Project Name: New Feature
 * Module Name : tacacs_mgr.C
 * Abstract    : This file is used to change the operation in TACACStask
 * Purpose     : to change the TACACS opearion mode
 *
 * 2002/05/06    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sysfun.h"
#include "tacacs_mgr.h"
#include "libtacacs.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "tacacs_om_private.h"
#include "tacacs_om.h"
#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#include "aaa_om.h"
#include "aaa_om_private.h"
#endif

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#include "sys_time.h"
#include "tac_account_c.h"
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) || \
 (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)
#include "tac_author_c.h"
#endif

#include "security_backdoor.h"
#include "sys_callback_mgr.h"
#include "l_stdlib.h"
#include "ip_lib.h"

#define TACACS_USE_CSC(a)
#define TACACS_RELEASE_CSC()
#define TACACS_USE_CSC_WITHOUT_RETURN_VALUE()

#define TACACS_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL) \
    TACACS_USE_CSC(RET_VAL); \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
       TACACS_RELEASE_CSC(); \
       return (RET_VAL); \
    }

#define TACACS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE() \
    TACACS_USE_CSC_WITHOUT_RETURN_VALUE(); \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
        TACACS_RELEASE_CSC(); \
        return; \
    }

#define TACACS_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) { \
        TACACS_RELEASE_CSC(); \
        return (RET_VAL); \
    }

#define TACACS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() { \
        TACACS_RELEASE_CSC(); \
        return; \
    }

#define TACACS_MGR_TRACE(fmt, args...)                          \
    {                                                           \
        if(SECURITY_BACKDOOR_IsOn(tacacs_mgr_backdoor_reg_no))  \
        {                                                       \
             printf("[%s:%d] ", __FUNCTION__, __LINE__);        \
             printf(fmt, ##args);                               \
             printf("\r\n");                                    \
        }                                                       \
    }



#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
static BOOL_T TACACS_MGR_AsyncAccountingRequest_Callback(const AAA_AccRequest_T *request);
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
static BOOL_T TACACS_MGR_LocalDestroyServerHost(UI32_T server_index);

/*------------------------------------------------------------------------
 * LOCAL VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
/* STATIC VARIABLE DECLARATIONS
 */
/* static SYS_TYPE_Stacking_Mode_T tacacs_operation_mode; */ /*Charles*/
/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#define TACACS_TASK_MSG_NO    10
static UI32_T tacacs_acc_queue_id;
static UI32_T tacacs_main_task_id;
#endif

static UI32_T tacacs_mgr_backdoor_reg_no;
static UI32_T tacacs_mgr_backdoor_reg_no_authen_c;
static UI32_T tacacs_mgr_backdoor_reg_no_author_c;
static UI32_T tacacs_mgr_backdoor_reg_no_acct;
static UI32_T tacacs_mgr_backdoor_reg_no_acct_c;
static UI32_T tacacs_mgr_backdoor_reg_no_packet;

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_IsValidIP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check the IP address is valid or not
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
static BOOL_T 
TACACS_MGR_IsValidIP(
    UI32_T serverip
    );

BOOL_T TACACS_MGR_Initiate_System_Resources(void)
{
 BOOL_T result;
    /* create semaphore */
    result = TACACS_OM_CreatSem();
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    tacacs_main_task_id = 0;
    AAA_MGR_Register_AccComponent_Callback(AAA_ACC_CPNT_TACACS_PLUS, TACACS_MGR_AsyncAccountingRequest_Callback);
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
    TACACS_MGR_SetConfigSettingToDefault();

    SECURITY_BACKDOOR_Register("tacacs_mgr",    &tacacs_mgr_backdoor_reg_no);
    SECURITY_BACKDOOR_Register("tac_authen_c",  &tacacs_mgr_backdoor_reg_no_authen_c);
    SECURITY_BACKDOOR_Register("tac_author_c",  &tacacs_mgr_backdoor_reg_no_author_c);
    SECURITY_BACKDOOR_Register("tac_acct_c",    &tacacs_mgr_backdoor_reg_no_acct_c);
    SECURITY_BACKDOOR_Register("tac_acct",      &tacacs_mgr_backdoor_reg_no_acct);
    SECURITY_BACKDOOR_Register("tac_packet",    &tacacs_mgr_backdoor_reg_no_packet);

    return result;
} /* End of TACACS_MGR_Initiate_System_Resources */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_GetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get debug flag
 * INPUT:    file_type
 * OUTPUT:   None.
 * RETURN:   If succeeded, debug flag is returned. Otherwise, 0 is returned.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_GetDebugFlag(UI32_T file_type)
{
    UI32_T reg_no;

    switch (file_type)
    {
        case TACACS_MGR:
            reg_no = tacacs_mgr_backdoor_reg_no;
            break;

        case TACACS_AUTHEN_C:
            reg_no = tacacs_mgr_backdoor_reg_no_authen_c;
            break;

        case TACACS_AUTHOR_C:
            reg_no = tacacs_mgr_backdoor_reg_no_author_c;
            break;

        case TACACS_ACCT_C:
            reg_no = tacacs_mgr_backdoor_reg_no_acct_c;
            break;

        case TACACS_ACCT:
            reg_no = tacacs_mgr_backdoor_reg_no_acct;
            break;

        case TACACS_PACKET:
            reg_no = tacacs_mgr_backdoor_reg_no_packet;
            break;

        default:
            return 0;
    }

    return SECURITY_BACKDOOR_IsOn(reg_no);
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_CreateTaskMsgQ
 *---------------------------------------------------------------------------
 * PURPOSE:  This function create msgQ for task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_CreateTaskMsgQ(void)
{
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    /* Create Queue */
    if(SYSFUN_CreateMsgQ(SYS_BLD_CSC_TACACS_TASK_EVENT_ACC_REQ,
        SYSFUN_MSGQ_UNIDIRECTIONAL, &tacacs_acc_queue_id) != SYSFUN_OK)
    {
        return FALSE;
    }
#endif
	return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE:  This function initializes all function pointer registration operations.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_Create_InterCSC_Relation(void)
{
    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_CurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of TACACS's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Tacacs_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */

SYS_TYPE_Stacking_Mode_T TACACS_MGR_CurrentOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE(); /*added by Charles*/
    /*return tacacs_operation_mode;*/       /*Marked by Charles*/
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_EnterMasterMode()
{
	/* tacacs_operation_mode = SYS_TYPE_STACKING_MASTER_MODE; */ /*Marked by Charles*/

	/* set mgr in master mode */
	SYSFUN_ENTER_MASTER_MODE();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_EnterSlaveMode()
{
   SYSFUN_ENTER_SLAVE_MODE();

   /*tacacs_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;*/ /*Marked by Charles*/
}

/* FUNCTION	NAME : TACACS_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void TACACS_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */

    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}	/*	end of TACACS_MGR_SetTransitionMode	*/

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_EnterTransitionMode()
{
   SYSFUN_ENTER_TRANSITION_MODE();

    TACACS_MGR_SetConfigSettingToDefault(); /*maggie liu*/
   /*tacacs_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;*/ /*Marked by Charles*/
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    TACACS_OM_AccInitialize(); /* should we re-initialize tacacs om ? */
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_MGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to TACACS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        TACACS_MGR_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetConfigSettingToDefault()
{
   return TACACS_OM_SetConfigSettingToDefault();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TCP port number of the remote TACACS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Set_Server_Port(UI32_T serverport)
{
  BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    ret =  TACACS_OM_Set_Server_Port(serverport);
    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_SetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global retransmit of the remote TACACS server
 * INPUT  :  retransmit
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetServerRetransmit(UI32_T retransmit)
{
    BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    ret =  TACACS_OM_SetServerRetransmit(retransmit);
    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_SetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global timeout of the remote TACACS server
 * INPUT  :  timeout
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetServerTimeout(UI32_T timeout)
{
    BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    ret =  TACACS_OM_SetServerTimeout(timeout);
    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Set_Server_Secret(UI8_T *serversecret)
{
  BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret =  TACACS_OM_Set_Server_Secret(serversecret);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_IsValidIP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check the IP address is valid or not
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
static BOOL_T 
TACACS_MGR_IsValidIP(
    UI32_T serverip)
{
    if(IP_LIB_IsValidForRemoteIp((UI8_T *) &serverip) != IP_LIB_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote TACACS server
 * INPUT:    TACACS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T 
TACACS_MGR_Set_Server_IP(
    UI32_T serverip)
{
  BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (TRUE == TACACS_MGR_IsValidIP(serverip))
    {
        ret =  TACACS_OM_Set_Server_IP(serverip);
    }
    else
    {
        ret = FALSE;
    }

    ret =  TACACS_OM_Set_Server_IP(serverip);

    if(ret == FALSE)
    	EH_MGR_Handle_Exception(SYS_MODULE_TACACS, TACACS_MGR_SET_SERVER_IP_FUNC_NO, EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);/*Mercury_V2-00030*/

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#if (SYS_CPNT_TACACS_PLUS_AUTHENTICATION == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    TACACS Authentication username  and password
 * RETURN:   TACACS Authentication result
 *           TACACS_AUTHENTICATION_CONNECT_FAIL     -1 = connect error
 *           TACACS_AUTHENTICATION_SUCCESS           1 = Authentication PASS
 *           TACACS_AUTHENTICATION_FAIL              2 = Authentication FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
I32_T TACACS_MGR_Auth_Check(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_addr,
    UI32_T *privilege)
{
    I32_T   ret = TAC_PLUS_AUTHEN_STATUS_FAIL;
    UI32_T  server_ip, server_port, retransmit, timeout;
    UI8_T   server_secret[TACACS_AUTH_KEY_MAX_LEN + 1];

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(TACACS_NOT_IN_MASTER_MODE_RC);

    TACACS_MGR_TRACE("username=%s, password=***\n", username);

    server_ip = 0;

#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE)
    {
        TACACS_Server_Host_T server_host;

        /* only support 1 server now
         */
        if (TRUE == TACACS_OM_Get_Server_Host(1, &server_host))
        {
            server_ip   = server_host.server_ip;
            server_port = server_host.server_port;
            retransmit  = server_host.retransmit;
            timeout     = server_host.timeout;
            memcpy(server_secret, server_host.secret, TACACS_AUTH_KEY_MAX_LEN);
            server_secret[TACACS_AUTH_KEY_MAX_LEN] = '\0';
        }
    }
#else
    server_ip   = TACACS_OM_Get_Server_IP();
    server_port = TACACS_OM_Get_Server_Port();
    retransmit  = TACACS_OM_GetServerRetransmit();
    timeout     = TACACS_OM_GetServerTimeout();
    memcpy(server_secret, TACACS_OM_Get_Server_Secret(), TACACS_AUTH_KEY_MAX_LEN);
    server_secret[TACACS_AUTH_KEY_MAX_LEN] = '\0';

#endif /* SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER */

    if (server_ip == 0)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(TACACS_AUTHENTICATION_CONNECT_FAIL);
    }

    /*maggie liu for authorization*/
    ret = tacacs_main_ascii_login(server_ip,
                                  server_secret,
                                  server_port,
                                  retransmit,
                                  timeout,
                                  (char *)username,
                                  (char *)password,
                                  TACACS_ASCII_LOGIN,
                                  TAC_PLUS_PRIV_LVL_MIN,
                                  sess_type,
                                  sess_id,
                                  rem_addr);

    /* fail to do user/pswd authentication */
    if (TAC_PLUS_AUTHEN_STATUS_PASS != ret)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    /* try to grant administrative privilege via
     * specified SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD
     * default is guest privilege
     */
    *privilege = TACACS_PRIVILEGE_OF_GUEST;

#if (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_ENABLE)

    {
        I32_T   check_result;

        /*maggie liu for authorization*/
        check_result =
        tacacs_main_enable_requests(server_ip,
                                    server_secret,
                                    server_port,
                                    retransmit,
                                    timeout,
                                    (char *)username,
                                    (char *)password,
                                    TACACS_ENABLE_REQUEST,
                                    TACACS_PRIVILEGE_OF_ADMIN,
                                    sess_type,
                                    sess_id,
                                    rem_addr);

        if(check_result != TAC_PLUS_AUTHEN_STATUS_PASS)
        {
            /*maggie liu for authorization*/
            check_result =
            tacacs_main_enable_requests(server_ip,
                                        server_secret,
                                        server_port,
                                        retransmit,
                                        timeout,
                                        SYS_DFLT_ENABLE_PASSWORD_USERNAME,
                                        (char *)password,
                                        TACACS_ENABLE_REQUEST,
                                        TACACS_PRIVILEGE_OF_ADMIN,
                                        sess_type,
                                        sess_id,
                                        rem_addr);
            if(check_result == TAC_PLUS_AUTHEN_STATUS_PASS)
            {
                *privilege = TACACS_PRIVILEGE_OF_ADMIN;
            }
        }
        else
        {
            *privilege = TACACS_PRIVILEGE_OF_ADMIN;
        }
    }
#elif (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)

    {
        TACACS_AuthorRequest_T  request;
        TACACS_AuthorReply_T    reply;

        /* if name/pswd length are too long, grant admin privilege fail.
         * user can pass authentication but privilege is 0 (guest)
         */
        if ((SYS_ADPT_MAX_USER_NAME_LEN < strlen((char *)username)) ||
            (SYS_ADPT_MAX_PASSWORD_LEN < strlen((char *)password))) /*maggie liu for authorization*/
        {
            TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
        }

        /* prepare request */
        memset(&request, 0, sizeof(TACACS_AuthorRequest_T));

        request.sess_type = sess_type;
        request.ifindex = sess_id;
        if (rem_addr)
        {
            request.rem_ip_addr = *rem_addr;
        }

        memcpy(request.user_name, username, SYS_ADPT_MAX_USER_NAME_LEN);
        request.user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0';

        memcpy(request.password, password, SYS_ADPT_MAX_PASSWORD_LEN);
        request.password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';

        request.server_ip   = server_ip;
        request.server_port = server_port;
        request.retransmit  = retransmit;
        request.timeout     = timeout;

        memcpy(request.secret, server_secret, TACACS_AUTH_KEY_MAX_LEN);
        request.secret[TACACS_AUTH_KEY_MAX_LEN] = '\0';

        request.current_privilege = TACACS_PRIVILEGE_OF_GUEST;
        request.authen_by_whom = TACACS_AUTHEN_BY_TACACS_PLUS;

        /* NOTE: here we try to grant admin privilege
         * therefore:
         *      1, MUST pass TACACS+ authorization
         *      2, If TACACS+ servers return privilege,
         *         the returned privilege MUST be TACACS_PRIVILEGE_OF_ADMIN
         *      3, If TACACS+ servers do not return privilege,
         *         allow this user grant admin privilege
         *
         * otherwise user grant guest privilege
         */
        if (TRUE == tacacs_author_shell_service(&request, &reply))
        {
#if (SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL == TRUE)
            if (TACACS_AuthorRequest_SUCCEEDED == reply.return_type)
            {
                *privilege = reply.new_privilege;
            }
#else
            if ((TACACS_AuthorRequest_SUCCEEDED == reply.return_type) &&
                (TACACS_PRIVILEGE_OF_ADMIN == reply.new_privilege))
            {
                *privilege = TACACS_PRIVILEGE_OF_ADMIN;
            }
            else if (TACACS_AuthorRequest_SUCCEEDED_WITH_NO_PRIV == reply.return_type)
            {
                *privilege = TACACS_PRIVILEGE_OF_ADMIN;
            }
#endif /* SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL == TRUE */
        }

    }
#else

    /* unknown grant admin privilege method */
    ret = TAC_PLUS_AUTHEN_STATUS_FAIL;

#endif /* SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD */

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_AsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   None.
 * NOTE:     Announce result by system callback.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_AsyncLoginAuth(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size)
{
    I32_T result;
    UI32_T privilege;

    TACACS_MGR_TRACE("user=%s, sess_type=%d, sess_id=%lu", username, sess_type, sess_id);

    result = TACACS_MGR_Auth_Check(username,
                                   password,
                                   sess_type,
                                   sess_id,
                                   rem_ip_addr,
                                   &privilege);

    TACACS_MGR_TRACE("result=%ld, privilege=%lu", result, privilege);

    SYS_CALLBACK_MGR_AnnounceRemServerAuthResult(
        SYS_MODULE_TACACS,
        result,
        privilege,
        cookie,
        cookie_size);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_AsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   None.
 * NOTE:     Announce result by system callback.
 *---------------------------------------------------------------------------
 */
I32_T TACACS_MGR_Auth_Enable_Requests(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_addr,
    UI32_T *privilege)
{
    I32_T   ret = TAC_PLUS_AUTHEN_STATUS_FAIL;
    UI32_T  server_ip, server_port, retransmit, timeout;
    UI8_T   server_secret[TACACS_AUTH_KEY_MAX_LEN + 1];

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(TACACS_NOT_IN_MASTER_MODE_RC);

    server_ip = 0;

#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE)
    {
        TACACS_Server_Host_T server_host;

        /* only support 1 server now
         */
        if (TRUE == TACACS_OM_Get_Server_Host(1, &server_host))
        {
            server_ip   = server_host.server_ip;
            server_port = server_host.server_port;
            retransmit  = server_host.retransmit;
            timeout     = server_host.timeout;
            memcpy(server_secret, server_host.secret, TACACS_AUTH_KEY_MAX_LEN);
            server_secret[TACACS_AUTH_KEY_MAX_LEN] = '\0';
        }
    }
#else
    server_ip   = TACACS_OM_Get_Server_IP();
    server_port = TACACS_OM_Get_Server_Port();
    retransmit  = TACACS_OM_GetServerRetransmit();
    timeout     = TACACS_OM_GetServerTimeout();
    memcpy(server_secret, TACACS_OM_Get_Server_Secret(), TACACS_AUTH_KEY_MAX_LEN);
    server_secret[TACACS_AUTH_KEY_MAX_LEN] = '\0';

#endif /* SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER */

    if (server_ip == 0)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(TACACS_AUTHENTICATION_CONNECT_FAIL);
    }

    ret =  tacacs_main_enable_requests(server_ip,
                                       server_secret,
                                       server_port,
                                       retransmit,
                                       timeout,
                                       (char *)username,
                                       (char *)password,
                                       TACACS_ENABLE_REQUEST,
                                       TAC_PLUS_PRIV_LVL_MAX,
                                       sess_type,
                                       sess_id,
                                       rem_addr);

    if(ret != TAC_PLUS_AUTHEN_STATUS_PASS)
    {
        ret = tacacs_main_enable_requests(server_ip,
                                          server_secret,
                                          server_port,
                                          retransmit,
                                          timeout,
                                          SYS_DFLT_ENABLE_PASSWORD_USERNAME,
                                          (char *)password,
                                          TACACS_ENABLE_REQUEST,
                                          TAC_PLUS_PRIV_LVL_MAX,
                                          sess_type,
                                          sess_id,
                                          rem_addr);
        if(ret == TAC_PLUS_AUTHEN_STATUS_PASS)
        {
            *privilege = 15;
        }
    }
    else
    {
        *privilege = 15;
    }

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_AsyncAuthenEnable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   None.
 * NOTE:     Announce result by system callback.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_AsyncAuthenEnable(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size)
{
    I32_T result;
    UI32_T privilege;

    TACACS_MGR_TRACE("user=%s, sess_type=%d, sess_id=%lu", username, sess_type, sess_id);

    result = TACACS_MGR_Auth_Enable_Requests(username,
                                             password,
                                             sess_type,
                                             sess_id,
                                             rem_ip_addr,
                                             &privilege);

    TACACS_MGR_TRACE("result=%ld, privilege=%lu", result, privilege);

    SYS_CALLBACK_MGR_AnnounceRemServerAuthResult(
        SYS_MODULE_TACACS,
        result,
        privilege,
        cookie,
        cookie_size);
}

#endif /* SYS_CPNT_TACACS_PLUS_AUTHENTICATION */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TACACS server host
 * INPUT:    server_index (1-based), server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:    server_ip;
 *		    server_port (1-65535)  - set 0 will use the global TACACS configuration
 *       	timeout     (1-65535)  - set 0 will use the global TACACS configuration
 *       	retransmit  (1-65535)  - set 0 will use the global TACACS configuration
 *        	secret      (length < TACACS_MAX_SECRET_LENGTH)  - set NULL will use the global TACACS configuration
 *---------------------------------------------------------------------------
 */
BOOL_T 
TACACS_MGR_Set_Server_Host(
    UI32_T server_index,
    TACACS_Server_Host_T *server_host)
{
    BOOL_T ret;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    UI32_T  sys_time;

    TACACS_Server_Host_T    om_entry;
    TACACS_AccUserInfo_T       user_info;
    BOOL_T is_sever_change = FALSE;
    TACACS_Acc_Queue_T         msg;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */


    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);


    if (NULL == server_host)
    {

        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)

    /* before ip changed, something have to do */
    if ((TRUE == TACACS_OM_Get_Server_Host(server_index, &om_entry)) &&
    	(om_entry.server_ip != server_host->server_ip))
    {
        is_sever_change = TRUE;
        /* send users' request to next server of server group
           (auto-send start if need but start time = current system time)
         */
        user_info.user_index = 0;
        while (TRUE == TACACS_OM_GetNextAccUserEntry(&user_info))
        {
            if (user_info.tacacs_entry_info.active_server_index != server_index)
                continue;

            SYSFUN_SendEvent(user_info.task_id, TACACS_TASK_EVENT_ACC_STOP);
        }

    }
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    server_host->server_index = server_index;

    if (TRUE == TACACS_MGR_IsValidIP(server_host->server_ip))
    {
    ret =  TACACS_OM_Set_Server_Host(TRUE, server_host);
    }
    else
    {
        ret = FALSE;
    }

#if (SYS_CPNT_AAA == TRUE)
    if (TRUE == ret)
        AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(server_index);
#endif /* SYS_CPNT_AAA == TRUE */
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)

    /* before ip changed, something have to do */
    if (is_sever_change == TRUE)
    {
        /* send users' request to next server of server group
           (auto-send start if need but start time = current system time)
         */
        user_info.user_index = 0;
        while (TRUE == TACACS_OM_GetNextAccUserEntry(&user_info))
        {
            if (user_info.tacacs_entry_info.active_server_index != server_index)
                continue;

            SYS_TIME_GetRealTimeBySec(&sys_time);
            TACACS_OM_SetAccUserEntrySessionStartTime(user_info.user_index, sys_time);

            /* send msgQ and event */
            memset(&msg, 0, sizeof(TACACS_Acc_Queue_T));
            msg.user_index = user_info.user_index;
            {
                UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(TACACS_Acc_Queue_T)];
                SYSFUN_Msg_T *req_msg_p;
                req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
                req_msg_p->msg_size= sizeof(TACACS_Acc_Queue_T);
                req_msg_p->msg_type= 1;
                memcpy(req_msg_p->msg_buf,&msg,sizeof(TACACS_Acc_Queue_T));
                SYSFUN_SendRequestMsg(tacacs_acc_queue_id, req_msg_p,
                    SYSFUN_TIMEOUT_NOWAIT,  TACACS_TASK_EVENT_ACC_REQ, 0, NULL);
            }
        }

    }
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetServerHostByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : setup server host by server_ip
 * INPUT    : server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified ip doesn't exist, then create it. or modify it
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetServerHostByIpAddress(TACACS_Server_Host_T *server_host)
{
    BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == server_host)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (TRUE == TACACS_MGR_IsValidIP(server_host->server_ip))
    {
    ret =  TACACS_OM_Set_Server_Host(FALSE, server_host);
    }
    else
    {
        ret = FALSE;
    }

#if (SYS_CPNT_AAA == TRUE)
    if (TRUE == ret)
    {
        if (TRUE == TACACS_OM_LookupServerIndexByIpAddress(server_host->server_ip, &server_host->server_index))
            AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(server_host->server_index);
    }
#endif /* SYS_CPNT_AAA == TRUE */

   TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_LocalDestroyServerHost
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_index (1-based)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     caller should lock/unlock semarphore.
 *---------------------------------------------------------------------------
 */
static BOOL_T TACACS_MGR_LocalDestroyServerHost(UI32_T server_index)
{
    BOOL_T ret;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    UI32_T  sys_time;

    TACACS_Server_Host_T    server_host;
    TACACS_AccUserInfo_T       user_info;
    TACACS_Acc_Queue_T         msg;

    /* before destroy entry, something have to do */
    if (TRUE == TACACS_OM_Get_Server_Host(server_index, &server_host))
    {
        /* send users' request to next server of server group
           (auto-send start if need but start time = current system time)
         */
        user_info.user_index = 0;
        while (TRUE == TACACS_OM_GetNextAccUserEntry(&user_info))
        {
            if (user_info.tacacs_entry_info.active_server_index != server_index)
                continue;

            SYSFUN_SendEvent(user_info.task_id, TACACS_TASK_EVENT_ACC_STOP);
            SYS_TIME_GetRealTimeBySec(&sys_time);
            TACACS_OM_SetAccUserEntrySessionStartTime(user_info.user_index, sys_time);

            if (TPACC_OPROCESS_SUCCESS == TACACS_MGR_ChangeAccUserServer(&user_info))
            {
                memset(&msg, 0, sizeof(TACACS_Acc_Queue_T));
                msg.user_index = user_info.user_index;
                {
                    UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(TACACS_Acc_Queue_T)];
                    SYSFUN_Msg_T *req_msg_p;
                    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
                    req_msg_p->msg_size= sizeof(TACACS_Acc_Queue_T);
                    req_msg_p->msg_type= 1;
                    memcpy(req_msg_p->msg_buf,&msg,sizeof(TACACS_Acc_Queue_T));
                    SYSFUN_SendRequestMsg(tacacs_acc_queue_id, req_msg_p,
                        SYSFUN_TIMEOUT_NOWAIT,TACACS_TASK_EVENT_ACC_REQ, 0, NULL);
                }
            }
        }
    }
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    ret = TACACS_OM_Destroy_Server_Host(server_index);


#if (SYS_CPNT_AAA == TRUE)
    if (TRUE == ret)
        AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(server_index);
#endif /* SYS_CPNT_AAA == TRUE */
    return ret;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Destroy_Server_Host_By_Index
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_index (1-based)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Destroy_Server_Host_By_Index(UI32_T server_index)
{
    BOOL_T ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);


    ret = TACACS_MGR_LocalDestroyServerHost(server_index);

     TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Destroy_Server_Host_By_Ip_Address
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_ip
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Destroy_Server_Host_By_Ip_Address(UI32_T server_ip)
{
    BOOL_T ret;
    UI32_T server_index;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (FALSE == TACACS_OM_LookupServerIndexByIpAddress(server_ip, &server_index))
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = TACACS_MGR_LocalDestroyServerHost(server_index);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetRunningServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default TACACS accounting port is successfully retrieved.
 *            Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default value.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T TACACS_MGR_GetRunningServerAcctPort(UI32_T *acct_port)
{
    UI32_T  port;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (NULL == acct_port)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    port = TACACS_OM_GetServerAcctPort();
    if (SYS_DFLT_TACACS_ACC_CLIENT_SERVER_PORT_NUMBER == port)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
    }

    *acct_port = port;

    TACACS_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetServerAcctPort(UI32_T *acct_port)
{
    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == acct_port)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *acct_port = TACACS_OM_GetServerAcctPort();

    TACACS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetServerAcctPort(UI32_T acct_port)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TACACS_OM_SetServerAcctPort(acct_port);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetAccUserEntryQty(UI32_T *qty)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TACACS_OM_GetAccUserEntryQty(qty);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetNextAccUserEntry(TPACC_UserInfoInterface_T *entry)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TACACS_OM_GetNextAccUserEntryInterface(entry);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetNextAccUserEntryFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name by index.
 * INPUT    : entry->user_index, entry->user_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetNextAccUserEntryFilterByName(TPACC_UserInfoInterface_T *entry)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TACACS_OM_GetNextAccUserEntryFilterByName(entry);

    TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* LOCAL SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_ChangeAccUserServer
 *-------------------------------------------------------------------------
 * PURPOSE  : assign another server to user
 * INPUT    : ip_address
 * OUTPUT   : none
 * RETURN   : TACACS_AccObjectProcessResult_T
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
TACACS_AccObjectProcessResult_T TACACS_MGR_ChangeAccUserServer(TACACS_AccUserInfo_T *user_info)
{
    AAA_QueryGroupIndexResult_T     query_result;
    AAA_TacacsPlusEntryInterface_T      tacacs_entry;

    if ((NULL == user_info) || (TPACC_ENTRY_DESTROYED == user_info->entry_status))
    {
        return TPACC_OPROCESS_DELETED;
    }

    /* find server host */
    if (FALSE == AAA_MGR_GetAccountingGroupIndex(user_info->client_type, user_info->ifindex, &query_result))
    {
        if (0 == user_info->tacacs_entry_info.active_server_index)
        {
            /* if setting is not ready, don't delete this user */
            return TPACC_OPROCESS_FAILURE;
        }

        /* currently, users stop accounting on this port
           and all requests are "retry-timeout"
         */
        TACACS_OM_FreeAccUser(user_info->user_index); /* should not use user_info thereafter */
        return TPACC_OPROCESS_DELETED;
    }

    switch (query_result.group_type)
    {
        case GROUP_TACACS_PLUS:
            break;

        case GROUP_RADIUS:
        case GROUP_UNKNOWN:
        default:
            /* currently, users change accounting server from RADIUS to others
               and all requests are "retry-timeout"
             */
            TACACS_OM_FreeAccUser(user_info->user_index); /* should not use user_info thereafter */
            return TPACC_OPROCESS_DELETED;
    }

    if (0 == user_info->tacacs_entry_info.aaa_tacacs_index)
    {
        /* user_info 's first request */
        tacacs_entry.tacacs_index = 0;
    }
    else
    {
        tacacs_entry.tacacs_index = user_info->tacacs_entry_info.aaa_tacacs_index;
    }

    while (TRUE == AAA_OM_GetNextTacacsPlusEntry(query_result.group_index, &tacacs_entry))
    {
        if (FALSE == TACACS_OM_IsServerHostValid(tacacs_entry.tacacs_server_index))
        {
            continue;
        }

        /* update input param */
        user_info->tacacs_entry_info.aaa_group_index = query_result.group_index;
        user_info->tacacs_entry_info.aaa_tacacs_index = tacacs_entry.tacacs_index;
        user_info->tacacs_entry_info.active_server_index = tacacs_entry.tacacs_server_index;
        AAA_OM_GetTacacsPlusEntryOrder(query_result.group_index, tacacs_entry.tacacs_index, &user_info->tacacs_entry_info.aaa_tacacs_order);

        /* update om */
        TACACS_OM_SetAccUserEntryAAATacacsInfo(user_info->user_index, &user_info->tacacs_entry_info);

        return TPACC_OPROCESS_SUCCESS;
    }
    return TPACC_OPROCESS_FAILURE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_AsyncAccountingRequest_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : identifier, ifindex, client_type, request_type,
 *            user_name     --  User name (terminated with '\0')
 *            call_back_func      --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static BOOL_T TACACS_MGR_AsyncAccountingRequest_Callback(const AAA_AccRequest_T *request)
{
    UI32_T                  sys_time;
    TACACS_AccUserInfo_T    user_info;
    TACACS_Acc_Queue_T      msg;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == request)
    {
       TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    switch (request->request_type)
    {
        case AAA_ACC_START:
            if (TRUE == TACACS_OM_GetAccUserEntryByKey(request->ifindex, request->user_name, request->client_type, &user_info))
            {
                /* send stop event */
                TACACS_OM_FreeAccUser(user_info.user_index);
            }

            SYS_TIME_GetRealTimeBySec(&sys_time);

            user_info.user_index = TACACS_OM_CreateAccUserEntry(request, sys_time);
            if (FALSE == TACACS_OM_GetAccUserEntry(&user_info))
            {
                TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            /* send msgQ and event */
            memset(&msg, 0, sizeof(TACACS_Acc_Queue_T));
            msg.user_index = user_info.user_index;
            {
                UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(TACACS_Acc_Queue_T)];
                SYSFUN_Msg_T *req_msg_p;
                req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
                req_msg_p->msg_size= sizeof(TACACS_Acc_Queue_T);
                req_msg_p->msg_type= 1;
                memcpy(req_msg_p->msg_buf,&msg,sizeof(TACACS_Acc_Queue_T));
                SYSFUN_SendRequestMsg(tacacs_acc_queue_id, req_msg_p,
                    SYSFUN_TIMEOUT_NOWAIT,TACACS_TASK_EVENT_ACC_REQ, 0, NULL);
            }
            break;

        case AAA_ACC_STOP:
            if (FALSE == TACACS_OM_GetAccUserEntryByKey(request->ifindex, request->user_name, request->client_type, &user_info))
            {
                TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
            {
                UI32_T tid;

                /* send msgQ and event */
                tid = user_info.task_id;
                TACACS_OM_FreeAccUser(user_info.user_index);

                SYSFUN_SendEvent(tid, TACACS_TASK_EVENT_ACC_STOP);
            }
            break;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
        case AAA_ACC_CMD_START:
        {
            /* AAA_ACC_CMD_START and AAA_ACC_CMD_STOP
             * should be paired and come here in order per ifindex
             */
            TACACS_OM_SetAccCmdInfo(
                request->ifindex,
                request->user_name,
                request->command,
                request->command_privilege,
                request->serial_number);
        }

        case AAA_ACC_CMD_STOP:
        {
            TPACC_AccCommandmdMessage_T *acc_cmd_msg_p;
            AAA_AccInterface_T          acc_interface;
            AAA_QueryGroupIndexResult_T query_result;
            UI32_T                      tmp_ser_no =0xffffffff;

            if (request->request_type == AAA_ACC_CMD_STOP)
            {
                /* AAA_ACC_CMD_START and AAA_ACC_CMD_STOP
                 * should be paired and come here in order per ifindex
                 */
                TACACS_OM_GetAccCmdInfo(
                    request->ifindex,
                    request->user_name,
                    request->command,
                    request->command_privilege,
                    &tmp_ser_no);
            }
            else
            {
                tmp_ser_no = request->serial_number;
            }

            /* get the group index by request
             */
            memset(&acc_interface, 0, sizeof(acc_interface));
            acc_interface.client_type = AAA_CLIENT_TYPE_COMMANDS;
            acc_interface.ifindex     = request->ifindex;
            acc_interface.priv_lvl    = request->command_privilege;

            if (FALSE == AAA_MGR_GetAccountingGroupIndex_ByInterface(&acc_interface, &query_result))
            {
                TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            if (GROUP_TACACS_PLUS != query_result.group_type)
            {
                TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            /* alloc memory for command message buffer
             */
            acc_cmd_msg_p = (TPACC_AccCommandmdMessage_T *) L_MM_Malloc(
                                sizeof(*acc_cmd_msg_p),
                                L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ASYNCACCOUNTINGREQUEST_CB));
            if (NULL == acc_cmd_msg_p)
            {
                TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            memset(acc_cmd_msg_p, 0, sizeof(*acc_cmd_msg_p));

            acc_cmd_msg_p->tacacs_entry_info.aaa_group_index = query_result.group_index;

            acc_cmd_msg_p->serial_number = tmp_ser_no;
            acc_cmd_msg_p->acct_flag = (request->request_type == AAA_ACC_CMD_START) ?
                                       TAC_PLUS_ACCT_FLAG_START : TAC_PLUS_ACCT_FLAG_STOP;

            acc_cmd_msg_p->ifindex = request->ifindex;
            acc_cmd_msg_p->rem_ip_addr = request->rem_ip_addr;

            strncpy(acc_cmd_msg_p->user_name, request->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
            acc_cmd_msg_p->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = 0;

            acc_cmd_msg_p->user_auth_privilege = request->auth_privilege;
            acc_cmd_msg_p->user_auth_by_whom   = request->auth_by_whom;

            strncpy(acc_cmd_msg_p->command, request->command, SYS_ADPT_CLI_MAX_BUFSIZE);
            acc_cmd_msg_p->command[SYS_ADPT_CLI_MAX_BUFSIZE] = 0;
            acc_cmd_msg_p->command_privilege   = request->command_privilege;

            SYS_TIME_GetRealTimeBySec(&acc_cmd_msg_p->accounting_start_time);

            /* send msgQ and event
             */
            memset(&msg, 0, sizeof(msg));
            msg.user_index = user_info.user_index;
            msg.msg_data_p = (UI8_T*)acc_cmd_msg_p;

            {
                UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(TACACS_Acc_Queue_T)];
                SYSFUN_Msg_T *req_msg_p;
                req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
                req_msg_p->msg_size= sizeof(TACACS_Acc_Queue_T);
                req_msg_p->msg_type= 1;
                memcpy(req_msg_p->msg_buf,&msg,sizeof(TACACS_Acc_Queue_T));
                SYSFUN_SendRequestMsg(tacacs_acc_queue_id, req_msg_p,
                    SYSFUN_TIMEOUT_NOWAIT, TACACS_TASK_EVENT_ACC_REQ, 0, NULL);
            }
            break;
        }
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

        default:
            TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }


    TACACS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetMainTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : get tacacs main task id
 * INPUT    : task_id     --  buffer of task id
 * OUTPUT   : task_id     --  task id
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetMainTaskId(UI32_T *task_id)
{
    if(task_id == NULL)
    {
        return FALSE;
    }
    *task_id = tacacs_main_task_id;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetMainTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : set tacacs main task id
 * INPUT    : task_id     --  task id
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetMainTaskId(UI32_T task_id)
{
    if(task_id == 0)
    {
        return FALSE;
    }
    tacacs_main_task_id = task_id;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetAccMsgQId
 *-------------------------------------------------------------------------
 * PURPOSE  : get tacacs acc message queue id
 * INPUT    : queue_id     --  buffer of queue id
 * OUTPUT   : queue_id     --  queue id
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetAccMsgQId(UI32_T *queue_id)
{
    if(queue_id == NULL)
    {
        return FALSE;
    }
    *queue_id = tacacs_acc_queue_id;
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetAccUserEntryTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : set task id of acc user
 * INPUT    : user_index  --  user index
 *            task_id     --  task id
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetAccUserEntryTaskId(UI16_T user_index, UI32_T task_id)
{
    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if (FALSE == TACACS_OM_SetAccUserEntryTaskId(user_index,task_id))
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    TACACS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetAccUserEntryLastUpdateTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time)
{
    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if(update_time == 0)
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == TACACS_OM_SetAccUserEntryLastUpdateTime(user_index,update_time))
    {
        TACACS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    TACACS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) /*maggie liu for authorization*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_Author_Check
 *-------------------------------------------------------------------------
 * PURPOSE  : do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : *reply -- output of authorization request
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_Author_Check(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
  //printf("\r\nTACACS_MGR_Author_Check start!");
    ret = tacacs_author_shell_service(request, reply);
//printf("\r\nTACACS_MGR_Author_Check end with return value %ld,type %ld,priv %ld!",ret,reply->return_type,reply->new_privilege);
      TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);

}
#else
#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_Author_Check
 *-------------------------------------------------------------------------
 * PURPOSE  : do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : *reply -- output of authorization request
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_Author_Check(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply)
{
    BOOL_T  ret;

    TACACS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
  //printf("\r\nTACACS_MGR_Author_Check start!");
    ret = tacacs_author_main(request, reply);
//printf("\r\nTACACS_MGR_Author_Check end with return value %ld,type %ld,priv %ld!",ret,reply->return_type,reply->new_privilege);
      TACACS_MGR_RETURN_AND_RELEASE_CSC(ret);

}
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
#endif

/* FUNCTION NAME - TACACS_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void TACACS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}

/* FUNCTION NAME - TACACS_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void TACACS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}

 /*------------------------------------------------------------------------------
  * ROUTINE NAME : TACACS_MGR_HandleIPCReqMsg
  *------------------------------------------------------------------------------
  * PURPOSE:
  *    Handle the ipc request message for cluster mgr.
  * INPUT:
  *    msg_p         --  the request ipc message buffer
  *    ipcmsgq_p     --  The handle of ipc message queue. The response message
  *                      will be sent through this handle.
  *
  * OUTPUT:
  *    None.
  *
  * RETURN:
  *    TRUE  --  There is a response need to send.
  *    FALSE --  No response need to send.
  * NOTES:
  *    None.
  *------------------------------------------------------------------------------
  */
BOOL_T TACACS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msg_p)
 {
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI32_T cmd;
    if(msg_p==NULL)
    {
        return FALSE;
    }

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    cmd = msg_data_p->type.cmd;
    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        /*EPR:NULL
         *Problem:When slave enter transition mode,if msg_size have not
         *        any value,will cause sender receive reply overflow.
         *Solution: use a default msg_size to reply the sender.
         *Fixed by:DanXie
         *Modify file:tacacs_mgr.c
         *Approved by:Hardsun
         */
        msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
        msg_data_p->type.result_ui32 = FALSE;
        goto exit;
    }

    switch(cmd)
    {

        case TACACS_MGR_IPCCMD_SET_SERVER_PORT:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Set_Server_Port(
                msg_data_p->data.ui32_v);
        }
        break;

        case TACACS_MGR_IPCCMD_SET_SERVER_RETRANSMIT:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_SetServerRetransmit(
                msg_data_p->data.ui32_v);
        }
        break;

        case TACACS_MGR_IPCCMD_SET_SERVER_TIMEOUT:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_SetServerTimeout(
                msg_data_p->data.ui32_v);
        }
        break;

        case TACACS_MGR_IPCCMD_SET_SERVER_SECRET:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Set_Server_Secret(
                msg_data_p->data.serversecret);
        }
        break;

        case TACACS_MGR_IPCCMD_SET_SERVER_IP:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Set_Server_IP(
                msg_data_p->data.ui32_v);
        }
        break;

#if (SYS_CPNT_TACACS_PLUS_AUTHENTICATION == TRUE)
        case TACACS_MGR_IPCCMD_ASYNC_LOGIN_AUTH:
            TACACS_MGR_AsyncLoginAuth(
                msg_data_p->data.async_author_request.user_name,
                msg_data_p->data.async_author_request.password,
                msg_data_p->data.async_author_request.sess_type,
                msg_data_p->data.async_author_request.sess_id,
                &msg_data_p->data.async_author_request.rem_ip_addr,
                msg_data_p->data.async_author_request.cookie,
                msg_data_p->data.async_author_request.cookie_size);
            break;

        case TACACS_MGR_IPCCMD_ASYNC_AUTHEN_ENABLE:
            TACACS_MGR_AsyncAuthenEnable(
                msg_data_p->data.async_author_request.user_name,
                msg_data_p->data.async_author_request.password,
                msg_data_p->data.async_author_request.sess_type,
                msg_data_p->data.async_author_request.sess_id,
                &msg_data_p->data.async_author_request.rem_ip_addr,
                msg_data_p->data.async_author_request.cookie,
                msg_data_p->data.async_author_request.cookie_size);
            break;
#endif /* #if (SYS_CPNT_TACACS_PLUS_AUTHENTICATION == TRUE) */

        case TACACS_MGR_IPCCMD_SET_SERVER_HOST:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Set_Server_Host(
                msg_data_p->data.index_serverhost.index,
                &msg_data_p->data.index_serverhost.server_host);
        }
        break;

        case TACACS_MGR_IPCCMD_SETSERVERHOSTBYIPADDRESS:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_SetServerHostByIpAddress(
                &msg_data_p->data.server_host);
        }
        break;

        case TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_INDEX:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Destroy_Server_Host_By_Index(
                msg_data_p->data.ui32_v);
        }
        break;

        case TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_IP_ADDRESS:
        {
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=TACACS_MGR_Destroy_Server_Host_By_Ip_Address(
                msg_data_p->data.ui32_v);
        }
        break;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
        case TACACS_MGR_IPCCMD_AUTHOR_CHECK:
        {
            /* the size of the respond size is equal the size of request message.
             */
            msg_data_p->type.result_bool=TACACS_MGR_Author_Check(
                &msg_data_p->data.request_reply.request,
                &msg_data_p->data.request_reply.reply);
        }
        break;
#endif /*#if (SYS_CPNT_AUTHORIZATION == TRUE)*/

        default:
            msg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd<TACACS_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

 }

