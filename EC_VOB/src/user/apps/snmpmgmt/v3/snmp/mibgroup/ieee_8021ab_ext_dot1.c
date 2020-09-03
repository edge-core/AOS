#include "sys_cpnt.h"
#if (SYS_CPNT_LLDP_EXT == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "ieee_8021ab_ext_dot1.h"
#include "leaf_ieeelldp.h"  /* should named "leaf_ieee8021ab_ext_dot1.h" */
#include "leaf_ieeelldpext.h"  /* should named "leaf_ieee8021ab.h" */
#include "lldp_type.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "snmp_mgr.h"

/*
 * lldpXdot1ConfigPortVlanTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1ConfigPortVlanTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,1 };

struct variable3 lldpXdot1ConfigPortVlanTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1ConfigPortVlanTxEnable,  ASN_INTEGER,  RWRITE,  var_lldpXdot1ConfigPortVlanTable, 3,  { 1, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1ConfigPortVlanTable module */
void
init_lldpXdot1ConfigPortVlanTable(void)
{

    DEBUGMSGTL(("lldpXdot1ConfigPortVlanTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1ConfigPortVlanTable", lldpXdot1ConfigPortVlanTable_variables, variable3,
               lldpXdot1ConfigPortVlanTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot1ConfigPortVlanTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpPortConfigPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpPortConfigPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot1ConfigPortVlanTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1ConfigPortVlanTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1ConfigEntry_T  entry;

    switch(vp->magic)
    {
    case LEAF_lldpXdot1ConfigPortVlanTxEnable:
        *write_method = write_lldpXdot1ConfigPortVlanTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1ConfigPortVlanTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1ConfigPortVlanTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1CONFIGPORTVLANENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1ConfigPortVlanTxEnable:
        *var_len = sizeof(long_return);
        long_return = entry.port_vlan_tx_enable;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

int
write_lldpXdot1ConfigPortVlanTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpXdot1ConfigPortVlanTxEnable_true:
                    break;
                case VAL_lldpXdot1ConfigPortVlanTxEnable_false:
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
            I32_T value=0;
            UI32_T lldpPortConfigPortNum = 0;

            if(lldpXdot1ConfigPortVlanTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &lldpPortConfigPortNum)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetXdot1ConfigPortVlanTxEnable( lldpPortConfigPortNum, value) != TRUE)
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
 * lldpXdot1ConfigProtoVlanTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1ConfigProtoVlanTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,1 };

struct variable3 lldpXdot1ConfigProtoVlanTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1ConfigProtoVlanTxEnable,  ASN_INTEGER,  RWRITE,  var_lldpXdot1ConfigProtoVlanTable, 3,  { 3, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1ConfigProtoVlanTable module */
void
init_lldpXdot1ConfigProtoVlanTable(void)
{

    DEBUGMSGTL(("lldpXdot1ConfigProtoVlanTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1ConfigProtoVlanTable", lldpXdot1ConfigProtoVlanTable_variables, variable3,
               lldpXdot1ConfigProtoVlanTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1ConfigProtoVlanTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocProtoVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocProtoVlanId=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1ConfigProtoVlanTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1ConfigProtoVlanTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN] = {0};

    LLDP_MGR_Xdot1ConfigEntry_T  entry;
    UI32_T  proto_vlan = 0;


    switch(vp->magic)
    {
    case LEAF_lldpXdot1ConfigProtoVlanTxEnable:
        *write_method = write_lldpXdot1ConfigProtoVlanTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1ConfigProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.lport, &proto_vlan)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1ConfigProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.lport, &proto_vlan);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=proto_vlan;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1CONFIGPROTOVLANENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1ConfigProtoVlanTxEnable:
        *var_len = sizeof(long_return);
        long_return = entry.proto_vlan_tx_enable;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

int
write_lldpXdot1ConfigProtoVlanTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpXdot1ConfigProtoVlanTxEnable_true:
                    break;
                case VAL_lldpXdot1ConfigProtoVlanTxEnable_false:
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
            I32_T value=0;
            UI32_T lldpLocPortNum = 0;
            UI32_T lldpXdot1LocProtoVlanId = 0;

            if(lldpXdot1ConfigProtoVlanTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &lldpLocPortNum, &lldpXdot1LocProtoVlanId)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetXdot1ConfigProtoVlanTxEnable( lldpLocPortNum, value) != TRUE)
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
 * lldpXdot1ConfigVlanNameTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1ConfigVlanNameTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,1 };

struct variable3 lldpXdot1ConfigVlanNameTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1ConfigVlanNameTxEnable,  ASN_INTEGER,  RWRITE,  var_lldpXdot1ConfigVlanNameTable, 3,  { 2, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1ConfigVlanNameTable module */
void
init_lldpXdot1ConfigVlanNameTable(void)
{

    DEBUGMSGTL(("lldpXdot1ConfigVlanNameTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1ConfigVlanNameTable", lldpXdot1ConfigVlanNameTable_variables, variable3,
               lldpXdot1ConfigVlanNameTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1ConfigVlanNameTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocVlanId=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1ConfigVlanNameTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1ConfigVlanNameTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1ConfigEntry_T  entry;
    UI32_T  vid = 0;
    switch(vp->magic)
    {
    case LEAF_lldpXdot1ConfigVlanNameTxEnable:
        *write_method = write_lldpXdot1ConfigVlanNameTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1ConfigVlanNameTable_OidIndexToData(exact,compc,compl, &entry.lport, &vid)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1ConfigVlanNameTable_OidIndexToData(exact,compc,compl, &entry.lport, &vid);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=vid;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1CONFIGVLANNAMEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1ConfigVlanNameTxEnable:
        *var_len = sizeof(long_return);
        long_return = entry.vlan_name_tx_enable;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

int
write_lldpXdot1ConfigVlanNameTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpXdot1ConfigVlanNameTxEnable_true:
                    break;
                case VAL_lldpXdot1ConfigVlanNameTxEnable_false:
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
            I32_T value=0;
            UI32_T lldpLocPortNum = 0;
            UI32_T lldpXdot1LocVlanId = 0;

            if(lldpXdot1ConfigVlanNameTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &lldpLocPortNum, &lldpXdot1LocVlanId)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetXdot1ConfigVlanNameTxEnable( lldpLocPortNum, value) != TRUE)
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
 * lldpXdot1ConfigProtocolTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1ConfigProtocolTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,1 };

struct variable3 lldpXdot1ConfigProtocolTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1ConfigProtocolTxEnable,  ASN_INTEGER,  RWRITE,  var_lldpXdot1ConfigProtocolTable, 3,  { 4, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1ConfigProtocolTable module */
void
init_lldpXdot1ConfigProtocolTable(void)
{

    DEBUGMSGTL(("lldpXdot1ConfigProtocolTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1ConfigProtocolTable", lldpXdot1ConfigProtocolTable_variables, variable3,
               lldpXdot1ConfigProtocolTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1ConfigProtocolTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocProtocolIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocProtocolIndex=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1ConfigProtocolTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1ConfigProtocolTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1ConfigEntry_T  entry;
    UI32_T  protocol_index = 0;
    switch(vp->magic)
    {
    case LEAF_lldpXdot1ConfigProtocolTxEnable:
        *write_method = write_lldpXdot1ConfigProtocolTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1ConfigProtocolTable_OidIndexToData(exact,compc,compl, &entry.lport, &protocol_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1ConfigProtocolTable_OidIndexToData(exact,compc,compl, &entry.lport, &protocol_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1ConfigEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1ConfigEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=protocol_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1CONFIGPROTOCOLENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1ConfigProtocolTxEnable:
        *var_len = sizeof(long_return);
        long_return = entry.protocol_tx_enable;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

int
write_lldpXdot1ConfigProtocolTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpXdot1ConfigProtocolTxEnable_true:
                    break;
                case VAL_lldpXdot1ConfigProtocolTxEnable_false:
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
            I32_T value=0;
            UI32_T lldpLocPortNum = 0;
            UI32_T lldpXdot1LocProtocolIndex = 0;

            if(lldpXdot1ConfigProtocolTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &lldpLocPortNum, &lldpXdot1LocProtocolIndex)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetXdot1ConfigProtocolTxEnable( lldpLocPortNum, value) != TRUE)
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
 * lldpXdot1LocTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1LocTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,2 };

struct variable3 lldpXdot1LocTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1LocPortVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocTable, 3,  { 1, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1LocTable module */
void
init_lldpXdot1LocTable(void)
{

    DEBUGMSGTL(("lldpXdot1LocTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1LocTable", lldpXdot1LocTable_variables, variable3,
               lldpXdot1LocTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1LOCENTRY_INSTANCE_LEN  1

BOOL_T lldpXdot1LocTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1LOCENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    return TRUE;
}

/*
 * var_lldpXdot1LocTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1LocTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1LOCENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1LOCENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1LocEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1LOCENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1LocTable_OidIndexToData(exact,compc,compl, &entry.lport)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1LocEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1LocTable_OidIndexToData(exact,compc,compl, &entry.lport);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1LocEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1LocEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1LocEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1LOCENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1LOCENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1LocPortVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.port_vlan_id;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1LocProtoVlanTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1LocProtoVlanTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,2 };

struct variable3 lldpXdot1LocProtoVlanTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1LocProtoVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocProtoVlanTable, 3,  { 2, 1, 1 }},
#endif

{LEAF_lldpXdot1LocProtoVlanSupported,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocProtoVlanTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXdot1LocProtoVlanEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocProtoVlanTable, 3,  { 2, 1, 3 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1LocProtoVlanTable module */
void
init_lldpXdot1LocProtoVlanTable(void)
{

    DEBUGMSGTL(("lldpXdot1LocProtoVlanTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1LocProtoVlanTable", lldpXdot1LocProtoVlanTable_variables, variable3,
               lldpXdot1LocProtoVlanTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1LocProtoVlanTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocProtoVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocProtoVlanId=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1LocProtoVlanTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1LocProtoVlanTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1LocProtoVlanEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1LocProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.proto_vlan_id)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1LocProtoVlanEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1LocProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.proto_vlan_id);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1LocProtoVlanEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1LocProtoVlanEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1LocProtoVlanEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=entry.proto_vlan_id;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1LOCPROTOVLANENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1LocProtoVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.proto_vlan_id;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1LocProtoVlanSupported:
        *var_len = sizeof(long_return);
        long_return = entry.proto_vlan_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot1LocProtoVlanEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.proto_vlan_enabled;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1LocVlanNameTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1LocVlanNameTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,2 };

struct variable3 lldpXdot1LocVlanNameTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1LocVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocVlanNameTable, 3,  { 3, 1, 1 }},
#endif

{LEAF_lldpXdot1LocVlanName,  ASN_OCTET_STR,  RONLY,   var_lldpXdot1LocVlanNameTable, 3,  { 3, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1LocVlanNameTable module */
void
init_lldpXdot1LocVlanNameTable(void)
{

    DEBUGMSGTL(("lldpXdot1LocVlanNameTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1LocVlanNameTable", lldpXdot1LocVlanNameTable_variables, variable3,
               lldpXdot1LocVlanNameTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1LocVlanNameTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocVlanId=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1LocVlanNameTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1LocVlanNameTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1LocVlanNameEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1LocVlanNameTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.vlan_id)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1LocVlanNameEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1LocVlanNameTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.vlan_id);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1LocVlanNameEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1LocVlanNameEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1LocVlanNameEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=entry.vlan_id;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1LOCVLANNAMEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1LocVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.vlan_id;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1LocVlanName:
        *var_len  = entry.vlan_name_len;
        memcpy(return_buf, entry.vlan_name,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1LocProtocolTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1LocProtocolTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,2 };

struct variable3 lldpXdot1LocProtocolTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1LocProtocolIndex,  ASN_INTEGER,  RONLY,   var_lldpXdot1LocProtocolTable, 3,  { 4, 1, 1 }},
#endif

{LEAF_lldpXdot1LocProtocolId,  ASN_OCTET_STR,  RONLY,   var_lldpXdot1LocProtocolTable, 3,  { 4, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1LocProtocolTable module */
void
init_lldpXdot1LocProtocolTable(void)
{

    DEBUGMSGTL(("lldpXdot1LocProtocolTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1LocProtocolTable", lldpXdot1LocProtocolTable_variables, variable3,
               lldpXdot1LocProtocolTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN  2

BOOL_T lldpXdot1LocProtocolTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpLocPortNum, UI32_T *lldpXdot1LocProtocolIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpLocPortNum=compl[0];
    *lldpXdot1LocProtocolIndex=compl[1];
    return TRUE;
}

/*
 * var_lldpXdot1LocProtocolTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1LocProtocolTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1LocProtocolEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1LocProtocolTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.protocol_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetXdot1LocProtocolEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1LocProtocolTable_OidIndexToData(exact,compc,compl, &entry.lport, &entry.protocol_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_PMGR_GetXdot1LocProtocolEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1LocProtocolEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1LocProtocolEntry(&entry)!=TRUE)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */

    best_inst[0]=entry.lport;
    best_inst[1]=entry.protocol_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1LOCPROTOCOLENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1LocProtocolIndex:
        *var_len = sizeof(long_return);
        long_return = entry.protocol_idnex;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1LocProtocolId:
        *var_len  = entry.protocol_id_len;
        memcpy(return_buf, entry.protocol_id,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1RemTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1RemTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,3 };

struct variable3 lldpXdot1RemTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
{LEAF_lldpXdot1RemPortVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemTable, 3,  { 1, 1, 1 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1RemTable module */
void
init_lldpXdot1RemTable(void)
{

    DEBUGMSGTL(("lldpXdot1RemTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1RemTable", lldpXdot1RemTable_variables, variable3,
               lldpXdot1RemTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1REMENTRY_INSTANCE_LEN  3

BOOL_T lldpXdot1RemTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1REMENTRY_INSTANCE_LEN) /* the constant size index*/
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
 * var_lldpXdot1RemTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1RemTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1REMENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1REMENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1RemEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1REMENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1RemTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot1RemEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1RemTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot1RemEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1RemEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1RemEntry(&entry)!=TRUE)
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
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1REMENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1REMENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpXdot1RemPortVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.rem_port_vlan_id;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1RemProtoVlanTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1RemProtoVlanTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,3 };

struct variable3 lldpXdot1RemProtoVlanTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1RemProtoVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemProtoVlanTable, 3,  { 2, 1, 1 }},
#endif

{LEAF_lldpXdot1RemProtoVlanSupported,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemProtoVlanTable, 3,  { 2, 1, 2 }},
{LEAF_lldpXdot1RemProtoVlanEnabled,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemProtoVlanTable, 3,  { 2, 1, 3 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1RemProtoVlanTable module */
void
init_lldpXdot1RemProtoVlanTable(void)
{

    DEBUGMSGTL(("lldpXdot1RemProtoVlanTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1RemProtoVlanTable", lldpXdot1RemProtoVlanTable_variables, variable3,
               lldpXdot1RemProtoVlanTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN  4

BOOL_T lldpXdot1RemProtoVlanTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex, UI32_T *lldpXdot1RemProtoVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    *lldpXdot1RemProtoVlanId=compl[3];
    return TRUE;
}

/*
 * var_lldpXdot1RemProtoVlanTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1RemProtoVlanTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1RemProtoVlanEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1RemProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_proto_vlan_id)==FALSE)        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot1RemProtoVlanEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1RemProtoVlanTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_proto_vlan_id);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot1RemProtoVlanEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1RemProtoVlanEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1RemProtoVlanEntry(&entry)!=TRUE)
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
    best_inst[3]=entry.rem_proto_vlan_id;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1REMPROTOVLANENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1RemProtoVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.rem_proto_vlan_id;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1RemProtoVlanSupported:
        *var_len = sizeof(long_return);
        long_return = entry.rem_proto_vlan_supported;
        return (u_char *) &long_return;
    case LEAF_lldpXdot1RemProtoVlanEnabled:
        *var_len = sizeof(long_return);
        long_return = entry.rem_proto_vlan_enabled;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpXdot1RemVlanNameTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1RemVlanNameTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,3 };

struct variable3 lldpXdot1RemVlanNameTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1RemVlanId,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemVlanNameTable, 3,  { 3, 1, 1 }},
#endif

{LEAF_lldpXdot1RemVlanName,  ASN_OCTET_STR,  RONLY,   var_lldpXdot1RemVlanNameTable, 3,  { 3, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1RemVlanNameTable module */
void
init_lldpXdot1RemVlanNameTable(void)
{

    DEBUGMSGTL(("lldpXdot1RemVlanNameTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1RemVlanNameTable", lldpXdot1RemVlanNameTable_variables, variable3,
               lldpXdot1RemVlanNameTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN  4

BOOL_T lldpXdot1RemVlanNameTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex, UI32_T *lldpXdot1RemVlanId)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    *lldpXdot1RemVlanId=compl[3];
    return TRUE;
}

/*
 * var_lldpXdot1RemVlanNameTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1RemVlanNameTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1RemVlanNameEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1RemVlanNameTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_vlan_id)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot1RemVlanNameEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1RemVlanNameTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_vlan_id);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot1RemVlanNameEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1RemVlanNameEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1RemVlanNameEntry(&entry)!=TRUE)
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
    best_inst[3]=entry.rem_vlan_id;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1REMVLANNAMEENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1RemVlanId:
        *var_len = sizeof(long_return);
        long_return = entry.rem_vlan_id;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1RemVlanName:
        *var_len  = entry.rem_vlan_name_len;
        memcpy(return_buf, entry.rem_vlan_name,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}


/*
 * lldpXdot1RemProtocolTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpXdot1RemProtocolTable_variables_oid[] = { 1,0,8802,1,1,2,1,5,32962,1,3 };

struct variable3 lldpXdot1RemProtocolTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpXdot1RemProtocolIndex,  ASN_INTEGER,  RONLY,   var_lldpXdot1RemProtocolTable, 3,  { 4, 1, 1 }},
#endif

{LEAF_lldpXdot1RemProtocolId,  ASN_OCTET_STR,  RONLY,   var_lldpXdot1RemProtocolTable, 3,  { 4, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpXdot1RemProtocolTable module */
void
init_lldpXdot1RemProtocolTable(void)
{

    DEBUGMSGTL(("lldpXdot1RemProtocolTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpXdot1RemProtocolTable", lldpXdot1RemProtocolTable_variables, variable3,
               lldpXdot1RemProtocolTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN  4

BOOL_T lldpXdot1RemProtocolTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *lldpRemTimeMark, UI32_T *lldpRemLocalPortNum, UI32_T *lldpRemIndex, UI32_T *lldpXdot1RemProtocolIndex)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */
        if(compc != LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *lldpRemTimeMark=compl[0];
    *lldpRemLocalPortNum=compl[1];
    *lldpRemIndex=compl[2];
    *lldpXdot1RemProtocolIndex=compl[3];
    return TRUE;
}

/*
 * var_lldpXdot1RemProtocolTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpXdot1RemProtocolTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN] = {0};
    oid best_inst[LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN] = {0};
    LLDP_MGR_Xdot1RemProtocolEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {

        /* get index */
        if(lldpXdot1RemProtocolTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_protocol_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetXdot1RemProtocolEntry(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        /*get index*/
        lldpXdot1RemProtocolTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_protocol_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            /* get data */
            if ( LLDP_POM_GetXdot1RemProtocolEntry(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextXdot1RemProtocolEntry(&entry)!=TRUE)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextXdot1RemProtocolEntry(&entry)!=TRUE)
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
    best_inst[3]=entry.rem_protocol_index;
    memcpy(name+vp->namelen,best_inst,LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPXDOT1REMPROTOCOLENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpXdot1RemProtocolIndex:
        *var_len = sizeof(long_return);
        long_return = entry.rem_protocol_index;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpXdot1RemProtocolId:
        *var_len  = entry.rem_protocol_id_len;
        memcpy(return_buf, entry.rem_protocol_id,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

#endif
