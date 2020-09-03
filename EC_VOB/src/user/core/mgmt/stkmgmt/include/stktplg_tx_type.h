/* MODULE NAME:  stktplg_tx_type.h
 * PURPOSE:
 *  This header file defines the data type required among several modules.
 *
 * NOTES:
 *
 * HISTORY
 *    8/7/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKTPLG_TX_TYPE_H
#define STKTPLG_TX_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"

#include "stktplg_om.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* following two definitions are defined for L_MM to prepare the dedicated buffer
 * pool for stktplg_tx
 */
#define STKTPLG_TX_TYPE_HBT1_DEDICATED_BUFFER_POOL_PARTITION_SIZE sizeof(STKTPLG_TX_TYPE_HBT1_T)
#define STKTPLG_TX_TYPE_HBT1_DEDICATED_BUFFER_POOL_PARTITION_NUM  8

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct STKTPLG_TX_TYPE_HBT1_S {
    STKTPLG_OM_HBT_0_1_T hbt1;
}__attribute__((packed, aligned(1)))STKTPLG_TX_TYPE_HBT1_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of STKTPLG_TX_TYPE_H */

