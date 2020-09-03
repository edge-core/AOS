#include "sys_cpnt.h"
#if (SYS_CPNT_LLDP_MED == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "l_stdlib.h"
#include "ansi_tia_1057.h"
#include "leaf_ansi_tia_1057.h"
#include "lldp_type.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "snmp_mgr.h"

/** Initializes the lldpXMedConfig module */
void
init_lldpXMedConfig(void)
{
    static oid lldpXMedLocDeviceClass_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,1,1,0 };
    static oid lldpXMedFastStartRepeatCount_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,1,3,0 };

    DEBUGMSGTL(("lldpXMedConfig", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocDeviceClass",
                                         get_lldpXMedLocDeviceClass,
                                         lldpXMedLocDeviceClass_oid,
                                         OID_LENGTH(lldpXMedLocDeviceClass_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpXMedFastStartRepeatCount",
                               do_lldpXMedFastStartRepeatCount,
                               lldpXMedFastStartRepeatCount_oid,
                               OID_LENGTH(lldpXMedFastStartRepeatCount_oid),
                               HANDLER_CAN_RWRITE));
}

int
get_lldpXMedLocDeviceClass(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedConfig_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_POM_GetXMedConfigEntry(&entry) == TRUE)
            {
                long_return = entry.lldp_xmed_loc_device_class;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_lldpXMedFastStartRepeatCount(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedConfig_T entry;

            if (LLDP_POM_GetXMedConfigEntry(&entry) == TRUE)
            {
                long_return = entry.lldp_xmed_fast_start_repeat_count;
                snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_UNSIGNED)
            {
                netsnmp_set_request_error(reqinfo, requests,
                                          SNMP_ERR_WRONGTYPE);
            }

            break;

        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer < MIN_lldpXMedFastStartRepeatCount ||
                *requests->requestvb->val.integer > MAX_lldpXMedFastStartRepeatCount)
            {
               netsnmp_set_request_error(reqinfo, requests,
                                         SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetXMedFastStartRepeatCount(*requests->requestvb->val.integer) != TRUE)
            {
                netsnmp_set_request_error(reqinfo, requests,
                                          SNMP_ERR_COMMITFAILED);
            }
        }
            break;

        case MODE_SET_COMMIT:
            break;

        case MODE_SET_UNDO:
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

oid lldpXMedPortConfigTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,1 };
/*
 * variable3 lldpXMedPortConfigTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedPortConfigTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedPortCapSupported,  ASN_OCTET_STR,  RONLY,   var_lldpXMedPortConfigTable, 3,  { 2, 1, 1 }},
{LEAF_lldpXMedPortConfigTLVsTxEnable,  ASN_OCTET_STR,  RWRITE,  var_lldpXMedPortConfigTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXMedPortConfigNotifEnable,  ASN_INTEGER,  RWRITE,  var_lldpXMedPortConfigTable, 3,  { 2, 1, 3 }},
};

/** Initializes the lldpXMedPortConfigTable module */
void
init_lldpXMedPortConfigTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedPortConfigTable", lldpXMedPortConfigTable_variables, variable3,
                 lldpXMedPortConfigTable_variables_oid);
}

#define LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN  1

BOOL_T lldpXMedPortConfigTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpPortConfigPortNum)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpPortConfigPortNum = compl[0];

    return TRUE;
}

/*
 * var_lldpXMedPortConfigTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedPortConfigTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedPortConfigEntry_T entry;

    switch (vp->magic)
    {
        case LEAF_lldpXMedPortConfigTLVsTxEnable:
            *write_method = write_lldpXMedPortConfigTLVsTxEnable;
            break;

        case LEAF_lldpXMedPortConfigNotifEnable:
            *write_method = write_lldpXMedPortConfigNotifEnable;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedPortConfigTable_OidIndexToData(exact, compc, compl, &entry.lport) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_PMGR_GetXMedPortConfigEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedPortConfigTable_OidIndexToData(exact, compc, compl, &entry.lport);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_PMGR_GetXMedPortConfigEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedPortConfigEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedPortConfigEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.lport;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDPORTCONFIGENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedPortCapSupported:
            *var_len = SIZE_lldpXMedPortCapSupported;
            SNMP_MGR_BitsFromCoreToSnmp(&entry.lldp_xmed_port_cap_supported, return_buf, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedPortConfigTLVsTxEnable:
            *var_len = SIZE_lldpXMedPortConfigTLVsTxEnable;
            SNMP_MGR_BitsFromCoreToSnmp(&entry.lldp_xmed_port_tlvs_tx_enabled, return_buf, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedPortConfigNotifEnable:
            *var_len = 4;
            long_return = entry.lldp_xmed_port_notif_enabled;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int
write_lldpXMedPortConfigTLVsTxEnable(int      action,
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
            if (var_val_len != SIZE_lldpXMedPortConfigTLVsTxEnable)
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
            UI32_T oid_name_length = 14;
            UI8_T byte_buffer[SIZE_lldpXMedPortConfigTLVsTxEnable] = {0};
            LLDP_MGR_XMedPortConfigEntry_T entry;
            UI16_T tlvs_tx_enabled;

            if (lldpXMedPortConfigTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]),  &entry.lport) == FALSE)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (LLDP_PMGR_GetXMedPortConfigEntry(&entry) != TRUE)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            SNMP_MGR_BitsFromSnmpToCore(byte_buffer, var_val, var_val_len);
            tlvs_tx_enabled = *(UI16_T *)byte_buffer;

            if ((~entry.lldp_xmed_port_cap_supported) & tlvs_tx_enabled)
            {
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            if (LLDP_PMGR_SetXMedPortConfigTlvsTx(entry.lport, tlvs_tx_enabled) != TRUE)
            {
                return SNMP_ERR_COMMITFAILED;
            }
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
write_lldpXMedPortConfigNotifEnable(int      action,
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
                case VAL_lldpXMedPortConfigNotifEnable_true:
                    break;

                case VAL_lldpXMedPortConfigNotifEnable_false:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 14;
            I32_T value = 0;
            UI32_T lldpPortConfigPortNum = 0;

            if (lldpXMedPortConfigTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]),  &lldpPortConfigPortNum) == FALSE)
                return SNMP_ERR_COMMITFAILED;

            value = *(long *)var_val;

            if (LLDP_PMGR_SetXMedPortConfigNotifEnabled( lldpPortConfigPortNum, (UI8_T)value) != TRUE)
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

oid lldpXMedLocMediaPolicyTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2 };
/*
 * variable3 lldpXMedLocMediaPolicyTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedLocMediaPolicyTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXMedLocMediaPolicyAppType,  ASN_OCTET_STR,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 1 }},
#endif

{LEAF_lldpXMedLocMediaPolicyVlanID,  ASN_INTEGER,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 2 }},
{LEAF_lldpXMedLocMediaPolicyPriority,  ASN_INTEGER,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 3 }},
{LEAF_lldpXMedLocMediaPolicyDscp,  ASN_INTEGER,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 4 }},
{LEAF_lldpXMedLocMediaPolicyUnknown,  ASN_INTEGER,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 5 }},
{LEAF_lldpXMedLocMediaPolicyTagged,  ASN_INTEGER,  RONLY,   var_lldpXMedLocMediaPolicyTable, 3,  { 1, 1, 6 }},
};

/** Initializes the lldpXMedLocMediaPolicyTable module */
void
init_lldpXMedLocMediaPolicyTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedLocMediaPolicyTable", lldpXMedLocMediaPolicyTable_variables, variable3,
                 lldpXMedLocMediaPolicyTable_variables_oid);
}

#define LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN 2

BOOL_T lldpXMedLocMediaPolicyTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpLocPortNum, UI8_T *lldpXMedLocMediaPolicyAppType)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if ( compc < 0 || compc > LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN) /* the dynamic size index */
        {
            return FALSE;
        }
    }

    *lldpLocPortNum = compl[0];
    *lldpXMedLocMediaPolicyAppType = compl[1];
    //memcpy(&lldpXMedLocMediaPolicyAppType[0], &compl[1], SIZE_lldpXMedLocMediaPolicyAppType);

    return TRUE;
}

/*
 * var_lldpXMedLocMediaPolicyTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedLocMediaPolicyTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedLocMediaPolicyEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedLocMediaPolicyTable_OidIndexToData(exact, compc, compl, &entry.lport, &entry.app_type) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_PMGR_GetXMedLocMediaPolicyEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedLocMediaPolicyTable_OidIndexToData(exact, compc, compl, &entry.lport, &entry.app_type);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_PMGR_GetXMedLocMediaPolicyEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedLocMediaPolicyEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedLocMediaPolicyEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.lport;
    best_inst[1] = entry.app_type;
    //memcpy(&best_inst[1], entry.app_type, SIZE_lldpXMedLocMediaPolicyAppType);
    memcpy(name + vp->namelen, best_inst,
        LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDLOCMEDIAPOLICYENTRY_INSTANCE_LEN;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_lldpXMedLocMediaPolicyAppType:
            *var_len = SIZE_lldpXMedLocMediaPolicyAppType;
            *return_buf = entry.app_type & 0xff;
            //memcpy(return_buf, entry.app_type, *var_len);
            return (u_char*)return_buf;

#endif
        case LEAF_lldpXMedLocMediaPolicyVlanID:
            *var_len = 4;
            long_return = entry.vid;
            return (u_char *) &long_return;

        case LEAF_lldpXMedLocMediaPolicyPriority:
            *var_len = 4;
            long_return = entry.priority;
            return (u_char *) &long_return;

        case LEAF_lldpXMedLocMediaPolicyDscp:
            *var_len = 4;
            long_return = entry.dscp;
            return (u_char *) &long_return;

        case LEAF_lldpXMedLocMediaPolicyUnknown:
            *var_len = 4;
            long_return = entry.unknown;
            return (u_char *) &long_return;

        case LEAF_lldpXMedLocMediaPolicyTagged:
            *var_len = 4;
            long_return = entry.tagged;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

/** Initializes the lldpXMedLocalData module */
void
init_lldpXMedLocalData(void)
{
    static oid lldpXMedLocHardwareRev_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,2,0 };
    static oid lldpXMedLocFirmwareRev_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,3,0 };
    static oid lldpXMedLocSoftwareRev_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,4,0 };
    static oid lldpXMedLocSerialNum_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,5,0 };
    static oid lldpXMedLocMfgName_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,6,0 };
    static oid lldpXMedLocModelName_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,7,0 };
    static oid lldpXMedLocAssetID_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,8,0 };

    DEBUGMSGTL(("lldpXMedLocalData", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocHardwareRev",
                                         get_lldpXMedLocHardwareRev,
                                         lldpXMedLocHardwareRev_oid,
                                         OID_LENGTH(lldpXMedLocHardwareRev_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocFirmwareRev",
                                         get_lldpXMedLocFirmwareRev,
                                         lldpXMedLocFirmwareRev_oid,
                                         OID_LENGTH(lldpXMedLocFirmwareRev_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocSoftwareRev",
                                         get_lldpXMedLocSoftwareRev,
                                         lldpXMedLocSoftwareRev_oid,
                                         OID_LENGTH(lldpXMedLocSoftwareRev_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocSerialNum",
                                         get_lldpXMedLocSerialNum,
                                         lldpXMedLocSerialNum_oid,
                                         OID_LENGTH(lldpXMedLocSerialNum_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocMfgName",
                                         get_lldpXMedLocMfgName,
                                         lldpXMedLocMfgName_oid,
                                         OID_LENGTH(lldpXMedLocMfgName_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocModelName",
                                         get_lldpXMedLocModelName,
                                         lldpXMedLocModelName_oid,
                                         OID_LENGTH(lldpXMedLocModelName_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocAssetID",
                                         get_lldpXMedLocAssetID,
                                         lldpXMedLocAssetID_oid,
                                         OID_LENGTH(lldpXMedLocAssetID_oid),
                                         HANDLER_CAN_RONLY));
}

int
get_lldpXMedLocHardwareRev(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_hardware_rev_len;
                memcpy(return_buf, entry.loc_hardware_rev, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocFirmwareRev(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_firmware_rev_len;
                memcpy(return_buf, entry.loc_firmware_rev, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocSoftwareRev(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_software_rev_len;
                memcpy(return_buf, entry.loc_software_rev, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocSerialNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_serial_num_len;
                memcpy(return_buf, entry.loc_serial_num, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocMfgName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_mfg_name_len;
                memcpy(return_buf, entry.loc_mfg_name, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocModelName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_model_name_len;
                memcpy(return_buf, entry.loc_model_name, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocAssetID(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocInventory_T entry;
            UI32_T var_len = 0;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocInventoryEntry(&entry) == TRUE)
            {
                var_len = entry.loc_asset_id_len;
                memcpy(return_buf, entry.loc_asset_id, var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                         (u_char *)return_buf, var_len);
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

oid lldpXMedLocLocationTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2 };
/*
 * variable3 lldpXMedLocLocationTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedLocLocationTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXMedLocLocationSubtype,  ASN_INTEGER,  RONLY,   var_lldpXMedLocLocationTable, 3,  { 9, 1, 1 }},
#endif

{LEAF_lldpXMedLocLocationInfo,  ASN_OCTET_STR,  RWRITE,  var_lldpXMedLocLocationTable, 3,  { 9, 1, 2 }},
};

/** Initializes the lldpXMedLocLocationTable module */
void
init_lldpXMedLocLocationTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedLocLocationTable", lldpXMedLocLocationTable_variables, variable3,
                 lldpXMedLocLocationTable_variables_oid);
}

#define LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN  2

BOOL_T lldpXMedLocLocationTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpLocPortNum, UI8_T *lldpXMedLocLocationSubtype)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpLocPortNum = compl[0];
    *lldpXMedLocLocationSubtype = compl[1];

    return TRUE;
}

/*
 * var_lldpXMedLocLocationTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedLocLocationTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedLocLocationEntry_T entry;

    switch (vp->magic)
    {
        case LEAF_lldpXMedLocLocationInfo:
            *write_method = write_lldpXMedLocLocationInfo;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedLocLocationTable_OidIndexToData(exact, compc, compl, &entry.lport, &entry.location_subtype) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_PMGR_GetXMedLocLocationEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedLocLocationTable_OidIndexToData(exact, compc, compl, &entry.lport, &entry.location_subtype);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_PMGR_GetXMedLocLocationEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedLocLocationEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedLocLocationEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.lport;
    best_inst[1] = entry.location_subtype;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDLOCLOCATIONENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_lldpXMedLocLocationSubtype:
            *var_len = 4;
            long_return = entry.location_subtype;
            return (u_char *) &long_return;

#endif
        case LEAF_lldpXMedLocLocationInfo:
            *var_len  =  entry.location_info_len;
            memcpy(return_buf, entry.location_info, *var_len);
            return (u_char*)return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int
write_lldpXMedLocLocationInfo(int      action,
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
            if (var_val_len < MINSIZE_lldpXMedLocLocationInfo ||
                var_val_len > MAXSIZE_lldpXMedLocLocationInfo)
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
            UI8_T byte_buffer[MAXSIZE_lldpXMedLocLocationInfo + 1] = {0};
            UI32_T oid_name_length = 14;
            UI32_T lldpLocPortNum = 0;
            UI8_T lldpXMedLocLocationSubtype = 0;
            LLDP_MGR_XMedLocLocationEntry_T entry;

            if (lldpXMedLocLocationTable_OidIndexToData(TRUE, name_len - oid_name_length,
                 &(name[oid_name_length]), &lldpLocPortNum, &lldpXMedLocLocationSubtype) == FALSE)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            memset(&entry, 0, sizeof(entry));
            memcpy(byte_buffer, var_val, var_val_len);
            byte_buffer[var_val_len] = '\0';
            entry.lport = lldpLocPortNum;
            entry.location_subtype = lldpXMedLocLocationSubtype;
            entry.location_info_len = var_val_len+1;
            memcpy(entry.location_info, byte_buffer, entry.location_info_len);

            if (LLDP_PMGR_SetXMedLocLocationEntry( &entry) != TRUE)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

/** Initializes the lldpXMedLocXPoEData module */
void
init_lldpXMedLocXPoEData(void)
{
    static oid lldpXMedLocXPoEDeviceType_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,10,0 };
    static oid lldpXMedLocXPoEPSEPowerSource_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,12,0 };
    static oid lldpXMedLocXPoEPDPowerReq_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,13,0 };
    static oid lldpXMedLocXPoEPDPowerSource_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,14,0 };
    static oid lldpXMedLocXPoEPDPowerPriority_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2,15,0 };

    DEBUGMSGTL(("lldpXMedLocXPoEData", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocXPoEDeviceType",
                                         get_lldpXMedLocXPoEDeviceType,
                                         lldpXMedLocXPoEDeviceType_oid,
                                         OID_LENGTH(lldpXMedLocXPoEDeviceType_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocXPoEPSEPowerSource",
                                         get_lldpXMedLocXPoEPSEPowerSource,
                                         lldpXMedLocXPoEPSEPowerSource_oid,
                                         OID_LENGTH(lldpXMedLocXPoEPSEPowerSource_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocXPoEPDPowerReq",
                                         get_lldpXMedLocXPoEPDPowerReq,
                                         lldpXMedLocXPoEPDPowerReq_oid,
                                         OID_LENGTH(lldpXMedLocXPoEPDPowerReq_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocXPoEPDPowerSource",
                                         get_lldpXMedLocXPoEPDPowerSource,
                                         lldpXMedLocXPoEPDPowerSource_oid,
                                         OID_LENGTH(lldpXMedLocXPoEPDPowerSource_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpXMedLocXPoEPDPowerPriority",
                                         get_lldpXMedLocXPoEPDPowerPriority,
                                         lldpXMedLocXPoEPDPowerPriority_oid,
                                         OID_LENGTH(lldpXMedLocXPoEPDPowerPriority_oid),
                                         HANDLER_CAN_RONLY));
}

int
get_lldpXMedLocXPoEDeviceType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocXPoeEntry_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocXPoeEntry(&entry) == TRUE)
            {
                long_return = entry.device_type;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocXPoEPSEPowerSource(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocXPoeEntry_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocXPoeEntry(&entry) == TRUE)
            {
                long_return = entry.pse_power_source;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocXPoEPDPowerReq(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocXPoeEntry_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocXPoeEntry(&entry) == TRUE)
            {
                long_return = entry.pd_power_req;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocXPoEPDPowerSource(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocXPoeEntry_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocXPoeEntry(&entry) == TRUE)
            {
                long_return = entry.pd_power_source;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_lldpXMedLocXPoEPDPowerPriority(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            LLDP_MGR_XMedLocXPoeEntry_T entry;

            memset(&entry, 0, sizeof(entry));

            if (LLDP_PMGR_GetXMedLocXPoeEntry(&entry) == TRUE)
            {
                long_return = entry.pd_power_priority;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                         (u_char *)&long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

oid lldpXMedLocXPoEPSEPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,2 };
/*
 * variable3 lldpXMedLocXPoEPSEPortTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedLocXPoEPSEPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedLocXPoEPSEPortPowerAv,  ASN_GAUGE,  RONLY,   var_lldpXMedLocXPoEPSEPortTable, 3,  { 11, 1, 1 }},
{LEAF_lldpXMedLocXPoEPSEPortPDPriority,  ASN_INTEGER,  RONLY,   var_lldpXMedLocXPoEPSEPortTable, 3,  { 11, 1, 2 }},
};

/** Initializes the lldpXMedLocXPoEPSEPortTable module */
void
init_lldpXMedLocXPoEPSEPortTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedLocXPoEPSEPortTable", lldpXMedLocXPoEPSEPortTable_variables, variable3,
                 lldpXMedLocXPoEPSEPortTable_variables_oid);
}

#define LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN  1

BOOL_T lldpXMedLocXPoEPSEPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpLocPortNum)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpLocPortNum = compl[0];

    return TRUE;
}

/*
 * var_lldpXMedLocXPoEPSEPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedLocXPoEPSEPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedLocXPoePsePortEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedLocXPoEPSEPortTable_OidIndexToData(exact, compc, compl, &entry.lport) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_PMGR_GetXMedLocXPoePsePortEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedLocXPoEPSEPortTable_OidIndexToData(exact, compc, compl, &entry.lport);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_PMGR_GetXMedLocXPoePsePortEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedLocXPoePsePortEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedLocXPoePsePortEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.lport;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDLOCXPOEPSEPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedLocXPoEPSEPortPowerAv:
            *var_len = 4;
            long_return = entry.power_av;
            return (u_char *) &long_return;

        case LEAF_lldpXMedLocXPoEPSEPortPDPriority:
            *var_len = 4;
            long_return = entry.pd_priority;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemCapabilitiesTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemCapabilitiesTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemCapabilitiesTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedRemCapSupported,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemCapabilitiesTable, 3,  { 1, 1, 1 }},
{LEAF_lldpXMedRemCapCurrent,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemCapabilitiesTable, 3,  { 1, 1, 2 }},
{LEAF_lldpXMedRemDeviceClass,  ASN_INTEGER,  RONLY,   var_lldpXMedRemCapabilitiesTable, 3,  { 1, 1, 3 }},
};

/** Initializes the lldpXMedRemCapabilitiesTable module */
void
init_lldpXMedRemCapabilitiesTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemCapabilitiesTable", lldpXMedRemCapabilitiesTable_variables, variable3,
                 lldpXMedRemCapabilitiesTable_variables_oid);
}

#define LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN  3

BOOL_T lldpXMedRemCapabilitiesTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];

    return TRUE;
}

/*
 * var_lldpXMedRemCapabilitiesTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemCapabilitiesTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemCapEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemCapabilitiesTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemCapEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemCapabilitiesTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemCapEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemCapEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemCapEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMCAPABILITIESENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedRemCapSupported:
            *var_len = SIZE_lldpXMedRemCapSupported;
            SNMP_MGR_BitsFromCoreToSnmp(&entry.rem_cap_supported, return_buf, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemCapCurrent:
            *var_len = SIZE_lldpXMedRemCapCurrent;
            SNMP_MGR_BitsFromCoreToSnmp(&entry.rem_cap_current, return_buf, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemDeviceClass:
            *var_len = 4;
            long_return = entry.rem_device_class;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemMediaPolicyTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemMediaPolicyTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemMediaPolicyTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXMedRemMediaPolicyAppType,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 1 }},
#endif

{LEAF_lldpXMedRemMediaPolicyVlanID,  ASN_INTEGER,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXMedRemMediaPolicyPriority,  ASN_INTEGER,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 3 }},
{LEAF_lldpXMedRemMediaPolicyDscp,  ASN_INTEGER,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 4 }},
{LEAF_lldpXMedRemMediaPolicyUnknown,  ASN_INTEGER,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 5 }},
{LEAF_lldpXMedRemMediaPolicyTagged,  ASN_INTEGER,  RONLY,   var_lldpXMedRemMediaPolicyTable, 3,  { 2, 1, 6 }},
};

/** Initializes the lldpXMedRemMediaPolicyTable module */
void
init_lldpXMedRemMediaPolicyTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemMediaPolicyTable", lldpXMedRemMediaPolicyTable_variables, variable3,
                 lldpXMedRemMediaPolicyTable_variables_oid);
}

#define LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN 4

BOOL_T lldpXMedRemMediaPolicyTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex, UI8_T *lldpXMedRemMediaPolicyAppType)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc < 0  || compc > LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN) /* the dynamic size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];
    *lldpXMedRemMediaPolicyAppType = compl[3];

    return TRUE;
}

/*
 * var_lldpXMedRemMediaPolicyTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemMediaPolicyTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemMediaPolicyEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemMediaPolicyTable_OidIndexToData(exact, compc, compl,
             &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_app_type) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemMediaPolicyEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemMediaPolicyTable_OidIndexToData(exact, compc, compl,
             &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_app_type);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemMediaPolicyEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemMediaPolicyEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemMediaPolicyEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    best_inst[3] = entry.rem_app_type;
    memcpy(name + vp->namelen, best_inst,
         LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMMEDIAPOLICYENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_lldpXMedRemMediaPolicyAppType:
            *var_len = SIZE_lldpXMedRemMediaPolicyAppType;
            *return_buf = entry.rem_app_type & 0xff;
            //memcpy(return_buf, entry.rem_app_type, *var_len);
            return (u_char*)return_buf;

#endif
        case LEAF_lldpXMedRemMediaPolicyVlanID:
            *var_len = 4;
            long_return = entry.rem_vid;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemMediaPolicyPriority:
            *var_len = 4;
            long_return = entry.rem_priority;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemMediaPolicyDscp:
            *var_len = 4;
            long_return = entry.rem_dscp;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemMediaPolicyUnknown:
            *var_len = 4;
            long_return = entry.rem_unknown;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemMediaPolicyTagged:
            *var_len = 4;
            long_return = entry.rem_tagged;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemInventoryTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemInventoryTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemInventoryTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedRemHardwareRev,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 1 }},
{LEAF_lldpXMedRemFirmwareRev,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 2 }},
{LEAF_lldpXMedRemSoftwareRev,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 3 }},
{LEAF_lldpXMedRemSerialNum,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 4 }},
{LEAF_lldpXMedRemMfgName,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 5 }},
{LEAF_lldpXMedRemModelName,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 6 }},
{LEAF_lldpXMedRemAssetID,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemInventoryTable, 3,  { 3, 1, 7 }},
};

/** Initializes the lldpXMedRemInventoryTable module */
void
init_lldpXMedRemInventoryTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemInventoryTable", lldpXMedRemInventoryTable_variables, variable3,
                 lldpXMedRemInventoryTable_variables_oid);
}

#define LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN  3

BOOL_T lldpXMedRemInventoryTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];

    return TRUE;
}

/*
 * var_lldpXMedRemInventoryTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemInventoryTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemInventoryEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemInventoryTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemInventoryEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemInventoryTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemInventoryEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemInventoryEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemInventoryEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMINVENTORYENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedRemHardwareRev:
            *var_len  =  entry.rem_hardware_rev_len;
            memcpy(return_buf, entry.rem_hardware_rev, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemFirmwareRev:
            *var_len  = entry.rem_firmware_rev_len;
            memcpy(return_buf, entry.rem_firmware_rev, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemSoftwareRev:
            *var_len  = entry.rem_software_rev_len;
            memcpy(return_buf, entry.rem_software_rev, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemSerialNum:
            *var_len  = entry.rem_serial_num_len;
            memcpy(return_buf, entry.rem_serial_num, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemMfgName:
            *var_len  = entry.rem_mfg_name_len;
            memcpy(return_buf, entry.rem_mfg_name, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemModelName:
            *var_len  = entry.rem_model_name_len;
            memcpy(return_buf, entry.rem_model_name, *var_len);
            return (u_char*)return_buf;

        case LEAF_lldpXMedRemAssetID:
            *var_len  = entry.rem_asset_id_len;
            memcpy(return_buf, entry.rem_asset_id, *var_len);
            return (u_char*)return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemLocationTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemLocationTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemLocationTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXMedRemLocationSubtype,  ASN_INTEGER,  RONLY,   var_lldpXMedRemLocationTable, 3,  { 4, 1, 1 }},
#endif

{LEAF_lldpXMedRemLocationInfo,  ASN_OCTET_STR,  RONLY,   var_lldpXMedRemLocationTable, 3,  { 4, 1, 2 }},
};

/** Initializes the lldpXMedRemLocationTable module */
void
init_lldpXMedRemLocationTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemLocationTable", lldpXMedRemLocationTable_variables, variable3,
                 lldpXMedRemLocationTable_variables_oid);
}

#define LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN  4

BOOL_T lldpXMedRemLocationTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex, UI8_T *lldpXMedRemLocationSubtype)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];
    *lldpXMedRemLocationSubtype = compl[3];

    return TRUE;
}

/*
 * var_lldpXMedRemLocationTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemLocationTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemLocationEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemLocationTable_OidIndexToData(exact, compc, compl,
             &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_location_subtype) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemLocationEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemLocationTable_OidIndexToData(exact, compc, compl,
            &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_location_subtype);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemLocationEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemLocationEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemLocationEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    best_inst[3] = entry.rem_location_subtype;

    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMLOCATIONENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_lldpXMedRemLocationSubtype:
            *var_len = 4;
            long_return = entry.rem_location_subtype;
            return (u_char *) &long_return;

#endif
        case LEAF_lldpXMedRemLocationInfo:
            *var_len  = entry.rem_location_info_len;
            memcpy(return_buf, entry.rem_location_info, *var_len);
            return (u_char*)return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemXPoETable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemXPoETable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemXPoETable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedRemXPoEDeviceType,  ASN_INTEGER,  RONLY,   var_lldpXMedRemXPoETable, 3,  { 5, 1, 1 }},
};

/** Initializes the lldpXMedRemXPoETable module */
void
init_lldpXMedRemXPoETable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemXPoETable", lldpXMedRemXPoETable_variables, variable3,
                 lldpXMedRemXPoETable_variables_oid);
}

#define LLDPXMEDREMXPOEENTRY_INSTANCE_LEN  3

BOOL_T lldpXMedRemXPoETable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMXPOEENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];

    return TRUE;
}

/*
 * var_lldpXMedRemXPoETable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemXPoETable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMXPOEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMXPOEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemPoeEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMXPOEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemXPoETable_OidIndexToData(exact, compc, compl,
            &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemPoeEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemXPoETable_OidIndexToData(exact, compc, compl,&entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemPoeEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemPoeEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemPoeEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMXPOEENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMXPOEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedRemXPoEDeviceType:
            *var_len = 4;
            long_return = entry.rem_poe_device_type;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemXPoEPSETable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemXPoEPSETable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemXPoEPSETable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedRemXPoEPSEPowerAv,  ASN_GAUGE,  RONLY,   var_lldpXMedRemXPoEPSETable, 3,  { 6, 1, 1 }},
{LEAF_lldpXMedRemXPoEPSEPowerSource,  ASN_INTEGER,  RONLY,   var_lldpXMedRemXPoEPSETable, 3,  { 6, 1, 2 }},
{LEAF_lldpXMedRemXPoEPSEPowerPriority,  ASN_INTEGER,  RONLY,   var_lldpXMedRemXPoEPSETable, 3,  { 6, 1, 3 }},
};

/** Initializes the lldpXMedRemXPoEPSETable module */
void
init_lldpXMedRemXPoEPSETable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemXPoEPSETable", lldpXMedRemXPoEPSETable_variables, variable3,
                 lldpXMedRemXPoEPSETable_variables_oid);
}

#define LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN  3

BOOL_T lldpXMedRemXPoEPSETable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];

    return TRUE;
}

/*
 * var_lldpXMedRemXPoEPSETable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemXPoEPSETable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemPoePseEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemXPoEPSETable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemPoePseEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemXPoEPSETable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemPoePseEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemPoePseEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemPoePseEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMXPOEPSEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedRemXPoEPSEPowerAv:
            *var_len = 4;
            long_return = entry.rem_pse_power_av;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemXPoEPSEPowerSource:
            *var_len = 4;
            long_return = entry.rem_pse_power_source;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemXPoEPSEPowerPriority:
            *var_len = 4;
            long_return = entry.rem_pse_power_priority;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

oid lldpXMedRemXPoEPDTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4795,1,3 };
/*
 * variable3 lldpXMedRemXPoEPDTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpXMedRemXPoEPDTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXMedRemXPoEPDPowerReq,  ASN_GAUGE,  RONLY,   var_lldpXMedRemXPoEPDTable, 3,  { 7, 1, 1 }},
{LEAF_lldpXMedRemXPoEPDPowerSource,  ASN_INTEGER,  RONLY,   var_lldpXMedRemXPoEPDTable, 3,  { 7, 1, 2 }},
{LEAF_lldpXMedRemXPoEPDPowerPriority,  ASN_INTEGER,  RONLY,   var_lldpXMedRemXPoEPDTable, 3,  { 7, 1, 3 }},
};

/** Initializes the lldpXMedRemXPoEPDTable module */
void
init_lldpXMedRemXPoEPDTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXMedRemXPoEPDTable", lldpXMedRemXPoEPDTable_variables, variable3,
                 lldpXMedRemXPoEPDTable_variables_oid);
}

#define LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN  3

BOOL_T lldpXMedRemXPoEPDTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *lldpRemTimeMark = compl[0];
    *lldpRemLocalPortNum = compl[1];
    *lldpRemIndex = compl[2];

    return TRUE;
}

/*
 * var_lldpXMedRemXPoEPDTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXMedRemXPoEPDTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    oid compl[LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_XMedRemPoePdEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (lldpXMedRemXPoEPDTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (LLDP_POM_GetXMedRemPoePdEntry(&entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        lldpXMedRemXPoEPDTable_OidIndexToData(exact, compc, compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (LLDP_POM_GetXMedRemPoePdEntry(&entry) != TRUE)
            {
                /* get next data */
                if (LLDP_PMGR_GetNextXMedRemPoePdEntry(&entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (LLDP_PMGR_GetNextXMedRemPoePdEntry(&entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = entry.rem_time_mark;
    best_inst[1] = entry.rem_local_port_num;
    best_inst[2] = entry.rem_index;
    memcpy(name + vp->namelen, best_inst,
           LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + LLDPXMEDREMXPOEPDENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_lldpXMedRemXPoEPDPowerReq:
            *var_len = 4;
            long_return = entry.rem_pd_power_req;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemXPoEPDPowerSource:
            *var_len = 4;
            long_return = entry.rem_pd_power_source;
            return (u_char *) &long_return;

        case LEAF_lldpXMedRemXPoEPDPowerPriority:
            *var_len = 4;
            long_return = entry.rem_pd_power_priority;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

#endif  /* #if (SYS_CPNT_LLDP_MED == TRUE) */
