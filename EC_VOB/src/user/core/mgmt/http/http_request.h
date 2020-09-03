/* ----------------------------------------------------------------------
 * FILE NAME: http\htReqest.h

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for core of HTTP server.
 * Include file for the "Process Request" task.

 * HISTORY:
 * 1997-04-30 (Fri): Modified by Daniel K. Chung.
 * 1998-04-30 (Thu): Removed "httpcfg" (HTTP_CFG).
 * 1998-05-08 (Fri): Name changed from httpmisc.h to htReqest.h.
 * 1999-04-28 (Wed): Use function call to process request (HTTP_REQTASK).

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

/***********************************************************************/
/*                                                                     */
/*   MODULE: sys\libc\src\http\httpmisc.h                              */
/*   DATE:   96/04/23                                                  */
/*   PURPOSE:                                                          */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*          Copyright 1991 - 1993, Integrated Systems, Inc.            */
/*                      ALL RIGHTS RESERVED                            */
/*                                                                     */
/*   Permission is hereby granted to licensees of Integrated Systems,  */
/*   Inc. products to use or abstract this computer program for the    */
/*   sole purpose of implementing a product based on Integrated        */
/*   Systems, Inc. products.   No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in       */
/*   whole, are granted.                                               */
/*                                                                     */
/*   Integrated Systems, Inc. makes no representation or warranties    */
/*   with respect to the performance of this computer program, and     */
/*   specifically disclaims any responsibility for any damages,        */
/*   special or consequential, connected with the use of this program. */
/*                                                                     */
/***********************************************************************/


#ifndef _HTREQEST_H
#define _HTREQEST_H

#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

void http_process_request(HTTP_Event_T *event);
void http_request_finalize(HTTP_Event_T *event);

HTTP_Request_T * http_request_new();

void http_request_free(HTTP_Request_T **req_pp);

void http_perror(HTTP_Request_T *req, char *str);

/* parameters */
#define HTTP_REQ_POSTSIZE 20480


#if __cplusplus
}
#endif

#endif/* _HTREQEST_H*/

