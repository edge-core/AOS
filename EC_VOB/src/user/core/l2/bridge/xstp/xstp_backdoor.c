/*-------------------------------------------------------------------------
 * Module Name  :   xstp_backdoor.c
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for the XSTP
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    05/30/2001 - Allen Cheng, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */


/************************************
 ***   INCLUDE FILE DECLARATIONS  ***
 ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "backdoor_lib.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "swctrl.h"
#include "xstp_backdoor.h"
#include "xstp_mgr.h"
#include "xstp_om.h"
#include "xstp_om_private.h"
#include "xstp_type.h"


#define XSTP_BACKDOOR_CMD_MAXLEN    16
#define KEY_ESC                     27
#define KEY_BACKSPACE               8
#define KEY_EOS                     0


typedef struct
{
    char   *cmd_str;
    char   *cmd_title;
    char   *cmd_description;
    void    (*cmd_function)(char *cmd_buf);
} XSTP_BACKDOOR_CommandStruct_T;

typedef struct
{
    UI32_T  flag;
    char    *flag_string;
} XSTP_BACKDOOR_DebugStruct_T;

static  void    XSTP_BACKDOOR_CliDebugMenu(char *cmd_buf);
static  void    XSTP_BACKDOOR_Engine(void);
static  void    XSTP_BACKDOOR_ParseCmd(char *cmd_buf, UI8_T ch);
static  void    XSTP_BACKDOOR_ExecuteCmd(char *cmd_buf);
static  void    XSTP_BACKDOOR_TerminateCmd(char *cmd_buf);

static  void    XSTP_BACKDOOR_PrintMenu(char *cmd_buf);
static  void    XSTP_BACKDOOR_ShowDebugFlag(char *cmd_buf);
static  void    XSTP_BACKDOOR_SetDebugFlag(char *cmd_buf);
static  void    XSTP_BACKDOOR_SetSMDebugFlag(char *cmd_buf);
static  void    XSTP_BACKDOOR_SetFlagTable(char *cmd_buf, XSTP_BACKDOOR_DebugStruct_T *table);
static  void    XSTP_BACKDOOR_SetDebugPort(char *cmd_buf);
static  void    XSTP_BACKDOOR_SetDebugMst(char *cmd_buf);
static  void    XSTP_BACKDOOR_ShowSMVar(char *cmd_buf);

static  void    XSTP_BACKDOOR_BridgeInfo(char *cmd_buf);
static  void    XSTP_BACKDOOR_PortInfo(char *cmd_buf);
static  void    XSTP_BACKDOOR_TestStrToBuf(char *cmd_buf);

static  void    XSTP_BACKDOOR_NotAvailable(char *cmd_buf);
static  BOOL_T  XSTP_BACKDOOR_StrToVal(char *str, UI32_T *value);

static  XSTP_BACKDOOR_CommandStruct_T XSTP_BACKDOOR_CommandTable[] =
{
    /*  cmd_str,    cmd_title,              cmd_descritption,                           cmd_function            */
    /*  ------------------------------------------------------------------------------------------------------  */
    {   "",         "File System",          "Engineering Mode Main Menu",               XSTP_BACKDOOR_PrintMenu     },
    {   "d",        "Debug Flag",           "To set the debug flag",                    XSTP_BACKDOOR_PrintMenu     },
    {   "dd",       "Set Debug Flag",       "To set the debug flag",                    XSTP_BACKDOOR_SetDebugFlag  },
    {   "ds",       "Show Debug Flag",      "To show the debug flag",                   XSTP_BACKDOOR_ShowDebugFlag },
    {   "dm",       "Debug State Machine",  "To set for state machine debug",           XSTP_BACKDOOR_PrintMenu     },
    {   "dmd",      "Set State Debug Flag", "To set the state debug flag",              XSTP_BACKDOOR_SetSMDebugFlag},
    {   "dmp",      "Set State Debug Port", "To set the state debug by port",           XSTP_BACKDOOR_SetDebugPort  },
    {   "dmm",      "Set State Debug MST",  "To set the state debug by Mstid",          XSTP_BACKDOOR_SetDebugMst   },
    {   "dmv",      "Show Variables",       "To show the state machine variables",      XSTP_BACKDOOR_ShowSMVar     },
    {   "i",        "XSTP Information",     "To show the XSTP information",             XSTP_BACKDOOR_PrintMenu     },
    {   "ib",       "Bridge Information",   "To show the bridge information",           XSTP_BACKDOOR_BridgeInfo    },
    {   "ip",       "Port Information",     "To show the port information",             XSTP_BACKDOOR_PortInfo      },
    {   "is",       "System Information",   "To show system information",               XSTP_BACKDOOR_NotAvailable  },
    {   "t",        "Test",                 "To test the functions",                    XSTP_BACKDOOR_PrintMenu     },
    {   "ts",       "Test str_to_buf",      "To test the conversion from hex-string",   XSTP_BACKDOOR_TestStrToBuf  },
    {   "c",        "Set CLI Command",      "To set the cli command",                   XSTP_BACKDOOR_CliDebugMenu  },
    {   "\x7F",     "End of Table",         "End of the Command Table",                 XSTP_BACKDOOR_PrintMenu     }
};

static  XSTP_BACKDOOR_DebugStruct_T XSTP_BACKDOOR_DebugFlagTable[] =
{
    {   XSTP_TYPE_DEBUG_FLAG_NONE,      "XSTP_TYPE_DEBUG_FLAG_NONE"     },
    {   XSTP_TYPE_DEBUG_FLAG_TXTCN,     "XSTP_TYPE_DEBUG_FLAG_TXTCN"    },
    {   XSTP_TYPE_DEBUG_FLAG_TXCFG,     "XSTP_TYPE_DEBUG_FLAG_TXCFG"    },
    {   XSTP_TYPE_DEBUG_FLAG_TXRSTP,    "XSTP_TYPE_DEBUG_FLAG_TXRSTP"   },
    {   XSTP_TYPE_DEBUG_FLAG_RXTCN,     "XSTP_TYPE_DEBUG_FLAG_RXTCN"    },
    {   XSTP_TYPE_DEBUG_FLAG_RXCFG,     "XSTP_TYPE_DEBUG_FLAG_RXCFG"    },
    {   XSTP_TYPE_DEBUG_FLAG_RXRSTP,    "XSTP_TYPE_DEBUG_FLAG_RXRSTP"   },
    {   XSTP_TYPE_DEBUG_FLAG_DBGMSG,    "XSTP_TYPE_DEBUG_FLAG_DBGMSG"   },
    {   XSTP_TYPE_DEBUG_FLAG_ERRMSG,    "XSTP_TYPE_DEBUG_FLAG_ERRMSG"   },
    {   XSTP_TYPE_DEBUG_FLAG_ALL,       "XSTP_TYPE_DEBUG_FLAG_ALL"      }
};

static  XSTP_BACKDOOR_DebugStruct_T XSTP_BACKDOOR_SMDebugFlagTable[] =
{
    {   XSTP_TYPE_DEBUG_FLAG_NONE,      "XSTP_TYPE_DEBUG_FLAG_NONE"     },
    {   XSTP_TYPE_DEBUG_FLAG_DBGSM,     "XSTP_TYPE_DEBUG_FLAG_DBGSM"    },
    {   XSTP_TYPE_DEBUG_FLAG_ALL,       "XSTP_TYPE_DEBUG_FLAG_ALL"      }
};

static  char   XSTP_BACKDOOR_ClearScreen[] =
{ KEY_ESC, '[', '2', 'J', KEY_EOS };

static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XSTP_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void XSTP_BACKDOOR_Main(void)
{
    tg_handle = L2_L4_PROC_COMM_GetStaGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    XSTP_BACKDOOR_Engine();

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
} /* End of XSTP_BACKDOOR_Main */

static  void    XSTP_BACKDOOR_Engine(void)
{
    char    cmd_buf[XSTP_BACKDOOR_CMD_MAXLEN+1];
    BOOL_T  engine_continue;
    char    ch;

    cmd_buf[0]      = 0;
    engine_continue = TRUE;
    while (engine_continue)
    {
        XSTP_BACKDOOR_ExecuteCmd(cmd_buf);
        ch = BACKDOOR_MGR_GetChar();

        switch (ch)
        {
            case KEY_ESC:       /* Go to the main menu */
                cmd_buf[0]      = '\0';
                break;

            case KEY_BACKSPACE: /* Go to the up level menu */
                if (cmd_buf[0] != '\0')
                    cmd_buf[strlen(cmd_buf)-1]    = '\0';
                break;

            case 'q':           /* Quit the engineering mode */
            case 'Q':
                engine_continue = FALSE;
                break;

            default:
                BACKDOOR_MGR_Printf("%c", ch);
                XSTP_BACKDOOR_ParseCmd(cmd_buf, ch);
                break;
        }
    }

    return;
} /* End of XSTP_BACKDOOR_Engine */

/*
 * If { cmd_buf[] | {ch} } exists in the XSTP_BACKDOOR_CommandTable
 * then cmd_buf[] |= ch;
 */
static  void    XSTP_BACKDOOR_ParseCmd(char *cmd_buf, UI8_T ch)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= XSTP_BACKDOOR_CMD_MAXLEN)
        return;
    cmd_buf[cmd_length]     = ch;
    cmd_buf[cmd_length+1]   = 0;

    while ((!cmd_found) && (XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F'))
    {
        if (strcmp(cmd_buf, XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
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

static  void    XSTP_BACKDOOR_ExecuteCmd(char *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= XSTP_BACKDOOR_CMD_MAXLEN)
        return;

    while ((!cmd_found) && (XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F'))
    {
        if (strcmp(cmd_buf, XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (cmd_found)
    {
        BACKDOOR_MGR_Printf("%s", XSTP_BACKDOOR_ClearScreen);
        /* Print Title and Description */
        BACKDOOR_MGR_Printf("\r\n%s    %s",
                             XSTP_BACKDOOR_CommandTable[cmd_index].cmd_title,
                             XSTP_BACKDOOR_CommandTable[cmd_index].cmd_description);
        BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------");
        XSTP_BACKDOOR_CommandTable[cmd_index].cmd_function(cmd_buf);
    }

    return;
}

static  void    XSTP_BACKDOOR_TerminateCmd(char *cmd_buf)
{
    /* Terminating */
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n[Esc]       : Main Menu");
    BACKDOOR_MGR_Printf("\r\n[Back_Space]: Up Menu");
    BACKDOOR_MGR_Printf("\r\nQ/q         : To quit");
    BACKDOOR_MGR_Printf("\r\nPress any key to continue ...");

    return;
} /* End of XSTP_BACKDOOR_TerminateCmd */

/* ------------------------------------------------------------------------
 * Command Function
 */
static  void    XSTP_BACKDOOR_PrintMenu(char *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;

    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= XSTP_BACKDOOR_CMD_MAXLEN)
        return;

    BACKDOOR_MGR_Printf("\r\n");
    while ( XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F')
    {
        if (    ( (UI8_T)(strlen(XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str) ) == (cmd_length + 1) )
             && ( strncmp(XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str, cmd_buf, cmd_length) == 0)
           )
        {
            BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",
                                 XSTP_BACKDOOR_CommandTable[cmd_index].cmd_str[cmd_length],
                                 XSTP_BACKDOOR_CommandTable[cmd_index].cmd_title,
                                 XSTP_BACKDOOR_CommandTable[cmd_index].cmd_description
                                );
        }
        cmd_index++;
    }

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",'Q', "Quit", "To exit the XSTP engineering mode");
    BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",'q', "Quit", "To exit the XSTP engineering mode");
    BACKDOOR_MGR_Printf("\r\n[Esc]       : Main Menu");
    BACKDOOR_MGR_Printf("\r\n[Back_Space]: Up Menu");
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\nSelect : ");

    return;
} /* XSTP_BACKDOOR_PrintMenu */

static  void    XSTP_BACKDOOR_ShowDebugFlag(char *cmd_buf)
{
    UI32_T  flag;

    XSTP_OM_GetDebugFlag(&flag);
    BACKDOOR_MGR_Printf("\r\nDebugFlag = %08lx", flag);

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_ShowDebugFlag */

static  void    XSTP_BACKDOOR_SetDebugFlag(char *cmd_buf)
{
    XSTP_BACKDOOR_SetFlagTable(cmd_buf, XSTP_BACKDOOR_DebugFlagTable);
    return;
} /* End of XSTP_BACKDOOR_SetDebugFlag */

static  void    XSTP_BACKDOOR_SetSMDebugFlag(char *cmd_buf)
{
    XSTP_BACKDOOR_SetFlagTable(cmd_buf, XSTP_BACKDOOR_SMDebugFlagTable);
    return;
} /* End of XSTP_BACKDOOR_SetSMDebugFlag */

static  void    XSTP_BACKDOOR_SetFlagTable(char *cmd_buf, XSTP_BACKDOOR_DebugStruct_T *table)
{
    UI8_T   i, select, ch;
    UI32_T  flag, set_flag;
    char    *status;
    BOOL_T  set_continue;

    set_continue = TRUE;

    while (set_continue)
    {
        BACKDOOR_MGR_Printf("%s", XSTP_BACKDOOR_ClearScreen);
        BACKDOOR_MGR_Printf("\r\nItem  %-32s   %s","Flag", "Status");
        BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------");
        i = 1;

        XSTP_OM_GetDebugFlag(&flag);

        while (table[i].flag < (UI32_T)XSTP_TYPE_DEBUG_FLAG_ALL)
        {
            /* Print the menu and the current status */
            if ((flag & table[i].flag) != 0)
            {
                status = "ON";
            }
            else
            {
                status = "OFF";
            }
            BACKDOOR_MGR_Printf("\r\n%2d -- %-32s : %-3s", i, table[i].flag_string, status);
            i++;
        }
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nSelect (0 = Reset all flags; q = quit) to turn ON/OFF : ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", ch);
        if (ch != 'q')
        {
            select = (UI8_T)(ch - '0');

            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if (select == 0)
            {
                XSTP_OM_SetDebugFlag(XSTP_TYPE_DEBUG_FLAG_NONE);
            }
            else if ( (select > 0) && (select <= 9) )
            {
                set_flag = (UI32_T)table[select].flag;
                XSTP_OM_SetDebugFlag(flag ^ set_flag);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
        }
        else
        {
            set_continue = FALSE;
        }
    } /* End of while */

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_SetDebugFlag */

static  void    XSTP_BACKDOOR_SetDebugPort(char *cmd_buf)
{
    BOOL_T  set_continue;
    char    buf[8];
    UI32_T  lport;
    UI16_T  i, list_begin, list_end;
    UI8_T   page_num;

    set_continue    = TRUE;
    page_num        = 0;

    while (set_continue)
    {
        BACKDOOR_MGR_Printf("%s", XSTP_BACKDOOR_ClearScreen);
        BACKDOOR_MGR_Printf("\r\nItem  %-32s   %s","Flag", "Status");
        BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------");

        list_begin  = page_num*128;
        list_end    = list_begin +127;
        for (i = list_begin; i <= list_end; i++)
        {
            if (i%8 == 0)
            {
                BACKDOOR_MGR_Printf("\r\n");
            }
            BACKDOOR_MGR_Printf("[%03d:", i+1);
            if (XSTP_OM_StateDebugPort( (UI32_T)i+1) )
            {
                BACKDOOR_MGR_Printf(" ON]");
            }
            else
            {
                BACKDOOR_MGR_Printf("OFF]");
            }
        }
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nInput Lport number (1 - 512; 0 = change page; or 'q' = quit) to turn ON/OFF : ");

        BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
        if (buf[0] == 'q')
        {
            set_continue = FALSE;
        }
        else
        if (buf[0] == 0x00)
        {
            page_num++;
            page_num    %= 4;
        }
        else
        {
            if (XSTP_BACKDOOR_StrToVal(buf, &lport) )
            {
                if (lport == 0)
                {
                    page_num++;
                    page_num    %= 4;
                }
                else
                if (lport <= 512)
                {
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    XSTP_OM_StateDebugPortSwitch(lport);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                else
                    BACKDOOR_MGR_Printf("\r\nInput Error!!!");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\nInput Error!!!");
            }
        }
    } /* End of while */

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_SetDebugPort */


static  void    XSTP_BACKDOOR_SetDebugMst(char *cmd_buf)
{
    BOOL_T  set_continue;
    char    buf[8];
    UI32_T  mstid;
    UI16_T  i, list_begin, list_end;

    set_continue    = TRUE;

    while (set_continue)
    {
        BACKDOOR_MGR_Printf("%s", XSTP_BACKDOOR_ClearScreen);
        BACKDOOR_MGR_Printf("\r\nItem  %-32s   %s","Flag", "Status");
        BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------");

        list_begin  = 0;
        list_end    = 64;
        for (i = list_begin; i <= list_end; i++)
        {
            if (i%8 == 0)
            {
                BACKDOOR_MGR_Printf("\r\n");
            }

            BACKDOOR_MGR_Printf("[%03d:", i);

            if (XSTP_OM_StateDebugMst( (UI32_T)i) )
            {
                BACKDOOR_MGR_Printf(" ON]");
            }
            else
            {
                BACKDOOR_MGR_Printf("OFF]");
        }
        }
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nInput Instance Id (0 - 64; 0 = CIST; or 'q' = quit) to turn ON/OFF : ");

        BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
        if (buf[0] == 'q')
        {
            set_continue = FALSE;
        }
        else
        if (buf[0] == '\0')
        {
            /* Enter key pressed */
            /* Do nothing */
        }
        else
        {
            if (XSTP_BACKDOOR_StrToVal(buf, &mstid) )
            {
                if (mstid < 65)
                {
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    XSTP_OM_StateDebugMstSwitch(mstid);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                else
                    BACKDOOR_MGR_Printf("\r\nInput Error!!!");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\nInput Error!!!");
            }
        }
    } /* End of while */

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_SetDebugMst */

static  void    XSTP_BACKDOOR_ShowSMVar(char *cmd_buf)
{
    UI8_T   ch;
    UI32_T  xstid;
    BOOL_T  print_continue;

    print_continue = TRUE;
    while (print_continue)
    {
        xstid   = 0;
        do
        {
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

            BACKDOOR_MGR_Printf("\r\n");
            if ( XSTP_OM_StateDebugMst(xstid) )
            {
                XSTP_OM_InstanceData_T  *om_ptr;
                om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
                if (om_ptr->instance_exist)
                {
                    UI32_T  lport;

                    /* Show bridge variables */
                    BACKDOOR_MGR_Printf("\r\n");
                    BACKDOOR_MGR_Printf("\r\n======================================");
                    BACKDOOR_MGR_Printf("\r\n Bridge variables of instance %ld", xstid);
                    BACKDOOR_MGR_Printf("\r\n static_bridge_priority=%d", om_ptr->bridge_info.static_bridge_priority);
                    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                    BACKDOOR_MGR_Printf("\r\n begin=%d begin=%d tx_hold_count=%d", om_ptr->bridge_info.common->begin, om_ptr->bridge_info.common->tx_hold_count, om_ptr->bridge_info.common->migrate_time);

                    BACKDOOR_MGR_Printf("\r\n sms_prs=%d", om_ptr->bridge_info.sms_prs);
                    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

                    lport   = 0;
                    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
                    {
                        if ( XSTP_OM_StateDebugPort(lport) )
                        {
                            /* Show port variables */
                            XSTP_OM_PortVar_T       *pom_ptr;
                            pom_ptr = &(om_ptr->port_info[lport-1]);
                            if (pom_ptr->is_member)
                            {
                                BACKDOOR_MGR_Printf("\r\n");
                                BACKDOOR_MGR_Printf("\r\n======================================");
                                BACKDOOR_MGR_Printf("\r\nVariables of Port %ld instance %ld", lport, xstid);

                                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                                BACKDOOR_MGR_Printf("\r\n=== Common ===");
                                BACKDOOR_MGR_Printf("\r\n mdelay_while=%d hello_when=%d external_port_path_cost=%ld", pom_ptr->common->mdelay_while, pom_ptr->common->hello_when, pom_ptr->common->external_port_path_cost);
                                BACKDOOR_MGR_Printf("\r\n mcheck=%d tick=%d tx_count=%d", pom_ptr->common->mcheck, pom_ptr->common->tick, pom_ptr->common->tx_count);
                                BACKDOOR_MGR_Printf("\r\n oper_edge=%d port_enabled=%d info_internal=%d", pom_ptr->common->oper_edge, pom_ptr->common->port_enabled, pom_ptr->common->info_internal);
                                BACKDOOR_MGR_Printf("\r\n new_info_cist=%d new_info_msti=%d rcvd_internal=%d", pom_ptr->common->new_info_cist, pom_ptr->common->new_info_msti, pom_ptr->common->rcvd_internal);
                                BACKDOOR_MGR_Printf("\r\n init_pm=%d rcvd_rstp=%d rcvd_stp=%d", pom_ptr->common->init_pm, pom_ptr->common->rcvd_rstp, pom_ptr->common->rcvd_stp);
                                BACKDOOR_MGR_Printf("\r\n rcvd_tc_ack=%d rcvd_tcn=%d send_rstp=%d", pom_ptr->common->rcvd_tc_ack, pom_ptr->common->rcvd_tcn, pom_ptr->common->send_rstp);
                                BACKDOOR_MGR_Printf("\r\n tc_ack=%d rcvd_bpdu=%d admin_edge=%d", pom_ptr->common->tc_ack, pom_ptr->common->rcvd_bpdu, pom_ptr->common->admin_edge);

                                BACKDOOR_MGR_Printf("\r\n=== Per Instance ===");
                                BACKDOOR_MGR_Printf("\r\n static_internal_path_cost=%d internal_port_path_cost=%ld", pom_ptr->static_internal_path_cost, pom_ptr->internal_port_path_cost);
                                BACKDOOR_MGR_Printf("\r\n sms_pti=%d sms_prx=%d sms_pim=%d sms_prt=%d", pom_ptr->sms_pti, pom_ptr->sms_prx, pom_ptr->sms_pim, pom_ptr->sms_prt);
                                BACKDOOR_MGR_Printf("\r\n sms_pst=%d sms_tcm=%d sms_ppm=%d sms_ptx=%d", pom_ptr->sms_pst, pom_ptr->sms_tcm, pom_ptr->sms_ppm, pom_ptr->sms_ptx);
                                BACKDOOR_MGR_Printf("\r\n fd_while=%d rr_while=%d rb_while=%d tc_while=%d rcvd_info_while=%d", pom_ptr->fd_while, pom_ptr->rr_while, pom_ptr->rb_while, pom_ptr->tc_while, pom_ptr->rcvd_info_while);
                                BACKDOOR_MGR_Printf("\r\n forward=%d forwarding=%d info_is=%d", pom_ptr->forward, pom_ptr->forwarding, pom_ptr->info_is);
                                BACKDOOR_MGR_Printf("\r\n learn=%d learning=%d proposed=%d proposing=%d", pom_ptr->learn, pom_ptr->learning, pom_ptr->proposed, pom_ptr->proposing);
                                BACKDOOR_MGR_Printf("\r\n rcvd_tc=%d re_root=%d reselect=%d selected=%d", pom_ptr->rcvd_tc, pom_ptr->re_root, pom_ptr->reselect, pom_ptr->selected);
                                BACKDOOR_MGR_Printf("\r\n tc_prop=%d updt_info=%d agreed=%d rcvd_info=%d", pom_ptr->tc_prop, pom_ptr->updt_info, pom_ptr->agreed, pom_ptr->rcvd_info);
                                BACKDOOR_MGR_Printf("\r\n role=%d selected_role=%d sync=%d synced=%d", pom_ptr->role, pom_ptr->selected_role, pom_ptr->sync, pom_ptr->synced);
                                BACKDOOR_MGR_Printf("\r\n msti_master=%d msti_mastered=%d changed_master=%d", pom_ptr->msti_master, pom_ptr->msti_mastered, pom_ptr->changed_master);
                                BACKDOOR_MGR_Printf("\r\n agree=%d rcvd_msg=%d", pom_ptr->agree, pom_ptr->rcvd_msg);
                                BACKDOOR_MGR_Printf("\r\n static_port_priority=%d", pom_ptr->static_port_priority);
                                #endif /* XSTP_TYPE_PROTOCOL_MSTP */

                                #ifdef  XSTP_TYPE_PROTOCOL_RSTP
                                BACKDOOR_MGR_Printf("\r\n static_path_cost=%d port_path_cost=%ld", pom_ptr->static_path_cost, pom_ptr->port_path_cost);
                                #endif /* XSTP_TYPE_PROTOCOL_RSTP */
                                BACKDOOR_MGR_Printf("\r\n static_port_priority=%d", pom_ptr->static_port_priority);
                            }
                            else
                            {
                                BACKDOOR_MGR_Printf("\r\nPort %ld is not the member of instance %ld", lport, xstid);
                            } /* End of if (port_is_member) */
                        } /* End of if (lport_debug_flag_enabled) */
                    } /* End of while (all_logical_port) */
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nInstance %ld does not exist.", xstid);
                } /* End of if (instance_exist) */
            } /* End of if (mst_debug_flag_enabled) */

            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

            BACKDOOR_MGR_Printf("\r\nPress any key to continue (q = quit)...");
            ch = BACKDOOR_MGR_GetChar();
            if (ch == 'q')
                print_continue = FALSE;
        } while ( print_continue && XSTP_OM_GetNextExistedInstance(&xstid) );
    } /* while (print_continue) */

    /* Terminating */
    BACKDOOR_MGR_Printf("\r\n");
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_ShowSMVar */

static  void    XSTP_BACKDOOR_BridgeInfo(char *cmd_buf)
{
#if 0
    UI8_T   i;
    XSTP_MGR_Dot1dStpEntry_T stp_entry;

    printf("\r\n");
    if (XSTP_MGR_GetDot1dStpEntry(&stp_entry) )
    {
        printf("\r\n%-24s: %12d", "Protocol Specification", stp_entry.dot1d_stp_protocol_specification);
        printf("\r\n%-24s: %12d", "Bridge Priority",        stp_entry.dot1d_stp_priority);
        printf("\r\n%-24s: %12d", "Time since TC",          stp_entry.dot1d_stp_time_since_topology_change);
        printf("\r\n%-24s: %12d", "Topology Changes",       stp_entry.dot1d_stp_top_changes);
        printf("\r\n%-24s: %4d",  "Designated Root",        stp_entry.dot1d_stp_designated_root.priority);
                for (i=0; i<6; i++) printf(":%02x",        stp_entry.dot1d_stp_designated_root.addr[i]);
        printf("\r\n%-24s: %12d", "Root Cost",              stp_entry.dot1d_stp_root_cost);
        printf("\r\n%-24s: %12d", "Root Port",              stp_entry.dot1d_stp_root_port);
        printf("\r\n%-24s: %12d", "Max Age",                stp_entry.dot1d_stp_max_age);
        printf("\r\n%-24s: %12d", "Hello Time",             stp_entry.dot1d_stp_hello_time);
        printf("\r\n%-24s: %12d", "Hold Time",              stp_entry.dot1d_stp_hold_time);
        printf("\r\n%-24s: %12d", "Forward Delay",          stp_entry.dot1d_stp_forward_delay);
        printf("\r\n%-24s: %12d", "Bridge Max Age",         stp_entry.dot1d_stp_bridge_max_age);
        printf("\r\n%-24s: %12d", "Bridge Hello Time",      stp_entry.dot1d_stp_bridge_hello_time);
        printf("\r\n%-24s: %12d", "Bridge Forward Delay",   stp_entry.dot1d_stp_bridge_forward_delay);
    }
#endif

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_ResetRxCfgFlag */

static  void    XSTP_BACKDOOR_PortInfo(char *cmd_buf)
{
#if 0
    UI8_T   i, j, ch;
    BOOL_T  print_continue;

    XSTP_MGR_Dot1dStpPortEntry_T stp_port_entry;

    stp_port_entry.dot1d_stp_port   = 0;
    j = 0;
    print_continue = TRUE;
    while ( (print_continue) && (XSTP_MGR_GetNextDot1dStpPortEntry(&stp_port_entry) ) )
    {
        printf("\r\n");
        printf("\r\nPort%d::[Prty=%d][State=%d][Adm=%s][PathCost=%d][DsCost=%d][FwTran=%d][DsPort=%d:%d]",
                stp_port_entry.dot1d_stp_port,
                stp_port_entry.dot1d_stp_port_priority,
                stp_port_entry.dot1d_stp_port_state,
                (stp_port_entry.dot1d_stp_port_enable == VAL_dot1dStpPortEnable_enabled)?"ON":"OFF",
                stp_port_entry.dot1d_stp_port_path_cost,
                stp_port_entry.dot1d_stp_port_designated_cost,
                stp_port_entry.dot1d_stp_port_forward_transitions,
                stp_port_entry.dot1d_stp_port_designated_port.priority,
                stp_port_entry.dot1d_stp_port_designated_port.port_no
              );
        printf("\r\n[DesignatedRoot=%d",                stp_port_entry.dot1d_stp_port_designated_root.priority);
                for (i=0; i<6; i++) printf(":%02x",     stp_port_entry.dot1d_stp_port_designated_root.addr[i]);
                printf("]");
        printf("\r\n[DesignatedBridge=%d",              stp_port_entry.dot1d_stp_port_designated_bridge.priority);
                for (i=0; i<6; i++) printf(":%02x",     stp_port_entry.dot1d_stp_port_designated_bridge.addr[i]);
                printf("]");
        j++;
        if ( (j%5) == 0 )
        {
            printf("\r\nPress any key to continue (q = quit)...");
            ch = BACKDOOR_MGR_GetChar();
            if (ch == 'q')
                print_continue = FALSE;
        }
    }
#endif

    /* Terminating */
    BACKDOOR_MGR_Printf("\r\n");
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
}

static  void    XSTP_BACKDOOR_TestStrToBuf(char *cmd_buf)
{
    char    hex_str[65];
    UI8_T   val_buf[64];
    UI8_T   index;

    BACKDOOR_MGR_Printf("\r\nInput the hexadecimal string ( max 64 characters): ");
    BACKDOOR_MGR_RequestKeyIn(hex_str, sizeof(hex_str)-1);

    if (XSTP_OM_LongHexStrToVal(hex_str, val_buf, sizeof(val_buf)) )
    {
        BACKDOOR_MGR_Printf("\r\nHexadecimal string is %s",hex_str);
        BACKDOOR_MGR_Printf("\r\nHexadecimal value  is ");
        for (index = 0; index < sizeof(val_buf); index++)
        {
            BACKDOOR_MGR_Printf(" %02x", val_buf[sizeof(val_buf)-1-index]);
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nError : Hexadecimal string %s is invalid!", hex_str);
    }

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_TestStrToBuf */

static  void    XSTP_BACKDOOR_NotAvailable(char *cmd_buf)
{
    BACKDOOR_MGR_Printf("\r\nSorry!! Not available now ...");

    /* Terminating */
    XSTP_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of XSTP_BACKDOOR_NotAvailable */

/*****************************************************************************/
/****************   Debug User Menu For XSTP   *******************************/
/*****************************************************************************/

static void XSTP_BACKDOOR_CliDebugMenu(char *cmd_buf)
{
    UI32_T  select_value = 0;
    UI32_T  set_value = 0;
    UI32_T  lport = 0;
    UI32_T  mstid = 0;
    UI32_T  vlan  =0;
    int     i;
    char    name[33] = {'\0'};
    UI8_T   vlan_list[128];
    UI32_T  status = 0;
    XSTP_MGR_Dot1dStpEntry_T            dot1d_entry;
    XSTP_MGR_Dot1dStpPortEntry_T        port_entry;
    XSTP_MGR_Dot1dStpExtPortEntry_T     ext_port_entry;
    XSTP_MGR_MstpEntry_T                mstp_entry;
    XSTP_MGR_MstpInstanceEntry_T        instance_entry;
    UI32_T                              state;
    XSTP_OM_InstanceData_T              *om_ptr;
    BOOL_T                              flood_bpdu;

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n\t 1: Show XSTP Information for a instance");
        BACKDOOR_MGR_Printf("\r\n\t 2: Enable XSTP Status");
        BACKDOOR_MGR_Printf("\r\n\t 3: Disable XSTP Status");
        BACKDOOR_MGR_Printf("\r\n\t 4: Set version: [STP(0), RSTp(2), MSTP(3)]");
        BACKDOOR_MGR_Printf("\r\n\t 5: Set Forward Delay [%lu..%lu] (Time unit is 1/100 sec)", XSTP_TYPE_MIN_FORWARD_DELAY*XSTP_TYPE_TICK_TIME_UNIT, XSTP_TYPE_MAX_FORWARD_DELAY*XSTP_TYPE_TICK_TIME_UNIT);
        BACKDOOR_MGR_Printf("\r\n\t 6: Set Hello Time [%lu..%lu] (Time unit is 1/100 sec)", XSTP_TYPE_MIN_HELLO_TIME*XSTP_TYPE_TICK_TIME_UNIT, XSTP_TYPE_MAX_HELLO_TIME*XSTP_TYPE_TICK_TIME_UNIT);
        BACKDOOR_MGR_Printf("\r\n\t 7: Set Max Age [%lu..%lu] (Time unit is 1/100 sec)", XSTP_TYPE_MIN_MAXAGE*XSTP_TYPE_TICK_TIME_UNIT, XSTP_TYPE_MAX_MAXAGE*XSTP_TYPE_TICK_TIME_UNIT);
        BACKDOOR_MGR_Printf("\r\n\t 8: Set Path Cost Method: [short(1), long(2)]");
        BACKDOOR_MGR_Printf("\r\n\t 9: Set Transmission Limit [%lu..%lu]", XSTP_TYPE_MIN_TX_HOLD_COUNT, XSTP_TYPE_MAX_TX_HOLD_COUNT);
        BACKDOOR_MGR_Printf("\r\n\t10: Set Global Priority [%lu..%lu]", XSTP_TYPE_MIN_BRIDGE_PRIORITY, XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP);
        BACKDOOR_MGR_Printf("\r\n\t11: Show Information for a port");
        BACKDOOR_MGR_Printf("\r\n\t12: Set Port Path Cost");
        BACKDOOR_MGR_Printf("\r\n\t13: Set Port Priority [%lu..%lu]", XSTP_TYPE_MIN_PORT_PRIORITY, XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP);
        BACKDOOR_MGR_Printf("\r\n\t14: Set Port Link Type Mode [point_to_point(0), shared(1), auto(2) ]");
        BACKDOOR_MGR_Printf("\r\n\t15: Set Port Protocol Migration Mode [True(1), False(2)]");
        BACKDOOR_MGR_Printf("\r\n\t16: Set Port Edge_Port Mode  [True(1), False(2)]");
        BACKDOOR_MGR_Printf("\r\n\t17: Show Mstp Configuration Information");
        BACKDOOR_MGR_Printf("\r\n\t18: Set mstp_region_name");
        BACKDOOR_MGR_Printf("\r\n\t19: Set mstp_region_revision[0~65535]");
        BACKDOOR_MGR_Printf("\r\n\t20: Set mstp_max_hop_count [%lu..%lu]", XSTP_TYPE_MSTP_MIN_MAXHOP, XSTP_TYPE_MSTP_MAX_MAXHOP);
        BACKDOOR_MGR_Printf("\r\n\t21: Get Mstp Instance Vlan Mapped");
        BACKDOOR_MGR_Printf("\r\n\t22: Set Vlan to Mst [max_instance number : %lu]",XSTP_TYPE_MAX_INSTANCE_NUM-1);
        BACKDOOR_MGR_Printf("\r\n\t23: Set Priority for a instance [%lu..%lu]", XSTP_TYPE_MIN_BRIDGE_PRIORITY, XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP);
        BACKDOOR_MGR_Printf("\r\n\t24: Set Port Path Cost for a instance");
        BACKDOOR_MGR_Printf("\r\n\t25: Set Port Priority for a instance [%lu..%lu]", XSTP_TYPE_MIN_PORT_PRIORITY, XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP);
        BACKDOOR_MGR_Printf("\r\n\t26: Remove Vlan from Mst [max_instance number : %lu]",XSTP_TYPE_MAX_INSTANCE_NUM-1);
        BACKDOOR_MGR_Printf("\r\n\t27: Set vlan_list to Mst (region....[1K:1][2K:2][3K:3][4K:4])");
        BACKDOOR_MGR_Printf("\r\n\t28: Get Next Dot1dMstPortEntry [max_instance number : %lu]",XSTP_TYPE_MAX_INSTANCE_NUM-1);
        BACKDOOR_MGR_Printf("\r\n\t29: Get port state");
        BACKDOOR_MGR_Printf("\r\n\t30: Generate Configuration Digest [for root_mstp]");
        BACKDOOR_MGR_Printf("\r\n\t31: Set Port Spanning Tree Status [1:Enabled 2:Disabled]");
        BACKDOOR_MGR_Printf("\r\n\t32: Get Port Spanning Tree Status [1:Enabled 2:Disabled]");
        BACKDOOR_MGR_Printf("\r\n\t33: Flood Bpdu when STP is disabled");
        BACKDOOR_MGR_Printf("\r\n\t34: Discard Bpdu when STP is disabled");
        BACKDOOR_MGR_Printf("\r\n\t35: Get Bpdu Behavior when STP is disabled [0:Discard 1:Flood]");
        BACKDOOR_MGR_Printf("\r\n\t99: Exit");

        select_value = BACKDOOR_LIB_RequestUI32("\r\n Enter Selection", 99);
        BACKDOOR_MGR_Printf(" select value is %lu", select_value); /* Debug message */

        if (select_value == 1 || select_value == 21)
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid", 0);
        }

        if (select_value == 4 || select_value == 5 || select_value == 6 || select_value == 7 || select_value == 8 ||
            select_value == 9 ||select_value == 10 || select_value == 19 ||select_value == 20)
        {
            set_value = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the setting value", 0);
        }

        if (select_value == 18)
        {
            BACKDOOR_MGR_Printf("\r\nPlease enter the name: ");
            BACKDOOR_MGR_RequestKeyIn(name, sizeof(name)-1);
        }

        if (select_value == 12 || select_value == 13 || select_value == 14 || select_value == 15 || select_value == 16 || select_value == 31)
        {
            lport = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the lport number", 1);
            set_value = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the setting value", 0);
        }

        if (select_value == 22 || select_value == 26)
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid number", 0);
            vlan = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the vlan value", 1);
        }

        if (select_value == 27)
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid number", 0);
            set_value = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the range value", 0);
        }

        if (select_value == 11 || select_value == 28)
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid number", 0);
            lport = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the lport value", 1);
        }

        if (select_value == 23 )
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid number", 0);
            set_value = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the setting value", 0);
        }

        if (select_value == 24 || select_value == 25)
        {
            mstid = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the mstid number", 0);
            lport = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the lport number", 1);
            set_value = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the setting value", 0);
        }

        if (select_value == 29)
        {
            vlan = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the vlan number", 1);
            lport = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the lport value", 1);
        }

        if (select_value == 32)
        {
            lport = BACKDOOR_LIB_RequestUI32("\r\nPlease enter the lport value", 1);
        }

        /* Get execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
        switch(select_value)
        {
            case 1:
                if(XSTP_MGR_GetDot1dMstEntry(mstid, &dot1d_entry)==TRUE)
                {
                    XSTP_OM_GetRunningSystemSpanningTreeStatus(&status);
                    BACKDOOR_MGR_Printf("\r\n status(1: enable, 2: disable) [%ld]", status);
                    BACKDOOR_MGR_Printf("\r\n protocol_specification [%d]", dot1d_entry.dot1d_stp_protocol_specification);
                    BACKDOOR_MGR_Printf("\r\n max_age [%d]", dot1d_entry.dot1d_stp_max_age);
                    BACKDOOR_MGR_Printf("\r\n hello_time [%d]", dot1d_entry.dot1d_stp_hello_time);
                    BACKDOOR_MGR_Printf("\r\n forward_delay [%d]", dot1d_entry.dot1d_stp_forward_delay);
                    BACKDOOR_MGR_Printf("\r\n bridge_max_age [%d]", dot1d_entry.dot1d_stp_bridge_max_age);
                    BACKDOOR_MGR_Printf("\r\n bridge_hello_time [%d]", dot1d_entry.dot1d_stp_bridge_hello_time);
                    BACKDOOR_MGR_Printf("\r\n bridge_forward_delay [%d]", dot1d_entry.dot1d_stp_bridge_forward_delay);
                    BACKDOOR_MGR_Printf("\r\n version [%d]", dot1d_entry.dot1d_stp_version);
                    BACKDOOR_MGR_Printf("\r\n transmission limit [%d]", dot1d_entry.dot1d_stp_tx_hold_count);
                    BACKDOOR_MGR_Printf("\r\n path_cost_default [%d]", dot1d_entry.dot1d_stp_path_cost_default);
                    BACKDOOR_MGR_Printf("\r\n priority [%ld]", dot1d_entry.dot1d_stp_priority);
                }
                break;
            case 2:
                XSTP_MGR_SetSystemSpanningTreeStatus(XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED);
                break;

            case 3:
                XSTP_MGR_SetSystemSpanningTreeStatus(XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED);
                break;
            case 4:
                XSTP_MGR_SetSystemSpanningTreeVersion(set_value);
                break;
            case 5:
                XSTP_MGR_SetForwardDelay(set_value);
                break;
            case 6:
                XSTP_MGR_SetHelloTime(set_value);
                break;
            case 7:
                XSTP_MGR_SetMaxAge(set_value);
                break;
            case 8:
                XSTP_MGR_SetPathCostMethod(set_value);
                break;
            case 9:
                XSTP_MGR_SetTransmissionLimit(set_value);
                break;
            case 10:
                XSTP_MGR_SetSystemGlobalPriority(set_value);
                break;
            case 11:
                port_entry.dot1d_stp_port = (UI16_T)lport;
                XSTP_OM_GetDot1dMstPortEntry(mstid, &port_entry);
                XSTP_OM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port_entry);
                /* port patt_cost */
                BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: path_cost [%ld]", mstid, lport, port_entry.dot1d_stp_port_path_cost);
                /* port priority*/
                BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: priority [%d]", mstid, lport, port_entry.dot1d_stp_port_priority );
                /* port like_type */
                if (ext_port_entry.dot1d_stp_port_admin_point_to_point == XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: admin_link_type [point_to_point]", mstid, lport);
                }
                else if (ext_port_entry.dot1d_stp_port_admin_point_to_point == XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: admin_link_type [shared]", mstid, lport);
                }
                else if (ext_port_entry.dot1d_stp_port_admin_point_to_point == XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: admin_link_type [auto]", mstid, lport);
                }
                if (ext_port_entry.dot1d_stp_port_oper_point_to_point == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: op_link_type [point_to_point]", mstid, lport );
                }
                else if (ext_port_entry.dot1d_stp_port_oper_point_to_point == FALSE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: op_link_type [shared]", mstid, lport );
                }
                /* port protocol_migration mode */
                if (ext_port_entry.dot1d_stp_port_protocol_migration == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: protocol_migration [TRUE]", mstid, lport);
                }
                else if (ext_port_entry.dot1d_stp_port_protocol_migration == FALSE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: protocol_migration [FALSE]", mstid, lport);
                }
                /* port edge_port mode */
                if (ext_port_entry.dot1d_stp_port_admin_edge_port == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: admin_edge_port [ENABLE]", mstid, lport);
                }
                else if (ext_port_entry.dot1d_stp_port_admin_edge_port == FALSE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: admin_edge_port [DISABLE]", mstid, lport);
                }
                if (ext_port_entry.dot1d_stp_port_oper_edge_port == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: op_edge_port [ENABLE]", mstid, lport );
                }
                else if (ext_port_entry.dot1d_stp_port_oper_edge_port == FALSE)
                {
                    BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: op_edge_port [DISABLE]", mstid, lport );
                }
                BACKDOOR_MGR_Printf("\r\n mstid [%ld] port [%ld]: dot1d_stp_port_long_path_cost [%ld]", mstid, lport, ext_port_entry.dot1d_stp_port_long_path_cost  );
                break;

            case 12:
                XSTP_MGR_SetPortPathCost(lport, set_value);
                break;

            case 13:
                XSTP_MGR_SetPortPriority(lport, set_value);
                break;

            case 14:
                XSTP_MGR_SetPortLinkTypeMode(lport, set_value);
                break;

            case 15:
                XSTP_MGR_SetPortProtocolMigration(lport, set_value);
                break;

            case 16:
                XSTP_MGR_SetPortAdminEdgePort(lport, set_value);
                break;

            case 17:
                XSTP_OM_GetMstpConfigurationEntry(&mstp_entry);
                BACKDOOR_MGR_Printf("\r\n mstp max_instance_numberms : [%ld]", mstp_entry.mstp_max_instance_number);
                BACKDOOR_MGR_Printf("\r\n mstp region_name : [%s]", mstp_entry.mstp_region_name);
                BACKDOOR_MGR_Printf("\r\n mstp region_revision : [%ld]", mstp_entry.mstp_region_revision);
                BACKDOOR_MGR_Printf("\r\n mstp max_hop_count : [%ld]", mstp_entry.mstp_max_hop_count);
                break;

            case 18:
                XSTP_MGR_SetMstpConfigurationName(name);
                break;

            case 19:
                XSTP_MGR_SetMstpRevisionLevel(set_value);
                break;

            case 20:
                XSTP_MGR_SetMstpMaxHop(set_value);
                break;
            case 21:
                if (XSTP_OM_GetMstpInstanceVlanMapped(mstid, &instance_entry)==TRUE)
                {
                    for (i=0; i<128; i++)
                    {
                        BACKDOOR_MGR_Printf("\r\n vlans1k (byte %d) : [%d]", i, instance_entry.mstp_instance_vlans_mapped [i]);
                    }
                    BACKDOOR_MGR_Printf("\n");
                    for (i=0; i<128; i++)
                    {
                        BACKDOOR_MGR_Printf("\r\n vlans2k (byte %d) : [%d]", i, instance_entry.mstp_instance_vlans_mapped2k[i]);
                    }
                    BACKDOOR_MGR_Printf("\n");
                    for (i=0; i<128; i++)
                    {
                        BACKDOOR_MGR_Printf("\r\n vlans3k (byte %d) : [%d]", i, instance_entry.mstp_instance_vlans_mapped3k[i]);
                    }
                    BACKDOOR_MGR_Printf("\n");
                    for (i=0; i<128; i++)
                    {
                        BACKDOOR_MGR_Printf("\r\n vlans4k (byte %d) : [%d]", i, instance_entry.mstp_instance_vlans_mapped4k[i]);
                    }
                    BACKDOOR_MGR_Printf("\n");
                    BACKDOOR_MGR_Printf("\r\n remaining_hop_count : [%ld]", instance_entry.mstp_instance_remaining_hop_count);
                }
                break;

            case 22:
                XSTP_MGR_SetVlanToMstConfigTable(mstid, vlan);
                break;

            case 23:
                XSTP_MGR_SetMstPriority(mstid, set_value);
                break;

            case 24:
                XSTP_MGR_SetMstPortPathCost(lport, mstid, set_value);
                break;

            case 25:
                XSTP_MGR_SetMstPortPriority(lport, mstid, set_value);
                break;
            case 26:
                XSTP_MGR_RemoveVlanFromMstConfigTable(mstid, vlan);
                break;
            case 27:
                memset(vlan_list , 0 , 128);
                vlan_list[100]=0x55;
                XSTP_MGR_SetVlanListToMstConfigTable(mstid, set_value, vlan_list);
                break;
            case 28:
                port_entry.dot1d_stp_port = (UI16_T)lport;
                XSTP_OM_GetNextDot1dMstPortEntry(&mstid, &port_entry);
                break;
            case 29:
                XSTP_MGR_GetPortStateByVlan(vlan, lport, &state);
                if (state == XSTP_TYPE_PORT_STATE_DISCARDING)
                {
                    BACKDOOR_MGR_Printf("\r\n state is [discarding] for lport[%ld] in vlan[%ld]", lport, vlan);
                }
                else if (state == XSTP_TYPE_PORT_STATE_LEARNING)
                {
                    BACKDOOR_MGR_Printf("\r\n state is [learning] for lport[%ld] in vlan[%ld]", lport, vlan);
                }
                else if (state == XSTP_TYPE_PORT_STATE_FORWARDING)
                {
                    BACKDOOR_MGR_Printf("\r\n state is [forwarding] for lport[%ld] in vlan[%ld]", lport, vlan);
                }
                else
                {
                  BACKDOOR_MGR_Printf("Error!!");
                }
                break;

            case 30:
                om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                XSTP_OM_GenerateConfigurationDigest(om_ptr);
                #endif /* XSTP_TYPE_PROTOCOL_MSTP */
                break;

            case 31:
                XSTP_MGR_SetPortSpanningTreeStatus(lport, set_value);
                break;

            case 32:
                XSTP_OM_GetRunningPortSpanningTreeStatus(lport, &state);
                BACKDOOR_MGR_Printf("\r\n state is [%ld]", state);
                break;
            case 33:
                XSTP_MGR_SetFloodingBpduWhenStpDisabled(TRUE);
                XSTP_MGR_GetBpduBehaviorWhenStpDisabled(&flood_bpdu);
                BACKDOOR_MGR_Printf("\r\n Bpdu Behavior is [%d]", flood_bpdu);
                break;
            case 34:
                XSTP_MGR_SetFloodingBpduWhenStpDisabled(FALSE);
                XSTP_MGR_GetBpduBehaviorWhenStpDisabled(&flood_bpdu);
                BACKDOOR_MGR_Printf("\r\n Bpdu Behavior is [%d]", flood_bpdu);
                break;
            case 35:
                XSTP_MGR_GetBpduBehaviorWhenStpDisabled(&flood_bpdu);
                BACKDOOR_MGR_Printf("\r\n Bpdu Behavior is [%d]", flood_bpdu);
                break;

            case 99:
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                BACKDOOR_MGR_Printf("\r\n Exit XSTP CLI Menu");
                /* Terminating */
                XSTP_BACKDOOR_TerminateCmd(cmd_buf);
                return;
        }
        /* Release execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
    }
}

/* Utility */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - XSTP_BACKDOOR_StrToVal
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert a string to a value
 * INPUT    : str       -- string with a ending NULL
 *                         the leading characters
 *                         0x       -- hexadecimal
 *                         0        -- octal
 *                         (number) -- decimal
 * OUTPUT   : value     -- value
 * RETURN   : TRUE if convert successful, else FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  XSTP_BACKDOOR_StrToVal(char *str, UI32_T *value)
{
    UI8_T   index, base, val_index, ch;
    UI32_T  str_value;
    BOOL_T  error;

    index       = 0;
    val_index   = 0;
    base        = 10;
    str_value   = 0;
    error       = FALSE;
    while ( (!error) && (str[index] != 0) )
    {
        ch = str[index];
        switch (ch)
        {
            case 'x':
            case 'X':
                if (val_index == 1)
                {
                    if (base == 8)
                        base = 16;
                    else
                        error = TRUE;
                }
                break;
            case ' ':
                index++;
                break;
            default:
                if ( (ch >= '0') && (ch <= '9'))
                {
                    if ( (val_index == 0) && (ch == '0') )
                    {
                        base = 8;
                    }
                    else
                    {
                        str_value = (UI32_T)(str_value * base) + (UI32_T)(ch - '0');
                    }
                    val_index++;
                }
                else
                if (base == 16)
                {
                    if ( (ch >= 'a') && (ch <= 'f') )
                    {
                        str_value = (UI32_T)(str_value * base) + (UI32_T)(ch - 'a' + 10);
                        val_index++;
                    }
                    else
                    if ( (ch >= 'A') && (ch <= 'F') )
                    {
                        str_value = (UI32_T)(str_value * base) + (UI32_T)(ch - 'A' + 10);
                        val_index++;
                    }
                    else
                        error = TRUE;

                }
                else
                {
                    error = TRUE;
                }
                break;
        } /* End of switch */
        index++;
    }
    *value = str_value;

    return (!error);
} /* End of XSTP_BACKDOOR_StrToVal */
