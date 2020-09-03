/* MODULE NAME:  sshd_task.c
* PURPOSE: 
*   SSHD initiation and SSHD task creation
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
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"

#include "skt_vx.h"
#include "socket.h"
#include "iproute.h"

#include "sshd_type.h"
#include "sshd_record.h"
#include "sshd_task.h"
#include "sshd_mgr.h"
#include "sshd_vm.h"
#include "cli_mgr.h"

#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt_mgr.h"
#endif

int sshd_main(void);
extern int printf(const char *_format, ...);
extern void *memset(void *b, int c, unsigned int len);
extern int sprintf( char *buffer, const char *format, ... );



/* NAMING CONSTANT DECLARATIONS
 */
#define SSHD_TASK_EVENT_ENTER_TRANSITION    BIT_1

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
/* FUNCTION NAME:  SSHD_TASK_Main
 * PURPOSE: 
 *			SSHD starting routine.
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
 *          This function is invoked in SSHD_TASK_CreateTask().
 */
static void SSHD_TASK_Main(void);



/* FUNCTION NAME:  SSHD_TASK_EnterMainRoutine
 * PURPOSE: 
 *			SSHD task main routine.
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
 *          This function is invoked in SSHD_TASK_Main().
 */
static void SSHD_TASK_EnterMainRoutine(void);



/* FUNCTION NAME:  SSHD_TASK_IsProvisionComplete
 * PURPOSE: 
 *          This function will check the SSHD module can start.
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
 *        
 */
BOOL_T SSHD_TASK_IsProvisionComplete(void);



/* STATIC VARIABLE DECLARATIONS 
 */
static  BOOL_T  sshd_task_is_port_changed = FALSE;   
static  BOOL_T  is_transition_done;
static  UI32_T  sshd_task_main_id;
static  BOOL_T  is_provision_complete = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_TASK_Init
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
 *          This function is invoked in SSHD_INIT_InitiateSystemResources.
 */
BOOL_T SSHD_TASK_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    is_transition_done = FALSE;
    
    return TRUE;
}



/* FUNCTION NAME:  SSHD_TASK_CreateTask
 * PURPOSE: 
 *			This function create sshd main task.
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_CreateTasks().
 */ 
BOOL_T SSHD_TASK_CreateTask(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
	if (SYSFUN_SpawnTask (	SYS_BLD_SSHD_MAIN_TASK, 
                        	SYS_BLD_SSHD_MAIN_TASK_PRIORITY, 
                        	SYS_BLD_TASK_COMMON_STACK_SIZE,
                        	0, 
                        	SSHD_TASK_Main, 
                        	0, 
                        	&sshd_task_main_id) != SYSFUN_OK )
	{
        EH_MGR_Handle_Exception(SYS_MODULE_SSH, SSHD_TASK_CreateTask_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
		return FALSE;
	} /* End of if */                        

	return TRUE;
	
} /* end of SSHD_TASK_CreateTask() */



/* FUNCTION NAME:  SSHD_TASK_PortChanged
 * PURPOSE: 
 *			This function set flag when sshd port is changed.
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_MGR_SetSshdPort().
 */ 
void SSHD_TASK_PortChanged(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
	sshd_task_is_port_changed = TRUE;
	
	return ;
}



/* FUNCTION NAME : SSHD_TASK_SetTransitionMode
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
void SSHD_TASK_SetTransitionMode(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
 
    /* BODY */
    is_transition_done = FALSE;
    SYSFUN_SendEvent(sshd_task_main_id, SSHD_TASK_EVENT_ENTER_TRANSITION);
}



/* FUNCTION NAME : SSHD_TASK_EnterTransitionMode
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
void SSHD_TASK_EnterTransitionMode(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
 
    /* BODY */
    /*	want task release all resources	*/
	SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);   
}



/* FUNCTION NAME:  SSHD_TASK_ProvisionComplete
 * PURPOSE: 
 *          This function will tell the SSHD module to start.
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
 *          This function is invoked in SSHD_INIT_ProvisionComplete().
 */
void SSHD_TASK_ProvisionComplete(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
 
    /* BODY */
    is_provision_complete = TRUE;
}











/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_TASK_Main
 * PURPOSE: 
 *			SSHD starting routine.
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
 *          This function is invoked in SSHD_TASK_CreateTask().
 */
static void SSHD_TASK_Main(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/
    UI32_T	                wait_events;
    UI32_T	                rcv_events;
    UI32_T	                event_var;
    UI32_T	                ret_value;
	SSHD_Session_Record_T	*session_record;
    UI32_T                  i;

	/* BODY */
    /*	Prepare waiting event and init. event var.	*/
    wait_events = SSHD_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;
	
	while (TRUE)
    {
/*        switch ( SSHD_MGR_GetOpMode() )*/
        switch ( SSHD_MGR_GetOperationMode() )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:
                
                if( SSHD_TASK_IsProvisionComplete() == FALSE )
                {
        			SYSFUN_Sleep(10);
             		break;
                }
                /* The SSHD_TASK_Enter_Main_Routine() is a forever loop, and will 
                 * return (exit) only when SSHD subsystem enters TRANSITION_MODE/SLAVE_MODE.
                 */ 
                {
                    /* BODY
                     * Note: This is poor design!
                     */
                    if ( SSHD_MGR_GetSshdStatus() == SSHD_STATE_ENABLED )
                    {
                        SSHD_TASK_EnterMainRoutine();
                    }
                }    
        			 SYSFUN_Sleep(10);
                break;
    
            case SYS_TYPE_STACKING_TRANSITION_MODE:
                if ( (ret_value = SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY, 
                                                 SYSFUN_TIMEOUT_NOWAIT, &rcv_events)) != SYSFUN_OK )
                {
                    SYSFUN_Sleep(10);
       		        break;
                }
                
                event_var |= rcv_events;
                
                if (event_var==0)
                {
                	/*	Log to system : ERR--Receive Event Failure */
                    SYSFUN_Sleep(10);
                    break;
                }
    
	    		if (event_var & SSHD_TASK_EVENT_ENTER_TRANSITION )
	    		{
	    		    /* Check all ssh connection had disconnect */
                	session_record = SSHD_VM_GetSshdSessionRecord();
            	    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
                	{
    	                while(1)
                	    {
    	                    if ( (session_record->connection[i].keepalive == 0) && (session_record->connection[i].tid == 0) )
                	        {
		                        break;
            		        }
            		        else
		                    {
            		            SYSFUN_Sleep(10);
		                    }
            		    }
                	}
	    		    
	            	is_transition_done = TRUE;	/* Turn on the transition done flag */
        			event_var = 0;
        			is_provision_complete = FALSE;
	            }
                SYSFUN_Sleep(10);
                break;

            case SYS_TYPE_STACKING_SLAVE_MODE:
                /* Release allocated resource */
                event_var = 0;
                is_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;
                
            default:
                /* log error; */
                SYSFUN_Sleep(10);
                break;
            
        } /* End of switch */ 
        
    } /* End of while */    
	
} /* End of SSHD_TASK_Main () */



/* FUNCTION NAME:  SSHD_TASK_EnterMainRoutine
 * PURPOSE: 
 *			SSHD task main routine.
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
 *          This function is invoked in SSHD_TASK_Main().
 */
static void SSHD_TASK_EnterMainRoutine(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/
	
	/* LOCAL VARIABLES DECLARATIONS 
	*/
	struct  sockaddr_in     server_addr, client_addr;
    int                     sockfd, newsockfd, ret, addr_len, i, on = 1, id =0;
	UI32_T                  port, task_id;
    fd_set                  read_fds;
	struct 	timeval         timeout;
    UI8_T                   name[6];
    int                     rc;
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    struct  sockaddr    	peer_sock_addr;
    struct  sockaddr_in 	*peer_sock_in;
    int     				peer_sock_addr_size=sizeof(peer_sock_addr);
#endif

	/* BODY */
    /* set mode for roundrobin among sshd tasks */
    SYSFUN_EnableRoundRobin();

	/* Initiate socket */
	if ((sockfd = socket(AF_INET4, SOCK_STREAM, IP_PROT_TCP))<0)
	{
        UI8_T   *arg_p = "initiate";
	        
        EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
	    return;
    }

   	port = SSHD_MGR_GetSshdPort();

    memset((UI8_T *) &server_addr,0, sizeof(server_addr));
    server_addr.sin_family = AF_INET4;                   /* must be AF_INET */
    memset(&server_addr.sin_addr, 0, sizeof(server_addr.sin_addr));
    server_addr.sin_port = htons(port);                 /* select port 22 by default */

    /* bind */
    if ( bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    {
        UI8_T   *arg_p = "bind";
	        
        EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
	    s_close(sockfd);    /* close socket */
	    return;
    } 

    /* listen */
    if(listen (sockfd, 1) < 0)
    {
        UI8_T   *arg_p = "listen";
	        
        EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
       	s_close(sockfd);    /* close socket */
	    return;
    }
 
    /* prepare select */
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);
	timeout.tv_sec = 1;     /*  no.  of seconds  */
    timeout.tv_usec = 0;    /*  no. of micro seconds  */
 
    /* set socket options */
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (UI8_T *)&on, sizeof(on));

/*	while ( (SSHD_MGR_GetOpMode() == SYS_TYPE_STACKING_MASTER_MODE) && (SSHD_MGR_GetSshdStatus() == SSHD_STATE_ENABLED) )*/
    while ( (SSHD_MGR_GetOperationMode() == SYS_TYPE_STACKING_MASTER_MODE) && (SSHD_MGR_GetSshdStatus() == SSHD_STATE_ENABLED) )
    {
        /* select */
   		ret = select(sockfd+1, &read_fds, 0, 0, &timeout);  
        if (ret < 0 )
	    {
	   	    break;
	    }

        if ( (ret > 0) && (FD_ISSET(sockfd, &read_fds)) )
        {
            /* accept a new connection */
            addr_len = sizeof(client_addr);
			newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
			if (newsockfd < 0)
			{
                UI8_T   *arg_p = "accept";
	        
                EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
        	    s_close(sockfd);    /* close socket */
				return;
			}
            
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
	        /* isiah.2003-03-10 */
			getpeername(newsockfd, &peer_sock_addr, &peer_sock_addr_size);
			peer_sock_in = (struct sockaddr_in*) &peer_sock_addr;
	        if( MGMT_IP_FLT_MGR_IsValidIpFilterAddress(MGMT_IP_FLT_MGR_TELNET, (UI32_T)peer_sock_in->sin_addr) == FALSE )
			{
 		       	s_close(newsockfd);
 		       	//tnpd_exit();
	            continue;	
	        }
#endif

            /* proceed only if not exceeding max. # of concurrent sessions */
/* isiah.2004-01-02*/
/*move session number to CLI */
/*            if ( SSHD_VM_GetCreatedSessionNumber() >= SSHD_DEFAULT_MAX_SESSION_NUMBER )*/
            if ( CLI_MGR_IncreaseRemoteSession() == FALSE )
			{
				s_close(newsockfd);
				continue;
			}

            setsockopt(newsockfd, SOL_SOCKET, SO_KEEPALIVE, (UI8_T *)&on, sizeof(on));

            for ( i = 0 ; i < 100 ; i++ ) 
            {
                if ( ++id > 99 )
                {
                    id = 1;
                }
                sprintf(name, SYS_BLD_SSHD_CHILD_TASK, id);
                if (SYSFUN_TaskNameToID(name, &task_id))
                {
                    break;
                }
            }
	        if (i >= 100)
			{
                printf("sshd_task : No system resources for a new session\r\n");
                s_close(newsockfd);
                continue;
 			}
 
            if ( SYSFUN_SpawnTask (name, /* SYS_BLD_SSHD_CHILD_TASK */
                                   SYS_BLD_SSHD_CHILD_TASK_PRIORITY,
                                   SYS_BLD_TASK_LARGE_STACK_SIZE,
                                   SYSFUN_TASK_NO_FP,
                                   sshd_main,
                                   0, 
                                   &task_id) != SYSFUN_OK )
            {
                EH_MGR_Handle_Exception(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
                s_close(newsockfd);
/* isiah.2004-01-02*/
/*move session number to CLI */
                CLI_MGR_DecreaseRemoteSession();
                continue;
            }   /*  end of if (SYSFUN_SpawnTask())  */

            if ( SSHD_VM_SetTaskID(task_id) == TRUE )
            {
                /*isiah.2003-01-27*/
                //SYSFUN_SuspendTask (task_id);

                if ( ( rc = dup_sockusr(task_id) ) < 0 )
                {
                    UI8_T   *arg_p = "duplicate";
	        
                    EH_MGR_Handle_Exception1(SYS_MODULE_SSH, SSHD_TASK_EnterMainRoutine_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
/* isiah.2004-01-02*/
/*move session number to CLI */
                    CLI_MGR_DecreaseRemoteSession();
                    SYSFUN_DeleteTask(task_id);
                }
    
/*isiah.2003-02-10*/
#if 0
                SYSFUN_SetTaskPriority(task_id, SYS_BLD_SSHD_SERVER_TASK_PRIORITY);
                SYSFUN_ResumeTask(task_id);
#endif
               /*  end of SYSFUN_SpawnTask modification    */

                /* created sessions number + 1 */
/* isiah.2004-01-02*/
/*move session number to CLI */
/*                SSHD_VM_SetCreatedSessionNumber( SSHD_VM_GetCreatedSessionNumber()+1 );*/
            }
            else
            {
/* isiah.2004-01-02*/
/*move session number to CLI */
                CLI_MGR_DecreaseRemoteSession();
                SYSFUN_DeleteTask(task_id);
            }

            s_close(newsockfd);

        } /* end of if ( (ret > 0) && (FD_ISSET(sockfd, &read_fds)) )*/
        
        /* check for sshd_socket reopen */
        if ( sshd_task_is_port_changed == TRUE )
        {
            sshd_task_is_port_changed = FALSE;
            break;
        }
        
    } /* end of while */

    s_close(sockfd); /* close socket */

    for ( i=0 ; i<SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        if ( SSHD_MGR_CheckSshConnection(i) == TRUE )
        {
            CLI_MGR_SetKillWorkingSpaceFlag(i+1);
        }
    }

}



/* FUNCTION NAME:  SSHD_TASK_IsProvisionComplete
 * PURPOSE: 
 *          This function will check the SSHD module can start.
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
 *        
 */
BOOL_T SSHD_TASK_IsProvisionComplete(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
 
    /* BODY */
    return( is_provision_complete );
}







