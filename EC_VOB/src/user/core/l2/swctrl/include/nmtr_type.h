/* MODULE NAME:  nmtr_type.h
 * PURPOSE:
 *    This is a sample code for definition required by nmtr
 *
 * NOTES:
 *    This file shall contain the common data type.
 *
 * REASON:
 * Description:
 * HISTORY
 *    8/4/2007 - kh shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef NMTR_TYPE_H
#define NMTR_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* definitions of result of the operation in NMTR
 */
enum
{
    NMTR_TYPE_RESULT_OK=0,
    NMTR_TYPE_RESULT_FAIL
};

#ifndef SYS_ADPT_NMTR_HIST_MAX_CTRL_NAME_LEN
#define SYS_ADPT_NMTR_HIST_MAX_CTRL_NAME_LEN    31
#endif

#define NMTR_TYPE_HIST_CTRL_INDEX_MIN           MIN_portHistControlIndex
#define NMTR_TYPE_HIST_CTRL_INDEX_MAX           MAX_portHistControlIndex
#define NMTR_TYPE_HIST_CTRL_NAME_LEN_MIN        1
#define NMTR_TYPE_HIST_CTRL_NAME_LEN_MAX        SYS_ADPT_NMTR_HIST_MAX_CTRL_NAME_LEN
#define NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MIN     MIN_portHistControlDataSource
#define NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MAX     MAX_portHistControlDataSource
#define NMTR_TYPE_HIST_CTRL_INTERVAL_MIN        SYS_ADPT_NMTR_HIST_MIN_CTRL_INTERVAL
#define NMTR_TYPE_HIST_CTRL_INTERVAL_MAX        SYS_ADPT_NMTR_HIST_MAX_CTRL_INTERVAL
#define NMTR_TYPE_HIST_CTRL_BUCKETS_MIN         1
#define NMTR_TYPE_HIST_CTRL_BUCKETS_MAX         SYS_ADPT_NMTR_HIST_MAX_CTRL_BUCKETS

#define NMTR_TYPE_HIST_SAMPLE_INDEX_MIN         MIN_portHistCurrentSampleIndex
#define NMTR_TYPE_HIST_SAMPLE_INDEX_MAX         MAX_portHistCurrentSampleIndex

enum NMTR_TYPE_HistCtrlField_E {
    NMTR_TYPE_HIST_CTRL_FIELD_INDEX = 1,
    NMTR_TYPE_HIST_CTRL_FIELD_NAME,
    NMTR_TYPE_HIST_CTRL_FIELD_DATA_SOURCE,
    NMTR_TYPE_HIST_CTRL_FIELD_INTERVAL,
    NMTR_TYPE_HIST_CTRL_FIELD_BUCKETS_REQUESTED,
    NMTR_TYPE_HIST_CTRL_FIELD_BUCKETS_GRANTED,
    NMTR_TYPE_HIST_CTRL_FIELD_STATUS,
};

enum NMTR_TYPE_HistCtrlStatus_E {
    NMTR_TYPE_HIST_CTRL_STATUS_NOT_EXIST        = 0,
    NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE           = VAL_portHistControlStatus_active,
    NMTR_TYPE_HIST_CTRL_STATUS_NOT_IN_SERVICE   = VAL_portHistControlStatus_notInService,
    NMTR_TYPE_HIST_CTRL_STATUS_NOT_READY        = VAL_portHistControlStatus_notReady,
    NMTR_TYPE_HIST_CTRL_STATUS_CREATE_AND_GO    = VAL_portHistControlStatus_createAndGo,
    NMTR_TYPE_HIST_CTRL_STATUS_CREATE_AND_WAIT  = VAL_portHistControlStatus_createAndWait,
    NMTR_TYPE_HIST_CTRL_STATUS_DESTROY          = VAL_portHistControlStatus_destroy,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef union
{
    UI32_T basis_point;             /* in 0.01% */
    UI64_T xmit_bytes;              /* for internal use */
} NMTR_TYPE_HistUtilization_T;

typedef struct
{
    UI32_T start_time;      /* in ticks */
    UI32_T interval;        /* in ticks */
    UI64_T ifInOctets;
    UI64_T ifInUcastPkts;
    UI64_T ifInMulticastPkts;
    UI64_T ifInBroadcastPkts;
    UI64_T ifInDiscards;
    UI64_T ifInErrors;
    UI64_T ifInUnknownProtos;
    UI64_T ifOutOctets;
    UI64_T ifOutUcastPkts;
    UI64_T ifOutMulticastPkts;
    UI64_T ifOutBroadcastPkts;
    UI64_T ifOutDiscards;
    UI64_T ifOutErrors;
    NMTR_TYPE_HistUtilization_T ifInUtilization;
    NMTR_TYPE_HistUtilization_T ifOutUtilization;
} NMTR_TYPE_HistCounterInfo_T;

typedef struct
{
    UI32_T ctrl_idx;        /* key */
    UI32_T data_source;     /* ifindex */
    UI32_T interval;        /* in seconds */
    UI32_T buckets_requested;
    UI32_T buckets_granted;
    UI32_T status;
    UI32_T refresh_time;    /* in ticks */
    char name[NMTR_TYPE_HIST_CTRL_NAME_LEN_MAX+1];
} NMTR_TYPE_HistCtrlInfo_T;

typedef struct
{
    UI32_T ctrl_idx;        /* key */
    UI32_T sample_idx;      /* key */
    NMTR_TYPE_HistCounterInfo_T counter;
} NMTR_TYPE_HistSampleEntry_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* NMTR_TYPE_H */

