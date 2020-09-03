/*
 * $Id: options.h,v 1.6 1998/06/28 00:08:17 lf Exp $
 *
 * Copyright (C) 1996 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#define OPTION_LEN	64

/* ids for different option types */
#define OT_STR		(1<<0)	  /* string */
#define OT_INT		(1<<1)	  /* integer */
#define OT_SRV		(1<<2)	  /* server list */
#define OT_AUO		(1<<3)    /* authentication order */

#define OT_ANY		((unsigned int)~0) /* used internally */

/* status types */
#define ST_UNDEF	(1<<0)	  /* option is undefined */

typedef struct _option {
	char name[OPTION_LEN];	  /* name of the option */
	int type, status;	  /* type and status    */
	void *val;		  /* pointer to option value */
} OPTION;



