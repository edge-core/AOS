/************************************************************************/
/*                                                                      */
/*   MODULE: pshcfg.h                                                   */
/*   PRODUCT: pSH+                                                      */
/*   PURPOSE: pSH+ login/server                                         */
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
#ifndef _pshcfg_h
#define _pshcfg_h

#ifndef ulist_t
struct ulist_t {
    char  *login_name;     /* user login name */
    char  *login_passwd;   /* user login password */
    long  reserved[4];     /* reserved for future use, must be 0 */
};
typedef struct ulist_t ulist_t;
#endif

struct appdata_t {
    char  *app_name;       /* application name */
    char  *app_help;       /* help string */
    void  (*app_entry)();  /* entry point */
    char  *app_tname;      /* task name */
    long  app_tprio;       /* task priority */
    long  app_sssize;      /* system stack size */
    long  app_ussize;      /* user stack size */
    unsigned short app_reentrant_flag;  /* reentrant flag */
    unsigned short app_reentrant_lock;  /* reentrant lock */
};
typedef struct appdata_t appdata_t;

struct cmddata_t {
    char  *cmd_name;       /* command name */
    char  *cmd_help;       /* help string */
    void  (*cmd_entry)();  /* entry point */
    unsigned short cmd_reentrant_flag;  /* reentrant flag */
    unsigned short cmd_reentrant_lock;  /* reentrant lock */
};
typedef struct cmddata_t cmddata_t;

struct pshcfg_t {
    long      flag;              /* services options */
    long      task_prio;         /* priority for pshd task */
    char      *def_vol_name;     /* name of the default login volume */
    ulist_t   *ulist;            /* ptr to the list of trusted users */
    appdata_t *app;              /* ptr to the list of applications */
    cmddata_t *cmd;              /* ptr to the list of commands */
    unsigned long console_dev;   /* psh console device number */
    long      reserved[3];       /* reserved for future use, must be 0 */
};
typedef struct pshcfg_t pshcfg_t;

#endif

/*
 * CHANGE LOG:
 *     Created August 5, 1992 by Tuyen Nguyen
 */
