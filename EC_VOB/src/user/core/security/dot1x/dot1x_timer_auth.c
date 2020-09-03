/* INCLUDE FILE DECLARATIONS
 */
#include "dot1x_sm_auth.h"
#include "dot1x_timer_auth.h"
#include "1x_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#if 0
#define DOT1X_TIMER_AUTH_ARRAY_SIZE(array) (UI32_T)(sizeof(array)/sizeof(array[0]))
#define DOT1X_TIMER_AUTH_LPORT_TO_INDEX(l_port) (UI32_T)(l_port - 1)
#define DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, array) ((index < DOT1X_TIMER_AUTH_ARRAY_SIZE(array))? TRUE : FALSE)
#endif

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */
#if 0
DOT1X_TIMER_AUTH_Obj_T timeoutTimer[DOT1X_MAX_PORT];
DOT1X_TIMER_AUTH_Obj_T reauthTimer[DOT1X_MAX_PORT];
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_InitTimeoutTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Initialize timeout timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_InitTimeoutTimer(UI32_T lport)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, timeoutTimer))
    {
        return FALSE;
    }

    timeoutTimer[index].enabled = FALSE;
    timeoutTimer[index].seconds = 0;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_InitReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Initialize reauthentication timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_InitReauthTimer(UI32_T lport)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, reauthTimer))
    {
        return FALSE;
    }

    reauthTimer[index].enabled = FALSE;
    reauthTimer[index].seconds = 0;

    return TRUE;
}
#endif

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_StartTimeoutTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Start timeout timer
 * INPUT   : lport - port number
 *           seconds - time to wait to timeout
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_StartTimeoutTimer(UI32_T lport, UI32_T seconds)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, timeoutTimer))
    {
        return FALSE;
    }

    lib1x_message1(MESS_DBG_BSM, "Timeout timer START (seconds = %lu)", seconds);

    timeoutTimer[index].enabled = TRUE;
    timeoutTimer[index].seconds = seconds;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_StopTimeoutTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Stop timeout timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_StopTimeoutTimer(UI32_T lport)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, timeoutTimer))
    {
        return FALSE;
    }

    lib1x_message(MESS_DBG_BSM, "Timeout timer STOP");

    timeoutTimer[index].enabled = FALSE;
    timeoutTimer[index].seconds = 0;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_StartReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Start reauthentication timer
 * INPUT   : lport - port number
 *           seconds - time to wait to timeout
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_StartReauthTimer(UI32_T lport, UI32_T seconds)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, reauthTimer))
    {
        return FALSE;
    }

    lib1x_message1(MESS_DBG_BSM, "Reauth timer START (seconds = %lu)", seconds);

    reauthTimer[index].enabled = TRUE;
    reauthTimer[index].seconds = seconds;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_StopReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Stop reauthentication timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_StopReauthTimer(UI32_T lport)
{
    UI32_T index = DOT1X_TIMER_AUTH_LPORT_TO_INDEX(lport);

    if (FALSE == DOT1X_TIMER_AUTH_IS_VALID_ARRAY_INDEX(index, reauthTimer))
    {
        return FALSE;
    }

    lib1x_message(MESS_DBG_BSM, "Reauth timer STOP");

    reauthTimer[index].enabled = FALSE;
    reauthTimer[index].seconds = 0;

    return TRUE;
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_TIMER_AUTH_ProcessTimeoutEvent
 * ---------------------------------------------------------------------
 * PURPOSE : Process timeout event
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_TIMER_AUTH_ProcessTimeoutEvent(UI32_T lport)
{
    UI32_T i;

    for (i = 0; i < DOT1X_OM_MAX_SUPP_NBR_PER_PORT; ++i)
    {
        DOT1X_SM_AUTH_Obj_T *obj_p;
        BOOL_T is_reauth_enabled;

        obj_p = DOT1X_OM_GetSMObj(lport, i);

        if (TRUE == obj_p->gen_timer.is_started)
        {
            if (0 < obj_p->gen_timer.seconds)
            {
                --obj_p->gen_timer.seconds;
            }

            if (0 == obj_p->gen_timer.seconds)
            {
                obj_p->gen_timer.seconds = FALSE;
                DOT1X_SM_AUTH_Go(obj_p, DOT1X_SM_AUTH_TIMEOUT_EV);
            }
        }

        is_reauth_enabled = (VAL_dot1xAuthReAuthEnabled_true == DOT1X_OM_Get_PortReAuthEnabled(lport)) ? TRUE : FALSE;

        if (   (TRUE == is_reauth_enabled)
            && (TRUE == obj_p->reauth_timer.is_started))
        {
            if (0 < obj_p->reauth_timer.seconds)
            {
                --obj_p->reauth_timer.seconds;
            }

            if (0 == obj_p->reauth_timer.seconds)
            {
                obj_p->reauth_timer.is_started = FALSE;
                DOT1X_SM_AUTH_Go(obj_p, DOT1X_SM_AUTH_REAUTH_EV);
            }
        }
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
