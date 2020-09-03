/* =====================================================================================
 *  Module Name: MIB_OSPF.C
 *  Purpose :
 *  Notes:
 *  History :
 *  Modify: 2009.02.06 by Donny.li
* =====================================================================================
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "mib_ospf.h"
#include "netcfg_netdevice.h"
#include "sys_cpnt.h"
#include "sysORTable.h"
#include "sys_type.h"
#include "es3626a_superset.h"
#include <string.h>
#include "ip_lib.h"
#include "l_stdlib.h"
#include <asn1.h>
#include "table.h"
#include "vlan_lib.h"
#include "ospf_pmgr.h"
#include "ospf_type.h"
#include "sysfun.h"

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)

static BOOL_T ospfNetworkAreaTable_get(int compc,oid *compl,OSPF_TYPE_Network_Area_T *data);
static BOOL_T ospfNetworkAreaTable_next(int compc,oid *compl,OSPF_TYPE_Network_Area_T *data);
static BOOL_T ospfAreaTable_get(int compc,oid *compl,OSPF_TYPE_Area_T *data);
static BOOL_T ospfAreaTable_next(int compc,oid *compl,OSPF_TYPE_Area_T *data);
static BOOL_T ospfStubAreaTable_get(int compc,oid *compl,OSPF_TYPE_Stub_Area_T *data);
static BOOL_T ospfStubAreaTable_next(int compc,oid *compl, OSPF_TYPE_Stub_Area_T *data);
static BOOL_T ospfAreaAggregateTable_get(int compc,oid *compl,OSPF_TYPE_Area_Range_T *data);
static BOOL_T ospfAreaAggregateTable_next(int compc,oid *compl,OSPF_TYPE_Area_Range_T *data);
static BOOL_T ospfNssaTable_get(int compc,oid *compl,OSPF_TYPE_Nssa_Area_T *data);
static BOOL_T ospfNssaTable_next(int compc,oid *compl,OSPF_TYPE_Nssa_Area_T *data);

static BOOL_T ospfIfParamTable_get(int compc,oid *compl,OSPF_TYPE_IfParam_T *data);
static BOOL_T ospfIfParamTable_next(int compc,oid *compl,OSPF_TYPE_IfParam_T *data);
static BOOL_T ospfMultiProcessNbrTable_get(int compc,oid *compl,OSPF_TYPE_MultiProcessNbr_T *data);
static BOOL_T ospfMultiProcessNbrTable_next(int compc,oid *compl,OSPF_TYPE_MultiProcessNbr_T *data);
static BOOL_T ospfMultiProcessLsdbTable_get(int compc,oid *compl, OSPF_TYPE_MultiProcessLsdb_T *data);
static BOOL_T ospfMultiProcessLsdbTable_next(int compc,oid *compl, OSPF_TYPE_MultiProcessLsdb_T *data);
static BOOL_T ospfMultiProcessExtLsdbTable_get(int compc,oid *compl, OSPF_TYPE_MultiProcessExtLsdb_T *data);
static BOOL_T ospfMultiProcessExtLsdbTable_next(int compc,oid *compl, OSPF_TYPE_MultiProcessExtLsdb_T *data);
static BOOL_T ospfMultiProcessIfAuthMd5Table_get(int compc,oid *compl, OSPF_TYPE_MultiProcessIfAuthMd5_T *data);
static BOOL_T ospfMultiProcessIfAuthMd5Table_next(int compc,oid *compl, OSPF_TYPE_MultiProcessIfAuthMd5_T *data);
static BOOL_T ospfMultiProcessSystemTable_get(int compc,oid *compl, OSPF_TYPE_MultiProcessSystem_T *data);
static BOOL_T ospfMultiProcessSystemTable_next(int compc,oid *compl, OSPF_TYPE_MultiProcessSystem_T *data);



#define MultiProcSummaryAddrEntry_INSTANCE_LEN    8
#define MultiProcSummaryAddrType                  1
#define MultiProcSummaryAddrStart                 3
#define ADDRESS_IPV4_SIZE                         4

#define MultiProcRedistEntry_INSTANCE_LEN    2


int
write_ospfMultiProcRedistributeEntry(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    long    value;
    UI32_T  gauge;
    u_int8_t magic;
    char protocol[10] = {0};
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    magic= name[name_len - 3];
    entry.proc_id = name[name_len - 2];
    entry.proto = name[name_len - 1];



    if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_RIP)
    {
        strcpy(protocol,"rip");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_STATIC)
    {
        strcpy(protocol,"static");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_CONNECTED)
    {
        strcpy(protocol,"connected");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_BGP)
    {
        strcpy(protocol,"bgp");
    }

    switch (action) {
    case RESERVE1:
        if (name_len!= MultiProcRedistEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        switch(magic)
        {
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE:
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC:
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS:
                if (var_val_type != ASN_INTEGER)
                    return SNMP_ERR_WRONGTYPE;
                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG:
                if (var_val_type != ASN_GAUGE)
                    return SNMP_ERR_WRONGTYPE;
                break;

            default:
                break;
        }

        if (var_val_len > sizeof(long)) {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        value = *(long *)var_val;

        switch(magic)
        {
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE:
                if((value <OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE_1) || (value >OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE_2))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC:
                if((value <OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_MIN) || (value >OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG:
                gauge = value;
                if((gauge <OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG_MIN) || (gauge >OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS:
                if((value <OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS_MIN) || (value >OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            default:
                break;
        }
        break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        value = *(long *)var_val;
        switch(magic)
        {
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE:
                if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(OSPF_PMGR_RedistributeMetricTypeSet(vr_id, entry.proc_id, protocol, value)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC:
                if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(OSPF_PMGR_RedistributeMetricSet(vr_id, entry.proc_id, protocol, value)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG:
                gauge = value;
                if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(OSPF_PMGR_RedistributeTagSet(vr_id, entry.proc_id, protocol, gauge)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS:
                switch(value)
                {
                    case OSPF_SNMP_ROW_STATUS_ACTIVE:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDGO:
                    {
                        if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) == NETCFG_TYPE_OK)
                        {
                            return NETCFG_TYPE_OK;
                        }
                        if(OSPF_PMGR_RedistributeProtoSet(vr_id, entry.proc_id, protocol)!= NETCFG_TYPE_OK)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_DESTROY:
                    {
                        if(OSPF_PMGR_RedistributeProtoUnset(vr_id, entry.proc_id, protocol))
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }

                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_NOTINSERVICE:
                    case OSPF_SNMP_ROW_STATUS_NOTREADY:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDWAIT:
                    default:
                        return SNMP_ERR_COMMITFAILED;
                        break;
                }
                break;
            default:
                break;
        }
    }
    break;

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
write_ospfMultiProcRedistributeEntryChar(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    u_int8_t magic;
    //char    rmap[OSPF_MULTI_PROCESS_REDISTRIBUTE_NAME_LENGTH_MAX + 1] = {0};
    //char    fmap[OSPF_MULTI_PROCESS_REDISTRIBUTE_NAME_LENGTH_MAX + 1] = {0};
    char protocol[10] = {0};
    //UI32_T   vr_id = SYS_DFLT_VR_ID;
    //UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    magic= name[oid_name_length - 1];
    entry.proc_id = name[oid_name_length];
    entry.proto = name[++oid_name_length];

    if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_RIP)
    {
        strcpy(protocol,"rip");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_STATIC)
    {
        strcpy(protocol,"static");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_CONNECTED)
    {
        strcpy(protocol,"connected");
    }
    else if(entry.proto == OSPF_MULTI_PROCESS_REDISTRIBUTE_BGP)
    {
        strcpy(protocol,"bgp");
    }

    switch (action) {

    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        switch(magic)
        {
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTE_MAP:
                if ((var_val_len > OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTEMAP_NAME_MAX) || (var_val_len <= OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTEMAP_NAME_MIN))
                {
                    return SNMP_ERR_WRONGLENGTH;
                }
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_FILTER_LIST_NAME:
                if ((var_val_len > OSPF_MULTI_PROCESS_REDISTRIBUTE_LIST_NAME_MAX) || (var_val_len <= OSPF_MULTI_PROCESS_REDISTRIBUTE_LIST_NAME_MIN))
                {
                    return SNMP_ERR_WRONGLENGTH;
                }
            default:
                break;
        }
        break;
    case RESERVE2:

        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        switch(magic)
        {
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTE_MAP:
                 break;
#if 0
                if(var_val_len)
                {
                    strncpy(rmap, (char *)var_val, var_val_len);
                }
                if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(OSPF_PMGR_RedistributeRoutemapSet(vr_id, entry.proc_id, protocol, rmap)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;
#endif
            case OSPF_MULTI_PROCESS_REDISTRIBUTE_FILTER_LIST_NAME:
                break;
#if 0

                if(var_val_len)
                {
                    strncpy(fmap, (char *)var_val, var_val_len);
                }
                if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(OSPF_PMGR_RedistributeFilterlistnameSet(vr_id, entry.proc_id, protocol, fmap)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
#endif
            default:
                break;
        }
    }
    break;

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

static BOOL_T ospfMultiProcRedistTable_get(int      compc,
                                oid     *compl,
                                OSPF_TYPE_Multi_Proc_Redist_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    entry->proc_id = compl[0];
    entry->proto = compl[1];

    if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;

    /*End of if */
}
static BOOL_T ospfMultiProcRedistTable_next(int   compc,
                                 oid     *compl,
                                 OSPF_TYPE_Multi_Proc_Redist_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if(compc > 0)
        entry->proc_id = compl[0];
    else
        entry->proc_id = 0xffffffff;
    if(compc > 1)
        entry->proto = compl[1];

    if(OSPF_PMGR_GetNextMultiProcRedistEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME :var_ospfMultiProcessRedistTable
 * PURPOSE:
 *       var_ospfMultiProcessRedistTable;
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessRedistTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    UI32_T compc = 0;
    oid compl[MultiProcRedistEntry_INSTANCE_LEN] = {0};

    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, MultiProcRedistEntry_INSTANCE_LEN);

    switch (v->magic)
    {
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE:               /* 2 -- ospfRedistributeMetricType. */
           *write_method = write_ospfMultiProcRedistributeEntry;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC:                    /* 3 -- ospfRedistributeMetric. */
           *write_method = write_ospfMultiProcRedistributeEntry;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTE_MAP:                 /* 4 -- ospfRedistributeRouteMap. */
           *write_method = write_ospfMultiProcRedistributeEntryChar;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG:                       /* 5 -- ospfRedistributeTag. */
           *write_method = write_ospfMultiProcRedistributeEntry;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS:                    /* 6 -- ospfRedistributeStatus. */
           *write_method = write_ospfMultiProcRedistributeEntry;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_FILTER_LIST_NAME:          /* 7-- ospfRedistributeFilterListName. */
           *write_method = write_ospfMultiProcRedistributeEntryChar;
           break;
       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcRedistTable_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {

        if( ospfMultiProcRedistTable_next(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }
    }

    memcpy(name, v->name, v->namelen*sizeof(oid));
    name[v->namelen] = entry.proc_id;
    name[v->namelen + 1] = entry.proto;
    *length = v->namelen +MultiProcRedistEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic)
    {
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE:               /* 2 -- ospfRedistributeMetricType. */
           long_return = entry.metric_type;
           return (u_char*) &long_return;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC:                    /* 3 -- ospfRedistributeMetric. */
           long_return = entry.metric;
           return (u_char*) &long_return;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG:                       /* 4 -- ospfRedistributeTag. */
           long_return = entry.tag;
           return (u_char*) &long_return;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_FILTER_LIST_NAME:          /* 5-- ospfRedistributeFilterListName. */
           *var_len = strlen (entry.listname);
           if(strlen(entry.listname))
               strcpy((char *)return_buf, entry.listname);
           else
               strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS:                    /* 6 -- ospfRedistributeStatus. */
           long_return = entry.status;
           return (u_char*) &long_return;
           break;
       case OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTE_MAP:                 /* 7 -- ospfRedistributeRouteMap. */
           *var_len = strlen (entry.mapname);
           if(strlen(entry.mapname))
               strcpy((char *)return_buf, entry.mapname);
           else
               strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
       default:
           ERROR_MSG("");
           break;
    }
    return NULL;
}


#define MultiProcIfEntry_INSTANCE_LEN    6
#define MultiProcIfAddrType              1
#define MultiProcIfStart                 2

int
write_ospfMultiProcIfEntry(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    u_int8_t magic;
    long    value;
    int i;
    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T address;
    int     size;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Msg_OspfInterfac_T entry;
    BOOL_T addr_flag = FALSE;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Msg_OspfInterfac_T));

    magic= name[oid_name_length - 1];

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + MultiProcIfStart + i];
    }
    IP_LIB_ArraytoUI32(addr, &address);
    entry.if_addr.s_addr = address;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER)
            return SNMP_ERR_WRONGTYPE;
        if (var_val_len > sizeof(long))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:

        value = *(long *)var_val;
        switch(magic)
        {
            case OSPFMULTIPROCESSIFCOST:
                if((value < OSPF_MULTI_PROCESS_IF_COST_MIN) || (value > OSPF_MULTI_PROCESS_IF_COST_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFMTU:
                if((value <OSPF_MULTI_PROCESS_IF_MTU_MIN) || (value >OSPF_MULTI_PROCESS_IF_MTU_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFMTUIGNORE:
                if((value <OSPF_MULTI_PROCESS_IF_MUTIGNORE_MIN) || (value >OSPF_MULTI_PROCESS_IF_MUTIGNORE_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFRTRPRIORITY:
                if((value <OSPF_MULTI_PROCESS_IF_PRIORITY_MIN) || (value >OSPF_MULTI_PROCESS_IF_PRIORITY_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFTRANSITDELAY:
                if((value <OSPF_MULTI_PROCESS_IF_TRANSITDELAY_MIN) || (value >OSPF_MULTI_PROCESS_IF_TRANSITDELAY_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFRETRANSINTERVAL:
                if((value <OSPF_MULTI_PROCESS_IF_RETRANSINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_IF_RETRANSINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFHELLOINTERVAL:
                if((value <OSPF_MULTI_PROCESS_IF_HELLOINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_IF_HELLOINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFRTRDEADINTERVAL:
                if((value <OSPF_MULTI_PROCESS_IF_RTRDEADINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_IF_RTRDEADINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFSTATUS:
                if((value <OSPF_MULTI_PROCESS_IF_STATUS_MIN) || (value >OSPF_MULTI_PROCESS_IF_STATUS_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSIFAUTHTYPE:
                if((value <OSPF_MULTI_PROCESS_IF_AUTHTYPE_MIN) || (value >OSPF_MULTI_PROCESS_IF_AUTHTYPE_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            default:
                break;
        }
        break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        value = * (long *) var_val;
        if (name_len!= MultiProcIfEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_COMMITFAILED;
        }
        if((name[oid_name_length] != MultiProcIfAddrType) || (name[oid_name_length + 1] != SYS_ADPT_IPV4_ADDR_LEN))
            return SNMP_ERR_COMMITFAILED;

        switch(magic)
        {

            case OSPFMULTIPROCESSIFCOST:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfCostSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;
            case OSPFMULTIPROCESSIFMTU:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfMtuSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFMTUIGNORE:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfMtuIgnoreSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFRTRPRIORITY:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfPrioritySet(vr_id, entry.ifindex, (UI8_T)value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFTRANSITDELAY:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfTransmitDelaySet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFRETRANSINTERVAL:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfRetransmitIntervalSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;

            case OSPFMULTIPROCESSIFHELLOINTERVAL:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfHelloIntervalSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFRTRDEADINTERVAL:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfDeadIntervalSet(vr_id, entry.ifindex, value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSIFSTATUS:
                switch(value)
                {
                    case OSPF_SNMP_ROW_STATUS_ACTIVE:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDGO:
                    {
                        if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) == NETCFG_TYPE_OK)
                        {
                            return NETCFG_TYPE_OK;
                        }
/*only support read only*/
#if 0
                        if(OSPF_PMGR_IfStatusSet(vr_id, entry.proc_id, protocol)!= NETCFG_TYPE_OK)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
#endif
                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_DESTROY:
                    {
/*only support read only*/
#if 0
                        if(OSPF_PMGR_IfStatusUnset(vr_id, entry.proc_id, protocol))
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
#endif
                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_NOTINSERVICE:
                    case OSPF_SNMP_ROW_STATUS_NOTREADY:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDWAIT:
                    default:
                        return SNMP_ERR_COMMITFAILED;
                        break;
                }
                break;

            case OSPFMULTIPROCESSIFAUTHTYPE:
                if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                addr_flag = TRUE;
                if(OSPF_PMGR_IfAuthenticationTypeSet(vr_id, entry.ifindex, (UI8_T)value, addr_flag,
                                        entry.if_addr)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;

            default:
                break;
        }
    }
    break;

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
write_ospfMultiProcIfEntryIP(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    int i;
    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T address;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Msg_OspfInterfac_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Msg_OspfInterfac_T));

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + MultiProcIfStart + i];
    }
    IP_LIB_ArraytoUI32(addr, &address);
    entry.if_addr.s_addr = address;

    switch (action) {
    case RESERVE1:

        if (var_val_type != ASN_IPADDRESS)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        break;

    case RESERVE2:
        if (var_val_len != SYS_TYPE_IPV4_ADDR_LEN)
        {
            return SNMP_ERR_WRONGLENGTH;
        }

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        if (name_len!= MultiProcIfEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_COMMITFAILED;
        }

        if((name[oid_name_length] != MultiProcIfAddrType) || (name[oid_name_length + 1] != SYS_ADPT_IPV4_ADDR_LEN))
            return SNMP_ERR_COMMITFAILED;

        if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
        {
            return SNMP_ERR_COMMITFAILED;
        }
/* not support currently*/
#if 0
        if(OSPF_PMGR_IfAreaIdSet(vr_id, vrf_id, pal_addr, area_id)!= NETCFG_TYPE_OK)
        {
            return SNMP_ERR_COMMITFAILED;
        }
#endif
    }
    break;

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
write_ospfMultiProcIfEntryChar(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    char    auth_key[OSPF_MULTI_PROCESS_IF_AUTHKEY_LENGTH_MAX + 1] = {0};
    int i;
    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T address;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Msg_OspfInterfac_T entry;
    BOOL_T addr_flag = FALSE;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Msg_OspfInterfac_T));

    if((name[oid_name_length] != MultiProcIfAddrType) || (name[oid_name_length + 1] != SYS_ADPT_IPV4_ADDR_LEN))
        return SNMP_ERR_COMMITFAILED;

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + MultiProcIfStart + i];
    }
    IP_LIB_ArraytoUI32(addr, &address);
    entry.if_addr.s_addr = address;

    switch (action) {

    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if ((var_val_len > OSPF_MULTI_PROCESS_IF_AUTHKEY_LENGTH_MAX) || (var_val_len < 0))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;
    case RESERVE2:

        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        if (name_len!= MultiProcIfEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_COMMITFAILED;
        }
        if((name[oid_name_length] != MultiProcIfAddrType) || (name[oid_name_length + 1] != SYS_ADPT_IPV4_ADDR_LEN))
            return SNMP_ERR_COMMITFAILED;
        if(var_val_len)
        {
            strncpy(auth_key, (char *)var_val, var_val_len);
        }
        if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, &entry) != NETCFG_TYPE_OK)
        {
            return SNMP_ERR_COMMITFAILED;
        }
        addr_flag = TRUE;
        if(var_val_len)
        {
            if(OSPF_PMGR_IfAuthenticationKeySet(vr_id, entry.ifindex, auth_key, addr_flag, entry.if_addr)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        else
        {
            if(OSPF_PMGR_IfAuthenticationKeyUnset(vr_id, entry.ifindex, addr_flag, entry.if_addr)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
    }
    break;

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

static BOOL_T ospfMultiProcIfTable_get(int      compc,
                                oid     *compl,
                                OSPF_TYPE_Msg_OspfInterfac_T *entry)
{

    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T ipaddr = 0;
    int i;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (compc !=MultiProcIfEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    if (compl[0] != MultiProcIfAddrType)
    {
        return FALSE;
    }

    if (compl[1] != SYS_ADPT_IPV4_ADDR_LEN)
    {
        return FALSE;
    }

    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN;i++)
    {
        addr[i] = compl[i+2];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->if_addr.s_addr = ipaddr;

    if(OSPF_PMGR_GetMultiProcIfEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}
static BOOL_T ospfMultiProcIfTable_next(int   compc,
                                 oid     *compl,
                                 OSPF_TYPE_Msg_OspfInterfac_T *entry)
{
    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T ipaddr = 0;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    int i;

    memset(entry,0,sizeof(OSPF_TYPE_Msg_OspfInterfac_T));

    if ((compl[0] == 0) || ((compl[0] == MultiProcIfAddrType) && (compl[1] >= 0) && (compl[1] <SYS_ADPT_IPV4_ADDR_LEN)))
    {
        SNMP_MGR_ConvertRemainToZero(MultiProcIfStart,MultiProcIfEntry_INSTANCE_LEN, compl);
    }
    else if((compl[0] > MultiProcIfAddrType) || compl[1] > SYS_ADPT_IPV4_ADDR_LEN)
    {
        return FALSE;
    }


    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN;i++)
    {
        addr[i] = compl[i+2];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->if_addr.s_addr = ipaddr;

    if(OSPF_PMGR_GetNextMultiProcIfEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME :var_ospfSummaryAddressTable
 * PURPOSE:
 *       var_ospfSummaryAddressTable;
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessIfTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_Msg_OspfInterfac_T entry;
    UI32_T compc = 0;
    oid compl[MultiProcIfEntry_INSTANCE_LEN] = {0};
    oid best_inst[MultiProcIfEntry_INSTANCE_LEN];
    UI8_T  addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;
    int offset = 0;

    memset(&entry,0,sizeof(OSPF_TYPE_Msg_OspfInterfac_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, MultiProcIfEntry_INSTANCE_LEN);

    switch (v->magic)
    {
       case OSPFMULTIPROCESSIFCOST:
           *write_method = write_ospfMultiProcIfEntry;
           break;
#if 0
       case OSPFMULTIPROCESSIFMTU:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFMTUIGNORE:
           *write_method = write_ospfMultiProcIfEntry;
           break;
#endif
/* not support write*/
#if 0
       case OSPFMULTIPROCESSIFAREAID:
           *write_method = write_ospfMultiProcIfEntryIP;
           break;
#endif
       case OSPFMULTIPROCESSIFRTRPRIORITY:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFTRANSITDELAY:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFRETRANSINTERVAL:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFHELLOINTERVAL:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFRTRDEADINTERVAL:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       case OSPFMULTIPROCESSIFAUTHKEY:
           *write_method = write_ospfMultiProcIfEntryChar;
           break;
#if 0
       case OSPFMULTIPROCESSIFSTATUS:
           *write_method = write_ospfMultiProcIfEntry;
           break;
#endif
       case OSPFMULTIPROCESSIFAUTHTYPE:
           *write_method = write_ospfMultiProcIfEntry;
           break;
       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcIfTable_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {
       if( ospfMultiProcIfTable_next(compc, compl, &entry) != TRUE)
       {
           return NULL;
       }
    }

    best_inst[offset] = MultiProcIfAddrType;
    best_inst[++offset] = SYS_ADPT_IPV4_ADDR_LEN;
    IP_LIB_UI32toArray(entry.if_addr.s_addr, addr);
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = addr[i];
    memcpy(name, v->name, v->namelen*sizeof(oid));
    memcpy(name + v->namelen, best_inst, MultiProcIfEntry_INSTANCE_LEN*sizeof(oid));
    *length = v->namelen +MultiProcIfEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic)
    {
       case OSPFMULTIPROCESSIFCOST:
           long_return = entry.output_cost;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFMTU:
           long_return = entry.mtu;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFMTUIGNORE:
           long_return = entry.mtu_ignore;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFAREAID:
           *var_len = sizeof(ipaddr_return);
           IP_LIB_UI32toArray(entry.area_id.s_addr, (UI8_T *)&ipaddr_return);
           return (u_char*) &ipaddr_return;
           break;
       case OSPFMULTIPROCESSIFRTRPRIORITY:
           long_return = entry.priority;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFTRANSITDELAY:
           long_return = entry.transmit_delay;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFRETRANSINTERVAL:
           long_return = entry.retransmit_interval;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFHELLOINTERVAL:
           long_return = entry.hello_interval;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFRTRDEADINTERVAL:
           long_return = entry.dead_interval;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFSTATE:
           long_return = entry.state;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFDESIGNATEDROUTER:
           *var_len = sizeof(ipaddr_return);
           IP_LIB_UI32toArray(entry.dr_addr.s_addr, (UI8_T *)&ipaddr_return);
           return (u_char*) &ipaddr_return;
           break;

       case OSPFMULTIPROCESSIFBACKUPDESIGNATEDROUTER:
           *var_len = sizeof(ipaddr_return);
           IP_LIB_UI32toArray(entry.bdr_addr.s_addr, (UI8_T *)&ipaddr_return);
           return (u_char*) &ipaddr_return;
           break;
       case OSPFMULTIPROCESSIFEVENTS:
           long_return = entry.events;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFAUTHKEY:
           /*reading the auth key will always return zero */
#if 0
           *var_len = strlen (entry.auth_key);
           if(strlen(entry.auth_key))
               strcpy((char *)return_buf, entry.auth_key);
           else
#endif
               *var_len = 0;
               strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
       case OSPFMULTIPROCESSIFSTATUS:
           long_return = OSPF_SNMP_ROW_STATUS_ACTIVE;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFAUTHTYPE:
           long_return = entry.auth_type;
           return (u_char*) &long_return;
           break;
       case OSPFMULTIPROCESSIFDESIGNATEDROUTERID:
           *var_len = sizeof(ipaddr_return);
           IP_LIB_UI32toArray(entry.dr_id.s_addr, (UI8_T *)&ipaddr_return);
           return (u_char*) &ipaddr_return;
           break;
       case OSPFMULTIPROCESSIFBACKUPDESIGNATEDROUTERID:
           *var_len = sizeof(ipaddr_return);
           IP_LIB_UI32toArray(entry.bdr_id.s_addr, (UI8_T *)&ipaddr_return);
           return (u_char*) &ipaddr_return;
           break;
       default:
           ERROR_MSG("");
           break;
    }
    return NULL;
}



int
write_ospfMultiProcSummaryAddrEntry(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    long    value;
    UI8_T   addr[PREFIX_STRING_SIZE+1]={0};
    int i;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Multi_Proc_Summary_Addr_T entry;
    int     addr_len;
    UI32_T oid_name_length = OSPFMULTIPROCSUMMARYADDROIDNAMELENGTH;

    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Summary_Addr_T));

    entry.proc_id = name[oid_name_length];
    entry.summary_address_type = name[oid_name_length + 1];
    addr_len = name[oid_name_length + 2];
    for(i = 0; i < SUMMARY_ADDR_MAX_SIZE; i++)
    {
        addr[i] = name[oid_name_length + MultiProcSummaryAddrStart + i];
    }
    IP_LIB_ArraytoUI32(addr, &(entry.summary_address));
    entry.summary_pfxlen = name[oid_name_length + MultiProcSummaryAddrStart + SUMMARY_ADDR_MAX_SIZE];
    switch (action) {
    case RESERVE1:

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
        if ((value <OSPF_SNMP_ROW_STATUS_ACTIVE) || (value >OSPF_SNMP_ROW_STATUS_DESTROY))
            return SNMP_ERR_WRONGVALUE;
        break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        value = * (long *) var_val;
        switch(value)
        {
            case OSPF_SNMP_ROW_STATUS_ACTIVE:
            case OSPF_SNMP_ROW_STATUS_CREATEANDGO:
            {
                if (name_len!= MultiProcSummaryAddrEntry_INSTANCE_LEN + oid_name_length)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if((entry.summary_address_type != MultiProcSummaryAddrType) || (addr_len != ADDRESS_IPV4_SIZE))
                    return SNMP_ERR_COMMITFAILED;
                if(OSPF_PMGR_GetMultiProcSummaryAddrEntry(vr_id, vrf_id, &entry) == NETCFG_TYPE_OK)
                {
                    return NETCFG_TYPE_OK;
                }
                if(OSPF_PMGR_SummaryAddressSet(vr_id, entry.proc_id, entry.summary_address, entry.summary_pfxlen)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
                break;
            case OSPF_SNMP_ROW_STATUS_DESTROY:
            {
                if((entry.summary_address_type != MultiProcSummaryAddrType) || (addr_len != ADDRESS_IPV4_SIZE))
                    return SNMP_ERR_COMMITFAILED;
                if(OSPF_PMGR_SummaryAddressUnset(vr_id, entry.proc_id, entry.summary_address, entry.summary_pfxlen)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

            }
                break;
            case OSPF_SNMP_ROW_STATUS_NOTINSERVICE:
            case OSPF_SNMP_ROW_STATUS_NOTREADY:
            case OSPF_SNMP_ROW_STATUS_CREATEANDWAIT:
            default:
                return SNMP_ERR_COMMITFAILED;
                break;
        }
    }
    break;

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

static BOOL_T ospfMultiProcSummaryAddrTable_get(int      compc,
                                oid     *compl,
                                OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry)
{

    UI8_T  addr[SUMMARY_ADDR_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    int i;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (compc !=MultiProcSummaryAddrEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    if (compl[1] != MultiProcSummaryAddrType)
    {
        return FALSE;
    }

    if (compl[2] != ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    for(i = 0;i< SUMMARY_ADDR_MAX_SIZE;i++)
    {
        addr[i] = compl[i+3];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->proc_id = compl[0];
    entry->summary_address_type = compl[1];
    entry->summary_pfxlen = compl[MultiProcSummaryAddrEntry_INSTANCE_LEN - 1];
    entry->summary_address = ipaddr;

    if(OSPF_PMGR_GetMultiProcSummaryAddrEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}
static BOOL_T ospfMultiProcSummaryAddrTable_next(int   compc,
                                 oid     *compl,
                                 OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry)
{
    UI8_T  addr[SUMMARY_ADDR_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    int i;

    entry->proc_id = compl[0];
    entry->indexlen = MultiProcSummaryAddrEntry_INSTANCE_LEN;

    if(compc == 0)
    {
        entry->proc_id = 0xffffffff;
        entry->indexlen = 0;
    }
    else if(compl[1] > MultiProcSummaryAddrType)
    {
        entry->proc_id++;
        SNMP_MGR_ConvertRemainToZero(MultiProcSummaryAddrStart,MultiProcSummaryAddrEntry_INSTANCE_LEN, compl);
    }
    else if ((compl[1] == 0) || ((compl[1] == MultiProcSummaryAddrType) && (compl[2] >= 0) && (compl[2] <ADDRESS_IPV4_SIZE)))
    {
        SNMP_MGR_ConvertRemainToZero(MultiProcSummaryAddrStart,MultiProcSummaryAddrEntry_INSTANCE_LEN, compl);
    }
    else if( compl[2] > INET_ADDRESS_IPV4_SIZE)
    {
        entry->proc_id++;
        SNMP_MGR_ConvertRemainToZero(MultiProcSummaryAddrStart,MultiProcSummaryAddrEntry_INSTANCE_LEN, compl);
    }

    for(i = 0;i< SUMMARY_ADDR_MAX_SIZE;i++)
    {
        addr[i] = compl[i+3];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->summary_address_type = compl[1];
    entry->summary_pfxlen = compl[MultiProcSummaryAddrEntry_INSTANCE_LEN - 1];
    entry->summary_address = ipaddr;


    if(OSPF_PMGR_GetNextMultiProcSummaryAddrEntry(vr_id, vrf_id, entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME :var_ospfSummaryAddressTable
 * PURPOSE:
 *       var_ospfSummaryAddressTable;
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessSummaryAddressTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_Multi_Proc_Summary_Addr_T entry;
    UI32_T compc = 0;
    oid compl[MultiProcSummaryAddrEntry_INSTANCE_LEN] = {0};
    oid best_inst[MultiProcSummaryAddrEntry_INSTANCE_LEN];
    UI8_T  addr[SUMMARY_ADDR_MAX_SIZE]= {0};
    int i;
    int offset = 0;

    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Summary_Addr_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, MultiProcSummaryAddrEntry_INSTANCE_LEN);

    switch (v->magic)
    {
       case OSPFMULTIPROCSUMMARYSTATUS:
           *write_method = write_ospfMultiProcSummaryAddrEntry;
           break;

       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcSummaryAddrTable_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {

        if( ospfMultiProcSummaryAddrTable_next(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }
    }

    best_inst[offset] = entry.proc_id;
    best_inst[++offset] = MultiProcSummaryAddrType;
    best_inst[++offset] = ADDRESS_IPV4_SIZE;
    IP_LIB_UI32toArray(entry.summary_address, addr);
    for(i = 0; i < SUMMARY_ADDR_MAX_SIZE; i++)
        best_inst[++offset] = addr[i];
    best_inst[++offset] = entry.summary_pfxlen;
    memcpy(name, v->name, v->namelen*sizeof(oid));

    memcpy(name + v->namelen, best_inst, MultiProcSummaryAddrEntry_INSTANCE_LEN*sizeof(oid));
    *length = v->namelen +MultiProcSummaryAddrEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic) {

       case OSPFMULTIPROCSUMMARYSTATUS:
           long_return = entry.summary_status;
           return (u_char*) &long_return;
           break;
       default:
           ERROR_MSG("");
    }
    return NULL;
}


#define MultiProcVirtIfAuthMd5Key_INSTANCE_LEN 10

int
write_MultiProcVirtIfAuthMd5Key(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
    {

        int i;
        UI32_T proc_id;
        UI32_T format;
        UI32_T  vr_id = SYS_DFLT_VR_ID;
        UI8_T  area[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        UI8_T  neighbor[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        UI32_T  area_id;
        OSPF_TYPE_Area_Virtual_Link_Para_T entry;
        UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

        memset(&entry,0,sizeof(OSPF_TYPE_Area_Virtual_Link_Para_T));

        proc_id = name[oid_name_length];
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            area[i] = name[oid_name_length + 1 + i];
        }
            for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            neighbor[i] = name[oid_name_length + 5 + i];
        }
        IP_LIB_ArraytoUI32(area, &area_id);
        IP_LIB_ArraytoUI32(neighbor, &entry.vlink_addr);
        entry.key_id = name[oid_name_length + 9];

        switch (action) {

        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if ((var_val_len > OSPFMULTIPROCVIRTIFAUTHMD5KEYLENGTH) || (var_val_len < 0))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case RESERVE2:

            break;

        case FREE:
            /*
             * Release any resources that have been allocated
             */
            break;

        case ACTION:
        {

            format = OSPF_AREA_ID_FORMAT_ADDRESS;
            if(var_val_len)
            {
                strncpy(entry.md5_key, (char *)var_val, var_val_len);
            }
            SET_FLAG(entry.config, OSPF_AREA_VLINK_MESSAGEDIGESTKEY);
            if((entry.key_id > OSPFMULTIPROCVIRTIFAUTHMD5KEYIDMAX) && (entry.key_id < OSPFMULTIPROCVIRTIFAUTHMD5KEYIDMIN))
                return SNMP_ERR_COMMITFAILED;
            if(var_val_len)
            {
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_AreaVirtualLinkUnset(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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


static BOOL_T ospfMultiProcVirtIfAuthMd5Table_get(int      compc, oid     *compl,
                                                                OSPF_TYPE_Vlink_T *entry)
{

    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  neighbor[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;

    memset(entry,0,sizeof(OSPF_TYPE_Vlink_T));
    if (compc !=MultiProcVirtIfAuthMd5Key_INSTANCE_LEN)
    {
        return FALSE;
    }

    entry->vr_id = SYS_DFLT_VR_ID;
    entry->proc_id = compl[0];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area_id[i] = compl[i + 1];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        neighbor[i] = compl[i + 5];
    }

    IP_LIB_ArraytoUI32(area_id, (UI32_T *)&entry->area_id.s_addr);
    IP_LIB_ArraytoUI32(neighbor, (UI32_T *)&entry->peer_id.s_addr);
    entry->key_id = compl[9];

    if(OSPF_PMGR_GetMultiProcVirtIfAuthMd5Entry(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}

static BOOL_T ospfMultiProcVirtIfAuthMd5Table_next(int   compc, oid     *compl,
                                 OSPF_TYPE_Vlink_T *entry)
{
    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  neighbor[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;

    memset(entry,0,sizeof(OSPF_TYPE_Vlink_T));

    entry->vr_id = SYS_DFLT_VR_ID;
    if(compc == 0)
        entry->proc_id = 0xFFFFFFFF;
    else
        entry->proc_id = compl[0];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area_id[i] = compl[i + 1];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        neighbor[i] = compl[i + 5];
    }
    IP_LIB_ArraytoUI32(area_id, (UI32_T *)&entry->area_id.s_addr);
    IP_LIB_ArraytoUI32(neighbor, (UI32_T *)&entry->peer_id.s_addr);
    entry->key_id = compl[9];

    if(OSPF_PMGR_GetNextMultiProcVirtIfAuthMd5Entry(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
}


/* FUNCTION NAME :var_ospfSummaryAddressTable
 * PURPOSE:
 *       var_ospfSummaryAddressTable;
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcVirtIfAuthMd5Table (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_Vlink_T entry;
    UI32_T compc = 0;
    oid compl[MultiProcVirtIfAuthMd5Key_INSTANCE_LEN] = {0};
    oid best_inst[MultiProcVirtIfAuthMd5Key_INSTANCE_LEN];
    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  neighbor[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;
    int offset = 0;

    memset(&entry,0,sizeof(OSPF_TYPE_Vlink_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, MultiProcVirtIfAuthMd5Key_INSTANCE_LEN);

    switch (v->magic)
    {
       case OSPFMULTIPROCVIRTIFAUTHMD5KEY:
           *write_method = write_MultiProcVirtIfAuthMd5Key;
           break;

       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcVirtIfAuthMd5Table_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {

        if( ospfMultiProcVirtIfAuthMd5Table_next(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }
    }

    best_inst[offset] = entry.proc_id;
    IP_LIB_UI32toArray(entry.area_id.s_addr, area_id);
    IP_LIB_UI32toArray(entry.peer_id.s_addr, neighbor);
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = area_id[i];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = neighbor[i];
    best_inst[++offset] = entry.key_id;

    memcpy(name, v->name, v->namelen*sizeof(oid));
    memcpy(name + v->namelen, best_inst, MultiProcVirtIfAuthMd5Key_INSTANCE_LEN * sizeof(oid));
    *length = v->namelen + MultiProcVirtIfAuthMd5Key_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic) {

       case OSPFMULTIPROCSUMMARYSTATUS:
           /*reading the md5 key will always return zero */
#if 0
           *var_len = strlen (entry.auth_crypt[entry.key_id]);
           if(strlen(entry.auth_crypt[entry.key_id]))
               strcpy((char *)return_buf, entry.auth_crypt[entry.key_id]);
           else
#endif
           *var_len = 0;
           strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
       default:
           ERROR_MSG("");
    }
    return NULL;
}

#define ospfMultiProcessVirtNbrEntry_INSTANCE_LEN    9

static BOOL_T ospfMultiProcVirtNbrTable_get(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessVirtNbr_T  *data)
    {

        UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        UI8_T  router_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        int i;

        memset(data,0,sizeof(OSPF_TYPE_MultiProcessVirtNbr_T));
        if (compc !=ospfMultiProcessVirtNbrEntry_INSTANCE_LEN)
        {
            return FALSE;
        }

        data->vr_id = SYS_DFLT_VR_ID;
        data->proc_id = compl[0];
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            area_id[i] = compl[i + 1];
        }
            for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            router_id[i] = compl[i + 5];
        }

        IP_LIB_ArraytoUI32(area_id, (UI32_T *)&data->area_id.s_addr);
        IP_LIB_ArraytoUI32(router_id, (UI32_T *)&data->router_id.s_addr);

        if(OSPF_PMGR_GetMultiProcVirtNbrEntry(data)!= NETCFG_TYPE_OK)
        {
            return FALSE;
        }
        return TRUE;
        /*End of if */
    }



static BOOL_T ospfMultiProcVirtNbrTable_next(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessVirtNbr_T  *data)
    {
        UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        UI8_T  router_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
        int i;

        memset(data,0,sizeof(OSPF_TYPE_MultiProcessVirtNbr_T));

        data->vr_id = SYS_DFLT_VR_ID;
        if(compc == 0)
            data->proc_id = 0xFFFFFFFF;
        else
            data->proc_id = compl[0];
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            area_id[i] = compl[i + 1];
        }
            for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            router_id[i] = compl[i + 5];
        }
        IP_LIB_ArraytoUI32(area_id, (UI32_T *)&data->area_id.s_addr);
        IP_LIB_ArraytoUI32(router_id, (UI32_T *)&data->router_id.s_addr);

        if(OSPF_PMGR_GetNextMultiProcVirtNbrEntry(data)!= NETCFG_TYPE_OK)
        {
            return FALSE;
        }

        return TRUE;
    }


/* FUNCTION NAME :var_ospfMultiProcessVirtNbrTable
 * PURPOSE:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessVirtNbrTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_MultiProcessVirtNbr_T entry;
    UI32_T compc = 0;
    oid compl[ospfMultiProcessVirtNbrEntry_INSTANCE_LEN] = {0};
    oid best_inst[ospfMultiProcessVirtNbrEntry_INSTANCE_LEN];
    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  router_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;
    int offset = 0;

    memset(&entry,0,sizeof(OSPF_TYPE_MultiProcessVirtNbr_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, ospfMultiProcessVirtNbrEntry_INSTANCE_LEN);

    switch (v->magic)
    {
       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcVirtNbrTable_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {
       if( ospfMultiProcVirtNbrTable_next(compc, compl, &entry) != TRUE)
       {
           return NULL;
       }
    }

    best_inst[offset] = entry.proc_id;
    IP_LIB_UI32toArray(entry.area_id.s_addr, area_id);
    IP_LIB_UI32toArray(entry.router_id.s_addr, router_id);
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = area_id[i];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = router_id[i];

    memcpy(name, v->name, v->namelen*sizeof(oid));
    memcpy(name + v->namelen, best_inst, ospfMultiProcessVirtNbrEntry_INSTANCE_LEN * sizeof(oid));
    *length = v->namelen +ospfMultiProcessVirtNbrEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic)
    {

        case OSPFMULTIPROCESSVIRTNBRIPADDR:
          *var_len = sizeof(ipaddr_return);
          IP_LIB_UI32toArray(entry.ipaddr.s_addr, (UI8_T *)&ipaddr_return);
          return (u_char*) &ipaddr_return;
        case OSPFMULTIPROCESSVIRTNBROPTIONS:
          long_return = entry.options;
          return (u_char*) &long_return;
        case OSPFMULTIPROCESSVIRTNBRSTATE:
          long_return = entry.state;
          return (u_char*) &long_return;
        case OSPFMULTIPROCESSVIRTNBREVENTS:
          long_return = entry.event;
          return (u_char*) &long_return;
        case OSPFMULTIPROCESSVIRTNBRLSRETRANSQLEN:
          long_return = entry.lsretransqlen;
          return (u_char*) &long_return;
        default:
           ERROR_MSG("");
           break;
    }
    return NULL;
}


#define ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN 8
static BOOL_T ospfNetworkAreaTable_get(int compc,
                                              oid *compl,
                                              OSPF_TYPE_Network_Area_T *data)
{
    int i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    if (compc !=ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    data->vr_id = SYS_DFLT_VR_ID;
    data->proc_id  = compl[0];
    if((compl[1] != OspfInetNetworkType) || (compl[2] != SYS_ADPT_IPV4_ADDR_LEN ))
    {
        return FALSE;
    }
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+3];
    }
    memcpy(&data->network_addr, addr, sizeof(UI32_T));
    data->network_pfx = compl[7];

    if (OSPF_PMGR_GetNetworkAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ospfNetworkAreaTable_next(int compc,
                                               oid *compl,
                                               OSPF_TYPE_Network_Area_T *data)
{
    int i,len;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    if(data == NULL)
    {
        return FALSE;
    }
    memset(data,0, sizeof(OSPF_TYPE_Network_Area_T));

    data->vr_id = SYS_DFLT_VR_ID;

    if(compc == 0)
    {
        data->proc_id = 0xffffffff;
    }
    else if(compc == 1)
    {
        data->proc_id = compl[0];
    }
    else if(compc == 2)
    {
        if(compl[1] <= OspfInetNetworkType)
        {
            data->proc_id = compl[0];
        }
        else
        {
            data->proc_id = compl[0] + 1;/*type > 1, don't match network , get the next exist proc's first network entry*/
        }
    }
    else
    {
        if(compl[1] < OspfInetNetworkType)
        {
            data->proc_id = compl[0];/*type < 1, don't match network , get this proc's first network entry*/
        }
        else if(compl[1] > OspfInetNetworkType)
        {
            data->proc_id = compl[0] + 1;/*type > 1, don't match network , get the next exist proc's first network entry*/
        }
        else
        {
            if(compl[2] > INET_ADDRESS_IPV4_SIZE)
            {
                data->proc_id = compl[0] + 1;/*len > 4, don't match network , get the next exist proc's first network entry*/
            }
            else if(compl[2] == INET_ADDRESS_IPV4_SIZE)
            {
                data->proc_id = compl[0];
                data->indexlen = 8;
                if(compc >= ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN)
                {
                    memset(addr, 0, sizeof(addr));
                    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
                    {
                        addr[i] = compl[i+3];
                    }
                    memcpy(&data->network_addr, addr, sizeof(UI32_T));
                    data->network_pfx = compl[7];
                }
                else
                {
                    len = compc - 3;
                    memset(addr, 0, sizeof(addr));
                    for(i = 0; i < len; i++)
                    {
                        addr[i] = compl[i+3];
                    }
                    memcpy(&data->network_addr, addr, sizeof(UI32_T));
                    if((data->network_addr != 0) && (len != SYS_ADPT_IPV4_ADDR_LEN))
                    {
                        data->network_addr = data->network_addr - 1;
                    }
                }

            }
            else
            {
                data->proc_id = compl[0];/*len < 4, don't match network , get this proc's first network entry*/
            }
        }
    }

    if (OSPF_PMGR_GetNextNetworkAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
write_ospfMultiProcessNetworkAreaId(int action,
                                             u_char * var_val,
                                             u_char var_val_type,
                                             size_t var_val_len,
                                             u_char * statP,
                                             oid * name,
                                             size_t name_len)
{
    OSPF_TYPE_Network_Area_T  data;
    UI32_T area_id;
    int  i;
    UI32_T  format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;
    char    addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    memset(&data, 0, sizeof(OSPF_TYPE_Network_Area_T));
    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    data.vr_id = SYS_DFLT_VR_ID;
    data.proc_id = name[oid_name_length];
    if((name[oid_name_length + 1] != OspfInetNetworkType) || (name[oid_name_length + 2] != SYS_ADPT_IPV4_ADDR_LEN ))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 3 + i];
    }
    memcpy(&data.network_addr, addr, sizeof(UI32_T));
    data.network_pfx = name[oid_name_length + 7];

    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(u_long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
              /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            area_id = *(u_long *) var_val;
            if (OSPF_PMGR_GetNetworkAreaTable(&data)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                    return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                if (OSPF_PMGR_NetworkUnset(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,data.area_id,format)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                else
                {
                    if (OSPF_PMGR_NetworkSet(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,area_id,format)!= OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }
                }
            }
        }
        break;

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
write_ospfMultiProcessNetworkAreaStatus(int action,
                                                  u_char * var_val,
                                                  u_char var_val_type,
                                                  size_t var_val_len,
                                                  u_char * statP,
                                                  oid * name,
                                                  size_t name_len)
{
    OSPF_TYPE_Network_Area_T  data;
    int  i,status;
    UI32_T  format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;
    char    addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    memset(&data, 0, sizeof(OSPF_TYPE_Network_Area_T));
    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    data.vr_id = SYS_DFLT_VR_ID;
    data.proc_id = name[oid_name_length];
    if((name[oid_name_length + 1] != OspfInetNetworkType) || (name[oid_name_length + 2] != SYS_ADPT_IPV4_ADDR_LEN ))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 3 + i];
    }
    memcpy(&data.network_addr, addr, sizeof(UI32_T));
    data.network_pfx = name[oid_name_length + 7];

    switch ( action )
    {
        case RESERVE1:
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
            status = *(long *)var_val;
            if((status != OSPF_SNMP_ROW_STATUS_CREATEANDGO) && (status != OSPF_SNMP_ROW_STATUS_DESTROY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
              /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            status = *(long *)var_val;
            if(status == OSPF_SNMP_ROW_STATUS_CREATEANDGO)
            {
                if (OSPF_PMGR_GetNetworkAreaTable(&data)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    if (OSPF_PMGR_NetworkSet(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,0,format)!= OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }
                }
            }
            else
            {
                if (OSPF_PMGR_GetNetworkAreaTable(&data) == OSPF_TYPE_RESULT_SUCCESS)
                {
                    if (OSPF_PMGR_NetworkUnset(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,data.area_id,format)!= OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }
                }
            }
        }
        break;

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
write_ospfMultiProcessNetworkAreaId2(int action,
                                              u_char * var_val,
                                              u_char var_val_type,
                                              size_t var_val_len,
                                              u_char * statP,
                                              oid * name,
                                              size_t name_len)
{
    OSPF_TYPE_Network_Area_T  data;
    int  i;
    UI32_T  area_id;
    UI32_T  format = OSPF_SNMP_AREA_ID_FORMAT_ADDRESS;
    char    addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    memset(&data, 0, sizeof(OSPF_TYPE_Network_Area_T));
    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    data.vr_id = SYS_DFLT_VR_ID;
    data.proc_id = name[oid_name_length];
    if((name[oid_name_length + 1] != OspfInetNetworkType) || (name[oid_name_length + 2] != SYS_ADPT_IPV4_ADDR_LEN ))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 3 + i];
    }
    memcpy(&data.network_addr, addr, sizeof(UI32_T));
    data.network_pfx = name[oid_name_length + 7];

    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != SYS_TYPE_IPV4_ADDR_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
              /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            memset(addr,0,sizeof(addr));
            for(i = 0;i<4;i++)
                addr[i] = var_val[i];
            memcpy(&area_id, addr, var_val_len);
            if (OSPF_PMGR_GetNetworkAreaTable(&data)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                if (OSPF_PMGR_NetworkUnset(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,data.area_id,format)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                else
                {
                    if (OSPF_PMGR_NetworkSet(data.vr_id,data.proc_id,data.network_addr,data.network_pfx,area_id,format)!= OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }
                }
            }
        }
        break;

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

/* FUNCTION NAME : var_ospfMultiProcessNetworkAreaAddressTable
 * PURPOSE:
 *       var_ospfMultiProcessNetworkAreaAddressTable
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessNetworkAreaAddressTable (struct variable *vp,
                                                         oid *name,
                                                         size_t *length,
                                                         int exact,
                                                         size_t *var_len,
                                                         WriteMethod **write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN];
    OSPF_TYPE_Network_Area_T  data;
    UI8_T addr[INET_ADDRESS_IPV4_SIZE]= {0};
    int i;

    memset(&data, 0, sizeof(OSPF_TYPE_Network_Area_T));
    /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSNETWORKAREAAREAID:
            *write_method = write_ospfMultiProcessNetworkAreaId;
            break;
        case OSPFMULTIPROCESSNETWORKAREASTATUS:
            *write_method = write_ospfMultiProcessNetworkAreaStatus;
            break;
        case OSPFMULTIPROCESSNETWORKAREAAREAID2:
            *write_method = write_ospfMultiProcessNetworkAreaId2;
            break;
        default:
            *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN);
    /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfNetworkAreaTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ospfNetworkAreaTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    best_inst[1] = OspfInetNetworkType;
    best_inst[2] = INET_ADDRESS_IPV4_SIZE;
    memcpy(addr, &(data.network_addr),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[3+i] = addr[i];
    }
    best_inst[3+INET_ADDRESS_IPV4_SIZE] = data.network_pfx;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessNetworkAreaEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSNETWORKAREAAREAID:
            long_return =  L_STDLIB_Ntoh32(data.area_id);
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNETWORKAREASTATUS:
            long_return = OSPF_SNMP_ROW_STATUS_ACTIVE;
            return (u_char*) &long_return;
            break;
        case OSPFMULTIPROCESSNETWORKAREAAREAID2:
            *var_len = sizeof(ipaddr_return);
            ipaddr_return =  data.area_id;
            return (u_char *) &ipaddr_return;
            break;
        default:
            ERROR_MSG("");
    }
    return NULL;


}

#define MultiProcVirtIfEntry_INSTANCE_LEN    9
#define SET_FLAG(V,F)        (V) = (V) | (F)

int
write_ospfMultiProcVirtIfEntry(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    UI8_T   magic;
    long    value;
    int i;
    UI32_T proc_id;
    UI32_T format;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI8_T  area[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  peer_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T  area_id;
    OSPF_TYPE_Area_Virtual_Link_Para_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Area_Virtual_Link_Para_T));

    magic = name[oid_name_length - 1];
    proc_id = name[oid_name_length];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area[i] = name[oid_name_length + 1 + i];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        peer_id[i] = name[oid_name_length + 5 + i];
    }
    IP_LIB_ArraytoUI32(area, &area_id);
    IP_LIB_ArraytoUI32(peer_id, &entry.vlink_addr);


    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER)
            return SNMP_ERR_WRONGTYPE;
        if (var_val_len > sizeof(long))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:

        value = *(long *)var_val;
        switch(magic)
        {
            case OSPFMULTIPROCESSVIRTIFTRANSITDELAY:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_TRANSITDELAY_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_TRANSITDELAY_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSVIRTIFRETRANSINTERVAL:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_RETRANSINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_RETRANSINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSVIRTIFHELLOINTERVAL:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_HELLOINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_HELLOINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSVIRTIFRTRDEADINTERVAL:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_RTRDEADINTERVAL_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_RTRDEADINTERVAL_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSVIRTIFSTATUS:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_STATUS_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_STATUS_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            case OSPFMULTIPROCESSVIRTIFAUTHTYPE:
                if((value <OSPF_MULTI_PROCESS_VIRT_IF_AUTHTYPE_MIN) || (value >OSPF_MULTI_PROCESS_VIRT_IF_AUTHTYPE_MAX))
                    return SNMP_ERR_WRONGVALUE;
                break;
            default:
                break;
        }
        break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        value = * (long *) var_val;
        if (name_len!= MultiProcVirtIfEntry_INSTANCE_LEN + oid_name_length)
        {
            return SNMP_ERR_COMMITFAILED;
        }
        format = OSPF_AREA_ID_FORMAT_ADDRESS;

        switch(magic)
        {

            case OSPFMULTIPROCESSVIRTIFTRANSITDELAY:
                SET_FLAG(entry.config, OSPF_AREA_VLINK_TRANSMIT_DELAY);
                entry.transmit_delay = value;
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSVIRTIFRETRANSINTERVAL:
                SET_FLAG(entry.config, OSPF_AREA_VLINK_RETRANSMIT_INTERVAL);
                entry.retransmit_interval = value;
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;

            case OSPFMULTIPROCESSVIRTIFHELLOINTERVAL:
                SET_FLAG(entry.config, OSPF_AREA_VLINK_HELLO_INTERVAL);
                entry.hello_interval = value;
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
            case OSPFMULTIPROCESSVIRTIFRTRDEADINTERVAL:
                SET_FLAG(entry.config, OSPF_AREA_VLINK_DEAD_INTERVAL);
                entry.dead_interval = value;
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;
            case OSPFMULTIPROCESSVIRTIFSTATUS:
                switch(value)
                {
                    case OSPF_SNMP_ROW_STATUS_ACTIVE:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDGO:
                    {
                        if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_DESTROY:
                    {

                        if(OSPF_PMGR_AreaVirtualLinkUnset(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
                    }
                        break;
                    case OSPF_SNMP_ROW_STATUS_NOTINSERVICE:
                    case OSPF_SNMP_ROW_STATUS_NOTREADY:
                    case OSPF_SNMP_ROW_STATUS_CREATEANDWAIT:
                    default:
                        return SNMP_ERR_COMMITFAILED;
                        break;
                }
                break;

            case OSPFMULTIPROCESSVIRTIFAUTHTYPE:
                SET_FLAG(entry.config, OSPF_AREA_VLINK_AUTHENTICATION);
                entry.auth_type= value;
                if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

                break;

            default:
                break;
        }
    }
    break;

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
write_ospfMultiProcVirtIfEntryChar(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    int i;
    UI32_T proc_id;
    UI32_T format;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI8_T  area[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  peer_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T  area_id;
    OSPF_TYPE_Area_Virtual_Link_Para_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    memset(&entry,0,sizeof(OSPF_TYPE_Area_Virtual_Link_Para_T));

    proc_id = name[oid_name_length];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area[i] = name[oid_name_length + 1 + i];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        peer_id[i] = name[oid_name_length + 5 + i];
    }
    IP_LIB_ArraytoUI32(area, &area_id);
    IP_LIB_ArraytoUI32(peer_id, &entry.vlink_addr);

    switch (action) {

    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if ((var_val_len > OSPF_MULTI_PROCESS_VIRT_IF_AUTHKEY_LENGTH_MAX) || (var_val_len < 0))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;
    case RESERVE2:

        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {

        format = OSPF_AREA_ID_FORMAT_ADDRESS;
        if(var_val_len)
        {
            strncpy(entry.auth_key, (char *)var_val, var_val_len);
        }
        SET_FLAG(entry.config, OSPF_AREA_VLINK_AUTHENTICATIONKEY);
        if(var_val_len)
        {
            if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        else
        {
            if(OSPF_PMGR_AreaVirtualLinkUnset(vr_id, proc_id, area_id, format, &entry)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
    }
    break;

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

static BOOL_T ospfMultiProcVirtIfTable_get(int      compc,
                                oid     *compl,
                                OSPF_TYPE_Vlink_T *entry)
{

    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  peer_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;

    memset(entry,0,sizeof(OSPF_TYPE_Vlink_T));
    if (compc !=MultiProcVirtIfEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    entry->vr_id = SYS_DFLT_VR_ID;
    entry->proc_id = compl[0];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area_id[i] = compl[i + 1];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        peer_id[i] = compl[i + 5];
    }

    IP_LIB_ArraytoUI32(area_id, (UI32_T *)&entry->area_id.s_addr);
    IP_LIB_ArraytoUI32(peer_id, (UI32_T *)&entry->peer_id.s_addr);

    if(OSPF_PMGR_GetMultiProcVirtualLinkEntry(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}
static BOOL_T ospfMultiProcVirtIfTable_next(int   compc,
                                 oid     *compl,
                                 OSPF_TYPE_Vlink_T *entry)
{
    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  peer_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;

    memset(entry,0,sizeof(OSPF_TYPE_Vlink_T));

    entry->vr_id = SYS_DFLT_VR_ID;
    if(compc == 0)
        entry->proc_id = 0xFFFFFFFF;
    else
        entry->proc_id = compl[0];
    for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        area_id[i] = compl[i + 1];
    }
        for(i = 0;i< SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        peer_id[i] = compl[i + 5];
    }
    IP_LIB_ArraytoUI32(area_id, (UI32_T *)&entry->area_id.s_addr);
    IP_LIB_ArraytoUI32(peer_id, (UI32_T *)&entry->peer_id.s_addr);

    if(OSPF_PMGR_GetNextMultiProcVirtualLinkEntry(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME :var_ospfMultiProcessVirtIfTable
 * PURPOSE:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessVirtIfTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    OSPF_TYPE_Vlink_T entry;
    UI32_T compc = 0;
    oid compl[MultiProcVirtIfEntry_INSTANCE_LEN] = {0};
    oid best_inst[MultiProcVirtIfEntry_INSTANCE_LEN];
    UI8_T  area_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI8_T  peer_id[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    int i;
    int offset = 0;

    memset(&entry,0,sizeof(OSPF_TYPE_Vlink_T));

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc, compl, MultiProcVirtIfEntry_INSTANCE_LEN);

    switch (v->magic)
    {
       case OSPFMULTIPROCESSVIRTIFTRANSITDELAY:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       case OSPFMULTIPROCESSVIRTIFRETRANSINTERVAL:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       case OSPFMULTIPROCESSVIRTIFHELLOINTERVAL:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       case OSPFMULTIPROCESSVIRTIFRTRDEADINTERVAL:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       case OSPFMULTIPROCESSVIRTIFAUTHKEY:
           *write_method = write_ospfMultiProcVirtIfEntryChar;
           break;
       case OSPFMULTIPROCESSVIRTIFSTATUS:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       case OSPFMULTIPROCESSVIRTIFAUTHTYPE:
           *write_method = write_ospfMultiProcVirtIfEntry;
           break;
       default:
           *write_method = 0;
           break;
    }

    if (exact)
    {
        if(ospfMultiProcVirtIfTable_get(compc, compl, &entry) != TRUE)
        {
            return NULL;
        }

    }
    else
    {
       if( ospfMultiProcVirtIfTable_next(compc, compl, &entry) != TRUE)
       {
           return NULL;
       }
    }

    best_inst[offset] = entry.proc_id;
    IP_LIB_UI32toArray(entry.area_id.s_addr, area_id);
    IP_LIB_UI32toArray(entry.peer_id.s_addr, peer_id);
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = area_id[i];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        best_inst[++offset] = peer_id[i];

    memcpy(name, v->name, v->namelen*sizeof(oid));
    memcpy(name + v->namelen, best_inst, MultiProcVirtIfEntry_INSTANCE_LEN * sizeof(oid));
    *length = v->namelen +MultiProcVirtIfEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    switch (v->magic)
    {

        case OSPFMULTIPROCESSVIRTIFTRANSITDELAY:
           long_return = entry.transmit_delay;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFRETRANSINTERVAL:
           long_return = entry.retransmit_interval;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFHELLOINTERVAL:
           long_return = entry.hello_interval;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFRTRDEADINTERVAL:
           long_return = entry.dead_interval;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFSTATE:
           long_return = entry.oi_state;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFEVENTS:
           long_return = entry.events;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFAUTHKEY:
           /*reading the auth key will always return zero */

#if 0
           *var_len = strlen (entry.auth_simple);
           if(strlen(entry.auth_simple))
               strcpy((char *)return_buf, entry.auth_simple);
           else
#endif
           *var_len = 0;
           strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
        case OSPFMULTIPROCESSVIRTIFSTATUS:
           long_return = OSPF_SNMP_ROW_STATUS_ACTIVE;
           return (u_char*) &long_return;
           break;
        case OSPFMULTIPROCESSVIRTIFAUTHTYPE:
           long_return = entry.auth_type;
           return (u_char*) &long_return;
           break;
        default:
           ERROR_MSG("");
           break;
    }
    return NULL;
}

#define ospfMultiProcessAreaEntry_INSTANCE_LEN 5

static BOOL_T ospfAreaTable_get(int compc,
                                    oid *compl,
                                    OSPF_TYPE_Area_T *data)
{
    int i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    memset(data,0, sizeof(OSPF_TYPE_Area_T));
    if (compc !=ospfMultiProcessAreaEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    data->vr_id = SYS_DFLT_VR_ID;
    data->proc_id  = compl[0];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+1];
    }
    memcpy(&data->area_id, addr, sizeof(UI32_T));

    if (OSPF_PMGR_GetAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ospfAreaTable_next(int compc,
                                      oid *compl,
                                      OSPF_TYPE_Area_T *data)
{
    int i,len;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    if(data == NULL)
    {
        return FALSE;
    }
    memset(data,0, sizeof(OSPF_TYPE_Area_T));

    data->vr_id = SYS_DFLT_VR_ID;

    len = compc - 1;

    if(compc == 0)
    {
        data->proc_id = 0xffffffff;
    }
    else if(compc == 1)
    {
        data->proc_id = compl[0];
    }
    else if(compc > ospfMultiProcessAreaEntry_INSTANCE_LEN)
    {
        data->proc_id = compl[0];
        data->indexlen = 4;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
    }
    else
    {
        data->proc_id = compl[0];
        data->indexlen = 4;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < len; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
    }

    if (OSPF_PMGR_GetNextAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
write_ospfMultiProcessAreaSummary(int action,
                                           u_char * var_val,
                                           u_char var_val_type,
                                           size_t var_val_len,
                                           u_char * statP,
                                           oid * name,
                                           size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    int  i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessAreaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }

    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 1 + i];
    }
    memcpy(&area_id, addr, sizeof(UI32_T));

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;

            if((val != OSPF_SNMP_NO_AREA_SUMMARY) && (val != OSPF_SNMP_SEND_AREA_SUMMARY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            if(OSPF_PMGR_AreaSummarySet(vr_id,proc_id,area_id,val)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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


/* FUNCTION NAME : var_ospfMultiProcessAreaTable
 * PURPOSE:
 *       var_ospfMultiProcessAreaTable
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessAreaTable (struct variable *vp,
                                      oid *name,
                                      size_t *length,
                                      int exact,
                                      size_t *var_len,
                                      WriteMethod **write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessAreaEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessAreaEntry_INSTANCE_LEN];
    OSPF_TYPE_Area_T  data;
    char addr[INET_ADDRESS_IPV4_SIZE]= {0};
    int i;

    memset(&data, 0, sizeof(OSPF_TYPE_Area_T));
    /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSAREASUMMARY:
            *write_method = write_ospfMultiProcessAreaSummary;
            break;
        default:
            *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessAreaEntry_INSTANCE_LEN);

    /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfAreaTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ospfAreaTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    memcpy(addr, &(data.area_id),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[1+i] = addr[i];
    }
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessAreaEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessAreaEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSIMPORTASEXTERN:
            if(data.external_routing == OSPF_AREA_DEFAULT)
            {
                long_return =  OSPF_SNMP_AREA_IMPORT_EXTERNAL;
            }
            else if(data.external_routing == OSPF_AREA_STUB)
            {
                long_return =  OSPF_SNMP_AREA_IMPORT_NO_EXTERNAL;
            }
            else if(data.external_routing == OSPF_AREA_NSSA)
            {
                long_return =  OSPF_SNMP_AREA_IMPORT_NSSA;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSSPFRUNS:
            long_return = data.spf_calc_count;
            return (u_char*) &long_return;
            break;
        case OSPFMULTIPROCESSAREABDRRTRCOUNT:
            if(CHECK_FLAG (data.top_flags, OSPF_ROUTER_ABR))
            {
                long_return = data.abr_count + 1;
            }
            else
            {
                long_return = data.abr_count;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSASBDRRTRCOUNT:
            if(CHECK_FLAG (data.top_flags, OSPF_ROUTER_ASBR))
            {
                long_return = data.asbr_count + 1;
            }
            else
            {
                long_return = data.asbr_count;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSAREALSACOUNT:
            long_return = data.lsa_count;
            return (u_char*) &long_return;
            break;
        case OSPFMULTIPROCESSAREALSACKSUMSUM:
            long_return =  data.lsa_checksum;
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSAREASUMMARY:
            if(CHECK_FLAG (data.config, OSPF_AREA_CONF_NO_SUMMARY))
            {
                long_return = OSPF_SNMP_NO_AREA_SUMMARY;
            }
            else
            {
                long_return = OSPF_SNMP_SEND_AREA_SUMMARY;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSAREASTATUS:
            long_return = data.status;
            return (u_char*) &long_return;
            break;
        default:
            ERROR_MSG("");
    }
    return NULL;


}


#define ospfMultiProcessStubAreaEntry_INSTANCE_LEN 6
static BOOL_T ospfStubAreaTable_get(int compc,
                                          oid *compl,
                                          OSPF_TYPE_Stub_Area_T *data)
{
    int i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    memset(data,0, sizeof(OSPF_TYPE_Stub_Area_T));
    if ((compc !=ospfMultiProcessStubAreaEntry_INSTANCE_LEN) || compl[5] != OspfStubAreaTosValue)
    {
        return FALSE;
    }
    data->vr_id = SYS_DFLT_VR_ID;
    data->proc_id  = compl[0];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+1];
    }
    memcpy(&data->area_id, addr, sizeof(UI32_T));
    data->stub_tos = compl[5];

    if (OSPF_PMGR_GetStubAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ospfStubAreaTable_next(int compc,
                                           oid *compl,
                                           OSPF_TYPE_Stub_Area_T *data)
{
    int i,len;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    if(data == NULL)
    {
        return FALSE;
    }
    memset(data,0, sizeof(OSPF_TYPE_Stub_Area_T));

    data->vr_id = SYS_DFLT_VR_ID;

    if(compc == 0)
    {
        data->proc_id = 0xffffffff;
    }
    else if(compc == 1)
    {
        data->proc_id = compl[0];
    }
    else if(compc >= ospfMultiProcessStubAreaEntry_INSTANCE_LEN)
    {
        data->indexlen = 5;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
        data->stub_tos = compl[5];
    }
    else
    {
        len = compc - 1;
        data->proc_id = compl[0];
        data->indexlen = 5;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < len; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
        if((data->area_id != 0) && (compc < ospfMultiProcessStubAreaEntry_INSTANCE_LEN))
        {
            data->area_id = data->area_id - 1;
        }
    }

    if (OSPF_PMGR_GetNextStubAreaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
write_ospfMultiProcessStubAreaMetric(int action,
                                             u_char * var_val,
                                             u_char var_val_type,
                                             size_t var_val_len,
                                             u_char * statP,
                                             oid * name,
                                             size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val,tos;
    int  i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if((name_len - oid_name_length != ospfMultiProcessStubAreaEntry_INSTANCE_LEN) ||
       (name[oid_name_length + 5] != OspfStubAreaTosValue))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 1 + i];
    }
    memcpy(&area_id, addr, sizeof(UI32_T));
    tos = name[oid_name_length + 5];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < 0) || (val > 16777215))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            if(OSPF_PMGR_StubAreaMetricSet(vr_id,proc_id,area_id,tos,val)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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
write_ospfMultiProcessStubAreaStatus(int action,
                                             u_char * var_val,
                                             u_char var_val_type,
                                             size_t var_val_len,
                                             u_char * statP,
                                             oid * name,
                                             size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val,tos,ret;
    int  i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if((name_len - oid_name_length != ospfMultiProcessStubAreaEntry_INSTANCE_LEN) ||
       (name[oid_name_length + 5] != OspfStubAreaTosValue))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 1 + i];
    }
    memcpy(&area_id, addr, sizeof(UI32_T));
    tos = name[oid_name_length + 5];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;

            if((val < OSPF_SNMP_ROW_STATUS_ACTIVE) || (val > OSPF_SNMP_ROW_STATUS_DESTROY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            ret = OSPF_PMGR_StubAreaStatusSet(vr_id,proc_id,area_id,tos,val);
            if(ret != OSPF_TYPE_RESULT_SUCCESS)
            {
                if(ret == OSPF_TYPE_RESULT_INCONSISTENT_VALUE)
                {
                    return SNMP_ERR_INCONSISTENTVALUE;
                }
                else
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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


/* FUNCTION NAME : var_ospfMultiProcessStubAreaTable
 * PURPOSE:
 *       var_ospfMultiProcessStubAreaTable
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessStubAreaTable(struct variable *vp,
                                           oid *name,
                                           size_t *length,
                                           int exact,
                                           size_t *var_len,
                                           WriteMethod **write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessStubAreaEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessStubAreaEntry_INSTANCE_LEN];
    OSPF_TYPE_Stub_Area_T  data;
    char addr[INET_ADDRESS_IPV4_SIZE]= {0};
    int i;

    memset(&data, 0, sizeof(OSPF_TYPE_Stub_Area_T));
    /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSSTUBMETRIC:
            *write_method = write_ospfMultiProcessStubAreaMetric;
            break;
        case OSPFMULTIPROCESSSTUBSTATUS:
            *write_method = write_ospfMultiProcessStubAreaStatus;
            break;
        default:
            *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessStubAreaEntry_INSTANCE_LEN);

    /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfStubAreaTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ospfStubAreaTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    memcpy(addr, &(data.area_id),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[1+i] = addr[i];
    }
    best_inst[5] = OspfStubAreaTosValue;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessStubAreaEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessStubAreaEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSSTUBMETRIC:
            long_return = data.default_cost;
            return (u_char*) &long_return;
            break;
        case OSPFMULTIPROCESSSTUBSTATUS:
            long_return = data.status;
            return (u_char*) &long_return;
            break;
        default:
            ERROR_MSG("");
    }
    return NULL;


}

#define ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN    14

static BOOL_T ospfAreaAggregateTable_get(int compc,
                                                oid *compl,
                                                OSPF_TYPE_Area_Range_T *data)
{
    int i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    memset(data,0, sizeof(OSPF_TYPE_Area_Range_T));
    if ((compc != ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN) || compl[5] != OspfAreaAggregateLsdbSummaryLink)
    {
        return FALSE;
    }
    data->vr_id = SYS_DFLT_VR_ID;
    data->proc_id  = compl[0];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+1];
    }
    memcpy(&data->area_id, addr, sizeof(UI32_T));
    data->type = compl[5];
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+6];
    }
    memcpy(&data->range_addr, addr, sizeof(UI32_T));
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = compl[i+10];
    }
    memcpy(&data->range_mask, addr, sizeof(UI32_T));

    if (OSPF_PMGR_GetAreaRangeTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ospfAreaAggregateTable_next(int compc,
                                                  oid *compl,
                                                  OSPF_TYPE_Area_Range_T *data)
{
    int i,len;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];

    if(data == NULL)
    {
        return FALSE;
    }
    memset(data,0, sizeof(OSPF_TYPE_Area_Range_T));

    data->vr_id = SYS_DFLT_VR_ID;

    if(compc == 0)/*not any index*/
    {
        data->proc_id = 0xffffffff;
    }
    else if(compc == 1)/*only have proc_id*/
    {
        data->proc_id = compl[0];
    }
    else if((compc > 1) && (compc < 6))/*have proc_id and area_id*/
    {
        data->indexlen = 13;
        len = compc - 1;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < len; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
        if((data->area_id != 0) && (len != SYS_ADPT_IPV4_ADDR_LEN))
        {
            data->area_id = data->area_id - 1;
        }
    }
    else if((compc >= 6) && (compc < 11))/*have proc_id,area_id,type and range */
    {
        data->indexlen = 13;
        len = compc - 6;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
        data->type = compl[5];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < len; i++)
        {
            addr[i] = compl[i+6];
        }
        memcpy(&data->range_addr, addr, sizeof(UI32_T));
        if((data->range_addr != 0) && (len != SYS_ADPT_IPV4_ADDR_LEN))
        {
            data->range_addr = data->range_addr - 1;
        }
    }
    else/*have proc_id,area_id,type,range and mask*/
    {
        data->indexlen = 13;
        len = compc - 10;
        data->proc_id = compl[0];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            addr[i] = compl[i+1];
        }
        memcpy(&data->area_id, addr, sizeof(UI32_T));
        data->type = compl[5];
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
        {
            addr[i] = compl[i+6];
        }
        memcpy(&data->range_addr, addr, sizeof(UI32_T));
        memset(addr, 0, sizeof(addr));
        for(i = 0; i < len; i++)
        {
            addr[i] = compl[i+10];
        }
        memcpy(&data->range_mask, addr, sizeof(UI32_T));
        if((data->range_mask != 0) && (len != SYS_ADPT_IPV4_ADDR_LEN))
        {
            data->range_mask = data->range_mask - 1;
        }
    }

    if (OSPF_PMGR_GetNextAreaRangeTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
write_ospfMultiProcessAreaAggregateStatus(int action,
                                                    u_char * var_val,
                                                    u_char var_val_type,
                                                    size_t var_val_len,
                                                    u_char * statP,
                                                    oid * name,
                                                    size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,type,range_addr,range_mask,val,ret;
    int  i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if((name_len - oid_name_length != ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN) ||
       (name[oid_name_length + 5] != OspfAreaAggregateLsdbSummaryLink))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 1 + i];
    }
    memcpy(&area_id, addr, sizeof(UI32_T));
    type = name[oid_name_length + 5];
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 6 + i];
    }
    memcpy(&range_addr, addr, sizeof(UI32_T));
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 10 + i];
    }
    memcpy(&range_mask, addr, sizeof(UI32_T));

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_ROW_STATUS_ACTIVE) || (val > OSPF_SNMP_ROW_STATUS_DESTROY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            ret = OSPF_PMGR_AreaAggregateStatusSet(vr_id,proc_id,area_id,type,range_addr,range_mask,val);
            if(ret != OSPF_TYPE_RESULT_SUCCESS)
            {
                if(ret == OSPF_TYPE_RESULT_INCONSISTENT_VALUE)
                {
                    return SNMP_ERR_INCONSISTENTVALUE;
                }
                else
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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
write_ospfMultiProcessAreaAggregateEffect(int action,
                                                    u_char * var_val,
                                                    u_char var_val_type,
                                                    size_t var_val_len,
                                                    u_char * statP,
                                                    oid * name,
                                                    size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,type,range_addr,range_mask,val;
    int  i;
    char addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T  oid_name_length;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if((name_len - oid_name_length != ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN) ||
       (name[oid_name_length + 5] != OspfAreaAggregateLsdbSummaryLink))
    {
        return SNMP_ERR_COMMITFAILED;
    }

    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 1 + i];
    }
    memcpy(&area_id, addr, sizeof(UI32_T));
    type = name[oid_name_length + 5];
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 6 + i];
    }
    memcpy(&range_addr, addr, sizeof(UI32_T));
    memset(addr, 0 ,sizeof(addr));
    for(i = 0; i < SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        addr[i] = name[oid_name_length + 10 + i];
    }
    memcpy(&range_mask, addr, sizeof(UI32_T));

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_RANGE_AREA_ADVERTISEMATCHING) || (val > OSPF_SNMP_RANGE_AREA_DONOTADVERTISEMATCHING))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            if(OSPF_PMGR_AreaAggregateEffectSet(vr_id,proc_id,area_id,type,range_addr,range_mask,val)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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

/* FUNCTION NAME : var_ospfMultiProcessAreaAggregateTable
 * PURPOSE:
 *       var_ospfMultiProcessAreaAggregateTable
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessAreaAggregateTable (struct variable *vp,
                                                  oid *name,
                                                  size_t *length,
                                                  int exact,
                                                  size_t *var_len,
                                                  WriteMethod **write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN];
    OSPF_TYPE_Area_Range_T  data;
    char addr[INET_ADDRESS_IPV4_SIZE]= {0};
    int i;

    memset(&data, 0, sizeof(OSPF_TYPE_Area_Range_T));
    /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSAREAAGGREGETESTATUS:
            *write_method = write_ospfMultiProcessAreaAggregateStatus;
            break;
        case OSPFMULTIPROCESSAREAAGGREGATEEFFECT:
            *write_method = write_ospfMultiProcessAreaAggregateEffect;
            break;
        default:
            *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN);
    /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfAreaAggregateTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ospfAreaAggregateTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    memset(addr, 0, sizeof(addr));
    memcpy(addr, &(data.area_id),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[1+i] = addr[i];
    }
    best_inst[5] = data.type;
    memset(addr, 0, sizeof(addr));
    memcpy(addr, &(data.range_addr),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[6+i] = addr[i];
    }
    memset(addr, 0, sizeof(addr));
    memcpy(addr, &(data.range_mask),sizeof(UI32_T));
    for(i=0;i<INET_ADDRESS_IPV4_SIZE;i++)
    {
        best_inst[10+i] = addr[i];
    }
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessAreaAggregateEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSAREAAGGREGETESTATUS:
            long_return =  data.status;
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSAREAAGGREGATEEFFECT:
            if (CHECK_FLAG (data.flags, OSPF_AREA_RANGE_ADVERTISE))
            {
                long_return = OSPF_SNMP_RANGE_AREA_ADVERTISEMATCHING;
            }
            else
            {
                long_return = OSPF_SNMP_RANGE_AREA_DONOTADVERTISEMATCHING;
            }
            return (u_char*) &long_return;
            break;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

#define ospfMultiProcessNssaEntry_INSTANCE_LEN  2

static BOOL_T ospfNssaTable_get(int compc,
                                     oid *compl,
                                     OSPF_TYPE_Nssa_Area_T *data)
{
    memset(data,0, sizeof(OSPF_TYPE_Nssa_Area_T));
    if (compc != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    data->vr_id = SYS_DFLT_VR_ID;
    data->proc_id  = compl[0];
    data->area_id = compl[1];

    if (OSPF_PMGR_GetNssaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ospfNssaTable_next(int compc,
                                      oid *compl,
                                      OSPF_TYPE_Nssa_Area_T *data)
{
    if(data == NULL)
    {
        return FALSE;
    }
    memset(data,0, sizeof(OSPF_TYPE_Nssa_Area_T));

    data->vr_id = SYS_DFLT_VR_ID;

    if(compc == 0)/*not any index*/
    {
        data->proc_id = 0xffffffff;
    }
    else if(compc == 1)/*only have proc_id*/
    {
        data->proc_id = compl[0];
    }
    else
    {
        data->indexlen = 4;
        data->proc_id = compl[0];
        data->area_id = compl[1];
    }

    if (OSPF_PMGR_GetNextNssaTable(data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
write_ospfMultiProcessNssaTranslatorRole(int action,
                                                  u_char * var_val,
                                                  u_char var_val_type,
                                                  size_t var_val_len,
                                                  u_char * statP,
                                                  oid * name,
                                                  size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_NSSA_TRANSLATE_NEVER) || (val > OSPF_SNMP_NSSA_TRANSLATE_CANDIDATE))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            SET_FLAG(nssa_para.config,OSPF_TYPE_AREA_NSSA_CONF_TRANSLATOR_ROLE);
            val = *(long *) var_val;
            if(val == OSPF_SNMP_NSSA_TRANSLATE_NEVER)
            {
                nssa_para.translator_role = OSPF_TYPE_TRANSLATOR_NEVER;
            }
            else if(val == OSPF_SNMP_NSSA_TRANSLATE_ALWAYS)
            {
                nssa_para.translator_role = OSPF_TYPE_TRANSLATOR_ALAWAYS;
            }
            else if(val == OSPF_SNMP_NSSA_TRANSLATE_CANDIDATE)
            {
                nssa_para.translator_role = OSPF_TYPE_TRANSLATOR_CANDIDATE;
            }
            if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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
write_ospfMultiProcessNssaRedistributeStatus(int action,
                                                       u_char * var_val,
                                                       u_char var_val_type,
                                                       size_t var_val_len,
                                                       u_char * statP,
                                                       oid * name,
                                                       size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_NSSA_REDISTRIBUTE_ENABLED) || (val > OSPF_SNMP_NSSA_REDISTRIBUTE_DISABLED))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            SET_FLAG(nssa_para.config,OSPF_TYPE_AREA_NSSA_CONF_NO_REDISTRIBUTION);
            val = *(long *) var_val;
            if(val == OSPF_SNMP_NSSA_REDISTRIBUTE_ENABLED)
            {
                if(OSPF_PMGR_AreaNssaUnset(vr_id,proc_id,area_id,format,nssa_para.config)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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
write_ospfMultiProcessNssaOriginateDefaultInfoStatus(int action,
                                                                u_char * var_val,
                                                                u_char var_val_type,
                                                                size_t var_val_len,
                                                                u_char * statP,
                                                                oid * name,
                                                                size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_ENABLED) || (val > OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_DISABLED))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            SET_FLAG(nssa_para.config,OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION);
            val = *(long *) var_val;
            if(val == OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_ENABLED)
            {
                if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_AreaNssaUnset(vr_id,proc_id,area_id,format,nssa_para.config)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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
write_ospfMultiProcessNssaMetricType(int action,
                                              u_char * var_val,
                                              u_char var_val_type,
                                              size_t var_val_len,
                                              u_char * statP,
                                              oid * name,
                                              size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < OSPF_SNMP_NSSA_EXTERNAL_METRIC_TYPE_1) || (val > OSPF_SNMP_NSSA_EXTERNAL_METRIC_TYPE_2))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            SET_FLAG(nssa_para.config,OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC_TYPE);
            nssa_para.metric_type = *(long *) var_val;
            if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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
write_ospfMultiProcessNssaMetric(int action,
                                        u_char * var_val,
                                        u_char var_val_type,
                                        size_t var_val_len,
                                        u_char * statP,
                                        oid * name,
                                        size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val < 0) || (val > 16777214))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            SET_FLAG(nssa_para.config,OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC);
            nssa_para.metric = *(long *) var_val;
            if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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
write_ospfMultiProcessNssaStatus(int action,
                                         u_char * var_val,
                                         u_char var_val_type,
                                         size_t var_val_len,
                                         u_char * statP,
                                         oid * name,
                                         size_t name_len)
{
    UI32_T vr_id,proc_id,area_id,val;
    UI32_T  oid_name_length;
    OSPF_TYPE_Area_Nssa_Para_T nssa_para;
    UI32_T format = OSPF_SNMP_AREA_ID_FORMAT_DECIMAL;

    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    if(name_len - oid_name_length != ospfMultiProcessNssaEntry_INSTANCE_LEN)
    {
        return SNMP_ERR_COMMITFAILED;
    }
    memset(&nssa_para, 0, sizeof(OSPF_TYPE_Area_Nssa_Para_T));
    vr_id = SYS_DFLT_VR_ID;
    proc_id = name[oid_name_length];
    area_id = name[oid_name_length + 1];

    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *) var_val;
            if((val != OSPF_SNMP_ROW_STATUS_CREATEANDGO) && (val != OSPF_SNMP_ROW_STATUS_DESTROY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *) var_val;
            if(val == OSPF_SNMP_ROW_STATUS_CREATEANDGO)
            {
                if(OSPF_PMGR_AreaNssaSet(vr_id,proc_id,area_id,format,&nssa_para)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_AreaNssaUnset(vr_id,proc_id,area_id,format,nssa_para.config)!= OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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

/* FUNCTION NAME : var_ospfMultiProcessNssaTable
 * PURPOSE:
 *       var_ospfMultiProcessNssaTable
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 *       None.
 */
u_char *
var_ospfMultiProcessNssaTable (struct variable *vp,
                                       oid *name,
                                       size_t *length,
                                       int exact,
                                       size_t *var_len,
                                       WriteMethod **write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessNssaEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessNssaEntry_INSTANCE_LEN];
    OSPF_TYPE_Nssa_Area_T  data;

    memset(&data, 0, sizeof(OSPF_TYPE_Nssa_Area_T));
    /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/

    switch (vp->magic)
    {
        case OSPFMULTIPROCESSNSSATRANSLATORROLE:
            *write_method = write_ospfMultiProcessNssaTranslatorRole;
            break;
        case OSPFMULTIPROCESSNSSAREDISTRIBUTESTATUS:
            *write_method = write_ospfMultiProcessNssaRedistributeStatus;
            break;
        case OSPFMULTIPROCESSNSSAORIGINATEDEFAULTINFOSTATUS:
            *write_method = write_ospfMultiProcessNssaOriginateDefaultInfoStatus;
            break;
        case OSPFMULTIPROCESSNSSAMETRICTYPE:
            *write_method = write_ospfMultiProcessNssaMetricType;
            break;
        case OSPFMULTIPROCESSNSSAMETRIC:
            *write_method = write_ospfMultiProcessNssaMetric;
            break;
        case OSPFMULTIPROCESSNSSASTATUS:
            *write_method = write_ospfMultiProcessNssaStatus;
            break;
        default:
            *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessNssaEntry_INSTANCE_LEN);
    /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfNssaTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ospfNssaTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    best_inst[1] = data.area_id;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessNssaEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessNssaEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPFMULTIPROCESSNSSATRANSLATORROLE:
            if (CHECK_FLAG (data.config, OSPF_AREA_CONF_NSSA_TRANSLATOR))
            {
                long_return = data.translator_role + 1;
            }
            else
            {
                long_return = OSPF_SNMP_NSSA_TRANSLATE_CANDIDATE;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSAREDISTRIBUTESTATUS:
            if(CHECK_FLAG(data.config, OSPF_AREA_CONF_NO_REDISTRIBUTION))
            {
                long_return = OSPF_SNMP_NSSA_REDISTRIBUTE_DISABLED;
            }
            else
            {
                long_return = OSPF_SNMP_NSSA_REDISTRIBUTE_ENABLED;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSAORIGINATEDEFAULTINFOSTATUS:
            if(CHECK_FLAG(data.config, OSPF_AREA_CONF_DEFAULT_ORIGINATE))
            {
                long_return = OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_ENABLED;
            }
            else
            {
                long_return = OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_DISABLED;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSAMETRICTYPE:
            long_return =  data.metric_type;
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSAMETRIC:
            if(CHECK_FLAG(data.config, OSPF_AREA_CONF_METRIC))
            {
                long_return = data.metric;
            }
            else
            {
                long_return = 1;
            }
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSASTATUS:
            long_return =  OSPF_SNMP_ROW_STATUS_ACTIVE;
            return (u_char *) &long_return;
            break;
        case OSPFMULTIPROCESSNSSATRANSLATORSTATE:
            if(data.translator_state == OSPF_NSSA_TRANSLATOR_DISABLED)
            {
                long_return = data.translator_state + 3;
            }
            else
            {
                long_return =  data.translator_state;
            }
            return (u_char *) &long_return;
            break;
        default:
            ERROR_MSG("");
    }
    return NULL;
}


/*wang.tong add*/
#define ospfMgt_OID_NAME_LEN    (SNMP_MGR_Get_PrivateMibRootLen() + 6)
#define ospfIfParamEntry_INSTANCE_LEN    6

static BOOL_T ospfIfParamTable_get(int compc,
                                oid *compl,
                                OSPF_TYPE_IfParam_T  *data)
{
    oid tmp_compl[ospfIfParamEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    UI32_T s_addr;
    if (compc !=ospfIfParamEntry_INSTANCE_LEN)
    {
 	    return FALSE;
    }
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(1, 4, tmp_compl,255);
    SNMP_MGR_ConvertRemainToZero(compc,ospfIfParamEntry_INSTANCE_LEN, tmp_compl);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,1,  &s_addr);
    data->ip_address = s_addr;
    data->ifindex = compl[0];
    data->tos = compl[5];
    if(data->tos != 0)
        return FALSE;
    if (OSPF_PMGR_GetIfParamEntry(vr_id, vrf_id, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */
}

static BOOL_T ospfIfParamTable_next(int compc,
                                 oid *compl,
                                 OSPF_TYPE_IfParam_T *data)
{
    oid tmp_compl[ospfIfParamEntry_INSTANCE_LEN];
    UI32_T vr_id, vrf_id;
    UI32_T s_addr;
    /* Generate the instance of each table entry and find the
    * smallest instance that's larger than compc/compl.
    *
    * Step 1: Verify and extract the input key from "compc" and "compl"
    * Note: The number of input key is defined by "compc".
    *       The key for the specified instance is defined in compl.
    */
    vr_id = SYS_DFLT_VR_ID;
    vrf_id = SYS_DFLT_VRF_ID;
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(1, 4, tmp_compl,255);
    SNMP_MGR_ConvertRemainToZero(compc,ospfIfParamEntry_INSTANCE_LEN, tmp_compl);
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 1, &s_addr);
    data->ip_address = s_addr;
    data->ifindex = tmp_compl[0];
    data->tos = tmp_compl[5];
    if(data->tos != 0)
        return FALSE;
    data->config_type = OSPF_CONFIGURATION_TYPE_SNMP;

    if (OSPF_PMGR_GetNextIfParamEntry(vr_id, vrf_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    return TRUE;
}


/* This entry is used for entry which have the ability to row create*/
static OSPF_TYPE_IfParam_T ospfIfParamEntry;

/*
 * var_ospfIfMetricTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_ospfIfParamTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    oid compl[ospfIfParamEntry_INSTANCE_LEN];
    oid best_inst[ospfIfParamEntry_INSTANCE_LEN];
    OSPF_TYPE_IfParam_T data;
    UI32_T compc = 0;
    memset(&data, 0, sizeof(OSPF_TYPE_IfParam_T));

   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
      case OSPF_IFPARAM_COST:
        *write_method = write_ospfIfParamCost;
        break;
      case OSPF_IFPARAM_STATUS:
        *write_method = write_ospfIfParamStatus;
        break;
      default:
        *write_method =0;
         break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfIfParamEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ospfIfParamTable_get(compc, compl, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ospfIfParamTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    SNMP_MGR_BindIpInstance(data.ip_address, 1, best_inst);
    best_inst[0]= data.ifindex;
    best_inst[5]= data.tos;
    memcpy(name + vp->namelen, best_inst, ospfIfParamEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfIfParamEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
      case OSPF_IFPARAM_IFINDEX:
        long_return = data.ifindex;
        return (u_char*) &long_return;
      case OSPF_IFPARAM_IPADDRESS:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.ipaddr.s_addr;
        return (u_char*) &ipaddr_return;
      case OSPF_IFPARAM_TOS:
        long_return = data.tos;
        return (u_char*) &long_return;
#endif
      case OSPF_IFPARAM_COST:
        long_return = data.output_cost;
        return (u_char*) &long_return;
      case OSPF_IFPARAM_STATUS:
        long_return = OSPF_SNMP_IF_PARAM_VALID;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


int write_ospfIfParamCost(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T ip_address;
    UI16_T ifindex_offset = ospfMgt_OID_NAME_LEN ;
    UI16_T ip_net_offset = ospfMgt_OID_NAME_LEN + 1;
    UI16_T tos_offset = ospfMgt_OID_NAME_LEN +5;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;

    if (name_len !=  (ospfIfParamEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    SNMP_MGR_ReadIpFromCompl(name, ip_net_offset,  &ip_address);
    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
                return SNMP_ERR_WRONGTYPE;
            if (var_val_len > sizeof(long))
                return SNMP_ERR_WRONGLENGTH;
            break;
        case RESERVE2:
           /* In reserve2, we overwrite the set value to ospfIfParamEntry*/
           ospfIfParamEntry.output_cost = *(long *)var_val;
           if ((ospfIfParamEntry.output_cost < MIN_OSPF_IFPARAM_COST) || (ospfIfParamEntry.output_cost > MAX_OSPF_IFPARAM_COST))
               return SNMP_ERR_WRONGVALUE;
           break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
            ospfIfParamEntry.ip_address = ip_address;
            ospfIfParamEntry.ifindex = name[ifindex_offset];
            ospfIfParamEntry.tos = name[tos_offset];
            ospfIfParamEntry.config = OSPF_IF_PARAMS_OUTPUT_COST;
            if(ospfIfParamEntry.tos != 0)
                return SNMP_ERR_COMMITFAILED;

            if(OSPF_PMGR_SetIfParamEntry(vr_id, vrf_id, &ospfIfParamEntry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;

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

int write_ospfIfParamStatus(int action,
                         u_char * var_val,
                         u_char var_val_type,
                         size_t var_val_len,
                         u_char * statP, oid * name, size_t name_len)
{
    UI32_T ip_address;
    UI16_T ifindex_offset = ospfMgt_OID_NAME_LEN;
    UI16_T ip_net_offset = ospfMgt_OID_NAME_LEN + 1;
    UI16_T tos_offset = ospfMgt_OID_NAME_LEN +5;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;

   /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  ospfIfParamEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    SNMP_MGR_ReadIpFromCompl(name, ip_net_offset,  &ip_address);

    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
                return SNMP_ERR_WRONGTYPE;
            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case RESERVE2:
           /* In reserve2, we overwrite the set value to ospfIfParamEntry*/
           ospfIfParamEntry.config = *(long *)var_val;
           if ((ospfIfParamEntry.config != OSPF_SNMP_IF_PARAM_VALID) && (ospfIfParamEntry.config != OSPF_SNMP_IF_PARAM_INVALID))
           {
               return SNMP_ERR_WRONGVALUE;
           }
           break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
            {
                ospfIfParamEntry.ip_address = ip_address;
                ospfIfParamEntry.ifindex = name[ifindex_offset];
                ospfIfParamEntry.tos = name[tos_offset];
                if(ospfIfParamEntry.tos != 0)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                if(ospfIfParamEntry.config == OSPF_SNMP_IF_PARAM_VALID)
                {
                    if (OSPF_PMGR_GetIfParamEntry(vr_id, vrf_id, &ospfIfParamEntry) == OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_NOERROR;
                    }
                    ospfIfParamEntry.config = 0xffff;
                    ospfIfParamEntry.output_cost = DEFAULT_OSPF_IFPARAM_COST;
                }
                else
                {
                    if (OSPF_PMGR_GetIfParamEntry(vr_id, vrf_id, &ospfIfParamEntry) != OSPF_TYPE_RESULT_SUCCESS)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }
                    ospfIfParamEntry.config = 0;
                }

                if(OSPF_PMGR_SetIfParamEntry(vr_id, vrf_id, &ospfIfParamEntry) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
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


#define ospfMultiProcessSystemEntry_INSTANCE_LEN    1

static BOOL_T ospfMultiProcessSystemTable_get(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessSystem_T  *data)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    if (compc !=ospfMultiProcessSystemEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    data->proc_id = compl[0];

    if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id, vrf_id, data) != OSPF_TYPE_RESULT_SUCCESS)
        return FALSE;

    return TRUE;
}


static BOOL_T ospfMultiProcessSystemTable_next(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessSystem_T  *data)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    if(compc != 0)
    {
        data->proc_id =  compl[0];
    }

    if (compc < 1)
    {
        if (OSPF_PMGR_GetMultiProcessSystemEntry(vr_id, vrf_id, data) != OSPF_TYPE_RESULT_SUCCESS)
        {
            if (OSPF_PMGR_GetNextMultiProcessSystemEntry(vr_id, vrf_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (OSPF_PMGR_GetNextMultiProcessSystemEntry(vr_id, vrf_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return FALSE;
        }
    }

    return TRUE;
}


unsigned char  *
var_ospfMultiProcessSystemTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    UI32_T compc=0;
    oid compl[ospfMultiProcessSystemEntry_INSTANCE_LEN] ;
    oid best_inst;
    OSPF_TYPE_MultiProcessSystem_T data;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessSystem_T));
    switch(vp->magic)
    {
        case OSPF_MULTI_PROCESS_RFC1583_COMPATIBLE_STATE:

            *write_method = write_ospfMultiProcessSystemCompatibleRfc1853;
            break;

        case OSPF_MULTI_PROCESS_AUTO_COST:
            *write_method = write_ospfMultiProcessSystemAutoCost;
            break;

        case OSPF_MULTI_PROCESS_ORIGINATE_DEFAULT_ROUTE:
            *write_method = write_ospfMultiProcessSystemOriginateDefaultRoute;
            break;

        case OSPF_MULTI_PROCESS_ADVERTISE_DEFAULT_ROUTE:
            *write_method = write_ospfMultiProcessSystemAdvertiseDefaultRoute;
            break;

        case OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE:
            *write_method = write_ospfMultiProcessSystemExternalMetricType;
            break;

        case OSPF_MULTI_PROCESS_DEFAULT_EXTERNAL_METRIC:
            *write_method = write_ospfMultiProcessSystemDefaultExternalMetric;
            break;

        case OSPF_MULTI_PROCESS_SPF_HOLD_TIME:
            *write_method = write_ospfMultiProcessSystemSpfHoldTimer;
            break;

        case OSPF_MULTI_PROCESS_AREA_LIMIT:
            *write_method = write_ospfMultiProcessSystemAreaLimit;
            break;

        case OSPF_MULTI_PROCESS_SYSTEM_STATUS:
            *write_method = write_ospfMultiProcessSystemStatus;
            break;

        case OSPF_MULTI_PROCESS_ROUTER_ID:
            *write_method = write_ospfMultiProcessSystemRouterId;
            break;

        case OSPF_MULTI_PROCESS_SPF_DELAY_TIME:
            *write_method = write_ospfMultiProcessSystemSpfdelayTimer;
            break;

        case OSPF_MULTI_PROCESS_DEFAULT_METRIC:
            *write_method = write_ospfMultiProcessSystemDefaultMetric;
            break;

        default:
            *write_method =0;
            break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessSystemEntry_INSTANCE_LEN);
    if(exact)
    {
        if(!ospfMultiProcessSystemTable_get(compc, compl, &data))
        {
            return NULL;
        }
    }
    else
    {
         if(!ospfMultiProcessSystemTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst = data.proc_id;
    memcpy(name + vp->namelen, &best_inst, ospfMultiProcessSystemEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessSystemEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

     /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPF_MULTI_PROCESS_ROUTER_ID_TYPE:
            long_return =  data.routerId_type;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_RFC1583_COMPATIBLE_STATE:
            long_return =  data.rfc1583CompatibleState;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_AUTO_COST:
            long_return =  data.autoCost;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_ORIGINATE_DEFAULT_ROUTE:
            long_return =  data.originateDefaultRoute;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_ADVERTISE_DEFAULT_ROUTE:
            long_return =  data.advertiseDefaultRoute;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE:
            long_return =  data.externalMetricType;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_DEFAULT_EXTERNAL_METRIC:
            long_return =  data.defaultExternalMetric;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_SPF_HOLD_TIME:
            long_return =  data.spfHoldTime;
            return (u_char *) &long_return;
            break;

        case  OSPF_MULTI_PROCESS_AREA_NUMBER:
            long_return =  data.areaNumber;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_AREA_LIMIT:
            long_return =  data.areaLimit;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_SYSTEM_STATUS:
            long_return =  data.systemStatus;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_ROUTER_ID:
             *var_len = sizeof(ipaddr_return);
            ipaddr_return =  data.routerId.s_addr;
            return (u_char *) &ipaddr_return;
            break;

        case OSPF_MULTI_PROCESS_ADMIN_STAT:
            long_return =  data.adminStat;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_VERSION_NUMBER:
            long_return =  data.versionNumber;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_AREA_BDR_RTR_STATUS:
            long_return =  data.areaBdrRtrStatus;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_AS_BDR_RTR_STATUS:
            long_return =  data.asbdrRtrStatus;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_EXTERN_LSA_COUNT:
            long_return =  data.externLsaCount;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_EXTERN_LSA_CKSUM_SUM:
            long_return =  data.externLsaCksumSum;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_ORIGINATE_NEW_LSAS:
            long_return =  data.originateNewLsas;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_RX_NEW_LSAS:
            long_return =  data.rxNewLsas;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_AS_LSA_COUNT:
            long_return =  data.asLsaCount;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_SPF_DELAY_TIME:
            long_return =  data.spfDelayTime;
            return (u_char *) &long_return;
            break;

        case OSPF_MULTI_PROCESS_DEFAULT_METRIC:
            long_return =  data.defaultMetric;
            return (u_char *) &long_return;
            break;

        default:
            ERROR_MSG("");
    }
    return NULL;

}

int write_ospfMultiProcessSystemCompatibleRfc1853(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id, val;
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( (val != OSPF_RFC1583_COMPATIBLE_STATE_ENABLED)  && (val != OSPF_RFC1583_COMPATIBLE_STATE_DISABLED))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *)var_val;
            if(val == OSPF_RFC1583_COMPATIBLE_STATE_ENABLED)
            {
                if(OSPF_PMGR_CompatibleRfc1853Set(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
           else
           {
                if(OSPF_PMGR_CompatibleRfc1853Unset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
           }
        }
        break;

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


int write_ospfMultiProcessSystemAutoCost(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id, val;
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( val < OSPF_MULTI_PROCESS_AUTOCOST_MIN  ||val > OSPF_MULTI_PROCESS_AUTOCOST_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *)var_val;
            if(OSPF_PMGR_AutoCostSet(vr_id, proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return  SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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



int write_ospfMultiProcessSystemOriginateDefaultRoute(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    OSPF_TYPE_MultiProcessSystem_T data;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessSystem_T));
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( (val != OSPF_ORIGINATE_DEFAULT_ROUTE_ENABLED) && (val != OSPF_ORIGINATE_DEFAULT_ROUTE_DISABLED))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *)var_val;
            data.proc_id = proc_id;
            if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id,vrf_id, & data) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            if(data.originateDefaultRoute == val)
            {
                return SNMP_ERR_NOERROR;
            }

            if(val == OSPF_ORIGINATE_DEFAULT_ROUTE_ENABLED)
            {
                if(OSPF_PMGR_DefaultInfoSet(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return  SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_DefaultInfoUnset( vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }


        }
        break;

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


int write_ospfMultiProcessSystemAdvertiseDefaultRoute(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( (val != OSPF_ADVERTISE_DEFAULT_ROUTE_ALWAYS)  && (val != OSPF_ADVERTISE_DEFAULT_ROUTE_NOT_ALWAYS))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            entry.proc_id = proc_id;
            entry.proto = OSPF_TYPE_REDISTRIBUTE_DEFAULT;
            val = *(long *)var_val;
            if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if(entry.origin_type == OSPF_DEFAULT_ORIGINATE_UNSPEC)
                return SNMP_ERR_COMMITFAILED;

            if(val == OSPF_ADVERTISE_DEFAULT_ROUTE_ALWAYS)
            {
                if(OSPF_PMGR_DefaultInfoAlwaysSet(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
            else
            {
                if(OSPF_PMGR_DefaultInfoAlwaysUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                    return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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



int write_ospfMultiProcessSystemExternalMetricType(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( (val != OSPF_EXTERNAL_METRIC_TYPE_1)  && (val  != OSPF_EXTERNAL_METRIC_TYPE_2))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            entry.proc_id = proc_id;
            entry.proto = OSPF_TYPE_REDISTRIBUTE_DEFAULT;
            val = *(long *)var_val;
            if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if(entry.origin_type == OSPF_DEFAULT_ORIGINATE_UNSPEC)
                return SNMP_ERR_COMMITFAILED;

            if(val == OSPF_EXTERNAL_METRIC_TYPE_1)
            {
                if(OSPF_PMGR_DefaultInfoMetricTypeSet(vr_id,  proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_DefaultInfoMetricTypeUnset(vr_id,  proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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

int write_ospfMultiProcessSystemDefaultExternalMetric(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id;
    OSPF_TYPE_Multi_Proc_Redist_T entry;
    int val;
    memset(&entry,0,sizeof(OSPF_TYPE_Multi_Proc_Redist_T));

    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if(val < OSPF_DEFAULTEXTERNALMETRIC_MIN  ||val > OSPF_DEFAULTEXTERNALMETRIC_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            entry.proc_id = proc_id;
            entry.proto = OSPF_TYPE_REDISTRIBUTE_DEFAULT;
            val = *(long *)var_val;

            if(OSPF_PMGR_GetMultiProcRedistEntry(vr_id, vrf_id, &entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if(entry.origin_type == OSPF_DEFAULT_ORIGINATE_UNSPEC)
                return SNMP_ERR_COMMITFAILED;

            if(val == OSPF_DEFAULTEXTERNALMETRIC_MIN)
            {
                if(OSPF_PMGR_DefaultInfoMetricUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_DefaultInfoMetricSet(vr_id, proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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


int write_ospfMultiProcessSystemSpfHoldTimer(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    OSPF_TYPE_MultiProcessSystem_T data;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessSystem_T));
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( val < OSPF_SPF_TIMER_MIN  ||val > OSPF_SPF_TIMER_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            UI32_T delaytime;
            val = *(long *)var_val;
            data.proc_id = proc_id;
            if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id,vrf_id, & data) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            delaytime = data.spfDelayTime;
            if(OSPF_PMGR_TimerSet(vr_id, proc_id, delaytime, val) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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



int write_ospfMultiProcessSystemAreaLimit(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id, val;
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_UNSIGNED)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            val = *(u_long *)var_val;
            if( val < OSPF_AREA_LIMIT_MIN  ||val > OSPF_AREA_LIMIT_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(u_long *)var_val;
            if(OSPF_PMGR_AreaLimitSet(vr_id, proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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


int write_ospfMultiProcessSystemRouterId(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id, val;
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != SYS_TYPE_IPV4_ADDR_LEN)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            memcpy(&val, var_val, var_val_len);
            if(OSPF_PMGR_RouterIdSet(vr_id, proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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

int write_ospfMultiProcessSystemStatus(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    OSPF_TYPE_MultiProcessSystem_T data;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessSystem_T));
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if((val != OSPF_SNMP_ROW_STATUS_CREATEANDGO) && (val != OSPF_SNMP_ROW_STATUS_DESTROY))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            {
               data.proc_id = proc_id;
               val = *(long *)var_val;
               if(val == OSPF_SNMP_ROW_STATUS_CREATEANDGO)
               {
                    if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id,vrf_id, &data) != OSPF_TYPE_RESULT_SUCCESS)
                    {
                        if(OSPF_PMGR_RouterOspfSet(vr_id, vrf_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
                    }
                }
                else
                {
                    if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id,vrf_id, &data) == OSPF_TYPE_RESULT_SUCCESS)
                    {
                        if(OSPF_PMGR_RouterOspfUnset(vr_id, vrf_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                        {
                            return SNMP_ERR_COMMITFAILED;
                        }
                    }
                }
            }
            break;

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


int write_ospfMultiProcessSystemSpfdelayTimer(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    OSPF_TYPE_MultiProcessSystem_T data;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id, val;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessSystem_T));
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( val < OSPF_SPF_TIMER_MIN  ||val > OSPF_SPF_TIMER_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            UI32_T holdtime;
            val = *(long *)var_val;
            data.proc_id = proc_id;
            if(OSPF_PMGR_GetMultiProcessSystemEntry(vr_id,vrf_id, & data) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            holdtime = data.spfHoldTime;
            if(OSPF_PMGR_TimerSet(vr_id, proc_id, val, holdtime) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        break;

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



int write_ospfMultiProcessSystemDefaultMetric(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id, val;
    if (name_len !=  (ospfMultiProcessSystemEntry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proc_id = name[ospfMgt_OID_NAME_LEN];
    switch ( action )
    {
        case RESERVE1:
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
            val = *(long *)var_val;
            if( (int)val < OSPF_DEFAULTMETRIC_MIN  ||(int)val > OSPF_DEFAULTMETRIC_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            val = *(long *)var_val;
            if((int)val == OSPF_DEFAULTMETRIC_MIN)
            {
                if(OSPF_PMGR_DefaultMetricUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            else
            {
                if(OSPF_PMGR_DefaultMetricSet(vr_id, proc_id, val) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
        }
        break;

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


#define ospfMultiProcessNbrEntry_INSTANCE_LEN    5

static BOOL_T ospfMultiProcessNbrTable_get(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessNbr_T  *data)
{
    oid tmp_compl[ospfMultiProcessNbrEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T s_addr;
    if (compc !=ospfMultiProcessNbrEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ReadIpFromCompl(tmp_compl,1,  &s_addr);
    data->proc_id = tmp_compl[0];
    data->NbrIpAddr.s_addr = (in_addr_t)s_addr;

    if (OSPF_PMGR_GetMultiProcessNbrEntry(vr_id, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */
}


static BOOL_T ospfMultiProcessNbrTable_next(int compc,
                                oid *compl,
                                OSPF_TYPE_MultiProcessNbr_T  *data)
{
    oid tmp_compl[ospfMultiProcessNbrEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T s_addr;

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(1, 4, tmp_compl,255);
    SNMP_MGR_ConvertRemainToZero(compc,ospfMultiProcessNbrEntry_INSTANCE_LEN, tmp_compl);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,1,  &s_addr);
    data->proc_id = tmp_compl[0];
    data->NbrIpAddr.s_addr = (in_addr_t)s_addr;

    if (OSPF_PMGR_GetNextMultiProcessNbrEntry(vr_id, compc, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */
}


unsigned char  *
var_ospfMultiProcessNbrTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    oid compl[ospfMultiProcessNbrEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessNbrEntry_INSTANCE_LEN];
    OSPF_TYPE_MultiProcessNbr_T data;
    UI32_T compc = 0;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessNbr_T));

    switch (vp->magic)
    {
        default:
            *write_method =0;
            break;
    }

   SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessNbrEntry_INSTANCE_LEN);

    if (exact)/*get,set*/
    {
        if (!ospfMultiProcessNbrTable_get(compc, compl, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ospfMultiProcessNbrTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    SNMP_MGR_BindIpInstance(data.NbrIpAddr.s_addr,1, best_inst);
    best_inst[0]= data.proc_id;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessNbrEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessNbrEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
      case OSPF_MULTI_PROCESS_NBR_IPADDR:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.NbrIpAddr.s_addr;
        return (u_char*) &ipaddr_return;
#endif
      case OSPF_MULTI_PROCESS_NBR_RTR_ID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.router_id.s_addr;
        return (u_char*) &ipaddr_return;
      case OSPF_MULTI_PROCESS_NBR_OPTIONS:
        long_return = data.NbrOptions;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_NBR_PRIORITY:
        long_return = data.NbrPriority;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_NBR_STATE:
        long_return = data.NbrState;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_NBR_EVENTS:
        long_return = data.NbrEvent;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_NBR_LS_RETRANS_QLEN:
        long_return = data.NbrLsRetransQlen;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


#define ospfMultiProcessLsdbEntry_INSTANCE_LEN   14

static BOOL_T ospfMultiProcessLsdbTable_get(int compc,oid *compl,OSPF_TYPE_MultiProcessLsdb_T *data)
{
    oid tmp_compl[ospfMultiProcessLsdbEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T area_id;
    UI32_T ls_id;
    UI32_T router_id;
    if (compc !=ospfMultiProcessLsdbEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ReadIpFromCompl(tmp_compl,1,  &area_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,6,  &ls_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,10,  &router_id);
    data->proc_id = tmp_compl[0];
    data->LsdbArea_id.s_addr = (in_addr_t)area_id;
    data->LsdbType = tmp_compl[5];
    data->LsdbLsid.s_addr = (in_addr_t)ls_id;
    data->LsdbRouter_id.s_addr = (in_addr_t)router_id;
    if (OSPF_PMGR_GetMultiProcessLsdbEntry(vr_id, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */
}

static BOOL_T ospfMultiProcessLsdbTable_next(int compc,oid *compl,OSPF_TYPE_MultiProcessLsdb_T *data)
{
    oid tmp_compl[ospfMultiProcessLsdbEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T area_id;
    UI32_T ls_id;
    UI32_T router_id;
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(1, 4, tmp_compl,255);
    SNMP_MGR_checkCompl(6, 9, tmp_compl,255);
    SNMP_MGR_checkCompl(10, 13, tmp_compl,255);
    SNMP_MGR_ConvertRemainToZero(compc,ospfMultiProcessLsdbEntry_INSTANCE_LEN, tmp_compl);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,1,  &area_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,6,  &ls_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl,10,  &router_id);
    data->proc_id = tmp_compl[0];
    data->LsdbArea_id.s_addr = (in_addr_t)area_id;
    data->LsdbType = tmp_compl[5];
    data->LsdbLsid.s_addr = (in_addr_t)ls_id;
    data->LsdbRouter_id.s_addr = (in_addr_t)router_id;
    if (OSPF_PMGR_GetNextMultiProcessLsdbEntry(vr_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}

unsigned char  *
var_ospfMultiProcessLsdbTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    oid compl[ospfMultiProcessLsdbEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessLsdbEntry_INSTANCE_LEN];
    OSPF_TYPE_MultiProcessLsdb_T data;
    UI32_T compc = 0;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessLsdb_T));

    switch (vp->magic)
    {
        default:
            *write_method =0;
            break;
    }
   SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessLsdbEntry_INSTANCE_LEN);

    if (exact)/*get,set*/
    {
        if (!ospfMultiProcessLsdbTable_get(compc, compl, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ospfMultiProcessLsdbTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0]= data.proc_id;
    SNMP_MGR_BindIpInstance(data.LsdbArea_id.s_addr,1, best_inst);
    SNMP_MGR_BindIpInstance(data.LsdbLsid.s_addr, 6, best_inst);
    SNMP_MGR_BindIpInstance(data.LsdbRouter_id.s_addr, 10, best_inst);
    best_inst[5] = data.LsdbType;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessLsdbEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessLsdbEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
      case OSPF_MULTI_PROCESS_LSDB_AREA_ID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.LsdbArea_id.s_addr;
        return (u_char*) &ipaddr_return;
      case OSPF_MULTI_PROCESS_LSDB_TYPE:
        long_return = data.LsdbType;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_LSDB_LSID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.LsdbLsid.s_addr;
        return (u_char*) &ipaddr_return;
      case OSPF_MULTI_PROCESS_LSDB_ROUTER_ID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.LsdbRouter_id.s_addr;
        return (u_char*) &ipaddr_return;
#endif
      case OSPF_MULTI_PROCESS_LSDB_SQUENCE:
        long_return = data.LsdbSeqence;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_LSDB_AGE:
        long_return = data.LsdbAge;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_LSDB_CHK_SUM:
        long_return = data.LsdbChecksum;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_LSDB_ADVERTISE:
        *var_len = data.LsdbAdvertise_size;
        memcpy(return_buf, data.LsdbAdvertise, *var_len);
        return return_buf;

    default:
        ERROR_MSG("");
    }
    return NULL;
}

#define ospfMultiProcessExtLsdbEntry_INSTANCE_LEN   10

static BOOL_T ospfMultiProcessExtLsdbTable_get(int compc,oid *compl, OSPF_TYPE_MultiProcessExtLsdb_T *data)
{
    oid tmp_compl[ospfMultiProcessExtLsdbEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T ls_id;
    UI32_T router_id;

    if (compc !=ospfMultiProcessExtLsdbEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 2,  &ls_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 6,  &router_id);
    data->proc_id = tmp_compl[0];
    data->ExtLsdbType = tmp_compl[1];
    data->ExtLsdbLsid.s_addr = (in_addr_t)ls_id;
    data->ExtLsdbRouter_id.s_addr = (in_addr_t)router_id;
    if (OSPF_PMGR_GetMultiProcessExtLsdbEntry(vr_id, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */

}


static BOOL_T ospfMultiProcessExtLsdbTable_next(int compc, oid *compl, OSPF_TYPE_MultiProcessExtLsdb_T *data)
{
    oid tmp_compl[ospfMultiProcessExtLsdbEntry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T ls_id;
    UI32_T router_id;
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(2, 5, tmp_compl,255);
    SNMP_MGR_checkCompl(6, 9, tmp_compl,255);
    SNMP_MGR_ConvertRemainToZero(compc,ospfMultiProcessExtLsdbEntry_INSTANCE_LEN, tmp_compl);
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 2,  &ls_id);
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 6,  &router_id);
    data->proc_id = tmp_compl[0];
    data->ExtLsdbType = tmp_compl[1];
    data->ExtLsdbLsid.s_addr = (in_addr_t)ls_id;
    data->ExtLsdbRouter_id.s_addr = (in_addr_t)router_id;
    if (OSPF_PMGR_GetNextMultiProcessExtLsdbEntry(vr_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }
    return TRUE;
}


unsigned char  *
var_ospfMultiProcessExtLsdbTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    oid compl[ospfMultiProcessExtLsdbEntry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessExtLsdbEntry_INSTANCE_LEN];
    OSPF_TYPE_MultiProcessExtLsdb_T data;
    UI32_T compc = 0;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessExtLsdb_T));
    switch (vp->magic)
    {
        default:
            *write_method =0;
            break;
    }
   SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessExtLsdbEntry_INSTANCE_LEN);

    if (exact)/*get,set*/
    {
        if (!ospfMultiProcessExtLsdbTable_get(compc, compl, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ospfMultiProcessExtLsdbTable_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.proc_id;
    best_inst[1] = data.ExtLsdbType;
    SNMP_MGR_BindIpInstance(data.ExtLsdbLsid.s_addr,2, best_inst);
    SNMP_MGR_BindIpInstance(data.ExtLsdbRouter_id.s_addr, 6, best_inst);
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessExtLsdbEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessExtLsdbEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
      case OSPF_MULTI_PROCESS_EXT_LSDB_TYPE:
        long_return = data.ExtLsdbType;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_EXT_LSDB_LSID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.ExtLsdbLsid.s_addr;
        return (u_char*) &ipaddr_return;
      case OSPF_MULTI_PROCESS_EXT_LSDB_ROUTER_ID:
        *var_len = sizeof(ipaddr_return);
        ipaddr_return = data.ExtLsdbRouter_id.s_addr;
        return (u_char*) &ipaddr_return;
#endif
      case OSPF_MULTI_PROCESS_EXT_LSDB_SQUENCE:
        long_return = data.ExtLsdbSeqence;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_EXT_LSDB_AGE:
        long_return = data.ExtLsdbAge;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_EXT_LSDB_CHK_SUM:
        long_return = data.ExtLsdbChecksum;
        return (u_char*) &long_return;
      case OSPF_MULTI_PROCESS_EXT_LSDB_ADVERTISE:
        *var_len = data.ExtLsdbAdvertise_size;
        memcpy(return_buf, data.ExtLsdbAdvertise, *var_len);
        return return_buf;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


#define ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN   7

/* This entry is used for entry which have the ability to row create*/
static OSPF_TYPE_MultiProcessIfAuthMd5_T ospfIfAuthMd5Entry;

static BOOL_T ospfMultiProcessIfAuthMd5Table_get(int compc,oid *compl, OSPF_TYPE_MultiProcessIfAuthMd5_T *data)
{
    oid tmp_compl[ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T addr;

    if (compc !=ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN)
    {
        return FALSE;
    }

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 2,  &addr);
    data->address_type = tmp_compl[0];
    data->network_type = tmp_compl[1];
    data->addr.s_addr = (in_addr_t)addr;
    data->key_id = tmp_compl[6];
    if((data->address_type != INET_ADDRESS_TYPE_IPV4) || (data->network_type > INET_ADDRESS_IPV4_SIZE))
        return FALSE;
    if (OSPF_PMGR_GetIfAuthMd5Key(vr_id, vrf_id, data)!= OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */

}

static BOOL_T ospfMultiProcessIfAuthMd5Table_next(int compc,oid *compl, OSPF_TYPE_MultiProcessIfAuthMd5_T *data)
{
    oid tmp_compl[ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN];
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T addr;

    if ((compl[0] == 0) || ((compl[0] == INET_ADDRESS_TYPE_IPV4) && (compl[1] >= 0) && (compl[1] <INET_ADDRESS_IPV4_SIZE)))
    {
        SNMP_MGR_ConvertRemainToZero(2, ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN, compl);
    }
    else if((compl[0] > INET_ADDRESS_TYPE_IPV4) || compl[1] > INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_checkCompl(2, 5, tmp_compl,255);
    SNMP_MGR_ReadIpFromCompl(tmp_compl, 2,  &addr);
    data->address_type = INET_ADDRESS_TYPE_IPV4;
    data->network_type = INET_ADDRESS_IPV4_SIZE;
    data->addr.s_addr = (in_addr_t)addr;
    data->key_id = tmp_compl[6];

    if (OSPF_PMGR_GetNextIfAuthMd5Key(vr_id, vrf_id, compc, data) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
	/*End of if */
}

int write_ospfMultiProcessIfAuthMd5Key(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    UI32_T ip_address;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    UI16_T addresstype_offset = ospfMgt_OID_NAME_LEN ;
    UI16_T network_offset = ospfMgt_OID_NAME_LEN + 1;
    UI16_T address_offset = ospfMgt_OID_NAME_LEN +2;
    UI16_T keyid_offset = ospfMgt_OID_NAME_LEN +6;

    if (name_len !=  (ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN + ospfMgt_OID_NAME_LEN))
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    SNMP_MGR_ReadIpFromCompl(name, address_offset,  &ip_address);

    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
                return SNMP_ERR_WRONGTYPE;

           if(var_val_len > OSPF_AUTH_MD5_SIZE)
               return SNMP_ERR_WRONGLENGTH;
           break;
        case RESERVE2:
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
            memcpy (ospfIfAuthMd5Entry.key, var_val, var_val_len);
            ospfIfAuthMd5Entry.key[var_val_len] = '\0';
            ospfIfAuthMd5Entry.address_type = name[addresstype_offset];
            ospfIfAuthMd5Entry.addr.s_addr = ip_address;
            ospfIfAuthMd5Entry.network_type = name[network_offset];
            ospfIfAuthMd5Entry.key_id = name[keyid_offset];

            if((ospfIfAuthMd5Entry.address_type != INET_ADDRESS_TYPE_IPV4) ||(ospfIfAuthMd5Entry.network_type > INET_ADDRESS_IPV4_SIZE))
                return SNMP_ERR_COMMITFAILED;
            if(OSPF_PMGR_SetIfAuthMd5Key(vr_id, vrf_id, &ospfIfAuthMd5Entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }
            break;

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


unsigned char  *
var_ospfMultiProcessIfAuthMd5Table(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    oid compl[ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN];
    oid best_inst[ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN];
    OSPF_TYPE_MultiProcessIfAuthMd5_T data;
    UI32_T compc = 0;
    memset(&data, 0, sizeof(OSPF_TYPE_MultiProcessIfAuthMd5_T));

    switch (vp->magic)
    {
        case OSPF_MULTI_PROCESS_IF_AUTH_MD5_KEY:
            *write_method = write_ospfMultiProcessIfAuthMd5Key;
            break;
        default:
            *write_method = 0;
            break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN);

    if (exact)/*get,set*/
    {
        if (!ospfMultiProcessIfAuthMd5Table_get(compc, compl, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ospfMultiProcessIfAuthMd5Table_next(compc, compl, &data))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = data.address_type;
    best_inst[1] = data.network_type;
    SNMP_MGR_BindIpInstance(data.addr.s_addr,2, best_inst);
    best_inst[6] = data.key_id;
    memcpy(name + vp->namelen, best_inst, ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ospfMultiProcessIfAuthMd5Entry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic)
    {
        case OSPF_MULTI_PROCESS_IF_AUTH_MD5_KEY:
             *var_len = 0;  /* return Null string for md5 key */
             memcpy(return_buf, data.key, strlen((char*)data.key));
             return return_buf;

        default:
            ERROR_MSG("");
    }
    return NULL;
}

#define OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN  10

BOOL_T ospfMultiProcessRouteTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, OspfMultiProcessRouteNexthopEntry_T *entry_p)
{
    UI32_T i;

    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    entry_p->proc_id = compl[0];

    entry_p->route_dest.addrlen = entry_p->nexthop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    entry_p->route_dest.type = entry_p->nexthop.type = L_INET_ADDR_TYPE_IPV4;

    for (i = 0; i < entry_p->route_dest.addrlen; i++)
    {
        entry_p->route_dest.addr[i] = compl[1 + i];
    }

    entry_p->route_dest.preflen = compl[5];

    for (i = 0; i < entry_p->nexthop.addrlen; i++)
    {
        entry_p->nexthop.addr[i] = compl[6 + i];
    }

    return TRUE;
}

/*
 * var_ospfMultiProcessRouteTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_ospfMultiProcessRouteTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0;
    UI32_T i;
    oid compl[OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN] = {0};
    oid best_inst[OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN] = {0};
    OspfMultiProcessRouteNexthopEntry_T entry;

    switch (vp->magic)
    {
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact) /* get or set */
    {
        if (ospfMultiProcessRouteTable_OidIndexToData(exact, compc, compl, &entry) == FALSE)
        {
            return NULL;
        }

        if (OSPF_PMGR_GetOspfMultiProcessRouteNexthopEntry(&entry) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NULL;
        }
    }
    else /* getnext */
    {
        ospfMultiProcessRouteTable_OidIndexToData(exact, compc, compl, &entry);

        /* check the length of inputing index,if < 1 we should try get
         * {0.0.0.0.0...}
         */
        if (compc < 1)
        {
            if (OSPF_PMGR_GetOspfMultiProcessRouteNexthopEntry(&entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                if (OSPF_PMGR_GetNextOspfMultiProcessRouteNexthopEntry(&entry) != OSPF_TYPE_RESULT_SUCCESS)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if (OSPF_PMGR_GetNextOspfMultiProcessRouteNexthopEntry(&entry) != OSPF_TYPE_RESULT_SUCCESS)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = entry.proc_id;

    for (i = 0; i < entry.route_dest.addrlen; i++)
    {
        best_inst[1 + i] = entry.route_dest.addr[i];
    }

    best_inst[5] = entry.route_dest.preflen;

    for (i = 0; i < entry.nexthop.addrlen; i++)
    {
        best_inst[6 + i] = entry.nexthop.addr[i];
    }

    memcpy(name + vp->namelen, best_inst, OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + OSPFMULTIPROCESSROUTEENTRY_INSTANCE_LEN;

    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_ospfMultiProcessRouteDest:
            *var_len = entry.route_dest.addrlen;
            memcpy(return_buf, entry.route_dest.addr, *var_len);
            return (u_char*)return_buf;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_ospfMultiProcessRoutePfxLen:
            *var_len = sizeof(long_return);
            long_return = entry.route_dest.preflen;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_ospfMultiProcessRouteNexthop:
            *var_len = entry.nexthop.addrlen;
            memcpy(return_buf, entry.nexthop.addr, *var_len);
            return (u_char*)return_buf;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_ospfMultiProcessRouteInterface:
            *var_len = strlen(entry.ifname);
            memcpy(return_buf, entry.ifname, *var_len);
            return (u_char *) return_buf;

        case LEAF_ospfMultiProcessRouteCost:
            *var_len = sizeof(long_return);
            long_return = entry.path_cost;
            return (u_char *) &long_return;

        case LEAF_ospfMultiProcessRoutePathType:
            *var_len = sizeof(long_return);
            long_return = entry.path_code_type;
            return (u_char *) &long_return;

        case LEAF_ospfMultiProcessRouteAreaId:
            *var_len = SYS_ADPT_IPV4_ADDR_LEN;
            IP_LIB_UI32toArray(entry.area_id, return_buf);
            return (u_char*)return_buf;

        case LEAF_ospfMultiProcessRouteTransitArea:
            *var_len = sizeof(long_return);
            long_return = (TRUE == entry.istransit) ? VAL_ospfMultiProcessRouteTransitArea_transitArea : VAL_ospfMultiProcessRouteTransitArea_normalArea;
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
    }

    return NULL;
}
