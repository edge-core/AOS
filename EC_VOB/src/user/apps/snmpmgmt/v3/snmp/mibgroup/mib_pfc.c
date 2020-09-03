#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_PFC == TRUE)
#include "sysfun.h"
#include "mibtreeinit.h"
#include "sys_pmgr.h"
#include "mib_pfc.h"
#include "pfc_pmgr.h"
#include "nmtr_pmgr.h"

static oid pfcMgt_variables_oid[] = { SYS_ADPT_PRIVATEMIB_OID, 1, 78};

/********************************************
 ************** pfcPortTable ****************
 ********************************************
 */
struct variable3 pfcPortTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_pfcPortIndex, ASN_INTEGER, RONLY, var_pfcPortTable, 3, { 1, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_pfcPortMode, ASN_INTEGER, RWRITE, var_pfcPortTable, 3, { 1, 1, 2 }},
    { LEAF_pfcPortOperMode, ASN_INTEGER, RONLY, var_pfcPortTable, 3, { 1, 1, 3 }},
    { LEAF_pfcPortPriEnableList, ASN_OCTET_STR, RWRITE, var_pfcPortTable, 3, { 1, 1, 4 }},
    { LEAF_pfcPortOperPriEnableList, ASN_OCTET_STR, RONLY, var_pfcPortTable, 3, { 1, 1, 5 }},
};

void
init_pfcPortTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_MIB("pfcPortTable", pfcPortTable_variables, variable3,
                  pfcMgt_variables_oid);
}

/********************************************
 *********** pfcPortStatsTable **************
 ********************************************
 */
struct variable3 pfcPortStatsTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_pfcPortStatsIndex, ASN_INTEGER, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 1 }},
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_pfcPortStatsSentPri0Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 2 }},
    { LEAF_pfcPortStatsSentPri1Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 3 }},
    { LEAF_pfcPortStatsSentPri2Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 4 }},
    { LEAF_pfcPortStatsSentPri3Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 5 }},
    { LEAF_pfcPortStatsSentPri4Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 6 }},
    { LEAF_pfcPortStatsSentPri5Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 7 }},
    { LEAF_pfcPortStatsSentPri6Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 8 }},
    { LEAF_pfcPortStatsSentPri7Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 9 }},
    { LEAF_pfcPortStatsRecvPri0Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 10 }},
    { LEAF_pfcPortStatsRecvPri1Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 11 }},
    { LEAF_pfcPortStatsRecvPri2Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 12 }},
    { LEAF_pfcPortStatsRecvPri3Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 13 }},
    { LEAF_pfcPortStatsRecvPri4Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 14 }},
    { LEAF_pfcPortStatsRecvPri5Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 15 }},
    { LEAF_pfcPortStatsRecvPri6Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 16 }},
    { LEAF_pfcPortStatsRecvPri7Pkts, ASN_COUNTER64, RONLY, var_pfcPortStatsTable, 3, { 2, 1, 17 }},
    { LEAF_pfcPortStatsClearAction, ASN_INTEGER, RWRITE, var_pfcPortStatsTable, 3, { 2, 1, 18 }},
};

void
init_pfcPortStatsTable(void)
{
    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);


    _REGISTER_MIB("pfcPortStatsTable", pfcPortStatsTable_variables, variable3,
                  pfcMgt_variables_oid);
}

void init_pfcMgt(void)
{
}

/********************************************
 *************** pfcPortTable ***************
 ********************************************
 */
#define PFCPORTENTRY_INSTANCE_LEN  1

BOOL_T pfcPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *pfcPortIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != PFCPORTENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *pfcPortIndex = compl[0];

    return TRUE;
}

/*
 * var_pfcPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_pfcPortTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, lport =0, tmp_data;
    oid compl[PFCPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[PFCPORTENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_pfcPortMode:
            *write_method = write_pfcPortMode;
            break;

        case LEAF_pfcPortPriEnableList:
            *write_method = write_pfcPortPriEnableList;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        PFCPORTENTRY_INSTANCE_LEN);

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! pfcPortTable_OidIndexToData(exact, compc, compl, &lport))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (PFC_PMGR_GetDataByField(
                lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_data) != TRUE)
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        pfcPortTable_OidIndexToData(exact, compc, compl, &lport);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < PFCPORTENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (PFC_PMGR_GetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_ENABLE, &tmp_data) != TRUE)
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_NXT_PORT, &lport) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (PFC_PMGR_GetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_NXT_PORT, &lport) != TRUE)
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
    memcpy(name + vp->namelen, best_inst, PFCPORTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + PFCPORTENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_pfcPortIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_pfcPortMode:
            if (TRUE == PFC_PMGR_GetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &long_return))
            {
                *var_len = sizeof(long_return);

                switch(long_return)
                {
                case PFC_TYPE_PMODE_OFF:
                default:
                    long_return = VAL_pfcPortMode_off;
                    break;
                case PFC_TYPE_PMODE_ON:
                    long_return = VAL_pfcPortMode_on;
                    break;
                case PFC_TYPE_PMODE_AUTO:
                    long_return = VAL_pfcPortMode_auto;
                    break;
                }

                return (u_char *) &long_return;
            }
            else
            {
                return NULL;
            }

        case LEAF_pfcPortOperMode:
            if (TRUE == PFC_PMGR_GetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &long_return))
            {
                *var_len = sizeof(long_return);

                switch(long_return)
                {
                case PFC_TYPE_PMODE_OFF:
                default:
                    long_return = VAL_pfcPortMode_off;
                    break;
                case PFC_TYPE_PMODE_ON:
                    long_return = VAL_pfcPortMode_on;
                    break;
                }

                return (u_char *) &long_return;
            }
            else
            {
                return NULL;
            }

        case LEAF_pfcPortPriEnableList:
            if (TRUE == PFC_PMGR_GetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &tmp_data))
            {
                UI8_T   tmp_byte = tmp_data & 0xff;

                *var_len = SIZE_pfcPortPriEnableList;

                SNMP_MGR_BitsFromCoreToSnmp(
                    &tmp_byte, return_buf, SIZE_pfcPortPriEnableList);
                return (u_char *) return_buf;
            }
            else
            {
                return NULL;
            }

        case LEAF_pfcPortOperPriEnableList:
            if (TRUE == PFC_PMGR_GetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &tmp_data))
            {
                UI8_T   tmp_byte = tmp_data & 0xff;

                *var_len = SIZE_pfcPortPriEnableList;

                SNMP_MGR_BitsFromCoreToSnmp(
                    &tmp_byte, return_buf, SIZE_pfcPortPriEnableList);
                return (u_char *) return_buf;
            }
            else
            {
                return NULL;
            }

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_pfcPortMode(int action,
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
                case VAL_pfcPortMode_off:
                    break;

                case VAL_pfcPortMode_on:
                    break;

                case VAL_pfcPortMode_auto:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            UI32_T pfcPortIndex = 0, tmp_pmode;
            I32_T value = 0;

            /* extract index
             */
            if (! pfcPortTable_OidIndexToData(TRUE, name_len - oid_name_length,
                        &(name[oid_name_length]), &pfcPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            switch (value)
            {
            case VAL_pfcPortMode_off:
                tmp_pmode = PFC_TYPE_PMODE_OFF;
                break;

            case VAL_pfcPortMode_on:
                tmp_pmode = PFC_TYPE_PMODE_ON;
                break;

            case VAL_pfcPortMode_auto:
                tmp_pmode = PFC_TYPE_PMODE_AUTO;
                break;
            }

            /* set to core layer
             */
            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            pfcPortIndex, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_pmode))
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

int write_pfcPortPriEnableList(int action,
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
            if (var_val_len != SIZE_pfcPortPriEnableList)
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
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            UI32_T pfcPortIndex = 0, tmp_pri_en;
            UI8_T byte_buffer[SIZE_pfcPortPriEnableList] = {0};

            /* extract index
             */
            if (! pfcPortTable_OidIndexToData(TRUE, name_len - oid_name_length,
                     &(name[oid_name_length]), &pfcPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            SNMP_MGR_BitsFromSnmpToCore(byte_buffer, var_val, var_val_len);

            tmp_pri_en = byte_buffer[0];

            /* set to core layer
             */
            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            pfcPortIndex, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &tmp_pri_en))
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
 ************* pfcPortStatsTable ************
 ********************************************
 */
#define PFCPORTSTATSENTRY_INSTANCE_LEN  1

BOOL_T pfcPortStatsTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *pfcPortStatsIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != PFCPORTSTATSENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *pfcPortStatsIndex = compl[0];

    return TRUE;
}

/*
 * var_pfcPortStatsTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_pfcPortStatsTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0, lport =0, tmp_data;
    SWDRV_PfcStats_T pfc_stats;
    oid compl[PFCPORTSTATSENTRY_INSTANCE_LEN] = {0};
    oid best_inst[PFCPORTSTATSENTRY_INSTANCE_LEN] = {0};

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_pfcPortStatsClearAction:
            *write_method = write_pfcPortStatsClearAction;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        PFCPORTSTATSENTRY_INSTANCE_LEN);

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! pfcPortStatsTable_OidIndexToData(exact, compc, compl, &lport))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (  (TRUE != PFC_PMGR_GetDataByField(
                lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_data))
            ||(! NMTR_PMGR_GetSystemwidePfcStats(lport, &pfc_stats))
           )
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        pfcPortStatsTable_OidIndexToData(exact, compc, compl, &lport);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < PFCPORTSTATSENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (  (TRUE != PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_data))
                ||(! NMTR_PMGR_GetSystemwidePfcStats(lport, &pfc_stats))
               )
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (  (TRUE != PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_NXT_PORT, &lport))
                    ||(! NMTR_PMGR_GetSystemwidePfcStats(lport, &pfc_stats))
                   )
                {
                    return NULL;
                }
            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (  (TRUE != PFC_PMGR_GetDataByField(
                    lport, PFC_TYPE_FLDE_PORT_NXT_PORT, &lport))
                ||(! NMTR_PMGR_GetSystemwidePfcStats(lport, &pfc_stats))
               )
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
    memcpy(name + vp->namelen, best_inst, PFCPORTSTATSENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + PFCPORTSTATSENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_pfcPortStatsIndex:
            *var_len = sizeof(long_return);
            long_return = lport;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_pfcPortStatsSentPri0Pkts:
        case LEAF_pfcPortStatsSentPri1Pkts:
        case LEAF_pfcPortStatsSentPri2Pkts:
        case LEAF_pfcPortStatsSentPri3Pkts:
        case LEAF_pfcPortStatsSentPri4Pkts:
        case LEAF_pfcPortStatsSentPri5Pkts:
        case LEAF_pfcPortStatsSentPri6Pkts:
        case LEAF_pfcPortStatsSentPri7Pkts:
            *var_len = sizeof(long64_return);
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,
                    pfc_stats.pri[vp->magic-LEAF_pfcPortStatsSentPri0Pkts].ieee8021PfcRequests);
            return (u_char *) &long64_return;

        case LEAF_pfcPortStatsRecvPri0Pkts:
        case LEAF_pfcPortStatsRecvPri1Pkts:
        case LEAF_pfcPortStatsRecvPri2Pkts:
        case LEAF_pfcPortStatsRecvPri3Pkts:
        case LEAF_pfcPortStatsRecvPri4Pkts:
        case LEAF_pfcPortStatsRecvPri5Pkts:
        case LEAF_pfcPortStatsRecvPri6Pkts:
        case LEAF_pfcPortStatsRecvPri7Pkts:
            *var_len = sizeof(long64_return);
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,
                    pfc_stats.pri[vp->magic-LEAF_pfcPortStatsRecvPri0Pkts].ieee8021PfcIndications);
            return (u_char *) &long64_return;

        case LEAF_pfcPortStatsClearAction:
            *var_len = sizeof(long_return);
            long_return = VAL_pfcPortStatsClearAction_noClear;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_pfcPortStatsClearAction(int action,
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
                case VAL_pfcPortStatsClearAction_clear:
                    break;

                case VAL_pfcPortStatsClearAction_noClear:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            UI32_T pfcPortStatsIndex = 0, tmp_data;
            I32_T value = 0;

            /* extract index
             */
            if (  (! pfcPortStatsTable_OidIndexToData(TRUE, name_len - oid_name_length,
                     &(name[oid_name_length]), &pfcPortStatsIndex))
                ||(TRUE != PFC_PMGR_GetDataByField(
                    pfcPortStatsIndex, PFC_TYPE_FLDE_PORT_MODE_ADM, &tmp_data))
               )
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            /* set to core layer
             */
            if(VAL_pfcPortStatsClearAction_clear == (*(long *)var_val))
            {
                NMTR_PMGR_ClearSystemwidePfcStats(pfcPortStatsIndex);
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

#endif /* #if (SYS_CPNT_PFC == TRUE) */

