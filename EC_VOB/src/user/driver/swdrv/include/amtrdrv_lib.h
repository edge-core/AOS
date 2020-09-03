/* MODULE NAME:  amtrdrv_lib.h
 * PURPOSE:
 *         This file provides utility routines which are related
 *         to AMTR/AMTRDRV, such as update counters by a given mac
 *         entry.
 *
 * NOTES:
 *
 * HISTORY
 *    12/8/2013 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2013
 */
#ifndef AMTRDRV_LIB_H
#define AMTRDRV_LIB_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "amtr_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTRDRV_LIB_UpdateCount
 * ------------------------------------------------------------------------
 * Purpose: Update AMTR counters by the given AMTR Mac address entry.
 * INPUT  : addr_entry_p    - The Mac address entry to be added or deleted
 *          is_to_increase  - TRUE:  addr_entry_p will be added to OM and
 *                                   will increase the related counters
 *                            FALSE: addr_entry_p will be removed from OM and
 *                                   will decrease the related counters
 * OUTPUT : amtr_counters_p - The corresponding counters will be updated by
 *                            the given addr_entry_p and is_to_increase.
 * RETURN : TRUE  -  Success
 *          FALSE -  Fail
 * NOTES  :
 *   1. Caller need to protect amtr_counters_p for race condition.
 *   2. If the address.life_time = other(under create entry) or source = self
 *      (CPU mac) or ifindex ==0(under create entry), we only update total
 *      counter.
 *
 * ------------------------------------------------------------------------
 */
 BOOL_T AMTRDRV_LIB_UpdateCount(AMTR_TYPE_AddrEntry_T *addr_entry_p, BOOL_T is_to_increase, AMTR_TYPE_Counters_T* amtr_counters_p);

#endif    /* End of AMTRDRV_LIB_H */

