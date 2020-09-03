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

#ifndef _MIBGROUP_EVENT_H
#define _MIBGROUP_EVENT_H
#include "snmp_mgr.h"


config_require(util_funcs)

    /*
     * function prototypes 
     */
 void            init_event(void);

BOOL_T EVENT_GetEventTable(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T EVENT_GetNextEventTable(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T EVENT_CreateEventEntry(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T EVENT_DeleteEventEntry(UI32_T index);
BOOL_T EVENT_IsEventEntryModified(SNMP_MGR_RmonEventEntry_T *entry_p);

#endif                          /* _MIBGROUP_EVENT_H */
