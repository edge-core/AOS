/* MODULE NAME: WEBAUTH_TYPE.H  
 * PURPOSE: 
 *      1.Definitions for the webauth type declaration.
 *
 * NOTES:
 *
 *
 * HISTORY:
 * 01/30/2007     --  Rich Lee , Create
 *
 *
 * Copyright(C)       Accton Corporation, 2007 
 */

#ifndef _WEBAUTH_TYPE_H
#define _WEBAUTH_TYPE_H

/* INCLUDE FILE DECLARATTIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h" 
#include "sys_adpt.h" 
/* NAMING CONSTANT DECLARATIONS
 */

/* the max url length count from http://.... */
#define WEBAUTH_TYPE_MAX_URL_LENGTH     120 
#define WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT    \
        (SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT/2)*SYS_ADPT_TOTAL_NBR_OF_LPORT

/* MACRO FUNCTION DECLARATIONS
 */
 
/* DATA TYPE DECLARATIONS
 */
 
typedef struct
{
    UI16_T  session_timeout;    
    UI16_T  quiet_period;
    UI8_T   status;    
    UI8_T   max_login_attempt;
    char    external_login_url_ar[WEBAUTH_TYPE_MAX_URL_LENGTH+1];
    char    external_login_fail_url_ar[WEBAUTH_TYPE_MAX_URL_LENGTH+1];
    char    external_login_success_url_ar[WEBAUTH_TYPE_MAX_URL_LENGTH+1];
}WEBAUTH_TYPE_System_Info_T;

typedef struct WEBAUTH_TYPE_Host_Info_S
{
    struct WEBAUTH_TYPE_Host_Info_S    *next_host_p;
    UI32_T                      ip;     /* network order */
    UI32_T                      lport;
    UI16_T                      remaining_time;
    /* quiet timer of session timer */
    UI8_T                       state;
}WEBAUTH_TYPE_Host_Info_T;


typedef struct WEBAUTH_TYPE_Port_Info_S
{
    WEBAUTH_TYPE_Host_Info_T
                success_entries_ar[SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT];
    WEBAUTH_TYPE_Host_Info_T
                black_entries_ar[SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT];
    UI16_T      success_count;
    UI16_T      black_count;
    UI8_T       status;
}WEBAUTH_TYPE_Port_Info_T;

/* following enum is defined for get url function by it's type */
typedef enum WEBAUTH_EXTERNAL_URL_E
{
    WEBAUTH_TYPE_EXTERNAL_URL_LOGIN=1, 
    WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_FAIL, 
    WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_SUCCESS, 
}WEBAUTH_TYPE_EXTERNAL_URL_T;

enum WEBAUTH_TYPE_RETURN_CODE_E
{
    WEBAUTH_TYPE_RETURN_OK=1,    
    WEBAUTH_TYPE_RETURN_ERROR, 
    WEBAUTH_TYPE_RETURN_NO_CHANGE, 
    WEBAUTH_TYPE_RETURN_NO_EFFECT, 
    WEBAUTH_TYPE_RETURN_CS_FLAG_OFF,
    WEBAUTH_TYPE_RETURN_CS_FLAG_ON,
    WEBAUTH_TYPE_RETURN_NOT_FOUND ,
    WEBAUTH_TYPE_RETURN_HOST_BLACK,
    WEBAUTH_TYPE_RETURN_HOST_TRYING,
    WEBAUTH_TYPE_RETURN_HOST_SUCCESS
};

enum WEBAUTH_TYPE_EVENT_MASK_E
{
    WEBAUTH_TYPE_EVENT_NONE                =   0x0000L,
    WEBAUTH_TYPE_EVENT_MSGRCVD             =   0x0001L,
    WEBAUTH_TYPE_EVENT_TIMER               =   0x0002L,
    WEBAUTH_TYPE_EVENT_ENTER_TRANSITION    =   0x0004L,
    WEBAUTH_TYPE_EVENT_ALL                 =   0xFFFFL
};

enum WEBAUTH_TYPE_LOG_FUN_E
{
    WEBAUTH_TYPE_LOG_FUN_WEBAUTH_TASK_CREATE_TASK = 0,
    WEBAUTH_TYPE_LOG_FUN_WEBAUTH_TASK_WAITEVENT,
    WEBAUTH_TYPE_LOG_FUN_OTHER
};

enum WEBAUTH_TYPE_LOG_ERR_E
{
    WEBAUTH_TYPE_LOG_ERR_WEBAUTH_TASK_CREATE_TASK = 0,
    WEBAUTH_TYPE_LOG_ERR_WEBAUTH_TASK_WAITEVENT,
    WEBAUTH_TYPE_LOG_ERR_OTHER
};

enum WEBAUTH_TYPE_HOST_STATE_E
{
    WEBAUTH_TYPE_HOST_STATE_INITIAL,
    WEBAUTH_TYPE_HOST_STATE_SUCCESS,
    WEBAUTH_TYPE_HOST_STATE_BLACK,
    WEBAUTH_TYPE_HOST_STATE_TRYING,    
};

typedef struct WEBAUTH_TYPE_Host_Trying_S
{
    struct WEBAUTH_TYPE_Host_Trying_S     *next_host_p;
    UI32_T      ip;     /*network order */
    UI32_T      lport;
    UI8_T       login_attempt;       
}WEBAUTH_TYPE_Host_Trying_T;
  
#endif  /* #ifndef _WEBAUTH_TYPE_H */
