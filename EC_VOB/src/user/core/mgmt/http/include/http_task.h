/*
 * Project Name: Mercury
 * Module Name : HTTP_TASK.h
 * Abstract    : to be included in root.c
 * Purpose     : HTTP initiation and HTTP task creation
 *
 * History :
 *          Date        Modifier        Reason
 *
 * Copyright(C)      Accton Corporation, 2001
 *
 * Note    :
 */

#ifndef _HTTP_TASK_H_
#define _HTTP_TASK_H_
#include "sys_cpnt.h"

#if __cplusplus
extern "C" {
#endif

/* FUNCTION NAME:  HTTP_TASK_Init
 * PURPOSE:
 *          This function init the message queue.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_INIT_InitiateSystemResources.
 */
BOOL_T HTTP_TASK_Init(void);



/* FUNCTION NAME : HTTP_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void HTTP_TASK_SetTransitionMode();



/* FUNCTION NAME : HTTP_TASK_EnterTransitionMode
 * PURPOSE:
 *		Leave CSC Task while transition done.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void HTTP_TASK_EnterTransitionMode();



/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Create_Task
 *------------------------------------------------------------------------
 * PURPOSE: This function creates the HTTP task for network
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Create_Task ();

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_EnterMainRoutine
 *------------------------------------------------------------------------
 * PURPOSE: This function creates mail loop
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Enter_Main_Routine ();

/* FUNCTION NAME:  HTTP_TASK_StartHttpService
 * PURPOSE:
 *          Start HTTP service.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - socket number.
 * NOTES:
 *          none.
 */
int HTTP_TASK_StartHttpService();

#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_TASK_StartHttpsService
 * PURPOSE:
 *          Initialize the security socket.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - socket number.
 * NOTES:
 *          none.
 */
int HTTP_TASK_StartHttpsService();


#endif /* if (SYS_CPNT_HTTPS == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Port_Changed_Callback
 *------------------------------------------------------------------------
 * PURPOSE: This function is an event callback when the port is changed
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Port_Changed (void);



/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_RifDestroyed
 *------------------------------------------------------------------------
 * PURPOSE: This function is an event callback when the RIF is destroyed
 *.
 * INPUT   : ip_address
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_RifDestroyed(L_INET_AddrIp_T *ip_addr_p);



#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_TASK_Secure_Port_Changed
 * PURPOSE:
 *			This function set flag when security port is changed.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          (Something must be known to use this function.)
 */
void HTTP_TASK_Secure_Port_Changed();



#endif  /* if SYS_CPNT_HTTPS */



/* FUNCTION NAME:  HTTP_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the HTTP module to start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *          This function is invoked in HTTP_INIT_ProvisionComplete().
 */
void HTTP_TASK_ProvisionComplete(void);

BOOL_T HTTP_TASK_IsValidMgmtRemoteAddress(int remotefd);

#if (SYS_CPNT_CLUSTER == TRUE)
BOOL_T HTTP_TASK_GetClusterMemberIp(int newsockfd);
#endif /* SYS_CPNT_CLUSTER */

#if __cplusplus
}
#endif

#endif /* _HTTP_TASK_H_ */

