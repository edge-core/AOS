/*-----------------------------------------------------------------------------
 * FILE NAME: CWMP_TYPE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Declares named constants and data structures used by CWMP and other
 *    CSCs.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/12/24     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef CWMP_TYPE_H
#define CWMP_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define CWMP_TYPE_STR_LEN_256       256
#define CWMP_TYPE_STR_LEN_32        32
#define CWMP_TYPE_MAX_DATATIME_LEN  20
#define CWMP_TYPE_MIN_INTERVAL      1
#define CWMP_TYPE_MAX_INTERVAL      4294967295  /* 2^32 - 1 */

#define CWMP_TYPE_EVENT_NONE                    0
#define CWMP_TYPE_EVENT_ENTER_TRANSITION        BIT_0
#define CWMP_TYPE_EVENT_ENTER_MASTER            BIT_1
#define CWMP_TYPE_EVENT_PERIODIC_INFORM         BIT_2
#define CWMP_TYPE_EVENT_TIMER                   BIT_3
#define CWMP_TYPE_EVENT_CONNECTION_REQUEST      BIT_4
#define CWMP_TYPE_EVENT_IP_ACTIVE               BIT_5


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    char    url[CWMP_TYPE_STR_LEN_256+1];
    char    username[CWMP_TYPE_STR_LEN_256+1];
    BOOL_T  periodic_inform_enable;
    UI32_T  periodic_inform_interval;
    char    conn_req_username[CWMP_TYPE_STR_LEN_256+1];
} CWMP_TYPE_ConfigEntry_T;


#endif /* #ifndef CWMP_TYPE_H */
