/* Module Name:	DHCP_TIME.H
 * Purpose:
 *		
 *
 * Notes:
 *		1. 
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.24	--	William, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */



#ifndef		_DHCP_TIME_H
#define		_DHCP_TIME_H


/* INCLUDE FILE	DECLARATIONS
 */
#include "dhcp_type.h"
#include "sys_bld.h"

/* NAME	CONSTANT DECLARATIONS
 */
#define DHCP_TIME_MAX_SYSTEM_SECONDS      (0xFFFFFFA0/SYS_BLD_TICKS_PER_SECOND)

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */



/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION	NAME : DHCP_TIME_Init
 * PURPOSE:
 *		Initialize DHCP_TIME software components.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void	DHCP_TIME_Init (void);

/* FUNCTION	NAME : DHCP_TIME_ClearList
 * PURPOSE:
 *		ReInit whole dhcp timer
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		Penny added for free the DHCP timer 2002/2/9
 */
void DHCP_TIME_ClearList(void);


/* FUNCTION	NAME : DHCP_TIME_add_timeout
 * PURPOSE:
 *		Add a time_out request to time out list.
 *
 * INPUT:
 *		when  -- the time stamp to be timeout.
 *		where -- the function be called when timeout.
 *		what  -- the pointer passed to where when timeout.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. the timeout list is sorted by when, the time stamp.
 *		2. timestamp is absolut time from system started.
 */
void DHCP_TIME_add_timeout (DHCP_TIME when, DHCP_FUNC where, void *what);


/* FUNCTION	NAME : DHCP_TIME_cancel_timeout
 * PURPOSE:
 *		Remove time_out request from time out list.
 *
 * INPUT:
 *		where -- the function be called when timeout.
 *		what  -- the pointer passed to where when timeout.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. Both value, where and what are matched, the request will be removed.
 */
void DHCP_TIME_cancel_timeout (DHCP_FUNC where, void *what);
void DHCP_TIME_Cancel_interface_timeout (void *what);


/* FUNCTION	NAME : DHCP_TIME_IsTimeout
 * PURPOSE:
 *		If any request is timeout in time out list,
 *		return the (where, what), one by one.
 *
 * INPUT:
 *		time_seconds -- system ticks from system started.
 *
 * OUTPUT:
 *		where -- the function be called when timeout.
 *		what  -- the pointer passed to where when timeout.
 *
 * RETURN:
 *		TRUE	-- there is a timeout-ed request in waiting list.
 *		FALSE	-- no (or not timeout) timeout-ed request in list.
 *
 * NOTES:
 *		1. Both value, where and what are matched, the request will be removed.
 */
BOOL_T	DHCP_TIME_IsTimeout (TIME time_seconds, DHCP_FUNC *where, void **what);

/* FUNCTION	NAME : DHCP_TIME_ReCalTimeout
 * PURPOSE:
 *		Recalculate timeout value in timeout list
 *
 * INPUT:
 *		time_seconds .
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		If timeout value larger than maximum value of system ticks(0xFFFFFFA0),
 *      all corresponding timeout value should be recalculated.
 */
void DHCP_TIME_ReCalTimeout(TIME time_seconds);

#endif	 /*	_DHCP_TIME_H */
