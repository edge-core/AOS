/* Module Name: SWCTRL_BACKDOOR.C
 * Purpose:

 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier         Reason
 *       2002/9/23   Charles Cheng    Create this file -- obsolete swctrl_cmn.c
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_bld.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "swctrl.h"
#include "swctrl_om.h"
#include "swctrl_backdoor.h"
#include "backdoor_mgr.h"
#include "backdoor_lib.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "stktplg_pom.h"

/* NAMING CONSTANT
 */
#define SWCTRL_BACKDOOR_CMD_MAXLEN      16
#define KEY_ESC                         27
#define KEY_BACKSPACE                   8
#define KEY_EOS                         0
#define LRLF                            "\033[B"
#define UP                              "\033[A"
#define DOWN                            "\033[B"
#define RIGHT                           "\033[C"
#define LEFT                            "\033[D"

/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for the Switch Control
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    05/10/2002 -  Arthur Wu, created
 *-------------------------------------------------------------------------
 */
typedef struct
{
    char   *cmd_str;
    char   *cmd_title;
    char   *cmd_description;
    void    (*cmd_function)(char *cmd_buf);
} SWCTRL_BACKDOOR_CommandStruct_T;

typedef struct
{
    UI32_T  flag;
    char   *flag_string;
} SWCTRL_BACKDOOR_DebugStruct_T;

enum SWCTRL_BACKDOOR_FONT_COLOR_E
{
    BLACK   = 0,
    RED     = 1,
    GREEN   = 2,
    YELLOW  = 3,
    BLUE    = 4,
    MAGENTA = 5,
    CYAN    = 6,
    WHITE   = 7
};

enum SWCTRL_BACKDOOR_FONT_ATTRIBUTE_E
{
    NORMAL      = 0,
    HIGHLIGHT   = 1,
    BLINK       = 5,
    INVERSE     = 7,
    INVISIBLE   = 8
};

/* TYPE DECLARATIONS
 */

/* LOCAL VARIABLES
 */
static  void    SWCTRL_BACKDOOR_BackdoorMain (void);
static  void    SWCTRL_BACKDOOR_Engine (void);
static  void    SWCTRL_BACKDOOR_ParseCmd (char *cmd_buf, UI8_T ch);
static  void    SWCTRL_BACKDOOR_ExecuteCmd (char *cmd_buf, L_THREADGRP_Handle_T tg_handle,UI32_T backdoor_member_id);
static  void    SWCTRL_BACKDOOR_TerminateCmd (void);

static  void    SWCTRL_BACKDOOR_PrintMenu (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_SetDebugFlag (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_PrintAllDebugFlag (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_PrintDebugFlag (UI32_T flag, char *flag_string);

static  void    SWCTRL_BACKDOOR_SwitchInfo (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_PktTrapInfo (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_NotAvailable (char *cmd_buf);
static  void    SWCTRL_BACKDOOR_SfpPresent(char *cmd_buf);

static  void    SWCTRL_BACKDOOR_MoveCursorTo (UI8_T x, UI8_T y);
static  void    SWCTRL_BACKDOOR_Print ( char *str_format,
                                   char *str,
                                   UI8_T foreground_color,
                                   UI8_T background_color,
                                   UI8_T attribute);
static  void    SWCTRL_BACKDOOR_ClearScreen ();

#if (SYS_CPNT_HASH_SELECTION == TRUE)
static void SWCTRL_BACKDOOR_HashSelection(char *cmd_buf);
#endif

static void SWCTRL_BACKDOOR_ATC_BackdoorMain(void);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
static  void    SWCTRL_BACKDOOR_ForPrivateVlan();
static  void    SWCTRL_BACKDOOR_ShowPrivateVlanPortList();
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

static void SWCTRL_BACKDOOR_Dormant_BackdoorMain(void);

static UI32_T  debug_flag;

static  SWCTRL_BACKDOOR_CommandStruct_T SWCTRL_BACKDOOR_CommandTable[] =
{
    /*  cmd_str,    cmd_title,              cmd_descritption,                           cmd_function            */
    /*  ------------------------------------------------------------------------------------------------------  */
    {   "",         "Switch Control",       "Engineering Mode Main Menu",               SWCTRL_BACKDOOR_PrintMenu},
    {   "D",        "Debug Flag",           "To set the debug flag",                    SWCTRL_BACKDOOR_PrintMenu},
    {   "DD",       "Debug Flag Setting",   "To set the debug flag",                    SWCTRL_BACKDOOR_SetDebugFlag},
    {   "DS",       "Show Debug Flag",      "To show the debug flag",                   SWCTRL_BACKDOOR_PrintAllDebugFlag},
    {   "S",        "Switch Info",          "To show the SWITCH information",           SWCTRL_BACKDOOR_PrintMenu},
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    {   "SG",       "Sfp Present Status",   "To show the sfp present status",           SWCTRL_BACKDOOR_SfpPresent},
#endif
    {   "SS",       "System Info",          "To show the system information",           SWCTRL_BACKDOOR_SwitchInfo},
    {   "SP",       "Port Info",            "To show the port information",             SWCTRL_BACKDOOR_NotAvailable},
    {   "P",        "Pkt Trap Info",        "To show the packet trap information",      SWCTRL_BACKDOOR_PktTrapInfo},
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    {   "H",        "Hash Selection",       "To set hash selection for service",        SWCTRL_BACKDOOR_HashSelection},
#endif
    {   "\377",     "End of Table",         "End of the Command Table",                 SWCTRL_BACKDOOR_PrintMenu}
};

static  SWCTRL_BACKDOOR_DebugStruct_T  SWCTRL_BACKDOOR_DebugFlagTable[] =
{
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_NONE,                     "SWCTRL_BACKDOOR_DEBUG_FLAG_NONE"       },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_CALLBACK_NOTIFY,          "SWCTRL_BACKDOOR_DEBUG_FLAG_CALLBACK_NOTIFY"      },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_PORT_OPER_STATUS_CHANGED, "SWCTRL_BACKDOOR_DEBUG_FLAG_PORT_OPER_STATUS_CHANGED"},
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_RXTCN,                    "Reserve"      },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_AMTR,                     "SWCTRL_BACKDOOR_DEBUG_FLAG_AMTR(Temp)"      },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_ERRMSG,                   "Reserve"     },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_DBGMSG,                   "Reserve"     },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_MDIX,                     "SWCTRL_BACKDOOR_DEBUG_FLAG_MDIX"     },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_ABIL,                     "SWCTRL_BACKDOOR_DEBUG_FLAG_ABIL"     },
    {   SWCTRL_BACKDOOR_DEBUG_FLAG_SFP_DDM_TRAP,            "SWCTRL_BACKDOOR_DEBUG_FLAG_SFP_DDM_TRAP"     },

    {   SWCTRL_BACKDOOR_DEBUG_FLAG_ALL,                      "SWCTRL_BACKDOOR_DEBUG_FLAG_ALL"        }
};
static UI32_T debug_flag_atc;

/* No use now
 */
/*
static  UI8_T   SWCTRL_BACKDOOR_ClearScreenCode[] =
{ KEY_ESC, '[', '2', 'J', KEY_EOS };
 */
static  UI8_T present_x = 0, present_y = 0;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_BackdoorMain
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_BackdoorMain (void)
{

    SWCTRL_BACKDOOR_Engine();

    return;
} /* End of SWCTRL_BACKDOOR_BackdoorMain */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Engine
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_Engine(void)
{
    BOOL_T  engine_continue;
    UI8_T   cmd_index, cmd_buf_index;
    char   cmd_buf[SWCTRL_BACKDOOR_CMD_MAXLEN+1];
    L_THREADGRP_Handle_T tg_handle;
    UI32_T backdoor_member_id;
    int ch;

    tg_handle =(L_THREADGRP_Handle_T) L2_L4_PROC_COMM_GetSwctrlGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    engine_continue = TRUE;
    cmd_buf[0]      = 0;
    cmd_index       = 0;
    cmd_buf_index   = 0;
    while (engine_continue)
    {
        SWCTRL_BACKDOOR_ExecuteCmd (cmd_buf,tg_handle,backdoor_member_id);

        if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
        {
            continue;
        }

        switch (ch)
        {
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
            case 'r':
            case 'R':
                SWCTRL_BACKDOOR_ForPrivateVlan();
                break;
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

            case 'o':
            case 'O':
                SWCTRL_BACKDOOR_Dormant_BackdoorMain();
                break;

            case 'm':
            case 'M':
            case KEY_ESC:       /* Go to the main menu */
                cmd_buf[0]      = 0;
                break;

            case 'u':
            case 'U':
            case KEY_BACKSPACE: /* Go to the up level menu */
                if (cmd_buf[0] != 0)
                    cmd_buf[strlen(cmd_buf)-1]    = 0;
                break;

            case 'q':           /* Quit the engineering mode */
            case 'Q':
                engine_continue = FALSE;
                SWCTRL_BACKDOOR_Print ("", "", WHITE, BLACK, NORMAL);
                break;

            default:
                SWCTRL_BACKDOOR_ParseCmd (cmd_buf, ch);
                break;
        }

    }
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
} /* End of SWCTRL_BACKDOOR_Engine */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ParseCmd
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : If { cmd_buf[] | {ch} } exists in the SWCTRL_BACKDOOR_CommandTable
 *             then cmd_buf[] |= ch;
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_ParseCmd(char *cmd_buf, UI8_T ch)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SWCTRL_BACKDOOR_CMD_MAXLEN)
        return;

    if (ch >= 'a' && ch <= 'z')
        ch -= ('a' - 'A');
    cmd_buf[cmd_length]     = ch;
    cmd_buf[cmd_length+1]   = 0;

    while ((!cmd_found) && (SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\377'))
    {
        if (strcmp(cmd_buf, SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (!cmd_found)
        cmd_buf[cmd_length] = 0;

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ExecuteCmd
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_ExecuteCmd(char *cmd_buf, L_THREADGRP_Handle_T tg_handle,UI32_T backdoor_member_id)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SWCTRL_BACKDOOR_CMD_MAXLEN)
        return;

    while ((!cmd_found) && (SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\377'))
    {
        if (strcmp(cmd_buf, SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (cmd_found)
    {
        int i;

        SWCTRL_BACKDOOR_Print ("", "", GREEN, BLACK, NORMAL);
        SWCTRL_BACKDOOR_ClearScreen ();
        /* Print Title and Description */
        SWCTRL_BACKDOOR_MoveCursorTo (1, 1);
        for (i = 0; i < cmd_index ; i++)
            if (memcmp (SWCTRL_BACKDOOR_CommandTable[i].cmd_str
                       ,SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str
                       ,strlen (SWCTRL_BACKDOOR_CommandTable[i].cmd_str)) == 0)
        SWCTRL_BACKDOOR_Print("\\%s",  SWCTRL_BACKDOOR_CommandTable[i].cmd_title,
                                              GREEN, BLACK, NORMAL);
        SWCTRL_BACKDOOR_Print("\\%s",  SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_title,
                                              GREEN, BLACK, HIGHLIGHT);
        SWCTRL_BACKDOOR_Print("    %s",   SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_description
                                            , GREEN, BLACK, NORMAL);
        SWCTRL_BACKDOOR_Print ("%s", LRLF, GREEN, BLACK, NORMAL);
        SWCTRL_BACKDOOR_Print ("%s", "----------------------------------------------------------------",
                                    GREEN, BLACK, NORMAL);
        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
        SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_function(cmd_buf);
        /* Release execution permission from the thread group handler if necessary
        */
        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    }

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_TerminateCmd
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_TerminateCmd()
{
    SWCTRL_BACKDOOR_MoveCursorTo (23, 1);
    SWCTRL_BACKDOOR_Print ("%s", "    M", GREEN, WHITE, HIGHLIGHT);
    SWCTRL_BACKDOOR_Print ("%s", "ain Menu      ", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_Print ("%s", "U", GREEN, WHITE, HIGHLIGHT);
    SWCTRL_BACKDOOR_Print ("%s", "p Menu        ", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_Print ("%s", "                                      ", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_Print ("%s", "Q", GREEN, WHITE, HIGHLIGHT);
    SWCTRL_BACKDOOR_Print ("%s", "uit    ", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_MoveCursorTo (22, 35);
    SWCTRL_BACKDOOR_Print ("%s", "Copyright(C) Accton Corporation, 1999, 2000", BLUE, BLACK, HIGHLIGHT);
    SWCTRL_BACKDOOR_MoveCursorTo (23, 80);
    SWCTRL_BACKDOOR_Print ("", "", WHITE, BLACK, NORMAL);
    return;
} /* End of SWCTRL_BACKDOOR_TerminateCmd */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_PrintMenu
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_PrintMenu(char *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;

    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SWCTRL_BACKDOOR_CMD_MAXLEN)
        return;

    SWCTRL_BACKDOOR_Print ("%s", LRLF, GREEN, BLACK, NORMAL);
    while ( SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\377')
    {
        if (    ( (UI8_T)(strlen(SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str) ) == (cmd_length + 1) )
             && ( strncmp(SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str, cmd_buf, cmd_length) == 0)
           )
        {
            SWCTRL_BACKDOOR_Print ("%s", LRLF, GREEN, BLACK, NORMAL);
            SWCTRL_BACKDOOR_Print ("    %s",&(SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_str[cmd_length]),
                                       GREEN, BLACK, HIGHLIGHT);
            SWCTRL_BACKDOOR_Print ("%-24s -- ", &(SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_title[1]),
                                       GREEN, BLACK, NORMAL);
            SWCTRL_BACKDOOR_Print ("%-40s", SWCTRL_BACKDOOR_CommandTable[cmd_index].cmd_description,
                                       GREEN, BLACK, NORMAL);         
        }
        cmd_index++;
    }

    SWCTRL_BACKDOOR_TerminateCmd();

    return;
} /* SWCTRL_BACKDOOR_PrintMenu */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_PrintDebugFlag
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_PrintDebugFlag (UI32_T flag, char *flag_string)
{
    char   *status, chr[] = {0,0};

    if (SWCTRL_BACKDOOR_DebugFlagTable[flag].flag >= (UI32_T)SWCTRL_BACKDOOR_DEBUG_FLAG_ALL)
        return;

    if ((debug_flag & SWCTRL_BACKDOOR_DebugFlagTable[flag].flag) != 0)
        status = "ON";
    else
        status = "OFF";

    chr[0] = (UI8_T)('0' + (UI8_T)flag);
    SWCTRL_BACKDOOR_MoveCursorTo ((UI8_T) (2+flag*2), 2);
    SWCTRL_BACKDOOR_Print("%2s", chr, GREEN, BLACK, HIGHLIGHT);
    SWCTRL_BACKDOOR_MoveCursorTo ((UI8_T) (2+flag*2), 6);
    SWCTRL_BACKDOOR_Print(" -- %-32s", flag_string, GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_MoveCursorTo ((UI8_T) (2+flag*2), 60);
    SWCTRL_BACKDOOR_Print("%3s", status, GREEN, WHITE, NORMAL);
    return;
} /* End of SWCTRL_BACKDOOR_PrintDebugFlag */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_PrintAllDebugFlag
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_PrintAllDebugFlag (char *cmd_buf)
{
    UI8_T i = 1;

    while (SWCTRL_BACKDOOR_DebugFlagTable[i].flag < (UI32_T)SWCTRL_BACKDOOR_DEBUG_FLAG_ALL)
    {
        /* Print the menu and the current status */
        SWCTRL_BACKDOOR_PrintDebugFlag ( i,
                                    SWCTRL_BACKDOOR_DebugFlagTable[i].flag_string);
        i++;
    }

    SWCTRL_BACKDOOR_MoveCursorTo (23, 1);
    SWCTRL_BACKDOOR_Print ("%4s", "C", GREEN, WHITE, HIGHLIGHT);
    SWCTRL_BACKDOOR_Print ("%s", "lear all falg", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_Print ("%37s", "S", GREEN, WHITE, HIGHLIGHT);
    SWCTRL_BACKDOOR_Print ("%s", "top debug flag setting    ", GREEN, WHITE, NORMAL);
    SWCTRL_BACKDOOR_MoveCursorTo (22, 35);
    SWCTRL_BACKDOOR_Print ("%s", "Copyright(C) Accton Corporation, 1999, 2000", BLUE, BLACK, HIGHLIGHT);
    SWCTRL_BACKDOOR_MoveCursorTo (23, 80);
    SWCTRL_BACKDOOR_Print ("", "", WHITE, BLACK, NORMAL);

    if (strcmp(cmd_buf, "DS") == 0)
        SWCTRL_BACKDOOR_TerminateCmd ();

    return;
} /* End of SWCTRL_BACKDOOR_PrintAllDebugFlag*/


/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_SetDebugFlag
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_SetDebugFlag (char *cmd_buf)
{
    UI8_T   select, ch;
    UI32_T  set_flag;
    BOOL_T  set_continue;

    set_continue = TRUE;

    SWCTRL_BACKDOOR_PrintAllDebugFlag ("");

    while (set_continue)
    {
        ch = (UI8_T) BACKDOOR_MGR_GetChar();
        if (ch != 's' && ch != 'S' )
        {
            select = (UI8_T)(ch - '0');
            if (ch == 'c' || ch == 'C' )
            {
                debug_flag = 0x0;
                SWCTRL_BACKDOOR_PrintAllDebugFlag ("");
            }
            else if ( (select > 0) && (select < sizeof(SWCTRL_BACKDOOR_DebugFlagTable)/sizeof(*SWCTRL_BACKDOOR_DebugFlagTable)-1) )
            {
                set_flag = (UI32_T)SWCTRL_BACKDOOR_DebugFlagTable[select].flag;
                debug_flag ^= set_flag;
                SWCTRL_BACKDOOR_PrintDebugFlag ( select,
                                            SWCTRL_BACKDOOR_DebugFlagTable[select].flag_string);
                SWCTRL_BACKDOOR_MoveCursorTo (23, 80);
                SWCTRL_BACKDOOR_Print ("", "", WHITE, BLACK, NORMAL);
            }
            else
            {
                SWCTRL_BACKDOOR_ClearScreen ();
                SWCTRL_BACKDOOR_PrintAllDebugFlag ("");
            }
        }
        else
        {
            set_continue = FALSE;
        }
    } /* End of while */

    /* Terminating */
    SWCTRL_BACKDOOR_TerminateCmd();

    return;
} /* End of SWCTRL_BACKDOOR_SetDebugFlag */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_SwitchInfo
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_SwitchInfo(char *cmd_buf)
{
    SWCTRL_BACKDOOR_PrintMenu(cmd_buf);

    SWCTRL_BACKDOOR_TerminateCmd();
/*Charles*/
#if 0

    SWCTRL_Unit_Info_T  *unit_info;
    UI8_T               board_type[45], i;
#if defined(STRAWMAN) || defined(NOVAL) || defined(UNICORN)
    UI32_T              jumbo_frame_status;
#endif

    SWCTRL_GetUnitInfoPtr (&unit_info);

    switch (unit_info[1].board_type)
    {
    case 1:/* stackable */
        strcpy(board_type, "MAIN_BOARD_24_PORTS");
        break;
    case 2:/* stackable */
        strcpy(board_type, "MAIN_BOARD_48_PORTS");
        break;
    case 3:/* stackable */
        strcpy(board_type, "MAIN_BOARD_72_PORTS");
        break;
    case 4:
        strcpy(board_type, "MAIN_BOARD_24_PORTS_STANDALONE");
        break;
    case 5:
        strcpy(board_type, "MAIN_BOARD_48_PORTS_STANDALONE");
        break;
    case 6:
        strcpy(board_type, "MAIN_BOARD_72_PORTS_STANDALONE");
        break;
    case 7:
        strcpy(board_type, "MAIN_BOARD_8_PORTS_GIGA");
        break;
    case 8:   /* Unicorn nike temp */
        strcpy(board_type, "MAIN_BOARD_13_PORTS_GIGA");
        break;
    case 9:   /* Strawman nike temp */
        strcpy(board_type, "MAIN_BOARD_11_PORTS_GIGA");
        break;
    case 10:
        strcpy(board_type, "MAIN_BOARD_VS2512A");
        break;
    case 11:   /* NOVAL */
        strcpy(board_type, "MAIN_BOARD_24_PORTS_GIGA");
        break;
    case 12:   /* StrawmanHD */
        strcpy(board_type, "MAIN_BOARD_19_PORTS_PLUS_2_GIGA");
        break;
    case 13: /* Ruby */
        strcpy(board_type, "MAIN_BOARD_8_1000TX_PLUS_16_FIBER_GIGA");
        break;
    case 14: /* JBOS */
        strcpy(board_type, "MAIN_BOARD_24_PORTS_GIGA_PLUS_1_PORT");
        break;
    default:
        strcpy(board_type, "MAIN_BOARD_UNKNOWN");
    }
    SWCTRL_BACKDOOR_MoveCursorTo (6, 10);
    SWCTRL_BACKDOOR_Print ("%-20s", "Main Board Type:", GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_Print ("%s", board_type, GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_MoveCursorTo (8, 10);
    SWCTRL_BACKDOOR_Print ("%-20s", "Total Port Number:", GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_Print ("%d", (UI8_T*)((UI32_T)unit_info[1].port_number), GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_MoveCursorTo (10, 10);
    SWCTRL_BACKDOOR_Print ("%-20s", "MAC Address:", GREEN, BLACK, NORMAL);
    for (i = 0; i < 5; i++)
        SWCTRL_BACKDOOR_Print ("%02X:", (UI8_T*)((UI32_T)unit_info[1].mac[i]), GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_Print ("%02X", (UI8_T*)((UI32_T)unit_info[1].mac[i]), GREEN, BLACK, NORMAL);

#if defined(STRAWMAN) || defined(NOVAL) || defined(UNICORN)
    SWCTRL_GetJumboFrameStatus (&jumbo_frame_status);
    SWCTRL_BACKDOOR_MoveCursorTo (12, 10);
    SWCTRL_BACKDOOR_Print ("%-20s", "Jumbo Frame:", GREEN, BLACK, NORMAL);
    SWCTRL_BACKDOOR_Print ("%s", (jumbo_frame_status)? "Enable" : "Disable" , GREEN, BLACK, NORMAL);
#endif
    /* Terminating */
    SWCTRL_BACKDOOR_TerminateCmd();

#endif
    return;
} /* End of SWCTRL_BACKDOOR_ResetRxCfgFlag */

static void SWCTRL_BACKDOOR_PktTrapInfo(char *cmd_buf)
{
    char *owner_name[][32] =
    {
        [0] = {
            "IGMP",
            "IGMPSNP",
            "PIM",
            "MLD",
            "MLDSNP",
            "PIM6",
            "MVR",
            "MVR6",
        },
        [1] = {
            "DHCP_CLIENT",
            "DHCPSNP",
            "L2_RELAY",
            "L3_RELAY",
            "DHCP_SERVER",
            "UDP_HELPER_67",
            "UDP_HELPER_68",
        },
        [2] = {
            "DHCP6_CLIENT",
            "DHCP6_RELAY",
            "DHCP6_SERVER",
            "DHCP6SNP",
        },
        [3] = {
            "CLUSTER",
            "ERPS",
            "LBD",
        },
        [4] = {
            "UDLD",
        },
        [5] = {
            "L2PT",
            "UDLD",
        },
        [6] = {
            "L2PT",
        },
        [7] = {
            "NETCFG_ND"
            "NDSNP",
        },
        [8] = {
            "PSEC",
            "DOT1X",
            "NETACCESS",
            "DHCPSNP",
        },
    };

    struct {
        char *name;
        char **owner_name;
    }
    pkt_types[SWCTRL_PKTTYPE_MAX] =
    {
        { "IGMP",           owner_name[0] },
        { "RESERVED_UDP",   owner_name[0] },
        { "MLD",            owner_name[0] },
        { "IPMC",           owner_name[0] },
        { "IPMC6",          owner_name[0] },
        { "DHCP_SERVER",    owner_name[1] },
        { "DHCP_CLIENT",    owner_name[1] },
        { "DHCP6_SERVER",   owner_name[2] },
        { "DHCP6_CLIENT",   owner_name[2] },
        { "ORG_SPECIFIC",   owner_name[3] },
        { "ORG_SPECIFIC2",  owner_name[4] },
        { "CDP",            owner_name[5] },
        { "PVST",           owner_name[6] },
        { "NDSNP",          owner_name[7] },
        { "INTRUDER",       owner_name[8] },
    };

    SWCTRL_PktType_T pkt_type;
    SWCTRL_TrapPktOwner_T owner;
    UI32_T trap_owners;
    UI32_T drop_owners;
    UI32_T ifindex = 0;
    UI32_T cpu_rate;
    BOOL_T is_trap, is_drop;
    char buf[32];
    int i;

    BACKDOOR_MGR_Printf("\033[%d;%dm", NORMAL, (30 + WHITE));
    BACKDOOR_MGR_Printf("\n\n");

    while (1)
    {
        /* menu of pke_type
         */
        BACKDOOR_MGR_Printf("\n    pkt_type         TRAP     DROP     ACTION      CPU RATE");
        BACKDOOR_MGR_Printf("\n -- ---------------- -------- -------- ----------- --------");
                             /* 00 IGMP             00000001 00000001 TRAP TO CPU        0 */

        for (pkt_type = 0; pkt_type < SWCTRL_PKTTYPE_MAX; pkt_type++)
        {
            if (ifindex == 0)
            {
                if (!SWCTRL_GetPktTrapStatusEx(pkt_type, &trap_owners, &drop_owners))
                {
                    break;
                }
            }
            else
            {
                if (!SWCTRL_GetPortPktTrapStatusEx(ifindex, pkt_type, &trap_owners, &drop_owners))
                {
                    break;
                }
            }

            BACKDOOR_MGR_Printf("\n %2d %-16s %08lx %08lx %-11s",
                pkt_type,
                pkt_types[pkt_type].name,
                trap_owners,
                drop_owners,
                trap_owners ?
                    (drop_owners ? "TRAP TO CPU" : "COPY TO CPU") :
                    (drop_owners ? "DROP" : "FLOOD"));

            if (ifindex == 0)
            {
                if (!SWCTRL_GetCpuRateLimit(pkt_type, &cpu_rate))
                {
                    cpu_rate = 0;
                }

                BACKDOOR_MGR_Printf(" %8lu", cpu_rate);
            }
        }

        BACKDOOR_MGR_Printf("\n p: Port: %lu%s", ifindex, (ifindex == 0) ? " (Global)" : "");
        BACKDOOR_MGR_Printf("\n q: Quit");
        BACKDOOR_MGR_Printf("\n Select> ");
        BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
        BACKDOOR_MGR_Printf("\n");

        if (buf[0] == 'q' || buf[0] == 'Q')
            break;

        if (buf[0] == 'p' || buf[0] == 'P')
        {
            BACKDOOR_MGR_Printf("Port> ");
            BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
            BACKDOOR_MGR_Printf("\n");
            sscanf(buf, "%lu", &ifindex);
            continue;
        }

        if (sscanf(buf, "%d", &pkt_type) < 1)
            continue;

        while (1)
        {
            /* menu of owner
             */
            if (ifindex == 0)
            {
                if (!SWCTRL_GetPktTrapStatusEx(pkt_type, &trap_owners, &drop_owners))
                {
                    break;
                }
            }
            else
            {
                if (!SWCTRL_GetPortPktTrapStatusEx(ifindex, pkt_type, &trap_owners, &drop_owners))
                {
                    break;
                }
            }

            if (pkt_types[pkt_type].owner_name == NULL)
            {
                break;
            }

            BACKDOOR_MGR_Printf("\n %2d %-16s %08lx %08lx %s",
                pkt_type,
                pkt_types[pkt_type].name,
                trap_owners,
                drop_owners,
                trap_owners ?
                    (drop_owners ? "TRAP TO CPU" : "COPY TO CPU") :
                    (drop_owners ? "DROP" : "FLOOD"));
            BACKDOOR_MGR_Printf("\n");

            BACKDOOR_MGR_Printf("\n    owner            TRAP DROP");
            BACKDOOR_MGR_Printf("\n -- ---------------- ---- ----");
                                 /* 00 IGMP                1    0 */

            for (i = 0; i < 16; i++)
            {
                owner = 1 << i;

                if (!pkt_types[pkt_type].owner_name[i] &&
                    !(trap_owners & owner) &&
                    !(drop_owners & owner))
                {
                    continue;
                }

                BACKDOOR_MGR_Printf("\n %2d %-16s %4d %4d",
                    i,
                    pkt_types[pkt_type].owner_name[i] ? pkt_types[pkt_type].owner_name[i] : "Unknown",
                    !!(trap_owners & owner),
                    !!(drop_owners & owner));
            }

            if (ifindex == 0)
            {
                if (!SWCTRL_GetCpuRateLimit(pkt_type, &cpu_rate))
                {
                    cpu_rate = 0;
                }

                BACKDOOR_MGR_Printf("\n r: Rate: %lu", cpu_rate);
            }

            BACKDOOR_MGR_Printf("\n p: Port: %lu", ifindex);
            BACKDOOR_MGR_Printf("\n q: Quit");
            BACKDOOR_MGR_Printf("\n Select> ");
            BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
            BACKDOOR_MGR_Printf("\n");

            if (buf[0] == 'q' || buf[0] == 'Q')
                break;

            if (buf[0] == 'p' || buf[0] == 'P')
            {
                BACKDOOR_MGR_Printf("Port> ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\n");
                sscanf(buf, "%lu", &ifindex);
                continue;
            }

            if (ifindex == 0)
            {
                if (buf[0] == 'r' || buf[0] == 'R')
                {
                    BACKDOOR_MGR_Printf("Rate> ");
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                    BACKDOOR_MGR_Printf("\n");

                    if (1 == sscanf(buf, "%lu", &cpu_rate))
                    {
                        BACKDOOR_MGR_Printf(" result: %d\n",
                            SWCTRL_SetCpuRateLimit(pkt_type, cpu_rate));
                    }
                    continue;
                }
            }

            if (sscanf(buf, "%d", &i) < 1)
                continue;

            owner = 1 << i;

            BACKDOOR_MGR_Printf(" trap? ");
            BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
            BACKDOOR_MGR_Printf("\n");
            is_trap = atoi(buf);
            BACKDOOR_MGR_Printf(" drop? ");
            BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
            BACKDOOR_MGR_Printf("\n");
            is_drop = atoi(buf);

            BACKDOOR_MGR_Printf(" result: %d\n",
                SWCTRL_SetPktTrapStatus(pkt_type, owner, is_trap, is_drop));
        }
    }

    SWCTRL_BACKDOOR_TerminateCmd();
}

#if (SYS_CPNT_HASH_SELECTION == TRUE)
static void SWCTRL_BACKDOOR_HashSelection(char *cmd_buf)
{
    UI8_T   ch;
    UI8_T   list_index;
    UI8_T   action;

    BACKDOOR_MGR_Printf("\r\nHash_Selction_List <1-4> for ECMP  = ");

    BACKDOOR_MGR_RequestKeyIn(&ch, 1);
    list_index = (UI8_T)(ch - '0');

    BACKDOOR_MGR_Printf("\r\nAction <0: Unbind, 1: Bind>  = ");

    BACKDOOR_MGR_RequestKeyIn(&ch, 1);
    action = (UI8_T)(ch - '0');

    if (action == 0)
    {
        if (TRUE != SWCTRL_UnBindHashSelForService(SWCTRL_OM_HASH_SEL_SERVICE_ECMP, list_index))
        {
           BACKDOOR_MGR_Printf("\r\nUnbind hash selection for ECMP fail"); 
        }
    }
    else
    {
        if (TRUE != SWCTRL_BindHashSelForService(SWCTRL_OM_HASH_SEL_SERVICE_ECMP, list_index))
        {
           BACKDOOR_MGR_Printf("\r\nBind hash selection for ECMP fail"); 
        }
    }

    /* Terminating */
    SWCTRL_BACKDOOR_TerminateCmd();
}
#endif

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_NotAvailable
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_NotAvailable(char *cmd_buf)
{
    SWCTRL_BACKDOOR_MoveCursorTo (12, 27);
    SWCTRL_BACKDOOR_Print ("%s", ">>", RED, BLACK, BLINK);
    SWCTRL_BACKDOOR_Print ("%s", "Sorry!! Not available now ...", RED, BLACK, HIGHLIGHT);

    /* Terminating */
    SWCTRL_BACKDOOR_TerminateCmd();

    return;
} /* End of SWCTRL_BACKDOOR_NotAvailable */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_SfpPresent
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_SfpPresent(char *cmd_buf)
{
    UI32_T ifindex, unit, port, sfp_index, trunk_id;
    BOOL_T is_present;

    BACKDOOR_MGR_Printf("\r\n");
    for(ifindex=1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
    {
        if(SWCTRL_LPORT_NORMAL_PORT == SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
        {
            if(TRUE == STKTPLG_POM_UserPortToSfpIndex(unit, port, &sfp_index))
            {
                SWCTRL_GetPortSfpPresent(unit, sfp_index, &is_present);
                BACKDOOR_MGR_Printf("%s-%d unit:%ld port:%ld, sfp_index:%ld is_present:0x%x\r\n", __FUNCTION__, __LINE__,
                       unit, port, sfp_index, is_present);
            }

        }
    }

    /* Terminating */
    SWCTRL_BACKDOOR_TerminateCmd();

    return;
} /* End of SWCTRL_BACKDOOR_SfpPresent */
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_MoveCursorTo
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_MoveCursorTo (UI8_T x, UI8_T y)
{
    BACKDOOR_MGR_Printf("\x1b[%d;%dH",x ,y);
    present_x = x;
    present_y = y;

    return;
} /* End of SWCTRL_BACKDOOR_MoveCursorTo */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Print
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_Print ( char *str_format,
                                            char *str,
                                            UI8_T foreground_color,
                                            UI8_T background_color,
                                            UI8_T attribute)
{
    char str_buf[80];

    BACKDOOR_MGR_Printf ("\033[%d;%d;%dm", attribute, (30 + foreground_color), (40 + background_color));
    sprintf (str_buf, str_format, str);
    BACKDOOR_MGR_Printf (str_buf);
    /* adjust coordinate
     */
    if (strcmp (str, LRLF) == 0 )
        SWCTRL_BACKDOOR_MoveCursorTo ((UI8_T) (present_x+1), 1);
    else
        present_y = (UI8_T) (present_y + strlen (str_buf));

    return;
}/* End of SWCTRL_BACKDOOR_Print */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ClearScreen
 * ------------------------------------------------------------------------
 * FUNCTION :
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_ClearScreen ()
{
    SWCTRL_BACKDOOR_MoveCursorTo (1, 1);
    BACKDOOR_MGR_Printf("\x1b[2J");

    return;
} /* End of SWCTRL_BACKDOOR_ClearScreen */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ATC_BackdoorMain
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the atc backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SWCTRL_BACKDOOR_ATC_BackdoorMain(void)
{
#define MAXLINE  255
    char line_buffer[MAXLINE] = {0};
    int select_value = 0;

    while(1)
    {
        BACKDOOR_MGR_Print("\r\n\r\nPress <enter> to continue.");

        BACKDOOR_MGR_Print("\r\n===========================================");
        BACKDOOR_MGR_Print("\r\n  Auto Traffic Control Manager Menu 2007/01/01  ");
        BACKDOOR_MGR_Print("\r\n===========================================");

        BACKDOOR_MGR_Print("\r\n [1] Enable Packet Counter Flag. ");
        BACKDOOR_MGR_Print("\r\n [2] Disable Packet Counter Flag. ");
#if (SYS_CPNT_ATC_BSTORM == TRUE)
        BACKDOOR_MGR_Print("\r\n [11] Enable Broadcast Event Flag. ");
        BACKDOOR_MGR_Print("\r\n [12] Enable Broadcast Transfer State Machine Flag. ");
        BACKDOOR_MGR_Print("\r\n [13] Disable Broadcast Event Flag. ");
        BACKDOOR_MGR_Print("\r\n [14] Disable Broadcast Transfer State Machine Flag. ");
#endif /* SYS_CPNT_ATC_BSTORM */
#if (SYS_CPNT_ATC_MSTORM == TRUE)
        BACKDOOR_MGR_Print("\r\n [21] Enable Multicast Event Flag. ");
        BACKDOOR_MGR_Print("\r\n [22] Enable Multicast Transfer State Machine Flag. ");
        BACKDOOR_MGR_Print("\r\n [23] Disable Multicast Event Flag. ");
        BACKDOOR_MGR_Print("\r\n [24] Disable Multicast Transfer State Machine Flag. ");
#endif /* SYS_CPNT_ATC_MSTORM */
        BACKDOOR_MGR_Print("\r\n [99] Exit Auto Traffic Control Manager Menu!!");
        BACKDOOR_MGR_Print("\r\n Enter Selection: ");

        BACKDOOR_MGR_RequestKeyIn(line_buffer, (MAXLINE-1));
        select_value = atoi(line_buffer);
        BACKDOOR_MGR_Printf("\r\nSelect value is %d", select_value); /* Debug message */

        switch(select_value)
        {

            case 1:
                debug_flag_atc |= SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_PACKET_COUNTER;
                BACKDOOR_MGR_Print("\r\n [1] Enable Packet Counter Flag. ");
                break;

            case 2:
                debug_flag_atc &= ~SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_PACKET_COUNTER;
                BACKDOOR_MGR_Print("\r\n [2] Disable Packet Counter Flag. ");
                break;

#if (SYS_CPNT_ATC_BSTORM == TRUE)
            case 11:
                debug_flag_atc |= SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_EVENT;
                BACKDOOR_MGR_Print("\r\n [11] Enable Broadcast Event Flag. ");
                break;

            case 12:
                debug_flag_atc |= SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_TRANSFER_STATE_MACHINE;
                BACKDOOR_MGR_Print("\r\n [12] Enable Broadcast Transfer State Machine Flag. ");
                break;

            case 13:
                debug_flag_atc &= ~SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_EVENT;
                BACKDOOR_MGR_Print("\r\n [13] Disable Broadcast Event Flag. ");
                break;

            case 14:
                debug_flag_atc &= ~SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_TRANSFER_STATE_MACHINE;
                BACKDOOR_MGR_Print("\r\n [14] Disable Broadcast Transfer State Machine Flag. ");
                break;
#endif /* SYS_CPNT_ATC_BSTORM */

#if (SYS_CPNT_ATC_MSTORM == TRUE)
            case 21:
                debug_flag_atc |= SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_EVENT;
                BACKDOOR_MGR_Print("\r\n [21] Enable Multicast Event Flag. ");
                break;

            case 22:
                debug_flag_atc |= SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_TRANSFER_STATE_MACHINE;
                BACKDOOR_MGR_Print("\r\n [22] Enable Multicast Transfer State Machine Flag. ");
                break;

            case 23:
                debug_flag_atc &= ~SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_EVENT;
                BACKDOOR_MGR_Print("\r\n [23] Disable Multicast Event Flag. ");
                break;

            case 24:
                debug_flag_atc &= ~SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_TRANSFER_STATE_MACHINE;
                BACKDOOR_MGR_Print("\r\n [24] Disable Multicast Transfer State Machine Flag. ");
                break;
#endif /* SYS_CPNT_ATC_MSTORM */

            case 99:
                BACKDOOR_MGR_Print("\r\n [99] Exit Auto Traffic Control Manager Menu!!");
                return;

            default:
                BACKDOOR_MGR_Print("\r\n Invalid select. ");
                break;

        } /* End of switch(select_value) */


    } /* End of while(1) */

}


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/****************************************************************************/
/* Switch Initialization */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl_init.c
 * -------------------------------------------------------------------------*/
void SWCTRL_BACKDOOR_Init(void)
{
    debug_flag = 0x0;
    debug_flag_atc = 0x0;
} /* End of SWCTRL_BACKDOOR_Init() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl_init.c
 * -------------------------------------------------------------------------*/
void SWCTRL_BACKDOOR_Create_InterCSC_Relation(void)
{
    /* register call back function for backdoor program
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("swctrl",
        SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,SWCTRL_BACKDOOR_BackdoorMain);

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack ("atc_mgr",
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,SWCTRL_BACKDOOR_ATC_BackdoorMain);

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_IsDebugFlagOn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the debug flag is on or not
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl.c
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_BACKDOOR_IsDebugFlagOn(UI32_T flag)
{
    return (((debug_flag & flag) == flag) ? TRUE : FALSE);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_IsDebugFlagATCOn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the debug flag of ATC is on or not
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl.c
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_BACKDOOR_IsDebugFlagATCOn(UI32_T flag)
{
    return (((debug_flag_atc & flag) == flag) ? TRUE : FALSE);
}

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ForPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function a backdoor for private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void SWCTRL_BACKDOOR_ForPrivateVlan()
{

#define SWCTRL_BACKDOOR_BYTE_IN_BITMAP(INDEX)   ((int)((INDEX-1)/8))
#define SWCTRL_BACKDOOR_BIT_IN_BITMAP(INDEX)    (1 << (7 - ((INDEX-1) - SWCTRL_BACKDOOR_BYTE_IN_BITMAP((INDEX))*8)))

    UI32_T  session_id;
    UI32_T  dw_port, up_port;
    char    string[10], ch;
    UI8_T   up_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   dw_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    BOOL_T  eof = FALSE;
    UI8_T   BACKDOOR_ClearScreen[] = { "\033[2J" };

    BACKDOOR_MGR_Printf("%s", BACKDOOR_ClearScreen);

    while (eof!=TRUE)
    {
        BACKDOOR_MGR_Print("\n\r");
        BACKDOOR_MGR_Print("==== SWCTRL Private VLAN Backdoor =============\n");
        BACKDOOR_MGR_Print("a/A : enable/disable private vlan              \n");
        BACKDOOR_MGR_Print("b   : add uplink/downlink port by group id     \n");
        BACKDOOR_MGR_Print("c   : remove uplink/downlink port by group id  \n");
        BACKDOOR_MGR_Print("d   : destroy private vlan session             \n");
        BACKDOOR_MGR_Print("e/E : enable/disable blocking uplink ports     \n");
        BACKDOOR_MGR_Print("s   : display private vlan                     \n");
        BACKDOOR_MGR_Print("q/Q : exit                                     \n");
        BACKDOOR_MGR_Print("===============================================\n");
        BACKDOOR_MGR_Print(" Select =");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c",ch);

        switch (ch)
        {
            case 'a':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Print("The Private VLAN is :");
                if (SWCTRL_EnablePrivateVlan()== TRUE)
                {
                    BACKDOOR_MGR_Print(" (Enabled)\n\r");
                }
                else
                {
                    BACKDOOR_MGR_Print(" (Disabled)\n\r");
                }
                break;

            case 'A':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Print("The Private VLAN is :");
                if (SWCTRL_DisablePrivateVlan() == TRUE)
                {
                    BACKDOOR_MGR_Print(" (Disabled)\n\r");
                }
                else
                {
                    BACKDOOR_MGR_Print(" (Enabled)\n\r");
                }
                break;

            case 'b':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Printf("\n\r please enter group id [1-%d]: ",SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS);

                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                session_id = atoi(string);

                /* check session_id is exceed */
                if (session_id > SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS || session_id <=0)
                {
                    BACKDOOR_MGR_Print("\r\nThe session id is wrong\n\r");
                    break;
                }

                BACKDOOR_MGR_Print("\n\r If port is 0, do nothing");
                BACKDOOR_MGR_Print("\n\r uplink port : ");
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                up_port = atoi(string);

                if(up_port >SYS_ADPT_TOTAL_NBR_OF_LPORT)
                {
                    BACKDOOR_MGR_Print("\r\nUplink Port Invalid\n\r");
                    break;
                }

                BACKDOOR_MGR_Print("\n\r downlink port : ");
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                dw_port = atoi(string);

                if(dw_port >SYS_ADPT_TOTAL_NBR_OF_LPORT)
                {
                    BACKDOOR_MGR_Print("\r\nDownlink Port Invalid\n\r");
                    break;
                }

                memset(up_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(dw_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

                if (up_port>0)
                up_port_list[ SWCTRL_BACKDOOR_BYTE_IN_BITMAP(up_port) ] |= SWCTRL_BACKDOOR_BIT_IN_BITMAP(up_port);

                if (dw_port>0)
                dw_port_list[ SWCTRL_BACKDOOR_BYTE_IN_BITMAP(dw_port) ] |= SWCTRL_BACKDOOR_BIT_IN_BITMAP(dw_port);

                if (FALSE == SWCTRL_SetPrivateVlanBySessionId(session_id, up_port_list, dw_port_list))
                {
                    BACKDOOR_MGR_Print("\r\n Failed to add private vlan ports\n\r");
                }

                break;
            case 'c':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Printf("\n\r please enter group id [1-%d]: ",SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS);

                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                session_id = atoi(string);

                /* check session_id is exceed */
                if (session_id > SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS || session_id <=0)
                {
                    BACKDOOR_MGR_Print("\r\nThe session id is wrong\n\r");
                    break;
                }

                BACKDOOR_MGR_Print("\n\r If port is 0, clean all port with session id");
                BACKDOOR_MGR_Print("\n\r enter remove uplink port : ");
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                up_port = atoi(string);

                if(up_port >SYS_ADPT_TOTAL_NBR_OF_LPORT)
                {
                    BACKDOOR_MGR_Print("\r\nUplink Port Invalid\n\r");
                    break;
                }

                BACKDOOR_MGR_Print("\n\r enter remove downlink port : ");
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                dw_port = atoi(string);

                if(dw_port >SYS_ADPT_TOTAL_NBR_OF_LPORT)
                {
                    BACKDOOR_MGR_Print("\r\nDownlink Port Invalid\n\r");
                    break;
                }

                memset(up_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(dw_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

                SWCTRL_GetPrivateVlanBySessionId(session_id, up_port_list, dw_port_list);

                if (up_port!=0)
                {
                    memset(up_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                    up_port_list[ SWCTRL_BACKDOOR_BYTE_IN_BITMAP(up_port) ] |= SWCTRL_BACKDOOR_BIT_IN_BITMAP(up_port);
                }

                if (dw_port != 0)
                {
                    memset(dw_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                    dw_port_list[ SWCTRL_BACKDOOR_BYTE_IN_BITMAP(dw_port) ] |= SWCTRL_BACKDOOR_BIT_IN_BITMAP(dw_port);
                }

                if (FALSE == SWCTRL_DeletePrivateVlanPortlistBySessionId(session_id, up_port_list, dw_port_list))
                {
                    BACKDOOR_MGR_Print("\r\n Failed to remove private vlan ports\n\r");
                }

                break;

            case 'd':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Printf("\n\r please enter group id [1-%d]: ",SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS);

                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                session_id = atoi(string);

                /* check session_id is exceed */
                if (session_id > SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS || session_id <=0)
                {
                    BACKDOOR_MGR_Print("\r\nThe session id is wrong\n\r");
                    break;
                }

                if (FALSE == SWCTRL_DestroyPrivateVlanSession(session_id, TRUE, TRUE))
                {
                    BACKDOOR_MGR_Print("\r\n Failed to destroy private vlan session\n\r");
                }

                break;
            case 'e':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Print("The uplink ports blocking mode is :");
                if (SWCTRL_EnablePrivateVlanUplinkToUplinkBlockingMode()== TRUE)
                {
                    BACKDOOR_MGR_Print(" (Enabled)\n\r");
                }
                else
                {
                    BACKDOOR_MGR_Print(" (Disabled)\n\r");
                }
                break;

            case 'E':
                BACKDOOR_MGR_Print("\n\r");
                BACKDOOR_MGR_Print("The uplink ports blocking mode is :");
                if (SWCTRL_DisablePrivateVlanUplinkToUplinkBlockingMode() == TRUE)
                {
                    BACKDOOR_MGR_Print(" (Disabled)\n\r");
                }
                else
                {
                    BACKDOOR_MGR_Print(" (Enabled)\n\r");
                }
                break;
            case 's':
                BACKDOOR_MGR_Print("\n\r");
                SWCTRL_BACKDOOR_ShowPrivateVlanPortList();
                BACKDOOR_MGR_Print("\n\r");
                break;
            case 'q':
            case 'Q':
                BACKDOOR_MGR_Printf("%s", BACKDOOR_ClearScreen);
                eof = TRUE;
                break;
            default :
                ch = 0;
                BACKDOOR_MGR_Printf("%s", BACKDOOR_ClearScreen);
                BACKDOOR_MGR_Print("\n\r");
                break;
        }
    }

    return;
}/* End of SWCTRL_BACKDOOR_ForPrivateVlan()*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_ShowPrivateVlanPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will show private vlan port list
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void SWCTRL_BACKDOOR_ShowPrivateVlanPortList()
{
    UI32_T  s_uId=0, s_dId=0, nPort = 0;
    UI32_T  columns = 24;
    UI32_T  pvlan_status;
    UI32_T  up_status;
    UI8_T   up_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   dw_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    memset(up_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(dw_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    BACKDOOR_MGR_Print("\n\r");
    SWCTRL_GetPrivateVlanStatus(&pvlan_status);
    if (pvlan_status == VAL_privateVlanStatus_enabled)
    {
        BACKDOOR_MGR_Print("Private VLAN enabled\n\r");
    }
    else
    {
        BACKDOOR_MGR_Print("Private VLAN disabled\n\r");
    }

    SWCTRL_GetPrivateVlanUplinkToUplinkStatus(&up_status);
    if (up_status == 1) /* SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_BLOCKING */
    {
        BACKDOOR_MGR_Print("Private VLAN Uplink-ToUplink is blocking\n\r");
    }
    else
    {
        BACKDOOR_MGR_Print("Private VLAN Uplink-ToUplink is forwarding\n\r");
    }

    BACKDOOR_MGR_Print("========================================================\n\r");

    while (TRUE ==(SWCTRL_GetNextSessionFromPrivateVlanPortList(&s_uId, up_port_list, dw_port_list)))
    {
        BACKDOOR_MGR_Printf("\n\r(%2luU) ",s_uId);
        /* get uplink port list */
        for(nPort = 1; nPort<=SYS_ADPT_TOTAL_NBR_OF_LPORT; nPort++)
        {
            if ((up_port_list[(nPort-1)/8]<<((nPort-1)%8))&0x80 )
            {
                BACKDOOR_MGR_Print(" 1");
            }
            else
            {
                BACKDOOR_MGR_Print(" 0");
            }

            /* display in next line */
            if ((nPort%columns)== 0x0)
            {
                BACKDOOR_MGR_Print("\n\r      ");
            }
        }

        BACKDOOR_MGR_Printf("\n\r(%2luD) ",s_dId);
        /* get downlink port list */
        for(nPort = 1; nPort<=SYS_ADPT_TOTAL_NBR_OF_LPORT; nPort++)
        {
            if ((dw_port_list[(nPort-1)/8]<<((nPort-1)%8))&0x80 )
            {
                BACKDOOR_MGR_Print(" 1");
            }
            else
            {
                BACKDOOR_MGR_Print(" 0");
            }

            /* display in next line */
            if ((nPort%columns)== 0x0)
            {
                BACKDOOR_MGR_Print("\n\r      ");
            }
        }

        BACKDOOR_MGR_Print("\n\r");
        BACKDOOR_MGR_Print("========================================================\n\r");
    }
} /* End of SWCTRL_BACKDOOR_ShowPrivateVlanPortList() */

#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

static void SWCTRL_BACKDOOR_Dormant_BackdoorMain(void)
{
    UI32_T ifindex = 1;
    UI32_T level = 0;
    UI32_T dormant_status, dormant_active;
    BOOL_T is_valid;
    int ch;

    while (1)
    {
        if (!(is_valid = SWCTRL_GetPortOperDormantStatus(ifindex, level, &dormant_status, &dormant_active)))
        {
            dormant_status = 0;
            dormant_active = 0;
        }

        BACKDOOR_MGR_Print("\r\n 0. exit");
        BACKDOOR_MGR_Printf("\r\n i. ifindex : %lu", ifindex);
        BACKDOOR_MGR_Printf("\r\n l. level   : %lu", level);
        if (is_valid)
        {
            BACKDOOR_MGR_Printf("\r\n s. status  : %d", !!(dormant_status & BIT_VALUE(level)));
            BACKDOOR_MGR_Printf("\r\n a. active  : %d", !!(dormant_active & BIT_VALUE(level)));
        }
        BACKDOOR_MGR_Print("\r\n select = ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\r\n", (isprint(ch) ? ch : '?'));

        switch (ch)
        {
            case '0':
                return;
            case 'i':
                ifindex = BACKDOOR_LIB_RequestUI32("ifindex", ifindex);
                break;
            case 'l':
                level = BACKDOOR_LIB_RequestUI32("level", level);
                break;
            case 's':
                if (is_valid)
                {
                    BOOL_T enable = (dormant_status & BIT_VALUE(level));
                    BOOL_T stealth = FALSE;
                    BOOL_T ret;

                    enable = BACKDOOR_LIB_RequestBool("enable", enable);
                    stealth = BACKDOOR_LIB_RequestBool("stealth", stealth);

                    ret = SWCTRL_SetPortOperDormantStatus(ifindex, level, enable, stealth);

                    BACKDOOR_MGR_Printf("ret: %d\n", ret);
                }
                break;
            case 'a':
                if (is_valid)
                {
                    UI32_T event = (dormant_active & BIT_VALUE(level)) ? SWCTRL_OPER_DORMANT_EV_ENTER : SWCTRL_OPER_DORMANT_EV_LEAVE;
                    BOOL_T ret;

                    event = BACKDOOR_LIB_RequestUI32("event", event);

                    ret = SWCTRL_TriggerPortOperDormantEvent(ifindex, level, event);

                    BACKDOOR_MGR_Printf("ret: %d\n", ret);
                }
                break;
        }
    }
}

