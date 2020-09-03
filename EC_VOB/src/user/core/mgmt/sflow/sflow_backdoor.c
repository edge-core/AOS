#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "swctrl_pom.h"
#include "sflow_proc_comm.h"
#include "sflow_mgr.h"

#if (SYS_CPNT_SFLOW == TRUE)

#define MAX_INPUT_STR_LEN SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN

#define INPUT_RECEIVER_INDEX()                                                       \
    BACKDOOR_MGR_Printf("\n  Input receiver index (%lu - %lu): ",                    \
        (unsigned long)SFLOW_MGR_MIN_RECEIVER_INDEX,                                                \
        (unsigned long)SFLOW_MGR_MAX_RECEIVER_INDEX);                                               \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_OWNER_NAME()                                                  \
    BACKDOOR_MGR_Printf("\n  Input owner name (maximum length %lu): ",               \
        (unsigned long)SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN);                                  \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_TIMEOUT()                                                     \
    BACKDOOR_MGR_Printf("\n  Input owner timeout (%lu - %lu): ",                     \
        (unsigned long)SYS_ADPT_SFLOW_MIN_RECEIVER_TIMEOUT,                                         \
        (unsigned long)SYS_ADPT_SFLOW_MAX_RECEIVER_TIMEOUT);                                        \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_DESTINATION()                                                 \
    BACKDOOR_MGR_Printf("\n  Input destination ip address: ");                       \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_SOCK_PORT()                                                   \
    BACKDOOR_MGR_Printf("\n  Input sock port (%lu - %lu): ",                         \
        (unsigned long)SFLOW_MGR_MIN_RECEIVER_SOCK_PORT,                                            \
        (unsigned long)SFLOW_MGR_MAX_RECEIVER_SOCK_PORT);                                           \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_MAX_DATAGRAM_SIZE()                                           \
    BACKDOOR_MGR_Printf("\n  Input maximum datagram size (%lu - %lu): ",             \
        (unsigned long)SYS_ADPT_SFLOW_MIN_RECEIVER_DATAGRAM_SIZE,                                   \
        (unsigned long)SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE);                                  \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_RECEIVER_DATAGRAM_VERSION()                                            \
    BACKDOOR_MGR_Printf("\n  Input datagram version (%lu - %lu): ",                  \
        (unsigned long)SFLOW_MGR_MIN_RECEIVER_DATAGRAM_VERSION,                                     \
        (unsigned long)SFLOW_MGR_MAX_RECEIVER_DATAGRAM_VERSION);                                    \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_INTERFACE()                                                            \
    BACKDOOR_MGR_Printf("\n  Input ifindex : ");                                     \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_INSTANCE_ID()                                                          \
    BACKDOOR_MGR_Printf("\n  Input instance id (%lu - %lu): ",                       \
        (unsigned long)SFLOW_MGR_MIN_INSTANCE_ID,                                                   \
        (unsigned long)SFLOW_MGR_MAX_INSTANCE_ID);                                                  \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_SAMPLING_RATE()                                                        \
    BACKDOOR_MGR_Printf("\n  Input rate (%lu - %lu): ",                              \
        (unsigned long)SYS_ADPT_SFLOW_MIN_SAMPLING_RATE,                                            \
        (unsigned long)SYS_ADPT_SFLOW_MAX_SAMPLING_RATE);                                           \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_SAMPLING_MAX_HEADER_SIZE()                                             \
    BACKDOOR_MGR_Printf("\n  Input maximun header size (%lu - %lu): ",               \
        (unsigned long)SYS_ADPT_SFLOW_MIN_SAMPLING_HEADER_SIZE,                                     \
        (unsigned long)SYS_ADPT_SFLOW_MAX_SAMPLING_HEADER_SIZE);                                    \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define INPUT_POLLING_INTERVAL()                                                     \
    BACKDOOR_MGR_Printf("\n  Input interval (%lu - %lu): ",                          \
        (unsigned long)SYS_ADPT_SFLOW_MIN_POLLING_INTERVAL,                                         \
        (unsigned long)SYS_ADPT_SFLOW_MAX_POLLING_INTERVAL);                                        \
    BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);                         \
    BACKDOOR_MGR_Printf("\n");

#define IFINDEX_TO_DATA_SOURCE_STRING(ifindex)                                       \
    SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);              \
    snprintf(data_source_str, sizeof(data_source_str), "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);   \
    data_source_str[sizeof(data_source_str)-1] = '\0';


static void SFLOW_BACKDOOR_PrintMenu(void);
static void SFLOW_BACKDOOR_Main(void);

static void SFLOW_BACKDOOR_PrintMenu(void)
{
    BACKDOOR_MGR_Printf("\n\n  ------------------ SFLOW Backdoor ------------------\n");
    BACKDOOR_MGR_Printf("\n   0 : Exit");
    BACKDOOR_MGR_Printf("\n   1 : Set receiver owner");
    BACKDOOR_MGR_Printf("\n   2 : Set receiver timeout");
    BACKDOOR_MGR_Printf("\n   3 : Set receiver destination");
    BACKDOOR_MGR_Printf("\n   4 : Set receiver sock port");
    BACKDOOR_MGR_Printf("\n   5 : Set receiver maximum datagram size");
    BACKDOOR_MGR_Printf("\n   6 : Set receiver datagram version");
    BACKDOOR_MGR_Printf("\n   7 : Get receiver entry");
    BACKDOOR_MGR_Printf("\n   8 : Get next receiver entry");

    BACKDOOR_MGR_Printf("\n   9 : Set sampling rate");
    BACKDOOR_MGR_Printf("\n  10 : Set sampling maximun header size");
    BACKDOOR_MGR_Printf("\n  11 : Set sampling receiver index");
    BACKDOOR_MGR_Printf("\n  12 : Get sampling entry");
    BACKDOOR_MGR_Printf("\n  13 : Get next sampling entry");

    BACKDOOR_MGR_Printf("\n  14 : Set polling interval");
    BACKDOOR_MGR_Printf("\n  15 : Set polling receiver index");
    BACKDOOR_MGR_Printf("\n  16 : Get polling entry");
    BACKDOOR_MGR_Printf("\n  17 : Get next polling entry");

    BACKDOOR_MGR_Printf("\n  18 : Create receiver owner");
    BACKDOOR_MGR_Printf("\n  19 : Destroy receiver owner");
    BACKDOOR_MGR_Printf("\n  20 : Create sampling instance");
    BACKDOOR_MGR_Printf("\n  21 : Destroy sampling instance");
    BACKDOOR_MGR_Printf("\n  22 : Create polling instance");
    BACKDOOR_MGR_Printf("\n  23 : Destroy polling instance");
    BACKDOOR_MGR_Printf("\n  24 : Show sflow information");
    BACKDOOR_MGR_Printf("\n  25 : Show sflow owner information");
    BACKDOOR_MGR_Printf("\n  26 : Show sflow interface information");
    BACKDOOR_MGR_Printf("\n\n  ------------------ Press 0 to EXIT ------------------\n");
    return;
}

void SFLOW_BACKDOOR_Main(void)
{
    enum
    {
        UI32_MAX_LEN         = 10 , /* 4294967295 */
        DATA_SOURCE_STR_LEN = (sizeof("Eth ")+UI32_MAX_LEN+sizeof("/")+UI32_MAX_LEN)-1
    };

    SFLOW_MGR_Receiver_T  receiver_entry;
    SFLOW_MGR_Sampling_T  sampling_entry;
    SFLOW_MGR_Polling_T   polling_entry;
    L_INET_AddrIp_T ip_address;
    UI32_T menu_item = 0;
    UI32_T receiver_index = 0;
    UI32_T timeout = 0;
    UI32_T udp_port = 0;
    UI32_T max_datagram_size = 0;
    UI32_T datagram_version = 0;
    UI32_T ifindex = 0;
    UI32_T instance_id = 0;
    UI32_T rate = 0;
    UI32_T max_header_size = 0;
    UI32_T interval = 0;
    UI32_T unit = 0, port = 0, trunk_id = 0;
    char input_str[MAX_INPUT_STR_LEN+1] = {0};
    char owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN+1] = {0};
    char ip_address_str[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};
    char data_source_str[DATA_SOURCE_STR_LEN + 1] = {0};

    while (1)
    {
        SFLOW_BACKDOOR_PrintMenu();
        BACKDOOR_MGR_Printf("\n  Please Enter Your Choice : ");
        BACKDOOR_MGR_RequestKeyIn(input_str, MAX_INPUT_STR_LEN);
        BACKDOOR_MGR_Printf("\n\n");
        menu_item = atoi(input_str);

        switch (menu_item)
        {
            case 1: /* Set receiver owner */
            {
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(owner_name, input_str, sizeof(owner_name)-1);
                owner_name[sizeof(owner_name)-1] = '\0';

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverOwner(receiver_index, owner_name))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver owner success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver owner fail.\n");
                }
                break;
            }

            case 2: /* Set receiver timeout */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_TIMEOUT();
                timeout = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverTimeout(receiver_index, timeout))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver timeout success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver timeout fail.\n");
                }
                break;

            case 3: /* Set receiver destination */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                memset(&ip_address, 0, sizeof(ip_address));
                INPUT_RECEIVER_DESTINATION();
                L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, input_str, (L_INET_Addr_T *)&ip_address, sizeof(ip_address));

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverDestination(receiver_index, &ip_address))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver destination success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver destination fail.\n");
                }
                break;

            case 4: /* Set receiver sock port */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_SOCK_PORT();
                udp_port = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverSockPort(receiver_index, udp_port))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver sock port success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver sock port fail.\n");
                }
                break;

            case 5: /* Set receiver maximum datagram size */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_MAX_DATAGRAM_SIZE();
                max_datagram_size = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverMaxDatagramSize(receiver_index, max_datagram_size))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver maximum datagram size success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver maximum datagram size fail.\n");
                }
                break;

            case 6: /* Set receiver datagram version */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_DATAGRAM_VERSION();
                datagram_version = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetReceiverDatagramVersion(receiver_index, datagram_version))
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver datagram version success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set receiver datagram version fail.\n");
                }
                break;

            case 7: /* Get receiver entry */
                memset(&receiver_entry, 0, sizeof(receiver_entry));

                INPUT_RECEIVER_INDEX();
                receiver_entry.receiver_index = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetReceiverEntry(&receiver_entry))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &receiver_entry.address,
                        ip_address_str, sizeof(ip_address_str));

                    BACKDOOR_MGR_Printf("  Get receiver entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)receiver_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", receiver_entry.owner_name);
                    BACKDOOR_MGR_Printf("  Receiver Timeout      : %lu\n", (unsigned long)receiver_entry.timeout);
                    BACKDOOR_MGR_Printf("  Receiver Destination  : %s\n", ip_address_str);
                    BACKDOOR_MGR_Printf("  Receiver Socket Port  : %lu\n", (unsigned long)receiver_entry.udp_port);
                    BACKDOOR_MGR_Printf("  Maximum Datagram Size : %lu\n", (unsigned long)receiver_entry.max_datagram_size);
                    BACKDOOR_MGR_Printf("  Datagram Version      : %lu\n", (unsigned long)receiver_entry.datagram_version);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get receiver entry fail.\n");
                }
                break;

            case 8: /* Get next receiver entry */
                memset(&receiver_entry, 0, sizeof(receiver_entry));

                INPUT_RECEIVER_INDEX();
                receiver_entry.receiver_index = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextReceiverEntry(&receiver_entry))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &receiver_entry.address,
                        ip_address_str, sizeof(ip_address_str));

                    BACKDOOR_MGR_Printf("  Get next receiver entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)receiver_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", receiver_entry.owner_name);
                    BACKDOOR_MGR_Printf("  Receiver Timeout      : %lu\n", (unsigned long)receiver_entry.timeout);
                    BACKDOOR_MGR_Printf("  Receiver Destination  : %s\n", ip_address_str);
                    BACKDOOR_MGR_Printf("  Receiver Socket Port  : %lu\n", (unsigned long)receiver_entry.udp_port);
                    BACKDOOR_MGR_Printf("  Maximum Datagram Size : %lu\n", (unsigned long)receiver_entry.max_datagram_size);
                    BACKDOOR_MGR_Printf("  Datagram Version      : %lu\n", (unsigned long)receiver_entry.datagram_version);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get next receiver entry fail.\n");
                }
                break;

            case 9: /* Set sampling rate */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                INPUT_SAMPLING_RATE();
                rate = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetSamplingRate(ifindex, instance_id, rate))
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling rate success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling rate fail.\n");
                }
                break;

            case 10: /* Set sampling maximun header size */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                INPUT_SAMPLING_MAX_HEADER_SIZE();
                max_header_size = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetSamplingMaxHeaderSize(ifindex, instance_id, max_header_size))
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling maximun header size success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling maximun header size fail.\n");
                }
                break;

            case 11: /* Set sampling receiver index */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetSamplingReceiverIndex(ifindex, instance_id, receiver_index))
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling receiver index success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set sampling receiver index fail.\n");
                }
                break;

            case 12: /* Get sampling entry */
                memset(&sampling_entry, 0, sizeof(sampling_entry));

                INPUT_INTERFACE();
                sampling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                sampling_entry.instance_id = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetSamplingEntry(&sampling_entry))
                {
                    BACKDOOR_MGR_Printf("  Get sampling entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)sampling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)sampling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Sampling Rate         : %lu\n", (unsigned long)sampling_entry.sampling_rate);
                    BACKDOOR_MGR_Printf("  Maximum Header Size   : %lu\n", (unsigned long)sampling_entry.max_header_size);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)sampling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", sampling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get sampling entry fail.\n");
                }
                break;

            case 13: /* Get next sampling entry */
                memset(&sampling_entry, 0, sizeof(sampling_entry));

                INPUT_INTERFACE();
                sampling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                sampling_entry.instance_id = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextSamplingEntry(&sampling_entry))
                {
                    BACKDOOR_MGR_Printf("  Get  next sampling entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)sampling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)sampling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Sampling Rate         : %lu\n", (unsigned long)sampling_entry.sampling_rate);
                    BACKDOOR_MGR_Printf("  Maximum Header Size   : %lu\n", (unsigned long)sampling_entry.max_header_size);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)sampling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", sampling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get  next sampling entry fail.\n");
                }
                break;

            case 14: /* Set polling interval */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                INPUT_POLLING_INTERVAL();
                interval = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetPollingInterval(ifindex, instance_id, interval))
                {
                    BACKDOOR_MGR_Printf("\n  Set polling interval success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set polling interval fail.\n");
                }
                break;

            case 15: /* Set polling receiver index */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_SetPollingReceiverIndex(ifindex, instance_id, receiver_index))
                {
                    BACKDOOR_MGR_Printf("\n  Set polling receiver index success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Set polling receiver index fail.\n");
                }
                break;

            case 16: /* Get polling entry */
                memset(&polling_entry, 0, sizeof(polling_entry));

                INPUT_INTERFACE();
                polling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                polling_entry.instance_id = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetPollingEntry(&polling_entry))
                {
                    BACKDOOR_MGR_Printf("  Get polling entry success.\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)polling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)polling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Polling Interval      : %lu\n", (unsigned long)polling_entry.polling_interval);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)polling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", polling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get polling entry fail.\n");
                }
                break;

            case 17: /* Get next polling entry */
                memset(&polling_entry, 0, sizeof(polling_entry));

                INPUT_INTERFACE();
                polling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                polling_entry.instance_id = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextPollingEntry(&polling_entry))
                {
                    BACKDOOR_MGR_Printf("  Get next polling entry success.\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)polling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)polling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Polling Interval      : %lu\n", (unsigned long)polling_entry.polling_interval);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)polling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", polling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Get next polling entry fail.\n");
                }
                break;

            case 18: /* Create receiver owner */
                memset(&receiver_entry, 0, sizeof(receiver_entry));

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(receiver_entry.owner_name, input_str, sizeof(receiver_entry.owner_name)-1);
                receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

                INPUT_RECEIVER_TIMEOUT();
                receiver_entry.timeout = atoi(input_str);

                INPUT_RECEIVER_DESTINATION();
                L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, input_str,
                    (L_INET_Addr_T *)&receiver_entry.address, sizeof(receiver_entry.address));

                INPUT_RECEIVER_SOCK_PORT();
                receiver_entry.udp_port = atoi(input_str);

                INPUT_RECEIVER_MAX_DATAGRAM_SIZE();
                receiver_entry.max_datagram_size = atoi(input_str);

                INPUT_RECEIVER_DATAGRAM_VERSION();
                receiver_entry.datagram_version = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_CreateReceiverEntry(&receiver_entry))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &receiver_entry.address,
                        ip_address_str, sizeof(ip_address_str));

                    BACKDOOR_MGR_Printf("  Create receiver entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)receiver_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", receiver_entry.owner_name);
                    BACKDOOR_MGR_Printf("  Receiver Timeout      : %lu\n", (unsigned long)receiver_entry.timeout);
                    BACKDOOR_MGR_Printf("  Receiver Destination  : %s\n", ip_address_str);
                    BACKDOOR_MGR_Printf("  Receiver Socket Port  : %lu\n", (unsigned long)receiver_entry.udp_port);
                    BACKDOOR_MGR_Printf("  Maximum Datagram Size : %lu\n", (unsigned long)receiver_entry.max_datagram_size);
                    BACKDOOR_MGR_Printf("  Datagram Version      : %lu\n", (unsigned long)receiver_entry.datagram_version);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Create receiver entry fail.\n");
                }
                break;

            case 19: /* Destroy receiver owner */
                INPUT_RECEIVER_INDEX();
                receiver_index = atoi(input_str);

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(owner_name, input_str, sizeof(owner_name)-1);
                owner_name[sizeof(owner_name)-1] = '\0';

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_DestroyReceiverEntryByOwnerName(owner_name))
                {
                    BACKDOOR_MGR_Printf("\n  Destroy receiver owner success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Destroy receiver owner fail.\n");
                }
                break;

            case 20: /* Create sampling instance */
                memset(&sampling_entry, 0, sizeof(sampling_entry));

                INPUT_INTERFACE();
                sampling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                sampling_entry.instance_id = atoi(input_str);

                INPUT_SAMPLING_RATE();
                sampling_entry.sampling_rate = atoi(input_str);

                INPUT_SAMPLING_MAX_HEADER_SIZE();
                sampling_entry.max_header_size = atoi(input_str);

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(sampling_entry.receiver_owner_name, input_str, sizeof(sampling_entry.receiver_owner_name)-1);
                sampling_entry.receiver_owner_name[sizeof(sampling_entry.receiver_owner_name)-1] = '\0';

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_CreateSamplingEntry(&sampling_entry))
                {
                    BACKDOOR_MGR_Printf("  Create sampling entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)sampling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)sampling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Sampling Rate         : %lu\n", (unsigned long)sampling_entry.sampling_rate);
                    BACKDOOR_MGR_Printf("  Maximum Header Size   : %lu\n", (unsigned long)sampling_entry.max_header_size);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)sampling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", sampling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Create sampling entry fail.\n");
                }
                break;

            case 22: /* Create polling instance */
                memset(&polling_entry, 0, sizeof(polling_entry));

                INPUT_INTERFACE();
                polling_entry.ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                polling_entry.instance_id = atoi(input_str);

                INPUT_POLLING_INTERVAL();
                polling_entry.polling_interval = atoi(input_str);

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(polling_entry.receiver_owner_name, input_str, sizeof(polling_entry.receiver_owner_name)-1);
                polling_entry.receiver_owner_name[sizeof(polling_entry.receiver_owner_name)-1] = '\0';

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_CreatePollingEntry(&polling_entry))
                {
                    BACKDOOR_MGR_Printf("  Create polling instance success.\n");
                    BACKDOOR_MGR_Printf("  Data Source           : %lu\n", (unsigned long)polling_entry.ifindex);
                    BACKDOOR_MGR_Printf("  Instance ID           : %lu\n", (unsigned long)polling_entry.instance_id);
                    BACKDOOR_MGR_Printf("  Polling Interval      : %lu\n", (unsigned long)polling_entry.polling_interval);
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)polling_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", polling_entry.receiver_owner_name);
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Create polling instance fail.\n");
                }
                break;

            case 23: /* Destroy polling instance */
                INPUT_INTERFACE();
                ifindex = atoi(input_str);

                INPUT_INSTANCE_ID();
                instance_id = atoi(input_str);

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_DestroyPollingEntry(ifindex, instance_id))
                {
                    BACKDOOR_MGR_Printf("\n  Destroy polling instance success.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\n  Destroy polling instance fail.\n");
                }
                break;

            case 24: /* Show sflow information */
                memset(&receiver_entry, 0, sizeof(receiver_entry));

                while (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextActiveReceiverEntry(&receiver_entry))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &receiver_entry.address,
                        ip_address_str, sizeof(ip_address_str));

                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)receiver_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", receiver_entry.owner_name);
                    BACKDOOR_MGR_Printf("  Receiver Timeout      : %lu\n", (unsigned long)receiver_entry.timeout);
                    BACKDOOR_MGR_Printf("  Receiver Destination  : %s\n", ip_address_str);
                    BACKDOOR_MGR_Printf("  Receiver Socket Port  : %lu\n", (unsigned long)receiver_entry.udp_port);
                    BACKDOOR_MGR_Printf("  Maximum Datagram Size : %lu\n", (unsigned long)receiver_entry.max_datagram_size);
                    BACKDOOR_MGR_Printf("  Datagram Version      : %lu\n", (unsigned long)receiver_entry.datagram_version);

                    BACKDOOR_MGR_Printf("  Sampling\n");
                    BACKDOOR_MGR_Printf("  Data Source Instance ID Rate       Maximum Header Size \n");
                    BACKDOOR_MGR_Printf("  ----------- ----------- ---------- --------------------\n");

                    memset(&sampling_entry, 0, sizeof(sampling_entry));
                    strncpy(sampling_entry.receiver_owner_name, receiver_entry.owner_name,
                        sizeof(sampling_entry.receiver_owner_name)-1);
                    sampling_entry.receiver_owner_name[sizeof(sampling_entry.receiver_owner_name)-1] = '\0';

                    while (SFLOW_MGR_RETURN_SUCCESS ==
                        SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName(&sampling_entry))
                    {
                        IFINDEX_TO_DATA_SOURCE_STRING(polling_entry.ifindex);

                        BACKDOOR_MGR_Printf("  %-11s %11lu %10lu %20lu %-32s \n",
                            data_source_str,
                            (unsigned long)sampling_entry.instance_id,
                            (unsigned long)sampling_entry.sampling_rate,
                            (unsigned long)sampling_entry.max_header_size,
                            sampling_entry.receiver_owner_name);
                    }

                    BACKDOOR_MGR_Printf("  Polling\n");
                    BACKDOOR_MGR_Printf("  Data Source Instance ID Interval  \n");
                    BACKDOOR_MGR_Printf("  ----------- ----------- ----------\n");

                    memset(&polling_entry, 0, sizeof(polling_entry));
                    strncpy(polling_entry.receiver_owner_name, receiver_entry.owner_name,
                        sizeof(polling_entry.receiver_owner_name)-1);
                    polling_entry.receiver_owner_name[sizeof(polling_entry.receiver_owner_name)-1] = '\0';

                    while (SFLOW_MGR_RETURN_SUCCESS ==
                        SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName(&polling_entry))
                    {
                        IFINDEX_TO_DATA_SOURCE_STRING(polling_entry.ifindex);

                        BACKDOOR_MGR_Printf("  %-11s %11lu %10lu %-32s \n",
                            data_source_str,
                            (unsigned long)polling_entry.instance_id,
                            (unsigned long)polling_entry.polling_interval,
                            polling_entry.receiver_owner_name);
                    }
                }
                break;

            case 25: /* Show sflow owner information */
                memset(&receiver_entry, 0, sizeof(receiver_entry));

                INPUT_RECEIVER_OWNER_NAME();
                strncpy(receiver_entry.owner_name, input_str, sizeof(receiver_entry.owner_name)-1);
                receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

                if (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &receiver_entry.address,
                        ip_address_str, sizeof(ip_address_str));

                    BACKDOOR_MGR_Printf("  Get receiver entry success.\n\n");
                    BACKDOOR_MGR_Printf("  Receiver Index        : %lu\n", (unsigned long)receiver_entry.receiver_index);
                    BACKDOOR_MGR_Printf("  Receiver Owner Name   : %s\n", receiver_entry.owner_name);
                    BACKDOOR_MGR_Printf("  Receiver Timeout      : %lu\n", (unsigned long)receiver_entry.timeout);
                    BACKDOOR_MGR_Printf("  Receiver Destination  : %s\n", ip_address_str);
                    BACKDOOR_MGR_Printf("  Receiver Socket Port  : %lu\n", (unsigned long)receiver_entry.udp_port);
                    BACKDOOR_MGR_Printf("  Maximum Datagram Size : %lu\n", (unsigned long)receiver_entry.max_datagram_size);
                    BACKDOOR_MGR_Printf("  Datagram Version      : %lu\n", (unsigned long)receiver_entry.datagram_version);
                }
                else
                {
                    BACKDOOR_MGR_Printf("  Get receiver entry fail.\n");
                }

                BACKDOOR_MGR_Printf("  Sampling\n");
                BACKDOOR_MGR_Printf("  Data Source Instance ID Rate       Maximum Header Size \n");
                BACKDOOR_MGR_Printf("  ----------- ----------- ---------- --------------------\n");

                memset(&sampling_entry, 0, sizeof(sampling_entry));
                strncpy(sampling_entry.receiver_owner_name, receiver_entry.owner_name,
                    sizeof(sampling_entry.receiver_owner_name)-1);
                sampling_entry.receiver_owner_name[sizeof(sampling_entry.receiver_owner_name)-1] = '\0';

                while (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName(&sampling_entry))
                {
                    IFINDEX_TO_DATA_SOURCE_STRING(polling_entry.ifindex);

                    BACKDOOR_MGR_Printf("  %-11s %11lu %10lu %20lu %-32s \n",
                        data_source_str,
                        (unsigned long)sampling_entry.instance_id,
                        (unsigned long)sampling_entry.sampling_rate,
                        (unsigned long)sampling_entry.max_header_size,
                        sampling_entry.receiver_owner_name);
                }

                BACKDOOR_MGR_Printf("  Polling\n");
                BACKDOOR_MGR_Printf("  Data Source Instance ID Interval  \n");
                BACKDOOR_MGR_Printf("  ----------- ----------- ----------\n");

                memset(&polling_entry, 0, sizeof(polling_entry));
                strncpy(polling_entry.receiver_owner_name, receiver_entry.owner_name,
                    sizeof(polling_entry.receiver_owner_name)-1);
                polling_entry.receiver_owner_name[sizeof(polling_entry.receiver_owner_name)-1] = '\0';

                while (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName(&polling_entry))
                {
                    IFINDEX_TO_DATA_SOURCE_STRING(polling_entry.ifindex);

                    BACKDOOR_MGR_Printf("\n  %-11s %11lu %10lu %-32s \n",
                        data_source_str,
                        (unsigned long)polling_entry.instance_id,
                        (unsigned long)polling_entry.polling_interval,
                        polling_entry.receiver_owner_name);
                }
                break;

            case 26: /* Show sflow interface information */
                memset(&sampling_entry, 0, sizeof(sampling_entry));
                memset(&polling_entry, 0, sizeof(polling_entry));

                INPUT_INTERFACE();
                sampling_entry.ifindex = atoi(input_str);
                polling_entry.ifindex = sampling_entry.ifindex;

                INPUT_INSTANCE_ID();
                sampling_entry.instance_id = atoi(input_str);
                polling_entry.instance_id = sampling_entry.instance_id;

                BACKDOOR_MGR_Printf("\n  Sampling\n");
                BACKDOOR_MGR_Printf("\n  Data Source Instance ID Rate       Maximum Header Size  Receiver Owner Name             \n");
                BACKDOOR_MGR_Printf("\n  ----------- ----------- ---------- -------------------- --------------------------------\n");

                while (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextActiveSamplingEntryByDatasource(&sampling_entry))
                {
                    IFINDEX_TO_DATA_SOURCE_STRING(sampling_entry.ifindex);

                    BACKDOOR_MGR_Printf("\n  %-11s %11lu %10lu %20lu %-32s \n",
                        data_source_str,
                        (unsigned long)sampling_entry.instance_id,
                        (unsigned long)sampling_entry.sampling_rate,
                        (unsigned long)sampling_entry.max_header_size,
                        sampling_entry.receiver_owner_name);
                }

                BACKDOOR_MGR_Printf("\n  Polling\n");
                BACKDOOR_MGR_Printf("\n  Data Source Instance ID Interval   Receiver Owner Name             \n");
                BACKDOOR_MGR_Printf("\n  ----------- ----------- ---------- --------------------------------\n");

                while (SFLOW_MGR_RETURN_SUCCESS ==
                    SFLOW_MGR_GetNextActivePollingEntryByDatasource(&polling_entry))
                {
                    IFINDEX_TO_DATA_SOURCE_STRING(polling_entry.ifindex);

                    BACKDOOR_MGR_Printf("\n  %-11s %11lu %10lu %-32s \n",
                        data_source_str,
                        (unsigned long)polling_entry.instance_id,
                        (unsigned long)polling_entry.polling_interval,
                        polling_entry.receiver_owner_name);
                }
                break;

            case 0:
            default:
                return;
        }
    }
}

void SFLOW_BACKDOOR_CallBack(void)
{
#ifdef SFLOW_NO_L_THREADGRP
#define SFLOW_HAVE_L_THREADGRP    0
#else
#define SFLOW_HAVE_L_THREADGRP    1
#endif

#if (SFLOW_HAVE_L_THREADGRP == 1)
    L_THREADGRP_Handle_T    tg_handle;

    UI32_T backdoor_member_id;

    tg_handle = SFLOW_PROC_COMM_GetSflowTGHandle();

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }
#endif /* SFLOW_HAVE_L_THREADGRP */

    SFLOW_BACKDOOR_Main();

#if (SFLOW_HAVE_L_THREADGRP == 1)
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
#endif /* SFLOW_HAVE_L_THREADGRP */
}

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
