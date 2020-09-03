/* MODULE NAME: netaccess_backdoor.c
 * PURPOSE:
 *  Implement netaccess backdoor
 *
 * NOTES:
 *
 * History:
 *    2007/11/27: Squid Ro      Create this file
 *
 * Copyright(C)      Accton Corporation, 2007
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_backdoor.h"

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "backdoor_mgr.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "l_threadgrp.h"
#include "1x_om.h"

#include "l2_l4_proc_comm.h"

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_om.h"
#include "netaccess_mgr.h"
#endif /* SYS_CPNT_NETACCESS == TRUE */

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_BACKDOOR_PROMPT       "\r\ncommand > "
#define NEWLINE                         "\x0A\x0D"

/* MACRO FUNCTION DECLARATIONS
 */
#define GET_TOKEN_AND_MOVE_NEXT(str, token, pos) { \
    *pos = '\0'; token = str; str = pos + 1; }

#define IS_DIGIT(c)                     (('0' <= c) && (c <= '9'))
#define IS_UPALPHA(c)                   (('A' <= c) && (c <= 'Z'))
#define IS_LOALPHA(c)                   (('a' <= c) && (c <= 'z'))
#define IS_ALPHA(c)                     (IS_UPALPHA(c) || IS_LOALPHA(c))
#define PRINT                           BACKDOOR_MGR_Printf

/* DATA TYPE DECLARATIONS
 */
typedef void (*NetaccessFuncPtr)(int argc, char** argv);

typedef struct NETACCESS_Backdoor_Cmd_S
{
    const char*         cmd;
    NetaccessFuncPtr    exec;
} NETACCESS_Backdoor_Cmd_T;

typedef struct NETACCESS_Backdoor_Menu_S
{
    const char*     name;
    const int       nbr_of_sub_menus;
    const int       nbr_of_cmd_items;

    struct NETACCESS_Backdoor_Menu_S *parent_menu;
    struct NETACCESS_Backdoor_Menu_S **sub_menus;
    NETACCESS_Backdoor_Cmd_T         *cmd_items;
} NETACCESS_Backdoor_Menu_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* ---------------------------------------------------------------------
 * common functions
 * --------------------------------------------------------------------- */
static void NETACCESS_Backdoor_ShowIpAddr(UI8_T *addr);
static BOOL_T NETACCESS_Backdoor_Str2IpAddr(char *str, UI8_T *addr);

static void NETACCESS_Backdoor_ShowMacAddr(UI8_T *addr);
static BOOL_T NETACCESS_Backdoor_Str2MacAddr(char *str, UI8_T *addr);

static void NETACCESS_Backdoor_ShowRowStatus(UI32_T row_status);

static void NETACCESS_Backdoor_Help(int argc, char** argv);
static void NETACCESS_BACKDOOR_SetDebugFlag(int argc, char** argv);
static int NETACCESS_Backdoor_StrCmpIgnoreCase(const char *dst, const char *src);
static BOOL_T NETACCESS_Backdoor_ChopString(char *str);


/* ---------------------------------------------------------------------
 * backdoor command functions
 * --------------------------------------------------------------------- */

/* portauthsrvc backdoor command functions */
#if (PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE)

    static void NETACCESS_Backdoor_PortAuthSrvc_TestVlanQosAssignment(int argc, char** argv);

#endif /* PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* netaccess backdoor command functions */
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    static void NETACCESS_Backdoor_NetAccess_SetDebugFlag(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetSecurePortMode(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetSecureNumberAddresses(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetDynamicVlanStatus(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetDynamicQosStatus(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetSecureAddrRowStatus(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationStatus(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationNumberAddresses(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationIntrustionAction(int argc, char * * argv);

    #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    static void NETACCESS_Backdoor_NetAccess_SetMacAddressAgingMode(int argc, char * * argv);
    #endif

    #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    static void NETACCESS_Backdoor_NetAccess_SetLinkDetection(int argc, char** argv);
    #endif

    static void NETACCESS_Backdoor_NetAccess_ShowSecurePort(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_ShowStateMachine(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_ShowSecureAddress(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_ShowNextSecureAddressEntry(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_ShowAllSecureAddressByPort(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_CreateNewMacMsg(int argc, char** argv);
    static void NETACCESS_Backdoor_NetAccess_SetGuestVlanId(int argc, char** argv);
    #if (SYS_CPNT_DOT1X == TRUE)
    static void NETACCESS_Backdoor_NetAccess_SetDot1xIntrustion(int argc, char** argv);
    #endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    #if (SYS_CPNT_DOT1X == TRUE)
    static void NETACCESS_Backdoor_Dot1x_OpenDebugMessage(int argc, char **argv);
    static void NETACCESS_Backdoor_Dot1x_CloseDebugMessage(int argc, char **argv);
    #endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
    static void NETACCESS_Backdoor_NetAccess_SetEapolFramePassThrough(int argc, char** argv);
    #endif

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* STATIC VARIABLE DECLARATIONS
 */

/* ---------------------------------------------------------------------
 * backdoor menu tree
 * --------------------------------------------------------------------- */
#if (PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE)

    /* portauthsrvc backdoor command items */
    static NETACCESS_Backdoor_Cmd_T portauthsrvc_command_items[] = {
        {"help", NETACCESS_Backdoor_Help},
        {"test_vlan_qos_assignment", NETACCESS_Backdoor_PortAuthSrvc_TestVlanQosAssignment},
    };

    /* portauthsrvc backdoor menu */
    static NETACCESS_Backdoor_Menu_T portauthsrvc_menu[] = {
        {"port_auth_srvc", 0, sizeof(portauthsrvc_command_items)/sizeof(NETACCESS_Backdoor_Cmd_T),NULL, NULL, portauthsrvc_command_items},
    };

#endif /* PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    /* netaccess backdoor basic test command items */
    static NETACCESS_Backdoor_Cmd_T netaccess_basic_test_cmd[] = {
        {"help", NETACCESS_Backdoor_Help},
        {"set_secure_port_mode", NETACCESS_Backdoor_NetAccess_SetSecurePortMode},
        {"set_secure_number_address", NETACCESS_Backdoor_NetAccess_SetSecureNumberAddresses},
        {"set_dynamic_vlan_status", NETACCESS_Backdoor_NetAccess_SetDynamicVlanStatus},
        {"set_dynamic_qos_status", NETACCESS_Backdoor_NetAccess_SetDynamicQosStatus},
        {"set_secure_addr_row_status", NETACCESS_Backdoor_NetAccess_SetSecureAddrRowStatus},
        {"set_mac_auth_status", NETACCESS_Backdoor_NetAccess_SetMacAuthenticationStatus},
        {"set_mac_auth_max_mac_count", NETACCESS_Backdoor_NetAccess_SetMacAuthenticationNumberAddresses},
        {"set_mac_auth_intrustion_action", NETACCESS_Backdoor_NetAccess_SetMacAuthenticationIntrustionAction},
    #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
        {"set_mac_address_aging_mode", NETACCESS_Backdoor_NetAccess_SetMacAddressAgingMode},
    #endif
    #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        {"set_link_detection", NETACCESS_Backdoor_NetAccess_SetLinkDetection},
    #endif
        {"create_new_mac_msg", NETACCESS_Backdoor_NetAccess_CreateNewMacMsg},
        {"set_guest_vlan_id", NETACCESS_Backdoor_NetAccess_SetGuestVlanId},
    #if (SYS_CPNT_DOT1X == TRUE)
        {"set_dot1x_intrusion_action", NETACCESS_Backdoor_NetAccess_SetDot1xIntrustion},
    #endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    };

    /* netaccess backdoor basic function test menu */
    static NETACCESS_Backdoor_Menu_T netaccess_basic_test_menu[] = {
        {"basic_function_test", 0, sizeof(netaccess_basic_test_cmd)/sizeof(NETACCESS_Backdoor_Cmd_T), NULL, NULL, netaccess_basic_test_cmd},
    };

    static NETACCESS_Backdoor_Menu_T *sub_menu_of_netaccess_menu[] = {
        netaccess_basic_test_menu,
    };

    /* netaccess backdoor command items */
    static NETACCESS_Backdoor_Cmd_T netaccess_command_items[] = {
        {"help", NETACCESS_Backdoor_Help},
        {"set_debug_flag", NETACCESS_Backdoor_NetAccess_SetDebugFlag},
        {"show_secure_port", NETACCESS_Backdoor_NetAccess_ShowSecurePort},
        {"show_state_machine", NETACCESS_Backdoor_NetAccess_ShowStateMachine},
        {"show_secure_address", NETACCESS_Backdoor_NetAccess_ShowSecureAddress},
        {"show_next_secure_address", NETACCESS_Backdoor_NetAccess_ShowNextSecureAddressEntry},
        {"show_all_secure_address_by_port", NETACCESS_Backdoor_NetAccess_ShowAllSecureAddressByPort},
    };

    /* netaccess backdoor menu */
    static NETACCESS_Backdoor_Menu_T netaccess_menu[] = {
        {"net_access",
         sizeof(sub_menu_of_netaccess_menu)/sizeof(NETACCESS_Backdoor_Menu_T*),
         sizeof(netaccess_command_items)/sizeof(NETACCESS_Backdoor_Cmd_T),
         NULL,
         sub_menu_of_netaccess_menu,
         netaccess_command_items},
    };

    /* dot1x backdoor command items */
    static NETACCESS_Backdoor_Cmd_T dot1x_command_items[] = {
        {"help", NETACCESS_Backdoor_Help},
    #if (SYS_CPNT_DOT1X == TRUE)
        {"open_debug_message", NETACCESS_Backdoor_Dot1x_OpenDebugMessage},
        {"close_debug_message", NETACCESS_Backdoor_Dot1x_CloseDebugMessage},
    #endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
        {"set_eapol_frame_pass_thru", NETACCESS_Backdoor_NetAccess_SetEapolFramePassThrough},
    #endif
    };

    /* dot1x backdoor menu */
    static NETACCESS_Backdoor_Menu_T dot1x_menu[] = {
        {"dot1x",
         0,
         sizeof(dot1x_command_items)/sizeof(NETACCESS_Backdoor_Cmd_T),
         NULL,
         NULL,
         dot1x_command_items},
    };

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */


static NETACCESS_Backdoor_Cmd_T  main_command_items[] = {
    {"debug", NETACCESS_BACKDOOR_SetDebugFlag},
    {"help", NETACCESS_Backdoor_Help},
};


static NETACCESS_Backdoor_Menu_T *sub_menu_of_main_menu[] = {
    #if (PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE)
        portauthsrvc_menu,
    #endif /* PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE */

    #if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
        netaccess_menu,
        dot1x_menu,
    #endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */
    };

static NETACCESS_Backdoor_Menu_T main_menu[] = {
        {"netaccess",
         sizeof(sub_menu_of_main_menu)/sizeof(NETACCESS_Backdoor_Menu_T*),
         sizeof(main_command_items)/sizeof(NETACCESS_Backdoor_Cmd_T),
         NULL,
         sub_menu_of_main_menu,
         main_command_items},
    };

/* ---------------------------------------------------------------------
 * backdoor operation variables
 * --------------------------------------------------------------------- */

static NETACCESS_Backdoor_Menu_T    *current_menu = NULL;
static L_THREADGRP_Handle_T         curr_tg_handle = NULL;
static UI32_T                       backdoor_member_id;


/* EXPORTED SUBPROGRAM BODIES
 */
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_Backdoor_Register_SubsysBackdoorFunc
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void NETACCESS_Backdoor_Register_SubsysBackdoorFunc(void)
{
    /* register call back function for backdoor program
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("netaccess",
        SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, NETACCESS_Backdoor_CallBack);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : security backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void NETACCESS_Backdoor_CallBack(void)
{
    const int               buf_len = 512;
    char                     buffer[buf_len];
    const int               size = 20;
    char                    *pos, *ptr, *token[size];
    int                     cnt, i, j, max, shift;

    curr_tg_handle = L2_L4_PROC_COMM_GetNetaccessGroupTGHandle();

/* initialize parent menu */
#if (PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE)
    portauthsrvc_menu[0].parent_menu = main_menu;
#endif /* PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    netaccess_basic_test_menu[0].parent_menu = netaccess_menu;
    netaccess_menu[0].parent_menu = main_menu;
#if (SYS_CPNT_DOT1X == TRUE)
    dot1x_menu[0].parent_menu = main_menu;
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

    BACKDOOR_MGR_Printf("\r\nNetAccess backdoor");
    NETACCESS_Backdoor_Help(0, NULL);


    /* Join thread group
     */
    if(L_THREADGRP_Join(curr_tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (TRUE)
    {
        BACKDOOR_MGR_Printf(NETACCESS_BACKDOOR_PROMPT);
        BACKDOOR_MGR_RequestKeyIn(buffer, buf_len);

        if (!NETACCESS_Backdoor_ChopString(buffer))
            continue;

        if (!NETACCESS_Backdoor_StrCmpIgnoreCase(buffer, "?"))
        {
            NETACCESS_Backdoor_Help(0, NULL);
            continue;
        }

        if (!NETACCESS_Backdoor_StrCmpIgnoreCase(buffer, "quit"))
            break;

        if (!NETACCESS_Backdoor_StrCmpIgnoreCase(buffer, "exit"))
        {
            if ((NULL == current_menu) || (NULL == current_menu->parent_menu))
                break;

            current_menu = current_menu->parent_menu;
            NETACCESS_Backdoor_Help(0, NULL);
            continue;
        }

        ptr = buffer;
        memset(token, 0, sizeof(token));

        for (cnt = 0, shift = 0; size > cnt; ++cnt)
        {

            pos = strchr(ptr, ' ');
            if (pos)
            {
                GET_TOKEN_AND_MOVE_NEXT(ptr, token[cnt], pos);
            }
            else
            {
                token[cnt] = ptr;
                break;
            }
        }

        if (!token[0] || !token[0][0])
            continue;

        /* sub menus */
        for (i = 0, max = current_menu->nbr_of_sub_menus; max > i; ++i)
        {
            if (NETACCESS_Backdoor_StrCmpIgnoreCase(token[0], current_menu->sub_menus[i]->name))
                continue;

            current_menu = current_menu->sub_menus[i];
            NETACCESS_Backdoor_Help(0, NULL);
            break;
        }

        if (max > i)
            continue;

        /* command items */
        for (j = 0, max = current_menu->nbr_of_cmd_items; max > j; ++j)
        {
            if (NETACCESS_Backdoor_StrCmpIgnoreCase(token[0], current_menu->cmd_items[j].cmd))
                continue;

            current_menu->cmd_items[j].exec(cnt, token + 1);

            if (TRUE == L_THREADGRP_Execution_Request(curr_tg_handle, backdoor_member_id))
            {
                current_menu->cmd_items[j].exec(cnt, token + 1);

                L_THREADGRP_Execution_Release(curr_tg_handle, backdoor_member_id);
            }
            else
            {
                BACKDOOR_MGR_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            }
            break;
        }

        if (max > j)
            continue;

        /* qqq command */
        max = strlen(token[0]);
        for (i = 0; max > i; ++i)
        {
            if (('q' != token[0][i]) && ('Q' != token[0][i]))
                break;
        }

        if (max <= i)
        {
            for (i = 0; max > i; ++i)
            {
                current_menu = current_menu->parent_menu;
                if (NULL == current_menu)
                    return;
            }

            NETACCESS_Backdoor_Help(0, NULL);
        }
        else
        {
            /* number selection */
            i = atoi(token[0]);
            if ((0 <= i) && (IS_DIGIT(token[0][0])))
            {
                if (current_menu->nbr_of_sub_menus > i)
                {
                    current_menu = current_menu->sub_menus[i];
                    NETACCESS_Backdoor_Help(0, NULL);
                    continue;
                }

                i -= current_menu->nbr_of_sub_menus;
                if (current_menu->nbr_of_cmd_items > i)
                {
                    if (NULL != curr_tg_handle)
                    {
                        L_THREADGRP_Execution_Request(curr_tg_handle, backdoor_member_id);
                    }

                    current_menu->cmd_items[i].exec(cnt, token + 1);

                    if (NULL != curr_tg_handle)
                    {
                        L_THREADGRP_Execution_Release(curr_tg_handle, backdoor_member_id);
                    }

                    continue;
                }
            }

            BACKDOOR_MGR_Printf("\r\nbackdoor: %s: command not found", token[0]);
        }
    } /* while */

    L_THREADGRP_Leave(curr_tg_handle, backdoor_member_id);
}



#define MAX_SHOW_NAME_STR_LEN   20
#define MAX_BACKDOOR_ITEM_NUM   10

typedef struct
{
    char    show_name[MAX_SHOW_NAME_STR_LEN + 1];
    BOOL_T  on_off;
}NETACCESS_BACKDOOR_PoolItem_T;

static NETACCESS_BACKDOOR_PoolItem_T netaccess_backdoor_pool[MAX_BACKDOOR_ITEM_NUM];

NETACCESS_BACKDOOR_PoolItem_T* NETACCESS_BACKDOOR_PollItem(UI32_T i)
{
    return &netaccess_backdoor_pool[i];
}

BOOL_T NETACCESS_BACKDOOR_Register(const char *show_name, UI32_T *reg_no_p)
{
    NETACCESS_BACKDOOR_PoolItem_T *item;
    int i = 0;

    for (; i < MAX_BACKDOOR_ITEM_NUM; ++i)
    {
        item = NETACCESS_BACKDOOR_PollItem(i);
        if (item->show_name[0] == 0)
        {
            strncpy(item->show_name, show_name, MAX_SHOW_NAME_STR_LEN);
            *reg_no_p = i;
            return TRUE;
        }
    }

    return TRUE;
}

BOOL_T NETACCESS_BACKDOOR_IsOn(UI32_T reg_no)
{
    NETACCESS_BACKDOOR_PoolItem_T *item;

    if (MAX_BACKDOOR_ITEM_NUM < reg_no)
        return FALSE;

    item = NETACCESS_BACKDOOR_PollItem(reg_no);
    if (item->show_name[0] == 0)
        return FALSE;

    return item->on_off;
}

static void NETACCESS_BACKDOOR_Switch(UI32_T i)
{
    NETACCESS_BACKDOOR_PoolItem_T *item;

    if (MAX_BACKDOOR_ITEM_NUM < i)
        return;

    item = NETACCESS_BACKDOOR_PollItem(i);

    if (item->on_off)
        item->on_off = 0;
    else
        item->on_off = 1;
}

static void NETACCESS_BACKDOOR_ShowMenu()
{
    NETACCESS_BACKDOOR_PoolItem_T *item;
    int i = 0;

    PRINT("\n");
    for (; i < MAX_BACKDOOR_ITEM_NUM; ++i)
    {
        item = NETACCESS_BACKDOOR_PollItem(i);

        if (item->show_name[0] != 0)
        {
            PRINT("[%d] %s = %s\n", i, item->show_name, item->on_off? "on":"off");
        }
    }
}

static void NETACCESS_BACKDOOR_SetDebugFlag(int argc, char * * argv)
{
    char ch;

    while (1)
    {
        NETACCESS_BACKDOOR_ShowMenu();

        PRINT("\r\n Press 'q' to exit : ");

        ch = getchar();

        if (ch == 'q' || ch == 'Q')
            break;

        if ('0' <= ch && ch <= '9')
        {
            NETACCESS_BACKDOOR_Switch(ch - '0');
        }
    }
}

#endif /* #if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE) */



/* LOCAL SUBPROGRAM BODIES
 */

static void NETACCESS_Backdoor_ShowIpAddr(UI8_T *addr)
{
    if (NULL == addr)
        return;

    BACKDOOR_MGR_Printf("%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

static BOOL_T NETACCESS_Backdoor_Str2IpAddr(char *str, UI8_T *addr)
{
    char *pos;
    int  i;

    if ((str == NULL) || (addr == NULL))
        return FALSE;

    for (i = 0; i < 4; ++i)
    {
        pos = strchr(str, '.');
        if (pos == NULL)
        {
            if (i != 3)
            {
                BACKDOOR_MGR_Printf("\r\nbad ip addr: %s!", str);
                return FALSE;
            }
        }
        else
            *pos = '\0';

        addr[i] = atoi(str);
        str = pos + 1;
    }

    return TRUE;
}

static void NETACCESS_Backdoor_ShowMacAddr(UI8_T *addr)
{
    if (NULL == addr)
        return;

    BACKDOOR_MGR_Printf("%02x-%02x-%02x-%02x-%02x-%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static BOOL_T NETACCESS_Backdoor_Str2MacAddr(char *str, UI8_T *addr)
{
    int i;

    if ((str == NULL) || (addr == NULL))
        return FALSE;

    if (strlen(str) != 17)
    {
        BACKDOOR_MGR_Printf("\r\nbad mac addr: %s!", str);
        return FALSE;
    }

    for (i = 0; i < SYS_ADPT_MAC_ADDR_LEN; ++i)
    {
        addr[i] = 0;
        str[2] = '\0';

        if (IS_DIGIT(str[0]))
        {
            addr[i] += (str[0] - '0') * 16;
        }
        else if (IS_ALPHA(str[0]))
        {
            addr[i] += (str[0] - 'a' + 10) * 16;
        }
        else
            return FALSE;

        if (IS_DIGIT(str[1]))
        {
            addr[i] += (str[1] - '0');
        }
        else if (IS_ALPHA(str[1]))
        {
            addr[i] += (str[1] - 'a' + 10);
        }
        else
            return FALSE;

        str += 3;
    }

    return TRUE;
}

static void NETACCESS_Backdoor_ShowRowStatus(UI32_T row_status)
{
    BACKDOOR_MGR_Printf("%s",
        (1 == row_status) ? "active" :
        (2 == row_status) ? "notInService" :
        (3 == row_status) ? "notReady" :
        (4 == row_status) ? "createAndGo" :
        (5 == row_status) ? "createAndWait" :
        (6 == row_status) ? "destroy" :
        "unknown");
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_Backdoor_Help
 *---------------------------------------------------------------------------
 * PURPOSE  : show current backdoor menu
 * INPUT    : argc, argv
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *---------------------------------------------------------------------------*/
static void NETACCESS_Backdoor_Help(int argc, char** argv)
{
    const char sub_menu_fmt[]  = "\r\n      <sub menu>";
    const char menu_edge_fmt[] = "\r\n  |   +--(%d) %s";
    const char menu_mide_fmt[] = "\r\n  |   +--(%d) %s";
    const char menu_cmd_fmt[]  = "\r\n  |";
    const char cmd_edge_fmt[]  = "\r\n  +------(%d) %s";
    const char cmd_mide_fmt[]  = "\r\n  +------(%d) %s";

    int     i, j, max, tmp;
    const char *fmt;

    if (NULL == current_menu)
        current_menu = main_menu;

    BACKDOOR_MGR_Printf("\r\n-----------------------------------------------");

    if (0 < current_menu->nbr_of_sub_menus)
        BACKDOOR_MGR_Printf((char *)sub_menu_fmt);

    for (i = 0, max = current_menu->nbr_of_sub_menus, tmp = max - 1; max > i; ++i)
    {
        if ((0 == i) || (tmp == i))
            fmt = menu_edge_fmt;
        else
            fmt = menu_mide_fmt;

        BACKDOOR_MGR_Printf((char *)fmt, i, current_menu->sub_menus[i]->name);
    }

    BACKDOOR_MGR_Printf((char *)menu_cmd_fmt);
    for (j = 0, max = current_menu->nbr_of_cmd_items, tmp = max - 1; max > j; ++j)
    {
        if ((0 == j) || (tmp == j))
            fmt = cmd_edge_fmt;
        else
            fmt = cmd_mide_fmt;

        BACKDOOR_MGR_Printf((char *)fmt, i + j, current_menu->cmd_items[j].cmd);
    }

    BACKDOOR_MGR_Printf("\r\n\r\nexit -- back to up menu, quit -- back to accton backdoor");
    BACKDOOR_MGR_Printf("\r\nq[q..] -- shortcut");
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_Backdoor_StrCmpIgnoreCase
 *-------------------------------------------------------------------------
 * PURPOSE  : compare two strings case-insensitivly
 * INPUT    : dst, src (terminate by '\0')
 * OUTPUT   : none
 * RETURN   : 0 -- equal, > 0 -- dst > src, < 0 -- small
 * NOTE     :
 *-------------------------------------------------------------------------*/
static int NETACCESS_Backdoor_StrCmpIgnoreCase(const char *dst, const char *src)
{
    const UI8_T diff = 'a' - 'A';
    UI8_T       s, d;

    if ((NULL == dst) || (NULL == src)) /* can't compare */
        return -1;

    do
    {
        s = (('A' <= *src) && ('Z' >= *src)) ? (*src + diff) : *src;
        d = (('A' <= *dst) && ('Z' >= *dst)) ? (*dst + diff) : *dst;

        dst++;
        src++;
    }
    while (d && d == s);

    return (d - s);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_Backdoor_ChopString
 *-------------------------------------------------------------------------
 * PURPOSE  : chop source string then put the result back
 * INPUT    : src_str (terminate by '\0')
 * OUTPUT   : src_str
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_Backdoor_ChopString(char *str)
{
    char      *src_iter, *dst_iter, *dst_end;

    if (NULL == str)
        return FALSE;

    dst_iter = dst_end = NULL;
    src_iter = str;

    for ( ; '\0' != *src_iter; ++src_iter)
    {
        switch (*src_iter)
        {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if (NULL == dst_iter)
                    continue;

                if (NULL == dst_end)
                    dst_end = dst_iter;

                break;

            default:
                if (NULL != dst_end) /* e.g. "substring substring" or "substring\tsubsting" */
                    dst_end = NULL;
                break;
        }

        if (NULL == dst_iter)
            dst_iter = str;

        *dst_iter++ = *src_iter;
    }

    if (NULL != dst_end) /* e.g. "substring " or "substring substring/r/n" */
        *dst_end = '\0';
    else if ((NULL != dst_iter) && (dst_iter != src_iter)) /* e.g. " a" */
        *dst_iter = '\0';

    return TRUE;
}

#if (PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE)

#define MAX_KEY_IN_STRING_MENU  100
#define MAX_KEY_IN_STRING_VLAN SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1
#define MAX_KEY_IN_STRING_QOS SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1

static PortAuthSrvAssign_T* NETACCESS_Backdoor_PortAuthSrvc_GetMacItem(PortAuthSrvAssign_T *base, UI32_T index)
{
    UI32_T i;

    for (i=0; i < index; i++)
    {
        if (base->next)
            base = base->next;
        else
            break;
    }
    return base;
}

static void NETACCESS_Backdoor_PortAuthSrvc_DisplayMac(PortAuthSrvAssign_T *all_mac, UI32_T count)
{
    UI32_T i;

    if(count == 0)
    {
        BACKDOOR_MGR_Printf(NEWLINE"  No MAC exists."NEWLINE);
        return;
    }

    for(i = 1; i <= count; i++)
    {
        BACKDOOR_MGR_Printf(NEWLINE"  %ld. MAC%ld [%s] [%s]", i, i, all_mac->vlan_assignment, all_mac->qos_assignment);
        all_mac = all_mac->next;
    }
    BACKDOOR_MGR_Printf(NEWLINE);
}

static void NETACCESS_Backdoor_PortAuthSrvc_ClearMacList(PortAuthSrvAssign_T *all_mac)
{
    PortAuthSrvAssign_T *first = all_mac;

    if (all_mac == NULL || all_mac->next == NULL)
        return;
    all_mac = all_mac->next;
    while (all_mac)
    {
        PortAuthSrvAssign_T *temp = all_mac->next;
        free(all_mac);
        all_mac = temp;
    }
    memset(first, 0, sizeof(PortAuthSrvAssign_T));
}

static UI16_T NETACCESS_Backdoor_PortAuthSrvc_AppendMac(PortAuthSrvAssign_T *all_mac)
{
    /*go to last node. */
    while (all_mac->next != NULL)
        all_mac = all_mac->next;

    if ((all_mac->next = (PortAuthSrvAssign_T*)malloc(sizeof(PortAuthSrvAssign_T))))
    {
        memset(all_mac->next, 0, sizeof(PortAuthSrvAssign_T));
        return 1;
    }

    return 0;
}

static void NETACCESS_Backdoor_PortAuthSrvc_SetMacItem(PortAuthSrvAssign_T *item, UI32_T index)
{
    UI8_T new_vlan[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    UI8_T new_qos[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
    UI32_T len;

    BACKDOOR_MGR_Printf(NEWLINE"\tEnter VLAN for MAC%ld: ", index+1);
    memset(new_vlan, 0, sizeof(new_vlan));
    BACKDOOR_MGR_RequestKeyIn(new_vlan, sizeof(new_vlan));

    if (new_vlan[0] != '\n')
    {
        len = strlen(new_vlan);
        if (new_vlan[len-1] == '\n')
            new_vlan[len-1] = '\0';
        strcpy(item->vlan_assignment, new_vlan);
    }

    BACKDOOR_MGR_Printf(NEWLINE"\tEnter QoS for MAC%ld: ", index+1);
    memset(new_qos, 0, sizeof(new_qos));
    BACKDOOR_MGR_RequestKeyIn(new_qos, sizeof(new_qos));
    if (new_qos[0] != '\n')
    {
        len = strlen(new_qos);
        if (new_qos[len-1] == '\n')
            new_qos[len-1] = '\0';
        strcpy(item->qos_assignment, new_qos);
    }
}

static void NETACCESS_Backdoor_PortAuthSrvc_MenuCreateMacList(PortAuthSrvAssign_T *all_mac, UI32_T *mac_count)
{
    UI8_T *menu = {NEWLINE"\t================ Create MACs ================"
                                NEWLINE"\tc. Create a new MAC"
                                NEWLINE"\t0. Exit"NEWLINE};

    NETACCESS_Backdoor_PortAuthSrvc_ClearMacList(all_mac);
    *mac_count = 0;

    while (1)
    {   /* Create MAC menu */
        UI8_T line[MAX_KEY_IN_STRING_MENU];
        UI32_T i, choice;
        PortAuthSrvAssign_T *item;

        BACKDOOR_MGR_Printf("%s", menu);
        for (i=0; i < *mac_count; i++)
        {
            item = NETACCESS_Backdoor_PortAuthSrvc_GetMacItem(all_mac, i);
            if (item)
                BACKDOOR_MGR_Printf(NEWLINE"\t%ld. MAC%ld [%s][%s]", i+1, i+1, item->vlan_assignment, item->qos_assignment);
        }
        BACKDOOR_MGR_Printf(NEWLINE NEWLINE"\tChoose : ");
        memset(line, 0, sizeof(line));
        BACKDOOR_MGR_RequestKeyIn(line, sizeof(line));

        if (line[0] == '\r' || line[0] == '\n')
            continue;
        if (line[0] == 'c' || line[0] == 'C')
        {
            if (*mac_count == 0)
            {
                memset(all_mac, 0, sizeof(PortAuthSrvAssign_T));
                (*mac_count)++; /*use the first node. */
            }
            else if (NETACCESS_Backdoor_PortAuthSrvc_AppendMac(all_mac))
                (*mac_count) ++;
        }
        else
        {
            choice = atoi(line);
            if (choice < 0 || choice > *mac_count)
                continue;
            if (choice == 0)
                return;

            choice--;
            if (choice <= 0)
                choice = 0;
            item = NETACCESS_Backdoor_PortAuthSrvc_GetMacItem(all_mac, choice);
            if (item)
                NETACCESS_Backdoor_PortAuthSrvc_SetMacItem(item, choice);
        }
    }/* End of while */
}/* End of NETACCESS_Backdoor_PortAuthSrvc_MenuCreateMacList */

static void NETACCESS_Backdoor_PortAuthSrvc_TestVlanQosAssignment(int argc, char** argv)
{
    UI32_T choice, mac_count;
    PortAuthSrvAssign_T all_mac;
    PortAuthSrvAssign_T mac_bak;

    UI8_T *menu = {NEWLINE"================ PortAuthSrv Menu ================"
                                NEWLINE"  1. Create VLAN and QoS Profile for MACs"
                                NEWLINE"  2. Display MAC info"
                                NEWLINE"  3. Check VLAN Validity"
                                NEWLINE"  4. Check QoS Validity"
                                NEWLINE"  5. Set VLAN to Oper"
                                NEWLINE"  6. Set QoS to Oper"
                                NEWLINE"  7. Set VLAN to Admin"
                                NEWLINE"  8. Set QoS to Admin"
                                NEWLINE"  0. Quit"};

    mac_count = 0;
    memset(&all_mac, 0, sizeof(all_mac));

    while(1)
    {   /* main menu */
        UI8_T input_string[MAX_KEY_IN_STRING_MENU];

        BACKDOOR_MGR_Printf("%s"NEWLINE, menu);
        BACKDOOR_MGR_Printf(NEWLINE"  [Main Menu] Enter a choice : ");
        memset(input_string, 0, sizeof(input_string));
        BACKDOOR_MGR_RequestKeyIn(input_string, MAX_KEY_IN_STRING_MENU);
        if(input_string[0] > '9' || input_string[0] < '0')
           choice = 99;
        else
            choice = atoi(input_string);

        switch(choice)
        {
            case 0:
                NETACCESS_Backdoor_PortAuthSrvc_ClearMacList(&all_mac);
                return;

            case 1:
                NETACCESS_Backdoor_PortAuthSrvc_MenuCreateMacList(&all_mac, &mac_count);
                break;

            case 2:
                NETACCESS_Backdoor_PortAuthSrvc_DisplayMac(&all_mac, mac_count);
                break;

            case 3:
                if(PORTAUTHSRVC_MGR_CheckVlanValidity(&all_mac))
                    BACKDOOR_MGR_Printf(NEWLINE"  Vlan Validity: OK."NEWLINE);
                else
                    BACKDOOR_MGR_Printf(NEWLINE"  Vlan Validity: Not OK."NEWLINE);
                break;

            case 4:
                if(PORTAUTHSRVC_MGR_CheckQoSValidity(&all_mac))
                    BACKDOOR_MGR_Printf(NEWLINE"  QoS Validity: OK."NEWLINE);
                else
                    BACKDOOR_MGR_Printf(NEWLINE"  QoS Validity: Not OK."NEWLINE);
                break;

            case 5:
            {
                UI32_T port_no;
                UI8_T input_string[MAX_KEY_IN_STRING_MENU];

                BACKDOOR_MGR_Printf(NEWLINE NEWLINE"  Enter Port number: ");
                memset(input_string, 0, sizeof(input_string));
                BACKDOOR_MGR_RequestKeyIn(input_string, sizeof(input_string));
                port_no = atoi(input_string);

                all_mac.port = port_no;
                memcpy(&mac_bak, &all_mac, sizeof(PortAuthSrvAssign_T));    /* preserve the origioal data */

                if(PORTAUTHSRVC_MGR_SetVlanToOper(&all_mac) == FALSE)
                    BACKDOOR_MGR_Printf(NEWLINE"  Set Vlan To Oper: FAILED."NEWLINE);
                else
                {
                    if(strcmp(all_mac.vlan_assignment, "-0Default0-") == 0)
                        BACKDOOR_MGR_Printf(NEWLINE"  All MACs contain null VLANs. Didn't change the VLAN of the port."
                                    NEWLINE"  Auto Vlan: %s"NEWLINE, all_mac.vlan_assignment);
                    else if(all_mac.vlan_assignment[0] == NULL)
                        BACKDOOR_MGR_Printf(NEWLINE"  The subset of VLANs for all MACs is empty."
                                    NEWLINE"  Auto Vlan: %s"NEWLINE, all_mac.vlan_assignment);
                    else
                        BACKDOOR_MGR_Printf(NEWLINE"  Auto Vlan Subset is: %s"NEWLINE, all_mac.vlan_assignment);
                }

                memcpy(&all_mac, &mac_bak, sizeof(PortAuthSrvAssign_T)); /* restore the origional data */
                break;
            }

            case 6:
            {
                UI32_T port_no;
                UI8_T input_string[MAX_KEY_IN_STRING_MENU];

                BACKDOOR_MGR_Printf(NEWLINE NEWLINE"  Enter Port number: ");
                memset(input_string, 0, sizeof(input_string));
                BACKDOOR_MGR_RequestKeyIn(input_string, sizeof(input_string));
                port_no = atoi(input_string);

                all_mac.port = port_no;
                memcpy(&mac_bak, &all_mac, sizeof(PortAuthSrvAssign_T));   /* preserve the origioal data */

                if(PORTAUTHSRVC_MGR_SetQosToOper(&all_mac))
                    BACKDOOR_MGR_Printf(NEWLINE"  Auto Vlan Subset is: %s"NEWLINE, all_mac.vlan_assignment);
                else
                    BACKDOOR_MGR_Printf(NEWLINE"  Set QoS to Oper: FAILED."NEWLINE);

                memcpy(&all_mac, &mac_bak, sizeof(PortAuthSrvAssign_T)); /* restore the origional data */
                break;
            }

            case 7:
            {
                UI32_T port_no;
                UI8_T input_string[MAX_KEY_IN_STRING_MENU];

                BACKDOOR_MGR_Printf(NEWLINE NEWLINE"  Enter Port number: ");
                memset(input_string, 0, sizeof(input_string));
                BACKDOOR_MGR_RequestKeyIn(input_string, sizeof(input_string));
                port_no = atoi(input_string);

                all_mac.port = port_no;
                if(PORTAUTHSRVC_MGR_SetVlanToAdmin(&all_mac) == FALSE)
                    BACKDOOR_MGR_Printf(NEWLINE"  Set Vlan To Admin: FAILED."NEWLINE);
                else
                    BACKDOOR_MGR_Printf(NEWLINE"  Set Vlan To Admin: SUCCEED."NEWLINE);

                break;
            }

            case 8:
            {
                UI32_T port_no;
                UI8_T input_string[MAX_KEY_IN_STRING_MENU];

                BACKDOOR_MGR_Printf(NEWLINE NEWLINE"  Enter Port number: ");
                memset(input_string, 0, sizeof(input_string));
                BACKDOOR_MGR_RequestKeyIn(input_string, sizeof(input_string));
                port_no = atoi(input_string);

                all_mac.port = port_no;
                if(PORTAUTHSRVC_MGR_SetQosToAdmin(&all_mac) == FALSE)
                    BACKDOOR_MGR_Printf(NEWLINE"  Set QoS To Admin: FAILED."NEWLINE);
                else
                    BACKDOOR_MGR_Printf(NEWLINE"  Set QoS To Admin: SUCCEED."NEWLINE);

                break;
            }

            case 99:
                break;

            default:
                BACKDOOR_MGR_Printf("Incorrect choice");
                break;
        }/* End of switch case */
    }/* End of while */
}

#endif /* PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

static void NETACCESS_Backdoor_NetAccess_SetDebugFlag(int argc, char** argv)
{
    UI32_T  debug_flag = NETACCESS_OM_GetDebugFlag();
    char    ch;

    /* if hold the L_THREADGRP_Execution too long,
     *   the system will hang up...
     */
    L_THREADGRP_Execution_Release(curr_tg_handle, backdoor_member_id);

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\nNetwork-Access Debug type:\r\n");
        BACKDOOR_MGR_Printf(" 1. VM ERROR             : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_VM_ERR) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 2. VM RESULT            : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_VM_RST) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 3. VM INFORMATION       : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_VM_IFO) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 4. VM TRACE             : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_VM_TRC) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 5. VM TIMER             : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_VM_TMR) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 6. OM ERROR             : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_OM_ERR) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 7. OM INFORMATION       : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_OM_IFO) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" 8. OM TRACE             : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_OM_TRC) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" a. MGR ERROR            : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_MG_ERR) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" b. MGR INFORMATION      : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_MG_IFO) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" c. MGR TRACE            : %s \r\n", (debug_flag&NETACCESS_OM_DEBUG_MG_TRC) ? "ON": "OFF");
        BACKDOOR_MGR_Printf(" f. Show function(option): %s \r\n", (debug_flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE) ? "ON": "OFF");
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf(" e. All ERROR          v. Vitual Mang \r\n");
        BACKDOOR_MGR_Printf(" i. All INFORMATION    o. Object Mang \r\n");
        BACKDOOR_MGR_Printf(" s. All TRACE          m. Management  \r\n");
        BACKDOOR_MGR_Printf(" d. All Debug Message(except VM TMR)  \r\n");
        BACKDOOR_MGR_Printf(" D. All Debug Message                 \r\n");
        BACKDOOR_MGR_Printf(" r. Clear All Debug Message           \r\n");
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("Press x or q to exit                  \r\n");
        BACKDOOR_MGR_Printf("Enter your choice: ");

        ch = BACKDOOR_MGR_GetChar();

        switch(ch)
        {
            case '1':/*vm error*/
                if (debug_flag&NETACCESS_OM_DEBUG_VM_ERR)
                    debug_flag &= ~NETACCESS_OM_DEBUG_VM_ERR;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_VM_ERR;
                break;

            case '2':/*vm result*/
                if (debug_flag&NETACCESS_OM_DEBUG_VM_RST)
                    debug_flag &= ~NETACCESS_OM_DEBUG_VM_RST;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_VM_RST;
                break;

            case '3':/*vm information*/
                if (debug_flag&NETACCESS_OM_DEBUG_VM_IFO)
                    debug_flag &= ~NETACCESS_OM_DEBUG_VM_IFO;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_VM_IFO;
                break;

            case '4':/*vm trace*/
                if (debug_flag&NETACCESS_OM_DEBUG_VM_TRC)
                    debug_flag &= ~NETACCESS_OM_DEBUG_VM_TRC;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_VM_TRC;
                break;

            case '5':/*vm timer*/
                if (debug_flag&NETACCESS_OM_DEBUG_VM_TMR)
                    debug_flag &= ~NETACCESS_OM_DEBUG_VM_TMR;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_VM_TMR;
                break;

            case '6':/*om error*/
                if (debug_flag&NETACCESS_OM_DEBUG_OM_ERR)
                    debug_flag &= ~NETACCESS_OM_DEBUG_OM_ERR;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_OM_ERR;
                break;

            case '7':/*om information*/
                if (debug_flag&NETACCESS_OM_DEBUG_OM_IFO)
                    debug_flag &= ~NETACCESS_OM_DEBUG_OM_IFO;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_OM_IFO;
                break;

            case '8':/*om trace*/
                if (debug_flag&NETACCESS_OM_DEBUG_OM_TRC)
                    debug_flag &= ~NETACCESS_OM_DEBUG_OM_TRC;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_OM_TRC;
                break;

            case 'a':/*mgr error*/
            case 'A':
                if (debug_flag&NETACCESS_OM_DEBUG_MG_ERR)
                    debug_flag &= ~NETACCESS_OM_DEBUG_MG_ERR;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_MG_ERR;
                break;

            case 'b':/*mgr information*/
            case 'B':
                if (debug_flag&NETACCESS_OM_DEBUG_MG_IFO)
                    debug_flag &= ~NETACCESS_OM_DEBUG_MG_IFO;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_MG_IFO;
                break;

            case 'c':/*mgr trace*/
            case 'C':
                if (debug_flag&NETACCESS_OM_DEBUG_MG_TRC)
                    debug_flag &= ~NETACCESS_OM_DEBUG_MG_TRC;
                else
                    debug_flag |= NETACCESS_OM_DEBUG_MG_TRC;
                break;

            case 'e':/*all error*/
            case 'E':
                debug_flag |= NETACCESS_OM_DEBUG_VM_ERR|NETACCESS_OM_DEBUG_OM_ERR|NETACCESS_OM_DEBUG_MG_ERR;
                break;

            case 'i':/*all information*/
            case 'I':
                debug_flag |= NETACCESS_OM_DEBUG_VM_IFO|NETACCESS_OM_DEBUG_OM_IFO|NETACCESS_OM_DEBUG_MG_IFO;
                break;

            case 's':/*all trace*/
            case 'S':
                debug_flag |= NETACCESS_OM_DEBUG_VM_TRC|NETACCESS_OM_DEBUG_OM_TRC|NETACCESS_OM_DEBUG_MG_TRC;
                break;

            case 'v':/*vm*/
            case 'V':
                debug_flag |= NETACCESS_OM_DEBUG_VM_ERR|NETACCESS_OM_DEBUG_VM_RST|NETACCESS_OM_DEBUG_VM_IFO|NETACCESS_OM_DEBUG_VM_TRC|NETACCESS_OM_DEBUG_VM_TMR;
                break;

            case 'o':/*om*/
            case 'O':
                debug_flag |= NETACCESS_OM_DEBUG_OM_ERR|NETACCESS_OM_DEBUG_OM_IFO|NETACCESS_OM_DEBUG_OM_TRC;
                break;

            case 'm':/*mgr*/
            case 'M':
                debug_flag |= NETACCESS_OM_DEBUG_MG_ERR|NETACCESS_OM_DEBUG_MG_IFO|NETACCESS_OM_DEBUG_MG_TRC;
                break;

            case 'D':/*all message*/
                debug_flag |= NETACCESS_OM_DEBUG_VM_TMR;

            case 'd':/*all message except vm trm*/
                debug_flag |=  NETACCESS_OM_DEBUG_VM_ERR|NETACCESS_OM_DEBUG_VM_RST|NETACCESS_OM_DEBUG_VM_IFO|NETACCESS_OM_DEBUG_VM_TRC
                              |NETACCESS_OM_DEBUG_OM_ERR|NETACCESS_OM_DEBUG_OM_IFO|NETACCESS_OM_DEBUG_OM_TRC
                              |NETACCESS_OM_DEBUG_MG_ERR|NETACCESS_OM_DEBUG_MG_IFO|NETACCESS_OM_DEBUG_MG_TRC;
                break;

            case 'f':
            case 'F':
                if (debug_flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE)
                    debug_flag &= ~SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE;
                else
                    debug_flag |= SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE;
                break;

            case 'r':/*clear all*/
            case 'R':
                debug_flag = 0;
                break;

            default:
                break;
        } /* switch */

        if (ch == 'x' || ch == 'X' || ch == 'q' || ch == 'Q')
        {
            L_THREADGRP_Execution_Request(curr_tg_handle, backdoor_member_id);

            if(NETACCESS_OM_SetDebugFlag(debug_flag) == FALSE)
            {
                BACKDOOR_MGR_Printf("\r\nfailed to set debug flag");
            }
            return;
        }
    } /* while */
}

static void NETACCESS_Backdoor_NetAccess_SetSecurePortMode(int argc, char** argv)
{
    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {port mode}");
        BACKDOOR_MGR_Printf("\r\n\tport mode:");
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (noRestrictions)", NETACCESS_PORTMODE_NO_RESTRICTIONS);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (continuousLearning)", NETACCESS_PORTMODE_CONTINUOS_LEARNING);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (autoLearn)", NETACCESS_PORTMODE_AUTO_LEARN);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (secure)", NETACCESS_PORTMODE_SECURE);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (userLogin)", NETACCESS_PORTMODE_USER_LOGIN);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (userLoginSecure)", NETACCESS_PORTMODE_USER_LOGIN_SECURE);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (userLoginWithOUI)", NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (macAddressWithRadius)", NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (macAddressOrUserLoginSecure)", NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (macAddressElseUserLoginSecure)", NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (macAuthentication)", NETACCESS_PORTMODE_MAC_AUTHENTICATION);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (portSecurity)", NETACCESS_PORTMODE_PORT_SECURITY);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (dot1x)", NETACCESS_PORTMODE_DOT1X);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetSecurePortMode(atoi(argv[0]), atoi(argv[1])))
        BACKDOOR_MGR_Printf("\r\nfailed to set port mode");
}

static void NETACCESS_Backdoor_NetAccess_SetSecureNumberAddresses(int argc, char** argv)
{
    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {number}");
        return;
    }

    if (FALSE == NETACCESS_MGR_SetSecureNumberAddresses(atoi(argv[0]), atoi(argv[2])))
        BACKDOOR_MGR_Printf("\r\nfailed to set secure address number");
}

static void NETACCESS_Backdoor_NetAccess_SetDynamicVlanStatus(int argc, char** argv)
{
    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} { %d(disable) | %d(enable) }",
            VAL_networkAccessPortDynamicVlan_disabled,
            VAL_networkAccessPortDynamicVlan_enabled);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetSecureDynamicVlanStatus(atoi(argv[0]), atoi(argv[1])))
        BACKDOOR_MGR_Printf("\r\nfailed to set secure dynamic vlan status");
}

static void NETACCESS_Backdoor_NetAccess_SetDynamicQosStatus(int argc, char** argv)
{
    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} { %d(disable) | %d(enable) }",
            VAL_networkAccessPortLinkDynamicQos_disabled,
            VAL_networkAccessPortLinkDynamicQos_enabled);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetDynamicQosStatus(atoi(argv[0]), atoi(argv[1])))
        BACKDOOR_MGR_Printf("\r\nfailed to set secure dynamic QoS status");
}

static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationStatus(int argc, char** argv)
{
    UI32_T lport, status =0;
    char  input[5];

    if (argc == 0)
    {
        BACKDOOR_MGR_Printf("\r\n Which lport? : ");
        memset(input, 0, sizeof(input));
        BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
        lport = atoi(input);

        do{
            BACKDOOR_MGR_Printf("\r\n Which status? (%d)enabled (%d)disabled : ", NETACCESS_TYPE_MACAUTH_ENABLED, NETACCESS_TYPE_MACAUTH_DISABLED);
            memset(input, 0, sizeof(input));
            BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
            if (input[0] != 0)
            {
                status = atoi(input);
            }
        }while((status != NETACCESS_TYPE_MACAUTH_ENABLED) && (status !=NETACCESS_TYPE_MACAUTH_DISABLED));
    }
    else if (argc == 2)
    {
        lport  = atoi(argv[0]);
        status = atoi(argv[1]);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {status}");
        BACKDOOR_MGR_Printf("\r\n\tstatus:");
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (enabled)", NETACCESS_TYPE_MACAUTH_ENABLED);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (disable)", NETACCESS_TYPE_MACAUTH_DISABLED);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetMacAuthPortStatus(lport, status))
        BACKDOOR_MGR_Printf("\r\nfailed to set mac-authentication status");
}

static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationNumberAddresses(int argc, char** argv)
{
    UI32_T lport, status =0;
    char  input[5];

    if (argc == 0)
    {
        BACKDOOR_MGR_Printf("\r\n Which lport? : ");
        memset(input, 0, sizeof(input));
        BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
        lport = atoi(input);

        do{
            BACKDOOR_MGR_Printf("\r\n Which status? (%d)enabled (%d)disabled : ", NETACCESS_TYPE_MACAUTH_ENABLED, NETACCESS_TYPE_MACAUTH_DISABLED);
            memset(input, 0, sizeof(input));
            BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
            if (input[0] != 0)
            {
                status = atoi(input);
            }
        }while((status != NETACCESS_TYPE_MACAUTH_ENABLED) && (status !=NETACCESS_TYPE_MACAUTH_DISABLED));
    }
    else if (argc == 2)
    {
        lport  = atoi(argv[0]);
        status = atoi(argv[1]);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {number}");
        return;
    }

    if (FALSE == NETACCESS_MGR_SetMacAuthPortMaxMacCount(lport, status))
        BACKDOOR_MGR_Printf("\r\nfailed to set secure address number");
}

static void NETACCESS_Backdoor_NetAccess_SetMacAuthenticationIntrustionAction(int argc, char * * argv)
{
    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {action}");
        BACKDOOR_MGR_Printf("\r\n\tintrustion action:");
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (block)", VAL_macAuthPortIntrusionAction_block_traffic);
        BACKDOOR_MGR_Printf("\r\n\t\t%2d (pass)",  VAL_macAuthPortIntrusionAction_pass_traffic);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetMacAuthPortIntrusionAction(atoi(argv[0]), atoi(argv[1])))
        BACKDOOR_MGR_Printf("\r\nfailed to set mac-authentication intrustion action");
}

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
static void NETACCESS_Backdoor_NetAccess_SetMacAddressAgingMode(int argc, char ** argv)
{
    UI32_T aging_mode = 1;
    char  input[5];

    if (argc != 1)
    {
        do{
            BACKDOOR_MGR_Printf("\r\n Which aging mode? (%d)dynamic (%d)static : ", VAL_networkAccessAging_enabled, VAL_networkAccessAging_disabled);
            memset(input, 0, sizeof(input));
            BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
            if (input[0] != 0)
            {
                aging_mode = atoi(input);
            }
        }while((aging_mode != VAL_networkAccessAging_enabled) && (aging_mode !=VAL_networkAccessAging_disabled));
    }

    if (FALSE == NETACCESS_MGR_SetMacAddressAgingMode(aging_mode))
        BACKDOOR_MGR_Printf("\r\n failed to set MAC address aging mode");
}
#endif /*#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)*/

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
static void NETACCESS_Backdoor_NetAccess_SetLinkDetection(int argc, char** argv)
{
    UI32_T lport, status, mode, action;
    char  input[2];

    BACKDOOR_MGR_Printf("\r\n Which lport? : ");
    memset(input, 0, sizeof(input));
    BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
    lport = atoi(input);

    BACKDOOR_MGR_Printf("\r\n Which status(%lu: enable, %lu: disable)? : ",
        VAL_networkAccessPortLinkDetectionStatus_enabled,
        VAL_networkAccessPortLinkDetectionStatus_disabled);
    memset(input, 0, sizeof(input));
    BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
    status = atoi(input);

    BACKDOOR_MGR_Printf("\r\n Which mode(%lu: link-up, %lu: link-down, %lu: link-up-down)? : ",
        VAL_networkAccessPortLinkDetectionMode_linkUp,
        VAL_networkAccessPortLinkDetectionMode_linkDown,
        VAL_networkAccessPortLinkDetectionMode_linkUpDown);
    memset(input, 0, sizeof(input));
    BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
    mode = atoi(input);

    BACKDOOR_MGR_Printf("\r\n Which action(%lu: trap, %lu: shutdown, %lu: trap and shutdown)? : ",
        VAL_networkAccessPortLinkDetectionAciton_trap,
        VAL_networkAccessPortLinkDetectionAciton_shutdown,
        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown);
    memset(input, 0, sizeof(input));
    BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
    action = atoi(input);

    if (FALSE == NETACCESS_MGR_SetLinkDetectionStatus(lport, status))
        BACKDOOR_MGR_Printf("\r\nfailed to set link detection status");
    if (FALSE == NETACCESS_MGR_SetLinkDetectionMode(lport, mode))
        BACKDOOR_MGR_Printf("\r\nfailed to set link detection mode");
    if (FALSE == NETACCESS_MGR_SetLinkDetectionAction(lport, action))
        BACKDOOR_MGR_Printf("\r\nfailed to set link detection action");
}
#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

static void NETACCESS_Backdoor_NetAccess_SetSecureAddrRowStatus(int argc, char** argv)
{
#if 0
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN];

    if (argc != 4)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {unit} {port} {mac} {row status}"),
        BACKDOOR_MGR_Printf("\r\n\tmac format: XX-XX-XX-XX-XX-XX");
        BACKDOOR_MGR_Printf("\r\n\trow status:");
        BACKDOOR_MGR_Printf("\r\n\t\t(%d) active", NETACCESS_ROWSTATUS_ACTIVE);
        BACKDOOR_MGR_Printf("\r\n\t\t(%d) destroy", NETACCESS_ROWSTATUS_DESTROY);
        BACKDOOR_MGR_Printf("\r\n\t\t(%d) createAndG", NETACCESS_ROWSTATUS_CREATE_AND_GO);
        BACKDOOR_MGR_Printf("\r\n\t\t(%d) createAndWait", NETACCESS_ROWSTATUS_CREATE_AND_WAIT);
        BACKDOOR_MGR_Printf("\r\n\t\t(%d) notInServic", NETACCESS_ROWSTATUS_NOT_IN_SERVICE);
        return;
    }

    if (FALSE == NETACCESS_Backdoor_Str2MacAddr(argv[2], mac))
        return;

    if (FALSE == NETACCESS_MGR_SetSecureAddrRowStatus(atoi(argv[0]), atoi(argv[1]), mac, atoi(argv[3])))
        BACKDOOR_MGR_Printf("\r\nfailed to set secure address row status");
#endif
}

static void NETACCESS_Backdoor_NetAccess_ShowSecurePort(int argc, char** argv)
{
    UI32_T      status;
    NETACCESS_PortMode_T port_mode;
    UI32_T      result;
    UI32_T      unit, port, lport, trunk_id;
    UI32_T      used_buffer;

    NETACCESS_MGR_SecurePortEntry_T port_entry;

    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport}");
        return;
    }

    lport = atoi(argv[0]);

    if (TRUE == NETACCESS_MGR_GetSecurePortMode(lport, &port_mode))
    {
        BACKDOOR_MGR_Printf("\r\nport mode: %s",
            (NETACCESS_PORTMODE_NO_RESTRICTIONS== port_mode) ? "noRestrictions" :
            (NETACCESS_PORTMODE_CONTINUOS_LEARNING== port_mode) ? "continuousLearning" :
            (NETACCESS_PORTMODE_AUTO_LEARN== port_mode) ? "autoLearn" :
            (NETACCESS_PORTMODE_SECURE== port_mode) ? "secure" :
            (NETACCESS_PORTMODE_USER_LOGIN== port_mode) ? "userLogin" :
            (NETACCESS_PORTMODE_USER_LOGIN_SECURE== port_mode) ? "userLoginSecure" :
            (NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI== port_mode) ? "userLoginWithOUI" :
            (NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS== port_mode) ? "macAddressWithRadius" :
            (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE== port_mode) ? "macAddressOrUserLoginSecure" :
            (NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE == port_mode) ? "macAddressElseUserLoginSecure" :
            (NETACCESS_PORTMODE_MAC_AUTHENTICATION== port_mode) ? "macAuthentication" :
            (NETACCESS_PORTMODE_PORT_SECURITY== port_mode) ? "portSecurity" :
            (NETACCESS_PORTMODE_DOT1X== port_mode) ? "dot1x" :
            "unknown");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nfailed to get secure port mode");
    }

    if (TRUE == NETACCESS_MGR_GetSecureNumberAddresses(lport, &result))
        BACKDOOR_MGR_Printf("\r\nsecure number address: %lu", result);
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get secure address number");

    if (TRUE == NETACCESS_MGR_GetSecureNumberAddressesStored(lport, &result))
    {
        BACKDOOR_MGR_Printf("\r\nsecure number address stored: %lu", result);
        if (    (NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE == port_mode)
             || (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == port_mode)
           )
        {
           BACKDOOR_MGR_Printf(" (dot1x:%lu mac-auth:%lu)",
            NETACCESS_MGR_GetSecureNumberAddressesStoredByDot1x(lport),
            NETACCESS_MGR_GetSecureNumberAddressesStoredByRada(lport));
        }
    }
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get secure address number stored");

    if (TRUE == NETACCESS_MGR_GetSecureMaximumAddresses(lport, &result))
        BACKDOOR_MGR_Printf("\r\nsecure maximun number address: %lu", result);
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get secure maxumun address number");

    if (0 == NETACCESS_MGR_GetSecurePortEntry(NETACCESS_FID_SECURE_PORT_ENTRY_IFINDEX, &lport, &port_entry.lport, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nlport: %lu", port_entry.lport);
    }

    if (0 == NETACCESS_MGR_GetSecurePortEntry(NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_AUTHORIZED_ADDRESSES, &lport, &port_entry.nbr_of_authorized_addresses, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nsecure number authorized address: %lu", port_entry.nbr_of_authorized_addresses);
    }

    if (0 == NETACCESS_MGR_GetSecurePortEntry(NETACCESS_FID_SECURE_PORT_ENTRY_NBR_OF_LEARN_AUTHORIZED_ADDRESSES, &lport, &port_entry.nbr_of_learn_authorized_addresses, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nsecure number learnt authorized address: %lu", port_entry.nbr_of_learn_authorized_addresses);
    }

    if (SWCTRL_LPORT_NORMAL_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        if (TRUE == NETACCESS_MGR_GetSecureDynamicVlanStatus(lport, &status))
        {
            BACKDOOR_MGR_Printf("\r\nDynamic VLAN status: %s",
                (VAL_networkAccessPortDynamicVlan_enabled == status) ?
                "true" : "false");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get dynamic VLAN status");

        if (TRUE == NETACCESS_MGR_GetDynamicQosStatus(lport, &status))
        {
            BACKDOOR_MGR_Printf("\r\nDynamic QoS status: %s",
                (VAL_networkAccessPortLinkDynamicQos_enabled== status) ?
                "true" : "false");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get dynamic QoS status");

        if (TRUE == NETACCESS_MGR_GetSecureGuestVlanId(lport, &result))
        {
            BACKDOOR_MGR_Printf("\r\nGuest VLAN ID: %lu", result);
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get guest VLAN ID");

#if (SYS_CPNT_DOT1X == TRUE)
        if (TRUE == NETACCESS_MGR_GetDot1xPortIntrusionAction(lport, &result))
        {
            BACKDOOR_MGR_Printf("\r\nDot1x intrusion action: %s",
                (result==VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic)?"block-traffic":
                (result==VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan)?"guest-vlan": "???");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to dot1x intrusion action");
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        if (TRUE == NETACCESS_MGR_GetLinkDetectionStatus(lport, &result))
        {
            BACKDOOR_MGR_Printf("\r\nLink-detection status: %s",
                (VAL_networkAccessPortLinkDetectionStatus_enabled == result)?"enabled" :
                (VAL_networkAccessPortLinkDetectionStatus_disabled == result)?"disabled" :
                "???");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get link-detection status");

        if (TRUE == NETACCESS_MGR_GetLinkDetectionMode(lport, &result))
        {
            BACKDOOR_MGR_Printf("\r\nLink-detection status: %s",
                (VAL_networkAccessPortLinkDetectionMode_linkUp == result)?"link-up" :
                (VAL_networkAccessPortLinkDetectionMode_linkDown == result)?"link-down" :
                (VAL_networkAccessPortLinkDetectionMode_linkUpDown == result)?"link-up-down" :
                "???");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get link-detection mode");

        if (TRUE == NETACCESS_MGR_GetLinkDetectionAction(lport, &result))
        {
            BACKDOOR_MGR_Printf("\r\nLink-detection status: %s",
                (VAL_networkAccessPortLinkDetectionAciton_trap == result)?"trap" :
                (VAL_networkAccessPortLinkDetectionAciton_shutdown == result)?"shutdown" :
                (VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown == result)?"trap-and-shutdown" :
                "???");
        }
        else
            BACKDOOR_MGR_Printf("\r\nfailed to get link-detection mode");
#endif
    }

    if (TRUE == NETACCESS_MGR_GetNextSecurePortEntry(NETACCESS_FID_SECURE_PORT_ENTRY_IFINDEX, &lport, &port_entry.lport, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nnext SecurePortEntry lport: %lu", port_entry.lport);
    }
    else
        BACKDOOR_MGR_Printf("\r\nno more secure port entry");

    /* show mac-authentication information */
    if (    (port_mode == NETACCESS_PORTMODE_MAC_AUTHENTICATION)
         || (port_mode == NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE)
         || (port_mode == NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE)
       )
    {
        UI32_T used_buffer;
        NETACCESS_MGR_MacAuthPortEntry_T mac_auth_entry;

        BACKDOOR_MGR_Printf("\r\nMAC authentication information:");
        if (TRUE == NETACCESS_MGR_GetMacAuthPortEntry(SYS_TYPE_FID_ALL, &lport, &mac_auth_entry, sizeof(mac_auth_entry), &used_buffer))
        {
            BACKDOOR_MGR_Printf("\r\nIntrustion Action: %s", (mac_auth_entry.intrusion_action == VAL_macAuthPortIntrusionAction_block_traffic) ?
                "block" : "pass");
            BACKDOOR_MGR_Printf("\r\nMaximun number address: %lu", mac_auth_entry.configured_number_addresses);
        }
    }
}

static void NETACCESS_Backdoor_NetAccess_ShowStateMachine(int argc, char** argv)
{
    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport}");
        return;
    }

    if (FALSE == NETACCESS_MGR_ShowStateMachineStatus(atoi(argv[0])))
        BACKDOOR_MGR_Printf("\r\nfailed to show state machine");
}

static void NETACCESS_Backdoor_NetAccess_ShowSecureAddress(int argc, char** argv)
{
    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {mac index}");
        return;
    }

    if (FALSE == NETACCESS_MGR_ShowSecureAddressByIndex(atoi(argv[0])))
        BACKDOOR_MGR_Printf("\r\nfailed to show secure address");
}

static void NETACCESS_Backdoor_NetAccess_ShowNextSecureAddressEntry(int argc, char** argv)
{
    NETACCESS_MGR_SecureAddressEntryKey_T key;
    NETACCESS_MGR_SecureAddressEntry_T  entry;
    UI32_T used_buffer;

    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {mac}"),
        BACKDOOR_MGR_Printf("\r\n\tmac format: XX-XX-XX-XX-XX-XX");
        return;
    }

    key.lport = atoi(argv[0]);
    if (FALSE == NETACCESS_Backdoor_Str2MacAddr(argv[1], key.mac_address))
        return;

    if (0 == NETACCESS_MGR_GetNextSecureAddressEntry(NETACCESS_FID_SECURE_ADDRESS_ENTRY_IFINDEX, &key, &entry.addr_lport, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nlport: %lu", entry.addr_lport);
    }
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get next secure address");

    if (0 == NETACCESS_MGR_GetNextSecureAddressEntry(NETACCESS_FID_SECURE_ADDRESS_ENTRY_MAC, &key, &entry.addr_MAC, SYS_ADPT_MAC_ADDR_LEN, &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\n       mac: ");
        NETACCESS_Backdoor_ShowMacAddr(entry.addr_MAC);
    }
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get next secure address");


    if (0 == NETACCESS_MGR_GetNextSecureAddressEntry(NETACCESS_FID_SECURE_ADDRESS_ENTRY_ROW_STATUS, &key, &entry.addr_row_status, sizeof(UI32_T), &used_buffer))
    {
        BACKDOOR_MGR_Printf("\r\nrow status: ");
        NETACCESS_Backdoor_ShowRowStatus(entry.addr_row_status);
    }
    else
        BACKDOOR_MGR_Printf("\r\nfailed to get next secure address");

}

static void NETACCESS_Backdoor_NetAccess_ShowAllSecureAddressByPort(int argc, char** argv)
{
    UI32_T  used_buffer, org_port;
    NETACCESS_MGR_SecureAddressEntryKey_T key;
    NETACCESS_MGR_SecureAddressEntry_T  entry;
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN];

    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport}");
        return;
    }

    org_port = key.lport = atoi(argv[0]);
    memset(key.mac_address, 0, sizeof(mac));

    BACKDOOR_MGR_Printf("\r\nlport: %lu", org_port);
    BACKDOOR_MGR_Printf("\r\n\tMAC Address       row status");
    BACKDOOR_MGR_Printf("\r\n\t----------------- -----------------");
    while (0 == NETACCESS_MGR_GetNextSecureAddressEntry(SYS_TYPE_FID_ALL, &key, &entry, sizeof(NETACCESS_MGR_SecureAddressEntry_T), &used_buffer))
    {
        if (org_port != key.lport)
            break;

        BACKDOOR_MGR_Printf("\r\n\t");
        NETACCESS_Backdoor_ShowMacAddr(entry.addr_MAC);
        BACKDOOR_MGR_Printf(" ");
        NETACCESS_Backdoor_ShowRowStatus(entry.addr_row_status);
    }
}

static void NETACCESS_Backdoor_NetAccess_CreateNewMacMsg(int argc, char** argv)
{
    NETACCESS_NEWMAC_DATA_T     newmac_data;
    NETACCESS_NEWMAC_MSGQ_T     msg;

    if (argc != 5)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {src mac} {reason} {is tag} {vid}");
        BACKDOOR_MGR_Printf("\r\n\tmac format: XX-XX-XX-XX-XX-XX");
        BACKDOOR_MGR_Printf("\r\n\treason: 1(intruder) 2(port move)");
        BACKDOOR_MGR_Printf("\r\n\tis tag: 0(FALSE) other(TRUE)");
        return;
    }

    memset(&msg, 0, sizeof(msg));
    msg.m_newmac_data = &newmac_data;

    newmac_data.lport = atoi(argv[0]);

    if (FALSE == NETACCESS_Backdoor_Str2MacAddr(argv[1], newmac_data.src_mac))
        return;

    newmac_data.reason = atoi(argv[2]);
    newmac_data.is_tag_packet = (0 == atoi(argv[3])) ? FALSE : TRUE;
    newmac_data.vid = atoi(argv[4]);;

    if (FALSE == NETACCESS_MGR_ProcessNewMacMsg(&msg))
        BACKDOOR_MGR_Printf("\r\nfailed to process new mac msg");
}

static void NETACCESS_Backdoor_NetAccess_SetGuestVlanId(int argc, char** argv)
{
    UI32_T lport, vid;
    char  input[5];

    if (argc == 0)
    {
        BACKDOOR_MGR_Printf("\r\n Which lport? : ");
        memset(input, 0, sizeof(input));
        BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
        lport = atoi(input);

        BACKDOOR_MGR_Printf("\r\n Which VLAN ID? : ");
        memset(input, 0, sizeof(input));
        BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
        vid = atoi(input);
    }
    else if (argc == 2)
    {
        lport  = atoi(argv[0]);
        vid    = atoi(argv[1]);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {guest_vlan_id}");
        return;
    }

    if (FALSE == NETACCESS_MGR_SetSecureGuestVlanId(lport, vid))
        BACKDOOR_MGR_Printf("\r\nfailed to set guest VLAN ID");
}

static void NETACCESS_Backdoor_NetAccess_SetDot1xIntrustion(int argc, char** argv)
{
    UI32_T lport, action;
    char  input[5];

    if (argc == 0)
    {
        BACKDOOR_MGR_Printf("\r\n Which lport? : ");
        memset(input, 0, sizeof(input));
        BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
        lport = atoi(input);

        do{
            BACKDOOR_MGR_Printf("\r\n Which intrusion action status? (b)lock-traffic (g)uest-vlan: ");
            memset(input, 0, sizeof(input));
            BACKDOOR_MGR_RequestKeyIn(input, 1);
        }while(input[0]!='b' && input[0]!='B' && input[0]!='g' && input[0]!='G');
        if (input[0] == 'b' || input[0] == 'B')
            action = VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic;
        else
            action = VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan;
    }
    else if (argc == 2)
    {
        lport  = atoi(argv[0]);
        if (argv[1][0] == 'b' || argv[1][0] == 'B')
            action = VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic;
        else if (argv[1][0] == 'g' || argv[1][0] == 'G')
            action = VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan;
        else
        {
            BACKDOOR_MGR_Printf("\r\ninvalid prameter: %s", argv[1]);
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {lport} {(b)lock-traffic | (g)uest-vlan}");
        return;
    }

    if (FALSE == NETACCESS_MGR_SetDot1xPortIntrusionAction(lport, action))
        BACKDOOR_MGR_Printf("\r\nfailed to set guest VLAN ID");
}

#if (SYS_CPNT_DOT1X == TRUE)
static void NETACCESS_Backdoor_Dot1x_OpenDebugMessage(int argc, char **argv)
{
    DOT1X_OM_SetDebugFlag(0xFFFFFFFF);
}

static void NETACCESS_Backdoor_Dot1x_CloseDebugMessage(int argc, char **argv)
{
    DOT1X_OM_SetDebugFlag(0);
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
static void NETACCESS_Backdoor_NetAccess_SetEapolFramePassThrough(int argc, char** argv)
{
    UI32_T status = VAL_dot1xEapolPassThrough_disabled;
    char  input[5];

    if (argc == 0)
    {
        do{
            PRINT("\r\n Which status of EAPOL Frame Pass-Through? (%lu)enabled (%lu)disable : ",
                VAL_dot1xEapolPassThrough_enabled, VAL_dot1xEapolPassThrough_disabled);
            memset(input, 0, sizeof(input));
            BACKDOOR_MGR_RequestKeyIn(input, sizeof(input));
            if (input[0] != 0)
            {
                status = atoi(input);
            }
        }while((status != VAL_dot1xEapolPassThrough_enabled) && (status !=VAL_dot1xEapolPassThrough_disabled));
    }
    else if (argc == 1)
    {
        status = atoi(input);
    }
    else
    {
        PRINT("\r\nparameters: {status}");
        PRINT("\r\n\tstatus:");
        PRINT("\r\n\t\t%2lu (enabled)", VAL_dot1xEapolPassThrough_enabled);
        PRINT("\r\n\t\t%2lu (disabled)",  VAL_dot1xEapolPassThrough_disabled);
        return;
    }

    if (FALSE == NETACCESS_MGR_SetDot1xEapolPassThrough(status))
    {
        PRINT("\r\nNETACCESS_MGR_SetDot1xEapolFramesPassThrough() failed\r\n");
    }
}
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_Backdoor_CleanWarningDummyFunc
 *-------------------------------------------------------------------------
 * PURPOSE  : to avoid compiler warnings because some backdoor may be disabled and static functions may not be used
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void NETACCESS_Backdoor_CleanWarningDummyFunc()
{
    NETACCESS_Backdoor_ShowIpAddr(NULL);
    NETACCESS_Backdoor_Str2IpAddr(NULL, NULL);
    NETACCESS_Backdoor_ShowMacAddr(NULL);
    NETACCESS_Backdoor_Str2MacAddr(NULL, NULL);
    NETACCESS_Backdoor_ShowRowStatus(0);
    NETACCESS_Backdoor_Help(0, NULL);
    NETACCESS_Backdoor_StrCmpIgnoreCase(NULL, NULL);
    NETACCESS_Backdoor_ChopString(NULL);
}


#endif /* SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE */
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
