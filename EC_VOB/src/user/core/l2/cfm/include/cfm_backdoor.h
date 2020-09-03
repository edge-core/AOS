/*-----------------------------------------------------------------------------
 * Module Name: cfm_backdoor.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the CFM backdoor
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    02/02/2007 - Macauley Cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */
#ifndef _CFM_BACKDOOR_H
#define _CFM_BACKDOOR_H

#include <stdio.h>
#include "backdoor_mgr.h"
#if (SYS_CPNT_CFM == TRUE)
#define CFM_BD_MSG_S(str, ...)  BACKDOOR_MGR_Printf(str, ##__VA_ARGS__); fflush(stdout);

#define CFM_BD_MSG(flag, ...)         \
{                                               \
    if (CFM_BACKDOOR_Debug(flag))               \
    {                                           \
        CFM_BD_MSG_S("\n\r(%5d):%s, ", __LINE__, __FUNCTION__);    \
        CFM_BD_MSG_S(__VA_ARGS__);    \
    }                                           \
}

enum CFM_BACKDOOR_DEBUG_FAG_E
{
    CFM_BACKDOOR_DEBUG_FLAG_NONE           =  BIT_0,
    CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW =  BIT_1,
    CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW =  BIT_2,
    CFM_BACKDOOR_DEBUG_FLAG_TIMER          =  BIT_3,
    CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE  =  BIT_4,
    CFM_BACKDOOR_DEBUG_FLAG_TRAP           =  BIT_5,
    CFM_BACKDOOR_DEBUG_FLAG_UI             =  BIT_6,
    CFM_BACKDOOR_DEBUG_FLAG_DM             =  BIT_7,
    CFM_BACKDOOR_DEBUG_FLAG_TM             =  BIT_8,
    CFM_BACKDOOR_DEBUG_FLAG_LT             =  BIT_9,
    CFM_BACKDOOR_DEBUG_FLAG_CCM            =  BIT_10,
    CFM_BACKDOOR_DEBUG_FLAG_LB             =  BIT_11,
    CFM_BACKDOOR_DEBUG_FLAG_AIS            =  BIT_12,
    CFM_BACKDOOR_DEBUG_FLAG_OM             =  BIT_13,
    CFM_BACKDOOR_DEBUG_FLAG_ALL            =  0xFFFFFFFFL
};

/* ------------------------------------------------------------------------
 * ROUTINE NAME -CFM_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_BACKDOOR_Main() ;

void    CFM_BACKDOOR_SetDebugFlag(UI32_T flag);
BOOL_T  CFM_BACKDOOR_Debug(UI32_T flag);

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

#endif /* End of CFM_BACKDOOR_H */

