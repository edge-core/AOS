/************************************************************************/
/*                                                                      */
/*   MODULE: tnpdcfg.h                                                  */
/*   PRODUCT: pTEL+                                                     */
/*   PURPOSE: TELNET server                                             */
/*   DATE:  08/20/1992                                                  */
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
#ifndef _tnpdcfg_h
#define _tnpdcfg_h

struct tnpcfg_t {
    long  task_prio;    /* priority for tnpd task */
    long  max_sessions; /* max. # of concurrent sessions */
    char  **hlist;      /* ptr to the list of trusted clients */
    long  reserved[2];  /* reserved for future use, must be 0 */
};
typedef struct tnpcfg_t tnpcfg_t;

#endif

/*
 * CHANGE LOG:
 *     Created Aug 20, 1992 by Tuyen Nguyen
 */
