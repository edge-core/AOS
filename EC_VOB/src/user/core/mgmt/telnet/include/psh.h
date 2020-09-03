/************************************************************************/
/*                                                                      */
/*   MODULE: psh.h                                                      */
/*   PRODUCT: pSH+                                                      */
/*   PURPOSE:                                                           */
/*   DATE:  08/17/1992                                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*              Copyright 1992, Integrated Systems Inc.                 */
/*                      ALL RIGHTS RESERVED                             */
/*                                                                      */
/*   This computer program is the property of Integrated Systems Inc.   */
/*   Santa Clara, California, U.S.A. and may not be copied              */
/*   in any form or by any means, whether in part or in whole,          */
/*   except under license expressly granted by Integrated Systems Inc.  */
/*                                                                      */
/*   All copies of this program, whether in part or in whole, and       */
/*   whether modified or not, must display this and all other           */
/*   embedded copyright and ownership notices in full.                  */
/*                                                                      */
/************************************************************************/
/*
 *	Modification History :
 *	2001.10.28, William, mask-off some include-files used for pSos.
 *				Due to system function standardized, we use sysfun.h
 *				instead of pSos.h, prepc.h, phile.h,...
 */
#ifndef _psh_h
#define _psh_h

#include "nudefs.h"
#include "ctype.h"
#include "sysfun.h"         /*  2001.10.28, William, change from psos.h */
/*
#include "stdio.h"
*/
/*	2001.10.28, William, mask-off for porting to VxWorks system.
 *	following include-files are used in pSos environment.
 *
 *	#include "drv_intf.h"	
 *	#include "psysses.h"
 *	#include "prepc.h"
 *	#include "pna.h"
 *	#include "phile.h"
 *	#include "psys.h"
 */
#include "pshcfg.h"

#ifndef u_char
#define u_char unsigned char
#define u_short unsigned short
#define u_int unsigned int
#define u_long unsigned long
#endif

#ifndef NULL
#define NULL    0
#endif

#define NODE_LEN            12
#define PSH_SSSIZE          0x1000
#define PSH_USSIZE          0x2000

#define PSH_MAX_PATHLEN     256
#define PSH_MAX_ARGV        32
#define PSH_MAX_ENV         32
#define PSH_MAX_ARGVLEN     256
#define PSH_MAX_ENVLEN      512

struct tasktype_t {
    int  argc;
    char *argv[PSH_MAX_ARGV];
    char *env[PSH_MAX_ENV];
    char argvBuf[PSH_MAX_ARGVLEN];
    char envBuf[PSH_MAX_ENVLEN];
    unsigned long tid;
    unsigned long ptid;
    void (*app_entry)();
    char *infile;
    char *outfile;
    int  append;
    int  sin;
    int  sout;
    struct tasktype_t *next;
};
typedef struct tasktype_t tasktype_t;

#endif

/*
 * CHANGE LOG:
 *     Created Aug 17, 1992 by Tuyen Nguyen
 */
