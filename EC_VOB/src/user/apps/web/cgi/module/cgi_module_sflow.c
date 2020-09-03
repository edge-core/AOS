#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "sflow_pmgr.h"

/**----------------------------------------------------------------------
 * This API is used to set sFlow.
 *
 * @param owner (required, string) owner name
 * @param destination (required, string) owner IP address
 * @param timeout (required, number) Timeout of sFlow receiver; <30-10000000> in seconds
 * @param maxDatagramSize (optional, number) Maximum size of sFlow receiver datagram; <200-1500>
 * @param maxHeaderSize (optional, number) Maximum number of bytes that should be copied from a sampled packet; <64-256>
 * @param pollingInterval (required, number) Timeout of sFlow receiver; <30-10000000> in seconds
 * @param samplingRate (required, number) Rate of packet sampling; <256-16777215>
 * @param ports (required, array) Interface for sFlow
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SFLOW_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *owner_p;
    json_t  *dest_p;
    json_t  *timeout_p;
    json_t  *max_datagram_p;
    json_t  *max_header_p;
    json_t  *polling_p;
    json_t  *rate_p;
    json_t  *ports_p;
    const char*  owner_str_p;
    const char*  dest_str_p;
    SFLOW_MGR_Receiver_T receiver_entry;
    UI32_T max_header_size = SYS_DFLT_SFLOW_MAX_SAMPLING_HEADER_SIZE;
    UI32_T polling = 0, rate = 0;
    UI32_T instance_id = SFLOW_MGR_MIN_INSTANCE_ID;
    UI32_T port_num = 0, idx = 0;

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.datagram_version = SYS_DFLT_SFLOW_RECEIVER_DATAGRAM_VERSION;
    receiver_entry.max_datagram_size = SYS_DFLT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE;
    receiver_entry.udp_port = SFLOW_MGR_RECEIVER_SOCK_PORT;
    owner_p = CGI_REQUEST_GetBodyValue(http_request, "owner");

    if (owner_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get owner.");
    }
    owner_str_p = json_string_value(owner_p);
    strncpy(receiver_entry.owner_name, owner_str_p, sizeof(receiver_entry.owner_name)-1);
    receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

    dest_p = CGI_REQUEST_GetBodyValue(http_request, "destination");

    if (dest_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get destination address.");
    }
    dest_str_p = json_string_value(dest_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
            dest_str_p, (L_INET_Addr_T *)&receiver_entry.address, sizeof(receiver_entry.address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dest IP format of sFlow.");
    }

    timeout_p = CGI_REQUEST_GetBodyValue(http_request, "timeout");

    if (timeout_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get timeout.");
    }
    receiver_entry.timeout = json_integer_value(timeout_p);

    max_datagram_p = CGI_REQUEST_GetBodyValue(http_request, "maxDatagramSize");

    if (max_datagram_p != NULL)
    {
        receiver_entry.max_datagram_size = json_integer_value(max_datagram_p);
    }

    max_header_p = CGI_REQUEST_GetBodyValue(http_request, "maxHeaderSize");

    if (max_header_p != NULL)
    {
        max_header_size = json_integer_value(max_header_p);
    }

    polling_p = CGI_REQUEST_GetBodyValue(http_request, "pollingInterval");

    if (polling_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get pollingInterval.");
    }
    polling = json_integer_value(polling_p);

    rate_p = CGI_REQUEST_GetBodyValue(http_request, "samplingRate");

    if (rate_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get samplingRate.");
    }
    rate = json_integer_value(rate_p);

    ports_p = CGI_REQUEST_GetBodyValue(http_request, "ports");

    if (ports_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ports.");
    }
    port_num = json_array_size(ports_p);

    if (0 >= port_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ports.");
    }

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreateReceiverEntry(&receiver_entry))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.CreateReceiverError", "Failed.");
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        json_t  *port_p;
        SFLOW_MGR_Sampling_T  sampling_entry;
        SFLOW_MGR_Polling_T  polling_entry;

        port_p = json_array_get(ports_p, idx);

        if (NULL == port_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port in ports.");
        }

        memset(&sampling_entry, 0, sizeof(sampling_entry));
        sampling_entry.instance_id = instance_id;
        sampling_entry.ifindex = json_integer_value(port_p);
        strncpy(sampling_entry.receiver_owner_name, receiver_entry.owner_name, sizeof(sampling_entry.receiver_owner_name)-1);
        sampling_entry.sampling_rate = rate;
        sampling_entry.max_header_size = max_header_size;

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreateSamplingEntry(&sampling_entry))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.CreateSamplingError", "Failed.");
        }

        memset(&polling_entry, 0, sizeof(polling_entry));
        polling_entry.ifindex = sampling_entry.ifindex;
        polling_entry.instance_id = instance_id;
        strncpy(polling_entry.receiver_owner_name, receiver_entry.owner_name, sizeof(polling_entry.receiver_owner_name)-1);
        polling_entry.polling_interval = polling;

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_CreatePollingEntry(&polling_entry))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.CreatePollingError", "Failed.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read sFlow setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SFLOW_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ports_p = json_array();
    SFLOW_MGR_Receiver_T receiver_entry;
    SFLOW_MGR_Sampling_T sampling_entry;
    SFLOW_MGR_Polling_T  polling_entry;
    char ip_address_ar[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};
    UI32_T instance_id = SFLOW_MGR_MIN_INSTANCE_ID;
    UI32_T polling = 0, rate = 0, max_header_size = 0;
    UI32_T ifindex = 0;

    memset(&receiver_entry, 0, sizeof(receiver_entry));

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetNextActiveReceiverEntry(&receiver_entry))
    {
        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&receiver_entry.address,
                                                           ip_address_ar,
                                                           sizeof(ip_address_ar)))
        {
            continue;
        }

        for (ifindex = SFLOW_MGR_MIN_IFINDEX; ifindex <= SFLOW_MGR_MAX_IFINDEX; ++ifindex)
        {
            memset(&sampling_entry, 0, sizeof(sampling_entry));
            sampling_entry.ifindex = ifindex;
            sampling_entry.instance_id = instance_id;

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetActiveSamplingEntry(&sampling_entry) ||
                    (0 != strcmp(sampling_entry.receiver_owner_name, receiver_entry.owner_name)))
            {
                continue; //should not happen
            }

            memset(&polling_entry, 0, sizeof(polling_entry));
            polling_entry.ifindex = ifindex;
            polling_entry.instance_id = instance_id;

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetActivePollingEntry(&polling_entry) ||
                (0 != strcmp(polling_entry.receiver_owner_name, receiver_entry.owner_name)))
            {
                continue; //should not happen
            }

            rate = sampling_entry.sampling_rate;
            max_header_size = sampling_entry.max_header_size;
            polling = polling_entry.polling_interval;
            json_array_append_new(ports_p, json_integer(ifindex));
        } //for ifindex

        json_object_set_new(result_p, "owner", json_string(receiver_entry.owner_name));
        json_object_set_new(result_p, "destination", json_string(ip_address_ar));
        json_object_set_new(result_p, "timeout", json_integer(receiver_entry.timeout));
        json_object_set_new(result_p, "maxDatagramSize", json_integer(receiver_entry.max_datagram_size));
        json_object_set_new(result_p, "maxHeaderSize", json_integer(max_header_size));
        json_object_set_new(result_p, "pollingInterval", json_integer(polling));
        json_object_set_new(result_p, "samplingRate", json_integer(rate));
        json_object_set_new(result_p, "ports", ports_p);
        break; //should only has one receiver
    } //SFLOW_PMGR_GetNextActiveReceiverEntry

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete sFlow.
 *
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SFLOW_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    SFLOW_MGR_Receiver_T receiver_entry;
    SFLOW_MGR_Sampling_T sampling_entry;
    SFLOW_MGR_Polling_T  polling_entry;
    UI32_T instance_id = SFLOW_MGR_MIN_INSTANCE_ID;
    UI32_T ifindex = 0;

    memset(&receiver_entry, 0, sizeof(receiver_entry));

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetNextActiveReceiverEntry(&receiver_entry))
    {
        for (ifindex = SFLOW_MGR_MIN_IFINDEX; ifindex <= SFLOW_MGR_MAX_IFINDEX; ++ifindex)
        {
            memset(&sampling_entry, 0, sizeof(sampling_entry));
            sampling_entry.ifindex = ifindex;
            sampling_entry.instance_id = instance_id;

            if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetActiveSamplingEntry(&sampling_entry))
            {
                if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroySamplingEntry(ifindex, instance_id))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.DestroySamplingError", "Failed.");
                }
            }

            memset(&polling_entry, 0, sizeof(polling_entry));
            polling_entry.ifindex = ifindex;
            polling_entry.instance_id = instance_id;

            if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_PMGR_GetActivePollingEntry(&polling_entry))
            {
                if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroyPollingEntry(ifindex, instance_id))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.DestroyPollingError", "Failed.");
                }
            }
        } //for ifindex

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_DestroyReceiverEntryByOwnerName(receiver_entry.owner_name))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "sflow.DestroyReceiverError", "Failed.");
        }
        break; //should only has one receiver
    } //SFLOW_PMGR_GetNextActiveReceiverEntry

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_SFLOW_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_SFLOW_Read;
    handlers.create_handler = CGI_MODULE_SFLOW_Create;
    handlers.delete_handler = CGI_MODULE_SFLOW_Delete;
    CGI_MAIN_Register("/api/v1/sflow", &handlers, 0);
}

static void CGI_MODULE_SFLOW_Init()
{
    CGI_MODULE_SFLOW_RegisterHandlers();
}
