/*-------------------------------------------------------------------------
 * Module Name  :   vlan_backdoor.c
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for the VLAN
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    03/11/2001 -  Allen Cheng, created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */


/************************************
 ***   INCLUDE FILE DECLARATIONS  ***
 ************************************/
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "vlan_backdoor.h"
#include "vlan_lib.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_type.h"

#define VLAN_BACKDOOR_CMD_MAXLEN    16
#define KEY_ESC                     27
#define KEY_BACKSPACE               8
#define KEY_EOS                     0
#define MAX_BUF_LEN                 20

typedef struct
{
    char    *cmd_str;
    char    *cmd_title;
    char    *cmd_description;
    void    (*cmd_function)(char *cmd_buf);
} VLAN_BACKDOOR_CommandStruct_T;

typedef struct
{
    UI32_T  flag;
    char    *flag_string;
} VLAN_BACKDOOR_DebugStruct_T;

static  void    VLAN_BACKDOOR_Engine(void);
static  void    VLAN_BACKDOOR_ParseCmd(char *cmd_buf, UI8_T ch);
static  void    VLAN_BACKDOOR_ExecuteCmd(char *cmd_buf);
static  void    VLAN_BACKDOOR_TerminateCmd(char *cmd_buf);

static  void    VLAN_BACKDOOR_PrintMenu(char *cmd_buf);
static  void    VLAN_BACKDOOR_ShowDebugFlag(char *cmd_buf);
static  void    VLAN_BACKDOOR_SetDebugFlag(char *cmd_buf);
static  void    VLAN_BACKDOOR_PrintVlanTable(char *cmd_buf);
static  void    VLAN_BACKDOOR_PrintVlanId(char *cmd_buf);
static  void    VLAN_BACKDOOR_PrintPortId(char *cmd_buf);
static  void    VLAN_BACKDOOR_SetPortMode(char *cmd_buf);
static  void    VLAN_BACKDOOR_SetDefaultVlan(char *cmd_buf);
static  BOOL_T  VLAN_BACKDOOR_StrToVal(char *str, UI32_T *value);
static void     VLAN_BACKDOOR_PrintPortlist(UI8_T *portlist);


static  VLAN_BACKDOOR_CommandStruct_T VLAN_BACKDOOR_CommandTable[] =
{
    /*  cmd_str,    cmd_title,              cmd_descritption,                           cmd_function            */
    /*  ------------------------------------------------------------------------------------------------------  */
    {   "",         "VLAN",                 "Engineering Mode Main Menu",               VLAN_BACKDOOR_PrintMenu},
    {   "d",        "Debug Flag",           "To set the debug flag",                    VLAN_BACKDOOR_PrintMenu},
    {   "dd",       "Set Debug Flag",       "To set the debug flag",                    VLAN_BACKDOOR_SetDebugFlag},
    {   "ds",       "Show Debug Flag",      "To show the debug flag",                   VLAN_BACKDOOR_ShowDebugFlag},
    {   "v",        "Show Vlan",            "To show vlan info",                        VLAN_BACKDOOR_PrintMenu},
    {   "vi",       "Show Vlan Info",       "To show individual vlan info",             VLAN_BACKDOOR_PrintVlanId},
    {   "vt",       "Show Vlan Table",      "To show vlan table",                       VLAN_BACKDOOR_PrintVlanTable},
    {   "p",        "Show Port",            "To show port info",                        VLAN_BACKDOOR_PrintMenu},
    {   "pi",       "Show Port",            "To show individual port info",             VLAN_BACKDOOR_PrintPortId},
    {   "s",        "Set Port Mode",        "To Set Vlan Port mode",                    VLAN_BACKDOOR_SetPortMode},
    {   "c",        "Change Default VLAN",  "To Change Default VLAN",                   VLAN_BACKDOOR_SetDefaultVlan},
    {   "\x7F",     "End of Table",         "End of the Command Table",                 VLAN_BACKDOOR_PrintMenu}
};

static  VLAN_BACKDOOR_DebugStruct_T  VLAN_BACKDOOR_DebugFlagTable[] =
{
    {   VLAN_DEBUG_FLAG_NONE,           "VLAN_DEBUG_FLAG_NONE"          },
    {   VLAN_DEBUG_FLAG_ERRMSG,         "VLAN_DEBUG_FLAG_ERRMSG"        },
    {   VLAN_DEBUG_FLAG_TRUNK,          "VLAN_DEBUG_FLAG_TRUNK"         },
    {   VLAN_DEBUG_FLAG_VLAN,           "VLAN_DEBUG_FLAG_VLAN"          },
    {   VLAN_DEBUG_FLAG_NOTIFY,         "VLAN_DEBUG_FLAG_NOTIFY"       },
    {   VLAN_DEBUG_FLAG_DBGMSG4,        "VLAN_DEBUG_FLAG_DBGMSG4"       },
    {   VLAN_DEBUG_FLAG_ALL,            "VLAN_DEBUG_FLAG_ALL"           }
};


static  UI8_T   VLAN_BACKDOOR_ClearScreen[] =
{ KEY_ESC, '[', '2', 'J', KEY_EOS };

static  UI32_T  VLAN_DebugFlag;

static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

BOOL_T  VLAN_Debug(UI32_T flag)
{
    return ( (VLAN_DebugFlag & flag) != 0);
}

void    VLAN_SetDebugFlag(UI32_T flag)
{
    VLAN_DebugFlag    = flag;
    return;
}

void    VLAN_GetDebugFlag(UI32_T *flag)
{
    *flag   = VLAN_DebugFlag;
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Init(void)
{
    VLAN_SetDebugFlag( (UI32_T)VLAN_DEBUG_FLAG_NONE);

    return;
} /* End of VLAN_BACKDOOR_Init */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 * FUNCTION : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("vlan",
        SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY, VLAN_BACKDOOR_Main);
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Main(void)
{
    tg_handle = L2_L4_PROC_COMM_GetSwctrlGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    VLAN_BACKDOOR_Engine();

    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
} /* End of VLAN_BACKDOOR_Main */

static  void    VLAN_BACKDOOR_Engine(void)
{
    BOOL_T  engine_continue;
    UI8_T   ch;
    char    cmd_buf[VLAN_BACKDOOR_CMD_MAXLEN+1];

    engine_continue = TRUE;
    cmd_buf[0]      = 0;

    while (engine_continue)
    {
        VLAN_BACKDOOR_ExecuteCmd(cmd_buf);
        ch = BACKDOOR_MGR_GetChar();

        switch (ch)
        {
            case KEY_ESC:       /* Go to the main menu */
                cmd_buf[0]      = 0;
                break;

            case KEY_BACKSPACE: /* Go to the up level menu */
                if (cmd_buf[0] != 0)
                    cmd_buf[strlen(cmd_buf)-1]    = 0;
                break;

            case 'q':           /* Quit the engineering mode */
            case 'Q':
                engine_continue = FALSE;
                break;

            default:
                BACKDOOR_MGR_Printf("%c", ch);
                VLAN_BACKDOOR_ParseCmd(cmd_buf, ch);
                break;
        }

    }

    return;
} /* End of VLAN_BACKDOOR_Engine */

/*
 * If { cmd_buf[] | {ch} } exists in the VLAN_BACKDOOR_CommandTable
 * then cmd_buf[] |= ch;
 */
static  void    VLAN_BACKDOOR_ParseCmd(char *cmd_buf, UI8_T ch)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= VLAN_BACKDOOR_CMD_MAXLEN)
        return;
    cmd_buf[cmd_length]     = ch;
    cmd_buf[cmd_length+1]   = 0;

    while ((!cmd_found) && (VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F'))
    {
        if (strcmp(cmd_buf, VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
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

static  void    VLAN_BACKDOOR_ExecuteCmd(char *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= VLAN_BACKDOOR_CMD_MAXLEN)
        return;

    while ((!cmd_found) && (VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F'))
    {
        if (strcmp(cmd_buf, VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (cmd_found)
    {
        BACKDOOR_MGR_Printf("%s", VLAN_BACKDOOR_ClearScreen);
        /* Print Title and Description */
        BACKDOOR_MGR_Printf("\r\n%s    %s",  VLAN_BACKDOOR_CommandTable[cmd_index].cmd_title,
                                VLAN_BACKDOOR_CommandTable[cmd_index].cmd_description);
        BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------\n");
        VLAN_BACKDOOR_CommandTable[cmd_index].cmd_function(cmd_buf);
    }

    return;
}

static  void    VLAN_BACKDOOR_TerminateCmd(char *cmd_buf)
{
    /* Terminating */
    cmd_buf[strlen(cmd_buf)-1] = 0;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n[Esc]       : Main Menu");
    BACKDOOR_MGR_Printf("\r\n[Back_Space]: Up Menu");
    BACKDOOR_MGR_Printf("\r\nQ/q         : To quit");
    BACKDOOR_MGR_Printf("\r\nPress any key to continue ...");

    return;
} /* End of VLAN_BACKDOOR_TerminateCmd */

/* ------------------------------------------------------------------------
 * Command Function
 */
static  void    VLAN_BACKDOOR_PrintMenu(char *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;

    cmd_length  = (UI8_T)strlen(cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= VLAN_BACKDOOR_CMD_MAXLEN)
        return;

    BACKDOOR_MGR_Printf("\r\n");
    while ( VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!='\x7F')
    {
        if (    ( (UI8_T)(strlen(VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str) ) == (cmd_length + 1) )
             && ( strncmp(VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str, cmd_buf, cmd_length) == 0)
           )
        {
            BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",
                    VLAN_BACKDOOR_CommandTable[cmd_index].cmd_str[cmd_length],
                    VLAN_BACKDOOR_CommandTable[cmd_index].cmd_title,
                    VLAN_BACKDOOR_CommandTable[cmd_index].cmd_description
                  );
        }
        cmd_index++;
    }

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",'Q', "Quit", "To exit the VLAN engineering mode");
    BACKDOOR_MGR_Printf("\r\n%c : %-24s -- %-40s",'q', "Quit", "To exit the VLAN engineering mode");
    BACKDOOR_MGR_Printf("\r\n[Esc]       : Main Menu");
    BACKDOOR_MGR_Printf("\r\n[Back_Space]: Up Menu");
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\nSelect : ");

    return;
} /* VLAN_BACKDOOR_PrintMenu */

static  void    VLAN_BACKDOOR_ShowDebugFlag(char *cmd_buf)
{
    UI32_T  flag;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
    VLAN_GetDebugFlag(&flag);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
    BACKDOOR_MGR_Printf("\r\nDebugFlag = %08x",(UI16_T) flag);

    /* Terminating */
    VLAN_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of VLAN_BACKDOOR_ShowDebugFlag */

static  void    VLAN_BACKDOOR_SetDebugFlag(char *cmd_buf)
{
    UI8_T   i, select, value, ch;
    UI32_T  flag, set_flag;
    char    *status;

    i = 1;
    VLAN_GetDebugFlag(&flag);
    while (VLAN_BACKDOOR_DebugFlagTable[i].flag < (UI32_T)VLAN_DEBUG_FLAG_ALL)
    {
        /* Print the menu and the current status */
        if ((flag & VLAN_BACKDOOR_DebugFlagTable[i].flag) != 0)
        {
            status = "ON";
        }
        else
        {
            status = "OFF";
        }
        BACKDOOR_MGR_Printf("\r\n%2d -- %-32s : %-3s", i, VLAN_BACKDOOR_DebugFlagTable[i].flag_string, status);
        i++;
    }
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\nSelect (0 = end) : ");
    ch = BACKDOOR_MGR_GetChar();
    BACKDOOR_MGR_Printf("%c", ch);
    select = (UI8_T)(ch - '0');
    set_flag = (UI32_T)VLAN_BACKDOOR_DebugFlagTable[select].flag;
    if (select != 0)
    {
        BACKDOOR_MGR_Printf("          Set (1 = ON, 0 = OFF) : ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", ch);
        value = (UI8_T)(ch - '0');
        /* Get execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
        if (value)
        {
            VLAN_SetDebugFlag(flag | set_flag);
        }
        else
        {
            VLAN_SetDebugFlag(flag & ~set_flag);
        }
        /* Release execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
    }
    else
    {
        /* Terminating */
        VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    }

    return;
} /* End of VLAN_BACKDOOR_SetDebugFlag */

static void     VLAN_BACKDOOR_PrintVlanTable(char *cmd_buf)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    /* BODY */

    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));

    while (VLAN_OM_GetNextVlanEntry(&vlan_info) == TRUE)
    {
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("VLAN INDEX: %d\n", (UI16_T)vlan_info.dot1q_vlan_index);
        BACKDOOR_MGR_Printf("VLAN create time: %d\n",(UI16_T) vlan_info.dot1q_vlan_creation_time);
        BACKDOOR_MGR_Printf("VLAN update time: %d\n",(UI16_T) vlan_info.dot1q_vlan_time_mark);
        BACKDOOR_MGR_Printf("VLAN STATUS (1)other (2)permanent (3)dynamicGVRP: %d\n",(UI16_T)vlan_info.dot1q_vlan_status);
        BACKDOOR_MGR_Printf("VLAN Address Method (1)user (2)bootp (3)dhcp: %d\n",(UI16_T) vlan_info.vlan_address_method);
        BACKDOOR_MGR_Printf("VLAN IP Interface state (1)none (2)IPv4 (3)IPv6: %d\n",(UI16_T) vlan_info.vlan_ip_state);
        BACKDOOR_MGR_Printf("VLAN Operation Status (1) UP (2) DOWN: %d\n",(UI16_T) vlan_info.if_entry.vlan_operation_status);
        BACKDOOR_MGR_Printf("VLAN Row Status: %d\n",(UI16_T) vlan_info.dot1q_vlan_static_row_status);
        BACKDOOR_MGR_Printf("VLAN Operation Status: %d\n",(UI16_T) vlan_info.if_entry.vlan_operation_status);
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Current Egress Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_current_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Static Egress Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_static_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Untagged Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_current_untagged_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Forbidden Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_forbidden_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");
    } /* end of while */

    VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* end of if */

static void     VLAN_BACKDOOR_PrintVlanId(char *cmd_buf)
{
    char    buf[MAX_BUF_LEN];
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T  vid, vid_ifindex;

    /* BODY */

    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));

    BACKDOOR_MGR_Printf("Input VLAN ID:\n");
    BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN-1);

    if (!VLAN_BACKDOOR_StrToVal(buf, &vid) )
    {
        return;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index  = (UI16_T)vid_ifindex;

    if (VLAN_OM_GetVlanEntry(&vlan_info) == TRUE)
    {
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("VLAN INDEX: %d\n",(UI16_T) vlan_info.dot1q_vlan_index);
        BACKDOOR_MGR_Printf("VLAN create time: %d\n",(UI16_T) vlan_info.dot1q_vlan_creation_time);
        BACKDOOR_MGR_Printf("VLAN update time: %d\n",(UI16_T) vlan_info.dot1q_vlan_time_mark);
        BACKDOOR_MGR_Printf("VLAN STATUS (1)other (2)permanent (3)dynamicGVRP: %d\n",(UI16_T)vlan_info.dot1q_vlan_status);
        BACKDOOR_MGR_Printf("VLAN Address Method (1)user (2)bootp (3)dhcp: %d\n",(UI16_T) vlan_info.vlan_address_method);
        BACKDOOR_MGR_Printf("VLAN IP Interface state (1)none (2)IPv4 (3)IPv6: %d\n",(UI16_T) vlan_info.vlan_ip_state);
        BACKDOOR_MGR_Printf("VLAN IfType (1)L2_IFTYPE (2)L3_IP_IFTYPE (3)L3_IP_TUNNELTYPE: %d\n",(UI16_T) vlan_info.if_entry.ifType);

        BACKDOOR_MGR_Printf("VLAN Row Status: %d\n",(UI16_T) vlan_info.dot1q_vlan_static_row_status);
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Current Egress Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_current_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Static Egress Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_static_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Untagged Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_current_untagged_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");

        BACKDOOR_MGR_Printf("VLAN Forbidden Port List\n");
        VLAN_BACKDOOR_PrintPortlist(vlan_info.dot1q_vlan_forbidden_egress_ports);
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("\n");
    } /* end of if */

    VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static  void    VLAN_BACKDOOR_PrintPortId(char *cmd_buf)
{
    char    buf[MAX_BUF_LEN];
    VLAN_OM_Vlan_Port_Info_T     vlan_port_info;
    UI32_T  port_num, vid, index, vid_ifindex;

    /* BODY */

    memset(&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));

    BACKDOOR_MGR_Printf("Input Port Number:\n");
    BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN-1);

    if (!VLAN_BACKDOOR_StrToVal(buf, &port_num) )
    {
        return;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    BACKDOOR_MGR_Printf("\r\n");
    vlan_port_info.lport_ifindex = port_num;
    if (VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        BACKDOOR_MGR_Printf("lport ifindex: %d\r\n",(UI16_T) vlan_port_info.lport_ifindex);
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("PVID: %d\n",(UI16_T)vlan_port_info.port_item.dot1q_pvid_index);
        BACKDOOR_MGR_Printf("port trunk mode (TRUE)Trunk Member (FALSE)NORMAL PORT: %d\r\n",(UI16_T) vlan_port_info.port_trunk_mode);
        BACKDOOR_MGR_Printf("vlan port mode  (1)Hybrid (2)Q-Trunk (3) Access: %d\r\n",(UI16_T) vlan_port_info.vlan_port_entry.vlan_port_mode);
        BACKDOOR_MGR_Printf("Acceptable Frame Types (1)ALL (2)Tagged Only:  %d\r\n",(UI16_T)vlan_port_info.port_item.dot1q_port_acceptable_frame_types);
        BACKDOOR_MGR_Printf("Ingress Filter Status  (1)Enabled (2)Disabled: %d\r\n",(UI16_T)vlan_port_info.port_item.dot1q_port_ingress_filtering);
        BACKDOOR_MGR_Printf("Port Gvrp Status:      (1)Enabled (2)Disabled: %d\r\n",(UI16_T)vlan_port_info.port_item.dot1q_port_gvrp_status);
        BACKDOOR_MGR_Printf("GVRP Fail Registration Counter: %d\r\n",(UI16_T)vlan_port_info.port_item.dot1q_port_gvrp_failed_registrations);
        BACKDOOR_MGR_Printf("Static joined VLAN Counter: %d\r\n",(UI16_T)vlan_port_info.port_item.static_joined_vlan_count);
        BACKDOOR_MGR_Printf("Untagged joined VLAN Counter: %d\r\n",(UI16_T)vlan_port_info.port_item.untagged_joined_vlan_count);
    } /* end of if */

    BACKDOOR_MGR_Printf("VLAN MEMBERSHIPS:");
    vid = index = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
        if (VLAN_OM_IsPortVlanMember(vid_ifindex, port_num))
        {
            if (VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, port_num))
            {
                BACKDOOR_MGR_Printf(" %lu(U)", (unsigned long)vid);
                index++;
            }
            else
            {
                BACKDOOR_MGR_Printf(" %lu(T)", (unsigned long)vid);
                index++;
            }

        }
        else if (VLAN_OM_IsVlanForbiddenPortListMember(vid_ifindex, port_num))
        {
            BACKDOOR_MGR_Printf(" %lu(F)", (unsigned long)vid);
            index++;
        }
        else
        {
            continue;
        }

        if (index < 9)
        {
            BACKDOOR_MGR_Printf(",");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n");
            index = 0;
        }
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static  void    VLAN_BACKDOOR_SetPortMode(char *cmd_buf)
{
    char    buf[MAX_BUF_LEN];
    UI32_T  port_num, port_mode;

    /* BODY */

    BACKDOOR_MGR_Printf("Input Port Number:\n");
    BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN-1);

    if (!VLAN_BACKDOOR_StrToVal(buf, &port_num) )
    {
        return;
    }

    BACKDOOR_MGR_Printf("Input Port Mode (1)Hybrid (2)Q-Trunk (3)Access:\n");
    BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN-1);

    if (!VLAN_BACKDOOR_StrToVal(buf, &port_mode) )
    {
        return;
    }

    switch (port_mode)
    {
        case 1:
            port_mode = VAL_vlanPortMode_hybrid;
            break;
        case 2:
            port_mode = VAL_vlanPortMode_dot1qTrunk;
            break;
        case 3:
            port_mode = VAL_vlanPortMode_access;
            break;
        default:
            break;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if (!VLAN_MGR_SetVlanPortMode(port_num, port_mode))
    {
        BACKDOOR_MGR_Printf("VLAN Set Port Mode Fails\n");
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    return;

}

static void VLAN_BACKDOOR_PrintPortlist(UI8_T *portlist)
{
    UI8_T   local_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  port_num, index = 0;

    /* BODY */

    memcpy(local_portlist, portlist, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    for (port_num = 1; port_num <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST * 8); port_num++)
    {
        if (local_portlist[(port_num-1)/8] == 0)
        {
            port_num += 7;
            index++;
            continue;
        }

        if ((((port_num-1) % 8) == 0))
        {
            BACKDOOR_MGR_Printf("\n");
            BACKDOOR_MGR_Printf("   Index [%lu]: ", (unsigned long)index);
            index++;
        }

        if (local_portlist[(port_num-1)/8] & ((0x01) << ( 7 - ((port_num-1)%8))))
        {
            BACKDOOR_MGR_Printf("1");
        }
        else
        {
            BACKDOOR_MGR_Printf("0");
        }
    } /* end of for */
} /* end of VALN_BACKDOOR_PrintPortlist() */

static  BOOL_T  VLAN_BACKDOOR_StrToVal(char *str, UI32_T *value)
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
                {
                    error = TRUE;
                }
                break;
        } /* End of switch */
        index++;
    }
    *value = str_value;

    return (!error);
} /* End of VLAN_BACKDOOR_StrToVal */

static  void    VLAN_BACKDOOR_SetDefaultVlan(char *cmd_buf)
{
    char    buf[MAX_BUF_LEN];
    UI32_T  vid;

    /* BODY */

    if (!VLAN_OM_GetGlobalDefaultVlan_Ex(&vid))
        return;

    BACKDOOR_MGR_Printf("Old VID:%lu\n", (unsigned long)vid);

    BACKDOOR_MGR_Printf("Input VID:\n");
    BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN-1);

    if (!VLAN_BACKDOOR_StrToVal(buf, &vid) )
    {
        return;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if (!VLAN_MGR_SetGlobalDefaultVlan(vid))
    {
        BACKDOOR_MGR_Printf("VLAN Set Port Mode Fails\n");
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    VLAN_BACKDOOR_TerminateCmd(cmd_buf);
    return;

}
