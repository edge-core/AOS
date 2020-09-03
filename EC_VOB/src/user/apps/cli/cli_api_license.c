#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
#include <stdio.h>
#include "l_inet.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "cli_type.h"
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_msg.h"
#include "cli_io.h"
#include "cli_task.h"
#include "cli_lib.h"
#include "cli_pars.h"
#include "cli_cmd.h"
#include "cli_def.h"
#include "cli_api.h"
#include "sys_adpt.h"
#include "cli_api_license.h"

#if defined(FTTH_OKI)
#include "sys_cpnt.h"
#endif

#include "fs_type.h"
#include "ini.h"

typedef struct
{
    BOOL_T is_quit;
    UI32_T ret_value;
    UI32_T line_num;
} print_license_res_arg;

static UI32_T print_license_res(UI32_T line_num, const char *str_p);
static int license_res_handler(void *arg_p, const char *section_p, const char *name_p, const char *value_p);

static FS_RETURN_CODE_T set_license_activation_code(const char *activation_code)
{
    UI32_T  unit_id;

    if (activation_code == NULL)
    {
        return FS_RETURN_ERROR;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);

    return FS_WriteFile(unit_id,
                        (unsigned char *)SYS_ADPT_LICENSE_AC_FILE_NAME,
                        (UI8_T *)"cli",
                        FS_FILE_TYPE_LICENSE,
                        (unsigned char *)activation_code,
                        SYS_ADPT_LICENSE_AC_LEN,
                        0);
}

UI32_T CLI_API_License_ActivationCode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    if (set_license_activation_code(arg[0]) != FS_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to set license activation-code.\r\n");
    }

    return CLI_NO_ERROR;
}

static UI32_T print_license_res(UI32_T line_num, const char *str_p)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    snprintf(buff, sizeof(buff), "%s\r\n", str_p);
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static int license_res_handler(void *arg_p, const char *section_p, const char *name_p, const char *value_p)
{
    print_license_res_arg *print_arg_p = (print_license_res_arg *)arg_p;

    if (   (FALSE == print_arg_p->is_quit)
        && (0 == strcmp(section_p, ""))
        && (0 == strcmp(name_p, "valid-state"))
        )
    {
        print_arg_p->line_num = print_license_res(print_arg_p->line_num, value_p);
        if (JUMP_OUT_MORE == print_arg_p->line_num)
        {
            print_arg_p->is_quit = TRUE;
            print_arg_p->ret_value = CLI_NO_ERROR;
        }
        else if (EXIT_SESSION_MORE == print_arg_p->line_num)
        {
            print_arg_p->is_quit = TRUE;
            print_arg_p->ret_value = CLI_EXIT_SESSION;
        }
    }

    return 1;
}

UI32_T CLI_API_Show_License(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  line_num = 0;
    print_license_res_arg parse_arg;

    parse_arg.is_quit = FALSE;
    parse_arg.ret_value = 0;
    parse_arg.line_num = line_num;

    if (0 > ini_parse(SYS_ADPT_LICENSE_FILE_PATH SYS_ADPT_LICENSE_RESULT_FILE_NAME, license_res_handler, &parse_arg))
    {
        CLI_LIB_PrintStr("No active license available.\r\n");
        return CLI_NO_ERROR;
    }

    line_num = parse_arg.line_num;

    if (TRUE == parse_arg.is_quit)
    {
        return parse_arg.ret_value;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_LicenseFile(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE]  = {0};

    UI8_T   name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    FILE *fp;
    char line[512];

    sprintf((char *)name,"%s%s", SYS_ADPT_LICENSE_FILE_PATH, SYS_ADPT_LICENSE_FILE_NAME);
    fp=fopen((char *)name, "r");

    if (fp)
    {
        while (fgets(line, sizeof(line), fp)){
            PROCESS_MORE(line);
        }
        fclose(fp);
    }
    else
    {
        CLI_LIB_PrintStr("No license file.\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_LicenseActivationCodeFile(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI8_T   name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    FILE    *fp;
    char    line[512];

    sprintf((char *)name,"%s%s", SYS_ADPT_LICENSE_FILE_PATH, SYS_ADPT_LICENSE_AC_FILE_NAME);
    fp = fopen((char *)name, "r");

    if (fp)
    {
        PROCESS_MORE("License Activation-Code: ");
        while (fgets(line, sizeof(line), fp)){
            PROCESS_MORE(line);
        }
        fclose(fp);
        PROCESS_MORE("\r\n");
    }
    else
    {
        CLI_LIB_PrintStr("No license activation-code.\r\n");
    }

    return CLI_NO_ERROR;
}
