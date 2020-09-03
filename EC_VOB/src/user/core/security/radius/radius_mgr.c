/* Project Name: New Feature
 * Module Name : Radius_mgr.C
 * Abstract    : This file is used to change the operation in Radiustask
 * Purpose     : to change the Radius opearion mode
 *
 * 2001/11/22    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <errno.h>
#include "radius_mgr.h"

#include "unistd.h"
#define s_close(fd) close(fd)

#include "sys_cpnt.h"
#include "sys_time.h"
#include "sysfun.h"

#if (SYS_CPNT_EH == TRUE)
#include "syslog_type.h"
#include "syslog_om.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif
#include "mib2_pom.h"
#include "radiusclient.h"
#include "sys_module.h"
/*isiah.2003-06-02.move secret key to CFGDB*/
#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
#include "cfgdb_mgr.h"
#endif
#include "radius_om.h"

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    #include "syslog_type.h"
    #include "syslog_pmgr.h"
    #include "sys_module.h"
    #include "aaa_mgr.h"
    #include "aaa_om.h"
    #include "swdrv_type.h"
    #include "nmtr_pmgr.h"
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#include "sys_callback_mgr.h"
#include "l_stdlib.h"
#include "ip_lib.h"
#include "auth_protocol_group.h"

#define RADIUS_MGR_DEBUG_MODE
/* MACRO FUNCTION DECLARATIONS
 */
#ifdef RADIUS_MGR_DEBUG_MODE
#ifndef _MSC_VER
#define RADIUS_MGR_TRACE(fmt, args...)                              \
    {                                                               \
        if (SECURITY_BACKDOOR_IsOn(radius_mgr_backdoor_reg_no))     \
        {                                                           \
            printf("[%s] "fmt"\r\n", __FUNCTION__, ##args);         \
        }                                                           \
    }
#else
#define RADIUS_MGR_TRACE(fmt, ...)                                  \
    {                                                               \
        if (SECURITY_BACKDOOR_IsOn(radius_mgr_backdoor_reg_no))     \
        {                                                           \
            printf("[%s] "fmt"\r\n", __FUNCTION__, ##__VA_ARGS__);  \
        }                                                           \
    }
#endif
#else
#define RADIUS_MGR_TRACE(fmt, args...)        ((void)0)
#endif

#ifndef ASSERT
#define ASSERT(eq)
#endif /* ASSERT */

#define RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL) \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
       return (RET_VAL); \
    }

#define RADIUS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE() \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
        return; \
    }

#define RADIUS_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) { \
        return (RET_VAL); \
    }

#define RADIUS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() { \
        return; \
    }

#define RADIUS_MGR_EnterCriticalSection()    //SYSFUN_OM_ENTER_CRITICAL_SECTION()
#define RADIUS_MGR_LeaveCriticalSection()    //SYSFUN_OM_LEAVE_CRITICAL_SECTION()
#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
#define CFGDB_MGR_SECTION_ID_RADIUS_SECRET_KEY  CFGDB_MGR_SECTION_ID_RADIUS_1
#endif


#define RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_VERSION       3
#define RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_MGMT          2
#define RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_POLICY        6

#define FILTER_ID_SEPARATOR_1         ':'
/*#define FILTER_ID_SEPARATOR_2         ';'*/

#define RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_ETS        "enterasys"
#define RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_VERSION    "version"
#define RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_MGMT       "mgmt"
#define RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_POLICY     "policy"

#define RADIUS_MAX_LEN_OF_FILTER_ID                         255
#define RADIUS_MAX_LEN_OF_FILTER_ID_TOKEN                   31

/* DATA TYPE DECLARATIONS
 */

typedef struct RADIUS_AttrFilterIdBitmap_S
{
    UI8_T   enterasys_existed:1;        /* whether the reserved word exist or not */
    UI8_T   version_existed:1;          /* whether the reserved word exist or not */
    UI8_T   mgmt_existed:1;             /* whether the reserved word exist or not */
    UI8_T   policy_existed:1;           /* whether the reserved word exist or not */

    UI8_T   undecorated_filter_id:1;    /* if a returned Filter-ID with a single word, which is decoded as the policy name */

    UI8_T   reserved_bits:3;
} RADIUS_AttrFilterIdBitmap_T;

typedef struct RADIUS_AttrFilterId_S
{
    RADIUS_AttrFilterIdBitmap_T attr_bitmap;

    UI8_T   version[RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_VERSION + 1];
    UI8_T   mgmt[RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_MGMT + 1];
    UI8_T   policy[RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_POLICY + 1];

    UI8_T   recognized_counter;     /* counter of recognozed segment */
    UI8_T   malformed_counter;      /* counter of malformed segment */
} RADIUS_AttrFilterId_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static RADIUS_ReturnValue_T RADIUS_MGR_InitRequestServerArray(
    UI32_T request_index);
static RADIUS_ReturnValue_T RADIUS_MGR_FindNextExistServerHost(
    UI32_T request_id);
static RADIUS_ReturnValue_T RADIUS_MGR_AnnounceResult(UI32_T request_index,
    UI32_T result);
static BOOL_T RADIUS_MGR_IsSuccessCodeByRequestType(
    RADIUS_OM_RadiusRequestType_T type, UI32_T code);
static RADIUS_ReturnValue_T RADIUS_MGR_OpenIdQueueSocket(
    RADIUS_OM_RadiusRequestType_T type, int *socket_id_p);
static RADIUS_ReturnValue_T RADIUS_MGR_CloseIdQueueSocket(
    RADIUS_OM_RadiusRequestType_T type);
static RADIUS_ReturnValue_T RADIUS_MGR_AddRequestIntoIdQueue(
    RADIUS_OM_RadiusRequestType_T type, UI32_T request_index,
    UI32_T *identifier_p);
static RADIUS_ReturnValue_T RADIUS_MGR_DeleteRequestFromIdQueue(
    RADIUS_OM_RadiusRequestType_T type, UI32_T identifier);
static RADIUS_ReturnValue_T RADIUS_MGR_RunRequestStateMachineOneStep(
    UI32_T request_index, BOOL_T *ret_is_state_changed_p,
    RADIUS_OM_RequestState_T *ret_new_state_p);
static void RADIUS_MGR_IncreaseAccessCounterByCode(UI32_T server_index,
    UI32_T code);
static void RADIUS_MGR_IncreasePendingRequestCounter(UI32_T server_index);
static void RADIUS_MGR_DecreasePendingRequestCounter(UI32_T server_index);
static void RADIUS_MGR_IncreaseErrorCounterByErrorType(UI32_T server_index,
    BOOL_T is_auth_request, RADIUSCLIENT_ResponsePacketErrorType_T error);
static BOOL_T RADIUS_MGR_ReceivePacketFromSocket(int socket_id,
    UI8_T **packet_pp, UI32_T *packet_len_p);
static BOOL_T RADIUS_MGR_ProcessReceivedPacket(UI8_T *recv_packet_p,
    UI32_T recv_packet_len,
    RADIUS_ReturnValue_T(*get_id_from_id_queue_fn_p)(UI32_T id, UI32_T *request_index_p));
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static RADIUS_ReturnValue_T RADIUS_MGR_CreateAccountingRequest(
    RADACC_AcctStatusType_T request_type, UI32_T user_index,
    UI32_T current_time, BOOL_T is_wait_for_response, UI32_T *request_index_p);
static RADIUS_ReturnValue_T RADIUS_MGR_AnnounceAccountingResult(
    UI32_T request_index, UI32_T result);
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
static BOOL_T RADIUS_MGR_CloseAuthReqIdSocket();
static BOOL_T RADIUS_MGR_CloseAcctReqIdSocket();
static BOOL_T RADIUS_MGR_IsValidIP(UI32_T serverip);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static BOOL_T RADIUS_MGR_AsyncAccountingRequest_Callback(const AAA_AccRequest_T *request);
static void RADIUS_MGR_LocalStopAndFreeAllAccUsers(AAA_AccTerminateCause_T reason);
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/* STATIC VARIABLE DECLARATIONS
 */

/*static SYS_TYPE_Stacking_Mode_T radius_operation_mode;*/ /*marked by Charles*/
/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

/*isiah.2003-06-02.move secret key to CFGDB*/
#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
static UI32_T   radius_secret_key_session_handler;
#endif

#ifdef RADIUS_MGR_DEBUG_MODE
static UI32_T   radius_mgr_backdoor_reg_no;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

BOOL_T RADIUS_MGR_InitiateProcessResources(void)
{
    BOOL_T result;
    result = RADIUS_OM_CreatSem();
    RADIUS_MGR_SetConfigSettingToDefault();

#ifdef RADIUS_MGR_DEBUG_MODE
    SECURITY_BACKDOOR_Register("radius_mgr", &radius_mgr_backdoor_reg_no);
#endif

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    AAA_MGR_Register_AccComponent_Callback(AAA_ACC_CPNT_RADIUS, RADIUS_MGR_AsyncAccountingRequest_Callback);
    NMTR_PMGR_InitiateProcessResource();
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    return result;
} /* End of RADIUS_MGR_InitiateProcessResources */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE:  This function initializes all function pointer registration operations.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Create_InterCSC_Relation(void)
{
    return;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetDebugFlag()
{
    return RADIUS_OM_GetDebugFlag();
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_SetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_SetDebugFlag(UI32_T flag)
{
    RADIUS_OM_SetDebugFlag(flag);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_CurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of Radius's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Radius_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */

SYS_TYPE_Stacking_Mode_T RADIUS_MGR_CurrentOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();  /*added by Charles*/
    /*return radius_operation_mode;*/ /*marked by Charles*/
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_EnterMasterMode()
{
    /*isiah.2003-06-02.move secret key to CFGDB*/
#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
    UI8_T   *auth_secret;
    UI8_T   default_secret[MAXSIZE_radiusServerGlobalKey + 1] = {0};
    BOOL_T  is_default_secret_need_to_sync;
#endif

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    /* call RADIUS_OM_AccIncSessionIdBootPart() before call RADIUS_OM_SetConfigSettingToDefault()
       then session id(boot part) can be counted from 0
     */
    RADIUS_OM_AccIncSessionIdBootPart();
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

   /* RADIUS_MGR_SetConfigSettingToDefault();*/
    /* radius_operation_mode = SYS_TYPE_STACKING_MASTER_MODE; */ /*marked by Charles*/
     /* set mgr in master mode */

    /*isiah.2003-06-02.move secret key to CFGDB*/
#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
    if( CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_RADIUS_SECRET_KEY,
                   sizeof(UI8_T),
                   MAXSIZE_radiusServerGlobalKey + 1,
                   &radius_secret_key_session_handler,
                   CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                   &is_default_secret_need_to_sync) == TRUE )
    {
        if( TRUE == is_default_secret_need_to_sync )
        {
            CFGDB_MGR_SyncSection(radius_secret_key_session_handler, (void *)default_secret);
            auth_secret = RADIUS_OM_Get_Server_Secret();
            memcpy(auth_secret, default_secret, MAXSIZE_radiusServerGlobalKey + 1);
        }
        else
        {
            auth_secret = RADIUS_OM_Get_Server_Secret();
            CFGDB_MGR_ReadSection(radius_secret_key_session_handler, auth_secret);
        }
    }
#endif

    SYSFUN_ENTER_MASTER_MODE();
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_EnterSlaveMode()
{
   SYSFUN_ENTER_SLAVE_MODE();
   /*radius_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;*/  /*marked by Charles*/
}

/* FUNCTION NAME : RADIUS_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void RADIUS_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */

    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}   /*  end of RADIUS_MGR_SetTransitionMode */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_EnterTransitionMode()
{
   /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();
    RADIUS_MGR_CloseAuthReqIdSocket();
    RADIUS_MGR_CloseAcctReqIdSocket();
    RADIUS_OM_SetConfigSettingToDefault();
   /*radius_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE; */   /*marked by Charles*/
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SetConfigSettingToDefault
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to RADIUS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        RADIUS_MGR_EnterMasterMode()
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetConfigSettingToDefault()
{
   return RADIUS_OM_SetConfigSettingToDefault();
}

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningRequestTimeout(UI32_T *timeout)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningRequestTimeout(timeout);
    return ret;
}
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_Get_Request_Timeout(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_Get_Request_Timeout();
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Set_Request_Timeout( UI32_T timeval )
{
    BOOL_T ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_Set_Request_Timeout(timeval);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningServerPort(UI32_T *serverport)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningServerPort(serverport);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_Get_Server_Port(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_Get_Server_Port();
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Set_Server_Port(UI32_T serverport)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_Set_Server_Port(serverport);
    return ret;
}



#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetRunningServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default RADIUS accounting port is successfully retrieved.
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
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningServerAcctPort(UI32_T *acct_port)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningServerAcctPort(acct_port);
    return ret;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetServerAcctPort(UI32_T *acct_port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        *acct_port = RADIUS_OM_GetServerAcctPort();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetServerAcctPort(UI32_T acct_port)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret = RADIUS_OM_SetServerAcctPort(acct_port);
    return ret;
}

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */




/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerSecret
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_MGR_GetRunningServerSecret(UI8_T serversecret[])
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
         return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningServerSecret(serversecret);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret text string pointer
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_MGR_Get_Server_Secret(void)
{
    UI8_T *ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
         return NULL;
    else
        ret =  RADIUS_OM_Get_Server_Secret();
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Set_Server_Secret( UI8_T * serversecret )
{
    BOOL_T ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_Set_Server_Secret(serversecret);

#if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE)
    ret &= CFGDB_MGR_WriteSection(radius_secret_key_session_handler, serversecret);
#endif /* #if (SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB == TRUE) */

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningRetransmitTimes
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningRetransmitTimes(UI32_T *retimes)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningRetransmitTimes(retimes);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_Get_Retransmit_Times(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_Get_Retransmit_Times();
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Set_Retransmit_Times(UI32_T retryval)
{
    BOOL_T ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_Set_Retransmit_Times(retryval);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerIP
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningServerIP(UI32_T *serverip)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
        ret =  RADIUS_OM_GetRunningServerIP(serverip);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_MGR_Get_Server_IP(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_Get_Server_IP();
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_IsValidIP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check the IP address is valid or not
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE    : None.
 *---------------------------------------------------------------------------
 */
static BOOL_T
RADIUS_MGR_IsValidIP(
    UI32_T serverip)
{
    if (IP_LIB_IsValidForRemoteIp((UI8_T *)&serverip) != IP_LIB_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T
RADIUS_MGR_Set_Server_IP(
    UI32_T serverip)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (TRUE == RADIUS_MGR_IsValidIP(serverip))
    {
        ret =  RADIUS_OM_Set_Server_IP(serverip);
    }
    else
    {
        ret = FALSE;
    }

#if (SYS_CPNT_EH == TRUE)
    if(ret == FALSE)
        EH_MGR_Handle_Exception(SYS_MODULE_RADIUS, RADIUS_MGR_SET_SERVER_IP_FUNC_NO, EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);/*Mercury_V2-00030*/
#endif

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_AsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE.
 * NOTE:     Announce result by system callback.
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_AsyncLoginAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size)
{
    UI32_T username_len;
    UI32_T passwd_len;
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (   (NULL == username)
        || (NULL == password)
        || (NULL == cookie)
        )
    {
        RADIUS_MGR_TRACE("Fail. Invalid arg");
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    username_len = strlen(username);
    passwd_len = strlen(password);
    if (   (username_len > SYS_ADPT_MAX_USER_NAME_LEN)
        || (passwd_len > SYS_ADPT_MAX_PASSWORD_LEN)
        || (cookie_size > RADIUS_MGR_ASYNC_MAX_COOKIE_SIZE)
        )
    {
        RADIUS_MGR_TRACE("Fail. Invalid arg length");

        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&request_entry, 0, sizeof(request_entry));

    request_entry.type = RADIUS_REQUEST_TYPE_USER_AUTH;
    request_entry.is_wait_for_response = TRUE;
    strncpy(request_entry.user_auth_data.username, username, username_len);
    request_entry.user_auth_data.username[username_len] = '\0';
    strncpy(request_entry.user_auth_data.password, password, passwd_len);
    request_entry.user_auth_data.password[passwd_len] = '\0';
    memcpy(request_entry.user_auth_data.cookie.cli.value, cookie, cookie_size);
    request_entry.user_auth_data.cookie.cli.len = cookie_size;

    if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
    {
        RADIUS_MGR_TRACE("Success to add into queue");

        SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
    }
    else
    {
        RADIUS_MGR_TRACE("Failed to add into queue");

        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Request ok");

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_AsyncEnablePasswordAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE.
 * NOTE:     Announce result by system callback.
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_AsyncEnablePasswordAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size)
{
    return RADIUS_MGR_AsyncLoginAuth(SYS_DFLT_ENABLE_PASSWORD_USERNAME,
        password, cookie, cookie_size);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_AsyncRadiusAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do user authentication
 * INPUT:    username
 *           password
 *           privilege
 *           cookie
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_AsyncRadiusAuthCheck(const char *username, const char *password,I32_T *privilege, UI32_T cookie)
{
    UI32_T username_len;
    UI32_T passwd_len;
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (   (NULL == username)
        || (NULL == password)
        || (NULL == privilege)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("username = %s, password = %s, cookie = %lu",
        username, password, cookie);

    username_len = strlen(username);
    passwd_len = strlen(password);
    if (   (username_len > SYS_ADPT_MAX_USER_NAME_LEN)
        || (passwd_len > SYS_ADPT_MAX_PASSWORD_LEN)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&request_entry, 0, sizeof(request_entry));

    request_entry.type = RADIUS_REQUEST_TYPE_WEB_AUTH;
    request_entry.is_wait_for_response = TRUE;
    strncpy(request_entry.user_auth_data.username, username, username_len);
    request_entry.user_auth_data.username[username_len] = '\0';
    strncpy(request_entry.user_auth_data.password, password, passwd_len);
    request_entry.user_auth_data.password[passwd_len] = '\0';
    request_entry.user_auth_data.cookie.web = cookie;

    if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
    {
        SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
    }
    else
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Request ok");

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_AsyncEapAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do EAP authentication
 * INPUT:    eap_data     --- EAP packet data
 *           eap_datalen  --- EAP packet data length
 *           radius_id    --- RADIUS sequent ID
 *           state_data   --- RADIUS STATE type packet data
 *           state_datale --- RADIUS STATE type packet data length
 *           src_port     --- source port
 *           src_mac      --- source mac address
 *           src_vid      --- source vid
 *           cookie       --- MSGQ_ID for return result
 *           service_type --- which component need to be service
 *           server_ip    --- Use this server IP address first. 0 means not
 *                            specified
 *           username_p   --- Username
 *           flag         --- Control flag
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     All asynchornous request will be enqueued and be proccessed by
 *           RADIUS task.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_AsyncEapAuthCheck(
    UI8_T   *eap_data,
    UI32_T  eap_datalen,
    UI32_T  radius_id,
    UI8_T   *state_data,
    UI32_T  state_datalen,
    UI32_T  src_port,
    UI8_T   *src_mac,
    UI32_T  src_vid,
    UI32_T  cookie,
    UI32_T  service_type,
    UI32_T  server_ip,
    char    *username_p,
    RADIUS_AsyncRequestControlFlag_T flag)
{
    UI32_T username_len;
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (   (NULL == eap_data)
        || (NULL == state_data)
        || (NULL == src_mac)
        || (NULL == username_p)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    username_len = strlen(username_p);
    if (   (sizeof(request_entry.dot1x_auth_data.eap_data) < eap_datalen)
        || (sizeof(request_entry.dot1x_auth_data.state_data) < state_datalen)
        || (username_len > DOT1X_USERNAME_LENGTH)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Username = %s", username_p);

    memset(&request_entry, 0, sizeof(request_entry));

    request_entry.type = RADIUS_REQUEST_TYPE_DOT1X_AUTH;
    request_entry.is_wait_for_response = TRUE;
    request_entry.dot1x_auth_data.src_port = src_port;
    request_entry.dot1x_auth_data.src_vid = src_vid;
    strncpy((char *)request_entry.dot1x_auth_data.username, username_p,
        username_len);
    request_entry.dot1x_auth_data.username[username_len] = '\0';
    memcpy(request_entry.dot1x_auth_data.src_mac, src_mac,
        sizeof(request_entry.dot1x_auth_data.src_mac));
    memcpy(request_entry.dot1x_auth_data.eap_data, eap_data, eap_datalen);
    request_entry.dot1x_auth_data.eap_data_len = eap_datalen;
    memcpy(request_entry.dot1x_auth_data.state_data, state_data, state_datalen);
    request_entry.dot1x_auth_data.state_data_len = state_datalen;
    request_entry.dot1x_auth_data.cookie = cookie;
    request_entry.dot1x_auth_data.server_ip = server_ip;

    /* Request which is asking to delete the request may be able deleted from
     * waiting queue (not de-queue to run state machine yet) or request queue
     * (already running state maching).
     */
    if (RADIUS_ASYNC_REQ_FLAG_CANCEL_REQUEST == flag)
    {
        UI32_T request_index;
        BOOL_T is_user_exist = FALSE;

        if (RADIUS_RETURN_SUCCESS == RADIUS_OM_DeleteAuthReqFromWaitingQueue(
            &request_entry))
        {
            is_user_exist = TRUE;
        }

        if (RADIUS_RETURN_SUCCESS == RADIUS_OM_GetRequestIndex(&request_entry,
            &request_index))
        {
            UI32_T current_time;

            SYS_TIME_GetRealTimeBySec(&current_time);

            RADIUS_OM_SetRequestDestroyFlag(request_index, TRUE);
            RADIUS_OM_SetRequestTimeout(request_index, current_time);
            RADIUS_OM_ResortRequestByTimeoutOrder(request_index);

            is_user_exist = TRUE;
            SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
        }

        if (FALSE == is_user_exist)
        {
            RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
    {
        SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
    }
    else
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Request ok");

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_RadaAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do RADA authentication
 * INPUT:    src_port, src_mac, rada_username, rada_passwd,
 *           cookie (MSGQ_ID to return the result)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_RadaAuthCheck(
    UI32_T  src_port,       UI8_T   *src_mac,
    char    *rada_username, char    *rada_passwd,   UI32_T  cookie)
{
    UI32_T username_len;
    UI32_T password_len;
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("Enter");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (   (NULL == src_mac)
        || (NULL == rada_username)
        || (NULL == rada_passwd)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    username_len = strlen(rada_username);
    password_len = strlen(rada_passwd);
    if (   (username_len > RADIUS_MAX_MAC_STRING_LENGTH)
        || (password_len > RADIUS_MAX_MAC_STRING_LENGTH)
        )
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&request_entry, 0, sizeof(request_entry));

    request_entry.type = RADIUS_REQUEST_TYPE_RADA_AUTH;
    request_entry.is_wait_for_response = TRUE;
    request_entry.rada_auth_data.src_port = src_port;
    strncpy((char *)request_entry.rada_auth_data.username, rada_username,
        username_len);
    request_entry.rada_auth_data.username[username_len] = '\0';
    strncpy((char *)request_entry.rada_auth_data.password, rada_passwd,
        password_len);
    request_entry.rada_auth_data.username[password_len] = '\0';
    memcpy(request_entry.rada_auth_data.src_mac, src_mac,
        sizeof(request_entry.rada_auth_data.src_mac));
    request_entry.rada_auth_data.cookie = cookie;

    if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
    {
        SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
    }
    else
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Request ok");

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*  For MIB */

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_MGR_Get_UnknowAddress_Packets(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_SNMP_Get_UnknowAddress_Packets();
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_NAS_ID
 *---------------------------------------------------------------------------
 * PURPOSE:  Get he NAS-Identifier of the RADIUS authentication client.
 *           This is not necessarily the same as sysName in MIB II.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   NASID
 *           NASID = NULL  ---No NAS ID
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Get_NAS_ID(UI8_T *nas_id)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret =  RADIUS_OM_SNMP_Get_NAS_ID(nas_id);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T GetAuthServerTable(UI32_T index,AuthServerEntry *ServerEntry)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if(index <=0 || index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
        {
            return FALSE;
        }
        index--;
        ret =  RADIUS_OM_SNMP_GetAuthServerTable(index,ServerEntry);
    }
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T GetNextAuthServerTable(UI32_T *index,AuthServerEntry *ServerEntry)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if(*index < 0 || *index >= SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
        {
            return FALSE;
        }
        ret =  RADIUS_OM_SNMP_GetNextAuthServerTable(index,ServerEntry);
    }
    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the RADIUS server host
 * INPUT:    server_index,server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:        server_ip;
 *      server_port (1-65535)  - set 0 will use the global radius configuration
 *          timeout     (1-65535)  - set 0 will use the global radius configuration
 *          retransmit  (1-65535)  - set 0 will use the global radius configuration
 *          secret      (length < RADIUS_MAX_SECRET_LENGTH)  - set NULL will use the global radius configuration
 *---------------------------------------------------------------------------
 */
BOOL_T
RADIUS_MGR_Set_Server_Host(
    UI32_T server_index,
    RADIUS_Server_Host_T *server_host)
{
    BOOL_T ret;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T  global_acct_port;
    RADIUS_LightServerHost_T    om_entry;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if (TRUE == RADIUS_MGR_IsValidIP(server_host->server_ip))
    {
        ret =  RADIUS_OM_Set_Server_Host(server_index,server_host);
    }
    else
    {
        ret = FALSE;
    }

#if (SYS_CPNT_AAA == TRUE)
    if (TRUE == ret)
    {
        AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup(server_index);
    }
#endif /* SYS_CPNT_AAA == TRUE */

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the RADIUS server host
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Destroy_Server_Host(UI32_T server_index)
{
    BOOL_T ret;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    RADIUS_LightServerHost_T    server_host;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = RADIUS_OM_Destroy_Server_Host(server_index);
    }

#if (SYS_CPNT_AAA == TRUE)
    if (TRUE == ret)
        AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup(server_index);
#endif /* SYS_CPNT_AAA == TRUE */

    return ret;
}

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the RADIUS server host configuration
 * INPUT:    server_index
 * OUTPUT:   server_host.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetNext_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return FALSE;
    else
        ret = RADIUS_OM_GetNext_Server_Host(index,server_host);
    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetNextRunning_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server host is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host)
{
    UI32_T ret;

    if(*index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else
       ret =  RADIUS_OM_GetNextRunning_Server_Host(index,server_host);

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the RADIUS server host configuration
 * INPUT:    server_index
 * OUTPUT:   server_host.
 * RETURN:   TRUE/FALSE
 * NOTE:     The index range is from 1 to SYS_ADPT_RADIUS_MAX_NUMBER_OF_SERVERS
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if(index <= 0 || index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
        {
            return FALSE;
        }
        index--;
        ret = RADIUS_OM_Get_Server_Host(index,server_host);
    }

    return ret;
}
#endif

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetServerHostMaxRetransmissionTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_GetServerHostMaxRetransmissionTimeout(max_retransmission_timeout);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_IsServerHostValid
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_IsServerHostValid(UI32_T server_index)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_IsServerHostValid(server_index);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_LookupServerIndexByIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_LookupServerIndexByIpAddress(ip_address, server_index);

     RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE: This function to get the number of RADIUS Accounting-Response packets
 *          received from unknown addresses.
 * INPUT:   none.
 * OUTPUT:  UI32_T *invalid_server_address_counter  --  The number of RADIUS Accounting-Response
 *                                                      packets received from unknown addresses.
 * RETURN:  TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:   none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == invalid_server_address_counter)
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = RADIUS_OM_GetAccClientInvalidServerAddresses(invalid_server_address_counter);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccClientIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE  : This function to get the NAS-Identifier of the RADIUS accounting client.
 * INPUT    : none.
 * OUTPUT   : client_identifier  --  the NAS-Identifier of the RADIUS accounting client.
 * RETURN   : TRUE to indicate successful and FALSE to indicate failure.
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccClientIdentifier(RADACC_AccClientIdentifier_T *client_identifier)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == client_identifier)
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = MIB2_POM_GetSysName(client_identifier->identifier);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQty
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQty(UI32_T *qty)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_GetAccUserEntryQty(qty);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByNameAndType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_GetAccUserEntryQtyFilterByNameAndType(name, client_type, qty);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_GetAccUserEntryQtyFilterByType(client_type, qty);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = RADIUS_OM_GetAccUserEntryQtyFilterByPort(ifindex, qty);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserRunningInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserRunningInfo(UI32_T ifindex, const char *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info)
{
    BOOL_T  ret;

    RADACC_UserInfo_T   entry;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == running_info)
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = RADIUS_OM_GetAccUserEntryByKey(ifindex, name, client_type, &entry);
    if (TRUE == ret)
    {
        running_info->session_start_time = entry.session_start_time;
        running_info->connect_status = entry.connect_status;

        if (FALSE == RADIUS_OM_GetServerHostIpAddress(entry.radius_entry_info.active_server_index, &running_info->in_service_ip))
        {
            running_info->in_service_ip = 0;
        }
    }

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/* FUNCTION NAME - RADIUS_MGR_HandleHotInsertion
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
void RADIUS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    return;
}

/* FUNCTION NAME - RADIUS_MGR_HandleHotRemoval
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
void RADIUS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    return;
}

#if 0
/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_CollectReturnedAttribute
 *---------------------------------------------------------------------------
 * PURPOSE  : collect the returned attributes we want from the RADIUS returned attributes link list.
 * INPUT    : return_attribute_list, eap_flag
 * OUTPUT   : attribute_collector
 * RETURN   : None.
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_CollectReturnedAttribute(Radius_Attribute_Collector_T *attribute_collector, VALUE_PAIR *return_attribute_list)
{
    VALUE_PAIR *vp, *got_vp;

    attribute_collector->service_type = 0;
    memset(attribute_collector->filter_id,0,FILTER_ID_STRING_LEN);
    attribute_collector->tunnel_type = 0;
    attribute_collector->tunnel_medium_type = 0;
    attribute_collector->tunnel_private_group_id = 0;

    got_vp = return_attribute_list;

    vp = got_vp;
    if ((vp = rc_avpair_get(vp, PW_SERVICE_TYPE)))
        attribute_collector->service_type = vp->lvalue;

    vp = got_vp;
    if ((vp = rc_avpair_get(vp, PW_FILTER_ID)))
    {
        memcpy(attribute_collector->filter_id, vp->strvalue,vp->lvalue);
    }

/***** add for tunnel *****/
    vp = got_vp;
    if ((vp = rc_avpair_get(vp, PW_TUNNEL_TYPE)))
    {
        attribute_collector->tunnel_type = vp->lvalue;
    }

    vp = got_vp;
    if ((vp = rc_avpair_get(vp, PW_TUNNEL_MEDIUM_TYPE)))
    {
        attribute_collector->tunnel_medium_type = vp->lvalue;
    }

    vp = got_vp;
    if ((vp = rc_avpair_get(vp, PW_TUNNEL_PRIVATE_GROUP)))
    {
        if(vp->strvalue[0] <= 0x1F)
            attribute_collector->tunnel_private_group_id = atoi(vp->strvalue+1);
        else
            attribute_collector->tunnel_private_group_id = atoi(vp->strvalue);
    }
/**************************/

}


/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_LocalParseFilterId
 *---------------------------------------------------------------------------
 * PURPOSE  : parse filter-id string then fill those value to struct attr_filter_id
 * INPUT    : filter_id_str (terminate by '\0')
 * OUTPUT   : attr_filter_id
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_LocalParseFilterId(const UI8_T *filter_id_str, RADIUS_AttrFilterId_T *attr_filter_id)
{
    const UI8_T *segment_start, *segment_stop;

    if ((NULL == filter_id_str) || (NULL == attr_filter_id))
        return FALSE;

    if (RADIUS_MAX_LEN_OF_FILTER_ID < strlen((char *)filter_id_str))
        return FALSE;

    memset(attr_filter_id, 0, sizeof(RADIUS_AttrFilterId_T)); /* initialize struct */

    /*ES3526VX-60-00422*/
    if(filter_id_str[strlen((char *)filter_id_str)-1] == '\"')
#if 0 /* wakka: to avoid warning */
        filter_id_str[strlen(filter_id_str)-1] = '\0';
#else
        *(UI8_T *)&filter_id_str[strlen((char *)filter_id_str)-1] = '\0';
#endif

   if(*filter_id_str == '\"')
    filter_id_str++;


    for (segment_start = segment_stop = filter_id_str; ; ++segment_stop)
    {
        if ((FILTER_ID_SEPARATOR_1 == *segment_stop) ||
            /*(FILTER_ID_SEPARATOR_2 == *segment_stop) ||*/
            ('\0' == *segment_stop))
        {
            RADIUS_MGR_LocalParseFilterIdSegment(segment_start, segment_stop, attr_filter_id);
            if ('\0' == *segment_stop)
                break;

            segment_start = segment_stop + 1;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_LocalParseFilterIdSegment
 *---------------------------------------------------------------------------
 * PURPOSE  : parse segment string then fill related field to struct attr_filter_id
 * INPUT    : segment string start(include) pointer and stop(exclude) pointer
 * OUTPUT   : attr_filter_id
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_LocalParseFilterIdSegment(const UI8_T *start, const UI8_T *stop, RADIUS_AttrFilterId_T *attr_filter_id)
{
    const int   max_token_number = 3;
    const UI8_T *cp;
    UI8_T   token[max_token_number][RADIUS_MAX_LEN_OF_FILTER_ID_TOKEN + 1];
    int     i;

    if ((NULL == start) || (NULL == stop) || (NULL == attr_filter_id))
        return FALSE;

    if (start == stop)
    {
         ++(attr_filter_id->malformed_counter);
        return FALSE;
    }

    for (cp =  start, i = 0; cp < stop; )
    {
        if (max_token_number <= i) /* too many tokens */
        {
            ++(attr_filter_id->malformed_counter);
            return FALSE;
        }

        cp = RADIUS_MGR_LocalGetFilterIdToken(cp, token[i]);
        if (NULL == cp)
        {
            ++(attr_filter_id->malformed_counter);
            return FALSE;
        }

        if (strlen((char *)token[i]) > 0) /* avoid this kind of string: "   " */
            ++i;
    }

    switch (i)
    {
        case 1:
            RADIUS_MGR_LocalCheckFilter(token[0], NULL, ('\0' == *stop), attr_filter_id);
            break;

        case 3:
            if (strcmp((char *)token[1], "="))
            {
                ++(attr_filter_id->malformed_counter);
                return FALSE;
            }

            RADIUS_MGR_LocalCheckFilter(token[0], token[2], ('\0' == *stop), attr_filter_id);
            break;

        default:
            ++(attr_filter_id->malformed_counter);
            return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_LocalGetFilterIdToken
 *---------------------------------------------------------------------------
 * PURPOSE  : get a token from source string
 * INPUT    : source (terminated by '\0' or ':' or ';'
 * OUTPUT   : token
 * RETURN   : the start position of next searching
 * NOTE     : token size is at least RADIUS_MAX_LEN_OF_FILTER_ID_TOKEN
 *---------------------------------------------------------------------------
 */
static const UI8_T *RADIUS_MGR_LocalGetFilterIdToken(const UI8_T *source, UI8_T *token)
{
    const UI8_T *cp;
    UI8_T       count;

    if ((NULL == source) || (NULL == token))
        return NULL;

    cp = source;

    /* bypass leading blank */
    while ((' ' == *cp) ||
        ('\t' == *cp) ||
        ('\n' == *cp) ||
        ('\r' == *cp))
        cp++;

    for (count = 0; ; cp++, count++)
    {
        if ((FILTER_ID_SEPARATOR_1 == *cp) ||
            /*(FILTER_ID_SEPARATOR_2 == *cp) ||*/
            ('\0' == *cp))
            break;

        if (RADIUS_MAX_LEN_OF_FILTER_ID_TOKEN < count)
            return NULL;

        token[count] = *cp;
        if ('=' == *cp)
        {
            if (0 == count) /* token is '=' */
            {
                cp ++;
                count ++;
            }
            break;
        }
        if ((' ' == *cp) ||
            ('\t' == *cp) ||
            ('\n' == *cp) ||
            ('\r' == *cp))
            break;
    }
    token[count] = '\0';

    /* need to know: no more token
       so that cp will point to stop-pointer
     */
    while ((' ' == *cp) ||
        ('\t' == *cp) ||
        ('\n' == *cp) ||
        ('\r' == *cp))
        cp++;

    return cp;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_LocalCheckFilter
 *---------------------------------------------------------------------------
 * PURPOSE  : check keyword and option
 * INPUT    : keyword, option (terminated by '\0')
 * OUTPUT   : attr_filter_id
 * RETURN   : none
 * NOTE     : option == NULL implies no option
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_LocalCheckFilter(UI8_T *keyword, UI8_T *option, BOOL_T is_last, RADIUS_AttrFilterId_T *attr_filter_id)
{
    const UI8_T diff = 'a' - 'A';
    UI8_T       *cp;

    if ((NULL == keyword) || (NULL == attr_filter_id))
        return;

    /* convert keywrod to lower */
    for (cp = keyword; '\0' != *cp; ++cp)
        *cp = (('A' <= *cp) && ('Z' >= *cp)) ? (*cp + diff) : *cp;

    if (! strcmp((char *)keyword, RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_ETS))
    {
        if ((NULL != option) || /* e.g. enterasys=xxx */
            (0 < (attr_filter_id->recognized_counter + attr_filter_id->malformed_counter))) /* not first segment */
        {
            ++(attr_filter_id->malformed_counter);
            return;
        }

        attr_filter_id->attr_bitmap.enterasys_existed = 1;
        ++(attr_filter_id->recognized_counter);
        return;
    }
    else if (! strcmp((char *)keyword, RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_VERSION))
    {
        if ((NULL == option) ||
            (1 == attr_filter_id->attr_bitmap.version_existed) ||
            (strlen((char *)option) > RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_VERSION))
        {
            ++(attr_filter_id->malformed_counter);
            return;
        }

        attr_filter_id->attr_bitmap.version_existed = 1;
        ++(attr_filter_id->recognized_counter);
        strcpy((char *)attr_filter_id->version, (char *)option);
        return;
    }
    else if (! strcmp((char *)keyword, RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_MGMT))
    {
        if ((NULL == option) ||
            (1 == attr_filter_id->attr_bitmap.mgmt_existed) ||
            (strlen((char *)option) > RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_MGMT))
        {
            ++(attr_filter_id->malformed_counter);
            return;
        }

        attr_filter_id->attr_bitmap.mgmt_existed = 1;
        ++(attr_filter_id->recognized_counter);
        strcpy((char *)attr_filter_id->mgmt, (char *)option);
        return;
    }
    else if (! strcmp((char *)keyword, RADIUS_ENTERASYS_FILTER_ID_RESERVED_WORD_POLICY))
    {
        if ((NULL == option) ||
            (1 == attr_filter_id->attr_bitmap.policy_existed) ||
            (strlen((char *)option) > RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_POLICY))
        {
            ++(attr_filter_id->malformed_counter);
            return;
        }

        attr_filter_id->attr_bitmap.policy_existed = 1;
        ++(attr_filter_id->recognized_counter);
        strcpy((char *)attr_filter_id->policy, (char *)option);
        return;
    }
    else
    {
        if ((NULL != option) ||
            (FALSE == is_last) || /* not last segment */
            (0 < (attr_filter_id->recognized_counter + attr_filter_id->malformed_counter)) || /* not first segment */
            (strlen((char *)keyword) > RADIUS_MAX_LEN_OF_ENTERASYS_FILTER_ID_POLICY))
        {
            ++(attr_filter_id->malformed_counter);
            return;
        }

        /* un-decorated filter id */
        ++(attr_filter_id->recognized_counter);
        attr_filter_id->attr_bitmap.undecorated_filter_id = 1;
        strcpy((char *)attr_filter_id->policy, (char *)keyword);
        return;
    }
}

#if(SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE == TRUE)
/*---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: mode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------*/

SYS_TYPE_Get_Running_Cfg_T RADIUS_MGR_GetRunningAuthCheckingServiceTypeEnabled(BOOL_T *mode)
{

    SYS_TYPE_Get_Running_Cfg_T ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = RADIUS_OM_GetRunningAuthCheckingServiceTypeEnabled(mode);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_SetAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------------
 * PURPOSE  : Set whether to check service-type when authorizing from radius server
 * INPUT    : mode
 * OUTPUT   : None
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE:    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetAuthCheckingServiceTypeEnabled(BOOL_T mode)
{
    BOOL_T  ret;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    ret = RADIUS_OM_SetAuthCheckingServiceTypeEnabled(mode);
    RADIUS_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_GetAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------------
 * PURPOSE  : Get whether to check service-type when authorizing from radius server
 * INPUT    : None
 * OUTPUT   : mode
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE:    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAuthCheckingServiceTypeEnabled(BOOL_T *mode)
{
   BOOL_T  ret;
   ret = RADIUS_OM_GetAuthCheckingServiceTypeEnabled(mode);
   return ret;

}

#endif
#endif

#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_Backdoor_ShowAccUser
 *---------------------------------------------------------------------------
 * PURPOSE  : show accounting user information
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : for radius backdoor
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Backdoor_ShowAccUser()
{
    UI32_T  sys_time, ip_address;
    UI8_T   *ip_b;

    RADACC_UserInfo_T   entry;

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    SYS_TIME_GetRealTimeBySec(&sys_time);

    entry.user_index = 0;
    while (TRUE == RADIUS_OM_GetNextAccUserEntry(&entry))
    {
        printf("\r\nname(%s), ifindex(%lu), client_type(%s)",
            entry.user_name, entry.ifindex,
            (AAA_CLIENT_TYPE_DOT1X == entry.client_type) ? "DOT1X" :
            (AAA_CLIENT_TYPE_DOT1X == entry.client_type) ? "EXEC" :
            "UNKNOW TYPE");

        if (FALSE == RADIUS_OM_GetServerHostIpAddress(entry.radius_entry_info.active_server_index, &ip_address))
            ip_address = 0;

        ip_b = (UI8_T*)&ip_address;
        printf("\r\n\ttime elapsed(%lu) status(%s) server(%d.%d.%d.%d)",
            sys_time - entry.session_start_time,
            (AAA_ACC_CNET_IDLE == entry.connect_status) ? "idle" :
            (AAA_ACC_CNET_CONNECTING == entry.connect_status) ? "connecting" :
            (AAA_ACC_CNET_CONNECTED == entry.connect_status) ? "connected" :
            (AAA_ACC_CNET_TIMEOUT == entry.connect_status) ? "timeout" :
            (AAA_ACC_CNET_FAILED == entry.connect_status) ? "failed" :
            "unknown",
            ip_b[0], ip_b[1], ip_b[2], ip_b[3]);
    }

    RADIUS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */


#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_Reload_CallBack
 *---------------------------------------------------------------------------
 * PURPOSE  : system reload callback
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Reload_CallBack()
{
    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    /* ES3526VX-60-00364,
       need to send accounting-stop while reload or TCN
     */
    RADIUS_MGR_LocalStopAndFreeAllAccUsers(AAA_ACC_TERM_BY_ADMIN_REBOOT);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

#if 0
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_TCN_CallBack
 *---------------------------------------------------------------------------
 * PURPOSE  : system TCN callback
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_TCN_CallBack()
{
    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    /* ES3526VX-60-00364,
       need to send accounting-stop while reload or TCN
     */
    RADIUS_MGR_LocalStopAndFreeAllAccUsers(AAA_ACC_TERM_BY_NAS_REQUEST);

    RADIUS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}
#endif /* #if 0 */

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_AsyncAccountingRequest_Callback
 *---------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : identifier, ifindex, client_type, request_type,
 *            user_name     --  User name (terminated with '\0')
 *            call_back_func      --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_AsyncAccountingRequest_Callback(const AAA_AccRequest_T *request)
{
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == request)
    {
        RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&request_entry, 0, sizeof(request_entry));

    request_entry.type = RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE;
    request_entry.is_wait_for_response = TRUE;
    memcpy(&request_entry.accounting_create_data.request, request,
        sizeof(request_entry.accounting_create_data.request));

    switch (request->request_type)
    {
        case AAA_ACC_START:

            RADIUS_MGR_TRACE("Start accounting");

            if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
            {
                SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
            }
            else
            {
                RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        case AAA_ACC_STOP:
        {
            RADACC_UserInfo_T user_entry;

            BOOL_T is_user_exist = FALSE;

            RADIUS_MGR_TRACE("Stop accounting");

            if (RADIUS_RETURN_SUCCESS == RADIUS_OM_DeleteAuthReqFromWaitingQueue(
                &request_entry))
            {
                is_user_exist = TRUE;
            }

            if (TRUE == RADIUS_OM_GetAccUserEntryByKey(request->ifindex,
                request->user_name, request->client_type, &user_entry))
            {
                UI32_T current_time;

                SYS_TIME_GetRealTimeBySec(&current_time);

                RADIUS_OM_SetAccUserEntryDestroyFlag(user_entry.user_index, TRUE);
                RADIUS_OM_SetAccUserEntryTimeout(user_entry.user_index, current_time);
                RADIUS_OM_ResortAccUserEntryByTimeoutOrder(user_entry.user_index);

                is_user_exist = TRUE;
                SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
            }

            if (FALSE == is_user_exist)
            {
                RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;
        }

        default:
            RADIUS_MGR_TRACE("Invalid type (%d)", request->request_type);

            RADIUS_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    RADIUS_MGR_TRACE("Request ok");

    RADIUS_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_LocalStopAndFreeAllAccUsers
 *---------------------------------------------------------------------------
 * PURPOSE  : flush all requests, send stop for all users and free all users
 * INPUT    : reason
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_LocalStopAndFreeAllAccUsers(AAA_AccTerminateCause_T reason)
{
    RADACC_UserInfo_T user_info;
    UI32_T current_time;

    SYS_TIME_GetRealTimeBySec(&current_time);

    user_info.user_index = 0;
    while (TRUE == RADIUS_OM_GetNextAccUserEntry(&user_info))
    {
        UI32_T request_index;

        RADIUS_MGR_TRACE("Stop user (%s)", user_info.user_name);

        RADIUS_OM_GetAccUserEntryRequestIndex(user_info.user_index, &request_index);
        if (0 != request_index)
        {
            RADIUS_OM_SetRequestDestroyFlag(request_index, TRUE);
            RADIUS_MGR_RunRequestStateMachine(request_index);
        }

        RADIUS_OM_SetAccUserEntryTerminateCause(user_info.user_index, reason);

        RADIUS_MGR_CreateAccountingRequest(RADACC_STOP, user_info.user_index,
            current_time, FALSE, &request_index);
        RADIUS_OM_SetAccUserEntryRequestIndex(user_info.user_index,
            request_index);
        RADIUS_MGR_RunRequestStateMachine(request_index);

        /* The user is deleted during running state machine
         */

        user_info.user_index = 0;
    }
}
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SubmitRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : Submit a request, call RADIUS_OM_SubmitRequest to create a
 *            request in request queue.
 * INPUT    : request_p - Request
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_SubmitRequest(
    RADIUS_MGR_RequestContext_T *request_p)
{
    RADIUS_ReturnValue_T ret;
    RADIUS_OM_RequestEntry_T request_entry;

    RADIUS_MGR_TRACE("");

    if((NULL == request_p)||
       (request_p->blocking == TRUE))
    {
        return RADIUS_RETURN_FAIL;
    }

    switch (request_p->type)
    {
#if (SYS_CPNT_IGMPAUTH == TRUE)
        case RADIUS_REQUEST_TYPE_IGMPAUTH:
            memcpy(&request_entry.igmp_auth_data , &request_p->igmp_auth_data,sizeof(request_entry.igmp_auth_data));
            break;
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

        case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
            memcpy(&request_entry.dot1x_auth_data, &request_p->dot1x_auth_data, sizeof(request_entry.dot1x_auth_data));
            break;

        case RADIUS_REQUEST_TYPE_RADA_AUTH:
            memcpy(&request_entry.rada_auth_data, &request_p->rada_auth_data, sizeof(request_entry.rada_auth_data));
            break;

        case RADIUS_REQUEST_TYPE_WEB_AUTH:
        case RADIUS_REQUEST_TYPE_USER_AUTH:
            memcpy(&request_entry.user_auth_data, &request_p->user_auth_data, sizeof(request_entry.user_auth_data));
            break;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        case RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE:
            memcpy(&request_entry.accounting_create_data, &request_p->accounting_create_data, sizeof(request_entry.accounting_create_data));
            break;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        default:
            return RADIUS_RETURN_FAIL;
    }

    request_entry.type = request_p->type;
    request_entry.is_wait_for_response = TRUE;

    if (RADIUS_RETURN_SUCCESS == RADIUS_OM_AddAuthReqIntoWaitingQueue(&request_entry))
    {
        SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_NEW_REQ);
    }
    else
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_MGR_TRACE("Request ok");

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_CreateAuthReqSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Create the socket to send and receive authentcation request(s).
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_CreateAuthReqSocket()
{
    int socket_id;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    if (-1 != socket_id)
    {
        return RADIUS_RETURN_FAIL;
    }

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_id < 0)
    {
        /* Socket is created already.
         */
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_RETURN_FAIL == RADIUS_OM_SetAuthReqIdQueueSocketId(socket_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_DestroyAuthReqSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Destroy the socket to send and receive authentcation request(s).
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_DestroyAuthReqSocket()
{
    int socket_id;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    if (-1 == socket_id)
    {
        /* Socket is not created yet.
         */
        return RADIUS_RETURN_FAIL;
    }

    s_close(socket_id);
    socket_id = -1;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_SetAuthReqIdQueueSocketId(socket_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_AnnounceResult
 *---------------------------------------------------------------------------
 * PURPOSE  : Announce the result of the request.
 * INPUT    : request_index - Request index
 *            result        - Result
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_AnnounceResult(UI32_T request_index,
    UI32_T result)
{
    RADIUS_OM_RadiusRequestType_T type;
    RADIUS_OM_RequestEntry_T request_data;
    RADIUS_OM_RequestResult_T result_data;
    UI8_T *response_packet_p;
    UI32_T response_packet_len;

    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index,
            &type))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestData(request_index,
            &request_data))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestResponsePacket(request_index,
            &response_packet_p, &response_packet_len))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestResultData(request_index,
            &result_data))
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    switch (type)
    {
#if (SYS_CPNT_IGMPAUTH == TRUE)
    case RADIUS_REQUEST_TYPE_IGMPAUTH:
        SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult(SYS_MODULE_RADIUS,
            result,
            request_data.igmp_auth_data.auth_port,
            request_data.igmp_auth_data.ip_address,
            request_data.igmp_auth_data.auth_mac,
            request_data.igmp_auth_data.vlan_id,
            request_data.igmp_auth_data.src_ip,
            request_data.igmp_auth_data.msg_type);
        break;
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

    case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
    {
        I32_T callback_result;
        char empty_tunnel_private_group_id[] = "";
        char *callback_tunnel_private_group_id_p =
            result_data.dot1x_auth.tunnel_private_group_id;

        callback_result = (RADIUS_RESULT_SUCCESS == result) ? OK_RC :
            (RADIUS_RESULT_FAIL == result) ? ERROR_RC : TIMEOUT_RC;

        RADIUS_MGR_TRACE(
            "dot1x result = %ld, tunnel type = %lu, tunnel medium type = %lu, private group ID = '%s', filter ID = '%s'",
            callback_result, result_data.dot1x_auth.tunnel_type,
            result_data.dot1x_auth.tunnel_medium_type,
            result_data.dot1x_auth.tunnel_private_group_id,
            result_data.dot1x_auth.filter_id);

        /* Tunnel type and tunnel medium type should be expect values
         */
        if (   (PW_VLAN != result_data.dot1x_auth.tunnel_type)
            || (PW_802 != result_data.dot1x_auth.tunnel_medium_type)
            )
        {
            callback_tunnel_private_group_id_p = empty_tunnel_private_group_id;
        }

        RADIUS_MGR_TRACE("Callback with private group ID %s",
            callback_tunnel_private_group_id_p);

        SYS_CALLBACK_MGR_AnnounceRADIUSPacket(SYS_MODULE_RADIUS,
            request_data.dot1x_auth_data.cookie, callback_result,
            response_packet_p,
            response_packet_len,
            request_data.dot1x_auth_data.src_port,
            request_data.dot1x_auth_data.src_mac,
            request_data.dot1x_auth_data.src_vid,
            callback_tunnel_private_group_id_p,
            result_data.dot1x_auth.filter_id,
            result_data.dot1x_auth.session_timeout,
            result_data.server_ip);
        break;
    }

    case RADIUS_REQUEST_TYPE_RADA_AUTH:
    {
        BOOL_T callback_result;
        char empty_tunnel_private_group_id[] = "";
        char *callback_tunnel_private_group_id_p =
            result_data.rada_auth.tunnel_private_group_id;

        callback_result = (RADIUS_RESULT_SUCCESS == result) ? TRUE : FALSE;

        RADIUS_MGR_TRACE(
            "rada result = %u, tunnel type = %lu, tunnel medium type = %lu, private group ID = '%s', filter ID = '%s'",
            callback_result, result_data.rada_auth.tunnel_type,
            result_data.rada_auth.tunnel_medium_type,
            result_data.rada_auth.tunnel_private_group_id,
            result_data.rada_auth.filter_id);

        /* Tunnel type and tunnel medium type should be expect values
        */
        if ((PW_VLAN != result_data.rada_auth.tunnel_type)
            || (PW_802 != result_data.rada_auth.tunnel_medium_type)
            )
        {
            callback_tunnel_private_group_id_p = empty_tunnel_private_group_id;
        }

        SYS_CALLBACK_MGR_AnnounceRadiusAuthorizedResult(SYS_MODULE_RADIUS,
            request_data.rada_auth_data.cookie,
            request_data.rada_auth_data.src_port,
            request_data.rada_auth_data.src_mac,
            0, callback_result,
            (UI8_T *)callback_tunnel_private_group_id_p,
            (UI8_T *)result_data.rada_auth.filter_id,
            result_data.rada_auth.session_timeout,
            result_data.server_ip);
        break;
    }

    case RADIUS_REQUEST_TYPE_WEB_AUTH:
    {
        I32_T callback_result;
        UI32_T ret;

        callback_result = (RADIUS_RESULT_SUCCESS == result) ? OK_RC :
            (RADIUS_RESULT_FAIL == result) ? ERROR_RC : TIMEOUT_RC;

        RADIUS_MGR_TRACE("web auth result = %ld, privilege = %lu, cookie = %lu",
            callback_result, result_data.user_auth.privilege,
            request_data.user_auth_data.cookie.web);

        AUTH_PROTOCOL_GROUP_SendAuthCheckResponseMsg(callback_result,
            result_data.user_auth.privilege,
            request_data.user_auth_data.cookie.web);
        break;
    }

    case RADIUS_REQUEST_TYPE_USER_AUTH:
    {
        I32_T callback_result;
        UI32_T privilege;

        callback_result = (RADIUS_RESULT_SUCCESS == result) ? OK_RC :
            (RADIUS_RESULT_FAIL == result) ? BADRESP_RC : TIMEOUT_RC;

        if (SYS_ADPT_RADIUS_USE_SERVICE_TYPE_AS_PRIVILEGE == TRUE)
        {
            if (PW_ADMINISTRATIVE == result_data.user_auth.privilege)
            {
                privilege = AUTH_ADMINISTRATIVE;
            }
            else
            {
                privilege = AUTH_LOGIN;
            }
        }
        if (SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE == TRUE)
        {
            if (AUTH_LOGIN_ERROR == result_data.user_auth.privilege)
            {
                callback_result = BADRESP_RC;
            }
        }

        RADIUS_MGR_TRACE("user auth result = %ld, privilege = %lu",
            callback_result, result_data.user_auth.privilege);

        SYS_CALLBACK_MGR_AnnounceRemServerAuthResult(SYS_MODULE_RADIUS,
            callback_result, privilege, request_data.user_auth_data.cookie.cli.value,
            request_data.user_auth_data.cookie.cli.len);
        break;
    }

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    case RADIUS_REQUEST_TYPE_ACCOUNTING:
        RADIUS_MGR_AnnounceAccountingResult(request_index, result);
        break;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    default:
        return RADIUS_RETURN_FAIL;
    }

    return RADIUS_RETURN_SUCCESS;
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_AnnounceAccountingResult
 *---------------------------------------------------------------------------
 * PURPOSE  : Announce the result of the accounting request.
 * INPUT    : request_index - Request index
 *            result        - Result
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_AnnounceAccountingResult(
    UI32_T request_index, UI32_T result)
{
    UI32_T user_index;
    RADACC_UserInfo_T user_entry;
    RADIUS_OM_RequestEntry_T request_data;
    RADACC_AcctStatusType_T request_type;
    BOOL_T is_need_to_update_timeout = FALSE;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestData(request_index,
        &request_data))
    {
        return RADIUS_RETURN_FAIL;
    }

    user_index = request_data.accounting_data.user_index;
    request_type = request_data.accounting_data.request_type;

    user_entry.user_index = user_index;
    if (FALSE == RADIUS_OM_GetAccUserEntry(&user_entry))
    {
        return RADIUS_RETURN_FAIL;
    }

    if (RADACC_START == request_type)
    {
        if (RADIUS_RESULT_SUCCESS == result)
        {
            RADIUS_OM_SetAccUserEntryStartResponseSent(user_index, TRUE);
        }

        is_need_to_update_timeout = TRUE;
    }
    else if (RADACC_InterimUpdate == request_type)
    {
        is_need_to_update_timeout = TRUE;
    }
    else if (RADACC_STOP == request_type)
    {
        /* Not to try stop request anymore
         */
        RADIUS_OM_FreeAccUser(user_index);
    }

    if (TRUE == is_need_to_update_timeout)
    {
        UI32_T current_time;
        UI32_T update_interval;

        SYS_TIME_GetRealTimeBySec(&current_time);
        AAA_OM_GetAccUpdateInterval(&update_interval);
        RADIUS_OM_SetAccUserEntryTimeout(user_index,
            current_time + update_interval * 60);
        RADIUS_OM_ResortAccUserEntryByTimeoutOrder(user_index);
    }

    return RADIUS_RETURN_SUCCESS;
}
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_RunRequestStateMachine
 *---------------------------------------------------------------------------
 * PURPOSE  : Process state machine of a specified request.
 * INPUT    : request_index - Request index
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_RunRequestStateMachine(UI32_T request_index)
{
    RADIUS_OM_RequestState_T state;
    BOOL_T is_state_changed;
    BOOL_T is_destroy_flag;

    if (   (RADIUS_RETURN_SUCCESS == RADIUS_OM_GetRequestState(request_index,
            &state))
        && (RADIUS_RETURN_SUCCESS == RADIUS_OM_GetRequestDestroyFlag(
            request_index, &is_destroy_flag))
        && (TRUE == is_destroy_flag)
        )
    {
        RADIUS_OM_SetRequestState(request_index,
            RADIUS_REQUEST_STATE_CANCELLED);
    }

    do
    {
        is_state_changed = FALSE;

        if (FALSE == RADIUS_MGR_RunRequestStateMachineOneStep(request_index,
            &is_state_changed, &state))
        {
            break;;
        }

        if (   (FALSE == is_state_changed)
            && (RADIUS_REQUEST_STATE_DESTROY == state)
            )
        {
            break;
        }
    }
    while (TRUE == is_state_changed);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_IsSuccessCodeByRequestType
 *---------------------------------------------------------------------------
 * PURPOSE  : Check whether the code in the packet is success according to
 *            the request type. (ex: ACCEPT is success and REJECT is not for
 *            authentication. ACCOUNTING-RESPONSE is only success code for
 *            accounting, etc.)
 * INPUT    : type  - Request type
 *            code  - Code field in the packet
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_IsSuccessCodeByRequestType(
    RADIUS_OM_RadiusRequestType_T type, UI32_T code)
{
    switch (type)
    {
    case RADIUS_REQUEST_TYPE_IGMPAUTH:
    case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
    case RADIUS_REQUEST_TYPE_RADA_AUTH:
    case RADIUS_REQUEST_TYPE_WEB_AUTH:
    case RADIUS_REQUEST_TYPE_USER_AUTH:
        if (   (PW_ACCESS_ACCEPT == code)
            || (PW_ACCESS_CHALLENGE == code)
            )
        {
            return TRUE;
        }
        break;

    case RADIUS_REQUEST_TYPE_ACCOUNTING:
        if (PW_ACCOUNTING_RESPONSE == code)
        {
            return TRUE;
        }
        break;

    default:
        break;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_OpenIdQueueSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Open socket for the ID queue.
 * INPUT    : type          - Request type
 * OUTPUT   : socket_id_p   - Socket ID
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_OpenIdQueueSocket(
    RADIUS_OM_RadiusRequestType_T type, int *socket_id_p)
{
    int socket_id;

    if (NULL == socket_id_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id);
    }
    else
    {
        RADIUS_OM_GetAcctReqIdQueueSocketId(&socket_id);
    }

    /* Not to allocate again if already done before.
     */
    if (0 < socket_id)
    {
        *socket_id_p = socket_id;
        return RADIUS_RETURN_SUCCESS;
    }

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_id < 0)
    {
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        RADIUS_OM_SetAuthReqIdQueueSocketId(socket_id);
    }
    else
    {
        RADIUS_OM_SetAcctReqIdQueueSocketId(socket_id);
    }

    *socket_id_p = socket_id;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_CloseIdQueueSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Close socket for the ID queue.
 * INPUT    : type  - Request type
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_CloseIdQueueSocket(
    RADIUS_OM_RadiusRequestType_T type)
{
    int socket_id;

    /* Close the socket if it doesn't used by any reuqest.
     */
    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id);
        if (0 >= socket_id)
        {
            return RADIUS_RETURN_SUCCESS;
        }

        if (FALSE == RADIUS_OM_HasAnyRequestInAuthReqIdQueue())
        {
            s_close(socket_id);
            RADIUS_OM_SetAuthReqIdQueueSocketId(-1);
        }
    }
    else
    {
        RADIUS_OM_GetAcctReqIdQueueSocketId(&socket_id);
        if (0 >= socket_id)
        {
            return RADIUS_RETURN_SUCCESS;
        }

        if (FALSE == RADIUS_OM_HasAnyRequestInAcctReqIdQueue())
        {
            s_close(socket_id);
            RADIUS_OM_SetAcctReqIdQueueSocketId(-1);
        }
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_AddRequestIntoIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE  : Add request into the ID queue.
 * INPUT    : type          - Request type
 *            request_index - Request index
 * OUTPUT   : identifier_p  - Identifier
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_AddRequestIntoIdQueue(
    RADIUS_OM_RadiusRequestType_T type, UI32_T request_index,
    UI32_T *identifier_p)
{
    if (NULL == identifier_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        if (RADIUS_RETURN_FAIL == RADIUS_OM_AddRequestIntoAuthReqIdQueue(
            request_index, identifier_p))
        {
            return RADIUS_RETURN_FAIL;
        }
    }
    else
    {
        if (RADIUS_RETURN_FAIL == RADIUS_OM_AddRequestIntoAcctReqIdQueue(
            request_index, identifier_p))
        {
            return RADIUS_RETURN_FAIL;
        }
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_DeleteRequestFromIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE  : Delete request from the ID queue.
 * INPUT    : type          - Request type
 *            identifier    - Identifier
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_DeleteRequestFromIdQueue(
    RADIUS_OM_RadiusRequestType_T type, UI32_T identifier)
{
    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        RADIUS_OM_DeleteRequestFromAuthReqIdQueue(identifier);
    }
    else
    {
        RADIUS_OM_DeleteRequestFromAcctReqIdQueue(identifier);
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_RunRequestStateMachineOneStep
 *---------------------------------------------------------------------------
 * PURPOSE  : Process state machine of a specified request.
 * INPUT    : request_index             - Request index
 * OUTPUT   : ret_is_state_changed_p    - State is changed or not
 *            ret_new_state_p           - New state
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_RunRequestStateMachineOneStep(
    UI32_T request_index, BOOL_T *ret_is_state_changed_p,
    RADIUS_OM_RequestState_T *ret_new_state_p)
{
    UI32_T server_index;
    UI32_T user_index = 0;
    RADIUS_OM_RequestEntry_T request_data;
    RADIUS_OM_RequestState_T state;
    RADIUS_OM_RadiusRequestType_T type;
    BOOL_T is_state_changed = FALSE;
    BOOL_T is_auth_request = FALSE;

    if (   (NULL == ret_is_state_changed_p)
        || (NULL == ret_new_state_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestState(request_index,
            &state))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index, &type))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestServerIndex(request_index,
            &server_index))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestData(request_index,
            &request_data))
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    is_auth_request = (RADIUS_REQUEST_TYPE_ACCOUNTING != type) ? TRUE : FALSE;
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    if (FALSE == is_auth_request)
    {
        user_index = request_data.accounting_data.user_index;
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    switch (state)
    {
    case RADIUS_REQUEST_STATE_INIT:
    {
        RADIUS_MGR_TRACE("INIT state");

        if (RADIUS_RETURN_FAIL == RADIUS_MGR_InitRequestServerArray(
            request_index))
        {
            RADIUS_MGR_TRACE("INIT: Failed to initialize");

            state = RADIUS_REQUEST_STATE_FAIL;
        }
        else if (RADIUS_RETURN_FAIL == RADIUS_MGR_FindNextExistServerHost(
                 request_index))
        {
            RADIUS_MGR_TRACE("INIT: No any server so jump to DESTROY state");

            RADIUS_MGR_AnnounceResult(request_index, RADIUS_RESULT_TIMEOUT);

            state = RADIUS_REQUEST_STATE_DESTROY;
        }
        else
        {
            state = RADIUS_REQUEST_STATE_SEND;
        }

        is_state_changed = TRUE;
        RADIUS_OM_SetRequestState(request_index, state);

        break;
    }

    case RADIUS_REQUEST_STATE_SEND:
    {
        int socket_id;
        UI32_T current_time;
        UI32_T timeout;
        UI32_T retry_times;
        UI32_T identifier;
        UI8_T vector[AUTH_VECTOR_LEN];
        char secret_key[MAXSIZE_radiusServerGlobalKey + 1];

        RADIUS_MGR_TRACE("SEND state");

        if (RADIUS_RETURN_FAIL == RADIUS_MGR_OpenIdQueueSocket(type,
            &socket_id))
        {
            RADIUS_MGR_TRACE("SEND: Failed to create socket");
            break;
        }

        RADIUS_OM_GetRequestIdentifier(request_index, &identifier);

        /* Recycle previous identifier and change the next one.
         * Every time to send (or re-send) the packet whose packet content is
         * different (vector, etc.) so that need to change a new one.
         */
        if (RADIUS_OM_INVALID_IDENTIFIER != identifier)
        {
            RADIUS_MGR_DeleteRequestFromIdQueue(type, identifier);
        }
        if (RADIUS_RETURN_FAIL == RADIUS_MGR_AddRequestIntoIdQueue(type,
            request_index, &identifier))
        {
            RADIUS_MGR_TRACE("SEND: Failed to add an identifier");
            break;
        }
        RADIUS_OM_SetRequestIdentifier(request_index, identifier);

        RADIUS_OM_GetRequestRetryTimes(request_index, &retry_times);

        if (OK_RC != SENDSERVER_SendRequestPacket(request_index, socket_id,
            vector, secret_key, &timeout))
        {
            RADIUS_MGR_TRACE("SEND: Failed to send packet");

            if (RADIUS_RETURN_SUCCESS == RADIUS_MGR_FindNextExistServerHost(
                request_index))
            {
                RADIUS_MGR_TRACE("SEND: Get next server");

                if (retry_times > 0)
                {
                    if (TRUE == is_auth_request)
                    {
                        RADIUS_OM_IncreaseServerAccessRetransmissionsCounter(
                            server_index);
                    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
                    else
                    {
                        RADIUS_OM_IncAccClientRetransmissions(server_index, 1);
                    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
                }

                state = RADIUS_REQUEST_STATE_SEND;
                is_state_changed = TRUE;
                RADIUS_OM_SetRequestState(request_index, state);
            }
            else
            {
                RADIUS_MGR_TRACE("SEND: No next server so jump to DESTROY state");

                RADIUS_MGR_AnnounceResult(request_index, RADIUS_RESULT_TIMEOUT);

                state = RADIUS_REQUEST_STATE_DESTROY;
                is_state_changed = TRUE;
                RADIUS_OM_SetRequestState(request_index, state);
            }
        }
        else
        {
            RADIUS_MGR_TRACE("SEND: Send success");

            if (FALSE == request_data.is_wait_for_response)
            {
                RADIUS_MGR_TRACE("SEND: Not wait for response so jump to SUCCESS state");

                state = RADIUS_REQUEST_STATE_SUCCESS;
                is_state_changed = TRUE;
                RADIUS_OM_SetRequestState(request_index, state);

                break;
            }

            RADIUS_OM_SetRequestLastSentTime(request_index,
                SYS_TIME_GetSystemTicksBy10ms());

            SYS_TIME_GetRealTimeBySec(&current_time);
            RADIUS_OM_SetRequestTimeout(request_index, current_time + timeout);
            RADIUS_OM_ResortRequestByTimeoutOrder(request_index);

            RADIUS_OM_SetRequestVector(request_index, vector);
            RADIUS_OM_SetRequestSecretKey(request_index, secret_key);

            RADIUS_OM_SetRequestPendingRequestCounterFlag(request_index, TRUE);

            if (TRUE == is_auth_request)
            {
                if (0 == retry_times)
                {
                    RADIUS_OM_IncreaseServerAccessRequestCounter(server_index);
                }

                RADIUS_MGR_IncreasePendingRequestCounter(server_index);
            }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
            else
            {
                RADIUS_OM_IncAccClientPendingRequests(server_index, 1);
            }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

            state = RADIUS_REQUEST_STATE_WAIT_RESPONSE;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }

        break;
    }

    case RADIUS_REQUEST_STATE_WAIT_RESPONSE:
    {
        UI32_T current_time;
        UI32_T timeout;
        BOOL_T has_reply_flag;
        UI8_T *response_packet_p;
        UI32_T response_packet_len;

        RADIUS_MGR_TRACE("WAIT_RESPONSE state");

        RADIUS_OM_GetRequestResponsePacket(request_index, &response_packet_p,
            &response_packet_len);

        if (NULL != response_packet_p)
        {
            RADIUS_MGR_TRACE("WAIT_RESPONSE: Response is available");

            state = RADIUS_REQUEST_STATE_RECEIVE;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }
        else
        {
            SYS_TIME_GetRealTimeBySec(&current_time);
            RADIUS_OM_GetRequestTimeout(request_index, &timeout);

            if (current_time >= timeout)
            {
                UI32_T retry_times = 0;
                UI32_T server_index;
                RADIUS_Server_Host_T server_host;

                RADIUS_OM_GetRequestRetryTimes(request_index, &retry_times);
                retry_times++;
                RADIUS_OM_SetRequestRetryTimes(request_index, retry_times);

                RADIUS_OM_GetRequestServerIndex(request_index, &server_index);
                RADIUS_OM_Get_Server_Host(server_index, &server_host);

                /* Check with (retry_times - 1) because first time to send
                 * packet is not retry.
                 */
                if ((retry_times - 1) >= server_host.retransmit)
                {
                    RADIUS_MGR_TRACE("WAIT_RESPONSE: Timeout");

                    state = RADIUS_REQUEST_STATE_TIMEOUT;
                    is_state_changed = TRUE;
                    RADIUS_OM_SetRequestState(request_index, state);
                }
                else
                {
                    RADIUS_MGR_TRACE("WAIT_RESPONSE: Retransmit (%lu) times",
                        retry_times);

                    state = RADIUS_REQUEST_STATE_SEND;
                    is_state_changed = TRUE;
                    RADIUS_OM_SetRequestState(request_index, state);
                }
            }
        }

        break;
    }

    case RADIUS_REQUEST_STATE_RECEIVE:
    {
        int rc;
        UI32_T server_index;
        UI32_T last_sent_time;
        UI32_T code;
        RADIUS_Server_Host_T server_host;
        RADIUSCLIENT_ResponsePacketErrorType_T error;
        UI8_T *response_packet_p;
        UI32_T response_packet_len;

        RADIUS_MGR_TRACE("RECEIVE");

        RADIUS_OM_GetRequestServerIndex(request_index, &server_index);
        RADIUS_OM_Get_Server_Host(server_index, &server_host);
        RADIUS_OM_SetRequestResultServerIp(request_index,
            server_host.server_ip);
        RADIUS_OM_GetRequestResponsePacket(request_index, &response_packet_p,
            &response_packet_len);

        if (NULL == response_packet_p)
        {
            state = RADIUS_REQUEST_STATE_DESTROY;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);

            break;
        }

        rc = SENDSERVER_ProcessReceivedPacket(request_index,
            response_packet_p, response_packet_len, &code,
            &error);

        RADIUS_OM_SetRequestPendingRequestCounterFlag(request_index, FALSE);
        RADIUS_OM_GetRequestLastSentTime(request_index, &last_sent_time);

        if (TRUE == is_auth_request)
        {
            RADIUS_MGR_IncreaseAccessCounterByCode(server_index, code);
            RADIUS_MGR_DecreasePendingRequestCounter(server_index);
            RADIUS_OM_SetServerRoundTripTime(server_index,
                SYS_TIME_GetSystemTicksBy10ms() - last_sent_time);
        }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        else
        {
            RADIUS_OM_IncAccClientPendingRequests(server_index, -1);
            RADIUS_OM_IncAccClientResponses(server_index, 1);
            RADIUS_OM_SetAccClientRoundTripTime(server_index,
                SYS_TIME_GetSystemTicksBy10ms() - last_sent_time);
        }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        RADIUS_MGR_IncreaseErrorCounterByErrorType(server_index,
            is_auth_request, error);

        if (   (OK_RC == rc)
            && (TRUE == RADIUS_MGR_IsSuccessCodeByRequestType(type, code))
            )
        {
            RADIUS_MGR_TRACE("RECEIVE: Packet is success so jump to SUCCESS state");

            state = RADIUS_REQUEST_STATE_SUCCESS;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }
        else
        {
            RADIUS_MGR_TRACE("RECEIVE: Packet is fail so jump to FAIL state");

            state = RADIUS_REQUEST_STATE_FAIL;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }

        break;
    }

    case RADIUS_REQUEST_STATE_TIMEOUT:
    {
        UI32_T server_index;

        RADIUS_MGR_TRACE("TIMEOUT state");

        RADIUS_OM_GetRequestServerIndex(request_index, &server_index);

        RADIUS_OM_SetRequestPendingRequestCounterFlag(request_index, FALSE);

        if (TRUE == is_auth_request)
        {
            RADIUS_OM_IncreaseServerTimeoutCounter(server_index);
        }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        else
        {
            RADIUS_OM_IncAccClientTimeouts(server_index, 1);
        }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        if (RADIUS_RETURN_SUCCESS == RADIUS_MGR_FindNextExistServerHost(
            request_index))
        {
            RADIUS_MGR_TRACE("TIMEOUT: Success to get next server so jump to SEND state");

            RADIUS_OM_SetRequestRetryTimes(request_index, 0);

            state = RADIUS_REQUEST_STATE_SEND;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }
        else
        {
            RADIUS_MGR_TRACE("TIMEOUT: No next server so jump to DESTROY state");

            RADIUS_MGR_AnnounceResult(request_index, RADIUS_RESULT_TIMEOUT);

            state = RADIUS_REQUEST_STATE_DESTROY;
            is_state_changed = TRUE;
            RADIUS_OM_SetRequestState(request_index, state);
        }

        break;
    }

    case RADIUS_REQUEST_STATE_SUCCESS:
    {
        RADIUS_MGR_TRACE("SUCCESS state");

        RADIUS_MGR_AnnounceResult(request_index, RADIUS_RESULT_SUCCESS);

        state = RADIUS_REQUEST_STATE_DESTROY;
        is_state_changed = TRUE;
        RADIUS_OM_SetRequestState(request_index, state);

        break;
    }

    case RADIUS_REQUEST_STATE_FAIL:
    {
        RADIUS_MGR_TRACE("FAIL state");

        RADIUS_MGR_AnnounceResult(request_index, RADIUS_RESULT_FAIL);

        state = RADIUS_REQUEST_STATE_DESTROY;
        is_state_changed = TRUE;
        RADIUS_OM_SetRequestState(request_index, state);

        break;
    }

    case RADIUS_REQUEST_STATE_CANCELLED:
    {
        BOOL_T flag;

        RADIUS_MGR_TRACE("CANCELLED state");

        RADIUS_OM_GetRequestPendingRequestCounterFlag(request_index, &flag);
        if (TRUE == flag)
        {
            UI32_T server_index;

            RADIUS_OM_GetRequestServerIndex(request_index, &server_index);

            if (TRUE == is_auth_request)
            {
                RADIUS_MGR_DecreasePendingRequestCounter(server_index);
            }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
            else
            {
                RADIUS_OM_IncAccClientPendingRequests(server_index, -1);
            }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
        }

        state = RADIUS_REQUEST_STATE_DESTROY;
        is_state_changed = TRUE;
        RADIUS_OM_SetRequestState(request_index, state);
        break;
    }

    case RADIUS_REQUEST_STATE_DESTROY:
    {
        UI32_T identifier;
        UI8_T *response_packet_p = NULL;
        UI32_T response_packet_len;

        RADIUS_MGR_TRACE("DESTROY state");

        if (   (RADIUS_RETURN_SUCCESS == RADIUS_OM_GetRequestIdentifier(
                request_index, &identifier))
            && (RADIUS_OM_INVALID_IDENTIFIER != identifier)
            )
        {
            RADIUS_MGR_DeleteRequestFromIdQueue(type, identifier);
        }

        RADIUS_OM_GetRequestResponsePacket(request_index, &response_packet_p,
            &response_packet_len);
        if (NULL != response_packet_p)
        {
            L_MM_Free(response_packet_p);
        }

        RADIUS_OM_DeleteRequest(request_index);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        if (FALSE == is_auth_request)
        {
            RADIUS_OM_SetAccUserEntryRequestIndex(user_index, 0);
        }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        RADIUS_MGR_CloseIdQueueSocket(type);

        break;
    }

    default:
        RADIUS_MGR_TRACE("Unrecognized state %d", state);

        return RADIUS_RETURN_FAIL;
    }

    *ret_is_state_changed_p = is_state_changed;
    *ret_new_state_p = state;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_IncreaseAccessCounterByCode
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the access related counter in MIB by code.
 * INPUT    : server_index  - RADIUS server index
 *            code          - Code in packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_IncreaseAccessCounterByCode(UI32_T server_index,
    UI32_T code)
{
    RADIUS_Server_Host_T *server_host_p;

    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host_p)
    {
        return;
    }

    switch (code)
    {
    case PW_ACCESS_ACCEPT:
        RADIUS_OM_MIB_Set_AccessAcceptsCounter(RADIUS_OM_MIB_Get_AccessAcceptsCounter() + 1);
        server_host_p->server_table.radiusAuthClientAccessAccepts++;
        break;

    case PW_ACCESS_REJECT:
        RADIUS_OM_MIB_Set_AccessRejectsCounter(RADIUS_OM_MIB_Get_AccessRejectsCounter() + 1);
        server_host_p->server_table.radiusAuthClientAccessRejects++;
        break;

    case PW_ACCESS_CHALLENGE:
        RADIUS_OM_MIB_Set_AccessChallengesCounter(RADIUS_OM_MIB_Get_AccessChallengesCounter() + 1);
        server_host_p->server_table.radiusAuthClientAccessChallenges++;
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_IncreasePendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the pending request counter in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_IncreasePendingRequestCounter(UI32_T server_index)
{
    RADIUS_OM_MIB_Set_PendingRequestsCounter(RADIUS_OM_MIB_Get_PendingRequestsCounter() + 1);
    RADIUS_OM_IncreaseServerPendingRequestCounter(server_index);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_DecreasePendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Decrease the pending request counter in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_DecreasePendingRequestCounter(UI32_T server_index)
{
    RADIUS_OM_MIB_Set_PendingRequestsCounter(RADIUS_OM_MIB_Get_PendingRequestsCounter() - 1);
    RADIUS_OM_DecreaseServerPendingRequestCounter(server_index);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_IncreaseErrorCounterByErrorType
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the error-related counter in MIB and server table in
 *            MIB according to error type.
 * INPUT    : server_index      - RADIUS server index
 *            is_auth_request   - Authentication request or not (accounting)
 *            error             - Error type
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_IncreaseErrorCounterByErrorType(UI32_T server_index,
    BOOL_T is_auth_request, RADIUSCLIENT_ResponsePacketErrorType_T error)
{
    RADIUS_Server_Host_T *server_host_p;

    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host_p)
    {
        return;
    }

    if (TRUE == is_auth_request)
    {
        switch (error)
        {
        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_NONE:
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE:
            RADIUS_OM_MIB_Set_UnknownTypesCounter(RADIUS_OM_MIB_Get_UnknownTypesCounter() + 1);
            server_host_p->server_table.radiusAuthClientUnknownTypes++;
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_BAD_AUTHENTICATOR:
            RADIUS_OM_MIB_Set_BadAuthenticatorsCounter(RADIUS_OM_MIB_Get_BadAuthenticatorsCounter() + 1);
            server_host_p->server_table.radiusAuthClientBadAuthenticators++;
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_SEQ_NBR:
            RADIUS_OM_MIB_Set_PacketsDroppedCounter(RADIUS_OM_MIB_Get_PacketsDroppedCounter() + 1);
            server_host_p->server_table.radiusAuthClientPacketsDropped++;
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_LENGTH:
            RADIUS_OM_MIB_Set_MalformedAccessResponsesCounter(RADIUS_OM_MIB_Get_MalformedAccessResponsesCounter() + 1);
            server_host_p->server_table.radiusAuthClientMalformedAccessResponses++;
            break;

        default:
            break;
        }
    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    else
    {
        switch (error)
        {
        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_NONE:
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE:
            RADIUS_OM_IncAccClientUnknownTypes(server_index, 1);
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_BAD_AUTHENTICATOR:
            RADIUS_OM_IncAccClientBadAuthenticators(server_index, 1);
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_SEQ_NBR:
            RADIUS_OM_IncAccClientPacketsDropped(server_index, 1);
            break;

        case RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_LENGTH:
            RADIUS_OM_IncAccClientMalformedResponses(server_index, 1);
            break;

        default:
            break;
        }
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_NotifyMonitorTaskWatchSockets
 *---------------------------------------------------------------------------
 * PURPOSE  : Notify monitor task with sockets of authentication and
 *            accounting and specified time (notifies when expires).
 * INPUT    : socket_id  - Socket to send command for notify
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_NotifyMonitorTaskWatchSockets(int socket_id)
{
    RADIUS_MGR_MonitorCommandMsg_T notify_msg;
    int auth_socket_id = -1;
    int acct_socket_id = -1;
    int ret_size;
    UI32_T next_request_timeout;
    UI32_T next_accounting_timeout = RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME;
    UI32_T smallest_timeout;
    UI32_T timeout;
    UI32_T current_time;

    RADIUS_MGR_TRACE("");

    RADIUS_OM_GetAuthReqIdQueueSocketId(&auth_socket_id);
    RADIUS_OM_GetAcctReqIdQueueSocketId(&acct_socket_id);

    if (   (FALSE == RADIUS_OM_HasAnyRequestEntry())
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        && (FALSE == RADIUS_OM_HasAnyAccUserEntry())
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
        && (-1 == auth_socket_id)
        && (-1 == acct_socket_id)
        )
    {
        RADIUS_MGR_TRACE("Nothing need to notify. Stop.");

        return TRUE;
    }

    RADIUS_OM_GetRecentRequestTimeout(&next_request_timeout);
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    RADIUS_OM_GetRecentAccUserEntryTimeout(&next_accounting_timeout);
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
    SYS_TIME_GetRealTimeBySec(&current_time);
    smallest_timeout = MIN(next_request_timeout, next_accounting_timeout);
    timeout = (smallest_timeout > current_time) ?
        (smallest_timeout - current_time) : 0;

    notify_msg.command = RADIUS_MGR_MONITOR_COMMAND_WATCH;
    notify_msg.data.u.watch.auth_socket_id = auth_socket_id;
    notify_msg.data.u.watch.acct_socket_id = acct_socket_id;
    notify_msg.data.u.watch.timeout = timeout;

    RADIUS_MGR_TRACE(
        "Monitor sender socket = %d, auth/acct socket = %d/%d, timeout = %lu",
        socket_id, auth_socket_id, acct_socket_id, smallest_timeout);

    /* Write data to monitor socket (pipe) to notify the new event.
     */
    ret_size = write(socket_id, &notify_msg, sizeof(notify_msg));
    if (sizeof(notify_msg) != ret_size)
    {
        RADIUS_MGR_TRACE("Write insufficient size (%d). Correct msg size = %d",
            ret_size, sizeof(notify_msg));

        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_NotifyMonitorTaskStopMonitor
 *---------------------------------------------------------------------------
 * PURPOSE : Notify monitor task to stop monitor.
 * INPUT   : socket_id  - Socket to send command for notify
 * OUTPUT  : None
 * RETUEN  : TRUE/FALSE
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_NotifyMonitorTaskStopMonitor(int socket_id)
{
    int ret;
    RADIUS_MGR_MonitorCommandMsg_T notify_msg;

    notify_msg.command = RADIUS_MGR_MONITOR_COMMAND_STOP_MONITOR;

    ret = write(socket_id, &notify_msg, sizeof(notify_msg));

    RADIUS_MGR_TRACE("Send command %d bytes", ret);

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SelectMonitorSockets
 *---------------------------------------------------------------------------
 * PURPOSE  : Select on RADIUS monitor sockets (control, authentication and
 *            accounting) with specified timeout.
 * INPUT    : auth_socket_id            - Authentication socket ID
 *            acct_socket_id            - Accounting socket ID
 *            monitor_socket_id         - Monitor socket ID
 *            auth_and_acct_timeout     - Timeout in seconds
 * OUTPUT   : result_p                  - Result of monitor
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SelectMonitorSockets(int auth_socket_id,
    int acct_socket_id, int monitor_socket_id, UI32_T auth_and_acct_timeout,
    RADIUS_MGR_MONITOR_RESULT_T *result_p)
{
    int max_socket = 0;
    int ret;
    fd_set fdset;
    struct timeval timeout;
    struct timeval *timeout_p;
    BOOL_T is_auth_or_acct_socket_set = FALSE;

    RADIUS_MGR_TRACE("Auth socket = %d, acct socket = %d, monitor socket = %d, timeout = %lu",
        auth_socket_id, acct_socket_id, monitor_socket_id,
        auth_and_acct_timeout);

    if (NULL == result_p)
    {
        return FALSE;
    }

    FD_ZERO(&fdset);

    FD_SET(monitor_socket_id, &fdset);
    max_socket = monitor_socket_id;

    if (-1 != auth_socket_id)
    {
        RADIUS_MGR_TRACE("Select on auth socket");

        FD_SET(auth_socket_id, &fdset);
        max_socket = MAX(max_socket, auth_socket_id);

        is_auth_or_acct_socket_set = TRUE;
    }

    if (-1 != acct_socket_id)
    {
        RADIUS_MGR_TRACE("Select on acct socket");

        FD_SET(acct_socket_id, &fdset);
        max_socket = MAX(max_socket, acct_socket_id);

        is_auth_or_acct_socket_set = TRUE;
    }

    if (TRUE == is_auth_or_acct_socket_set)
    {
        timeout.tv_sec = auth_and_acct_timeout;
        timeout.tv_usec = 0;

        timeout_p = &timeout;

        RADIUS_MGR_TRACE("Select for %lu seconds", timeout.tv_sec);
    }
    else
    {
        RADIUS_MGR_TRACE("Select forever");

        timeout_p = NULL;
    }

    ret = select(max_socket + 1, &fdset, NULL, NULL, timeout_p);
    while (ret < 0)
    {
        if (EINTR != errno)
        {
            return FALSE;
        }

        ret = select(max_socket + 1, &fdset, NULL, NULL, timeout_p);
    }

    if (FD_ISSET(monitor_socket_id, &fdset))
    {
        RADIUS_MGR_TRACE("Result: Monitor command is available");

        *result_p = RADIUS_MGR_MONITOR_RESULT_COMMAND;
    }
    else if (   (-1 != auth_socket_id)
             && (FD_ISSET(auth_socket_id, &fdset))
             )
    {
        RADIUS_MGR_TRACE("Result: Authentication socket is available");

        *result_p = RADIUS_MGR_MONITOR_RESULT_AUTH_SOCKET_AVAILABLE;
    }
    else if (   (-1 != acct_socket_id)
             && (FD_ISSET(acct_socket_id, &fdset))
             )
    {
        RADIUS_MGR_TRACE("Result: Accounting socket is available");

        *result_p = RADIUS_MGR_MONITOR_RESULT_ACCT_SOCKET_AVAILABLE;
    }
    else
    {
        RADIUS_MGR_TRACE("Result: Select is timeout");

        *result_p = RADIUS_MGR_MONITOR_RESULT_TIMEOUT;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ReceiveMonitorCommand
 *---------------------------------------------------------------------------
 * PURPOSE  : Receive monitor command from the specified socket.
 * INPUT    : socket_id     - Socket ID
 * OUTPUT   : context_p - Monitor context
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_ReceiveMonitorCommand(int socket_id,
    RADIUS_MGR_MonitorContext_T *context_p)
{
    int ret;

    RADIUS_MGR_TRACE("");

    if (NULL == context_p)
    {
        return FALSE;
    }

    ret = read(socket_id,
        ((UI8_T *)&context_p->command) + context_p->command_len,
        sizeof(context_p->command) - context_p->command_len);
    if (ret <= 0)
    {
        return FALSE;
    }

    context_p->command_len += ret;

    RADIUS_MGR_TRACE("Read %d bytes. Total read command len = %lu",
        ret, context_p->command_len);

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorCommand
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor command and extract it into context.
 * INPUT    : context_p - Monitor context
 * OUTPUT   : context_p - Monitor context
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_ProcessMonitorCommand(
    RADIUS_MGR_MonitorContext_T *context_p)
{
    if (NULL == context_p)
    {
        return FALSE;
    }

    switch (context_p->command.command)
    {
    case RADIUS_MGR_MONITOR_COMMAND_WATCH:
        context_p->auth_socket_id = context_p->command.data.u.watch.auth_socket_id;
        context_p->acct_socket_id = context_p->command.data.u.watch.acct_socket_id;
        context_p->timeout = context_p->command.data.u.watch.timeout;
        break;

    case RADIUS_MGR_MONITOR_COMMAND_STOP_MONITOR:
        context_p->is_to_stop_monitor = TRUE;
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ReceivePacketFromSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Receive the response packet from socket.
 * INPUT    : socket_id     - Socket ID
 * OUTPUT   : packet_pp     - Received packet
 *            packet_len_p  - Length of received packet
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_ReceivePacketFromSocket(int socket_id,
    UI8_T **packet_pp, UI32_T *packet_len_p)
{
#define PARTIAL_HEADER_LEN  4   /* first 4 bytes include "code", "packet identifier" and "length" */

    int ret;
    UI8_T *recv_packet_p;
    UI8_T partial_packet[PARTIAL_HEADER_LEN];
    UI32_T packet_len;

    RADIUS_MGR_TRACE("");

    if (   (NULL == packet_pp)
        || (NULL == packet_len_p)
        )
    {
        return FALSE;
    }

    ret = recv(socket_id, partial_packet, PARTIAL_HEADER_LEN, MSG_PEEK);
    if (ret < PARTIAL_HEADER_LEN)
    {
        RADIUS_MGR_TRACE("Error when to receive header (ret = %d)", ret);

        return FALSE;
    }

    RADIUS_MGR_TRACE("Received bytes of partial header = %d", ret);

    /* Check length field of the received header
     */
    packet_len = partial_packet[2] << 8 | partial_packet[3];
    if (packet_len < AUTH_HDR_LEN)
    {
        RADIUS_MGR_TRACE("Header length (%lu) is less than expect (%d)",
            packet_len, AUTH_HDR_LEN);

        /* Receive again to discard the packet
         */
        ret = recv(socket_id, partial_packet, PARTIAL_HEADER_LEN, 0);

        return FALSE;
    }

    RADIUS_MGR_TRACE("Length field in header = %lu", packet_len);

    recv_packet_p = (UI8_T *)L_MM_Malloc(BUFFER_LEN,
        L_MM_USER_ID2(SYS_MODULE_RADIUS,
        RADIUS_TYPE_TRACE_ID_RADIUS_MGR_RECEIVEPACKETFROMSOCKET));
    if (NULL == recv_packet_p)
    {
        return FALSE;
    }

    ret = recv(socket_id, recv_packet_p, packet_len, 0);

    /* In here, we only try to receive packet according to length field of
     * RADIUS packet. The case of insufficient length or others are leave to
     * state machine.
     */
    if (ret < 0)
    {
        RADIUS_MGR_TRACE("Error when to receive body (ret = %d)", ret);

        L_MM_Free(recv_packet_p);

        return FALSE;
    }

    *packet_pp = recv_packet_p;
    *packet_len_p = ret;

    RADIUS_MGR_TRACE("Receive a packet (len = %lu bytes)", *packet_len_p);

    return TRUE;

#undef PARTIAL_HEADER_LEN
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessReceivedPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Process received packet.
 * INPUT    : recv_packet_p             - Received packet
 *            recv_packet_len           - Length of received packet
 *            get_id_from_id_queue_fn_p - Function to retreive id of state
 *                                        machine from id in packet
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_ProcessReceivedPacket(UI8_T *recv_packet_p,
    UI32_T recv_packet_len,
    RADIUS_ReturnValue_T(*get_id_from_id_queue_fn_p)(UI32_T id, UI32_T *request_index_p))
{
    UI32_T request_index;
    AUTH_HDR *auth_p;

    RADIUS_MGR_TRACE("");

    if (   (NULL == recv_packet_p)
        || (0 == recv_packet_len)
        || (NULL == get_id_from_id_queue_fn_p)
        )
    {
        return FALSE;
    }

    auth_p = (AUTH_HDR *)recv_packet_p;

    RADIUS_MGR_TRACE("Received packet id = %u", auth_p->id);

    if (RADIUS_RETURN_FAIL == get_id_from_id_queue_fn_p(auth_p->id,
        &request_index))
    {
        RADIUS_MGR_TRACE("Failed to get request ID from packet id");

        return FALSE;
    }

    if (RADIUS_RETURN_FAIL == RADIUS_OM_SetRequestResponsePacket(request_index,
        recv_packet_p, recv_packet_len))
    {
        RADIUS_MGR_TRACE("Failed to set response packet into state machine");

        return FALSE;
    }

    RADIUS_MGR_TRACE("Run state machine (id = %lu)", request_index);

    RADIUS_MGR_RunRequestStateMachine(request_index);

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessTimeoutRequests
 *---------------------------------------------------------------------------
 * PURPOSE  : Process timeout requests.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessTimeoutRequests()
{
    UI32_T current_time;
    UI32_T next_request_index;
    UI32_T request_index = 0;
    BOOL_T is_end_of_requests = FALSE;

    RADIUS_MGR_TRACE("");

    SYS_TIME_GetRealTimeBySec(&current_time);

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetNextRequestByTimeoutOrder(
        &request_index))
    {
        return;
    }

    while (1)
    {
        UI32_T timeout;

        /* Keeping next entry before to process here is to prevent from the
         * processing entry is re-sorted so that some entries may be ignored
         * which violates our expection.
         */
        next_request_index = request_index;
        if (RADIUS_RETURN_FAIL == RADIUS_OM_GetNextRequestByTimeoutOrder(
            &next_request_index))
        {
            is_end_of_requests = TRUE;
        }

        RADIUS_OM_GetRequestTimeout(request_index, &timeout);
        if (timeout > current_time)
        {
            break;
        }

        RADIUS_MGR_TRACE("Run state machine (id = %lu)", request_index);
        RADIUS_MGR_RunRequestStateMachine(request_index);

        if (TRUE == is_end_of_requests)
        {
            break;
        }

        request_index = next_request_index;
    }
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessTimeoutAccUsers
 *---------------------------------------------------------------------------
 * PURPOSE  : Process timeout accounting users.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessTimeoutAccUsers()
{
    UI32_T current_time;
    UI32_T next_user_index;
    UI32_T user_index = 0;
    BOOL_T is_end_of_requests = FALSE;

    SYS_TIME_GetRealTimeBySec(&current_time);

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetNextAccUserByTimeoutOrder(
        &user_index))
    {
        return;
    }

    while (1)
    {
        UI32_T timeout;
        RADACC_UserInfo_T user_entry;

        /* Keeping next entry before to process here is to prevent from the
         * processing entry is re-sorted so that some entries may be ignored
         * which violates our expection.
         */
        next_user_index = user_index;
        if (RADIUS_RETURN_FAIL == RADIUS_OM_GetNextAccUserByTimeoutOrder(
            &next_user_index))
        {
            is_end_of_requests = TRUE;
        }

        RADIUS_OM_GetAccUserEntryTimeout(user_index, &timeout);
        if (timeout > current_time)
        {
            break;
        }

        user_entry.user_index = user_index;
        if (TRUE == RADIUS_OM_GetAccUserEntry(&user_entry))
        {
            if (TRUE == user_entry.destroy_flag)
            {
                UI32_T request_index;

                /* Clear the request belonging to this user (if exist)
                 */
                RADIUS_OM_SetRequestDestroyFlag(user_entry.request_index, TRUE);
                RADIUS_MGR_RunRequestStateMachine(user_entry.request_index);

                if (RADIUS_RETURN_SUCCESS == RADIUS_MGR_CreateAccountingRequest(
                    RADACC_STOP, user_index, current_time, TRUE, &request_index))
                {
                    RADIUS_OM_SetAccUserEntryRequestIndex(user_index,
                        request_index);
                    RADIUS_MGR_RunRequestStateMachine(request_index);

                    /* Reset destroy flag and set time to maximum value to
                     * prevent trigger again during sending stop request
                     */
                    RADIUS_OM_SetAccUserEntryDestroyFlag(user_index, FALSE);
                    RADIUS_OM_SetAccUserEntryTimeout(user_entry.user_index,
                        RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME);
                    RADIUS_OM_ResortAccUserEntryByTimeoutOrder(user_entry.user_index);

                    /* User is already deleted in announce function during run
                     * state machine
                     */
                }
            }
            else if (0 == user_entry.request_index)
            {
                /* Only processes the user which doesn't hook any request yet
                 */
                UI32_T request_index;
                RADACC_AcctStatusType_T request_type;

                request_type = (0 == user_entry.ctrl_bitmap.start_packet_response) ?
                    RADACC_START : RADACC_InterimUpdate;

                if (RADIUS_RETURN_SUCCESS == RADIUS_MGR_CreateAccountingRequest(
                    request_type, user_index, current_time, TRUE,
                    &request_index))
                {
                    UI32_T request_timeout;

                    RADIUS_OM_SetAccUserEntryRequestIndex(user_index,
                        request_index);
                    RADIUS_MGR_RunRequestStateMachine(request_index);

                    /* Update timeout of the accounting user
                     */
                    RADIUS_OM_GetRequestTimeout(request_index, &request_timeout);
                    RADIUS_OM_SetAccUserEntryTimeout(user_index, request_timeout);
                    RADIUS_OM_ResortAccUserEntryByTimeoutOrder(user_index);
                }
            }
        }

        if (TRUE == is_end_of_requests)
        {
            break;
        }

        user_index = next_user_index;
    }
}
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorTimeoutEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor timeout event.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorTimeoutEvent()
{
    RADIUS_MGR_TRACE("");

    RADIUS_MGR_ProcessTimeoutRequests();

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    RADIUS_MGR_ProcessTimeoutAccUsers();
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorAuthSocketAvailableEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor event indicates that authentication socket is
 *            available now.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorAuthSocketAvailableEvent()
{
    int socket_id;
    UI8_T *packet_p = NULL;
    UI32_T packet_len;

    RADIUS_MGR_TRACE("");

    RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id);
    if (-1 == socket_id)
    {
        return;
    }

    if (FALSE == RADIUS_MGR_ReceivePacketFromSocket(socket_id, &packet_p,
        &packet_len))
    {
        return;
    }

    if (FALSE == RADIUS_MGR_ProcessReceivedPacket(packet_p, packet_len,
        RADIUS_OM_GetRequestByIdFromAuthReqIdQueue))
    {
        L_MM_Free(packet_p);
        return;
    }

    /* Leave response_packet_p be freed by state machine when it destroyed
     * by itself
     */
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorAcctSocketAvailableEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor event indicates that accounting socket is
 *            available now.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorAcctSocketAvailableEvent()
{
    int socket_id;
    UI8_T *packet_p = NULL;
    UI32_T packet_len;

    RADIUS_MGR_TRACE("");

    RADIUS_OM_GetAcctReqIdQueueSocketId(&socket_id);
    if (-1 == socket_id)
    {
        return;
    }

    if (FALSE == RADIUS_MGR_ReceivePacketFromSocket(socket_id, &packet_p,
        &packet_len))
    {
        return;
    }

    if (FALSE == RADIUS_MGR_ProcessReceivedPacket(packet_p, packet_len,
        RADIUS_OM_GetRequestByIdFromAcctReqIdQueue))
    {
        L_MM_Free(packet_p);
        return;
    }

    /* Leave response_packet_p be freed by state machine when it destroyed
     * by itself
     */
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessRequestFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE  : Process new submitted RADIUS requests from waiting queue.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessRequestFromWaitingQueue()
{
    RADIUS_OM_RequestEntry_T request_data;

    RADIUS_MGR_TRACE("");

    RADIUS_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    while (   (TRUE == RADIUS_OM_HasAnyFreeRequestEntry())
           && (RADIUS_RETURN_SUCCESS == RADIUS_OM_DeleteFirstAuthReqFromWaitingQueue(&request_data)))
    {
        switch (request_data.type)
        {
        case RADIUS_REQUEST_TYPE_IGMPAUTH:
        case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
        case RADIUS_REQUEST_TYPE_RADA_AUTH:
        case RADIUS_REQUEST_TYPE_WEB_AUTH:
        case RADIUS_REQUEST_TYPE_USER_AUTH:
        case RADIUS_REQUEST_TYPE_ACCOUNTING:
        {
            UI32_T request_index;

            RADIUS_OM_AddRequest(&request_data, &request_index);
            RADIUS_MGR_RunRequestStateMachine(request_index);
            break;
        }

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        case RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE:
        {
            UI32_T current_time;
            UI32_T request_index;
            UI32_T user_index;
            AAA_AccRequest_T *request_p;
            RADACC_UserInfo_T user_entry;
            SWDRV_IfTableStats_T if_table_stats;

            SYS_TIME_GetRealTimeBySec(&current_time);

            request_p = &request_data.accounting_create_data.request;

            if (TRUE == RADIUS_OM_GetAccUserEntryByKey(request_p->ifindex,
                request_p->user_name, request_p->client_type, &user_entry))
            {
                /* Clear the request belonging to this user (if exist)
                 */
                RADIUS_OM_SetRequestDestroyFlag(user_entry.request_index, TRUE);
                RADIUS_MGR_RunRequestStateMachine(user_entry.request_index);

                if (RADIUS_RETURN_FAIL == RADIUS_MGR_CreateAccountingRequest(
                    RADACC_STOP, user_entry.user_index, current_time, FALSE,
                    &request_index))
                {
                    RADIUS_OM_FreeAccUser(user_entry.user_index);
                }
                else
                {
                    RADIUS_OM_SetAccUserEntryTerminateCause(
                        user_entry.user_index, AAA_ACC_TERM_BY_NAS_REQUEST);
                    RADIUS_OM_SetAccUserEntryRequestIndex(user_entry.user_index,
                        request_index);
                    RADIUS_MGR_RunRequestStateMachine(request_index);
                }
            }

            user_index = RADIUS_OM_CreateAccUserEntry(request_p, current_time);
            if (0 == user_index)
            {
                /* Failed to create the user. Drop the request.
                 */
                continue;
            }

            /* Dot1x needs to collect and initialize statisti info
            */
            if (   (AAA_CLIENT_TYPE_DOT1X == request_p->client_type)
                && (TRUE == NMTR_PMGR_GetIfTableStats(request_p->ifindex,
                    &if_table_stats))
                )
            {
                user_entry.statistic_info.ifInOctets = if_table_stats.ifInOctets;
                user_entry.statistic_info.ifOutOctets = if_table_stats.ifOutOctets;
                user_entry.statistic_info.ifInUcastPkts = if_table_stats.ifInUcastPkts;
                user_entry.statistic_info.ifOutUcastPkts = if_table_stats.ifOutUcastPkts;

                RADIUS_OM_SetAccUserEntryStatisticInfo(user_index,
                    &user_entry.statistic_info);
            }

            /* Add first request (start) for this accounting user.
             */
            if (RADIUS_RETURN_SUCCESS == RADIUS_MGR_CreateAccountingRequest(
                RADACC_START, user_index, current_time, TRUE, &request_index))
            {
                RADIUS_OM_SetAccUserEntryRequestIndex(user_index, request_index);
                RADIUS_MGR_RunRequestStateMachine(request_index);
            }

            break;
        }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        default:
            /* Drop the invalid request.
             */
            continue;
        }
    }

    RADIUS_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_CreateAccountingRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : Create a new accounting request in the request queue.
 * INPUT    : request_type          - Request type
 *            user_index            - User index
 *            current_time          - Current time
 *            is_wait_for_response  - Wait for response or not
 * OUTPUT   : request_index_p   - Request index
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_CreateAccountingRequest(
    RADACC_AcctStatusType_T request_type, UI32_T user_index,
    UI32_T current_time, BOOL_T is_wait_for_response, UI32_T *request_index_p)
{
    RADIUS_OM_RequestEntry_T request_data;

    if (NULL == request_index_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    request_data.type = RADIUS_REQUEST_TYPE_ACCOUNTING;
    request_data.is_wait_for_response = is_wait_for_response;
    request_data.accounting_data.request_type = request_type;
    request_data.accounting_data.user_index = user_index;
    request_data.accounting_data.created_time = current_time;

    return RADIUS_OM_AddRequest(&request_data, request_index_p);
}
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_CloseAuthReqIdSocket
 *---------------------------------------------------------------------------
 * PURPOSE : Close socket used for authentication request ID queue.
 * INPUT   : None
 * OUTPUT  : None
 * RETUEN  : TRUE/FALSE
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_CloseAuthReqIdSocket()
{
    int socket_id;

    RADIUS_OM_GetAuthReqIdQueueSocketId(&socket_id);
    if (-1 != socket_id)
    {
        s_close(socket_id);
        RADIUS_OM_SetAuthReqIdQueueSocketId(-1);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_CloseAcctReqIdSocket
 *---------------------------------------------------------------------------
 * PURPOSE : Close socket used for accounting request ID queue.
 * INPUT   : None
 * OUTPUT  : None
 * RETUEN  : TRUE/FALSE
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_MGR_CloseAcctReqIdSocket()
{
    int socket_id;

    RADIUS_OM_GetAcctReqIdQueueSocketId(&socket_id);
    if (-1 != socket_id)
    {
        s_close(socket_id);
        RADIUS_OM_SetAcctReqIdQueueSocketId(-1);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_InitRequestServerArray
 *---------------------------------------------------------------------------
 * PURPOSE:  Initialize server array of the specified request.
 * INPUT:    request_index  - Request index
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_FAIL
 *           RADIUS_RETURN_SUCCESS
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_InitRequestServerArray(
    UI32_T request_index)
{
    RADIUS_OM_RadiusRequestType_T type;
    RADIUS_OM_ServerArray_T server_array;

    RADIUS_MGR_TRACE("");

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index, &type))
    {
        return RADIUS_RETURN_FAIL;
    }

    memset(&server_array, 0, sizeof(server_array));
    server_array.current_index = 0xFF;

    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        RADIUS_Server_Host_T host;
        UI32_T server_index = 0;

        RADIUS_MGR_TRACE("Index of authentication servers:");

        while (TRUE == RADIUS_OM_GetNext_Server_Host(&server_index, &host))
        {
            RADIUS_MGR_TRACE("%lu", server_index);

            server_array.servers[server_array.count] = server_index;
            server_array.count++;
        }

        RADIUS_OM_SetRequestServerArray(request_index, &server_array);
    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    else
    {
        AAA_QueryGroupIndexResult_T query_result;
        RADIUS_OM_RequestEntry_T request_data;
        RADACC_UserInfo_T user_entry;
        AAA_RadiusEntryInterface_T aaa_radius_entry;

        if (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestData(request_index,
            &request_data))
        {
            return RADIUS_RETURN_FAIL;
        }

        user_entry.user_index = request_data.accounting_data.user_index;
        if (   (FALSE == RADIUS_OM_GetAccUserEntry(&user_entry))
            || (FALSE == AAA_MGR_GetAccountingGroupIndex(user_entry.client_type,
                user_entry.ifindex, &query_result))
            )
        {
            return RADIUS_RETURN_FAIL;
        }

        RADIUS_MGR_TRACE("Grou index = %u, group type = %d",
            query_result.group_index, query_result.group_type);
        RADIUS_MGR_TRACE("Index of accounting servers:");

        aaa_radius_entry.radius_index = 0;
        while (TRUE == AAA_OM_GetNextRadiusEntry(query_result.group_index, &aaa_radius_entry))
        {
            RADIUS_MGR_TRACE("%lu", aaa_radius_entry.radius_server_index);

            server_array.servers[server_array.count] = aaa_radius_entry.radius_server_index;
            server_array.count++;
        }

        RADIUS_OM_SetRequestServerArray(request_index, &server_array);
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_FindNextExistServerHost
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will find the next exist server host
 * INPUT:    request_index  - Request index
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_FAIL
 *           RADIUS_RETURN_SUCCESS
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_MGR_FindNextExistServerHost(
    UI32_T request_index)
{
    UI32_T server_index;
    RADIUS_OM_ServerArray_T server_array;
    RADIUS_Server_Host_T server_host;
    RADIUS_ReturnValue_T ret = RADIUS_RETURN_SUCCESS;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestServerArray(request_index,
        &server_array))
    {
        return RADIUS_RETURN_FAIL;
    }

    while (TRUE)
    {
        if (0xFF == server_array.current_index)
        {
            if (0 == server_array.count)
            {
                ret = RADIUS_RETURN_FAIL;
                break;
            }

            server_array.current_index = 0;
        }
        else
        {
            if (server_array.current_index + 1 > server_array.count)
            {
                ret = RADIUS_RETURN_FAIL;
                break;
            }

            server_array.current_index += 1;
        }

        server_index = server_array.servers[server_array.current_index] - 1;

        if (TRUE == RADIUS_OM_Get_Server_Host(server_index, &server_host))
        {
            RADIUS_OM_SetRequestServerIndex(request_index, server_index);
            break;
        }
    }

    RADIUS_OM_SetRequestServerArray(request_index, &server_array);

    return ret;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_HandleIPCReqMsg
 *---------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RADIUS mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_IPC_RESULT_FAIL;
        ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch(cmd=RADIUS_MGR_MSG_CMD(ipcmsg_p))
    {
        case RADIUS_MGR_IPC_CMD_SET_REQUEST_TIMEOUT:
        {
            RADIUS_MGR_IPCMsg_RequestTimeout_T *data_p = (RADIUS_MGR_IPCMsg_RequestTimeout_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Request_Timeout(data_p->timeval);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_SET_SERVER_PORT:
        {
            RADIUS_MGR_IPCMsg_ServerPort_T *data_p = (RADIUS_MGR_IPCMsg_ServerPort_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Server_Port(data_p->serverport);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_SET_SERVER_SECRET:
        {
            RADIUS_MGR_IPCMsg_ServerSecret_T *data_p = (RADIUS_MGR_IPCMsg_ServerSecret_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Server_Secret(data_p->serversecret);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_SET_RETRANSMIT_TIMES:
        {
            RADIUS_MGR_IPCMsg_RetransmitTimes_T *data_p = (RADIUS_MGR_IPCMsg_RetransmitTimes_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Retransmit_Times(data_p->retryval);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_SET_SERVER_IP:
        {
            RADIUS_MGR_IPCMsg_ServerIP_T *data_p = (RADIUS_MGR_IPCMsg_ServerIP_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Server_IP(data_p->serverip);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_AUTH_CHECK:
        {
            RADIUS_MGR_IPCMsg_AuthCheck_T *data_p = (RADIUS_MGR_IPCMsg_AuthCheck_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            /*maggie liu for RADIUS authentication ansync*/
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_AsyncRadiusAuthCheck(data_p->username, data_p->password, &data_p->privilege, ipcmsg_p->msg_type);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_ASYNC_EAP_AUTH_CHECK:
        {
            RADIUS_MGR_IPCMsg_AsyncAuthCheck_T *data_p = (RADIUS_MGR_IPCMsg_AsyncAuthCheck_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_AsyncEapAuthCheck(
                                                               data_p->eap_data,
                                                            data_p->eap_datalen,
                                                              data_p->radius_id,
                                                             data_p->state_data,
                                                          data_p->state_datalen,
                                                               data_p->src_port,
                                                                data_p->src_mac,
                                                                data_p->src_vid,
                                                                 data_p->cookie,
                                                           data_p->service_type,
                                                              data_p->server_ip,
                                                               data_p->username,
                                                                  data_p->flag);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_SET_SERVER_HOST:
        {
            RADIUS_MGR_IPCMsg_SetServerHost_T *data_p = (RADIUS_MGR_IPCMsg_SetServerHost_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Set_Server_Host(data_p->server_index, &data_p->server_host);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_MGR_IPC_CMD_DESTROY_SERVER_HOST:
        {
            RADIUS_MGR_IPCMsg_DestroyServerHost_T *data_p = (RADIUS_MGR_IPCMsg_DestroyServerHost_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_Destroy_Server_Host(data_p->server_index);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        case RADIUS_MGR_IPC_CMD_GET_SERVER_ACCT_PORT:
        {
            RADIUS_MGR_IPCMsg_ServerAcctPort_T *data_p = (RADIUS_MGR_IPCMsg_ServerAcctPort_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_SetServerAcctPort(data_p->acct_port);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif
        case RADIUS_MGR_IPC_CMD_RADA_AUTH_CHECK:
        {
            RADIUS_MGR_IPCMsg_RadaAuthCheck_T *data_p = (RADIUS_MGR_IPCMsg_RadaAuthCheck_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_RadaAuthCheck(
                    data_p->src_lport, data_p->src_mac,
                    data_p->rada_username, data_p->rada_password, data_p->cookie);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        case RADIUS_MGR_IPC_CMD_GET_ACC_CLIENT_IDENTIFIER:
        {
            RADACC_AccClientIdentifier_T *data_p = (RADACC_AccClientIdentifier_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_GetAccClientIdentifier(data_p);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE(RADACC_AccClientIdentifier_T);
            break;
        }
#endif

        case RADIUS_MGR_IPC_CMD_ASYNC_LOGIN_AUTH:
        {
            RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T *data_p = (RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_AsyncLoginAuth(data_p->username,
                                      data_p->password,
                                      data_p->cookie,
                                      data_p->cookie_size);
            break;
        }

        case RADIUS_MGR_IPC_CMD_ASYNC_ENABLE_PASSWORD_AUTH:
        {
            RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T *data_p = (RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_AsyncEnablePasswordAuth(data_p->username,
                                               data_p->password,
                                               data_p->cookie,
                                               data_p->cookie_size);
            break;
        }

        case RADIUS_MGR_IPC_CMD_SUBMIT_REQUEST:
        {
            RADIUS_MGR_IPCMsg_SubmitRequest_T  *data_p = (RADIUS_MGR_IPCMsg_SubmitRequest_T *)RADIUS_MGR_MSG_DATA(ipcmsg_p);
            RADIUS_MGR_MSG_RETVAL(ipcmsg_p) = RADIUS_MGR_SubmitRequest(&data_p->request_p);
            ipcmsg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }

        default:
            SYSFUN_Debug_Printf("\r\n%s(): Invalid cmd.", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    if(cmd <RADIUS_MGR_IPC_CMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* RADIUS_MGR_HandleIPCReqMsg */
