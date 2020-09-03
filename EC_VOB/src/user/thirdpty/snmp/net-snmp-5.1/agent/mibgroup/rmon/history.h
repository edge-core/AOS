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

#ifndef _MIBGROUP_HISTORY_H
#define _MIBGROUP_HISTORY_H
#include "netsnmp_port.h"

#ifdef VXWORKS
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"

/*
 * Implementation headers 
 */
#include "agutil_api.h"
#include "row_api.h"
#include "snmp_mgr.h"
#endif

config_require(util_funcs)

    /*
     * function prototypes 
     */
     void            init_history(void);
void            init_etherHistoryHighCapacityTable(void);
FindVarMethod   var_etherHistoryHighCapacityTable;

#ifdef VXWORKS
#define CTRL_INDEX		3
#define CTRL_DATASOURCE		4
#define CTRL_BUCKETSREQUESTED	5
#define CTRL_BUCKETSGRANTED	6
#define CTRL_INTERVAL		7
#define CTRL_OWNER		8
#define CTRL_STATUS		9

#define DATA_INDEX		3
#define DATA_SAMPLEINDEX	4
#define DATA_INTERVALSTART	5
#define DATA_DROPEVENTS		6
#define DATA_OCTETS		7
#define DATA_PKTS		8
#define DATA_BROADCASTPKTS	9
#define DATA_MULTICASTPKTS	10
#define DATA_CRCALIGNERRORS	11
#define DATA_UNDERSIZEPKTS	12
#define DATA_OVERSIZEPKTS	13
#define DATA_FRAGMENTS		14
#define DATA_JABBERS		15
#define DATA_COLLISIONS		16
#define DATA_UTILIZATION	17

#define ETHERHISTORYHIGHCAPACITYOVERFLOWPKTS    1
#define ETHERHISTORYHIGHCAPACITYPKTS            2
#define ETHERHISTORYHIGHCAPACITYOVERFLOWOCTETS  3
#define ETHERHISTORYHIGHCAPACITYOCTETS          4


/* kinghong moved from the structue from history.c for support RMON API*/ 
typedef struct data_struct_t {
    struct data_struct_t *next;
    u_long          data_index;
    u_long          start_interval;
    u_long          utilization;
    ETH_STATS_T     EthData;
    ETH_STATS_HC_T  EthData_HC;
} HISTORY_DATA_ENTRY_T;

typedef struct {
    u_long          interval;
    u_long          timer_id;
    VAR_OID_T       data_source;

    u_long          coeff;
    HISTORY_DATA_ENTRY_T    previous_bucket;
    SCROLLER_T      scrlr;
} HISTORY_CRTL_ENTRY_T;


/* added by kinghong to get the next history control Entry*/
BOOL_T rmon_HiscEntryGetFirst( int *index, HISTORY_CRTL_ENTRY_T *entry);
BOOL_T rmon_HiscEntryGetNext (int *index, HISTORY_CRTL_ENTRY_T *entry );
BOOL_T rmon_HiseEntryGetFirst(int ctrl_index, UI32_T *sample_index, HISTORY_DATA_ENTRY_T *entry);
BOOL_T rmon_HiseEntryGetNext(int *ctrl_index, UI32_T *sample_index, HISTORY_DATA_ENTRY_T *entry);

BOOL_T HISTORY_GetHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T HISTORY_GetNextHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T HISTORY_CreateHistoryControlEntry(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T HISTORY_DeleteHistoryControlEntryByLport(UI32_T if_index, UI32_T index);
BOOL_T HISTORY_GetNextHistoryControlTableByLport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T HISTORY_IsHistoryControlEntryModified(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T HISTORY_GetNextHistoryTableByControlIndex(SNMP_MGR_RmonHistoryEntry_T *entry_p);

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
BOOL_T HISTORY_GetNextDeletedDefaultEntry(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);

#endif /*end of #ifdef VXWORKS*/
#endif                          /* _MIBGROUP_HISTORY_H */
