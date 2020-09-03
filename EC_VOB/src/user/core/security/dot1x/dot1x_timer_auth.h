#ifndef DOT1X_TIMER_AUTH_H
#define DOT1X_TIMER_AUTH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DOT1X_TIMER_AUTH_INFINITE_TIME (-1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    BOOL_T enabled;
    I32_T seconds;
} DOT1X_TIMER_AUTH_Obj_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T DOT1X_TIMER_AUTH_InitTimeoutTimer(UI32_T lport);

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
BOOL_T DOT1X_TIMER_AUTH_InitReauthTimer(UI32_T lport);
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
BOOL_T DOT1X_TIMER_AUTH_StartTimeoutTimer(UI32_T lport, UI32_T seconds);

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
BOOL_T DOT1X_TIMER_AUTH_StopTimeoutTimer(UI32_T lport);

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
BOOL_T DOT1X_TIMER_AUTH_StartReauthTimer(UI32_T lport, UI32_T seconds);

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
BOOL_T DOT1X_TIMER_AUTH_StopReauthTimer(UI32_T lport);
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
BOOL_T DOT1X_TIMER_AUTH_ProcessTimeoutEvent(UI32_T lport);

#endif   /* End of DOT1X_TIMER_AUTH_H */
