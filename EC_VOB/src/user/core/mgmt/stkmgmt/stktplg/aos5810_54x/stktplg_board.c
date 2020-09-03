/* Module Name: STKTPLG_BOARD.C
 *
 * Purpose: This file will keep board information of different boards. If any
 *          mainboard has different board information, you must define corresponding
 *          board information in this file.  
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "stktplg_board.h"
#include "leaf_es3626a.h"  /* we should use more generic name */
#include "leaf_3636.h"
#include "stktplg_type.h"
#include "stktplg_board.h"    /* wuli, 2004-05-18 */
#include "phyaddr_access.h"
#ifdef INCLUDE_DIAG
#include "dev_swdrv.h"
#else
#include "dev_swdrv_pmgr.h"
#endif
#include "uc_mgr.h"
#include "backdoor_mgr.h"
#include "sysfun.h"
#include "l_charset.h"
#include "onlpdrv_psu.h"
#include "onlpdrv_sfp.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define STKTPLG_BOARD_MAX_BOARD_ID    ((sizeof(board_id_2_info_mapping)/sizeof(UI16_T)) - 1)
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
#define STKTPLG_BOARD_INIT_SIGNATURE    0x21436587
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define DEBUG_MSG(fmtstr, ...) do { \
    if(board_info_table_p->debug_mode == TRUE) \
        {printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

/* DATA TYPE DECLARATIONS
 */

typedef enum STKTPLG_BOARD_BoardType_E
{
    MAINBOARD_TYPE_0 = 0,   /* AS5812-54X */
    MAINBOARD_TYPE_1 = 1,   /* AS5812-54T */
    MAINBOARD_TYPE_2 = 2,   /* AS6812-32X */
    MAINBOARD_TYPE_MAX
} STKTPLG_BOARD_BoardType_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T STKTPLG_BOARD_GetPowerStatusInfo_Internal(UI32_T board_id, UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p);
static UI8_T STKTPLG_BOARD_GetThermalNumberFromOnlp(void);
static UI8_T STKTPLG_BOARD_GetFanNumberFromOnlp(void);

/* STATIC VARIABLE DECLARATIONS 
 */
 
/* board id to board information mapping, for every project,
 * we will have different mapping table.
 */
  
static UI16_T board_id_2_info_mapping[] =
{
    MAINBOARD_TYPE_0,
    MAINBOARD_TYPE_1,
    MAINBOARD_TYPE_2,
};


/* MAINBAORD_TYPE_0: mapping information
 */
/* Definition for BoardType 0 */
#define type0_port2ModId \
{ \
    STKTPLG_MODULE_INDEX_0, \
    STKTPLG_MODULE_INDEX_1, \
}


#define type0_portMediaNum \
{ \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1 \
}


#define type0_portMediaType \
{ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 2 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 4 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 6 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 8 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 10 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 12 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 14 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 16 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 18 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 20 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 22 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 24 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 26 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 28 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 30 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 32 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 34 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 36 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 38 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 40 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 42 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 44 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 46 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 48 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 50 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 52 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 54 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
}

#define type0_portJackType \
{ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 2  */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 4  */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 6  */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 8  */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 10 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 12 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 14 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 16 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 18 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 20 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 22 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 24 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 26 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 28 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 30 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 32 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 34 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 36 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 38 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 40 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 42 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 44 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 46 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 48 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 50 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 52 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 54 */ \
    VAL_ifJackType_other,   VAL_ifJackType_other,   /* 56 */ \
}

#define type0_portFigureType \
{ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /*  2 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /*  4 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /*  6 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /*  8 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 10 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 12 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 14 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 16 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 18 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 20 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 22 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 24 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 26 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 28 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 30 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 32 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 34 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 36 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 38 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 40 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 42 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 44 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 46 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,       /* 48 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 50 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 52 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 54 */ \
}

#define type0_portMediaCap \
{ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 2  */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 4  */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 6  */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 8  */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 10 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 12 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 14 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 16 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 18 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 20 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 22 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 24 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 26 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 28 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 30 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 32 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 34 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 36 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 38 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 40 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 42 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 44 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 46 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 48 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 50 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 52 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 54 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER,  STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER,  /* 56 */ \
}

/* MAINBAORD_TYPE_1: mapping information
 */
/* Definition for BoardType 1 */

#define type1_port2ModId    type0_port2ModId

#define type1_portMediaNum \
{ \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1 \
}


#define type1_portMediaType \
{ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 2 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 4 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 6 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 8 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 10 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 12 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 14 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 16 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 18 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 20 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 22 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 24 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 26 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 28 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 30 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 32 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 34 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 36 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 38 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 40 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 42 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 44 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 46 */ \
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* 48 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 50 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 52 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 54 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER,       STKTPLG_TYPE_MEDIA_TYPE_OTHER,       /* 56 */ /* RFC4836/RFC3636 does not have media type for 40G */ \
}

#define type1_portJackType \
{ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /*  2 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /*  4 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /*  6 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /*  8 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 10 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 12 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 14 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 16 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 18 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 20 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 22 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 24 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 26 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 28 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 30 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 32 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 34 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 36 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 38 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 40 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 42 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 44 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 46 */ \
    VAL_ifJackType_rj45,    VAL_ifJackType_rj45,    /* 48 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 50 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 52 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC, /* 54 */ \
    VAL_ifJackType_other,   VAL_ifJackType_other,   /* 56 */ \
}

#define type1_portFigureType \
{ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /*  2 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /*  4 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /*  6 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /*  8 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 10 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 12 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 14 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 16 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 18 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 20 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 22 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 24 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 26 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 28 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 30 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 32 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 34 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 36 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 38 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 40 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 42 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 44 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 46 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45, STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,         /* 48 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 50 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 52 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,     /* 54 */ \
}

#define type1_portMediaCap \
{ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /*  2 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /*  4 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /*  6 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /*  8 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 10 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 12 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 14 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 16 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 18 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 20 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 22 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 24 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 26 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 28 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 30 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 32 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 34 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 36 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 38 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 40 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 42 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 44 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 46 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER, /* 48 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 50 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 52 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,  /* 54 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER,  STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER,  /* 56 */ \
}

/* MAINBAORD_TYPE_3: mapping information
 */
/* Definition for BoardType 3 */

#define type2_port2ModId    type0_port2ModId

#define type2_portMediaNum \
{ \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, \
}

#define type2_portMediaType \
{ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /*  2 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /*  4 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /*  6 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /*  8 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 10 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 12 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 14 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 16 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 18 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 20 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 22 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 24 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 26 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 28 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 30 */ \
    STKTPLG_TYPE_MEDIA_TYPE_OTHER, STKTPLG_TYPE_MEDIA_TYPE_OTHER, /* 32 */ \
}

#define type2_portJackType \
{ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /*  2 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /*  4 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /*  6 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /*  8 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 10 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 12 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 14 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 16 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 18 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 20 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 22 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 24 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 26 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 28 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 30 */ \
    VAL_ifJackType_fiberSC, VAL_ifJackType_fiberSC,       /* 32 */ \
}

#define type2_portFigureType \
{ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /*  2 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /*  4 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /*  6 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /*  8 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 10 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 12 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 14 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 16 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 18 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 20 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 22 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 24 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 26 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 28 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 30 */ \
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE, STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,       /* 32 */ \
}

#define type2_portMediaCap \
{ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /*  2 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /*  4 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /*  6 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /*  8 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 10 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 12 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 14 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 16 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 18 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 20 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 22 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 24 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 26 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 28 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 30 */ \
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER, /* 32 */ \
}

/* MAINBAORD_TYPE_n: mapping information
 * If more boards needed, please defined here
 */
 
/* Products-wide view of every boards
 */
typedef struct
{
    STKTPLG_BOARD_BoardInfo_T board_info_table[MAINBOARD_TYPE_MAX];
    BOOL_T                    show_debug_msg;
    BOOL_T                    debug_mode;
    UI8_T                     my_board_id;
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    UI8_T                     hw_port_mode[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI32_T                    init_signature;
#endif
}STKTPLG_BOARD_INFO_TABLE_STRUCT;

/* Pointer to shared memory address */
static STKTPLG_BOARD_INFO_TABLE_STRUCT   *board_info_table_p;
#define board_info_table (board_info_table_p->board_info_table)
#define FRU_CONF_TO_PROFILE_TABLE (board_info_table_p->fru_conf_to_fan_speed_mode_sm_profile_tbl)
/* Shared memory key  for stktplg board database */
#define STKTPLG_BOARD_SHARED_MEM_KEY       0x5368
static UI32_T shm_id_static;
static UI32_T sem_id_static;
static STKTPLG_BOARD_BoardInfo_T board_info_table_static[MAINBOARD_TYPE_MAX] =
{
    /* MAINBOARD_TYPE_0
     */
    {
        54,  /* main board have 54 port */
        0,   /* not used , max_hot_swap_port_number*/
        0,   /* max ports for expansion module */
        0,   /* start_expansion_port_number */
        54,  /* sfp_port_number (48 sfp+ ports + 6 qsfp ports ) */
        1,   /* start_sfp_port_number */
        6,   /* qsfp_port_number */
        49,  /* qsfp_port_start */
        55,  /* break_out_port_start */
        54,  /* max_port_number_of_this_unit */
        1,   /* chip_number_of_this_unit */
        type0_port2ModId,
        type0_portMediaNum,
        type0_portMediaType,
        type0_portJackType,
        type0_portFigureType,
        type0_portMediaCap,
        NULL,   /* not used */
        NULL,   /* not used */
        {
           {{0,0, 1, 1,VAL_portType_tenGBaseSFP},
            {0,0, 2, 2,VAL_portType_tenGBaseSFP},
            {0,0, 3, 3,VAL_portType_tenGBaseSFP},
            {0,0, 4, 4,VAL_portType_tenGBaseSFP},
            {0,0, 5, 5,VAL_portType_tenGBaseSFP}, /*  5 */
            {0,0, 6, 6,VAL_portType_tenGBaseSFP},
            {0,0, 7, 7,VAL_portType_tenGBaseSFP},
            {0,0, 8, 8,VAL_portType_tenGBaseSFP},
            {0,0, 9, 9,VAL_portType_tenGBaseSFP},
            {0,0,10,10,VAL_portType_tenGBaseSFP}, /* 10 */
            {0,0,11,11,VAL_portType_tenGBaseSFP},
            {0,0,12,12,VAL_portType_tenGBaseSFP},
            {0,0,13,13,VAL_portType_tenGBaseSFP},
            {0,0,14,14,VAL_portType_tenGBaseSFP},
            {0,0,15,15,VAL_portType_tenGBaseSFP}, /* 15 */
            {0,0,16,16,VAL_portType_tenGBaseSFP},
            {0,0,17,17,VAL_portType_tenGBaseSFP},
            {0,0,18,18,VAL_portType_tenGBaseSFP},
            {0,0,19,19,VAL_portType_tenGBaseSFP},
            {0,0,20,20,VAL_portType_tenGBaseSFP}, /* 20 */
            {0,0,21,21,VAL_portType_tenGBaseSFP},
            {0,0,22,22,VAL_portType_tenGBaseSFP},
            {0,0,23,23,VAL_portType_tenGBaseSFP},
            {0,0,24,24,VAL_portType_tenGBaseSFP},
            {0,0,25,25,VAL_portType_tenGBaseSFP}, /* 25 */
            {0,0,26,26,VAL_portType_tenGBaseSFP},
            {0,0,27,27,VAL_portType_tenGBaseSFP},
            {0,0,28,28,VAL_portType_tenGBaseSFP},
            {0,0,29,29,VAL_portType_tenGBaseSFP},
            {0,0,30,30,VAL_portType_tenGBaseSFP}, /* 30 */
            {0,0,31,31,VAL_portType_tenGBaseSFP},
            {0,0,32,32,VAL_portType_tenGBaseSFP},
            {0,0,33,33,VAL_portType_tenGBaseSFP},
            {0,0,34,34,VAL_portType_tenGBaseSFP},
            {0,0,35,35,VAL_portType_tenGBaseSFP}, /* 35 */
            {0,0,36,36,VAL_portType_tenGBaseSFP},
            {0,0,37,37,VAL_portType_tenGBaseSFP},
            {0,0,38,38,VAL_portType_tenGBaseSFP},
            {0,0,39,39,VAL_portType_tenGBaseSFP},
            {0,0,40,40,VAL_portType_tenGBaseSFP}, /* 40 */
            {0,0,41,41,VAL_portType_tenGBaseSFP},
            {0,0,42,42,VAL_portType_tenGBaseSFP},
            {0,0,43,43,VAL_portType_tenGBaseSFP},
            {0,0,44,44,VAL_portType_tenGBaseSFP},
            {0,0,45,45,VAL_portType_tenGBaseSFP}, /* 45 */
            {0,0,46,46,VAL_portType_tenGBaseSFP},
            {0,0,47,47,VAL_portType_tenGBaseSFP},
            {0,0,48,48,VAL_portType_tenGBaseSFP},
            {0,0,49,49,VAL_portType_fortyGBaseQSFP},
            {0,0,53,53,VAL_portType_fortyGBaseQSFP}, /* 50 */
            {0,0,57,57,VAL_portType_fortyGBaseQSFP},
            {0,0,61,61,VAL_portType_fortyGBaseQSFP},
            {0,0,65,65,VAL_portType_fortyGBaseQSFP},
            {0,0,69,69,VAL_portType_fortyGBaseQSFP},
            {255,255, 255, 255,VAL_portType_other},/* 55 */
            {255,255, 255, 255,VAL_portType_other}
           } 
        },
        FALSE, 0, 0,
        TRUE /* is_support_rpu */,
        SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT,
        SYS_HWCFG_NBR_OF_THERMAL_BID_0
    },
    /* MAINBOARD_TYPE_1
     */
    {
        54,  /* main board have 54 port */
        0,   /* not used , max_hot_swap_port_number*/
        0,   /* max ports for expansion module */
        0,   /* start_expansion_port_number */
        6,   /* sfp_port_number (6 qsfp ports ) */
        49,  /* start_sfp_port_number */
        6,   /* qsfp_port_number */
        49,  /* qsfp_port_start */
        55,  /* break_out_port_start */
        54,  /* max_port_number_of_this_unit */
        1,   /* chip_number_of_this_unit */
        type1_port2ModId,
        type1_portMediaNum,
        type1_portMediaType,
        type1_portJackType,
        type1_portFigureType,
        type1_portMediaCap,
        NULL,   /* not used */
        NULL,   /* not used */
        {
           {{0,0, 1, 1,VAL_portType_tenGBaseT},
            {0,0, 2, 2,VAL_portType_tenGBaseT},
            {0,0, 3, 3,VAL_portType_tenGBaseT},
            {0,0, 4, 4,VAL_portType_tenGBaseT},
            {0,0, 5, 5,VAL_portType_tenGBaseT}, /*  5 */
            {0,0, 6, 6,VAL_portType_tenGBaseT},
            {0,0, 7, 7,VAL_portType_tenGBaseT},
            {0,0, 8, 8,VAL_portType_tenGBaseT},
            {0,0, 9, 9,VAL_portType_tenGBaseT},
            {0,0,10,10,VAL_portType_tenGBaseT}, /* 10 */
            {0,0,11,11,VAL_portType_tenGBaseT},
            {0,0,12,12,VAL_portType_tenGBaseT},
            {0,0,13,13,VAL_portType_tenGBaseT},
            {0,0,14,14,VAL_portType_tenGBaseT},
            {0,0,15,15,VAL_portType_tenGBaseT}, /* 15 */
            {0,0,16,16,VAL_portType_tenGBaseT},
            {0,0,17,17,VAL_portType_tenGBaseT},
            {0,0,18,18,VAL_portType_tenGBaseT},
            {0,0,19,19,VAL_portType_tenGBaseT},
            {0,0,20,20,VAL_portType_tenGBaseT}, /* 20 */
            {0,0,21,21,VAL_portType_tenGBaseT},
            {0,0,22,22,VAL_portType_tenGBaseT},
            {0,0,23,23,VAL_portType_tenGBaseT},
            {0,0,24,24,VAL_portType_tenGBaseT},
            {0,0,25,25,VAL_portType_tenGBaseT}, /* 25 */
            {0,0,26,26,VAL_portType_tenGBaseT},
            {0,0,27,27,VAL_portType_tenGBaseT},
            {0,0,28,28,VAL_portType_tenGBaseT},
            {0,0,29,29,VAL_portType_tenGBaseT},
            {0,0,30,30,VAL_portType_tenGBaseT}, /* 30 */
            {0,0,31,31,VAL_portType_tenGBaseT},
            {0,0,32,32,VAL_portType_tenGBaseT},
            {0,0,33,33,VAL_portType_tenGBaseT},
            {0,0,34,34,VAL_portType_tenGBaseT},
            {0,0,35,35,VAL_portType_tenGBaseT}, /* 35 */
            {0,0,36,36,VAL_portType_tenGBaseT},
            {0,0,37,37,VAL_portType_tenGBaseT},
            {0,0,38,38,VAL_portType_tenGBaseT},
            {0,0,39,39,VAL_portType_tenGBaseT},
            {0,0,40,40,VAL_portType_tenGBaseT}, /* 40 */
            {0,0,41,41,VAL_portType_tenGBaseT},
            {0,0,42,42,VAL_portType_tenGBaseT},
            {0,0,43,43,VAL_portType_tenGBaseT},
            {0,0,44,44,VAL_portType_tenGBaseT},
            {0,0,45,45,VAL_portType_tenGBaseT}, /* 45 */
            {0,0,46,46,VAL_portType_tenGBaseT},
            {0,0,47,47,VAL_portType_tenGBaseT},
            {0,0,48,48,VAL_portType_tenGBaseT},
            {0,0,49,49,VAL_portType_fortyGBaseQSFP},
            {0,0,53,53,VAL_portType_fortyGBaseQSFP}, /* 50 */
            {0,0,57,57,VAL_portType_fortyGBaseQSFP},
            {0,0,61,61,VAL_portType_fortyGBaseQSFP},
            {0,0,65,65,VAL_portType_fortyGBaseQSFP},
            {0,0,69,69,VAL_portType_fortyGBaseQSFP},
            {255,255, 255, 255,VAL_portType_other},/* 55 */
            {255,255, 255, 255,VAL_portType_other}
           } 
        },
        FALSE, 0, 0,
        TRUE /* is_support_rpu */,
        SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT,
        SYS_HWCFG_NBR_OF_THERMAL_BID_1
    },   
    /* MAINBOARD_TYPE_2
     */
    {
        32,  /* main board have 32 port */
        0,   /* not used , max_hot_swap_port_number*/
        0,   /* max ports for expansion module */
        0,   /* start_expansion_port_number */
        32,  /* sfp_port_number (32 qsfp ports ) */
        1,   /* start_sfp_port_number */
        32,  /* qsfp_port_number */
        1,   /* qsfp_port_start */
        33,  /* break_out_port_start */
        32,  /* max_port_number_of_this_unit */
        1,   /* chip_number_of_this_unit */
        type2_port2ModId,
        type2_portMediaNum,
        type2_portMediaType,
        type2_portJackType,
        type2_portFigureType,
        type2_portMediaCap,
        NULL,   /* not used */
        NULL,   /* not used */
        {
           {{0,0, 1,  1,VAL_portType_fortyGBaseQSFP},
            {0,0, 2,  2,VAL_portType_fortyGBaseQSFP},
            {0,0, 3,  3,VAL_portType_fortyGBaseQSFP},
            {0,0, 4,  4,VAL_portType_fortyGBaseQSFP},
            {0,0, 5,  5,VAL_portType_fortyGBaseQSFP}, /*  5 */
            {0,0, 6,  6,VAL_portType_fortyGBaseQSFP},
            {0,0, 7,  7,VAL_portType_fortyGBaseQSFP},
            {0,0, 8,  8,VAL_portType_fortyGBaseQSFP},
            {0,0, 9,  9,VAL_portType_fortyGBaseQSFP},
            {0,0,10, 10,VAL_portType_fortyGBaseQSFP}, /* 10 */
            {0,0,11, 11,VAL_portType_fortyGBaseQSFP},
            {0,0,12, 12,VAL_portType_fortyGBaseQSFP},
            {0,0,13, 13,VAL_portType_fortyGBaseQSFP},
            {0,0,14, 14,VAL_portType_fortyGBaseQSFP},
            {0,0,15, 15,VAL_portType_fortyGBaseQSFP}, /* 15 */
            {0,0,16, 16,VAL_portType_fortyGBaseQSFP},
            {0,0,17, 17,VAL_portType_fortyGBaseQSFP},
            {0,0,18, 18,VAL_portType_fortyGBaseQSFP},
            {0,0,19, 19,VAL_portType_fortyGBaseQSFP},
            {0,0,20, 20,VAL_portType_fortyGBaseQSFP}, /* 20 */
            {0,0,21, 21,VAL_portType_fortyGBaseQSFP},
            {0,0,22, 22,VAL_portType_fortyGBaseQSFP},
            {0,0,23, 23,VAL_portType_fortyGBaseQSFP},
            {0,0,24, 24,VAL_portType_fortyGBaseQSFP},
            {0,0,25, 25,VAL_portType_fortyGBaseQSFP}, /* 25 */
            {0,0,26, 26,VAL_portType_fortyGBaseQSFP},
            {0,0,27, 27,VAL_portType_fortyGBaseQSFP},
            {0,0,28, 28,VAL_portType_fortyGBaseQSFP},
            {0,0,29, 29,VAL_portType_fortyGBaseQSFP},
            {0,0,30, 30,VAL_portType_fortyGBaseQSFP}, /* 30 */
            {0,0,31, 31,VAL_portType_fortyGBaseQSFP},
            {0,0,32, 32,VAL_portType_fortyGBaseQSFP},
           }
        },
        FALSE, 0, 0,
        TRUE /* is_support_rpu */,
        SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT,
        SYS_HWCFG_NBR_OF_THERMAL_BID_2
    },
};


/* 05/03/2004 10:01¤W¤È vivid added to get port number of option module 
 */
typedef struct STKTPLG_BOARD_BoardPortInfo_S 
{
    UI32_T port_nbr;
}STKTPLG_BOARD_BoardPortInfo_T;

static STKTPLG_BOARD_BoardPortInfo_T module_port_info_table[MODULE_TYPE_MAX]=

{
    {8},
    {1}
};

/* wuli, 2004-05-18, add define for 10G potional module */
static STKTPLG_BOARD_ModuleInfo_T module_info_table[MODULE_TYPE_MAX] =
{
    
#if 0 /* SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT is 0, need not to define */
    { /* MODULE_TYPE_8SFX */
            { /* module_id, device_id, device_port_id, phy_id, port_type */
                {0,0,0,1,VAL_portType_thousandBaseSfp},  
                {0,0,2,3,VAL_portType_thousandBaseSfp},
                {0,0,4,5,VAL_portType_thousandBaseSfp}, 
                {0,0,6,7,VAL_portType_thousandBaseSfp},
                {0,0,1,2,VAL_portType_thousandBaseSfp},
                {0,0,3,4,VAL_portType_thousandBaseSfp}, 
                {0,0,5,6,VAL_portType_thousandBaseSfp}, 
                {0,0,7,8,VAL_portType_thousandBaseSfp} 
         }                
    }, /* MODULE_TYPE_8SFX */   
    
    { /* MODULE_TYPE_ONE_PORT_10G */
        { /* module_type, device_id, device_port_id, phy_id, port_type */
            {0,0,0,1,VAL_portType_tenG},  
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other},
                {255,255,255,255,VAL_portType_other}
         }
                
    } /* MODULE_TYPE_ONE_PORT_10G */
#endif

};

/* PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0:
 *      Defines the on-board physical port number of board id 0.
 */
#define PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0 54
#define PHYSICAL_ON_BOARD_PORT_NUMBER_BID_1 54
#define PHYSICAL_ON_BOARD_PORT_NUMBER_BID_2 32

/* LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_0:
 *     Defines the on-board logical port number of board id 0.
 */
#define LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_0 (6*4)
#define LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_1 (6*4)
#define LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_2 (32*4)

#define FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_0  49
#define FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_1  49
#define FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_2   1

/* aos_uport_to_onlp_port_bid_0 defines the mapping between the AOS user port
 * id and ONLP sfp port id. Note that the mapping for AOS logical user port ids
 * for break-out ports does not define here.
 */
static I16_T aos_uport_to_onlp_port_bid_0[PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0] =
{
     0,  1,  2,  3,  4, /* AOS uport  1 -  5 */
     5,  6,  7,  8,  9, /* AOS uport  6 - 10 */
    10, 11, 12, 13, 14, /* AOS uport 11 - 15 */
    15, 16, 17, 18, 19, /* AOS uport 16 - 20 */
    20, 21, 22, 23, 24, /* AOS uport 21 - 25 */
    25, 26, 27, 28, 29, /* AOS uport 26 - 30 */
    30, 31, 32, 33, 34, /* AOS uport 31 - 35 */
    35, 36, 37, 38, 39, /* AOS uport 36 - 40 */
    40, 41, 42, 43, 44, /* AOS uport 41 - 45 */
    45, 46, 47, 48, 49, /* AOS uport 46 - 50 */
    50, 51, 52, 53        /* AOS uport 51 - 54 */
};

static I16_T aos_uport_to_onlp_port_bid_1[PHYSICAL_ON_BOARD_PORT_NUMBER_BID_1] =
{
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport  1 -  5 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport  6 - 10 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 11 - 15 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 16 - 20 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 21 - 25 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 26 - 30 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 31 - 35 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 36 - 40 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, /* AOS uport 41 - 45 */
    ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, ONLP_SFP_INVALID_PORT_ID, 48, 49, /* AOS uport 46 - 50 */
    50, 51, 52, 53   /* AOS uport 51 - 55 */
};

static I16_T aos_uport_to_onlp_port_bid_2[PHYSICAL_ON_BOARD_PORT_NUMBER_BID_2] =
{
     0,  1,  2,  3,  4, /* AOS uport  1 -  5 */
     5,  6,  7,  8,  9, /* AOS uport  6 - 10 */
    10, 11, 12, 13, 14, /* AOS uport 11 - 15 */
    15, 16, 17, 18, 19, /* AOS uport 16 - 20 */
    20, 21, 22, 23, 24, /* AOS uport 21 - 25 */
    25, 26, 27, 28, 29, /* AOS uport 26 - 30 */
    30, 31                   /* AOS uport 31 - 32 */
};

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
typedef struct
{
    UI8_T max_port_number_on_board;
    UI8_T sfp_port_number;
    UI8_T max_port_number_of_this_unit;

    struct {
        UI8_T portMediaNum;
        UI8_T portMediaType;
        UI8_T portJackType;
        UI8_T portFigureType;
        UI8_T portMediaCap;
        UI8_T port_type;
    } port_info[STKTPLG_TYPE_HW_PORT_MODE_MAX];

    DEV_SWDRV_Device_Port_Mapping_T userPortMappingTable[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]; 
}
STKTPLG_BOARD_HwPortMode_BoardInfo_T;

typedef struct
{
    UI8_T uport_mapping_4x10G[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI8_T uport_mapping_1x40G[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI8_T drv_physical_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
}
STKTPLG_BOARD_HwPortMode_PortMapping_T;

typedef struct
{
    STKTPLG_BOARD_HwPortMode_BoardInfo_T board_info;
    STKTPLG_BOARD_HwPortMode_PortMapping_T port_mapping;
    UI8_T dflt_hw_port_mode[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
}
STKTPLG_BOARD_HwPortModeDb_T;

static DEV_SWDRV_Device_Port_Mapping_T stktplg_board_unused_port =
    {0xff,0xff,0xff,0xff,VAL_portType_other};

static STKTPLG_BOARD_HwPortModeDb_T stktplg_board_hw_port_mode_db[MAINBOARD_TYPE_MAX] =
{
    [MAINBOARD_TYPE_0] =
    {
        .board_info =
        {
            .max_port_number_on_board = 78,
            .sfp_port_number = 78,
            .max_port_number_of_this_unit = 78,
            .port_info =
            {
                [STKTPLG_TYPE_HW_PORT_MODE_4x10G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR,
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_tenGBaseSFP,
                },
                [STKTPLG_TYPE_HW_PORT_MODE_1x40G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* TBD: define new value for 40g? */
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_fortyGBaseQSFP,
                },
            },
            .userPortMappingTable =
            {
                [54] =
                /* 55 */ {0,0,49,49,VAL_portType_other},
                /* 56 */ {0,0,50,50,VAL_portType_other},
                /* 57 */ {0,0,51,51,VAL_portType_other},
                /* 58 */ {0,0,52,52,VAL_portType_other},
                /* 59 */ {0,0,53,53,VAL_portType_other},
                /* 60 */ {0,0,54,54,VAL_portType_other},
                /* 61 */ {0,0,55,55,VAL_portType_other},
                /* 62 */ {0,0,56,56,VAL_portType_other},
                /* 63 */ {0,0,57,57,VAL_portType_other},
                /* 64 */ {0,0,58,58,VAL_portType_other},
                /* 65 */ {0,0,59,59,VAL_portType_other},
                /* 66 */ {0,0,60,60,VAL_portType_other},
                /* 67 */ {0,0,61,61,VAL_portType_other},
                /* 68 */ {0,0,62,62,VAL_portType_other},
                /* 69 */ {0,0,63,63,VAL_portType_other},
                /* 70 */ {0,0,64,64,VAL_portType_other},
                /* 71 */ {0,0,65,65,VAL_portType_other},
                /* 72 */ {0,0,66,66,VAL_portType_other},
                /* 73 */ {0,0,67,67,VAL_portType_other},
                /* 74 */ {0,0,68,68,VAL_portType_other},
                /* 75 */ {0,0,69,69,VAL_portType_other},
                /* 76 */ {0,0,70,70,VAL_portType_other},
                /* 77 */ {0,0,71,71,VAL_portType_other},
                /* 78 */ {0,0,72,72,VAL_portType_other},
            },
        },
        .port_mapping =
        {
            .uport_mapping_4x10G =
            {
                /*  1 */  1,  2,  3,  4,  5,  6,  7,  8,
                /*  9 */  9, 10, 11, 12, 13, 14, 15, 16,
                /* 17 */ 17, 18, 19, 20, 21, 22, 23, 24,
                /* 25 */ 25, 26, 27, 28, 29, 30, 31, 32,
                /* 33 */ 33, 34, 35, 36, 37, 38, 39, 40,
                /* 41 */ 41, 42, 43, 44, 45, 46, 47, 48,
                /* 49 */ 55, 59, 63, 67, 71, 75, 55, 56,
                /* 57 */ 57, 58, 59, 60, 61, 62, 63, 64,
                /* 65 */ 65, 66, 67, 68, 69, 70, 71, 72,
                /* 73 */ 73, 74, 75, 76, 77, 78,
            },
            .uport_mapping_1x40G =
            {
                /*  1 */  1,  1,  1,  1,  5,  5,  5,  5,
                /*  9 */  9,  9,  9,  9, 13, 13, 13, 13,
                /* 17 */ 17, 17, 17, 17, 21, 21, 21, 21,
                /* 25 */ 25, 25, 25, 25, 29, 29, 29, 29,
                /* 33 */ 33, 33, 33, 33, 37, 37, 37, 37,
                /* 41 */ 41, 41, 41, 41, 45, 45, 45, 45,
                /* 49 */ 49, 50, 51, 52, 53, 54, 49, 49,
                /* 57 */ 49, 49, 50, 50, 50, 50, 51, 51,
                /* 65 */ 51, 51, 52, 52, 52, 52, 53, 53,
                /* 73 */ 53, 53, 54, 54, 54, 54,
            },
            .drv_physical_port =
            {
                /*  1 */  13,  14,  15,  16,  21,  22,  23,  24,
                /*  9 */  25,  26,  27,  28,  29,  30,  31,  32,
                /* 17 */  45,  46,  47,  48,  49,  50,  51,  52,
                /* 25 */  53,  54,  55,  56,  57,  58,  59,  60,
                /* 33 */  61,  62,  63,  64,  65,  66,  67,  68,
                /* 41 */  69,  70,  71,  72,  73,  74,  75,  76,
                /* 49 */  97, 101,  81, 105, 109,  77,  97,  98,
                /* 57 */  99, 100, 101, 102, 103, 104,  81,  82,
                /* 65 */  83,  84, 105, 106, 107, 108, 109, 110,
                /* 73 */ 111, 112,  77,  78,  79,  80,
            },
        },
        .dflt_hw_port_mode =
        {
            /*  1 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  3 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  5 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  7 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  9 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 11 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 13 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 15 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 17 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 19 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 21 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 23 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 25 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 27 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 29 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 31 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 33 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 35 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 37 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 39 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 41 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 43 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 45 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 47 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 49 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 51 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 53 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 55 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 57 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 59 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 61 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 63 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 65 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 67 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 69 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 71 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 73 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 75 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 77 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
        },
    }, /* MAINBOARD_TYPE_0 */
    [MAINBOARD_TYPE_1] =
    {
        .board_info =
        {
            .max_port_number_on_board = 78,
            .sfp_port_number = 30,
            .max_port_number_of_this_unit = 78,
            .port_info =
            {
                [STKTPLG_TYPE_HW_PORT_MODE_4x10G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR,
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_tenGBaseSFP,
                },
                [STKTPLG_TYPE_HW_PORT_MODE_1x40G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* TBD: define new value for 40g? */
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_fortyGBaseQSFP,
                },
            },
            .userPortMappingTable =
            {
                [54] =
                /* 55 */ {0,0,49,49,VAL_portType_other},
                /* 56 */ {0,0,50,50,VAL_portType_other},
                /* 57 */ {0,0,51,51,VAL_portType_other},
                /* 58 */ {0,0,52,52,VAL_portType_other},
                /* 59 */ {0,0,53,53,VAL_portType_other},
                /* 60 */ {0,0,54,54,VAL_portType_other},
                /* 61 */ {0,0,55,55,VAL_portType_other},
                /* 62 */ {0,0,56,56,VAL_portType_other},
                /* 63 */ {0,0,57,57,VAL_portType_other},
                /* 64 */ {0,0,58,58,VAL_portType_other},
                /* 65 */ {0,0,59,59,VAL_portType_other},
                /* 66 */ {0,0,60,60,VAL_portType_other},
                /* 67 */ {0,0,61,61,VAL_portType_other},
                /* 68 */ {0,0,62,62,VAL_portType_other},
                /* 69 */ {0,0,63,63,VAL_portType_other},
                /* 70 */ {0,0,64,64,VAL_portType_other},
                /* 71 */ {0,0,65,65,VAL_portType_other},
                /* 72 */ {0,0,66,66,VAL_portType_other},
                /* 73 */ {0,0,67,67,VAL_portType_other},
                /* 74 */ {0,0,68,68,VAL_portType_other},
                /* 75 */ {0,0,69,69,VAL_portType_other},
                /* 76 */ {0,0,70,70,VAL_portType_other},
                /* 77 */ {0,0,71,71,VAL_portType_other},
                /* 78 */ {0,0,72,72,VAL_portType_other},
            },
        },
        .port_mapping =
        {
            .uport_mapping_4x10G =
            {
                /*  1 */  1,  2,  3,  4,  5,  6,  7,  8,
                /*  9 */  9, 10, 11, 12, 13, 14, 15, 16,
                /* 17 */ 17, 18, 19, 20, 21, 22, 23, 24,
                /* 25 */ 25, 26, 27, 28, 29, 30, 31, 32,
                /* 33 */ 33, 34, 35, 36, 37, 38, 39, 40,
                /* 41 */ 41, 42, 43, 44, 45, 46, 47, 48,
                /* 49 */ 55, 59, 63, 67, 71, 75, 55, 56,
                /* 57 */ 57, 58, 59, 60, 61, 62, 63, 64,
                /* 65 */ 65, 66, 67, 68, 69, 70, 71, 72,
                /* 73 */ 73, 74, 75, 76, 77, 78,
            },
            .uport_mapping_1x40G =
            {
                /*  1 */  0,  0,  0,  0,  0,  0,  0,  0,
                /*  9 */  0,  0,  0,  0,  0,  0,  0,  0,
                /* 17 */  0,  0,  0,  0,  0,  0,  0,  0,
                /* 25 */  0,  0,  0,  0,  0,  0,  0,  0,
                /* 33 */  0,  0,  0,  0,  0,  0,  0,  0,
                /* 41 */  0,  0,  0,  0,  0,  0,  0,  0,
                /* 49 */ 49, 50, 51, 52, 53, 54, 49, 49,
                /* 57 */ 49, 49, 50, 50, 50, 50, 51, 51,
                /* 65 */ 51, 51, 52, 52, 52, 52, 53, 53,
                /* 73 */ 53, 53, 54, 54, 54, 54,
            },
            .drv_physical_port =
            {
                /*  1 */  13,  14,  15,  16,  21,  22,  23,  24,
                /*  9 */  25,  26,  27,  28,  29,  30,  31,  32,
                /* 17 */  45,  46,  47,  48,  49,  50,  51,  52,
                /* 25 */  53,  54,  55,  56,  57,  58,  59,  60,
                /* 33 */  61,  62,  63,  64,  65,  66,  67,  68,
                /* 41 */  69,  70,  71,  72,  73,  74,  75,  76,
                /* 49 */  97, 101,  77, 105, 109,  81,  97,  98,
                /* 57 */  99, 100, 101, 102, 103, 104,  77,  78,
                /* 65 */  79,  80, 105, 106, 107, 108, 109, 110,
                /* 73 */ 111, 112,  81,  82,  83,  84,
            },
        },
        .dflt_hw_port_mode =
        {
            /*  1 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  3 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  5 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  7 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /*  9 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 11 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 13 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 15 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 17 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 19 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 21 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 23 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 25 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 27 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 29 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 31 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 33 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 35 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 37 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 39 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 41 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 43 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 45 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 47 */ STKTPLG_TYPE_HW_PORT_MODE_4x10G,  STKTPLG_TYPE_HW_PORT_MODE_4x10G,
            /* 49 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 51 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 53 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 55 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 57 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 59 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 61 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 63 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 65 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 67 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 69 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 71 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 73 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 75 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 77 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
        },
    }, /* MAINBOARD_TYPE_1 */
    [MAINBOARD_TYPE_2] =
    {
        .board_info =
        {
            .max_port_number_on_board = 160,
            .sfp_port_number = 160,
            .max_port_number_of_this_unit = 160,
            .port_info =
            {
                [STKTPLG_TYPE_HW_PORT_MODE_4x10G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR,
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_tenGBaseSFP,
                },
                [STKTPLG_TYPE_HW_PORT_MODE_1x40G] =
                {
                    .portMediaNum = 1,
                    .portMediaType = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR, /* TBD: define new value for 40g? */
                    .portJackType = VAL_ifJackType_fiberSC,
                    .portFigureType = STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,
                    .portMediaCap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER,
                    .port_type = VAL_portType_fortyGBaseQSFP,
                },
            },
            .userPortMappingTable =
            {
                [32] = 
                /*  33 */ {0,0,  1,  1,VAL_portType_other},
                /*  34 */ {0,0,0xff,  1,VAL_portType_other},
                /*  35 */ {0,0,0xff,  1,VAL_portType_other},
                /*  36 */ {0,0,0xff,  1,VAL_portType_other},
                /*  37 */ {0,0,  2,  2,VAL_portType_other},
                /*  38 */ {0,0,0xff,  2,VAL_portType_other},
                /*  39 */ {0,0,0xff,  2,VAL_portType_other},
                /*  40 */ {0,0,0xff,  2,VAL_portType_other},
                /*  41 */ {0,0,  3,  3,VAL_portType_other},
                /*  42 */ {0,0,0xff,  3,VAL_portType_other},
                /*  43 */ {0,0,0xff,  3,VAL_portType_other},
                /*  44 */ {0,0,0xff,  3,VAL_portType_other},
                /*  45 */ {0,0,  4,  4,VAL_portType_other},
                /*  46 */ {0,0,0xff,  4,VAL_portType_other},
                /*  47 */ {0,0,0xff,  4,VAL_portType_other},
                /*  48 */ {0,0,0xff,  4,VAL_portType_other},
                /*  49 */ {0,0,  5,  5,VAL_portType_other},
                /*  50 */ {0,0,0xff,  5,VAL_portType_other},
                /*  51 */ {0,0,0xff,  5,VAL_portType_other},
                /*  52 */ {0,0,0xff,  5,VAL_portType_other},
                /*  53 */ {0,0,  6,  6,VAL_portType_other},
                /*  54 */ {0,0,0xff,  6,VAL_portType_other},
                /*  55 */ {0,0,0xff,  6,VAL_portType_other},
                /*  56 */ {0,0,0xff,  6,VAL_portType_other},
                /*  57 */ {0,0,  7,  7,VAL_portType_other},
                /*  58 */ {0,0,0xff,  7,VAL_portType_other},
                /*  59 */ {0,0,0xff,  7,VAL_portType_other},
                /*  60 */ {0,0,0xff,  7,VAL_portType_other},
                /*  61 */ {0,0,  8,  8,VAL_portType_other},
                /*  62 */ {0,0,0xff,  8,VAL_portType_other},
                /*  63 */ {0,0,0xff,  8,VAL_portType_other},
                /*  64 */ {0,0,0xff,  8,VAL_portType_other},
                /*  65 */ {0,0,  9,  9,VAL_portType_other},
                /*  66 */ {0,0,0xff,  9,VAL_portType_other},
                /*  67 */ {0,0,0xff,  9,VAL_portType_other},
                /*  68 */ {0,0,0xff,  9,VAL_portType_other},
                /*  69 */ {0,0, 10, 10,VAL_portType_other},
                /*  70 */ {0,0,0xff, 10,VAL_portType_other},
                /*  71 */ {0,0,0xff, 10,VAL_portType_other},
                /*  72 */ {0,0,0xff, 10,VAL_portType_other},
                /*  73 */ {0,0, 11, 11,VAL_portType_other},
                /*  74 */ {0,0,0xff, 11,VAL_portType_other},
                /*  75 */ {0,0,0xff, 11,VAL_portType_other},
                /*  76 */ {0,0,0xff, 11,VAL_portType_other},
                /*  77 */ {0,0, 12, 12,VAL_portType_other},
                /*  78 */ {0,0,0xff, 12,VAL_portType_other},
                /*  79 */ {0,0,0xff, 12,VAL_portType_other},
                /*  80 */ {0,0,0xff, 12,VAL_portType_other},
                /*  81 */ {0,0, 13, 13,VAL_portType_other},
                /*  82 */ {0,0,0xff, 13,VAL_portType_other},
                /*  83 */ {0,0,0xff, 13,VAL_portType_other},
                /*  84 */ {0,0,0xff, 13,VAL_portType_other},
                /*  85 */ {0,0, 14, 14,VAL_portType_other},
                /*  86 */ {0,0,0xff, 14,VAL_portType_other},
                /*  87 */ {0,0,0xff, 14,VAL_portType_other},
                /*  88 */ {0,0,0xff, 14,VAL_portType_other},
                /*  89 */ {0,0, 15, 15,VAL_portType_other},
                /*  90 */ {0,0,0xff, 15,VAL_portType_other},
                /*  91 */ {0,0,0xff, 15,VAL_portType_other},
                /*  92 */ {0,0,0xff, 15,VAL_portType_other},
                /*  93 */ {0,0, 16, 16,VAL_portType_other},
                /*  94 */ {0,0,0xff, 16,VAL_portType_other},
                /*  95 */ {0,0,0xff, 16,VAL_portType_other},
                /*  96 */ {0,0,0xff, 16,VAL_portType_other},
                /*  97 */ {0,0, 17, 17,VAL_portType_other},
                /*  98 */ {0,0,0xff, 17,VAL_portType_other},
                /*  99 */ {0,0,0xff, 17,VAL_portType_other},
                /* 100 */ {0,0,0xff, 17,VAL_portType_other},
                /* 101 */ {0,0, 18, 18,VAL_portType_other},
                /* 102 */ {0,0,0xff, 18,VAL_portType_other},
                /* 103 */ {0,0,0xff, 18,VAL_portType_other},
                /* 104 */ {0,0,0xff, 18,VAL_portType_other},
                /* 105 */ {0,0, 19, 19,VAL_portType_other},
                /* 106 */ {0,0,0xff, 19,VAL_portType_other},
                /* 107 */ {0,0,0xff, 19,VAL_portType_other},
                /* 108 */ {0,0,0xff, 19,VAL_portType_other},
                /* 109 */ {0,0, 20, 20,VAL_portType_other},
                /* 110 */ {0,0,0xff, 20,VAL_portType_other},
                /* 111 */ {0,0,0xff, 20,VAL_portType_other},
                /* 112 */ {0,0,0xff, 20,VAL_portType_other},
                /* 113 */ {0,0, 21, 21,VAL_portType_other},
                /* 114 */ {0,0,0xff, 21,VAL_portType_other},
                /* 115 */ {0,0,0xff, 21,VAL_portType_other},
                /* 116 */ {0,0,0xff, 21,VAL_portType_other},
                /* 117 */ {0,0, 22, 22,VAL_portType_other},
                /* 118 */ {0,0,0xff, 22,VAL_portType_other},
                /* 119 */ {0,0,0xff, 22,VAL_portType_other},
                /* 120 */ {0,0,0xff, 22,VAL_portType_other},
                /* 121 */ {0,0, 23, 23,VAL_portType_other},
                /* 122 */ {0,0,0xff, 23,VAL_portType_other},
                /* 123 */ {0,0,0xff, 23,VAL_portType_other},
                /* 124 */ {0,0,0xff, 23,VAL_portType_other},
                /* 125 */ {0,0, 24, 24,VAL_portType_other},
                /* 126 */ {0,0,0xff, 24,VAL_portType_other},
                /* 127 */ {0,0,0xff, 24,VAL_portType_other},
                /* 128 */ {0,0,0xff, 24,VAL_portType_other},
                /* 129 */ {0,0, 25, 25,VAL_portType_other},
                /* 130 */ {0,0,0xff, 25,VAL_portType_other},
                /* 131 */ {0,0,0xff, 25,VAL_portType_other},
                /* 132 */ {0,0,0xff, 25,VAL_portType_other},
                /* 133 */ {0,0, 26, 26,VAL_portType_other},
                /* 134 */ {0,0,0xff, 26,VAL_portType_other},
                /* 135 */ {0,0,0xff, 26,VAL_portType_other},
                /* 136 */ {0,0,0xff, 26,VAL_portType_other},
                /* 137 */ {0,0, 27, 27,VAL_portType_other},
                /* 138 */ {0,0,0xff, 27,VAL_portType_other},
                /* 139 */ {0,0,0xff, 27,VAL_portType_other},
                /* 140 */ {0,0,0xff, 27,VAL_portType_other},
                /* 141 */ {0,0, 28, 28,VAL_portType_other},
                /* 142 */ {0,0,0xff, 28,VAL_portType_other},
                /* 143 */ {0,0,0xff, 28,VAL_portType_other},
                /* 144 */ {0,0,0xff, 28,VAL_portType_other},
                /* 145 */ {0,0, 29, 29,VAL_portType_other},
                /* 146 */ {0,0,0xff, 29,VAL_portType_other},
                /* 147 */ {0,0,0xff, 29,VAL_portType_other},
                /* 148 */ {0,0,0xff, 29,VAL_portType_other},
                /* 149 */ {0,0, 30, 30,VAL_portType_other},
                /* 150 */ {0,0,0xff, 30,VAL_portType_other},
                /* 151 */ {0,0,0xff, 30,VAL_portType_other},
                /* 152 */ {0,0,0xff, 30,VAL_portType_other},
                /* 153 */ {0,0, 31, 31,VAL_portType_other},
                /* 154 */ {0,0,0xff, 31,VAL_portType_other},
                /* 155 */ {0,0,0xff, 31,VAL_portType_other},
                /* 156 */ {0,0,0xff, 31,VAL_portType_other},
                /* 157 */ {0,0, 32, 32,VAL_portType_other},
                /* 158 */ {0,0,0xff, 32,VAL_portType_other},
                /* 159 */ {0,0,0xff, 32,VAL_portType_other},
                /* 160 */ {0,0,0xff, 32,VAL_portType_other},
            },
        },
        .port_mapping =
        {
            .uport_mapping_4x10G =
            {
                /*   1 */  33,  37,  41,  45,  49,  53,  57,  61,
                /*   9 */  65,  69,  73,  77,  81,  85,  89,  93,
                /*  17 */  97, 101, 105, 109, 113, 117, 121, 125,
                /*  25 */ 129, 133, 137, 141, 145, 149, 153, 157,
                /*  33 */  33,  34,  35,  36,  37,  38,  39,  40,
                /*  41 */  41,  42,  43,  44,  45,  46,  47,  48,
                /*  49 */  49,  50,  51,  52,  53,  54,  55,  56,
                /*  57 */  57,  58,  59,  60,  61,  62,  63,  64,
                /*  65 */  65,  66,  67,  68,  69,  70,  71,  72,
                /*  73 */  73,  74,  75,  76,  77,  78,  79,  80,
                /*  81 */  81,  82,  83,  84,  85,  86,  87,  88,
                /*  89 */  89,  90,  91,  92,  93,  94,  95,  96,
                /*  97 */  97,  98,  99, 100, 101, 102, 103, 104,
                /* 105 */ 105, 106, 107, 108, 109, 110, 111, 112,
                /* 113 */ 113, 114, 115, 116, 117, 118, 119, 120,
                /* 121 */ 121, 122, 123, 124, 125, 126, 127, 128,
                /* 129 */ 129, 130, 131, 132, 133, 134, 135, 136,
                /* 137 */ 137, 138, 139, 140, 141, 142, 143, 144,
                /* 145 */ 145, 146, 147, 148, 149, 150, 151, 152,
                /* 153 */ 153, 154, 155, 156, 157, 158, 159, 160,
            },
            .uport_mapping_1x40G =
            {
                /*   1 */  1,  2,  3,  4,  5,  6,  7,  8,
                /*   9 */  9, 10, 11, 12, 13, 14, 15, 16,
                /*  17 */ 17, 18, 19, 20, 21, 22, 23, 24,
                /*  25 */ 25, 26, 27, 28, 29, 30, 31, 32,
                /*  33 */  1,  1,  1,  1,  2,  2,  2,  2,
                /*  41 */  3,  3,  3,  3,  4,  4,  4,  4,
                /*  49 */  5,  5,  5,  5,  6,  6,  6,  6,
                /*  57 */  7,  7,  7,  7,  8,  8,  8,  8,
                /*  65 */  9,  9,  9,  9, 10, 10, 10, 10,
                /*  73 */ 11, 11, 11, 11, 12, 12, 12, 12,
                /*  81 */ 13, 13, 13, 13, 14, 14, 14, 14,
                /*  89 */ 15, 15, 15, 15, 16, 16, 16, 16,
                /*  97 */ 17, 17, 17, 17, 18, 18, 18, 18,
                /* 105 */ 19, 19, 19, 19, 20, 20, 20, 20,
                /* 113 */ 21, 21, 21, 21, 22, 22, 22, 22,
                /* 121 */ 23, 23, 23, 23, 24, 24, 24, 24,
                /* 129 */ 25, 25, 25, 25, 26, 26, 26, 26,
                /* 137 */ 27, 27, 27, 27, 28, 28, 28, 28,
                /* 145 */ 29, 29, 29, 29, 30, 30, 30, 30,
                /* 153 */ 31, 31, 31, 31, 32, 32, 32, 32,
            },
            .drv_physical_port =
            {
                /*   1 */   1,   5,   9,  13,  17,  21,  25,  29,
                /*   9 */  33,  37,  41,  45,  49,  51,  55,  59,
                /*  17 */  63,  67,  71,  75,  81,  85,  89,  93,
                /*  25 */  97, 101, 105, 109, 113, 117, 121, 125,
                /*  33 */   1,   2,   3,   4,   5,   6,   7,   8,
                /*  41 */   9,  10,  11,  12,  13,  14,  15,  16,
                /*  49 */  17,  18,  19,  20,  21,  22,  23,  24,
                /*  57 */  25,  26,  27,  28,  29,  30,  31,  32,
                /*  65 */  33,  34,  35,  36,  37,  38,  39,  40,
                /*  73 */  41,  42,  43,  44,  45,  46,  47,  48,
                /*  81 */  49,  50,  51,  52,  53,  54,  55,  56,
                /*  89 */  57,  58,  59,  60,  61,  62,  63,  64,
                /*  97 */  65,  66,  67,  68,  69,  70,  71,  72,
                /* 105 */  73,  74,  75,  76,  77,  78,  79,  80,
                /* 113 */  81,  82,  83,  84,  85,  86,  87,  88,
                /* 121 */  89,  90,  91,  92,  93,  94,  95,  96,
                /* 129 */  97,  98,  99, 100, 101, 102, 103, 104,
                /* 137 */ 105, 106, 107, 108, 109, 110, 111, 112,
                /* 145 */ 113, 114, 115, 116, 117, 118, 119, 120,
                /* 153 */ 121, 122, 123, 124, 125, 126, 127, 128,
            },
        },
        .dflt_hw_port_mode =
        {
            /*   1 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*   3 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*   5 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*   7 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*   9 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  11 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  13 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  15 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  17 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  19 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  21 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  23 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  25 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  27 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  29 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  31 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  33 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  35 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  37 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  39 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  41 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  43 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  45 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  47 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  49 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  51 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  53 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  55 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  57 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  59 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  61 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  63 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  65 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  67 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  69 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  71 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  73 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  75 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  77 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  79 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  81 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  83 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  85 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  87 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  89 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  91 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  93 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  95 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  97 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /*  99 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 101 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 103 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 105 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 107 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 109 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 111 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 113 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 115 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 117 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 119 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 121 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 123 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 125 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 127 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 129 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 131 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 133 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 135 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 137 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 139 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 141 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 143 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 145 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 147 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 149 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 151 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 153 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 155 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 157 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
            /* 159 */ STKTPLG_TYPE_HW_PORT_MODE_1x40G,  STKTPLG_TYPE_HW_PORT_MODE_1x40G,
        },
    }, /* MAINBOARD_TYPE_2 */
};
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

/* FUNCTION NAME : STKTPLG_BOARD_AttachProcessResources
 * PURPOSE: This function attach board resources
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_BOARD_AttachProcessResources(void)
{
    UC_MGR_Sys_Info_T uc_sys_info;
    int ret;

    ret = SYSFUN_CreateShMem( STKTPLG_BOARD_SHARED_MEM_KEY,
                              sizeof(STKTPLG_BOARD_INFO_TABLE_STRUCT),
                              &shm_id_static );
    if ( ret != SYSFUN_OK )
    {
        BACKDOOR_MGR_Printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }

    board_info_table_p = 
        (STKTPLG_BOARD_INFO_TABLE_STRUCT *)SYSFUN_AttachShMem(shm_id_static);
    if ( board_info_table_p == NULL )
    {
        BACKDOOR_MGR_Printf("%s(): STKTPLG_BOARD_InitiateProcessResources fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateRwSem(STKTPLG_BOARD_SHARED_MEM_KEY, &sem_id_static);
    if ( ret != SYSFUN_OK )
    {
        BACKDOOR_MGR_Printf("%s(): SYSFUN_GetRwSem fails.\n", __FUNCTION__);
        return FALSE;
    }
    if(UC_MGR_InitiateProcessResources() == FALSE)
    {  
        BACKDOOR_MGR_Printf("%s(): UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
        return FALSE;
    }
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {   
        BACKDOOR_MGR_Printf("%s(): Get UC System Information Fail.\n", __FUNCTION__);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    return TRUE;
}
/* FUNCTION NAME : STKTPLG_BOARD_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG board which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_BOARD_InitiateProcessResources(void)
{
    int loop;
    int ret;
    UC_MGR_Sys_Info_T uc_sys_info;

    ret = SYSFUN_CreateShMem( STKTPLG_BOARD_SHARED_MEM_KEY,
                              sizeof(STKTPLG_BOARD_INFO_TABLE_STRUCT),
                              &shm_id_static );
    if ( ret != SYSFUN_OK )
    {
        BACKDOOR_MGR_Printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }

    board_info_table_p = 
        (STKTPLG_BOARD_INFO_TABLE_STRUCT *)SYSFUN_AttachShMem(shm_id_static);
    if ( board_info_table_p == NULL )
    {
        BACKDOOR_MGR_Printf("%s(): STKTPLG_BOARD_InitiateProcessResources fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateRwSem(STKTPLG_BOARD_SHARED_MEM_KEY, &sem_id_static);
    if ( ret != SYSFUN_OK )
    {
        BACKDOOR_MGR_Printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    if (board_info_table_p->init_signature == STKTPLG_BOARD_INIT_SIGNATURE)
    {
        return TRUE;
    }
#endif

    memset(board_info_table_p, 0, sizeof(*board_info_table_p));
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    board_info_table_p->init_signature = STKTPLG_BOARD_INIT_SIGNATURE;
#endif

    for ( loop = 0; loop < MAINBOARD_TYPE_MAX; loop ++ )
        board_info_table[loop] = board_info_table_static[loop];

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        BACKDOOR_MGR_Printf("Get UC System Information Fail.\r\n");

        return FALSE;
    }

    if (uc_sys_info.board_id > STKTPLG_BOARD_MAX_BOARD_ID)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d, max board id=%d, force board id to be 0.\r\n",
            __FUNCTION__, __LINE__, (int)(uc_sys_info.board_id), STKTPLG_BOARD_MAX_BOARD_ID);
        board_info_table_p->my_board_id = 0;
    }
    else
        board_info_table_p->my_board_id = uc_sys_info.board_id;

    board_info_table_p->show_debug_msg = FALSE;

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
{
    UI16_T board_info_index;
    UI8_T port_idx, old_port_num, max_port_num;

    board_info_index = board_id_2_info_mapping[board_info_table_p->my_board_id];

    old_port_num = board_info_table[board_info_index].max_port_number_on_board;

    board_info_table[board_info_index].max_port_number_on_board =
        stktplg_board_hw_port_mode_db[board_info_index].board_info.max_port_number_on_board;
    board_info_table[board_info_index].sfp_port_number =
        stktplg_board_hw_port_mode_db[board_info_index].board_info.sfp_port_number;
    board_info_table[board_info_index].max_port_number_of_this_unit =
        stktplg_board_hw_port_mode_db[board_info_index].board_info.max_port_number_of_this_unit;

    max_port_num = SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;

    for (port_idx = old_port_num; port_idx < max_port_num; port_idx++)
    {
        memcpy(
            &board_info_table[board_info_index].userPortMappingTable[0][port_idx],
            &stktplg_board_unused_port,
            sizeof(*board_info_table[board_info_index].userPortMappingTable[0]));
    }

    memcpy(
        board_info_table_p->hw_port_mode,
        stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode,
        sizeof(board_info_table_p->hw_port_mode));
}
#endif

    board_info_table[board_info_table_p->my_board_id].thermal_number=STKTPLG_BOARD_GetThermalNumberFromOnlp();
    board_info_table[board_info_table_p->my_board_id].fan_number=STKTPLG_BOARD_GetFanNumberFromOnlp();

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_BOARD_SetDebugMode
 * PURPOSE : This function will set debug mode
 * INPUT   : debug_mode
 * OUTPUT  : None.
 * RETUEN  : None.
 * NOTES   : None.
 */
void STKTPLG_BOARD_SetDebugMode(BOOL_T debug_mode)
{
    board_info_table_p->debug_mode = debug_mode;
}

/* FUNCTION NAME : STKTPLG_BOARD_GetDebugMode
 * PURPOSE : This function will get debug mode
 * INPUT   : debug_mode
 * OUTPUT  : None.
 * RETUEN  : debug_mode
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetDebugMode(void)
{
    return board_info_table_p->debug_mode;
}

/* FUNCTION NAME : STKTPLG_BOARD_GetEPLDVer
 * PURPOSE : This function will get the epld version string
 * INPUT   : board_id      - board id of the local unit
 * OUTPUT  : epld_ver_p    - the epld version string
*            epld_ver_val_p- the raw value of the epld version register
 * RETUEN  : TRUE  -  Success
 *           FALSE -  Failed
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetEPLDVer(UI32_T board_id, char epld_ver_p[SYS_ADPT_EPLD_VER_STR_LEN+1], UI8_T* epld_ver_val_p)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -c";
    char* generated_shell_cmd_p=NULL;
    UI32_T output_buf_size=SYS_ADPT_EPLD_VER_STR_LEN+1;
    int ver_h, ver_l, dot_pos, i;
    BOOL_T ret_val=TRUE;

    if (epld_ver_p==NULL || epld_ver_val_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):Invalid input argument.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base_p);
    if(generated_shell_cmd_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    memset(epld_ver_p, 0, SYS_ADPT_EPLD_VER_STR_LEN+1);
    if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, epld_ver_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
        ret_val=FALSE;
        goto error_exit;
    }
    L_CHARSET_TrimTrailingNonPrintableChar(epld_ver_p);

    for (dot_pos=-1, i=0; i<output_buf_size; i++)
    {
        if (epld_ver_p[i]=='.')
        {
            dot_pos=i;
            break;
        }
    }

    if (dot_pos>0)
    {
        ver_h=atoi(epld_ver_p);
        ver_l=atoi(epld_ver_p+dot_pos+1);
        *epld_ver_val_p = (ver_h&0xF)<<4 | (ver_l&0xF);
    }

error_exit:
    if(generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;
}

/* FUNCTION NAME : STKTPLG_BOARD_SetBoardInformation
 * PURPOSE: This function will Set corresponding board information based
 *          on board ID.
 * INPUT:   board_id   -- board ID for this unit, it is project independent.
 *          board_info -- correponding board information.
 * OUTPUT:  None
 * RETUEN:  TRUE  -- successful
 *          FALSE -- fail
 * NOTES:
 *          None
 */
BOOL_T STKTPLG_BOARD_SetBoardInformation(UI8_T board_id, STKTPLG_BOARD_BoardInfo_T board_info)
{
    UI16_T  board_info_index;    

    /* check if this is a valid board id
     */ 
    if (board_id > STKTPLG_BOARD_MAX_BOARD_ID)      
    {
        return (FALSE);     
    }

    /* get index in the board information array
     */
    board_info_index =  board_id_2_info_mapping[board_id];

    /* report corresponding board information entry
     */
    SYSFUN_TakeWriteSem(sem_id_static);
    board_info_table[board_info_index]= board_info;
    SYSFUN_GiveWriteSem(sem_id_static);

    return (TRUE);

} /* End of STKTPLG_BOARD_GetBoardInformation() */

/* EXPORTED SUBPROGRAM BODIES
 */
 

/* FUNCTION NAME : STKTPLG_BOARD_GetBoardInformation
 * PURPOSE: This function will return corresponding board information based
 *          on board ID.
 * INPUT:   board_id -- board ID for this unit, it is project independent.
 * OUTPUT:  board_info -- correponding board information.
 * RETUEN:  TRUE -- successful
 *          FALSE -- fail          
 * NOTES:
 *          Due to board information table is a fixed table, caller only passes
 *          a pointer for board_info, this function will return related pointer
 *          to corresponding board information entry.
 */
BOOL_T STKTPLG_BOARD_GetBoardInformation(UI8_T board_id, STKTPLG_BOARD_BoardInfo_T *board_info)
{

    UI16_T  board_info_index;
       
    /* check if this is a valid board id
     */
    if (board_id > STKTPLG_BOARD_MAX_BOARD_ID)     
    {
        return (FALSE);    
    }
    
    /* get index in the board information array
     */
    board_info_index =  board_id_2_info_mapping[board_id];
    
    /* report corresponding board information entry
     */
    if (board_info_table == NULL)
    {
        *board_info = board_info_table_static[board_info_index];
        return (TRUE);
    }
    SYSFUN_TakeReadSem(sem_id_static);
    *board_info = board_info_table[board_info_index]; 
    SYSFUN_GiveReadSem(sem_id_static);
    
    return (TRUE);
       
} /* End of STKTPLG_BOARD_GetBoardInformation() */


/* wuli 2004-05-18 
 * INPUT:
 * module MODULE_TYPE_ONE_PORT_10G  = 0,
 * MODULE_TYPE_8SFX = 1
 */
BOOL_T STKTPLG_BOARD_GetModuleInformation(STKTPLG_BOARD_ModuleType_T module_id, STKTPLG_BOARD_ModuleInfo_T **module_info)
{
    UI8_T  module_type;

    /* Charles hard code here to wait for generic code for Alice.
     */
    if (VAL_swExpansionSlot1_eightPortSfpModule == module_id)
    {
        /* 8-port SFP module
         */
        module_type  = 0;
    }
    else
    {
        /* 10G module
         */
        module_type  = 1;
    }

    if ( module_type  >= MODULE_TYPE_MAX)
        return FALSE;

    *module_info = &module_info_table[module_type];
    
    return (TRUE);

} /* End of STKTPLG_BOARD_GetBoardInformation() */

BOOL_T  STKTPLG_BOARD_GetPortNumberInformation(UI8_T module_id, UI32_T *port_nbr)
{
   /* Modified as such so that this function works similar to 
    * STKTPLG_BOARD_GetBoardInformation
    */
    
    module_id = module_id - VAL_swModuleType_eightPortSfpModule;

    if(module_id >= MODULE_TYPE_MAX)
    {
    return FALSE;
    }

    *port_nbr=module_port_info_table[module_id].port_nbr;
    return TRUE;
  

} /* End of STKTPLG_BOARD_GetPortNumberInformation() */

UI8_T STKTPLG_BOARD_GetMaxPortNumberOnBoard(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].max_port_number_on_board;;
}

UI8_T STKTPLG_BOARD_GetMaxHotSwapPortNumber(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].max_hot_swap_port_number;
}

UI8_T STKTPLG_BOARD_GetMaxSlotsForExpansion(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].max_slots_for_expansion;
}

UI8_T STKTPLG_BOARD_GetStartExpansionPortNumber(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].start_expansion_port_number;    
}

UI8_T STKTPLG_BOARD_GetSfpPortNumber(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].sfp_port_number;    
}

UI8_T STKTPLG_BOARD_GetStartSfpPortNumber(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].start_sfp_port_number;
}

UI8_T STKTPLG_BOARD_GetMaxPortNumberOfThisUnit(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].max_port_number_of_this_unit;
}

UI8_T STKTPLG_BOARD_GetChipNumberOfThisUnit(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].chip_number_of_this_unit;
}

/* FUNCTION NAME : STKTPLG_BOARD_GetPortFigureType
 * PURPOSE : This function will output the port figure type by the specified board
 *           id and port id. 
 * INPUT   : board_id -- board ID
 *           port     -- port ID
 * OUTPUT  : port_figure_type_p -- port figure type of the specified port.
 * RETUEN  : TRUE  -- Output port figure type successfully.
 *           FALSE -- Failed to output port figure type.
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetPortFigureType(UI8_T board_id, UI16_T port, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p)
{
    if (board_id >= MAINBOARD_TYPE_MAX)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board_id %hu\r\n", __FUNCTION__, __LINE__, board_id);
        return FALSE;
    }

    if ((port==0) || (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid port %hu\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;    
    }

    if (port_figure_type_p == NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)port_figure_type_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    *port_figure_type_p = board_info_table[board_id].portFigureType_ptr[port - 1];

    return TRUE;
}

UI8_T STKTPLG_BOARD_GetFanNumber(void)
{
    UI16_T  board_info_index;

    board_info_index =  board_id_2_info_mapping[board_info_table_p->my_board_id];

    return board_info_table[board_info_index].fan_number;
}

static UI8_T STKTPLG_BOARD_GetFanNumberFromOnlp(void)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -f";
    char* generated_shell_cmd_p=NULL;
    char output_buf[32];
    int fan_number=0;
    UI32_T output_buf_size=sizeof(output_buf);

    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base_p);
    if (generated_shell_cmd_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
        goto error_exit;
    }
    fan_number=atoi(output_buf);
    if(fan_number>SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s(%d):Got total fan number as %d, but it should not be greater than %d\r\n",
            __FUNCTION__, __LINE__, fan_number, SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT);
        fan_number=SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;
    }

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return fan_number;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetFanControllerInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get fan controller related info for the given fan index.
 * INPUT:   fan_idx         -- the index of the fan (1 based)
 * OUTPUT:  fan_ctl_info_p  -- fan controller related info, see the comment for
 *                             SYS_HWCFG_FanControllerInfo_T in sys_hwcfg_common.h
 *                             for details.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetFanControllerInfo(UI32_T fan_idx, SYS_HWCFG_FanControllerInfo_T * fan_ctl_info_p)
{
    if (board_info_table_p->my_board_id>STKTPLG_BOARD_MAX_BOARD_ID)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__,
            (int)(board_info_table_p->my_board_id));
        return FALSE;
    }

    if ((fan_idx == 0) || (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT))
    {
        DEBUG_MSG("Invalid fan_idx(%lu), max fan_idx is %d\r\n",
            fan_idx, SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT);
        return FALSE;
    }

    if (fan_ctl_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): fan_ctl_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    memset(fan_ctl_info_p, 0, sizeof(*fan_ctl_info_p));
    if (board_info_table_p->my_board_id==0 || board_info_table_p->my_board_id==1 || board_info_table_p->my_board_id==2)
    {
        fan_ctl_info_p->fan_ctl_idx = 0;
        fan_ctl_info_p->fan_ctl_type = SYS_HWCFG_FAN_ONLP;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid board id(%hu)\r\n", __FUNCTION__, __LINE__,
            board_info_table_p->my_board_id);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetThermalControllerInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get thermal controller related info for the given thermal index.
 * INPUT:   thermal_idx     -- the index of the thermal sensor(1 based)
 * OUTPUT:  thermal_ctl_info_p
 *                          -- thermal controller related info, see the comment for
 *                             SYS_HWCFG_ThermalControlInfo_T in sys_hwcfg_common.h
 *                             for details.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetThermalControllerInfo(UI32_T thermal_idx, SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    if (board_info_table_p->my_board_id>STKTPLG_BOARD_MAX_BOARD_ID)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__,
            (int)(board_info_table_p->my_board_id));
        return FALSE;
    }

    if (thermal_ctl_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): thermal_ctl_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    memset(thermal_ctl_info_p, 0, sizeof(*thermal_ctl_info_p));

    if (board_info_table_p->my_board_id==0 || board_info_table_p->my_board_id==1 || board_info_table_p->my_board_id==2)
    {
        thermal_ctl_info_p->thermal_ctl_idx=0;
        thermal_ctl_info_p->thermal_type=SYS_HWCFG_THERMAL_ONLP;
    }
    else /* else if (board_info_table_p->my_board_id==1) */
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid board id(%hu).\r\n", __FUNCTION__, __LINE__, board_info_table_p->my_board_id);
        return FALSE;
    } /* end of if (board_info_table_p->my_board_id==0) */

    return TRUE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetPowerStatusInfoByBoardId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get power status related info for the given power index of the
 *          given board id.
 * INPUT:   board id      -- board id of the power status info to be got
 *          power_idx     -- the index of the power (1 based)
 * OUTPUT:  power_status_reg_info_p  -- power status register related info
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetPowerStatusInfoByBoardId(UI32_T board_id, UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p)
{
    if (board_id>STKTPLG_BOARD_MAX_BOARD_ID)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__,
            (int)(board_id));
        return FALSE;
    }

    if ((power_idx == 0) || (power_idx > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT))
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid power idx(%lu), max power idx is %d\r\n",
            __FUNCTION__, __LINE__, power_idx, SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT);
        return FALSE;
    }
    if (power_status_reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): power_status_reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return STKTPLG_BOARD_GetPowerStatusInfo_Internal(board_id, power_idx, power_status_reg_info_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetPowerStatusInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get power status related info for the given power index of the local
 *          unit.
 * INPUT:   power_idx     -- the index of the power (1 based)
 * OUTPUT:  power_status_reg_info_p  -- power status register related info
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetPowerStatusInfo(UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p)
{
    if (board_info_table_p->my_board_id>STKTPLG_BOARD_MAX_BOARD_ID)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__,
            (int)(board_info_table_p->my_board_id));
        return FALSE;
    }

    if ((power_idx == 0) || (power_idx > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT))
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid power idx(%lu), max power idx is %d\r\n",
            __FUNCTION__, __LINE__, power_idx, SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT);
        return FALSE;
    }
    if (power_status_reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): power_status_reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return STKTPLG_BOARD_GetPowerStatusInfo_Internal(board_info_table_p->my_board_id, power_idx, power_status_reg_info_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetPowerStatusInfo_Internal
 *---------------------------------------------------------------------------------
 * PURPOSE: Internal function to get power status related info for the given
 *          power index of the given board id
 * INPUT:   board_id      -- board id of the power status info to be got
 *          power_idx     -- the index of the power (1 based)
 * OUTPUT:  power_status_reg_info_p  -- power status register related info
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   1. This function is called when SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD is TRUE.
 *          2. This function assumes that the caller had validated the input
 *             arguments and will not do sanity check again.
 *---------------------------------------------------------------------------------
 */
static BOOL_T STKTPLG_BOARD_GetPowerStatusInfo_Internal(UI32_T board_id, UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p)
{
    memset(power_status_reg_info_p, 0, sizeof(*power_status_reg_info_p));

    if (board_info_table_p->my_board_id<=STKTPLG_BOARD_MAX_BOARD_ID)
    {
        /* This project uses ONLP driver
         */
        power_status_reg_info_p->power_is_present_val = 0;
        power_status_reg_info_p->power_present_mask=ONLPDRV_PSU_STATUS_PRESENT;

        power_status_reg_info_p->power_is_alert_val = 0;
        power_status_reg_info_p->power_alert_mask=ONLPDRV_PSU_STATUS_FAILED;

        power_status_reg_info_p->power_status_ok_val = 0;
        power_status_reg_info_p->power_status_mask=ONLPDRV_PSU_STATUS_UNPLUGGED;
        return TRUE;
    } /* if (board_id==0) */
    else
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid board id(%hu).\r\n", __FUNCTION__, __LINE__, board_id);
        return FALSE;
    } /* end of if (board_id==0) */

    return TRUE;
}

UI8_T STKTPLG_BOARD_GetThermalNumber(void)
{
    return board_info_table[board_info_table_p->my_board_id].thermal_number;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetThermalNumberFromOnlp
 *---------------------------------------------------------------------------------
 * PURPOSE: This function returns the number of thermal through ONLP library.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  Number of thermal on this local unit.
 * NOTES:   If the number of thermal got through ONLP library is larger than
 *          SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT, the number of thermal returned
 *          by this function will  be SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT.
 *---------------------------------------------------------------------------------
 */
static UI8_T STKTPLG_BOARD_GetThermalNumberFromOnlp(void)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -t";
    char* generated_shell_cmd_p=NULL;
    char output_buf[32];
    int thermal_number=0;
    UI32_T output_buf_size=sizeof(output_buf);

    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base_p);
    if(generated_shell_cmd_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
        goto error_exit;
    }
    thermal_number=atoi(output_buf);
    if(thermal_number>SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s(%d):Got total thermal number as %d, but it should not be greater than %d\r\n",
            __FUNCTION__, __LINE__, thermal_number, SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT);
        thermal_number=SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;
    }

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return thermal_number;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetONLPSFPPortMapping
 *---------------------------------------------------------------------------------
 * PURPOSE: This function output the array which maping the AOS user port id
 *          to ONLP port id.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  Number of thermal on this local unit.
 * NOTES:   All unused ports or non-sfp ports should be filled with ONLP_SFP_INVALID_PORT_ID.
 *          The mapping examples are shown below:
 *          If aos port id 1 is a sfp port and its corresponding onlp sfp port
 *          id is 0, then aos_uport_to_onlp_port[0] should be set as 0.
 *          If aos port id 2 is not a sfp port, then aos_uport_to_onlp_port[1]
 *          should be set as ONLP_SFP_INVALID_PORT_ID.
 *          If the maximum aos port id is 48, then all of the array elements
 *          after(including) aos_uport_to_onlp_port[48] should be set as
 *          ONLP_SFP_INVALID_PORT_ID when there is any unused elements in the
 *          tail of the array.
 *---------------------------------------------------------------------------------
 */

BOOL_T STKTPLG_BOARD_GetONLPSFPPortMapping(I16_T aos_uport_to_onlp_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    I16_T logical_uport, physical_uport;
    I16_T PHYSICAL_ON_BOARD_PORT_NUMBER, LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER;
    I16_T FIRST_PHYSICAL_BREAKOUT_PORT_ID;
    I16_T *aos_uport_to_onlp_port_dflt;


    switch(board_info_table_p->my_board_id)
    {
        case 0:
            PHYSICAL_ON_BOARD_PORT_NUMBER = PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0;
            LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER = LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_0;
			FIRST_PHYSICAL_BREAKOUT_PORT_ID = FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_0;
            aos_uport_to_onlp_port_dflt = aos_uport_to_onlp_port_bid_0;
            break;
        case 1:
            PHYSICAL_ON_BOARD_PORT_NUMBER = PHYSICAL_ON_BOARD_PORT_NUMBER_BID_1;
            LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER = LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_1;
			FIRST_PHYSICAL_BREAKOUT_PORT_ID = FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_1;
            aos_uport_to_onlp_port_dflt = aos_uport_to_onlp_port_bid_1;
            break;
        case 2:
            PHYSICAL_ON_BOARD_PORT_NUMBER = PHYSICAL_ON_BOARD_PORT_NUMBER_BID_2;
            LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER = LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER_BID_2;
			FIRST_PHYSICAL_BREAKOUT_PORT_ID = FIRST_PHYSICAL_BREAKOUT_PORT_ID_BID_2;
            aos_uport_to_onlp_port_dflt = aos_uport_to_onlp_port_bid_2;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__,
                (int)(board_info_table_p->my_board_id));
            return FALSE;
    }

    /* init all entries with ONLP_SFP_INVALID_PORT_ID(-1)
     */
    memset(aos_uport_to_onlp_port, 0xFF, sizeof(I16_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
	
    /* Fills the entries that the logical user port id is the same with
     * real physical user port id.
     */
    memcpy(aos_uport_to_onlp_port, aos_uport_to_onlp_port_dflt,
        sizeof(I16_T)*PHYSICAL_ON_BOARD_PORT_NUMBER);

    /* Here fiils the entries belongs to the logical user ports.
     * The conversion rule between logical user port id and physical user port id is:
     *   Given that logical user port id > PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0
     *   <real physical user port id> = integer part of ((<logical user port id> - PHYSICAL_ON_BOARD_PORT_NUMBER_BID_0 - 1) / 4) + 1
     */
    for (logical_uport=PHYSICAL_ON_BOARD_PORT_NUMBER+1;
         logical_uport<=PHYSICAL_ON_BOARD_PORT_NUMBER+LOGICAL_ON_BOARD_BREAKOUT_PORT_NUMBER;
         logical_uport++)
    {
        physical_uport = ((logical_uport - PHYSICAL_ON_BOARD_PORT_NUMBER - 1) / 4) + FIRST_PHYSICAL_BREAKOUT_PORT_ID;
        aos_uport_to_onlp_port[logical_uport-1] = aos_uport_to_onlp_port_dflt[physical_uport-1];
    }

    return TRUE;
}

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_GetHwPortModeInfo
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode info
 * INPUT   : board_id
 *           port
 *           hw_port_mode
 * OUTPUT  : hw_port_mode_info_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetHwPortModeInfo(
    UI32_T board_id,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode,
    STKTPLG_BOARD_HwPortModeInfo_T *hw_port_mode_info_p)
{
    DEV_SWDRV_Device_Port_Mapping_T *static_port_mapping_p;
    STKTPLG_BOARD_HwPortMode_PortMapping_T *hw_port_mode_mapping_p;
    UI16_T board_info_index;
    UI8_T mapping_uport = 0;

    if (board_id > STKTPLG_BOARD_MAX_BOARD_ID)
    {
        return FALSE;
    }

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    board_info_index = board_id_2_info_mapping[board_id];
    hw_port_mode_mapping_p = &stktplg_board_hw_port_mode_db[board_info_index].port_mapping;

    if (port > stktplg_board_hw_port_mode_db[board_info_index].board_info.max_port_number_of_this_unit)
    {
        return FALSE;
    }

    if (hw_port_mode == STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED)
    {
        hw_port_mode = stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1];
    }

    switch (hw_port_mode)
    {
        case STKTPLG_TYPE_HW_PORT_MODE_4x10G:
            mapping_uport = hw_port_mode_mapping_p->uport_mapping_4x10G[port-1];
            break;
        case STKTPLG_TYPE_HW_PORT_MODE_1x40G:
            mapping_uport = hw_port_mode_mapping_p->uport_mapping_1x40G[port-1];
            break;
        default:
            return FALSE;
    }

    if (mapping_uport == 0)
    {
        return FALSE;
    }

    if (port <= board_info_table_static[board_info_index].max_port_number_on_board)
    {
        static_port_mapping_p = board_info_table_static[board_info_index].userPortMappingTable[0];
    }
    else
    {
        static_port_mapping_p = stktplg_board_hw_port_mode_db[board_info_index].board_info.userPortMappingTable;
    }

    hw_port_mode_info_p->mapping_uport = mapping_uport;
    hw_port_mode_info_p->drv_device_id = static_port_mapping_p[port-1].device_id;
    hw_port_mode_info_p->drv_logical_port = static_port_mapping_p[port-1].device_port_id;
    hw_port_mode_info_p->drv_physical_port = hw_port_mode_mapping_p->drv_physical_port[port-1];
    hw_port_mode_info_p->dflt_hw_port_mode = stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_SetHwPortModeInfo
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode info
 * INPUT   : board_id
 *           port
 *           hw_port_mode
 *           hw_port_mode_info_p
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_SetHwPortModeInfo(
    UI32_T board_id,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode,
    STKTPLG_BOARD_HwPortModeInfo_T *hw_port_mode_info_p)
{
    DEV_SWDRV_Device_Port_Mapping_T *static_port_mapping_p;
    STKTPLG_BOARD_HwPortMode_PortMapping_T *hw_port_mode_mapping_p;
    UI16_T board_info_index;
    UI8_T *mapping_uport_p = NULL;

    if (board_id != 2 /* AS6812-32X */)
    {
        return FALSE;
    }

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    board_info_index = board_id_2_info_mapping[board_id];
    hw_port_mode_mapping_p = &stktplg_board_hw_port_mode_db[board_info_index].port_mapping;

    if (port > stktplg_board_hw_port_mode_db[board_info_index].board_info.max_port_number_of_this_unit)
    {
        return FALSE;
    }

    if (hw_port_mode == STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED)
    {
        hw_port_mode = stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1];
    }

    switch (hw_port_mode)
    {
        case STKTPLG_TYPE_HW_PORT_MODE_4x10G:
            mapping_uport_p = &hw_port_mode_mapping_p->uport_mapping_4x10G[port-1];
            break;
        case STKTPLG_TYPE_HW_PORT_MODE_1x40G:
            mapping_uport_p = &hw_port_mode_mapping_p->uport_mapping_1x40G[port-1];
            break;
        default:
            return FALSE;
    }

    if (mapping_uport_p == NULL)
    {
        return FALSE;
    }

    if (port <= board_info_table_static[board_info_index].max_port_number_on_board)
    {
        static_port_mapping_p = board_info_table_static[board_info_index].userPortMappingTable[0];
    }
    else
    {
        static_port_mapping_p = stktplg_board_hw_port_mode_db[board_info_index].board_info.userPortMappingTable;
    }

    static_port_mapping_p[port-1].device_port_id = hw_port_mode_info_p->drv_logical_port;
    hw_port_mode_info_p->dflt_hw_port_mode = stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1];

    *mapping_uport_p = hw_port_mode_info_p->mapping_uport;
    static_port_mapping_p[port-1].device_id = hw_port_mode_info_p->drv_device_id;
    static_port_mapping_p[port-1].device_port_id = hw_port_mode_info_p->drv_logical_port;
    hw_port_mode_mapping_p->drv_physical_port[port-1] = hw_port_mode_info_p->drv_physical_port;
    stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1] = hw_port_mode_info_p->dflt_hw_port_mode;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_SetHwPortModeStatus
 * -------------------------------------------------------------------------
 * PURPOSE : To set oper HW port mode
 * INPUT   : port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : For DEV_SWDRV_ChipInit only
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_SetHwPortModeStatus(
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode)
{
    DEV_SWDRV_Device_Port_Mapping_T *static_port_mapping_p;
    DEV_SWDRV_Device_Port_Mapping_T *port_mapping_p;
    STKTPLG_BOARD_BoardInfo_T *board_info_p;
    STKTPLG_BOARD_HwPortMode_PortMapping_T *hw_port_mode_mapping_p;
    UI16_T board_info_index;
    UI8_T mapping_uport = 0;

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    if (hw_port_mode == STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED ||
        hw_port_mode >= STKTPLG_TYPE_HW_PORT_MODE_MAX)
    {
        return FALSE;
    }

    if (hw_port_mode == board_info_table_p->hw_port_mode[port-1])
    {
        return TRUE;
    }

    board_info_index = board_id_2_info_mapping[board_info_table_p->my_board_id];
    hw_port_mode_mapping_p = &stktplg_board_hw_port_mode_db[board_info_index].port_mapping;

    switch (hw_port_mode)
    {
        case STKTPLG_TYPE_HW_PORT_MODE_4x10G:
            mapping_uport = hw_port_mode_mapping_p->uport_mapping_4x10G[port-1];
            break;
        case STKTPLG_TYPE_HW_PORT_MODE_1x40G:
            mapping_uport = hw_port_mode_mapping_p->uport_mapping_1x40G[port-1];
            break;
        default:
            return FALSE;
    }

    if (port <= board_info_table_static[board_info_index].max_port_number_on_board)
    {
        static_port_mapping_p = board_info_table_static[board_info_index].userPortMappingTable[0];
    }
    else
    {
        static_port_mapping_p = stktplg_board_hw_port_mode_db[board_info_index].board_info.userPortMappingTable;
    }

    board_info_p = &board_info_table[board_info_index];
    port_mapping_p = board_info_p->userPortMappingTable[0];

    if (mapping_uport == 0 || mapping_uport != port)
    {
        memcpy(&port_mapping_p[port-1], &stktplg_board_unused_port, sizeof(port_mapping_p[port-1]));
    }
    else
    {
        board_info_p->portMediaNum_ptr[port-1] =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].portMediaNum;
        board_info_p->portMediaType_ptr[port-1] =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].portMediaType;
        board_info_p->portJackType_ptr[port-1] =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].portJackType;
        board_info_p->portFigureType_ptr[port-1] =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].portFigureType;
        board_info_p->portMediaCap_ptr[port-1] =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].portMediaCap;
        port_mapping_p[port-1] = static_port_mapping_p[port-1];
        port_mapping_p[port-1].port_type =
            stktplg_board_hw_port_mode_db[board_info_index].board_info.port_info[hw_port_mode].port_type;
    }

    board_info_table_p->hw_port_mode[port-1] = hw_port_mode;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_GetAllHwPortModeStatus
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode oper status
 * INPUT   : None
 * OUTPUT  : oper_hw_port_mode_ar
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetAllHwPortModeStatus(UI8_T oper_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    memcpy(oper_hw_port_mode_ar, board_info_table_p->hw_port_mode, sizeof(board_info_table_p->hw_port_mode));

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_CheckHwPortModeConflict
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode oper status
 * INPUT   : cfg_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE (TRUE for no conflict)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_CheckHwPortModeConflict(
    UI32_T board_id,
    UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    STKTPLG_TYPE_HwPortMode_T hw_port_mode;
    STKTPLG_BOARD_HwPortMode_PortMapping_T *hw_port_mode_mapping_p;
    UI16_T board_info_index;
    UI8_T mapping_uport = 0;
    UI32_T port, max_port_number_on_board;

    int drv_logical_port_num_pipe[2] = {0};

    if (board_id != 2 /* AS6812-32X */)
    {
        return TRUE;
    }

    board_info_index = board_id_2_info_mapping[board_id];
    hw_port_mode_mapping_p = &stktplg_board_hw_port_mode_db[board_info_index].port_mapping;
    max_port_number_on_board = stktplg_board_hw_port_mode_db[board_info_index].board_info.max_port_number_on_board;

    for (port = 1; port <= max_port_number_on_board; port++)
    {
        hw_port_mode = cfg_hw_port_mode_ar[port-1];

        if (hw_port_mode == STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED)
        {
            hw_port_mode = stktplg_board_hw_port_mode_db[board_info_index].dflt_hw_port_mode[port-1];
        }

        switch (hw_port_mode)
        {
            case STKTPLG_TYPE_HW_PORT_MODE_4x10G:
                mapping_uport = hw_port_mode_mapping_p->uport_mapping_4x10G[port-1];
                break;
            case STKTPLG_TYPE_HW_PORT_MODE_1x40G:
                mapping_uport = hw_port_mode_mapping_p->uport_mapping_1x40G[port-1];
                break;
            default:
                return FALSE;
        }

        if (mapping_uport != port)
        {
            continue;
        }

        if ((port >= 1 && port <= 16) ||
            (port >= 33 && port <= 96))
        {
            drv_logical_port_num_pipe[0]++;
        }
        else
        {
            drv_logical_port_num_pipe[1]++;
        }
    }

    if ((drv_logical_port_num_pipe[0] > 52) ||
        (drv_logical_port_num_pipe[1] > 52))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

