/* MODULE NAME: WEBAUTH_OM.C
 * PURPOSE:
 *      Definitions for the WEBAUTH OM
 * NOTES:
 *
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATION
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "webauth_type.h"
#include "webauth_mgr.h"
#include "webauth_om.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define WEBAUTH_OM_DEBUG_MSG(msg) \
    { \
        if (webauth_om_shmem_data_p->debug_flag) \
            printf(msg); \
    }

#define WEBAUTH_OM_LOCK()       orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(webauth_om_sem_id)
#define WEBAUTH_OM_UNLOCK()     SYSFUN_OM_LEAVE_CRITICAL_SECTION(webauth_om_sem_id, orig_priority)

/* DATA TYPE DECLARATIONS
 */
typedef struct WEBAUTH_OM_ShmemData_S
{
    WEBAUTH_TYPE_System_Info_T  system_info;
    WEBAUTH_TYPE_Port_Info_T    portinfo_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT];

    UI16_T                      total_trying_host;

    UI32_T                      debug_flag;
 } WEBAUTH_OM_ShmemData_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* TODO: Put all OM data on shared memory.
 * For now, only OM data which are not pointers put on shared memory. The
 * pointers (next_host_p in WEBAUTH_TYPE_Host_Info_T and success/black
 * lists) should be converted by L_CVRT_GET_OFFSET and L_CVRT_GET_PTR macros.
 * And, trying lists are consist of elements allocated by malloc() which are not
 * able to put on shared memory.
 */

/* The OM data not put on shared memory yet.
 */
static  WEBAUTH_TYPE_Host_Trying_T      *webauth_om_host_trying_list_p=NULL;
static  WEBAUTH_TYPE_Host_Trying_T      *webauth_om_host_trying_list_tail_p=NULL;
static  WEBAUTH_TYPE_Host_Info_T        *webauth_om_success_list_head_p=NULL;
static  WEBAUTH_TYPE_Host_Info_T        *webauth_om_success_list_tail_p=NULL;
static  WEBAUTH_TYPE_Host_Info_T        *webauth_om_black_list_head_p=NULL;
static  WEBAUTH_TYPE_Host_Info_T        *webauth_om_black_list_tail_p=NULL;

static  WEBAUTH_OM_ShmemData_T          *webauth_om_shmem_data_p;
static  UI32_T                          webauth_om_sem_id;
static  UI32_T                          orig_priority;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: WEBAUTH_OM_InitWebauthOm
 * PURPOSE: This function will init all data of OM
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_InitWebAuthOm(void)
{
    /* system info */
    memset(&webauth_om_shmem_data_p->system_info, 0, sizeof(webauth_om_shmem_data_p->system_info));

    WEBAUTH_OM_ClearWebAuthOm();
} /*End of WEBAUTH_OM_InitWebauthOm */

/* FUNCTION NAME: WEBAUTH_OM_AttachSystemResources
 * PURPOSE: This function will init all share memory data
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_AttachSystemResources(void)
{
    webauth_om_shmem_data_p = (WEBAUTH_OM_ShmemData_T *) SYSRSC_MGR_GetShMem(SYSRSC_MGR_WEBAUTH_OM_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_WEBAUTH_OM, &webauth_om_sem_id);
}

/* FUNCTION NAME: WEBAUTH_OM_GetShMemInfo
 * PURPOSE: This function will provide shared memory information for SYSRSC.
 * INPUT:   none
 * OUTPUT:  segid_p  -  shared memory segment id
 *          seglen_p -  length of the shared memroy segment
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_WEBAUTH_OM_SHMEM_SEGID;
    *seglen_p = sizeof(*webauth_om_shmem_data_p);
}

/* FUNCTION NAME: WEBAUTH_OM_ClearWebAuthOm
 * PURPOSE: This function will init all data of OM
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   this function mainly for disable webauth, then free list and
 *          temp data
 */
void WEBAUTH_OM_ClearWebAuthOm(void)
{
    WEBAUTH_TYPE_Host_Trying_T *host_trying_p;
    UI32_T i = 0;

    /* port array info */
    memset(webauth_om_shmem_data_p, 0, sizeof(*webauth_om_shmem_data_p));

    /* status can't be set to zero
     * VAL_webAuthPortConfigStatus_enabled = 1L
     * VAL_webAuthPortConfigStatus_disabled = 2L
     */
    for(i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        webauth_om_shmem_data_p->portinfo_ar[i].status = SYS_DFLT_WEBAUTH_PORT_STATUS;
    }

    while(webauth_om_host_trying_list_p != NULL)
    {
        host_trying_p = webauth_om_host_trying_list_p ->next_host_p;
        free(webauth_om_host_trying_list_p);
        webauth_om_host_trying_list_p = host_trying_p;
    }

    /* host retry fail count not each max */
    webauth_om_host_trying_list_tail_p = NULL;
    /* authenticated hsot list */
    webauth_om_success_list_head_p = NULL;
    /* authenticated hsot list */
    webauth_om_success_list_tail_p = NULL;
    /* black list hosts */
    webauth_om_black_list_head_p = NULL;
    /* black list hosts */
    webauth_om_black_list_tail_p = NULL;

    webauth_om_shmem_data_p->total_trying_host = 0;

    webauth_om_shmem_data_p->debug_flag = 0;
} /*End of WEBAUTH_OM_ClearWebAuthOm */


/* FUNCTION NAME: WEBAUTH_OM_GetSystemInfoPointer
 * PURPOSE: This function will return the point of webauth system info.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_System_Info_T*
 * NOTES:   none
 */
WEBAUTH_TYPE_System_Info_T* WEBAUTH_OM_GetSystemInfoPointer(void)
{
    return &webauth_om_shmem_data_p->system_info;
} /*End of WEBAUTH_OM_GetWebauthSystemInfoPointer */

/* FUNCTION NAME: WEBAUTH_OM_SetSystemStatus
 * PURPOSE: this function will set webauth status
 * INPUT:   VAL_webauthEnable_enabled / VAL_webauthEnable_disabled
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetSystemStatus(UI8_T status)
{
    WEBAUTH_OM_LOCK();
    webauth_om_shmem_data_p->system_info.status = status;
    WEBAUTH_OM_UNLOCK();

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_SetSystemStatus */

/* FUNCTION NAME: WEBAUTH_OM_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * NOTES:   none
 */
UI8_T WEBAUTH_OM_GetSystemStatus(void)
{
    return webauth_om_shmem_data_p->system_info.status;
} /* End of WEBAUTH_OM_GetSystemStatus */

/* FUNCTION NAME: WEBAUTH_OM_GetLPortInfoPTR
 * PURPOSE: This function will return the pointer of webauth lport info.
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_Port_Info_T* ; NULL
 * NOTES:   none
 */
WEBAUTH_TYPE_Port_Info_T* WEBAUTH_OM_GetLPortInfoPTR(UI32_T lport)
{
    if ((lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) || (lport == 0))
        return   NULL;
    else
        return &webauth_om_shmem_data_p->portinfo_ar[lport-1];
} /*End of WEBAUTH_OM_GetLPortInfoPTR */

/* FUNCTION NAME: WEBAUTH_OM_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetQuietPeriod(UI16_T quiet_period)
{
    WEBAUTH_OM_LOCK();
    webauth_om_shmem_data_p->system_info.quiet_period = quiet_period;
    WEBAUTH_OM_UNLOCK();

    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   none.
 * OUTPUT:  *quiet_period_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetQuietPeriod(UI16_T *quiet_period_p)
{
    *quiet_period_p = webauth_om_shmem_data_p->system_info.quiet_period;

    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt value
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetMaxLoginAttempts(UI8_T max_login_attempt)
{
    WEBAUTH_OM_LOCK();
    webauth_om_shmem_data_p->system_info.max_login_attempt = max_login_attempt;
    WEBAUTH_OM_UNLOCK();

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_SetMaxLoginAttempts */

/* FUNCTION NAME: WEBAUTH_OM_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   *max_login_attempt_p
 * OUTPUT:  *max login attempt_p value
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetMaxLoginAttempts(UI8_T *max_login_attempt_p)
{
    *max_login_attempt_p = webauth_om_shmem_data_p->system_info.max_login_attempt;

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_SetMaxLoginAttempts */

/* FUNCTION NAME: WEBAUTH_OM_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetSessionTimeout(UI16_T session_timeout)
{
    WEBAUTH_OM_LOCK();
    webauth_om_shmem_data_p->system_info.session_timeout = session_timeout;
    WEBAUTH_OM_UNLOCK();

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_SetSessionTimeout */

/* FUNCTION NAME: WEBAUTH_OM_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   *session_timeout_p
 * OUTPUT:  sesstion timeout_p value
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetSessionTimeout(UI16_T *session_timeout_p)
{
     *session_timeout_p = webauth_om_shmem_data_p->system_info.session_timeout;

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_SetSessionTimeout */


/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginURL
 * PURPOSE: This function set external login URL
 * INPUT:   *url_p,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginURL(char *url_p)
{
    WEBAUTH_OM_LOCK();
    strcpy(webauth_om_shmem_data_p->system_info.external_login_url_ar, url_p);
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginFailURL
 * PURPOSE: This function set external login fail URL
 * INPUT:   *url_p,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginFailURL(char *url_p)
{
    WEBAUTH_OM_LOCK();
    strcpy(webauth_om_shmem_data_p->system_info.external_login_fail_url_ar, url_p);
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginSuccessURL
 * PURPOSE: This function will set login success URL
 * INPUT:   *url_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginSuccessURL(char *url_p)
{
    WEBAUTH_OM_LOCK();
    strcpy(webauth_om_shmem_data_p->system_info.external_login_success_url_ar, url_p);
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginURL
 * PURPOSE: This function get external login URL
 * INPUT:   *url_p,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  webauth_om_shmem_data_p->system_info.external_login_url_ar
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginURL(char *url_p)
{
    strcpy(url_p, webauth_om_shmem_data_p->system_info.external_login_url_ar);

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginFailURL
 * PURPOSE: This function will get login fail URL
 * INPUT:   *url_p
 * OUTPUT:  *url_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginFailURL(char *url_p)
{
    strcpy(url_p, webauth_om_shmem_data_p->system_info.external_login_fail_url_ar);

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginSuccessURL
 * PURPOSE: This function will get external login success URL
 * INPUT:   *url_p
 * OUTPUT:  *url_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginSuccessURL(char *url_p)
{
    strcpy(url_p, webauth_om_shmem_data_p->system_info.external_login_success_url_ar);

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetSuccessListHead
 * PURPOSE: This function will get success list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_success_list_head_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Info_T * WEBAUTH_OM_GetSuccessListHead(void)
{
    return  webauth_om_success_list_head_p;
}

/* FUNCTION NAME: WEBAUTH_OM_SetSuccessListHead
 * PURPOSE: This function will set scuuess list head
 * INPUT:   *success_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetSuccessListHead(WEBAUTH_TYPE_Host_Info_T *success_list_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_success_list_head_p = success_list_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetSuccessListTail
 * PURPOSE: This function will set scuuess list tail
 * INPUT:   *success_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetSuccessListTail(WEBAUTH_TYPE_Host_Info_T *success_list_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_success_list_tail_p = success_list_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetBlackListHead
 * PURPOSE: This function will get black list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_black_list_head_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Info_T* WEBAUTH_OM_GetBlackListHead(void)
{
    return  webauth_om_black_list_head_p;
}

/* FUNCTION NAME: WEBAUTH_OM_SetBlackListHead
 * PURPOSE: This function will set black list head
 * INPUT:   *black_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetBlackListHead(WEBAUTH_TYPE_Host_Info_T *black_list_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_black_list_head_p = black_list_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetBlackListTail
 * PURPOSE: This function will set black list tail
 * INPUT:   *black_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetBlackListTail(WEBAUTH_TYPE_Host_Info_T *black_list_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_black_list_tail_p = black_list_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetTryingListHead
 * PURPOSE: This function will get trying list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_host_trying_list_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Trying_T * WEBAUTH_OM_GetTryingListHead(void)
{
    return  webauth_om_host_trying_list_p;
}

/* FUNCTION NAME: WEBAUTH_OM_SetTryingListHead
 * PURPOSE: This function will set ptr to trying list head
 * INPUT:   *trying_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetTryingListHead(WEBAUTH_TYPE_Host_Trying_T *trying_list_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_host_trying_list_p = trying_list_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_SetTryingListTail
 * PURPOSE: This function will set ptr to trying list tail
 * INPUT:   *trying_list_tail_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetTryingListTail(WEBAUTH_TYPE_Host_Trying_T *trying_list_tail_p)
{
    WEBAUTH_OM_LOCK();
    webauth_om_host_trying_list_tail_p = trying_list_tail_p;
    WEBAUTH_OM_UNLOCK();

    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_GetTotalTryingCount
 * PURPOSE: This function will get total trying count
 * INPUT:   *count_p
 * OUTPUT:  *count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetTotalTryingCount(UI16_T *count_p)
{
    *count_p = webauth_om_shmem_data_p->total_trying_host;
    return  WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_CreateSuccessHostByLPort
 * PURPOSE: This function will create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T    *lport_info_p;
    WEBAUTH_TYPE_Host_Info_T    host_data;
    UI32_T                      ret = WEBAUTH_TYPE_RETURN_ERROR;
    UI16_T                      success_count, i;

    if (( lport > SYS_ADPT_TOTAL_NBR_OF_LPORT ) || (lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();

    lport_info_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1];

    /* check success count */
    if((lport_info_p->success_count) > SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT)
    {
        WEBAUTH_OM_UNLOCK();
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    success_count = lport_info_p->success_count;

    /* fill data */
    host_data.ip = ip_addr;
    host_data.state = WEBAUTH_TYPE_HOST_STATE_SUCCESS;
    host_data.remaining_time = webauth_om_shmem_data_p->system_info.session_timeout;
    host_data.lport = lport;
    host_data.next_host_p = NULL;

    /* process port array */
    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT;i++)
    {
        if(webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_INITIAL)
        {
            memcpy(&webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i], &host_data, sizeof(WEBAUTH_TYPE_Host_Info_T));
            webauth_om_shmem_data_p->portinfo_ar[lport-1].success_count++;
            ret = WEBAUTH_TYPE_RETURN_OK;
            break;
        }
    }

    if(ret == WEBAUTH_TYPE_RETURN_OK)
    {
        /* process success list */
        if(webauth_om_success_list_head_p == NULL)
        {
            webauth_om_success_list_head_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i];
            webauth_om_success_list_tail_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i];
        }
        else
        {
            webauth_om_success_list_tail_p->next_host_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i];
        }
        webauth_om_success_list_tail_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i];
        webauth_om_success_list_tail_p->next_host_p = NULL;
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
} /* End of WEBAUTH_OM_CreateSuccessHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_CreateBlackHostByLPort
 * PURPOSE: This function will create black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T    *lport_info_p;
    WEBAUTH_TYPE_Host_Info_T    host_data;
    UI16_T                      black_count, i;

    if ( lport > SYS_ADPT_TOTAL_NBR_OF_LPORT || (lport == 0))
    {
        WEBAUTH_OM_DEBUG_MSG("\n WEBAUTH_OM_CreateBlackHostByLPort lport error ");
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();

    lport_info_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1];
    /* check black count */
    if((lport_info_p->black_count) >= SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT)
    {
        WEBAUTH_OM_UNLOCK();
        WEBAUTH_OM_DEBUG_MSG("\n WEBAUTH_OM_CreateBlackHostByLPort brack count error ");
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    black_count = lport_info_p->black_count;

    /* fill data */
    host_data.ip = ip_addr;
    host_data.state = WEBAUTH_TYPE_HOST_STATE_BLACK;
    host_data.remaining_time = webauth_om_shmem_data_p->system_info.quiet_period;
    host_data.lport = lport;
    host_data.next_host_p = NULL;

    /* process port array */
    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT;i++)
    {
        if(webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_INITIAL)
        {
            memcpy(&webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i], &host_data, sizeof(WEBAUTH_TYPE_Host_Info_T));
            webauth_om_shmem_data_p->portinfo_ar[lport-1].black_count++;
            break;
        }
    }

    /* process black list */
    if(webauth_om_black_list_head_p == NULL)
    {
        webauth_om_black_list_head_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i];
        webauth_om_black_list_tail_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i];
    }
    else
    {
        webauth_om_black_list_tail_p->next_host_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i];
    }
    webauth_om_black_list_tail_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1].black_entries_ar[i];
    webauth_om_black_list_tail_p->next_host_p = NULL;

    WEBAUTH_OM_UNLOCK();
    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_CreateBlackHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_CreateTryingHostByLPort
 * PURPOSE: This function will create trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ;
 *          WEBAUTH_TYPE_RETURN_ERROR;
 *          WEBAUTH_TYPE_RETURN_HOST_TRYING, this is for cgi to check type
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateTryingHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Host_Trying_T   * host_data_p, *trying_list_tmp_p=NULL,*trying_list_prev_p =NULL;

    if ( lport > SYS_ADPT_TOTAL_NBR_OF_LPORT || (lport == 0))
    {
        WEBAUTH_OM_DEBUG_MSG("\n WEBAUTH_OM_CreateTryingHostByLPort lport error ");
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();

    if (1 == webauth_om_shmem_data_p->system_info.max_login_attempt)
    {
        WEBAUTH_OM_UNLOCK();
        WEBAUTH_OM_DEBUG_MSG("\n WEBAUTH_OM_CreateTryingHostByLPort max login attemp is 1 ");
        return WEBAUTH_TYPE_RETURN_HOST_BLACK;
    }

    trying_list_tmp_p =   webauth_om_host_trying_list_p;

    /* search client */
    while( trying_list_tmp_p != NULL)
    {
        if( trying_list_tmp_p->ip == ip_addr &&
            trying_list_tmp_p->lport == lport) /* got one check login attempt */
        {
            break;
        }
        else
        {
            trying_list_prev_p = trying_list_tmp_p;
            trying_list_tmp_p = trying_list_tmp_p->next_host_p;
        }
    }

    if(trying_list_tmp_p != NULL) /* find one*/
    {/* ipadate */
        trying_list_tmp_p->login_attempt++;
        if( trying_list_tmp_p->login_attempt >= webauth_om_shmem_data_p->system_info.max_login_attempt)
        {/*kill this */
           /* two situation: 1. head, 2: in middle 3. tail */
            if(trying_list_prev_p == NULL)
            {
                webauth_om_host_trying_list_p = trying_list_tmp_p->next_host_p;
            }
            else
                trying_list_prev_p->next_host_p =  trying_list_tmp_p->next_host_p;

            if(trying_list_tmp_p->next_host_p == NULL) /* i am tail , update tail*/
                webauth_om_host_trying_list_tail_p = trying_list_prev_p;

            free( trying_list_tmp_p);
            webauth_om_shmem_data_p->total_trying_host--;

            WEBAUTH_OM_UNLOCK();
            return WEBAUTH_TYPE_RETURN_HOST_BLACK;
        }

        WEBAUTH_OM_UNLOCK();
        return WEBAUTH_TYPE_RETURN_HOST_TRYING;
    }

    /* allocate data */
    host_data_p = (WEBAUTH_TYPE_Host_Trying_T *)malloc(sizeof(WEBAUTH_TYPE_Host_Info_T));

    if(host_data_p == NULL)
    {
        WEBAUTH_OM_UNLOCK();
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    host_data_p->ip = ip_addr;
    host_data_p->lport = lport;
    host_data_p->next_host_p = NULL;
    host_data_p->login_attempt = 1;

    webauth_om_shmem_data_p->total_trying_host++;
    if(webauth_om_shmem_data_p->total_trying_host == 1)
    {
        webauth_om_host_trying_list_p = webauth_om_host_trying_list_tail_p = host_data_p;
    }
    else
    {/* link to tail */
        webauth_om_host_trying_list_tail_p->next_host_p = host_data_p;
        webauth_om_host_trying_list_tail_p = host_data_p;
        webauth_om_host_trying_list_tail_p->next_host_p = NULL;
    }

    /* check if list full , add to tail and kill first */
    if(webauth_om_shmem_data_p->total_trying_host >WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT)
    {
        trying_list_tmp_p = webauth_om_host_trying_list_p;

        webauth_om_host_trying_list_p = trying_list_tmp_p->next_host_p;
        if(trying_list_tmp_p == NULL)
        {
            WEBAUTH_OM_DEBUG_MSG("\n in WEBAUTH_OM_CreateTryingHostByLPort free excess data but null");
        }
        else
        {
            free(trying_list_tmp_p);
            webauth_om_shmem_data_p->total_trying_host--;
        }
    }

    WEBAUTH_OM_UNLOCK();
    return WEBAUTH_TYPE_RETURN_HOST_TRYING;
} /* End of WEBAUTH_OM_CreateTryingHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_DeleteSuccessListHost
 * PURPOSE: This function will delete success host data in list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T  WEBAUTH_OM_DeleteSuccessListHost(UI32_T ip_addr, UI32_T lport )
{
    WEBAUTH_TYPE_Host_Info_T      *success_list_prev_p=NULL, *success_list_curr_p;
    UI32_T                                      ret = WEBAUTH_TYPE_RETURN_ERROR;

    WEBAUTH_OM_LOCK();

    success_list_curr_p=webauth_om_success_list_head_p;
    while(success_list_curr_p != NULL)
    {
        if(success_list_curr_p->ip == ip_addr &&
           success_list_curr_p->lport == lport)
            break;
        else
        {
            success_list_prev_p = success_list_curr_p;
            success_list_curr_p = success_list_curr_p->next_host_p;
        }
    }

    if(success_list_curr_p != NULL)
    {
        if(success_list_prev_p != NULL)
        {
            success_list_prev_p->next_host_p = success_list_curr_p->next_host_p;
        }
        else /* head*/
        {
            webauth_om_success_list_head_p =  success_list_curr_p->next_host_p;
        }

        if(webauth_om_success_list_tail_p == success_list_curr_p)
            webauth_om_success_list_tail_p = success_list_prev_p;

        ret =WEBAUTH_TYPE_RETURN_OK;

        /* now reset array data */
        success_list_curr_p->state = WEBAUTH_TYPE_HOST_STATE_INITIAL;
        success_list_curr_p->ip = 0;
        webauth_om_shmem_data_p->portinfo_ar[(success_list_curr_p->lport)-1].success_count--;
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
} /* End of WEBAUTH_OM_DeleteSuccessListHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_DeleteBlackListHost
 * PURPOSE: This function will delete black host data in list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteBlackListHost(UI32_T ip_addr, UI32_T lport )
{
    WEBAUTH_TYPE_Host_Info_T  *black_list_prev_p=NULL, *black_list_curr_p;
    UI32_T                                  ret = WEBAUTH_TYPE_RETURN_ERROR;

    WEBAUTH_OM_LOCK();

    black_list_curr_p=webauth_om_black_list_head_p;
    while(black_list_curr_p != NULL)
    {
        if(black_list_curr_p->ip == ip_addr &&
           black_list_curr_p->lport == lport)
            break;
        else
        {
            black_list_prev_p = black_list_curr_p;
            black_list_curr_p = black_list_curr_p->next_host_p;
        }
    }

    if(black_list_curr_p != NULL)
    {
        if(black_list_prev_p != NULL)
        {
            black_list_prev_p->next_host_p = black_list_curr_p->next_host_p;
        }
        else /* head*/
        {
            webauth_om_black_list_head_p =  black_list_curr_p->next_host_p;
        }

        if(webauth_om_black_list_tail_p == black_list_curr_p)
            webauth_om_black_list_tail_p = black_list_prev_p;

        ret =WEBAUTH_TYPE_RETURN_OK;

        /* now reset array data */
        black_list_curr_p->state = WEBAUTH_TYPE_HOST_STATE_INITIAL;
        black_list_curr_p->ip = 0;
        webauth_om_shmem_data_p->portinfo_ar[(black_list_curr_p->lport)-1].black_count--;
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
} /* End of WEBAUTH_OM_DeleteSuccessListHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_DeleteTryingHost
 * PURPOSE: This function will delete trying host data in list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteTryingHost(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Host_Trying_T   *trying_list_prev_p=NULL, *trying_list_curr_p;
    UI32_T                                      ret = WEBAUTH_TYPE_RETURN_ERROR;

    WEBAUTH_OM_LOCK();

    trying_list_curr_p=webauth_om_host_trying_list_p;
    while(trying_list_curr_p != NULL)
    {
        if(trying_list_curr_p->ip == ip_addr &&
           trying_list_curr_p->lport == lport)
            break;
        else
        {
            trying_list_curr_p = trying_list_curr_p->next_host_p;
            trying_list_prev_p = trying_list_curr_p;
        }
    }

    if(trying_list_curr_p != NULL)
    {
        if(trying_list_prev_p != NULL)
        {
            trying_list_prev_p->next_host_p = trying_list_curr_p->next_host_p;
        }
        else /* head*/
        {
            webauth_om_host_trying_list_p =  trying_list_curr_p->next_host_p;
        }

        if(webauth_om_host_trying_list_tail_p == trying_list_curr_p)
            webauth_om_host_trying_list_tail_p = trying_list_prev_p;

        ret =WEBAUTH_TYPE_RETURN_OK;

        if(webauth_om_shmem_data_p->total_trying_host == 0)
        {
            WEBAUTH_OM_DEBUG_MSG("\n in WEBAUTH_OM_DeleteTryingHost but del fail for total count = 0");
        }
        else
            webauth_om_shmem_data_p->total_trying_host--;
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
} /* End of WEBAUTH_OM_DeleteTryingHost */

/* FUNCTION NAME: WEBAUTH_OM_DeleteBlackListHostByIP
 * PURPOSE: This function will delete black host data in list
 * INPUT:   ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteBlackListHostByIP(UI32_T ip_addr)
{
    WEBAUTH_TYPE_Host_Info_T  *black_list_tmp_p, *black_list_prv_p;
    UI32_T                                  ret = WEBAUTH_TYPE_RETURN_ERROR;

    WEBAUTH_OM_LOCK();

    /* process success list */
    if(webauth_om_black_list_head_p == NULL)
    {
        WEBAUTH_OM_UNLOCK();
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    else if(webauth_om_black_list_head_p->ip == ip_addr)
    {
        if(webauth_om_black_list_head_p->next_host_p == NULL)
        {
            webauth_om_black_list_head_p = NULL;
            webauth_om_black_list_tail_p = NULL;
        }
        else
        {
            webauth_om_black_list_head_p = webauth_om_black_list_head_p->next_host_p;
        }
    }
    else
    {
        black_list_tmp_p = webauth_om_black_list_head_p->next_host_p;
        black_list_prv_p = webauth_om_black_list_head_p;

        while(black_list_tmp_p != NULL)
        {
            if(black_list_tmp_p->ip == ip_addr)
            {
                black_list_prv_p->next_host_p = black_list_tmp_p->next_host_p;
                if(black_list_prv_p->next_host_p == NULL)
                {
                    webauth_om_black_list_tail_p = black_list_prv_p ;
                }
                ret = WEBAUTH_TYPE_RETURN_OK;
                break;
            }
            else
            {
                black_list_prv_p = black_list_tmp_p;
                black_list_tmp_p = black_list_tmp_p->next_host_p;
                continue;
            }
        }
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
} /* End of WEBAUTH_OM_DeleteBlackListHostByLPort */

/* FUNCTION NAME: WEBAUTH_OM_DeleteSuccessHostArrayByIP
 * PURPOSE: This function will delete success host data by lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteSuccessHostArrayByIP(UI32_T lport, UI32_T ip_addr)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    UI32_T ip_addr_om;
    UI16_T count, i;
    UI16_T remain_count;

    WEBAUTH_OM_LOCK();

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    if(lport_info_p != NULL)
    {
        count = lport_info_p->success_count;

        if(count ==0)
        {
            WEBAUTH_OM_DEBUG_MSG("\n reauth function but no host \n");
        }

        for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            ip_addr_om = webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i].ip;

            /* delete success list for this host ip */
            if ( ip_addr == ip_addr_om)
            {

                /* delete this ip in port information table, copy remainder forwarder */
                remain_count = SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT-(i+1);
                memset( &webauth_om_shmem_data_p->portinfo_ar[lport-1].success_entries_ar[i], 0, sizeof(WEBAUTH_TYPE_Host_Info_T));
                /* minus success count */
                (lport_info_p->success_count)--;
                break;
            }
        }
    }

    WEBAUTH_OM_UNLOCK();
    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_DeleteSuccessHostArrayByIP */

/* FUNCTION NAME: WEBAUTH_OM_DeleteSuccessHostByLPort
 * PURPOSE: This function will delete success host data by lport
 * INPUT:
 *          lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteSuccessHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T    *lport_info_p;
    WEBAUTH_TYPE_Host_Info_T    host_data;
    UI16_T                      success_count;

    if( ( lport > SYS_ADPT_TOTAL_NBR_OF_LPORT ) || (lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();

    lport_info_p = &webauth_om_shmem_data_p->portinfo_ar[lport-1];
    /* check success count */
    if((lport_info_p->success_count) > SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT)
    {
        WEBAUTH_OM_UNLOCK();
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    success_count = lport_info_p->success_count;

    /* fill data */
    host_data.ip = ip_addr;
    host_data.state = WEBAUTH_TYPE_HOST_STATE_SUCCESS;
    host_data.remaining_time = 0;
    host_data.lport = lport;
    host_data.next_host_p = NULL;

    /* process port array */
    memcpy(&lport_info_p->success_entries_ar[success_count], &host_data, sizeof(WEBAUTH_TYPE_Host_Info_T));

    (lport_info_p->success_count)++;

    /* process success list */
    if(webauth_om_success_list_head_p == NULL)
    {
        webauth_om_success_list_head_p = &lport_info_p->success_entries_ar[success_count];
        webauth_om_success_list_tail_p = &lport_info_p->success_entries_ar[success_count];
    }
    else
    {
        webauth_om_success_list_tail_p->next_host_p = &lport_info_p->success_entries_ar[success_count];
    }
    webauth_om_success_list_tail_p = &lport_info_p->success_entries_ar[success_count];
    webauth_om_success_list_tail_p->next_host_p = NULL;

    WEBAUTH_OM_UNLOCK();
    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_OM_InitArrayByLPort
 * PURPOSE: This function do init port array by lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_InitArrayByLPort(UI32_T lport)
{
    if ((lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) || (lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();
    memset(&webauth_om_shmem_data_p->portinfo_ar[lport-1], 0, sizeof(WEBAUTH_TYPE_Port_Info_T));
    webauth_om_shmem_data_p->portinfo_ar[lport-1].status = SYS_DFLT_WEBAUTH_PORT_STATUS;
    WEBAUTH_OM_UNLOCK();

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_OM_InitArrayByLPort */

/* FUNCTION NAME: WEBAUTH_OM_SetDebugFlag
 * PURPOSE: This function to set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_SetDebugFlag(UI32_T debug_flag)
{
    WEBAUTH_OM_LOCK();
    webauth_om_shmem_data_p->debug_flag = debug_flag;
    WEBAUTH_OM_UNLOCK();
}

/* FUNCTION NAME: WEBAUTH_OM_IsIPValidByLPort
 * PURPOSE: This function will check ip is valid by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  TRUE: this ip is valid for this port
 *          FALSE: this ip is invalid for this port
 * NOTES:   none
 */
BOOL_T WEBAUTH_OM_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI32_T i;
    BOOL_T ret = FALSE;
    WEBAUTH_TYPE_Port_Info_T    *lport_info_p;

    if (   (SYS_ADPT_TOTAL_NBR_OF_LPORT < lport)
        || (0 == lport)
        )
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    WEBAUTH_OM_LOCK();

    lport_info_p = &webauth_om_shmem_data_p->portinfo_ar[lport - 1];

    for (i = 0; i < SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if (lport_info_p->success_entries_ar[i].ip == ip_addr)
        {
            ret = TRUE;
            break;
        }
    }

    WEBAUTH_OM_UNLOCK();
    return ret;
}
