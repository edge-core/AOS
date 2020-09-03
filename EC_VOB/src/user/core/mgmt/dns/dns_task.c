/* MODULE NAME: dns_task.c
 * PURPOSE:
 *   DNS initiation and DNS task creation
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2002-11-12      -- Isiah , created.
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdlib.h>

#include "sys_bld.h"
#include "sysfun.h"

//#include "iproute.h"

#include "dns.h"
#include "dns_type.h"
#include "dns_mgr.h"
#include "dns_vm.h"

#include "dns_resolver.h"
#include "dns_cmm.h"

extern void DNS_PROXY_Daemon(void);


/* NAMING CONSTANT DECLARATIONS
 */
#define DNS_TASK_EVENT_ENTER_TRANSITION    BIT_1

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* FUNCTION NAME:  DNS_TASK_ResolverMain
 * PURPOSE:
 *			Resolver starting routine.
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
 *          This function is invoked in DNS_TASK_CreateResolverTask().
 */
static void DNS_TASK_ResolverMain(void);



/* FUNCTION NAME:  DNS_TASK_ProxyMain
 * PURPOSE:
 *			Proxy starting routine.
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
 *          This function is invoked in DNS_TASK_CreateProxyDaemon().
 */
static void DNS_TASK_ProxyMain(void);



/* STATIC VARIABLE DECLARATIONS
 */
static  BOOL_T  is_resolver_transition_done;
static  BOOL_T  is_proxy_transition_done;
static  UI32_T  dns_resolver_task_id;
static  UI32_T  dns_proxy_task_id;
static  BOOL_T  is_resolver_provision_complete = FALSE;
static  BOOL_T  is_proxy_provision_complete = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  DNS_TASK_Init
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
 *          This function is invoked in DNS_INIT_Initiate_System_Resources.
 */
BOOL_T DNS_TASK_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  reset_status;
//	DNS_ResConfigSbelt_T* sbelt_p;

    /* BODY */
    is_resolver_transition_done = FALSE;
    is_proxy_transition_done = FALSE;

	if (NULL==DNS_VM_GetDnsSbelt())
	{
//		DNS_MGR_AddNameServer("202.96.209.5");
#if 0
        sbelt_p = malloc(sizeof(DNS_ResConfigSbelt_T));
		if(NULL == sbelt_p)
			return DNS_ERROR;							/* no sbelt,what can we do? */
		sbelt_p->next_p = NULL;
		sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr = dns_inet_addr("202.96.209.5");;
		strcpy(sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName,"dns.sbelt.cn");
		sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX; /* ??*/
		sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref = 1;
		strcpy(sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree,"");
		sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass = DNS_RRC_IN;	/* OK ?? */
		sbelt_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus = 0;
		DNS_MGR_SetDnsResConfigSbeltEntry(&(sbelt_p->dns_res_config_sbelt_entry));      /* ???  */
		free(sbelt_p);
#endif
	}

	reset_status = DNS_RES_RESET_INITIAL;
	DNS_VM_SetResResetStatus(reset_status);
	DNS_RESOLVER_Init();
	SYSFUN_Debug_Printf("DNS_ResolverInit..OK\n");
	DNS_PROXY_Init();
	SYSFUN_Debug_Printf("DNS_Proxy_init ....OK\n");

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DNS_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DNS_TASK_Create_InterCSC_Relation(void)
{
    return;
} /* end of DNS_TASK_Create_InterCSC_Relation */


/* FUNCTION NAME:  DNS_TASK_CreateResolverTask
 * PURPOSE:
 *			This function create dns resolver task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_INIT_Create_Tasks().
 */
BOOL_T DNS_TASK_CreateResolverTask(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/
	int  res_reset_status;

	/* BODY */
if(SYSFUN_SpawnThread(SYS_BLD_DNS_RESOLVER_CSC_THREAD_PRIORITY,
                          SYS_BLD_DNS_RESOLVER_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_DNS_RESOLVER_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          DNS_TASK_ResolverMain,
                          NULL,
                          &dns_resolver_task_id)!=SYSFUN_OK)
    {
        printf("\nDNS resolver task create failed! \n");
        return FALSE;
    }



    DNS_VM_SetResolverTaskId(dns_resolver_task_id);
	res_reset_status = DNS_RES_RESET_RUNNING;
	DNS_VM_SetResResetStatus(res_reset_status);

	return TRUE;

} /* end of DNS_TASK_CreateResolverTask() */



/* FUNCTION NAME:  DNS_TASK_CreateProxyDaemon
 * PURPOSE:
 *			This function create dns proxy daemon.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_INIT_Create_Tasks().
 */
BOOL_T DNS_TASK_CreateProxyDaemon(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/

	/* BODY */
    if(SYSFUN_SpawnThread(SYS_BLD_DNS_PROXY_CSC_THREAD_PRIORITY,
                          SYS_BLD_DNS_PROXY_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_DNS_PROXY_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          DNS_TASK_ProxyMain,
                          NULL,
                          &dns_proxy_task_id)!=SYSFUN_OK)
    {
        printf("\nDNS proxy task create failed! \n");
        return FALSE;
    }

	DNS_VM_SetServStatus(DNS_SERV_STARTED);
	DNS_VM_SetServResetStatus(VAL_dnsServConfigReset_running);
	DNS_VM_ServUpTimeInit();
	DNS_VM_ServResetTimeInit();

	return TRUE;

} /* end of DNS_TASK_CreateProxyDaemon() */



/* FUNCTION NAME : DNS_TASK_SetTransitionMode
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
void DNS_TASK_SetTransitionMode(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_resolver_transition_done = FALSE;
    is_proxy_transition_done = FALSE;
    SYSFUN_SendEvent(dns_resolver_task_id, DNS_TASK_EVENT_ENTER_TRANSITION);
    SYSFUN_SendEvent(dns_proxy_task_id, DNS_TASK_EVENT_ENTER_TRANSITION);
}



/* FUNCTION NAME : DNS_TASK_EnterTransitionMode
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
void DNS_TASK_EnterTransitionMode(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    /*	want task release all resources	*/

    DNS_MGR_ResetConfig(); /*maggie liu*/

	SYSFUN_TASK_ENTER_TRANSITION_MODE(is_resolver_transition_done);
	SYSFUN_TASK_ENTER_TRANSITION_MODE(is_proxy_transition_done);
}



/* FUNCTION NAME:  DNS_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the DNS module to start.
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
 *          This function is invoked in DNS_INIT_ProvisionComplete().
 */
void DNS_TASK_ProvisionComplete(void)
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_resolver_provision_complete = TRUE;
    is_proxy_provision_complete = TRUE;

}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  DNS_TASK_ResolverMain
 * PURPOSE:
 *			Resolver starting routine.
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
 *          This function is invoked in DNS_TASK_CreateResolverTask().
 */
static void DNS_TASK_ResolverMain(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/
    UI32_T	                wait_events;
    UI32_T	                rcv_events;
    UI32_T	                event_var;
    UI32_T	                ret_value;

	/* BODY */
    /*	Prepare waiting event and init. event var.	*/
    wait_events = DNS_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

	while (TRUE)
    {
        switch ( DNS_MGR_GetOperationMode() )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:

                if( is_resolver_provision_complete == FALSE )
                {
        			SYSFUN_Sleep(10);
             		break;
                }
                /* The DNS_TASK_Enter_Main_Routine() is a forever loop, and will
                 * return (exit) only when SSHD subsystem enters TRANSITION_MODE/SLAVE_MODE.
                 */
                {
                    /* BODY
                     * Note: This is poor design!
                     */
                    if ( DNS_ENABLE == DNS_MGR_GetDnsStatus() )
                    {
                        DNS_RESOLVER_Task();
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

	    		if (event_var & DNS_TASK_EVENT_ENTER_TRANSITION )
	    		{
	    		    DNS_RESOLVER_Disable();
	            	is_resolver_transition_done = TRUE;	/* Turn on the transition done flag */
        			event_var = 0;
        			is_resolver_provision_complete = FALSE;
	            }
                SYSFUN_Sleep(10);
                break;

            case SYS_TYPE_STACKING_SLAVE_MODE:
                /* Release allocated resource */
                event_var = 0;
                is_resolver_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;

            default:
                /* log error; */
                SYSFUN_Sleep(10);
                break;

        } /* End of switch */

    } /* End of while */

} /* End of DNS_TASK_ResolverMain () */



/* FUNCTION NAME:  DNS_TASK_ProxyMain
 * PURPOSE:
 *			Proxy starting routine.
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
 *          This function is invoked in DNS_TASK_CreateProxyDaemon().
 */
static void DNS_TASK_ProxyMain(void)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/
    UI32_T	                wait_events;
    UI32_T	                rcv_events;
    UI32_T	                event_var;
    UI32_T	                ret_value;

	/* BODY */
    /*	Prepare waiting event and init. event var.	*/
    wait_events = DNS_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

	while (TRUE)
    {
        switch ( DNS_MGR_GetOperationMode() )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:

                if( is_proxy_provision_complete == FALSE )
                {
        			SYSFUN_Sleep(10);
             		break;
                }
                /* The DNS_TASK_Enter_Main_Routine() is a forever loop, and will
                 * return (exit) only when SSHD subsystem enters TRANSITION_MODE/SLAVE_MODE.
                 */
                {
                    /* BODY
                     * Note: This is poor design!
                     */
                    if ( DNS_ENABLE == DNS_MGR_GetDnsStatus() )
                    {
                        DNS_PROXY_Daemon();
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

	    		if (event_var & DNS_TASK_EVENT_ENTER_TRANSITION )
	    		{
	    		    DNS_PROXY_Stop();
	            	is_proxy_transition_done = TRUE;	/* Turn on the transition done flag */
        			event_var = 0;
        			is_proxy_provision_complete = FALSE;
	            }
                SYSFUN_Sleep(10);
                break;

            case SYS_TYPE_STACKING_SLAVE_MODE:
                /* Release allocated resource */
                event_var = 0;
                is_proxy_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;

            default:
                /* log error; */
                SYSFUN_Sleep(10);
                break;

        } /* End of switch */

    } /* End of while */

} /* End of DNS_TASK_ProxyMain () */

#endif /* #if (SYS_CPNT_DNS == TRUE) */

