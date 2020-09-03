/* MODULE NAME:  syncedrv_type.h
 * PURPOSE:
 *  This header file contains definitions which are related to synchronous
 *  ethernet.
 * NOTES:
 *
 * HISTORY
 *    12/16/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
#ifndef SYNCEDRV_TYPE_H
#define SYNCEDRV_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SYNCEDRV_TYPE_RET_OK,
    SYNCEDRV_TYPE_RET_ERR_FAIL,
    SYNCEDRV_TYPE_RET_ERR_ARG,
    SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE,
} SYNCEDRV_TYPE_RET_T;

typedef enum SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_E
{
    SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL,      /* PHY use lock clock as clock source */
    SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL    /* PHY use external clock(from synce ASIC) as clock source */
} SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_T;

typedef enum SYNCEDRV_TYPE_CHIP_OPERATING_MODE_E
{
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_FREE_RUN,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_LOCKED,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_HOLDOVER,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_PRE_LOCKED,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_PRE_LOCKED2,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_LOST_PHASE,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_UNKNOWN,
    SYNCEDRV_TYPE_CHIP_OPERATING_MODE_NUM /* This enum must be kept in the last */
}SYNCEDRV_TYPE_CHIP_OPERATING_MODE_T;

typedef enum
{ 
    SYNCEDRV_CHIP_MODE_TYPE_NORMAL, /* output clock is locked to the selected input clock */
    SYNCEDRV_CHIP_MODE_TYPE_DCO,    /* output clock is controlled by chip register, which can be configured by driver (DCO = Digital Controlled Oscillator) */
    SYNCEDRV_CHIP_MODE_TYPE_NUMBER  /* this enum must be kept in the last */
} SYNCEDRV_CHIP_MODE_TYPE_T;

typedef struct
{
    UI32_T port;              /* user port id of the clock source */
    UI32_T priority;          /* priority of the clock source */
    BOOL_T is_good_status:1;  /* TRUE:clock source status good, FALSE: clock source status bad. */
    BOOL_T is_active:1;       /* TRUE:active, FALSE: not active. */
} SYNCEDRV_TYPE_ClockSource_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of SYNCEDRV_TYPE_H */

