#include <ctype.h>
#include "cli_def.h"
#include "cli_api.h"
#include "cli_lib.h"
#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#include "dos_om.h"
#endif

UI32_T CLI_API_DosProtection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOS == TRUE)
    UI32_T arg_chk_idx;
    BOOL_T no_form;
    BOOL_T use_dflt;

    /* dos-protection echo-chargen [bit-rate-in-kilo <rate>]
     * dos-protection land
     * dos-protection smurf
     * dos-protection tcp-flooding [bit-rate-in-kilo <rate>]
     * dos-protection tcp-null-scan
     * dos-protection tcp-scan
     * dos-protection tcp-syn-fin-scan
     * dos-protection tcp-udp-port-zero
     * dos-protection tcp-xmas-scan
     * dos-protection udp-flooding [bit-rate-in-kilo <rate>]
     * dos-protection win-nuke [bit-rate-in-kilo <rate>]
     */
    static struct {
        DOS_TYPE_FieldId_T field_id;
        struct {
            int offset;
            char ch;
        } cmp_ch[3];
    }
    field_info[] =
    {
        /* NOTE:
         *   to avoid parsing error when user keyin an incomplete keyword,
         *   shall only parse supported features.
         */
        #if (SYS_CPNT_DOS_ECHO_CHARGEN == TRUE)
        { DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS,      {{0,'e'}} },
        #endif
        #if (SYS_CPNT_DOS_LAND == TRUE)
        { DOS_TYPE_FLD_SYSTEM_LAND_STATUS,              {{0,'l'}} },
        #endif
        #if (SYS_CPNT_DOS_SMURF == TRUE)
        { DOS_TYPE_FLD_SYSTEM_SMURF_STATUS,             {{0,'s'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_FLOODING == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS,      {{0,'t'},{4,'f'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_NULL_SCAN == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS,     {{0,'t'},{4,'n'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_SCAN == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS,          {{0,'t'},{4,'s'},{5,'c'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_SYN_FIN_SCAN == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS,  {{0,'t'},{4,'s'},{5,'y'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_UDP_PORT_ZERO == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS, {{0,'t'},{4,'u'}} },
        #endif
        #if (SYS_CPNT_DOS_TCP_XMAS_SCAN == TRUE)
        { DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS,     {{0,'t'},{4,'x'}} },
        #endif
        #if (SYS_CPNT_DOS_UDP_FLOODING == TRUE)
        { DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS,      {{0,'u'}} },
        #endif
        #if (SYS_CPNT_DOS_WIN_NUKE == TRUE)
        { DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS,          {{0,'w'}} },
        #endif
    };

    DOS_TYPE_Error_T ret;
    DOS_TYPE_FieldId_T field_id;
    UI32_T val;
    int i, j, max_matched_nch;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_DOSPROTECTION:
            no_form = FALSE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_DOSPROTECTION:
            no_form = TRUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    arg_chk_idx = 0;

    max_matched_nch = 0;
    field_id = DOS_TYPE_FLD_NUM;

    for (i = 0; i < sizeof(field_info)/sizeof(*field_info); i++)
    {
        for (j = 0; j < sizeof(field_info->cmp_ch) / sizeof(*field_info->cmp_ch); j++)
        {
            int ch_offset = field_info[i].cmp_ch[j].offset;
            char cmp_ch = field_info[i].cmp_ch[j].ch;
            char arg_ch = tolower(arg[arg_chk_idx][ch_offset]);

            if (arg_ch != cmp_ch)
            {
                break;
            }
        }
        if (max_matched_nch < j)
        {
            max_matched_nch = j;
            field_id = field_info[i].field_id;
        }
    }

    if (field_id >= DOS_TYPE_FLD_NUM)
    {
        return CLI_ERR_INTERNAL;
    }

    arg_chk_idx++;

    val = 0;

    switch (field_id)
    {
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS:
            if (arg[arg_chk_idx])
            {
                arg_chk_idx++;
                field_id++;
                if (no_form)
                {
                    use_dflt = TRUE;
                }
                else
                {
                    use_dflt = FALSE;
                    val = atoi(arg[arg_chk_idx++]);
                }
            }
            else
            {
                use_dflt = FALSE;
                val = no_form ? DOS_TYPE_STATUS_DISABLED : DOS_TYPE_STATUS_ENABLED;
            }
            break;

        case DOS_TYPE_FLD_SYSTEM_LAND_STATUS:
        case DOS_TYPE_FLD_SYSTEM_SMURF_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS:
            use_dflt = FALSE;
            val = no_form ? DOS_TYPE_STATUS_DISABLED : DOS_TYPE_STATUS_ENABLED;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (use_dflt)
    {
        ret = DOS_PMGR_SetDataByField(field_id, NULL);
    }
    else
    {
        ret = DOS_PMGR_SetDataByField(field_id, &val);
    }

    switch (ret)
    {
        case DOS_TYPE_E_OK:
            break;

        case DOS_TYPE_E_NOT_SUPPORT:
            CLI_LIB_PrintStr("Not supported.\r\n");
            break;

        default:
            CLI_LIB_PrintStr("Failed to set DOS Protection.\r\n");
    }
#endif /* (SYS_CPNT_DOS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_DosProtection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOS == TRUE)
    UI32_T              line_num = 0;
    char                buff[CLI_DEF_MAX_BUFSIZE] = {0};

    enum {
        FILTER,
        RATE_LIMIT,
    };

    static struct {
        UI32_T method;
        char *name;
    }
    field_info[] =
    {
        [DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS]       = { RATE_LIMIT, "Echo/Chargen Attack" },
        [DOS_TYPE_FLD_SYSTEM_LAND_STATUS]               = { FILTER,     "LAND Attack" },
        [DOS_TYPE_FLD_SYSTEM_SMURF_STATUS]              = { FILTER,     "Smurf Attack" },
        [DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS]       = { RATE_LIMIT, "TCP Flooding Attack" },
        [DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS]      = { FILTER,     "TCP Null Scan" },
        [DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS]           = { FILTER,     "TCP Scan" },
        [DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS]   = { FILTER,     "TCP SYN/FIN Scan" },
        [DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS]  = { FILTER,     "TCP/UDP Packets with Port 0" },
        [DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS]      = { FILTER,     "TCP XMAS Scan" },
        [DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS]       = { RATE_LIMIT, "UDP Flooding Attack" },
        [DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS]           = { RATE_LIMIT, "WinNuke Attack" },
    };

    DOS_TYPE_FieldId_T field_id;
    DOS_TYPE_FieldDataBuf_T data;

    PROCESS_MORE("Global DoS Protection:\r\n");
    PROCESS_MORE("\r\n");

    for (field_id = 0; field_id < DOS_TYPE_FLD_NUM; field_id++)
    {
        if (DOS_PMGR_GetDataByField(field_id, &data) == DOS_TYPE_E_OK)
        {
            switch (field_info[field_id].method)
            {
                case RATE_LIMIT:
                    {
                        UI32_T rate;

                        if (DOS_PMGR_GetDataByField(field_id+1, &rate) == DOS_TYPE_E_OK)
                        {
                            sprintf(buff, " %-27s : %s, %lu kilobits per second\r\n",
                                field_info[field_id].name,
                                data.u32 == DOS_TYPE_STATUS_ENABLED ? "Enabled" : "Disabled",
                                rate);
                            PROCESS_MORE(buff);
                        }

                        field_id++;
                    }
                    break;

                case FILTER:
                    sprintf(buff, " %-27s : %s\r\n",
                        field_info[field_id].name,
                        data.u32 == DOS_TYPE_STATUS_ENABLED ? "Enabled" : "Disabled");
                    PROCESS_MORE(buff);
                    break;

                default:
                    continue;
            }
        }
    } /* end of for (field_id) */
#endif /* (SYS_CPNT_DOS == TRUE) */

    return CLI_NO_ERROR;
}

