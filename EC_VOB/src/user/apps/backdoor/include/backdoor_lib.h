/* MODULE NAME:  backdoor_lib.h
 * PURPOSE:
 *   This module includes utility functions for backdoor.
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
#ifndef BACKDOOR_LIB_H
#define BACKDOOR_LIB_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* maximum number of arguments that are allowed
 * to be used in command list while using BACKDOOR_LIB_CliShell()
 */
#define BACKDOOR_ARG_MAX_NUM 10

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    const char *prompt;
    char *buf;     /* required */
    int buf_size;  /* required */
    int (*getline)(char *s, int size); /* return value should be length of the line, but is not used now */
    int (*printf)(const char *format, ...); /* return value is not used now */
} BACKDOOR_LIB_SHELL_T;

typedef struct BACKDOOR_LIB_SHELL_CMD_S BACKDOOR_LIB_SHELL_CMD_T;
struct BACKDOOR_LIB_SHELL_CMD_S
{
    const char *name;  /* required */
    void *cookie;
    int (*func)(const BACKDOOR_LIB_SHELL_CMD_T *cmd, int argc, char *argv[]);
    const char *usage;
    const char *help;
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void BACKDOOR_LIB_CliShell(BACKDOOR_LIB_SHELL_T *shell, const BACKDOOR_LIB_SHELL_CMD_T cmd_list[], int cmd_count);

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
BOOL_T BACKDOOR_LIB_RequestBool(const char *prompt, BOOL_T dfl_val);

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
UI32_T BACKDOOR_LIB_RequestUI32(const char *prompt, UI32_T dfl_val);

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
UI8_T *BACKDOOR_LIB_RequestMacAddr(const char *prompt, UI8_T *addr);

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
UI32_T BACKDOOR_LIB_RequestHex(const char *prompt, UI32_T dfl_val);

#endif    /* End of BACKDOOR_LIB_H */

