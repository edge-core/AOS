/*-----------------------------------------------------------------------------
 * Module Name: cfm_timer.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Define the timer
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
#include <stdio.h>
#include <string.h>
#include "cfm_timer.h"
#include "sys_adpt.h"

#if (SYS_CPNT_CFM == TRUE)

static  CFM_Timer_T         cfm_timer_pool[SYS_ADPT_CFM_MAX_NBR_OF_TIMER];
static  I16_T               free_timer_head_idx;
static  CFM_Timer_Head_T    cfm_timer_list_head;

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
void CFM_TIMER_Init(void)
{
    CFM_TIMER_DestroyAllTimer();
}


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
                            UI32_T rcvd_ord)
{
    para_pp->md_index=md_index;
    para_pp->ma_index = ma_index;
    para_pp->mep_id=mp_id;
    para_pp->seq_num= seq_num;
    para_pp->rcvd_order= rcvd_ord;

    return;
}/*End of CFM_TIMER_AssignTimerParameter*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_LocalAddTimerToList
 *-------------------------------------------------------------------------
 * PURPOSE  : add a timer into appropriate in timer list
 * INPUT    : timer_idx - the timer index to add into list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_TIMER_LocalAddTimerToList(
    I16_T   timer_idx)
{
    CFM_Timer_T     *tmp_timer_p;
    BOOL_T          ret = FALSE;

    if ((0 <= timer_idx ) && (timer_idx <SYS_ADPT_CFM_MAX_NBR_OF_TIMER))
    {
        ret = TRUE;
        tmp_timer_p = &cfm_timer_pool[timer_idx];

        /*it can't add ther timer_idx with original time=0, at least it need 1*/
        if (0 == tmp_timer_p->orignalTime)
        {
            tmp_timer_p->orignalTime = 1;
        }

        /* if list empty, be the first timer_idx of the list */
        if (-1 == cfm_timer_list_head.fwd_idx)
        {
            tmp_timer_p->fwd_idx = -1;
            tmp_timer_p->bwd_idx = -1;
            tmp_timer_p->deltaTime = tmp_timer_p->orignalTime;
            cfm_timer_list_head.fwd_idx = timer_idx;
            cfm_timer_list_head.bwd_idx = timer_idx;
            cfm_timer_list_head.cumtime = tmp_timer_p->orignalTime;
        }
        /* this timre expired time afte the last timer_idx expired time in this list */
        else if (tmp_timer_p->orignalTime >= cfm_timer_list_head.cumtime)
        {
            /* put at end of list */
            cfm_timer_pool[cfm_timer_list_head.bwd_idx].fwd_idx = timer_idx;
            tmp_timer_p->bwd_idx   = cfm_timer_list_head.bwd_idx;
            cfm_timer_list_head.bwd_idx = timer_idx;
            tmp_timer_p->fwd_idx   = -1;
            tmp_timer_p->deltaTime = tmp_timer_p->orignalTime - cfm_timer_list_head.cumtime;
            cfm_timer_list_head.cumtime = tmp_timer_p->orignalTime;
        }
        else /*the timer_idx is in the middle of timer_idx list*/
        {
            I16_T   cur_timer_idx;
            UI32_T  odelta, cum =0;

            cur_timer_idx = cfm_timer_list_head.fwd_idx;
            while (cur_timer_idx >= 0)
            {
                cum += cfm_timer_pool[cur_timer_idx].deltaTime;
                if(cum > tmp_timer_p->orignalTime)
                {
                    break;
                }

                cur_timer_idx = cfm_timer_pool[cur_timer_idx].fwd_idx;
            }

            /* link in front of cur_timer_idx (if there is one) */
            if(cur_timer_idx >= 0)
            {
                tmp_timer_p->fwd_idx = cur_timer_idx;
                tmp_timer_p->bwd_idx = cfm_timer_pool[cur_timer_idx].bwd_idx;

                if (cfm_timer_pool[cur_timer_idx].bwd_idx >= 0)
                {
                    cfm_timer_pool[cfm_timer_pool[cur_timer_idx].bwd_idx].fwd_idx = timer_idx;
                }
                else
                {
                    cfm_timer_list_head.fwd_idx = timer_idx;
                }

                cfm_timer_pool[cur_timer_idx].bwd_idx = timer_idx;

                /* the delta time between the one in front of new and current one
                 */
                odelta = cfm_timer_pool[cur_timer_idx].deltaTime;

                /* udpate the delta time between new and current one
                 */
                cfm_timer_pool[cur_timer_idx].deltaTime = cum - tmp_timer_p->orignalTime;

                /* udpate the delta time between new and the one in front of new                 */
                tmp_timer_p->deltaTime = odelta - cfm_timer_pool[cur_timer_idx].deltaTime;
            }
            else
            {
                printf("CFM_TIMER_AddTimerToList: bad cum");
                ret = FALSE;
            }
        }
    }

    return ret;
}/*End of CFM_TIMER_LocalAddTimerToList*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_LocalDeleteTimerFromList
 *-------------------------------------------------------------------------
 * PURPOSE  : remove timer from timer list.
 * INPUT    : timer_idx - the time index want to remove from list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_TIMER_LocalDeleteTimerFromList(
    I16_T   timer_idx)
{
    I16_T   cur_timer_idx;
    BOOL_T  ret = FALSE;

    cur_timer_idx = cfm_timer_list_head.fwd_idx;
    while (cur_timer_idx >= 0)
    {
        if (cur_timer_idx == timer_idx)
        {
            break;
        }

        cur_timer_idx = cfm_timer_pool[cur_timer_idx].fwd_idx;
    }

    if (cur_timer_idx >= 0)
    {
        /* set prev one's fwd_idx to my fwd_idx if prev one exists
         */
        if (cfm_timer_pool[cur_timer_idx].bwd_idx >= 0)
        {
            cfm_timer_pool[cfm_timer_pool[cur_timer_idx].bwd_idx].fwd_idx =
                                    cfm_timer_pool[cur_timer_idx].fwd_idx;
        }

        /* set next one's bwd_idx to my bwd_idx if next one exists
         */
        if (cfm_timer_pool[cur_timer_idx].fwd_idx >= 0)
        {
            cfm_timer_pool[cfm_timer_pool[cur_timer_idx].fwd_idx].bwd_idx=
                                    cfm_timer_pool[cur_timer_idx].bwd_idx;

            /* first adjust delta time of following entry */
            cfm_timer_pool[cfm_timer_pool[cur_timer_idx].fwd_idx].deltaTime +=
            cfm_timer_pool[cur_timer_idx].deltaTime;
        }

        if (cfm_timer_pool[cur_timer_idx].fwd_idx == -1)
        {
            /* remove the last one
             */
            cfm_timer_list_head.bwd_idx  = cfm_timer_pool[cur_timer_idx].bwd_idx;
            /* is last on list, adjust cumtime */
            cfm_timer_list_head.cumtime -= cfm_timer_pool[cur_timer_idx].deltaTime;
        }

        if (cfm_timer_pool[cur_timer_idx].bwd_idx == -1)
        {
            /* remove the first one
             */
            cfm_timer_list_head.fwd_idx = cfm_timer_pool[cur_timer_idx].fwd_idx;
        }

        ret = TRUE;
    }

    return ret;
}/*End of CFM_TIMER_LocalDeleteTimerFromList*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TIMER_DeleteTimerFromList
 *-------------------------------------------------------------------------
 * PURPOSE  : delete and free all timer from timer list
 * INPUT    : head   - the head of timer list
 * OUTPUT   : None
 * RETURN   : TRUE   - destroy success
 *            FALSE  - destroy fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TIMER_DestroyAllTimer()
{
    UI32_T i;

    memset(cfm_timer_pool, 0 , sizeof(cfm_timer_pool));
    for(i =0; i <SYS_ADPT_CFM_MAX_NBR_OF_TIMER-1; i++)
    {
        cfm_timer_pool[i].fwd_idx = i+1;
    }

    cfm_timer_pool[i].fwd_idx   = -1;
    free_timer_head_idx         = 0;
    cfm_timer_list_head.fwd_idx = -1;
    cfm_timer_list_head.bwd_idx = -1;
    cfm_timer_list_head.cumtime = 0;

    return TRUE;
}/*End of CFM_TIMER_DestroyAllTimer*/

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
    UI16_T                      type)
{
    I16_T   new_timer_idx;

    new_timer_idx = free_timer_head_idx;

    if (new_timer_idx >= 0)
    {
        free_timer_head_idx = cfm_timer_pool[new_timer_idx].fwd_idx;
        cfm_timer_pool[new_timer_idx].callback      = callback_p;
        cfm_timer_pool[new_timer_idx].orignalTime   = newTime;
        cfm_timer_pool[new_timer_idx].type          = type;
        cfm_timer_pool[new_timer_idx].fwd_idx= -1;
        cfm_timer_pool[new_timer_idx].bwd_idx= -1;
        if (NULL != para_p)
        {
            cfm_timer_pool[new_timer_idx].call_back_para= *para_p;
        }
    }

    return new_timer_idx;
}/*End of CFM_TIMER_CreateTimer*/

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
    I16_T   *timer_idx)
{
    I16_T   tmp_timer_idx = *timer_idx;
    BOOL_T  ret = FALSE;

    /*remove from this list*/
    if (TRUE == CFM_TIMER_LocalDeleteTimerFromList(tmp_timer_idx))
    {
        memset(&cfm_timer_pool[tmp_timer_idx], 0, sizeof (cfm_timer_pool[tmp_timer_idx]));
        cfm_timer_pool[tmp_timer_idx].fwd_idx = free_timer_head_idx;
        free_timer_head_idx = tmp_timer_idx;
        *timer_idx = -1;
    }

    return ret;
}

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
    UI16_T                      type)
{
    BOOL_T  ret = FALSE;

    if (   (newTime > 0) && (NULL != param_p)
        && (NULL != newFcn_p)
        && ((CFM_TIMER_ONE_TIME == type || CFM_TIMER_CYCLIC == type))
       )
    {
        if (TRUE == CFM_TIMER_LocalDeleteTimerFromList(timer_idx))
        {
            CFM_Timer_T     *new_timer_p;

            new_timer_p = &cfm_timer_pool[timer_idx];

            new_timer_p->type       = type;
            new_timer_p->callback   = newFcn_p;
            new_timer_p->orignalTime= newTime;
            cfm_timer_pool[timer_idx].call_back_para= *param_p;

            ret = CFM_TIMER_LocalAddTimerToList(timer_idx);
        }
    }

    return ret;
}/*End of CFM_TIMER_ModifyTimer*/


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
    I16_T   timer_idx)
{
    return CFM_TIMER_LocalAddTimerToList(timer_idx);
}

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
    I16_T   timer_idx)
{
    return CFM_TIMER_LocalDeleteTimerFromList(timer_idx);
}


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
    funcPtr     newFcn_p)
{
    BOOL_T  ret = FALSE;

    /* remove from list */
    if(TRUE == CFM_TIMER_LocalDeleteTimerFromList(timer_idx))
    {
        CFM_Timer_T     *new_timer_p;

        new_timer_p = &cfm_timer_pool[timer_idx];

        new_timer_p->orignalTime  = newTime;
        new_timer_p->callback     = newFcn_p;

        /* add back */
        ret = CFM_TIMER_LocalAddTimerToList(timer_idx);
    }

    return ret;
}

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
    I16_T   timer_idx)
{
    I16_T   cur_timer_idx;
    UI32_T  ret =0, sum =0;

    cur_timer_idx = cfm_timer_list_head.fwd_idx;
    while (cur_timer_idx >= 0)
    {
        sum += cfm_timer_pool[cur_timer_idx].deltaTime;

        if (cur_timer_idx == timer_idx)
        {
            ret = sum;
            break;
        }
        cur_timer_idx =  cfm_timer_pool[cur_timer_idx].fwd_idx;
    }

    return ret;
}/*End of CFM_TIMER_QureyTime*/

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
    I16_T   *timer_idx)
{
    I16_T   tmp_timer_idx = *timer_idx;

    if ((0 <= tmp_timer_idx) && (tmp_timer_idx <SYS_ADPT_CFM_MAX_NBR_OF_TIMER))
    {
        memset(&cfm_timer_pool[tmp_timer_idx], 0, sizeof (cfm_timer_pool[tmp_timer_idx]));
        cfm_timer_pool[tmp_timer_idx].fwd_idx = free_timer_head_idx;
        free_timer_head_idx = tmp_timer_idx;

        *timer_idx = -1;
    }
}/*End of CFM_TIMER_FreeTimer*/

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
void CFM_TIMER_ProcessTimer(void)
{
    CFM_Timer_T     *cur_timer_p;
    I16_T           cur_timer_idx;
    BOOL_T          (*cb)(void *);

    cur_timer_idx = cfm_timer_list_head.fwd_idx;
    if (cur_timer_idx >= 0)
    {
        cfm_timer_list_head.cumtime --;
        cur_timer_p = &cfm_timer_pool[cur_timer_idx];
        cur_timer_p->deltaTime --;

        while (cur_timer_idx >= 0)
        {
            cur_timer_p = &cfm_timer_pool[cur_timer_idx];
            if (cur_timer_p->deltaTime != 0)
            {
                break;
            }

            CFM_TIMER_LocalDeleteTimerFromList(cur_timer_idx);

            /* restart if cyclic */
            if(CFM_TIMER_CYCLIC == cur_timer_p->type)
            {
                CFM_TIMER_LocalAddTimerToList(cur_timer_idx);
            }

            /* do callback */
            /* callback may call timerdelete */
            cb = cur_timer_p->callback;
            (*cb)(&cur_timer_p->call_back_para);
            cur_timer_idx = cfm_timer_list_head.fwd_idx;
        }
    }
}/*End of CFM_TIMER_ProcessTimer*/

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
    I16_T                       *timer_idx_p)
{
    I16_T           cur_timer_idx;
    BOOL_T          ret = FALSE;

    if ((NULL != src_fcn_p) && (NULL != parp_p) && (NULL != timer_idx_p))
    {
        cur_timer_idx = cfm_timer_list_head.fwd_idx;
        while (cur_timer_idx >= 0)
        {
            if (cfm_timer_pool[cur_timer_idx].callback == src_fcn_p)
            {
                memcpy(parp_p, &cfm_timer_pool[cur_timer_idx].call_back_para,
                        sizeof(CFM_Timer_CallBackPara_T));
                *timer_idx_p = cur_timer_idx;
                ret = TRUE;
                break;
            }

            cur_timer_idx = cfm_timer_pool[cur_timer_idx].fwd_idx;
        }
    }

    return ret;
}/*End of CFM_TIMER_FindFirstTimerByCBFunc*/
#endif /*#if (SYS_CPNT_CFM == TRUE)*/

