/* ----------------------------------------------------------------------
 * FILE NAME: http\htFilles.h

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for core of HTTP server.
 * Include file for the "Fileless" function.

 * HISTORY:
 * 1997-05-08 (Fri): Modified by Daniel K. Chung.
 * 1998-05-08 (Fri): Name changed from fileless.h to htFilles.h.
 * 1999-04-28 (Wed): Use function call to process request (HTTP_REQTASK).

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

#ifndef _HTTP_HEADER_FILE_H_
#define _HTTP_HEADER_FILE_H_

#include "http_type.h"
#include "l_lib.h"

#ifdef PLATFORM_UNIX
#  include "http_file_unix.h"
#endif

//#ifdef PLATFORM_WINDOWS
//#  include "http_file_win.h"
//#endif

#if __cplusplus
extern "C" {
#endif


#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_FILE_H_ */

////////////////////////////////////////////////////////////////////////////////

#ifndef _HTFILLES_H
#define _HTFILLES_H
#include "http_type.h"
#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

#define http_fileless1  cgi_main

#if __cplusplus
}
#endif

#endif /* _HTFILLES_H */

