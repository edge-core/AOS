/* Module Name: STKTPLG_TYPE.H
 * Purpose:
 *
 * Notes:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#ifndef     STKTPLG_TYPE_H
#define     STKTPLG_TYPE_H

#include "leaf_es3626a.h"


/* Those symbols will be removed after private-MIB definition those symbols
 * Aaron 2001/9/21, We will back here to removed those symbols.
 */
enum STKTPLG_PortType_E
{
    /* 1 ~ 100 is reserved vlaue, those value is define in
     * MIB(VAL_portType_XXX in leaf_es3626a.h" and using here
     */

    /* Those value not define in MIB, those are local values
     */
    STKTPLG_PORT_TYPE_NOT_EXIST  = 101,
    STKTPLG_PORT_TYPE_GBIC_EMPTY = 102,
    STKTPLG_PORT_TYPE_STACKING   = 103,
};


/* Type definition of main board
 */

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHBT = 0,
    STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHBT1,
    STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHELLO
};


enum STKTPLG_ModuleIndex_E
{
    STKTPLG_MODULE_INDEX_0    = 0,
    STKTPLG_MODULE_INDEX_1    = 1,
    STKTPLG_MODULE_INDEX_2    = 2,
    STKTPLG_MODULE_INDEX_3    = 3,
    STKTPLG_MODULE_INDEX_4    = 4,
    STKTPLG_MODULE_INDEX_5    = 5,
    STKTPLG_MODULE_INDEX_6    = 6,
    STKTPLG_MODULE_INDEX_7    = 7,
    STKTPLG_MODULE_INDEX_8    = 8,
    STKTPLG_MODULE_INDEX_9    = 9,
    STKTPLG_MODULE_INDEX_10   = 10,
    STKTPLG_MODULE_INDEX_11   = 11,
    STKTPLG_MODULE_INDEX_12   = 12,
    STKTPLG_MODULE_INDEX_13   = 13,
    STKTPLG_MODULE_INDEX_14   = 14,
    STKTPLG_MODULE_INDEX_15   = 15,
    STKTPLG_MODULE_INDEX_16   = 16,
    STKTPLG_MODULE_INDEX_17   = 17,
    STKTPLG_MODULE_INDEX_18   = 18,
    STKTPLG_MODULE_INDEX_19   = 19,
};


/*  for media type
*/
enum STKTPLG_MGR_MEDIA_TYPE_E
{
    STKTPLG_TYPE_MEDIA_TYPE_OTHER                          = 1,
    STKTPLG_TYPE_MEDIA_TYPE_HUNDRED_BASE_TX                = 2,
    STKTPLG_TYPE_MEDIA_TYPE_HUNDRED_BASE_FX                = 3,
    STKTPLG_TYPE_MEDIA_TYPE_HUNDRED_BASE_BX                = 4,
    STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_SX               = 5,
    STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_LX               = 6,
    STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_T                = 7,
    STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_CX               = 8,
    STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_X                = 9,

   /*These are port types, not media types, should use 1000SX or 1000LX*/
   /*STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_GBIC            = VAL_portType_thousandBaseGBIC, */
   /*STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_SFP             = VAL_portType_thousandBaseSfp,  */

    STKTPLG_TYPE_MEDIA_TYPE_HUNDRED_BASE_FX_SC_SINGLE_MODE = 10,
    STKTPLG_TYPE_MEDIA_TYPE_HUNDRED_BASE_FX_SC_MULTI_MODE  = 11,
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR                    = 12,
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_ER                    = 13,
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_CX4                   = 14,
    STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_X                     = 15,
};

enum STKTPLG_TYPE_PORT_MEDIA_CAP_E
{
    STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER                      = 0x00,
    STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER                     = 0x01,
    STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER                      = 0x02,
    STKTPLG_TYPE_PORT_MEDIA_CAP_POE_PSE                    = 0x04,
    STKTPLG_TYPE_PORT_MEDIA_CAP_POE_PD                     = 0x08
};

 /* STKTPLG_TYPE_Port_Figure_Type_E:
  *     This enum type defines all of the types for the apperance of a port. It
  *     will be used by WEB to draw the shape of a port on the front panel of
  *     web pages.
  */
typedef enum STKTPLG_TYPE_Port_Figure_Type_E
{
    STKTPLG_TYPE_PORT_FIGURE_TYPE_NULL, /* port not exists because the port is on a module and the module is not inserted */
    /* Start of the port figure type definitions copied from RFC2668-MIB
     * ifJackType. These types applies to the port that has built-in connector.
     */
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_RJ45S,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_DB9,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_BNC,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_FAUI,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_MAUI,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_FIBERSC,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_FIBERMIC,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_FIBERST,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_TELCO,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_MTRJ,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_HSSDC,
    /* End of the port figure type definitions copied from RFC2668-MIB
     * ifJackType.
     */
    STKTPLG_TYPE_PORT_FIGURE_TYPE_COMBO_PORT,
    /* Start of the port figure type that does not have built-in connector.
     */
    STKTPLG_TYPE_PORT_FIGURE_TYPE_SFP_CAGE,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_QSFP_CAGE,
    STKTPLG_TYPE_PORT_FIGURE_TYPE_XFP_CAGE,
    /* End of the port figure type that does not have built-in connector.
     */
    STKTPLG_TYPE_PORT_FIGURE_TYPE_TOTAL_NUM,
} STKTPLG_TYPE_Port_Figure_Type_T;

typedef enum STKTPLG_TYPE_STACKING_PORT_TYPE_E
{
    STKTPLG_TYPE_STACKING_PORT_UP_LINK                     = 1,
    STKTPLG_TYPE_STACKING_PORT_DOWN_LINK                   = 2,
    STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_TYPE           = 2,
} STKTPLG_TYPE_STACKING_PORT_TYPE_T;

typedef enum STKTPLG_TYPE_Stacking_Port_Option_E
{
    STKTPLG_TYPE_STACKING_PORT_OPTION_OFF             = 0,
    STKTPLG_TYPE_STACKING_PORT_OPTION_ONE             = 1,
    STKTPLG_TYPE_STACKING_PORT_OPTION_TWO             = 2,
    STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_OPTION    = 3,
} STKTPLG_TYPE_Stacking_Port_Option_T;

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
typedef enum {
    STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED,
    STKTPLG_TYPE_HW_PORT_MODE_4x10G,
    STKTPLG_TYPE_HW_PORT_MODE_1x40G,
    STKTPLG_TYPE_HW_PORT_MODE_4x25G,
    STKTPLG_TYPE_HW_PORT_MODE_1x100G,
    STKTPLG_TYPE_HW_PORT_MODE_MAX,
}
STKTPLG_TYPE_HwPortMode_T;

typedef struct
{
    UI32_T unit;
    struct {
        UI32_T port_start;
        UI32_T port_num;
        UI32_T port_type;
    } port_info[STKTPLG_TYPE_HW_PORT_MODE_MAX-1];
    UI32_T port_info_count;
    UI32_T supported_hw_port_mode;
    STKTPLG_TYPE_HwPortMode_T cfg_hw_port_mode;
    STKTPLG_TYPE_HwPortMode_T oper_hw_port_mode;
}
STKTPLG_TYPE_HwPortModeEntry_T;
#endif

#endif   /* STKTPLG_TYPE_H */
