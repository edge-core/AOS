#include "sys_cpnt.h"

#if (SYS_CPNT_SFLOW == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_adpt.h"
#include "sflow_pmgr.h"
#include "leaf_sflowv5.h"
#include "sflowv5.h"

#include "l_inet.h"
#include "snmp_mgr.h"
#include "stktplg_pmgr.h"
#include "sys_pmgr.h"

oid sFlow5Table_variables_oid[] = { 1,3,6,1,4,1,14706,1,1 };

void init_sFlowAgent(void)
{
    static oid sFlowVersion_oid[] = { 1,3,6,1,4,1,14706,1,1,1, 0 };
    static oid sFlowAgentAddressType_oid[] = { 1,3,6,1,4,1,14706,1,1,2, 0 };
    static oid sFlowAgentAddress_oid[] = { 1,3,6,1,4,1,14706,1,1,3, 0 };

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowVersion",
                                         get_sFlow5Version,
                                         sFlowVersion_oid,
                                         OID_LENGTH(sFlowVersion_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowAgentAddressType",
                                         get_sFlow5AgentAddressType,
                                         sFlowAgentAddressType_oid,
                                         OID_LENGTH(sFlowAgentAddressType_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("sFlowAgentAddress",
                                         get_sFlow5AgentAddress,
                                         sFlowAgentAddress_oid,
                                         OID_LENGTH(sFlowAgentAddress_oid),
                                         HANDLER_CAN_RONLY));

}

struct variable3 sFlowRcvrTable_variables[] =
{
    /* magic number, variable type, ro/rw, callback fn, L, oidsuffix
     *     (L = length of the oidsuffix)
     */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_sFlowRcvrIndex, ASN_INTEGER, RONLY, var_sFlowRcvrTable, 3, { 4, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_sFlowRcvrOwner, ASN_OCTET_STR, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 2 }},
    { LEAF_sFlowRcvrTimeout, ASN_INTEGER, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 3 }},
    { LEAF_sFlowRcvrMaximumDatagramSize, ASN_INTEGER, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 4 }},
    { LEAF_sFlowRcvrAddressType, ASN_INTEGER, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 5 }},
    { LEAF_sFlowRcvrAddress, ASN_OCTET_STR, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 6 }},
    { LEAF_sFlowRcvrPort, ASN_INTEGER, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 7 }},
    { LEAF_sFlowRcvrDatagramVersion, ASN_INTEGER, RWRITE, var_sFlowRcvrTable, 3, { 4, 1, 8 }},
};

struct variable3 sFlowFsTable_variables[] =
{
    /* magic number, variable type, ro/rw, callback fn, L, oidsuffix
     *     (L = length of the oidsuffix)
     */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_sFlowFsDataSource, ASN_OBJECT_ID, RONLY, var_sFlowFsTable, 3, { 5, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_sFlowFsInstance, ASN_INTEGER, RONLY, var_sFlowFsTable, 3, { 5, 1, 2 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_sFlowFsReceiver, ASN_INTEGER, RWRITE, var_sFlowFsTable, 3, { 5, 1, 3 }},
    { LEAF_sFlowFsPacketSamplingRate, ASN_INTEGER, RWRITE, var_sFlowFsTable, 3, { 5, 1, 4 }},
    { LEAF_sFlowFsMaximumHeaderSize, ASN_INTEGER, RWRITE, var_sFlowFsTable, 3, { 5, 1, 5 }},
};


struct variable3 sFlowCpTable_variables[] =
{
    /* magic number, variable type, ro/rw, callback fn, L, oidsuffix
     *     (L = length of the oidsuffix)
     */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_sFlowCpDataSource, ASN_OBJECT_ID, RONLY, var_sFlowCpTable, 3, { 6, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_sFlowCpInstance, ASN_INTEGER, RONLY, var_sFlowCpTable, 3, { 6, 1, 2 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_sFlowCpReceiver, ASN_INTEGER, RWRITE, var_sFlowCpTable, 3, { 6, 1, 3 }},
    { LEAF_sFlowCpInterval, ASN_INTEGER, RWRITE, var_sFlowCpTable, 3, { 6, 1, 4 }},
};

void init_sFlowTable(void)
{
    /* Register ourselves with the agent to handle our MIB tree
     */
    REGISTER_MIB("sFlowRcvrTable", sFlowRcvrTable_variables, variable3,
                 sFlow5Table_variables_oid);

    REGISTER_MIB("sFlowFsTable", sFlowFsTable_variables, variable3,
                 sFlow5Table_variables_oid);

    REGISTER_MIB("sFlowCpTable", sFlowCpTable_variables, variable3,
                 sFlow5Table_variables_oid);

}

int get_sFlow5AgentAddress(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            L_INET_AddrIp_T addr;
            UI32_T var_len = 0;
            UI32_T out_inet_type, out_preflen;

            memset(&addr, 0, sizeof(L_INET_AddrIp_T));
            if (SFLOW_PMGR_GetAgentAddress(&addr) != SFLOW_MGR_RETURN_SUCCESS)
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

            break;
        }


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int get_sFlow5AgentAddressType(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T value = 0;

            if (SFLOW_MGR_RETURN_SUCCESS !=
                SFLOW_PMGR_GetAgentAddressType(&value))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = value;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&long_return, sizeof(long_return));
            break;
        }


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int get_sFlow5Version(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0;
            STKTPLG_MGR_Switch_Info_T data;
            char prod_manufacturer[MAXSIZE_swProdManufacturer + 1];

            /* get from core layer
             */
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

            break;
        }


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

/* sFlowRcvTable
 */
#define SFLOWRCVRENTRY_INSTANCE_LEN  1

BOOL_T sFlowRcvrTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *receiver_index)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != SFLOWRCVRENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *receiver_index = compl[0];

    return TRUE;
}

/*
 * var_sFlowRcvrTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_sFlowRcvrTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    SFLOW_MGR_Receiver_T entry;
    UI32_T compc = 0;
    oid compl[SFLOWRCVRENTRY_INSTANCE_LEN] = {0};
    oid best_inst[SFLOWRCVRENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_sFlowRcvrOwner:
            *write_method = write_sFlowRcvrOwner;
            break;

        case LEAF_sFlowRcvrTimeout:
            *write_method = write_sFlowRcvrTimeout;
            break;

        case LEAF_sFlowRcvrMaximumDatagramSize:
            *write_method = write_sFlowRcvrMaximumDatagramSize;
            break;

        case LEAF_sFlowRcvrAddressType:
            *write_method = write_sFlowRcvrAddressType;
            break;

        case LEAF_sFlowRcvrAddress:
            *write_method = write_sFlowRcvrAddress;
            break;

        case LEAF_sFlowRcvrPort:
            *write_method = write_sFlowRcvrPort;
            break;

        case LEAF_sFlowRcvrDatagramVersion:
            *write_method = write_sFlowRcvrDatagramVersion;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        SFLOWRCVRENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)  /* get or set */
    {
        if (TRUE != sFlowRcvrTable_OidIndexToData(exact, compc, compl, &entry.receiver_index))
        {
            return NULL;
        }

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetReceiverEntry(&entry))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        sFlowRcvrTable_OidIndexToData(exact, compc, compl, &entry.receiver_index);

        /* Check the length of inputing index. If compc is less than instance
         * length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < SFLOWRCVRENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetReceiverEntry(&entry))
            {
                if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextReceiverEntry(&entry))
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextReceiverEntry(&entry))
            {
                return NULL;
            }
        }
    }

    /* fill with ifindex oid
    */
    best_inst[0] = entry.receiver_index;

    /* assign data to the OID index
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, SFLOWRCVRENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + SFLOWRCVRENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_sFlowRcvrIndex:
            *var_len = sizeof(long_return);
            long_return = ifindex;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_sFlowRcvrOwner:
            *var_len = strlen((char *)entry.owner_name);
            memcpy(return_buf, entry.owner_name, *var_len);
            return (u_char *) return_buf;

        case LEAF_sFlowRcvrTimeout:
            *var_len = sizeof(long_return);
            long_return = entry.timeout;
            return (u_char *) &long_return;

        case LEAF_sFlowRcvrMaximumDatagramSize:
            *var_len = sizeof(long_return);
            long_return = entry.max_datagram_size;
            return (u_char *) &long_return;

        case LEAF_sFlowRcvrAddressType:
            *var_len = sizeof(long_return);
            long_return = entry.address.type;
            return (u_char *) &long_return;

        case LEAF_sFlowRcvrAddress:
        {
            UI32_T out_inet_type, out_inet_addr_len, out_preflen;

            if (!SNMP_MGR_ConvertLInetAddrToInetAddrAndType(
                    (L_INET_Addr_T *)&entry.address,
                    &out_inet_type,
                    &out_inet_addr_len,
                    (UI8_T *)return_buf,
                    &out_preflen))
            {
                return NULL;
            }

            *var_len = out_inet_addr_len;
        }
            return (u_char *) return_buf;

        case LEAF_sFlowRcvrPort:
            *var_len = sizeof(long_return);
            long_return = entry.udp_port;
            return (u_char *) &long_return;

        case LEAF_sFlowRcvrDatagramVersion:
            *var_len = sizeof(long_return);
            long_return = entry.datagram_version;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    return NULL;
}

int write_sFlowRcvrOwner(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    char owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};

    /* get user value
     */
    strncpy(owner_name, (char *)var_val, var_val_len);
    owner_name[var_val_len] = '\0';

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len != 0 &&
               (var_val_len < SFLOW_MGR_MIN_RECEIVER_OWNER_STR_LEN ||
                var_val_len > SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (! sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverOwner(
                receiver_index, owner_name))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrTimeout(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value < SYS_ADPT_SFLOW_MIN_RECEIVER_TIMEOUT ||
                value > SYS_ADPT_SFLOW_MAX_RECEIVER_TIMEOUT)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (TRUE != sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverTimeout(
                receiver_index, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrMaximumDatagramSize(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value < SYS_ADPT_SFLOW_MIN_RECEIVER_DATAGRAM_SIZE ||
                value > SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (TRUE != sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverMaxDatagramSize(
                receiver_index, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrAddressType(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            switch (*(long *) var_val)
            {
                case VAL_sFlowRcvrAddressType_unknown:
                case VAL_sFlowRcvrAddressType_ipv4:
                case VAL_sFlowRcvrAddressType_ipv6:
                case VAL_sFlowRcvrAddressType_ipv4z:
                case VAL_sFlowRcvrAddressType_ipv6z:
                case VAL_sFlowRcvrAddressType_dns:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            /* We do nothing. sFlowRcvrAddressType changes with sFlowRcvrAddress.
             */
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrAddress(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len < MINSIZE_sFlowRcvrAddress ||
                var_val_len > MAXSIZE_sFlowRcvrAddress)
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
            L_INET_AddrIp_T addr;
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (TRUE != sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
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

            if (TRUE != SNMP_MGR_ConvertInetAddrAndTypeToLInetAddr(
                addr.type, var_val_len, var_val, 0, (L_INET_Addr_T *)&addr))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverDestination(
                receiver_index, &addr))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrPort(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value < SFLOW_MGR_MIN_RECEIVER_SOCK_PORT ||
                value > SFLOW_MGR_MAX_RECEIVER_SOCK_PORT)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (TRUE != sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverSockPort(
                receiver_index, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowRcvrDatagramVersion(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value < SFLOW_MGR_MIN_RECEIVER_DATAGRAM_VERSION ||
                value > SFLOW_MGR_MAX_RECEIVER_DATAGRAM_VERSION)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T receiver_index = 0;
            UI32_T oid_name_length = 12;

            if (TRUE != sFlowRcvrTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &receiver_index))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetReceiverDatagramVersion(
                receiver_index, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

/* sFlowFsTable
 */
#define SFLOWENTRY_INSTANCE_LEN 12
#define IFINDEX_LEN             11

static oid sFlowDataSource_prefix_ifindex[IFINDEX_LEN] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};

BOOL_T sFlowFsTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *ifindex, UI32_T *instance_id)
{
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
        diff = (long)(compc - SFLOWENTRY_INSTANCE_LEN);
    }

    /* get or write
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != SFLOWENTRY_INSTANCE_LEN)
        {
            return FALSE;
        }
    }

    if (diff == 0)
    {
        *ifindex = (UI32_T)compl[IFINDEX_LEN - 1];
        *instance_id = (UI32_T)compl[SFLOWENTRY_INSTANCE_LEN - 1];
    }
    else
    {
        *ifindex = 0;
        *instance_id = 0;
    }

    return TRUE;
}

/*
 * var_sFlowFsTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_sFlowFsTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    SFLOW_MGR_Sampling_T entry;
    UI32_T compc = 0;
    oid compl[SYS_ADPT_MAX_OID_COUNT] = {0};
    oid best_inst[SFLOWENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_sFlowFsReceiver:
            *write_method = write_sFlowFsReceiver;
            break;

        case LEAF_sFlowFsPacketSamplingRate:
            *write_method = write_sFlowFsPacketSamplingRate;
            break;

        case LEAF_sFlowFsMaximumHeaderSize:
            *write_method = write_sFlowFsMaximumHeaderSize;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        SFLOWENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)  /* get or set */
    {
        if (TRUE != sFlowFsTable_OidIndexToData(exact, compc, compl,
            &entry.ifindex, &entry.instance_id))
        {
            return NULL;
        }

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetSamplingEntry(&entry))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        sFlowFsTable_OidIndexToData(exact, compc, compl,
            &entry.ifindex, &entry.instance_id);

        /* Check the length of inputing index. If compc is less than instance
         * length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < SFLOWRCVRENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetSamplingEntry(&entry))
            {
                if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextSamplingEntry(&entry))
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextSamplingEntry(&entry))
            {
                return NULL;
            }
        }
    }

    /* fill with ifindex oid
     */
    memcpy(best_inst, sFlowDataSource_prefix_ifindex, sizeof(sFlowDataSource_prefix_ifindex));
    best_inst[IFINDEX_LEN - 1] = entry.ifindex;
    best_inst[SFLOWENTRY_INSTANCE_LEN - 1] = entry.instance_id;

    /* assign data to the OID index
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, SFLOWENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + SFLOWENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_sFlowFsDataSource:
            *var_len = IFINDEX_LEN * sizeof(oid);
            memcpy(oid_return, best_inst, *var_len);
            return (u_char *)oid_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_sFlowFsInstance:
            *var_len = sizeof(long_return);
            long_return = entry.instance_id;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_sFlowFsReceiver:
            *var_len = sizeof(long_return);
            long_return = entry.receiver_index;
            return (u_char *) &long_return;

        case LEAF_sFlowFsPacketSamplingRate:
            *var_len = sizeof(long_return);
            long_return = entry.sampling_rate;
            return (u_char *) &long_return;

        case LEAF_sFlowFsMaximumHeaderSize:
            *var_len = sizeof(long_return);
            long_return = entry.max_header_size;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_sFlowFsReceiver(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value != 0 &&
               (value < SFLOW_MGR_MIN_RECEIVER_INDEX ||
                value > SFLOW_MGR_MAX_RECEIVER_INDEX))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T ifindex = 0;
            UI32_T instance_id = 0;

            if (TRUE != sFlowFsTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &ifindex, &instance_id))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetSamplingReceiverIndex(ifindex, instance_id, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowFsPacketSamplingRate(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value != SFLOW_MGR_SAMPLING_RATE_DISABLE &&
               (value < SYS_ADPT_SFLOW_MIN_SAMPLING_RATE ||
                value > SYS_ADPT_SFLOW_MAX_SAMPLING_RATE))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T ifindex = 0;
            UI32_T instance_id = 0;

            if (TRUE != sFlowFsTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &ifindex, &instance_id))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetSamplingRate(ifindex, instance_id, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_sFlowFsMaximumHeaderSize(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value < SYS_ADPT_SFLOW_MIN_SAMPLING_HEADER_SIZE ||
                value > SYS_ADPT_SFLOW_MAX_SAMPLING_HEADER_SIZE)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T ifindex = 0;
            UI32_T instance_id = 0;

            if (TRUE != sFlowFsTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &ifindex, &instance_id))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetSamplingMaxHeaderSize(
                ifindex, instance_id, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

/* sFlowCPTable
 */
 #define sFlowCpTable_OidIndexToData  sFlowFsTable_OidIndexToData

/*
 * var_sFlowCpTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_sFlowCpTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    SFLOW_MGR_Polling_T entry;
    UI32_T compc = 0;
    oid compl[SYS_ADPT_MAX_OID_COUNT] = {0};
    oid best_inst[SFLOWENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_sFlowCpReceiver:
            *write_method = write_sFlowCpReceiver;
            break;

        case LEAF_sFlowCpInterval:
            *write_method = write_sFlowCpInterval;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        SFLOWENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)  /* get or set */
    {
        if (TRUE != sFlowCpTable_OidIndexToData(exact, compc, compl,
            &entry.ifindex, &entry.instance_id))
        {
            return NULL;
        }

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetPollingEntry(&entry))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        sFlowCpTable_OidIndexToData(exact, compc, compl,
            &entry.ifindex, &entry.instance_id);

        /* Check the length of inputing index. If compc is less than instance
         * length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < SFLOWRCVRENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetPollingEntry(&entry))
            {
                if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextPollingEntry(&entry))
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_GetNextPollingEntry(&entry))
            {
                return NULL;
            }
        }
    }

    /* fill with ifindex oid
     */
    memcpy(best_inst, sFlowDataSource_prefix_ifindex, sizeof(sFlowDataSource_prefix_ifindex));
    best_inst[IFINDEX_LEN - 1] = entry.ifindex;
    best_inst[SFLOWENTRY_INSTANCE_LEN - 1] = entry.instance_id;

    /* assign data to the OID index
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, SFLOWENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + SFLOWENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_sFlowCpDataSource:
            *var_len = IFINDEX_LEN * sizeof(oid);
            memcpy(oid_return, best_inst, *var_len);
            return (u_char *)oid_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_sFlowCpInstance:
            *var_len = sizeof(long_return);
            long_return = entry.instance_id;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_sFlowCpReceiver:
            *var_len = sizeof(long_return);
            long_return = entry.receiver_index;
            return (u_char *) &long_return;

        case LEAF_sFlowCpInterval:
            *var_len = sizeof(long_return);
            long_return = entry.polling_interval;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    return NULL;
}

int write_sFlowCpReceiver(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value != 0 &&
               (value < SFLOW_MGR_MIN_RECEIVER_INDEX ||
                value > SFLOW_MGR_MAX_RECEIVER_INDEX))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T ifindex = 0;
            UI32_T instance_id = 0;

            if (TRUE != sFlowCpTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &ifindex, &instance_id))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetPollingReceiverIndex(ifindex, instance_id, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

int write_sFlowCpInterval(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI32_T value = 0;

    /* get user value
     */
    value = *(long *)var_val;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (value != SFLOW_MGR_POLLING_INTERVAL_DISABLE &&
               (value < SYS_ADPT_SFLOW_MIN_POLLING_INTERVAL ||
                value > SYS_ADPT_SFLOW_MAX_POLLING_INTERVAL))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 12;
            UI32_T ifindex = 0;
            UI32_T instance_id = 0;

            if (TRUE != sFlowCpTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &ifindex, &instance_id))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_PMGR_SetPollingInterval(ifindex, instance_id, value))
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
