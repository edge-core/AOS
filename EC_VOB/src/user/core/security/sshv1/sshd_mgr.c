/* MODULE NAME:  sshd_mgr.c
* PURPOSE: 
*   Initialize the resource and provide some functions for the sshd module.
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
#include <sys_type.h>
#include "sshd_task.h"
#include "sshd_mgr.h"
#include "sshd_om.h"
#include "sysfun.h"
#include "rsa.h"
#include "sshd.h"
#include "http_mgr.h"
#include "sshd_record.h"

#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"

extern void *memset(void *b, int c, unsigned int len);


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
SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_MGR_Init
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
 *          This function is invoked in SSHD_INIT_Initiate_System_Resources.
 */
BOOL_T SSHD_MGR_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	if ( SSHD_OM_Init() == FALSE ) 
	{
		return FALSE;
	}

	return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_EnterMasterMode
 * PURPOSE: 
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.                                                            
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
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *				switch will be initiated to the factory default value.
 *			2. SSHD will handle network requests only when this subsystem
 *				is in the Master Operation mode 
 *			3. This function is invoked in SSHD_INIT_EnterMasterMode.
 */
BOOL_T SSHD_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	SSHD_Session_Record_T	*session_record;
/*	SSHD_Session_Connection_T	*connection_record;*/
	SSHD_Context_T		*ssh_context;
	UI32_T	i;

    /* BODY */
	SSHD_OM_EnterCriticalSection();
	SSHD_OM_SetSshdStatus (SSHD_STATE_ENABLED);
	SSHD_OM_SetSshdPort (SSHD_DEFAULT_PORT_NUMBER);
//	SSHD_OM_LeaveCriticalSection();


	/*
	 */
	SSHD_OM_ResetSshdSessionRecord();
	SSHD_OM_SetAuthenticationRetries(SSHD_DEFAULT_AUTHENTICATION_RETRIES);
	SSHD_OM_SetNegotiationTimeout(SSHD_DEFAULT_NEGOTIATION_TIMEOUT);
	SSHD_OM_SetSshServerVersion(1,5);
	session_record = SSHD_OM_GetSshdSessionRecord();
	for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
	{
		ssh_context = &(session_record->connection[i].ssh_context);
		ssh_context->cipher = malloc(sizeof(SSHD_Cipher_T));
		if ( !ssh_context->cipher )
		{
        	SSHD_OM_LeaveCriticalSection();
			return FALSE;
		}
		memset(ssh_context->cipher,0,sizeof(SSHD_Cipher_T));
	}


//	SSHD_OM_EnterCriticalSection();
/*	SSHD_OM_SetOpMode(SYS_TYPE_STACKING_MASTER_MODE);*/
	SSHD_OM_LeaveCriticalSection();

/* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

	return TRUE;

}



/* FUNCTION NAME:  SSHD_MGR_EnterTransitionMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Transition Operation mode.
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
 *			.
 */
BOOL_T SSHD_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
/*    SYS_TYPE_Stacking_Mode_T opmode;*/
    UI32_T  i;
	SSHD_Session_Record_T	*session_record;
	SSHD_Context_T		*ssh_context;

    /* BODY */
    SYSFUN_ENTER_TRANSITION_MODE();

		SSHD_OM_EnterCriticalSection();
    	session_record = SSHD_OM_GetSshdSessionRecord();
		SSHD_OM_LeaveCriticalSection();

	    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    	{
    	    while(1)
    	    {
    	        if ( (session_record->connection[i].keepalive == 0) && (session_record->connection[i].tid == 0) )
    	        {
	    	        ssh_context = &(session_record->connection[i].ssh_context);
    	        if ( ssh_context->cipher != NULL )
    	        {
		            free(ssh_context->cipher);
	                ssh_context->cipher = NULL;
	            }
		            break;
		        }
		        else
		        {
	            SYSFUN_Sleep(10);
		        }
		    }
    	}

	return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_EnterSlaveMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Slave Operation mode.
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
 *          In Slave Operation mode, any network requests 
 *          will be ignored.
 */
void SSHD_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_ENTER_SLAVE_MODE();

	return;
	
} 



/* FUNCTION	NAME : SSHD_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
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
void SSHD_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
     
    /* BODY */
    
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}



/* FUNCTION	NAME : SSHD_MGR_GetOperationMode
 * PURPOSE:
 *		Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *		None.
 */
SYS_TYPE_Stacking_Mode_T SSHD_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}



/* FUNCTION NAME:  SSHD_MGR_SetSshdStatus
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
 *          This function maybe invoked in CLI command 'ip ssh server'.
 */
BOOL_T SSHD_MGR_SetSshdStatus (SSHD_State_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
	SSHD_OM_EnterCriticalSection();
	SSHD_OM_SetSshdStatus(state);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
	}

	return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetSshdStatus
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
SSHD_State_T SSHD_MGR_GetSshdStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    SSHD_State_T	state = SSHD_STATE_DISABLED;



    /* BODY */
    SYSFUN_USE_CSC(state);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
	SSHD_OM_EnterCriticalSection();
	state = SSHD_OM_GetSshdStatus();
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

 	return ( state );
}



/* FUNCTION NAME:  SSHD_MGR_SetSshdPort
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
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh port'.
 */
BOOL_T SSHD_MGR_SetSshdPort (UI32_T port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T  is_changed;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
	SSHD_OM_EnterCriticalSection();
	is_changed = SSHD_OM_SetSshdPort(port);
	SSHD_OM_LeaveCriticalSection();

    if ( is_changed == TRUE )
    {
        /* close socket */
        SSHD_TASK_PortChanged();
    }
        SYSFUN_RELEASE_CSC();
    }

	return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetSshdPort
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
UI32_T SSHD_MGR_GetSshdPort(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T	port = 0;

    /* BODY */
    SYSFUN_USE_CSC(port);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
	    port = SSHD_OM_GetSshdPort();
    	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

	return port;
}



/* FUNCTION NAME:  SSHD_MGR_SetAuthenticationRetries
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
 *          This function maybe invoked in CLI command 'ip ssh authentication-retries'.
 */
BOOL_T SSHD_MGR_SetAuthenticationRetries(UI32_T retries)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
	SSHD_OM_EnterCriticalSection();
	SSHD_OM_SetAuthenticationRetries(retries);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetAuthenticationRetries
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetAuthenticationRetries(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
	UI32_T	retries = 0;

    /* BODY */
    SYSFUN_USE_CSC(retries);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
	    retries = SSHD_OM_GetAuthenticationRetries();
    	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }
	
    return retries;
}



/* FUNCTION NAME:  SSHD_MGR_SetNegotiationTimeout
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
 *          This function maybe invoked in CLI command 'ip ssh timeout'.
 */
BOOL_T SSHD_MGR_SetNegotiationTimeout(UI32_T timeout)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
	SSHD_OM_EnterCriticalSection();
	SSHD_OM_SetNegotiationTimeout(timeout);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetNegotiationTimeout
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetNegotiationTimeout(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI32_T  timeout = 0;
    
    /* BODY */
    SYSFUN_USE_CSC(timeout);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
	    timeout = SSHD_OM_GetNegotiationTimeout();
    	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }
	
    return timeout;
}



/* FUNCTION NAME:  SSHD_MGR_GetSshServerVersion()
 * PURPOSE: 
 *          This function get version of ssh server.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          UI32_T * - number of major version.
 *          UI32_T * - number of minor version.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
BOOL_T SSHD_MGR_GetSshServerVersion(UI32_T *major, UI32_T *minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
        SSHD_OM_GetSshServerVersion(major,minor);
        SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

    return TRUE;
}



/* FUNCTION NAME : SSHD_MGR_GetSessionPair
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
 *      This function invoked in CLI_TASK_SetSessionContext().
 */
BOOL_T SSHD_MGR_GetSessionPair(UI32_T tnsh_port, UI32_T *user_ip, UI32_T *user_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T rc;
        
    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    if ((user_ip == NULL) || (user_port == NULL))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SSH, SSHD_MGR_GetSessionPair_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    
    *user_ip = 0;
    *user_port = 0;
    
	SSHD_OM_EnterCriticalSection();
    rc = SSHD_OM_GetSessionPair(tnsh_port,user_ip,user_port);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

	return rc;
}



/* FUNCTION NAME : SSHD_MGR_SetConnectionIDAndTnshID
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
 *      This function invoked in TNSHD_ChildTask().
 */
BOOL_T SSHD_MGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T rc;
        
    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
        rc = SSHD_OM_SetConnectionIDAndTnshID(tnsh_port,tid,cid);
    	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

	return rc;
}


/* FUNCTION NAME : SSHD_MGR_GetNextSshConnectionEntry
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
BOOL_T SSHD_MGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T rc;
        
    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    if ( (cid == NULL) || (info == NULL) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SSH, SSHD_MGR_GetNextSshConnectionEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    
	SSHD_OM_EnterCriticalSection();
    rc = SSHD_OM_GetNextSshConnectionEntry(cid,info);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

	return rc;
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionEntry
 * PURPOSE:
 *      Get Specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- Specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- Specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is Specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP .
 */
BOOL_T SSHD_MGR_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T rc;
        
    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    if ( (cid < 0) || (info == NULL) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SSH, SSHD_MGR_GetSshConnectionEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    
	SSHD_OM_EnterCriticalSection();
    rc = SSHD_OM_GetSshConnectionEntry(cid,info);
	SSHD_OM_LeaveCriticalSection();
        SYSFUN_RELEASE_CSC();
    }

	return rc;
}



/* FUNCTION NAME : SSHD_MGR_CheckSshConnection
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
 *      This function invoked in CLI command "disconnect ssh".
 */
BOOL_T SSHD_MGR_CheckSshConnection(UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    BOOL_T rc;
    UI8_T   *arg_p = "SSH connection";

    /* BODY */
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
    	SSHD_OM_EnterCriticalSection();
        rc = SSHD_OM_CheckSshConnection(cid);
	    SSHD_OM_LeaveCriticalSection();
        if( rc == FALSE )
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_MGR_CheckSshConnection_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, arg_p);
        }
        SYSFUN_RELEASE_CSC();
    }
	return rc;
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningNegotiationTimeout
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific SSH negotiation timeout with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Negotiation Timeout.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningNegotiationTimeout(UI32_T *timeout)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else 
    {
    *timeout = SSHD_MGR_GetNegotiationTimeout();
	if ( *timeout != SSHD_DEFAULT_NEGOTIATION_TIMEOUT )
	{
            SYSFUN_RELEASE_CSC();
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
	}
        SYSFUN_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningAuthenticationRetries
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific SSH authentication retries times with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Authentication Retries.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningAuthenticationRetries(UI32_T *retries)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else 
    {
    *retries = SSHD_MGR_GetAuthenticationRetries();
	if ( *retries != SSHD_DEFAULT_AUTHENTICATION_RETRIES )
	{
            SYSFUN_RELEASE_CSC();
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
	}
        SYSFUN_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningSshdStatus
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific Sshd Status with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Sshd Status.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningSshdStatus(UI32_T *state)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else 
    {
    *state = SSHD_MGR_GetSshdStatus();
	if ( *state != SSHD_DEFAULT_STATE )
	{
            SYSFUN_RELEASE_CSC();
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
	}
        SYSFUN_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
}



/* FUNCTION NAME - SSHD_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SSHD_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    return;
}



/* FUNCTION NAME - SSHD_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SSHD_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    return;
}





/* LOCAL SUBPROGRAM BODIES
 */



