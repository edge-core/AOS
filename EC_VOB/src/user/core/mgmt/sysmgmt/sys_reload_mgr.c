/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_RELOAD_MGR.C
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the Reload
 *          functions
 *
 * Notes:

 *  History
 *
 *   Andy_Chang     12/24/2007      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include "sysfun.h"
#if (SYS_CPNT_EH == TRUE)
#include "eh_type.h"
#include "eh_mgr.h"
#endif
#include "sys_module.h"
#include "stkctrl_pmgr.h"
#include "sys_callback_mgr.h"
#include "sys_reload_mgr.h"
#include "sys_reload_om.h"
#include "sys_reload_type.h"
/* UI message */


typedef enum
{
    SYS_RELOAD_MGR_RELOAD_IN_MODE           = 0,
    SYS_RELOAD_MGR_RELOAD_AT_MODE           = 1,
    SYS_RELOAD_MGR_RELOAD_REGULARITY_MODE   = 2,
    SYS_RELOAD_MGR_RELOAD_NONE_MODE         = 3
} SYS_RELOAD_MGR_RELOAD_MODE_E;

static  UI32_T                          is_provision_complete;
static  SYS_TYPE_CallBack_T             *reload_notify_callback;
static  I32_T                           remaining_reload_time;
static  UI32_T                          allowed_reload_mode;
static  UI32_T                          remaining_reload_in_time;
static  SYS_RELOAD_MGR_RELOAD_MODE_E    candidate_reload_mode;  /* Indicate who decides remaining_reload_time? */
static  UI32_T                          handler_event_time;
static  UI32_T                          sys_reload_mgr_sem_id;

static BOOL_T SYS_RELOAD_MGR_InitSemaphore();
static void SYS_RELOAD_MGR_AlertRemainReloadTime(I32_T remaining);
static void SYS_RELOAD_MGR_Notify_ReloadTime(UI32_T remaining);
static BOOL_T SYS_RELOAD_MGR_IsCorrectReloatatDate(SYS_RELOAD_OM_RELOADAT_DST date);
static BOOL_T SYS_RELOAD_MGR_IsCorrectReloatRegularityDate (SYS_RELOAD_OM_RELOADREGULARITY_DST date);

SYSFUN_DECLARE_CSC                    /* declare variables used for transition mode  */

/*--------------
    define
  --------------*/
#define SYS_RELOAD_MGR_FUNCTION_NUMBER          0xffffffff
#define SYS_RELOAD_MGR_LOCK()
#define SYS_RELOAD_MGR_UNLOCK()

#define SYS_RELOAD_MGR_NoReloadFlag             0
#define SYS_RELOAD_MGR_ReloadInFlag             1
#define SYS_RELOAD_MGR_ReloadAtFlag             2
#define SYS_RELOAD_MGR_ReloadRegularityFlag     4
#define SYS_RELOAD_MGR_ReloadAllFlag            8

#define SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME        0x7FFFFFFF
#define SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_IN_TIME     0xFFFFFFFF
/* CSC Macros definition
 */
#define SYS_RELOAD_MGR_USE_CSC(a)
#define SYS_RELOAD_MGR_RELEASE_CSC()

#define SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL)                     \
    SYS_RELOAD_MGR_USE_CSC(RET_VAL);                                        \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE){  \
        SYS_RELOAD_MGR_RELEASE_CSC();                                       \
        return (RET_VAL);                                                   \
    }

#define SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(RET_VAL)                      \
    {                                                                       \
        SYS_RELOAD_MGR_RELEASE_CSC();                                       \
        return (RET_VAL);                                                   \
    }

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_Init
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will init the system resource
 *
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. This routine will initialize the All Banner Message
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_MGR_Init(void)
{
    SYS_RELOAD_MGR_InitSemaphore();

    remaining_reload_time       = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    allowed_reload_mode         = SYS_RELOAD_MGR_NoReloadFlag;
    remaining_reload_in_time    = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_IN_TIME;
    candidate_reload_mode       = SYS_RELOAD_MGR_RELOAD_NONE_MODE;
    handler_event_time          = 0;

    SYS_RELOAD_OM_Init();
}

/*--------------------------------------------------------------------------
 * FUNCTION	NAME - SYS_RELOAD_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE	: This function	initializes all function pointer registration operations.
 * INPUT	: none
 * OUTPUT	: none
 * RETURN	: none
 * NOTES	: none
 *--------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_Create_InterCSC_Relation(void)
{
}

static BOOL_T SYS_RELOAD_MGR_InitSemaphore()
{
    sys_reload_mgr_sem_id = 0;
    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into master mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    return;
}/* end of SYS_RELOAD_MGR_EnterMasterMode */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into slave mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}/* end of SYS_RELOAD_MGR_EnterSlaveMode() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into transition mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    is_provision_complete = FALSE;
}/* end of SYS_RELOAD_MGR_EnterTransitionMode */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE  : This function Set Discp Entry to ARL Table when provision complete
 * INPUT    : None
 * OUTPUT   :
 * RETURN   : none
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_MGR_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
}

/*------------------------------------------------------------------------------
 * Function : SYS_RELOAD_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}/* end of SYS_RELOAD_MGR_SetTransitionMode */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - SYS_RELOAD_MGR_GetOperationMode
 *------------------------------------------------------------------------|
 * FUNCTION: This function will return present opertaion mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Current operation mode.
 *          1). SYS_TYPE_STACKING_TRANSITION_MODE
 *          2). SYS_TYPE_STACKING_MASTER_MODE
 *          3). SYS_TYPE_STACKING_SLAVE_MODE
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T SYS_RELOAD_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* end of SYS_RELOAD_MGR_GetOperationMode  */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadIn
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_SetReloadIn(UI32_T minute, UI8_T *reason)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( reason == NULL )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (SYS_ADPT_SYSMGMT_DEFERRED_RELOAD_MAX_MINUTES < minute)
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_SetReloadIn(minute, reason);
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    remaining_reload_in_time = minute*60;
    allowed_reload_mode      = (allowed_reload_mode | SYS_RELOAD_MGR_ReloadInFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST reload_at, UI8_T *reason)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( reason == NULL )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (!SYS_RELOAD_MGR_IsCorrectReloatatDate(reload_at))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_SetReloadAt(reload_at, reason);
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    allowed_reload_mode = (allowed_reload_mode | SYS_RELOAD_MGR_ReloadAtFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, UI8_T *reason)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( reason == NULL )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (!SYS_RELOAD_MGR_IsCorrectReloatRegularityDate(reload_regularity))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_SetReloadRegularity(reload_regularity, reason);
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    allowed_reload_mode = (allowed_reload_mode | SYS_RELOAD_MGR_ReloadRegularityFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadInInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : UI32_T        *remain_seconds
 *            SYS_TIME_DST  *next_reload_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetReloadInInfo(I32_T *remain_seconds, SYS_TIME_DST *next_reload_time)
{
    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( (remain_seconds == NULL) || (next_reload_time == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* Check reload-in function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadInFlag)== SYS_RELOAD_MGR_ReloadInFlag)
    {
        SYS_RELOAD_MGR_QueryNextReloadInTime(remaining_reload_in_time, next_reload_time);

        *remain_seconds = remaining_reload_in_time;

        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }
    else
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadAtInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active)
{
    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( (reload_at == NULL) || (next_reload_time == NULL) || (remain_seconds == NULL) || (function_active == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* Check reload-at function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadAtFlag) == SYS_RELOAD_MGR_ReloadAtFlag)
    {
        SYS_RELOAD_MGR_LOCK();
        SYS_RELOAD_OM_GetReloadAtInfo(reload_at);
        SYS_RELOAD_MGR_UNLOCK();

        SYS_RELOAD_MGR_QueryNextReloadAtTime(*reload_at, next_reload_time, function_active);
        SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadAtTime(*reload_at, remain_seconds);

        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }
    else
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadRegularityInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active)
{
    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( (reload_regularity == NULL) || (next_reload_time == NULL) || (remain_seconds == NULL) || (function_active == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* Check reload-regularity function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadRegularityFlag)== SYS_RELOAD_MGR_ReloadRegularityFlag)
    {
        SYS_RELOAD_MGR_LOCK();
        SYS_RELOAD_OM_GetReloadRegularityInfo(reload_regularity);
        SYS_RELOAD_MGR_UNLOCK();

        SYS_RELOAD_MGR_QueryNextReloadRegularityTime(*reload_regularity, next_reload_time, function_active);

        SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadRegularityTime(*reload_regularity, remain_seconds);

        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }
    else
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_TimeHandler
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to handle time event and count remain reload time
 * INPUT    : UI32_T event_timer_sec
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_MGR_TimeHandler(UI32_T event_timer_sec)
{
    BOOL_T                                  sys_time_change_flag = FALSE;
    I32_T                                   suspense_remaining_reload_time;
    SYS_RELOAD_OM_RELOADAT_DST             reload_at;
    SYS_RELOAD_OM_RELOADREGULARITY_DST     reload_regularity;

    /* Check the reload function is on
     */
    if (allowed_reload_mode == SYS_RELOAD_MGR_NoReloadFlag)
        return;

    memset(&reload_at, 0, sizeof(SYS_RELOAD_OM_RELOADAT_DST));
    memset(&reload_regularity, 0, sizeof(SYS_RELOAD_OM_RELOADREGULARITY_DST));

    sys_time_change_flag = SYS_TIME_IsTimeModify();

    handler_event_time = event_timer_sec;

    /* At least one reload mode is on
     */
    //remaining_reload_time = remaining_reload_time - handler_event_time;

    /* Check reload-in function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadInFlag)== SYS_RELOAD_MGR_ReloadInFlag)
    {
        remaining_reload_in_time = remaining_reload_in_time - handler_event_time;
        if (remaining_reload_time > remaining_reload_in_time)
        {
            remaining_reload_time = remaining_reload_in_time;
            candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_IN_MODE;
        }
    }

    /* Check reload-at function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadAtFlag) == SYS_RELOAD_MGR_ReloadAtFlag)
    {
        /* Count if remaining_reload_time need to be replaced
         */
        SYS_RELOAD_OM_GetReloadAtInfo(&reload_at);
        /* Check if reload-at time is pass away
         */
        SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadAtTime(reload_at, &suspense_remaining_reload_time);

        /* Only when sys_time is changed, the reload-at function is work
         */
        if (sys_time_change_flag)
        {
            if(suspense_remaining_reload_time > -10)
            {
                if (remaining_reload_time > suspense_remaining_reload_time)
                {
                    remaining_reload_time = suspense_remaining_reload_time;
                    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_AT_MODE;
                }
            }
            else
            {
                candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;
            }
        }
    }

    /* Check reload-regularity function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadRegularityFlag)== SYS_RELOAD_MGR_ReloadRegularityFlag)
    {
        /* Count if remaining_reload_time need to be replaced
         */
        SYS_RELOAD_OM_GetReloadRegularityInfo(&reload_regularity);
        /* Check if reload-at time is pass away
         */
        SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadRegularityTime(reload_regularity, &suspense_remaining_reload_time);

        /* Only when sys_time is changed, the reload-at function is work
         */
        if (sys_time_change_flag)
        {
            if (remaining_reload_time > suspense_remaining_reload_time)
            {
                remaining_reload_time = suspense_remaining_reload_time;
                candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_REGULARITY_MODE;
            }
        }
    }

    /* No candidate mode, remaining_reload_time doesnot be countdown */
    if ( candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_NONE_MODE )
    {
        remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    }
    /* Some functions call SYS_RELOAD_MGR_TimeHandler in order to recount reload-time relative,
       so doesnot need to do Alert and reload action
     */
    if ( (candidate_reload_mode != SYS_RELOAD_MGR_RELOAD_NONE_MODE) && (event_timer_sec > 0) )
    {
        /* Sent Alert
         */
        SYS_RELOAD_MGR_AlertRemainReloadTime(remaining_reload_time);

        /* Check if need to system reload
         */
        if ((remaining_reload_time <= 0) && (remaining_reload_time >= -10))
        {
            STKCTRL_PMGR_WarmStartSystem();
            /* Stop SYS_RELOAD_MGR_TimeHandler() work
             */
            allowed_reload_mode = SYS_RELOAD_MGR_NoReloadFlag;
        }
    }
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetDiffBetweenCurrentTimeAndReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get difference between reload-at time and current time
 *
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST reload_at
 * OUTPUT   : I32_T *difference_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadAtTime(SYS_RELOAD_OM_RELOADAT_DST reload_at, I32_T *difference_time)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;
    UI32_T      reload_at_time_seconds_from_TOD;
    UI32_T      count_time_seconds_from_TOD;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (difference_time == NULL)
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* Get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &count_time_seconds_from_TOD);

    SYS_TIME_ConvertDateTimeToSeconds(reload_at.year, reload_at.month, reload_at.day, reload_at.hour, reload_at.minute, 0, &reload_at_time_seconds_from_TOD);

    *difference_time = reload_at_time_seconds_from_TOD - count_time_seconds_from_TOD;

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetDiffBetweenCurrentTimeAndReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get difference between reload-at time and current time
 *
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 * OUTPUT   : I32_T *difference_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadRegularityTime(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, I32_T *difference_time)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;
    UI32_T  reload_regularity_time_seconds_from_TOD;
    UI32_T  count_time_seconds_from_TOD;
    I32_T   total_seconds_per_day    = 24*60*60;
    I32_T   total_seconds_per_week   = 7*24*60*60;
    UI32_T  day_of_week;
    UI32_T  next_reload_regularity_year;
    UI32_T  next_reload_regularity_month;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (difference_time == NULL)
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &count_time_seconds_from_TOD);


    switch (reload_regularity.period)
    {
        case SYS_RELOAD_TYPE_REGULARITY_PERIOD_NONE :
            SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);

        case SYS_RELOAD_TYPE_REGULARITY_PERIOD_DAILY :
            SYS_TIME_ConvertDateTimeToSeconds(year, month, day, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
            /* Check reload-regularity-time is on Today
             */
            if (reload_regularity_time_seconds_from_TOD >= count_time_seconds_from_TOD)
            {
                *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
            }
            /* reload-regularity-time is on tomorrow
             */
            else
            {
                *difference_time = total_seconds_per_day + reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
            }
            break;

        case SYS_RELOAD_TYPE_REGULARITY_PERIOD_WEEKLY :
            SYS_TIME_GetDayOfWeek(count_time_seconds_from_TOD, &day_of_week);
            /* reload-regularity-time must be on this week
             */
            if (reload_regularity.day_of_week > day_of_week)
            {
                *difference_time = (reload_regularity.day_of_week - day_of_week)*total_seconds_per_day
                                   + (reload_regularity.hour - hour)*60*60
                                   + (reload_regularity.minute - minute)*60
                                   + (0-second);
            }
            /* Check reload-regularity-time must be on Today or Next week
             */
            else if (reload_regularity.day_of_week == day_of_week)
            {
                SYS_TIME_ConvertDateTimeToSeconds(year, month, day, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
                /* Reload-regularity-time must be on Today
                 */
                if (reload_regularity_time_seconds_from_TOD >= count_time_seconds_from_TOD)
                {
                    *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
                }
                /* Reload-regularity-time must be on Next week
                 */
                else
                {
                    *difference_time = total_seconds_per_week + reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
                }
            }
            /* Reload-regularity-time must be on Today or Next week
             */
            else
            {
                *difference_time = (7 + reload_regularity.day_of_week - day_of_week)*total_seconds_per_day
                                   + (reload_regularity.hour - hour)*60*60
                                   + (reload_regularity.minute - minute)*60
                                   + (0-second);
            }
            break;

        case SYS_RELOAD_TYPE_REGULARITY_PERIOD_MONTHLY :
            /* Check reload-regularity time is in this month
             */
            if (reload_regularity.day_of_month > day)
            {
                SYS_TIME_ConvertDateTimeToSeconds(year, month, reload_regularity.day_of_month, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
                *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
            }
            else if (reload_regularity.day_of_month == day)
            {
                SYS_TIME_ConvertDateTimeToSeconds(year, month, reload_regularity.day_of_month, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
                /* Reload-regularity-time must be in this month
                 */
                if (reload_regularity_time_seconds_from_TOD >= count_time_seconds_from_TOD)
                {
                    *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
                }
                /* Reload-regularity-time must be on Next month
                 */
                else
                {
                    SYS_TIME_GetNextProcessDate((UI32_T)year, (UI32_T)month, reload_regularity.day_of_month, &next_reload_regularity_year, &next_reload_regularity_month);
                    SYS_TIME_ConvertDateTimeToSeconds(next_reload_regularity_year, next_reload_regularity_month, reload_regularity.day_of_month, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
                    *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
                }
            }
            else
            {
                SYS_TIME_GetNextProcessDate((UI32_T)year, (UI32_T)month, reload_regularity.day_of_month, &next_reload_regularity_year, &next_reload_regularity_month);
                SYS_TIME_ConvertDateTimeToSeconds(next_reload_regularity_year, next_reload_regularity_month, reload_regularity.day_of_month, reload_regularity.hour, reload_regularity.minute, 0, &reload_regularity_time_seconds_from_TOD);
                *difference_time = reload_regularity_time_seconds_from_TOD - count_time_seconds_from_TOD;
            }
            break;

            default:
                SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_AlertRemainReloadTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to sent alert if equal to specific time
 *
 * INPUT    : UI32_T remaining
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
static void SYS_RELOAD_MGR_AlertRemainReloadTime(I32_T remaining)
{
    I32_T  notify_time_ar[] = {3600, 1800, 300, 60};    /* ticks */
    int     count, ar_size = (sizeof (notify_time_ar) / sizeof (I32_T));

    if ( allowed_reload_mode != SYS_RELOAD_MGR_NoReloadFlag )
    {
        for(count=0; count<ar_size; count++)
        {
            if(remaining == notify_time_ar[count])
            {
                SYS_RELOAD_MGR_Notify_ReloadTime((UI32_T)remaining_reload_time/60);
                break;
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_Register_ReloadNotify_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    :
 * -------------------------------------------------------------------------
 */
void SYS_RELOAD_MGR_Register_ReloadNotify_CallBack(void (*fun)(UI32_T remaining))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(reload_notify_callback);
} /* End of SYS_RELOAD_MGR_Register_ReloadNotify_CallBack() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_Notify_ReloadTime
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function
 * INPUT   : UI32_T remaining   -- remaining time to reload (minutes)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SYS_RELOAD_MGR_Notify_ReloadTime(UI32_T remaining)
{
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_CALLBACK_MGR_AnnounceReloadReaminTime(SYS_MODULE_SYSMGMT, remaining);
#endif  /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */
} /* End of SYS_RELOAD_MGR_Notify_ReloadTime() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadIn
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-in function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_ReloadInCancel(void)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_ReloadInCancel();
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    remaining_reload_in_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_IN_TIME;
    allowed_reload_mode      = (allowed_reload_mode & ~SYS_RELOAD_MGR_ReloadInFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-at function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_ReloadAtCancel(void)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_ReloadAtCancel();
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    allowed_reload_mode = (allowed_reload_mode & ~SYS_RELOAD_MGR_ReloadAtFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-regularity function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_ReloadRegularityCancel(void)
{

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    SYS_RELOAD_MGR_LOCK();
    SYS_RELOAD_OM_ReloadRegularityCancel();
    SYS_RELOAD_MGR_UNLOCK();
    /* remaining_reload_time need to be recounted
     */
    remaining_reload_time = SYS_RELOAD_MGR_MAX_REMAINING_RELOAD_TIME;
    candidate_reload_mode = SYS_RELOAD_MGR_RELOAD_NONE_MODE;

    allowed_reload_mode = (allowed_reload_mode & ~SYS_RELOAD_MGR_ReloadRegularityFlag);

    /* Recount reload-time right now, but doesnot to do countdown
     */
    SYS_RELOAD_MGR_TimeHandler(0);
    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_GetRunningReloadAtInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : value
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_RELOAD_MGR_GetRunningReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at)
{
    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (reload_at == NULL)
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    /* Check reload-at function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadAtFlag) == SYS_RELOAD_MGR_ReloadAtFlag)
    {
        SYS_RELOAD_MGR_LOCK();
        SYS_RELOAD_OM_GetReloadAtInfo(reload_at);
        SYS_RELOAD_MGR_UNLOCK();

        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }
    else
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_GetRunningReloadRegularityInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_RELOAD_MGR_GetRunningReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity)
{
    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (reload_regularity == NULL)
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }
    /* Check reload-at function is on
     */
    if ((allowed_reload_mode & SYS_RELOAD_MGR_ReloadRegularityFlag)== SYS_RELOAD_MGR_ReloadRegularityFlag)
    {
        SYS_RELOAD_MGR_LOCK();
        SYS_RELOAD_OM_GetReloadRegularityInfo(reload_regularity);
        SYS_RELOAD_MGR_UNLOCK();

        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }
    else
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadInTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T remain_seconds
 * OUTPUT   : SYS_TIME_DST time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadInTime(I32_T remain_seconds, SYS_TIME_DST *next_reload_time)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;
    UI32_T      count_time_seconds_from_TOD;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( next_reload_time == NULL )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &count_time_seconds_from_TOD);

    //SYS_TIME_ConvertSecondsToDateTime(count_time_seconds_from_TOD+remain_seconds+86400 ,&year,&month,&day,&hour,&minute,&second);
    SYS_TIME_ConvertSecondsToDateTime(count_time_seconds_from_TOD+remain_seconds ,&year,&month,&day,&hour,&minute,&second);

    next_reload_time->year      = year;
    next_reload_time->month     = month;
    next_reload_time->day       = day;
    next_reload_time->hour      = hour;
    next_reload_time->minute    = minute;
    next_reload_time->second    = second;

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST reload_at
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadAtTime(SYS_RELOAD_OM_RELOADAT_DST reload_at, SYS_TIME_DST *next_reload_time, BOOL_T *function_active)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;
    UI32_T          count_time_seconds_from_TOD;
    UI32_T          reload_at_time_seconds_from_TOD;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( (next_reload_time == NULL) || (function_active == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (!SYS_RELOAD_MGR_IsCorrectReloatatDate(reload_at))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *function_active = SYS_TIME_IsTimeModify();
    /* get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    next_reload_time->year  = reload_at.year;
    next_reload_time->month = reload_at.month;
    next_reload_time->day   = reload_at.day;
    next_reload_time->hour  = reload_at.hour;
    next_reload_time->minute= reload_at.minute;
    next_reload_time->second= 0;

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &count_time_seconds_from_TOD);

    if (next_reload_time->month == 0)
    {
        SYS_TIME_ConvertDateTimeToSeconds(year, month, day, next_reload_time->hour, next_reload_time->minute, 0, &reload_at_time_seconds_from_TOD);
        if (reload_at_time_seconds_from_TOD < count_time_seconds_from_TOD)
        {
            /* System time is begin on 2001/1/1 00:00:00 and seconds is 86400
             */
            /* next reload time should one day after
             */
            //SYS_TIME_ConvertSecondsToDateTime(reload_at_time_seconds_from_TOD+86400+86400,&year,&month,&day,&hour,&minute,&second);
            SYS_TIME_ConvertSecondsToDateTime(reload_at_time_seconds_from_TOD+86400,&year,&month,&day,&hour,&minute,&second);
        }
        next_reload_time->year  = year;
        next_reload_time->month = month;
        next_reload_time->day   = day;
    }
    else if (next_reload_time->year == 0)
    {
        if ((next_reload_time->month ==2) && (next_reload_time->day ==29) )
        {
            /* This year is not leap year
             */
            if ( !((year%4) == 0) && (((year%100) != 0) || ((year%400) == 0)) )
            {
                /* Reload at time must in next leap year
                 */
                next_reload_time->year  = year + 4 - (year % 4);
                /* if next_reload_time->year is not leap year, the next 4 year must be leap year
                 */
                if ( !((next_reload_time->year%4) == 0) && (((next_reload_time->year%100) != 0) || ((next_reload_time->year%400) == 0)) )
                    next_reload_time->year = next_reload_time->year +4;
            }
            /* This year is leap year
             */
            else
            {
                SYS_TIME_ConvertDateTimeToSeconds(year, next_reload_time->month, next_reload_time->day, next_reload_time->hour, next_reload_time->minute, 0, &reload_at_time_seconds_from_TOD);
                /* Reload at time must in this year
                 */
                if (reload_at_time_seconds_from_TOD > count_time_seconds_from_TOD)
                {
                    next_reload_time->year = year;
                }
                else
                {
                    next_reload_time->year = year + 4;
                    /* if next_reload_time->year is not leap year, the next 4 year must be leap year
                     */
                    if ( !((next_reload_time->year%4) == 0) && (((next_reload_time->year%100) != 0) || ((next_reload_time->year%400) == 0)) )
                        next_reload_time->year = next_reload_time->year +4;
                }
            }
        }
        else
        {
            SYS_TIME_ConvertDateTimeToSeconds(year, next_reload_time->month, next_reload_time->day, next_reload_time->hour, next_reload_time->minute, 0, &reload_at_time_seconds_from_TOD);
            if (reload_at_time_seconds_from_TOD < count_time_seconds_from_TOD)
            {
                next_reload_time->year  = year+1;
            }
            else
                next_reload_time->year  = year;
        }
    }

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadRegularityTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload reload_regularity date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 *            ( 1<= reload_regularity.day_of_month <= 31)
 *            ( 1<= reload_regularity.day_of_week <= 7)
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadRegularityTime(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, SYS_TIME_DST *next_reload_time, BOOL_T *function_active)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;
    UI32_T      count_time_seconds_from_TOD;
    I32_T       suspense_remaining_reload_time;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if ( (next_reload_time == NULL) || (function_active == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (!SYS_RELOAD_MGR_IsCorrectReloatRegularityDate(reload_regularity))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    *function_active = SYS_TIME_IsTimeModify();
    /* get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &count_time_seconds_from_TOD);

    SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadRegularityTime(reload_regularity, &suspense_remaining_reload_time);

    /*Call this function to get date_time format from seconds */
    /* System time is begin on 2001/1/1 00:00:00 and seconds is 86400
     */
    //SYS_TIME_ConvertSecondsToDateTime(count_time_seconds_from_TOD+suspense_remaining_reload_time+86400,&year,&month,&day,&hour,&minute,&second);
    SYS_TIME_ConvertSecondsToDateTime(count_time_seconds_from_TOD+suspense_remaining_reload_time,&year,&month,&day,&hour,&minute,&second);

    next_reload_time->year      = year;
    next_reload_time->month     = month;
    next_reload_time->day       = day;
    next_reload_time->hour      = hour;
    next_reload_time->minute    = minute;
    next_reload_time->second    = 0;

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadTimeInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get remain time and reload time.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 * RETURN   : UI32_T
 * NOTES    : return value
 *              0 : Get value success. (remaining_reload_time is continue countdown)
 *              1 : Get value fail
 *              2 : No reload function on. (reload-in, reload-at, reload-regularity are all off)
 *              3 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (The System time is nerver changed by user or sntp)
 *              4 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (Reload at time passed)
 *              "3" and "4" may happen together, but we return "3" advanced.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_RELOAD_MGR_GetReloadTimeInfo(I32_T *remain_seconds, SYS_TIME_DST *reload_time)
{
    BOOL_T                              function_active_flag;
    SYS_RELOAD_OM_RELOADAT_DST          reload_at;
    SYS_RELOAD_OM_RELOADREGULARITY_DST  reload_regularity;

    SYS_RELOAD_MGR_USE_CSC_CHECK_OPER_MODE(SYS_RELOAD_MGR_RELOAD_TIME_INFO_INVAILD);

    if ( (remain_seconds == NULL) || (reload_time == NULL) )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_INVAILD);
    }
    if ( allowed_reload_mode == SYS_RELOAD_MGR_NoReloadFlag )
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_NO_RELOAD_FUNCTION_ON);
    }
    if ( (candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_NONE_MODE) && !(SYS_TIME_IsTimeModify()))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_NO_RELOAD_FUNCTION_WORK);
    }
    if ( (candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_NONE_MODE) && (allowed_reload_mode == SYS_RELOAD_MGR_ReloadAtFlag))
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_RELOAD_AT_TIME_PASSED);
    }

    if (candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_IN_MODE)
    {
        SYS_RELOAD_MGR_GetReloadInInfo(remain_seconds, reload_time);
    }
    else if (candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_REGULARITY_MODE)
    {
        SYS_RELOAD_MGR_GetReloadRegularityInfo(&reload_regularity, reload_time, remain_seconds, &function_active_flag);
    }
    else if (candidate_reload_mode == SYS_RELOAD_MGR_RELOAD_AT_MODE)
    {
        SYS_RELOAD_MGR_GetReloadAtInfo(&reload_at, reload_time, remain_seconds, &function_active_flag);
    }
    /* SYS_RELOAD_MGR_TimeHandler() not to do yet after set or cancel action */
    else
    {
        SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_INVAILD);
    }

    SYS_RELOAD_MGR_RETURN_AND_RELEASE_CSC(SYS_RELOAD_MGR_RELOAD_TIME_INFO_OK);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_IsCorrectReloatatDate
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to check the input date is correct reloat at time format
 *
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST date
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
static BOOL_T SYS_RELOAD_MGR_IsCorrectReloatatDate (SYS_RELOAD_OM_RELOADAT_DST date)
{
    int year   = 0;
    int month  = 0;
    int day    = 0;
    int hour   = 0;
    int minute = 0;
    int second = 0;

    /* get current time
     */
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    if (date.year == 0)
    {
        date.year = year;
    }

    if ((date.month == 0) && (date.day == 0))
    {
        date.month = month;
        date.day = day;
    }

    if (!SYS_TIME_ValidateTime(date.year, date.month, date.day, date.hour, date.minute, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_IsCorrectReloatRegularityDate
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to check the input date is correct reloat regularity time format
 *
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST date
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
static BOOL_T SYS_RELOAD_MGR_IsCorrectReloatRegularityDate (SYS_RELOAD_OM_RELOADREGULARITY_DST date)
{
    if ( (date.hour >= 24) || ( date.minute >= 60) || (date.period == SYS_RELOAD_TYPE_REGULARITY_PERIOD_NONE))
    {
        return FALSE;
    }
    /* day_of_week is from 0 to 6 */
    if ( (date.period == SYS_RELOAD_TYPE_REGULARITY_PERIOD_WEEKLY) && (date.day_of_week >=7) )
    {
        return FALSE;
    }
    /* day_of_week is from 1 to 31 */
    if ( (date.period == SYS_RELOAD_TYPE_REGULARITY_PERIOD_MONTHLY) && (date.day_of_month >31) )
    {
        return FALSE;
    }

    return TRUE;
}
