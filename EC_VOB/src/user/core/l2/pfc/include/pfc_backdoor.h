/* MODULE NAME: pfc_backdoor.h
 * PURPOSE:
 *   Declarations of backdoor APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _PFC_BACKDOOR_H
#define _PFC_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "pfc_type.h"
#include "pfc_om.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define PFC_DBG_ENUM(a,b)    a,
#define PFC_DBG_NAME(a,b)    b,

#define PFC_DBG_LST(_)                             \
    _(PFC_DBG_RESET,       "DBG RESET")            \
    _(PFC_DBG_PKT_CONTENT, "DBG PKT CONTENT")      \
    _(PFC_DBG_UI,          "DBG UI")               \
    _(PFC_DBG_OM,          "DBG OM")               \
    _(PFC_DBG_MGR,         "DBG MGR")              \
    _(PFC_DBG_ALL,         "DBG ALL")

#define PFC_BDR_PRINT(args...)     BACKDOOR_MGR_Printf(args);

#if (PFC_TYPE_BUILD_LINUX == TRUE)
#define PFC_BDR_MSG(flag_enum, fmt, ...)                       \
        {                                                       \
            if (PFC_OM_IsDbgFlagOn(flag_enum))                 \
            {                                                   \
                PFC_BDR_PRINT("(%5d):%s, " fmt "\r\n",         \
                    __LINE__, __FUNCTION__, ##__VA_ARGS__ );    \
            }                                                   \
        }
#else
    #define PFC_BDR_MSG(flag_enum, fmt, arg...)                \
        {                                                       \
            if (PFC_OM_IsDbgFlagOn(flag_enum))                 \
            {                                                   \
                PFC_BDR_PRINT("(%5d):%s, " fmt "\r\n",         \
                    __LINE__, __FUNCTION__ ,##arg);             \
            }                                                   \
        }
#endif

/* DATA TYPE DECLARATIONS
 */
enum
{
    PFC_DBG_LST(PFC_DBG_ENUM)
    PFC_DBG_MAX
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for PFC backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PFC_BACKDOOR_Main(void);

#endif /* End of _PFC_BACKDOOR_H */

