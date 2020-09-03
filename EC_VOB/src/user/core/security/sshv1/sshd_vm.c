/* MODULE NAME:  sshd_vm.c
* PURPOSE: 
*   Initialize the resource and provide some functions for the sshd module.
*   
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-09-19      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */
#include <sys_type.h>
#include "skt_vx.h"
#include "socket.h"
#include "sshd_mgr.h"
#include "sshd_om.h"
#include "sysfun.h"
#include "rsa.h"
#include "sshd.h"
#include "keygen_mgr.h"
#include "sshd_vm.h"
#include "sshd_record.h"



/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */

/* STATIC VARIABLE DECLARATIONS 
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_VM_GetSshdSessionRecord
 * PURPOSE: 
 *			This function get sshd session record pointer.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          void * - SSHD session record pointer.
 * NOTES:
 *          .
 */ 
void *SSHD_VM_GetSshdSessionRecord(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	SSHD_Session_Record_T	*session_record;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	session_record = SSHD_OM_GetSshdSessionRecord();
	SSHD_OM_LeaveCriticalSection();

	return (void *)session_record;
}



/* FUNCTION NAME:  SSHD_VM_GetCreatedSessionNumber
 * PURPOSE: 
 *			This function get number of  ssh connection.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          UI32_T -- number of  ssh connection.
 * NOTES:
 *          .
 */ 
UI32_T SSHD_VM_GetCreatedSessionNumber(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	created_session_number;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	created_session_number = SSHD_OM_GetCreatedSessionNumber();
	SSHD_OM_LeaveCriticalSection();

	return created_session_number;
}



/* FUNCTION NAME:  SSHD_VM_SetCreatedSessionNumber
 * PURPOSE: 
 *			This function set number of  ssh connection.
 * INPUT:   
 *          UI32_T -- number of  ssh connection.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */ 
void SSHD_VM_SetCreatedSessionNumber(UI32_T number)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	SSHD_OM_SetCreatedSessionNumber(number);
	SSHD_OM_LeaveCriticalSection();

	return ;
}



/* FUNCTION NAME:  SSHD_VM_SetTaskID
 * PURPOSE: 
 *			This function set task id to session record.
 * INPUT:   
 *          UI32_T -- task id.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          TRUE to indicate successful and FALSE to indicate failure.
 */ 
BOOL_T SSHD_VM_SetTaskID(UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	i;
	SSHD_Session_Record_T	*session_record;

	/* BODY */
	SSHD_OM_EnterCriticalSection();
	if ( (SSHD_MGR_GetOperationMode() == SYS_TYPE_STACKING_MASTER_MODE) && (SSHD_OM_GetSshdStatus() == SSHD_STATE_ENABLED) )
	{
    	session_record = SSHD_OM_GetSshdSessionRecord();
	    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	    {
		    if ( (session_record->connection[i].keepalive == 0) && (session_record->connection[i].tid == 0) )
		    {
		    	SSHD_OM_SetTaskID(i,tid);
			    break;
		    }
	    }
	    SSHD_OM_LeaveCriticalSection();
	    if ( i <= SSHD_DEFAULT_MAX_SESSION_NUMBER )
	    {
	        return TRUE;
	    }
	    else
	    {
	        return FALSE;
	    }
	}
	else
	{
        SSHD_OM_LeaveCriticalSection();
        return FALSE;
    }
}



/* FUNCTION NAME:  SSHD_VM_GetTaskID
 * PURPOSE: 
 *			This function get task id from session record.
 * INPUT:   
 *          UI32_T -- index of session record.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          UI32_T -- task id.
 * NOTES:
 *          .
 */ 
UI32_T SSHD_VM_GetTaskID(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	tid;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	tid = SSHD_OM_GetTaskID(index);
	SSHD_OM_LeaveCriticalSection();

	return tid;
}



/* FUNCTION NAME:  SSHD_VM_GetContextAddress
 * PURPOSE: 
 *			This function get ssh_context pointer.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          void * -- ssh_context pointer.
 * NOTES:
 *          .
 */ 
void *SSHD_VM_GetContextAddress(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T tid, i;
	SSHD_Context_T	*ssh_context = NULL;

    /* BODY */
	tid = SYSFUN_TaskIdSelf();

	SSHD_OM_EnterCriticalSection();

	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
			ssh_context = SSHD_OM_GetContextAddress(i);
			break;
		}
	}
	SSHD_OM_LeaveCriticalSection();

	return ssh_context;
}



/* FUNCTION NAME:  SSHD_VM_ResetTaskID
 * PURPOSE: 
 *			This function reset task id to session record.
 * INPUT:   
 *          UI32_T -- task id.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */ 
void SSHD_VM_ResetTaskID(UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	i;
	SSHD_Session_Record_T	*session_record;
	
	/* BODY */

	SSHD_OM_EnterCriticalSection();
	session_record = SSHD_OM_GetSshdSessionRecord();
	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( session_record->connection[i].tid == tid )
		{
				SSHD_OM_ResetTaskID(i);
				break;
		}
	}
	SSHD_OM_LeaveCriticalSection();

	return ;
}



/* FUNCTION NAME:  SSHD_VM_GetHostkey
 * PURPOSE: 
 *          This function get hostkey of sshd.
 *
 * INPUT:   
 *          void * -- pointer of ssh context.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetHostkey(void *ssh_context)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	SSHD_Context_T *context;

    /* BODY */
    context = (SSHD_Context_T *)ssh_context;
/*    if ( context->hostkey )
    {
        ssh_rsa_free(context->hostkey);
        context->hostkey = NULL;
    }*/
    
    context->tmp_hostkey = (EVP_PKEY *)KEYGEN_MGR_GetSshdHostkey();
    context->hostkey = (ssh_RSA *)context->tmp_hostkey->pkey.rsa;
	
    if ( (context->tmp_hostkey == NULL) || (context->hostkey == NULL) )
    {
        return FALSE;
    }
	
    return TRUE;
}



/* FUNCTION NAME:  SSHD_VM_GetServerkey
 * PURPOSE: 
 *          This function get serverkey of sshd.
 *
 * INPUT:   
 *          void * -- pointer of ssh context.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetServerkey(void *ssh_context)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	tid,i;
	SSHD_Context_T *context;

    /* BODY */
    context = (SSHD_Context_T *)ssh_context;
/*    if ( context->serverkey )
    {
        ssh_rsa_free(context->serverkey);
        context->serverkey = NULL;
    }*/

  	tid = SYSFUN_TaskIdSelf();

	SSHD_OM_EnterCriticalSection();
	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
			break;
		}
	}
	SSHD_OM_LeaveCriticalSection();

    if ( i<=SSHD_DEFAULT_MAX_SESSION_NUMBER )
    {
        context->tmp_serverkey = (EVP_PKEY *)KEYGEN_MGR_GetSshdServerkey(i);
        context->serverkey = (ssh_RSA *)context->tmp_serverkey->pkey.rsa;
        if ( (context->tmp_serverkey == NULL) || (context->serverkey == NULL) )
        {
            return FALSE;
        }
        return TRUE;
    }
	
    return FALSE;
}



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionStatus()
 * PURPOSE: 
 *          This function set status of ssh connection.
 *
 * INPUT:   
 *          SSHD_ConnectionState_T - current state of connection.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshConnectionStatus(SSHD_ConnectionState_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T tid, i;

    /* BODY */
	tid = SYSFUN_TaskIdSelf();

	SSHD_OM_EnterCriticalSection();

	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
            SSHD_OM_SetSshConnectionStatus(i,state);
		}
	}
	SSHD_OM_LeaveCriticalSection();

    if ( i<=SSHD_DEFAULT_MAX_SESSION_NUMBER )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionVersion()
 * PURPOSE: 
 *          This function set version of ssh connection.
 *
 * INPUT:   
 *          UI32_T - number of major version.
 *          UI32_T - number of minor version.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshConnectionVersion(UI32_T major, UI32_T minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T tid, i;

    /* BODY */
	tid = SYSFUN_TaskIdSelf();

	SSHD_OM_EnterCriticalSection();

	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
		    SSHD_OM_SetSshConnectionVersion(i,major,minor);
		    SSHD_OM_LeaveCriticalSection();
		    return TRUE;
		}
	}
	SSHD_OM_LeaveCriticalSection();
    return FALSE;
}



/* FUNCTION NAME:  SSHD_VM_SetSshServerVersion()
 * PURPOSE: 
 *          This function set version of ssh server.
 *
 * INPUT:   
 *          UI32_T - number of major version.
 *          UI32_T - number of minor version.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshServerVersion(UI32_T major, UI32_T minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	SSHD_OM_EnterCriticalSection();
    SSHD_OM_SetSshServerVersion(major,minor);
	SSHD_OM_LeaveCriticalSection();

    return TRUE;
}



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionUsername()
 * PURPOSE: 
 *          This function set username of ssh connection.
 *
 * INPUT:   
 *          UI8_T * - username
 *
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshConnectionUsername(UI8_T *username)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T tid, i;

    /* BODY */
	tid = SYSFUN_TaskIdSelf();

	SSHD_OM_EnterCriticalSection();

	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
		    SSHD_OM_SetSshConnectionUsername(i,username);
		    SSHD_OM_LeaveCriticalSection();
		    return TRUE;
		}
	}
	SSHD_OM_LeaveCriticalSection();
    return FALSE;
}



/* FUNCTION NAME:  SSHD_VM_GetLocalSessionName()
 * PURPOSE: 
 *          This function get local site session (ip address ,port) according socket-id.
 *
 * INPUT:   
 *          UI32_T - socket id.
 *                                  
 * OUTPUT:  
 *          UI32_T * - ip address.
 *          UI32_T * - number of port.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetLocalSessionName(UI32_T sock_id, UI32_T *ip, UI32_T *port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    struct  sockaddr    sock_addr;
    struct  sockaddr_in *sock_in;
    UI32_T  sock_addr_size=sizeof(sock_addr);
    UI32_T     res;

    /* BODY */
    *ip = 0;
    *port = 0;

	res = getsockname(sock_id, &sock_addr,(int *)&sock_addr_size);

    if (res==0)
    {
        sock_in = (struct sockaddr_in*) &sock_addr;
        *port = sock_in->sin_port;
        *ip   = ntohl(sock_in->sin_addr);
        return TRUE;
    }
    return FALSE;
}



/* FUNCTION NAME:  SSHD_VM_GetRemoteSessionName()
 * PURPOSE: 
 *          This function get remote site session (ip address ,port) according socket-id.
 *
 * INPUT:   
 *          UI32_T - socket id.
 *                                  
 * OUTPUT:  
 *          UI32_T * - ip address.
 *          UI32_T * - number of port.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetRemoteSessionName(UI32_T sock_id, UI32_T *ip, UI32_T *port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    struct  sockaddr    sock_addr;
    struct  sockaddr_in *sock_in;
    UI32_T  sock_addr_size=sizeof(sock_addr);
    UI32_T  res;

    /* BODY */
    *ip = 0;
    *port = 0;

	res = getpeername(sock_id, &sock_addr, (int *)&sock_addr_size);

    if (res==0)
    {
        sock_in = (struct sockaddr_in*) &sock_addr;
        *port = sock_in->sin_port;
        *ip   = ntohl(sock_in->sin_addr);
        return TRUE;
    }
    return FALSE;
}



/* FUNCTION NAME : SSHD_VM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *      UI32_T  -- remote site port of TNSH (pty) session.
 *      UI32_T  -- the local side port connect to TNSHD.
 *      UI32_T  -- local site port of SSHD (net) session.
 *      UI32_T  -- the ip of remote site in socket.
 *      UI32_T  -- the port of remote site in socket.(net)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_VM_SetSessionPair(UI32_T remote_tnsh_port, UI32_T tnsh_port,
                               UI32_T user_local_port,
                               UI32_T user_ip, UI32_T user_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  tid;
    UI32_T  i;

    /* BODY */
    tid = SYSFUN_TaskIdSelf();
    
   	SSHD_OM_EnterCriticalSection();

	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		if ( SSHD_OM_GetTaskID(i) == tid )
		{
		    SSHD_OM_SetSessionPair(i, remote_tnsh_port, tnsh_port,
                                        user_local_port,
                                        user_ip, user_port);
		    SSHD_OM_LeaveCriticalSection();
		    return TRUE;
		}
	}
	SSHD_OM_LeaveCriticalSection();
    return FALSE;

}



/* FUNCTION NAME:  SSHD_VM_GetSshdStatus
 * PURPOSE: 
 *          This function get sshd state.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          SSHD_State_T - SSHD status.
 * NOTES:
 *          .
 */
SSHD_State_T SSHD_VM_GetSshdStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    SSHD_State_T	state;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	state = SSHD_OM_GetSshdStatus();
	SSHD_OM_LeaveCriticalSection();

 	return ( state );
}



/* FUNCTION NAME:  SSHD_VM_GetNegotiationTimeout
 * PURPOSE: 
 *          This function get number of negotiation timeout .
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          UI32_T --  number of negotiation timeout .
 * NOTES:
 *          .
 */
UI32_T SSHD_VM_GetNegotiationTimeout(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  timeout;
    
    /* BODY */
	SSHD_OM_EnterCriticalSection();
	timeout = SSHD_OM_GetNegotiationTimeout();
	SSHD_OM_LeaveCriticalSection();
	
    return timeout;
}



/* FUNCTION NAME:  SSHD_VM_GetAuthenticationRetries
 * PURPOSE: 
 *          This function get number of retries for authentication user.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          UI32_T --  number of retries for authentication user.
 * NOTES:
 *          .
 */
UI32_T SSHD_VM_GetAuthenticationRetries(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	retries;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	retries = SSHD_OM_GetAuthenticationRetries();
	SSHD_OM_LeaveCriticalSection();
	
    return retries;
}









/* LOCAL SUBPROGRAM BODIES
 */



