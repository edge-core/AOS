/* FUNCTION NAME: add_backdoor.h
 * PURPOSE:
 *	1. ADD backdoor
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ADD == TRUE)

#ifdef LINUX_FRAMER
#include "l_threadgrp.h"
#endif
#include "sysfun.h"
#include "sys_bld.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "backdoor_mgr.h"
#include "add_backdoor.h"
#include "add_utest.h"
#include "add_mgr.h"
#include "leaf_es3626a.h"
#include "add_om.h"
#include "amtr_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/*#define TEST_TASK_BEHAVIOR*/

#define ADD_BACKDOOR_PROMPT             "\r\ncommand > "

/* MACRO FUNCTION DECLARATIONS
 */
#define GET_TOKEN_AND_MOVE_NEXT(str, token, pos) { \
        *pos = '\0'; token = str; str = pos + 1; }

#define IS_DIGIT(c)                     (('0' <= c) && (c <= '9'))
#define IS_UPALPHA(c)                   (('A' <= c) && (c <= 'Z'))
#define IS_LOALPHA(c)                   (('a' <= c) && (c <= 'z'))
#define IS_ALPHA(c)                     (IS_UPALPHA(c) || IS_LOALPHA(c))


/* DATA TYPE DECLARATIONS
 */
typedef void (*ADDFuncPtr)(int argc, char** argv);

typedef struct ADD_Backdoor_Cmd_S
{
    const char*     cmd;
    ADDFuncPtr exec;
} ADD_Backdoor_Cmd_T;

typedef struct ADD_Backdoor_Menu_S
{
    const char*     name;
    const int       nbr_of_sub_menus;
    const int       nbr_of_cmd_items;

    struct ADD_Backdoor_Menu_S *parent_menu;
    struct ADD_Backdoor_Menu_S **sub_menus;
    ADD_Backdoor_Cmd_T         *cmd_items;
} ADD_Backdoor_Menu_T;


/* ---------------------------------------------------------------------
 * common functions
 * --------------------------------------------------------------------- */
static BOOL_T Add_Backdoor_Str2MacAddr(UI8_T *s, UI8_T *Vals);
static void ADD_Backdoor_Help(int argc, char** argv);
static int ADD_Backdoor_StrCmpIgnoreCase(const UI8_T *dst, const UI8_T *src);
static BOOL_T ADD_Backdoor_ChopString(UI8_T *str);


/* ---------------------------------------------------------------------
 * backdoor command functions
 * --------------------------------------------------------------------- */
 #if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)
    static void ADD_Backdoor_SetVoiceVlanEnabledId(int argc, char** argv);
    static void ADD_Backdoor_SetVoiceVlanPortMode(int argc, char** argv);
    static void ADD_Backdoor_SetVoiceVlanPortSecurityState(int argc, char** argv);
    static void ADD_Backdoor_SetVoiceVlanPortProtocol(int argc, char** argv);
    static void ADD_Backdoor_ClearVoiceVlanPortProtocol(int argc, char** argv);
    static void ADD_Backdoor_SetVoiceVlanPortPriority(int argc, char** argv);
    static void ADD_Backdoor_SetVoiceVlanTimeout(int argc, char** argv);
    static void ADD_Backdoor_ShowVoiceVlanStatus(int argc, char** argv);
    static void ADD_Backdoor_AddOui(int argc, char** argv);
    static void ADD_Backdoor_DelOui(int argc, char** argv);
    static void ADD_Backdoor_SetOuiState(int argc, char** argv);
    static void ADD_Backdoor_SetOuiMaskAddress(int argc, char** argv);
    static void ADD_Backdoor_SetOuiDescription(int argc, char** argv);
    static void ADD_Backdoor_ShowOui(int argc, char** argv);
    static void ADD_Backdoor_SetDebugPrint(int argc, char** argv);

    #ifdef TEST_TASK_BEHAVIOR
        static void ADD_Backdoor_SpawnChildTask(int argc, char** argv);
        static void ADD_Backdoor_DeleteChildTask(int argc, char** argv);
        static void ADD_Backdoor_DeleteChildTaskByTaskId(int argc, char** argv);
    #endif /*#ifdef TEST_TASK_BEHAVIOR*/
#endif /* #if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE) */


#if (ADD_DO_UNIT_TEST == TRUE)
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
static void ADD_Backdoor_RunVoiceVlanUTest(int argc, char** argv);
#endif /* #if (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
#endif /* #if (ADD_DO_UNIT_TEST == TRUE) */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)

/* Voice VLAN backdoor command items */
static ADD_Backdoor_Cmd_T  voice_vlan_command_items[] = {
        {"help", ADD_Backdoor_Help},
        {"set_voice_vlan_enabled_id {vid}", ADD_Backdoor_SetVoiceVlanEnabledId},
        {"set_voice_vlan_port_mode {port} {none|manual|auto}", ADD_Backdoor_SetVoiceVlanPortMode},
        {"set_voice_vlan_port_security_state {port} {enable|disable}", ADD_Backdoor_SetVoiceVlanPortSecurityState},
        {"set_voice_vlan_port_protocol {port} {oui|lldp}", ADD_Backdoor_SetVoiceVlanPortProtocol},
        {"clear_voice_vlan_port_protocol {port} {oui|lldp|dhcp}", ADD_Backdoor_ClearVoiceVlanPortProtocol},
        {"set_voice_vlan_port_priority {port} {priority}", ADD_Backdoor_SetVoiceVlanPortPriority},
        {"set_voice_vlan_timeout {{day hour minute}|mintue}", ADD_Backdoor_SetVoiceVlanTimeout},
        {"show_voice_vlan_status", ADD_Backdoor_ShowVoiceVlanStatus},
        {"add_oui {oui} {mask} {description} [{sys_def}]", ADD_Backdoor_AddOui},
        {"del_oui {oui} {mask}", ADD_Backdoor_DelOui},
        {"set_oui_state oui {invalid|valid}", ADD_Backdoor_SetOuiState},
        {"set_oui_mask_address oui mask", ADD_Backdoor_SetOuiMaskAddress},
        {"set_oui_description oui description", ADD_Backdoor_SetOuiDescription},
        {"show_oui", ADD_Backdoor_ShowOui},
        {"set_debug_print {enable|disable}", ADD_Backdoor_SetDebugPrint},
#ifdef TEST_TASK_BEHAVIOR
        {"spawn_child_task", ADD_Backdoor_SpawnChildTask},
        {"delete_child_task", ADD_Backdoor_DeleteChildTask},
        {"delete_child_task_by_task_id", ADD_Backdoor_DeleteChildTaskByTaskId},
#endif /*#ifdef TEST_TASK_BEHAVIOR*/
    };

/* Voice VLAN backdoor menu */
static ADD_Backdoor_Menu_T voice_vlan_menu[] = {
    {"voice vlan", 0, sizeof(voice_vlan_command_items)/sizeof(ADD_Backdoor_Cmd_T), NULL, NULL, voice_vlan_command_items},
};
#endif /*#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)*/

#if (ADD_DO_UNIT_TEST == TRUE)
static ADD_Backdoor_Cmd_T  utest_command_items[] = {
    {"help", ADD_Backdoor_Help},
        #if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
        {"run_utest_for_voice_vlan", ADD_Backdoor_RunVoiceVlanUTest},
        #endif /* #if (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
    };

static ADD_Backdoor_Menu_T utest_menu[] = {
    {"unit test", 0, sizeof(utest_command_items)/sizeof(ADD_Backdoor_Cmd_T), NULL, NULL, utest_command_items},
};
#endif /* #if (ADD_DO_UNIT_TEST == TRUE) */



/* add backdoor main menu */

static ADD_Backdoor_Cmd_T  main_command_items[] = {
    {"help", ADD_Backdoor_Help},
};


static ADD_Backdoor_Menu_T *sub_menu_of_main_menu[] = {
#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)
        voice_vlan_menu,
#endif /*#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)*/
#if (ADD_DO_UNIT_TEST == TRUE)
        utest_menu,
#endif /* #if (ADD_DO_UNIT_TEST == TRUE) */
    };

static ADD_Backdoor_Menu_T main_menu[] = {
    {
        "add",
        sizeof(sub_menu_of_main_menu)/sizeof(ADD_Backdoor_Menu_T*),
        sizeof(main_command_items)/sizeof(ADD_Backdoor_Cmd_T),
        NULL,
        sub_menu_of_main_menu,
        main_command_items
    },
};


#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)
#if 0
static void ADD_Backdoor_SetVoiceVlanState(int argc, char** argv)
{
    BOOL_T state;
    if(argc != 1)
    {
        printf("\r\nparameters: {enable|disable}");
        return;
    }

    if(strcmp(argv[0], "enable") == 0)
        state = TRUE;
    else if(strcmp(argv[0], "disable") == 0)
        state = FALSE;
    else
    {
        printf("\r\nerror state.");
        return;
    }

    if(ADD_MGR_SetVoiceVlanState(state) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}
#endif

static void ADD_Backdoor_SetVoiceVlanEnabledId(int argc, char** argv)
{
    if(argc != 1)
    {
        printf("\r\nparameters: {vid}");
        return;
    }

    if(ADD_MGR_SetVoiceVlanEnabledId(atoi(argv[0])) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetVoiceVlanPortMode(int argc, char** argv)
{
    UI32_T lport;
    UI32_T mode;
    if(argc != 2)
    {
        printf("\r\nparameters: {port} {none|manual|auto}");
        return;
    }

    lport = atoi(argv[0]);

    if(strcmp(argv[1], "none")==0)
        mode = VAL_voiceVlanPortMode_none;
    else if(strcmp(argv[1], "manual")==0)
        mode = VAL_voiceVlanPortMode_manual;
    else if(strcmp(argv[1], "auto")==0)
        mode = VAL_voiceVlanPortMode_auto;
    else
    {
        printf("\r\nerror mode type.");
        return;
    }

    if(ADD_MGR_SetVoiceVlanPortMode(lport, mode) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetVoiceVlanPortSecurityState(int argc, char** argv)
{
    UI32_T lport;
    UI32_T state;
    if(argc != 2)
    {
        printf("\r\nparameters: {port} {enable|disable}");
        return;
    }

    lport = atoi(argv[0]);

    if(strcmp(argv[1], "enable")==0)
        state = VAL_voiceVlanPortSecurity_enabled;
    else if(strcmp(argv[1], "disable")==0)
        state = VAL_voiceVlanPortSecurity_disabled;
    else
    {
        printf("\r\nerror security state.");
        return;
    }

    if(ADD_MGR_SetVoiceVlanPortSecurityState(lport, state) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetVoiceVlanPortProtocol(int argc, char** argv)
{
    UI32_T lport;
    BOOL_T ret = FALSE;
    if(argc != 2)
    {
        printf("\r\nparameters: {port} {oui|lldp}");
        return;
    }

    lport = atoi(argv[0]);

    if(strcmp(argv[1], "oui") == 0)
    {
        ret = ADD_MGR_SetVoiceVlanPortOuiRuleState(lport, VAL_voiceVlanPortRuleOui_enabled);
    }
    else if(strcmp(argv[1], "lldp") == 0)
    {
        ret = ADD_MGR_SetVoiceVlanPortLldpRuleState(lport, VAL_voiceVlanPortRuleLldp_enabled);
    }
    else
    {
        printf("\r\nError protocol type.");
        return;
    }


    if(TRUE == ret)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_ClearVoiceVlanPortProtocol(int argc, char** argv)
{
    UI32_T lport;
    BOOL_T ret = FALSE;
    if(argc != 2)
    {
        printf("\r\nparameters: {port} {oui|lldp}");
        return;
    }

    lport = atoi(argv[0]);

    if(strcmp(argv[1], "oui") == 0)
    {
        ret = ADD_MGR_SetVoiceVlanPortOuiRuleState(lport, VAL_voiceVlanPortRuleOui_disabled);
    }
    else if(strcmp(argv[1], "lldp") == 0)
    {
        ret = ADD_MGR_SetVoiceVlanPortLldpRuleState(lport, VAL_voiceVlanPortRuleLldp_disabled);
    }
    else
    {
        printf("\r\nerror protocol type.");
        return;
    }

    if(TRUE == ret)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetVoiceVlanPortPriority(int argc, char** argv)
{
    UI32_T lport;
    UI8_T  priority;

    if(argc != 2)
    {
        printf("\r\nparameters: {port} {priority}");
        return;
    }

    lport = atoi(argv[0]);
    priority = atoi(argv[1]);

    if(ADD_MGR_SetVoiceVlanPortPriority(lport, priority) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetVoiceVlanTimeout(int argc, char** argv)
{
    if((argc != 1) && (argc != 3))
    {
        printf("\r\nparameters: {{day hour minute}|mintue}");
        return;
    }

    if(argc == 1)
    {
        UI32_T timeout;
        timeout = atoi(argv[0]);

        if(ADD_MGR_SetVoiceVlanAgingTime(timeout) == TRUE)
        {
            printf("\r\nSucceeds");
        }
        else
        {
            printf("\r\nFails");
        }
    }
    else
    {
        UI32_T day, hour, minute;
        day    = atoi(argv[0]);
        hour   = atoi(argv[1]);
        minute = atoi(argv[2]);
        if(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(day, hour, minute) == TRUE)
        {
            printf("\r\nSucceeds");
        }
        else
        {
            printf("\r\nFails");
        }
    }
}

static void ADD_Backdoor_ShowVoiceVlanStatus(int argc, char** argv)
{
    UI32_T from, to;
    UI8_T  rx_chr;
    from = 0;
    to   = 14;
    ADD_OM_ShowVoiceVlanStatusDebug(from, to);

    printf("\r\n---More---");

    rx_chr = getchar();

    if(rx_chr == ' ')
    {
        from = to+1;
        to   = SYS_ADPT_TOTAL_NBR_OF_LPORT;
    }
    else
        return;

    ADD_OM_ShowVoiceVlanStatusDebug(from, to);
}

static void ADD_Backdoor_AddOui(int argc, char** argv)
{
    ADD_MGR_VoiceVlanOui_T oui_entry;

    memset(&oui_entry, 0, sizeof(oui_entry));

    if((argc != 2) && (argc != 3))
    {
        printf("\r\nparameters: {oui} {mask} [description]");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[0], oui_entry.oui) == FALSE)
    {
        printf("\r\nerror oui address.");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[1], oui_entry.mask) == FALSE)
    {
        printf("\r\nerror mask address.");
        return;
    }

    if(argc == 3)
    {
        strncpy(oui_entry.description, argv[2], SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
    }

    if(ADD_MGR_AddOuiEntry(&oui_entry) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_DelOui(int argc, char** argv)
{
    UI8_T oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN];

    if(argc != 2)
    {
        printf("\r\nparameters: {oui} {mask}");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[0], oui) == FALSE)
    {
        printf("\r\nerror oui address.");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[1], mask) == FALSE)
    {
        printf("\r\nerror mask address.");
        return;
    }

    if(ADD_MGR_RemoveOuiEntry(oui, mask) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }

}

static void ADD_Backdoor_SetOuiState(int argc, char** argv)
{
    UI8_T oui[SYS_ADPT_MAC_ADDR_LEN];

    if(argc != 2)
    {
        printf("set_oui oui {invalid|valid}");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[0], oui) == FALSE)
    {
        printf("\r\nerror oui address.");
        return;
    }

    if(argv[1][0] == 'i' || argv[1][0] == 'I')
    {
        if(ADD_MGR_SetOuiState(oui, VAL_voiceVlanOuiStatus_invalid) == TRUE)
        {
            printf("\r\nSucceeds");
        }
        else
        {
            printf("\r\nFails");
        }
    }
    else if(argv[1][0] == 'v' || argv[1][0] == 'V')
    {
        if(ADD_MGR_SetOuiState(oui, VAL_voiceVlanOuiStatus_valid) == TRUE)
        {
            printf("\r\nSucceeds");
        }
        else
        {
            printf("\r\nFails");
        }
    }
}

static void ADD_Backdoor_SetOuiMaskAddress(int argc, char** argv)
{
    UI8_T oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN];

    if(argc != 2)
    {
        printf("set_oui oui mask");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[0], oui) == FALSE)
    {
        printf("\r\nerror oui address.");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[1], mask) == FALSE)
    {
        printf("\r\nerror mask address.");
        return;
    }

    if(ADD_MGR_SetOuiMaskAddress(oui, mask) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_SetOuiDescription(int argc, char** argv)
{
    UI8_T oui[SYS_ADPT_MAC_ADDR_LEN];

    if(argc != 2)
    {
        printf("set_oui oui description");
        return;
    }

    if( Add_Backdoor_Str2MacAddr(argv[0], oui) == FALSE)
    {
        printf("\r\nerror oui address.");
        return;
    }

    if(ADD_MGR_SetOuiDescription(oui, argv[1]) == TRUE)
    {
        printf("\r\nSucceeds");
    }
    else
    {
        printf("\r\nFails");
    }
}

static void ADD_Backdoor_ShowOui(int argc, char** argv)
{
    ADD_OM_ShowOui();
}

static void ADD_Backdoor_SetDebugPrint(int argc, char** argv)
{
    if(argc != 1)
    {
        printf("\r\nparameters: {enable|disable}");
        return;
    }

    if(argv[0][0] == 'e' || argv[0][0] == 'E')
    {
        ADD_MGR_SetDebugPrintStatus(TRUE);
        printf("\r\nDebugPrint = ON");
    }
    else if(argv[0][0] == 'd' || argv[0][0] == 'D')
    {
        ADD_MGR_SetDebugPrintStatus(FALSE);
        printf("\r\nDebugPrint = OFF");
    }
    else
        printf("\r\nparameters: {enable|disable}");
}

#ifdef TEST_TASK_BEHAVIOR
static BOOL_T delete_child_task;
static UI32_T child_task_id;
static void ADD_Backdoor_ChildBody()
{
    void* mm = malloc(2048);
    if(mm == NULL)
    {
        printf("\r\nmalloc fail");
    }

    while(1)
    {
        if(delete_child_task == TRUE)
            break;
    }
    free(mm);
}

static void ADD_Backdoor_SpawnChildTask(int argc, char** argv)
{
    delete_child_task = TRUE;
    SYSFUN_Sleep(10);
    delete_child_task = FALSE;

    if(SYSFUN_SpawnTask ("I am child",
                         SYS_BLD_ADD_TASK_PRIORITY,
                         SYS_BLD_TASK_COMMON_STACK_SIZE,
                         0,
                         ADD_Backdoor_ChildBody,
                         0,
                         &child_task_id) != SYSFUN_OK )
    {
        printf("\r\nSpawn child task fails");
    }
    else
    {
        printf("\r\nSuccess");
    }
}

static void ADD_Backdoor_DeleteChildTask(int argc, char** argv)
{
    delete_child_task = TRUE;
    SYSFUN_Sleep(10);
    child_task_id = 0;
}

static void ADD_Backdoor_DeleteChildTaskByTaskId(int argc, char** argv)
{
    if(0 != child_task_id)
    {
        if(SYSFUN_DeleteTask(child_task_id) != SYSFUN_OK)
        {
            printf("\r\nDelete child task fails");
        }
        else
        {
            printf("\r\nSuccess");
            child_task_id = 0;
        }
    }
}
#endif /*#ifdef TEST_TASK_BEHAVIOR*/

#endif /* #if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE) */


#if (ADD_DO_UNIT_TEST == TRUE)
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
static void ADD_Backdoor_RunVoiceVlanUTest(int argc, char** argv)
{
    printf("\r\nRun unit test for Voice VLAN:");
    ADD_UTEST_RunVoiceVlanUTest();
}
#endif /* #if (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
#endif /* #if (ADD_DO_UNIT_TEST == TRUE) */


static BOOL_T IsLess16(char c)
{
   if ( (c >= '0' && c<= '9') || (c >= 'A' && c<= 'F') || (c >= 'a' && c<= 'f') )
      return TRUE;
   else
      return FALSE;
}

static UI32_T AtoUl(UI8_T *s, int radix)
{
    int i;
    unsigned long n;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
        ;       /* skip white space */

    if (s[i] == '+' || s[i] == '-')
    {
        i++;    /* skip sign */
    }

    if (radix == 10)
    {
        for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
        {
            n = 10 * n + s[i] - '0';
        }
    }
    else if (radix == 16)
    {
        if ( (s[i] == '0') && (s[i+1] == 'x' || s[i+1] == 'X') ) // Charles,
           i=i+2;                                                // To skip the "0x" or "0X"


        for (n = 0;
            (s[i] >= '0' && s[i] <= '9') ||
            (s[i] >= 'A' && s[i] <= 'F') ||
            (s[i] >= 'a' && s[i] <= 'f');
            i++)
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                n = 16 * n + s[i] - '0';
            }
            else if (s[i] >= 'A' && s[i] <= 'F')
            {
                n = 16 * n + s[i] - 'A'+ 10;
            }
            else if (s[i] >= 'a' && s[i] <= 'f')
            {
                n = 16 * n + s[i] - 'a'+ 10;
            }
        }
    }
    return (n);
}

BOOL_T Add_Backdoor_Str2MacAddr(UI8_T *s, UI8_T *Vals)
{
   UI32_T i = 0;
   UI32_T count=0;
   UI32_T MinusCount=0;

   for ( i=0; i< strlen(s); i++)
   {
      if (!IsLess16(s[i]) && s[i] != '-')
      {
         return FALSE;
      }
      if (s[i] == '-')
      {
         if (!IsLess16(s[i+1]))
            return FALSE;
         MinusCount++;
      }
   }

   /*
     Convert Mac address XXXX-XXXX-XXXX into XX-XX-XX-XX-XX-XX or XXXXXXXXXXXX
     if (MinusCount==2) means Mac address XXXX-XXXX-XXXX
     if (MinusCount==5) means Mac address XX-XX-XX-XX-XX-XX
     if (MinusCount==0) means Mac address XXXXXXXXXXXX
   */
   if (MinusCount==0)
   {
       int i=0;
       UI8_T temp_string[18] = {0};

       count = 0;
       if(strlen(s)!=12)
       {
           return FALSE;
       }
       else
       {
           for(i=0;i<12;i++)
           {
               if(s[i]=='\b')
               {
                    return FALSE;
               }
               else
               {
                    if((i%2)==1)
                    {
                        temp_string[count]=s[i];
                        if(i!=11)
                        {
                            count++;
                            temp_string[count]='-';
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        temp_string[count]=s[i];
                    }
                    count++;
               }
           }
           temp_string[count+1]='\0';
           s = temp_string;
           MinusCount=5;
       }
   }

   if(MinusCount==5)
   {
       for (i=0;i<6;i++)
       {
          Vals[i] = (UI8_T) AtoUl(s, 16);
          s = strstr(s, "-");
          s++;
       }
   }
   else
   {
        return FALSE;
   }

   return TRUE;
}


/* ---------------------------------------------------------------------
 * backdoor operation variables
 * --------------------------------------------------------------------- */

static ADD_Backdoor_Menu_T *current_menu = NULL;


#if (ADD_SUPPORT_ACCTON_BACKDOOR == TRUE)

void ADD_Backdoor_Main_CallBack(void)
{
#ifdef LINUX_FRAMER
    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetNetaccessGroupTGHandle();
    UI32_T  member_id;

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("L_THREADGRP_Join failed");
        return;
    }
#endif

    ADD_Backdoor_Main();

#ifdef LINUX_FRAMER
    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_handle, member_id);
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  ADD_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : add backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void ADD_Backdoor_Main (void)
{
    const int   buf_len = 512;
    char        buffer[buf_len];
    const int   size = 20;
    char        *pos, *ptr, *token[size];
    int         cnt, i, j, max, shift;

/* initialize parent menu */
#if (VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR == TRUE)
    voice_vlan_menu[0].parent_menu = main_menu;
#endif /* RADIUS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#if (ADD_DO_UNIT_TEST == TRUE)
    utest_menu[0].parent_menu = main_menu;
#endif #if (ADD_DO_UNIT_TEST == TRUE)

    printf("\r\nsecurity backdoor");
        ADD_Backdoor_Help(0, NULL);

    while (TRUE)
    {
        printf(ADD_BACKDOOR_PROMPT);
        BACKDOOR_MGR_RequestKeyIn(buffer, buf_len);

        if ('\0' == buffer[0])
        {
            return;
        }

        if (!ADD_Backdoor_ChopString(buffer))
            continue;

        if (!ADD_Backdoor_StrCmpIgnoreCase(buffer, "?"))
        {
            ADD_Backdoor_Help(0, NULL);
            continue;
        }

        if (!ADD_Backdoor_StrCmpIgnoreCase(buffer, "quit"))
            return;

        if (!ADD_Backdoor_StrCmpIgnoreCase(buffer, "exit"))
        {
            if ((NULL == current_menu) || (NULL == current_menu->parent_menu))
                            return;

            current_menu = current_menu->parent_menu;
            ADD_Backdoor_Help(0, NULL);
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
            if (ADD_Backdoor_StrCmpIgnoreCase(token[0], current_menu->sub_menus[i]->name))
                continue;

            current_menu = current_menu->sub_menus[i];
            ADD_Backdoor_Help(0, NULL);
            break;
        }

        if (max > i)
            continue;

        /* command items */
        for (j = 0, max = current_menu->nbr_of_cmd_items; max > j; ++j)
        {
            if (ADD_Backdoor_StrCmpIgnoreCase(token[0], current_menu->cmd_items[j].cmd))
                continue;

            current_menu->cmd_items[j].exec(cnt, token + 1);
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

            ADD_Backdoor_Help(0, NULL);
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
                            ADD_Backdoor_Help(0, NULL);
                    continue;
                }

                i -= current_menu->nbr_of_sub_menus;
                if (current_menu->nbr_of_cmd_items > i)
                {
                                current_menu->cmd_items[i].exec(cnt, token + 1);
                    continue;
                }
            }

            printf("\r\nbackdoor: %s: command not found", token[0]);
        }
    }
} /* End of ADD_Backdoor_Main */
#endif /* #if (ADD_SUPPORT_ACCTON_BACKDOOR == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADD_Backdoor_Help
 *---------------------------------------------------------------------------
 * PURPOSE  : show current backdoor menu
 * INPUT    : argc, argv
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *---------------------------------------------------------------------------*/
static void ADD_Backdoor_Help(int argc, char** argv)
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

    printf("\r\n-----------------------------------------------");

    if (0 < current_menu->nbr_of_sub_menus)
            printf(sub_menu_fmt);

    for (i = 0, max = current_menu->nbr_of_sub_menus, tmp = max - 1; max > i; ++i)
    {
        if ((0 == i) || (tmp == i))
                fmt = menu_edge_fmt;
        else
            fmt = menu_mide_fmt;

                printf(fmt, i, current_menu->sub_menus[i]->name);
    }

    printf(menu_cmd_fmt);
    for (j = 0, max = current_menu->nbr_of_cmd_items, tmp = max - 1; max > j; ++j)
    {
        if ((0 == j) || (tmp == j))
                fmt = cmd_edge_fmt;
        else
            fmt = cmd_mide_fmt;

                printf(fmt, i + j, current_menu->cmd_items[j].cmd);
    }

    printf("\r\n\r\nexit -- back to up menu, quit -- back to accton backdoor");
    printf("\r\nq[q..] -- shortcut");
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - ADD_Backdoor_StrCmpIgnoreCase
 *-------------------------------------------------------------------------
 * PURPOSE  : compare two strings case-insensitivly
 * INPUT    : dst, src (terminate by '\0')
 * OUTPUT   : none
 * RETURN   : 0 -- equal, > 0 -- dst > src, < 0 -- small
 * NOTE     :
 *-------------------------------------------------------------------------*/
static int ADD_Backdoor_StrCmpIgnoreCase(const UI8_T *dst, const UI8_T *src)
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
 * FUNCTION NAME - ADD_Backdoor_ChopString
 *-------------------------------------------------------------------------
 * PURPOSE  : chop source string then put the result back
 * INPUT    : src_str (terminate by '\0')
 * OUTPUT   : src_str
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T ADD_Backdoor_ChopString(UI8_T *str)
{
    UI8_T *src_iter, *dst_iter, *dst_end;

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

#endif  /* #if (SYS_CPNT_ADD == TRUE) */

