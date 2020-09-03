/* MODULE NAME: mldsnp_timer.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - Give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitation}
*    {4. Any performance limitation}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/


/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_timer.h"
#include "l_mm.h"
#include "mldsnp_engine.h"
#include "mldsnp_querier.h"
#include "mldsnp_unknown.h"
/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
static MLDSNP_TimerHead_T mldsnp_timer_head_g;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_TIMER_Init
 *-------------------------------------------------------------------------
 * PURPOSE : This function is init the resource engine hold
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_TIMER_Init()
{
    /*init timer list head*/
    memset(&mldsnp_timer_head_g, 0, sizeof(mldsnp_timer_head_g));

}/*End of MLDSNP_TIMER_Init*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_TIMER_Local_AddTimerToList
 *-------------------------------------------------------------------------
 * PURPOSE  : add a timer into appropriate in timer list
 * INPUT    : timer   -  the time want to add into list
 *            head    - the head of timer list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_TIMER_Local_AddTimerToList(
    MLDSNP_Timer_T *timer_p)
{
    MLDSNP_Timer_T *tmp_timer_p;
    UI32_T odelta, cum;

    /*it can't add ther timer_p with original time=0, at least it need 1*/
    if (0 == timer_p->orgianl_time)
    {
        timer_p->orgianl_time = 1;
    }

    /* if list empty,be the first timer_p of  the list */
    if (NULL == mldsnp_timer_head_g.fwd_p)
    {
        timer_p->fwd_p                = NULL;
        timer_p->bwd_p                = NULL;
        mldsnp_timer_head_g.fwd_p   = timer_p;
        mldsnp_timer_head_g.bwd_p   = timer_p;
        timer_p->delta_time           = timer_p->orgianl_time;
        mldsnp_timer_head_g.cumtime = timer_p->orgianl_time;
    }
    /* this timre expired time afte the last timer_p expired time in this list */
    else if (timer_p->orgianl_time >= mldsnp_timer_head_g.cumtime)
    {
        /* put at end of list */
        tmp_timer_p                    = mldsnp_timer_head_g.bwd_p; /* old end */
        mldsnp_timer_head_g.bwd_p   = timer_p;
        tmp_timer_p->fwd_p             = timer_p;
        timer_p->fwd_p                = NULL;
        timer_p->bwd_p                = tmp_timer_p;
        timer_p->delta_time           = timer_p->orgianl_time - mldsnp_timer_head_g.cumtime;
        mldsnp_timer_head_g.cumtime = timer_p->orgianl_time;
    }
    else /*the timer_p is in the middle of timer_p list*/
    {
        /* find where to put this in the order */
        for (tmp_timer_p = mldsnp_timer_head_g.fwd_p, cum = 0; tmp_timer_p != NULL; tmp_timer_p = tmp_timer_p->fwd_p)
        {
            cum += tmp_timer_p->delta_time;
            if (cum > timer_p->orgianl_time)
            {
                break;
            }
        }
        /* link in front of tmp_timer_p (if there is one) */
        if (tmp_timer_p)
        {
            timer_p->fwd_p    = tmp_timer_p;
            timer_p->bwd_p    = tmp_timer_p->bwd_p;
            tmp_timer_p->bwd_p = timer_p;

            if (timer_p->bwd_p)
            {
                timer_p->bwd_p->fwd_p = timer_p;
            }
            else
            {
                mldsnp_timer_head_g.fwd_p = timer_p;
            }

            odelta               = tmp_timer_p->delta_time;
            tmp_timer_p->delta_time = cum - timer_p->orgianl_time;
            timer_p->delta_time    = odelta - tmp_timer_p->delta_time;
        }
        else
        {
            printf("MLDSNP_TIMER_Local_AddTimerToList: bad cum");
        }
    }
    return TRUE;
}/*End of MLDSNP_TIMER_Local_AddTimerToList*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_TIMER_Local_DeleteTimerFromList
 *-------------------------------------------------------------------------
 * PURPOSE  : remove timer from timer list.
 * INPUT    : timer  -  the time want to add into list
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_TIMER_Local_DeleteTimerFromList(MLDSNP_Timer_T *timer_p)
{

    MLDSNP_Timer_T *tmp_timer_p;

    /* verify it really is on list */
    for (tmp_timer_p = mldsnp_timer_head_g.fwd_p; tmp_timer_p != NULL; tmp_timer_p = tmp_timer_p->fwd_p)
    {
        if (tmp_timer_p == timer_p)
            break;
    }

    /* if not on active list, return */
    if (NULL == tmp_timer_p)
    {
        return FALSE;
    }

    /* first adjust delta time of following entry */
    if (NULL != timer_p->fwd_p)
    {
        timer_p->fwd_p->delta_time += timer_p->delta_time;
    }
    else
    {
        /* is last on list, adjust cumtime */
        mldsnp_timer_head_g.cumtime -= timer_p->delta_time;
    }

    /* now unlink */
    if (NULL != timer_p->bwd_p)
    {
        timer_p->bwd_p->fwd_p = timer_p->fwd_p;
    }
    else /*unlink the first timer_p in list*/
    {
        mldsnp_timer_head_g.fwd_p = timer_p->fwd_p;
    }

    if (NULL != timer_p->fwd_p)
    {
        timer_p->fwd_p->bwd_p = timer_p->bwd_p;
    }
    else /*the last timer_p in list*/
    {
        mldsnp_timer_head_g.bwd_p = timer_p->bwd_p;
    }

    return TRUE;
}/*End of MLDSNP_TIMER_Local_DeleteTimerFromList*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_TIMER_EnterTransition
 *-------------------------------------------------------------------------
 * PURPOSE : This function enter transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_TIMER_EnterTransition()
{
    MLDSNP_Timer_T *cur_timer_p, *free_timer_p;

    cur_timer_p = mldsnp_timer_head_g.fwd_p;
    /* verify it really is on list */
    while (NULL != cur_timer_p)
    {
        free_timer_p = cur_timer_p;
        cur_timer_p  = cur_timer_p->fwd_p;

        L_MM_Free(free_timer_p);
    }

    mldsnp_timer_head_g.fwd_p = NULL;
    mldsnp_timer_head_g.bwd_p = NULL;

    return;
}/*End of MLDSNP_TIMER_EnterTransition*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_TimerTickProcess
*------------------------------------------------------------------------------
* Purpose: This function process the timer
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_TIMER_TimerTickProcess()
{
    MLDSNP_Timer_T *timer_p;
    BOOL_T(*cb)(void *);

    timer_p = mldsnp_timer_head_g.fwd_p;

    /* decrement head */
    if (NULL != timer_p)
    {
        timer_p->delta_time --;
        mldsnp_timer_head_g.cumtime --;
    }
    else
    {
        return;
    }

    while ((NULL != timer_p)
            && (timer_p->delta_time == 0))
    {
        MLDSNP_TIMER_Local_DeleteTimerFromList(timer_p);

        /* restart if cyclic */
        if (MLDSNP_TIMER_CYCLIC == timer_p->type)
        {
            MLDSNP_TIMER_Local_AddTimerToList(timer_p);
        }

        /* do callback */
        /* callback may call timerdelete */
        cb = timer_p->func_p;
        (*cb)(&timer_p->param);

        timer_p = mldsnp_timer_head_g.fwd_p;
    }
    return;
}/*End of MLDSNP_TIMER_TimerTickProcess*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_LocalCreateTimer
*------------------------------------------------------------------------------
* Purpose: This function create the timer
* INPUT  : vid          - the vlan id
*          *gip_ap      - the group ip array pointer
*          *sip_list_ap - the source ip array list pointer
*          num_of_src   - the num of src ip in list
*          interval     - the interval
*          type         - the timer type
* OUTPUT : **timer_p    - the created timer
* RETURN : TRUE  - success
*          FALSE - fail
* NOTES  : MLDSNP_TIMER_Local_ONE_TIME, MLDSNP_TIMER_CYCLIC
*          This parameter will be creata too.
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_TIMER_LocalCreateTimer(
    mldsnp_timer_func_t         callback,
    UI16_T         vid,
    const UI8_T    *gip_ap,
    const UI8_T    *sip_list_ap,
    UI16_T         num_of_src,
    UI16_T         lport,
    UI32_T         interval,
    UI16_T         type,
    MLDSNP_Timer_T **timer_p)
{

    MLDSNP_Timer_T *new_timer_p = NULL;

    /* try to alloc struct.  (num_of_src+1) plus one for (vid, gip, 0), because don't let sip be null*/
    if (NULL == (new_timer_p = (MLDSNP_Timer_T *)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(sizeof(MLDSNP_Timer_T) + (num_of_src + 1) * MLDSNP_TYPE_IPV6_SRC_IP_LEN),
                               L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_CREATE_TIMER))))
    {
        return FALSE;
    }

    if (NULL == gip_ap)
    {
        memset(new_timer_p->param.gip_a, 0, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    }
    else
    {
        memcpy(new_timer_p->param.gip_a, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    }

    new_timer_p->param.num_of_src  = num_of_src;
    new_timer_p->param.lport       = lport;
    new_timer_p->param.vid         = vid;
    new_timer_p->param.create_time = SYS_TIME_GetSystemTicksBy10ms();

    if (NULL != sip_list_ap && 0 != num_of_src)
    {
        /*timer_para_p->sip_list_p is the sequential memory space after timer_para_p->gip*/
        memcpy(&new_timer_p->param.sip_list_a, sip_list_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN*num_of_src);
    }
    else
        memset(&new_timer_p->param.sip_list_a, 0, MLDSNP_TYPE_IPV6_DST_IP_LEN);

    new_timer_p->param.self_p = new_timer_p; /*record itself*/
    new_timer_p->func_p       = callback;
    new_timer_p->type         = type;
    new_timer_p->orgianl_time = interval;
    new_timer_p->bwd_p        = NULL;
    new_timer_p->fwd_p        = NULL;

    *timer_p = new_timer_p;
    return TRUE;
}/*End of MLDSNP_TIMER_LocalCreateTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_FreeTimer
*------------------------------------------------------------------------------
* Purpose: This function free the timer allocated memory
* INPUT  : *timer_p - the timer will be freed
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_FreeTimer(MLDSNP_Timer_T **timer_pp)
{
    if (*timer_pp == NULL)
    {
        return TRUE;
    }

    /*if(NULL!=(*timer_pp))*/
    {
        L_MM_Free(*timer_pp);
        (*timer_pp) = NULL;
    }

    return TRUE;
}/*End of MLDSNP_TIMER_FreeTimer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_ModifyTimer
*------------------------------------------------------------------------------
* Purpose: This function modify the timer
* INPUT :  **timer_p - the timer
*          new_func  - the timer's new callback function
*          new_time  - the new time
*          type      - one time timer or cyclic timer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_ModifyTimer(
    MLDSNP_Timer_T  **timer_pp,
    mldsnp_timer_func_t    new_fun,
    UI16_T     new_time,
    UI32_T     type)
{
    if (NULL == timer_pp || NULL == *timer_pp)
        return FALSE;

    if (MLDSNP_TIMER_ONE_TIME != type && MLDSNP_TIMER_CYCLIC != type)
    {
        return FALSE;
    }

    if (0 == new_time)
        return FALSE;

    MLDSNP_TIMER_Local_DeleteTimerFromList(*timer_pp);

    (*timer_pp)->func_p       = new_fun;
    (*timer_pp)->type         = type;
    (*timer_pp)->orgianl_time = new_time;

    MLDSNP_TIMER_Local_AddTimerToList(*timer_pp);

    return TRUE;
}/*End of MLDSNP_TIMER_ModifyTimer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_StartTimer
*------------------------------------------------------------------------------
* Purpose: This function add and start the timer to timer list
* INPUT  : *timer_p - the timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_StartTimer(
    MLDSNP_Timer_T *new_timer_p)
{
    if (NULL == new_timer_p)
        return FALSE;

    return MLDSNP_TIMER_Local_AddTimerToList(new_timer_p);
}/*End of MLDSNP_TIMER_StartTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_StopTimer
*------------------------------------------------------------------------------
* Purpose: This function stop and delete the timer to timer list
* INPUT  : *timer_p - the timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : if the timer needn't use, you shall call MLDSNP_TIMER_FreeTimer, too
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_StopTimer(
    MLDSNP_Timer_T   *timer_p)
{
    if (NULL == timer_p)
        return TRUE;

    /* unlink it */
    if (MLDSNP_TIMER_Local_DeleteTimerFromList(timer_p) == FALSE)
    {
        return FALSE;
    }

    timer_p->bwd_p = NULL;
    timer_p->fwd_p = NULL;

    return TRUE;
}/*End of MLDSNP_TIMER_StopTimer*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_StopAndFreeTimer
*------------------------------------------------------------------------------
* Purpose: This function stop and free the timers
* INPUT  : *timer_p - the timer pointer
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_TIMER_StopAndFreeTimer(
    MLDSNP_Timer_T **timer_p)
{
    if (NULL == *timer_p)
        return;

    /* unlink it */
    MLDSNP_TIMER_Local_DeleteTimerFromList(*timer_p);

    (*timer_p)->bwd_p = NULL;
    (*timer_p)->fwd_p = NULL;

    /*if(NULL!=(*timer_p))*/
    {
        L_MM_Free(*timer_p);
        (*timer_p) = NULL;
    }

    return;
}/*End of MLDSNP_TIMER_StopAndFreeTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_CreateAndStartTimer
*------------------------------------------------------------------------------
* Purpose: This function create and start the timer
* INPUT  : vid          - the vlan id
*          *gip_ap      - the group ip array pointer
*          *sip_list_ap - the source ip array pointer
*          num_of_srcip - the number of source ip in list
*          interval     - the interval
*          type         - the timer type
* OUTPUT : **timer_p    - the created timer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : MLDSNP_TIMER_Local_ONE_TIME, MLDSNP_TIMER_CYCLIC
*          This parameter will be creata too.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_CreateAndStartTimer(
    mldsnp_timer_func_t         callback,
    UI16_T         vid,
    const UI8_T    *gip_ap,
    const UI8_T    *sip_list_ap,
    UI16_T         num_of_src,
    UI16_T         lport,
    UI32_T         interval,
    UI16_T         type,
    MLDSNP_Timer_T **timer_pp)
{
    if (NULL == timer_pp)
        return FALSE;

    if (*timer_pp != NULL)
    {
        printf("Timer not free but won't to allocate another one.\r\n vid=%d, port=%d", vid, lport);
        MLDSNP_TIMER_StopAndFreeTimer(timer_pp);
    }

    if (FALSE == MLDSNP_TIMER_LocalCreateTimer(callback, vid, gip_ap, sip_list_ap, num_of_src, lport, interval, type, timer_pp))
    {
        *timer_pp = NULL;
        return FALSE;
    }

    /*if(NULL == *timer_pp)
        return FALSE;*/

    return MLDSNP_TIMER_Local_AddTimerToList(*timer_pp);
}/*end of MLDSNP_TIMER_CreateAndStartTimer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_UpdateTimerNewTime
*------------------------------------------------------------------------------
* Purpose: This function update the timer in timer list
* INPUT  : *timer_p - the new timer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_UpdateTimerNewTime(
    MLDSNP_Timer_T *timer_p,
    UI32_T    new_time)
{
    if (NULL == timer_p)
    {
        return FALSE;
    }

    /* remove from list */
    if (FALSE == MLDSNP_TIMER_Local_DeleteTimerFromList(timer_p))
    {
        return FALSE;
    }

    /* new params */
    timer_p->orgianl_time = new_time;

    /* add back */
    return MLDSNP_TIMER_Local_AddTimerToList(timer_p);
}/*End of MLDSNP_TIMER_UpdateTimerNewTime*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_QueryTimer
*------------------------------------------------------------------------------
* Purpose: This function get the timer still have how many sec to timeout
* INPUT  : *timer_p - the timer
* OUTPUT : r_time   - the sec last
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_QueryTimer(
    MLDSNP_Timer_T *timer_p,
    UI32_T    *r_time)
{
    MLDSNP_Timer_T *tmpTimer;
    UI16_T  ttl = 0;

    *r_time = 0;

    if (NULL == timer_p)
    {
        return FALSE;
    }

    for (tmpTimer = mldsnp_timer_head_g.fwd_p; tmpTimer; tmpTimer = tmpTimer->fwd_p)
    {
        ttl += tmpTimer->delta_time;

        if (tmpTimer == timer_p)
        {
            *r_time = ttl;
            return TRUE;
        }
    }

    return FALSE;
}/*End of MLDSNP_TIMER_UpdateTimerNewTime*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_GetTimerHeadPtr
*------------------------------------------------------------------------------
* Purpose: This function get the timer list header pointer
* INPUT  : *timer_p - the timer
* OUTPUT : r_time   - the sec last
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :only for backdoor use
*------------------------------------------------------------------------------*/
MLDSNP_TimerHead_T * MLDSNP_TIMER_GetTimerHeadPtr()
{
    return &mldsnp_timer_head_g;
}
