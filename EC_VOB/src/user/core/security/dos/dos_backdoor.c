/* ------------------------------------------------------------------------
 * FILE NAME - DOS_BACKDOOR.C
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             24/01/2011      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2011
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_DOS == TRUE)
#include "dos_type.h"
#include "dos_backdoor.h"
#include "dos_mgr.h"
#include "dos_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */
typedef enum
{
    DOS_BACKDOOR_E_NONE,
    DOS_BACKDOOR_E_EXIT,
    DOS_BACKDOOR_E_HANDLED,
} DOS_BACKDOOR_Error_T;

typedef enum
{
    DOS_BACKDOOR_MENU_ACT_SHOW_MENU,
    DOS_BACKDOOR_MENU_ACT_EXEC_CMD,
} DOS_BACKDOOR_MenuAction_T;

typedef enum
{
    DOS_BACKDOOR_MENU_ID_COMMON,
    DOS_BACKDOOR_MENU_ID_FLAGS,
    DOS_BACKDOOR_MENU_ID_CONFIG,
    DOS_BACKDOOR_MENU_ID_MAX,
} DOS_BACKDOOR_MenuId_T;

typedef struct
{
    DOS_BACKDOOR_MenuAction_T menu_act;
    DOS_BACKDOOR_MenuId_T menu_id;
    char buf[16];
} DOS_BACKDOOR_Context_T;

typedef DOS_BACKDOOR_Error_T (*DOS_BACKDOOR_MenuHandler_T)
(
    DOS_BACKDOOR_Context_T *menu_ctx_p;
);

/* MACRO DEFINITIONS
 */

/* LOCAL FUNCTIONS DECLARATIONS
 */
static void DOS_BACKDOOR_Main(void);
static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Common(DOS_BACKDOOR_Context_T *menu_ctx_p);
static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Flags(DOS_BACKDOOR_Context_T *menu_ctx_p);
static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Config(DOS_BACKDOOR_Context_T *menu_ctx_p);
static UI32_T DOS_BACKDOOR_RequestUI32(char *title, UI32_T dflt_val);

/* LOCAL VARIABLES DECLARATIONS
 */
static DOS_BACKDOOR_MenuHandler_T dos_backdoor_manu_handlers[DOS_BACKDOOR_MENU_ID_MAX] =
{
    [DOS_BACKDOOR_MENU_ID_COMMON]   = DOS_BACKDOOR_Menu_Common,
    [DOS_BACKDOOR_MENU_ID_FLAGS]    = DOS_BACKDOOR_Menu_Flags,
    [DOS_BACKDOOR_MENU_ID_CONFIG]   = DOS_BACKDOOR_Menu_Config,
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOS_BACKDOOR_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DOS_BACKDOOR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(
        "dos",
        DOS_TYPE_IPCMSG_KEY,
        DOS_BACKDOOR_Main);
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void DOS_BACKDOOR_Main(void)
{
    DOS_BACKDOOR_Context_T menu_ctx;
    int i;

    menu_ctx.menu_id = DOS_BACKDOOR_MENU_ID_CONFIG;
    menu_ctx.menu_act = DOS_BACKDOOR_MENU_ACT_SHOW_MENU;

    while (1)
    {
        DOS_BACKDOOR_MenuHandler_T handlers[] = {
            dos_backdoor_manu_handlers[DOS_BACKDOOR_MENU_ID_COMMON],
            dos_backdoor_manu_handlers[menu_ctx.menu_id],
        };

        for (i = 0; i < sizeof(handlers)/sizeof(*handlers); i++)
        {
            if (handlers[i])
            {
                switch (handlers[i](&menu_ctx))
                {
                    case DOS_BACKDOOR_E_EXIT:
                        return;

                    case DOS_BACKDOOR_E_HANDLED:
                        i = INT_MAX;
                        break;

                    default:
                        ;
                }
            }
        }

        switch (menu_ctx.menu_act)
        {
            case DOS_BACKDOOR_MENU_ACT_SHOW_MENU:
                memset(menu_ctx.buf, 0, sizeof(menu_ctx.buf));
                BACKDOOR_MGR_Print("\r\nSelect> ");
                BACKDOOR_MGR_RequestKeyIn(menu_ctx.buf, sizeof(menu_ctx.buf)-1);
                BACKDOOR_MGR_Print("\r\n");
                menu_ctx.menu_act = DOS_BACKDOOR_MENU_ACT_EXEC_CMD;
                break;

            case DOS_BACKDOOR_MENU_ACT_EXEC_CMD:
                menu_ctx.menu_act = DOS_BACKDOOR_MENU_ACT_SHOW_MENU;
                break;

            default:
                ;
        }
    } /* end of while (1) */
}

static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Common(DOS_BACKDOOR_Context_T *menu_ctx_p)
{
    switch (menu_ctx_p->menu_act)
    {
        case DOS_BACKDOOR_MENU_ACT_SHOW_MENU:
            BACKDOOR_MGR_Print("\r\n  q. exit");
            if (menu_ctx_p->menu_id != DOS_BACKDOOR_MENU_ID_FLAGS)
                BACKDOOR_MGR_Print("\r\n  f. debug flags");
            if (menu_ctx_p->menu_id != DOS_BACKDOOR_MENU_ID_CONFIG)
                BACKDOOR_MGR_Print("\r\n  c. config");
            BACKDOOR_MGR_Print("\r\n");
            break;

        case DOS_BACKDOOR_MENU_ACT_EXEC_CMD:
            switch (menu_ctx_p->buf[0])
            {
                case 'q':
                    return DOS_BACKDOOR_E_EXIT;

                case 'f':
                    menu_ctx_p->menu_id = DOS_BACKDOOR_MENU_ID_FLAGS;
                    break;

                case 'c':
                    menu_ctx_p->menu_id = DOS_BACKDOOR_MENU_ID_CONFIG;
                    break;

                default:
                    goto exit;
            }
            return DOS_BACKDOOR_E_HANDLED;

        default:
            ;
    }

exit:
    return DOS_BACKDOOR_E_NONE;
}

static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Flags(DOS_BACKDOOR_Context_T *menu_ctx_p)
{
    char *flag_name[] = { DOS_TYPE_DBG_LST(DOS_TYPE_LST_NAME) };
    DOS_TYPE_DbgFlag_T dbg_flag;
    int i;

    switch (menu_ctx_p->menu_act)
    {
        case DOS_BACKDOOR_MENU_ACT_SHOW_MENU:
            for (dbg_flag = 0; dbg_flag < DOS_TYPE_DBG_NUM; dbg_flag++)
            {
                BACKDOOR_MGR_Printf("\r\n %2d. %-32s: %d",
                    dbg_flag,
                    flag_name[dbg_flag],
                    DOS_OM_IsDebugFlagOn(dbg_flag));
            }
            BACKDOOR_MGR_Print("\r\n");
            break;

        case DOS_BACKDOOR_MENU_ACT_EXEC_CMD:
            if (sscanf(menu_ctx_p->buf, "%d", &i) != 1)
            {
                break;
            }

            dbg_flag = i;

            if (dbg_flag >= DOS_TYPE_DBG_NUM)
            {
                break;
            }

            DOS_OM_SetDebugFlag(dbg_flag, !DOS_OM_IsDebugFlagOn(dbg_flag));
            return DOS_BACKDOOR_E_HANDLED;

        default:
            ;
    }

    return DOS_BACKDOOR_E_NONE;
}

static DOS_BACKDOOR_Error_T DOS_BACKDOOR_Menu_Config(DOS_BACKDOOR_Context_T *menu_ctx_p)
{
    char *field_name[] = { DOS_TYPE_FLD_LST(DOS_TYPE_LST_NAME) };
    char *error_name[] = { DOS_TYPE_E_LST(DOS_TYPE_LST_NAME) };

    DOS_TYPE_FieldId_T field_id;
    DOS_TYPE_FieldDataBuf_T data;
    DOS_TYPE_Error_T ret;

    int i;

    switch (menu_ctx_p->menu_act)
    {
        case DOS_BACKDOOR_MENU_ACT_SHOW_MENU:
            for (field_id = 0; field_id < DOS_TYPE_FLD_NUM; field_id++)
            {
                if (DOS_OM_GetDataByField(field_id, &data))
                {
                    switch (DOS_OM_GetDataTypeByField(field_id))
                    {
                        case DOS_TYPE_FTYPE_UI32:
                            BACKDOOR_MGR_Printf("\r\n %2d. %-48s: %lu",
                                field_id,
                                field_name[field_id],
                                data.u32);
                        default:
                            ;
                    }
                }
            }
            BACKDOOR_MGR_Print("\r\n");
            break;

        case DOS_BACKDOOR_MENU_ACT_EXEC_CMD:
            if (sscanf(menu_ctx_p->buf, "%d", &i) != 1)
            {
                break;
            }

            field_id = i;

            if (field_id >= DOS_TYPE_FLD_NUM)
            {
                break;
            }

            DOS_OM_GetDataByField(field_id, &data);

            switch (DOS_OM_GetDataTypeByField(field_id))
            {
                case DOS_TYPE_FTYPE_UI32:
                    data.u32 = DOS_BACKDOOR_RequestUI32(field_name[field_id], data.u32);
                    break;
                default:
                    ;
            }

            ret = DOS_MGR_SetDataByField(field_id, &data);

            BACKDOOR_MGR_Printf("\r\n%s\r\n", error_name[ret]);
            return DOS_BACKDOOR_E_HANDLED;

        default:
            ;
    }

    return DOS_BACKDOOR_E_NONE;
}

static UI32_T DOS_BACKDOOR_RequestUI32(char *title, UI32_T dflt_val)
{
    char buf[11];
    UI32_T tmp, val = dflt_val;

    if (title)
    {
        BACKDOOR_MGR_Printf("%s [%lu] ", title, dflt_val);
    }

    if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1))
    {
        if (sscanf(buf, "%lu", &tmp) == 1)
        {
            val = tmp;
        }
    }

    return val;
}

#endif /* (SYS_CPNT_DOS == TRUE) */

