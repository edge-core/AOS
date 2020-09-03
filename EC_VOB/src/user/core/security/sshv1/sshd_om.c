/* MODULE NAME:  sshd_om.c
* PURPOSE: 
*   Initialize the database resource and provide some get/set functions for accessing the 
*   sshd database.   
*   
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-05-24      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "sshd_om.h"
#include "ssh_cipher.h"
#include "sys_adpt.h"
#include "sshd_record.h"

extern int printf(const char *_format, ...);
extern void *memset(void *b, int c, unsigned int len);
extern char *strcpy( char *strDestination, const char *strSource );

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
static UI32_T  sshd_om_semaphore_id;
static SYS_TYPE_Stacking_Mode_T sshd_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
static UI32_T sshd_port_number = SSHD_DEFAULT_PORT_NUMBER;
static SSHD_State_T sshd_status = SSHD_DEFAULT_STATE;
static SSHD_Session_Record_T sshd_session_context;



/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_OM_Init
 * PURPOSE: 
 *          Initiate the semaphore for SSHD objects
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
 *          This function is invoked in SSHD_MGR_Init.
 */
BOOL_T SSHD_OM_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    if ( SYSFUN_CreateSem (/*SEM_FULL*/ 1, SYSFUN_SEM_FIFO, &sshd_om_semaphore_id) != SYSFUN_OK )
    {
		printf("Create sshd_om_semaphore_id error \n");
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_EnterCriticalSection
 * PURPOSE: 
 *          Enter critical section before a task invokes the sshd objects.
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
 *          .
 */
BOOL_T SSHD_OM_EnterCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    if (SYSFUN_GetSem(sshd_om_semaphore_id, SYSFUN_TIMEOUT_WAIT_FOREVER) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_LeaveCriticalSection
 * PURPOSE: 
 *          Leave critical section after a task invokes the sshd objects.
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
 *          .
 */
BOOL_T SSHD_OM_LeaveCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    if (SYSFUN_SetSem(sshd_om_semaphore_id) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_SetSshdStatus
 * PURPOSE: 
 *          This function set sshd state.
 *
 * INPUT:   
 *          SSHD_State_T - SSHD status.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetSshdStatus(SSHD_State_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	/* if same state, ignore */
	if (state == sshd_status)
	{
		return;
	}
	
	/* set state */
	sshd_status = state;

	return;
}



/* FUNCTION NAME:  SSHD_OM_GetSshdStatus
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
SSHD_State_T SSHD_OM_GetSshdStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
 	return sshd_status;
}



/* FUNCTION NAME:  SSHD_OM_SetSshdPort
 * PURPOSE: 
 *          This function set sshd port number.
 *
 * INPUT:   
 *          UI32_T - SSHD port number.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshdPort (UI32_T port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	/* if same telnet port, ignore */
	if ( port == SYS_DFLT_TELNET_SOCKET_PORT )
	{
		printf(" Port %ld is use by telnetd.\n",port);
		return FALSE;
	}

	/* if same port, ignore */
	if (port == sshd_port_number)
	{
		return FALSE;
	}
	
	/* set port */
	sshd_port_number = port;

	return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_GetSshdPort
 * PURPOSE: 
 *			This function get sshd port number.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          UI32_T - SSHD port value.
 * NOTES:
 *          default is tcp/22.
 */ 
UI32_T SSHD_OM_GetSshdPort(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    return sshd_port_number;
}



/* FUNCTION NAME:  SSHD_OM_SetOpMode
 * PURPOSE: 
 *          This function set sshd operation mode.
 *
 * INPUT:   
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetOpMode (SYS_TYPE_Stacking_Mode_T opmode)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	sshd_operation_mode = opmode;
	return;
}



/* FUNCTION NAME:  SSHD_OM_GetOpMode
 * PURPOSE: 
 *          This function get sshd operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:
 *          .
 */
SYS_TYPE_Stacking_Mode_T SSHD_OM_GetOpMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    return sshd_operation_mode;
}



/* FUNCTION NAME:  SSHD_OM_ResetSshdSessionRecord
 * PURPOSE: 
 *          This function reset sshd session record.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_ResetSshdSessionRecord(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  index;

    /* BODY */
	memset(&sshd_session_context,0,sizeof(SSHD_Session_Record_T));
	for ( index=1 ; index<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; index++ )
	{
        sshd_session_context.connection[index].connection_id = -1;
    }
    return ;
}



/* FUNCTION NAME:  SSHD_OM_SetAuthenticationRetries
 * PURPOSE: 
 *          This function set number of retries for authentication user.
 *
 * INPUT:   
 *          UI32_T -- number of retries for authentication user.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetAuthenticationRetries(UI32_T retries)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	sshd_session_context.authentication_retries = retries;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_GetAuthenticationRetries
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
UI32_T SSHD_OM_GetAuthenticationRetries(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	
    return sshd_session_context.authentication_retries;
}



/* FUNCTION NAME:  SSHD_OM_SetNegotiationTimeout
 * PURPOSE: 
 *          This function set number of negotiation timeout .
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetNegotiationTimeout(UI32_T timeout)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	sshd_session_context.timeout = timeout;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_GetNegotiationTimeout
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
UI32_T SSHD_OM_GetNegotiationTimeout(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	
    return sshd_session_context.timeout;
}



/* FUNCTION NAME:  SSHD_OM_GetSshdSessionRecord
 * PURPOSE: 
 *			This function get sshd session record pointer.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          SSHD_Session_Record_T - SSHD session record pointer.
 * NOTES:
 *          .
 */ 
SSHD_Session_Record_T *SSHD_OM_GetSshdSessionRecord(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

	/* BODY */

	return &sshd_session_context;
}



/* FUNCTION NAME:  SSHD_OM_GetCreatedSessionNumber
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
UI32_T SSHD_OM_GetCreatedSessionNumber(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */

	return sshd_session_context.created_session_count;
}



/* FUNCTION NAME:  SSHD_OM_SetCreatedSessionNumber
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
void SSHD_OM_SetCreatedSessionNumber(UI32_T number)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */

	sshd_session_context.created_session_count = number;
	return;
}



/* FUNCTION NAME:  SSHD_OM_SetTaskID
 * PURPOSE: 
 *			This function set task id to session record.
 * INPUT:   
 *          UI32_T -- index of session record.
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
void SSHD_OM_SetTaskID(UI32_T index, UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	sshd_session_context.connection[index].keepalive = 1;
	sshd_session_context.connection[index].tid = tid;
	sshd_session_context.connection[index].connection_id = -1;
	sshd_session_context.connection[index].user_ip = 0;
	sshd_session_context.connection[index].user_port = 0;
	sshd_session_context.connection[index].user_local_port = 0;
	sshd_session_context.connection[index].tnsh_port = 0;
	sshd_session_context.connection[index].remote_tnsh_port = 0;

	return ;
}



/* FUNCTION NAME:  SSHD_OM_GetTaskID
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
UI32_T SSHD_OM_GetTaskID(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */

	return sshd_session_context.connection[index].tid;
}



/* FUNCTION NAME:  SSHD_OM_GetContextAddress
 * PURPOSE: 
 *			This function get ssh_context pointer.
 * INPUT:   
 *          UI32_T -- index of session record.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          SSHD_Context_T * -- ssh_context pointer.
 * NOTES:
 *          .
 */ 
SSHD_Context_T *SSHD_OM_GetContextAddress(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */

	return &(sshd_session_context.connection[index].ssh_context);
}



/* FUNCTION NAME:  SSHD_OM_ResetTaskID
 * PURPOSE: 
 *			This function reset task id to session record.
 * INPUT:   
 *          UI32_T -- index of session record.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */ 
void SSHD_OM_ResetTaskID(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	sshd_session_context.connection[index].keepalive = 0;
	sshd_session_context.connection[index].tid = 0;
	sshd_session_context.connection[index].connection_id = -1;
	sshd_session_context.connection[index].user_ip = 0;
	sshd_session_context.connection[index].user_port = 0;
	sshd_session_context.connection[index].user_local_port = 0;
	sshd_session_context.connection[index].tnsh_port = 0;
	sshd_session_context.connection[index].remote_tnsh_port = 0;

	return ;
}



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionStatus
 * PURPOSE: 
 *          This function set status of ssh connection.
 *
 * INPUT:   
 *          UI32_T                  - index of connection recond.
 *          SSHD_ConnectionState_T - Current connection status.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionStatus(UI32_T index ,SSHD_ConnectionState_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    sshd_session_context.connection[index].status = state;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionVersion
 * PURPOSE: 
 *          This function set version of ssh connection.
 *
 * INPUT:   
 *          UI32_T - index of connection recond.
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
BOOL_T SSHD_OM_SetSshConnectionVersion(UI32_T index, UI32_T major, UI32_T minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    sshd_session_context.connection[index].major_version = major;
    sshd_session_context.connection[index].minor_version = minor;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_SetSshServerVersion
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
BOOL_T SSHD_OM_SetSshServerVersion(UI32_T major, UI32_T minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    sshd_session_context.server_major_version = major;
    sshd_session_context.server_minor_version = minor;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_GetSshServerVersion
 * PURPOSE: 
 *          This function get version of ssh server.
 *
 * INPUT:   
 *                                  
 * OUTPUT:  
 *          UI32_T * - number of major version.
 *          UI32_T * - number of minor version.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerVersion(UI32_T *major, UI32_T *minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    *major = sshd_session_context.server_major_version;
    *minor = sshd_session_context.server_minor_version;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionUsername
 * PURPOSE: 
 *          This function set username of ssh connection.
 *
 * INPUT:   
 *          UI32_T  - number of connection range is 0-3.
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
BOOL_T SSHD_OM_SetSshConnectionUsername(UI32_T index, UI8_T *username)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    sshd_session_context.connection[index].username = username;
    /*strcpy(sshd_session_context.connection[index].username,username);*/

    return TRUE;
}



/* FUNCTION NAME : SSHD_OM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *      UI32_T  -- number of connection range is 0-3.
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
BOOL_T SSHD_OM_SetSessionPair(UI32_T index, UI32_T remote_tnsh_port, 
                                 UI32_T tnsh_port, UI32_T user_local_port,
                                 UI32_T user_ip, UI32_T user_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    sshd_session_context.connection[index].user_ip = user_ip;
    sshd_session_context.connection[index].user_port = user_port;
    sshd_session_context.connection[index].user_local_port = user_local_port;
    sshd_session_context.connection[index].tnsh_port = tnsh_port;
    sshd_session_context.connection[index].remote_tnsh_port = remote_tnsh_port;

    return TRUE;
}



/* FUNCTION NAME : SSHD_OM_GetSessionPair
 * PURPOSE:
 *      Retrieve a session pair from session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *
 * OUTPUT:
 *      UI32_T * -- the ip of remote site in socket.
 *      UI32_T * -- the port of remote site in socket.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_OM_GetSessionPair(UI32_T tnsh_port, UI32_T *user_ip, UI32_T *user_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  i;
        
    /* BODY */
    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        if ( sshd_session_context.connection[i].tnsh_port == tnsh_port )
        {
            *user_ip = sshd_session_context.connection[i].user_ip;
            *user_port = sshd_session_context.connection[i].user_port;
            return TRUE;
        }
    }
	return FALSE;
}



/* FUNCTION NAME : SSHD_OM_SetConnectionIDAndTnshID
 * PURPOSE:
 *      Set tnsh id and ssh connection id to session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *      UI32_T  -- TNSH id.
 *      UI32_T  -- ssh connection id.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *      FALSE - tnsh_port found and ssh server disabled.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  i;
        
    /* BODY */
    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        if ( sshd_session_context.connection[i].tnsh_port == tnsh_port )
        {
            if ( /*(sshd_operation_mode == SYS_TYPE_STACKING_MASTER_MODE) &&*/ (sshd_status == SSHD_STATE_ENABLED) )
            {
                sshd_session_context.connection[i].tnsh_tid = tid;
                sshd_session_context.connection[i].connection_id = cid;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
	return TRUE;
}



/* FUNCTION NAME : SSHD_OM_GetNextActiveConnectionID
 * PURPOSE:
 *      Get next active connection id .
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection id.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      Initial input value is -1.
 */
BOOL_T SSHD_OM_GetNextActiveConnectionID(I32_T *cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  i;
        
    /* BODY */
    while (1)
    {
        *cid = *cid + 1;
        if ( *cid >= SSHD_DEFAULT_MAX_SESSION_NUMBER )
        {
            return FALSE;
        }
    
        for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
        {
            if ( sshd_session_context.connection[i].connection_id == *cid )
            {
                if ( sshd_session_context.connection[i].keepalive == 1 )
                {
                    sshd_session_context.connection[i].keepalive = 0;
                    return TRUE;
                }
            }
        }/* end of for loop */
    }/* end of while(1) loop */
}



/* FUNCTION NAME : SSHD_OM_GetNextSshConnectionEntry
 * PURPOSE:
 *      Get next active connection entry.
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *      SSHD_ConnectionInfo_T * -- current active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in CLI command "show ssh".
 *      Initial input value is -1.
 */
BOOL_T SSHD_OM_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  i;
        
    /* BODY */
    while (1)
    {
        *cid = *cid + 1;
        if ( *cid >= SSHD_DEFAULT_MAX_SESSION_NUMBER )
        {
            return FALSE;
        }
    
        for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
        {
            if ( sshd_session_context.connection[i].connection_id == *cid )
            {
                info->connection_id = sshd_session_context.connection[i].connection_id;
                info->major_version = sshd_session_context.connection[i].major_version;
                info->minor_version = sshd_session_context.connection[i].minor_version;
                info->status = sshd_session_context.connection[i].status;
                info->cipher = sshd_session_context.connection[i].ssh_context.cipher->type;
                memset(info->username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy(info->username,sshd_session_context.connection[i].username, SYS_ADPT_MAX_USER_NAME_LEN);
                
                return TRUE;
            }
        }/* end of for loop */
    }/* end of while(1) loop */
}



/* FUNCTION NAME : SSHD_OM_GetSshConnectionEntry
 * PURPOSE:
 *      Get specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP.
 */
BOOL_T SSHD_OM_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  i;
        
    /* BODY */
    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        if ( sshd_session_context.connection[i].connection_id == cid )
        {
            info->connection_id = sshd_session_context.connection[i].connection_id;
            info->major_version = sshd_session_context.connection[i].major_version;
            info->minor_version = sshd_session_context.connection[i].minor_version;
            info->status = sshd_session_context.connection[i].status;
            info->cipher = sshd_session_context.connection[i].ssh_context.cipher->type;
            memset(info->username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
            strncpy(info->username,sshd_session_context.connection[i].username, SYS_ADPT_MAX_USER_NAME_LEN);
                
            return TRUE;
        }
    }/* end of for loop */
    
    return FALSE;
}



/* FUNCTION NAME : SSHD_OM_CheckSshConnection
 * PURPOSE:
 *      Check connection is ssh or not.
 *
 * INPUT:
 *      UI32_T -- connection id.
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - This connection is ssh connection.
 *      FALSE - This connection is not ssh connection.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_CheckSshConnection(UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T i;
        
    /* BODY */
    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        if ( sshd_session_context.connection[i].connection_id == cid )
        {
            return TRUE;
        }
    }/* end of for loop */
    return FALSE;
}
    




/* LOCAL SUBPROGRAM BODIES
 */



