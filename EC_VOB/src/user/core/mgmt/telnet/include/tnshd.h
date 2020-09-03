/* Module Name:	TNSHD.H
 * Purpose:
 *		TNSHD, TelNet SHell Daemon, a responser for telnet request.
 *		Telnet daemon needs a task to handle command request, this TNSHD
 *		take care of all request from telnet client.
 *
 * Notes:
 *
 * History:
 *		 Date		--  Modifier,  Reason
 *	0.1	2001.11.17	--	William,	Created.
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */
#ifndef		_TNSHD_H
#define		_TNSHD_H

/* INCLUDE FILE	DECLARATIONS
 */
#include "pshcfg.h"


/* NAME	CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION	NAME : TNSHD_CreateTask
 * PURPOSE:
 *		Create TelNet SHell Daemon task. This task is waiting request
 *		and take any request on the servce-socket.
 *
 * INPUT:
 *		cfg	--	system configuration information.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		0		--	TNSHD task created.
 *		-1		--	error.
 * NOTES:
 *			(  Something must be known to use this function. )
 */
int TNSHD_CreateTask(pshcfg_t *cfg);

void TNSHD_ProvisionComplete();

void TNSHD_EnterTransitionMode(void);

#endif	 /*	TNSHD_H */
