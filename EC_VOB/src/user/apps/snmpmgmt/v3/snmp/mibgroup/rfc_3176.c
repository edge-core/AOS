#include "sys_cpnt.h"
#if (SYS_CPNT_SFLOW == TRUE)

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_adpt.h"
#include "sflow_pmgr.h"
#include "rfc_3176.h"
#include "leaf_3176.h"

//#include "c_lib.h"
#include "l_inet.h"
#include "snmp_mgr.h"
#include "stktplg_pmgr.h"

oid sFlowTable_variables_oid[] = { 1, 3, 6, 1, 4, 1, 4300, 1, 1 };
/*
 * variable3 sFlowTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 sFlowTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_sFlowDataSource,              ASN_OBJECT_ID, RONLY,  var_sFlowTable, 3, { 4, 1, 1 }},
{LEAF_sFlowOwner,                   ASN_OCTET_STR, RWRITE, var_sFlowTable, 3, { 4, 1, 2 }},
{LEAF_sFlowTimeout,                 ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 3 }},
{LEAF_sFlowPacketSamplingRate,      ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 4 }},
{LEAF_sFlowCounterSamplingInterval, ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 5 }},
{LEAF_sFlowMaximumHeaderSize,       ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 6 }},
{LEAF_sFlowMaximumDatagramSize,     ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 7 }},
{LEAF_sFlowCollectorAddressType,    ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 8 }},
{LEAF_sFlowCollectorAddress,        ASN_OCTET_STR, RWRITE, var_sFlowTable, 3, { 4, 1, 9 }},
{LEAF_sFlowCollectorPort,           ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 10 }},
{LEAF_sFlowDatagramVersion,         ASN_INTEGER,   RWRITE, var_sFlowTable, 3, { 4, 1, 11 }},
};

void
init_sFlow(void)
{
    static oid sFlowVersion_oid[] =          { 1, 3, 6, 1, 4, 1, 4300, 1, 1, 1, 0 };
    static oid sFlowAgentAddressType_oid[] = { 1, 3, 6, 1, 4, 1, 4300, 1, 1, 2, 0 };
    static oid sFlowAgentAddress_oid[] =     { 1, 3, 6, 1, 4, 1, 4300, 1, 1, 3, 0 };

    /* sFlowVersion */
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowVersion",
                                         get_sFlowVersion,
                                         sFlowVersion_oid,
                                         OID_LENGTH(sFlowVersion_oid),
                                         HANDLER_CAN_RONLY));

    /* sFlowAgentAddressType */
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowAgentAddressType",
                                         get_sFlowAgentAddressType,
                                         sFlowAgentAddressType_oid,
                                         OID_LENGTH(sFlowAgentAddressType_oid),
                                         HANDLER_CAN_RONLY));

    /* sFlowAgentAddress */
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowAgentAddress",
                                         get_sFlowAgentAddress,
                                         sFlowAgentAddress_oid,
                                         OID_LENGTH(sFlowAgentAddress_oid),
                                         HANDLER_CAN_RONLY));

    /* sFlowTable */
    REGISTER_MIB("sFlowTable", sFlowTable_variables, variable3,
                 sFlowTable_variables_oid);
}

int
get_sFlowVersion(netsnmp_mib_handler          *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info   *reqinfo,
                 netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            UI32_T var_len = 0;
            STKTPLG_MGR_Switch_Info_T data;
            char prod_manufacturer[MAXSIZE_swProdManufacturer + 1];

            memset(&data, 0, sizeof(data));
            data.sw_unit_index = 1;
            if (STKTPLG_PMGR_GetSwitchInfo(&data) != TRUE)
                return SNMP_ERR_GENERR;

            if (SYS_PMGR_GetProductManufacturer(prod_manufacturer) != TRUE)
                return SNMP_ERR_GENERR;

            snprintf((char *)return_buf, sizeof(return_buf), "1.2;%s;%s", prod_manufacturer, data.sw_opcode_ver);
            var_len = strlen((char *)return_buf);
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)return_buf, var_len);
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_sFlowAgentAddressType(netsnmp_mib_handler          *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            UI32_T value = 0;

            if (SFLOW_PMGR_GetAgentAddressType(&value) != TRUE)
                return SNMP_ERR_GENERR;

            long_return = value;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&long_return, sizeof(long_return));
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_sFlowAgentAddress(netsnmp_mib_handler          *handler,
                      netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info   *reqinfo,
                      netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            L_INET_AddrIp_T addr;
            UI32_T var_len = 0;
            UI32_T out_inet_type, out_preflen;

            memset(&addr, 0, sizeof(L_INET_AddrIp_T));
            if (SFLOW_PMGR_GetAgentAddress(&addr) == FALSE)
            {
                return SNMP_ERR_GENERR;
            }

            if (!SNMP_MGR_ConvertLInetAddrToInetAddrAndType(
                    (L_INET_Addr_T *)&addr,
                    &out_inet_type,
                    &var_len,
                    (UI8_T *)return_buf,
                    &out_preflen))
            {
                return SNMP_ERR_GENERR;
            }

            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)return_buf, var_len);
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

#define SFLOWENTRY_INSTANCE_LEN 11
#define IFINDEX_LEN             11

static oid sFlowDataSource_prefix_ifindex[IFINDEX_LEN] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};

BOOL_T sFlowTable_OidIndexToData(UI32_T exact, UI32_T compc,
                                 oid *compl, UI32_T *port)
{
    UI32_T length = (UI32_T)compl[0];
    long diff = 0;
    int i;

    for (i = 0; i < compc - 1 && i < IFINDEX_LEN - 1; i++)
    {
        if (0 != (diff = (long)(compl[i] - sFlowDataSource_prefix_ifindex[i])))
        {
            break;
        }
    }
    if (diff == 0)
    {
        diff = (long)(compc - IFINDEX_LEN);
    }

    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (diff != 0)
        {
            return FALSE;
        }
    }

    if (diff == 0)
    {
        *port = (UI32_T)compl[IFINDEX_LEN - 1];
    }
    else if (diff < 0)
    {
        *port = 0;
    }
    else
    {
        *port = SYS_ADPT_TOTAL_NBR_OF_LPORT + 1;
    }

    return TRUE;
}

/*
 * var_sFlowTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_sFlowTable(struct variable *vp,
               oid     *name,
               size_t  *length,
               int     exact,
               size_t  *var_len,
               WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    UI32_T port = 0;
    char *p;
    oid best_inst[SFLOWENTRY_INSTANCE_LEN] = {0};
    oid compl[SYS_ADPT_MAX_OID_COUNT] = {0};
    SFLOW_OM_PortInfo_T entry;

    switch (vp->magic)
    {
        case LEAF_sFlowOwner:
            *write_method = write_sFlowOwner;
            break;

        case LEAF_sFlowTimeout:
            *write_method = write_sFlowTimeout;
            break;

        case LEAF_sFlowPacketSamplingRate:
            *write_method = write_sFlowPacketSamplingRate;
            break;

        case LEAF_sFlowCounterSamplingInterval:
            *write_method = write_sFlowCounterSamplingInterval;
            break;

        case LEAF_sFlowMaximumHeaderSize:
            *write_method = write_sFlowMaximumHeaderSize;
            break;

        case LEAF_sFlowMaximumDatagramSize:
            *write_method = write_sFlowMaximumDatagramSize;
            break;

        case LEAF_sFlowCollectorAddressType:
            *write_method = write_sFlowCollectorAddressType;
            break;

        case LEAF_sFlowCollectorAddress:
            *write_method = write_sFlowCollectorAddress;
            break;

        case LEAF_sFlowCollectorPort:
            *write_method = write_sFlowCollectorPort;
            break;

        case LEAF_sFlowDatagramVersion:
            *write_method = write_sFlowDatagramVersion;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, SFLOWENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (sFlowTable_OidIndexToData(exact, compc, compl, &port) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (port == 0 || SFLOW_PMGR_GetSflowPortStatus(port, &entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        sFlowTable_OidIndexToData(exact, compc, compl, &port);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (port == 0 || SFLOW_PMGR_GetSflowPortStatus(port, &entry) != TRUE)
            {
                /* get next data */
                if (SFLOW_PMGR_GetNextSflowPortStatus(&port, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (SFLOW_PMGR_GetNextSflowPortStatus(&port, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    /* fill with ifindex oid
     */
    memcpy(best_inst, sFlowDataSource_prefix_ifindex, sizeof(sFlowDataSource_prefix_ifindex));
    best_inst[IFINDEX_LEN - 1] = port;

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index */
    memcpy(name + vp->namelen, best_inst,
           SFLOWENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + SFLOWENTRY_INSTANCE_LEN;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_sFlowDataSource:
            *var_len = IFINDEX_LEN * sizeof(oid);
            memcpy(oid_return, best_inst, *var_len);
            return (u_char *)oid_return;

        case LEAF_sFlowOwner:
            *var_len = strlen((char *)entry.sflow_owner);
            memcpy(return_buf, entry.sflow_owner, *var_len);
            return (u_char *)return_buf;

        case LEAF_sFlowTimeout:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_timeout;
            return (u_char *)&long_return;

        case LEAF_sFlowPacketSamplingRate:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_rate;
            return (u_char *)&long_return;

        case LEAF_sFlowCounterSamplingInterval:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_interval;
            return (u_char *)&long_return;

        case LEAF_sFlowMaximumHeaderSize:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_maxheadersize;
            return (u_char *)&long_return;

        case LEAF_sFlowMaximumDatagramSize:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_maxdatagramsize;
            return (u_char *)&long_return;

        case LEAF_sFlowCollectorAddressType:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_rcvr_address_type;
            return (u_char *)&long_return;

        case LEAF_sFlowCollectorAddress:
        {
            UI32_T out_inet_type, out_inet_addr_len, out_preflen;

            if (!SNMP_MGR_ConvertLInetAddrToInetAddrAndType(
                    (L_INET_Addr_T *)&entry.sflow_rcvr_address,
                    &out_inet_type,
                    &out_inet_addr_len,
                    (UI8_T *)return_buf,
                    &out_preflen))
            {
                return NULL;
            }

            *var_len = out_inet_addr_len;
        }
            return (u_char*)return_buf;

        case LEAF_sFlowCollectorPort:
            *var_len = sizeof(long_return);
            long_return = entry.sflow_rcvr_port;
            return (u_char *)&long_return;

        case LEAF_sFlowDatagramVersion:
            /* only support version 4 */
            *var_len = sizeof(long_return);
            long_return = 4;
            return (u_char *)&long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int
write_sFlowOwner(int action,
                 u_char *var_val,
                 u_char var_val_type,
                 size_t var_val_len,
                 u_char *statP,
                 oid    *name,
                 size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len < MINSIZE_sFlowOwner ||
                var_val_len > SYS_ADPT_SFLOW_MAX_OWNER_STR_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            UI8_T byte_buffer[SYS_ADPT_SFLOW_MAX_OWNER_STR_LEN + 1] = {0};

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            memcpy(byte_buffer, var_val, var_val_len);

            byte_buffer[var_val_len] = '\0';
            if (SFLOW_PMGR_SetSflowOwner(port, byte_buffer) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowTimeout(int action,
                   u_char *var_val,
                   u_char var_val_type,
                   size_t var_val_len,
                   u_char *statP,
                   oid    *name,
                   size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
        {
            I32_T value = 0;

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            value = *(long *)var_val;

            if (value < SYS_ADPT_SFLOW_MIN_TIMEOUT ||
                value > SYS_ADPT_SFLOW_MAX_TIMEOUT)
            {
                return SNMP_ERR_WRONGVALUE;
            }
        }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetSflowTimeout(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowPacketSamplingRate(int action,
                              u_char *var_val,
                              u_char var_val_type,
                              size_t var_val_len,
                              u_char *statP,
                              oid    *name,
                              size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
        {
            I32_T value = 0;

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            value = *(long *)var_val;

            if (value < SYS_ADPT_SFLOW_MIN_RATE ||
                value > SYS_ADPT_SFLOW_MAX_RATE)
            {
                return SNMP_ERR_WRONGVALUE;
            }
        }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetPortSflowRate(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowCounterSamplingInterval(int action,
                                   u_char *var_val,
                                   u_char var_val_type,
                                   size_t var_val_len,
                                   u_char *statP,
                                   oid    *name,
                                   size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
        {
            I32_T value = 0;

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            value = *(long *)var_val;

            if (value < SYS_ADPT_SFLOW_MIN_INTERVAL ||
                value > SYS_ADPT_SFLOW_MAX_INTERVAL)
            {
                return SNMP_ERR_WRONGVALUE;
            }
        }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetPortSflowInterval(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowMaximumHeaderSize(int action,
                             u_char *var_val,
                             u_char var_val_type,
                             size_t var_val_len,
                             u_char *statP,
                             oid    *name,
                             size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
        {
            I32_T value = 0;

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            value = *(long *)var_val;

            if (value < SYS_ADPT_SFLOW_MIN_HEADER_SIZE ||
                value > SYS_ADPT_SFLOW_MAX_HEADER_SIZE)
            {
                return SNMP_ERR_WRONGVALUE;
            }
        }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetSflowMaxHeaderSize(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowMaximumDatagramSize(int      action,
                               u_char   *var_val,
                               u_char   var_val_type,
                               size_t   var_val_len,
                               u_char   *statP,
                               oid      *name,
                               size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
        {
            I32_T value = 0;

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            value = *(long *)var_val;

            if (value < SYS_ADPT_SFLOW_MIN_DATA_SIZE ||
                value > SYS_ADPT_SFLOW_MAX_DATA_SIZE)
            {
                return SNMP_ERR_WRONGVALUE;
            }
        }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetSflowMaxDatagramSize(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowCollectorAddressType(int      action,
                                u_char   *var_val,
                                u_char   var_val_type,
                                size_t   var_val_len,
                                u_char   *statP,
                                oid      *name,
                                size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            switch (*(long *)var_val)
            {
                case VAL_sFlowCollectorAddressType_ipv4:
                case VAL_sFlowCollectorAddressType_ipv4z:
                case VAL_sFlowCollectorAddressType_ipv6:
                case VAL_sFlowCollectorAddressType_ipv6z:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
            /* We do nothing. sFlowCollectorAddressType changes with sFlowCollectorAddress. */
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowCollectorAddress(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len < MINSIZE_sFlowCollectorAddress ||
                var_val_len > MAXSIZE_sFlowCollectorAddress)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            L_INET_AddrIp_T addr;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            switch (var_val_len)
            {
                case 4:
                    addr.type = L_INET_ADDR_TYPE_IPV4;
                    break;
                case 8:
                    addr.type = L_INET_ADDR_TYPE_IPV4Z;
                    break;
                case 16:
                    addr.type = L_INET_ADDR_TYPE_IPV6;
                    break;
                case 20:
                    addr.type = L_INET_ADDR_TYPE_IPV6Z;
                    break;
                default:
                    return SNMP_ERR_COMMITFAILED;
            }

            if (!SNMP_MGR_ConvertInetAddrAndTypeToLInetAddr(
                    addr.type,
                    var_val_len,
                    var_val,
                    0,
                    (L_INET_Addr_T *)&addr))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_PMGR_SetSflowRcvrAddress(port, &addr) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowCollectorPort(int      action,
                         u_char   *var_val,
                         u_char   var_val_type,
                         size_t   var_val_len,
                         u_char   *statP,
                         oid      *name,
                         size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T port;
            I32_T value = 0;

            if (sFlowTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]), &port) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (SFLOW_PMGR_SetSflowRcvrPort(port, value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_sFlowDatagramVersion(int      action,
                           u_char   *var_val,
                           u_char   var_val_type,
                           size_t   var_val_len,
                           u_char   *statP,
                           oid      *name,
                           size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            /* only support version 4 */
            if (*(long *)var_val != 4)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

#endif /* end of #if (SYS_CPNT_SFLOW == TRUE) */
