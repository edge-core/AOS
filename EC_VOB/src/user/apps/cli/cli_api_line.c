#include "cli_api.h"
#include "cli_api_line.h"
#include "userauth_pmgr.h"
#include <stdio.h>
#include "l_md5.h"
#include "sys_mgr.h"

static UI32_T prt_console(UI32_T line_num);
static UI32_T prt_vty(UI32_T line_num);

/**************************************<<LINE>>**********************************************/
/*change mode*/
UI32_T CLI_API_Line(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    /* get argument
     */
    if (arg[0] != NULL)
    {
        if (*arg[0] == 'c' || *arg[0] == 'C')
        {
            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_LINE_CONSOLE_MODE;
        }
        else if (*arg[0] == 'v' || *arg[0] == 'V')
        {
            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_LINE_VTY_MODE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Line(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    if (arg[0] == 0 || arg[0][0]=='|')
    {

#if (SYS_CPNT_CLI_TERMINAL == TRUE)
        PROCESS_MORE(" Terminal Configuration for this session:\r\n");
        sprintf(buff, "  Length                         : %lu\r\n", ctrl_P->CMenu.length);
        PROCESS_MORE(buff);
        sprintf(buff, "  Width                          : %lu\r\n", ctrl_P->CMenu.width);
        PROCESS_MORE(buff);
        sprintf(buff, "  History Size                   : %lu\r\n", ctrl_P->CMenu.histsize);
        PROCESS_MORE(buff);
        sprintf(buff, "  Escape Character(ASCII-number) : %u\r\n", ctrl_P->CMenu.escape_character);
        PROCESS_MORE(buff);

        switch (ctrl_P->CMenu.terminal_type)
        {
            case VT100:
                PROCESS_MORE("  Terminal Type                  : VT100\r\n");
                break;

            case VT102:
                PROCESS_MORE("  Terminal Type                  : VT102\r\n");
                break;

            case ANSI_BBS:
                PROCESS_MORE("  Terminal Type                  : ANSI-BBS\r\n");
                break;
        }
        PROCESS_MORE("\r\n");
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/

        line_num = prt_console(line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        PROCESS_MORE("\r\n");

        line_num = prt_vty(line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }
    else
    {
        switch (*arg[0])
        {
            case 'c':
            case 'C':
                line_num = prt_console(line_num);

                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                break;

            case 'v':
            case 'V':
                line_num = prt_vty(line_num);

                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    return CLI_NO_ERROR;
}

/*configuration*/
UI32_T CLI_API_Login_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    USERAUTH_Login_Method_T login_method;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_LOGIN:
            /* login local
             */
            if ((arg[0]!=NULL) && (arg[0][0] == 'l' || arg[0][0] == 'L'))
            {
                login_method = USERAUTH_LOGIN_LOGIN_LOCAL;
            }
            else
            {
                login_method = USERAUTH_LOGIN_LOGIN;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_LOGIN:
            login_method = USERAUTH_LOGIN_NO_LOGIN;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (USERAUTH_PMGR_SetConsoleLoginMethod(login_method) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set login mode\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Password_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_PASSWORD:
            if ((arg[0]!=NULL) && (arg[1]!=NULL))
            {
                /* not encrypted yet
                 */
                if (arg[0][0] == '0')
                {
                    /* encryption
                     */
                    memset(password, 0, sizeof(password));
                    L_MD5_MDString(password, (UI8_T *)arg[1], strlen(arg[1]));
                }
                /* already encrypted
                 */
                else
                {
                    if (!str_to_nonprintable(arg[1], password))
                    {
                        CLI_LIB_PrintStr("Failed to set password\r\n");
                        return CLI_NO_ERROR;
                    }
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_PASSWORD:
            password[0] = 0;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (USERAUTH_PMGR_SetConsoleLoginPassword(password) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set password\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PasswordThresh_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T password_threshold;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_PASSWORDTHRESH:
            password_threshold = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_PASSWORDTHRESH:
            password_threshold = SYS_DFLT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetPasswordThreshold(password_threshold) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set password threshold\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_ExecTimeout_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T time_out_value;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_EXECTIMEOUT:
            time_out_value = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_EXECTIMEOUT:
            time_out_value = SYS_DFLT_SYSMGR_CONSOLE_EXEC_TIMEOUT;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetConsoleExecTimeOut(time_out_value) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set exec-timeout\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Timeout_Login_Response_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T time_out_value;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W3_TIMEOUT_LOGIN_RESPONSE:
            time_out_value = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W4_NO_TIMEOUT_LOGIN_RESPONSE:
            time_out_value = SYS_DFLT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetConsoleLoginTimeOut(time_out_value) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set login-timeout\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_SilentTime_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T silent_time;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_SILENTTIME:
            silent_time = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_SILENTTIME:
            silent_time = SYS_DFLT_SYSMGR_CONSOLE_SILENT_TIME;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetConsoleSilentTime(silent_time) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set silent time\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Speed_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T baudrate = SYS_DFLT_UART_ADMIN_BAUDRATE;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_SPEED:
            if (arg[0]!= NULL)
            {
                switch (*arg[0])
                {
                    case'9':
                        baudrate = SYS_MGR_UART_BAUDRATE_9600;
                        break;

                    case'1':
                        if (arg[0][1]=='9')
                            baudrate = SYS_MGR_UART_BAUDRATE_19200;
                        else if (arg[0][1]=='1')
                            baudrate = SYS_MGR_UART_BAUDRATE_115200;
                        break;

                    case'3':
                        baudrate = SYS_MGR_UART_BAUDRATE_38400;
                        break;

                    case'5':
                        baudrate = SYS_MGR_UART_BAUDRATE_57600;
                        break;

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
                    case'a':
                        baudrate = SYS_MGR_UART_BAUDRATE_AUTO;
                        break;
#endif

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_SPEED:
            baudrate = SYS_DFLT_UART_OPER_BAUDRATE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetUartBaudrate(baudrate) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set UART baudrate\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Databits_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T data_bits;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_DATABITS:
            if (arg[0]!=NULL)
            {
                switch (*arg[0])
                {
                    case'5':
                        data_bits = SYS_MGR_UART_DATA_LENGTH_5_BITS;
                        break;

                    case'6':
                        data_bits = SYS_MGR_UART_DATA_LENGTH_6_BITS;
                        break;

                    case'7':
                        data_bits = SYS_MGR_UART_DATA_LENGTH_7_BITS;
                        break;

                    case'8':
                        data_bits = SYS_MGR_UART_DATA_LENGTH_8_BITS;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_DATABITS:
            data_bits = SYS_MGR_UART_DATA_LENGTH_DEF;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetUartDataBits(data_bits) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set UART databits\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Parity_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T parity;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_PARITY:
            if (arg[0]!=NULL)
            {
                switch (*arg[0])
                {
                    case'n':
                    case'N':
                        parity = SYS_MGR_UART_PARITY_NONE;
                        break;

                    case'e':
                    case'E':
                        parity = SYS_MGR_UART_PARITY_EVEN;
                        break;

                    case'o':
                    case'O':
                        parity = SYS_MGR_UART_PARITY_ODD;
                        break;

                    case'm':
                    case'M':
                        parity = SYS_MGR_UART_PARITY_MARK;
                        break;

                    case's':
                    case'S':
                        parity = SYS_MGR_UART_PARITY_SPACE;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_PARITY:
            parity = SYS_MGR_UART_PARITY_DEF;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetUartParity(parity) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set UART parity\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Stopbits_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T stop_bits;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W1_STOPBITS:
            if (arg[0]!=NULL)
            {
                if (arg[0][0]=='1')
                {
                    //if (arg[0][1] == 0)
                        stop_bits = SYS_MGR_UART_STOP_BITS_1_BITS;
                    //else
                    //  stop_bits = SYS_MGR_UART_STOP_BITS_1_AND_HALF_BITS;
                }
                else /*2*/
                {
                    stop_bits=SYS_MGR_UART_STOP_BITS_2_BITS;
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_NO_STOPBITS:
            stop_bits = SYS_MGR_UART_STOP_BITS_DEF;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetUartStopBits(stop_bits) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set UART stopbits\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Login_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    USERAUTH_Login_Method_T login_method;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W1_LOGIN:
            if ((arg[0]!=NULL) && (arg[0][0] == 'l' || arg[0][0] == 'L'))
            {
                login_method = USERAUTH_LOGIN_LOGIN_LOCAL;
            }
            else
            {
                login_method = USERAUTH_LOGIN_LOGIN;
            }
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_NO_LOGIN:
            login_method = USERAUTH_LOGIN_NO_LOGIN;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (USERAUTH_PMGR_SetTelnetLoginMethod(login_method) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set login mode\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Password_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W1_PASSWORD:
            if ((arg[0]!=NULL) && (arg[1]!=NULL))
            {
                /* not encrypted yet
                 */
                if (arg[0][0] == '0')
                {
                    /* encryption
                     */
                    memset(password, 0, sizeof(password));
                    L_MD5_MDString(password, (UI8_T *)arg[1], strlen(arg[1]));
                }
                /* already encrypted
                 */
                else
                {
                    if (!str_to_nonprintable(arg[1], password))
                    {
                        CLI_LIB_PrintStr("Failed to set password\r\n");
                        return CLI_NO_ERROR;
                    }
                }
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_NO_PASSWORD:
            password[0] = 0;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (USERAUTH_PMGR_SetTelnetLoginPassword(password) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set password\r\n");
   }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PasswordThresh_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T password_threshold;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W1_PASSWORDTHRESH:
            password_threshold = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_NO_PASSWORDTHRESH:
            password_threshold = SYS_DFLT_SYSMGR_TELNET_PASSWORD_THRESHOLD;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetTelnetPasswordThreshold(password_threshold) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set password threshold\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_ExecTimeout_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T time_out_value;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W1_EXECTIMEOUT:
            time_out_value = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_NO_EXECTIMEOUT:
            time_out_value = SYS_DFLT_SYSMGR_TELNET_EXEC_TIMEOUT;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetTelnetExecTimeOut(time_out_value) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set exec-timeout\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Timeout_Login_Response_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T time_out_value;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W3_TIMEOUT_LOGIN_RESPONSE:
            time_out_value = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W4_NO_TIMEOUT_LOGIN_RESPONSE:
            time_out_value = SYS_DFLT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetTelnetLoginTimeOut(time_out_value) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set login-timeout\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_SilentTime_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T silent_time;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W1_SILENTTIME:
            silent_time = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_NO_SILENTTIME:
            silent_time = SYS_DFLT_SYSMGR_TELNET_SILENT_TIME;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetTelnetSilentTime(silent_time) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set silent time\r\n");
    }

    return CLI_NO_ERROR;
}

static UI32_T prt_console(UI32_T line_num)
{
    SYS_MGR_Console_T  consoleCfg;
    SYS_MGR_Uart_Cfg_T uartCfg;
    char   buff[CLI_DEF_MAX_BUFSIZE]={0};

    memset(&consoleCfg, 0, sizeof(SYS_MGR_Console_T));
    memset(&uartCfg,    0, sizeof(SYS_MGR_Uart_Cfg_T));

    SYS_PMGR_GetConsoleCfg(&consoleCfg);
    SYS_PMGR_GetUartParameters(&uartCfg);

    PROCESS_MORE_FUNC(" Console Configuration:\r\n");

    if (consoleCfg.password_threshold == SYSMGMT_TYPE_PASSWORD_THRESHOLD_DISABLED)
    {
        PROCESS_MORE_FUNC("  Password Threshold : Disabled\r\n");
    }
    else
    {
        sprintf(buff, "  Password Threshold : %lu times\r\n", (unsigned long)consoleCfg.password_threshold);
        PROCESS_MORE_FUNC(buff);
    }

    if (consoleCfg.exec_timeout == SYSMGMT_TYPE_EXEC_TIMEOUT_DISABLED)
    {
        PROCESS_MORE_FUNC("  EXEC Timeout       : Disabled\r\n");
    }
    else
    {
        sprintf(buff, "  EXEC Timeout       : %lu seconds\r\n", (unsigned long)consoleCfg.exec_timeout);
        PROCESS_MORE_FUNC(buff);
    }

    sprintf(buff, "  Login Timeout      : %lu seconds\r\n", (unsigned long)consoleCfg.login_timeout);
    PROCESS_MORE_FUNC(buff);

    if (consoleCfg.silent_time == SYSMGMT_TYPE_SILENT_TIME_DISABLED)
    {
        PROCESS_MORE_FUNC("  Silent Time        : Disabled\r\n");
    }
    else
    {
        sprintf(buff, "  Silent Time        : %lu seconds\r\n", (unsigned long)consoleCfg.silent_time);
        PROCESS_MORE_FUNC(buff);
    }

    if (uartCfg.baudrate == SYS_DFLT_UART_ADMIN_BAUDRATE)
    {
        PROCESS_MORE_FUNC("  Baud Rate          : Auto\r\n");
    }
    else
    {
        sprintf(buff, "  Baud Rate          : %lu\r\n",(unsigned long)uartCfg.baudrate);
        PROCESS_MORE_FUNC(buff);
    }

    sprintf(buff, "  Data Bits          : %u\r\n", uartCfg.data_length);
    PROCESS_MORE_FUNC(buff);

    switch (uartCfg.parity)
    {
        case SYS_MGR_UART_PARITY_NONE:
            PROCESS_MORE_FUNC("  Parity             : None\r\n");
            break;

        case SYS_MGR_UART_PARITY_EVEN:
            PROCESS_MORE_FUNC("  Parity             : Even\r\n");
            break;

        case SYS_MGR_UART_PARITY_ODD:
            PROCESS_MORE_FUNC("  Parity             : Odd\r\n");
            break;

        case SYS_MGR_UART_PARITY_MARK:
            PROCESS_MORE_FUNC("  Parity             : Mark\r\n");
            break;

        case SYS_MGR_UART_PARITY_SPACE:
            PROCESS_MORE_FUNC("  Parity             : Space\r\n");
            break;

        default:
            PROCESS_MORE_FUNC("  Parity             : Unknown\r\n");
            break;
    }

    switch (uartCfg.stop_bits)
    {
        case SYS_MGR_UART_STOP_BITS_1_BITS:
            PROCESS_MORE_FUNC("  Stop Bits          : 1\r\n");
            break;

        //   case SYS_MGR_UART_STOP_BITS_1_AND_HALF_BITS:
        //      PROCESS_MORE_FUNC("  Stop Bits          : 1.5\r\n");
            break;

        case SYS_MGR_UART_STOP_BITS_2_BITS:
            PROCESS_MORE_FUNC("  Stop Bits          : 2\r\n");
            break;

        default:
            PROCESS_MORE_FUNC("  Stop Bits          : Unknown\r\n");
            break;
    }
    return line_num;
}

static UI32_T prt_vty(UI32_T line_num)
{
    SYS_MGR_Telnet_T telnetCfg;
    char   buff[CLI_DEF_MAX_BUFSIZE]={0};

    memset(&telnetCfg, 0, sizeof(SYS_MGR_Telnet_T));
    SYS_PMGR_GetTelnetCfg(&telnetCfg);

    PROCESS_MORE_FUNC(" VTY Configuration:\r\n");

    if (telnetCfg.password_threshold == SYSMGMT_TYPE_PASSWORD_THRESHOLD_DISABLED)
    {
        PROCESS_MORE_FUNC("  Password Threshold : Disabled\r\n");
    }
    else
    {
        sprintf(buff, "  Password Threshold : %lu times\r\n", (unsigned long)telnetCfg.password_threshold);
        PROCESS_MORE_FUNC(buff);
    }

    sprintf(buff, "  EXEC Timeout       : %lu seconds\r\n", (unsigned long)telnetCfg.exec_timeout);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "  Login Timeout      : %lu sec.\r\n", (unsigned long)telnetCfg.login_timeout);
    PROCESS_MORE_FUNC(buff);

    if (telnetCfg.silent_time == SYSMGMT_TYPE_SILENT_TIME_DISABLED)
    {
        PROCESS_MORE_FUNC("  Silent Time        : Disabled\r\n");
    }
    else
    {
        sprintf(buff, "  Silent Time        : %lu seconds\r\n", (unsigned long)telnetCfg.silent_time);
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}

UI32_T CLI_API_Line_Length(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_TERMINAL_LENGTH:
            ctrl_P->CMenu.length = atoi( arg[0] );

            if (ctrl_P->CMenu.length==0)
            {
                CLI_API_Set_Print_Interactive_Mode(FALSE);
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_TERMINAL_LENGTH:
            ctrl_P->CMenu.length = CLI_DEF_DISP_LINENUM;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Line_Width(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_TERMINAL_WIDTH:
            ctrl_P->CMenu.width = atoi( arg[0] );
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_TERMINAL_WIDTH:
            ctrl_P->CMenu.width = CLI_DEF_DISP_WIDTH;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Line_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_TERMINAL_HISTORY:
            if (arg[0])
            {
                ctrl_P->CMenu.histsize = atoi( arg[1] );
            }
            else
            {
                ctrl_P->CMenu.histsize = CLI_DEF_HIST;
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_TERMINAL_HISTORY:
            if (arg[0])
            {
                ctrl_P->CMenu.histsize = CLI_DEF_HIST;
            }
            else
            {
                ctrl_P->CMenu.histsize = 0;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Line_Terminaltype(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_TERMINAL_TERMINALTYPE:
            if ((arg[0][0]=='a')||(arg[0][0]=='A'))
            {
                ctrl_P->CMenu.terminal_type = ANSI_BBS;
            }
            else if ((arg[0][5]=='0'))
            {
                ctrl_P->CMenu.terminal_type = VT100;
            }
            else if ((arg[0][5]=='2'))
            {
                ctrl_P->CMenu.terminal_type = VT102;
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_TERMINAL_TERMINALTYPE:
            ctrl_P->CMenu.terminal_type = VT100;

            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Line_Escapecharacter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_TERMINAL_ESCAPECHARACTER:
            if (arg[0][1]=='s' || arg[0][1]=='S')
            {
                ctrl_P->CMenu.escape_character = (UI8_T)(atoi(arg[1]));
            }
            else
            {
                ctrl_P->CMenu.escape_character=(UI8_T)arg[0][0];
            }

            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_TERMINAL_ESCAPECHARACTER:
            ctrl_P->CMenu.escape_character=CLI_DEF_ESCAPE;

            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_CLI_TERMINAL == TRUE)*/
    return CLI_NO_ERROR;
}


