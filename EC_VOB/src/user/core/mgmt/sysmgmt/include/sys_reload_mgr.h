/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_RELOAD_MGR.H
 * ------------------------------------------------------------------------
 * PURPOSE: Management Interface in SYS RELOAD
 *
 * Notes:
 *
 *  History
 *
 *   Andy_Chang     12/24/2007      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */

#ifndef SYS_RELOAD_MGR_H
#define SYS_RELOAD_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_time.h"
#include "sys_reload_type.h"
#include "sys_reload_om.h"
/* UI message */

typedef enum
{
    SYS_RELOAD_MGR_RELOAD_TIME_INFO_OK                      = 0,
    SYS_RELOAD_MGR_RELOAD_TIME_INFO_INVAILD                 = 1,
    SYS_RELOAD_MGR_RELOAD_TIME_INFO_NO_RELOAD_FUNCTION_ON   = 2,
    SYS_RELOAD_MGR_RELOAD_TIME_INFO_NO_RELOAD_FUNCTION_WORK = 3,
    SYS_RELOAD_MGR_RELOAD_TIME_INFO_RELOAD_AT_TIME_PASSED   = 4
} SYS_RELOAD_MGR_RELOAD_TIME_INFO_E;

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
void SYS_RELOAD_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION	NAME - SYS_RELOAD_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE	: This function	initializes	all	function pointer registration operations.
 * INPUT	: none
 * OUTPUT	: none
 * RETURN	: none
 * NOTES	: none
 *--------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_Create_InterCSC_Relation(void);

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
BOOL_T SYS_RELOAD_MGR_SetReloadIn(UI32_T minute, UI8_T *reason);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST reload_at, UI8_T *reason);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, UI8_T *reason);

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
BOOL_T SYS_RELOAD_MGR_GetReloadInInfo(I32_T *remain_seconds, SYS_TIME_DST *next_reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadAtInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST reload_at
 *            SYS_TIME_DST  next_reload_time
 *            I32_T *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_GetReloadRegularityInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 *            SYS_TIME_DST  next_reload_time
 *            I32_T *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active);

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
UI32_T SYS_RELOAD_MGR_GetReloadTimeInfo(I32_T *remain_seconds, SYS_TIME_DST *reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_TimeHandler
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to handle time event and count remain reload time
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_MGR_TimeHandler();

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
void SYS_RELOAD_MGR_Register_ReloadNotify_CallBack(void (*fun)(UI32_T remaining));

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
BOOL_T SYS_RELOAD_MGR_ReloadInCancel(void);

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
BOOL_T SYS_RELOAD_MGR_ReloadAtCancel(void);

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
BOOL_T SYS_RELOAD_MGR_ReloadRegularityCancel(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into master mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterMasterMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into slave mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYS_RELOAD_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set sys_reload_mgr into transition mode.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : none.
 * NOTES    :
 * -------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_EnterTransitionMode(void);

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
void SYS_RELOAD_MGR_ProvisionComplete(void);

/*------------------------------------------------------------------------------
 * Function : SYS_RELOAD_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void SYS_RELOAD_MGR_SetTransitionMode(void);

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
SYS_TYPE_Stacking_Mode_T SYS_RELOAD_MGR_GetOperationMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_GetRunningReloadAtInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_RELOAD_MGR_GetRunningReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at);

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
SYS_TYPE_Get_Running_Cfg_T SYS_RELOAD_MGR_GetRunningReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadInTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : I32_T remain_seconds
 * OUTPUT   : SYS_TIME_DST *next_reload_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadInTime(I32_T remain_seconds, SYS_TIME_DST *next_reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST reload_at
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadAtTime(SYS_RELOAD_OM_RELOADAT_DST reload_at, SYS_TIME_DST *next_reload_time, BOOL_T *function_active);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_QueryNextReloadRegularityTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload reload_regularity date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 *            ( 1<= reload_regularity.day_of_month <= 31)
 *            ( 1<= reload_regularity.day_of_week <= 7)
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_RELOAD_MGR_QueryNextReloadRegularityTime(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, SYS_TIME_DST *next_reload_time, BOOL_T *function_active);

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
BOOL_T SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadAtTime(SYS_RELOAD_OM_RELOADAT_DST reload_at, I32_T *difference_time);

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
BOOL_T SYS_RELOAD_MGR_GetTimeDiffBetweenCurrentTimeAndReloadRegularityTime(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, I32_T *difference_time);
#endif /* END OF SYS_RELOAD_MGR_H */
