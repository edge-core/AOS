/* Module Name: ieee_8021ag.c
 * Purpose: 802.1ag MIB function
 * Notes:
 * History:
 *    2007/4/14 - Daniel, Created
 * Copyright(C)      Accton Corporation, 2007
 */


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ieee_8021ag.h"
#include "cfm_pmgr.h"
#include "cfm_om.h"
#include "sysORTable.h"
#include "sys_type.h"
#include "leaf_ieee8021ag.h"
#include "snmp_mgr.h"

#if(SYS_CPNT_CFM==TRUE)

#define SNMP_OID_IEEE802DOT1AG 1,3,111,2,802,1,1,8 //1,0,8802,1,1,3
#define SNMP_OID_IEEE802DOT1AG_OBJECT SNMP_OID_IEEE802DOT1AG,1
#define SNMP_OIDLEN_IEEE802DOT1AG_OBJECT 9 //7


/* ======Support Group========== */
#define SNMP_CFM_STACK_GROUP_SUPPORT TRUE
#define SNMP_CFM_CONFIG_ERRIR_LIST_GROUP_SUPPORT TRUE
#define SNMP_CFM_MD_GROUP_SUPPORT TRUE
#define SNMP_CFM_MA_GROUP_SUPPORT TRUE
#define SNMP_CFM_MEP_GROUP_SUPPORT TRUE
/* =========================== */

/*  @@==================
 *   dot1agCfmStackTable  (1)
 *  ==================== */

#if (SNMP_CFM_STACK_GROUP_SUPPORT==TRUE)
/*
 * dot1agCfmStack_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid dot1agCfmStackTable_variables_oid[] = { SNMP_OID_IEEE802DOT1AG_OBJECT, 1, 1 };
/*
 * variable3 dot1agCfmStack_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 dot1agCfmStackTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
    {DOT1AGCFMSTACKMDINDEX,  ASN_UNSIGNED,  RONLY,   var_dot1agCfmStackTable, 2,  { 1, 5 }},
    {DOT1AGCFMSTACKMAINDEX,  ASN_UNSIGNED,  RONLY,   var_dot1agCfmStackTable, 2,  { 1, 6 }},
    {DOT1AGCFMSTACKMEPID,  ASN_UNSIGNED,  RONLY,   var_dot1agCfmStackTable, 2,  { 1, 7 }},
    {DOT1AGCFMSTACKMACADDRESS,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmStackTable, 2,  { 1, 8 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the dot1agCfmStack module */
void
init_dot1agCfmStackTable(void)
{

    DEBUGMSGTL(("dot1agCfmStack", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("dot1agCfmStack", dot1agCfmStackTable_variables, variable3,
               dot1agCfmStackTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define DOT1AGCFMSTACKENTRY_INSTANCE_LEN  4

BOOL_T dot1agCfmStackTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmStackifIndex, UI32_T *dot1agCfmStackVlanIdOrNone, UI32_T *dot1agCfmStackMdLevel, UI32_T *dot1agCfmStackDirection)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMSTACKENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmStackifIndex = compl[0];
    *dot1agCfmStackVlanIdOrNone = compl[1];
    *dot1agCfmStackMdLevel = compl[2];
    *dot1agCfmStackDirection = compl[3];

    return TRUE;
}


unsigned char *
var_dot1agCfmStackTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMSTACKENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMSTACKENTRY_INSTANCE_LEN] = {0};
    CFM_OM_MepInfo_T entry;

    UI32_T stack_ifindex, level, stack_direction, vid;
    UI16_T nxt_vid;
    CFM_TYPE_MdLevel_T nxt_level;
    CFM_TYPE_MP_Direction_T nxt_direction;

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMSTACKENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (dot1agCfmStackTable_OidIndexToData(exact, compc, compl, &stack_ifindex, &vid, &level, &stack_direction) == FALSE)
        {
            return NULL;
        }
        nxt_vid = vid;
        nxt_level=level;
        nxt_direction = stack_direction;

        if (FALSE == CFM_OM_GetStackEntry(stack_ifindex, vid, level, stack_direction, &entry))
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        dot1agCfmStackTable_OidIndexToData(exact, compc, compl, &stack_ifindex, &vid, &level, &stack_direction) ;
        nxt_vid = vid;
        nxt_level=level;
        nxt_direction = stack_direction;
        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (FALSE == CFM_OM_GetStackEntry(stack_ifindex, vid, level, stack_direction, &entry))
            {
                if (CFM_OM_GetNextStackEntry(&stack_ifindex, &nxt_vid, &nxt_level, &nxt_direction, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextStackEntry(&stack_ifindex, &nxt_vid, &nxt_level, &nxt_direction, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = stack_ifindex;
    best_inst[1] = nxt_vid;
    best_inst[2] = nxt_level;
    best_inst[3] = nxt_direction;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMSTACKENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMSTACKENTRY_INSTANCE_LEN;


    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmStackifIndex:
            *var_len = sizeof(long_return);
            long_return =stack_ifindex;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmStackVlanIdOrNone:
            *var_len = sizeof(long_return);
            long_return = nxt_vid;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmStackMdLevel:
            *var_len = sizeof(long_return);
            long_return = nxt_level;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmStackDirection:
            *var_len = sizeof(long_return);
            long_return = nxt_direction;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_dot1agCfmStackMdIndex:
            *var_len = sizeof(long_return);
            long_return = entry.md_index;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmStackMaIndex:
            *var_len = sizeof(long_return);
            long_return = entry.ma_index;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmStackMepId:
            *var_len = sizeof(long_return);
            long_return = entry.identifier;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmStackMacAddress:
            memcpy(return_buf,  entry.mac_addr_a, SIZE_dot1agCfmStackMacAddress);
            *var_len = SIZE_dot1agCfmStackMacAddress;
            return (u_char *) return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}
#endif


/*  @@==================
 *   dot1agCfmConfigErrorList  (4)
 *  ==================== */
#if (SNMP_CFM_CONFIG_ERRIR_LIST_GROUP_SUPPORT==TRUE)

oid dot1agCfmConfigErrorListTable_variables_oid[] = { SNMP_OID_IEEE802DOT1AG_OBJECT, 4, 1 };
/*
 * variable3 dot1agCfmConfigErrorList_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 dot1agCfmConfigErrorListTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
    {DOT1AGCFMCONFIGERRORLISTERRORTYPE,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmConfigErrorListTable, 2,  { 1, 3 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the dot1agCfmConfigErrorList module */
void
init_dot1agCfmConfigErrorListTable(void)
{

    DEBUGMSGTL(("dot1agCfmConfigErrorList", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("dot1agCfmConfigErrorList", dot1agCfmConfigErrorListTable_variables, variable3,
               dot1agCfmConfigErrorListTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN  2

BOOL_T dot1agCfmConfigErrorListTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmConfigErrorListVid, UI32_T *dot1agCfmConfigErrorListIfIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmConfigErrorListVid= compl[0];
    *dot1agCfmConfigErrorListIfIndex = compl[1];

    return TRUE;
}

unsigned char *
var_dot1agCfmConfigErrorListTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN] = {0};
    CFM_OM_Error_T entry;
    UI32_T vid, ifindex;
    UI16_T nxt_vid;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (dot1agCfmConfigErrorListTable_OidIndexToData(exact, compc, compl, &vid, &ifindex) == FALSE)
        {
            return NULL;
        }
        nxt_vid = vid;

        if (CFM_OM_GetErrorInfo(vid, ifindex, &entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        dot1agCfmConfigErrorListTable_OidIndexToData(exact, compc, compl, &vid, &ifindex);
        nxt_vid = vid;

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (CFM_OM_GetErrorInfo(nxt_vid, ifindex, &entry) != TRUE)
            {
                if (CFM_OM_GetNextErrorInfo(&nxt_vid, &ifindex, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextErrorInfo(&nxt_vid, &ifindex, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = nxt_vid;
    best_inst[1] = ifindex;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMCONFIGERRORLISTENTRY_INSTANCE_LEN;

    /*
   * this is where we do the value assignments for the mib results.
   */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmConfigErrorListVid:
            *var_len = sizeof(long_return);
             long_return = nxt_vid;
            return (u_char*)long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmConfigErrorListIfIndex:
            *var_len = sizeof(long_return);
            long_return = ifindex;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_dot1agCfmConfigErrorListErrorType:
            *var_len = SIZE_dot1agCfmConfigErrorListErrorType;
            return_buf[0] =entry.reason_bit_map;
            return (u_char *) return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}
#endif



/*  @@==================
 *   dot1agCfmMdGroup  (5)
 *  ==================== */
#if (SNMP_CFM_MD_GROUP_SUPPORT==TRUE)
void
init_dot1agCfmMd(void)
{
    oid dot1agCfmMd_module_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 5};
    oid dot1agCfmMdTableNextIndex_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 5, 1,0};

    DEBUGMSGTL(("ieee8021cfm", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                    ("dot1agCfmMdTableNextIndex",
                                    get_dot1agCfmMdTableNextIndex,
                                    dot1agCfmMdTableNextIndex_oid,
                                    OID_LENGTH(dot1agCfmMdTableNextIndex_oid),
                                    HANDLER_CAN_RONLY));

    REGISTER_SYSOR_ENTRY(dot1agCfmMd_module_oid,
                                    "The MIB module to describe ieee8021cfm MIB.");
}

int
get_dot1agCfmMdTableNextIndex(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info *reqinfo,
                          netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    UI32_T cfmMdTableNextIndex;


    if ((cfmMdTableNextIndex=CFM_OM_GetNextAvailableMdIndex())==0)
        return SNMP_ERR_GENERR;

  //  printf("cfmMdTableNextIndex: %lu\n", cfmMdTableNextIndex);

    switch(reqinfo->mode)
    {
        case MODE_GET:
            long_return = cfmMdTableNextIndex;
            snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED, (u_char *) &long_return, sizeof(long_return));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

/* === dot1agCfmMdTable === */
oid dot1agCfmMdTable_variables_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 5, 2};

struct variable3 dot1agCfmMdTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {DOT1AGCFM_MD_FORMAT, ASN_INTEGER, RWRITE, var_dot1agCfmMdTable, 2, {1, 2}},
    {DOT1AGCFM_MD_NAME, ASN_OCTET_STR, RWRITE, var_dot1agCfmMdTable, 2, {1, 3}},
    {DOT1AGCFM_MD_MDLEVEL, ASN_INTEGER, RWRITE, var_dot1agCfmMdTable, 2, {1, 4}},
    {DOT1AGCFM_MD_MHF_CREATION, ASN_INTEGER, RWRITE, var_dot1agCfmMdTable, 2, {1, 5}},
    {DOT1AGCFM_MD_MHFID_PERMISSION, ASN_INTEGER, RWRITE, var_dot1agCfmMdTable, 2, {1, 6}},
    {DOT1AGCFM_MD_MATABLE_NEXTINDEX, ASN_UNSIGNED, RONLY, var_dot1agCfmMdTable, 2, {1, 7}},
    {DOT1AGCFM_MD_ROWSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMdTable, 2, {1, 8}},
};


/** Initializes the ieee_8021ag module */
void init_dot1agCfmMdTable(void)
{
    DEBUGMSGTL(("dot1agCfmMdTable", "Initializing\n"));
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("dot1agCfmMdTable",
                 dot1agCfmMdTable_variables, variable3,
                 dot1agCfmMdTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

#define DOT1AGCFMMDENTRY_INSTANCE_LEN  1

BOOL_T dot1agCfmMdTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMMDENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];

    return TRUE;
}

unsigned char *
var_dot1agCfmMdTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMMDENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMMDENTRY_INSTANCE_LEN] = {0};
    CFM_OM_MdInfo_T entry;
    UI32_T md_index;

    switch(vp->magic)
    {
        case DOT1AGCFM_MD_FORMAT:
            *write_method = write_dot1agCfmMdFormat;
            break;
        case DOT1AGCFM_MD_NAME:
            *write_method = write_dot1agCfmMdName;
            break;
        case DOT1AGCFM_MD_MDLEVEL:
            *write_method = write_dot1agCfmMdMdLevel;
            break;
        case DOT1AGCFM_MD_MHF_CREATION:
            *write_method = write_dot1agCfmMdMhfCreation;
            break;
        case DOT1AGCFM_MD_MHFID_PERMISSION:
            *write_method = write_dot1agCfmMdMhfIdPermission;
            break;
        case DOT1AGCFM_MD_ROWSTATUS:
            *write_method = write_dot1agCfmMdStatus;
            break;
        case DOT1AGCFM_MD_MATABLE_NEXTINDEX:
        default:
            *write_method =0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMMDENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (dot1agCfmMdTable_OidIndexToData(exact, compc, compl, &md_index) == FALSE)
        {
            return NULL;
        }

        if (CFM_OM_GetMdInfo(md_index, &entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        dot1agCfmMdTable_OidIndexToData(exact, compc, compl, &md_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (CFM_OM_GetMdInfo(md_index, &entry) != TRUE)
            {
                if (CFM_OM_GetNextMdInfo(&md_index, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextMdInfo(&md_index, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMMDENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMMDENTRY_INSTANCE_LEN;

    /*
   * this is where we do the value assignments for the mib results.
   */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmMdIndex:
            *var_len = sizeof(long_return);
            long_return = md_index;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_dot1agCfmMdFormat:
            *var_len = sizeof(long_return);
            long_return = entry.name_format;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMdName:
            *var_len = entry.name_len;
            memcpy(return_buf, entry.name_a, *var_len);
            return (u_char *) return_buf;

        case LEAF_dot1agCfmMdMdLevel:
            *var_len = sizeof(long_return);
            long_return = entry.level;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMdMhfCreation:
            *var_len = sizeof(long_return);
            long_return = entry.mhf_creation;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMdMhfIdPermission:
            *var_len = sizeof(long_return);
            long_return = entry.permission;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMdMaTableNextIndex:
            *var_len = sizeof(long_return);
            long_return = entry.next_index;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMdRowStatus:
            *var_len = sizeof(long_return);
            long_return = entry.row_status;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}

int
write_dot1agCfmMdFormat(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMdFormat_none) || (value > VAL_dot1agCfmMdFormat_charString))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];

            if(CFM_PMGR_SetDot1agCfmMdFormat(mdIndex, (CFM_TYPE_MD_Name_T) value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMdName(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            //printf(" ----- string name (%s)in var_var_len: %d -----\n" , var_val, var_val_len);
            if(var_val_len < MINSIZE_dot1agCfmMdName || var_val_len > MAXSIZE_dot1agCfmMdName)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        }

        case RESERVE2:
        {

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            mdIndex = name[oid_name_length];
            if(CFM_PMGR_SetDot1agCfmMdName(mdIndex, (char *)var_val, var_val_len)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMdMdLevel(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < MIN_dot1agCfmMdMdLevel) || (value > MAX_dot1agCfmMdMdLevel))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];

            if(CFM_PMGR_SetDot1agCfmMdLevel(mdIndex, (CFM_TYPE_MdLevel_T) value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMdMhfCreation(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMdMhfCreation_defMHFnone) || (value > VAL_dot1agCfmMdMhfCreation_defMHFdefer))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];

            if(CFM_PMGR_SetDot1agCfmMdMhfCreation(mdIndex, (CFM_TYPE_MhfCreation_T) value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMdMhfIdPermission(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMdMhfIdPermission_sendIdNone) || (value > VAL_dot1agCfmMdMhfIdPermission_sendIdDefer))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];

            if(CFM_PMGR_SetDot1agCfmMdMhfIdPermission(mdIndex, (CFM_TYPE_MhfIdPermission_T) value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMdStatus(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    CFM_OM_MdInfo_T entry;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+1)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMdRowStatus_active) || (value > VAL_dot1agCfmMdRowStatus_destroy))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            if(CFM_OM_GetMdInfo(mdIndex, &entry)==TRUE)
            {
                /*We only set destroy to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_destroy)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_DeleteDot1agCfmMd(mdIndex) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                /*We only set createAndGo to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_createAndGo)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_SetDot1agCfmMd(mdIndex) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}
#endif

/*  @@==================
 *   dot1agCfmMdGroup  (6)
 *  ==================== */

#if (SNMP_CFM_MA_GROUP_SUPPORT==TRUE)
/* === dot1agCfmMaTable === */
oid dot1agCfmMaTable_variables_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 6, 1};

struct variable3 dot1agCfmMaTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {DOT1AGCFMMAPRIMARYVLANID, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 2}},
    {DOT1AGCFMMAFORMAT, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 3}},
    {DOT1AGCFMMANAME, ASN_OCTET_STR, RWRITE, var_dot1agCfmMaTable, 2, {1, 4}},
    {DOT1AGCFMMAMHFCREATION, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 5}},
    {DOT1AGCFMMAIDPERMISSION, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 6}},
    {DOT1AGCFMMACCMINTERVAL, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 7}},
    {DOT1AGCFMMANUMBEROFVIDS, ASN_UNSIGNED, RWRITE, var_dot1agCfmMaTable, 2, {1, 8}},
    {DOT1AGCFMMAROWSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMaTable, 2, {1, 9}},
};


/** Initializes init_dot1agCfmMaTable*/
void init_dot1agCfmMaTable(void)
{
    DEBUGMSGTL(("dot1agCfmMdTable", "Initializing\n"));
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("dot1agCfmMaTable",
                 dot1agCfmMaTable_variables, variable3,
                 dot1agCfmMaTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

#define DOT1AGCFMMAENTRY_INSTANCE_LEN  2

BOOL_T dot1agCfmMaTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex, UI32_T *dot1agCfmMaIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMMAENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];
    *dot1agCfmMaIndex = compl[1];

    return TRUE;
}


unsigned char *
var_dot1agCfmMaTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMMAENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMMAENTRY_INSTANCE_LEN] = {0};
    UI32_T md_index, ma_index;
    CFM_OM_MaInfo_T entry;
    CFM_OM_MdInfo_T md_info;

    switch(vp->magic)
    {
        case DOT1AGCFMMAPRIMARYVLANID:
        case DOT1AGCFMMAFORMAT:
        case DOT1AGCFMMANAME:
        case DOT1AGCFMMAMHFCREATION:
        case DOT1AGCFMMAIDPERMISSION:
        case DOT1AGCFMMACCMINTERVAL:
        case DOT1AGCFMMAROWSTATUS:
            *write_method = write_dot1agCfmMaTable;
            break;
        case DOT1AGCFMMANUMBEROFVIDS:
        default:
            *write_method =0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMMAENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (dot1agCfmMaTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index) == FALSE)
        {
            return NULL;
        }

        if (CFM_OM_GetMaInfo(md_index, ma_index, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else /* getnext */
    {
        dot1agCfmMaTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (CFM_OM_GetMaInfo(md_index, ma_index, &entry) != TRUE)
            {
                if (CFM_OM_GetNextMaInfo(md_index, &ma_index, &entry) != TRUE)
                {
                    if(FALSE == CFM_OM_GetNextMdInfo(&md_index, &md_info))
                    {
                        return NULL;
                    }

                    ma_index=0;
                    if (CFM_OM_GetNextMaInfo(md_index, &ma_index, &entry) != TRUE)
                    {
                        return NULL;
                    }
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextMaInfo(md_index, &ma_index, &entry) != TRUE)
            {
                if(FALSE == CFM_OM_GetNextMdInfo(&md_index, &md_info))
                {
                    return NULL;
                }

                ma_index=0;
                if (CFM_OM_GetNextMaInfo(md_index, &ma_index, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    best_inst[1] = ma_index;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMMAENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMMAENTRY_INSTANCE_LEN;

    /*
   * this is where we do the value assignments for the mib results.
   */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmMaIndex:
            *var_len = sizeof(long_return);
            long_return = entry.ma_index;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_dot1agCfmMaPrimaryVlanId:
            *var_len = sizeof(long_return);
            long_return = entry.primary_vid;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaFormat:
            *var_len = sizeof(long_return);
            long_return = entry.name_format;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaName:
            strncpy((char *) return_buf, (char *)entry.ma_name_a, entry.ma_name_len);
            *var_len = entry.ma_name_len;
            return (u_char *) return_buf;

        case LEAF_dot1agCfmMaMhfCreation:
            *var_len = sizeof(long_return);
            long_return = entry.mhf_creation;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaIdPermission:
            *var_len = sizeof(long_return);
            long_return = entry.permission;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaCcmInterval:
            *var_len = sizeof(long_return);
            long_return = entry.interval;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaNumberOfVids:
            *var_len = sizeof(long_return);
            long_return = entry.num_of_vids;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMaRowStatus:
            *var_len = sizeof(long_return);
            long_return = entry.row_status;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}


int
write_dot1agCfmMaTable(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    CFM_OM_MaInfo_T entry;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+2)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            switch (name[oid_name_length-1])  // magic
            {
                case DOT1AGCFMMAPRIMARYVLANID:
                case DOT1AGCFMMAFORMAT:
                case DOT1AGCFMMAMHFCREATION:
                case DOT1AGCFMMAIDPERMISSION:
                case DOT1AGCFMMACCMINTERVAL:
                case DOT1AGCFMMAROWSTATUS:
                {
                    if(var_val_type != ASN_INTEGER)
                        return SNMP_ERR_WRONGTYPE;
                    if(var_val_len != sizeof(long))
                        return SNMP_ERR_WRONGLENGTH;

                    break;
                }
                case DOT1AGCFMMANAME:
                {
                    if(var_val_type != ASN_OCTET_STR)
                        return SNMP_ERR_WRONGTYPE;
                    if(var_val_len < MINSIZE_dot1agCfmMaName || var_val_len > MAXSIZE_dot1agCfmMaName)
                        return SNMP_ERR_WRONGLENGTH;

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
                    /* bcz PMGR has buffer issue
                     */
                    if(var_val_len > CFM_TYPE_MA_MAX_NAME_LENGTH)
                        return SNMP_ERR_COMMITFAILED;
#endif

                    break;
                }
                default:
                    break;
            }
            break;
        }
        case RESERVE2:
        {
            switch (name[oid_name_length-1])  // magic
            {
                case DOT1AGCFMMAPRIMARYVLANID:
                {
                    value = *(long *)var_val;
                    if((value < 0) || (value > 4094))   /* syntax: VlanIdOrNone */
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMAFORMAT:
                {
                    value = *(long *)var_val;
                    if((value < VAL_dot1agCfmMaFormat_primaryVid) || (value > VAL_dot1agCfmMaFormat_rfc2865VpnId))
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMAMHFCREATION:
                {
                    value = *(long *)var_val;
                    if((value < VAL_dot1agCfmMaMhfCreation_defMHFnone) || (value > VAL_dot1agCfmMaMhfCreation_defMHFdefer))
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMAIDPERMISSION:
                {
                    value = *(long *)var_val;
                    if((value < VAL_dot1agCfmMaIdPermission_sendIdNone) || (value > VAL_dot1agCfmMaIdPermission_sendIdDefer))
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMACCMINTERVAL:
                {
                    value = *(long *)var_val;
                    if((value < VAL_dot1agCfmMaCcmInterval_intervalInvalid) || (value > VAL_dot1agCfmMaCcmInterval_interval10min))
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMAROWSTATUS:
                {
                    value = *(long *)var_val;
                    if((value < VAL_dot1agCfmMaRowStatus_active) || (value > VAL_dot1agCfmMaRowStatus_destroy))
                        return SNMP_ERR_WRONGVALUE;
                    break;
                }
                case DOT1AGCFMMANAME:
                default:

                    break;
            }
            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex, maIndex;

            if (var_val_type == ASN_INTEGER)
                value = *(long *)var_val;

            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];

            switch (name[oid_name_length-1])  // magic
            {
                case DOT1AGCFMMAPRIMARYVLANID:
                {
                    if(CFM_PMGR_SetDot1agCfmMaPrimaryVlanVid(mdIndex, maIndex, (UI16_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMAFORMAT:
                {
                    if(CFM_PMGR_SetDot1agCfmMaFormat(mdIndex, maIndex, (CFM_TYPE_MA_Name_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMAMHFCREATION:
                {
                    if(CFM_PMGR_SetDot1agCfmMaMhfCreation(mdIndex, maIndex, (CFM_TYPE_MhfCreation_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMAIDPERMISSION:
                {
                    if(CFM_PMGR_SetDot1agCfmMaMhfIdPermission(mdIndex, maIndex, (CFM_TYPE_MhfIdPermission_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMACCMINTERVAL:
                {
                    if(CFM_PMGR_SetDot1agCfmMaCcmInterval(mdIndex, maIndex, (CFM_TYPE_CcmInterval_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMANAME:
                {
                    if(CFM_PMGR_SetDot1agCfmMaName(mdIndex, maIndex, (char *)var_val, (UI32_T)var_val_len)!=CFM_TYPE_CONFIG_SUCCESS)
                        return SNMP_ERR_COMMITFAILED;

                    break;
                }
                case DOT1AGCFMMAROWSTATUS:
                {
                    if(CFM_OM_GetMaInfo(mdIndex, maIndex, &entry)==TRUE)
                    {
                        /*We only set destroy to the ip of the interface when it is exist.
                         */
                        if(value!=VAL_dot1agCfmMdRowStatus_destroy)
                        {
                                return SNMP_ERR_COMMITFAILED;
                        }
                        if(CFM_PMGR_DeleteDot1agCfmMa(mdIndex, maIndex) != CFM_TYPE_CONFIG_SUCCESS)
                            return SNMP_ERR_COMMITFAILED;
                    }
                    else
                    {
                        /*We only set createAndGo to the ip of the interface when it is exist.
                         */
                        if(value!=VAL_dot1agCfmMdRowStatus_createAndGo)
                        {
                                return SNMP_ERR_COMMITFAILED;
                        }
                        if(CFM_PMGR_SetDot1agCfmMa(mdIndex, maIndex) != CFM_TYPE_CONFIG_SUCCESS)
                            return SNMP_ERR_COMMITFAILED;
                    }
                }
                default:
                    break;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


/* === dot1agCfmMaMepListTable === */
oid dot1agCfmMaMepListTable_variables_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 6, 3};

struct variable3 dot1agCfmMaMepListTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {DOT1AGCFMMAMEPLISTROWSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMaMepListTable, 2, {1, 2}},
};

/** Initializes var_dot1agCfmMaMepListTable*/
void init_dot1agCfmMaMepListTable(void)
{
    DEBUGMSGTL(("var_dot1agCfmMaMepListTable", "Initializing\n"));
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("var_dot1agCfmMaMepListTable",
                 dot1agCfmMaMepListTable_variables, variable3,
                 dot1agCfmMaMepListTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

#define DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN  3

BOOL_T dot1agCfmMaMepListTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex, UI32_T *dot1agCfmMaIndex, UI32_T *dot1agCfmMaMepListIdentifier)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];
    *dot1agCfmMaIndex = compl[1];
    *dot1agCfmMaMepListIdentifier = compl[2];

    return TRUE;
}


unsigned char *
var_dot1agCfmMaMepListTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN] = {0};
    UI32_T md_index, ma_index, mep_id;
    CFM_OM_RemoteMepCrossCheck_T rmep_info;

    switch(vp->magic)
    {
        case DOT1AGCFMMAMEPLISTROWSTATUS:
            *write_method = write_dot1agCfmMaMepListRowStatus;
            break;
        default:
            *write_method =0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN);

    memset(&rmep_info, 0, sizeof(rmep_info));

    if (exact) /* get or set */
    {
        if (dot1agCfmMaMepListTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id) == FALSE)
        {
            return NULL;
        }

        if (CFM_OM_GetRemoteMepInfo(md_index, ma_index, mep_id, SYS_TIME_GetSystemTicksBy10ms(),&rmep_info) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        dot1agCfmMaMepListTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (CFM_OM_GetRemoteMepInfo(md_index, ma_index, mep_id, SYS_TIME_GetSystemTicksBy10ms(),&rmep_info) != TRUE)
            {
                if (CFM_OM_GetNextRemoteMepInfo(&md_index, &ma_index,  &mep_id, SYS_TIME_GetSystemTicksBy10ms(), &rmep_info)!= TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextRemoteMepInfo(&md_index, &ma_index,  &mep_id, SYS_TIME_GetSystemTicksBy10ms(), &rmep_info)!= TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    best_inst[1] = ma_index;
    best_inst[2] = mep_id;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMMAMEPLISTENTRY_INSTANCE_LEN;

    /*
   * this is where we do the value assignments for the mib results.
   */

    switch(vp->magic)
    {
        case DOT1AGCFMMAMEPLISTROWSTATUS:
            *var_len = sizeof(long_return);
            long_return = rmep_info.row_status;
            return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}


int
write_dot1agCfmMaMepListRowStatus(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMaMepListRowStatus_active) || (value > VAL_dot1agCfmMaMepListRowStatus_destroy))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
       {
            UI32_T mdIndex, maIndex, mepId;
            CFM_OM_RemoteMepCrossCheck_T entry;
            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

             if (CFM_OM_GetRemoteMepInfo(mdIndex, maIndex, mepId, SYS_TIME_GetSystemTicksBy10ms(), &entry) == TRUE)
            {
                /*We only set destroy to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_destroy)
                {
                        return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_DeleteDot1agCfmMaMepListEntry(mdIndex, maIndex, mepId) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                /*We only set createAndGo to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_createAndGo)
                {
                        return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_SetDot1agCfmMaMepListEntry(mdIndex, maIndex, mepId) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}
#endif

/*  @@==================
 *   dot1agCfmMepGroup  (7)
 *  ==================== */

#if (SNMP_CFM_MEP_GROUP_SUPPORT==TRUE)
/* === dot1agCfmMaTable === */
oid dot1agCfmMepTable_variables_oid[] = {SNMP_OID_IEEE802DOT1AG_OBJECT, 7, 1};


struct variable3 dot1agCfmMepTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {DOT1AGCFMMEPIFINDEX, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 2}},
    {DOT1AGCFMMEPDIRECTION, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 3}},
    {DOT1AGCFMMEPPRIMARYVID, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 4}},
    {DOT1AGCFMMEPACTIVE, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 5}},
    {DOT1AGCFMMEPFNGSTATE, ASN_INTEGER, RONLY, var_dot1agCfmMepTable, 2, {1, 6}},
    {DOT1AGCFMMEPCCIENABLED, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 7}},
    {DOT1AGCFMMEPCCMLTMPRIORITY, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 8}},
    {DOT1AGCFMMEPMACADDRESS, ASN_OCTET_STR, RONLY, var_dot1agCfmMepTable, 2, {1, 9}},
    {DOT1AGCFMMEPLOWPRDEF, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 10}},
    {DOT1AGCFMMEPFNGALARMTIME, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 11}},
    {DOT1AGCFMMEPFNGRESETTIME, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 12}},
    {DOT1AGCFMMEPHIGHESTPRDEFECT, ASN_INTEGER, RONLY, var_dot1agCfmMepTable, 2, {1, 13}},
    {DOT1AGCFMMEPDEFECTS, ASN_OCTET_STR, RONLY, var_dot1agCfmMepTable, 2, {1, 14}},
    {DOT1AGCFMMEPERRORCCMLASTFAILURE, ASN_OCTET_STR, RONLY, var_dot1agCfmMepTable, 2, {1, 15}},
    {DOT1AGCFMMEPXCONCCMLASTFAILURE, ASN_OCTET_STR, RONLY, var_dot1agCfmMepTable, 2, {1, 16}},
    {DOT1AGCFMMEPCCMSEQUENCEERRORS, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 17}},
    {DOT1AGCFMMEPCCISENTCCMS, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 18}},
    {DOT1AGCFMMEPNEXTLBMTRANSID, ASN_UNSIGNED, RONLY, var_dot1agCfmMepTable, 2, {1, 19}},
    {DOT1AGCFMMEPLBRIN, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 20}},
    {DOT1AGCFMMEPLBRINOUTOFORDER, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 21}},
    {DOT1AGCFMMEPLBRBADMSDU, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 22}},
    {DOT1AGCFMMEPLTMNEXTSEQNUMBER, ASN_UNSIGNED, RONLY, var_dot1agCfmMepTable, 2, {1, 23}},
    {DOT1AGCFMMEPUNEXPLTRIN, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 24}},
    {DOT1AGCFMMEPLBROUT, ASN_COUNTER, RONLY, var_dot1agCfmMepTable, 2, {1, 25}},
    {DOT1AGCFMMEPTRANSMITLBMSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 26}},
    {DOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS, ASN_OCTET_STR, RWRITE, var_dot1agCfmMepTable, 2, {1, 27}},
    {DOT1AGCFMMEPTRANSMITLBMDESTMEPID, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 28}},
    {DOT1AGCFMMEPTRANSMITLBMDESTISMEPID, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 29}},
    {DOT1AGCFMMEPTRANSMITLBMMESSAGES, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 30}},
    {DOT1AGCFMMEPTRANSMITLBMDATATLV, ASN_OCTET_STR, RWRITE, var_dot1agCfmMepTable, 2, {1, 31}},
    {DOT1AGCFMMEPTRANSMITLBMVLANPRIORITY, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 32}},
    {DOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 33}},
    {DOT1AGCFMMEPTRANSMITLBMRESULTOK, ASN_INTEGER, RONLY, var_dot1agCfmMepTable, 2, {1, 34}},
    {DOT1AGCFMMEPTRANSMITLBMSEQNUMBER, ASN_UNSIGNED, RONLY, var_dot1agCfmMepTable, 2, {1, 35}},
    {DOT1AGCFMMEPTRANSMITLTMSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 36}},
    {DOT1AGCFMMEPTRANSMITLTMFLAGS, ASN_OCTET_STR, RWRITE, var_dot1agCfmMepTable, 2, {1, 37}},
    {DOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS, ASN_OCTET_STR, RWRITE, var_dot1agCfmMepTable, 2, {1, 38}},
    {DOT1AGCFMMEPTRANSMITLTMTARGETMEPID, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 39}},
    {DOT1AGCFMMEPTRANSMITLTMTARGETISMEPID, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 40}},
    {DOT1AGCFMMEPTRANSMITLTMTTL, ASN_UNSIGNED, RWRITE, var_dot1agCfmMepTable, 2, {1, 41}},
    {DOT1AGCFMMEPTRANSMITLTMRESULT, ASN_INTEGER, RONLY, var_dot1agCfmMepTable, 2, {1, 42}},
    {DOT1AGCFMMEPTRANSMITLTMSEQNUMBER, ASN_UNSIGNED, RONLY, var_dot1agCfmMepTable, 2, {1, 43}},
    {DOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER, ASN_OCTET_STR, RWRITE, var_dot1agCfmMepTable, 2, {1, 44}},
    {DOT1AGCFMMEPROWSTATUS, ASN_INTEGER, RWRITE, var_dot1agCfmMepTable, 2, {1, 45}},
};


/** Initializes dot1agCfmMepTable*/
void init_dot1agCfmMepTable(void)
{
    DEBUGMSGTL(("dot1agCfmMepTable", "Initializing\n"));
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("dot1agCfmMepTable",
                 dot1agCfmMepTable_variables, variable3,
                 dot1agCfmMepTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

#define DOT1AGCFMMEPENTRY_INSTANCE_LEN  3

BOOL_T dot1agCfmMepTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex, UI32_T *dot1agCfmMaIndex, UI32_T *dot1agCfmMepIdentifier)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMMEPENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];
    *dot1agCfmMaIndex = compl[1];
    *dot1agCfmMepIdentifier = compl[2];

    return TRUE;
}


unsigned char *
var_dot1agCfmMepTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMMEPENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMMEPENTRY_INSTANCE_LEN] = {0};
    CFM_OM_MepInfo_T entry;
    UI32_T md_index, ma_index, mep_id;
    UI32_T dataBufLen;
    UI8_T dataBuf[1522];

    switch(vp->magic)
    {
        case DOT1AGCFMMEPIFINDEX:
            *write_method = write_dot1agCfmMepIfIndex;
            break;
        case DOT1AGCFMMEPDIRECTION:
            *write_method = write_dot1agCfmMepDirection;
            break;
        case DOT1AGCFMMEPPRIMARYVID:
            *write_method = write_dot1agCfmMepPrimaryVid;
            break;
        case DOT1AGCFMMEPACTIVE:
            *write_method = write_dot1agCfmMepActive;
            break;
        case DOT1AGCFMMEPCCIENABLED:
            *write_method = write_dot1agCfmMepCciEnabled;
            break;
        case DOT1AGCFMMEPCCMLTMPRIORITY:
             *write_method = write_dot1agCfmMepCcmLtmPriority;
            break;
        case DOT1AGCFMMEPLOWPRDEF:
            *write_method = write_dot1agCfmMepLowPrDef;
            break;
        case DOT1AGCFMMEPFNGALARMTIME:
            *write_method = write_dot1agCfmMepFngAlarmTime;
            break;
        case DOT1AGCFMMEPFNGRESETTIME:
            *write_method = write_dot1agCfmMepFngResetTime;
            break;
        case DOT1AGCFMMEPTRANSMITLBMSTATUS:
            *write_method = write_dot1agCfmMepTransmitLbmStatus;
            break;
        case DOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS:
             *write_method = write_dot1agCfmMepTransmitLbmDestMacAddress;
            break;
        case DOT1AGCFMMEPTRANSMITLBMDESTMEPID:
            *write_method = write_dot1agCfmMepTransmitLbmDestMepId;
            break;
        case DOT1AGCFMMEPTRANSMITLBMDESTISMEPID:
            *write_method = write_dot1agCfmMepTransmitLbmDestIsMepId;
            break;
        case DOT1AGCFMMEPTRANSMITLBMMESSAGES:
            *write_method = write_dot1agCfmMepTransmitLbmMessages;
            break;
        case DOT1AGCFMMEPTRANSMITLBMDATATLV:
            *write_method = write_dot1agCfmMepTransmitLbmDataTlv;
            break;
        case DOT1AGCFMMEPTRANSMITLBMVLANPRIORITY:
            *write_method = write_dot1agCfmMepTransmitLbmVlanPriority;
            break;
        case DOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE:
            *write_method = write_dot1agCfmMepTransmitLbmVlanDropEnable;
            break;
        /* dot1agCfmMepTransmitLbmStatus在MIB是Read-Only, Macauley說要改成Read-Write. Daniel, 2007/4/27 */
        case DOT1AGCFMMEPTRANSMITLTMSTATUS:
            *write_method = write_dot1agCfmMepTransmitLtmStatus;
            break;
        case DOT1AGCFMMEPTRANSMITLTMFLAGS:
            *write_method = write_dot1agCfmMepTransmitLtmFlags;
            break;
        case DOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS:
            *write_method = write_dot1agCfmMepTransmitLtmTargetMacAddress;
            break;
        case DOT1AGCFMMEPTRANSMITLTMTARGETMEPID:
            *write_method = write_dot1agCfmMepTransmitLtmTargetMepId;
            break;
        case DOT1AGCFMMEPTRANSMITLTMTARGETISMEPID:
            *write_method = write_dot1agCfmMepTransmitLtmTargetIsMepId;
            break;
        case DOT1AGCFMMEPTRANSMITLTMTTL:
            *write_method = write_dot1agCfmMepTransmitLtmTtl;
            break;
        case DOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER:
            *write_method = write_dot1agCfmMepTransmitLtmEgressIdentifier;
            break;
        case DOT1AGCFMMEPROWSTATUS:
            *write_method = write_dot1agCfmMepRowStatus;
            break;
        default:
            *write_method =0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMMEPENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {

        if (dot1agCfmMepTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id) == FALSE)
        {
            return NULL;
        }

        if (CFM_OM_GetMepInfo(md_index, ma_index, mep_id, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else /* getnext */
    {
        dot1agCfmMepTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
             if (CFM_OM_GetMepInfo(md_index, ma_index, mep_id, &entry) != TRUE)
            {
                if (CFM_OM_GetDot1agNextMepInfo(&md_index, &ma_index, &mep_id, 0, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetDot1agNextMepInfo(&md_index, &ma_index, &mep_id, 0, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    best_inst[1] = ma_index;
    best_inst[2] = mep_id;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMMEPENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMMEPENTRY_INSTANCE_LEN;

    /*
   * this is where we do the value assignments for the mib results.
   */
    *var_len = sizeof(long_return);
    switch(vp->magic)
    {
        case DOT1AGCFMMEPIFINDEX:
            long_return = entry.lport;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPDIRECTION:
            long_return = entry.direction;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPPRIMARYVID:
            long_return = entry.primary_vid;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPACTIVE:
            long_return = entry.active;
            if (long_return==0)
                long_return = 2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPFNGSTATE:
            long_return = entry.fng_state;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPCCIENABLED:
            long_return = entry.ccm_status;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPCCMLTMPRIORITY:
            long_return = entry.ccm_ltm_priority;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPMACADDRESS:
            memcpy(return_buf, entry.mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);
            *var_len = SYS_ADPT_MAC_ADDR_LEN;
            return return_buf;
        case DOT1AGCFMMEPLOWPRDEF:
            long_return = entry.low_pri_def;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPFNGALARMTIME:
            long_return = entry.fng_alarm_time*100 ;  /* second -> centi-second */
            return (u_char*) &long_return;
        case DOT1AGCFMMEPFNGRESETTIME:
            long_return = entry.fng_reset_time*100;  /* second -> centi-second */
            return (u_char*) &long_return;
        case DOT1AGCFMMEPHIGHESTPRDEFECT:
            long_return = entry.highest_pri_defect;
            return (u_char*) &long_return;

        case DOT1AGCFMMEPDEFECTS:
        {
            return_buf[0] =entry.defects;
            *var_len = 1;
            return return_buf;
        }
        case DOT1AGCFMMEPERRORCCMLASTFAILURE:
            memset(dataBuf, 0, sizeof(dataBuf));
            if (CFM_OM_GetMepErrorCCMLastFailure(md_index, ma_index, mep_id, dataBuf, &dataBufLen)!=TRUE)
                return NULL;
            *var_len = dataBufLen;
            memcpy(return_buf, dataBuf, dataBufLen);
            return (u_char*) return_buf;
        case DOT1AGCFMMEPXCONCCMLASTFAILURE:
            memset(dataBuf, 0, sizeof(dataBuf));
            if (CFM_OM_GetMepXconCcmLastFailure(md_index, ma_index, mep_id, dataBuf, &dataBufLen)!=TRUE)
                return NULL;
            *var_len = dataBufLen;
            memcpy(return_buf, dataBuf, dataBufLen);
            return (u_char*) return_buf;
        case DOT1AGCFMMEPCCMSEQUENCEERRORS:
            long_return = entry.ccm_seq_error;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPCCISENTCCMS:
            long_return = entry.cci_sent_ccms;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPNEXTLBMTRANSID:
            long_return = entry.next_lbm_trans_id;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPLBRIN:
            long_return = entry.lbr_in;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPLBRINOUTOFORDER:
            long_return = entry.lbr_in_out_of_order;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPLBRBADMSDU:
            long_return = entry.lbr_bad_msdu;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPLTMNEXTSEQNUMBER:
            long_return = entry.ltm_next_seq_num;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPUNEXPLTRIN:
            long_return = entry.unexp_ltr_in;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPLBROUT:
            long_return = entry.lbr_out;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMSTATUS:
            long_return = entry.trans_lbm_status;
            if (long_return==0)
                long_return=2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS:
            memcpy(return_buf, entry.trans_lbm_dst_mac_addr_a, SIZE_dot1agCfmMepTransmitLbmDestMacAddress);
            *var_len = SIZE_dot1agCfmMepTransmitLbmDestMacAddress;
            return return_buf;
        case DOT1AGCFMMEPTRANSMITLBMDESTMEPID:
            long_return = entry.trans_lbm_dst_mep_id;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMDESTISMEPID:
            long_return = entry.trans_lbm_dst_is_mep_id;
            if (long_return == 0)
                long_return = 2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMMESSAGES:
            long_return = entry.trans_lbm_msg;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMDATATLV:
            memset(dataBuf, 0, sizeof(dataBuf));
            dataBufLen=0;
            if (CFM_OM_GetMepTransmitLbmDataTlv(md_index, ma_index, mep_id, dataBuf, &dataBufLen)!=TRUE)
                return NULL;
            *var_len = dataBufLen;
            memcpy(return_buf, dataBuf, dataBufLen);
            return (u_char*) return_buf;
        case DOT1AGCFMMEPTRANSMITLBMVLANPRIORITY:
            long_return = entry.trans_lbm_vlan_priority;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE:
            long_return = entry.trans_lbm_vlan_drop_enabled;
            if (long_return == 0)
                long_return = 2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMRESULTOK:
            long_return = entry.trans_lbm_result_ok;
            if (long_return == 0)
                long_return = 2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLBMSEQNUMBER:
            long_return = entry.trans_lbm_seq_num;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMSTATUS:
            long_return = entry.trans_ltm_status;
            if (long_return == 0)
                long_return = 2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMFLAGS:
            if (entry.ltm_use_fdb_only)
                return_buf[0] = 0x1<<7;
            else
                return_buf[0] = 0;
            *var_len = 1;
            return return_buf;
        case DOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS:
            memcpy(return_buf, entry.trans_ltm_target_mac_addr_a, SIZE_dot1agCfmMepTransmitLtmTargetMacAddress);
            *var_len = SIZE_dot1agCfmMepTransmitLtmTargetMacAddress;
            return (u_char*) return_buf;
        case DOT1AGCFMMEPTRANSMITLTMTARGETMEPID:
            long_return = entry.trans_ltm_target_mep_id;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMTARGETISMEPID:
            long_return = entry.trans_ltm_target_is_mep_id;
            if (long_return==0)
                long_return=2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMTTL:
            long_return = entry.trans_ltm_ttl;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMRESULT:
            long_return = entry.trans_ltm_result;
            if (long_return==0)
                long_return=2;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMSEQNUMBER:
            long_return = entry.trans_ltm_seq_num;
            return (u_char*) &long_return;
        case DOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER:
            memcpy(return_buf, entry.trans_ltm_egress_id_a, SIZE_dot1agCfmMepTransmitLtmEgressIdentifier);
            *var_len = SIZE_dot1agCfmMepTransmitLtmEgressIdentifier;
            return return_buf;
        case DOT1AGCFMMEPROWSTATUS:
            long_return = entry.row_status;
            return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

int
write_dot1agCfmMepIfIndex(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER) ||
               (value > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepIfIndex(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepDirection(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMepDirection_down) || (value > VAL_dot1agCfmMepDirection_up))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepDirection(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepPrimaryVid(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(u_long *)var_val;

            if((value < MIN_dot1agCfmMepPrimaryVid) || (value > MAX_dot1agCfmMepPrimaryVid))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepPrimaryVid(mdIndex, maIndex, mepId, (UI16_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepActive(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMepActive_true) || (value > VAL_dot1agCfmMepActive_false))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if (value == 2)
                value=0;
            if(CFM_PMGR_SetDot1agCfmMepActive(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

// -------
int
write_dot1agCfmMepCciEnabled(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMepCciEnabled_true) || (value > VAL_dot1agCfmMepCciEnabled_false))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepCciEnable(mdIndex, maIndex, mepId, (CFM_TYPE_CcmStatus_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepCcmLtmPriority(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(u_long *)var_val;

            if((value < MIN_dot1agCfmMepCcmLtmPriority) || (value > MAX_dot1agCfmMepCcmLtmPriority))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(u_long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepCcmLtmPriority(mdIndex, maIndex, mepId, (UI32_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepLowPrDef(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMepLowPrDef_allDef) || (value > VAL_dot1agCfmMepLowPrDef_noXcon))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepLowPrDef(mdIndex, maIndex, mepId, (CFM_TYPE_FNG_LowestAlarmPri_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepFngAlarmTime(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < MIN_dot1agCfmMepFngAlarmTime) || (value > MAX_dot1agCfmMepFngAlarmTime))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if (value<300)
                value=300;
            if(CFM_PMGR_SetDot1agCfmMepFngAlarmTime(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepFngResetTime(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;


            if((value < MIN_dot1agCfmMepFngResetTime) || (value > MAX_dot1agCfmMepFngResetTime))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            /* CFM core only support 3-10 second */
            if (value<300)
                value=300;
            if(CFM_PMGR_SetDot1agCfmMepFngResetTime(mdIndex, maIndex, mepId, (UI32_T)value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLbmStatus(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < 1) || (value > 2))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            /* only process the True value */
            if(value == 1 && CFM_PMGR_SetDot1agCfmMepTransmitLbmStatus(mdIndex, maIndex, mepId)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmDestMacAddress(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != SIZE_dot1agCfmMepTransmitLbmDestMacAddress)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        }

        case RESERVE2:
        {
            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMacAddress(mdIndex, maIndex, mepId, var_val)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmDestMepId(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(u_long *)var_val;

            if((value < MIN_dot1agCfmMepTransmitLbmDestMepId) || (value > MAX_dot1agCfmMepTransmitLbmDestMepId))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex, maIndex, mepId;

            value = *(u_long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMepId(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmDestIsMepId(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < 1) || (value > 2))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if (value==2)
                value=0;
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmDestIsMepId(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmMessages(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < MIN_dot1agCfmMepTransmitLbmMessages) || (value > MAX_dot1agCfmMepTransmitLbmMessages))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmMessages(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmDataTlv(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len > MAXSIZE_dot1agCfmMepTransmitLbmDataTlv)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmDataTlv(mdIndex, maIndex, mepId, (char *)var_val, var_val_len)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLbmVlanPriority(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < MIN_dot1agCfmMepTransmitLbmVlanPriority) || (value > MAX_dot1agCfmMepTransmitLbmVlanPriority))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanPriority(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLbmVlanDropEnable(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < 1) || (value > 2))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if (value == 2)
                value = 1;
            if(CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanDropEnable(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmStatus(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < 1) || (value > 2))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            /* only process the True value */
            if(value == 1 && CFM_PMGR_SetDot1agCfmMepTransmitLtmStatus(mdIndex, maIndex, mepId)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmFlags(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI8_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != 1)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            memcpy(&value, var_val, 1);

            if((value != VAL_dot1agCfmMepTransmitLtmFlags_useFDBonly) && (value != 128))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;
            BOOL_T useFDBonly;

            memcpy(&value, var_val, 1);
            if (value)
                useFDBonly = 1;
            else
                useFDBonly = 0;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmFlags(mdIndex, maIndex, mepId, useFDBonly)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmTargetMacAddress(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail */
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if(var_val_len != SIZE_dot1agCfmMepTransmitLtmTargetMacAddress)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        }

        case RESERVE2:
        {
            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            UI32_T mdIndex, maIndex, mepId;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMacAddress(mdIndex, maIndex, mepId, (UI8_T *) var_val)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmTargetMepId(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(u_long *)var_val;

            if((value < MIN_dot1agCfmMepTransmitLtmTargetMepId) || (value > MAX_dot1agCfmMepTransmitLtmTargetMepId))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

           value = *(u_long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMepId(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}

int
write_dot1agCfmMepTransmitLtmTargetIsMepId(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    I32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < 1) || (value > 2))  // TrueValue Failue
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            I32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];
            if (value==2)
                value=0;
            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetIsMepId(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmTtl(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T value;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(u_long *)var_val;

            if((value < MIN_dot1agCfmMepTransmitLtmTtl) || (value > MAX_dot1agCfmMepTransmitLtmTtl))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex, maIndex, mepId;

            value = *(u_long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmTtl(mdIndex, maIndex, mepId, value)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepTransmitLtmEgressIdentifier(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != SIZE_dot1agCfmMepTransmitLtmEgressIdentifier)
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            UI32_T mdIndex, maIndex, mepId;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

            if(CFM_PMGR_SetDot1agCfmMepTransmitLtmEgressIdentifier(mdIndex, maIndex, mepId, (UI8_T *) var_val)!=CFM_TYPE_CONFIG_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}


int
write_dot1agCfmMepRowStatus(int action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;
    CFM_OM_MepInfo_T entry;
    UI32_T oid_name_length = SNMP_OIDLEN_IEEE802DOT1AG_OBJECT+4 ;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if(name_len != oid_name_length+3)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch(action)
    {
        case RESERVE1:
        {
            if(var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if(var_val_len != sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *)var_val;

            if((value < VAL_dot1agCfmMepRowStatus_active) || (value > VAL_dot1agCfmMepRowStatus_destroy))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
        {
            UI32_T mdIndex, maIndex, mepId;

            value = *(long *)var_val;
            mdIndex = name[oid_name_length];
            maIndex = name[oid_name_length+1];
            mepId = name[oid_name_length+2];

             if (CFM_OM_GetMepInfo(mdIndex, maIndex, mepId, &entry) == TRUE)
            {
                /*We only set destroy to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_destroy)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_DeleteDot1agCfmMepEntry(mdIndex, maIndex, mepId) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                /*We only set createAndGo to the ip of the interface when it is exist.
                 */
                if(value!=VAL_dot1agCfmMdRowStatus_createAndGo)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(CFM_PMGR_SetDot1agCfmMepEntry(mdIndex, maIndex, mepId) != CFM_TYPE_CONFIG_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            break;
        }

        case UNDO:
             /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
            break;
    }

    return SNMP_ERR_NOERROR;
}



/* === dot1agCfmLtrTable === */
/*
 * dot1agCfmLtrTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid dot1agCfmLtrTable_variables_oid[] = { SNMP_OID_IEEE802DOT1AG_OBJECT, 7, 2};

/*
 * variable3 dot1agCfmLtrTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 dot1agCfmLtrTable_variables[] =
{
    /*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
    {DOT1AGCFMLTRTTL,  ASN_UNSIGNED,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 3 }},
    {DOT1AGCFMLTRFORWARDED,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 4 }},
    {DOT1AGCFMLTRTERMINALMEP,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 5 }},
    {DOT1AGCFMLTRLASTEGRESSIDENTIFIER,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 6 }},
    {DOT1AGCFMLTRNEXTEGRESSIDENTIFIER,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 7 }},
    {DOT1AGCFMLTRRELAY,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 8 }},
    {DOT1AGCFMLTRCHASSISIDSUBTYPE,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 9 }},
    {DOT1AGCFMLTRCHASSISID,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 10 }},
    {DOT1AGCFMLTRMANADDRESSDOMAIN,  ASN_OBJECT_ID,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 11 }},
    {DOT1AGCFMLTRMANADDRESS,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 12 }},
    {DOT1AGCFMLTRINGRESS,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 13 }},
    {DOT1AGCFMLTRINGRESSMAC,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 14 }},
    {DOT1AGCFMLTRINGRESSPORTIDSUBTYPE,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 15 }},
    {DOT1AGCFMLTRINGRESSPORTID,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 16 }},
    {DOT1AGCFMLTREGRESS,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 17 }},
    {DOT1AGCFMLTREGRESSMAC,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 18 }},
    {DOT1AGCFMLTREGRESSPORTIDSUBTYPE,  ASN_INTEGER,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 19 }},
    {DOT1AGCFMLTREGRESSPORTID,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 20 }},
    {DOT1AGCFMLTRORGANIZATIONSPECIFICTLV,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmLtrTable, 2,  { 1, 21 }},
};  /*    (L = length of the oidsuffix) */

/** Initializes the dot1agCfmLtrTable module */
void init_dot1agCfmLtrTable(void)
{

    DEBUGMSGTL(("dot1agCfmLtrTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("dot1agCfmLtrTable", dot1agCfmLtrTable_variables, variable3,
               dot1agCfmLtrTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define DOT1AGCFMLTRENTRY_INSTANCE_LEN  5

BOOL_T dot1agCfmLtrTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex, UI32_T *dot1agCfmMaIndex, UI32_T *dot1agCfmMepIdentifier, UI32_T *dot1agCfmLtrSeqNumber, UI32_T *dot1agCfmLtrReceiveOrder)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMLTRENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];
    *dot1agCfmMaIndex = compl[1];
    *dot1agCfmMepIdentifier = compl[2];
    *dot1agCfmLtrSeqNumber = compl[3];
    *dot1agCfmLtrReceiveOrder = compl[4];

    return TRUE;
}

unsigned char *
var_dot1agCfmLtrTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T  md_index, ma_index, mep_id, seq, rcv_order;
    CFM_OM_LinktraceReply_T entry;
    UI32_T  compc = 0;
    oid     compl[DOT1AGCFMLTRENTRY_INSTANCE_LEN] = {0};
    oid     best_inst[DOT1AGCFMLTRENTRY_INSTANCE_LEN] = {0};
    UI8_T   org_specific_tlv_buf[CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH];

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMLTRENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));
    entry.org_specific_tlv_p = org_specific_tlv_buf;

    if (exact) /* get or set */
    {
        if (dot1agCfmLtrTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id, &seq, &rcv_order) == FALSE)
        {
            return NULL;
        }

        if (CFM_OM_GetLinktraceReplyInfo(md_index, ma_index, mep_id, seq, rcv_order, &entry) != TRUE)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        dot1agCfmLtrTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id, &seq, &rcv_order);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (CFM_OM_GetLinktraceReplyInfo(md_index, ma_index, mep_id, seq, rcv_order, &entry) != TRUE)
            {
                if (CFM_OM_GetNextLinktraceReplyInfo(&md_index, &ma_index, &mep_id, &seq, &rcv_order, &entry) != TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (CFM_OM_GetNextLinktraceReplyInfo(&md_index, &ma_index, &mep_id, &seq, &rcv_order, &entry) != TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    best_inst[1] = ma_index;
    best_inst[2] = mep_id;
    best_inst[3] = seq;
    best_inst[4] = rcv_order;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMLTRENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMLTRENTRY_INSTANCE_LEN;
    *var_len = sizeof(long_return);
    /*
   * this is where we do the value assignments for the mib results.
   */

    switch(vp->magic)
    {
        case DOT1AGCFMLTRTTL:
            long_return = entry.reply_ttl;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRFORWARDED:
            long_return = entry.forwarded;
            if (long_return==0)
                long_return=2;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRTERMINALMEP:
            long_return = entry.terminal_mep;
            if (long_return==0)
                long_return=2;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRLASTEGRESSIDENTIFIER:
            memcpy(return_buf, entry.last_egress_id_a, SIZE_dot1agCfmLtrLastEgressIdentifier);
            *var_len = SIZE_dot1agCfmLtrLastEgressIdentifier;
            return (u_char*)return_buf;
        case DOT1AGCFMLTRNEXTEGRESSIDENTIFIER:
            memcpy(return_buf, entry.next_egress_id_a, SIZE_dot1agCfmLtrLastEgressIdentifier);
            *var_len = SIZE_dot1agCfmLtrLastEgressIdentifier;
            return (u_char*)return_buf;
        case DOT1AGCFMLTRRELAY:
            long_return = entry.relay_action;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRCHASSISIDSUBTYPE:
            long_return = entry.chassis_id_subtype;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRCHASSISID:
            memcpy(return_buf, entry.chassis_id_a, entry.chassis_id_len);
            *var_len = entry.chassis_id_len;
            return return_buf;
        case DOT1AGCFMLTRMANADDRESSDOMAIN:
            memcpy(return_buf, entry.mgmt_domain_a, entry.mgmt_domain_len);
           *var_len = (size_t) entry.mgmt_domain_len;
            return (u_char*) return_buf;
        case DOT1AGCFMLTRMANADDRESS:
            memcpy(return_buf, entry.mgmt_addr_a, entry.mgmt_addr_len);
            *var_len = entry.mgmt_addr_len;
            return (u_char*) return_buf;
        case DOT1AGCFMLTRINGRESS:
            long_return = entry.ingress_action;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRINGRESSMAC:
            memcpy(return_buf, entry.ingress_port_mac_a, SIZE_dot1agCfmLtrIngressMac);
            *var_len = SIZE_dot1agCfmLtrIngressMac;
            return (u_char*) return_buf;
        case DOT1AGCFMLTRINGRESSPORTIDSUBTYPE:
            long_return = entry.ingress_port_id_subtype;
            return (u_char*) &long_return;
        case DOT1AGCFMLTRINGRESSPORTID:
            memcpy(return_buf, entry.ingress_port_id_a, entry.ingress_port_id_len);
            *var_len = entry.ingress_port_id_len;
            return (u_char*) return_buf;
        case DOT1AGCFMLTREGRESS:
            long_return = entry.egress_action;
            return (u_char*) &long_return;
        case DOT1AGCFMLTREGRESSMAC:
            memcpy(return_buf, entry.egress_port_mac_a, SIZE_dot1agCfmLtrEgressMac);
            *var_len = SIZE_dot1agCfmLtrEgressMac;
            return (u_char*) return_buf;
        case DOT1AGCFMLTREGRESSPORTIDSUBTYPE:
            long_return = entry.egress_port_id_subtype;
            return (u_char*) &long_return;
        case DOT1AGCFMLTREGRESSPORTID:
            memcpy(return_buf, entry.egress_port_id_a, entry.egress_port_id_len);
            *var_len = (size_t) entry.egress_port_id_len;
            return (u_char*) return_buf;
        case DOT1AGCFMLTRORGANIZATIONSPECIFICTLV:
            memcpy(return_buf, entry.org_specific_tlv_p, entry.org_specific_tlv_len);
            *var_len = entry.org_specific_tlv_len;
            return (u_char*) return_buf;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

/* === dot1agCfmMepDbTable === */
oid dot1agCfmMepDbTable_variables_oid[] = { SNMP_OID_IEEE802DOT1AG_OBJECT, 7, 3 };
/*
 * variable3 dot1agCfmMepDbTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 dot1agCfmMepDbTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{DOT1AGCFMMEPDBRMEPSTATE,  ASN_INTEGER,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 2 }},
{DOT1AGCFMMEPDBRMEPFAILEDOKTIME,  ASN_TIMETICKS,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 3 }},
{DOT1AGCFMMEPDBMACADDRESS,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 4 }},
{DOT1AGCFMMEPDBRDI,  ASN_INTEGER,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 5 }},
{DOT1AGCFMMEPDBPORTSTATUSTLV,  ASN_INTEGER,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 6 }},
{DOT1AGCFMMEPDBINTERFACESTATUSTLV,  ASN_INTEGER,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 7 }},
{DOT1AGCFMMEPDBCHASSISIDSUBTYPE,  ASN_INTEGER,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 8 }},
{DOT1AGCFMMEPDBCHASSISID,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 9 }},
{DOT1AGCFMMEPDBMANADDRESSDOMAIN,  ASN_OBJECT_ID,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 10 }},
{DOT1AGCFMMEPDBMANADDRESS,  ASN_OCTET_STR,  RONLY,   var_dot1agCfmMepDbTable, 2,  { 1, 11 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the dot1agCfmMepDbTable module */
void
init_dot1agCfmMepDbTable(void)
{

    DEBUGMSGTL(("dot1agCfmMepDbTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("dot1agCfmMepDbTable", dot1agCfmMepDbTable_variables, variable3,
               dot1agCfmMepDbTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define DOT1AGCFMMEPDBENTRY_INSTANCE_LEN  4

BOOL_T dot1agCfmMepDbTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *dot1agCfmMdIndex, UI32_T *dot1agCfmMaIndex, UI32_T *dot1agCfmMepIdentifier, UI32_T *dot1agCfmMepDbRMepIdentifier)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != DOT1AGCFMMEPDBENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *dot1agCfmMdIndex = compl[0];
    *dot1agCfmMaIndex = compl[1];
    *dot1agCfmMepIdentifier = compl[2];
    *dot1agCfmMepDbRMepIdentifier = compl[3];

    return TRUE;
}


unsigned char *
var_dot1agCfmMepDbTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[DOT1AGCFMMEPDBENTRY_INSTANCE_LEN] = {0};
    oid best_inst[DOT1AGCFMMEPDBENTRY_INSTANCE_LEN] = {0};
    UI32_T md_index=0, ma_index=0, mep_id=0, rmep_id=0;
    CFM_OM_RemoteMepCrossCheck_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, DOT1AGCFMMEPDBENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (dot1agCfmMepDbTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id, &rmep_id) == FALSE)
        {
            return NULL;
        }

        if (FALSE == CFM_OM_GetMepDbTable(md_index, ma_index, mep_id, rmep_id, SYS_TIME_GetSystemTicksBy10ms(), &entry))
        {
            return NULL;
        }
    }
    else /* getnext */
    {
       dot1agCfmMepDbTable_OidIndexToData(exact, compc, compl, &md_index, &ma_index, &mep_id, &rmep_id) ;

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (FALSE == CFM_OM_GetMepDbTable(md_index, ma_index, mep_id, rmep_id, SYS_TIME_GetSystemTicksBy10ms(), &entry))
            {
                if (CFM_OM_GetNextMepDbTable(&md_index, &ma_index, &mep_id, &rmep_id, SYS_TIME_GetSystemTicksBy10ms(), &entry)!= TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {
                if (CFM_OM_GetNextMepDbTable(&md_index, &ma_index, &mep_id, &rmep_id, SYS_TIME_GetSystemTicksBy10ms(), &entry)!= TRUE)
                {
                    return NULL;
                }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = md_index;
    best_inst[1] = ma_index;
    best_inst[2] = mep_id;
    best_inst[3] = rmep_id;
    memcpy(name + vp->namelen, best_inst, DOT1AGCFMMEPDBENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + DOT1AGCFMMEPDBENTRY_INSTANCE_LEN;
    *var_len = sizeof(long_return);

    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_dot1agCfmMepDbRMepIdentifier:
            *var_len = sizeof(long_return);
            long_return =rmep_id;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_dot1agCfmMepDbRMepState:
            *var_len = sizeof(long_return);
            long_return = entry.row_status;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbRMepFailedOkTime:
            *var_len = sizeof(long_return);
            long_return = entry.failed_ok_time;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbMacAddress:
            *var_len = SIZE_dot1agCfmMepDbMacAddress;
            memcpy(return_buf, entry.mep_mac_a, *var_len);
            return (u_char *) return_buf;

        case LEAF_dot1agCfmMepDbRdi:
            *var_len = sizeof(long_return);
            long_return = entry.rdi;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbPortStatusTlv:
            *var_len = sizeof(long_return);
            long_return = entry.port_status;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbInterfaceStatusTlv:
            *var_len = sizeof(long_return);
            long_return = entry.interface_status;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbChassisIdSubtype:
            *var_len = sizeof(long_return);
            long_return = entry.chassis_id_subtype;
            return (u_char *) &long_return;

        case LEAF_dot1agCfmMepDbChassisId:
            *var_len = entry.chassis_id_len;
            memcpy(return_buf, entry.chassis_id_a, entry.chassis_id_len);
            return (u_char*)return_buf;

        case LEAF_dot1agCfmMepDbManAddressDomain:
            *var_len = entry.mgmt_addr_domain_len;
            memcpy(return_buf, entry.mgmt_addr_domain_a,  entry.mgmt_addr_domain_len);
            return (u_char *) return_buf;

        case LEAF_dot1agCfmMepDbManAddress:
            *var_len = entry.mgmt_len;
            memcpy(return_buf, entry.mgmt_addr_a,  entry.mgmt_len);
            return (u_char *) return_buf;

        default:
            ERROR_MSG("");
    }

    return NULL;
}
#endif
#endif /* #if(SYS_CPNT_CFM==TRUE)  */

