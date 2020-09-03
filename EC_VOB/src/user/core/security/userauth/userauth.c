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
 *   Jason Hsue     11/11/2001      new created
 *   Jason Hsue     1/15/2002       Modified
 *      1. Remove SNMP Trap Receiver(to TrapMgr)
 *      2. Rename Login user to Login Local
 *      3. Add Login password relative functions
 *   JJ Young       5/24/2003       Design Changed
 *      1. Auth_Method data structure changed
 *      2. Add TACACS authentication
 *      3. Add an API to provide login authentication service for different
 *         login user types
 *   balage         09/16/2002      transition mode MACRO create
 *   Jeff Kao       10/21/2002      add error code
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "string.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "userauth.h"
#include "l_md5.h"
#include "swctrl_pom.h"   /* Add by allen_lee, 2002.03.25*/
#include "swdrv.h"
#include "radius_mgr.h"
#include "radius_pmgr.h"
#if (SYS_CPNT_TACACS==TRUE)
#include "tacacs_pmgr.h"
#endif
#if (SYS_CPNT_CFGDB==TRUE)
#include "cfgdb_mgr.h"
#endif
/* #include "uimsg_mgr.h */
#include "eh_type.h"
#include "eh_mgr.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "backdoor_mgr.h"
#include "sys_bld.h"
#if (SYS_CPNT_AUTHORIZATION == TRUE)
#include "aaa_pmgr.h"
#endif
#include "sys_dflt.h"
#include <stdlib.h>

#include "snmp_pmgr.h"
#include "trap_event.h"
#include "netcfg_pmgr_route.h"

#define  USERAUTH_ENTER_CRITICAL_SECTION()
#define  USERAUTH_LEAVE_CRITICAL_SECTION()

#define USERAUTH_USE_CSC_WITHOUT_RETURN_VALUE()
#define USERAUTH_USE_CSC(a)
#define USERAUTH_RELEASE_CSC()

#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
#define CFGDB_MGR_SECTION_ID_USERAUTH_SNMP_COMMUNITY    CFGDB_MGR_SECTION_ID_USERAUTH_1
#endif
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
#define CFGDB_MGR_SECTION_ID_USERAUTH_LOGIN             CFGDB_MGR_SECTION_ID_USERAUTH_2
#endif
#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
#define CFGDB_MGR_SECTION_ID_USERAUTH_PASSWORD_RECOVERY CFGDB_MGR_SECTION_ID_USERAUTH_3
#endif

#ifndef NEWLINE
#define NEWLINE                                             "\r\n"
#endif

#define USERAUTH_IS_DEBUG_ERROR_ON(flag)                    (flag)

#define USERAUTH_PRINT_HEADER()                             \
    {                                                       \
        BACKDOOR_MGR_Printf("[%s:%d]" NEWLINE,              \
            __FUNCTION__, __LINE__);                        \
    }

#define USERAUTH_PRINT(fmt, ...)                            \
    {                                                       \
        BACKDOOR_MGR_Printf(fmt, ##__VA_ARGS__);            \
    }

#define USERAUTH_LOG(fmt,...)                               \
    {                                                       \
        if (USERAUTH_IS_DEBUG_ERROR_ON(userauth_dbg_flag))  \
        {                                                   \
            USERAUTH_PRINT_HEADER();                        \
            USERAUTH_PRINT(fmt NEWLINE, ##__VA_ARGS__);     \
        }                                                   \
    }

#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif /* _countof */

/* Function wise static variable declaration
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_Dispatch
 *-------------------------------------------------------------------------
 * PURPOSE  : Dispatch function used for processing asynchronous request
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PASS       - authentication accepted
 *            USERAUTH_REJECT     - authentication failed
 *            USERAUTH_PROCESSING - authencication is processing by remote
 *                                  server.
 *            Otherwise           - error code.
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_Dispatch(
    USERAUTH_AsyncAuthParam_T *param_p
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - USERAUTH_SendAuthenticationTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send authentication trap
 * INPUT   : param_p  -- user auth parameter
 *           ret      -- authentication retrun value
 *                       USERAUTH_PASS:
 *                       authentication accepted
 *
 *                       USERAUTH_REJECT:
 *                       authentication failed
 *
 *                       USERAUTH_PROCESSING:
 *                       authencication is processing by remote server
 *
 *                       Otherwise:
 *                       error code
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
USERAUTH_SendAuthenticationTrap(
    USERAUTH_AsyncAuthParam_T *param_p,
    USERAUTH_ReturnValue_T ret
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - USERAUTH_SendUserAccountTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send user account trap
 * INPUT   : user - Information of account user.
 *           trap_type - The type of trap you want to send.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
USERAUTH_SendUserAccountTrap(
    USERAUTH_LoginLocal_T *user,
    UI32_T trap_type
);

static USERAUTH_ReturnValue_T USERAUTH_Local(
    const char *username,
    const char *password,
    UI32_T *privilege
);

#if (SYS_CPNT_RADIUS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncRadiusLoginAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous login authentication request to RADIUS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncRadiusLoginAuth(
    USERAUTH_AsyncAuthParam_T *param_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncRadiusEnableAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous enable authentication request to RADIUS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncRadiusEnableAuth(
    USERAUTH_AsyncAuthParam_T *param_p
);
#endif /* #if (SYS_CPNT_RADIUS == TRUE) */



#if (SYS_CPNT_TACACS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncTacacsLoginAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous login authentication request to TACACS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncTacacsLoginAuth(
    USERAUTH_AsyncAuthParam_T *param_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncTacacsEnableAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous enable authentication request to TACACS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncTacacsEnableAuth(
    USERAUTH_AsyncAuthParam_T *param_p
);
#endif  /* #if (SYS_CPNT_TACACS == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_TotalNumberOfAdminUser
 * ---------------------------------------------------------------------
 * PURPOSE : This function will Calculate the number of administrator privilege
 *           level account.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : count - number of administrator privilege level account.
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static UI32_T
USERAUTH_TotalNumberOfAdminUser(
);

static BOOL_T USERAUTH_LocalAddDefaultUser(void);

static BOOL_T
USERAUTH_LocaIsDefaultUserUnChangeed(
USERAUTH_LoginLocal_T *checked_user_p
);

static SYS_TYPE_Get_Running_Cfg_T USERAUTH_GetRunningAllLoginLocalUser(
USERAUTH_LoginLocal_T login_user_ar[],
UI32_T *get_number_p);

static USERAUTH_SnmpCommunity_T         userauth_snmp_community[SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING];

/* Add extra super user in case customer deletes all or forgets all, and it's not deleted.
 * Keep a space for default "admin" user. The "admin" user can be deleted.
 * However, user can not create account in this spece.
 * userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER - 1] = default admin user
 * userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER] = super user
 */
static USERAUTH_LoginLocal_T            userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER + 1];
static USERAUTH_Privilege_Password_T    userauth_privilege_password[SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE];

/* design changed for TACACS by JJ, 2002-05-24 */
static USERAUTH_Auth_Method_T           userauth_auth_method[USERAUTH_NUMBER_OF_SESSION_TYPE][USERAUTH_NUMBER_Of_AUTH_METHOD];
static USERAUTH_Auth_Method_T           userauth_enable_auth_method[USERAUTH_NUMBER_Of_AUTH_METHOD];

#if (SYS_CPNT_AUTHENTICATION == TRUE)
static USERAUTH_Auth_Method_T           userauth_loginauth_pass_method;
#endif /* SYS_CPNT_AUTHENTICATION == TRUE */

static USERAUTH_Login_Method_T          userauth_console_login_method;
static USERAUTH_Login_Method_T          userauth_telnet_login_method;

static UI8_T    userauth_console_login_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1];
static UI8_T    userauth_telnet_login_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1];

static BOOL_T   userauth_dbg_flag = FALSE;

/*isiah*/
#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
static USERAUTH_PasswordRecovery_T  userauth_password_recovery;
static UI32_T   userauth_password_recovery_session_handler;
#endif
#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
static UI32_T   userauth_snmp_community_session_handler;
#endif
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
static UI32_T   userauth_login_local_session_handler;
#endif

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 *
 * INPUT: None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T USERAUTH_Init(void)
{
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_Create_InterCSC_Relation
 * ---------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *
 * INPUT:  None
 * OUTPUT: None
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void USERAUTH_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("userauth",
        SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,USERAUTH_BackdoorFunction);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_GetPasswdFromMac
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get superuser password form MAC address
 *
 * INPUT  : The address of output password to be returned
 * OUTPUT : Output password (8 characters)
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void USERAUTH_GetPasswdFromMac(UI8_T *output_passwd)
{
    int i;
    UI8_T seed[] =
        {
            0x70, 0x12, 0x9c, 0xa7,
            0x28, 0x83, 0x80, 0xea,
            0x3a, 0xde, 0x86, 0x58,
            0xcd, 0x20, 0x30, 0x78,
            0x7a, 0x10, 0x86, 0x41,
        };
    UI8_T buffer[SYS_ADPT_MAC_ADDR_LEN + sizeof(seed)];
    UI8_T key[16];
    char  b2c[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#if (SYS_ADPT_MAX_PASSWORD_LEN < 8)
    SYS_ADPT_MAX_PASSWORD_LEN MUST >= 8
#endif

    SWCTRL_POM_GetCpuMac(buffer);
    memcpy(&buffer[SYS_ADPT_MAC_ADDR_LEN], seed, sizeof(seed));

    L_MD5_MDString(key, buffer, sizeof(buffer));

    for (i=0; i<8; ++i)
    {
        int j = (key[2*i] + key[2*i +1]) % (sizeof(b2c)-1);
        output_passwd[i] = b2c [j];
    }

    output_passwd[i] = '\0';
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to SYS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        SYS_MGR_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
static void USERAUTH_SetConfigSettingToDefault()
{

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
    BOOL_T  is_userauth_password_recovery_need_to_sync;
#endif
#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
    BOOL_T  is_userauth_snmp_community_need_to_sync;
#endif
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
    BOOL_T  is_userauth_login_local_need_to_sync;
#endif

    memset(userauth_login_local, 0, sizeof(userauth_login_local));
    memset(userauth_privilege_password, 0, sizeof(userauth_privilege_password));
    memset(userauth_auth_method, 0, sizeof(userauth_auth_method));
    memset(userauth_enable_auth_method, 0, sizeof(userauth_enable_auth_method));
    /* Init SNMP Community String to default
     */
    /*Initiate_Snmp_Community_String:*/
    {
/*isiah.2003-05-26. for config DB*/
#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
        if( CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_USERAUTH_SNMP_COMMUNITY,
                       sizeof(USERAUTH_SnmpCommunity_T),
                       SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING,
                       &userauth_snmp_community_session_handler,
                       CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                       &is_userauth_snmp_community_need_to_sync) == TRUE )
        {
            if( TRUE == is_userauth_snmp_community_need_to_sync )
            {
                memset(userauth_snmp_community, 0, sizeof(USERAUTH_SnmpCommunity_T)*SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING);
                strncpy((char *)userauth_snmp_community[0].comm_string_name, "public",SYS_ADPT_MAX_COMM_STR_NAME_LEN);
                userauth_snmp_community[0].access_right = USERAUTH_ACCESS_RIGHT_READ_ONLY;
                userauth_snmp_community[0].status = USERAUTH_ENTRY_VALID;

        #if (SYS_CPNT_USERAUTH_MANAGER_STYLE == SYS_CPNT_USERAUTH_MANAGER_STYLE_3COM)

                strncpy((char *)userauth_snmp_community[1].comm_string_name, "manager",SYS_ADPT_MAX_COMM_STR_NAME_LEN);
                userauth_snmp_community[1].access_right = USERAUTH_ACCESS_RIGHT_READ_WRITE;
                userauth_snmp_community[1].status = USERAUTH_ENTRY_VALID;

        #endif

                strncpy((char *)userauth_snmp_community[2].comm_string_name, "private",SYS_ADPT_MAX_COMM_STR_NAME_LEN);
                userauth_snmp_community[2].access_right = USERAUTH_ACCESS_RIGHT_READ_WRITE;
                userauth_snmp_community[2].status = USERAUTH_ENTRY_VALID;
                CFGDB_MGR_SyncSection(userauth_snmp_community_session_handler, (void *)&userauth_snmp_community);
            }
            else
            {
                CFGDB_MGR_ReadSection(userauth_snmp_community_session_handler, &userauth_snmp_community);
            }
        }
#endif
    } /* End of Init SNMP Community String to default  */


    /* Init user password entries to default
     */
    /*Initiate_User_Password:*/
    {
        UI8_T   digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1];
        /* Add by allen_lee, 2002.03.25 */
        UI8_T   USERAUTH_SUPERUSER_PASSWORD_DEF[SYS_ADPT_MAX_PASSWORD_LEN+1] = {0};

        USERAUTH_GetPasswdFromMac(USERAUTH_SUPERUSER_PASSWORD_DEF);   /* Add by allen_lee, 2002.03.25*/

        /* 2004/2/18, mfhorng,
           the MD5 must be same as USERAUTH_Local(), so mark following code
        L_MD5_MDString(digest, USERAUTH_SUPERUSER_PASSWORD_DEF, SYS_ADPT_MAX_PASSWORD_LEN);
        */
        L_MD5_MDString(digest, USERAUTH_SUPERUSER_PASSWORD_DEF, strlen((char *)USERAUTH_SUPERUSER_PASSWORD_DEF));

        digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;
        memset(userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
        strncpy((char *)userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].username, USERAUTH_SUPERUSER_USERNAME_DEF,SYS_ADPT_MAX_USER_NAME_LEN);
        memset(userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);  /*modify by allen_lee, 2002.03.25*/
        memcpy(userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].password, digest, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);  /*modify by allen_lee, 2002.03.25*/
        userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].privilege = USERAUTH_SUPERUSER_USER_PRIVILEGE;
        userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER].status = USERAUTH_ENTRY_VALID;

/*isiah.2003-05-26. for config DB*/
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
        if( CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_USERAUTH_LOGIN,
                       sizeof(USERAUTH_LoginLocal_T),
                       SYS_ADPT_MAX_NBR_OF_LOGIN_USER + 1,
                       &userauth_login_local_session_handler,
                       CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                       &is_userauth_login_local_need_to_sync) == TRUE )
        {
            if( TRUE == is_userauth_login_local_need_to_sync )
            {
                L_MD5_MDString(digest, SYS_DFLT_USERAUTH_ADMIN_PASSWORD, strlen(SYS_DFLT_USERAUTH_ADMIN_PASSWORD));
                digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;
                memset(userauth_login_local[0].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy((char *)userauth_login_local[0].username, SYS_DFLT_USERAUTH_ADMIN_USERNAME, SYS_ADPT_MAX_USER_NAME_LEN);
                memset(userauth_login_local[0].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
                memcpy(userauth_login_local[0].password, digest, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
                userauth_login_local[0].privilege = SYS_DFLT_USERAUTH_ADMIN_PRIVILEGE;
                userauth_login_local[0].status = USERAUTH_ENTRY_VALID;

    #if (SYS_CPNT_USERAUTH_MANAGER_STYLE == SYS_CPNT_USERAUTH_MANAGER_STYLE_3COM)

                L_MD5_MDString(digest, SYS_DFLT_USERAUTH_MANAGER_PASSWORD, strlen((char *)SYS_DFLT_USERAUTH_MANAGER_PASSWORD));
                digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;
                memset(userauth_login_local[1].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy((char *)userauth_login_local[1].username, SYS_DFLT_USERAUTH_MANAGER_USERNAME, SYS_ADPT_MAX_USER_NAME_LEN);
                memset(userauth_login_local[1].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
                memcpy(userauth_login_local[1].password, digest, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
                userauth_login_local[1].privilege = SYS_DFLT_USERAUTH_MANAGER_PRIVILEGE;
                userauth_login_local[1].status = USERAUTH_ENTRY_VALID;
    #endif

                L_MD5_MDString(digest, SYS_DFLT_USERAUTH_GUEST_PASSWORD, strlen((char *)SYS_DFLT_USERAUTH_GUEST_PASSWORD));
                digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;
                memset(userauth_login_local[2].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy((char *)userauth_login_local[2].username, SYS_DFLT_USERAUTH_GUEST_USERNAME, SYS_ADPT_MAX_USER_NAME_LEN);
                memset(userauth_login_local[2].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
                memcpy(userauth_login_local[2].password, digest, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
                userauth_login_local[2].privilege = SYS_DFLT_USERAUTH_GUEST_PRIVILEGE;
                userauth_login_local[2].status = USERAUTH_ENTRY_VALID;

                CFGDB_MGR_SyncSection(userauth_login_local_session_handler, (void *)&userauth_login_local);
            }
            else
            {
                CFGDB_MGR_ReadSection(userauth_login_local_session_handler, &userauth_login_local);
            }
        }
#endif
    } /* End of Init user password entries to default */


    /* Init privilege password entries to default
     */
    /*Initiate_Privilege_Password:*/
    {
    }


    /*Initiate_Auth_and_Login_Method:*/
    {
        UI32_T i;

        /* design changed for TACACS by JJ, 2002-05-24 */
        for(i = 0; i < USERAUTH_NUMBER_OF_SESSION_TYPE; i ++)
        {
            userauth_auth_method[i][0] = USERAUTH_AUTH_LOCAL;
            userauth_auth_method[i][1] = USERAUTH_AUTH_NONE;
            userauth_auth_method[i][2] = USERAUTH_AUTH_NONE;
        }

        /* Add by Aaron Chuang, 2003-01-27 */
        userauth_enable_auth_method[0] = USERAUTH_AUTH_LOCAL;
        userauth_enable_auth_method[1] = USERAUTH_AUTH_NONE;
        userauth_enable_auth_method[2] = USERAUTH_AUTH_NONE;

        userauth_telnet_login_method  = USERAUTH_LOGIN_METHOD_DEF;
        userauth_console_login_method = USERAUTH_LOGIN_METHOD_DEF;
    }

#if (SYS_CPNT_AUTHENTICATION == TRUE)
    userauth_loginauth_pass_method = USERAUTH_AUTH_NONE;
#endif /* SYS_CPNT_AUTHENTICATION == TRUE */
#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
    /*Initiate_Password_Recovery:*/
    {
        if( CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_USERAUTH_PASSWORD_RECOVERY,
                       sizeof(USERAUTH_PasswordRecovery_T),
                       1,
                       &userauth_password_recovery_session_handler,
                       CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                       &is_userauth_password_recovery_need_to_sync) == TRUE )
        {
            if( TRUE == is_userauth_password_recovery_need_to_sync )
            {
                userauth_password_recovery.active = TRUE;
                userauth_password_recovery.status = USERAUTH_PASSWORD_RECOVERY_ENABLED;
                CFGDB_MGR_SyncSection(userauth_password_recovery_session_handler, (void *)&userauth_password_recovery);
            }
            else
            {
                CFGDB_MGR_ReadSection(userauth_password_recovery_session_handler, &userauth_password_recovery);
            }
        }
    }
#endif

    if(FALSE == USERAUTH_LocalAddDefaultUser())
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                 USERAUTH_GETNEXTLOGINLOCALUSER_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_INFO ,
                                 "Add default user account"/*arg_p*/);
    }
}

/*------------------------------------------------------------------------
* ROUTINE NAME - USERAUTH_LocalAddDefaultUser()
*------------------------------------------------------------------------
* FUNCTION: Initial Default account to userauth database
* INPUT   : NONE
* OUTPUT  : NONE
* RETURN  : NONE
* NOTE    : None
*------------------------------------------------------------------------
*/
static BOOL_T USERAUTH_LocalAddDefaultUser()
{
    UI32_T dlft_user_index;
    UI8_T  digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1];
    USERAUTH_LoginLocal_T userauth_dflt_user_ar[] = { USERAUTH_DFLTUSER_ACCOUNT(USERAUTH_ACCOUNTINFO) };

    for (dlft_user_index = 0; dlft_user_index < USERAUTH_MAX_DFLTACCOUNT_NUM; dlft_user_index++)
    {
        if(FALSE == USERAUTH_SetLoginLocalUserStatus(userauth_dflt_user_ar[dlft_user_index].username,
                                             userauth_dflt_user_ar[dlft_user_index].status))
        {
             return FALSE;
        }

        if(FALSE == USERAUTH_SetLoginLocalUserPrivilege((char *)userauth_dflt_user_ar[dlft_user_index].username,
                                                userauth_dflt_user_ar[dlft_user_index].privilege))
        {
            USERAUTH_SetLoginLocalUserStatus(userauth_dflt_user_ar[dlft_user_index].username,
                                             USERAUTH_ENTRY_INVALID);
            return FALSE;
        }

        L_MD5_MDString(digest, userauth_dflt_user_ar[dlft_user_index].password,
            strlen((char *)userauth_dflt_user_ar[dlft_user_index].password));
        digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        if(FALSE == USERAUTH_SetLoginLocalUserPassword((char *)userauth_dflt_user_ar[dlft_user_index].username, (char *)digest))
        {
            USERAUTH_SetLoginLocalUserStatus(userauth_dflt_user_ar[dlft_user_index].username,
                                             USERAUTH_ENTRY_INVALID);
             return FALSE;
        }

    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_TotalNumberOfAdminUser
 * ---------------------------------------------------------------------
 * PURPOSE : This function will Calculate the number of administrator privilege
 *           level account.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : count - number of administrator privilege level account.
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static UI32_T
USERAUTH_TotalNumberOfAdminUser()
{
    UI32_T count = 0;
    int i;

    for (i = 0; i < _countof(userauth_login_local); i ++)
    {
        if ((userauth_login_local[i].privilege == USERAUTH_ADMIN_USER_PRIVILEGE) &&
            (userauth_login_local[i].status == USERAUTH_ENTRY_VALID))
        {
            count ++;
        }
    }

    return count;
}

void USERAUTH_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system info
 *
 * INPUT: None
 * OUTPUT: None
 * RETURN: None
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void USERAUTH_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

    /* add by Zoe_Chu 03/30/2006
     * to initial userauth_login_local database.
     */

    memset(userauth_login_local, 0, sizeof(userauth_login_local));
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function will be called when provision complete
 * INPUT: None
 * OUTPUT: None
 * RETURN: None
 * NOTES   : Create default administrator accounts when booted and no
 *           administrator privilege level account exist.
 * ---------------------------------------------------------------------
 */
void USERAUTH_ProvisionComplete(void)
{

/* add by Zoe_Chu 03/30/2006
 * This is only for 3COM style to add a new "manager" user.
 * Because USERAUTH_SetConfigSettingToDefault() can add a new "manager" user
 * only when user initialize the switch setting.
 * So here we examine if the new user exists or not.
 * If it doesn't exist, add a new "manager" user.
 */

#if (SYS_CPNT_USERAUTH_MANAGER_STYLE == SYS_CPNT_USERAUTH_MANAGER_STYLE_3COM)

    USERAUTH_LoginLocal_T   temp_default_user;
    UI8_T   encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1];

    memset(&temp_default_user, 0, sizeof(temp_default_user));
    strncpy((char *)temp_default_user.username, SYS_DFLT_USERAUTH_MANAGER_USERNAME, SYS_ADPT_MAX_USER_NAME_LEN);
    L_MD5_MDString(encrypted_password, SYS_DFLT_USERAUTH_MANAGER_PASSWORD, strlen(SYS_DFLT_USERAUTH_MANAGER_PASSWORD));
    encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

    if(USERAUTH_GetLoginLocalUser(&temp_default_user) == FALSE )
    {
        if(USERAUTH_SetLoginLocalUserStatus( SYS_DFLT_USERAUTH_MANAGER_USERNAME, USERAUTH_ENTRY_VALID ) == FALSE ||
           USERAUTH_SetLoginLocalUserPassword( SYS_DFLT_USERAUTH_MANAGER_USERNAME, encrypted_password ) == FALSE ||
           USERAUTH_SetLoginLocalUserPrivilege( SYS_DFLT_USERAUTH_MANAGER_USERNAME, SYS_DFLT_USERAUTH_MANAGER_PRIVILEGE ) == FALSE)
            return;
}

#endif

    return;

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system info
 *
 * INPUT: None
 * OUTPUT: None
 * RETURN: None
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void USERAUTH_EnterMasterMode(void)
{
    USERAUTH_SetConfigSettingToDefault();
    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system info
 *
 * INPUT: None
 * OUTPUT: None
 * RETURN: None
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void USERAUTH_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_GetSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified SNMP community
 *          string can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community string
 * OUTPUT: comm_entry                  - SNMP community info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The SNMP community string can only be accessed by CLI and Web.
 *           SNMP management station CAN NOT access the SNMP community string.
 *        2. There is no MIB to define the SNMP community string.
 *        3. Status of each SNMP community is defined as following:
 *              - invalid(0): the entry of this community string is deleted/purged
 *              - enabled(1): this community string is enabled
 *              - disabled(2): this community string is disabled

 *           Set status to invalid(0) will delete/purge a community string.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *           Access right of community string could be
 *              - READ_ONLY(1)
 *              - READ_WRITE(2)
 *
 *        4. The total number of SNMP community string supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *        5. By default, two SNMM community string are configued:
 *              - "PUBLIC"      READ-ONLY       enabled
 *              - "PRIVATE"     READ-WRITE      enabled
 *        6. For any SNMP request command, all of enabled community string
 *           shall be used to authorize if this SNMP request is legal/permit.
 * ---------------------------------------------------------------------
 */
BOOL_T USERAUTH_GetSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry)
{
    UI32_T  i;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
        {
            if (userauth_snmp_community[i].status == USERAUTH_ENTRY_INVALID)
                continue;

            if (strcmp((char *)comm_entry->comm_string_name, (char *)userauth_snmp_community[i].comm_string_name) == 0)
                break;
        }

        if (i == SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING)
        {
            USERAUTH_LEAVE_CRITICAL_SECTION();
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETSNMPCOMMUNITY_FUNC_NO, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO ,"Snmp Community"/*arg_p*/);

            return FALSE;
        }

        //*comm_entry = userauth_snmp_community[i];
        memcpy(comm_entry, &(userauth_snmp_community[i]), sizeof(USERAUTH_SnmpCommunity_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_GetNextSnmpCommunity
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
BOOL_T USERAUTH_GetNextSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry)
{
    USERAUTH_SnmpCommunity_T    *tmp_entry = NULL;
    UI32_T                      i;
    BOOL_T                      found=FALSE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if(comm_entry == NULL)
        {
            USERAUTH_RELEASE_CSC();
            return FALSE;
        }
        if (strlen((char *)comm_entry->comm_string_name) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETnextSNMPCOMMUNITY_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
        {
            if (userauth_snmp_community[i].status == USERAUTH_ENTRY_INVALID)
                continue;

            /* amytu add 1-28-2002 */
            if (strcmp((char *)userauth_snmp_community[i].comm_string_name, (char *)comm_entry->comm_string_name) > 0)
            {
                if (found == FALSE)
                {
                    found = TRUE;
                    tmp_entry = &(userauth_snmp_community[i]);
                }

                if ((strcmp((char *)userauth_snmp_community[i].comm_string_name, (char *)tmp_entry->comm_string_name) < 0) &&
                    (strcmp((char *)userauth_snmp_community[i].comm_string_name, (char *)comm_entry->comm_string_name) > 0))
                {
                    tmp_entry = &(userauth_snmp_community[i]);
                }
            }
        }


        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (found == TRUE)
        {
            //*comm_entry = tmp_entry;
            memcpy(comm_entry, tmp_entry, sizeof(USERAUTH_SnmpCommunity_T));
            USERAUTH_RELEASE_CSC();
            return TRUE;
        }
    }
    USERAUTH_RELEASE_CSC();
    return FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_GetNextRunningSnmpCommunity
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
UI32_T USERAUTH_GetNextRunningSnmpCommunity(USERAUTH_SnmpCommunity_T *comm_entry)
{
    /*BOOL_T  found = FALSE;*/

    if (/*found = */USERAUTH_GetNextSnmpCommunity(comm_entry) == TRUE)
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_SetSnmpCommunityAccessRight
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
BOOL_T USERAUTH_SetSnmpCommunityAccessRight(UI8_T *comm_string_name, UI32_T access_right)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if( strlen((char *)comm_string_name) <= 0 )
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYACCESSRIGHT_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        if (strlen((char *)comm_string_name) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYACCESSRIGHT_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);

            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        if ((access_right != USERAUTH_ACCESS_RIGHT_READ_ONLY) &&
            (access_right != USERAUTH_ACCESS_RIGHT_READ_WRITE) )
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYACCESSRIGHT_FUNC_NO, EH_TYPE_MSG_IS_INVALID, SYSLOG_LEVEL_INFO ,"Access right"/*arg_p*/);

            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
        {
            if (userauth_snmp_community[i].status == USERAUTH_ENTRY_INVALID)
            {
                /* record the first empty entry.  When the entry is not existed, this
                 * entry will be updated as the input.
                 */
                if (available_entry == -1)
                    available_entry = i;
                continue;
            }

            if (strcmp((char *)comm_string_name, (char *)userauth_snmp_community[i].comm_string_name) == 0)
            {
                available_entry = i;
                break;
            }
        }


        if (i == SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING)
        {
            /* No found, create a new one, if no more available entry then return FALSE
             */
            if (available_entry != -1)
            {
                memset(userauth_snmp_community[available_entry].comm_string_name, 0, SYS_ADPT_MAX_COMM_STR_NAME_LEN+1);
                strncpy((char *)userauth_snmp_community[available_entry].comm_string_name, (char *)comm_string_name, SYS_ADPT_MAX_COMM_STR_NAME_LEN);

                /* Append the null character for the rest of characters which are not used
                 */
                for (i=strlen((char *)comm_string_name); i<=SYS_ADPT_MAX_COMM_STR_NAME_LEN; i++)
                    userauth_snmp_community[available_entry].comm_string_name[i] = 0;

                userauth_snmp_community[available_entry].access_right = access_right;
                return_val = TRUE;
            }
            else
            {
                /* no available entry, can not create new entry
                 */
                /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYACCESSRIGHT_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);

                return_val = FALSE;
            }
        }
        else
        {
            /* found an entry, update new access_right
             */
            userauth_snmp_community[available_entry].access_right = access_right;
            return_val = TRUE;
        }

#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
        CFGDB_MGR_WriteSection(userauth_snmp_community_session_handler, userauth_snmp_community);
#endif
        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_SetSnmpCommunityStatus
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
BOOL_T USERAUTH_SetSnmpCommunityStatus(UI8_T *comm_string_name, UI32_T status)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;


    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if( strlen((char *)comm_string_name) <= 0 )
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYSTATUS_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);
                    /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        if (strlen((char *)comm_string_name) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYSTATUS_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);

                    /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        if ((status != USERAUTH_ENTRY_VALID) &&
            (status != USERAUTH_ENTRY_INVALID) )
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYSTATUS_FUNC_NO, EH_TYPE_MSG_IS_INVALID, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
        {
            if (userauth_snmp_community[i].status == USERAUTH_ENTRY_INVALID)
            {
                /* record the first empty entry.  When the entry is not existed, this
                 * entry will be updated as the input.
                 */
                if (available_entry == -1)
                    available_entry = i;
                continue;
            }

            if (strcmp((char *)comm_string_name, (char *)userauth_snmp_community[i].comm_string_name) == 0)
            {
                available_entry = i;
                break;
            }
        }


        if (i == SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING)
        {
            /* No found, create a new one, if no more available entry then return FALSE
             */
            if (available_entry != -1)
            {
                memset(userauth_snmp_community[available_entry].comm_string_name, 0, SYS_ADPT_MAX_COMM_STR_NAME_LEN+1);
                strncpy((char *)userauth_snmp_community[available_entry].comm_string_name, (char *)comm_string_name, SYS_ADPT_MAX_COMM_STR_NAME_LEN);

                /* Append the null character for the rest of characters which are not used
                 */
                for (i=strlen((char *)comm_string_name); i<=SYS_ADPT_MAX_COMM_STR_NAME_LEN; i++)
                    userauth_snmp_community[available_entry].comm_string_name[i] = 0;

                userauth_snmp_community[available_entry].status = status;
                return_val = TRUE;
            }
            else
            {
                /* no available entry, can not create new entry
                 */
                    /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETSNMPCOMMUNITYSTATUS_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);

                return_val = FALSE;
            }
        }
        else
        {
            /* found an entry, update status.  By logically, it should be deleted.
             */
            userauth_snmp_community[available_entry].status = status;
            return_val = TRUE;
        }

#if (SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB == TRUE)
        CFGDB_MGR_WriteSection(userauth_snmp_community_session_handler, userauth_snmp_community);
#endif
        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetLoginLocalUser
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets login user's entry by specified user name
 *  INPUT   : login_user->username (key) in the record
 *  OUTPUT  : login_user a whole record on specified user name
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : If user use the key of super user's username, this function
 *            will return the inhibited super user record.
 */
BOOL_T USERAUTH_GetLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    UI32_T  i;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        /* Note that "+ 1" right here is correct, look at the declaration, we add
         * extra super user.
         */
        for (i=0; i<(SYS_ADPT_MAX_NBR_OF_LOGIN_USER + 1); i++)
        {
            if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
                continue;

            if (strcmp((char *)login_user->username, (char *)userauth_login_local[i].username) == 0)
                break;
        }

        if (i == (SYS_ADPT_MAX_NBR_OF_LOGIN_USER + 1))
        {
            USERAUTH_LEAVE_CRITICAL_SECTION();
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Login local user"/*arg_p*/);

            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            return FALSE;
        }

        //*login_user = userauth_login_local[i];
        memcpy(login_user,  &(userauth_login_local[i]), sizeof(USERAUTH_LoginLocal_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetNextLoginLocalUser
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets next legal login user's entry
 *  INPUT   : login_user->username (key) in the record
 *  OUTPUT  : login_user a whole record on specified user name
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
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
BOOL_T USERAUTH_GetNextLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    UI32_T  i, j;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if (strlen((char *)login_user->username) > SYS_ADPT_MAX_USER_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        if (login_user->username[0] != 0)
        {
            /* 1) Note that "+ 1" right here is not necessary, since we don't like CLI to show
             *    super user on the screen.
             * 2) username is null means user is not trying to get first username in the list.
             *    The username key must exist in the name list.
             */
            for (i=0; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER; i++)
            {
                if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
                    continue;

                if (strcmp((char *)login_user->username, (char *)userauth_login_local[i].username) == 0)
                {
                    i++;    /* go to next user, if there is any  */
                    break;
                }
            }

            if (i == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
            {
                USERAUTH_LEAVE_CRITICAL_SECTION();
                USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Login local user"/*arg_p*/);

                return FALSE;
            }
        }
        else
        {
            /* User wants to get first valid one. */
            i = 0;
        }

        /* Get next valid one
         */
        for (j=i; j<SYS_ADPT_MAX_NBR_OF_LOGIN_USER; j++)
        {
            if (userauth_login_local[j].status == USERAUTH_ENTRY_VALID)
                break;
        }

        /* The input username is the last one(for get next case)
         * or there is no valid entry(for get first one case).
         */
        if (j == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
        {
            USERAUTH_LEAVE_CRITICAL_SECTION();
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Login local user"/*arg_p*/);

            return FALSE;
        }

        //*login_user = userauth_login_local[j];
        memcpy(login_user, &(userauth_login_local[j]), sizeof(USERAUTH_LoginLocal_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_LocaIsDefaultUserUnChangeed
 * ---------------------------------------------------------------------
 * PURPOSE : This function will check default account hase been modified
 *           or not.
 * INPUT   : checked_user_p-userauth account
 * OUTPUT  : None
 * RETURN  : TRUE-changed FALSE-no changed
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static BOOL_T
USERAUTH_LocaIsDefaultUserUnChangeed(
USERAUTH_LoginLocal_T *checked_user_p
)
{
    UI32_T user_index;
    UI8_T digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1];
    USERAUTH_LoginLocal_T dflt_user_ar[] = { USERAUTH_DFLTUSER_ACCOUNT(USERAUTH_ACCOUNTINFO) };

    /* todo if default user no change return true.
     */
    for (user_index = 0; user_index < USERAUTH_MAX_DFLTACCOUNT_NUM; user_index++)
    {
        if (0 == strcmp((char *)checked_user_p->username, (char *)dflt_user_ar[user_index].username))
        {
            L_MD5_MDString(digest, dflt_user_ar[user_index].password, strlen((char *)dflt_user_ar[user_index].password));
            digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

            if ((checked_user_p->privilege != dflt_user_ar[user_index].privilege)
                || (memcmp(digest, checked_user_p->password, sizeof(digest)) != 0)
                )
            {
                    return FALSE;
            }

            return TRUE;
        }
    }

    return FALSE;
}

/* ---------------------------------------------------------------------
* ROUTINE NAME  - USERAUTH_GetRunningAllLoginLocalUser
* ---------------------------------------------------------------------
* PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
*          next available non-default user/password can be retrieved
*          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
*          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
* INPUT:
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
SYS_TYPE_Get_Running_Cfg_T USERAUTH_GetRunningAllLoginLocalUser(USERAUTH_LoginLocal_T login_user_ar[], UI32_T *get_number_p)
{
    UI32_T user_index;
    UI32_T get_user_index = 0;
    USERAUTH_LoginLocal_T dflt_user_ar[] = { USERAUTH_DFLTUSER_ACCOUNT(USERAUTH_ACCOUNTINFO) };

    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (get_number_p == NULL)
    {
        USERAUTH_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (login_user_ar == NULL)
    {
        USERAUTH_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    USERAUTH_ENTER_CRITICAL_SECTION();

    /* get the 1st level 15 user
     */
    for (user_index = 0; user_index < SYS_ADPT_MAX_NBR_OF_LOGIN_USER; user_index++)
    {
        if (userauth_login_local[user_index].status == USERAUTH_ENTRY_INVALID)
        {
            continue;
        }

        if (TRUE == USERAUTH_LocaIsDefaultUserUnChangeed(&userauth_login_local[user_index]))
        {
            continue;
        }

        if (userauth_login_local[user_index].privilege == SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            memcpy(&login_user_ar[get_user_index],
                    &userauth_login_local[user_index],
                    sizeof(login_user_ar[get_user_index]));
            get_user_index++;
            break;
        }
    }

    /* get removed default useraccount
     */
    for (user_index = 0; user_index < USERAUTH_MAX_DFLTACCOUNT_NUM; user_index++)
    {
        UI32_T sub_index;

        dflt_user_ar[user_index].status = USERAUTH_ENTRY_INVALID;

        for (sub_index = 0; sub_index < SYS_ADPT_MAX_NBR_OF_LOGIN_USER; sub_index++)
        {
            if (userauth_login_local[sub_index].status == USERAUTH_ENTRY_INVALID)
            {
                continue;
            }

            if (strcmp((char *)userauth_login_local[sub_index].username,
                        (char *)dflt_user_ar[user_index].username) == 0)
            {
                dflt_user_ar[user_index].status = USERAUTH_ENTRY_VALID;
                break;
            }
        }

        if (dflt_user_ar[user_index].status == USERAUTH_ENTRY_INVALID)
        {
            memcpy(&login_user_ar[get_user_index],
                    &dflt_user_ar[user_index],
                    sizeof(login_user_ar[get_user_index]));
            get_user_index++;
        }
    }

    /* get other user
     */
    for (user_index = 0; user_index < SYS_ADPT_MAX_NBR_OF_LOGIN_USER; user_index++)
    {
        if (userauth_login_local[user_index].status == USERAUTH_ENTRY_INVALID)
        {
            continue;
        }

        if (TRUE == USERAUTH_LocaIsDefaultUserUnChangeed(&userauth_login_local[user_index]))
        {
            continue;
        }

        if (get_user_index >= 1 &&
            strcmp((char *)login_user_ar[0].username,
                    (char *)userauth_login_local[user_index].username) == 0)
        {
            continue;
        }

        memcpy(&login_user_ar[get_user_index],
                &userauth_login_local[user_index],
                sizeof(login_user_ar[get_user_index]));
        get_user_index++;
    }

    *get_number_p = get_user_index;
    USERAUTH_LEAVE_CRITICAL_SECTION();
    USERAUTH_RELEASE_CSC();

    if (get_user_index == 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - USERAUTH_GetNextRunningLoginLocalUser
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
UI32_T USERAUTH_GetNextRunningLoginLocalUser(USERAUTH_LoginLocal_T *login_user)
{
    UI32_T  i, j;

    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if (strlen((char *)login_user->username) > SYS_ADPT_MAX_USER_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTRUNNINGLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Community string length"/*arg_p*/);

            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        if (login_user->username[0] != 0)
        {
            /* 1) Note that "+ 1" right here is not necessary, since we don't like CLI to show
             *    super user on the screen.
             * 2) username is null means user is not trying to get first username in the list.
             *    The username key must exist in the name list.
             */
            for (i=0; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER; i++)
            {
                if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
                    continue;

                if (strcmp((char *)login_user->username, (char *)userauth_login_local[i].username) == 0)
                {
                    i++;    /* go to next user, if there is any  */
                    break;
                }
            }

            if (i == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
            {
                USERAUTH_LEAVE_CRITICAL_SECTION();
                USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTRUNNINGLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Login local user"/*arg_p*/);

                return SYS_TYPE_GET_RUNNING_CFG_FAIL;
            }
        }
        else
        {
            /* User wants to get first valid one. */
            i = 0;
        }

        /* Get next valid one
         */
        for (j=i; j<SYS_ADPT_MAX_NBR_OF_LOGIN_USER; j++)
        {
            if (userauth_login_local[j].status == USERAUTH_ENTRY_VALID)
                break;
        }

        /* The input username is the last one(for get next case)
         * or there is no valid entry(for get first one case).
         */
        if (j == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
        {
            USERAUTH_LEAVE_CRITICAL_SECTION();
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTRUNNINGLOGINLOCALUSER_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Login local user"/*arg_p*/);

            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        //*login_user = userauth_login_local[j];
        memcpy(login_user, &(userauth_login_local[j]), sizeof(USERAUTH_LoginLocal_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetLoginLocalUserStatus
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets username password entry status based on
 *            specified privilege.  If the username is not in the database,
 *            we will create a new user if the user's number is not exceed
 *            the limit.
 *  INPUT   : username (key)
 *          : username_length
 *          : status(USERAUTH_ENTRY_VALID/USERAUTH_ENTRY_INVALID)
 *  OUTPUT  : username
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T
USERAUTH_SetLoginLocalUserStatus(
    UI8_T *username,
    UI32_T status)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    UI32_T  privilege;
    BOOL_T  return_val = FALSE;
    BOOL_T  isexist = FALSE;

    USERAUTH_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code);*/

        return FALSE;
    }
    else
    {
        if( strlen((char *)username) <= 0 )
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code);*/
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                     SYSLOG_LEVEL_INFO ,
                                     "Community string length"/*arg_p*/);

            return FALSE;
        }

        if (strlen((char *)username) > SYS_ADPT_MAX_USER_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code);*/
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                     SYSLOG_LEVEL_INFO ,
                                     "Community string length"/*arg_p*/);

            return FALSE;
        }

/*isiah.2003-08-05*/
#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)

        /* when set username as SYS_DFLT_3COM_PASSWORD_RECOVERY_USER
         * and password as SYS_DFLT_3COM_PASSWORD_RECOVERY_PASSWORD
         */

        if((strcmp((char *)username, SYS_DFLT_3COM_PASSWORD_RECOVERY_USER) == 0))
        {
           /* remember show error message
            */
           EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                    USERAUTH_SETLOGINLOCALUSERPASSWORD_FUNC_NO,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "user name and password reserved for password recovery");
           USERAUTH_RELEASE_CSC();

           return FALSE;
        }
#endif

/*isiah.2003-05-26*/
#if ( SYS_CPNT_USERAUTH_PROTECT_DEFAULT_USERS == TRUE )
        if( status == USERAUTH_ENTRY_INVALID )
        {
            #if (SYS_CPNT_USERAUTH_MANAGER_STYLE == SYS_CPNT_USERAUTH_MANAGER_STYLE_3COM)
            if( strcmp((char *)username, SYS_DFLT_USERAUTH_ADMIN_USERNAME) == 0 ||
                strcmp((char *)username, SYS_DFLT_USERAUTH_MANAGER_USERNAME) == 0 ||
                strcmp((char *)username, SYS_DFLT_USERAUTH_GUEST_USERNAME) == 0 )
            #else
            if( strcmp((char *)username, SYS_DFLT_USERAUTH_ADMIN_USERNAME) == 0 ||
                strcmp((char *)username, SYS_DFLT_USERAUTH_GUEST_USERNAME) == 0 )
            #endif
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                         USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO,
                                         EH_TYPE_MSG_FAILED_TO_DELETE,
                                         SYSLOG_LEVEL_INFO,
                                         "protected user");
                USERAUTH_RELEASE_CSC();

                return FALSE;
            }
        }
#endif /* SYS_CPNT_USERAUTH_PROTECT_DEFAULT_USERS == TRUE */

        if ((status != USERAUTH_ENTRY_VALID) &&
            (status != USERAUTH_ENTRY_INVALID) )
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO,
                                     EH_TYPE_MSG_IS_INVALID,
                                     SYSLOG_LEVEL_INFO,
                                     "Entry"/*arg_p*/);
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        privilege = 0;

        /* "+ 1" is not necessary.  We can not allow user to set super username and password.
         */
        for (i = 0; i < _countof(userauth_login_local) - 1; i ++)
        {
            if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
            {
                /* record the first empty entry.  When the entry is not existed, this
                 * entry will be updated as the input.
                 */
                if (available_entry == -1)
                {
                    available_entry = i;
                }

                continue;
            }

            if (strcmp((char *)username, (char *)userauth_login_local[i].username) == 0)
            {
                isexist = TRUE;
                available_entry = i;
                privilege = userauth_login_local[i].privilege;
                break;
            }

        }

        /* Device shall have at least one administrator account.
         */
        if ((status == USERAUTH_ENTRY_INVALID) &&
            (privilege == USERAUTH_ADMIN_USER_PRIVILEGE) &&
            (USERAUTH_TotalNumberOfAdminUser() == 1))
        {
            return FALSE;
        }

        /* Delete inexist user, return FALSE
         */
        if((isexist == FALSE) && (status == USERAUTH_ENTRY_INVALID))
            return FALSE;

        if (i == _countof(userauth_login_local) - 1)
        {
            /* No found, create a new one, if no more available entry then return FALSE.
             * userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER - 1] = default admin user
             * userauth_login_local[SYS_ADPT_MAX_NBR_OF_LOGIN_USER] = super user
             */
            if ((available_entry != -1) &&
                (available_entry != SYS_ADPT_MAX_NBR_OF_LOGIN_USER))
            {
                memset(userauth_login_local[available_entry].username, 0, SYS_ADPT_MAX_USER_NAME_LEN + 1);
                strncpy((char *)userauth_login_local[available_entry].username,
                        (char *)username,
                        SYS_ADPT_MAX_USER_NAME_LEN);
                memset(userauth_login_local[available_entry].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
                userauth_login_local[available_entry].status = status;

                USERAUTH_SendUserAccountTrap(&userauth_login_local[available_entry],
                                             TRAP_EVENT_USERAUTH_CREATE_USER);

                return_val = TRUE;
            }
            else
            {
                /* no available entry, can not create new entry
                 */
                /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                         USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO,
                                         EH_TYPE_MSG_NOT_EXIST,
                                         SYSLOG_LEVEL_INFO,
                                         "Entry"/*arg_p*/);

                return_val = FALSE;
            }
        }
        else
        {
            /* found an entry, update status.  By logically, it should be deleted.
             */
            userauth_login_local[available_entry].status = status;

            USERAUTH_SendUserAccountTrap(&userauth_login_local[available_entry],
                                         TRAP_EVENT_USERAUTH_DELETE_USER);

            return_val = TRUE;
        }
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
        CFGDB_MGR_WriteSection(userauth_login_local_session_handler, userauth_login_local);
#endif
        USERAUTH_LEAVE_CRITICAL_SECTION();

    }

    USERAUTH_RELEASE_CSC();

    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetLoginLocalUserPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets password based on specified privilege
 *  INPUT   : username (key)
 *          : password
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetLoginLocalUserPassword(const char *username, const char *password)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)

        /* when set username as SYS_DFLT_3COM_PASSWORD_RECOVERY_USER */
        /* and password as SYS_DFLT_3COM_PASSWORD_RECOVERY_PASSWORD */

        if((strcmp((char *)username,SYS_DFLT_3COM_PASSWORD_RECOVERY_USER) == 0)&&(strcmp((char *)password,SYS_DFLT_3COM_PASSWORD_RECOVERY_PASSWORD) == 0))
        {
           /* remember show error message */
           EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETLOGINLOCALUSERPASSWORD_FUNC_NO, EH_TYPE_MSG_FAILED_TO_ADD, SYSLOG_LEVEL_INFO, "user name and password reserved for password recovery");
           USERAUTH_RELEASE_CSC();
           return FALSE;
        }
#endif
        if( strlen((char *)username) <= 0 )
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETLOGINLOCALUSERPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Username"/*arg_p*/);
            return FALSE;
        }

        if (strlen((char *)username) > SYS_ADPT_MAX_USER_NAME_LEN)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETLOGINLOCALUSERPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Username"/*arg_p*/);

            return FALSE;
        }

#if 0  /* Now password are encoded as MD5, null character could be one of the character */
    if (strlen(password) > SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN)
        return FALSE;
#endif

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* "+ 1" is not necessary.  We can not allow user to set super username and password.
         */
        for (i=0; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER; i++)
        {
            if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
            {
                /* record the first empty entry.  When the entry is not existed, this
                 * entry will be updated as the input.
                 */
                if (available_entry == -1)
                    available_entry = i;
                continue;
            }

            if (strcmp((char *)username, (char *)userauth_login_local[i].username) == 0)
            {
                available_entry = i;
                break;
            }
        }

        if (i == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
        {
            /* Not found, create a new one, if no more available entry then return FALSE
             */
            if (available_entry != -1)
            {
                memset(userauth_login_local[available_entry].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy((char *)userauth_login_local[available_entry].username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);

                /* Append the null character for the rest of characters which are not used
                 */
                for (i=strlen((char *)username); i<=SYS_ADPT_MAX_USER_NAME_LEN; i++)
                    userauth_login_local[available_entry].username[i] = 0;

                memcpy(userauth_login_local[available_entry].password, password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);

#if 0 /* It's not a pure string any more after we use MD5 encode, it's a byte array */
            /* Append the null character for the rest of characters which are not used
             */
            for (i=strlen(password); i<=SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN; i++)
                userauth_login_local[available_entry].password[i] = 0;
#endif
                return_val = TRUE;
            }
            else
            {
                /* no available entry, can not create new entry
                 */
                /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETLOGINLOCALUSERPASSWORD_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);

                return_val = FALSE;
            }
        }
        else
        {
            /* found an entry, update status.  By logically, it should be deleted.
             */
            memcpy(userauth_login_local[available_entry].password, password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);

#if 0 /* It's not a pure string any more after we use MD5 encode, it's a byte array */
        /* Append the null character for the rest of characters which are not used
         */
        for (i=strlen(password); i<=SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN; i++)
            userauth_login_local[available_entry].password[i] = 0;
#endif
            return_val = TRUE;
        }

#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
        CFGDB_MGR_WriteSection(userauth_login_local_session_handler, userauth_login_local);
#endif
        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return return_val;

}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetLoginLocalUserPrivilege
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets password based on specified privilege
 *  INPUT   : username (key)
 *          : username_length
 *          : password
 *          : privilege
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T
USERAUTH_SetLoginLocalUserPrivilege(
    const char *username,
    UI32_T privilege)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        if (strlen((char *)username) <= 0 )
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERPRIVILEGE_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                     SYSLOG_LEVEL_INFO,
                                     "Username"/*arg_p*/);
            return FALSE;
        }

        if (SYS_ADPT_MAX_USER_NAME_LEN < strlen((char *)username))
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERPRIVILEGE_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                     SYSLOG_LEVEL_INFO,
                                     "Username"/*arg_p*/);

            return FALSE;
        }

        if (SYS_ADPT_MAX_LOGIN_PRIVILEGE < privilege)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                     USERAUTH_SETLOGINLOCALUSERPRIVILEGE_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                     SYSLOG_LEVEL_INFO,
                                     "Privilege"/*arg_p*/);

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* "+ 1" is not necessary.  We can not allow user to set super username and password.
         */
        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_LOGIN_USER; i++)
        {
            if (userauth_login_local[i].status == USERAUTH_ENTRY_INVALID)
            {
                /* record the first empty entry.  When the entry is not existed, this
                 * entry will be updated as the input.
                 */
                if (available_entry == -1)
                {
                    available_entry = i;
                }
                continue;
            }

            if (strcmp((char *)username, (char *)userauth_login_local[i].username) == 0)
            {
                available_entry = i;
                break;
            }
        }

        if (i == SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
        {
            /* No found, create a new one, if no more available entry then return FALSE
             */
            if (available_entry != -1)
            {
                memset(userauth_login_local[available_entry].username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy((char *)userauth_login_local[available_entry].username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);
                memset(userauth_login_local[available_entry].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
                userauth_login_local[available_entry].privilege = privilege;

                USERAUTH_SendUserAccountTrap(&userauth_login_local[available_entry],
                                             TRAP_EVENT_USERAUTH_CREATE_USER);

                return_val = TRUE;
            }
            else
            {
                /* no available entry, can not create new entry
                 */
                /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
                EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH,
                                         USERAUTH_SETLOGINLOCALUSERPRIVILEGE_FUNC_NO,
                                         EH_TYPE_MSG_NOT_EXIST,
                                         SYSLOG_LEVEL_INFO,
                                         "Entry"/*arg_p*/);

                return_val = FALSE;
            }
        }
        else
        {
            if (userauth_login_local[available_entry].privilege == privilege)
            {
                return_val = TRUE;
            }
            else if ((userauth_login_local[available_entry].privilege == USERAUTH_ADMIN_USER_PRIVILEGE) &&
                        (1 == USERAUTH_TotalNumberOfAdminUser()))
            {
                return_val = FALSE;
            }
            else
            {
                userauth_login_local[available_entry].privilege = privilege;
                USERAUTH_SendUserAccountTrap(&userauth_login_local[available_entry],
                                             TRAP_EVENT_USERAUTH_MODIFY_USER_PRIVILEGE);
                return_val = TRUE;
#if (SYS_CPNT_USERAUTH_USER_IN_CFGDB == TRUE)
                CFGDB_MGR_WriteSection(userauth_login_local_session_handler, userauth_login_local);
#endif
            }

        }

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }

    USERAUTH_RELEASE_CSC();

    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetPrivilegePassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *  OUTPUT  : privilege_password in record
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    UI32_T  level = privilege_password->privilege;
    BOOL_T  return_val = TRUE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if (level > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Login privilege level"/*arg_p*/);/*Mercury_V2-00030*/

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* The privilege key is just the same as index
         */
        if (userauth_privilege_password[level].status == USERAUTH_ENTRY_VALID)
            //*privilege_password = userauth_privilege_password[level];
            memcpy(privilege_password, &(userauth_privilege_password[level]), sizeof(USERAUTH_Privilege_Password_T));
        else
        {
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);

            return_val = FALSE;
        }

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetNextPrivilegePassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets next valid privilege password based on
 *            specified privilege
 *  INPUT   : privilege (key)
 *  OUTPUT  : privilege_password in record
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetNextPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    UI32_T  level = privilege_password->privilege;
    UI32_T  i;
    BOOL_T  return_val = TRUE;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        /* even the key is the last one, we still return FALSE, because there is no next entry
         */
        if (level >= SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Login privilege level"/*arg_p*/);

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* Search from next privilege level
         */
        for (i=level+1; i<=SYS_ADPT_MAX_LOGIN_PRIVILEGE; i++)
        {
            /* Search until we found a valid entry
             */
            if (userauth_privilege_password[i].status == USERAUTH_ENTRY_VALID)
                break;
        }

        if (i == SYS_ADPT_MAX_LOGIN_PRIVILEGE)
           {
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO ,"Entry"/*arg_p*/);

            return_val = FALSE;
           }
        else
            //*privilege_password = userauth_privilege_password[i];
        memcpy(privilege_password, &(userauth_privilege_password[i]), sizeof(USERAUTH_Privilege_Password_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetFirstRunningPrivilegePassword
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
BOOL_T USERAUTH_GetFirstRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    UI32_T  level = 0;/*Charles, init from 0 not privilege_password.privilege*/
    UI32_T  i;
    BOOL_T  return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        /* Even the key is the last one, we still return FALSE, because there is no next entry
         */
        if (level > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETFIRSTRUNNINGPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Login privilege level"/*arg_p*/);

            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* Search from the specified privilege level
         */
        /* Charles, the max value should be SYS_ADPT_MAX_LOGIN_PRIVILEGE,
         * so "=" is necessary
         */
        for (i=level; i<=SYS_ADPT_MAX_LOGIN_PRIVILEGE; i++)
        {
            /* Search until we found a valid entry
             */
            if (userauth_privilege_password[i].status == USERAUTH_ENTRY_VALID)
                break;
        }
        /* Charles, the max value should be SYS_ADPT_MAX_LOGIN_PRIVILEGE,
         * so "==" change to ">"
         */
        if (i > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
            return_val = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        else
            //*privilege_password = userauth_privilege_password[i];
        memcpy(privilege_password, &(userauth_privilege_password[i]), sizeof(USERAUTH_Privilege_Password_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetNextRunningPrivilegePassword
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
BOOL_T USERAUTH_GetNextRunningPrivilegePassword(USERAUTH_Privilege_Password_T *privilege_password)
{
    UI32_T  level = privilege_password->privilege;
    UI32_T  i;
    BOOL_T  return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;


    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        /* Even the key is the last one, we still return FALSE, because there is no next entry
         */
        if (level >= SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_GETNEXTRUNNINGPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Login privilege level"/*arg_p*/);

            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* Search from next privilege level
         */
        for (i=level+1; i<=SYS_ADPT_MAX_LOGIN_PRIVILEGE; i++)
        {
            /* Search until we found a valid entry
             */
            if (userauth_privilege_password[i].status == USERAUTH_ENTRY_VALID)
                break;
        }

        if (i > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
            return_val = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        else
            //*privilege_password = userauth_privilege_password[i];
        memcpy(privilege_password, &(userauth_privilege_password[i]), sizeof(USERAUTH_Privilege_Password_T));


        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return return_val;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetPrivilegePasswordStatus
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *            privilege_password
 *  OUTPUT  : status(USERAUTH_ENTRY_VALID/USERAUTH_ENTRY_INVALID)
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : User needs to be careful that he needs to set privilege password
 *            first and then set the status valid.  Otherwise, the password
 *            could be anything(previous setting).
 */
BOOL_T USERAUTH_SetPrivilegePasswordStatus(UI32_T privilege, UI32_T status)
{

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if (privilege > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETLOGINLOCALUSERSTATUS_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Privilege"/*arg_p*/);

            return FALSE;
        }

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* The privilege key is just the same as index
         */
        userauth_privilege_password[privilege].status = status;

        /* Charles,
         * Although the privilege key is just the same as index,
         * but the other place will use this information,
         * For example, USERAUTH_GetNextRunningPrivilegePassword
         */
        userauth_privilege_password[privilege].privilege = privilege;

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetPrivilegePassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets privilege password based on specified
 *            privilege
 *  INPUT   : privilege (key)
 *            password
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetPrivilegePassword(UI32_T privilege, UI8_T *password)
{

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if (privilege > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
        {
            USERAUTH_RELEASE_CSC();
            /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
            EH_MGR_Handle_Exception1(SYS_MODULE_USERAUTH, USERAUTH_SETPRIVILEGEPASSWORD_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Privilege"/*arg_p*/);

            return FALSE;
        }

#if 0 /* password is by MD5 encode, it's byte array instead of string any more */
    if (strlen(password) > SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN)
        return FALSE;
#endif

        USERAUTH_ENTER_CRITICAL_SECTION();

        /* The privilege key is just the same as index
         */
        memset(userauth_privilege_password[privilege].password, 0, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1);
        memcpy(userauth_privilege_password[privilege].password, password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        /* Charles,
         * Although the privilege key is just the same as index,
         * but the other place will use this information,
         * For example, USERAUTH_GetNextRunningPrivilegePassword
         */
        userauth_privilege_password[privilege].privilege = privilege;

        USERAUTH_LEAVE_CRITICAL_SECTION();

    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets authentication method
 *  INPUT   : auth_method
 *  OUTPUT  : auth_method
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */

/* design changed for TACACS by JJ, 2002-05-24 */

BOOL_T USERAUTH_GetAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    BOOL_T  ret;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_CONSOLE);
        USERAUTH_RELEASE_CSC();
        return ret;
    }
#if 0
        USERAUTH_ENTER_CRITICAL_SECTION();
        memcpy (auth_method, userauth_auth_method,
                sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
#endif
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets authentication method
 *  INPUT   : auth_method
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */

/* design changed for TACACS by JJ, 2002-05-24 */

BOOL_T USERAUTH_SetAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    BOOL_T  ret;

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_ALL);
        USERAUTH_RELEASE_CSC();
        return ret;
    }


#if 0
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy (userauth_auth_method, auth_method,
                sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
#endif
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningAuthMethod
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
UI32_T USERAUTH_GetRunningAuthMethod(USERAUTH_Auth_Method_T *auth_method)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        ret = USERAUTH_GetRunningAuthMethodBySessionType(auth_method, USERAUTH_SESSION_CONSOLE);
        USERAUTH_RELEASE_CSC();
        return ret;
    }
#if 0
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy (auth_method, userauth_auth_method,
            sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (*auth_method == USERAUTH_AUTH_LOCAL &&
            *(auth_method+1) == USERAUTH_AUTH_NONE &&
            *(auth_method+2) == USERAUTH_AUTH_NONE)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
#endif
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetEnableAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets enable authentication method
 *  INPUT   : enable_auth_method
 *  OUTPUT  : enable_auth_method
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();
        memcpy (enable_auth_method, userauth_enable_auth_method,
                sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetEnableAuthMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets enable authentication method
 *  INPUT   : enable_auth_method
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy (userauth_enable_auth_method, enable_auth_method,
                sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningEnableAuthMethod
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
UI32_T USERAUTH_GetRunningEnableAuthMethod(USERAUTH_Auth_Method_T *enable_auth_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy (enable_auth_method, userauth_enable_auth_method,
            sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (*enable_auth_method == USERAUTH_AUTH_LOCAL &&
            *(enable_auth_method+1) == USERAUTH_AUTH_NONE &&
            *(enable_auth_method+2) == USERAUTH_AUTH_NONE)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetTelnetLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets telnet login method
 *  INPUT   : login_method
 *  OUTPUT  : login_method
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetTelnetLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        //*login_method = userauth_telnet_login_method;
        memcpy(login_method, &userauth_telnet_login_method, sizeof(USERAUTH_Login_Method_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetTelnetLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets telnet login method
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetTelnetLoginMethod(USERAUTH_Login_Method_T login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        userauth_telnet_login_method = login_method;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningTelnetLoginMethod
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
UI32_T USERAUTH_GetRunningTelnetLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        //*login_method = userauth_telnet_login_method;
        memcpy(login_method, &userauth_telnet_login_method, sizeof(USERAUTH_Login_Method_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (*login_method == USERAUTH_LOGIN_METHOD_DEF)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetConsolLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets console login method
 *  INPUT   : login_method
 *  OUTPUT  : login_method
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetConsolLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        //*login_method = userauth_console_login_method;
        memcpy(login_method, &userauth_console_login_method, sizeof(USERAUTH_Login_Method_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetConsoleLoginMethod
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets console login method
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetConsoleLoginMethod(USERAUTH_Login_Method_T login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        userauth_console_login_method = login_method;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningConsoleLoginMethod
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
UI32_T USERAUTH_GetRunningConsoleLoginMethod(USERAUTH_Login_Method_T *login_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        //*login_method = userauth_console_login_method;
        memcpy(login_method, &userauth_console_login_method, sizeof(USERAUTH_Login_Method_T));

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (*login_method == USERAUTH_LOGIN_METHOD_DEF)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetConsoleLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets console login password
 *  INPUT   : password
 *  OUTPUT  : password
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetConsoleLoginPassword(char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(password, userauth_console_login_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetConsoleLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets console login password
 *  INPUT   : login_method
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetConsoleLoginPassword(const char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(userauth_console_login_password, password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        userauth_console_login_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningConsoleLoginPassword
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
UI32_T USERAUTH_GetRunningConsoleLoginPassword(char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(password, userauth_console_login_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (password[0] == 0)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetTelnetLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function gets telnet login password
 *  INPUT   : password
 *  OUTPUT  : password
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_GetTelnetLoginPassword(char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(password, userauth_telnet_login_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_SetTelnetLoginPassword
 * ---------------------------------------------------------------------
 *  FUNCTION: This function sets telnet login password
 *  INPUT   : password
 *  OUTPUT  : None
 *  RETURN  : BOOL_T True : Get successfully, False : Get failed
 * ---------------------------------------------------------------------
 *  NOTE    : None
 */
BOOL_T USERAUTH_SetTelnetLoginPassword(const char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(userauth_telnet_login_password, password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        userauth_telnet_login_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_GetRunningTelnetLoginPassword
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
UI32_T USERAUTH_GetRunningTelnetLoginPassword(char *password)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy(password, userauth_telnet_login_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN);
        password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (password[0] == 0)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}



#if (SYS_CPNT_AUTHENTICATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_LoginAuthPassByWhichMethod
 *-------------------------------------------------------------------------
 * PURPOSE  : to get the auth-method that USERAUTH_LoginAuth() used
 * INPUT    : none
 * OUTPUT   : auth_method
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if the return value of USERAUTH_LoginAuth() is FALSE (fail to auth),
 *            can't get auth_method (because it didn't pass authentication)
 *-------------------------------------------------------------------------*/
BOOL_T USERAUTH_LoginAuthPassByWhichMethod(USERAUTH_Auth_Method_T *auth_method)
{
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }

    USERAUTH_ENTER_CRITICAL_SECTION();

    if ((NULL == auth_method) || (USERAUTH_AUTH_NONE == userauth_loginauth_pass_method))
    {
        USERAUTH_RELEASE_CSC();
        USERAUTH_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    //*auth_method = userauth_loginauth_pass_method;
    memcpy(auth_method, &userauth_loginauth_pass_method, sizeof(USERAUTH_Auth_Method_T));

    USERAUTH_LEAVE_CRITICAL_SECTION();
    USERAUTH_RELEASE_CSC();
    return TRUE;
}

#endif /* SYS_CPNT_AUTHENTICATION == TRUE */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - USERAUTH_EnablePasswordAuth_Req
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
static USERAUTH_ReturnValue_T USERAUTH_EnablePasswordAuth_Req(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_Auth_Method_T      method;
    USERAUTH_ReturnValue_T      ret;
    char    *method_str_ary[]   =   USERAUTH_AUTH_METHOD_STRINGS;
    UI8_T   encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

    USERAUTH_LOG("name=%s, auth_method={%s, %s, %s}, curr_auth_index=%lu",
        param_p->name,
        method_str_ary[param_p->auth_method_list[0]],
        method_str_ary[param_p->auth_method_list[1]],
        method_str_ary[param_p->auth_method_list[2]],
        param_p->curr_auth_index);

    param_p->result.status = 0;
    param_p->result.privilege = 0;
    param_p->result.authen_by_whom = USERAUTH_AUTH_NONE;

    if (SYS_ADPT_MAX_LOGIN_PRIVILEGE < param_p->privilege)
    {
        USERAUTH_LOG("Wrong request privilege=%lu", param_p->privilege);
        return USERAUTH_REJECT;
    }

    for (;
         param_p->curr_auth_index < sizeof(param_p->auth_method_list)/sizeof(*param_p->auth_method_list);
         ++(param_p->curr_auth_index))
    {
        method = param_p->auth_method_list[ param_p->curr_auth_index ];
        param_p->result.authen_by_whom = method;

        USERAUTH_LOG("%s enable authentication", method_str_ary[method]);

        switch (method)
        {
            case USERAUTH_AUTH_LOCAL:
                if (param_p->sess_type == USERAUTH_SESSION_CONSOLE)
                {
                    if (userauth_console_login_method == USERAUTH_LOGIN_NO_LOGIN)
                    {
                        param_p->result.privilege = param_p->privilege;
                        return USERAUTH_PASS;
                    }
                }
                else if(param_p->sess_type == USERAUTH_SESSION_TELNET)
                {
                    if (userauth_telnet_login_method == USERAUTH_LOGIN_NO_LOGIN)
                    {
                        param_p->result.privilege = param_p->privilege;
                        return USERAUTH_PASS;
                    }
                }
                else
                {
                    USERAUTH_LOG("Unknown sess_type=%lu", param_p->sess_type);
                    return USERAUTH_REJECT;
                }

                if (userauth_privilege_password[param_p->privilege].status == USERAUTH_ENTRY_VALID)
                {
                    memset(encrypted_password, 0, sizeof(encrypted_password));
                    L_MD5_MDString(encrypted_password, (UI8_T*)param_p->password, strlen(param_p->password));

                    if (memcmp(encrypted_password, userauth_privilege_password[param_p->privilege].password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
                    {
                        param_p->result.privilege = param_p->privilege;
                        return USERAUTH_PASS;
                    }
                }

                return USERAUTH_REJECT;
#if (SYS_CPNT_RADIUS==TRUE)
            case USERAUTH_AUTH_RADIUS:
                 ret = USERAUTH_AsyncRadiusEnableAuth(param_p);

                 if (USERAUTH_PROCESSING == ret)
                 {
                    return ret;
                 }

                 break; /* other, go next */
#endif  /* #if (SYS_CPNT_RADIUS==TRUE) */
#if (SYS_CPNT_TACACS==TRUE)
            case USERAUTH_AUTH_TACACS:
                ret = USERAUTH_AsyncTacacsEnableAuth(param_p);

                if (USERAUTH_PROCESSING == ret)
                {
                    return ret;
                }

                break; /* other, go next */
#endif  /* #if (SYS_CPNT_TACACS==TRUE) */

            default:
                break;  /* go next */
         }

    }

    USERAUTH_LOG("No next, reject");
    return USERAUTH_REJECT;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_EnablePasswordAuth_Resp
 *-------------------------------------------------------------------------
 * PURPOSE  : This function gets privilege password based on specified
 *            privilege.
 * INPUT    : name             --  name of user trying to login
 *            password         --  password of the user
 *            sess_type        --  session type of user trying to login
 *            sess_id          --  (optional) CLI session ID. This parameter
 *                                 is required for authenticating user who
 *                                 login via CLI, telnet, or SSH only
 *            rem_ip_addr      --  (optional) caller IP address. This parameter
 *                                 is required for user login via telnet, SSH,
 *                                 or WEB
 *            cookie           --  caller command ID.
 * OUTPUT   : auth_result_p    --  authentication result
 * RETURN:  : TRUE  -- authentication successfully, or
 *            FALSE -- authentication fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_EnablePasswordAuth_Resp(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_Auth_Method_T      method;

    if (sizeof(param_p->auth_method_list)/sizeof(*param_p->auth_method_list) <= param_p->curr_auth_index)
    {
        USERAUTH_LOG("Wrong curr_auth_index(%lu)", param_p->curr_auth_index);
        return USERAUTH_REJECT;
    }

    method = param_p->auth_method_list[ param_p->curr_auth_index ];

    switch (method)
    {
#if (SYS_CPNT_RADIUS==TRUE)
        case USERAUTH_AUTH_RADIUS:
            if (OK_RC == param_p->result.status)
            {
                param_p->result.privilege = param_p->privilege;
                return USERAUTH_PASS;
            }
            else if (BADRESP_RC == param_p->result.status)
            {
                return USERAUTH_REJECT;
            }

            break;
#endif  /* #if (SYS_CPNT_RADIUS==TRUE) */
#if (SYS_CPNT_TACACS==TRUE)
        case USERAUTH_AUTH_TACACS:
            if (TACACS_AUTHENTICATION_SUCCESS == param_p->result.status)
            {
                param_p->result.privilege = param_p->privilege;
                return USERAUTH_PASS;
            }
            else if (TACACS_AUTHENTICATION_FAIL == param_p->result.status)
            {
                return USERAUTH_REJECT;
            }

            break;
#endif  /* #if (SYS_CPNT_TACACS==TRUE) */
        default:
            break;
    }

    param_p->curr_auth_index++;
    param_p->cmd = USERAUTH_ASYNC_ENABLE_AUTH_REQUEST;

    return USERAUTH_Dispatch(param_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_EnablePasswordAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : This function gets privilege password based on specified
 *            privilege.
 * INPUT    : name             --  name of user trying to login
 *            password         --  password of the user
 *            sess_type        --  session type of user trying to login
 *            sess_id          --  (optional) CLI session ID. This parameter
 *                                 is required for authenticating user who
 *                                 login via CLI, telnet, or SSH only
 *            rem_ip_addr      --  (optional) caller IP address. This parameter
 *                                 is required for user login via telnet, SSH,
 *                                 or WEB
 *            cookie           --  caller command ID.
 * OUTPUT   : auth_result_p    --  authentication result
 * RETURN:  : TRUE  -- authentication successfully, or
 *            FALSE -- authentication fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_EnablePasswordAuth(
    const char *name,
    const char *password,
    USERAUTH_SessionType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    UI32_T privilege,
    UI32_T cookie,
    USERAUTH_AuthResult_T *auth_result_p)
{
    USERAUTH_ReturnValue_T      ret;
    USERAUTH_AsyncAuthParam_T   param;
    char                        str_rem_ip_addr[L_INET_MAX_IPADDR_STR_LEN+1]={0};

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return USERAUTH_NOT_MASTER_MODE;
    }

    L_INET_InaddrToString((L_INET_Addr_T *) rem_ip_addr, str_rem_ip_addr, sizeof(str_rem_ip_addr));

    USERAUTH_LOG("name=%s, password=***, sess_type=%d, sess_id=%lu, "
                 "rem_ip_addr=%s, privilege=%lu, cookie=%lu",
                 name,
                 sess_type,
                 sess_id,
                 str_rem_ip_addr,
                 privilege,
                 cookie);

    param.cmd = USERAUTH_ASYNC_ENABLE_AUTH_REQUEST;
    USERAUTH_GetEnableAuthMethod(param.auth_method_list);
    param.curr_auth_index = 0;

    strncpy(param.name, name, sizeof(param.name)-1);
    param.name[sizeof(param.name)-1] = '\0';

    strncpy(param.password, password, sizeof(param.password)-1);
    param.password[sizeof(param.password)-1] = '\0';

    param.sess_type = sess_type;
    param.sess_id = sess_id;

    if (NULL != rem_ip_addr)
    {
        param.rem_ip_addr = *rem_ip_addr;
    }

    param.privilege = privilege;
    param.cookie = cookie;

    ret = USERAUTH_Dispatch(&param);

    auth_result_p->privilege = param.result.privilege;
    auth_result_p->authen_by_whom = param.result.authen_by_whom;

    USERAUTH_RELEASE_CSC();
    return ret;
}


/*-----------------------------------------------------------------------------
 * Functions copied from CLI
 *-----------------------------------------------------------------------------
 */

static USERAUTH_ReturnValue_T USERAUTH_Local(
    const char *username,
    const char *password,
    UI32_T *privilege)
{
    USERAUTH_LoginLocal_T login_user;
    UI8_T encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

    int nLen = 0;
    memset(&login_user, 0, sizeof(login_user));

   /* hyliao, 2004.08.17, Nessus Scan ID : 12201
      Too long basic authentication DoS
      Resolution : using strncpy instead of using strcpy ,
                   and checking the length of username.
    */

   if ( (nLen = strlen((char *)username)) > SYS_ADPT_MAX_USER_NAME_LEN )
                nLen = SYS_ADPT_MAX_USER_NAME_LEN;
   /*
   strcpy(login_user.username, username);
   */
   strncpy((char *)login_user.username, (char *)username,nLen);


   if (USERAUTH_GetLoginLocalUser(&login_user))
   {
      memset(encrypted_password, 0, sizeof(encrypted_password));
      L_MD5_MDString(encrypted_password, (UI8_T *)password, strlen(password));


      if(memcmp(login_user.password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
      {
         *privilege = login_user.privilege;
         return USERAUTH_PASS;
      }
      else
      {
         return USERAUTH_REJECT;
      }
   }

   return USERAUTH_USER_NOT_EXIST;
}

#if (SYS_CPNT_RADIUS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncRadiusLoginAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous login authentication request to RADIUS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncRadiusLoginAuth(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    BOOL_T ret;
    ret = RADIUS_PMGR_AsyncLoginAuth(param_p->name,
                                     param_p->password,
                                     param_p,
                                     sizeof(*param_p));

    if (FALSE == ret)
    {
        return USERAUTH_IPC_FAILED;
    }

    return USERAUTH_PROCESSING;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncRadiusEnableAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous enable authentication request to RADIUS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncRadiusEnableAuth(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    BOOL_T ret;
    ret = RADIUS_PMGR_AsyncEnablePasswordAuth(param_p->name,
                                              param_p->password,
                                              param_p,
                                              sizeof(*param_p));

    if (FALSE == ret)
    {
        return USERAUTH_IPC_FAILED;
    }

    return USERAUTH_PROCESSING;
}
#endif /* #if (SYS_CPNT_RADIUS == TRUE) */

/*-----------------------------------------------------------------------------
 * Functions added by JJ
 *-----------------------------------------------------------------------------
 */
#if (SYS_CPNT_TACACS==TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncTacacsLoginAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous login authentication request to TACACS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncTacacsLoginAuth(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    TACACS_SessType_T tac_sess_type;
    BOOL_T ret;

    tac_sess_type = (param_p->sess_type == USERAUTH_SESSION_CONSOLE) ? TACACS_SESS_TYPE_CONSOLE :
                    (param_p->sess_type == USERAUTH_SESSION_TELNET)  ? TACACS_SESS_TYPE_TELNET:
                    (param_p->sess_type == USERAUTH_SESSION_SSH)     ? TACACS_SESS_TYPE_SSH:
                    (param_p->sess_type == USERAUTH_SESSION_HTTP)    ? TACACS_SESS_TYPE_HTTP :
                    (param_p->sess_type == USERAUTH_SESSION_HTTPS)   ? TACACS_SESS_TYPE_HTTPS :
                                                                       TACACS_SESS_TYPE_UNKNOWN;

    ret = TACACS_PMGR_AnsyncLoginAuth(param_p->name,
                                      param_p->password,
                                      tac_sess_type,
                                      param_p->sess_id,
                                      &param_p->rem_ip_addr,
                                      param_p,
                                      sizeof(*param_p));

    if (FALSE == ret)
    {
        return USERAUTH_IPC_FAILED;
    }

    return USERAUTH_PROCESSING;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_AsyncTacacsEnableAuth
 *-------------------------------------------------------------------------
 * PURPOSE  : Send asynchronous enable authentication request to TACACS.
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PROCESSING - success;
 *            USERAUTH_IPC_FAILED - fail
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_AsyncTacacsEnableAuth(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    TACACS_SessType_T tac_sess_type;
    BOOL_T ret;

    tac_sess_type = (param_p->sess_type == USERAUTH_SESSION_CONSOLE) ? TACACS_SESS_TYPE_CONSOLE :
                    (param_p->sess_type == USERAUTH_SESSION_TELNET)  ? TACACS_SESS_TYPE_TELNET :
                    (param_p->sess_type == USERAUTH_SESSION_SSH)     ? TACACS_SESS_TYPE_SSH:
                    (param_p->sess_type == USERAUTH_SESSION_HTTP)    ? TACACS_SESS_TYPE_HTTP :
                    (param_p->sess_type == USERAUTH_SESSION_HTTPS)   ? TACACS_SESS_TYPE_HTTPS :
                                                                       TACACS_SESS_TYPE_UNKNOWN;
    ret = TACACS_PMGR_AnsyncEnablePasswordAuth(param_p->name,
                                               param_p->password,
                                               tac_sess_type,
                                               param_p->sess_id,
                                              &param_p->rem_ip_addr,
                                               param_p,
                                               sizeof(*param_p));

    if (FALSE == ret)
    {
        return USERAUTH_IPC_FAILED;
    }

    return USERAUTH_PROCESSING;
}
#endif  /* #if (SYS_CPNT_TACACS==TRUE) */

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
/* FUNCTION NAME:  USERAUTH_SetPasswordRecoveryActive
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
BOOL_T USERAUTH_SetPasswordRecoveryActive(BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        if( userauth_password_recovery.active == active )
        {
            USERAUTH_RELEASE_CSC();
            return TRUE;
        }
        USERAUTH_ENTER_CRITICAL_SECTION();
        userauth_password_recovery.active = active;
        ret = CFGDB_MGR_WriteSection(userauth_password_recovery_session_handler, &userauth_password_recovery);

        if (TRUE == ret && TRUE == active)
        {
            if( FALSE == CFGDB_MGR_Flush() )
            {
                ret = FALSE;
            }
        }

        USERAUTH_LEAVE_CRITICAL_SECTION();
        USERAUTH_RELEASE_CSC();
        return ret;
    }
}



/* FUNCTION NAME:  USERAUTH_GetPasswordRecoveryActive
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
BOOL_T USERAUTH_GetPasswordRecoveryActive(BOOL_T *active)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();
        *active = userauth_password_recovery.active;
        USERAUTH_LEAVE_CRITICAL_SECTION();
        USERAUTH_RELEASE_CSC();
        return TRUE;
    }
}

/* FUNCTION NAME:  USERAUTH_DoPasswordRecovery
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
BOOL_T USERAUTH_DoPasswordRecovery(UI8_T *password)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI8_T   digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1];

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        USERAUTH_SetLoginLocalUserStatus(SYS_DFLT_USERAUTH_ADMIN_USERNAME, USERAUTH_ENTRY_VALID);
        L_MD5_MDString(digest, password, strlen(password));
        digest[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN] = 0;
        USERAUTH_SetLoginLocalUserPassword(SYS_DFLT_USERAUTH_ADMIN_USERNAME, digest);
        USERAUTH_ENTER_CRITICAL_SECTION();
        userauth_password_recovery.active = FALSE;
        ret = CFGDB_MGR_WriteSection(userauth_password_recovery_session_handler, &userauth_password_recovery);
        USERAUTH_LEAVE_CRITICAL_SECTION();
        USERAUTH_RELEASE_CSC();
        return ret;
    }
}



/* FUNCTION NAME:  USERAUTH_SetPasswordRecoveryStatus
 * PURPOSE:
 *          This function set status of PasswordRecoveryEnabled.
 *
 * INPUT:
 *          BOOL_T  status  --  status of password recovery enabled.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          BOOL_T --  TRUE or FALSE.
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_SetPasswordRecoveryStatus(BOOL_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        if( userauth_password_recovery.status == status )
        {
            USERAUTH_RELEASE_CSC();
            return TRUE;
        }
        USERAUTH_ENTER_CRITICAL_SECTION();
        userauth_password_recovery.status = status;
        ret = CFGDB_MGR_WriteSection(userauth_password_recovery_session_handler, &userauth_password_recovery);
        USERAUTH_LEAVE_CRITICAL_SECTION();
        USERAUTH_RELEASE_CSC();
        return ret;
    }
}



/* FUNCTION NAME:  USERAUTH_GetPasswordRecoveryStatus
 * PURPOSE:
 *          This function get status of PasswordRecoveryEnabled.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *status --  status of password recovery enabled.
 *
 * RETURN:
 *          BOOL_T --  TRUE or FALSE.
 * NOTES:
 *          .
 */
BOOL_T USERAUTH_GetPasswordRecoveryStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();
        *status = userauth_password_recovery.status;
        USERAUTH_LEAVE_CRITICAL_SECTION();
        USERAUTH_RELEASE_CSC();
        return TRUE;
    }
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_PMGR_LoginAuthBySessionType_Req
 *-------------------------------------------------------------------------
 * PURPOSE:
 *          This function authenticate the user by session type who is trying
 *          to login and output the priviledge of this user if he/she is
 *          authenticated otherwise returns FALSE to indicate the failure of
 *          authentication
 *
 * INPUT:
 *          param_p          -- user authentication parameter
 *
 * OUTPUT:
 *          param_p          -- user authentication parameter
 *
 * RETURN:
 *          USERAUTH_PASS       - authentication accepted
 *          USERAUTH_REJECT     - authentication failed
 *          USERAUTH_PROCESSING - authencication is processing by remote
 *                                  server.
 *          Otherwise           - error code.
 * NOTES:
 *          None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_LoginAuthBySessionType_Req(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_Auth_Method_T      method;
    USERAUTH_ReturnValue_T      ret;
    char *method_str_ary[]   =   USERAUTH_AUTH_METHOD_STRINGS;

    USERAUTH_LOG("name=%s, auth_method={%s, %s, %s}, curr_auth_index=%lu",
        param_p->name,
        method_str_ary[param_p->auth_method_list[0]],
        method_str_ary[param_p->auth_method_list[1]],
        method_str_ary[param_p->auth_method_list[2]],
        param_p->curr_auth_index);

    param_p->result.status = 0;
    param_p->result.privilege = 0;
    param_p->result.authen_by_whom = USERAUTH_AUTH_NONE;

    for (;
         param_p->curr_auth_index < sizeof(param_p->auth_method_list)/sizeof(*param_p->auth_method_list);
         ++(param_p->curr_auth_index))
    {
        method = param_p->auth_method_list[ param_p->curr_auth_index ];
        param_p->result.authen_by_whom = method;

        USERAUTH_LOG("Do %s authentication", method_str_ary[method]);

        switch (method)
        {
            case USERAUTH_AUTH_LOCAL:

                /*
                 * For security issue, the super user just be used to login
                 * from console.
                 */
                if (param_p->sess_type != USERAUTH_SESSION_CONSOLE)
                {
                    if (strcmp(param_p->name, USERAUTH_SUPERUSER_USERNAME_DEF) == 0)
                    {
                        return USERAUTH_REJECT;
                    }
                }

                ret = USERAUTH_Local(param_p->name, param_p->password, &param_p->result.privilege);

                if (USERAUTH_PASS == ret || USERAUTH_REJECT == ret)
                {
                    return ret;
                }

                break; /* other, go next */
#if (SYS_CPNT_RADIUS==TRUE)
            case USERAUTH_AUTH_RADIUS:
                 ret = USERAUTH_AsyncRadiusLoginAuth(param_p);

                 if (USERAUTH_PROCESSING == ret)
                 {
                    return ret;
                 }

                 break; /* other, go next */
#endif  /* #if (SYS_CPNT_RADIUS==TRUE) */
#if (SYS_CPNT_TACACS==TRUE)
            case USERAUTH_AUTH_TACACS:
                ret = USERAUTH_AsyncTacacsLoginAuth(param_p);

                if (USERAUTH_PROCESSING == ret)
                {
                    return ret;
                }

                break; /* other, go next */
#endif  /* #if (SYS_CPNT_TACACS==TRUE) */

            default:
                break;  /* go next */
         }

    }

    USERAUTH_LOG("No next, reject");
    return USERAUTH_REJECT;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_LoginAuthBySessionType_Resp
 *-------------------------------------------------------------------------
 * PURPOSE:
 *          This function authenticate the user by session type who is trying
 *          to login and output the priviledge of this user if he/she is
 *          authenticated otherwise returns FALSE to indicate the failure of
 *          authentication
 *
 * INPUT:
 *          param_p          -- user authentication parameter
 *
 * OUTPUT:
 *          param_p          -- user authentication parameter
 *
 * RETURN:
 *          USERAUTH_PASS       - authentication accepted
 *          USERAUTH_REJECT     - authentication failed
 *          USERAUTH_PROCESSING - authencication is processing by remote
 *                                  server.
 *          Otherwise           - error code.
 * NOTES:
 *          None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_LoginAuthBySessionType_Resp(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_Auth_Method_T      method;

    if (sizeof(param_p->auth_method_list)/sizeof(*param_p->auth_method_list) <= param_p->curr_auth_index)
    {
        USERAUTH_LOG("Wrong curr_auth_index(%lu)", param_p->curr_auth_index);
        return USERAUTH_REJECT;
    }

    method = param_p->auth_method_list[ param_p->curr_auth_index ];

    switch (method)
    {
#if (SYS_CPNT_RADIUS==TRUE)
        case USERAUTH_AUTH_RADIUS:
            if (OK_RC == param_p->result.status)
            {
                return USERAUTH_PASS;
            }
            else if (BADRESP_RC == param_p->result.status)
            {
                return USERAUTH_REJECT;
            }

            break;
#endif  /* #if (SYS_CPNT_RADIUS==TRUE) */
#if (SYS_CPNT_TACACS==TRUE)
        case USERAUTH_AUTH_TACACS:
            if (TACACS_AUTHENTICATION_SUCCESS == param_p->result.status)
            {
                return USERAUTH_PASS;
            }
            else if (TACACS_AUTHENTICATION_FAIL == param_p->result.status)
            {
                return USERAUTH_REJECT;
            }

            break;
#endif  /* #if (SYS_CPNT_TACACS==TRUE) */
        default:
            break;
    }

    param_p->curr_auth_index++;
    param_p->cmd = USERAUTH_ASYNC_LOGIN_AUTH_REQUEST;

    return USERAUTH_Dispatch(param_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_PMGR_LoginAuthBySessionType
 *-------------------------------------------------------------------------
 * PURPOSE:   This function authenticate the user by session type who is trying
 *            to login and output the priviledge of this user if he/she is
 *            authenticated otherwise returns FALSE to indicate the failure of
 *            authentication
 * INPUT    : name             --  name of user trying to login
 *            password         --  password of the user
 *            sess_type        --  session type of user trying to login
 *            sess_id          --  (optional) CLI session ID. This parameter
 *                                 is required for authenticating user who
 *                                 login via CLI, telnet, or SSH only
 *            rem_ip_addr      --  (optional) caller IP address. This parameter
 *                                 is required for user login via telnet, SSH,
 *                                 or WEB
 *            cookie           --  caller command ID.
 * OUTPUT   : auth_result_p    --  authentication result
 * RETURN   : USERAUTH_PASS       - authentication accepted
 *            USERAUTH_REJECT     - authentication failed
 *            USERAUTH_PROCESSING - authencication is processing by remote
 *                                  server.
 *            Otherwise           - error code.
 * NOTES:
 *          None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_LoginAuthBySessionType(
    const char *name,
    const char *password,
    USERAUTH_SessionType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    UI32_T cookie,
    USERAUTH_AuthResult_T *auth_result_p)
{
    USERAUTH_ReturnValue_T      ret;
    USERAUTH_AsyncAuthParam_T   param;
    char                        str_rem_ip_addr[L_INET_MAX_IPADDR_STR_LEN+1]={0};

    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        USERAUTH_RELEASE_CSC();
        return USERAUTH_NOT_MASTER_MODE;
    }

    L_INET_InaddrToString((L_INET_Addr_T *) rem_ip_addr, str_rem_ip_addr, sizeof(str_rem_ip_addr));

    USERAUTH_LOG("name=%s, password=***, sess_type=%d, sess_id=%lu, "
                 "rem_ip_addr=%s, cookie=%lu",
                 name,
                 sess_type,
                 sess_id,
                 str_rem_ip_addr,
                 cookie);

    param.cmd = USERAUTH_ASYNC_LOGIN_AUTH_REQUEST;
    USERAUTH_GetAuthMethodBySessionType(param.auth_method_list, sess_type);
    param.curr_auth_index = 0;

    strncpy(param.name, name, sizeof(param.name)-1);
    param.name[sizeof(param.name)-1] = '\0';

    strncpy(param.password, password, sizeof(param.password)-1);
    param.password[sizeof(param.password)-1] = '\0';

    param.sess_type = sess_type;
    param.sess_id = sess_id;

    if (NULL != rem_ip_addr)
    {
        param.rem_ip_addr = *rem_ip_addr;
    }

    param.cookie = cookie;

    ret = USERAUTH_Dispatch(&param);

    auth_result_p->privilege = param.result.privilege;
    auth_result_p->authen_by_whom = param.result.authen_by_whom;
    auth_result_p->method = USERAUTH_PASSWORD;

    USERAUTH_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME:  USERAUTH_SetAuthMethodBySessionType
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
BOOL_T USERAUTH_SetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  i;

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        if( USERAUTH_SESSION_ALL != sess_type )
        {
            USERAUTH_ENTER_CRITICAL_SECTION();

            memcpy (userauth_auth_method[sess_type], auth_method,
                    sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

            USERAUTH_LEAVE_CRITICAL_SECTION();
        }
        else
        {
            for( i=0 ; i< USERAUTH_NUMBER_OF_SESSION_TYPE ; i++ )
            {
                USERAUTH_ENTER_CRITICAL_SECTION();

                memcpy (userauth_auth_method[i], auth_method,
                        sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

                USERAUTH_LEAVE_CRITICAL_SECTION();
            }
        }
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}



/* FUNCTION NAME:  USERAUTH_GetAuthMethodBySessionType
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
BOOL_T USERAUTH_GetAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();
        memcpy (auth_method, userauth_auth_method[sess_type],
                sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();
    }
    USERAUTH_RELEASE_CSC();
    return TRUE;
}



/* FUNCTION NAME:  USERAUTH_GetRunningAuthMethodBySessionType
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          authentication method is changed.  Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          UI32_T                  sess_type       --  session type
 *
 * OUTPUT:
 *          USERAUTH_Auth_Method_T  *auth_method    --  auth_method.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  USERAUTH_GetRunningAuthMethodBySessionType(USERAUTH_Auth_Method_T *auth_method, UI32_T sess_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        USERAUTH_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        USERAUTH_ENTER_CRITICAL_SECTION();

        memcpy (auth_method, userauth_auth_method[sess_type],
            sizeof(USERAUTH_Auth_Method_T) * USERAUTH_NUMBER_Of_AUTH_METHOD);

        USERAUTH_LEAVE_CRITICAL_SECTION();

        if (*auth_method == USERAUTH_AUTH_LOCAL &&
            *(auth_method+1) == USERAUTH_AUTH_NONE &&
            *(auth_method+2) == USERAUTH_AUTH_NONE)
        {
            USERAUTH_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    USERAUTH_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}



/* FUNCTION NAME - USERAUTH_INIT_HandleHotInsertion
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
void USERAUTH_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}



/* FUNCTION NAME - USERAUTH_INIT_HandleHotRemoval
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
void USERAUTH_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    USERAUTH_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}



/* FUNCTION NAME - USERAUTH_MGR_HandleHotInsertion
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
void USERAUTH_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME - USERAUTH_MGR_HandleHotRemoval
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
void USERAUTH_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME:  USERAUTH_BackdoorFunction
 * PURPOSE:
 *          Display back door available function and accept user seletion.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void USERAUTH_BackdoorFunction()
{
    UI8_T   keyin[10];
    I8_T    username[10];
    I8_T    password[10];
    UI32_T  priv = 0;
    USERAUTH_Auth_Method_T auth_method[USERAUTH_NUMBER_Of_AUTH_METHOD];
    BOOL_T  ret = FALSE;

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n1. USERAUTH_LoginAuthBySessionType.");
        BACKDOOR_MGR_Printf("\r\n2. USERAUTH_SetAuthMethodBySessionType.");
        BACKDOOR_MGR_Printf("\r\n3. USERAUTH_GetAuthMethodBySessionType.");
        BACKDOOR_MGR_Printf("\r\n0. Exit.\r\n");
        BACKDOOR_MGR_Printf("\r\nEnter your choice: ");

        *keyin = 0;
        BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
        BACKDOOR_MGR_Printf("\r\n");

        switch(*keyin)
        {
            case '0':
                return;

            case '1':
                BACKDOOR_MGR_Printf("\r\nInput username : ");
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 10);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy((char *)username, (char *)keyin);

                BACKDOOR_MGR_Printf("Input password : ");
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 10);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy((char *)password, (char *)keyin);

                BACKDOOR_MGR_Printf("Input session-type (1.console 2.telnet 3.web): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        /*ret = USERAUTH_LoginAuthBySessionType((UI8_T *)username, (UI8_T *)password, USERAUTH_SESSION_CONSOLE, &priv);*/  /*maggie liu for RADIUS authenticaiton ansync*/

                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("Authentication success, priv = %ld\n", priv);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("Authentication failure\n");
                        }
                        break;
                    case '2':
                        /*ret = USERAUTH_LoginAuthBySessionType((UI8_T *)username, (UI8_T *)password, USERAUTH_SESSION_TELNET, &priv);*/  /*maggie liu for RADIUS authenticaiton ansync*/

                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("Authentication success, priv = %ld\n", priv);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("Authentication failure\n");
                        }
                        break;
                    case '3':
                        /*ret = USERAUTH_LoginAuthBySessionType((UI8_T *)username, (UI8_T *)password, USERAUTH_SESSION_WEB, &priv);*/ /*maggie liu for RADIUS authenticaiton ansync*/

                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("Authentication success, priv = %ld\n", priv);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("Authentication failure\n");
                        }
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '2':
                BACKDOOR_MGR_Printf("\r\nInput first auth_method (0.none 1.local 2.radius 3.tacacs): ");
                memset(keyin, 0, 10);
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                auth_method[0] = atoi((char *)keyin);

                BACKDOOR_MGR_Printf("\r\nInput second auth_method (0.none 1.local 2.radius 3.tacacs): ");
                memset(keyin, 0, 10);
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                auth_method[1] = atoi((char *)keyin);

                BACKDOOR_MGR_Printf("\r\nInput third auth_method (0.none 1.local 2.radius 3.tacacs): ");
                memset(keyin, 0, 10);
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                auth_method[2] = atoi((char *)keyin);

                BACKDOOR_MGR_Printf("Input session-type (1.console 2.telnet 3.ssh 4.http 5.https): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_CONSOLE);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod success\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod failure\n");
                        }
                        break;
                    case '2':
                        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_TELNET);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod success\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod failure\n");
                        }
                        break;
                    case '3':
                        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_SSH);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod success\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod failure\n");
                        }
                        break;
                    case '4':
                        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_HTTP);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod success\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod failure\n");
                        }
                        break;
                    case '5':
                        ret = USERAUTH_SetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_HTTPS);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod success\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("set AuthMethod failure\n");
                        }
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '3':
                BACKDOOR_MGR_Printf("Input session-type (1.console 2.telnet 3.ssh 4.http 5.https): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_CONSOLE);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod success\n");
                            BACKDOOR_MGR_Printf("1st method = %d\n",auth_method[0]);
                            BACKDOOR_MGR_Printf("2nd method = %d\n",auth_method[1]);
                            BACKDOOR_MGR_Printf("3rd method = %d\n",auth_method[2]);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod failure\n");
                        }
                        break;
                    case '2':
                        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_TELNET);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod success\n");
                            BACKDOOR_MGR_Printf("1st method = %d\n",auth_method[0]);
                            BACKDOOR_MGR_Printf("2nd method = %d\n",auth_method[1]);
                            BACKDOOR_MGR_Printf("3rd method = %d\n",auth_method[2]);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod failure\n");
                        }
                        break;
                    case '3':
                        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_SSH);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod success\n");
                            BACKDOOR_MGR_Printf("1st method = %d\n",auth_method[0]);
                            BACKDOOR_MGR_Printf("2nd method = %d\n",auth_method[1]);
                            BACKDOOR_MGR_Printf("3rd method = %d\n",auth_method[2]);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod failure\n");
                        }
                        break;
                    case '4':
                        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_HTTP);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod success\n");
                            BACKDOOR_MGR_Printf("1st method = %d\n",auth_method[0]);
                            BACKDOOR_MGR_Printf("2nd method = %d\n",auth_method[1]);
                            BACKDOOR_MGR_Printf("3rd method = %d\n",auth_method[2]);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod failure\n");
                        }
                        break;
                    case '5':
                        ret = USERAUTH_GetAuthMethodBySessionType(auth_method, USERAUTH_SESSION_HTTPS);
                        if( ret == TRUE )
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod success\n");
                            BACKDOOR_MGR_Printf("1st method = %d\n",auth_method[0]);
                            BACKDOOR_MGR_Printf("2nd method = %d\n",auth_method[1]);
                            BACKDOOR_MGR_Printf("3rd method = %d\n",auth_method[2]);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("get AuthMethod failure\n");
                        }
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            default:
                continue;
      }
      BACKDOOR_MGR_Printf("\r\n\r\n\r\n\r\n");
      BACKDOOR_MGR_Printf("---------------------------\r\n");
   }

}


/* FUNCTION NAME:  USERAUTH_ProcessAuthResult
 * PURPOSE:
 *          Process authentication result from system callback.
 *
 * INPUT:
 *          param_p -- userauth parameter.
 *
 * OUTPUT:
 *          None.
 *
 * RETURN:
 *          If USERAUTH_PROCESSING is returned, .... Otherwise, send a
 *          response to orig authentication function.
 * NOTES:
 *          .
 */
USERAUTH_ReturnValue_T USERAUTH_ProcessAuthResult(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_ReturnValue_T    ret = USERAUTH_REJECT;

    USERAUTH_LOG("Received auth result");

    /* Reply by orig command
     */
    switch (param_p->cmd)
    {
        case USERAUTH_ASYNC_LOGIN_AUTH_REQUEST:
            param_p->cmd = USERAUTH_ASYNC_LOGIN_AUTH_RESPONSE;
            break;

        case USERAUTH_ASYNC_ENABLE_AUTH_REQUEST:
            param_p->cmd = USERAUTH_ASYNC_ENABLE_AUTH_RESPONSE;
            break;

        default:
            USERAUTH_LOG("Wrong command(%d)", param_p->cmd);
            return USERAUTH_WRONG_COMMAND;
    }

    ret = USERAUTH_Dispatch(param_p);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  USERAUTH_Dispatch
 *-------------------------------------------------------------------------
 * PURPOSE  : Dispatch function used for processing asynchronous request
 * INPUT    : param_p   - user auth parameter
 * OUTPUT   : None
 * RETURN   : USERAUTH_PASS       - authentication accepted
 *            USERAUTH_REJECT     - authentication failed
 *            USERAUTH_PROCESSING - authencication is processing by remote
 *                                  server.
 *            Otherwise           - error code.
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static USERAUTH_ReturnValue_T USERAUTH_Dispatch(
    USERAUTH_AsyncAuthParam_T *param_p)
{
    USERAUTH_ReturnValue_T ret = USERAUTH_REJECT;

    switch (param_p->cmd)
    {
        case USERAUTH_ASYNC_LOGIN_AUTH_REQUEST:
            ret = USERAUTH_LoginAuthBySessionType_Req(param_p);
            USERAUTH_SendAuthenticationTrap(param_p, ret);
            break;

        case USERAUTH_ASYNC_LOGIN_AUTH_RESPONSE:
            ret = USERAUTH_LoginAuthBySessionType_Resp(param_p);
            USERAUTH_SendAuthenticationTrap(param_p, ret);
            break;

        case USERAUTH_ASYNC_ENABLE_AUTH_REQUEST:
            ret = USERAUTH_EnablePasswordAuth_Req(param_p);
            break;

        case USERAUTH_ASYNC_ENABLE_AUTH_RESPONSE:
            ret = USERAUTH_EnablePasswordAuth_Resp(param_p);
            break;

        default:
            return USERAUTH_WRONG_COMMAND;
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - USERAUTH_SendAuthenticationTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send authentication trap
 * INPUT   : param_p  -- user auth parameter
 *           ret      -- authentication retrun value
 *                       USERAUTH_PASS:
 *                       authentication accepted
 *
 *                       USERAUTH_REJECT:
 *                       authentication failed
 *
 *                       USERAUTH_PROCESSING:
 *                       authencication is processing by remote server
 *
 *                       Otherwise:
 *                       error code
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
USERAUTH_SendAuthenticationTrap(
    USERAUTH_AsyncAuthParam_T *param_p,
    USERAUTH_ReturnValue_T ret)
{
    TRAP_EVENT_TrapData_T data;
    L_INET_AddrIp_T  nexthop_addr;

    memset(&data, 0, sizeof(data));
    data.community_specified = FALSE;

    switch (ret)
    {
        case USERAUTH_PASS:
            data.trap_type = TRAP_EVENT_USERAUTH_AUTHENTICATION_SUCCESS;
            break;

        case USERAUTH_REJECT:
            data.trap_type = TRAP_EVENT_USERAUTH_AUTHENTICATION_FAILURE;
            break;

        case USERAUTH_PROCESSING:
        default:
            return FALSE;
    }

    /* get user mac
     */
    if (NETCFG_TYPE_FAIL == NETCFG_PMGR_ROUTE_GetReversePathIpMac(&param_p->rem_ip_addr,
        &nexthop_addr, data.u.user_info.user_mac))
    {
        return FALSE;
    }

    strncpy(data.u.user_info.user_name, param_p->name,
        sizeof(data.u.user_info.user_name)-1);
    data.u.user_info.user_name[sizeof(data.u.user_info.user_name)-1] = '\0';
    data.u.user_info.session_type = param_p->sess_type;
    memcpy(&data.u.user_info.user_ip, &param_p->rem_ip_addr,
        sizeof(data.u.user_info.user_ip));

    SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    return TRUE;
}/* End of USERAUTH_SendAuthenticationTrap */

/*------------------------------------------------------------------------
 * ROUTINE NAME - USERAUTH_SendUserAccountTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send user account trap
 * INPUT   : user - Information of account user.
 *           trap_type - The type of trap you want to send.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
USERAUTH_SendUserAccountTrap(
    USERAUTH_LoginLocal_T *user,
    UI32_T trap_type)
{
    TRAP_EVENT_TrapData_T  data;

    memset(&data, 0, sizeof(data));
    data.trap_type = trap_type;
    data.community_specified = FALSE;

    strncpy(data.u.userauth_account.user_name,
            (char *)user->username,
            sizeof(data.u.userauth_account.user_name) - 1);
    data.u.userauth_account.privilege = user->privilege;

    SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);

    return TRUE;
}/* End of USERAUTH_SendUserAccountTrap */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : USERAUTH_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
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
BOOL_T USERAUTH_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    USERAUTH_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p = (USERAUTH_IPCMsg_T *)ipcmsg_p->msg_buf;
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
         *Modify file:userauth.c
         *Approved by:Hardsun
         */
        ipcmsg_p->msg_size = USERAUTH_MSGBUF_TYPE_SIZE;
        msg_data_p->type.result_ui32 = 0;
        goto exit;
    }

    switch(cmd)
    {

        case USERAUTH_IPCCMD_GETNEXTSNMPCOMMUNITY:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_entry)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetNextSnmpCommunity(
                &msg_data_p->data.comm_entry);
        }
        break;

        case USERAUTH_IPCCMD_GETNEXTRUNNINGSNMPCOMMUNITY:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.comm_entry)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetNextRunningSnmpCommunity(
                &msg_data_p->data.comm_entry);
        }
        break;

        case USERAUTH_IPCCMD_SETSNMPCOMMUNITYACCESSRIGHT:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetSnmpCommunityAccessRight(
                msg_data_p->data.comm_access.comm_string_name,
                msg_data_p->data.comm_access.access_right);
        }
        break;

        case USERAUTH_IPCCMD_SETSNMPCOMMUNITYSTATUS:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetSnmpCommunityStatus(
                msg_data_p->data.comm_status.comm_string_name,
                msg_data_p->data.comm_status.status);
        }
        break;

        case USERAUTH_IPCCMD_GETLOGINLOCALUSER:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetLoginLocalUser(
                &msg_data_p->data.login_user);
        }
        break;

        case USERAUTH_IPCCMD_GETNEXTLOGINLOCALUSER:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetNextLoginLocalUser(
                &msg_data_p->data.login_user);
        }
        break;

        case USERAUTH_IPCCMD_GETNEXTRUNNINGLOGINLOCALUSER:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_user)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetNextRunningLoginLocalUser(
                &msg_data_p->data.login_user);
        }
        break;

        case USERAUTH_IPCCMD_SETLOGINLOCALUSERSTATUS:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetLoginLocalUserStatus(
                msg_data_p->data.user_status.username,
                msg_data_p->data.user_status.status);
        }
        break;

        case USERAUTH_IPCCMD_SETLOGINLOCALUSERPASSWORD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetLoginLocalUserPassword(
                msg_data_p->data.user_password.username,
                msg_data_p->data.user_password.password);
        }
        break;

        case USERAUTH_IPCCMD_SETLOGINLOCALUSERPRIVILEGE:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetLoginLocalUserPrivilege(
                msg_data_p->data.user_privilege.username,
                msg_data_p->data.user_privilege.privilege);
        }
        break;

        case USERAUTH_IPCCMD_GETPRIVILEGEPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetPrivilegePassword(
                &msg_data_p->data.privilege_password);
        }
        break;

        case USERAUTH_IPCCMD_GETFIRSTRUNNINGPRIVILEGEPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetFirstRunningPrivilegePassword(
                &msg_data_p->data.privilege_password);
        }
        break;

        case USERAUTH_IPCCMD_GETNEXTRUNNINGPRIVILEGEPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.privilege_password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetNextRunningPrivilegePassword(
                &msg_data_p->data.privilege_password);
        }
        break;

        case USERAUTH_IPCCMD_SETPRIVILEGEPASSWORDSTATUS:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetPrivilegePasswordStatus(
                msg_data_p->data.ui32a1_ui32a2.ui32_a1,
                msg_data_p->data.ui32a1_ui32a2.ui32_a2);
        }
        break;

        case USERAUTH_IPCCMD_SETPRIVILEGEPASSWORD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetPrivilegePassword(
                msg_data_p->data.privilege_password.privilege,
                msg_data_p->data.privilege_password.password);
        }
        break;

        case USERAUTH_IPCCMD_GETAUTHMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
            + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_SETAUTHMETHOD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGAUTHMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_GETENABLEAUTHMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetEnableAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_SETENABLEAUTHMETHOD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetEnableAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGENABLEAUTHMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method_numbers)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningEnableAuthMethod(
                msg_data_p->data.auth_method_numbers);
        }
        break;

        case USERAUTH_IPCCMD_GETTELNETLOGINMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetTelnetLoginMethod(
                &msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_SETTELNETLOGINMETHOD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetTelnetLoginMethod(
                msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGTELNETLOGINMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningTelnetLoginMethod(
                &msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_GETCONSOLLOGINMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetConsolLoginMethod(
                &msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_SETCONSOLELOGINMETHOD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetConsoleLoginMethod(
                msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGCONSOLELOGINMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.login_method)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningConsoleLoginMethod(
                &msg_data_p->data.login_method);
        }
        break;

        case USERAUTH_IPCCMD_GETCONSOLELOGINPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetConsoleLoginPassword(
                msg_data_p->data.password);
        }
        break;

        case USERAUTH_IPCCMD_SETCONSOLELOGINPASSWORD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetConsoleLoginPassword(
                msg_data_p->data.password);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGCONSOLELOGINPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningConsoleLoginPassword(
                msg_data_p->data.password);
        }
        break;

        case USERAUTH_IPCCMD_GETTELNETLOGINPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetTelnetLoginPassword(
                msg_data_p->data.password);
        }
        break;

        case USERAUTH_IPCCMD_SETTELNETLOGINPASSWORD:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetTelnetLoginPassword(
                msg_data_p->data.password);
        }
        break;

        case USERAUTH_IPCCMD_GETRUNNINGTELNETLOGINPASSWORD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.password)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32=USERAUTH_GetRunningTelnetLoginPassword(
                msg_data_p->data.password);
        }
        break;
#if (SYS_CPNT_AUTHENTICATION == TRUE)
         case USERAUTH_IPCCMD_LOGINAUTHPASSBYWHICHMETHOD:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_method)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_LoginAuthPassByWhichMethod(
                &msg_data_p->data.auth_method);
        }
        break;
#endif  /* #if (SYS_CPNT_AUTHENTICATION==TRUE) */

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
        case USERAUTH_IPCCMD_SETPASSWORDRECOVERYACTIVE:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetPasswordRecoveryActive(
                msg_data_p->data.bool_v);
        }
        break;

        case USERAUTH_IPCCMD_GETPASSWORDRECOVERYACTIVE:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.bool_v)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetPasswordRecoveryActive(
                &msg_data_p->data.bool_v);
        }
        break;

        case USERAUTH_IPCCMD_DOPASSWORDRECOVERY:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_DoPasswordRecovery(
                &msg_data_p->data.password);
        }
        break;
#endif

        case USERAUTH_IPCCMD_SETAUTHMETHODBYSESSIONTYPE:
        {
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool=USERAUTH_SetAuthMethodBySessionType(
                msg_data_p->data.method_type.auth_method_numbers,
                msg_data_p->data.method_type.sess_type);
        }
        break;

        case USERAUTH_IPCCMD_GETAUTHMETHODBYSESSIONTYPE:
        {
            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->data.method_type)
                + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_bool=USERAUTH_GetAuthMethodBySessionType(
                msg_data_p->data.method_type.auth_method_numbers,
                msg_data_p->data.method_type.sess_type);
        }
        break;

        case USERAUTH_IPCCMD_LOGIN_AUTH_BY_SESSION_TYPE:
        {
#define req     data.login_auth.req
#define resp    data.login_auth.resp

            USERAUTH_LOG("Received asnyc login auth");

            USERAUTH_ReturnValue_T ret;
            USERAUTH_AuthResult_T auth_result = {0};

            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->resp)
                + USERAUTH_MSGBUF_TYPE_SIZE);

            ret = USERAUTH_LoginAuthBySessionType(
                            msg_data_p->req.name,
                            msg_data_p->req.password,
                            msg_data_p->req.sess_type,
                            msg_data_p->req.sess_id,
                           &msg_data_p->req.rem_ip_addr,
                            ipcmsg_p->msg_type,
                            &auth_result);

            if (USERAUTH_PROCESSING == ret)
            {
                USERAUTH_LOG("Wait remote server");
                return FALSE; /* No response */
            }

            msg_data_p->resp.result = auth_result;
            msg_data_p->type.result_bool = (USERAUTH_PASS == ret) ? TRUE : FALSE;
            break;
#undef req
#undef resp
        }

        case USERAUTH_IPCCMD_ENABLE_PASSWORD_AUTH:
        {
#define req     data.enable_auth.req
#define resp    data.enable_auth.resp

            USERAUTH_LOG("Received asnyc enable auth");

            USERAUTH_ReturnValue_T ret;
            USERAUTH_AuthResult_T auth_result = {0};

            ipcmsg_p->msg_size=(sizeof(((USERAUTH_IPCMsg_T *)0)->resp)
                + USERAUTH_MSGBUF_TYPE_SIZE);

            ret = USERAUTH_EnablePasswordAuth(
                            msg_data_p->req.name,
                            msg_data_p->req.password,
                            msg_data_p->req.sess_type,
                            msg_data_p->req.sess_id,
                           &msg_data_p->req.rem_ip_addr,
                            msg_data_p->req.privilege,
                            ipcmsg_p->msg_type,
                            &auth_result);

            if (USERAUTH_PROCESSING == ret)
            {
                USERAUTH_LOG("Wait remote server");
                return FALSE; /* No response */
            }

            msg_data_p->resp.result = auth_result;
            msg_data_p->type.result_bool = (USERAUTH_PASS == ret) ? TRUE : FALSE;

            break;
#undef req
#undef resp
        }

        case USERAUTH_IPCCMD_GETRUNNINGALLLOGINLOCALUSER:
        {
            ipcmsg_p->msg_size = (sizeof((USERAUTH_IPCMsg_T *)0)->data.auth_account_data
                                        + USERAUTH_MSGBUF_TYPE_SIZE);
            msg_data_p->type.result_ui32 = USERAUTH_GetRunningAllLoginLocalUser(
                msg_data_p->data.auth_account_data.alluser_data_ar, &msg_data_p->data.auth_account_data.user_count);
            break;
        }

        default:
            ipcmsg_p->msg_size=USERAUTH_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd<USERAUTH_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    return TRUE;
}

