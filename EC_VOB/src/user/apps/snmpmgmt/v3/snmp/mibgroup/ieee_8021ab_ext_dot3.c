#include "sys_cpnt.h"
#if (SYS_CPNT_LLDP_EXT == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "ieee_8021ab_ext_dot3.h"
#include "leaf_ieeelldp.h"
#include "leaf_ieee8021ab_ext_dot3.h"
#include "lldp_type.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "snmp_mgr.h"

/*
 * lldpXdot3PortConfigTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3PortConfigTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,1 };

struct variable3 lldpXdot3PortConfigTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3PortConfigTLVsTxEnable,  ASN_OCTET_STR,  RWRITE,  var_lldpXdot3PortConfigTable, 3,  { 1, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3PortConfigTable module */
void
init_lldpXdot3PortConfigTable(void)
{

    DEBUGMSGTL(("lldpXdot3PortConfigTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3PortConfigTable", lldpXdot3PortConfigTable_variables, variable3,
               lldpXdot3PortConfigTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot3PortConfigTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpPortConfigPortNum)
{
    /* get or write
     */
    if(exact)
    {
        /* check the index length
         */
        if(compc != LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpPortConfigPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot3PortConfigTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3PortConfigTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3PortConfigEntry_T  entry;
    switch(vp->magic)
    {
    case LEAF_lldpXdot3PortConfigTLVsTxEnable:
        *write_method = write_lldpXdot3PortConfigTLVsTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3PortConfigTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot3PortConfigEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3PortConfigTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot3PortConfigEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3PortConfigEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3PortConfigEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3PORTCONFIGENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic)
    {
    case LEAF_lldpXdot3PortConfigTLVsTxEnable:
        *var_len = SIZE_lldpXdot3PortConfigTLVsTxEnable;
        SNMP_MGR_BitsFromCoreToSnmp(&entry.tlvs_tx_enable, return_buf, *var_len);
        return (u_char*)return_buf;

    default:
        ERROR_MSG("");
    }

    return NULL;
}

int
write_lldpXdot3PortConfigTLVsTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != SIZE_lldpXdot3PortConfigTLVsTxEnable)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*var_val & 0x0f)
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
            UI32_T oid_name_length = 14;
            UI8_T byte_buffer;
            UI32_T lldpPortConfigPortNum = 0;

            if(lldpXdot3PortConfigTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &lldpPortConfigPortNum)==FALSE)
                return SNMP_ERR_COMMITFAILED;

            SNMP_MGR_BitsFromSnmpToCore(&byte_buffer, var_val, var_val_len);
            if(LLDP_PMGR_SetXdot3PortConfigEntry( lldpPortConfigPortNum, byte_buffer) != TRUE)
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

/*
 * lldpXdot3LocPortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3LocPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,2 };

struct variable3 lldpXdot3LocPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3LocPortAutoNegSupported,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPortTable, 3,  { 1, 1, 1 }},
{LEAF_lldpXdot3LocPortAutoNegEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPortTable, 3,  { 1, 1, 2 }},
{LEAF_lldpXdot3LocPortAutoNegAdvertisedCap,  ASN_OCTET_STR,  RONLY,   var_lldpXdot3LocPortTable, 3,  { 1, 1, 3 }},
{LEAF_lldpXdot3LocPortOperMauType,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPortTable, 3,  { 1, 1, 4 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3LocPortTable module */
void
init_lldpXdot3LocPortTable(void)
{

    DEBUGMSGTL(("lldpXdot3LocPortTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3LocPortTable", lldpXdot3LocPortTable_variables, variable3,
               lldpXdot3LocPortTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot3LocPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot3LocPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3LocPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3LocPortEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3LocPortTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot3LocPortEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3LocPortTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot3LocPortEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3LocPortEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3LocPortEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3LOCPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3LocPortAutoNegSupported:
        *var_len = sizeof(long_return);
        long_return = entry.loc_port_auto_neg_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPortAutoNegEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.loc_port_auto_neg_enabled;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPortAutoNegAdvertisedCap:
        *var_len = SIZE_lldpXdot3LocPortAutoNegAdvertisedCap;
        memcpy(return_buf, &entry.loc_port_auto_neg_adv_cap,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpXdot3LocPortOperMauType:
        *var_len = sizeof(long_return);
        long_return = entry.loc_port_oper_mau_type;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3LocPowerTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3LocPowerTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,2 };

struct variable3 lldpXdot3LocPowerTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3LocPowerPortClass,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 1 }},
{LEAF_lldpXdot3LocPowerMDISupported,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXdot3LocPowerMDIEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 3 }},
{LEAF_lldpXdot3LocPowerPairControlable,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 4 }},
{LEAF_lldpXdot3LocPowerPairs,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 5 }},
{LEAF_lldpXdot3LocPowerClass,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocPowerTable, 3,  { 2, 1, 6 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3LocPowerTable module */
void
init_lldpXdot3LocPowerTable(void)
{

    DEBUGMSGTL(("lldpXdot3LocPowerTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3LocPowerTable", lldpXdot3LocPowerTable_variables, variable3,
               lldpXdot3LocPowerTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot3LocPowerTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot3LocPowerTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3LocPowerTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3LocPowerEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3LocPowerTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot3LocPowerEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3LocPowerTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot3LocPowerEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3LocPowerEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3LocPowerEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3LOCPOWERENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3LocPowerPortClass:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_port_class;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPowerMDISupported:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_mdi_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPowerMDIEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_mdi_enabled;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPowerPairControlable:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_pair_controlable;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPowerPairs:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_pairs;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3LocPowerClass:
        *var_len = sizeof(long_return);
        long_return = entry.loc_power_class;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3LocLinkAggTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3LocLinkAggTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,2 };

struct variable3 lldpXdot3LocLinkAggTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3LocLinkAggStatus,  ASN_OCTET_STR,  RONLY,   var_lldpXdot3LocLinkAggTable, 3,  { 3, 1, 1 }},
{LEAF_lldpXdot3LocLinkAggPortId,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocLinkAggTable, 3,  { 3, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3LocLinkAggTable module */
void
init_lldpXdot3LocLinkAggTable(void)
{

    DEBUGMSGTL(("lldpXdot3LocLinkAggTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3LocLinkAggTable", lldpXdot3LocLinkAggTable_variables, variable3,
               lldpXdot3LocLinkAggTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot3LocLinkAggTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot3LocLinkAggTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3LocLinkAggTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3LocLinkAggEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3LocLinkAggTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot3LocLinkAggEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3LocLinkAggTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot3LocLinkAggEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3LocLinkAggEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3LocLinkAggEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3LOCLINKAGGENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3LocLinkAggStatus:
        *var_len = SIZE_lldpXdot3LocLinkAggStatus;
        memcpy(return_buf, &entry.loc_link_agg_status,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpXdot3LocLinkAggPortId:
        *var_len = sizeof(long_return);
        long_return = entry.loc_link_agg_port_id;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3LocMaxFrameSizeTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3LocMaxFrameSizeTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,2 };

struct variable3 lldpXdot3LocMaxFrameSizeTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3LocMaxFrameSize,  ASN_INTEGER,  RONLY,   var_lldpXdot3LocMaxFrameSizeTable, 3,  { 4, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3LocMaxFrameSizeTable module */
void
init_lldpXdot3LocMaxFrameSizeTable(void)
{

    DEBUGMSGTL(("lldpXdot3LocMaxFrameSizeTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3LocMaxFrameSizeTable", lldpXdot3LocMaxFrameSizeTable_variables, variable3,
               lldpXdot3LocMaxFrameSizeTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot3LocMaxFrameSizeTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot3LocMaxFrameSizeTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3LocMaxFrameSizeTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3LocMaxFrameSizeTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot3LocMaxFrameSizeEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3LocMaxFrameSizeTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot3LocMaxFrameSizeEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3LocMaxFrameSizeEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3LocMaxFrameSizeEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3LOCMAXFRAMESIZEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3LocMaxFrameSize:
        *var_len = sizeof(long_return);
        long_return = entry.loc_max_frame_size;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3RemPortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3RemPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,3 };

struct variable3 lldpXdot3RemPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3RemPortAutoNegSupported,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPortTable, 3,  { 1, 1, 1 }},
{LEAF_lldpXdot3RemPortAutoNegEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPortTable, 3,  { 1, 1, 2 }},
{LEAF_lldpXdot3RemPortAutoNegAdvertisedCap,  ASN_OCTET_STR,  RONLY,   var_lldpXdot3RemPortTable, 3,  { 1, 1, 3 }},
{LEAF_lldpXdot3RemPortOperMauType,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPortTable, 3,  { 1, 1, 4 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3RemPortTable module */
void
init_lldpXdot3RemPortTable(void)
{

    DEBUGMSGTL(("lldpXdot3RemPortTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3RemPortTable", lldpXdot3RemPortTable_variables, variable3,
               lldpXdot3RemPortTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3REMPORTENTRY_INSTANCE_LEN  3

BOOL_T lldpXdot3RemPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3REMPORTENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    return TRUE;
}

/*
 * var_lldpXdot3RemPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3RemPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3REMPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3REMPORTENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3RemPortEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3REMPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3RemPortTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot3RemPortEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3RemPortTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot3RemPortEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3RemPortEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3RemPortEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3REMPORTENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3REMPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3RemPortAutoNegSupported:
        *var_len = sizeof(long_return);
        long_return = entry.rem_port_auto_neg_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPortAutoNegEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.rem_port_auto_neg_enable;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPortAutoNegAdvertisedCap:
        *var_len = SIZE_lldpXdot3RemPortAutoNegAdvertisedCap;
        memcpy(return_buf, &entry.rem_port_auto_neg_adv_cap,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpXdot3RemPortOperMauType:
        *var_len = sizeof(long_return);
        long_return = entry.rem_port_oper_mau_type;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3RemPowerTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3RemPowerTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,3 };

struct variable3 lldpXdot3RemPowerTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3RemPowerPortClass,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 1 }},
{LEAF_lldpXdot3RemPowerMDISupported,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXdot3RemPowerMDIEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 3 }},
{LEAF_lldpXdot3RemPowerPairControlable,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 4 }},
{LEAF_lldpXdot3RemPowerPairs,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 5 }},
{LEAF_lldpXdot3RemPowerClass,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemPowerTable, 3,  { 2, 1, 6 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3RemPowerTable module */
void
init_lldpXdot3RemPowerTable(void)
{

    DEBUGMSGTL(("lldpXdot3RemPowerTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3RemPowerTable", lldpXdot3RemPowerTable_variables, variable3,
               lldpXdot3RemPowerTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN  3

BOOL_T lldpXdot3RemPowerTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    return TRUE;
}

/*
 * var_lldpXdot3RemPowerTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3RemPowerTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3RemPowerEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3RemPowerTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot3RemPowerEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3RemPowerTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot3RemPowerEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3RemPowerEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3RemPowerEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3REMPOWERENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3RemPowerPortClass:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_port_class;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPowerMDISupported:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_mdi_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPowerMDIEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_mdi_enabled;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPowerPairControlable:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_pair_controlable;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPowerPairs:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_pairs;
        return (u_char *) &long_return;
    case LEAF_lldpXdot3RemPowerClass:
        *var_len = sizeof(long_return);
        long_return = entry.rem_power_class;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3RemLinkAggTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3RemLinkAggTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,3 };

struct variable3 lldpXdot3RemLinkAggTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3RemLinkAggStatus,  ASN_OCTET_STR,  RONLY,   var_lldpXdot3RemLinkAggTable, 3,  { 3, 1, 1 }},
{LEAF_lldpXdot3RemLinkAggPortId,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemLinkAggTable, 3,  { 3, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3RemLinkAggTable module */
void
init_lldpXdot3RemLinkAggTable(void)
{

    DEBUGMSGTL(("lldpXdot3RemLinkAggTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3RemLinkAggTable", lldpXdot3RemLinkAggTable_variables, variable3,
               lldpXdot3RemLinkAggTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN  3

BOOL_T lldpXdot3RemLinkAggTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    return TRUE;
}

/*
 * var_lldpXdot3RemLinkAggTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3RemLinkAggTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3RemLinkAggEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3RemLinkAggTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot3RemLinkAggEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3RemLinkAggTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot3RemLinkAggEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3RemLinkAggEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3RemLinkAggEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3REMLINKAGGENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3RemLinkAggStatus:
        *var_len = SIZE_lldpXdot3RemLinkAggStatus;
        memcpy(return_buf, &entry.rem_link_agg_status,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpXdot3RemLinkAggPortId:
        *var_len = sizeof(long_return);
        long_return = entry.rem_link_agg_port_id;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot3RemMaxFrameSizeTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot3RemMaxFrameSizeTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,4623,1,3 };

struct variable3 lldpXdot3RemMaxFrameSizeTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot3RemMaxFrameSize,  ASN_INTEGER,  RONLY,   var_lldpXdot3RemMaxFrameSizeTable, 3,  { 4, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot3RemMaxFrameSizeTable module */
void
init_lldpXdot3RemMaxFrameSizeTable(void)
{

    DEBUGMSGTL(("lldpXdot3RemMaxFrameSizeTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot3RemMaxFrameSizeTable", lldpXdot3RemMaxFrameSizeTable_variables, variable3,
               lldpXdot3RemMaxFrameSizeTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN  3

BOOL_T lldpXdot3RemMaxFrameSizeTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    return TRUE;
}

/*
 * var_lldpXdot3RemMaxFrameSizeTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot3RemMaxFrameSizeTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot3RemMaxFrameSizeTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot3RemMaxFrameSizeEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot3RemMaxFrameSizeTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot3RemMaxFrameSizeEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT3REMMAXFRAMESIZEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot3RemMaxFrameSize:
        *var_len = sizeof(long_return);
        long_return = entry.rem_max_frame_size;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

#endif
