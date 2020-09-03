/************************************************************************/
/*                                                                      */
/*   MODULE: psysses.h                                                  */
/*   PRODUCT: pSH+                                                      */
/*   PURPOSE:                                                           */
/*   DATE:  08/05/1992                                                  */
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
#ifndef _psysses_h
#define _psysses_h

struct psysses_t {
    int  sin;              /* in socket */
    int  sout;             /* out socket */
    int  pvol_type;        /* previous type of volume */
    char pvol[12];         /* previous working volume */
    char pdir[256];        /* previous working directory */
    int  vol_type;         /* type of volume */
    char cvol[12];         /* present working volume */
    char cdir[256];        /* present working directory */
    char logname[64];      /* login name */
    char term[64];         /* terminal type */
};
typedef struct psysses_t psysses_t;

#define S_IFMT     0170000
#define S_IFREG    0100000
#define S_IFDIR    0040000
#define S_IFLNK    0120000

struct  ttychars {
    char    tc_erase;   /* erase last character */
    char    tc_kill;    /* erase entire line */
    char    tc_intrc;   /* interrupt */
    char    tc_quitc;   /* quit */
    char    tc_startc;  /* start output */
    char    tc_stopc;   /* stop output */
    char    tc_eofc;    /* end-of-file */
    char    tc_brkc;    /* input delimiter (like nl) */
    char    tc_suspc;   /* stop process signal */
    char    tc_dsuspc;  /* delayed stop process signal */
    char    tc_rprntc;  /* reprint line */
    char    tc_flushc;  /* flush output (toggles) */
    char    tc_werasc;  /* word erase */
    char    tc_lnextc;  /* literal next character */
};

#endif
/*
 * CHANGE LOG:
 *     Created Aug 5, 1992 by Tuyen Nguyen
 */
