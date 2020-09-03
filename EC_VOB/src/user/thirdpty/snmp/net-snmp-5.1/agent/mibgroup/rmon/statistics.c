/**************************************************************
 * Copyright (C) 2001 Tali Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * TALI ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

/* We will allocate the memory when a row is created, if it has not enough
 * memory, then a row created will failed.The maximum entries we can created is defined
 * in SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY. This constant is used in the function
 * init_statistics for initilize the maximum entry of Rmon 's group.
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
 * RMON_ EVENT_Data_ENTRY_T = 16
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
 * Event Group:   RMON_ENTRY_T +  RMON event _Ctrl_T + NO_OF_LOG_ENTRIES* ( RMON_ EVENT_Data_ENTRY_T +
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
#include "swctrl.h"
#include "sys_adpt.h"
#endif /*end of #ifndef VXWORKS*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "util_funcs.h"

#include "snmp_mgr.h"
#include "l_mm.h"
#include "swctrl_pom.h"
#include "vlan_lib.h"
#include "vlan_pom.h"
        /*
         * Implementation headers
         */
#include "agutil_api.h"
#include "agutil.h"
#include "row_api.h"
#include "rows.h"
#include "statistics.h"
        /*
         * File scope definitions section
         */
        /*
         * from MIB compilation
         */
#define MIB_DESCR	"EthStat"
#define etherStatsEntryFirstIndexBegin	11
#define IDetherStatsDroppedFrames        1
#define IDetherStatsCreateTime           2
#define IDetherStatsIndex                3
#define IDetherStatsDataSource           4
#define IDetherStatsDropEvents           5
#define IDetherStatsOctets               6
#define IDetherStatsPkts                 7
#define IDetherStatsBroadcastPkts        8
#define IDetherStatsMulticastPkts        9
#define IDetherStatsCRCAlignErrors       10
#define IDetherStatsUndersizePkts        11
#define IDetherStatsOversizePkts         12
#define IDetherStatsFragments            13
#define IDetherStatsJabbers              14
#define IDetherStatsCollisions           15
#define IDetherStatsPkts64Octets         16
#define IDetherStatsPkts65to127Octets    17
#define IDetherStatsPkts128to255Octets   18
#define IDetherStatsPkts256to511Octets   19
#define IDetherStatsPkts512to1023Octets  20
#define IDetherStatsPkts1024to1518Octets 21
#define IDetherStatsOwner                22
#define IDetherStatsStatus               23
#define Leaf_etherStatsDataSource        2
#define Leaf_etherStatsOwner             20
#define Leaf_etherStatsStatus            21

/* etherStatsHighCapacityTable */
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS                    1
#define ETHERSTATSHIGHCAPACITYPKTS                            2
#define ETHERSTATSHIGHCAPACITYOVERFLOWOCTETS                  3
#define ETHERSTATSHIGHCAPACITYOCTETS                          4
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS64OCTETS            5
#define ETHERSTATSHIGHCAPACITYPKTS64OCTETS                    6
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS65TO127OCTETS       7
#define ETHERSTATSHIGHCAPACITYPKTS65TO127OCTETS               8
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS128TO255OCTETS      9
#define ETHERSTATSHIGHCAPACITYPKTS128TO255OCTETS              10
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS256TO511OCTETS      11
#define ETHERSTATSHIGHCAPACITYPKTS256TO511OCTETS              12
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS512TO1023OCTETS     13
#define ETHERSTATSHIGHCAPACITYPKTS512TO1023OCTETS		      14
#define ETHERSTATSHIGHCAPACITYOVERFLOWPKTS1024TO1518OCTETS    15
#define ETHERSTATSHIGHCAPACITYPKTS1024TO1518OCTETS            16

#define STATISTICS_DEFAULT_DATA_SOURCE_WITH_KEY_ARR     {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 1}
#define STATISTICS_DEFAULT_DATA_SOURCE_WITHOUT_KEY_STR  "1.3.6.1.2.1.2.2.1.1"
#define STATISTICS_DEFAULT_OWNER                        ""


     typedef struct {
         VAR_OID_T              data_source;
         u_long                 etherStatsCreateTime;
         ETH_STATS_T            eth;
         ETH_STATS_T            create_time_eth; /* kinghong: create time eth data, actual packet should be eth-create_time_eth */
         ETH_STATS_HC_T         hc_eth;
         ETH_STATS_HC_T         hc_create_time_eth;

     } CRTL_ENTRY_T;

/*
 * Main section
 */

     static TABLE_DEFINTION_T
         StatCtrlTable;
     static TABLE_DEFINTION_T *
         table_ptr = &
         StatCtrlTable;
     BOOL_T statistics_init_done = FALSE;


static int dflt_create_ar[SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY+1];

static void get_statistics_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonStatisticsEntry_T *entry_p);
static BOOL_T getnext_statistics_entry_by_lport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);
static BOOL_T create_statistics_entry(UI32_T id, UI32_T if_index, char *owner_p, UI32_T status);

/*
 * Control Table RowApi Callbacks
 */

     int
     stat_Create(RMON_ENTRY_T * eptr)
{                               /* create the body: alloc it and set defaults */
    CRTL_ENTRY_T   *body;
    static VAR_OID_T data_src_if_index_1 =
        { 11, STATISTICS_DEFAULT_DATA_SOURCE_WITH_KEY_ARR };

    eptr->body = AGMALLOC(sizeof(CRTL_ENTRY_T));
    if (!eptr->body)
        return -3;
    body = (CRTL_ENTRY_T *) eptr->body;

    /*
     * set defaults
     */
    memcpy(&body->data_source, &data_src_if_index_1, sizeof(VAR_OID_T));
    eptr->owner = AGSTRDUP("");
    memset(&body->eth, 0, sizeof(ETH_STATS_T));

    return 0;
}

int
stat_Validate(RMON_ENTRY_T * eptr)
{
#if 0
       CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
      //  UI32_T unit;
       // UI32_T port;
       // UI32_T trunk_id;
        // UI32_T vid;

     //  VLAN_MGR_ConvertFromIfindex(body->data_source.objid[body->data_source.length - 1], &vid);

       if (SWCTRL_POM_LogicalPortExisting(body->data_source.objid[body->data_source.length - 1]))

            return SNMP_ERR_NOERROR;
      else
          return SNMP_ERR_BADVALUE;
 #endif
    return 0;
}

int
stat_Activate(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;

    body->etherStatsCreateTime = AGUTIL_sys_up_time();

    return 0;
}

int
stat_Copy(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    CRTL_ENTRY_T   *clone = (CRTL_ENTRY_T *) eptr->tmp;

    if (snmp_oid_compare
        (clone->data_source.objid, clone->data_source.length,
         body->data_source.objid, body->data_source.length)) {
        memcpy(&body->data_source, &clone->data_source, sizeof(VAR_OID_T));
    }

    return 0;
}

int
stat_Deactivate(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    memset(&body->eth, 0, sizeof(ETH_STATS_T));
    return 0;
}


/***************************************************
 * Function:var_etherStats2Entry
 * Purpose: Handles the request for etherStats2Entry variable instances
 ***************************************************/
u_char         *
var_etherStats2Entry(struct variable * vp, oid * name, size_t * length,
                     int exact, size_t * var_len,
                     WriteMethod ** write_method)
{
    static long     long_return;
    static CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;

    *write_method = NULL;

    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(CRTL_ENTRY_T));
    if (!hdr)
        return NULL;

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case IDetherStatsDroppedFrames:
        long_return = 0;
        return (u_char *) & long_return;
    case IDetherStatsCreateTime:
        long_return = theEntry.etherStatsCreateTime;
        return (u_char *) & long_return;
    default:
        break;
    } /* of switch by 'vp->magic'  */

    return NULL;
}


/***************************************************
 * Function:write_etherStatsEntry
 ***************************************************/
static int
write_etherStatsEntry(int action, u_char * var_val, u_char var_val_type,
                      size_t var_val_len, u_char * statP,
                      oid * name, size_t name_len)
{
    long            long_temp;
    int             leaf_id, snmp_status;
    static int      prev_action = COMMIT;
    RMON_ENTRY_T   *hdr;
    CRTL_ENTRY_T   *cloned_body;
    CRTL_ENTRY_T   *body;

    /* kinghong add the following sematic check for setting RMON EntryStatus
     * 1. Set Valid(1)              Permit original state:  Valid(1),UnderCreation(3).
     * 2. Set CreateRequest(2)      Permit original state:  Not Exist.
     * 3. Set UnderCreation(3)      Permit original state:  Valid(1), UnderCreation(3).
     * 4. Set Invalid(4)            Permit original state:  Valid(1), UnderCreation(3).
     */
    if (name[etherStatsEntryFirstIndexBegin - 1] == Leaf_etherStatsStatus)
    {
         /* This is the 'set' value from MIB*/
        memcpy(&long_temp, var_val, sizeof(long));

        /* Step 1: Check current Status before do anything*/
        hdr = ROWAPI_find(table_ptr, name[etherStatsEntryFirstIndexBegin]);

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
        snmp_status =
            ROWAPI_do_another_action(name, etherStatsEntryFirstIndexBegin,
                                     action, &prev_action, table_ptr,
                                     sizeof(CRTL_ENTRY_T));
        if (SNMP_ERR_NOERROR != snmp_status) {
            ag_trace("failed action %d with %d", action, snmp_status);
        }
        /*
         *EPR_ID:ES3628BT-FLF-ZZ-00763
         *Problem: SNMP-SimpleTester:semantic tests->Run mib2 and miniRMON test cause DUT exception.
         *Root Cause: write_etherStatsEntry defult case not return value.
         *Solution: return snmp_status.
         *
         *Modified files:
         *	src\user\thirdty\snmp\net-snmp-5.1\agent\mibgroup\rmon\statistics.c
         */
        return snmp_status;

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */

        long_temp = name[etherStatsEntryFirstIndexBegin];
        leaf_id = (int) name[etherStatsEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(table_ptr, long_temp);        /* it MUST be OK */
        cloned_body = (CRTL_ENTRY_T *) hdr->tmp;
        body = (CRTL_ENTRY_T *) hdr->body;

        switch (leaf_id) {
        case Leaf_etherStatsDataSource:
          {
             UI32_T vid;
             //UI32_T dump_value;
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
                return snmp_status;
            }
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                snmp_oid_compare(cloned_body->data_source.objid,
                                 cloned_body->data_source.length,
                                 body->data_source.objid,
                                 body->data_source.length))
                return SNMP_ERR_BADVALUE;

            }
            break;

            break;
        case Leaf_etherStatsOwner:
            if (hdr->new_owner)
                AGFREE(hdr->new_owner);
            hdr->new_owner = AGMALLOC(MAX_OWNERSTRING + 1);
			if (var_val_type != ASN_OCTET_STR)
				return SNMP_ERR_WRONGTYPE;
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
        case Leaf_etherStatsStatus:


            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               RMON1_ENTRY_VALID,
                                               RMON1_ENTRY_INVALID,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                ag_trace("cannot browse etherStatsStatus");
                return snmp_status;
            }
            hdr->new_status = long_temp;

            /* kinghong:, when set to 1(valid), assign "create_time_eth" to it,
             * when set to 3(uncreate) or 2 (create-request), clear the "create_time_eth" to 0.
             */
            if ((hdr->new_status == RMON1_ENTRY_VALID) && (hdr->status != RMON1_ENTRY_VALID))
            {
               SYSTEM_get_eth_statistics( &body->data_source, &body->create_time_eth);
               SYSTEM_get_eth_statistics_HC( &body->data_source, &body->hc_create_time_eth);
            }
            else if ((hdr->new_status == RMON1_ENTRY_UNDER_CREATION) || (hdr->new_status == RMON1_ENTRY_CREATE_REQUEST))
            {
                 memset(&body->create_time_eth, 0, sizeof(ETH_STATS_T));
                 memset(&body->hc_create_time_eth, 0, sizeof(ETH_STATS_HC_T));
            }


            break;
        default:
            ag_trace("%s:unknown leaf_id=%d\n", table_ptr->name,
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;
    }                           /* of switch by 'action' */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}

/***************************************************
 * Function:var_etherStatsEntry
 * Purpose: Handles the request for etherStatsEntry variable instances
 ***************************************************/
u_char         *
var_etherStatsEntry(struct variable * vp, oid * name, size_t * length,
                    int exact, size_t * var_len,
                    WriteMethod ** write_method)
{
    static long     long_return;
    static CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;

    switch(vp->magic)
    {
        case IDetherStatsDataSource:
        case IDetherStatsOwner:
        case IDetherStatsStatus:
            *write_method = write_etherStatsEntry;
            break;
        default:
            *write_method = 0;
            break;
    }
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(CRTL_ENTRY_T));
    if (!hdr)
        return NULL;
    /* We only get statisic entry when status == valid */
    if (RMON1_ENTRY_VALID == hdr->status)
    {
        SYSTEM_get_eth_statistics(&theEntry.data_source, &theEntry.eth);
    }

    *var_len = sizeof(long);

    switch (vp->magic) {
    case IDetherStatsIndex:
        long_return = hdr->ctrl_index;
        return (u_char *) & long_return;
    case IDetherStatsDataSource:
        *var_len = sizeof(oid) * theEntry.data_source.length;
        return (unsigned char *) theEntry.data_source.objid;
    case IDetherStatsDropEvents:
        long_return = 0;        /* theEntry.eth.etherStatsDropEvents; */
        return (u_char *) & long_return;
    case IDetherStatsOctets:
        long_return = theEntry.eth.octets - theEntry.create_time_eth.octets;
        return (u_char *) & long_return;
    case IDetherStatsPkts:
        long_return = theEntry.eth.packets - theEntry.create_time_eth.packets;
        return (u_char *) & long_return;
    case IDetherStatsBroadcastPkts:
        long_return = theEntry.eth.bcast_pkts - theEntry.create_time_eth.bcast_pkts;
        return (u_char *) & long_return;
    case IDetherStatsMulticastPkts:
        long_return = theEntry.eth.mcast_pkts - theEntry.create_time_eth.mcast_pkts;
        return (u_char *) & long_return;
    case IDetherStatsCRCAlignErrors:
        long_return = theEntry.eth.crc_align - theEntry.create_time_eth.crc_align;
        return (u_char *) & long_return;
    case IDetherStatsUndersizePkts:
        long_return = theEntry.eth.undersize - theEntry.create_time_eth.undersize;
        return (u_char *) & long_return;
    case IDetherStatsOversizePkts:
        long_return = theEntry.eth.oversize - theEntry.create_time_eth.oversize;
        return (u_char *) & long_return;
    case IDetherStatsFragments:
        long_return = theEntry.eth.fragments - theEntry.create_time_eth.fragments;
        return (u_char *) & long_return;
    case IDetherStatsJabbers:
        long_return = theEntry.eth.jabbers - theEntry.create_time_eth.jabbers;
        return (u_char *) & long_return;
    case IDetherStatsCollisions:
        long_return = theEntry.eth.collisions - theEntry.create_time_eth.collisions;
        return (u_char *) & long_return;
    case IDetherStatsPkts64Octets:
        long_return = theEntry.eth.pkts_64 -theEntry.create_time_eth.pkts_64 ;
        return (u_char *) & long_return;
    case IDetherStatsPkts65to127Octets:
        long_return = theEntry.eth.pkts_65_127 - theEntry.create_time_eth.pkts_65_127;
        return (u_char *) & long_return;
    case IDetherStatsPkts128to255Octets:
        long_return = theEntry.eth.pkts_128_255 - theEntry.create_time_eth.pkts_128_255;
        return (u_char *) & long_return;
    case IDetherStatsPkts256to511Octets:
        long_return = theEntry.eth.pkts_256_511 - theEntry.create_time_eth.pkts_256_511;
        return (u_char *) & long_return;
    case IDetherStatsPkts512to1023Octets:
        long_return = theEntry.eth.pkts_512_1023 - theEntry.create_time_eth.pkts_512_1023;
        return (u_char *) & long_return;
    case IDetherStatsPkts1024to1518Octets:
        long_return = theEntry.eth.pkts_1024_1518 - theEntry.create_time_eth.pkts_1024_1518;
        return (u_char *) & long_return;
    case IDetherStatsOwner:
        if (hdr->owner) {
            *var_len = strlen(hdr->owner);
            return (unsigned char *) hdr->owner;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }
    case IDetherStatsStatus:
        long_return = hdr->status;
        return (u_char *) & long_return;
    default:
        ERROR_MSG("");
    };                          /* of switch by 'vp->magic'  */

    return NULL;
}

#if 1                           /* debug, but may be used for init. TBD: may be token snmpd.conf ? */
int
add_statistics_entry(int ctrl_index, int ifIndex)
{
    int             ierr;

    ierr = ROWAPI_new(table_ptr, ctrl_index);
    switch (ierr) {
    case -1:
        ag_trace("max. number exedes\n");
        break;
    case -2:
        ag_trace("malloc failed");
        break;
    case -3:
        ag_trace("ClbkCreate failed");
        break;
    case 0:
        break;
    default:
        ag_trace("Unknown code %d", ierr);
        break;
    }

    if (!ierr) {
        register RMON_ENTRY_T *eptr = ROWAPI_find(table_ptr, ctrl_index);
        if (!eptr) {
            ag_trace("cannot find it");
            ierr = -4;
        } else {
            CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;

            body->data_source.objid[body->data_source.length - 1] =
                ifIndex;
            /* first time init to 0 */
            memset(&body->create_time_eth, 0, sizeof(ETH_STATS_T));
            memset(&body->hc_create_time_eth, 0, sizeof(ETH_STATS_HC_T));
            eptr->new_status = RMON1_ENTRY_VALID;
            ierr = ROWAPI_commit(table_ptr, ctrl_index);
            if (ierr) {
                ag_trace("ROWAPI_commit returned %d", ierr);
            }
        }
    }

    return ierr;
}
#endif

/***************************************************
 * define Variables callbacks
 ***************************************************/
oid             oidstatisticsVariablesOid[] = { 1, 3, 6, 1, 2, 1, 16, 1 };

struct variable7 oidstatisticsVariables[] = {
    {IDetherStatsIndex, ASN_INTEGER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 1}},
    {IDetherStatsDataSource, ASN_OBJECT_ID, RWRITE, var_etherStatsEntry, 3,
     {1, 1, 2}},
    {IDetherStatsDropEvents, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 3}},
    {IDetherStatsOctets, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 4}},
    {IDetherStatsPkts, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 5}},
    {IDetherStatsBroadcastPkts, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 6}},
    {IDetherStatsMulticastPkts, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 7}},
    {IDetherStatsCRCAlignErrors, ASN_COUNTER, RONLY, var_etherStatsEntry,
     3, {1, 1, 8}},
    {IDetherStatsUndersizePkts, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 9}},
    {IDetherStatsOversizePkts, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 10}},
    {IDetherStatsFragments, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 11}},
    {IDetherStatsJabbers, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 12}},
    {IDetherStatsCollisions, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 13}},
    {IDetherStatsPkts64Octets, ASN_COUNTER, RONLY, var_etherStatsEntry, 3,
     {1, 1, 14}},
    {IDetherStatsPkts65to127Octets, ASN_COUNTER, RONLY,
     var_etherStatsEntry, 3, {1, 1, 15}},
    {IDetherStatsPkts128to255Octets, ASN_COUNTER, RONLY,
     var_etherStatsEntry, 3, {1, 1, 16}},
    {IDetherStatsPkts256to511Octets, ASN_COUNTER, RONLY,
     var_etherStatsEntry, 3, {1, 1, 17}},
    {IDetherStatsPkts512to1023Octets, ASN_COUNTER, RONLY,
     var_etherStatsEntry, 3, {1, 1, 18}},
    {IDetherStatsPkts1024to1518Octets, ASN_COUNTER, RONLY,
     var_etherStatsEntry, 3, {1, 1, 19}},
    {IDetherStatsOwner, ASN_OCTET_STR, RWRITE, var_etherStatsEntry, 3,
     {1, 1, 20}},
    {IDetherStatsStatus, ASN_INTEGER, RWRITE, var_etherStatsEntry, 3,
     {1, 1, 21}},
    {IDetherStatsDroppedFrames, ASN_COUNTER, RONLY, var_etherStats2Entry,
     3, {4, 1, 1}},
    {IDetherStatsCreateTime, ASN_TIMETICKS, RONLY, var_etherStats2Entry, 3,
     {4, 1, 2}},
};

/***************************************************
 * Function:init_statistics
 * Purpose: register statistics objects in the agent
 ***************************************************/
void
init_statistics(void)
{
    REGISTER_MIB(MIB_DESCR, oidstatisticsVariables, variable7,
                 oidstatisticsVariablesOid);

    ROWAPI_init_table(&StatCtrlTable, MIB_DESCR, SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY, &stat_Create, NULL, /* &stat_Clone, */
                      NULL,     /* &stat_Delete, */
                      &stat_Validate,
                      &stat_Activate, &stat_Deactivate, &stat_Copy);

#if 0                           /* debug */
    {
        int             iii;
        for (iii = 1; iii < SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY; iii++)
        {
           if (SWCTRL_POM_LogicalPortExisting(iii))
            add_statistics_entry(iii, iii);
        }

    //    add_statistics_entry(10, 16);
      //  add_statistics_entry(12, 11);
    }
#endif
}

void STATISTICS_CreateDefaultEntry()
{
    int i;

    memset(dflt_create_ar, 0, sizeof(dflt_create_ar));

    for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY; i++)
    {
        if (FALSE == SWCTRL_POM_LogicalPortExisting(i))
        {
            continue;
        }

        add_statistics_entry(i, i);

        /* flag to record creation entries
         */
        dflt_create_ar[i] = 1;
    }
}

void STATISTICS_DeleteAllRow()
{
	register RMON_ENTRY_T *eptr;
	int i;

	for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY; i++)
	{
        eptr = ROWAPI_find(table_ptr, i);
        if (NULL == eptr)
        {
            continue;
        }

        ROWAPI_delete_clone(table_ptr, i);
        rowapi_delete(eptr);
    }
}

oid etherStatsHighCapacityTable_variables_oid[] = {1, 3, 6, 1, 2, 1, 16, 1};

struct variable3 etherStatsHighCapacityTable_variables[] =
{
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 1}},
    {ETHERSTATSHIGHCAPACITYPKTS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 2}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWOCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 3}},
    {ETHERSTATSHIGHCAPACITYOCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 4}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS64OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 5}},
    {ETHERSTATSHIGHCAPACITYPKTS64OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 6}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS65TO127OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 7}},
    {ETHERSTATSHIGHCAPACITYPKTS65TO127OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 8}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS128TO255OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 9}},
    {ETHERSTATSHIGHCAPACITYPKTS128TO255OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 10}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS256TO511OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 11}},
    {ETHERSTATSHIGHCAPACITYPKTS256TO511OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 12}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS512TO1023OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 13}},
    {ETHERSTATSHIGHCAPACITYPKTS512TO1023OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 14}},
    {ETHERSTATSHIGHCAPACITYOVERFLOWPKTS1024TO1518OCTETS, ASN_COUNTER, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 15}},
    {ETHERSTATSHIGHCAPACITYPKTS1024TO1518OCTETS, ASN_COUNTER64, RONLY, var_etherStatsHighCapacityTable, 3, {7, 1, 16}},
};


void init_etherStatsHighCapacityTable(void)
{
    REGISTER_MIB("etherStatsHighCapacityTable",
                 etherStatsHighCapacityTable_variables, variable3,
                 etherStatsHighCapacityTable_variables_oid);
}


unsigned char* var_etherStatsHighCapacityTable(struct variable *vp,
                                               oid             *name,
                                               size_t          *length,
                                               int             exact,
                                               size_t          *var_len,
                                               WriteMethod     **write_method)
{
    static long long_return;
    static CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T *hdr;
    UI64_T tempValue;

    *write_method = 0;

    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr, &theEntry, sizeof(CRTL_ENTRY_T));

    if(!hdr)
        return NULL;

    if(hdr->status == RMON1_ENTRY_VALID)
    {
        SYSTEM_get_eth_statistics_HC(&theEntry.data_source, &theEntry.hc_eth);
    }

    *var_len = sizeof(long_return);

    switch(vp->magic)
    {
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS:
            long_return = theEntry.hc_eth.hc_overflow_pkts - theEntry.hc_create_time_eth.hc_overflow_pkts;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts - theEntry.hc_create_time_eth.hc_pkts;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWOCTETS:
            long_return = theEntry.hc_eth.hc_overflow_octets - theEntry.hc_create_time_eth.hc_overflow_octets;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYOCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_octets - theEntry.hc_create_time_eth.hc_octets;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS64OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_64 - theEntry.hc_create_time_eth.hc_overflow_pkts_64;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS64OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_64 - theEntry.hc_create_time_eth.hc_pkts_64;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS65TO127OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_65_127 - theEntry.hc_create_time_eth.hc_overflow_pkts_65_127;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS65TO127OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_65_127 - theEntry.hc_create_time_eth.hc_pkts_65_127;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS128TO255OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_128_255 - theEntry.hc_create_time_eth.hc_overflow_pkts_128_255;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS128TO255OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_128_255 - theEntry.hc_create_time_eth.hc_pkts_128_255;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS256TO511OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_256_511 - theEntry.hc_create_time_eth.hc_overflow_pkts_256_511;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS256TO511OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_256_511 - theEntry.hc_create_time_eth.hc_pkts_256_511;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS512TO1023OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_512_1023 - theEntry.hc_create_time_eth.hc_overflow_pkts_512_1023;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS512TO1023OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_512_1023 - theEntry.hc_create_time_eth.hc_pkts_512_1023;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        case ETHERSTATSHIGHCAPACITYOVERFLOWPKTS1024TO1518OCTETS:
            long_return = theEntry.hc_eth.hc_overflow_pkts_1024_1518 - theEntry.hc_create_time_eth.hc_overflow_pkts_1024_1518;
            return (u_char *) & long_return;
        case ETHERSTATSHIGHCAPACITYPKTS1024TO1518OCTETS:
            *var_len = sizeof(long64_return);
            tempValue = theEntry.hc_eth.hc_pkts_1024_1518 - theEntry.hc_create_time_eth.hc_pkts_1024_1518;
            SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, tempValue);
            return (u_char *) & long64_return;
        default:
            ERROR_MSG("");
    }

    return NULL;
}

BOOL_T STATISTICS_GetStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p)
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

        get_statistics_entry(rmon_p, entry_p);
		return TRUE;
    }

    return FALSE;
}

BOOL_T STATISTICS_GetNextStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    return getnext_statistics_entry_by_lport(0 /* not specified */, entry_p);
}

BOOL_T STATISTICS_CreateStatisticsEntry(SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    if (FALSE == create_statistics_entry(entry_p->id,
        entry_p->if_index,
        entry_p->owner,
        entry_p->status))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T STATISTICS_DeleteStatisticsEntryByLport(UI32_T if_index, UI32_T index)
{
    RMON_ENTRY_T *rmon_p;
    CRTL_ENTRY_T ctrl_entry;

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

BOOL_T STATISTICS_GetNextStatisticsTableByLport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    return getnext_statistics_entry_by_lport(lport, entry_p);
}

BOOL_T STATISTICS_IsStatisticsEntryModified(SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    char dflt_data_source[512];

    /* only default entry need check has been modified or not
    */
    if ((entry_p->id <= SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY)
         && (1 == dflt_create_ar[entry_p->id]))
    {
        sprintf(dflt_data_source, "%s.%lu", STATISTICS_DEFAULT_DATA_SOURCE_WITHOUT_KEY_STR, entry_p->id);

        /* default entry but has been modified
         */
        if ( (VAL_etherStatsStatus_valid != entry_p->status)
                || (0 != strcmp(entry_p->data_source, dflt_data_source))
                || (0 != strcmp(STATISTICS_DEFAULT_OWNER, entry_p->owner))
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

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STATISTICS_GetNextDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default statistics entries
 *           that are not exist anymore.
 * INPUT   : lport
 *           entry_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T STATISTICS_GetNextDeletedDefaultEntry(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;
    CRTL_ENTRY_T ctrl_entry;
    UI32_T i;

    if (entry_p->id > SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY)
    {
        return FALSE;
    }

    for (i = entry_p->id + 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY; i++)
    {
        if (dflt_create_ar[i] == FALSE)
        {
            continue;
        }

        if (i != lport)
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

            if (ctrl_entry.data_source.objid[ctrl_entry.data_source.length - 1] != i)
            {
                entry_p->id = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}

static void get_statistics_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonStatisticsEntry_T *entry_p)
{
    CRTL_ENTRY_T ctrl_entry;

    memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));

    if (RMON1_ENTRY_VALID == rmon_p->status)
    {
        SYSTEM_get_eth_statistics(&ctrl_entry.data_source, &ctrl_entry.eth);
    }

    entry_p->if_index = ctrl_entry.data_source.objid[ctrl_entry.data_source.length - 1];
    entry_p->id = rmon_p->ctrl_index;
    SNMP_MGR_ConvertOidToStr(ctrl_entry.data_source.objid, ctrl_entry.data_source.length, entry_p->data_source, sizeof(entry_p->data_source));
    entry_p->drop_events = 0;
    entry_p->octets = ctrl_entry.eth.octets - ctrl_entry.create_time_eth.octets;;
    entry_p->packets = ctrl_entry.eth.packets - ctrl_entry.create_time_eth.packets;
    entry_p->bcast_pkts = ctrl_entry.eth.bcast_pkts - ctrl_entry.create_time_eth.bcast_pkts;
    entry_p->mcast_pkts = ctrl_entry.eth.mcast_pkts - ctrl_entry.create_time_eth.mcast_pkts;
    entry_p->crc_align = ctrl_entry.eth.crc_align - ctrl_entry.create_time_eth.crc_align;
    entry_p->undersize = ctrl_entry.eth.undersize - ctrl_entry.create_time_eth.undersize;;
    entry_p->oversize = ctrl_entry.eth.oversize - ctrl_entry.create_time_eth.oversize;
    entry_p->fragments = ctrl_entry.eth.fragments - ctrl_entry.create_time_eth.fragments;
    entry_p->jabbers = ctrl_entry.eth.jabbers - ctrl_entry.create_time_eth.jabbers;
    entry_p->collisions = ctrl_entry.eth.collisions - ctrl_entry.create_time_eth.collisions;
    entry_p->pkts_64 = ctrl_entry.eth.pkts_64 -ctrl_entry.create_time_eth.pkts_64;
    entry_p->pkts_65_127 = ctrl_entry.eth.pkts_65_127 - ctrl_entry.create_time_eth.pkts_65_127;
    entry_p->pkts_128_255 = ctrl_entry.eth.pkts_128_255 - ctrl_entry.create_time_eth.pkts_128_255;
    entry_p->pkts_256_511 = ctrl_entry.eth.pkts_256_511 - ctrl_entry.create_time_eth.pkts_256_511;
    entry_p->pkts_512_1023 = ctrl_entry.eth.pkts_512_1023 - ctrl_entry.create_time_eth.pkts_512_1023;
    entry_p->pkts_1024_1518 = ctrl_entry.eth.pkts_1024_1518 - ctrl_entry.create_time_eth.pkts_1024_1518;

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

static BOOL_T getnext_statistics_entry_by_lport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p)
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

        get_statistics_entry(rmon_p, entry_p);

        if (   (0 == lport)
            || (entry_p->if_index == lport))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL_T create_statistics_entry(UI32_T id, UI32_T if_index, char *owner_p, UI32_T status)
{
    RMON_ENTRY_T *rmon_p;
    CRTL_ENTRY_T *body_p;
    VAR_OID_T data_source = {11, STATISTICS_DEFAULT_DATA_SOURCE_WITH_KEY_ARR};

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

    body_p = (CRTL_ENTRY_T *)rmon_p->body;

    data_source.objid[data_source.length - 1] = if_index;
    memcpy(&body_p->data_source, &data_source, sizeof(body_p->data_source));

    memset(&body_p->create_time_eth, 0, sizeof(body_p->create_time_eth));
    memset(&body_p->hc_create_time_eth, 0, sizeof(body_p->hc_create_time_eth));

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
