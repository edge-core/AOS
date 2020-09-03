/* ----------------------------------------------------------------------
 * FILE NAME: http\htUtil.c

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for external interface of HTTP server.
 * HTTP utilities.

 * CLASSES AND FUNCTIONS:
 * http_cenvertDec2Str: Convert integer to string.
 * http_recv: Non-blocking version of "receive".
 * http_send: Non-blocking version of "send".

 * HISTORY:
 * 1998-11-26 (Tue): Created by Zhong Qiyao.
 * 1998-11-26 (Tue): http_convertDec2Str, Jon-Shan Shei (Xie Zongxian).
 * 1998-11-26 (Tue): http_send, http_recv, Grace Chung (Zhong Sumei).
 * 1999-04-28 (Wed): Decreased time-out value.

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

#include "http_loc.h"

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
// FIXME: Just passing the bfd, not whole connection
int HTTP_UTIL_Read(HTTP_Connection_T *http_connection, int sockfd, char *buf, int len, int flags)
{
    int rc;

    ASSERT(http_connection != NULL);
    if (http_connection == NULL)
    {
        return -1;
    }

    rc = HTTP_readsocket(&http_connection->req->bfd, buf, len, flags);
    return rc;
}


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
// FIXME: Just passing the bfd, not whole connection
int HTTP_UTIL_Write(HTTP_Connection_T *http_connection, int sockfd, char *buf, int len, int flags)
{
    int rc;

    ASSERT(http_connection != NULL);
    if (http_connection == NULL)
    {
        return -1;
    }

    rc = HTTP_writesocket(&http_connection->res->bfd, buf, len, flags);
    return rc;
}



