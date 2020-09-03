/*-----------------------------------------------------------------------------
 * Module Name: cfm_timer.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the timer
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    01/01/2007 - Macualey Cheng  , Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */

#ifndef _CFM_TIMER_H
#define _CFM_TIMER_H

#include "sys_type.h"
#include "l_timer.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_CFM == TRUE)
enum CFM_TIMER_E
{
    CFM_TIMER_ONE_TIME=0,
    CFM_TIMER_CYCLIC
};

//typedef BOOL_T (*funcPtr)(void *);

typedef struct CFM_Timer_CallBackPara_S
{
    UI32_T  md_index;
    UI32_T  ma_index;
    UI32_T  mep_id;
    UI32_T  seq_num;
    UI32_T  rcvd_order;
}CFM_Timer_CallBackPara_T;

typedef struct CFM_Timer_S
{
//    struct CFM_TIMER_S  *fwd;   /* link to next timer*/
//    struct CFM_TIMER_S  *bwd;   /* link to before timer*/
    UI32_T                      deltaTime;          /* delta ttl                                        */
    UI32_T                      orignalTime;        /* the timer will expired time                      */
    BOOL_T                      (*callback)(void *);/* pointer to callback fnc to call on expiration    */
    CFM_Timer_CallBackPara_T    call_back_para;     /* passed as param to callback                      */
    I16_T                       fwd_idx;            /* link to next timer                               */
    I16_T                       bwd_idx;            /* link to before timer                             */
    UI16_T                      type;               /* cycle or 1 times                                 */
}CFM_Timer_T;

typedef struct CFM_Timer_Head_S
{
    I16_T           fwd_idx;    /* point timer list first timer         */
    I16_T           bwd_idx;    /* point to timer list last timer       */
    UI32_T          cumtime;    /* the time when laster timer expired   */
}CFM_Timer_Head_T;


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_AssignTimerParameter
 *-------------------------------------------------------------------------
 * PURPOSE  : init the timer database
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_TIMER_Init(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_AssignTimerParameter
 *-------------------------------------------------------------------------
 * PURPOSE  : create timer callback parameter
 * INPUT    :md_index - the md index
 *           ma_index - the ma index
 *           mp_id - the mep id or remote mep id
 *           seq_num - the ltr sequnce number
 *           rcvd_ord - the ltr received order
 * OUTPUT   : para_p - the pointer of parameter
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_TIMER_AssignTimerParameter(
                            CFM_Timer_CallBackPara_T *para_pp,
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mp_id,
                            UI32_T seq_num,
                            UI32_T rcvd_ord);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_AddTimerToList
 *-------------------------------------------------------------------------
 * PURPOSE  : add a timer into appropriate in timer list
 * INPUT    : timer_idx - the timer index to add into list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_AddTimerToList(
    I16_T   timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_DeleteTimerFromList
 *-------------------------------------------------------------------------
 * PURPOSE  : remove timer from timer list.
 * INPUT    : timer_idx - the time index want to remove from list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_DeleteTimerFromList(
    I16_T   timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_DeleteTimerFromList
 *-------------------------------------------------------------------------
 * PURPOSE  : delete and free all timer from timer list
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE   - destroy success
 *            FALSE  - destroy fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_DestroyAllTimer(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_CreateTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : this function will create a timer and assign the parameters
 * INPUT    : callback_p - the timer's callback function
 *            para_p     - the parameter which will be used in timer's callback fucntion
 *            newTime    - the timer's original time to expire
 *            type       - one time timer or cyclic timer
 * OUTPUT   : None
 * RETURN   : timer
 * NOTE     : timer in this list are ordered in ascending timer to live (ttl)
 *            timer kept in the structure and delta from previous so that we only need
 *            check the first element on a 'tic'
 *            type = CFM_TIMER_ONE_TIME, CFM_TIMER_CYCLIC
 *-------------------------------------------------------------------------
 */
I16_T CFM_TIMER_CreateTimer(
    funcPtr                     callback_p,
    CFM_Timer_CallBackPara_T    *para_p,
    UI32_T                      newTime,
    UI16_T                      type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_DestroyTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : this function will destroy a timer
 * INPUT    : timer_idx - the time index to destroy
 * OUTPUT   : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : if the timer is not exist in this list return fail and won't process
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_DestroyTimer(
    I16_T   *timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_ModifyTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : modify the timer's attribute
 * INPUT    : timer_idx - the time index to modify
 *            newFcn_p  - the timer's callback function
 *            param_p   - the parameter which will be used in timer's callback fucntion
 *            newTime   -  the timer's original time to expire
 *            type      - one time timer or cyclic timer
 * RETURN   : None
 * NOTE     : if the newtime is 0 are unchanged
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_ModifyTimer (
    I16_T                       timer_idx,
    funcPtr                     newFcn_p,
    CFM_Timer_CallBackPara_T    *param_p,
    UI16_T                      newTime,
    UI16_T                      type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_StartTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : add timer to timer list.
 * INPUT    : timer_idx  -  the time index to add into list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_StartTimer(
    I16_T   timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_StopTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : remove timer from timer list.
 * INPUT    : timer  -  the time want to add into list
 *            head   - the head of timer list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_StopTimer(
    I16_T   timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_UpdateTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : This function update the expire time and the new callback function
 * INPUT    :
 *            callback  - the timer's callback function
 *            param     - the parameter which will be used in timer's callback fucntion
 *            newtime   -  the timer's original time to expire
 *            head      - the timer list header
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_UpdateTimer(
    I16_T       timer_idx,
    UI32_T      newTime,
    funcPtr     newFcn_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_QureyTime
 *-------------------------------------------------------------------------
 * PURPOSE  : query the max time when the  timer will time out.
 * INPUT    : timer  -  the time want to add into list
 *            head   - the head of timer list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : if the quere timer not exist, the return value is the time that all timer
 *            in the time list will expired.
 *-------------------------------------------------------------------------
 */
UI32_T CFM_TIMER_QureyTime(
    I16_T   timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_FreeTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : this function free the timer
 * INPUT    : timer  -  the time want to add into list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_TIMER_FreeTimer(
    I16_T   *timer_idx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_ProcessTimer
 *-------------------------------------------------------------------------
 * PURPOSE  :checks timer list by decrementing first timer when this goes to zero:
 *           - the related function is called.
 *           - the next element is moved to the head of the list.
 * INPUT    : head    - the head of timer list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Note that more than one entry can expire at a time.
 *-------------------------------------------------------------------------
 */
void CFM_TIMER_ProcessTimer(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_FindFirstTimerByCBFunc
 *-------------------------------------------------------------------------
 * PURPOSE  : To find the first timer with specified call back function.
 * INPUT    : src_fcn_p
 * OUTPUT   : parp_p, timer_idx_p
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_FindFirstTimerByCBFunc(
    funcPtr                     src_fcn_p,
    CFM_Timer_CallBackPara_T    *parp_p,
    I16_T                       *timer_idx_p);

#endif /*#if (SYS_CPNT_CFM == TRUE)*/
#endif /* #ifndef _CFM_TIMER_H */


