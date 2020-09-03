/* MODULE NAME: ets_backdoor.h
 * PURPOSE:
 *   Declarations of backdoor APIs for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _ETS_BACKDOOR_H
#define _ETS_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "ets_type.h"
#include "ets_om.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define ETS_BDR_PRINT(args...)     BACKDOOR_MGR_Printf(args);

#if (ETS_TYPE_BUILD_LINUX == TRUE)
#define ETS_BDR_MSG(flag_enum, fmt, ...)                       \
        {                                                       \
            if (ETS_OM_IsDbgFlagOn(flag_enum))                 \
            {                                                   \
                ETS_BDR_PRINT("(%5d):%s, " fmt "\r\n",         \
                    __LINE__, __FUNCTION__, ##__VA_ARGS__ );    \
            }                                                   \
        }
#else
    #define ETS_BDR_MSG(flag_enum, fmt, arg...)                \
        {                                                       \
            if (ETS_OM_IsDbgFlagOn(flag_enum))                 \
            {                                                   \
                ETS_BDR_PRINT("(%5d):%s, " fmt "\r\n",         \
                    __LINE__, __FUNCTION__ ,##arg);             \
            }                                                   \
        }
#endif

/* DATA TYPE DECLARATIONS
 */
 

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for ETS backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void ETS_BACKDOOR_Main(void);

#endif /* End of _ETS_BACKDOOR_H */


