/* MODULE NAME: pppoe_ia_backdoor.h
 * PURPOSE:
 *   Declarations of backdoor APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _PPPOE_IA_BACKDOOR_H
#define _PPPOE_IA_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define PPPOE_IA_DBG_ENUM(a,b)    a,
#define PPPOE_IA_DBG_NAME(a,b)    b,

#define PPPOE_IA_DBG_LST(_)                         \
    _(PPPOE_IA_DBG_TX_PKT_FLOW, "DBG TX FLOW")      \
    _(PPPOE_IA_DBG_RX_PKT_FLOW, "DBG RX FLOW")      \
    _(PPPOE_IA_DBG_PKT_CONTENT, "DBG PKT CONTENT")  \
    _(PPPOE_IA_DBG_UI,          "DBG UI")           \
    _(PPPOE_IA_DBG_OM,          "DBG OM")           \
    _(PPPOE_IA_DBG_ENG,         "DBG ENGINE")       \
    _(PPPOE_IA_DBG_ALL,         "DBG ALL")          \
    _(PPPOE_IA_DBG_RESET,       "DBG RESET")

#define PPPOE_IA_BDR_PRINT(args...)     printf(args);

#define PPPOE_IA_BDR_MSG(flag, fmt, ...)                \
        {                                               \
            if (PPPOE_IA_BACKDOOR_IsDebugOn(flag))      \
            {                                           \
                PPPOE_IA_BDR_PRINT("(%5d):%s, " fmt "\r\n",     \
                    __LINE__, __FUNCTION__, ##__VA_ARGS__ );    \
            }                                                   \
        }

/* DATA TYPE DECLARATIONS
 */
enum
{
    PPPOE_IA_DBG_LST(PPPOE_IA_DBG_ENUM)
    PPPOE_IA_DBG_MAX
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for PPPOE IA backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_BACKDOOR_Main(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_BACKDOOR_IsDebugOn
 * ------------------------------------------------------------------------
 * PURPOSE: To test if the input debug flag bitmap is on.
 * INPUT  : flag - content of debug flag bitmap
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
BOOL_T  PPPOE_IA_BACKDOOR_IsDebugOn(UI32_T flag);

#endif /* End of _PPPOE_IA_BACKDOOR_H */

