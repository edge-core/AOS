/* MODULE NAME:  backdoor_lib.c
 * PURPOSE:
 *    This module includes utility functions for backdoor.
 * 
 * NOTES:
 *
 * REASON:
 * Description:
 * CREATOR:
 * HISTORY
 *    12/8/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "backdoor_lib.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int backdoor_builtin_getline(char *s, int size);

/* STATIC VARIABLE DECLARATIONS
 */
static const BACKDOOR_LIB_SHELL_CMD_T backdoor_cmd_builtin[] = {
    /* name, func, cookie, usage, help */
    { "help", NULL, NULL, NULL, "show command list." },
    { "exit", NULL, NULL, NULL, "exit backdoor." },
/*  { "dummy", NULL, NULL, NULL, "for test." }, */
};

/* EXPORTED SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * FUNCTION NAME - BACKDOOR_LIB_CliShell
 * -------------------------------------------------------------------------
 * PURPOSE : A backdoor shell CLI will be executed after calling this function.
 * INPUT   : shell         --  shell->prompt  : prompt string for backdoor shell CLI (optinal, NULL is allowed)
 *                             shell->buf     : temporary buffer for cli input (mandatory)
 *                             shell->buf_size: size of shell->buf (mandatory)
 *                             shell->getline : function pointer for getting a line of string (optinal, NULL is allowed)
 *                             shell->printf  : function pointer for output messages (optinal, NULL is allowed)
 *           cmd_list      --  array of command list
 *           cmd_count     --  number of element in cmd_list
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void BACKDOOR_LIB_CliShell(BACKDOOR_LIB_SHELL_T *shell, const BACKDOOR_LIB_SHELL_CMD_T cmd_list[], int cmd_count)
{
#define builtin_cmd_count   (sizeof(backdoor_cmd_builtin)/sizeof(*backdoor_cmd_builtin))
#define backdoor_cli_getline()      shell->getline(shell->buf, shell->buf_size)
#define backdoor_cli_printf(...)    shell->printf(__VA_ARGS__)

    const BACKDOOR_LIB_SHELL_CMD_T *cmd_list_ar[] = { backdoor_cmd_builtin, cmd_list };
    const int cmd_count_ar[] = { builtin_cmd_count, cmd_count};
    const BACKDOOR_LIB_SHELL_CMD_T *cmd;
    char delims[] = " ";
    char comment_char = '#';
    char *p, *buf, *name, *argv[BACKDOOR_ARG_MAX_NUM];
    int i, /*len,*/ argc, list_idx, cmd_idx, ret, eof;

    if (shell->prompt == NULL)
        shell->prompt = "backdoor$ ";
    if (shell->getline == NULL)
        shell->getline = backdoor_builtin_getline;
    if (shell->printf == NULL)
        shell->printf = (int (*)(const char*, ...))BACKDOOR_MGR_Printf;

    for (eof = 0; !eof; )
    {
        backdoor_cli_printf("%s", shell->prompt);
        /*len = */backdoor_cli_getline();
        buf = shell->buf;

        /* parse input.
         * IN  : buf, len
         * OUT : name, argc, argv
         */
        name = NULL;
        argc = 0;
        if ((p = strtok(buf, delims)) != NULL && *p != comment_char)
        {
            name = p;
            while ((p = strtok(NULL, delims)) != NULL && *p != comment_char)
                argv[argc++] = p;
        }

        /* find command.
         * IN  : name
         * OUT : cmd
         */
        cmd = NULL;
        if (name != NULL)
        {
            for (list_idx = 0; list_idx < sizeof(cmd_list_ar)/sizeof(*cmd_list_ar); list_idx++)
            {
                for (cmd_idx = 0; cmd_idx < cmd_count_ar[list_idx]; cmd_idx++)
                {
                    if (strcmp(name, cmd_list_ar[list_idx][cmd_idx].name) == 0)
                    {
                        cmd = &cmd_list_ar[list_idx][cmd_idx];
                        goto end_of_find_cmd;
                    }
                }
            }
            end_of_find_cmd:;
        }

        /* execute command.
         * IN  : cmd, argc, argv
         * OUT : ret, eof
         */
        ret = 1;
        if (cmd != NULL)
        {
            if (cmd->func)
            {
                ret = cmd->func(cmd, argc, argv);
            }
            else
            {
                /* builtin commands */
                if (strcmp(cmd->name, "exit") == 0)
                {
                    eof = 1;
                    ret = 0;
                }
                else if (strcmp(cmd->name, "help") == 0)
                {
                    for (list_idx = 0; list_idx < sizeof(cmd_list_ar)/sizeof(*cmd_list_ar); list_idx++)
                    {
                        for (cmd_idx = 0; cmd_idx < cmd_count_ar[list_idx]; cmd_idx++)
                        {
                            backdoor_cli_printf("%-10s %s\n",
                                cmd_list_ar[list_idx][cmd_idx].name,
                                cmd_list_ar[list_idx][cmd_idx].help ?
                                    cmd_list_ar[list_idx][cmd_idx].help:
                                    cmd_list_ar[list_idx][cmd_idx].name);
                        }
                    }
                    ret = 0;
                }
                else if (strcmp(cmd->name, "dummy") == 0)
                {
                    backdoor_cli_printf("name = '%s'\n", cmd->name);
                    for (i = 0; i < argc; i++)
                        backdoor_cli_printf("arg[%d] = '%s'\n", i, argv[i]);
                    ret = 0;
                }
            }
        } /* end of if (cmd) */

        /* handle error.
         * IN  : name, cmd, ret
         * OUT : N/A
         */
        if (name != NULL && cmd == NULL)
            backdoor_cli_printf("unknown command.\n");

        if (cmd != NULL && ret != 0)
            backdoor_cli_printf("%s %s\n%s\n",
                cmd->name,
                cmd->usage ? cmd->usage : cmd->name,
                cmd->help ? cmd->help : cmd->name);
    } /* end of main loop */
} /* end of backdoor_cli */

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_LIB_RequestBool
 *-----------------------------------------------------------------
 * FUNCTION: To get an boolean value from UI.
 * INPUT   : prompt        - prompt string
 *           dfl_val       - default value
 * OUTPUT  : None
 * RETURN  : the value got from UI.
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_LIB_RequestBool(const char *prompt, BOOL_T dfl_val)
{
    int val;
    char buf[12];

    BACKDOOR_MGR_Printf("%s (1:TRUE 0:FALSE): ", prompt);

    if (!BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1))
    {
        buf[0] = 0;
    }

    if (1 != sscanf(buf, "%d", &val) || val < 0 || val > 1)
    {
        val = !!dfl_val;
        BACKDOOR_MGR_Printf(" ==> %s", val ? "TRUE" : "FALSE");
    }

    BACKDOOR_MGR_Print("\n");

    return val;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_LIB_RequestUI32
 *-----------------------------------------------------------------
 * FUNCTION: To get an UI32_T value from UI.
 * INPUT   : prompt        - prompt string
 *           dfl_val       - default value
 * OUTPUT  : None
 * RETURN  : the value got from UI.
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
UI32_T BACKDOOR_LIB_RequestUI32(const char *prompt, UI32_T dfl_val)
{
    unsigned long val;
    char buf[12];

    BACKDOOR_MGR_Printf("%s: ", prompt);

    if (!BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1))
    {
        buf[0] = 0;
    }

    if (1 != sscanf(buf, "%lu", &val))
    {
        val = dfl_val;
        BACKDOOR_MGR_Printf(" ==> %lu", val);
    }

    BACKDOOR_MGR_Print("\n");

    return val;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_LIB_RequestHex
 *-----------------------------------------------------------------
 * FUNCTION: To get an HEX value from UI.
 * INPUT   : prompt        - prompt string
 *           dfl_val       - default value
 * OUTPUT  : None
 * RETURN  : the value got from UI.
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
UI32_T BACKDOOR_LIB_RequestHex(const char *prompt, UI32_T dfl_val)
{
    unsigned long val;
    char buf[12];

    BACKDOOR_MGR_Printf("%s: ", prompt);

    if (!BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1))
    {
        buf[0] = 0;
    }

    if (1 != sscanf(buf, "%lx", &val))
    {
        val = dfl_val;
        BACKDOOR_MGR_Printf(" ==> 0x%lx", val);
    }

    BACKDOOR_MGR_Print("\n");

    return val;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_LIB_RequestMacAddr
 *-----------------------------------------------------------------
 * FUNCTION: To get a MAC address from UI.
 * INPUT   : prompt        - prompt string
 *           addr          - default value
 * OUTPUT  : addr          - the value got from UI.
 * RETURN  : the value got from UI. (the same with output)
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
UI8_T *BACKDOOR_LIB_RequestMacAddr(const char *prompt, UI8_T *addr)
{
#define _HEX2INT(ch) \
    (isdigit(ch) ? (ch - '0') : \
        (10 + (islower(ch) ? (ch - 'a') : (ch - 'A'))))


    char buf[18];
    int i, j, k;

    if (addr == NULL)
    {
        return NULL;
    }

    BACKDOOR_MGR_Printf("%s: ", prompt);

    if (!BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1))
    {
        buf[0] = 0;
    }

    memset(addr, 0, SYS_ADPT_MAC_ADDR_LEN);

    for (i = strlen(buf) - 1, j = SYS_ADPT_MAC_ADDR_LEN - 1, k = 0; i >= 0 && j >= 0; i--)
    {
        if (isxdigit(buf[i]))
        {
            addr[j] += _HEX2INT(buf[i]) << (k << 2);
            j -= k;
            k ^= 1;
        }
        else
        {
            j -= k;
            k = 0;
        }
    }

    BACKDOOR_MGR_Printf(" ==> %02x:%02x:%02x:%02x:%02x:%02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    return addr;
}


/* LOCAL SUBPROGRAM BODIES
 */
static int backdoor_builtin_getline(char *s, int size)
{
#if 1
    if (size < 1)
    {
        return 0;
    }
    if (size < 2)
    {
        s[0] = 0;
        return 0;
    }

    BACKDOOR_MGR_RequestKeyIn(s, size - 1);
    BACKDOOR_MGR_Printf("\n");
    return 1;

#else

    int len = 0;
    if (fgets(s, size, stdin))
    {
        len = strlen(s);
        if (s[len-1] == '\n')
            s[--len] = 0;
    }
    else
    {
        if (size > 0)
            s[0] = 0;
    }
    return len;
#endif
}

