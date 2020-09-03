/* MODULE NAME: pfc_type.h
 * PURPOSE:
 *   Declarations of variables/structure used for PFC
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

#ifndef _PFC_TYPE_H
#define _PFC_TYPE_H

#define PFC_TYPE_BUILD_LINUX        TRUE

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "leaf_es3626a.h"
#include "leaf_sys.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PFC_TYPE_MAX_NBR_OF_UPORT       (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
#define PFC_TYPE_DFT_PORT_PRI_EN_BMP    (  (SYS_DFLT_PFC_PORT_PRI_0_ENABLE << 0) \
                                         | (SYS_DFLT_PFC_PORT_PRI_1_ENABLE << 1) \
                                         | (SYS_DFLT_PFC_PORT_PRI_2_ENABLE << 2) \
                                         | (SYS_DFLT_PFC_PORT_PRI_3_ENABLE << 3) \
                                         | (SYS_DFLT_PFC_PORT_PRI_4_ENABLE << 4) \
                                         | (SYS_DFLT_PFC_PORT_PRI_5_ENABLE << 5) \
                                         | (SYS_DFLT_PFC_PORT_PRI_6_ENABLE << 6) \
                                         | (SYS_DFLT_PFC_PORT_PRI_7_ENABLE << 7) )

//#define PFC_TYPE_DFT_LDA_BITS           32456                   // SYS_DFLT ???

/* for debug string */
#define PFC_TYPE_FLDE_STR_PREFIX        14
#define PFC_TYPE_RCE_STR_PREFIX         13

/* MACRO FUNCTION DECLARATIONS
 */
#define PFC_TYPE_CLR_BIT(a,b)       \
    do { (a) &= ~(0x1 << (b)); } while(0)

#define PFC_TYPE_SET_BIT(a,b)       \
    do { (a) |= (0x1 << (b));  } while(0)

#define PFC_TYPE_TST_BIT(a,b)       \
    ((a) & (0x1 << (b)))
#define PFC_TYPE_MAX(a,b)          ((a)>(b) ? (a) : (b))

#define PFC_TYPE_SINGAL_ENUM(a)    a,
#define PFC_TYPE_SINGAL_NAME(a)    PFC_TYPE_SINGAL_NAME_X(a)
#define PFC_TYPE_SINGAL_NAME_X(a)  #a,


/* for PFC_TYPE_FLDE_PORT_PRI_EN_ADM,
 *     the result of set operation is incremental update
 *     data format for set operation,
 *          BIT  0-7: BITMAP of priority (0-7)
 */
#define PFC_TYPE_FLDE_LST(_)               \
    _(PFC_TYPE_FLDE_GLOBAL_IS_COS_OK    /* UI32 R,         */) \
    _(PFC_TYPE_FLDE_GLOBAL_MAX_TC_CAP   /* UI32 R          */) \
    _(PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN /* UI32 R          */) \
    _(PFC_TYPE_FLDE_PORT_NXT_PORT            /* UI32 R,         */) \
    _(PFC_TYPE_FLDE_PORT_ENABLE         /* UI32 R,         */) \
    _(PFC_TYPE_FLDE_PORT_MODE_ADM       /* UI32 RW,        */) \
    _(PFC_TYPE_FLDE_PORT_MODE_OPR       /* UI32 RW,        */) \
    _(PFC_TYPE_FLDE_PORT_PRI_EN_ADM     /* UI32 RW, BITMAP */) \
    _(PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD /* UI32 RW, BITMAP */) \
    _(PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM /* UI32 RW, BITMAP */) \
    _(PFC_TYPE_FLDE_PORT_PRI_EN_OPR     /* UI32 RW, BITMAP */)

#if 0
    _(PFC_TYPE_FLDE_PORT_LDA            /* UI32 RW,        */)
#endif

#define PFC_TYPE_RCE_LST(_)            \
    _(PFC_TYPE_RCE_OK                ) \
    _(PFC_TYPE_RCE_FIELD_OUT_OF_RANGE) \
    _(PFC_TYPE_RCE_VALUE_OUT_OF_RANGE) \
    _(PFC_TYPE_RCE_XOR_CHK_FAILED    ) \
    _(PFC_TYPE_RCE_NOT_IN_MASTER_MODE) \
    _(PFC_TYPE_RCE_TRUNK_NO_MBR      ) \
    _(PFC_TYPE_RCE_OPR_DISB_BY_COS   ) \
    _(PFC_TYPE_RCE_GET_ONLY          ) \
    _(PFC_TYPE_RCE_NOT_ADMIN_AUTO    ) \
    _(PFC_TYPE_RCE_UNKNOWN           )


/* TYPE DECLARATIONS
 */
enum PFC_TYPE_FieldId_E
{
    PFC_TYPE_FLDE_LST(PFC_TYPE_SINGAL_ENUM)
    PFC_TYPE_FLDE_MAX,          /* maximum no, for boundary checking  */
};

enum PFC_TYPE_ReturnCode_E
{
    PFC_TYPE_RCE_LST(PFC_TYPE_SINGAL_ENUM)
    PFC_TYPE_RCE_MAX,           /* maximum no, for boundary checking  */
};

enum PFC_TYPE_PortMode_E
{
    PFC_TYPE_PMODE_OFF,         /* ADMIN/OPER */
    PFC_TYPE_PMODE_ON,          /* ADMIN/OPER */
    PFC_TYPE_PMODE_AUTO,        /* ADMIN ONLY */
    PFC_TYPE_PMODE_MAX,         /* maximum no, for boundary checking  */
};

typedef struct
{
    UI8_T   mode_adm;
    UI8_T   en_pri_bmp_adm;
    UI8_T   mode_opr;
    UI8_T   en_pri_bmp_opr;
} PFC_TYPE_PortCtrlRec_T;

#endif /* End of _PFC_TYPE_H */

