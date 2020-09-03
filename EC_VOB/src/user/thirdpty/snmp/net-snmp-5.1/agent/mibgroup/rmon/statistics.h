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

#ifndef _statistics_h_included__
#define _statistics_h_included__
#include "snmp_mgr.h"

BOOL_T create_rmon_statistics_table(unsigned long ctrl_index, char *owner_name, int if_index);
BOOL_T delete_rmon_statistics_table(unsigned long ctrl_index);

config_require(util_funcs)
     void            init_statistics(void);
     void            init_etherStatsHighCapacityTable(void);
     FindVarMethod   var_etherStatsHighCapacityTable;

BOOL_T STATISTICS_GetStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T STATISTICS_GetNextStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T STATISTICS_CreateStatisticsEntry(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T STATISTICS_DeleteStatisticsEntryByLport(UI32_T if_index, UI32_T index);
BOOL_T STATISTICS_GetNextStatisticsTableByLport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T STATISTICS_IsStatisticsEntryModified(SNMP_MGR_RmonStatisticsEntry_T *entry_p);

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
BOOL_T STATISTICS_GetNextDeletedDefaultEntry(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);

#endif                          /* _statistics_h_included__ */

/*
 * end of file statistics.h 
 */
