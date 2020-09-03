/* Module Name: dns_init.c
 * Purpose: DNS initiation and task creation
 * Notes:
 * History:
 *    09/06/02        -- simon zhou, Create
 *    11/08/02        -- Isiah, porting to ACP@2.0
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include "backdoor_mgr.h"

#include "dns.h"
#include "dns_init.h"
#include "dns_task.h"
#include "dns_mgr.h"
/*#include "dns_cmm.h"
#include "dns_resolver.h"*/

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/*isiah.move to dns.h*/
#if 0
enum DNS_ResService_E
{
	DNS_RES_SERVICE_RECURSIVE = 1,
	DNS_RES_SERVICE_ITERATIVE,
	DNS_RES_SERVICE_MIX
};

enum DNS_ResResetStatus_E
{
	DNS_RES_RESET_OTHER = 1,
	DNS_RES_RESET,
	DNS_RES_RESET_INITIAL,
	DNS_RES_RESET_RUNNING
};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
#if 0
/*
 * FUNCTION NAME : DNS_Init
 *
 * PURPOSE:
 *		DNS module entry
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
int DNS_Init()
{
	DNS_ResConfigSbelt_T* sbelt_p;
	int reset_status;
	if (DNS_MGR_Init()==FALSE)
		return DNS_ERROR;

/* isiah.move to DNS_OM_Init() */
	DNS_MGR_ResCounterInit();
	DNS_MGR_ServCounterInit();
/*-----------------------------*/

	if(DNS_DISABLE == DNS_MGR_GetDnsStatus())
		return DNS_ERROR;

    /*DNS_MGR_ShowDnsConfig();		*/
	if (NULL==DNS_MGR_GetDnsSbelt())
	{
		DNS_MGR_AddNameServer("202.96.209.5");
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
	   	sbelt_p = NULL;
	}
/*isiah.move to DNS_TASK_Init() */
	reset_status = DNS_RES_RESET_INITIAL;
	DNS_MGR_SetDnsResConfigReset(&reset_status);
	DNS_RESOLVER_Init();

	DNS_PROXY_Init();

/*--------------------------------------*/
	/*DNS_RESOLVER_DebugTest();*/

	return DNS_OK;

}
#endif



/* FUNCTION NAME:  DNS_INIT_Initiate_System_Resources
 * PURPOSE:
 *          This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
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
 *          1. This function must be invoked before any tasks in this subsystem can be created.
 *          2. This function must be invoked before any services in this subsystem can be executed.
 */
void DNS_INIT_InitiateSystemResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    /* initialise data */
    DNS_MGR_Init();
    DNS_TASK_Init();

	return;

} /* end of DNS_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DNS_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DNS_INIT_Create_InterCSC_Relation(void)
{
    DNS_MGR_Create_InterCSC_Relation();
    DNS_TASK_Create_InterCSC_Relation();

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("dns",
                                                      SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
                                                      DNS_MGR_BackdoorFunction);

} /* end of DNS_INIT_Create_InterCSC_Relation */


/* FUNCTION NAME:  DNS_INIT_Create_Tasks
 * PURPOSE:
 *          This function creates all the task of this subsystem.
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
 *          1. This function shall not be invoked before DNS_INIT_Initiate_System_Resources() is performed.
 */
void DNS_INIT_Create_Tasks(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_TASK_CreateResolverTask();
    DNS_TASK_CreateProxyDaemon();

	return;

}



/* FUNCTION NAME:  DNS_INIT_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the DNS subsystem will enter the
 *          Master Operation mode.
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
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *             switch will be initiated to the factory default value.
 *          2. DNS will handle network requests only when this subsystem
 *             is in the Master Operation mode
 */
void DNS_INIT_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_MGR_EnterMasterMode();

	return;

}



/* FUNCTION NAME:  DNS_INIT_EnterSlaveMode
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
void DNS_INIT_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_MGR_EnterSlaveMode();

	return;

}



/* FUNCTION NAME:  DNS_INIT_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Teansition Operation mode.
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
 *          In Transition Operation mode, any network requests
 *          will be ignored.
 */
void DNS_INIT_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_MGR_EnterTransitionMode();
    DNS_TASK_EnterTransitionMode();

	return;

}



/* FUNCTION	NAME : DNS_INIT_SetTransitionMode
 * PURPOSE:
 *		This call will set dns_mgr into transition mode to prevent
 *		calling request.
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
void DNS_INIT_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
	DNS_MGR_SetTransitionMode();
	DNS_TASK_SetTransitionMode();
}



/* FUNCTION NAME:  DNS_INIT_ProvisionComplete
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
 *          This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call DNS_TASK_ProvisionComplete().
 *          If it is necessary this function will call SSHD_MGR_ProvisionComplete().
 */
void DNS_INIT_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
	DNS_TASK_ProvisionComplete();
}



/* FUNCTION NAME - DNS_INIT_HandleHotInsertion
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
void DNS_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}



/* FUNCTION NAME - DNS_INIT_HandleHotRemoval
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
void DNS_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    DNS_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}



/* LOCAL SUBPROGRAM BODIES
 */

 #endif /* #if (SYS_CPNT_DNS == TRUE) */

