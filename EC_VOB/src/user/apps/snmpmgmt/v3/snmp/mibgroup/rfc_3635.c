#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "rfc_3635.h"
#include "sysORTable.h"
#include "nmtr_mgr.h"
#include "nmtr_pmgr.h"
#include "swdrv_type.h"
#include "sys_type.h"
#include "leaf_3635.h"
#include "snmp_mgr.h"


#define    dot3StatsEntry_INSTANCE_LEN    1
#define    dot3StatsEntry_OID_NAME_LEN    11

/*
 * dot3StatsTable_variables_oid:
 *     This is the top level oid that we want to register under.
 *     This is essentially a prefix, with the suffix appearing in the variable below.
 */
oid dot3StatsTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 10, 7 };
oid dot3PauseTable_variables_oid[] = { 1,3,6,1,2,1,10,7};

/*
 * variable3 dot3StatsTable_variables:
 *     This variable defines function callbacks and type return information for the  mib section
 */
struct variable3 dot3StatsTable_variables[] =
{
    /* magic number                         variable type     ro/rw  callback fn         L  oidsuffix */
    {DOT3STATSINDEX,                        ASN_INTEGER,      RONLY, var_dot3StatsTable, 3, {2, 1, 1}},
    {DOT3STATSALIGNMENTERRORS,              ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 2}},
    {DOT3STATSFCSERRORS,                    ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 3}},
    {DOT3STATSSINGLECOLLISIONFRAMES,        ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 4}},
    {DOT3STATSMULTIPLECOLLISIONFRAMES,      ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 5}},
    {DOT3STATSSQETESTERRORS,                ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 6}},
    {DOT3STATSDEFERREDTRANSMISSIONS,        ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 7}},
    {DOT3STATSLATECOLLISIONS,               ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 8}},
    {DOT3STATSEXCESSIVECOLLISIONS,          ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 9}},
    {DOT3STATSINTERNALMACTRANSMITERRORS,    ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 10}},
    {DOT3STATSCARRIERSENSEERRORS,           ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 11}},
    {DOT3STATSFRAMETOOLONGS,                ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 13}},
    {DOT3STATSINTERNALMACRECEIVEERRORS,     ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 16}},
    {DOT3STATSETHERCHIPSET,                 ASN_OBJECT_ID,    RONLY, var_dot3StatsTable, 3, {2, 1, 17}},
    {DOT3STATSSYMBOLERRORS,                 ASN_COUNTER,      RONLY, var_dot3StatsTable, 3, {2, 1, 18}},
    {DOT3STATSDUPLEXSTATUS,                 ASN_INTEGER,      RONLY, var_dot3StatsTable, 3, {2, 1, 19}},
    {DOT3STATSRATECONTROLABILITY,           ASN_INTEGER,      RONLY, var_dot3StatsTable, 3, {2, 1, 20}},
    {DOT3STATSRATECONTROLSTATUS,            ASN_INTEGER,      RONLY, var_dot3StatsTable, 3, {2, 1, 21}},
};

/*
 * variable3 dot3PauseTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 dot3PauseTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_dot3PauseAdminMode,  ASN_INTEGER,  RWRITE,  var_dot3PauseTable, 3, {10, 1, 1}},
{LEAF_dot3PauseOperMode,  ASN_INTEGER,  RONLY,   var_dot3PauseTable, 3, {10, 1, 2}},
{LEAF_dot3InPauseFrames,  ASN_COUNTER,  RONLY,   var_dot3PauseTable, 3, {10, 1, 3}},
{LEAF_dot3OutPauseFrames,  ASN_COUNTER,  RONLY,   var_dot3PauseTable, 3, {10, 1, 4}},
};

/* Initializes the dot3StatsTable module
 */
void init_dot3StatsTable(void)
{
    oid etherlike_module_oid[] = { SNMP_OID_MIB2, 10, 7 };

    /* Register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("dot3StatsTable", dot3StatsTable_variables, variable3, dot3StatsTable_variables_oid);

    REGISTER_SYSOR_ENTRY(etherlike_module_oid, "Rfc3635 - The EthernetLike MIB.");
}


static BOOL_T dot3StatsTable_get(int                       compc,
                                 oid                       *compl,
                                 SWDRV_EtherlikeStats_T    *data,
                                 UI32_T                    *ifindex)
{
    /* Check the length of index
     */
    if (compc != dot3StatsEntry_INSTANCE_LEN)
        return FALSE;

    *ifindex = compl[0];

    /* Get the data
     */
    if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
        ! NMTR_PMGR_GetSystemwideEtherLikeStats(*ifindex, data)
#else
        ! NMTR_PMGR_GetEtherLikeStats(*ifindex, data)
#endif
        )
        return FALSE;

    return TRUE;
}


static BOOL_T dot3StatsTable_next(int                       compc,
                                  oid                       *compl,
                                  SWDRV_EtherlikeStats_T    *data,
                                  UI32_T                    *ifindex)
{
    oid tmp_compl[dot3StatsEntry_INSTANCE_LEN];

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_dot3StatsIndex);
    SNMP_MGR_ConvertRemainToZero(compc, dot3StatsEntry_INSTANCE_LEN, tmp_compl);

    *ifindex = tmp_compl[0];

    /* Get the first entry
     */
    if (compc < dot3StatsEntry_INSTANCE_LEN)
    {
        if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
            ! NMTR_PMGR_GetSystemwideEtherLikeStats(*ifindex, data)
#else
            ! NMTR_PMGR_GetEtherLikeStats(*ifindex, data)
#endif
            )
        {
            if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
                ! NMTR_PMGR_GetNextSystemwideEtherLikeStats(ifindex, data)
#else
                ! NMTR_PMGR_GetNextEtherLikeStats(ifindex, data)
#endif
                )
            {
                return FALSE;
            }
        }
    }

    /* Get the next entry
     */
    else
    {
        if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
            ! NMTR_PMGR_GetNextSystemwideEtherLikeStats(ifindex, data)
#else
            ! NMTR_PMGR_GetNextEtherLikeStats(ifindex, data)
#endif
            )
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*
 * var_dot3StatsTable():
 *     Handle this table separately from the scalar value case.
 *     The workings of this are basically the same as for var_ above.
 */
unsigned char* var_dot3StatsTable(struct variable    *vp,
                                  oid                *name,
                                  size_t             *length,
                                  int                exact,
                                  size_t             *var_len,
                                  WriteMethod        **write_method)
{   UI32_T ifindex = 0;
    UI32_T compc = 0;
    oid compl[dot3StatsEntry_INSTANCE_LEN];
    oid best_inst[dot3StatsEntry_INSTANCE_LEN];
    SWDRV_EtherlikeStats_T entry;

    memset(&entry, 0, sizeof(SWDRV_EtherlikeStats_T));

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, dot3StatsEntry_INSTANCE_LEN);

    /* Get, set
     */
    if (exact)
    {
        if (!dot3StatsTable_get(compc, compl, &entry, &ifindex))
            return NULL;
    }

    /* Getnext
     */
    else
    {
        if (!dot3StatsTable_next(compc, compl, &entry, &ifindex))
            return NULL;
    }

    best_inst[0] = ifindex;
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, dot3StatsEntry_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + dot3StatsEntry_INSTANCE_LEN;
    *write_method = 0;
    *var_len = sizeof(long_return);

    /* This is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case DOT3STATSINDEX:
            long_return = ifindex;
            return (u_char*) &long_return;
        case DOT3STATSALIGNMENTERRORS:
            long_return = entry.dot3StatsAlignmentErrors;
            return (u_char*) &long_return;
        case DOT3STATSFCSERRORS:
            long_return = entry.dot3StatsFCSErrors;
            return (u_char*) &long_return;
        case DOT3STATSSINGLECOLLISIONFRAMES:
            long_return = entry.dot3StatsSingleCollisionFrames;
            return (u_char*) &long_return;
        case DOT3STATSMULTIPLECOLLISIONFRAMES:
            long_return = entry.dot3StatsMultipleCollisionFrames;
            return (u_char*) &long_return;
        case DOT3STATSSQETESTERRORS:
            long_return = entry.dot3StatsSQETestErrors;
            return (u_char*) &long_return;
        case DOT3STATSDEFERREDTRANSMISSIONS:
            long_return = entry.dot3StatsDeferredTransmissions;
            return (u_char*) &long_return;
        case DOT3STATSLATECOLLISIONS:
            long_return = entry.dot3StatsLateCollisions;
            return (u_char*) &long_return;
        case DOT3STATSEXCESSIVECOLLISIONS:
            long_return = entry.dot3StatsExcessiveCollisions;
            return (u_char*) &long_return;
        case DOT3STATSINTERNALMACTRANSMITERRORS:
            long_return = entry.dot3StatsInternalMacTransmitErrors;
            return (u_char*) &long_return;
        case DOT3STATSCARRIERSENSEERRORS:
            long_return = entry.dot3StatsCarrierSenseErrors;
            return (u_char*) &long_return;
        case DOT3STATSFRAMETOOLONGS:
            long_return = entry.dot3StatsFrameTooLongs;
            return (u_char*) &long_return;
        case DOT3STATSINTERNALMACRECEIVEERRORS:
            long_return = entry.dot3StatsInternalMacReceiveErrors;
            return (u_char*) &long_return;
        case DOT3STATSETHERCHIPSET:
            *var_len = 2* sizeof( oid);
            return (u_char*) &nullOid;
        case DOT3STATSSYMBOLERRORS:
            long_return = entry.dot3StatsSymbolErrors;
            return (u_char*) &long_return;
        case DOT3STATSDUPLEXSTATUS:
            long_return = entry.dot3StatsDuplexStatus;
            return (u_char*) &long_return;
        case DOT3STATSRATECONTROLABILITY:
            long_return = entry.dot3StatsRateControlAbility;
            return (u_char*) &long_return;
        case DOT3STATSRATECONTROLSTATUS:
            long_return = entry.dot3StatsRateControlStatus;
            return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }

    return NULL;
}


#define    dot3HCStatsEntry_INSTANCE_LEN    1
#define    dot3HCStatsEntry_OID_NAME_LEN    11


/*
 * dot3HCStatsTable_variables_oid:
 *     This is the top level oid that we want to register under.
 *     This is essentially a prefix, with the suffix appearing in the variable below.
 */
oid dot3HCStatsTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 10, 7 };


/*
 * variable3 dot3HCStatsTable_variables:
 *     This variable defines function callbacks and type return information for the  mib section
 */
struct variable3 dot3HCStatsTable_variables[] =
{
    /* magic number                           variable type  ro/rw  callback fn           L  oidsuffix */
    {DOT3HCSTATSALIGNMENTERRORS,              ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 1}},
    {DOT3HCSTATSFCSERRORS,                    ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 2}},
    {DOT3HCSTATSINTERNALMACTRANSMITERRORS,    ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 3}},
    {DOT3HCSTATSFRAMETOOLONGS,                ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 4}},
    {DOT3HCSTATSINTERNALMACRECEIVEERRORS,     ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 5}},
    {DOT3HCSTATSSYMBOLERRORS,                 ASN_COUNTER64, RONLY, var_dot3HCStatsTable, 3, {11, 1, 6}},
};


/* Initializes the dot3HCStatsTable module
 */
void init_dot3HCStatsTable(void)
{
    /* Register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("dot3HCStatsTable", dot3HCStatsTable_variables, variable3, dot3HCStatsTable_variables_oid);
}


static BOOL_T dot3HCStatsTable_get(int                       compc,
                                   oid                       *compl,
                                   SWDRV_EtherlikeStats_T    *data,
                                   UI32_T                    *ifindex)
{
    /* Check the length of index
     */
    if (compc != dot3HCStatsEntry_INSTANCE_LEN)
        return FALSE;

    *ifindex = compl[0];

    /* Get the data
     */
    if (!NMTR_PMGR_GetEtherLikeStats(*ifindex, data))
        return FALSE;

    return TRUE;
}


static BOOL_T dot3HCStatsTable_next(int                         compc,
                                    oid                         *compl,
                                    SWDRV_EtherlikeStats_T    *data,
                                    UI32_T                      *ifindex)
{
    oid tmp_compl[dot3HCStatsEntry_INSTANCE_LEN];

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_dot3StatsIndex);
    SNMP_MGR_ConvertRemainToZero(compc, dot3HCStatsEntry_INSTANCE_LEN, tmp_compl);

    *ifindex = tmp_compl[0];

    /* Get the first entry
     */
    if (compc < dot3HCStatsEntry_INSTANCE_LEN)
    {
        if (!NMTR_PMGR_GetEtherLikeStats(*ifindex, data))
        {
            if (!NMTR_PMGR_GetNextEtherLikeStats(ifindex, data))
                return FALSE;
        }
    }

    /* Get the next entry
     */
    else
    {
        if (!NMTR_PMGR_GetNextEtherLikeStats(ifindex, data))
            return FALSE;
    }

    return TRUE;
}


/*
 * var_dot3HCStatsTable():
 *     Handle this table separately from the scalar value case.
 *     The workings of this are basically the same as for var_ above.
 */
unsigned char* var_dot3HCStatsTable(struct variable    *vp,
                                    oid                *name,
                                    size_t             *length,
                                    int                exact,
                                    size_t             *var_len,
                                    WriteMethod        **write_method)
{
    UI32_T ifindex = 0;
    UI32_T compc = 0;
    oid compl[dot3HCStatsEntry_INSTANCE_LEN];
    oid best_inst[dot3HCStatsEntry_INSTANCE_LEN];
    SWDRV_EtherlikeStats_T entry;

    memset(&entry, 0, sizeof(SWDRV_EtherlikeStats_T));

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, dot3HCStatsEntry_INSTANCE_LEN);

    /* Get, set
     */
    if (exact)
    {
        if (!dot3HCStatsTable_get(compc, compl, &entry, &ifindex))
            return NULL;
    }

    /* Getnext
     */
    else
    {
        if (!dot3HCStatsTable_next(compc, compl, &entry, &ifindex))
            return NULL;
    }

    best_inst[0] = ifindex;
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, dot3HCStatsEntry_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + dot3HCStatsEntry_INSTANCE_LEN;
    *write_method = 0;
    *var_len = sizeof(long64_return);

    /* This is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case DOT3HCSTATSALIGNMENTERRORS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsAlignmentErrors);
            return (u_char *) & long64_return;
        case DOT3HCSTATSFCSERRORS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsFCSErrors);
            return (u_char *) & long64_return;
        case DOT3HCSTATSINTERNALMACTRANSMITERRORS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsInternalMacTransmitErrors);
            return (u_char *) & long64_return;
        case DOT3HCSTATSFRAMETOOLONGS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsFrameTooLongs);
            return (u_char *) & long64_return;
        case DOT3HCSTATSINTERNALMACRECEIVEERRORS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsInternalMacReceiveErrors);
            return (u_char *) & long64_return;
        case DOT3HCSTATSSYMBOLERRORS:
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return,entry.dot3StatsSymbolErrors);
            return (u_char *) & long64_return;
        default:
            ERROR_MSG("");
    }

    return NULL;
}

/** Initializes the dot3PauseTable module */
void
init_dot3PauseTable(void)
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("dot3PauseTable", dot3PauseTable_variables, variable3,
                 dot3PauseTable_variables_oid);
}

#define DOT3PAUSEENTRY_INSTANCE_LEN  1

BOOL_T dot3PauseTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl,  UI32_T *dot3StatsIndex)
{

    /* get or write */
    if (exact)
    {
        /* check the index length */
        if (compc != DOT3PAUSEENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot3StatsIndex = compl[0];

    return TRUE;
}

/*
 * var_dot3PauseTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_dot3PauseTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc = 0;
    UI32_T nIndex = 0;
    oid compl[DOT3PAUSEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT3PAUSEENTRY_INSTANCE_LEN] = {0};
    SWDRV_EtherlikePause_T entry;

    switch (vp->magic)
    {
        case LEAF_dot3PauseAdminMode:
            *write_method = write_dot3PauseAdminMode;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,
                           compl, DOT3PAUSEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get,set */
    {
        /* get index */
        if (dot3PauseTable_OidIndexToData(exact, compc, compl,  &nIndex) == FALSE)
        {
            return NULL;
        }

        /* get data */
        if (NMTR_PMGR_GetEtherLikePause(nIndex, &entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        /* get index */
        dot3PauseTable_OidIndexToData(exact, compc, compl,  &nIndex);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            /* get data */
            if (NMTR_PMGR_GetEtherLikePause(nIndex, &entry) != TRUE)
            {
                /* get next data */
                if (NMTR_PMGR_GetNextEtherLikePause(&nIndex, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            /* get next data */
            if (NMTR_PMGR_GetNextEtherLikePause(&nIndex, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index */
    best_inst[0] = nIndex;
    memcpy(name + vp->namelen, best_inst,
           DOT3PAUSEENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT3PAUSEENTRY_INSTANCE_LEN;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case LEAF_dot3PauseAdminMode:
            *var_len = sizeof(long_return);
            long_return = entry.dot3PauseAdminMode;
            return (u_char *) &long_return;

        case LEAF_dot3PauseOperMode:
            *var_len = sizeof(long_return);
            long_return = entry.dot3PauseOperMode;
            return (u_char *) &long_return;

        case LEAF_dot3InPauseFrames:
            *var_len = sizeof(long_return);
            long_return = entry.dot3InPauseFrames;
            return (u_char *) &long_return;

        case LEAF_dot3OutPauseFrames:
            *var_len = sizeof(long_return);
            long_return = entry.dot3OutPauseFrames;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int
write_dot3PauseAdminMode(int      action,
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
                case VAL_dot3PauseAdminMode_disabled:
                    break;

                case VAL_dot3PauseAdminMode_enabledXmit:
                    break;

                case VAL_dot3PauseAdminMode_enabledRcv:
                    break;

                case VAL_dot3PauseAdminMode_enabledXmitAndRcv:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 11;
            I32_T value = 0;
            UI32_T nIndex = 0;

            if (dot3PauseTable_OidIndexToData(TRUE, name_len - oid_name_length, &(name[oid_name_length]),  &nIndex) == FALSE)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            value = *(long *)var_val;

            if (NMTR_PMGR_SetEtherLikePauseAdminMode(nIndex, value) != TRUE)
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
