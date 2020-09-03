/* static char SccsId[] = "+-<>?!NTP_TYPE.H   22.1  05/05/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_TYPE.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */

#ifndef _NTP_TYPE_H
#define _NTP_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "ntp_dbg.h"

/* NAMING CONSTANT DECLARATIONS
 */




/* TYPE DECLARATIONS
 */

#define NTP_TASK_EVENT_SENDPACKET                  BIT_3   /* Backdoor use to send ntp packet */
#define NTP_TASK_EVENT_TIMER_ALARM_TWENTY_TICK     BIT_1   /* Backdoor use to send ntp packet */
#define NTP_TYPE_EVENT_TIMER_INTERVAL              20
#define NTP_TYPE_EVENT_TIMER                       BIT_2

/* Define NTP packet operation mode
*/
#define VAL_ntpServiceMode_broadcast  2
#define VAL_ntpServiceMode_anycast    3
#define VAL_ntpServerKey_no           0

typedef enum
{
    NTP_UNICAST_MODE = VAL_ntpServiceMode_unicast,
    NTP_BROCAST_MODE = VAL_ntpServiceMode_broadcast,
    NTP_ANYCAST_MODE = VAL_ntpServiceMode_anycast

}NTP_OPERATION_MODE_E;

typedef enum
{
    NTP_TYPE_ENABLE = VAL_ntpStatus_enabled,
    NTP_TYPE_DISABLE = VAL_ntpStatus_disabled
} NTP_SERVICE_E;


typedef enum
{
    NTP_TYPE_AUTHENABLE = VAL_ntpAuthenticateStatus_enabled,
    NTP_TYPE_AUTHDISABLE = VAL_ntpAuthenticateStatus_disabled
} NTP_AUTHENTICATE_E;

/* Define the return error code
 */
typedef enum
{
    NTP_MSG_FAIL        = 0,
    NTP_MSG_SUCCESS     = 1,
    NTP_MSG_NoSupport   = 2,
    NTP_MSG_NoBuffer    = 3,
    NTP_MSG_NoData      = 4,
    NTP_MSG_NoFound     = 5,
    NTP_MSG_NoChange    = 6,
    NTP_MSG_UnknowError = 7,
    NTP_MSG_ExceedRange = 8
}NTP_MSG_E ;

typedef struct
{
    UI32_T ipaddress;
    UI32_T priority;
    UI32_T version;
    UI32_T rowStatus;
} NTP_SERVER_ENTRY_T;

typedef struct
{
    UI32_T config_mode;
    UI32_T polling_interval;
} NTP_SERVER_CONFIG_T;


#endif
