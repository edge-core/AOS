/* =====================================================================================
 *  Module Name: MIB_SNMP.C
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
#include "mib_rip.h"
#include "netcfg_netdevice.h"
#include "sys_cpnt.h"
#include "sysORTable.h"
#include "leaf_1724.h"
#include "sys_type.h"
#include "netcfg_pmgr_rip.h"
#include "netcfg_type.h"
#include "es3626a_superset.h"
#include <string.h>
#include "ip_lib.h"
#include "l_stdlib.h"
#include <asn1.h>
#include "table.h"
#include "vlan_lib.h"
/*Define snmp local macros*/
#define temp_size 20
#define RipLnetNetworkType              1
#define RipDistanceInstanceIP_AddrStart         2
#define RipDistanceEntry_INSTANCE_LEN   7
#define RipInetInstanceIp_Addrstart             2
#define RipNeighborInstanceIp_Addrstart         2





/* RIP-MIB instances. */
oid rip_oid [] = { RIPMIB };

/* Define SNMP local variables. */
static RIP_Distance_snmp_T distance_temp_database[temp_size];

#define VID_TO_VLANNAME(dev_name, vid)    sprintf(dev_name, "vlan%d",vid)

/* Hook functions. */
static BOOL_T
del_distance(RIP_SNMP_DistanceTableIndex_T distance_index);
static BOOL_T
get_distance(RIP_Distance_T *distance_entry,RIP_SNMP_DistanceTableIndex_T distance_index);
static BOOL_T
add_distance(RIP_Distance_T *distance_entry,RIP_SNMP_DistanceTableIndex_T distance_index);
static BOOL_T
header_ripRedistributeEntry(struct variable *vp, oid * name, size_t * length, int exact, UI32_T *index);

static BOOL_T
header_ClearRipByNetworkEntry(struct variable *vp, oid * name, size_t * length, int exact);

/* ripMgt*/
int
do_ripUpdateTime(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    NETCFG_TYPE_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_Timer_T timer;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    memset(&timer,0,sizeof(NETCFG_TYPE_RIP_Timer_T));

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            if (NETCFG_PMGR_RIP_GetInstanceEntry(&entry)!= NETCFG_TYPE_OK)
            {
                long_return = RIP_UPDATE_TIMER_DEFAULT;
            }
            else
            {
                long_return = entry.timer.update;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<MIN_ripUpdateTime) ||(value>MAX_ripUpdateTime))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;
        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            NETCFG_PMGR_RIP_GetInstanceEntry(&entry);
            value = (*requests->requestvb->val.integer);
            timer.update = value;
            timer.garbage = entry.timer.garbage;
            timer.timeout= entry.timer.timeout;
            if( NETCFG_PMGR_RIP_TimerSet(&timer)!= NETCFG_TYPE_OK)
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}
/*timeout*/
//do_ripTimeoutTime
int
do_ripTimeoutTime(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    NETCFG_TYPE_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_Timer_T timer;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));

    switch(reqinfo->mode) {

        case MODE_GET:
        {
            if (NETCFG_PMGR_RIP_GetInstanceEntry(&entry)!= NETCFG_TYPE_OK)
            {
                long_return = RIP_TIMEOUT_TIMER_DEFAULT;
            }
            else
            {
                long_return = entry.timer.timeout;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;

        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<MIN_ripUpdateTime)||(value>MAX_ripUpdateTime))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;

            NETCFG_PMGR_RIP_GetInstanceEntry(&entry);
            value = (*requests->requestvb->val.integer);
            timer.timeout = value;
            timer.garbage = entry.timer.garbage;
            timer.update = entry.timer.update;
            if( NETCFG_PMGR_RIP_TimerSet(&timer)!= NETCFG_TYPE_OK)
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
        break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


/*garbage*/
int
do_ripGarbageCollectionTime(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    NETCFG_TYPE_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_Timer_T timer;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            if (NETCFG_PMGR_RIP_GetInstanceEntry(&entry)!= NETCFG_TYPE_OK)
            {
                long_return = RIP_GARBAGE_TIMER_DEFAULT;
            }
            else
            {
                long_return = entry.timer.garbage;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */

        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<MIN_ripUpdateTime)||(value>MAX_ripUpdateTime))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
        break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        /* XXX: perform the value change here */
        {
            UI32_T value;

            NETCFG_PMGR_RIP_GetInstanceEntry(&entry);
            value = (*requests->requestvb->val.integer);
            timer.timeout = entry.timer.timeout;
            timer.garbage = value;
            timer.update = entry.timer.update;
            if( NETCFG_PMGR_RIP_TimerSet(&timer)!= NETCFG_TYPE_OK)
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
/*wei.zhang 7/25*/
/*do_ripDefaultMetric*/
int
do_ripDefaultMetric(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    NETCFG_TYPE_RIP_Instance_T entry;

    entry.instance = RIP_SNMP_DEFAULT_VRF_ID;

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            if (NETCFG_PMGR_RIP_GetInstanceEntry(&entry)!= NETCFG_TYPE_OK)
            {
                long_return = RIP_DEFAULT_METRIC;
            }
            else
            {
                long_return = entry.default_metric;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<RIP_DEFAULT_METRIC_DEFAULT)||(value>RIP_METRIC_INFINITY))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if ( NETCFG_PMGR_RIP_DefaultMetricSet(value) != NETCFG_TYPE_OK)
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
/*do_ripMaxPrefix*/
int
do_ripMaxPrefix(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    NETCFG_TYPE_RIP_Instance_T entry;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    entry.instance = RIP_SNMP_DEFAULT_VRF_ID;

    switch(reqinfo->mode) {

        case MODE_GET:
        {
            if(NETCFG_PMGR_RIP_GetInstanceEntry(&entry) != NETCFG_TYPE_OK)
            {
                long_return = RIP_MAX_PREFIX;
            }
            else
            {
                long_return = entry.pmax;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            UI32_T value = 1;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value < RIP_MIN_MAXPREFIX)||(value > RIP_MAX_MAXPREFIX))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if ( NETCFG_PMGR_RIP_MaxPrefixSet(value) != NETCFG_TYPE_OK )
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
/*do_ripDefaultInformationOriginate*/
int
do_ripDefaultInformationOriginate(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    NETCFG_TYPE_RIP_Instance_T entry;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    entry.instance = RIP_SNMP_DEFAULT_VRF_ID;
    switch(reqinfo->mode) {
        case MODE_GET:
        {
            if(NETCFG_PMGR_RIP_GetInstanceEntry(&entry) != NETCFG_TYPE_OK)
            {
                long_return = RIP_DEFAULT_INFORMATION_ORIGINATE;
            }
            else
            {
                if(entry.default_information)
                {
                    long_return = RIP_SNMP_ROUTING_INFORMATION_ORIGINATE;
                }
                else
                {
                    long_return = RIP_SNMP_ROUTING_INFORMATION_NOORIGINATE;
                }
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
    /*EPR:ES3628BT-FLF-ZZ-00109 donny.li*/
            break;
    /*EPR:ES3628BT-FLF-ZZ-00109 donny.li end*/
        case MODE_SET_RESERVE1:
        {
            UI32_T value = 1;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<RIP_NO_ORIGINATE) ||(value>RIP_ORIGINATE))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value = 0;
            value = (*requests->requestvb->val.integer);
            if(value == RIP_SNMP_ROUTING_INFORMATION_ORIGINATE)
            {
                if(NETCFG_PMGR_RIP_DefaultAdd() != NETCFG_TYPE_OK)
                {
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                }
            }
            else if(value == RIP_SNMP_ROUTING_INFORMATION_NOORIGINATE)
            {
                if( NETCFG_PMGR_RIP_DefaultDelete()!= NETCFG_TYPE_OK)
                {
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                }
            }
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
/*do_ripDefaultDistance*/
int
do_ripDefaultDistance(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    NETCFG_TYPE_RIP_Instance_T entry;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    entry.instance = RIP_SNMP_DEFAULT_VRF_ID;

    switch(reqinfo->mode) {

        case MODE_GET:
        {
            if(NETCFG_PMGR_RIP_GetInstanceEntry(&entry) != NETCFG_TYPE_OK)
            {
                long_return = RIP_DEFAULT_DISTANCE;
            }
            else
            {
                long_return = entry.distance;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            UI32_T value = 0;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<MIN_DEFAULTDISTANCE)||(value>MAX_DEFAULTDISTANCE))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if(NETCFG_PMGR_RIP_DistanceDefaultSet(value) != NETCFG_TYPE_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

/*do_ripClearByType*/
int
do_ripClearByType(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    int ret;
    char typestr[temp_size]={0};
    UI32_T value = 0;

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            long_return = VAL_RIP_RouteClearByType_noClear;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<VAL_RIP_RouteClearByType_noClear)||(value>VAL_RIP_RouteClearByType_static))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            value = (*requests->requestvb->val.integer);
            if(value == VAL_RIP_RouteClearByType_noClear)
                return SNMP_ERR_NOERROR;
            else
            {
                if(value == VAL_RIP_RouteClearByType_all)
                    strcpy(typestr, "all");
                else if(value == VAL_RIP_RouteClearByType_connected)
                    strcpy(typestr, "connected");
                else if(value == VAL_RIP_RouteClearByType_ospf)
                    strcpy(typestr, "ospf");
                else if(value == VAL_RIP_RouteClearByType_rip)
                    strcpy(typestr, "rip");
                else if(value == VAL_RIP_RouteClearByType_static)
                    strcpy(typestr, "static");
                ret = NETCFG_PMGR_RIP_ClearRoute(typestr);
                if(ret != NETCFG_TYPE_OK)
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_ripRouterVersion(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    NETCFG_TYPE_RIP_Instance_T entry;
    entry.instance = RIP_SNMP_DEFAULT_VRF_ID;

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            if(NETCFG_PMGR_RIP_GetInstanceEntry(&entry) != NETCFG_TYPE_OK)
            {
                long_return = RIP_ROUTER_VERSION;
            }
            else
            {
                long_return = entry.version;
            }
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<VAL_ripRouterVersion_rip1)||(value>VAL_ripRouterVersion_byInterface))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if ( NETCFG_PMGR_RIP_VersionSet(value) != NETCFG_TYPE_OK )
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_ripStatisticsReset(netsnmp_mib_handler *handler,
                      netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info *reqinfo,
                      netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
        {

            long_return = VAL_ripStatisticsReset_noReset;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;
        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<VAL_ripStatisticsReset_reset)||(value>VAL_ripStatisticsReset_noReset))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if (value == VAL_ripStatisticsReset_reset)
            {
                if( NETCFG_PMGR_RIP_ClearStatistics() != NETCFG_TYPE_OK)
                {
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                }
            }
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_ripRoutingProcessStatus(netsnmp_mib_handler *handler,
                           netsnmp_handler_registration *reginfo,
                           netsnmp_agent_request_info *reqinfo,
                           netsnmp_request_info *requests)
{

    NETCFG_TYPE_RIP_Instance_T entry;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    switch(reqinfo->mode) {

        case MODE_GET:
        {
            BOOL_T  value;
            if (NETCFG_PMGR_RIP_GetInstanceEntry(&entry)== NETCFG_TYPE_OK)
                value = VAL_ripRoutingProcessStatus_enabled;
            else
                value = VAL_ripRoutingProcessStatus_disabled;
            long_return = value;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
            break;

        case MODE_SET_RESERVE1:
        {
            UI32_T value;

            if(requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
            if(requests->requestvb->val_len > sizeof(long))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                break;
            }
            value = (*requests->requestvb->val.integer);
            if ((value<VAL_ripRoutingProcessStatus_enabled)||(value>VAL_ripRoutingProcessStatus_disabled))
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
            }
        }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            switch(value)
            {
                case VAL_ripRoutingProcessStatus_enabled:
                if (NETCFG_PMGR_RIP_RouterRipSet()!= NETCFG_TYPE_OK)
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                    break;
                case VAL_ripRoutingProcessStatus_disabled:
                if (NETCFG_PMGR_RIP_RouterRipUnset() != NETCFG_TYPE_OK)
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                    break;
                default:
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                    break;
            }
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

/* ripInstabilityPreventingTable*/
#define rip2InstabilityPreventingEntry_OID_NAME_LEN 15
#define ripInstabilityPreventingEntry_INSTANCE_LEN 1
static BOOL_T ripInstabilityPreventingTable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                UI32_T   *value)
{
    NETCFG_TYPE_RIP_If_T entry;

    if (compc !=ripInstabilityPreventingEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    *index= compl[0];
    entry.ifindex = *index;
    if(NETCFG_PMGR_RIP_GetInterfaceEntry(&entry) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    else
    {
        *value = entry.split_horizon;
        return TRUE;
    } /*End of if */
}
static BOOL_T ripInstabilityPreventingTable_next(int   compc,
                                 oid     *compl,
                                 UI32_T    *index,
                                 UI32_T    *value)
{
    oid tmp_compl[ripInstabilityPreventingEntry_INSTANCE_LEN];
    NETCFG_TYPE_RIP_If_T entry;
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,ripInstabilityPreventingEntry_INSTANCE_LEN, tmp_compl);
    *index = tmp_compl[0];
    entry.ifindex = *index;

    if (NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    *value =  entry.split_horizon;
    *index = entry.ifindex;
    return TRUE;
}

/*
 * var_ripInstabilityPreventingTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_ripInstabilityPreventingTable(struct variable *vp,
                                  oid * name,
                                  size_t * length,
                                  int exact,
                                  size_t * var_len,
                                  WriteMethod ** write_method)
{
    UI32_T compc=0;
    oid compl[ripInstabilityPreventingEntry_INSTANCE_LEN];
    oid best_inst[ripInstabilityPreventingEntry_INSTANCE_LEN];
    UI32_T value = 0, index = 0;

   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case RIPSPLITHORIZONSTATUS:
        *write_method = write_ripSplitHorizonStatus;
            break;
        default:
        *write_method =0;
            break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ripInstabilityPreventingEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ripInstabilityPreventingTable_get(compc, compl, &index, &value))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ripInstabilityPreventingTable_next(compc, compl, &index, &value))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = index;
    memcpy(name + vp->namelen, best_inst, ripInstabilityPreventingEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ripInstabilityPreventingEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
        #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case RIPVLANINDEX:
            long_return = index;
        return (u_char*) &long_return;
        #endif
        case RIPSPLITHORIZONSTATUS:
            if(value == NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED)
            {
                value = VAL_ripSplitHorizonStatus_poisonReverse;
            }
            else if(value == NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE)
            {
                value = VAL_ripSplitHorizonStatus_none;
            }
            else
            {
                value = VAL_ripSplitHorizonStatus_splitHorizon;
            }

            long_return = value;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

int
write_ripSplitHorizonStatus(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    long value;

    UI32_T index;

    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  ripInstabilityPreventingEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    index = name[oid_name_length];

    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
            return SNMP_ERR_WRONGTYPE;
          }
            if (var_val_len > sizeof(long)) {
            return SNMP_ERR_WRONGLENGTH;
          }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            if ((value <VAL_ripSplitHorizonStatus_splitHorizon) || (value >VAL_ripSplitHorizonStatus_none))
                return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            value = * (long *) var_val;
            if(value == VAL_ripSplitHorizonStatus_splitHorizon)
            {
               value =  NETCFG_TYPE_RIP_SPLIT_HORIZON;
            }
            else if(value == VAL_ripSplitHorizonStatus_poisonReverse)
            {
                value = NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED;
            }
            else
            {
                value = NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE;
                if(NETCFG_PMGR_RIP_SplitHorizonUnset(index)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

            }

            if (NETCFG_PMGR_RIP_SplitHorizonSet(index, value) != NETCFG_TYPE_OK)
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

#define RipNetworkByInetAddrEntry_INSTANCE_LEN  7
int
write_ripNetworkByInetAddrStatus(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    RIP_SNMP_NetworkAddrTableIndex_T network_index;

    char addressStr[PREFIX_STRING_SIZE+1]={0};

    long value;
    char  addr[PREFIX_STRING_SIZE+1]={0};
    UI32_T inetAddrSize;
    L_PREFIX_T network_address;

    UI32_T oid_name_length ;
    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RipNetworkByInetAddrEntry_INSTANCE_LEN+ oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    memset(&network_index, 0, sizeof(RIP_SNMP_NetworkAddrTableIndex_T));
    network_index.type = name[oid_name_length];
    inetAddrSize = name[++oid_name_length];

    sprintf(addr,"%ld.%ld.%ld.%ld", (long)name[oid_name_length + 1],(long)name[oid_name_length + 2],(long)name[oid_name_length + 3],(long)name[oid_name_length +4]);

    network_index.pfx_len =  name[oid_name_length +5];
    sprintf(addressStr,"%s/%d",addr,network_index.pfx_len);
    memset(&network_address, 0, sizeof(network_address));
    L_PREFIX_Str2Prefix(addressStr, &network_address);

    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            if ((value <VAL_ripNetworkAddrStatus_valid) || (value >VAL_ripNetworkAddrStatus_invalid))
                return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            value = * (long *) var_val;
            switch(value)
            {
                case VAL_ripNetworkAddrStatus_valid:
                if ( NETCFG_PMGR_RIP_NetworkSetByAddress(network_address) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                    break;
                case  VAL_ripNetworkAddrStatus_invalid:
                if ( NETCFG_PMGR_RIP_NetworkUnsetByAddress(network_address) != NETCFG_TYPE_OK)
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

#define ripNetworkByInetAddrEntry_INSTANCE_LEN 7
static BOOL_T ripNetworkByInetAddrTable_get(int      compc,
                                oid     *compl,
                                NETCFG_TYPE_RIP_Network_T  *index,
                                UI32_T   *value)
{

    NETCFG_TYPE_RIP_Network_T entry;
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr;
    if (compc !=ripNetworkByInetAddrEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    if (compl[0] != RipLnetNetworkType)
    {
        return FALSE;
    }

    if (compl[1] != INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }
    int i;
    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }
    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry.pfxlen = compl[6];
    entry.ip_addr = ipaddr;
    index->ip_addr = entry.ip_addr;
    index->pfxlen =  entry.pfxlen;

    if(NETCFG_PMGR_RIP_GetNetworkTable(&entry)!= NETCFG_TYPE_OK)
    {
        *value = RIP_SNMP_NETWORKADDR_INVALID;
        return FALSE;
    }
    *value = RIP_SNMP_NETWORKADDR_VALID;
    return TRUE;
    /*End of if */
}
static BOOL_T ripNetworkByInetAddrTable_next(int   compc,
                                 oid     *compl,
                                 NETCFG_TYPE_RIP_Network_T    *index,
                                 UI32_T    *value)
{
    oid tmp_compl[ripNetworkByInetAddrEntry_INSTANCE_LEN];
    NETCFG_TYPE_RIP_Network_T entry;
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr;
    int i;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Network_T));

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,ripNetworkByInetAddrEntry_INSTANCE_LEN, tmp_compl);

    if ((compl[0] == 0) || ((compl[0] == RipLnetNetworkType) && (compl[1] >= 0) && (compl[1] <INET_ADDRESS_IPV4_SIZE)))
    {
        SNMP_MGR_ConvertRemainToZero(RipInetInstanceIp_Addrstart,ripNetworkByInetAddrEntry_INSTANCE_LEN, compl);
    }
    else if((compl[0] > RipLnetNetworkType) || compl[1] > INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    //memcpy(addr,&compl[2],sizeof(oid)*4);
    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }
    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry.pfxlen = compl[6];
    entry.ip_addr = ipaddr;

    if(NETCFG_PMGR_RIP_GetNextNetworkTable(&entry)!= NETCFG_TYPE_OK)
    {
        *value = RIP_SNMP_NETWORKADDR_INVALID;
        return FALSE;
    }
    *value = RIP_SNMP_NETWORKADDR_VALID;
    index->pfxlen = entry.pfxlen;
    index->ip_addr = entry.ip_addr;
    return TRUE;
}

/*
 * var_ripInstabilityPreventingTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */

unsigned char  *
var_ripNetworkByInetAddrTable(struct variable *vp,
                                  oid * name,
                                  size_t * length,
                                  int exact,
                                  size_t * var_len,
                                  WriteMethod ** write_method)
{
    UI32_T compc = 0;
    oid compl[ripNetworkByInetAddrEntry_INSTANCE_LEN];
    oid best_inst[ripNetworkByInetAddrEntry_INSTANCE_LEN];
    UI32_T value = RIP_SNMP_NETWORKADDR_INVALID;
    NETCFG_TYPE_RIP_Network_T index;
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};

    memset(&index, 0, sizeof(index));
   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case RIPNETWORKBYINETADDRSTATUS:
        *write_method = write_ripNetworkByInetAddrStatus;
            break;
        default:
        *write_method =0;
            break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ripNetworkByInetAddrEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ripNetworkByInetAddrTable_get(compc, compl, &index, &value))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ripNetworkByInetAddrTable_next(compc, compl, &index, &value))
        {
            return NULL;
        }
    }

    int offset = 0;
    int i;
    best_inst[offset] = 1;
    best_inst[++offset] = INET_ADDRESS_IPV4_SIZE;
    IP_LIB_UI32toArray(index.ip_addr, addr);
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
        best_inst[++offset] = addr[i];
    best_inst[++offset] = index.pfxlen;
    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    memcpy(name + vp->namelen, best_inst, ripNetworkByInetAddrEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ripNetworkByInetAddrEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
        case RIPNETWORKBYINETADDRSTATUS:
            long_return = value;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

#define RipRedistributeEntry_INSTANCE_LEN 1
#define Connect_RipRedistributeProtocol 1
#define Static_RipRedistributeProtocol  2
#define Ospf_RipRedistributeProtocol 3
#define Bgp_RipRedistributeProtocol 4

static BOOL_T header_ripRedistributeEntry(struct variable *vp,
                                               oid * name,
                                               size_t * length,
                                               int exact,
                                               UI32_T *index)
{
    int     result,len;

    result = snmp_oid_compare(name, *length, vp->name, vp->namelen);

    if(exact)/*Get */
    {
        /* Check the length. */
        if (result < 0 || *length - vp->namelen != 1)
        {
            return FALSE;
        }
        /* Get index*/
        *index = name[vp->namelen];
        return TRUE;
    }
    else
    {
        if (result >= 0)
        {
            len = *length - vp->namelen;
            if(len == 0)
            {
                *index = 0;
            }
            else if(len == 1)
            {
                *index = name[vp->namelen];
            }
            else if(len > 1)
            {
                *index = name[vp->namelen];
            }
            return TRUE;
        }
        else
        {
            /* set the user's oid to be ours */
            memcpy (name, vp->name, ((int) vp->namelen) * sizeof (oid));
            return TRUE;
        }
    }
}
int
write_ripRedistributeStatus(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    u_int32_t proto;
    long value;
    char protocol[10] = {0};
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RipRedistributeEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proto = name[oid_name_length];
    if(proto < Connect_RipRedistributeProtocol || proto > Bgp_RipRedistributeProtocol)
        return SNMP_ERR_WRONGVALUE;

    if(proto == Connect_RipRedistributeProtocol)
    {
        strcpy(protocol,"connected");
    }
    else if(proto == Static_RipRedistributeProtocol)
    {
        strcpy(protocol,"static");
    }
    else if(proto == Ospf_RipRedistributeProtocol)
    {
        strcpy(protocol,"ospf");
    }
    else if(proto == Bgp_RipRedistributeProtocol)
    {
        strcpy(protocol,"bgp");
    }
    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case RESERVE2:
            value = *(long *)var_val;
            if ((value <RIP_API_REDIST_STATUS_VALID) || (value >RIP_API_REDIST_STATUS_INVALID))
                return SNMP_ERR_WRONGVALUE;
            break;
        case FREE:
            /* Release any resources that have been allocated */
            break;
        case ACTION:
        {
            value = * (long *) var_val;
        if(value == RIP_API_REDIST_STATUS_VALID)
        {
            if( NETCFG_PMGR_RIP_RedistributeSet(protocol)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        else if(value == RIP_API_REDIST_STATUS_INVALID)
        {
            if( NETCFG_PMGR_RIP_RedistributeUnset(protocol)!= NETCFG_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }
        }
        else
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

int
write_ripRedistributeMetric(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    u_int32_t proto;
    long value;
    char protocol[10] = {0};
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RipRedistributeEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proto = name[oid_name_length];
    if(proto < Connect_RipRedistributeProtocol || proto > Bgp_RipRedistributeProtocol)
    {
        return SNMP_ERR_NOSUCHNAME;
    }
    if(proto == Connect_RipRedistributeProtocol)
    {
        strcpy(protocol,"connected");
    }
    else if(proto == Static_RipRedistributeProtocol)
    {
        strcpy(protocol,"static");
    }
    else if(proto == Ospf_RipRedistributeProtocol)
    {
        strcpy(protocol,"ospf");
    }
    else if(proto == Bgp_RipRedistributeProtocol)
    {
        strcpy(protocol,"bgp");
    }
    //printf("protocol:%s,proto:%d\n",protocol,proto);
    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                //printf("%s,%d\n",__FUNCTION__,__LINE__);
                return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              //printf("%s,%d\n",__FUNCTION__,__LINE__);
              return SNMP_ERR_WRONGLENGTH;
          }
            break;
        case RESERVE2:
            value = *(long *)var_val;
            if ((value <MINSIZE_RIP_DEFAULTMETRIC) || (value >MAXSIZE_RIP_DEFAULTMETRIC))
            {
                //printf("%s,%d\n",__FUNCTION__,__LINE__);
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;
        case ACTION:
            value = *(long *)var_val;
            if(NETCFG_PMGR_RIP_RedistributeMetricSet (protocol, value) != NETCFG_TYPE_OK)
            {
                //printf("%s,%d\n",__FUNCTION__,__LINE__);
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
    //printf("%s,%d\n",__FUNCTION__,__LINE__);
    return SNMP_ERR_NOERROR;
}

#if 0
int
write_ripRedistributeRmap(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    u_int32_t proto;
    char protocol[10] = {0};
    char    rmap[SYS_ADPT_ACL_MAX_NAME_LEN + 1] = {0};
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RipRedistributeEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    proto = name[oid_name_length];
    if(proto < Connect_RipRedistributeProtocol || proto > Ospf_RipRedistributeProtocol)
        return SNMP_ERR_WRONGVALUE;

    if(proto == Connect_RipRedistributeProtocol)
    {
        strcpy(protocol,"connected");
    }
    else if(proto == Static_RipRedistributeProtocol)
    {
        strcpy(protocol,"static");
    }
    else if(proto == Ospf_RipRedistributeProtocol)
    {
        strcpy(protocol,"ospf");
    }

    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if ((var_val_len > 16) || (var_val_len <= 0))
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case RESERVE2:

            if (!((strlen((char *)var_val) <= SYS_ADPT_ACL_MAX_NAME_LEN) && (strlen((char *)var_val) >0)))
                return SNMP_ERR_WRONGVALUE;
            break;
        case FREE:
            /* Release any resources that have been allocated */
            break;
        case ACTION:
            if(strlen((char *)var_val))
            {
                strcpy(rmap, (char *)var_val);
            }
            else
            {
                return SNMP_ERR_WRONGVALUE;
            }

            if( NETCFG_PMGR_RIP_RedistributeRmapSet(protocol,rmap)!= NETCFG_TYPE_OK)
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
#endif /* #if 0 */

/* FUNCTION NAME :var_ripRedistribueTable
 * PURPOSE:
 *       rip redistribute snmp entry;
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
var_ripRedistribueTable (struct variable *v, oid *name, size_t *length,
                             int exact, size_t *var_len, WriteMethod **write_method)
{
    UI32_T  index = 0;
    UI32_T  status = RIP_API_REDIST_STATUS_INVALID;
    UI32_T  metric = 0;
    char    rmap_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1] = {0};
    NETCFG_TYPE_RIP_Redistribute_Table_T entry;
    NETCFG_TYPE_RIP_Instance_T rip_instance;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    memset(&rip_instance,0,sizeof(NETCFG_TYPE_RIP_Instance_T));
    switch (v->magic)
    {
       case RIPREDISTRIBUTEMETRIC:
           *write_method = write_ripRedistributeMetric;
           break;

       case RIPREDISTRIBUTESTATUS:
           *write_method = write_ripRedistributeStatus;
           break;

#if 0
       case RIPREDISTRIBUTEREMAPNAME:
           *write_method = write_ripRedistributeRmap;
           break;
#endif

       default:
           *write_method = 0;
           break;
    }

    if (header_ripRedistributeEntry(v, name, length, exact, &index) == FALSE)
        return NULL;


    if (exact)
    {
        if( NETCFG_PMGR_RIP_GetNextRedistributeTable(&entry) != NETCFG_TYPE_OK)
        {
            return NULL;
        }

        //printf("%s,%d,metric:%d,rmap:%s\n",__FUNCTION__,__LINE__,entry.table.metric,entry.table.rmap_name);
        metric = entry.table.metric;
        if(strlen(entry.table.rmap_name))
            strcpy(rmap_name, entry.table.rmap_name);
        status = RIP_API_REDIST_STATUS_VALID;
        //printf("%s,%d,protocol:%d,metric:%d,status:%d,rmap_name:%s\n",__FUNCTION__,__LINE__,index,metric,status,rmap_name);
    }
    else
    {
        if(index != 0)
        {
            entry.protocol = index - 1;
        }
        else
        {
            entry.protocol = NETCFG_TYPE_RIP_Redistribute_Max;
        }

        if( NETCFG_PMGR_RIP_GetNextRedistributeTable(&entry) != NETCFG_TYPE_OK)
        {
            return NULL;
        }
        metric = entry.table.metric;
        if(strlen(entry.table.rmap_name))
            strcpy(rmap_name, entry.table.rmap_name);
        status = RIP_API_REDIST_STATUS_VALID;
        //printf("%s,%d,protocol:%d,metric:%d,status:%d,rmap_name%s\n",__FUNCTION__,__LINE__,entry.protocol,metric,status,rmap_name);
        name[v->namelen] = entry.protocol + 1;
        *length = v->namelen + 1;
    }

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (v->magic) {
       case RIPREDISTRIBUTEMETRIC:
           *var_len = sizeof(long_return);
           long_return = metric;
           return (u_char*) &long_return;
           break;

       case RIPREDISTRIBUTESTATUS:
           *var_len = sizeof(long_return);
           long_return = status;
           return (u_char*) &long_return;
           break;

#if 0
       case RIPREDISTRIBUTEREMAPNAME:
           *var_len = strlen (rmap_name);
           if(strlen(rmap_name))
               strcpy((char *)return_buf, rmap_name);
           else
               strcpy((char *)return_buf,"");
           return (u_char*) return_buf;
           break;
#endif

       default:
           ERROR_MSG("");
    }
    return NULL;
}


#define RipNeighborEntry_INSTANCE_LEN 6
int
write_ripNeighborStatus(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)

{
    int ret;
    long value;
    int i;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    char  inetaddr_index[INET_ADDRESS_IPV4_SIZE +1] = {0};
    long intval;
    UI32_T addr;
    UI32_T ipaddr;

    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
        inetaddr_index[i] = name[oid_name_length + 2 + i];
    IP_LIB_ArraytoUI32((UI8_T *)inetaddr_index, &ipaddr);
    addr = ipaddr;
    switch ( action ) {
        case RESERVE1:
            if (name_len!=  RipNeighborEntry_INSTANCE_LEN+ oid_name_length)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case RESERVE2:
            intval = *(long *)var_val;
            if (intval != RIP_SNMP_NEIGHBOR_VALID && intval != RIP_SNMP_NEIGHBOR_INVALID )
                return SNMP_ERR_BADVALUE;
                break;
        case FREE:
             /* Release any resources that have been allocated */
            break;
        case ACTION:
        {
            value = *(long *)var_val;
            if(value == RIP_SNMP_NEIGHBOR_VALID)
            {
                ret = NETCFG_PMGR_RIP_NeighborSet(addr);
                if(ret == NETCFG_TYPE_OK)
                    return SNMP_ERR_NOERROR;
            }
            else
            {
                if( NETCFG_PMGR_RIP_NeighborUnset(addr) == NETCFG_TYPE_OK)
                    return SNMP_ERR_NOERROR;
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
static BOOL_T ripNeighborTable_get(int      compc,
                                oid     *compl,
                                NETCFG_TYPE_RIP_Network_T *entry)
{

    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    int i;

    if (compc !=RipNeighborEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    if (compl[0] != RipLnetNetworkType)
    {
        return FALSE;
    }

    if (compl[1] != INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->ip_addr = ipaddr;


    if(NETCFG_PMGR_RIP_GetNeighborTable(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}
static BOOL_T ripNeiborTable_next(int   compc,
                                 oid     *compl,
                                 NETCFG_TYPE_RIP_Network_T *entry)
{
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    int i;
    memset(entry,0,sizeof(NETCFG_TYPE_RIP_Network_T));

    if ((compl[0] == 0) || ((compl[0] == RipLnetNetworkType) && (compl[1] <INET_ADDRESS_IPV4_SIZE)))
    {
        SNMP_MGR_ConvertRemainToZero(RipNeighborInstanceIp_Addrstart,RipNeighborEntry_INSTANCE_LEN, compl);
    }
    else if((compl[0] > RipLnetNetworkType) || compl[1] > INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->ip_addr = ipaddr;

    if(NETCFG_PMGR_RIP_GetNextNeighborTable(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME :var_ripNeighborTable
 * PURPOSE:
 *       ripNeighborTable
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
var_ripNeighborTable(struct variable *v, oid *name, size_t *length,
               int exact, size_t *var_len, WriteMethod **write_method)
{

    NETCFG_TYPE_RIP_Network_T entry;
    UI32_T compc = 0;
    int i;
    oid compl[RipNeighborEntry_INSTANCE_LEN] = {0};
    oid best_inst[RipNeighborEntry_INSTANCE_LEN];
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    int lvalue = RIP_SNMP_NEIGHBOR_INVALID;
    int offset = 0;

    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Network_T));
    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc,compl, RipNeighborEntry_INSTANCE_LEN);

    switch (v->magic)
    {
        case RIPNEIGHBORADDRESSSTATUS:
        /* MAX-ACCESS   read-create */
        *write_method = write_ripNeighborStatus;

            break;
        default:
            break;
    }

    if (exact)/*get*/
    {
        if (!ripNeighborTable_get(compc, compl, &entry))
            return NULL;
        lvalue = RIP_SNMP_NEIGHBOR_VALID;
    }
    else/*getnext*/
    {
        if (!ripNeiborTable_next(compc, compl, &entry))
        {
            return NULL;
        }
        lvalue = RIP_SNMP_NEIGHBOR_VALID;
    }

    best_inst[offset] = 1;
    best_inst[++offset] = INET_ADDRESS_IPV4_SIZE;
    IP_LIB_UI32toArray(entry.ip_addr, addr);
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
        best_inst[++offset] = addr[i];
    memcpy(name, v->name, v->namelen*sizeof(oid));

    memcpy(name + v->namelen, best_inst, RipNeighborEntry_INSTANCE_LEN*sizeof(oid));
    *length = v->namelen +RipNeighborEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (v->magic)
    {
     case RIPNEIGHBORADDRESSSTATUS:
         long_return = lvalue;
         return (u_char*) &long_return;
     default:
         ERROR_MSG("");
    }
    return NULL;
}

#define ripNetworkAddrEntry_INSTANCE_LEN 1
static BOOL_T ripNetworkByInterfaceTable_get(int      compc,
                                oid     *compl,
                                UI32_T   *data)
{
	NETCFG_TYPE_RIP_If_T Interface;

	if (compc !=ripNetworkAddrEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    memset(&Interface,0,sizeof(NETCFG_TYPE_RIP_If_T));
    *data = compl[0];
    Interface.ifindex =  *data;


    if(NETCFG_PMGR_RIP_GetInterfaceEntry(&Interface) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    else
    {
        if(Interface.network_if)
         return TRUE;
    }
    return FALSE;
   /*End of if */
}

static BOOL_T ripNetworkByInterfaceTable_next(int      compc,
                                 oid     *compl,
                                 UI32_T    *data)
{
    UI32_T ifindex ;
    NETCFG_TYPE_RIP_If_T Interface;
    oid tmp_compl[ripNetworkAddrEntry_INSTANCE_LEN];

    memcpy(tmp_compl, compl, sizeof(tmp_compl));

    SNMP_MGR_ConvertRemainToZero(compc,ripNetworkAddrEntry_INSTANCE_LEN, tmp_compl);
    ifindex = (UI32_T)tmp_compl[0];
    Interface.ifindex = ifindex;

    if (NETCFG_PMGR_RIP_GetNextInterfaceEntry(&Interface) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    else
    {
        if(!Interface.network_if)
        {
            return FALSE;
        }
    }
    *data = Interface.ifindex;
    return TRUE;
}

/*
 * var_ripNetworkByInterfaceTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_ripNetworkByInterfaceTable(struct variable *vp,
                        oid * name,
                        size_t * length,
                        int exact,
                        size_t * var_len, WriteMethod ** write_method)
{
    UI32_T compc=0;
    oid compl[ripNetworkAddrEntry_INSTANCE_LEN];
    UI32_T index ;
   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case RIPNETWORKADDRSTATUS:
        *write_method = write_ripNetworkByInterfaceStatus;
        break;
        default:
         *write_method =0;
        break;
    }
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, ripNetworkAddrEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ripNetworkByInterfaceTable_get(compc, compl, &index))
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        if (!ripNetworkByInterfaceTable_next(compc, compl, &index))
        {
            return NULL;
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    memcpy(name + vp->namelen, &index, ripNetworkAddrEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +ripNetworkAddrEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
        #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case RIPNETWORKIFINDEX:
            long_return = index;
            return (u_char*) &long_return;
        #endif
        case RIPNETWORKADDRSTATUS:
            long_return = VAL_ripNetworkAddrStatus_valid;
            return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}
int
write_ripNetworkByInterfaceStatus(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    long value;
    UI32_T ifindex ;
    UI32_T nVlan;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!= ripNetworkAddrEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    ifindex = name[oid_name_length];
    VLAN_OM_ConvertFromIfindex(ifindex, &nVlan);

    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
          break;

        case RESERVE2:
            value = *(long *)var_val;
            if ((value <VAL_ripNetworkAddrStatus_valid) || (value >VAL_ripNetworkAddrStatus_invalid))
               return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            value = *(long *)var_val;
            switch(value)
            {
                case VAL_ripNetworkAddrStatus_valid:
                if ( NETCFG_PMGR_RIP_NetworkSetByVid(nVlan) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                    break;
                case  VAL_ripNetworkAddrStatus_invalid:
                if ( NETCFG_PMGR_RIP_NetworkUnsetByVid(nVlan) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                    break;
                default:
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

/* ripPassiveInterfaceTable*/
#define RIPPASSIVEINTERFACE_INSTANCE_LEN 1
static BOOL_T ripPassiveInterfaceTable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                UI32_T   *value)
{
    NETCFG_TYPE_RIP_If_T entry;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_If_T));
    if (compc !=RIPPASSIVEINTERFACE_INSTANCE_LEN)
    {
        return FALSE;
    }
    *index= compl[0];
    entry.ifindex = *index;
    if(NETCFG_PMGR_RIP_GetInterfaceEntry(&entry) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    else
    {
        *value = entry.pass_if;
        return TRUE;
    } /*End of if */
}
static BOOL_T ripPassiveInterfaceTable_next(int   compc,
                                 oid     *compl,
                                 UI32_T    *index,
                                 UI32_T    *value)
{
    oid tmp_compl[RIPPASSIVEINTERFACE_INSTANCE_LEN];
    NETCFG_TYPE_RIP_If_T entry;
    /* Generate the instance of each table entry and find the
     * smallest instance that's larger than compc/compl.
     *
     * Step 1: Verify and extract the input key from "compc" and "compl"
     * Note: The number of input key is defined by "compc".
     *       The key for the specified instance is defined in compl.
     */
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_If_T));

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,RIPPASSIVEINTERFACE_INSTANCE_LEN, tmp_compl);

    *index = tmp_compl[0];
    entry.ifindex = *index;
    if (NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    *value =  entry.pass_if;
    *index = entry.ifindex;
    return TRUE;
}

/*
 * var_ripInstabilityPreventingTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_ripPassiveInterfaceTable(struct variable *vp,
                                  oid * name,
                                  size_t * length,
                                  int exact,
                                  size_t * var_len,
                                  WriteMethod ** write_method)
{
    UI32_T compc=0;
    oid compl[RIPPASSIVEINTERFACE_INSTANCE_LEN];
    oid best_inst[RIPPASSIVEINTERFACE_INSTANCE_LEN];
    UI32_T value = 0, index = 0;

   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
        case RIPPASSIVEINTERFACESTATUS:
        *write_method = write_ripPassiveInterfaceStatus;
            break;
        default:
        *write_method =0;
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, RIPPASSIVEINTERFACE_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!ripPassiveInterfaceTable_get(compc, compl, &index, &value))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ripPassiveInterfaceTable_next(compc, compl, &index, &value))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0] = index;
    memcpy(name + vp->namelen, best_inst, RIPPASSIVEINTERFACE_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +RIPPASSIVEINTERFACE_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
        #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case RIPINTERFACEINDEX:
            long_return = index;
            return (u_char*) &long_return;
        #endif
        case RIPPASSIVEINTERFACESTATUS:
        *var_len = sizeof(long_return);
        if(value)
        {
            long_return = RIP_SNMP_PASSIVE_INTERFACE_VALID;
        }
        else
        {
            long_return = RIP_SNMP_PASSIVE_INTERFACE_INVALID;
        }
        return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

int
write_ripPassiveInterfaceStatus(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    long value;
    UI32_T index, nVlan;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;
    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RIPPASSIVEINTERFACE_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    index = name[oid_name_length];
    VLAN_OM_ConvertFromIfindex(index, &nVlan);

    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            if ((value <RIP_SNMP_PASSIVE_INTERFACE_VALID)||(value >RIP_SNMP_PASSIVE_INTERFACE_INVALID))
                return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            value = *(long *)var_val;

            if (value == RIP_SNMP_PASSIVE_INTERFACE_VALID)
            {
                if (NETCFG_PMGR_RIP_PassiveIfAdd(nVlan) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
            }
            if (value == RIP_SNMP_PASSIVE_INTERFACE_INVALID)
            {
                if (NETCFG_PMGR_RIP_PassiveIfDelete(nVlan) != NETCFG_TYPE_OK)
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

#define RipRouteClearByNetworkEntry_INSTANCE_LEN  7
static BOOL_T
header_ClearRipByNetworkEntry(struct variable *vp, oid * name, size_t * length, int exact)
{
    int     result;

    result = snmp_oid_compare(name, *length, vp->name, vp->namelen);

    if(exact)/*Get */
    {
        return TRUE;
    }
    else
    {
        if (result >= 0)
        {
            return TRUE;
        }
        else
        {
            /* set the user's oid to be ours */
            memcpy (name, vp->name, ((int) vp->namelen) * sizeof (oid));
            return TRUE;
        }
    }
}



int
write_ripRouteClearByNetworkStatus(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    RIP_SNMP_NetworkAddrTableIndex_T network_index;

    char addressStr[PREFIX_STRING_SIZE+1]={0};

    long value;
    char  addr[PREFIX_STRING_SIZE+1]={0};
    UI32_T inetAddrSize;

    UI32_T oid_name_length ;
    oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  RipRouteClearByNetworkEntry_INSTANCE_LEN+ oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    memset(&network_index, 0, sizeof(RIP_SNMP_NetworkAddrTableIndex_T));
    network_index.type = name[oid_name_length];
    inetAddrSize = name[++oid_name_length];

    sprintf(addr,"%ld.%ld.%ld.%ld", (long)name[oid_name_length + 1],(long)name[oid_name_length + 2],(long)name[oid_name_length + 3],(long)name[oid_name_length + 4]);

    network_index.pfx_len =  name[oid_name_length + 5];
    sprintf(addressStr,"%s/%d",addr,network_index.pfx_len);
    switch ( action ) {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER) {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long)) {
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            value = *(long *)var_val;
            if ((value <VAL_ripRouteClearByNetwork_noclear) || (value >VAL_ripRouteClearByNetwork_clear))
                return SNMP_ERR_WRONGVALUE;
            break;

        case FREE:
             /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            value = *(long *)var_val;
            switch(value)
            {
                case VAL_ripRouteClearByNetwork_clear:
                if ( NETCFG_PMGR_RIP_ClearRoute(addressStr) != NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                break;
                default:
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

/* FUNCTION NAME : var_ripRouteClearByNetworkTable
 * PURPOSE:
 *       var_ripRouteClearByNetworkTable
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
var_ripRouteClearByNetworkTable (struct variable *vp, oid *name, size_t *length,
	     int exact, size_t *var_len, WriteMethod **write_method)
{

    if (header_ClearRipByNetworkEntry(vp, name, length, exact) == FALSE)
        return NULL;

    if (!exact)
    {
        return NULL;
    }

#if 0
    if (!exact)
    {
        memcpy(name, vp->name, ((int) vp->namelen) * sizeof (oid));
        *length = vp->namelen + 7;
        return NULL;
    }
#endif

    switch (vp->magic)
    {
        case RIPROUTECLEARBYNETWORKSTATUS:
            *write_method = write_ripRouteClearByNetworkStatus;
            *var_len = sizeof(long_return);
            long_return = RIP_SNMP_ROUTECLEARBYNETWORK_NOCLEAR;
            return (u_char*) &long_return;
            break;
        default:
            break;
    }
    return NULL;
}


/*Function Name:RIP_Distance_ActiveCheck
 *  Purpose: This function do semantic check before the row become active
 *  Input:
 *         rp_ptr: RIP_Distance pointer.
 *
 *  output:
 *          none
 *  Return: if success return 0 (TRUE), If failure return -1 (FALSE)
 *  Note:
 */
BOOL_T RIP_Distance_ActiveCheck (void *rp_ptr)
{
    RIP_Distance_T *distance_entry;
    distance_entry = (RIP_Distance_T *)rp_ptr;

    if(distance_entry->distance == 0)
        return FALSE;
    else
        return TRUE;

}
static BOOL_T
add_distance(RIP_Distance_T *distance_entry,RIP_SNMP_DistanceTableIndex_T distance_index)
{
/*use index to lookup this table*/
/*if don't find that create this entry */
/*else return FOUND */
/* */
    int i;
    for(i = 0;i < temp_size; i++)
    {
        if((distance_temp_database[i].ip_addr == distance_index.addr)&&(distance_temp_database[i].pfxlen == distance_index.pfx_len))
        {
            if((distance_temp_database[i].used == TRUE))
            {
                distance_temp_database[i].ip_addr = distance_index.addr;
                distance_temp_database[i].pfxlen = distance_index.pfx_len;
                distance_temp_database[i].distance = distance_entry->distance;
                distance_temp_database[i].status = distance_entry->status;
                strncpy(distance_temp_database[i].alist_name,distance_entry->alist_name ,SYS_ADPT_ACL_MAX_NAME_LEN +1);
                return TRUE;
            }
        }
    }
    for(i = 0; i< temp_size; i++)
    {
        if(distance_temp_database[i].used != TRUE)
        {
            distance_temp_database[i].used = TRUE;
            distance_temp_database[i].ip_addr = distance_index.addr;
            distance_temp_database[i].pfxlen = distance_index.pfx_len;
            distance_temp_database[i].distance = distance_entry->distance;
            distance_temp_database[i].status = distance_entry->status;
            strncpy(distance_temp_database[i].alist_name,distance_entry->alist_name ,SYS_ADPT_ACL_MAX_NAME_LEN +1);
            return TRUE;
        }
    }
    return FALSE;
}
static BOOL_T
get_distance(RIP_Distance_T *distance_entry,RIP_SNMP_DistanceTableIndex_T distance_index)
{
    int i;
    for(i = 0;i < temp_size; i++)
    {
        if((distance_temp_database[i].ip_addr == distance_index.addr)&&(distance_temp_database[i].pfxlen == distance_index.pfx_len))
        {
            if(distance_temp_database[i].used == TRUE)
            {
                distance_entry->status = distance_temp_database[i].status;
                distance_entry->distance = distance_temp_database[i].distance;
                strncpy(distance_entry->alist_name,distance_temp_database[i].alist_name,(SYS_ADPT_ACL_MAX_NAME_LEN +1));
                return TRUE;
            }
        }
    }
    return FALSE;
}
static BOOL_T
del_distance(RIP_SNMP_DistanceTableIndex_T distance_index)
{
/*use index to lookup this table*/
/*if don't find that destroy this entry */
/*else return NO_FOUND */
/* */
	int i;

	for(i = 0;i < temp_size; i++)
	{
		if((distance_temp_database[i].ip_addr == distance_index.addr)&&(distance_temp_database[i].pfxlen == distance_index.pfx_len))
        {
            if(distance_temp_database[i].used == TRUE)
            {
                memset(&distance_temp_database[i],0,sizeof(RIP_Distance_snmp_T));
                return TRUE;
            }
        }
	}
	return FALSE;
}

static BOOL_T
RipDistanceRowStatus(RIP_Distance_T *distance_entry,RIP_SNMP_DistanceTableIndex_T distance_index, UI8_T new_status)
{
    UI32_T	state;
    UI32_T	transition;
    RIP_Distance_T local_distance_entry;

    memcpy(&local_distance_entry, distance_entry, sizeof(local_distance_entry));
    /* Preparation for action */
    switch (new_status)
    {
    case L_RSTATUS_NOT_EXIST:
    case L_RSTATUS_NOTREADY:
        /* invalid actions; however, they are valid states */
        /* leave them to state machine for semantic check */
        state = L_RSTATUS_NOT_EXIST;
        break;
    case L_RSTATUS_ACTIVE:
    case L_RSTATUS_NOTINSERVICE:
    case L_RSTATUS_DESTROY:
        /* Valid actions; however, the entry shall be already existed. */
        if(!get_distance(&local_distance_entry, distance_index))
        {
            return FALSE;
        }

        state = distance_entry->status;
        break;
    case L_RSTATUS_SET_OTHER:
        if(!get_distance(&local_distance_entry, distance_index))
        {
            return FALSE;
        }
        state = distance_entry->status;
        break;
    case L_RSTATUS_CREATEANDGO:
        return FALSE;	 /* do not support */

    case L_RSTATUS_CREATEANDWAIT:
    /* Semantic check, the entry shall not be already existed. */
        if(get_distance(&local_distance_entry, distance_index))
        {
            return FALSE;
        }
        #if 0
        if ( new_status != L_RSTATUS_NOT_EXIST && new_status != L_RSTATUS_ALLOCATED )
        {
            return RIP_API_GET_ERROR;
        }
        #endif
        state = L_RSTATUS_NOT_EXIST;/* Yes, it does not exist. */
        /*create a entry*/
        #if 0
        if(!add_distance(distance_entry, distance_index))
        {
            return RIP_API_SET_ERROR;
        }
        #endif
        break;
    default:
    /* Other values are invalid row status. */
        return FALSE;
    } /* end of switch */

    if ( state == L_RSTATUS_ALLOCATED )
        state = L_RSTATUS_NOTREADY;

    transition = L_RSTATUS_Fsm(new_status, &state, RIP_Distance_ActiveCheck, distance_entry);

    switch (transition)
    {
    case L_RSTATUS_NOTEXIST_2_ACTIVE:
    case L_RSTATUS_NOTREADY_2_ACTIVE:
        /* ActiveCheck return TRUE */
        distance_entry->status = state;
        break;

    case L_RSTATUS_ACTIVE_2_ACTIVE:
        distance_entry->status = state;
        break;

    case L_RSTATUS_NOTEXIST_2_NOTREADY:
    case L_RSTATUS_ACTIVE_2_NOTREADY:
    case L_RSTATUS_NOTREADY_2_NOTREADY:
    /* ActiveCheck return FALSE */
        distance_entry->status = state;
        break;
    case L_RSTATUS_ACTIVE_2_NOTEXIST:
    /* may need to check if this entry can be destroyed */
        if(!del_distance(distance_index))
        {
            return FALSE;
        }
        if(NETCFG_PMGR_RIP_DistanceUnset(distance_index.addr, (UI32_T)distance_index.pfx_len)!= NETCFG_TYPE_OK)
        {
            return FALSE;
        }
        return TRUE;

        case L_RSTATUS_NOTREADY_2_NOTEXIST:
        /* destory this entry (free the resource in current red profile) */
        if(!del_distance(distance_index))
        {
            return FALSE;
        }
        if( NETCFG_PMGR_RIP_DistanceUnset(distance_index.addr, (UI32_T)distance_index.pfx_len)!= NETCFG_TYPE_OK)
        {
            return FALSE;
        }
        return TRUE;

    case L_RSTATUS_NOTEXIST_2_NOTEXIST:
    /* do nothing */
        return FALSE;

    case L_RSTATUS_TRANSITION_STATE_ERROR:
    /*"SetRowStatus: Invalid status change.\n"*/
        return FALSE;
    } /* end of switch */

    /* save result in current setting */
    if(!add_distance(distance_entry, distance_index))
    {
        return FALSE;
    }
    if(distance_entry->status == VAL_Status_active)
    {
        if(NETCFG_PMGR_RIP_DistanceSet(distance_entry->distance,distance_index.addr, (UI32_T)distance_index.pfx_len,(char *)distance_entry->alist_name)!= NETCFG_TYPE_OK)
        {
            return FALSE;
        }
    }
    return TRUE;
}

int
write_ripDistanceValue(int action,
                              u_char * var_val,
                              u_char var_val_type,
                              size_t var_val_len,
                              u_char * statP, oid * name, size_t name_len)
{
    long            value;
    UI8_T           row_status;
    UI8_T   addr[PREFIX_STRING_SIZE+1]={0};
    UI32_T inetAddrSize;
    int i;
    UI32_T type;
    RIP_Distance_T distanceEntry;
    RIP_SNMP_DistanceTableIndex_T distance_index;

    memset(&distanceEntry, 0, sizeof(RIP_Distance_T));
    memset(&distance_index, 0, sizeof(distance_index));

    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;

    type = name[oid_name_length];
    inetAddrSize = name[++oid_name_length];
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
    {
        addr[i] = name[++oid_name_length];
    }
    distance_index.pfx_len= name[++oid_name_length];
    IP_LIB_ArraytoUI32(addr, &(distance_index.addr));
    switch (action) {
    case RESERVE1:
        #if 0
        if (name_len!=  RipDistanceEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        #endif
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
        if ((value <MIN_RIP_SNMP_DISTANCE) || (value >MAX_RIP_SNMP_DISTANCE))
        {
            return SNMP_ERR_WRONGVALUE;
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

        BOOL_T rc_bool = FALSE;

        if(!get_distance(&distanceEntry,distance_index))
        {
            return SNMP_ERR_COMMITFAILED;
        }
        distanceEntry.distance = value;
        row_status = L_RSTATUS_SET_OTHER;
        rc_bool = RipDistanceRowStatus(&distanceEntry,distance_index, row_status);

        break;
    }
    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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
write_ripDistanceAliseName(int action,
                              u_char * var_val,
                              u_char var_val_type,
                              size_t var_val_len,
                              u_char * statP, oid * name, size_t name_len)
{
    char value[SYS_ADPT_ACL_MAX_NAME_LEN+1];
    int size;
    UI8_T           row_status;
    RIP_Distance_T distanceEntry;
    RIP_SNMP_DistanceTableIndex_T distance_index;
    UI8_T   addr[PREFIX_STRING_SIZE+1]={0};
    UI32_T inetAddrSize;
    int i;
    UI32_T type;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;
    memset(value, 0x0, (SYS_ADPT_ACL_MAX_NAME_LEN+1));
    memset(&distanceEntry, 0, sizeof(RIP_Distance_T));
    memset(&distance_index, 0 ,sizeof(RIP_SNMP_DistanceTableIndex_T));
    type = name[oid_name_length];
    inetAddrSize = name[++oid_name_length];
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
    {
        addr[i] = name[++oid_name_length];
    }
    distance_index.pfx_len = name[++oid_name_length];
    IP_LIB_ArraytoUI32(addr, &(distance_index.addr));
    switch (action) {
    case RESERVE1:
        #if 0
        if (name_len!=  RipDistanceEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        #endif
        if (var_val_type != ASN_OCTET_STR)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(UI8_T)*MAXRIPDISTANCEALISENAME)
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:

        size  = var_val_len;
        memcpy(value, var_val, size);
        value[size] = 0;
        if (!L_STDLIB_StrIsAsciiPrintWithCount(value,size))
        {
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
        BOOL_T rc_bool = FALSE;
        size  = var_val_len;
        if(!get_distance(&distanceEntry, distance_index))
        {
            return SNMP_ERR_COMMITFAILED;
        }
        /*not support acl*/
        if(size > 0)
            return SNMP_ERR_WRONGVALUE;

            distanceEntry.alist_name[0] = '\0';
        row_status = L_RSTATUS_SET_OTHER;
        rc_bool = RipDistanceRowStatus(&distanceEntry,distance_index, row_status);
    }
        break;
    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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
write_ripDistanceRowStatus(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{

    long    value;
    UI8_T   addr[PREFIX_STRING_SIZE+1]={0};
    int i;
    NETCFG_TYPE_RIP_Distance_T entry;
    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 7;
    memset(&entry,0,sizeof(NETCFG_TYPE_RIP_Distance_T));
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
    {
        addr[i] = name[oid_name_length + RipDistanceInstanceIP_AddrStart + i];
    }
    IP_LIB_ArraytoUI32(addr, &(entry.ip_addr));
    entry.pfxlen = name[oid_name_length + RipDistanceInstanceIP_AddrStart + INET_ADDRESS_IPV4_SIZE];

    switch (action) {
    case RESERVE1:
        if (name_len!= RipDistanceEntry_INSTANCE_LEN+ oid_name_length)
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        if (var_val_type != ASN_INTEGER) {
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        /* 2004/May/24 kelin added. not auto-generated */
        value = *(long *)var_val;
            if ((value <VAL_Status_active) || (value >VAL_Status_destroy))
                return SNMP_ERR_WRONGVALUE;
            break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        value = *(long *)var_val;
        switch(value)
        {
            case VAL_rip2IfConfStatus_active:
            case VAL_rip2IfConfStatus_createAndGo:
            {
               if(NETCFG_PMGR_RIP_GetDistanceTable(&entry)!= NETCFG_TYPE_OK)
                {
                    entry.distance = RIP_DEFAULT_DISTANCE;
                }
                if(NETCFG_PMGR_RIP_DistanceSet(entry.distance, entry.ip_addr, entry.pfxlen, entry.alist_name)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }
                /* for the row_status transition control */
                RIP_Distance_T distanceEntry;
                RIP_SNMP_DistanceTableIndex_T distance_index;
                memset(&distanceEntry, 0, sizeof(RIP_Distance_T));
                memset(&distance_index, 0, sizeof(distance_index));
                distance_index.addr = entry.ip_addr;
                distance_index.pfx_len = entry.pfxlen;
                distanceEntry.distance = entry.distance;
                strncpy(distanceEntry.alist_name, entry.alist_name, sizeof(distanceEntry.alist_name));
                distanceEntry.status = VAL_rip2IfConfStatus_active;
                if(!add_distance(&distanceEntry, distance_index))
                {
                    return FALSE;
                }


            }
                break;
            case VAL_rip2IfConfStatus_destroy:
            {
                if(NETCFG_PMGR_RIP_DistanceUnset(entry.ip_addr, entry.pfxlen)!= NETCFG_TYPE_OK)
                {
                    return SNMP_ERR_COMMITFAILED;
                }

            }
                break;
            case VAL_rip2IfConfStatus_createAndWait:
            {
                /* for the row_status transition control */
                RIP_Distance_T distanceEntry;
                RIP_SNMP_DistanceTableIndex_T distance_index;

                memset(&distanceEntry, 0, sizeof(RIP_Distance_T));
                memset(&distance_index, 0, sizeof(distance_index));
                distance_index.addr = entry.ip_addr;
                distance_index.pfx_len = entry.pfxlen;
                distanceEntry.distance = entry.distance;
                strncpy(distanceEntry.alist_name, entry.alist_name, sizeof(distanceEntry.alist_name));
                distanceEntry.status = VAL_rip2IfConfStatus_createAndWait;
                if(!add_distance(&distanceEntry, distance_index))
                {
                    return FALSE;
                }

                break;
            }
            case VAL_rip2IfConfStatus_notInService:
            case VAL_rip2IfConfStatus_notReady:
            default:
                return SNMP_ERR_WRONGVALUE;
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


 /*Donny.li  ES3628BT-FLF-ZZ-00348*/
static BOOL_T ripDistanceByInetAddrTable_get(int      compc,
                                oid     *compl,
                                NETCFG_TYPE_RIP_Distance_T *entry)
{

    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    int i;

    if (compc !=RipDistanceEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    if (compl[0] != RipLnetNetworkType)
    {
        return FALSE;
    }

    if (compl[1] != INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }

    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->pfxlen = compl[6];
    entry->ip_addr = ipaddr;


    if(NETCFG_PMGR_RIP_GetDistanceTable(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }
    return TRUE;
    /*End of if */
}
static BOOL_T ripDistanceByInetAddrTable_next(int   compc,
                                 oid     *compl,
                                 NETCFG_TYPE_RIP_Distance_T *entry)
{
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    UI32_T ipaddr = 0;
    int i;

    memset(entry,0,sizeof(NETCFG_TYPE_RIP_Distance_T));

    if ((compl[0] == 0) || ((compl[0] == RipLnetNetworkType) && (compl[1] >= 0) && (compl[1] <INET_ADDRESS_IPV4_SIZE)))
    {
        SNMP_MGR_ConvertRemainToZero(RipDistanceInstanceIP_AddrStart,RipDistanceEntry_INSTANCE_LEN, compl);
    }
    else if((compl[0] > RipLnetNetworkType) || compl[1] > INET_ADDRESS_IPV4_SIZE)
    {
        return FALSE;
    }

    for(i = 0;i<INET_ADDRESS_MAX_SIZE;i++)
    {
        addr[i] = compl[i+2];
    }
    IP_LIB_ArraytoUI32(addr, &ipaddr);
    entry->pfxlen = compl[6];
    entry->ip_addr = ipaddr;

    if(NETCFG_PMGR_RIP_GetNextDistanceTable(entry)!= NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME : var_ripDistanceTable
 * PURPOSE:
 *       RIP Distance entry;
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
var_ripDistanceTable(struct variable *v, oid *name, size_t *length,
         int exact, size_t *var_len, WriteMethod **write_method)
{

    NETCFG_TYPE_RIP_Distance_T entry;
    UI32_T compc = 0;
    int i;
    oid compl[RipDistanceEntry_INSTANCE_LEN] = {0};
    oid best_inst[RipDistanceEntry_INSTANCE_LEN];
    UI8_T  addr[INET_ADDRESS_MAX_SIZE]= {0};
    int result;
    int offset = 0;

    SNMP_MGR_RetrieveCompl(v->name, v->namelen, name, *length, &compc,compl, RipDistanceEntry_INSTANCE_LEN);

    switch (v->magic)
    {
        case RIPDISTANCEVALUE:
            *write_method = write_ripDistanceValue;
            break;
        case RIPDISTANCEALISENAME:
            *write_method = write_ripDistanceAliseName;
            break;
        case RIPDISTANCEROWSTATUS:
            *write_method = write_ripDistanceRowStatus;
            break;
        default:
            break;
    }

    if (exact)/*get*/
    {
        if (!ripDistanceByInetAddrTable_get(compc, compl, &entry))
            return NULL;
        result = L_RSTATUS_ACTIVE;
    }
    else/*getnext*/
    {
        if (!ripDistanceByInetAddrTable_next(compc, compl, &entry))
        {
            return NULL;
        }
        result = L_RSTATUS_ACTIVE;
    }


    best_inst[offset] = 1;
    best_inst[++offset] = INET_ADDRESS_IPV4_SIZE;
    IP_LIB_UI32toArray(entry.ip_addr, addr);
    for(i = 0; i < INET_ADDRESS_IPV4_SIZE; i++)
        best_inst[++offset] = addr[i];
    best_inst[++offset] = entry.pfxlen;
    memcpy(name, v->name, v->namelen*sizeof(oid));

    memcpy(name + v->namelen, best_inst, RipDistanceEntry_INSTANCE_LEN*sizeof(oid));
    *length = v->namelen +RipDistanceEntry_INSTANCE_LEN;

    *var_len = sizeof(long_return);

    /*
     * * this is where we do the value assignments for the mib results.
     */

    switch (v->magic)
    {
        case RIPDISTANCEVALUE:
            long_return =  entry.distance;
            return (u_char *) &long_return;
        case RIPDISTANCEALISENAME:
            strncpy((char *)return_buf, entry.alist_name,SYS_ADPT_ACL_MAX_NAME_LEN);
            *var_len = strlen((char *)return_buf);
            return (u_char *) return_buf;
        case RIPDISTANCEROWSTATUS:
            long_return = result;
            return (u_char *) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}
/*Donny.li end  ES3628BT-FLF-ZZ-00348*/

