/* static char SccsId[] = "+-<>?!SNTP_TYPE.H   22.1  05/05/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_TYPE.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  05-05-2002   Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */

#ifndef	_SNTP_TYPE_H
#define	_SNTP_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sntp_dbg.h"
#include "leaf_es3626a.h"
#include "leaf_a3com515sntp.h"
#include "l_inet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SNTP_WAIT_TIMEOUT                       3   /* Seconds before resending       */

#define SNTP_1900_TO_1970_SECS  	            0x83aa7e80  /* 1970 - 1900 in seconds */
#define SNTP_1900_TO_1990_SECS                  0xa9491c00 	/* Seconds between 1900 and 1990*/
#define SNTP_1900_TO_2001_SECS     	            0xBDFA4700	/* Seconds between 1900 and 2001*/

#define SNTP_MAX_SERVER                         3   /* Define the max. SNTP server number  in database*/

 /*Define the default value of SNTP
  */
#if (SYS_CPNT_A3COM515_SNTP_MIB==TRUE)
    #define SNTP_DEFAULT_POLLINGTIME		    360    	/* Minutes */
#else
    #define SNTP_DEFAULT_POLLINGTIME		    16    	/* Seconds */
#endif

#if (SYS_CPNT_A3COM515_SNTP_MIB==TRUE)
    #define SNTP_MAX_POLLTIME				    MAX_sntpPollInterval3com			/* Max polling time for 3com*/
    #define SNTP_MIN_POLLTIME				    MIN_sntpPollInterval3com				/* Min polling time for 3com*/
#else
    #define SNTP_MAX_POLLTIME				    MAX_sntpPollInterval			/* Max polling time */
    #define SNTP_MIN_POLLTIME				    MIN_sntpPollInterval				/* Min polling time */
#endif

#define SNTP_DEFAULT_OPERATIONMODE		        VAL_sntpServiceMode_unicast			   /* unicast mode */
#define SNTP_DEFAULT_STATUS			            VAL_sntpStatus_disabled
#define SNTP_DEFAILT_SERVER_IP                          0x0A010001   /* 10.1.0.1 */

/* TYPE DECLARATIONS
 */
 /* Define SNTP packet operation mode
  */
typedef enum
{
	UNICAST_MODE = VAL_sntpServiceMode_unicast,
	BROCAST_MODE = VAL_sntpServiceMode_broadcast,
	ANYCAST_MODE = VAL_sntpServiceMode_anycast
} SNTP_OPERATION_MODE_E;

typedef enum
{
    ENABLE      = VAL_sntpStatus_enabled,
    DISABLE     = VAL_sntpStatus_disabled
} SNTP_SERVICE_E;

typedef enum
{
    WAS_CLEARED = 0,
    NO_RESPONSE,
    ACTIVE
} SNTP_SERVER_STATUS_E;

/* Define the return error code */
typedef enum
{
    SNTP_MSG_FAIL       = 0,
    SNTP_MSG_SUCCESS    = 1,
    SNTP_MSG_NoSupport  = 2,
    SNTP_MSG_NoBuffer   = 3,
    SNTP_MSG_NoData     = 4,
    SNTP_MSG_NoFound    = 5,
    SNTP_MSG_NoChange   = 6,
    SNTP_MSG_UnknowError= 7,
    SNTP_MSG_ExceedRange= 8
} SNTP_MSG_E;

typedef enum
{
    SERVICE_MODE        = 0,
    POLLING_TIME
} SNTP_PARAMETER_CHANGED_E;

typedef struct
{
    L_INET_AddrIp_T ipaddress;
    UI32_T  priority;
    UI32_T  version;
    UI32_T  rowStatus;
} SNTP_SERVER_ENTRY_T;

typedef struct
{
    UI32_T  config_mode;
    UI32_T  polling_interval;
} SNTP_SERVER_CONFIG_T;

typedef struct
{
    UI32_T                  Current_time;
    UI32_T                  Last_SNTP_Update;
    UI32_T                  Poll_inteval;
    UI32_T                  From_server;
    SNTP_OPERATION_MODE_E   Current_mode;
} SNTP_INFO_T;

typedef struct
{
    L_INET_AddrIp_T         Current_server;
    UI32_T                  Current_time;
    SNTP_SERVER_STATUS_E    Server_Status;
} SNTP_UPDATE_STATUS_T;

#endif

