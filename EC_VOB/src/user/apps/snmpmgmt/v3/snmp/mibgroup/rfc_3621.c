/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf,v 1.5 2002/07/18 14:18:52 dts12 Exp $
 */
#include "sys_cpnt.h"


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "rfc_3621.h"
#include "sys_adpt.h"
#include "sys_mgr.h"
#include "sysORTable.h"
#include "sys_type.h"
#include "sysfun.h"
// #include "amtr_mgr.h"
// #include "nmtr_mgr.h"
// #include "swctrl.h"
// #undef LEAF_pethPsePortDetectionStatus
// #undef VAL_pethPsePortDetectionStatus_deliveringPower
// #undef VAL_pethPsePortDetectionStatus_fault
// #undef VAL_pethPsePortDetectionStatus_test
// #undef LEAF_pethPsePortPowerPriority
// #undef LEAF_pethPsePortMPSAbsentCounter
// #undef LEAF_pethPsePortType
// #undef LEAF_pethPsePortPowerClassifications
// #undef LEAF_pethMainPseUsageThreshold
#include "leaf_3621.h"

// #if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
// #include "xstp_mgr.h"
// #else
// #include "sta_mgr.h"
// #endif

#if (SYS_CPNT_POE == TRUE)
#include "poe_pmgr.h"
#include "poe_pom.h"
#include "poe_type.h"
#endif

/*
 * pethPsePortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid pethPsePortTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 105, 1};

struct variable3 pethPsePortTable_variables[] = {

    #define PETHPSEPORTGROUPINDEX               1
    #define PETHPSEPORTINDEX                    2
    #define PETHPSEPORTADMINENABLE              3
    #define PETHPSEPORTPOWERPAIRSCONTROLABILITY 4
    #define PETHPSEPORTPOWERPAIRS               5
    #define PETHPSEPORTDETECTIONSTATUS          6
    #define PETHPSEPORTPOWERPRIORITY            7
    #define PETHPSEPORTMPSABSENTCOUNTER         8
    #define PETHPSEPORTTYPE                     9
    #define PETHPSEPORTPOWERCLASSIFICATIONS     10
    #define PETHPSEPORTINVALIDSIGNATURECOUNTER  11
    #define PETHPSEPORTPOWERDENIEDCOUNTER       12
    #define PETHPSEPORTOVERLOADCOUNTER          13
    #define PETHPSEPORTSHORTCOUNTER             14

    {PETHPSEPORTGROUPINDEX,               ASN_INTEGER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 1}},
    {PETHPSEPORTINDEX,                    ASN_INTEGER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 2}},
    {PETHPSEPORTADMINENABLE,              ASN_INTEGER,   RWRITE, var_pethPsePortTable, 3, {1, 1, 3}},
    {PETHPSEPORTPOWERPAIRSCONTROLABILITY, ASN_INTEGER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 4}},
    {PETHPSEPORTPOWERPAIRS,               ASN_INTEGER,   RWRITE, var_pethPsePortTable, 3, {1, 1, 5}},
    {PETHPSEPORTDETECTIONSTATUS,          ASN_INTEGER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 6}},
    {PETHPSEPORTPOWERPRIORITY,            ASN_INTEGER,   RWRITE, var_pethPsePortTable, 3, {1, 1, 7}},
    {PETHPSEPORTMPSABSENTCOUNTER,         ASN_COUNTER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 8}},
    {PETHPSEPORTTYPE,                     ASN_OCTET_STR, RWRITE, var_pethPsePortTable, 3, {1, 1, 9}},
    {PETHPSEPORTPOWERCLASSIFICATIONS,     ASN_INTEGER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 10}},
    {PETHPSEPORTINVALIDSIGNATURECOUNTER,  ASN_COUNTER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 11}},
    {PETHPSEPORTPOWERDENIEDCOUNTER,       ASN_COUNTER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 12}},
    {PETHPSEPORTOVERLOADCOUNTER,          ASN_COUNTER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 13}},
    {PETHPSEPORTSHORTCOUNTER,             ASN_COUNTER,   RONLY,  var_pethPsePortTable, 3, {1, 1, 14}},
};

/** Initializes the pethPsePortTable module */
void
init_pethPsePortTable(void)
{
    oid powerEthernetMIB_module_oid[] = { SNMP_OID_MIB2, 105 };

    DEBUGMSGTL(("pethPsePortTable", "Initializing\n"));
    REGISTER_MIB("pethPsePortTable", pethPsePortTable_variables, variable3,
                 pethPsePortTable_variables_oid);

    REGISTER_SYSOR_ENTRY(powerEthernetMIB_module_oid,
                         "The MIB module to Configure  POWER-ETHERNET MIB.");
}

/*
 * var_pethPsePortTable():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
#if (SYS_CPNT_POE == TRUE)
/********************************************
 ***************pethPsePortTable********
 ********************************************
 */
#define pethPsePortEntry_INSTANCE_LEN 2

static UI32_T getPethPsePortTableOidNameLen(void)
{
    return 11;
}

static BOOL_T pethPsePortTable_get(int    compc,
                                   oid    *compl,
                                   UI32_T *idx1,
                                   UI32_T *idx2,
                                   POE_OM_PsePort_T *data)
{
    if (compc != pethPsePortEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    *idx1 = compl[0];
    *idx2 = compl[1];

    if (POE_POM_GetPsePortEntry(*idx1, *idx2, data) != POE_TYPE_RETURN_OK)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T pethPsePortTable_next(int    compc,
                                    oid    *compl,
                                    UI32_T *idx1,
                                    UI32_T *idx2,
                                    POE_OM_PsePort_T *data)
{
    oid tmp_compl[pethPsePortEntry_INSTANCE_LEN];

    /* Generate the instance of each table entry and find the
     * smallest instance that's larger than compc/compl.
     *
     * Step 1: Verify and extract the input key from "compc" and "compl"
     * Note: The number of input key is defined by "compc".
     *       The key for the specified instance is defined in compl.
     */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_pethPsePortGroupIndex);
    SNMP_MGR_checkCompl(1, 1, tmp_compl, MAX_pethPsePortIndex);
    SNMP_MGR_ConvertRemainToZero(compc, pethPsePortEntry_INSTANCE_LEN, tmp_compl);

    *idx1 = tmp_compl[0];
    *idx2 = tmp_compl[1];

    if (SNMP_MGR_IsDebugMode())
    {
        SYSFUN_Debug_Printf("pethPsePortTable_next:idx1=[%lu], idx2=[%lu]\n", *idx1, *idx2);
    }

    if (compc < pethPsePortEntry_INSTANCE_LEN)
    {
        if (POE_POM_GetPsePortEntry(*idx1, *idx2, data) != POE_TYPE_RETURN_OK)
        {
            if (POE_POM_GetNextPsePortEntry(idx1, idx2, data) != POE_TYPE_RETURN_OK)
            {
                if (SNMP_MGR_IsDebugMode())
                {
                    SYSFUN_Debug_Printf("pethPsePortTable_next: get & genext return false\n");
                }
                return FALSE;
            }
        }
    }
    else
    {
        if (POE_POM_GetNextPsePortEntry(idx1, idx2, data) != POE_TYPE_RETURN_OK)
        {
            if (SNMP_MGR_IsDebugMode())
            {
                SYSFUN_Debug_Printf("pethPsePortTable_next: get return false\n");
            }
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * var_pethPsePortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_pethPsePortTable(
    struct variable *vp,
    oid             *name,
    size_t          *length,
    int             exact,
    size_t          *var_len,
    WriteMethod     **write_method)
{
    UI32_T           compc = 0;
    oid              compl[pethPsePortEntry_INSTANCE_LEN];
    oid              best_inst[pethPsePortEntry_INSTANCE_LEN];
    UI32_T           idx1, idx2;
    POE_OM_PsePort_T data;

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, pethPsePortEntry_INSTANCE_LEN);

    /*check compc, retrive compl*/
    if (exact)  /*get,set*/
    {
        if (!pethPsePortTable_get(compc, compl, &idx1, &idx2, &data))
            return NULL;
    }
    else        /*getnext*/
    {
        if (!pethPsePortTable_next(compc, compl, &idx1, &idx2, &data))
            return NULL;
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    best_inst[0] = idx1;
    best_inst[1] = idx2;

    memcpy(name + vp->namelen, best_inst, pethPsePortEntry_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + pethPsePortEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (vp->magic)
    {
        case PETHPSEPORTADMINENABLE:
            *write_method = write_pethPsePortAdminEnable;
            long_return = data.pse_port_admin_enable;
            return (u_char *) &long_return;

        case PETHPSEPORTPOWERPAIRSCONTROLABILITY:
            long_return = data.pse_port_power_pairs_ctrl_ability;
            return (u_char *) &long_return;

        case PETHPSEPORTPOWERPAIRS:
            *write_method = write_pethPsePortPowerPairs;
            long_return = data.pse_port_power_pairs;
            return (u_char *) &long_return;

        case PETHPSEPORTDETECTIONSTATUS:
            long_return = data.pse_port_detection_status;
            return (u_char *) &long_return;

        case PETHPSEPORTPOWERPRIORITY:
            *write_method = write_pethPsePortPowerPriority;
            long_return = data.pse_port_power_priority;
            return (u_char *) &long_return;

        case PETHPSEPORTMPSABSENTCOUNTER:
            long_return = data.pse_port_mpsabsent_counter;
            return (u_char *) &long_return;

        case PETHPSEPORTTYPE:
            *write_method = write_pethPsePortType;
            strcpy(return_buf, data.pse_port_type);
            *var_len = strlen(return_buf);
            return (u_char *) return_buf;

        case PETHPSEPORTPOWERCLASSIFICATIONS:
            long_return = data.pse_port_power_classifications;
            return (u_char *) &long_return;

#if (SYS_CPNT_POE_COUNTER_SUPPORT == TRUE)
        case PETHPSEPORTINVALIDSIGNATURECOUNTER:
            long_return = data.pse_port_invalid_signature_counter;
            return (u_char *) &long_return;

        case PETHPSEPORTPOWERDENIEDCOUNTER:
            long_return = data.pse_port_power_denied_counter;
            return (u_char *) &long_return;

        case PETHPSEPORTOVERLOADCOUNTER:
            long_return = data.pse_port_overload_counter;
            return (u_char *) &long_return;

        case PETHPSEPORTSHORTCOUNTER:
            long_return = data.pse_port_short_counter;
            return (u_char *) &long_return;
#else
        case PETHPSEPORTINVALIDSIGNATURECOUNTER:
        case PETHPSEPORTPOWERDENIEDCOUNTER:
        case PETHPSEPORTOVERLOADCOUNTER:
        case PETHPSEPORTSHORTCOUNTER:
#endif
        default:
            ERROR_MSG("");
    }

    return NULL;
}

int write_pethPsePortAdminEnable(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long value;

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethPsePortTableOidNameLen() + pethPsePortEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;

            switch (value)
            {
                case VAL_pethPsePortAdminEnable_true:
                case VAL_pethPsePortAdminEnable_false:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
            {
                /*
                  * The variable has been stored in 'value' for you to use,
                  * and you have just been asked to do something with it.
                  * Note that anything done here must be reversable in the UNDO case
                  */
                UI32_T  idx1, idx2;

                idx1 = name[getPethPsePortTableOidNameLen()];
                idx2 = name[getPethPsePortTableOidNameLen() + 1];

                value = *(long *)var_val;

                if (SNMP_MGR_IsDebugMode())
                {
                    SYSFUN_Debug_Printf("write_pethPsePortAdminEnable:idx1=[%lu], idx2=[%lu], value=[%lu]\n", idx1, idx2, value);
                }

                if (POE_PMGR_SetPsePortAdmin(idx1, idx2, value) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                        SYSFUN_Debug_Printf("write_pethPsePortAdminEnable:POE_PMGR_SetPsePortAdmin return false\n");
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_pethPsePortPowerPairs(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long value;

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethPsePortTableOidNameLen() + pethPsePortEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            switch (value)
            {
                case VAL_pethPsePortPowerPairs_signal:
                case VAL_pethPsePortPowerPairs_spare:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /*
             * Release any resources that have been allocated
             */
            break;

        case ACTION:
            {
                /*
                 * The variable has been stored in 'value' for you to use,
                 * and you have just been asked to do something with it.
                 * Note that anything done here must be reversable in the UNDO case
                 */
                UI32_T  idx1, idx2;
                idx1 = name[getPethPsePortTableOidNameLen()];
                idx2 = name[getPethPsePortTableOidNameLen() + 1];
                value = *(long *)var_val;

                if (SNMP_MGR_IsDebugMode())
                {
                    SYSFUN_Debug_Printf("write_pethPsePortPowerPairs:idx1=[%lu], idx2=[%lu], value=[%lu]\n", idx1, idx2, value);
                }

                if (POE_PMGR_SetPsePortPowerPairs(idx1, idx2, value) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                        SYSFUN_Debug_Printf("write_pethPsePortPowerPairs:POE_PMGR_SetPsePortPowerPairs return false\n");
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_pethPsePortPowerPriority(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long value;

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethPsePortTableOidNameLen() + pethPsePortEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            switch (value)
            {
                case VAL_pethPsePortPowerPriority_critical:
                case VAL_pethPsePortPowerPriority_high:
                case VAL_pethPsePortPowerPriority_low:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }

        case FREE:
            break;

        case ACTION:
            {
                /*
                 * The variable has been stored in 'value' for you to use,
                 * and you have just been asked to do something with it.
                 * Note that anything done here must be reversable in the UNDO case
                 */
                UI32_T  idx1, idx2;
                idx1 = name[getPethPsePortTableOidNameLen()];
                idx2 = name[getPethPsePortTableOidNameLen() + 1];
                value = *(long *)var_val;
                if (SNMP_MGR_IsDebugMode())
                {
                    SYSFUN_Debug_Printf("write_pethPsePortPowerPriority:idx1=[%lu], idx2=[%lu], value=[%lu]\n", idx1, idx2, value);
                }

                if (POE_PMGR_SetPsePortPowerPriority(idx1, idx2, value) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                        SYSFUN_Debug_Printf("write_pethPsePortPowerPriority:POE_PMGR_SetPsePortPowerPriority return false\n");
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

int write_pethPsePortType(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long    value;
    UI8_T   buffer[MAXSIZE_pethPsePortType + 1];

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethPsePortTableOidNameLen() + pethPsePortEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            if ((var_val_len < MINSIZE_pethPsePortType) || (var_val_len > MAXSIZE_pethPsePortType))
                return SNMP_ERR_WRONGVALUE;

            break;

        case FREE:
            break;

        case ACTION:
            {
                /*
                 * The variable has been stored in 'value' for you to use,
                 * and you have just been asked to do something with it.
                 * Note that anything done here must be reversable in the UNDO case
                 */
                UI32_T  idx1, idx2;
                idx1 = name[getPethPsePortTableOidNameLen()];
                idx2 = name[getPethPsePortTableOidNameLen() + 1];

                memcpy(buffer, var_val, var_val_len);
                buffer[var_val_len] = '\0';

                if (POE_PMGR_SetPsePortType(idx1, idx2, buffer, var_val_len + 1) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                        SYSFUN_Debug_Printf("write_pethPsePortType:POE_PMGR_SetPsePortType return false\n");
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}


/********************************************
 ***************pethMainPseTable*************
 ********************************************
 */

#define pethMainPseEntry_INSTANCE_LEN 1

oid pethMainPseTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 105, 1, 3};

/*
 * variable3 pethMainPseTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 pethMainPseTable_variables[] = {

    #define PETHMAINPSEGROUPINDEX       1
    #define PETHMAINPSEPOWER            2
    #define PETHMAINPSEOPERSTATUS       3
    #define PETHMAINPSECONSUMPTIONPOWER 4
    #define PETHMAINPSEUSAGETHRESHOLD   5

    {PETHMAINPSEGROUPINDEX,       ASN_INTEGER, RONLY,  var_pethMainPseTable, 3, {1, 1, 1}},
    {PETHMAINPSEPOWER,            ASN_GAUGE,   RONLY,  var_pethMainPseTable, 3, {1, 1, 2}},
    {PETHMAINPSEOPERSTATUS,       ASN_INTEGER, RONLY,  var_pethMainPseTable, 3, {1, 1, 3}},
    {PETHMAINPSECONSUMPTIONPOWER, ASN_GAUGE,   RONLY,  var_pethMainPseTable, 3, {1, 1, 4}},
    {PETHMAINPSEUSAGETHRESHOLD,   ASN_INTEGER, RWRITE, var_pethMainPseTable, 3, {1, 1, 5}},
};

void
init_pethMainPseTable(void)
{
    DEBUGMSGTL(("pethMainPseTable", "Initializing\n"));
    REGISTER_MIB("pethMainPseTable", pethMainPseTable_variables, variable3, pethMainPseTable_variables_oid);
}

static UI32_T getPethMainPseTableOidNameLen(void)
{
    return 12;
}

static BOOL_T pethMainPseTable_get(int     compc,
                                   oid     *compl,
                                   UI32_T  *idx1,
                                   POE_OM_MainPse_T   *data)
{
    if (compc != pethMainPseEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    *idx1 = compl[0];

    if (SNMP_MGR_IsDebugMode())
        SYSFUN_Debug_Printf("pethMainPseTable_get:idx1=[%lu]\n", *idx1);

    if (POE_POM_GetPethMainPseEntry(*idx1, data) != POE_TYPE_RETURN_OK)
    {
        if (SNMP_MGR_IsDebugMode())
            SYSFUN_Debug_Printf("pethMainPseTable_get:POE_POM_GetPethMainPseEntry return false\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }   /*End of if */
}

static BOOL_T pethMainPseTable_next(int     compc,
                                    oid     *compl,
                                    UI32_T  *idx1,
                                    POE_OM_MainPse_T    *data)
{
    oid tmp_compl[pethMainPseEntry_INSTANCE_LEN];

    /* Generate the instance of each table entry and find the
     * smallest instance that's larger than compc/compl.
     *
     * Step 1: Verify and extract the input key from "compc" and "compl"
     * Note: The number of input key is defined by "compc".
     *       The key for the specified instance is defined in compl.
     */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_pethMainPseGroupIndex);
    SNMP_MGR_ConvertRemainToZero(compc, pethMainPseEntry_INSTANCE_LEN, tmp_compl);
    *idx1 = tmp_compl[0];

    if (SNMP_MGR_IsDebugMode())
        SYSFUN_Debug_Printf("pethMainPseTable_next:idx1=[%lu]\n", *idx1);

    if (compc < pethMainPseEntry_INSTANCE_LEN)
    {
        if (POE_POM_GetPethMainPseEntry(*idx1, data) != POE_TYPE_RETURN_OK)
        {
            if (POE_POM_GetNextMainPseEntry(idx1, data) != POE_TYPE_RETURN_OK)
            {
                if (SNMP_MGR_IsDebugMode())
                    SYSFUN_Debug_Printf("pethMainPseTable_next: get & genext return false\n");
                return FALSE;
            }
        }
    }
    else
    {
        if (POE_POM_GetNextMainPseEntry(idx1, data) != POE_TYPE_RETURN_OK)
        {
            if (SNMP_MGR_IsDebugMode())
                SYSFUN_Debug_Printf("pethMainPseTable_next: get return false\n");
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * var_pethMainPseTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_pethMainPseTable(
    struct variable *vp,
    oid             *name,
    size_t          *length,
    int             exact,
    size_t          *var_len,
    WriteMethod     **write_method)
{
    UI32_T           compc = 0;
    oid              compl[pethMainPseEntry_INSTANCE_LEN];
    oid              best_inst[pethMainPseEntry_INSTANCE_LEN];
    UI32_T           idx1;
    POE_OM_MainPse_T data;

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, pethMainPseEntry_INSTANCE_LEN);

    /*check compc, retrive compl*/
    if (exact)  /*get,set*/
    {
        if (!pethMainPseTable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else        /*getnext*/
    {
        if (!pethMainPseTable_next(compc, compl, &idx1, &data))
            return NULL;
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    best_inst[0] = idx1;

    memcpy(name + vp->namelen, best_inst, pethMainPseEntry_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + pethMainPseEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case PETHMAINPSEPOWER:
            long_return = data.main_pse_power;
            return (u_char *) &long_return;

        case PETHMAINPSEOPERSTATUS:
            long_return = data.main_pse_oper_status;
            return (u_char *) &long_return;

        case PETHMAINPSECONSUMPTIONPOWER:
            long_return = data.main_pse_consumption_power;
            return (u_char *) &long_return;

        case PETHMAINPSEUSAGETHRESHOLD:
            *write_method = write_pethMainPseUsageThreshold;
            long_return = data.main_pse_usage_threshold;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int write_pethMainPseUsageThreshold(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long    value;

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethMainPseTableOidNameLen() + pethMainPseEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            if (!((MIN_pethMainPseUsageThreshold <= value) && (value <= MAX_pethMainPseUsageThreshold)))
                return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
            break;

        case ACTION:
            {
                /*
                 * The variable has been stored in 'value' for you to use,
                 * and you have just been asked to do something with it.
                 * Note that anything done here must be reversable in the UNDO case
                 */
                UI32_T  idx1;

                idx1 = name[getPethMainPseTableOidNameLen()];
                value = *(long *)var_val;

                if (SNMP_MGR_IsDebugMode())
                    SYSFUN_Debug_Printf("write_pethMainPseUsageThreshold:idx1=[%lu], value=[%lu]\n", idx1, value);

                if (POE_PMGR_SetMainPseUsageThreshold(idx1, value) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                    {
                        SYSFUN_Debug_Printf("write_pethMainPseUsageThreshold:POE_PMGR_SetMainPseUsageThreshold return false\n");
                    }

                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

/********************************************
 *********pethNotificationControlTable*******
 ********************************************
 */
#define pethNotificationControlEntry_INSTANCE_LEN 1

oid pethNotificationControlTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 105, 1, 4};

struct variable3 pethNotificationControlTable_variables[] = {

    #define PETHNOTIFICATIONCONTROLGROUPINDEX	1
    #define PETHNOTIFICATIONCONTROLENABLE	    2

    {PETHNOTIFICATIONCONTROLGROUPINDEX, ASN_INTEGER, RONLY,  var_pethNotificationControlTable, 3, {1, 1, 1}},
    {PETHNOTIFICATIONCONTROLENABLE,     ASN_INTEGER, RWRITE, var_pethNotificationControlTable, 3, {1, 1, 2}},
};

void
init_pethNotificationControlTable(void)
{

    DEBUGMSGTL(("pethNotificationControlTable", "Initializing\n"));
    REGISTER_MIB("pethNotificationControlTable",
                 pethNotificationControlTable_variables, variable3,
                 pethNotificationControlTable_variables_oid);
}

static UI32_T getPethNotificationControlTableOidNameLen(void)
{
    return 12;
}

static BOOL_T pethNotificationControlTable_get(int     compc,
                                               oid     *compl,
                                               UI32_T  *idx1,
                                               UI32_T  *data)
{
    if (compc != pethNotificationControlEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    *idx1 = compl[0];

    if (SNMP_MGR_IsDebugMode())
        SYSFUN_Debug_Printf("pethNotificationControlTable_get:idx1=[%lu]\n", *idx1);

    if (POE_POM_GetPseNotificationCtrl(*idx1, data) != POE_TYPE_RETURN_OK)
    {
        if (SNMP_MGR_IsDebugMode())
        {
            SYSFUN_Debug_Printf("pethNotificationControlTable_get:POE_POM_GetPseNotificationCtrl return false\n");
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }   /*End of if */
}


static BOOL_T pethNotificationControlTable_next(int     compc,
                                                oid     *compl,
                                                UI32_T  *idx1,
                                                UI32_T  *data)
{
    oid tmp_compl[pethNotificationControlEntry_INSTANCE_LEN];

    /* Generate the instance of each table entry and find the
	 * smallest instance that's larger than compc/compl.
	 *
	 * Step 1: Verify and extract the input key from "compc" and "compl"
	 * Note: The number of input key is defined by "compc".
	 *       The key for the specified instance is defined in compl.
	 */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_pethNotificationControlGroupIndex);
    SNMP_MGR_ConvertRemainToZero(compc, pethNotificationControlEntry_INSTANCE_LEN, tmp_compl);
    *idx1 = tmp_compl[0];

    if (SNMP_MGR_IsDebugMode())
        SYSFUN_Debug_Printf("pethNotificationControlTable_next:idx1=[%lu]\n", *idx1);

    if (compc < pethNotificationControlEntry_INSTANCE_LEN)
    {
        if (POE_POM_GetPseNotificationCtrl(*idx1, data) != POE_TYPE_RETURN_OK)
        {
            if (POE_POM_GetNextNotificationCtrl(idx1, data) != POE_TYPE_RETURN_OK)
            {
                if (SNMP_MGR_IsDebugMode())
                    SYSFUN_Debug_Printf("pethNotificationControlTable_next: get & genext return false\n");
                return FALSE;
            }
        }
    }
    else
    {
        if (POE_POM_GetNextNotificationCtrl(idx1, data) != POE_TYPE_RETURN_OK)
        {
            if (SNMP_MGR_IsDebugMode())
                SYSFUN_Debug_Printf("pethNotificationControlTable_next: get return false\n");
            return FALSE;
        }
    }
    return TRUE;
}


/*
 * var_pethNotificationControlTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_pethNotificationControlTable(
    struct variable *vp,
    oid             *name,
    size_t          *length,
    int             exact,
    size_t          *var_len,
    WriteMethod     **write_method)
{
    UI32_T  compc = 0;
    oid     compl[pethNotificationControlEntry_INSTANCE_LEN];
    oid     best_inst[pethNotificationControlEntry_INSTANCE_LEN];
    UI32_T  idx1, data;

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, pethNotificationControlEntry_INSTANCE_LEN);

    /*check compc, retrive compl*/
    if (exact)  /*get,set*/
    {
        if (!pethNotificationControlTable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else        /*getnext*/
    {
        if (!pethNotificationControlTable_next(compc, compl, &idx1, &data))
            return NULL;
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    best_inst[0] = idx1;

    memcpy(name + vp->namelen, best_inst, pethNotificationControlEntry_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + pethNotificationControlEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (vp->magic)
    {
        case PETHNOTIFICATIONCONTROLENABLE:
            *write_method = write_pethNotificationControlEnable;
            long_return = data;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int write_pethNotificationControlEnable(
    int     action,
    u_char  *var_val,
    u_char  var_val_type,
    size_t  var_val_len,
    u_char  *statP,
    oid     *name,
    size_t  name_len)
{
    long    value;

    switch (action)
    {
        case RESERVE1:
            /* check 1: check if the input index is exactly match, if not return fail*/
            if (name_len != getPethNotificationControlTableOidNameLen() + pethNotificationControlEntry_INSTANCE_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            switch (value)
            {
                case VAL_pethNotificationControlEnable_true:
                case VAL_pethNotificationControlEnable_false:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
            {
                /*
                 * The variable has been stored in 'value' for you to use,
                 * and you have just been asked to do something with it.
                 * Note that anything done here must be reversable in the UNDO case
                 */
                UI32_T  idx1;
                idx1 = name[getPethNotificationControlTableOidNameLen()];
                value = *(long *)var_val;

                if (SNMP_MGR_IsDebugMode())
                    SYSFUN_Debug_Printf("write_pethNotificationControlEnable:idx1=[%lu], value=[%lu]\n", idx1, value);

                if (POE_PMGR_SetNotificationCtrl(idx1, value) != POE_TYPE_RETURN_OK)
                {
                    if (SNMP_MGR_IsDebugMode())
                    {
                        SYSFUN_Debug_Printf("write_pethNotificationControlEnable:POE_PMGR_SetNotificationCtrl return false\n");
                    }

                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            }

        case UNDO:
        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

#endif/*end of #if (SYS_CPNT_POE == TRUE)*/