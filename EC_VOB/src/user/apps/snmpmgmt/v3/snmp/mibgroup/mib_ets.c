#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ETS == TRUE)
#include "sysfun.h"
#include "mibtreeinit.h"
#include "sys_pmgr.h"
#include "mib_ets.h"
#include "ets_pmgr.h"

static oid etsMgt_variables_oid[] = { SYS_ADPT_PRIVATEMIB_OID, 1, 77};

/********************************************
 ************** etsPortTable ****************
 ********************************************
 */
struct variable3 etsPortTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_etsPortIndex, ASN_INTEGER, RONLY, var_etsPortTable, 3, { 1, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_etsPortMode, ASN_INTEGER, RWRITE, var_etsPortTable, 3, { 1, 1, 2 }},
    { LEAF_etsPortOperMode, ASN_INTEGER, RONLY, var_etsPortTable, 3, { 1, 1, 3 }},
};

void
init_etsPortTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_MIB("etsPortTable", etsPortTable_variables, variable3,
                  etsMgt_variables_oid);
}

/********************************************
 ****** etsPortPriorityAssignmentTable ******
 ********************************************
 */
struct variable3 etsPortPriorityAssignmentTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_etsPortPaIndex, ASN_INTEGER, RONLY, var_etsPortPriorityAssignmentTable, 3, { 2, 1, 1 }},
    { LEAF_etsPortPaPriority, ASN_INTEGER, RONLY, var_etsPortPriorityAssignmentTable, 3, { 2, 1, 2 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_etsPortPaTrafficClass, ASN_INTEGER, RWRITE, var_etsPortPriorityAssignmentTable, 3, { 2, 1, 3 }},
    { LEAF_etsPortPaOperTrafficClass, ASN_INTEGER, RONLY, var_etsPortPriorityAssignmentTable, 3, { 2, 1, 4 }},
};

void
init_etsPortPriorityAssignmentTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_MIB("etsPortPriorityAssignmentTable", etsPortPriorityAssignmentTable_variables, variable3,
                  etsMgt_variables_oid);
}

/********************************************
 ****** etsPortTrafficClassWeightTable ******
 ********************************************
 */
struct variable3 etsPortTrafficClassWeightTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_etsPortTcwIndex, ASN_INTEGER, RONLY, var_etsPortTrafficClassWeightTable, 3, { 3, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_etsPortTcwWeightList, ASN_OCTET_STR, RWRITE, var_etsPortTrafficClassWeightTable, 3, { 3, 1, 2 }},
    { LEAF_etsPortTcwOperWeightList, ASN_OCTET_STR, RONLY, var_etsPortTrafficClassWeightTable, 3, { 3, 1, 3 }},
};

void
init_etsPortTrafficClassWeightTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_MIB("etsPortTrafficClassWeightTable", etsPortTrafficClassWeightTable_variables, variable3,
                  etsMgt_variables_oid);
}

/********************************************
 ** etsPortTrafficSelectionAlgorithmTable ***
 ********************************************
 */
struct variable3 etsPortTrafficSelectionAlgorithmTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_etsPortTsaIndex, ASN_INTEGER, RONLY, var_etsPortTrafficSelectionAlgorithmTable, 3, { 4, 1, 1 }},
    { LEAF_etsPortTsaTrafficClass, ASN_INTEGER, RONLY, var_etsPortTrafficSelectionAlgorithmTable, 3, { 4, 1, 2 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_etsPortTsaTrafficSelectionAlgorithm, ASN_INTEGER, RWRITE, var_etsPortTrafficSelectionAlgorithmTable, 3, { 4, 1, 3 }},
    { LEAF_etsPortTsaOperTrafficSelectionAlgorithm, ASN_INTEGER, RONLY, var_etsPortTrafficSelectionAlgorithmTable, 3, { 4, 1, 4 }},
};

void
init_etsPortTrafficSelectionAlgorithmTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_MIB("etsPortTrafficSelectionAlgorithmTable", etsPortTrafficSelectionAlgorithmTable_variables, variable3,
                  etsMgt_variables_oid);
}

void init_etsMgt(void)
{
}

/********************************************
 *************** etsPortTable ***************
 ********************************************
 */
#define ETSPORTENTRY_INSTANCE_LEN  1

BOOL_T etsPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *etsPortIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != ETSPORTENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *etsPortIndex = compl[0];

    return TRUE;
}

/*
 * var_etsPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_etsPortTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, lport =0;
    ETS_TYPE_PortEntry_T ets_pentry;
    oid compl[ETSPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[ETSPORTENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_etsPortMode:
            *write_method = write_etsPortMode;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        ETSPORTENTRY_INSTANCE_LEN);

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! etsPortTable_OidIndexToData(exact, compc, compl, &lport))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, ETS_TYPE_DB_CONFIG))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        etsPortTable_OidIndexToData(exact, compc, compl, &lport);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < ETSPORTENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, ETS_TYPE_DB_CONFIG))
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, ETS_TYPE_DB_CONFIG))
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, ETS_TYPE_DB_CONFIG))
            {
                return NULL;
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = lport;
    memcpy(name + vp->namelen, best_inst, ETSPORTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + ETSPORTENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_etsPortIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_etsPortMode:
        {
            ETS_TYPE_MODE_T tmp_mode;

            if (ETS_TYPE_RETURN_OK == ETS_PMGR_GetMode(lport, &tmp_mode))
            {
                *var_len = sizeof(long_return);

                switch(tmp_mode)
                {
                case ETS_TYPE_MODE_USER:
                    long_return = VAL_etsPortMode_on;
                    break;
                case ETS_TYPE_MODE_OFF:
                default:
                    long_return = VAL_etsPortMode_off;
                    break;
                case ETS_TYPE_MODE_AUTO:
                    long_return = VAL_etsPortMode_auto;
                    break;
                }

                return (u_char *) &long_return;
            }
            else
            {
                return NULL;
            }
        }
        case LEAF_etsPortOperMode:
        {
            ETS_TYPE_MODE_T tmp_mode;

            if (ETS_TYPE_RETURN_OK == ETS_PMGR_GetOperMode(lport, &tmp_mode))
            {
                *var_len = sizeof(long_return);

                switch(tmp_mode)
                {
                case ETS_TYPE_MODE_USER:
                    long_return = VAL_etsPortOperMode_on;
                    break;
                case ETS_TYPE_MODE_OFF:
                default:
                    long_return = VAL_etsPortOperMode_off;
                    break;
                }

                return (u_char *) &long_return;
            }
            else
            {
                return NULL;
            }
        }            return (u_char *) &long_return;
        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_etsPortMode(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_etsPortMode_off:
                    break;

                case VAL_etsPortMode_on:
                    break;

                case VAL_etsPortMode_auto:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T etsPortIndex = 0;
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* extract index
             */
            if (! etsPortTable_OidIndexToData(TRUE, name_len - oid_name_length,
                        &(name[oid_name_length]), &etsPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;
            {
                ETS_TYPE_MODE_T tmp_mode;

                switch (value)
                {
                case VAL_etsPortMode_on:
                    tmp_mode = ETS_TYPE_MODE_USER;
                    break;
                case VAL_etsPortMode_off:
                default:
                    tmp_mode = ETS_TYPE_MODE_OFF;
                    break;
                case VAL_etsPortMode_auto:
                    tmp_mode = ETS_TYPE_MODE_AUTO;
                    break;
                }

                /* set to core layer
                 */
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_SetMode(etsPortIndex, tmp_mode))
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
            break;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

/********************************************
 ****** etsPortPriorityAssignmentTable ******
 ********************************************
 */
#define ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN  2

BOOL_T etsPortPriorityAssignmentTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *etsPortPaIndex, UI32_T *etsPortPaPriority)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *etsPortPaIndex = compl[0];
    *etsPortPaPriority = compl[1];

    return TRUE;
}

/*
 * var_etsPortPriorityAssignmentTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_etsPortPriorityAssignmentTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, lport =0, pri =0, tmp_tc, db_type = ETS_TYPE_DB_CONFIG;
    ETS_TYPE_PortEntry_T ets_pentry;
    oid compl[ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_etsPortPaTrafficClass:
            *write_method = write_etsPortPaTrafficClass;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN);

    if (LEAF_etsPortPaOperTrafficClass == vp->magic)
        db_type = ETS_TYPE_DB_OPER;

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! etsPortPriorityAssignmentTable_OidIndexToData(exact, compc, compl,
            &lport, &pri))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortPrioAssign(lport, pri, &tmp_tc, db_type))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        etsPortPriorityAssignmentTable_OidIndexToData(exact, compc, compl,
            &lport, &pri);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, db_type))
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
                {
                    return NULL;
                }
                else
                {
                    pri = MIN_etsPortPaPriority;
                    tmp_tc = ets_pentry.priority_assign[pri];
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            switch (pri)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                pri++;
                if (ETS_TYPE_RETURN_OK == ETS_PMGR_GetPortPrioAssign(lport, pri, &tmp_tc, db_type))
                {
                    break;
                }

            case 7:
            default:
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
                {
                    return NULL;
                }
                else
                {
                    pri = MIN_etsPortPaPriority;
                    tmp_tc = ets_pentry.priority_assign[pri];
                }
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = lport;
    best_inst[1] = pri;
    memcpy(name + vp->namelen, best_inst, ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + ETSPORTPRIORITYASSIGNMENTENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_etsPortPaIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
        case LEAF_etsPortPaPriority:
            *var_len = sizeof(long_return);
            long_return = pri;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_etsPortPaTrafficClass:
            *var_len = sizeof(long_return);
            long_return = tmp_tc;
            return (u_char *) &long_return;

        case LEAF_etsPortPaOperTrafficClass:
            *var_len = sizeof(long_return);
            long_return = tmp_tc;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_etsPortPaTrafficClass(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            if ((*(long *) var_val < MIN_etsPortPaTrafficClass)
                || (*(long *) var_val >= SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T etsPortPaIndex = 0;
            UI32_T etsPortPaPriority = 0;
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* extract index
             */
            if (! etsPortPriorityAssignmentTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &etsPortPaIndex, &etsPortPaPriority))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            /* set to core layer
             */
            if (ETS_TYPE_RETURN_OK !=ETS_PMGR_SetPortPrioAssignByUser(etsPortPaIndex, etsPortPaPriority, value))
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

/********************************************
 ****** etsPortTrafficClassWeightTable ******
 ********************************************
 */
#define ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN  1

BOOL_T etsPortTrafficClassWeightTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *etsPortTcwIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *etsPortTcwIndex = compl[0];

    return TRUE;
}

/*
 * var_etsPortTrafficClassWeightTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_etsPortTrafficClassWeightTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, idx, lport =0, db_type = ETS_TYPE_DB_CONFIG;;
    ETS_TYPE_PortEntry_T ets_pentry;
    oid compl[ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN] = {0};
    oid best_inst[ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_etsPortTcwWeightList:
            *write_method = write_etsPortTcwWeightList;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN);

    if (LEAF_etsPortTcwOperWeightList == vp->magic)
        db_type = ETS_TYPE_DB_OPER;

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! etsPortTrafficClassWeightTable_OidIndexToData(
                exact, compc, compl, &lport))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, db_type))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        etsPortTrafficClassWeightTable_OidIndexToData(
            exact, compc, compl, &lport);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, db_type))
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
            {
                return NULL;
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = lport;
    memcpy(name + vp->namelen, best_inst, ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + ETSPORTTRAFFICCLASSBANDWIDTHENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_etsPortTcwIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_etsPortTcwWeightList:
            *var_len = SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS;
            for (idx =0; idx < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; idx++)
                return_buf[idx] = ets_pentry.tc_weight[idx];

            return (u_char *) return_buf;

        case LEAF_etsPortTcwOperWeightList:
            *var_len = SIZE_etsPortTcwOperWeightList;
            for (idx =0; idx < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; idx++)
                return_buf[idx] = ets_pentry.tc_weight[idx];

            return (u_char *) return_buf;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_etsPortTcwWeightList(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != SIZE_etsPortTcwWeightList)
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
            UI32_T etsPortTcwIndex = 0;
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            UI32_T idx, tmp_weight_buf[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];

            /* extract index
             */
            if (! etsPortTrafficClassWeightTable_OidIndexToData(TRUE,
                    name_len - oid_name_length,
                    &(name[oid_name_length]),
                    &etsPortTcwIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            for (idx =0; idx <var_val_len; idx++)
                tmp_weight_buf[idx] = var_val[idx];

            /* set to core layer
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_SetWeightByUser(etsPortTcwIndex, tmp_weight_buf))
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

/********************************************
 ** etsPortTrafficSelectionAlgorithmTable ***
 ********************************************
 */
#define ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN  2

BOOL_T etsPortTrafficSelectionAlgorithmTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *etsPortTsaIndex, UI32_T *etsPortTsaTrafficClass)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *etsPortTsaIndex = compl[0];
    *etsPortTsaTrafficClass = compl[1];

    return TRUE;
}

/*
 * var_etsPortTrafficSelectionAlgorithmTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_etsPortTrafficSelectionAlgorithmTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, lport =0, tc =0, tmp_tsa, db_type = ETS_TYPE_DB_CONFIG;
    ETS_TYPE_PortEntry_T ets_pentry;
    oid compl[ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN] = {0};
    oid best_inst[ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_etsPortTsaTrafficSelectionAlgorithm:
            *write_method = write_etsPortTsaTrafficSelectionAlgorithm;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN);

    if (LEAF_etsPortTsaOperTrafficSelectionAlgorithm == vp->magic)
        db_type = ETS_TYPE_DB_OPER;

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! etsPortTrafficSelectionAlgorithmTable_OidIndexToData(exact, compc, compl,
            &lport, &tc))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetTSA( lport, tc, &tmp_tsa, db_type))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        etsPortTrafficSelectionAlgorithmTable_OidIndexToData(exact, compc, compl,
            &lport, &tc);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetPortEntry(lport, &ets_pentry, db_type))
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
                {
                    return NULL;
                }
                else
                {
                    tc = MIN_etsPortTsaTrafficClass;
                    tmp_tsa = ets_pentry.tsa[tc];
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            switch (tc)
            {
            case 0:
            case 1:
                tc++;
                if (ETS_TYPE_RETURN_OK == ETS_PMGR_GetTSA(lport, tc, &tmp_tsa, db_type))
                {
                    break;
                }

            case 2:
            default:
                if (ETS_TYPE_RETURN_OK != ETS_PMGR_GetNextPortEntry(&lport, &ets_pentry, db_type))
                {
                    return NULL;
                }
                else
                {
                    tc = MIN_etsPortTsaTrafficClass;
                    tmp_tsa = ets_pentry.tsa[tc];
                }
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = lport;
    best_inst[1] = tc;
    memcpy(name + vp->namelen, best_inst, ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + ETSPORTTRAFFICSELECTIONALGORITHMENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_etsPortTsaIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
        case LEAF_etsPortTsaTrafficClass:
            *var_len = sizeof(long_return);
            long_return = tc;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_etsPortTsaTrafficSelectionAlgorithm:
            switch (tmp_tsa)
            {
            case ETS_TYPE_TSA_SP:
                *var_len = sizeof(long_return);
                long_return = VAL_etsPortTsaTrafficSelectionAlgorithm_strict;
                return (u_char *) &long_return;
            case ETS_TYPE_TSA_ETS:
                *var_len = sizeof(long_return);
                long_return = VAL_etsPortTsaTrafficSelectionAlgorithm_ets;
                return (u_char *) &long_return;
            default:
                break;
            }
        case LEAF_etsPortTsaOperTrafficSelectionAlgorithm:
            switch (tmp_tsa)
            {
            case ETS_TYPE_TSA_SP:
                *var_len = sizeof(long_return);
                long_return = VAL_etsPortTsaOperTrafficSelectionAlgorithm_strict;
                return (u_char *) &long_return;
            case ETS_TYPE_TSA_ETS:
                *var_len = sizeof(long_return);
                long_return = VAL_etsPortTsaOperTrafficSelectionAlgorithm_ets;
                return (u_char *) &long_return;
            default:
                break;
            }

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_etsPortTsaTrafficSelectionAlgorithm(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_etsPortTsaTrafficSelectionAlgorithm_strict:
                    break;

                case VAL_etsPortTsaTrafficSelectionAlgorithm_ets:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T etsPortTsaIndex = 0;
            UI32_T etsPortTsaTrafficClass = 0;
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* extract index
             */
            if (! etsPortTrafficSelectionAlgorithmTable_OidIndexToData(TRUE,
                        name_len - oid_name_length,
                        &(name[oid_name_length]),
                        &etsPortTsaIndex, &etsPortTsaTrafficClass))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            if (VAL_etsPortTsaTrafficSelectionAlgorithm_strict == value)
                value = ETS_TYPE_TSA_SP;
            else
                value = ETS_TYPE_TSA_ETS;

            /* set to core layer
             */
            if (ETS_TYPE_RETURN_OK != ETS_PMGR_SetTSAByUser(etsPortTsaIndex, etsPortTsaTrafficClass, value))
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

#endif /* #if (SYS_CPNT_ETS == TRUE) */

