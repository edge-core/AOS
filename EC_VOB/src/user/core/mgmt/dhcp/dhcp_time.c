/* Module Name: DHCP_TIME.C
 * Purpose:
 *      .
 *
 * Notes:
 *      .
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_mpool.h"
#include "dhcp_time.h"
#include "dhcp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define	DHCP_TIME_TIMEOUT_REQUEST_NBR		32	/*	SYS_BLD_DEFAULT_MSG_NBR	*/


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct	DHCP_TIME_TimeOutInfo_S
{
	struct	DHCP_TIME_TimeOutInfo_S	*next;
	DHCP_TIME	when;			/*	timeout in system time	*/
	DHCP_FUNC	func;			/*	handling routine when timeout	*/
	void		*what;			/*	parameter passing to handling routine	*/
}	DHCP_TIME_TimeOutInfo_T;


typedef struct	DHCP_TIME_LibControlBlock_S
{
	DHCP_TIME_TimeOutInfo_T	*timeout_list;
	UI32_T					timeout_request_no;
	L_MPOOL_HANDLE			req_pool;
}	DHCP_TIME_LibControlBlock_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */



/* STATIC VARIABLE DECLARATIONS
 */
static	DHCP_TIME_LibControlBlock_T		dhcp_time_lcb;


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION	NAME : DHCP_TIME_Init
 * PURPOSE:
 *		Initialize DHCP_TIME.
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
void	DHCP_TIME_Init (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    memset ((char*)&dhcp_time_lcb, 0, sizeof(DHCP_TIME_LibControlBlock_T));
    /*	Create request pool to avoid fragmentation	*/
    dhcp_time_lcb.req_pool = L_MPOOL_CreateBlockPool (sizeof(DHCP_TIME_TimeOutInfo_T),DHCP_TIME_TIMEOUT_REQUEST_NBR);
    if (0 == dhcp_time_lcb.req_pool)
    {
    	/*	log message to system, no more space	*/
    	return;
    }
}	/*	end of DHCP_TIME_Init	*/

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
void DHCP_TIME_ClearList(void)
{
	 /* LOCAL VARIABLES DEFINITION
     */
	DHCP_TIME_TimeOutInfo_T *q;

    /* BODY */

	/*for (q = dhcp_time_lcb.timeout_list; q;)
	{
		dhcp_time_lcb.timeout_list = q -> next;
		L_MPOOL_FreeBlock(dhcp_time_lcb.req_pool,q);
	}
	*/
	q = dhcp_time_lcb.timeout_list;
	while(dhcp_time_lcb.timeout_list)
	{
		dhcp_time_lcb.timeout_list = q -> next;
		L_MPOOL_FreeBlock(dhcp_time_lcb.req_pool,q);
		q = dhcp_time_lcb.timeout_list;
	}
	dhcp_time_lcb.timeout_list = NULL;
}

/* FUNCTION	NAME : DHCP_TIME_add_timeout
 * PURPOSE:
 *		Add a time_out request to time out list.
 *
 * INPUT:
 *		when  -- the time stamp to be timeout .
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
 *		1. the timeout list is sorted by when (in second), the time stamp.
 *		2. timestamp is absolut time from system started.
 */
void DHCP_TIME_add_timeout (DHCP_TIME when, DHCP_FUNC where, void *what)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	DHCP_TIME_TimeOutInfo_T *t, *q;
    /* BODY */
    /*	Is the function already hook ?
     *
     */

	/* See if this timeout supersedes an existing timeout. */
	t = (DHCP_TIME_TimeOutInfo_T*)0;
	for (q = dhcp_time_lcb.timeout_list; q; q = q -> next) {
		if (q -> func == where && q -> what == what) {
			if (t)
				t -> next = q -> next;
			else
				dhcp_time_lcb.timeout_list = q -> next;
			break;
		}
		t = q;
	}

	/* If we didn't supersede a timeout, allocate a timeout
	   structure now. */
	if (!q) {
	    if (dhcp_time_lcb.req_pool != 0)
	    {
	    	if (L_MPOOL_AllocateBlock (dhcp_time_lcb.req_pool, (void**)&q))
	    	{
				q -> func = where;
				q -> what = what;
				dhcp_time_lcb.timeout_request_no++;
			}
			else
			{
				/* Log message to system : no more space	*/
				/* Maybe need DHCP_MGR_Restart */
				return;

			}
	    }
	}

	q -> next = NULL;
	q -> when = when; /* Penny 1/11/2002 */

	/* Now sort this timeout into the timeout list. */

	/* Beginning of list? */
	if (!dhcp_time_lcb.timeout_list || dhcp_time_lcb.timeout_list -> when > q -> when) {
		q -> next = dhcp_time_lcb.timeout_list;
		dhcp_time_lcb.timeout_list = q;
		return;
	}

	/* Middle of list? */
	for (t = dhcp_time_lcb.timeout_list; t -> next; t = t -> next) {
		if (t -> next -> when > q -> when) {
			q -> next = t -> next;
			t -> next = q;
			return;
		}
	}

	/* End of list. */
	t -> next = q;
	q -> next = (DHCP_TIME_TimeOutInfo_T *)0;


}	/*	end of DHCP_TIME_add_timeout	*/


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
void DHCP_TIME_cancel_timeout (DHCP_FUNC where, void *what)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	DHCP_TIME_TimeOutInfo_T *t, *q;
    /* BODY */

	/* Look for this timeout on the list, and unlink it if we find it. */
	t = (DHCP_TIME_TimeOutInfo_T *)0;
	for (q = dhcp_time_lcb.timeout_list; q; q = q -> next) {
		if (q -> func == where && q -> what == what) {
			if (t)
				t -> next = q -> next;
			else
				dhcp_time_lcb.timeout_list = q -> next;
			break;
		}
		t = q;
	}

	/* If we found the timeout, put it on the free list. */
	if (q) {
		L_MPOOL_FreeBlock(dhcp_time_lcb.req_pool,q);
	}
}	/*	end of DHCP_TIME_cancel_timeout	*/

/* FUNCTION	NAME : DHCP_TIME_cancel_interface_timeout
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
void DHCP_TIME_Cancel_interface_timeout (void *what)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TIME_TimeOutInfo_T *t, *q;
    /* BODY */

    /* Look for this timeout on the list, and unlink it if we find it. */
    t = (DHCP_TIME_TimeOutInfo_T *)0;
    for (q = dhcp_time_lcb.timeout_list; q; q = q -> next) 
    {
    	if (q -> what == what) {
    		if (t)
    			t -> next = q -> next;
    		else
    			dhcp_time_lcb.timeout_list = q -> next;
    		break;
    	}
    	t = q;
    }

    /* If we found the timeout, put it on the free list. */

	if (q) 
	L_MPOOL_FreeBlock(dhcp_time_lcb.req_pool,q);
    
}	/*	end of DHCP_TIME_cancel_timeout	*/

/* FUNCTION	NAME : DHCP_TIME_IsTimeout
 * PURPOSE:
 *		If any request is timeout in time out list,
 *		return the (where, what), one by one.
 *
 * INPUT:
 *		time_seconds -- system ticks from system started ( in second).
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
BOOL_T	DHCP_TIME_IsTimeout (TIME time_seconds, DHCP_FUNC *where, void **what)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	DHCP_TIME_TimeOutInfo_T *t, *q=dhcp_time_lcb.timeout_list;
    /* BODY */
    /*
     *	Is the first one time out ?
     *	if timeout,
     *		return	the first one info. and free the first one space.
     */

     t = (DHCP_TIME_TimeOutInfo_T *)0; /* Penny added 1-14-2002 */

	if (dhcp_time_lcb.timeout_list)
	{
		if (q->when <= time_seconds)
		{
			t = dhcp_time_lcb.timeout_list;
			dhcp_time_lcb.timeout_list = dhcp_time_lcb.timeout_list -> next;
			*where = t->func;
			*what  = t->what;
			L_MPOOL_FreeBlock(dhcp_time_lcb.req_pool,q); /* Penny 1-14-2002 */
			return	TRUE;
		}
	}
	return	FALSE;
}	/*	end of DHCP_TIME_IsTimeout	*/


/* FUNCTION	NAME : DHCP_TIME_ReCalTimeout
 * PURPOSE:
 *		Recalculate timeout value in timeout list
 *
 * INPUT:
 *		time_seconds  
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
void DHCP_TIME_ReCalTimeout(TIME time_seconds)
{   
    DHCP_TIME_TimeOutInfo_T *t=NULL;

    t = dhcp_time_lcb.timeout_list;
    while(t)
    {   
    	if (t->when >=  DHCP_TIME_MAX_SYSTEM_SECONDS)
    	{
    	    t->when = t->when - DHCP_TIME_MAX_SYSTEM_SECONDS;
    	}
           
        t= t->next;
    }
    
    return;
}


/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */




