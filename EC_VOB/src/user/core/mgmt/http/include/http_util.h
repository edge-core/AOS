/* ----------------------------------------------------------------------
 * FILE NAME: http\htUtil.h

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for core of HTTP server.
 * HTTP utilities.

 * HISTORY:
 * 1998-11-28 (Tue): Created by Daniel K. Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

#ifndef _HTUTIL_H
#define _HTUTIL_H

#if __cplusplus
extern "C" {
#endif


/* ----------------------------------------------------------------------
 * HTTP_UTIL_Read: Non-blocking version of "receive".

 * Input:
 * s: Socket.
 * buf: Buffer.
 * len: How many to receive.
 * flags: Passed to "recv".

 * Output:
 * -1: Error.
 * >=0: How many bytes recieved.
 * ---------------------------------------------------------------------- */
int HTTP_UTIL_Read(HTTP_Connection_T *http_connection, int sockfd, char *buf, int len, int flags);



/* ----------------------------------------------------------------------
 * HTTP_UTIL_Write: "send" data out.

 * Input:
 * s: Socket.
 * buf: Buffer.
 * len: How many to send.
 * flags: Passed to "send".

 * Output:
 * -1: Error.
 * >=0: How many bytes send.
 * ---------------------------------------------------------------------- */
int HTTP_UTIL_Write(HTTP_Connection_T *http_connection, int sockfd, char *buf, int len, int flags);


#if __cplusplus
}
#endif

#endif /* _HTUTIL_H */