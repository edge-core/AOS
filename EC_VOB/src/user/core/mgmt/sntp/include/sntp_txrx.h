/* static char SccsId[] = "+-<>?!SNTP_TXRX.H   22.1  22/04/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_TXRX.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  04-22-2002  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */
#ifndef	_SNTP_TXRX_H
#define	_SNTP_TXRX_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sntp_dbg.h"
#include "sntp_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* constants used by the NTP packet. See RFC 1769 for details. */

/* 2 bit leap indicator field */

#define SNTP_LI_MASK    0xC0
#define SNTP_LI_0       0x00        /* no warning */
#define SNTP_LI_1       0x40        /* last minute has 61 seconds */
#define SNTP_LI_2       0x80        /* last minute has 59 seconds */
#define SNTP_LI_3       0xC0        /* alarm condition (clock not synchronized) */

/* 3 bit version number field */
#define SNTP_VN_MASK    0x38
#define SNTP_VN_0       0x00        /* not supported */
#define SNTP_VN_1       0x08        /* the earliest version */
#define SNTP_VN_2       0x10
#define SNTP_VN_3       0x18
#define SNTP_VN_4       0x20
#define SNTP_VN_5       0x28        /* reserved */
#define SNTP_VN_6       0x30        /* reserved */
#define SNTP_VN_7       0x38        /* reserved */

/* 3 bit mode field */
#define SNTP_MODE_MASK  0x07
#define SNTP_MODE_0     0x00        /* reserve */
#define SNTP_MODE_1     0x01        /* symmetric active */
#define SNTP_MODE_2     0x02        /* symmetric passive */
#define SNTP_MODE_3     0x03        /* client */
#define SNTP_MODE_4     0x04        /* server */
#define SNTP_MODE_5     0x05        /* broadcast */
#define SNTP_MODE_6     0x06        /* reserve for NTP control message */
#define SNTP_MODE_7     0x07        /* reserve for private use */

#define SNTP_DRIFT_TOLERANCE    3   /* Seconds */
#define SNTP_PORT               123 /* NTP/SNTP udp port */

typedef struct sntpPacket
{
    UI8_T   leapVerMode;
    UI8_T   stratum;
    char    poll;
    char    precision;
    UI32_T  rootDelay;
    UI32_T  rootDispersion;
    UI32_T  referenceIdentifier;

    /* latest available time, in 64-bit NTP timestamp format */
    UI32_T  referenceTimestampSec;
    UI32_T  referenceTimestampFrac;

    /* client transmission time, in 64-bit NTP timestamp format */
    UI32_T  originateTimestampSec;
    UI32_T  originateTimestampFrac;

    /* server reception time, in 64-bit NTP timestamp format */
    UI32_T  receiveTimestampSec;
    UI32_T  receiveTimestampFrac;

    /* server transmission time, in 64-bit NTP timestamp format */
    UI32_T  transmitTimestampSec;
    UI32_T  transmitTimestampFrac;
}__attribute__((packed, aligned(1))) SNTP_PACKET;

typedef enum
{
    SNTP_TXRX_MSG_SUCCESS           = 0,
    SNTP_TXRX_MSG_INVALID_PARAMETER,
    SNTP_TXRX_MSG_FAIL,
    SNTP_TXRX_MSG_TIMEOUT
} SNTP_TXRX_STATUS_E;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Tx/Rx initialize   Initialize receive local port,version and mode of sntp
 *			  and initialize socket
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_TXRX_Init(void);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_ReInit
 *------------------------------------------------------------------------------
 * PURPOSE  : Tx/Rx Reinitialize   socket ,version number, packet mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_TXRX_ReInit(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_Client
 *------------------------------------------------------------------------------
 * PURPOSE  : Using this routine as SNTP brocast/unicast client
 * INPUT    : ServerIpaddress: 1.if 0 then, implies brocast mode
 *							   2.input must be network order
 *			  Delaytime : Timeout when wait a response from server , in seconds.
 * OUTPUT   : ServerIpaddress : 1.If input is 0 ,then if it has received broadcast sntp packet form
 *								  server , fill with the ipaddress.
 *								2.time : Total seconds since 1900/01/01 00:00 from SNTP server.
 *							    3.tick : Current tick of time.
 * RETURN   : SNTP_TXRX_MSG_SUCCESS : Receive successfully,
 *			  SNTP_TXRX_MSG_INVALID_PARAMETER
 *			  SNTP_TXRX_MSG_FAIL
 *			  SNTP_TXRX_MSG_TIMEOUT
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SNTP_TXRX_STATUS_E  SNTP_TXRX_Client(L_INET_AddrIp_T *ServerIpaddress,
                                     UI32_T  Delaytime,
                                     UI32_T  *time,
                                     UI32_T  *tick);

/* Debug use */
void SNTP_TXRX_SetLocalPort(unsigned int LocalPort);
void SNTP_TXRX_SetVersion(UI8_T Version);

#endif

