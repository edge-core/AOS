/* MODULE NAME: mldsnp_timer.H
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


#ifndef _MLDSNP_TIMER_H
#define _MLDSNP_TIMER_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_type.h"

/* NAMING CONSTANT DECLARATIONS
*/
/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

enum MLDSNP_TYPE_TIMER_E
{
    MLDSNP_TIMER_ONE_TIME=0,
    MLDSNP_TIMER_CYCLIC
};

typedef struct MLDSNP_TIMER_TimerPara_S
{
    UI16_T vid;
    UI16_T lport;
    UI16_T num_of_src;
    UI32_T create_time;

    struct MLDSNP_Timer_S *self_p;

    UI8_T  gip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];
    UI8_T  sip_list_a[0];
}MLDSNP_TIMER_TimerPara_T;

typedef BOOL_T (*mldsnp_timer_func_t)(void *);

typedef struct MLDSNP_Timer_S
{
	struct MLDSNP_Timer_S	*fwd_p; 	    /* link to next timer*/
	struct MLDSNP_Timer_S	*bwd_p; 	    /* link to before timer*/
	UI32_T	 delta_time;	                /* delta ttl */
	UI32_T   orgianl_time;                  /* the timer will expired time */
	UI16_T   type;		                    /* cycle or 1 times */
	BOOL_T   (*func_p)(void *);             /*pointer to func_p fnc to call on expiration */
	struct MLDSNP_TIMER_TimerPara_S param;  /* a pointer to a generic struct,passed as param to func_p */
}MLDSNP_Timer_T;


typedef struct MLDSNP_TimerHead_S
{
    MLDSNP_Timer_T   *fwd_p;    /*point timer list first timer*/
    MLDSNP_Timer_T   *bwd_p;    /*point to timer list last timer*/
    UI32_T           cumtime;   /*the time when laster timer expired*/
}MLDSNP_TimerHead_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_Init
 *-------------------------------------------------------------------------
 * PURPOSE : This function is init the resource engine hold
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_TIMER_Init();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_EnterTransition
 *-------------------------------------------------------------------------
 * PURPOSE : This function enter transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_TIMER_EnterTransition();
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_TimerTickProcess
*------------------------------------------------------------------------------
* Purpose: This function process the timer
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_TIMER_TimerTickProcess();


/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_FreeTimer
*------------------------------------------------------------------------------
* Purpose: This function free the timer allocated memory
* INPUT  : *timer_p - the timer will be free
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_FreeTimer(MLDSNP_Timer_T **timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_ModifyTimer
*------------------------------------------------------------------------------
* Purpose: This function modify the timer
* INPUT :  **timer_p  - the timer
*          new_func  - the timer's new callback function
*          new_time - the new time
*          type      - one time timer or cyclic timer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_ModifyTimer(
                                MLDSNP_Timer_T  **timer_p,
                                mldsnp_timer_func_t    new_fun,
                                UI16_T     new_time,
                                UI32_T     type);
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
BOOL_T MLDSNP_TIMER_StartTimer(MLDSNP_Timer_T *timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_StopTimer
*------------------------------------------------------------------------------
* Purpose: This function stop and delete the timer to timer list
* INPUT  : *timer_p - the timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : if the timer needn't use, you shall call MLDSNP_TIMER_FreeTimer
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_StopTimer(MLDSNP_Timer_T *timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_StopAndFreeTimer
*------------------------------------------------------------------------------
* Purpose: This function stop and free the timers
* INPUT  : *timer_p - the timer pointer
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_TIMER_StopAndFreeTimer(MLDSNP_Timer_T **timer_p);

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
* NOTES  : L_TIMER_ONE_TIME, L_TIMER_CYCLIC
*          This parameter will be creata too.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_CreateAndStartTimer(mldsnp_timer_func_t callback,
                                        UI16_T        vid,
                                        const UI8_T         *gip_ap,
                                        const UI8_T         *sip_list_ap,
                                        UI16_T        num_of_src,
                                        UI16_T        lport,
                                        UI32_T        interval,
                                        UI16_T type,
                                        MLDSNP_Timer_T     **timer_p);
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
BOOL_T MLDSNP_TIMER_UpdateTimerNewTime(MLDSNP_Timer_T *timer_p,
                                       UI32_T    new_time);
/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_QueryTimer
*------------------------------------------------------------------------------
* Purpose: This function get the timer still have how many sec to timeout
* INPUT  : *timer_p - the timer
* OUTPUT : r_time - the sec last
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_TIMER_QueryTimer(MLDSNP_Timer_T *timer_p,
                               UI32_T    *r_time);

/*------------------------------------------------------------------------------
* Function : MLDSNP_TIMER_GetTimerHeadPtr
*------------------------------------------------------------------------------
* Purpose: This function get the timer list header pointer
* INPUT  : *timer_p - the timer
* OUTPUT : r_time - the sec last
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :only for backdoor use
*------------------------------------------------------------------------------*/
MLDSNP_TimerHead_T * MLDSNP_TIMER_GetTimerHeadPtr();
#endif


