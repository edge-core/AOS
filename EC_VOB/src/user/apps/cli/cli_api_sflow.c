#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_api_sflow.h"
#include "sflow_pmgr.h"
#include "l_inet.h"

// #if (SYS_CPNT_STACKING == TRUE)
// #include "stktplg_pmgr.h"
// #endif

#define UI32_MAX_LEN            10    /* 4294967295 */
#define DATA_SOURCE_STR_LEN    (sizeof("Eth ")+UI32_MAX_LEN+sizeof("/")+UI32_MAX_LEN)-1

#if (SYS_CPNT_SFLOW == TRUE)
static UI32_T
input_datasource_to_ifindex(
    char *type_str_p,
    char *datasource_str_p,
    UI32_T *ifindex_p);

static UI32_T
ifindex_to_datasource_string(
    UI32_T ifindex,
    char *data_source_str_p);

static UI32_T
show_receiver_owner_info(
    SFLOW_MGR_Receiver_T  *receiver_entry_p,
    UI32_T line_num);

static UI32_T
show_sampling_info(
    SFLOW_MGR_Sampling_T *sampling_entry_p,
    UI32_T line_num);

static UI32_T
show_polling_info(
    SFLOW_MGR_Polling_T *polling_entry_p,
    UI32_T line_num);

 #endif /* #if (SYS_CPNT_SFLOW == TRUE) */

UI32_T CLI_API_SFlow_Owner(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFLOW == TRUE)

    if (NULL == arg[0])
    {
        return CLI_ERR_INTERNAL;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_SFLOW_OWNER:
        {
            SFLOW_MGR_Receiver_T receiver_entry;

            memset(&receiver_entry, 0, sizeof(receiver_entry));
            strncpy(receiver_entry.owner_name, arg[0], sizeof(receiver_entry.owner_name)-1);
            receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

            if (NULL == arg[1] ||
                NULL == arg[2])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[1][0])
            {
                /* timeout
                 */
                case 't':
                case 'T':
                    receiver_entry.timeout = atoi(arg[2]);
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (NULL != arg[3] &&
                NULL != arg[4])
            {
                switch (arg[3][0])
                {
                    /* destination
                     */
                    case 'd':
                    case 'D':

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                           arg[4],
                                                                           (L_INET_Addr_T *)&receiver_entry.address,
                                                                           sizeof(receiver_entry.address)))
                        {
                            CLI_LIB_PrintStr("Invalid IP address.\r\n");
                            return CLI_ERR_INTERNAL;
                        }

                        if (NULL != arg[5] &&
                            NULL != arg[6])
                        {
                            switch (arg[5][0])
                            {
                                /* max-datagram-size
                                 */
                                case 'm':
                                case 'M':
                                    receiver_entry.max_datagram_size = atoi(arg[6]);

                                    if (NULL != arg[7] &&
                                        NULL != arg[8])
                                    {
                                        switch (arg[7][0])
                                        {
                                            /* version
                                             */
                                            case 'v':
                                            case 'V':
                                                switch (arg[8][1])
                                                {
                                                    case '4':
                                                        receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_4;
                                                        break;

                                                    case '5':
                                                        receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_5;
                                                        break;

                                                    default:
                                                        return CLI_ERR_INTERNAL;
                                                }
                                                break;

                                            default:
                                                return CLI_ERR_INTERNAL;
                                        }
                                    }
                                    break;

                                /* port
                                 */
                                case 'p':
                                case 'P':
                                    receiver_entry.udp_port = atoi(arg[6]);

                                    if (NULL != arg[7] &&
                                        NULL != arg[8])
                                    {
                                        switch (arg[7][0])
                                        {
                                            /* max-datagram-size
                                             */
                                            case 'm':
                                            case 'M':
                                                receiver_entry.max_datagram_size = atoi(arg[8]);

                                                if (NULL != arg[9] &&
                                                    NULL != arg[10])
                                                {
                                                    switch (arg[9][0])
                                                    {
                                                        /* version
                                                         */
                                                        case 'v':
                                                        case 'V':
                                                            switch (arg[10][1])
                                                            {
                                                                case '4':
                                                                    receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_4;
                                                                    break;

                                                                case '5':
                                                                    receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_5;
                                                                    break;

                                                                default:
                                                                    return CLI_ERR_INTERNAL;
                                                            }
                                                            break;

                                                        default:
                                                            return CLI_ERR_INTERNAL;
                                                    }
                                                }
                                                break;

                                            /* version
                                             */
                                            case 'v':
                                            case 'V':
                                                switch (arg[8][1])
                                                {
                                                    case '4':
                                                        receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_4;
                                                        break;

                                                    case '5':
                                                        receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_5;
                                                        break;

                                                    default:
                                                        return CLI_ERR_INTERNAL;
                                                }
                                                break;

                                            default:
                                                return CLI_ERR_INTERNAL;
                                        }
                                    }
                                    break;

                                /* version
                                 */
                                case 'v':
                                case 'V':
                                    switch (arg[6][1])
                                    {
                                        case '4':
                                            receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_4;
                                            break;

                                        case '5':
                                            receiver_entry.datagram_version = SFLOW_MGR_DATAGRAM_VERSION_5;
                                            break;

                                        default:
                                            return CLI_ERR_INTERNAL;
                                    }
                                    break;

                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                        }
                        break;

                    /* port
                     */
                    case 'p':
                    case 'P':
                        receiver_entry.udp_port = atoi(arg[4]);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreateReceiverEntry(&receiver_entry))
            {
                CLI_LIB_PrintStr("Failed to create receiver owner.\r\n");
                return CLI_NO_ERROR;
            }
        }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_SFLOW_OWNER:

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroyReceiverEntryByOwnerName(arg[0]))
            {
                CLI_LIB_PrintStr("Failed to destroy receiver owner.\r\n");
                return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
 #endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_SFlow_Sampling_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFLOW == TRUE)
    UI32_T ifindex = 0;
    UI32_T instance_id = 0;

    if (CLI_NO_ERROR !=
        input_datasource_to_ifindex(arg[0], arg[1], &ifindex))
    {
        return CLI_NO_ERROR;
    }

    if (NULL == arg[2] ||
        NULL == arg[3])
    {
        return CLI_ERR_INTERNAL;
    }

    switch (arg[2][0])
    {
        /* instance
         */
        case 'i':
        case 'I':
            instance_id = atoi(arg[3]);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W3_SFLOW_SAMPLING_INTERFACE:
        {
            SFLOW_MGR_Sampling_T  sampling_entry;

            memset(&sampling_entry, 0, sizeof(sampling_entry));
            sampling_entry.ifindex = ifindex;
            sampling_entry.instance_id = instance_id;

            if (NULL == arg[4] ||
                NULL == arg[5])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[4][0])
            {
                /* receiver
                 */
                case 'r':
                case 'R':
                    strncpy(sampling_entry.receiver_owner_name, arg[5],
                        sizeof(sampling_entry.receiver_owner_name)-1);
                    sampling_entry.receiver_owner_name[sizeof(sampling_entry.receiver_owner_name)-1] = '\0';
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (NULL == arg[6] ||
                NULL == arg[7])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[6][0])
            {
                /* sampling-rate
                 */
                case 's':
                case 'S':
                    sampling_entry.sampling_rate = atoi(arg[7]);
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (NULL != arg[8] &&
                NULL != arg[9])
            {
                switch (arg[8][0])
                {
                    /* max-header-size
                     */
                    case 'm':
                    case 'M':
                        sampling_entry.max_header_size = atoi(arg[9]);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreateSamplingEntry(&sampling_entry))
            {
                CLI_LIB_PrintStr("Failed to create sampling instance.\r\n");
                return CLI_NO_ERROR;
            }
        }
            break;

        case PRIVILEGE_EXEC_CMD_W4_NO_SFLOW_SAMPLING_INTERFACE:

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroySamplingEntry(ifindex, instance_id))
            {
                CLI_LIB_PrintStr("Failed to destroy sampling instance.\r\n");
                return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
 #endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_SFlow_Polling_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFLOW == TRUE)
    UI32_T ifindex = 0;
    UI32_T instance_id = 0;

    if (CLI_NO_ERROR !=
        input_datasource_to_ifindex(arg[0], arg[1], &ifindex))
    {
        return CLI_NO_ERROR;
    }

    if (NULL == arg[2] ||
        NULL == arg[3])
    {
        return CLI_ERR_INTERNAL;
    }

    switch (arg[2][0])
    {
        /* instance
         */
        case 'i':
        case 'I':
            instance_id = atoi(arg[3]);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W3_SFLOW_POLLING_INTERFACE:
        {
            SFLOW_MGR_Polling_T  polling_entry;

            memset(&polling_entry, 0, sizeof(polling_entry));
            polling_entry.ifindex = ifindex;
            polling_entry.instance_id = instance_id;

            if (NULL == arg[4] ||
                NULL == arg[5])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[4][0])
            {
                /* receiver
                 */
                case 'r':
                case 'R':
                    strncpy(polling_entry.receiver_owner_name, arg[5],
                        sizeof(polling_entry.receiver_owner_name)-1);
                    polling_entry.receiver_owner_name[sizeof(polling_entry.receiver_owner_name)-1] = '\0';
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }

            if (NULL == arg[6] ||
                NULL == arg[7])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[6][0])
            {
                /* polling-interval
                 */
                case 'p':
                case 'P':
                    polling_entry.polling_interval = atoi(arg[7]);
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreatePollingEntry(&polling_entry))
            {
                CLI_LIB_PrintStr("Failed to create polling instance.\r\n");
                return CLI_NO_ERROR;
            }
        }
            break;

        case PRIVILEGE_EXEC_CMD_W4_NO_SFLOW_POLLING_INTERFACE:

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroyPollingEntry(ifindex, instance_id))
            {
                CLI_LIB_PrintStr("Failed to destroy polling instance.\r\n");
                return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
 #endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_SFlow(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFLOW == TRUE)

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_SFlow(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFLOW == TRUE)
    SFLOW_MGR_Receiver_T receiver_entry;
    SFLOW_MGR_Sampling_T sampling_entry;
    SFLOW_MGR_Polling_T  polling_entry;
    UI32_T line_num = 0;
    UI32_T ifindex;
    UI32_T instance_id;
    UI32_T filter_ifindex = 0;
    char  filter_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};
    BOOL_T show_receiver;
    BOOL_T show_sampling;
    BOOL_T show_polling;

    if (NULL != arg[0])
    {
        switch (arg[0][0])
        {
            /* interface
             */
            case 'i':
            case 'I':
                if (NULL == arg[1] ||
                    NULL == arg[2])
                {
                    return CLI_ERR_INTERNAL;
                }

                if (CLI_NO_ERROR !=
                    input_datasource_to_ifindex(arg[1], arg[2], &filter_ifindex))
                {
                    return CLI_NO_ERROR;
                }
                break;

            /* owner
             */
            case 'o':
            case 'O':
                strncpy(filter_owner_name, arg[1], sizeof(filter_owner_name)-1);
                filter_owner_name[sizeof(filter_owner_name)-1] = '\0';

                if (NULL != arg[2])
                {
                    switch (arg[2][0])
                    {
                        /* interface
                         */
                        case 'i':
                        case 'I':
                            if (NULL == arg[3] ||
                                NULL == arg[4])
                            {
                                return CLI_ERR_INTERNAL;
                            }

                            if (CLI_NO_ERROR !=
                                input_datasource_to_ifindex(arg[3], arg[4], &filter_ifindex))
                            {
                                return CLI_NO_ERROR;
                            }
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    memset(&receiver_entry, 0, sizeof(receiver_entry));

    while (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_PMGR_GetNextActiveReceiverEntry(&receiver_entry))
    {
        if (('\0' != filter_owner_name[0]) &&
            (0 != strcmp(filter_owner_name, receiver_entry.owner_name)))
        {
            continue;
        }

        show_receiver = TRUE;

        if (0 == filter_ifindex)
        {
            line_num = show_receiver_owner_info(&receiver_entry, line_num);

            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }

            show_receiver = FALSE;
        }

        for (ifindex = SFLOW_MGR_MIN_IFINDEX; ifindex <= SFLOW_MGR_MAX_IFINDEX; ++ifindex)
        {
            if ((0 != filter_ifindex) &&
               (ifindex != filter_ifindex))
            {
                continue;
            }

            for (instance_id = SFLOW_MGR_MIN_INSTANCE_ID; instance_id <= SFLOW_MGR_MAX_INSTANCE_ID; ++instance_id)
            {
                show_sampling = FALSE;
                show_polling = FALSE;

                memset(&sampling_entry, 0, sizeof(sampling_entry));
                sampling_entry.ifindex = ifindex;
                sampling_entry.instance_id = instance_id;

                if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetActiveSamplingEntry(&sampling_entry) &&
                    (0 == strcmp(sampling_entry.receiver_owner_name, receiver_entry.owner_name)))
                {
                    show_sampling = TRUE;
                }

                memset(&polling_entry, 0, sizeof(polling_entry));
                polling_entry.ifindex = ifindex;
                polling_entry.instance_id = instance_id;

                if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetActivePollingEntry(&polling_entry) &&
                    (0 == strcmp(polling_entry.receiver_owner_name, receiver_entry.owner_name)))
                {
                    show_polling = TRUE;
                }

                if ((TRUE == show_receiver) && (TRUE == show_sampling || TRUE == show_polling))
                {
                    line_num = show_receiver_owner_info(&receiver_entry, line_num);

                    if (line_num == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }

                    show_receiver = FALSE;
                }

                if (TRUE == show_sampling)
                {
                    line_num = show_sampling_info(&sampling_entry, line_num);

                    if (line_num == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }

                if (TRUE == show_polling)
                {
                    line_num = show_polling_info(&polling_entry, line_num);

                    if (line_num == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }
            } /* end of instance id for loop */
        } /* end of ifindex for loop */
    }

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_SFLOW == TRUE)
static UI32_T
input_datasource_to_ifindex(
    char *type_str_p,
    char *datasource_str_p,
    UI32_T *ifindex_p)
{
    if (NULL == type_str_p ||
        NULL == datasource_str_p)
    {
        return CLI_ERR_INTERNAL;
    }

    switch (type_str_p[0])
    {
        /* ethernet
         */
        case 'e':
        case 'E':
        {
            UI32_T verify_unit = 0;
            UI32_T verify_port = 0;
            CLI_API_EthStatus_T verify_ret;

            if (isdigit(datasource_str_p[0]))
            {
                verify_unit = atoi(datasource_str_p);
                verify_port = atoi(strchr(datasource_str_p,'/')+1);
            }
#if (CLI_SUPPORT_PORT_NAME == 1)
            else /*port name*/
            {
                UI32_T trunk_id = 0;

                if (TRUE != IF_PMGR_IfnameToIfindex(datasource_str_p, &ifindex))
                {
                    CLI_LIB_PrintStr_1("%s does not exist\r\n",datasource_str_p);
                    return CLI_NO_ERROR;
                }

                SWCTRL_POM_LogicalPortToUserPort(ifindex, &verify_unit, &verify_port, &trunk_id);
            }
#endif

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, ifindex_p)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

static UI32_T
ifindex_to_datasource_string(
    UI32_T ifindex,
    char *data_source_str_p)
{
    UI32_T unit = 0, port = 0, trunk_id = 0;

    if (SWCTRL_LPORT_NORMAL_PORT !=
        SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
    {
        return CLI_ERR_INTERNAL;
    }

    snprintf(data_source_str_p, DATA_SOURCE_STR_LEN+1, "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
    data_source_str_p[DATA_SOURCE_STR_LEN] = '\0';

    return CLI_NO_ERROR;
}

static UI32_T
show_receiver_owner_info(
    SFLOW_MGR_Receiver_T  *receiver_entry_p,
    UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char ip_address_str[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};

    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&receiver_entry_p->address,
                                                       ip_address_str,
                                                       sizeof(ip_address_str)))
    {
        return CLI_ERR_INTERNAL;
    }

    snprintf(buff, sizeof(buff), "\r\n  Receiver Owner Name   : %s\r\n", receiver_entry_p->owner_name);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Receiver Timeout      : %lu sec\r\n", (unsigned long)receiver_entry_p->timeout);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Receiver Destination  : %s\r\n", ip_address_str);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Receiver Socket Port  : %lu\r\n", (unsigned long)receiver_entry_p->udp_port);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Maximum Datagram Size : %lu bytes\r\n", (unsigned long)receiver_entry_p->max_datagram_size);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Datagram Version      : %lu\r\n", (unsigned long)receiver_entry_p->datagram_version);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T
show_sampling_info(
    SFLOW_MGR_Sampling_T *sampling_entry_p,
    UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char data_source_str[DATA_SOURCE_STR_LEN + 1] = {0};

    ifindex_to_datasource_string(sampling_entry_p->ifindex, data_source_str);

    snprintf(buff, sizeof(buff), "\r\n  Data Source           : %s\r\n", data_source_str);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Sampling Instance ID  : %lu\r\n", (unsigned long)sampling_entry_p->instance_id);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Sampling Rate         : %lu\r\n", (unsigned long)sampling_entry_p->sampling_rate);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Maximum Header Size   : %lu bytes\r\n", (unsigned long)sampling_entry_p->max_header_size);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T
show_polling_info(
    SFLOW_MGR_Polling_T *polling_entry_p,
    UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char data_source_str[DATA_SOURCE_STR_LEN + 1] = {0};

    ifindex_to_datasource_string(polling_entry_p->ifindex, data_source_str);

    snprintf(buff, sizeof(buff), "\r\n  Data Source           : %s\r\n", data_source_str);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Polling Instance ID   : %lu\r\n", (unsigned long)polling_entry_p->instance_id);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), "  Polling Interval      : %lu sec\r\n", (unsigned long)polling_entry_p->polling_interval);
    buff[sizeof(buff)-1] = '\0';
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

 #endif /* #if (SYS_CPNT_SFLOW == TRUE) */

