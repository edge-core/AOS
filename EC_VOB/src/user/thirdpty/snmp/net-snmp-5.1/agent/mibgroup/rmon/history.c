/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

/* We will allocate the memory when a row is created, if it has not enough
 * memory, then a row created will failed.The maximum entries we can created is defined
 * in SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY. This constant is used in the function
 * init_history for initilize the maximum entry of Rmon 's group.
 * All Groups need to memory allocate a RMON_ENTRY_T plus it Ctrl_T. Since the memory is
 * not pre-allocated, we should ensured the system has enough memorys reserved for RMON to
 * satisy the spec required entry.
 *
 * The detail information is as follow:
 * RMON_ENTRY_T = 44
 *
 * RMON_Statisics_Ctrl_T = 588
 *
 * RMON_HISTORY_Data ENTRY_T = 84
 * RMON_History_Ctrl_T = 656
 *
 * RMON_alarm_Ctrl_T = 560
 *
 * RMON_ EVENT_HISTORY_DATA_ENTRY_T = 16
 * RMON_EVENT _Ctrl_T = 60
 *
 * Group Statistics: RMON_ENTRY_T +  RMON_Statisics Ctrl_T+ strlen(owners)  ~= 44+ 588+ 32= 664
 *
 *
 * History Group:    RMON_ENTRY_T + RMON_History_Ctrl_T+ NO_OF_SAMPLES* RMON_HISTORY_Data ENTRY_T+ strlen(owners)
 *                ~= 656 + 44 +   NO_OF_SAMPLES*84 + 32 = 1404
 *
 * (Note: The etherHistory Entry will not malloc until a history log is created, but we should compute the max memory
 *        need when the etherhistory Entry is Full, here assume the no. of samples is 8.)
 *
 *
 * Alarm Group:   RMON_ENTRY_T + RMON_alarm  Ctrl_T + strlen(owners) ~= 44 +560 + 32 = 636
 *
 *
 * Event Group:   RMON_ENTRY_T +  RMON event _Ctrl_T + NO_OF_LOG_ENTRIES* ( RMON_ EVENT_HISTORY_DATA_ENTRY_T +
 *               strlen(log_description)) + strlen(owners)+ strlen(community)+ strlen(event_description)
 *
 *              ~= 44 + 60 + 8*(16+314) + 32+32+32 = 104+ 20+ 8*330+96= 2860
 * kinghong, 2003, 6/9
 */

#include "netsnmp_port.h"
#ifndef VXWORKS
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#else
/* system dependent header, added by kinghong*/
#include "l_mm.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#endif /*endof #ifndef VXWORKS*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"

#include "history.h"

/*
 * Implementation headers
 */
#include "agutil.h"
#include "agutil_api.h"
#include "row_api.h"
#include "snmp_mgr.h"
#include "rows.h"
#include "vlan_om.h"
#include "vlan_pom.h"
#include "vlan_lib.h"
/*
 * File scope definitions section
 */

#define historyControlEntryFirstIndexBegin      11


/*
 * defaults & limitations
 */

#define MAX_BUCKETS_IN_CRTL_ENTRY	50
#define HIST_DEF_BUCK_REQ		50
#define HIST_DEF_INTERVAL		1800

#define HISTORY_DEFAULT_DATA_SOURCE_WITH_KEY_ARR        {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 1}
#define HISTORY_DEFAULT_DATA_SOURCE_WITHOUT_KEY_STR     "1.3.6.1.2.1.2.2.1.1"
#define HISTORY_DEFAULT_OWNER                           ""

static VAR_OID_T DEFAULT_DATA_SOURCE = { 11, HISTORY_DEFAULT_DATA_SOURCE_WITH_KEY_ARR };
static int dflt_create_ar[SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY+1];

#ifndef VXWORKS
typedef struct data_struct_t {
    struct data_struct_t *next;
    u_long          data_index;
    u_long          start_interval;
    u_long          utilization;
    ETH_STATS_T     EthData;
} DATA_ENTRY_T;

typedef struct {
    u_long          interval;
    u_long          timer_id;
    VAR_OID_T       data_source;

    u_long          coeff;
    DATA_ENTRY_T    previous_bucket;
    SCROLLER_T      scrlr;

} CRTL_ENTRY_T;
#endif
static TABLE_DEFINTION_T HistoryCtrlTable;
static TABLE_DEFINTION_T *table_ptr = &HistoryCtrlTable;

static void get_history_control_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
static BOOL_T getnext_history_control_table_by_lport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
static BOOL_T create_history_control_entry(UI32_T id, UI32_T if_index, UI32_T buckets_requested, UI32_T interval, char *owner_p, UI32_T status);
static NEXTED_PTR_T *getnext_history_entry(RMON_ENTRY_T *rmon_p, UI32_T data_index);
static void get_rmon_history_table(UI32_T control_index, NEXTED_PTR_T *src_entry_p, SNMP_MGR_RmonHistoryEntry_T *dst_entry_p);

/*
 * Main section
 */

#  define Leaf_historyControlDataSource                    2
#  define Leaf_historyControlBucketsRequested              3
#  define Leaf_historyControlInterval                      5
#  define Leaf_historyControlOwner                         6
#  define Leaf_historyControlStatus                        7

static int
write_historyControl(int action, u_char * var_val, u_char var_val_type,
                     size_t var_val_len, u_char * statP,
                     oid * name, size_t name_len)
{
    long            long_temp;
    int             leaf_id, snmp_status;
    static int      prev_action = COMMIT;
    RMON_ENTRY_T   *hdr;
#ifndef VXWORKS
    CRTL_ENTRY_T   *cloned_body;
    CRTL_ENTRY_T   *body;
#else
    HISTORY_CRTL_ENTRY_T   *cloned_body;
    HISTORY_CRTL_ENTRY_T   *body;
#endif

    /* kinghong add the following sematic check for setting Rmon EntryStatus.
     * 1. Set Valid(1)              Permit original state:  Valid(1),UnderCreation(3).
     * 2. Set CreateRequest(2)      Permit original state:  Not Exist.
     * 3. Set UnderCreation(3)      Permit original state:  Valid(1), UnderCreation(3).
     * 4. Set Invalid(4)            Permit original state:  Valid(1), UnderCreation(3).
     */
    if (name[historyControlEntryFirstIndexBegin - 1] == Leaf_historyControlStatus)
    {
         /* This is the 'set' value from MIB*/
        memcpy(&long_temp, var_val, sizeof(long));

        /* Step 1: Check current Status before do anything*/
        hdr = ROWAPI_find(table_ptr, name[historyControlEntryFirstIndexBegin]);

        /*Step 2: Check if the data exist*/
        if (!hdr) /*: not exist, only can perform create request */
        {
            if (long_temp != RMON1_ENTRY_CREATE_REQUEST )
            {
                return SNMP_ERR_BADVALUE;
            }

        }
        /*:already exist, block any request that want to set create request.
         */
        else if ((long_temp == RMON1_ENTRY_CREATE_REQUEST) && (action == 0))
        {
            return SNMP_ERR_BADVALUE;
        }
    }

    switch (action) {
    case RESERVE1:
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
        #ifndef VXWORKS
        return ROWAPI_do_another_action(name,
                                        historyControlEntryFirstIndexBegin,
                                        action, &prev_action, table_ptr,
                                        sizeof(CRTL_ENTRY_T));
        #else
        return ROWAPI_do_another_action(name,
                                        historyControlEntryFirstIndexBegin,
                                        action, &prev_action, table_ptr,
                                        sizeof(HISTORY_CRTL_ENTRY_T));
        #endif
    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_temp = name[historyControlEntryFirstIndexBegin];
        leaf_id = (int) name[historyControlEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(table_ptr, long_temp);        /* it MUST be OK */
        #ifndef VXWORKS
        cloned_body = (CRTL_ENTRY_T *) hdr->tmp;
        body = (CRTL_ENTRY_T *) hdr->body;
        if( (hdr->status == 1) &&( leaf_id!=Leaf_historyControlStatus))
               return SNMP_ERR_BADVALUE;
        #else
        cloned_body = (HISTORY_CRTL_ENTRY_T *) hdr->tmp;
        body = (HISTORY_CRTL_ENTRY_T *) hdr->body;
        #endif
        switch (leaf_id) {
        case Leaf_historyControlDataSource:
               {
             UI32_T vid;
              oid   *oid_var;
            oid_var = (oid *) var_val;
            VLAN_OM_ConvertFromIfindex(oid_var[var_val_len/4-1], &vid);
            if (var_val_type != ASN_OBJECT_ID)
                return SNMP_ERR_WRONGTYPE;
            if (!((SWCTRL_POM_LogicalPortExisting(oid_var[var_val_len/4-1]))||(VLAN_POM_IsVlanExisted(vid))))
                 return SNMP_ERR_BADVALUE;
            snmp_status = AGUTIL_get_oid_value(var_val, var_val_type,
                                               var_val_len,
                                               &cloned_body->data_source);
            if (SNMP_ERR_NOERROR != snmp_status) {
                ag_trace("can't browse historyControlDataSource");
                return snmp_status;
            }
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                snmp_oid_compare(cloned_body->data_source.objid,
                                 cloned_body->data_source.length,
                                 body->data_source.objid,
                                 body->data_source.length)) {
                ag_trace
                    ("can't change historyControlDataSource - not Creation");
                return SNMP_ERR_BADVALUE;
            }
            }
            break;
        case Leaf_historyControlBucketsRequested:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               MIN_historyControlBucketsRequested,
                                               MAX_historyControlBucketsRequested,
                                               (long *)&cloned_body->scrlr.data_requested);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
#if 0
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                cloned_body->scrlr.data_requested !=
                body->scrlr.data_requested)
                return SNMP_ERR_BADVALUE;
#endif
            break;
        case Leaf_historyControlInterval:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               MIN_historyControlInterval,
                                               MAX_historyControlInterval,
                                               (long *)&cloned_body->interval);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
#if 0
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                cloned_body->interval != body->interval)
                return SNMP_ERR_BADVALUE;
#endif
            break;
        case Leaf_historyControlOwner:
            if (hdr->new_owner)
                AGFREE(hdr->new_owner);
            hdr->new_owner = AGMALLOC(MAX_OWNERSTRING + 1);
            if (!hdr->new_owner)
                return SNMP_ERR_TOOBIG;
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_OWNERSTRING,
                                                  1, NULL, hdr->new_owner);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }

            break;
        case Leaf_historyControlStatus:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               RMON1_ENTRY_VALID,
                                               RMON1_ENTRY_INVALID,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            hdr->new_status = long_temp;
            break;
        default:
            ag_trace("%s:unknown leaf_id=%d\n", table_ptr->name,
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;

    }                           /* of switch by actions */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}

/*
 * var_historyControlTable():
 */
unsigned char  *
var_historyControlTable(struct variable *vp,
                        oid * name,
                        size_t * length,
                        int exact,
                        size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    #ifndef VXWORKS
    static CRTL_ENTRY_T theEntry;
    #else
    static HISTORY_CRTL_ENTRY_T theEntry;
    #endif
    RMON_ENTRY_T   *hdr;
    switch(vp->magic)
    {
        case CTRL_DATASOURCE:
        case CTRL_BUCKETSREQUESTED:
        case CTRL_INTERVAL:
        case CTRL_OWNER:
        case CTRL_STATUS:
            *write_method = write_historyControl;
            break;
        default:
            *write_method = 0;
            break;
    }

    #ifndef VXWORKS
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(CRTL_ENTRY_T));
    #else
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(HISTORY_CRTL_ENTRY_T));
    #endif
    if (!hdr)
        return NULL;

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case CTRL_INDEX:
        long_ret = hdr->ctrl_index;
        return (unsigned char *) &long_ret;

    case CTRL_DATASOURCE:
        *var_len = sizeof(oid) * theEntry.data_source.length;
        return (unsigned char *) theEntry.data_source.objid;

    case CTRL_BUCKETSREQUESTED:
        long_ret = theEntry.scrlr.data_requested;
        return (unsigned char *) &long_ret;

    case CTRL_BUCKETSGRANTED:

        long_ret = theEntry.scrlr.data_granted;
        return (unsigned char *) &long_ret;

    case CTRL_INTERVAL:
        long_ret = theEntry.interval;
        return (unsigned char *) &long_ret;

    case CTRL_OWNER:
        if (hdr->owner) {
            *var_len = strlen(hdr->owner);
            return (unsigned char *) hdr->owner;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }

    case CTRL_STATUS:
        long_ret = hdr->status;
        return (unsigned char *) &long_ret;

    default:
        ag_trace("HistoryControlTable: unknown vp->magic=%d",
                 (int) vp->magic);
        ERROR_MSG("");
    }
    return NULL;
}

/*
 * history row management control callbacks
 */

static void
compute_delta(ETH_STATS_T * delta,
              ETH_STATS_T * newval, ETH_STATS_T * prevval)
{
#define CNT_DIF(X) delta->X = newval->X - prevval->X

    CNT_DIF(octets);
    /*EPR_ID:ES3628BT-FLF-ZZ-00180,ES4827G-FLF-ZZ-00484
    Problem: NetworkMonitor:Node "etherHistoryUtilization" return worng value while the port works at 1000full mode.
    Root Cause: ETH_STATS_T->octets was defined as the 32-bit.The value will overfolw while these ports work at
                1000full mode.
    Solution: add new unsigned long long value for 1000full mode.
    Modified files:
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\history.c
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil_api.h
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil.c
    Modified by: Shumin.Wang
    Approved by: Tiger Liu
    */
    CNT_DIF(octets_64);
    CNT_DIF(packets);
    CNT_DIF(bcast_pkts);
    CNT_DIF(mcast_pkts);
    CNT_DIF(crc_align);
    CNT_DIF(undersize);
    CNT_DIF(oversize);
    CNT_DIF(fragments);
    CNT_DIF(jabbers);
    CNT_DIF(collisions);
}


static void
compute_delta_hc(ETH_STATS_HC_T * delta,
                 ETH_STATS_HC_T * newval, ETH_STATS_HC_T * prevval)
{
#define CNT_DIF(X) delta->X = newval->X - prevval->X

    CNT_DIF(hc_overflow_pkts);
    CNT_DIF(hc_pkts);
    CNT_DIF(hc_overflow_octets);
    CNT_DIF(hc_octets);
}


/*
 * bittimes = 64+ (8 * octets) + 96 + (32*collisions)
 *              = SFDpreamble + packet + interpacket gap + collisionExtra if any
 *
 *                              bittimes
 * util = 100 * 100 * ---------------------------
 *                              seconds * 10000000
 *
 *                 bittimes             8 * (8 + octets + 12 + (4 * collisions)
 *              = ----------------  = ----------------------------------------
 *                 seconds * 1000       seconds * 1000
 *
 *                 (20 * total_packet) + interval_total_octets  + (4 * collisions)
 *              = -----------------------------------------------------------------
 *                  seconds * constants
 *
 * The situation below may look confusing but we must consider the condition
 * where the numbers wrap after addition or multiplication occurs.
 *
 */
static void
history_get_backet(unsigned int clientreg, void *clientarg)
{
    RMON_ENTRY_T   *hdr_ptr;
    #ifdef VXWORKS
    HISTORY_CRTL_ENTRY_T   *body;
    HISTORY_DATA_ENTRY_T   *bptr;
    #else
    CRTL_ENTRY_T   *body;
    DATA_ENTRY_T   *bptr;
    #endif
    ETH_STATS_T     newSample;
    ETH_STATS_HC_T  newSample_hc;

    Port_Info_T     speedDuplexStatus;
    UI32_T          constants;
    UI32_T          nIndex  = 0;
    UI32_T          nPortNo = 0;
    UI64_T          uti     = 0;
    UI64_T          num     = 0;
    /*
     * ag_trace ("history_get_backet: timer_id=%d", (int) clientreg);
     */
    hdr_ptr = (RMON_ENTRY_T *) clientarg;

    if (!hdr_ptr)
    {
        ag_trace("Err: history_get_backet: hdr_ptr=NULL ? (Inserted in shock)");
        return;
    }

    #ifndef VXWORKS
    body = (CRTL_ENTRY_T *) hdr_ptr->body;
    #else
    body = (HISTORY_CRTL_ENTRY_T *) hdr_ptr->body;
    #endif

    if (!body)
    {
        ag_trace("Err: history_get_backet: body=NULL ? (Inserted in shock)");
        return;
    }

    if (RMON1_ENTRY_VALID != hdr_ptr->status)
    {
        ag_trace("Err: history_get_backet when entry %d is not valid ?!!", (int) hdr_ptr->ctrl_index);
        /*
         * snmp_alarm_print_list ();
         */
        snmp_alarm_unregister(body->timer_id);
        ag_trace("Err: unregistered %ld", (long) body->timer_id);
        return;
    }

    SYSTEM_get_eth_statistics(&body->data_source, &newSample);
    SYSTEM_get_eth_statistics_HC(&body->data_source, &newSample_hc);

    bptr = ROWDATAAPI_locate_new_data(&body->scrlr);
    if (!bptr)
    {
        ag_trace("Err: history_get_backet for %d: empty bucket's list !??\n", (int) hdr_ptr->ctrl_index);
        return;
    }

    bptr->data_index = ROWDATAAPI_get_total_number(&body->scrlr);
    bptr->start_interval = body->previous_bucket.start_interval;

    compute_delta(&bptr->EthData, &newSample, &body->previous_bucket.EthData);
    compute_delta_hc(&bptr->EthData_HC, &newSample_hc, &body->previous_bucket.EthData_HC);

    nIndex  = body->data_source.length;
    nPortNo = body->data_source.objid[nIndex-1];

    memset(&speedDuplexStatus, 0, sizeof(speedDuplexStatus));
    SWCTRL_POM_GetPortInfo( nPortNo, &speedDuplexStatus);

    switch(speedDuplexStatus.speed_duplex_oper)
    {
        case VAL_portSpeedDpxCfg_halfDuplex10:
             constants = 125; /* default 10MBPS segment */
             break;

        case VAL_portSpeedDpxCfg_fullDuplex10:
             constants = 250;
             break;

        case VAL_portSpeedDpxCfg_halfDuplex100:
             constants = 1250; /* for 100MBPS segment */
             break;

        case VAL_portSpeedDpxCfg_fullDuplex100:
             constants = 2500;
             break;

        case VAL_portSpeedDpxCfg_halfDuplex1000:
             constants = 12500; /* for 1GMBPS segment */
             break;

        case VAL_portSpeedDpxCfg_fullDuplex1000:
             constants = 25000;
             break;

        default:
             constants = 125; /* default 10MBPS segment */
    }
    /*EPR_ID:ES3628BT-FLF-ZZ-00180,ES4827G-FLF-ZZ-00484
    Problem: NetworkMonitor:Node "etherHistoryUtilization" return worng value while the port works at 1000full mode.
    Root Cause: ETH_STATS_T->octets was defined as the 32-bit.The value will overfolw while these ports work at
                1000full mode.
    Solution: add new unsigned long long value for 1000full mode.
    Modified files:
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\history.c
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil_api.h
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil.c
    Modified by: Shumin.Wang
    Approved by: Tiger Liu
    */
    num = 20 * (UI64_T)(bptr->EthData.packets) + bptr->EthData.octets_64 + (4 * (UI64_T)(bptr->EthData.collisions));

    if ( (num > bptr->EthData.octets_64) &&
         (num > (UI64_T)(bptr->EthData.collisions)))
    {
        uti = num / (UI64_T)(body->interval * constants);
    }
    else
    {
        uti  = (bptr->EthData.octets_64) / (UI64_T)(body->interval * constants);

        /* Mulitply by 4 and see if it would wrap */
        if ((bptr->EthData.collisions << 2) >= bptr->EthData.collisions)
        {
            /* No, everyting is ok, just divide the answer */
            uti += (bptr->EthData.collisions << 2) / (body->interval * constants);
        }
        else
        {
            /* Yes, in this case, do the division and then the multiplication */
            uti += ((bptr->EthData.collisions / (body->interval * constants)) << 2);
        }
    }

    //wuli, 10-20-98, to make util less than 100%
    if ( uti > 10000)
        uti = 10000; //wuli

    bptr->utilization = (UI32_T)uti;

    /*
     * update previous_bucket
     */
    body->previous_bucket.start_interval = AGUTIL_sys_up_time();
    memcpy(&body->previous_bucket.EthData, &newSample, sizeof(ETH_STATS_T));
    memcpy(&body->previous_bucket.EthData_HC, &newSample_hc, sizeof(ETH_STATS_HC_T));
}

/*
 * Control Table RowApi Callbacks
 */

int
history_Create(RMON_ENTRY_T * eptr)
{                               /* create the body: alloc it and set defaults */
    #ifndef VXWORKS
    CRTL_ENTRY_T   *body;
    eptr->body = AGMALLOC(sizeof(CRTL_ENTRY_T));
    #else
    HISTORY_CRTL_ENTRY_T   *body;
    eptr->body = AGMALLOC(sizeof(HISTORY_CRTL_ENTRY_T));
    #endif


    if (!eptr->body)
        return -3;
    #ifndef VXWORKS
    body = (CRTL_ENTRY_T *) eptr->body;
    #else
    body = (HISTORY_CRTL_ENTRY_T *) eptr->body;
    #endif
    /*
     * set defaults
     */
    body->interval = HIST_DEF_INTERVAL;
    body->timer_id = 0;
    memcpy(&body->data_source, &DEFAULT_DATA_SOURCE, sizeof(VAR_OID_T));

    #ifndef VXWORKS
    ROWDATAAPI_init(&body->scrlr, HIST_DEF_BUCK_REQ,
                    MAX_BUCKETS_IN_CRTL_ENTRY, sizeof(DATA_ENTRY_T), NULL);
    #else
      ROWDATAAPI_init(&body->scrlr, SYS_DFLT_RMON_HISTORY_BUCKETS_REQUESTED,
                    SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_LOG_ENTRY, sizeof(HISTORY_DATA_ENTRY_T), NULL);
    #endif

    return 0;
}

int
history_Validate(RMON_ENTRY_T * eptr)
{
    /*
     * T.B.D. (system dependent) check valid inteface in body->data_source;
     */
    return 0;
}

int
history_Activate(RMON_ENTRY_T * eptr)
{
    #ifndef VXWORKS
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    #else
    HISTORY_CRTL_ENTRY_T   *body = (HISTORY_CRTL_ENTRY_T *) eptr->body;
    #endif
    body->coeff = 100000L * (long) body->interval;

    ROWDATAAPI_set_size(&body->scrlr,
                        body->scrlr.data_requested,
                        RMON1_ENTRY_VALID == eptr->status);

    SYSTEM_get_eth_statistics(&body->data_source,
                              &body->previous_bucket.EthData);
    SYSTEM_get_eth_statistics_HC(&body->data_source, &body->previous_bucket.EthData_HC);  /* Jenny */

    body->previous_bucket.start_interval = AGUTIL_sys_up_time();

    body->scrlr.current_data_ptr = body->scrlr.first_data_ptr;
    /*
     * ag_trace ("Dbg:   registered in history_Activate");
     */
    body->timer_id = snmp_alarm_register(body->interval, SA_REPEAT,
                                         history_get_backet, eptr);
    return 0;
}

int
history_Deactivate(RMON_ENTRY_T * eptr)
{
    #ifndef VXWORKS
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    #else
    HISTORY_CRTL_ENTRY_T   *body = (HISTORY_CRTL_ENTRY_T *) eptr->body;
    #endif
    snmp_alarm_unregister(body->timer_id);
    /*
     * ag_trace ("Dbg: unregistered in history_Deactivate timer_id=%d",
     * (int) body->timer_id);
     */

    /*
     * free data list
     */
    ROWDATAAPI_descructor(&body->scrlr);

    return 0;
}

int
history_Copy(RMON_ENTRY_T * eptr)
{
    #ifndef VXWORKS
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    CRTL_ENTRY_T   *clone = (CRTL_ENTRY_T *) eptr->tmp;
    #else
    HISTORY_CRTL_ENTRY_T   *body = (HISTORY_CRTL_ENTRY_T *) eptr->body;
    HISTORY_CRTL_ENTRY_T   *clone = (HISTORY_CRTL_ENTRY_T *) eptr->tmp;
    #endif
    if (body->scrlr.data_requested != clone->scrlr.data_requested) {
        ROWDATAAPI_set_size(&body->scrlr, clone->scrlr.data_requested,
                            RMON1_ENTRY_VALID == eptr->status);
    }

    if (body->interval != clone->interval) {
        if (RMON1_ENTRY_VALID == eptr->status) {
            snmp_alarm_unregister(body->timer_id);
            body->timer_id =
                snmp_alarm_register(clone->interval, SA_REPEAT,
                                    history_get_backet, eptr);
        }

        body->interval = clone->interval;
    }

    if (snmp_oid_compare
        (clone->data_source.objid, clone->data_source.length,
         body->data_source.objid, body->data_source.length)) {
        memcpy(&body->data_source, &clone->data_source, sizeof(VAR_OID_T));
    }

    return 0;
}

static SCROLLER_T *
history_extract_scroller(void *v_body)
{
    #ifndef VXWORKS
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) v_body;
    #else
    HISTORY_CRTL_ENTRY_T   *body = (HISTORY_CRTL_ENTRY_T *) v_body;
    #endif
    return &body->scrlr;
}

/*
 * var_etherHistoryTable():
 */
unsigned char  *
var_etherHistoryTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    #ifndef VXWORKS
    static DATA_ENTRY_T theBucket;
    RMON_ENTRY_T   *hdr;
    CRTL_ENTRY_T   *ctrl;
    #else
    static HISTORY_DATA_ENTRY_T theBucket;
    RMON_ENTRY_T   *hdr;
    HISTORY_CRTL_ENTRY_T   *ctrl;
    #endif

    *write_method = NULL;
    #ifndef VXWORKS
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      table_ptr,
                                      &history_extract_scroller,
                                      sizeof(DATA_ENTRY_T), &theBucket);
    #else
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      table_ptr,
                                      &history_extract_scroller,
                                      sizeof(HISTORY_DATA_ENTRY_T), &theBucket);
    #endif
    if (!hdr)
        return NULL;

    *var_len = sizeof(long);    /* default */
    #ifndef VXWORKS
    ctrl = (CRTL_ENTRY_T *) hdr->body;
    #else
    ctrl = (HISTORY_CRTL_ENTRY_T *) hdr->body;
    #endif

    switch (vp->magic) {
    case DATA_INDEX:
        long_ret = hdr->ctrl_index;
        return (unsigned char *) &long_ret;
    case DATA_SAMPLEINDEX:
        long_ret = theBucket.data_index;
        return (unsigned char *) &long_ret;
    case DATA_INTERVALSTART:
        long_ret = 0;
        return (unsigned char *) &theBucket.start_interval;
    case DATA_DROPEVENTS:
        long_ret = 0;
        return (unsigned char *) &long_ret;
    case DATA_OCTETS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.octets;
    case DATA_PKTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.packets;
    case DATA_BROADCASTPKTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.bcast_pkts;
    case DATA_MULTICASTPKTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.mcast_pkts;
    case DATA_CRCALIGNERRORS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.crc_align;
    case DATA_UNDERSIZEPKTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.undersize;
    case DATA_OVERSIZEPKTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.oversize;
    case DATA_FRAGMENTS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.fragments;
    case DATA_JABBERS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.jabbers;
    case DATA_COLLISIONS:
        long_ret = 0;
        return (unsigned char *) &theBucket.EthData.collisions;
    case DATA_UTILIZATION:
        long_ret = 0;
        return (unsigned char *) &theBucket.utilization;
    default:
        ag_trace("etherHistoryTable: unknown vp->magic=%d",
                 (int) vp->magic);
        ERROR_MSG("");
    }
    return NULL;
}

#if 1                           /* debug, but may be used for init. TBD: may be token snmpd.conf ? */
int
add_hist_entry(int ctrl_index, int ifIndex,
               u_long interval, u_long requested)
{
    register RMON_ENTRY_T *eptr;
    #ifndef VXWORKS
    register CRTL_ENTRY_T *body;
    #else
    register HISTORY_CRTL_ENTRY_T *body;
    #endif
    int             ierr;

    ierr = ROWAPI_new(table_ptr, ctrl_index);
    if (ierr) {
        ag_trace("ROWAPI_new failed with %d", ierr);
        return ierr;
    }

    eptr = ROWAPI_find(table_ptr, ctrl_index);
    if (!eptr) {
        ag_trace("ROWAPI_find failed");
        return -4;
    }
    #ifndef VXWORKS
    body = (CRTL_ENTRY_T *) eptr->body;
    #else
    body = (HISTORY_CRTL_ENTRY_T *) eptr->body;
    #endif

    /*
     * set parameters
     */

    body->data_source.objid[body->data_source.length - 1] = ifIndex;
    body->interval = interval;
    body->scrlr.data_requested = requested;

    eptr->new_status = RMON1_ENTRY_VALID;
    ierr = ROWAPI_commit(table_ptr, ctrl_index);
    if (ierr) {
        ag_trace("ROWAPI_commit failed with %d", ierr);
    }

    return ierr;

}

#endif

/*
 * Registration & Initializatio section
 */

oid             historyControlTable_variables_oid[] =
    { 1, 3, 6, 1, 2, 1, 16, 2, 1 };

struct variable2 historyControlTable_variables[] = {
    /*
     * magic number        , variable type, ro/rw , callback fn  ,           L, oidsuffix
     */
    {CTRL_INDEX, ASN_INTEGER, RONLY, var_historyControlTable, 2, {1, 1}},
    {CTRL_DATASOURCE, ASN_OBJECT_ID, RWRITE, var_historyControlTable, 2,
     {1, 2}},
    {CTRL_BUCKETSREQUESTED, ASN_INTEGER, RWRITE, var_historyControlTable,
     2, {1, 3}},
    {CTRL_BUCKETSGRANTED, ASN_INTEGER, RONLY, var_historyControlTable, 2,
     {1, 4}},
    {CTRL_INTERVAL, ASN_INTEGER, RWRITE, var_historyControlTable, 2,
     {1, 5}},
    {CTRL_OWNER, ASN_OCTET_STR, RWRITE, var_historyControlTable, 2,
     {1, 6}},
    {CTRL_STATUS, ASN_INTEGER, RWRITE, var_historyControlTable, 2, {1, 7}},

};

oid             etherHistoryTable_variables_oid[] =
    { 1, 3, 6, 1, 2, 1, 16, 2, 2 };

struct variable2 etherHistoryTable_variables[] = {
    /*
     * magic number     , variable type , ro/rw , callback fn  ,        L, oidsuffix
     */
    {DATA_INDEX, ASN_INTEGER, RONLY, var_etherHistoryTable, 2, {1, 1}},
    {DATA_SAMPLEINDEX, ASN_INTEGER, RONLY, var_etherHistoryTable, 2,
     {1, 2}},
    {DATA_INTERVALSTART, ASN_TIMETICKS, RONLY, var_etherHistoryTable, 2,
     {1, 3}},
    {DATA_DROPEVENTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 4}},
    {DATA_OCTETS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2, {1, 5}},
    {DATA_PKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2, {1, 6}},
    {DATA_BROADCASTPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 7}},
    {DATA_MULTICASTPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 8}},
    {DATA_CRCALIGNERRORS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 9}},
    {DATA_UNDERSIZEPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 10}},
    {DATA_OVERSIZEPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 11}},
    {DATA_FRAGMENTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 12}},
    {DATA_JABBERS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2, {1, 13}},
    {DATA_COLLISIONS, ASN_COUNTER, RONLY, var_etherHistoryTable, 2,
     {1, 14}},
    {DATA_UTILIZATION, ASN_INTEGER, RONLY, var_etherHistoryTable, 2,
     {1, 15}},

};

void
init_history(void)
{
    REGISTER_MIB("historyControlTable", historyControlTable_variables,
                 variable2, historyControlTable_variables_oid);
    REGISTER_MIB("etherHistoryTable", etherHistoryTable_variables,
                 variable2, etherHistoryTable_variables_oid);
    #ifdef VXWORKS
    ROWAPI_init_table(&HistoryCtrlTable, "History", SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_ENTRY, &history_Create, NULL,   /* &history_Clone, */
                      NULL,     /* &history_Delete, */
                      &history_Validate,
                      &history_Activate,
                      &history_Deactivate, &history_Copy);
    #else
    ROWAPI_init_table(&HistoryCtrlTable, "History", 0, &history_Create, NULL,   /* &history_Clone, */
                      NULL,     /* &history_Delete, */
                      &history_Validate,
                      &history_Activate,
                      &history_Deactivate, &history_Copy);
    #endif
    /*
     * add_hist_entry (2, 3, 4, 2);
     */
}


oid etherHistoryHighCapacityTable_variables_oid[] = {1, 3, 6, 1, 2, 1, 16, 2};

struct variable3 etherHistoryHighCapacityTable_variables[] =
{
    {ETHERHISTORYHIGHCAPACITYOVERFLOWPKTS, ASN_GAUGE, RONLY, var_etherHistoryHighCapacityTable, 3, {6, 1, 1}},
    {ETHERHISTORYHIGHCAPACITYPKTS, ASN_COUNTER64, RONLY, var_etherHistoryHighCapacityTable, 3, {6, 1, 2}},
    {ETHERHISTORYHIGHCAPACITYOVERFLOWOCTETS, ASN_GAUGE, RONLY, var_etherHistoryHighCapacityTable, 3, {6, 1, 3}},
    {ETHERHISTORYHIGHCAPACITYOCTETS, ASN_COUNTER64, RONLY, var_etherHistoryHighCapacityTable, 3, {6, 1, 4}},
};


void init_etherHistoryHighCapacityTable(void)
{
    REGISTER_MIB("etherHistoryHighCapacityTable",
                 etherHistoryHighCapacityTable_variables, variable3,
                 etherHistoryHighCapacityTable_variables_oid);
}


unsigned char  *
var_etherHistoryHighCapacityTable(struct variable *vp,
                                  oid * name,
                                  size_t * length,
                                  int exact,
                                  size_t * var_len,
                                  WriteMethod ** write_method)
{
    static long long_ret;
#ifndef VXWORKS
    static DATA_ENTRY_T theBucket;
    RMON_ENTRY_T *hdr;
    CRTL_ENTRY_T *ctrl;
#else
    static HISTORY_DATA_ENTRY_T theBucket;
    RMON_ENTRY_T *hdr;
    HISTORY_CRTL_ENTRY_T *ctrl;
#endif

    *write_method = 0;

#ifndef VXWORKS
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      table_ptr,
                                      &history_extract_scroller,
                                      sizeof(DATA_ENTRY_T), &theBucket);
#else
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      table_ptr,
                                      &history_extract_scroller,
                                      sizeof(HISTORY_DATA_ENTRY_T), &theBucket);
#endif

    if(!hdr)
        return NULL;

    *var_len = sizeof(long_ret);

#ifndef VXWORKS
    ctrl = (CRTL_ENTRY_T *) hdr->body;
#else
    ctrl = (HISTORY_CRTL_ENTRY_T *) hdr->body;
#endif

    switch (vp->magic)
    {
        case ETHERHISTORYHIGHCAPACITYOVERFLOWPKTS:
            long_ret = 0;
            return (unsigned char *) &theBucket.EthData_HC.hc_overflow_pkts;
        case ETHERHISTORYHIGHCAPACITYPKTS:
            *var_len = sizeof(long64_return);
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, theBucket.EthData_HC.hc_pkts);
            return (unsigned char *) &long64_return;
        case ETHERHISTORYHIGHCAPACITYOVERFLOWOCTETS:
            long_ret = 0;
            return (unsigned char *) &theBucket.EthData_HC.hc_overflow_octets;
        case ETHERHISTORYHIGHCAPACITYOCTETS:
            *var_len = sizeof(long64_return);
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, theBucket.EthData_HC.hc_octets);
            return (unsigned char *) &long64_return;
        default:
            ERROR_MSG("");
    }

    return NULL;
}


#ifdef VXWORKS
/* add by kinghong to create default*/
void HISTORY_CreateDefaultEntry()
{
    int i;

    memset(dflt_create_ar, 0, sizeof(dflt_create_ar));

    for (i= 0; i < SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY; i++)
    {
        if (SWCTRL_POM_LogicalPortExisting(i/2+1) )
	{
            if (i%2)
            {
                add_hist_entry (  (i+1), (i/2+1), SYS_DFLT_RMON_HISTORY_INTERVAL_1, SYS_DFLT_RMON_HISTORY_BUCKETS_REQUESTED);
	    }
	    else
	    {
	    	add_hist_entry( (i+1), (i/2+1), SYS_DFLT_RMON_HISTORY_INTERVAL_2, SYS_DFLT_RMON_HISTORY_BUCKETS_REQUESTED);
	    }

            /* flag to record creation entries
             * id starts from 1
             */
            dflt_create_ar[i+1] = 1;
	}

    }
}


void HISTORY_DeleteAllRow()
{
	register RMON_ENTRY_T *eptr;
	int i;

	for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_ENTRY; i++)
	{
	   eptr = ROWAPI_find(table_ptr, i);
	   if (eptr)
	   {
  	       ROWAPI_delete_clone(table_ptr, i);
               rowapi_delete(eptr);
           }
        }
}

/* kinghong added the below four API for RMON 3com get history*/

/* added by kinghong to get the first history control Entry*/
BOOL_T rmon_HiscEntryGetFirst( int *index, HISTORY_CRTL_ENTRY_T *entry)
{
    RMON_ENTRY_T *row_entry_ptr= NULL;

    row_entry_ptr= ROWAPI_first(table_ptr);

    if (row_entry_ptr)
    {
        memcpy(entry, row_entry_ptr->body, sizeof(HISTORY_CRTL_ENTRY_T));
        *index = row_entry_ptr->ctrl_index;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/* added by kinghong to get the next history control Entry*/
BOOL_T rmon_HiscEntryGetNext (int *index, HISTORY_CRTL_ENTRY_T *entry )
{
    RMON_ENTRY_T   *row_entry_ptr = NULL;

    row_entry_ptr = ROWAPI_next(table_ptr, *index);
    if (row_entry_ptr)
    {
        memcpy(entry, row_entry_ptr->body, sizeof(HISTORY_CRTL_ENTRY_T));
        *index = row_entry_ptr->ctrl_index;
        return TRUE;

    }
    else
    {
        return FALSE;
    }

}


BOOL_T rmon_HiseEntryGetFirst(int ctrl_index, UI32_T *sample_index, HISTORY_DATA_ENTRY_T *entry)
{

    RMON_ENTRY_T   *hdr = NULL;
    SCROLLER_T     *scrlr = NULL;
    NEXTED_PTR_T   *bptr = NULL;

    hdr = ROWAPI_find(table_ptr, ctrl_index);

    if (hdr)
    {
        scrlr = history_extract_scroller(hdr->body);
        bptr = scrlr->first_data_ptr;

    }
    if (bptr)
    {
        *sample_index = bptr->data_index;
    }
    else
    {
        hdr = NULL;
    }

    if (hdr)
    {
        memcpy(entry, bptr, sizeof(HISTORY_DATA_ENTRY_T));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL_T rmon_HiseEntryGetNext(int *ctrl_index, UI32_T *sample_index, HISTORY_DATA_ENTRY_T *entry)
{

    RMON_ENTRY_T   *hdr = NULL;
    SCROLLER_T     *scrlr = NULL;
    NEXTED_PTR_T   *bptr = NULL;
    register u_long iii;

    hdr = ROWAPI_find(table_ptr, *ctrl_index);

    if (hdr)
    {
        scrlr = history_extract_scroller(hdr->body);
        bptr = scrlr->first_data_ptr;
        for (iii = 0; iii < scrlr->data_stored && bptr; iii++, bptr = bptr->next)
        {
            if (bptr->data_index && bptr->data_index > *sample_index)
            {
                break;
            }
        }

        if (bptr && bptr->data_index <= *sample_index)
        {
            bptr = NULL;
        }
        if (!bptr)
        {
            for (hdr = hdr->next; hdr; hdr = hdr->next)
            {
                if (RMON1_ENTRY_VALID != hdr->status)
                {
                    continue;
                }
                scrlr = history_extract_scroller(hdr->body);
                if (scrlr->data_stored <= 0)
                {
                    continue;
                }
                for (bptr = scrlr->first_data_ptr; bptr; bptr = bptr->next)
                {
                    if (bptr->data_index)
                    {
                        break;
                    }
                }
                if (bptr)
                {
                    break;
                }
            }
        }
        if (bptr)
        {
            *ctrl_index = hdr->ctrl_index;
            *sample_index = bptr->data_index;
        }
        else
        {
            hdr = NULL;
        }
        if (hdr)
        {
            memcpy(entry, bptr, sizeof(HISTORY_DATA_ENTRY_T));
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}
#endif

BOOL_T HISTORY_GetHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;

    if (NULL == entry_p)
    {
        return FALSE;
    }

    while (1)
    {
        rmon_p = ROWAPI_find(table_ptr, entry_p->id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }

        get_history_control_entry(rmon_p, entry_p);
		return TRUE;
    }

    return FALSE;
}

BOOL_T HISTORY_GetNextHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    return getnext_history_control_table_by_lport(0 /* not specified */, entry_p);
}

BOOL_T HISTORY_CreateHistoryControlEntry(SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    if (FALSE == create_history_control_entry(entry_p->id,
        entry_p->if_index,
        entry_p->buckets_requested,
        entry_p->interval,
        entry_p->owner,
        entry_p->status))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T HISTORY_DeleteHistoryControlEntryByLport(UI32_T if_index, UI32_T index)
{
    RMON_ENTRY_T *rmon_p;
    HISTORY_CRTL_ENTRY_T ctrl_entry;

    rmon_p = ROWAPI_find(table_ptr, index);

    if (FALSE == rmon_p)
    {
        return FALSE;
    }

    memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));

#if 0
    /* Cannot to create and delete in difference interface.
     */
    if (if_index != ctrl_entry.data_source.objid[ctrl_entry.data_source.length - 1])
    {
        return FALSE;
    }
#endif

    ROWAPI_delete_clone(table_ptr, index);
    rowapi_delete(rmon_p);

    return TRUE;
}

BOOL_T HISTORY_GetNextHistoryControlTableByLport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    return getnext_history_control_table_by_lport(lport, entry_p);
}

BOOL_T HISTORY_IsHistoryControlEntryModified(SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    char dflt_data_source[512];

    /* only default entry need check has been modified or not
    */
    if ((entry_p->id <= SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY)
         && (1 == dflt_create_ar[entry_p->id]))
    {
        sprintf(dflt_data_source, "%s.%lu", HISTORY_DEFAULT_DATA_SOURCE_WITHOUT_KEY_STR, (entry_p->id-1)/2+1);

        /* default entry but has been modified
         */
        if ( (VAL_eventStatus_valid != entry_p->status)
                || (0 != strcmp(entry_p->data_source, dflt_data_source))
                || (   (SYS_DFLT_RMON_HISTORY_INTERVAL_1 != entry_p->interval)
                    && (0 == (entry_p->id % 2)))
                || (   (SYS_DFLT_RMON_HISTORY_INTERVAL_2 != entry_p->interval)
                    && (1 == (entry_p->id % 2)))
                || (SYS_DFLT_RMON_HISTORY_BUCKETS_REQUESTED != entry_p->buckets_requested)
                || (0 != strcmp(HISTORY_DEFAULT_OWNER, entry_p->owner))
            )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    /* the extra added entry
     */
    return TRUE;
}

BOOL_T HISTORY_GetNextHistoryTableByControlIndex(SNMP_MGR_RmonHistoryEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;
    NEXTED_PTR_T *history_entry_p;
    UI32_T data_index;

    if (   (NULL == entry_p)
        || (0 == entry_p->control_index))
    {
        return FALSE;
    }

    rmon_p = ROWAPI_find(table_ptr, entry_p->control_index);

    if (NULL == rmon_p)
    {
        return FALSE;
    }

    data_index = entry_p->data_index;
    history_entry_p = getnext_history_entry(rmon_p, data_index);

    if (   (NULL != history_entry_p)
        && (history_entry_p->data_index > data_index))
    {
        get_rmon_history_table(rmon_p->ctrl_index, history_entry_p, entry_p);
        return TRUE;
    }

    /* used to getnext from next control_index
     */
#if 0
    for (rmon_p = rmon_p->next; rmon_p; rmon_p = rmon_p->next)
    {
        if (VAL_historyControlStatus_valid != rmon_p->status)
        {
            continue;
        }

        history_entry_p = getnext_history_entry(rmon_p, 0 /* get first entry */);

        if (NULL != history_entry_p)
        {
            get_rmon_history_table(rmon_p->ctrl_index, history_entry_p, entry_p);
            return TRUE;
        }
    }
#endif /* #if 0 */

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - HISTORY_GetNextDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default history entries
 *           that are not exist anymore.
 * INPUT   : lport
 *           entry_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T HISTORY_GetNextDeletedDefaultEntry(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;
    HISTORY_CRTL_ENTRY_T ctrl_entry;
    UI32_T i;

    for (i = entry_p->id + 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY; i++)
    {
        if (dflt_create_ar[i] == 0)
        {
            continue;
        }

        if (((i - 1) / 2 + 1) != lport)
        {
            continue;
        }

        rmon_p = ROWAPI_find(table_ptr, i);

        if (rmon_p == NULL)
        {
            /* entry not existed
            */
            entry_p->id = i;
            return TRUE;
        }
        else
        {
            /* entry existed but port has changed
            */
            memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));

            if (ctrl_entry.data_source.objid[ctrl_entry.data_source.length - 1] != ((i - 1) / 2 + 1))
            {
                entry_p->id = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}

static void get_history_control_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    HISTORY_CRTL_ENTRY_T ctrl_entry;

    memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));
    entry_p->if_index = ctrl_entry.data_source.objid[ctrl_entry.data_source.length - 1];
    entry_p->id = rmon_p->ctrl_index;
    SNMP_MGR_ConvertOidToStr(ctrl_entry.data_source.objid, ctrl_entry.data_source.length, entry_p->data_source, sizeof(entry_p->data_source));
    entry_p->buckets_requested = ctrl_entry.scrlr.data_requested;
    entry_p->buckets_granted = ctrl_entry.scrlr.data_granted;
    entry_p->interval = ctrl_entry.interval;

    if (NULL == rmon_p->owner)
    {
        entry_p->owner[0] = '\0';
    }
    else
    {
        strncpy(entry_p->owner, rmon_p->owner, sizeof(entry_p->owner)-1);
        entry_p->owner[sizeof(entry_p->owner)-1] = '\0';
    }

    entry_p->status = rmon_p->status;
}

static BOOL_T getnext_history_control_table_by_lport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;

    if (NULL == entry_p)
    {
        return FALSE;
    }

    while (1)
    {
        rmon_p = ROWAPI_next(table_ptr, entry_p->id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }

        get_history_control_entry(rmon_p, entry_p);

        if (   (0 == lport)
            || (entry_p->if_index == lport))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL_T create_history_control_entry(UI32_T id, UI32_T if_index, UI32_T buckets_requested, UI32_T interval, char *owner_p, UI32_T status)
{
    RMON_ENTRY_T *rmon_p;
    HISTORY_CRTL_ENTRY_T *body_p;
    VAR_OID_T data_source = {11, HISTORY_DEFAULT_DATA_SOURCE_WITH_KEY_ARR};

    if (NULL == owner_p)
    {
        return FALSE;
    }

    rmon_p = ROWAPI_find(table_ptr, id);

    if (NULL == rmon_p)
    {
        if (SNMP_ERR_NOERROR != ROWAPI_new(table_ptr, id))
        {
            return FALSE;
       }

        rmon_p = ROWAPI_find(table_ptr, id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }
    }

    body_p = (HISTORY_CRTL_ENTRY_T *)rmon_p->body;

    data_source.objid[data_source.length - 1] = if_index;
    memcpy(&body_p->data_source, &data_source, sizeof(body_p->data_source));

    body_p->scrlr.data_requested = (0 == buckets_requested) ? SYS_DFLT_RMON_HISTORY_BUCKETS_REQUESTED : buckets_requested;
    body_p->interval = (0 == interval) ? HIST_DEF_INTERVAL : interval;;

    rmon_p->new_owner = AGSTRDUP(owner_p);
    rmon_p->new_status = status;

    if (SNMP_ERR_NOERROR != ROWAPI_commit(table_ptr, id))
    {
        ROWAPI_delete_clone(table_ptr, id);
        rowapi_delete(rmon_p);
        return FALSE;
    }

    return TRUE;
}

static NEXTED_PTR_T *getnext_history_entry(RMON_ENTRY_T *rmon_p, UI32_T data_index)
{
    SCROLLER_T *scrlr_p;
    NEXTED_PTR_T *entry_p;

    if (NULL == rmon_p)
    {
        return NULL;
    }

    scrlr_p = history_extract_scroller(rmon_p->body);

    if (scrlr_p->data_stored <= 0)
    {
        return NULL;
    }

    for (entry_p = scrlr_p->first_data_ptr; entry_p; entry_p = entry_p->next)
    {
        /* get first entry
         */
        if (0 == data_index)
        {
            if (entry_p->data_index > 0)
            {
                return entry_p;
            }
        }
        else
        {
            if (entry_p->data_index > data_index)
            {
                return entry_p;
            }
        }
    }

    return NULL;
}

static void get_rmon_history_table(UI32_T control_index, NEXTED_PTR_T *src_entry_p, SNMP_MGR_RmonHistoryEntry_T *dst_entry_p)
{
    HISTORY_DATA_ENTRY_T src_data_entry;

    if (   (NULL == src_entry_p)
        || (NULL == dst_entry_p))
    {
        return;
    }

    memcpy(&src_data_entry, src_entry_p, sizeof(src_data_entry));

    dst_entry_p->control_index = control_index;
    dst_entry_p->data_index = src_data_entry.data_index;
    dst_entry_p->start_interval = src_data_entry.start_interval;
    dst_entry_p->drop_events = 0;
    dst_entry_p->octets = src_data_entry.EthData.octets;
    dst_entry_p->packets = src_data_entry.EthData.packets;
    dst_entry_p->bcast_pkts = src_data_entry.EthData.bcast_pkts;
    dst_entry_p->mcast_pkts = src_data_entry.EthData.mcast_pkts;
    dst_entry_p->crc_align = src_data_entry.EthData.crc_align;
    dst_entry_p->undersize = src_data_entry.EthData.undersize;
    dst_entry_p->oversize = src_data_entry.EthData.oversize;
    dst_entry_p->fragments = src_data_entry.EthData.fragments;
    dst_entry_p->jabbers = src_data_entry.EthData.jabbers;
    dst_entry_p->collisions = src_data_entry.EthData.collisions;
    dst_entry_p->utilization = src_data_entry.utilization;
}
