/* MODULE NAME: security_backdoor.h
 * PURPOSE:
 *  Implement security backdoor
 *
 * NOTES:
 *
 * History:
 *    2004/07/05 : mfhorng      Create this file
 *    2004/10/16 : mfhorng      add portauthsrvc backdoor
 *
 * Copyright(C)      Accton Corporation, 2004
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "security_backdoor.h"

#if (SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#include "aaa_om_private.h"
#include "aaa_om.h"
#endif

#include "radius_mgr.h"
#include "tacacs_mgr.h"
#include "tacacs_om.h"
#include "userauth.h"
#include "l_threadgrp.h"
#include "radius_om.h"

#include "auth_protocol_proc_comm.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SECURITY_BACKDOOR_PROMPT    "\r\ncommand > "
#define NEWLINE		                "\x0A\x0D"

/* MACRO FUNCTION DECLARATIONS
 */
#define GET_TOKEN_AND_MOVE_NEXT(str, token, pos) { \
	*pos = '\0'; token = str; str = pos + 1; }

#define IS_DIGIT(c)		            (('0' <= c) && (c <= '9'))
#define IS_UPALPHA(c)               (('A' <= c) && (c <= 'Z'))
#define IS_LOALPHA(c)               (('a' <= c) && (c <= 'z'))
#define IS_ALPHA(c)                 (IS_UPALPHA(c) || IS_LOALPHA(c))
#define PRINT                       BACKDOOR_MGR_Printf

/* DATA TYPE DECLARATIONS
 */
typedef void (*SecurityFuncPtr)(int argc, char** argv);

typedef struct SECURITY_Backdoor_Cmd_S
{
	const char*     cmd;
	SecurityFuncPtr exec;
} SECURITY_Backdoor_Cmd_T;

typedef struct SECURITY_Backdoor_Menu_S
{
	const char*     name;
    const int       nbr_of_sub_menus;
    const int       nbr_of_cmd_items;

    struct SECURITY_Backdoor_Menu_S *parent_menu;
    struct SECURITY_Backdoor_Menu_S **sub_menus;
    SECURITY_Backdoor_Cmd_T         *cmd_items;
} SECURITY_Backdoor_Menu_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* ---------------------------------------------------------------------
 * common functions
 * --------------------------------------------------------------------- */
static void SECURITY_Backdoor_ShowIpAddr(UI8_T *addr);
static BOOL_T SECURITY_Backdoor_Str2IpAddr(char *str, UI8_T *addr);

static void SECURITY_Backdoor_ShowMacAddr(UI8_T *addr);
static BOOL_T SECURITY_Backdoor_Str2MacAddr(char *str, UI8_T *addr);

static void SECURITY_Backdoor_ShowRowStatus(UI32_T row_status);

static void SECURITY_Backdoor_Help(int argc, char** argv);
static void SECURITY_BACKDOOR_SetDebugFlag(int argc, char** argv);
static int SECURITY_Backdoor_StrCmpIgnoreCase(const char *dst, const char *src);
static BOOL_T SECURITY_Backdoor_ChopString(UI8_T *str);


/* ---------------------------------------------------------------------
 * backdoor command functions
 * --------------------------------------------------------------------- */
/* aaa backdoor functions */
#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_AccFastSetup(int argc, char** argv);
static void SECURITY_Backdoor_AAA_AccSimpleTest(int argc, char** argv);
static void SECURITY_Backdoor_AAA_SetAccUpdateInterval(int argc, char** argv);
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_SetRadiusGroup(int argc, char** argv);
static void SECURITY_Backdoor_AAA_DelRadiusGroup(int argc, char** argv);
static void SECURITY_Backdoor_AAA_SetRadiusMethod(int argc, char** argv);
static void SECURITY_Backdoor_AAA_DelRadiusMethod(int argc, char** argv);
static void SECURITY_Backdoor_AAA_SetAccExecEntry(int argc, char** argv);
static void SECURITY_Backdoor_AAA_SetAccDot1xEntry(int argc, char** argv);
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE*/

static void SECURITY_Backdoor_AAA_ShowRadiusGroupEntry(int argc, char** argv);
static void SECURITY_Backdoor_AAA_ShowTacacsPlusGroupEntry(int argc, char** argv);

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_ShowAccUpdateInterval(int argc, char** argv);
static void SECURITY_Backdoor_AAA_ShowAccDot1xEntry(int argc, char** argv);
static void SECURITY_Backdoor_AAA_ShowAccExecEntry(int argc, char** argv);
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static void SECURITY_Backdoor_AAA_ShowAccCommandEntry(int argc, char** argv);
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
static void SECURITY_Backdoor_AAA_ShowAccListEntry(int argc, char** argv);
static void SECURITY_Backdoor_AAA_ShowAccUser(int argc, char** argv);
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
static void SECURITY_Backdoor_AAA_ShowAuthorization(int argc, char** argv);
#endif

#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* radius backdoor command functions */
#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    static void SECURITY_Backdoor_Radius_SetServerHost(int argc, char** argv);
    static void SECURITY_Backdoor_Radius_ShowServerHost(int argc, char** argv);
    static void SECURITY_Backdoor_Radius_Show2618Mib(int argc, char** argv);

    #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    static void SECURITY_Backdoor_Radius_Show2620Mib(int argc, char** argv);
    static void SECURITY_Backdoor_Radius_ShowAccUser(int argc, char** argv);
    static void SECURITY_Backdoor_Enable_Console_Radius_Acc(int argc, char** argv);
    #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */
    static void SECURITY_Backdoor_Radius_IGMPAuth(int argc, char** argv);

#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    static void SECURITY_Backdoor_Tacacs_ShowServerHost(int argc, char** argv);

#endif /* TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* userauth backdoor command functions */
#if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)

    static void SECURITY_Backdoor_UserAuth_SetAuthMethod(int argc, char** argv);
    static void SECURITY_Backdoor_UserAuth_ShowAuthMethod(int argc, char** argv);

#endif /* USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* STATIC VARIABLE DECLARATIONS
 */

/* ---------------------------------------------------------------------
 * backdoor menu tree
 * --------------------------------------------------------------------- */
#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)

    /* aaa backdoor command items */
    static SECURITY_Backdoor_Cmd_T  aaa_command_items[] = {
        {"help",                    SECURITY_Backdoor_Help},
    #if (SYS_CPNT_ACCOUNTING == TRUE)
        {"acc_fast_setup",          SECURITY_Backdoor_AAA_AccFastSetup},
        {"acc_simple_test",         SECURITY_Backdoor_AAA_AccSimpleTest},
        {"acc_set_update_interval", SECURITY_Backdoor_AAA_SetAccUpdateInterval},
    #endif /* SYS_CPNT_ACCOUNTING == TRUE */

    #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        {"set_radius_group",        SECURITY_Backdoor_AAA_SetRadiusGroup},
        {"del_radius_group",        SECURITY_Backdoor_AAA_DelRadiusGroup},
        {"set_radius_method_list",  SECURITY_Backdoor_AAA_SetRadiusMethod},
        {"del_radius_method_list",  SECURITY_Backdoor_AAA_DelRadiusMethod},
        {"set_exec_acc_entry",      SECURITY_Backdoor_AAA_SetAccExecEntry},
        {"set_dot1x_entry",         SECURITY_Backdoor_AAA_SetAccDot1xEntry},
    #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE*/

    {"show_radius_group",           SECURITY_Backdoor_AAA_ShowRadiusGroupEntry},
    {"show_tacacs+_group",          SECURITY_Backdoor_AAA_ShowTacacsPlusGroupEntry},

    #if (SYS_CPNT_ACCOUNTING == TRUE)
        {"show_acc_update_interval",SECURITY_Backdoor_AAA_ShowAccUpdateInterval},
        {"show_acc_dot1x_entry",    SECURITY_Backdoor_AAA_ShowAccDot1xEntry},
        {"show_acc_exec_entry",     SECURITY_Backdoor_AAA_ShowAccExecEntry},
        #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        {"show_acc_command_entry", SECURITY_Backdoor_AAA_ShowAccCommandEntry},
        #endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
        {"show_acc_method_list",    SECURITY_Backdoor_AAA_ShowAccListEntry},
        {"show_acc_user",           SECURITY_Backdoor_AAA_ShowAccUser},
    #endif /* SYS_CPNT_ACCOUNTING == TRUE */

    #if (SYS_CPNT_AUTHORIZATION == TRUE)
        {"show_author",             SECURITY_Backdoor_AAA_ShowAuthorization},
    #endif

    };

    /* aaa backdoor menu */
    static SECURITY_Backdoor_Menu_T aaa_menu[] = {
        {"aaa", 0, sizeof(aaa_command_items)/sizeof(SECURITY_Backdoor_Cmd_T), NULL, NULL, aaa_command_items},
    };

#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    /* radius backdoor command items */
    static SECURITY_Backdoor_Cmd_T  radius_command_items[] = {
    	{"help", SECURITY_Backdoor_Help},
    	{"set_server_host", SECURITY_Backdoor_Radius_SetServerHost},
    	{"show_server_host", SECURITY_Backdoor_Radius_ShowServerHost},
    	{"show_2618_mib", SECURITY_Backdoor_Radius_Show2618Mib},
    #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    	{"show_2620_mib", SECURITY_Backdoor_Radius_Show2620Mib},
        {"show_acc_user", SECURITY_Backdoor_Radius_ShowAccUser},
        {"enable_console_radius_acc", SECURITY_Backdoor_Enable_Console_Radius_Acc},
    #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */
        {"igmp_auth", SECURITY_Backdoor_Radius_IGMPAuth},

    };

    /* radius backdoor menu */
    static SECURITY_Backdoor_Menu_T radius_menu[] = {
        {"radius", 0, sizeof(radius_command_items)/sizeof(SECURITY_Backdoor_Cmd_T), NULL, NULL, radius_command_items},
    };

#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */


#if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

    /* tacacs+ backdoor command items */
    static SECURITY_Backdoor_Cmd_T  tacacsplus_command_items[] = {
    	{"help", SECURITY_Backdoor_Help},
    	{"show_tacacs_server", SECURITY_Backdoor_Tacacs_ShowServerHost},
    };

    /* tacacs+ backdoor menu */
    static SECURITY_Backdoor_Menu_T tacacsplus_menu[] = {
        {"tacacs+", 0, sizeof(tacacsplus_command_items)/sizeof(SECURITY_Backdoor_Cmd_T), NULL, NULL, tacacsplus_command_items},
    };

#endif /* TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE */


#if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
    /* userauth backdoor command items */
    static SECURITY_Backdoor_Cmd_T userauth_command_items[] = {
    	{"help", SECURITY_Backdoor_Help},
    	{"set_auth_method", SECURITY_Backdoor_UserAuth_SetAuthMethod},
    	{"show_auth_method", SECURITY_Backdoor_UserAuth_ShowAuthMethod},
    };

    /* userauth backdoor menu */
    static SECURITY_Backdoor_Menu_T userauth_menu[] = {
        {"user_auth", 0, sizeof(userauth_command_items)/sizeof(SECURITY_Backdoor_Cmd_T),NULL, NULL, userauth_command_items},
    };

#endif /* USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* security backdoor main menu */

/* tacacs+ backdoor command items */
static SECURITY_Backdoor_Cmd_T  main_command_items[] = {
    {"debug", SECURITY_BACKDOOR_SetDebugFlag},
	{"help", SECURITY_Backdoor_Help},
};


static SECURITY_Backdoor_Menu_T *sub_menu_of_main_menu[] = {
    #if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)
        aaa_menu,
    #endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */

    #if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
        radius_menu,
    #endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

    #if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
        tacacsplus_menu,
    #endif /* TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

    #if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
        userauth_menu,
    #endif /* USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */
    };

static SECURITY_Backdoor_Menu_T main_menu[] = {
        {"security",
         sizeof(sub_menu_of_main_menu)/sizeof(SECURITY_Backdoor_Menu_T*),
         sizeof(main_command_items)/sizeof(SECURITY_Backdoor_Cmd_T),
         NULL,
         sub_menu_of_main_menu,
         main_command_items},
    };

/* ---------------------------------------------------------------------
 * backdoor operation variables
 * --------------------------------------------------------------------- */

static SECURITY_Backdoor_Menu_T *current_menu = NULL;


/* EXPORTED SUBPROGRAM BODIES
 */
#if (SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SECURITY_Backdoor_Register_SubsysBackdoorFunc
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void SECURITY_Backdoor_Register_SubsysBackdoorFunc(void)
{
    /* register call back function for backdoor program
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("security",
        SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY,SECURITY_Backdoor_CallBack);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  SECURITY_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : security backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void SECURITY_Backdoor_CallBack()
{
    const int               buf_len = 512;
	char                    buffer[buf_len];
	const int               size = 20;
	char                    *pos, *ptr, *token[size];
	int                     cnt, i, j, max, shift;
    L_THREADGRP_Handle_T    auth_proc_tg_handle,
                            old_tg_handle = NULL, curr_tg_handle = NULL;
    UI32_T                  backdoor_member_id;

    auth_proc_tg_handle     = AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle();

/* initialize parent menu */
#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    radius_menu[0].parent_menu = main_menu;
#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    tacacsplus_menu[0].parent_menu = main_menu;
#endif /* TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
    userauth_menu[0].parent_menu = main_menu;
#endif /* USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */

    BACKDOOR_MGR_Printf("\r\nsecurity backdoor");
 	SECURITY_Backdoor_Help(0, NULL);

    while (TRUE)
    {
        BACKDOOR_MGR_Printf(SECURITY_BACKDOOR_PROMPT);
        BACKDOOR_MGR_RequestKeyIn(buffer, buf_len);

        if (!SECURITY_Backdoor_ChopString((UI8_T *)buffer))
            continue;

		if (!SECURITY_Backdoor_StrCmpIgnoreCase(buffer, "?"))
        {
            SECURITY_Backdoor_Help(0, NULL);
            continue;
        }

		if (!SECURITY_Backdoor_StrCmpIgnoreCase(buffer, "quit"))
            return;

		if (!SECURITY_Backdoor_StrCmpIgnoreCase(buffer, "exit"))
        {
            if ((NULL == current_menu) || (NULL == current_menu->parent_menu))
			    return;

            current_menu = current_menu->parent_menu;

            if ((current_menu == main_menu) && (NULL != curr_tg_handle))
            {
                L_THREADGRP_Leave(curr_tg_handle, backdoor_member_id);
                old_tg_handle = curr_tg_handle = NULL;
            }

            SECURITY_Backdoor_Help(0, NULL);
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
			if (SECURITY_Backdoor_StrCmpIgnoreCase(token[0], current_menu->sub_menus[i]->name))
				continue;

            current_menu = current_menu->sub_menus[i];

#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)
            if (current_menu == aaa_menu)
            {
                curr_tg_handle = auth_proc_tg_handle;
            }
#endif

#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
            if (current_menu == radius_menu)
            {
                curr_tg_handle = auth_proc_tg_handle;
            }
#endif

#if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)
            if (current_menu == tacacsplus_menu)
            {
                curr_tg_handle = auth_proc_tg_handle;
            }
#endif

#if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
            if (current_menu == userauth_menu)
            {
                curr_tg_handle = sysmgmt_proc_tg_handle;
            }
#endif

            if (old_tg_handle != curr_tg_handle)
            {
                old_tg_handle = curr_tg_handle;
                /* Join thread group
                 */
                if(L_THREADGRP_Join(curr_tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
                    return;
                }
            }

		    SECURITY_Backdoor_Help(0, NULL);
            break;
        }

        if (max > i)
            continue;

        /* command items */
    	for (j = 0, max = current_menu->nbr_of_cmd_items; max > j; ++j)
        {
			if (SECURITY_Backdoor_StrCmpIgnoreCase(token[0], current_menu->cmd_items[j].cmd))
				continue;

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

    	    SECURITY_Backdoor_Help(0, NULL);
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
        		    SECURITY_Backdoor_Help(0, NULL);
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
    }
}

#define MAX_SHOW_NAME_STR_LEN   20
#define MAX_BACKDOOR_ITEM_NUM   10

typedef struct
{
    char    show_name[MAX_SHOW_NAME_STR_LEN + 1];
    BOOL_T  on_off;
}SECURITY_BACKDOOR_PoolItem_T;

static SECURITY_BACKDOOR_PoolItem_T security_backdoor_pool[MAX_BACKDOOR_ITEM_NUM];

SECURITY_BACKDOOR_PoolItem_T* SECURITY_BACKDOOR_PollItem(UI32_T i)
{
    return &security_backdoor_pool[i];
}

BOOL_T SECURITY_BACKDOOR_Register(const char *show_name, UI32_T *reg_no_p)
{
    SECURITY_BACKDOOR_PoolItem_T *item;
    int i;

    for (i=0; i < MAX_BACKDOOR_ITEM_NUM; ++i)
    {
        item = SECURITY_BACKDOOR_PollItem(i);
        if (strcmp(item->show_name, show_name) == 0)
        {
            *reg_no_p = i;
            return TRUE;
        }
    }

    for (i=0; i < MAX_BACKDOOR_ITEM_NUM; ++i)
    {
        item = SECURITY_BACKDOOR_PollItem(i);
        if (item->show_name[0] == 0)
        {
            strncpy(item->show_name, show_name, MAX_SHOW_NAME_STR_LEN);
            *reg_no_p = i;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL_T SECURITY_BACKDOOR_IsOn(UI32_T reg_no)
{
    SECURITY_BACKDOOR_PoolItem_T *item;

    if (MAX_BACKDOOR_ITEM_NUM < reg_no)
        return FALSE;

    item = SECURITY_BACKDOOR_PollItem(reg_no);
    if (item->show_name[0] == 0)
        return FALSE;

    return item->on_off;
}

static void SECURITY_BACKDOOR_Switch(UI32_T i)
{
    SECURITY_BACKDOOR_PoolItem_T *item;

    if (MAX_BACKDOOR_ITEM_NUM < i)
        return;

    item = SECURITY_BACKDOOR_PollItem(i);

    if (item->on_off)
        item->on_off = 0;
    else
        item->on_off = 1;
}

static void SECURITY_BACKDOOR_ShowMenu()
{
    SECURITY_BACKDOOR_PoolItem_T *item;
    int i = 0;

    PRINT("\n");
    for (; i < MAX_BACKDOOR_ITEM_NUM; ++i)
    {
        item = SECURITY_BACKDOOR_PollItem(i);

        if (item->show_name[0] != 0)
        {
            PRINT("[%d] %s = %s\n", i, item->show_name, item->on_off? "on":"off");
        }
    }
}

static void SECURITY_BACKDOOR_SetDebugFlag(int argc, char * * argv)
{
    char ch;

    while (1)
    {
        SECURITY_BACKDOOR_ShowMenu();

        PRINT("\r\n Press 'q' to exit : ");

        ch = getchar();

        if (ch == 'q' || ch == 'Q')
            break;

        if ('0' <= ch && ch <= '9')
        {
            SECURITY_BACKDOOR_Switch(ch - '0');
        }
    }
}
#endif /* SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE */



/* LOCAL SUBPROGRAM BODIES
 */

static void SECURITY_Backdoor_ShowIpAddr(UI8_T *addr)
{
    if (NULL == addr)
        return;

    BACKDOOR_MGR_Printf("%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

static BOOL_T SECURITY_Backdoor_Str2IpAddr(char *str, UI8_T *addr)
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

static void SECURITY_Backdoor_ShowMacAddr(UI8_T *addr)
{
    if (NULL == addr)
        return;

    BACKDOOR_MGR_Printf("%02x-%02x-%02x-%02x-%02x-%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static BOOL_T SECURITY_Backdoor_Str2MacAddr(char *str, UI8_T *addr)
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

static void SECURITY_Backdoor_ShowRowStatus(UI32_T row_status)
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
 * ROUTINE NAME - SECURITY_Backdoor_Help
 *---------------------------------------------------------------------------
 * PURPOSE  : show current backdoor menu
 * INPUT    : argc, argv
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *---------------------------------------------------------------------------*/
static void SECURITY_Backdoor_Help(int argc, char** argv)
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
 * FUNCTION NAME - SECURITY_Backdoor_StrCmpIgnoreCase
 *-------------------------------------------------------------------------
 * PURPOSE  : compare two strings case-insensitivly
 * INPUT    : dst, src (terminate by '\0')
 * OUTPUT   : none
 * RETURN   : 0 -- equal, > 0 -- dst > src, < 0 -- small
 * NOTE     :
 *-------------------------------------------------------------------------*/
static int SECURITY_Backdoor_StrCmpIgnoreCase(const char *dst, const char *src)
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
 * FUNCTION NAME - SECURITY_Backdoor_ChopString
 *-------------------------------------------------------------------------
 * PURPOSE  : chop source string then put the result back
 * INPUT    : src_str (terminate by '\0')
 * OUTPUT   : src_str
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T SECURITY_Backdoor_ChopString(UI8_T *str)
{
    UI8_T       *src_iter, *dst_iter, *dst_end;

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
    else if (dst_iter != src_iter) /* e.g. " a" */
        *dst_iter = '\0';


    return TRUE;
}

#if (SYS_CPNT_AAA == TRUE)

#if 0
static const char* SECURITY_Backdoor_StrAaaClientType(AAA_ClientType_T type)
{
    return (type==AAA_CLIENT_TYPE_DOT1X)    ? "dot1x":
           (type==AAA_CLIENT_TYPE_EXEC)     ? "exec" :
           (type==AAA_CLIENT_TYPE_COMMANDS) ? "cmd":
            "error";
}
#endif

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static const char* SECURITY_Backdoor_StrAaaExecType(AAA_ExecType_T type)
{
    return (type==AAA_EXEC_TYPE_CONSOLE)    ? "console":
           (type==AAA_EXEC_TYPE_VTY)        ? "vty" :
           (type==AAA_EXEC_TYPE_NONE)       ? "none":
            "error";
}

static const char* SECURITY_Backdoor_StrAaaConfigureMode(AAA_ConfigureMode_T mode)
{
    return (mode==AAA_AUTO_CONFIGURE)    ? "auto":
           (mode==AAA_MANUAL_CONFIGURE)  ? "manual" :
            "error";
}
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

#if 0
static const char* SECURITY_Backdoor_StrAaaGroupType(AAA_ServerGroupType_T type)
{
    return (type==GROUP_RADIUS)       ? "radius":
           (type==GROUP_TACACS_PLUS)  ? "tacacs":
           (type==GROUP_UNKNOWN)      ? "unknow":
            "error";
}

static const char* SECURITY_Backdoor_StrAaaAuthentic(AAA_Authentic_T auth)
{
    return (auth==AAA_AUTHEN_BY_UNKNOWN)      ? "unknow":
           (auth==AAA_AUTHEN_BY_RADIUS)       ? "radius":
           (auth==AAA_AUTHEN_BY_TACACS_PLUS)  ? "tacacs":
           (auth==AAA_AUTHEN_BY_LOCAL)        ? "local":
            "error";
}

static const char* SECURITY_Backdoor_StrAaaWorkingMode(AAA_AccWorkingMode_T mode)
{
    return (mode==ACCOUNTING_START_STOP)      ? "start-stop":
            "error";
}
#endif
#endif /*SYS_CPNT_AAA*/

#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_AccFastSetup(int argc, char** argv)
{
    AAA_RadiusGroupEntryInterface_T group_entry = {2, "radGroup1"};
    AAA_RadiusEntryInterface_T      radius_entry = {2, 1};
    AAA_AccListEntryInterface_T     list_entry1 = {1, "default", "radius", AAA_CLIENT_TYPE_DOT1X, GROUP_RADIUS, ACCOUNTING_START_STOP};
    AAA_AccListEntryInterface_T     list_entry2 = {1, "default", "tacacs+", AAA_CLIENT_TYPE_EXEC, GROUP_TACACS_PLUS, ACCOUNTING_START_STOP};
    AAA_AccDot1xEntry_T             dot1x_entry = {1, "default", AAA_MANUAL_CONFIGURE};
    AAA_AccExecEntry_T              exec_entry = {AAA_ACC_EXEC_CONSOLE, "default", AAA_MANUAL_CONFIGURE};

    if (FALSE == AAA_MGR_SetRadiusGroupEntry(&group_entry))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetRadiusGroupEntry failed!");

    if (FALSE == AAA_MGR_SetRadiusEntry(group_entry.group_index, &radius_entry, NULL))
        BACKDOOR_MGR_Printf("\r\nfailed! please setup radius-server host 1");

    if (FALSE == AAA_MGR_SetAccListEntry(&list_entry1))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccListEntry failed!");

    if (FALSE == AAA_MGR_SetAccListEntry(&list_entry2))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccListEntry failed!");

    if (FALSE == AAA_MGR_SetAccDot1xEntry(&dot1x_entry))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccDot1xEntry failed!");

    if (FALSE == AAA_MGR_SetAccExecEntry(&exec_entry))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccExecEntry failed!");
}

static void SECURITY_Backdoor_AAA_AccSimpleTest(int argc, char** argv)
{
    AAA_AccRequest_T    request = {1, "eaptest", AAA_CLIENT_TYPE_DOT1X, AAA_ACC_START, 1};

    if (argc != 3 && argc != 4)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {user_name} {stop|start} {dot1x|console|vty} [ifindex]");
        return;
    }

    strncpy((char *)request.user_name, argv[0], SYS_ADPT_MAX_USER_NAME_LEN);

    if (strcmp(argv[1], "start") == 0)
        request.request_type = AAA_ACC_START;
    else if (strcmp(argv[1], "stop") == 0)
        request.request_type = AAA_ACC_STOP;
    else
    {
        BACKDOOR_MGR_Printf("\r\nbad request_type");
        return;
    }

    if (strcmp(argv[2], "dot1x") == 0)
    {
        request.client_type = AAA_CLIENT_TYPE_DOT1X;

        if (argv[3])
        {
            request.ifindex = atoi(argv[3]);
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\nno assign ifindex for dot1x");
            return;
        }
    }
    else if (strcmp(argv[2], "console") == 0)
    {
        request.client_type = AAA_CLIENT_TYPE_EXEC;
        request.ifindex = AAA_EXEC_TYPE_CONSOLE;
    }
    else if (strcmp(argv[2], "vty") == 0)
    {
        request.client_type = AAA_CLIENT_TYPE_EXEC;
        request.ifindex = AAA_EXEC_TYPE_VTY;
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nbad client_type");
        return;
    }

    if (FALSE == AAA_MGR_AsyncAccountingRequest(&request))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_AsyncAccountingRequest failed!");

}

static void SECURITY_Backdoor_AAA_SetAccUpdateInterval(int argc, char** argv)
{
    UI32_T      value;
    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {minutes}");
        return;
    }

    value = atoi(argv[0]);
    AAA_MGR_SetAccUpdateInterval(value);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_SetRadiusGroup(int argc, char** argv)
{
    AAA_RadiusGroupEntryInterface_T entry;
    AAA_RadiusEntryInterface_T      radius_entry;
    BOOL_T existed = FALSE;

    switch (argc)
    {
        case 2:
            radius_entry.radius_server_index = atoi(argv[1]);
            break;
        case 1:
            break;

        default:
            BACKDOOR_MGR_Printf("\r\nparameters: {group_name} [{server_index}]");
            return;
    }

    entry.group_index = 0;
    while (TRUE == AAA_OM_GetNextRadiusGroupEntry(&entry))
    {
        if(strcmp(argv[0], (char *)entry.group_name) == 0)
        {
            existed = TRUE;
            if(argc == 2)
            {
                AAA_MGR_SetRadiusEntry(entry.group_index, &radius_entry, NULL);
                return;
            }
        }
    }

    if(existed == FALSE)
    {
        strcpy((char *)entry.group_name, argv[0]);
        AAA_MGR_SetRadiusGroupEntry(&entry);
        if(argc == 2)
        {
            while (TRUE == AAA_OM_GetNextRadiusGroupEntry(&entry))
            {
                if(strcmp(argv[0], (char *)entry.group_name) == 0)
                {
                    AAA_MGR_SetRadiusEntry(entry.group_index, &radius_entry, NULL);
                }
            }
        }
    }
}

static void SECURITY_Backdoor_AAA_DelRadiusGroup(int argc, char** argv)
{
    AAA_WarningInfo_T warning;

    if(argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {group_name}");
            return;
    }
    AAA_MGR_DestroyRadiusGroupEntry(argv[0], &warning);
    switch (warning.warning_type)
    {
        case AAA_LIST_REF2_BAD_GROUP:
            BACKDOOR_MGR_Printf("Method list references a non-existent or removed group");
            break;
        case AAA_GROUP_REF2_BAD_SERVER:
            BACKDOOR_MGR_Printf("Server group references a non-existent or removed server host");
            break;
        case AAA_GROUP_HAS_NO_ENTRY:
            BACKDOOR_MGR_Printf("Server group doesn't have any entry");
            break;
        case AAA_ACC_DOT1X_REF2_BAD_LIST:
            BACKDOOR_MGR_Printf("Accounting dot1x references a non-existent or removed method-list");
            break;
        case AAA_ACC_EXEC_REF2_BAD_LIST:
            BACKDOOR_MGR_Printf("Accounting exec references a non-existent or removed method-list");
            break;
        case AAA_NO_WARNING:
            default:
            break;
        }
}

static void SECURITY_Backdoor_AAA_SetRadiusMethod(int argc, char** argv)
{
    AAA_AccListEntryInterface_T     list_entry = {1, "default", "radius", AAA_CLIENT_TYPE_EXEC, GROUP_RADIUS, ACCOUNTING_START_STOP};

    if (argc != 3)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {dot1x|exec} {method_name} {group_name}");
        return;
    }

    if (strcmp(argv[0], "dot1x") == 0)
        list_entry.client_type = AAA_CLIENT_TYPE_DOT1X;
    else if (strcmp(argv[0], "exec") == 0)
        list_entry.client_type = AAA_CLIENT_TYPE_EXEC;
    else
    {
        BACKDOOR_MGR_Printf("\r\nbad client_type");
        return;
    }

    strncpy((char *)list_entry.list_name, argv[1], SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    strncpy((char *)list_entry.group_name, argv[2], SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);

    if(AAA_MGR_SetAccListEntry(&list_entry) == FALSE)
    {
        BACKDOOR_MGR_Printf("\r\nSet radius method fails.");
    }
}

static void SECURITY_Backdoor_AAA_DelRadiusMethod(int argc, char** argv)
{
    AAA_ClientType_T client_type;
    AAA_WarningInfo_T warning;

    if (argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {dot1x|exec} {method_name}");
        return;
    }

    if (strcmp(argv[0], "dot1x") == 0)
        client_type = AAA_CLIENT_TYPE_DOT1X;
    else if (strcmp(argv[0], "exec") == 0)
        client_type = AAA_CLIENT_TYPE_EXEC;
    else
    {
        BACKDOOR_MGR_Printf("\r\nbad client_type");
        return;
    }

    if(AAA_MGR_DestroyAccListEntry(argv[1], client_type, &warning) == FALSE)
    {
        BACKDOOR_MGR_Printf("\r\nDel radius method fails.");
    }
    switch (warning.warning_type)
    {
        case AAA_LIST_REF2_BAD_GROUP:
            BACKDOOR_MGR_Printf("Method list references a non-existent or removed group");
            break;
        case AAA_GROUP_REF2_BAD_SERVER:
            BACKDOOR_MGR_Printf("Server group references a non-existent or removed server host");
            break;
        case AAA_GROUP_HAS_NO_ENTRY:
            BACKDOOR_MGR_Printf("Server group doesn't have any entry");
            break;
        case AAA_ACC_DOT1X_REF2_BAD_LIST:
            BACKDOOR_MGR_Printf("Accounting dot1x references a non-existent or removed method-list");
            break;
        case AAA_ACC_EXEC_REF2_BAD_LIST:
            BACKDOOR_MGR_Printf("Accounting exec references a non-existent or removed method-list");
            break;
        case AAA_NO_WARNING:
            default:
            break;
        }
}

/*maggie liu, 2009-03-09*/
static void SECURITY_Backdoor_AAA_SetAccExecEntry(int argc, char * * argv)
{
    AAA_AccExecEntry_T              exec_entry = {AAA_ACC_EXEC_CONSOLE, "default", AAA_MANUAL_CONFIGURE};

    if(argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {console|vty} {method_name}");
        return;
    }

    if (strcmp(argv[0], "console") == 0)
        exec_entry.exec_type = AAA_ACC_EXEC_CONSOLE;
    else if (strcmp(argv[0], "vty") == 0)
        exec_entry.exec_type = AAA_ACC_EXEC_VTY;
    else
    {
        BACKDOOR_MGR_Printf("\r\nbad client_type");
        return;
    }

    strcpy((char *)exec_entry.list_name, argv[1]);

    if (FALSE == AAA_MGR_SetAccExecEntry(&exec_entry))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccExecEntry failed!");

}

static void SECURITY_Backdoor_AAA_SetAccDot1xEntry(int argc, char** argv)
{
    AAA_AccDot1xEntry_T             dot1x_entry = {1, "default", AAA_MANUAL_CONFIGURE};

    if(argc != 2)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {port} {method_name}");
        return;
    }

    dot1x_entry.ifindex = atoi(argv[0]);
    strcpy((char *)dot1x_entry.list_name, argv[1]);

    if (FALSE == AAA_MGR_SetAccDot1xEntry(&dot1x_entry))
        BACKDOOR_MGR_Printf("\r\nAAA_MGR_SetAccDot1xEntry failed!");
}
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

static void SECURITY_Backdoor_AAA_ShowRadiusGroupEntry(int argc, char** argv)
{
    AAA_RadiusGroupEntryInterface_T entry;
    AAA_RadiusEntryInterface_T radius_entry;

    if (argc == 0)
    {
        entry.group_index = 0;
        while (TRUE == AAA_OM_GetNextRadiusGroupEntry(&entry))
        {
            BACKDOOR_MGR_Printf("\r\ngroup: index(%d) name(%s)", entry.group_index, entry.group_name);

            radius_entry.radius_index = 0;
            while (TRUE == AAA_OM_GetNextRadiusEntry(entry.group_index, &radius_entry))
            {
                BACKDOOR_MGR_Printf("\r\ngroup entry: server index(%lu)", radius_entry.radius_server_index);
            }
        }
    }
    else if (argc == 1)
    {
        UI32_T group_index = atoi(argv[0]);

        radius_entry.radius_index = 0;
        while (TRUE == AAA_OM_GetNextRadiusEntry(group_index, &radius_entry))
        {
            BACKDOOR_MGR_Printf("\r\ngroup entry: server index(%lu)", radius_entry.radius_server_index);
        }
    }
}

static void SECURITY_Backdoor_AAA_ShowTacacsPlusGroupEntry(int argc, char** argv)
{
    AAA_TacacsPlusGroupEntryInterface_T entry;
    AAA_TacacsPlusEntryInterface_T tacacs_entry;

    entry.group_index = 0;
    while (TRUE == AAA_OM_GetNextTacacsPlusGroupEntry(&entry))
    {
        BACKDOOR_MGR_Printf("\r\ngroup: index(%d) name(%s)", entry.group_index, entry.group_name);

        tacacs_entry.tacacs_index = 0;
        while (TRUE == AAA_OM_GetNextTacacsPlusEntry(entry.group_index, &tacacs_entry))
        {
            BACKDOOR_MGR_Printf("\r\ngroup entry: server index(%lu)", tacacs_entry.tacacs_server_index);
        }
    }
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_AAA_ShowAccUpdateInterval(int argc, char** argv)
{
    UI32_T      value;
    if (FALSE == AAA_OM_GetAccUpdateInterval(&value))
        return;

    BACKDOOR_MGR_Printf("\r\nUpdateInterval = %lu", value);
}

static void SECURITY_Backdoor_AAA_ShowAccDot1xEntry(int argc, char** argv)
{
    AAA_AccDot1xEntry_T entry;
    entry.ifindex = 0;
    while (TRUE == AAA_OM_GetNextAccDot1xEntry(&entry))
    {
        BACKDOOR_MGR_Printf("\r\ngroup: index(%lu) name(%s)", entry.ifindex, entry.list_name);
    }
}

static void SECURITY_Backdoor_AAA_ShowAccExecEntry(int argc, char** argv)
{
    AAA_AccExecEntry_T entry;
    entry.exec_type = 0;
    while (TRUE == AAA_OM_GetNextAccExecEntry(&entry))
    {
        BACKDOOR_MGR_Printf("\r\ngroup: exec_type(%d) name(%s)", entry.exec_type, entry.list_name);
    }
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static void SECURITY_Backdoor_AAA_ShowAccCommandEntry(int argc, char** argv)
{
    AAA_AccCommandEntry_T entry;
    BOOL_T                first_entry = TRUE;
    UI32_T                index;

    enum{MAX_STRING_LENGTH = 3};
    UI8_T               input_string[MAX_STRING_LENGTH+1] = {0};
    BOOL_T              show_all    = FALSE;

    do{
        printf(" Show all entry? (Y/N):\r\n");
        BACKDOOR_MGR_RequestKeyIn(input_string, sizeof(input_string));
    }while(input_string[0]!='y' && input_string[0]!='Y' && input_string[0]!='n' && input_string[0]!='N');

    if (input_string[0] == 'y' || input_string[0] == 'Y')
        show_all = TRUE;
    else
        show_all = FALSE;

    index = 0;
    while (TRUE == AAA_OM_GetNextAccCommandEntry(&index, &entry))
    {
        if (show_all == FALSE)
        {
            if (entry.list_name[0] == '\0')
                continue;
        }

        if (first_entry == TRUE)
        {
            printf("\r\n *idx name     execType priv-lvl configMode");
            first_entry = FALSE;
        }

        printf("\r\n  %3lu %-8s %-8s %8ld %-10s",
            index,
            entry.list_name,
            SECURITY_Backdoor_StrAaaExecType(entry.exec_type),
            entry.priv_lvl,
            SECURITY_Backdoor_StrAaaConfigureMode(entry.configure_mode));
    }

    if (first_entry == TRUE)
        printf("\r\n No entry");
}
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

static void SECURITY_Backdoor_AAA_ShowAccListEntry(int argc, char** argv)
{
    AAA_AccListEntryInterface_T entry;
    entry.list_index = 0;
    while (TRUE == AAA_OM_GetNextAccListEntry(&entry))
    {
        BACKDOOR_MGR_Printf("\r\nlist: index(%d) name(%s) group(%s) type(%s)",
            entry.list_index, entry.list_name, entry.group_name,
            entry.group_type == GROUP_RADIUS ? "GROUP_RADIUS" :
            entry.group_type == GROUP_TACACS_PLUS ? "GROUP_TACACS_PLUS" :
            entry.group_type == GROUP_UNKNOWN ? "GROUP_UNKNOWN" :
            "unknown");
//    AAA_AccWorkingMode_T    working_mode;
    }
}

static void SECURITY_Backdoor_AAA_ShowAccUser(int argc, char** argv)
{
    AAA_AccUserInfoInterface_T   entry;

    entry.user_index = 0;
    while (TRUE == AAA_OM_GetNextAccUserEntryInfo(&entry))
    {
        BACKDOOR_MGR_Printf("\r\nname(%s), ifindex(%lu), client_type(%s), protocol(%s)",
            (char *)entry.user_name, entry.ifindex,
            (AAA_CLIENT_TYPE_DOT1X == entry.client_type) ? "DOT1X" :
            (AAA_CLIENT_TYPE_EXEC == entry.client_type) ? "EXEC" :
            "UNKNOW TYPE",
            (GROUP_RADIUS == entry.group_type) ? "radius" :
            (GROUP_TACACS_PLUS== entry.group_type) ? "tacacs+" :
            "NA");
    }
}
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
void SECURITY_Backdoor_AAA_ShowAuthorization(int argc, char** argv)
{
    AAA_AuthorListEntryInterface_T  list_entry = {0};
    AAA_AuthorExecEntry_T           exec_entry = {0};

    BACKDOOR_MGR_Printf("\n");

    while (AAA_OM_GetNextAuthorListEntryInterface(&list_entry))
    {
        BACKDOOR_MGR_Printf("array idx(%hu), list_name(%s), client_type(%s), group_name(%s)\n",
            list_entry.list_index,
            list_entry.list_name,
            (list_entry.list_type.client_type == AAA_CLIENT_TYPE_DOT1X)   ? "dot1x"   :
            (list_entry.list_type.client_type == AAA_CLIENT_TYPE_EXEC)    ? "exec"    :
                                                                  "??"      ,
            list_entry.group_name/*,
            (list_entry.group_type == GROUP_RADIUS)             ? "RADIUS"  :
            (list_entry.group_type == GROUP_TACACS_PLUS)        ? "TACACS+" :
                                                                  "??"*/
            );
    }

    BACKDOOR_MGR_Printf("\n");

    while (AAA_OM_GetNextAuthorExecEntry(&exec_entry))
    {
        BACKDOOR_MGR_Printf("%s, list_name(%s), config(%s)\n",
            (exec_entry.exec_type == AAA_EXEC_TYPE_CONSOLE)     ? "console" :
            (exec_entry.exec_type == AAA_EXEC_TYPE_VTY)         ? "vty"     :
                                                                  "??"      ,
            exec_entry.list_name,
            (exec_entry.configure_mode == AAA_AUTO_CONFIGURE)   ? "auto"    :
            (exec_entry.configure_mode == AAA_MANUAL_CONFIGURE) ? "manual"  :
                                                                  "??"
            );
    }
}
#endif /* SYS_CPNT_AUTHORIZATION */

#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

static void SECURITY_Backdoor_Radius_SetServerHost(int argc, char** argv)
{
    RADIUS_Server_Host_T    server_host;
    UI32_T      server_index;

    memset(&server_host, 0, sizeof(server_host));

    switch (argc)
    {
        case 7:
            server_host.server_port = atoi(argv[3]);

        #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
            server_host.acct_port = atoi(argv[4]);
        #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

            server_host.retransmit = atoi(argv[5]);
            server_host.timeout = atoi(argv[6]);

        case 3:
            server_index = atoi(argv[0]);
            SECURITY_Backdoor_Str2IpAddr(argv[1], (UI8_T*)&server_host.server_ip);
            strcpy((char *)server_host.secret, argv[2]);
            break;

        default:
            BACKDOOR_MGR_Printf("\r\nparameters: {server_index} {ip} {secret} [{auth-port} {acct-port} {retransmit} {timeout}]");
            return;
    }

    if (FALSE == RADIUS_MGR_Set_Server_Host(server_index, &server_host))
        BACKDOOR_MGR_Printf("\r\nfailed to set server host");
}

static void SECURITY_Backdoor_Radius_ShowServerHost(int argc, char** argv)
{
    RADIUS_Server_Host_T    server_host;
    UI32_T      server_index;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T      acc_port;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    BACKDOOR_MGR_Printf("\r\nglobal retransmit: %lu", RADIUS_OM_Get_Retransmit_Times());
    BACKDOOR_MGR_Printf("\r\nglobal timeout: %lu",    RADIUS_OM_Get_Request_Timeout());
    BACKDOOR_MGR_Printf("\r\nglobal auth-port: %lu",  RADIUS_OM_Get_Server_Port());

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    if (TRUE == RADIUS_MGR_GetServerAcctPort(&acc_port))
        BACKDOOR_MGR_Printf("\r\nglobal acct-port: %lu", acc_port);
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    server_index = 0;
    while (TRUE == RADIUS_OM_GetNext_Server_Host(&server_index, &server_host))
    {
        BACKDOOR_MGR_Printf("\r\n(%lu) ip(", server_host.server_index);
        SECURITY_Backdoor_ShowIpAddr((UI8_T*)&server_host.server_ip);
        BACKDOOR_MGR_Printf(") key(%s)", server_host.secret);
        BACKDOOR_MGR_Printf(" auth-po(%lu)", server_host.server_port);

    #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        BACKDOOR_MGR_Printf(" acct-po(%lu)", server_host.acct_port);
    #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

        BACKDOOR_MGR_Printf(" retx(%lu)", server_host.retransmit);
        BACKDOOR_MGR_Printf(" timeout(%lu)", server_host.timeout);
    }
}

static void SECURITY_Backdoor_Radius_Show2618Mib(int argc, char** argv)
{
    RADIUS_Server_Host_T    server_host;

    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {server_index}");
        return;
    }

    if (FALSE == RADIUS_OM_Get_Server_Host(atoi(argv[0]), &server_host))
    {
        BACKDOOR_MGR_Printf("\r\nfailed to get server host");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthServerIndex", server_host.server_table.radiusAuthServerIndex);
    BACKDOOR_MGR_Printf("\r\n %40s = ", "radiusAuthServerAddress");
    SECURITY_Backdoor_ShowIpAddr((UI8_T*)&server_host.server_table.radiusAuthServerAddress);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientServerPortNumber", server_host.server_table.radiusAuthClientServerPortNumber);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientRoundTripTime", server_host.server_table.radiusAuthClientRoundTripTime);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientAccessRequests", server_host.server_table.radiusAuthClientAccessRequests);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientAccessRetransmissions", server_host.server_table.radiusAuthClientAccessRetransmissions);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientAccessAccepts", server_host.server_table.radiusAuthClientAccessAccepts);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientAccessRejects", server_host.server_table.radiusAuthClientAccessRejects);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientAccessChallenges", server_host.server_table.radiusAuthClientAccessChallenges);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientMalformedAccessResponses", server_host.server_table.radiusAuthClientMalformedAccessResponses);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientBadAuthenticators", server_host.server_table.radiusAuthClientBadAuthenticators);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientPendingRequests", server_host.server_table.radiusAuthClientPendingRequests);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientTimeouts", server_host.server_table.radiusAuthClientTimeouts);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientUnknownTypes", server_host.server_table.radiusAuthClientUnknownTypes);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAuthClientPacketsDropped", server_host.server_table.radiusAuthClientPacketsDropped);
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static void SECURITY_Backdoor_Radius_Show2620Mib(int argc, char** argv)
{
    RADIUS_Server_Host_T    server_host;

    if (argc != 1)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {server_index}");
        return;
    }

    if (FALSE == RADIUS_OM_Get_Server_Host(atoi(argv[0]), &server_host))
    {
        BACKDOOR_MGR_Printf("\r\nfailed to get server host");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n %40s = ", "radiusAccServerAddress");
    SECURITY_Backdoor_ShowIpAddr((UI8_T*)&server_host.std_acc_cli_mib.radiusAccServerAddress);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientServerPortNumber", server_host.std_acc_cli_mib.radiusAccClientServerPortNumber);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientRoundTripTime", server_host.std_acc_cli_mib.radiusAccClientRoundTripTime);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientRequests", server_host.std_acc_cli_mib.radiusAccClientRequests);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientRetransmissions", server_host.std_acc_cli_mib.radiusAccClientRetransmissions);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientResponses", server_host.std_acc_cli_mib.radiusAccClientResponses);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientMalformedResponses", server_host.std_acc_cli_mib.radiusAccClientMalformedResponses);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientBadAuthenticators", server_host.std_acc_cli_mib.radiusAccClientBadAuthenticators);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientPendingRequests", server_host.std_acc_cli_mib.radiusAccClientPendingRequests);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientTimeouts", server_host.std_acc_cli_mib.radiusAccClientTimeouts);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientUnknownTypes", server_host.std_acc_cli_mib.radiusAccClientUnknownTypes);
    BACKDOOR_MGR_Printf("\r\n %40s = %lu", "radiusAccClientPacketsDropped", server_host.std_acc_cli_mib.radiusAccClientPacketsDropped);
}

static void SECURITY_Backdoor_Radius_ShowAccUser(int argc, char** argv)
{
    RADIUS_MGR_Backdoor_ShowAccUser();
}
static void SECURITY_Backdoor_Enable_Console_Radius_Acc(int argc, char** argv)
{

}
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

static void SECURITY_Backdoor_Radius_IGMPAuth(int argc, char** argv)
{   
    RADIUS_MGR_RequestContext_T request;
    
    request.type = RADIUS_REQUEST_TYPE_IGMPAUTH;
    request.blocking = FALSE;
    request.igmp_auth_data.auth_port = 100;
    request.igmp_auth_data.ip_address = 0x6301A8C0;
    request.igmp_auth_data.vlan_id = 1;
    request.igmp_auth_data.src_ip = 0x0101A8C0;
    request.igmp_auth_data.msg_type = 0x16;
    request.igmp_auth_data.auth_mac[0]= 0;
    request.igmp_auth_data.auth_mac[1]= 1;
    request.igmp_auth_data.auth_mac[2]= 2;
    request.igmp_auth_data.auth_mac[3]= 3;
    request.igmp_auth_data.auth_mac[4]= 4;
    request.igmp_auth_data.auth_mac[5]= 5;
    
    if(RADIUS_RETURN_SUCCESS != RADIUS_MGR_SubmitRequest(&request))
    {
        printf("Fail to Submit Request.");
    }
    
}

#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE)

static void SECURITY_Backdoor_Tacacs_ShowServerHost(int argc, char** argv)
{
    TACACS_Server_Host_T    server_host;
    UI32_T                  server_index;
    UI8_T  key[MAXSIZE_tacacsServerKey+1];

    BACKDOOR_MGR_Printf("\r\nglobal port: %lu", TACACS_OM_Get_Server_Port());
    TACACS_OM_Get_Server_Secret(key);
    BACKDOOR_MGR_Printf("\r\nglobal secret-key: %s", key);

    server_index = 0;
    while (TRUE == TACACS_OM_GetNext_Server_Host(&server_index, &server_host))
    {
        BACKDOOR_MGR_Printf("\r\n(%lu) ip(", server_host.server_index);
        SECURITY_Backdoor_ShowIpAddr((UI8_T*)&server_host.server_ip);
        BACKDOOR_MGR_Printf(") key(%s)", server_host.secret);
        BACKDOOR_MGR_Printf(" port(%lu)", server_host.server_port);
        BACKDOOR_MGR_Printf(" timeout(%lu)", server_host.timeout);
    }
}

#endif /* TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)

static void SECURITY_Backdoor_UserAuth_SetAuthMethod(int argc, char** argv)
{
    int i;
    USERAUTH_Auth_Method_T      method[USERAUTH_NUMBER_Of_AUTH_METHOD];
    if (0 >= argc)
    {
        BACKDOOR_MGR_Printf("\r\nparameters: {r|t|l} [r|t|l] [radius|tacacs|local]");
        goto exit;
    }

    memset(method, 0, sizeof(method));

    for (i = 0; USERAUTH_NUMBER_Of_AUTH_METHOD > i; ++i)
    {
        if (i >= argc)
            break;

        switch (argv[i][0])
        {
            case 'l':
                method[i] = USERAUTH_AUTH_LOCAL;
                break;
            case 'r':
                method[i] = USERAUTH_AUTH_RADIUS;
                break;
            case 't':
                method[i] = USERAUTH_AUTH_TACACS;
                break;
        }
    }

    if (method[0] != 0)
    {
        USERAUTH_PMGR_SetAuthMethod(method);
    }
    exit:

}

static void SECURITY_Backdoor_UserAuth_ShowAuthMethod(int argc, char** argv)
{
    int i;
    USERAUTH_Auth_Method_T      method[USERAUTH_NUMBER_Of_AUTH_METHOD];

    if (FALSE == USERAUTH_PMGR_GetAuthMethod(method))
        goto exit;;

    for (i = 0; USERAUTH_NUMBER_Of_AUTH_METHOD > i; ++i)
    {
        switch (method[i])
        {
            case USERAUTH_AUTH_LOCAL:
                BACKDOOR_MGR_Printf("\r\nlocal");
                break;
            case USERAUTH_AUTH_RADIUS:
                BACKDOOR_MGR_Printf("\r\nradius");
                break;
            case USERAUTH_AUTH_TACACS:
                BACKDOOR_MGR_Printf("\r\ntacacs+");
                break;
			default:
				break;
        }
    }
    exit:

}

#endif /* USERAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  SECURITY_Backdoor_CleanWarningDummyFunc
 *-------------------------------------------------------------------------
 * PURPOSE  : to avoid compiler warnings because some backdoor may be disabled and static functions may not be used
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void SECURITY_Backdoor_CleanWarningDummyFunc()
{
    SECURITY_Backdoor_ShowIpAddr(NULL);
    SECURITY_Backdoor_Str2IpAddr(NULL, NULL);
    SECURITY_Backdoor_ShowMacAddr(NULL);
    SECURITY_Backdoor_Str2MacAddr(NULL, NULL);
    SECURITY_Backdoor_ShowRowStatus(0);
    SECURITY_Backdoor_Help(0, NULL);
    SECURITY_Backdoor_StrCmpIgnoreCase(NULL, NULL);
    SECURITY_Backdoor_ChopString(NULL);
}


#endif /* SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE */
