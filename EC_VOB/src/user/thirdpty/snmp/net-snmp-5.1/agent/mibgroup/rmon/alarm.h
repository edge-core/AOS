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

#ifndef _MIBGROUP_ALARM_H
#define _MIBGROUP_ALARM_H
#include "snmp_mgr.h"

config_require(util_funcs)

    /*
     * function prototypes 
     */
void            init_alarm(void);

BOOL_T ALARM_GetAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T ALARM_GetNextAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T ALARM_CreateAlarmEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T ALARM_DeleteAlarmEntry(UI32_T index);
BOOL_T ALARM_IsAlarmEntryModified(SNMP_MGR_RmonAlarmEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ALARM_GetNextDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default alarm entries
 *           that are not exist anymore.
 * INPUT   : entry_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ALARM_GetNextDeletedDefaultEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p);

#endif                          /* _MIBGROUP_ALARM_H */
