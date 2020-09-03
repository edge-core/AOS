#ifndef CLI_PARS_H
#define CLI_PARS_H

#include "sys_type.h"

#define CLI_PARS_ARG_MATCH       0
#define CLI_PARS_ARG_SUBMATCH    1
#define CLI_PARS_ARG_NOMATCH     2
#define CLI_PARS_ARG_NOIMPL     (-1)

typedef struct
{
    const char *buf;
    UI32_T len;
} CLI_PARS_Token_T;

typedef struct
{
    CLI_PARS_Token_T        token;
    UI16_T                  err_kind;
} CLI_PARS_Result_T;

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_Token_Each
 *---------------------------------------------------------------------------
 * PURPOSE  : Filter token
 * INPUT    : token - input token
 *            func  - filter function
 * OUTPUT   : None
 * RETURN   : matched token
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Token_T
CLI_PARS_Token_Each(
    const CLI_PARS_Token_T *token,
    int (*func)(int)
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_ReadDigit
 *---------------------------------------------------------------------------
 * PURPOSE  : Read [0-9] characters
 * INPUT    : token - input token
 * OUTPUT   : None
 * RETURN   : matched token
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Token_T
CLI_PARS_ReadDigit(
    const CLI_PARS_Token_T *token
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_ReadHexDigit
 *---------------------------------------------------------------------------
 * PURPOSE  : Read [0-9a-fA-F] characters
 * INPUT    : token - input token
 * OUTPUT   : None
 * RETURN   : matched token
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Token_T
CLI_PARS_ReadHexDigit(
    const CLI_PARS_Token_T *token
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_ReadChar
 *---------------------------------------------------------------------------
 * PURPOSE  : Read the specified character
 * INPUT    : token - input token
 * OUTPUT   : None
 * RETURN   : matched token
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Token_T
CLI_PARS_ReadChar(
    const CLI_PARS_Token_T *token,
    char c
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_Token_AddOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : Add offset
 * INPUT    : token     - base token
 * INPUT    : offset    - offset
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
void
CLI_PARS_Token_AddOffset(
    CLI_PARS_Token_T *base,
    const CLI_PARS_Token_T *offset
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_Token_ToUI32
 *---------------------------------------------------------------------------
 * PURPOSE  : Convert token to unsigned integer 32 bits value
 * INPUT    : token     - token
 * OUTPUT   : value     - output value
 * RETURN   : TRUE - if secceeded / FALSE - if failed (e.g., overflow)
 * NOTE     : The length of token should not equal 0
 *---------------------------------------------------------------------------
 */
BOOL_T
CLI_PARS_Token_ToUI32(
    const CLI_PARS_Token_T *token,
    UI32_T *val
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_Token_Digit
 *---------------------------------------------------------------------------
 * PURPOSE  : Parse a decimal digit token
 * INPUT    : token             - token
 *            min_quantifier    - min quantifier
 *            max_quantifier    - max quantifier
 * OUTPUT   : None
 * RETURN   : err_kind = CLI_NO_ERROR - if secceeded / FALSE - if failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Result_T
CLI_PARS_Token_Digit(
    const CLI_PARS_Token_T *token,
    UI32_T min_quantifier,
    UI32_T max_quantifier
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_Token_Hex
 *---------------------------------------------------------------------------
 * PURPOSE  : Parse a hexdecimal digit token
 * INPUT    : token             - token
 *            min_quantifier    - min quantifier
 *            max_quantifier    - max quantifier
 * OUTPUT   : None
 * RETURN   : err_kind = CLI_NO_ERROR - if secceeded / FALSE - if failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
CLI_PARS_Result_T
CLI_PARS_Token_Hex(
    const CLI_PARS_Token_T *token,
    UI32_T min_quantifier,
    UI32_T max_quantifier
);

UI8_T  CLI_PARS_GetKeyState(CLI_TASK_WorkingArea_T *ctrl_P);
void   CLI_PARS_ResetKeyState(CLI_TASK_WorkingArea_T *ctrl_P);
void   CLI_PARS_SetKeyState(UI8_T status, CLI_TASK_WorkingArea_T *ctrl_P);
UI16_T CLI_PARS_GetKey(UI8_T *ch);
UI16_T CLI_PARS_TransLowUpper(char *buf, I16_T si, UI16_T key_type);
UI16_T CLI_PARS_GetKeyForwardSize(char *buf, I16_T si);
UI16_T CLI_PARS_GetKeyBackwardSize(char *buf, I16_T si);
UI16_T CLI_PARS_ReadLine(char *buff, I16_T bufsize, BOOL_T Is_CtrlKey_Skipped, BOOL_T Is_Hidden);
UI16_T CLI_PARS_GetAWord(char *buff, UI16_T *str_idx, char *word_buf);
UI16_T CLI_PARS_GetWords(UI8_T *buff, UI8_T *words[], UI16_T widx[], UI16_T max);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_GetNeiborCommandIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Get neibor command index from starting index
 * INPUT    : cmd_lst       - command list
 *            start_index   - starting command index
 * OUTPUT   : None
 * RETURN   : command index or NULL_CMD
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
UI16_T
CLI_PARS_GetNeiborCommandIndex(
    const CMDLIST * cmd_list,
    UI16_T start_idx
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_GetCommandIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Get command index from starting index
 * INPUT    : cmd_lst       - command list
 *            start_index   - starting command index
 * OUTPUT   : None
 * RETURN   : command index or NULL_CMD
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
UI16_T
CLI_PARS_GetCommandIndex(
    const CMDLIST * cmd_list,
    UI16_T start_idx
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_LookupCmdWord
 *---------------------------------------------------------------------------
 * PURPOSE  : Lookup a command word in the command words list
 * ARGUMENT : word_buf  - word pointer
 *            start_idx - the start point that to be searched
 *            near_idx  - the nearest matched command word index
 * OUTPUT   : None
 * RETURN   : CLI_NO_ERROR          => when exactly match one command word
 *            CLI_ERR_CMD_INVALID   => when not match any command word
 *            CLI_ERR_CMD_AMBIGUOUS => when found ambiguous words
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
UI16_T
CLI_PARS_LookupCmdWord(
    char *word_buf,
    UI16_T start_idx,
    UI16_T *near_idx
);

UI8_T  *CLI_PARS_TOKEN_UTIL_Get_Token(UI8_T *s, UI8_T *Token);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_RegisterArgumentParseHandler
 *---------------------------------------------------------------------------
 * PURPOSE  : Register parse function for argument
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Invoke this function before use CLI_PARS_ParseArgument and
 *            CLI_PARS_GetArgumentHelpMessage.
 *---------------------------------------------------------------------------
 */
void
CLI_PARS_RegisterArgumentParseHandler();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_ParseArgument
 *---------------------------------------------------------------------------
 * PURPOSE  : Parse an argument
 * INPUT    : token     - Argument. The token may not a null-terminal string
 *            token_len - String of the argument
 *            arg_list  - Agurment array list
 *            arg_idx   - Current argument index
 * OUTPUT   : match_len - How many characters matched
 *            err_kind  - The kind of error
 * RETURN   : ARG_MATCH when exactly match
 *            ARG_SUBMATCH when sub match
 *            ARG_NOMATCH  when no match
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
int
CLI_PARS_ParseArgument(
    const char *token,
    UI32_T token_len,
    const ARGLIST arg_list[],
    UI16_T  arg_idx,
    UI16_T *match_len,
    UI16_T *err_kind
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PARS_GetArgumentHelpMessage
 *---------------------------------------------------------------------------
 * PURPOSE  : Get help message of an argument
 * INPUT    : token     - Argument. The token may not a null-terminal string
 *            token_len - String of the argument
 *            arg_list  - Agurment array list
 *            arg_idx   - Current argument index
 *            name_len  - Buffer size of name
 *            help_len  - Buffer size fo help
 * OUTPUT   : name      - Help message for 1st column
 *            help      - Help message for 2nd column
 * RETURN   : ARG_MATCH when exactly match
 *            ARG_SUBMATCH when sub match
 *            ARG_NOMATCH  when no match
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
int
CLI_PARS_GetArgumentHelpMessage(
    const char *token,
    UI32_T token_len,
    const ARGLIST arg_list[],
    UI16_T  arg_idx,
    char *name,
    UI32_T name_len,
    char *help,
    UI32_T help_len
);

#endif
