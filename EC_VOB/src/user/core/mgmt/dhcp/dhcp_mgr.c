/* Module Name: DHCP_MGR.C
 * Purpose:
 *      DHCP_MGR provides all DHCP accessing functions, include Provision Start/Complete,
 *      Restart. All task access-protections are kept in this module.
 *
 * Notes:
 *      1. CLI command includes :
 *          Enter Master/Slave/Transition mode.
 *          ProvisionComplete
 *          Bind DHCP to interface.
 *          Set Client ip, gateway_ip, server_ip
 *          Set role of interface, Client, Server, or Relay Agent.
 *      2. Execution function includes :
 *          Restart dhcp processing.
 *          Processing received packet
 *          Processing timeout request.
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *  0.2 2002.02.23  --  Penny, Modified
 *  0.3 2003.09.18  --  Jamescyl, Enhanced DHCP Relay Server APIs
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

#define  BACKDOOR_OPEN
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "dhcp_algo.h"
#include "dhcp_error_print.h"
#include "dhcp_mgr.h"
#include "dhcp_om.h"
#include "dhcp_time.h"
#include "dhcp_txrx.h"
#include "dhcp_type.h"
#include "dhcp_wa.h"
#include "dhcp_backdoor.h"
#include "eh_mgr.h"
#include "ip_lib.h"
#include "memory.h"
#include "netcfg_pom_main.h"
#include "netcfg_pom_ip.h"
#include "netcfg_type.h"
#include "syslog_type.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "sys_pmgr.h"
#include "netcfg_pmgr_route.h"
#include "l_threadgrp.h"
#include "ip_service_proc_comm.h"
#include "swctrl_pmgr.h"
#include "sys_time.h"
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE|| SYS_CPNT_DNS == TRUE )
#include "dns_pmgr.h"
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "swctrl_pom.h"
#include "dhcpsnp_pmgr.h"
#include "l4_pmgr.h"
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
#include "netcfg_pmgr_ip.h"
#endif

#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define ARPHRD_ETHER    1                   /* ethernet hardware format */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct DHCP_MGR_LCB_S
{
    UI32_T                      api_semaphore;              /*  Semaphore used in API protection    */
    DHCP_TYPE_SYSTEM_STATE_T    system_state;
    UI32_T                      object_mode;                /*  DHCP/BOOTP/USER_DEFINE  */
    BOOL_T                      dhcp_enable;
}   DHCP_MGR_LCB_T;


/*  System state : Master/Slave/Transition/ProvisionComplete    */
typedef void (*DHCP_TASK_PROVISION_COMPLETE_CALLBACK_T)(UI32_T restart_object);
/*  Mode : DHCP/BOOTP/USER_DEFINE   */
/*typedef void (*DHCP_TASK_CHANGE_MODE_CALLBACK_T)(UI32_T mode);*/


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T DHCP_MGR_CheckInterfaceCondition(UI32_T vid_ifIndex, UI32_T role);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
static UI32_T DHCP_MGR_AppendRelayServerAddress(UI32_T number_of_address,
        UI32_T address[],
        UI32_T current_number_of_address,
        UI32_T *current_address_p);
static UI32_T DHCP_MGR_DeleteRelayServerAddress(UI32_T number_of_address,
        UI32_T address[],
        UI32_T current_number_of_address,
        UI32_T *current_address_p);
static void DHCP_MGR_ShiftRelayServerAddress(UI32_T *relay_server_address);
static void DHCP_MGR_SortRelayServerAddress(UI32_T *relay_server_address);
#endif /* (SYS_CPNT_DHCP_SERVER == TRUE) */

#if (SYS_CPNT_DHCP_SERVER == TRUE)
static BOOL_T   DHCP_MGR_CheckPhysicalAddress(UI8_T *phy_addr);
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Status(UI32_T *status_p);
static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Policy(UI32_T *policy_p);
static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82RidMode(UI32_T *mode_p);
static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Format(BOOL_T *subtype_format_p);
static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningRelayServerAddress(UI32_T *relay_server);
#endif
/* STATIC VARIABLE DECLARATIONS
 */
static  DHCP_MGR_LCB_T                              dhcp_mgr_lcb;
static  DHCP_TASK_PROVISION_COMPLETE_CALLBACK_T dhcp_task_provision_complete_callback;
static  UI32_T                                      last_system_seconds;

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

/* DHCP server debug flag */
UI8_T   dhcp_server_debug_flag;

#if (SYS_CPNT_CRAFT_DHCLIENT == TRUE)
/* let CRAFT port be able to have IP address
 */
static void DHCP_MGR_HaveIP_CraftPort()
{
    NETCFG_TYPE_CraftInetAddress_T craft_addr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    memset(&craft_addr, 0, sizeof(craft_addr));
    craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr))
    {
        return;
    }
    else
    {

        /* if dhclient is invoked -> terminate it
         */
        FILE *fd=fopen(DHCLIENT_CRAFT_PID_FILE, "r");
        if (fd != NULL) /* it's still running */
        {
          //  sprintf(buf, "kill -15 `cat %s`", DHCLIENT_CRAFT_ZTP_PID_FILE);
          //  system(buf); /* to terminate process */
           fclose(fd);
           return;
        }
        else /* dhclient for craft is not running from AOS */
        {
            system("dhclient -nw -q -pf "DHCLIENT_CRAFT_PID_FILE" CRAFT &");
        }
    }
}
#endif
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : DHCP_MGR_Init
 * PURPOSE:
 *      Initialize DHCP_MGR used system resource, eg. protection semaphore.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
#define DHCP_SEM_EMPTY  0
#define DHCP_SEM_FULL   1

    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
#if (SYS_CPNT_DHCP_SERVER)
    DHCP_MEMORY_Init();
#endif
    /*  Create semaphore.
     */
    memset((char*)&dhcp_mgr_lcb, 0, sizeof(DHCP_MGR_LCB_T));
    dhcp_task_provision_complete_callback = NULL;

}   /*  end of DHCP_MGR_Init    */


/* FUNCTION NAME : DHCP_MGR_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_EnterMasterMode(void)
{

    dhcp_mgr_lcb.system_state = DHCP_TYPE_SYSTEM_STATE_MASTER;

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCP_OM_SetDhcpRelayOption82Status(SYS_DFLT_DHCP_OPTION82_STATUS);
    DHCP_OM_SetDhcpRelayOption82Policy(SYS_DFLT_DHCP_OPTION82_POLICY);
    DHCP_OM_SetDhcpRelayOption82RidMode(SYS_DFLT_DHCP_OPTION82_RID_MODE);
    DHCP_OM_SetDhcpRelayOption82Format(SYS_DLFT_DHCP_OPTION82_SUBTYPE_FORMAT);
#endif

    /* init last system seconds */
    last_system_seconds = SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND;
    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

}   /*  end of DHCP_MGR_EnterMasterMode */

/* FUNCTION NAME : DHCP_MGR_GetOperationMode
 * PURPOSE:
 *      Get current dhcp operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      operation_mode -- DHCP_TYPE_STACKING_TRANSITION_MODE | DHCP_TYPE_STACKING_MASTER_MODE |
 *                        DHCP_TYPE_SYSTEM_STATE_SLAVE | DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE.
 *
 * NOTES:
 *      None.
 */
UI32_T DHCP_MGR_GetOperationMode(void)
{
    return dhcp_mgr_lcb.system_state;
}   /*  end of DHCP_MGR_GetOperationMode    */

/* FUNCTION NAME : DHCP_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();

    /*  Record the system state and
     *  send signal to DHCP_TASK
     */
    dhcp_mgr_lcb.system_state = SYS_TYPE_STACKING_TRANSITION_MODE;

}   /*  end of DHCP_MGR_SetTransitionMode   */

/* FUNCTION NAME : DHCP_MGR_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_EnterTransitionMode(void)
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();

    /*  Record the system state and
     *  send signal to DHCP_TASK
     */
    dhcp_mgr_lcb.system_state = DHCP_TYPE_SYSTEM_STATE_TRANSITION;

    /* clear timer */
    DHCP_TIME_ClearList();

    /* clear OM including server OM -- memory.c */
    DHCP_OM_ReInitAllDatabases();

    /* clear WA */
    DHCP_WA_ReInit();

    DHCP_ALGO_Init(); /* set bootp_packet_handler = NULL */

}   /*  end of DHCP_MGR_EnterTransitionMode */


/* FUNCTION NAME : DHCP_MGR_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_EnterSlaveMode(void)
{

    dhcp_mgr_lcb.system_state = DHCP_TYPE_SYSTEM_STATE_SLAVE;

    SYSFUN_ENTER_SLAVE_MODE();
}   /*  end of DHCP_MGR_EnterSlaveMode  */

/* FUNCTION NAME : DHCP_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void DHCP_MGR_Create_InterCSC_Relation(void)
{
    return;
}


/* FUNCTION NAME : DHCP_MGR_CheckSystemState
 * PURPOSE:
 *      To check current system state [master | slave | transition | provision c]
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      (DHCP_TYPE_SYSTEM_STATE_T) dhcp_mgr_lcb.system_state
 *
 * NOTES:
 *      None.
 */
int DHCP_MGR_CheckSystemState(void)
{
    return (int)dhcp_mgr_lcb.system_state;
}

/* FUNCTION NAME : DHCP_MGR_CheckRestartObj
 * PURPOSE:
 *      To check current system role server /relay/client
 *
 * INPUT:
 *      UI32_T   restart_object     --DHCP_MGR_RESTART_RELAY
  *                                          --  DHCP_MGR_RESTART_SERVER
  *                                           -- DHCP_MGR_RESTART_CLIENT
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
  *      DHCP_MGR_DYNAMIC_IP
  *      DHCP_MGR_SERVER_ON
   *     DHCP_MGR_RELAY_ON
 *
 * NOTES:
 *      None.
 */

UI32_T DHCP_MGR_CheckRestartObj(UI32_T restart_object)
{
    UI32_T address_mode;
    UI32_T vid_ifindex;

    if (restart_object == DHCP_MGR_RESTART_RELAY || restart_object == DHCP_MGR_BIND_SERVER)
    {

        if (restart_object == DHCP_MGR_RESTART_RELAY)
        {
            if (DHCP_OM_IsServerOn())
                return DHCP_MGR_SERVER_ON;
        }
        else
            if (DHCP_OM_IsRelayOn())
                return DHCP_MGR_RELAY_ON;

        vid_ifindex = 0;
        while (NETCFG_POM_IP_GetNextIpAddressMode(&vid_ifindex, &address_mode) == NETCFG_TYPE_OK)
        {
            if (address_mode == DHCP_TYPE_INTERFACE_MODE_USER_DEFINE)
                return DHCP_MGR_OK;
        }

        return DHCP_MGR_DYNAMIC_IP;

    }

    return DHCP_MGR_OK;
}

/* FUNCTION NAME : DHCP_MGR_ProvisionComplete
 * PURPOSE:
 *      Be signaled by STKCTRL, CLI had provision all configuration commands, all
 *      CSC should begin do his job.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. ProvisionComplete, task should begins his job. The job is not implememted in
 *         MGR, TASK takes the responsibility.
 *      2. This function is called by STKCTRL.
 *
 */
void DHCP_MGR_ProvisionComplete()
{
#if (SYS_CPNT_CRAFT_DHCLIENT == TRUE)
        DHCP_MGR_HaveIP_CraftPort();
#endif
    /* Penny waits for porting L3 to 2.0 to verify should it be none or client org is client */

    /* 2009-10-14 Jimi, It should restart all object when provision is completed */
    DHCP_MGR_ProvisionComplete3(DHCP_MGR_RESTART_CLIENT);

    return;
}

/* FUNCTION NAME : DHCP_MGR_ProvisionComplete3
 * PURPOSE:
 *      Be signaled by STKCTRL, CLI had provision all configuration commands, all
 *      CSC should begin do his job.
 *
 * INPUT:
 *      restart_object
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. ProvisionComplete, task should begins his job. The job is not implememted in
 *         MGR, TASK takes the responsibility.
 *      2. This function is called by STKCTRL.
 *      3. Possible Value for restart_object:
 *          DHCP_MGR_RESTART_NONE
 *          DHCP_MGR_RESTART_CLIENT
 *          DHCP_MGR_RESTART_SERVER
 *          DHCP_MGR_RESTART_RELAY
 */
void DHCP_MGR_ProvisionComplete3(UI32_T restart_object)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        /*  Record the system state and
         *  send signal to DHCP_TASK
         */

        dhcp_mgr_lcb.system_state = DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE;
        DHCP_BD(EVENT, "Clear timer list");
        DHCP_TIME_ClearList();
        DHCP_MGR_ReactiveProcessing(restart_object);
        DHCP_MGR_Dispatch();
    }

}   /*  end of DHCP_MGR_ProvisionComplete3  */


/* FUNCTION NAME : DHCP_MGR_Register_ProvisionComplete_Callback
 * PURPOSE:
 *      Register SystemStateChanged function of DHCP_TASK. Each time, STK_CTRL signaling
 *      system, MGR should signal TASK.
 *
 * INPUT:
 *      function    -- handling routine of callback function.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Before provision, each CSC will enter transition mode, then enter master mode,
 *         begin provision.
 *      2. This function is for DHCP_TASK_SystemStateChanged_Callback.
 */
void DHCP_MGR_Register_ProvisionComplete_Callback(void *function)
{
    dhcp_task_provision_complete_callback = (DHCP_TASK_PROVISION_COMPLETE_CALLBACK_T) function;
}   /*  end of DHCP_MGR_Register_SystemStateChanged_Callback    */

/* FUNCTION NAME : DHCP_MGR_ReactiveProcessing
 * PURPOSE:
 *      Restart function to starting stage, discovery all interfaces and build
 *      working data structures.
 *
 * INPUT:
 *      restart_object - the specific object need to restart
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This function performs 3 functions :
 *         a. Clear DHCP_OM (not include DHCP_WA) and DHCP_ALGO.
 *         b. Read configuration and lease from DHCP_WA to DHCP_OM.
 *         b. Discover Interface
 *         c. Construct DHCP_OM/protocols link list.
 *      2. Possible Value for restart_object:
 *          DHCP_TYPE_RESTART_NONE
 *          DHCP_TYPE_RESTART_CLIENT
 *          DHCP_TYPE_RESTART_SERVER
 *          DHCP_TYPE_RESTART_RELAY
 */
void DHCP_MGR_ReactiveProcessing(UI32_T restart_object)
{

    struct interface_info   *if_pointer, *if_ptr_addr;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }

    DHCP_BD(EVENT, "restart object[%lu]", (unsigned long)restart_object);

    /*
         *  Clear all data in DHCP_OM include interface info.
         *  We will build up a updated interface info list
         */
    if_ptr_addr = NULL;
    if_pointer = DHCP_OM_GetNextInterface(if_ptr_addr);
    while (if_pointer != NULL)
    {
        if (!(if_pointer->role & DHCP_TYPE_BIND_CLIENT))
        {
            if_pointer = if_pointer->next;
            continue;
        }
        DHCP_MGR_DeleteClientDefaultGateway(if_pointer->vid_ifIndex);
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
        DHCP_MGR_DeleteClientNameServer(if_pointer->vid_ifIndex);
#endif /*#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)*/
        if_pointer = if_pointer->next;
    }
    DHCP_OM_ReInit(restart_object);

    if (restart_object & DHCP_TYPE_RESTART_SERVER)
    {
#if (SYS_CPNT_DHCP_SERVER == TRUE)
        /* clear active list */
        DHCP_BD(SERVER, "Clear active pool and pool config");
        DHCP_WA_InitActivePool();
        DHCP_MEMORY_ReInit();

        readconf();    //-- > move WA's config to all parse functions
        /* Set up the bootp packet handler... */
        //bootp_packet_handler = do_packet;
        DHCP_ALGO_SetupServerHandler();
#endif
        DHCP_WA_ClearRestartObject(DHCP_TYPE_RESTART_SERVER);
    }

    /*build dhcp om interface from netcfg*/
    if (!discover_interfaces(restart_object))
    {
        return;
    }

    if (restart_object & DHCP_TYPE_RESTART_RELAY)
    {
#if (SYS_CPNT_DHCP_RELAY == TRUE)
        /* Set up the bootp packet handler... */
        //bootp_packet_handler = relay;
        DHCP_ALGO_SetupRelayHandler();
        DHCP_WA_ClearRestartObject(DHCP_TYPE_RESTART_RELAY);
#endif
    }

    if (restart_object & DHCP_TYPE_RESTART_CLIENT)
    {
        /* check the interface in OM which binding role is client and see
         * if ip address method in NETCFG for this vid_ifIndex is the same
         * as here. If different, which means user just change the current
         * interface's ip address method. If user just changes from client
         * to user-defined, just update interface ip address method from
         * client to user-defined.
         */
        if_ptr_addr = NULL;
        if_pointer = DHCP_OM_GetNextInterface(if_ptr_addr);
        while (if_pointer != NULL)
        {
            if (if_pointer->role != DHCP_TYPE_BIND_CLIENT)
            {
                if_pointer = if_pointer->next;
                continue;
            }
            /* Parse the dhclient.conf file. */
            read_client_conf(if_pointer->vid_ifIndex);

            /* Parse the lease database. */
            read_client_leases(if_pointer->vid_ifIndex);

            if_pointer->client->state = S_INIT;
            state_reboot(if_pointer);
            if_pointer = if_pointer->next;
        }
        DHCP_WA_ClearRestartObject(DHCP_TYPE_RESTART_CLIENT);
    }
}

/* FUNCTION NAME : DHCP_MGR_Restart3
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *      (change to provision complete) and inform DHCP task to do
 *       the restart.
 *
 *
 * INPUT:
 *      restart_object
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *          instead, it will activate restart in DHCP_task when get
 *          provision complete state
 *        2. Possible Value for restart_object:
 *          DHCP_MGR_RESTART_NONE
 *          DHCP_MGR_RESTART_CLIENT
 *          DHCP_MGR_RESTART_SERVER
 *          DHCP_MGR_RESTART_RELAY
 *
 */
void DHCP_MGR_Restart3(UI32_T restart_object)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    DHCP_MGR_ProvisionComplete3(restart_object);


}/* end of DHCP_MGR_Restart3 */

/* FUNCTION NAME : DHCP_MGR_Restart
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *      (change to provision complete) and inform DHCP task to do
 *       the restart.
 *
 *
 * INPUT:
 *      restart_object
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *          instead, it will activate restart in DHCP_task when get
 *          provision complete state
 *        2. Possible Value for restart_object:
 *          DHCP_TYPE_RESTART_NONE
 *          DHCP_TYPE_RESTART_CLIENT
 *          DHCP_TYPE_RESTART_SERVER
 *          DHCP_TYPE_RESTART_RELAY
 *
 */
void    DHCP_MGR_Restart()
{
    DHCP_MGR_Restart3(DHCP_TYPE_RESTART_CLIENT);

} /* DHCP_MGR_Restart */

/* FUNCTION NAME : DHCP_MGR_IsSocketDirty
 * PURPOSE:
 *      To retrieve socket dirty status from engine
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_MGR_IsSocketDirty(void)
{
    return DHCP_ALGO_IsSocketDirty();
}

/* FUNCTION NAME : DHCP_MGR_Dispatch
 * PURPOSE:
 *      To receive packet thru socket under semaphore protection
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_Dispatch()
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (dhcp_mgr_lcb.system_state == DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE)
    {
        dispatch();
    }
} /* end of DHCP_MGR_Dispatch */


/* FUNCTION NAME : DHCP_MGR_do_packet
 * PURPOSE:
 *      convert received DHCP packet to do_packet() acceptable parameters and
 *      call do_packet() to take some proper processing.
 *
 * INPUT:
 *      mem_ref         --  holder of received packet buffer.
 *      packet_length   --  received packet length.
 *      rxRifNum            --  the RIF packet coming,
 *                          -1 if not a configured interface.
 *      dst_mac         --  the destination hardware mac address in frame.
 *      src_mac         --  the source hardware mac address in frame.
 *      vid             --  the vlan the packet coming from.
 *      src_lport_ifIndex-- the ingress physical port ifIndex.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DHCP_MGR_do_packet(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T packet_length,
    UI32_T ifindex,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI32_T vid,
    UI32_T src_lport_ifIndex)
{
    DHCP_TYPE_IpHeader_T    *ip_pkt;
    DHCP_TYPE_UdpHeader_T   *udp_header;
    struct interface_info   *interface;
    struct dhcp_packet      *packet;
    struct hardware         hfrom;
    struct iaddr            from;
    int                     len;
    unsigned int            from_port;
    UI32_T                  vidIfIndex;
    UI32_T                  src_ip, pdu_len;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        goto release_mref;
    }

    /* Base on input parameter, find out all parameters needed
     *  by do_packet(). And call do_packet to process received packet.
     */
    if (mref_handle_p == NULL)
    {
        return;
    }

    /* construct hardware, iaddr, from_port.
     */
    /*  incoming hardware address from src_mac  */
    hfrom.htype = ARPHRD_ETHER;
    hfrom.hlen  = 6;
    memcpy(hfrom.haddr, src_mac, 6);
    /*  copy 'ip-addr' from ip-header in packet */
    ip_pkt = (DHCP_TYPE_IpHeader_T*) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    from.len    = 4;
    memcpy(from.iabuf, &(ip_pkt->sip), 4);
    /*  2002.01.09, William, save (ip,mac) in TXRX.
     *              Because DHCP never send h/w mac.
     *  Keep (source-ip, source-mac) pair in DHCP_TXRT.
     *  1. Transfer srouce ip to host format
     *  2. Save to DHCP_TXRX
     */
    /*src_ip = nltoh(&ip_pkt->sip); */
    src_ip = ip_pkt->sip;

    DHCP_TXRX_SaveIpMac(src_ip, ifindex, src_mac);

    /*  get from_port from udp header source port   */
    udp_header = (DHCP_TYPE_UdpHeader_T*)((UI8_T*)ip_pkt + sizeof(DHCP_TYPE_IpHeader_T));
    from_port = ntohl(udp_header->src_port);
    /*  packet point to udp-packet  */
    packet = (struct dhcp_packet*)((UI8_T*)udp_header + sizeof(DHCP_TYPE_UdpHeader_T));
    /*  len */
    len = packet_length - sizeof(DHCP_TYPE_IpHeader_T) - sizeof(DHCP_TYPE_UdpHeader_T);

    /* L2 relay won't create DHCP interface,
     * so we must ignore interface checking when we enable L2 relay
     */
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    {
        UI32_T option82_status = 0;
        UI8_T  cpu_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

        /* If L2 relay is enabled, handover packet to DHCP_ALGO_Handle_Option82()
         */
        if (DHCP_OM_OK == DHCP_OM_GetDhcpRelayOption82Status(&option82_status))
        {
            if (option82_status == DHCP_OPTION82_ENABLE)
            {
                /* Get system mac_address
                 */
                if (SWCTRL_POM_GetCpuMac(cpu_mac))
                {
                    /* If client's hardware address is not DUT,
                     * this should be processed by L2 relay
                     */
                    if (memcmp(cpu_mac, packet->chaddr, SYS_ADPT_MAC_ADDR_LEN) != 0)
                    {
                        DHCP_ALGO_Handle_Option82(vidIfIndex,
                                                  mref_handle_p,
                                                  packet_length,
                                                  ifindex,
                                                  dst_mac,
                                                  src_mac,
                                                  vid,
                                                  src_lport_ifIndex);
                        return;
                    }
                }
            }
        }

    }
#endif /* End of #if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE) */

    /* Base on dst_mac or vid, search interface-list, find out which interface
     * belong to.
     */
    VLAN_OM_ConvertToIfindex(vid, &vidIfIndex);
    interface = DHCP_OM_FindInterfaceByVidIfIndex(vidIfIndex);

    if (interface == NULL)
    {
        goto release_mref;
    }

    /* Drop incoming not Client packet.  */
    if (interface->role != DHCP_TYPE_BIND_CLIENT)
    {
        DHCP_BD(PACKET, "L2 relay drop packet, role %lu", (unsigned long)interface->role);
        goto release_mref;
    }

    if (interface->mode == DHCP_TYPE_INTERFACE_MODE_USER_DEFINE)
    {
#if (SYS_CPNT_DHCP_INFORM == TRUE)
        if ((interface->dhcp_inform == FALSE))
#endif /* SYS_CPNT_DHCP_INFORM */
        {
            goto release_mref;
        }
    }

    /* Call handling routine. */
    do_packet(interface, packet, len, from_port, from, &hfrom);

release_mref:
    /* Free the incoming packet.
     * in do_packet(), new mem_ref will be constructed by DHCP_TXRX,
     * so we need to release old mem_ref
     */
    if (L_MM_Mref_Release(&mref_handle_p) == L_MM_ERROR_RETURN_VAL)
    {
        /*  Log to system : Invalid incoming packet */
		SYSFUN_LogErrMsg("Failed to release mref.");
    }
    return;
}


/* FUNCTION NAME : DHCP_MGR_CheckTimeout
 * PURPOSE:
 *      Call DHCP_TIME_IsTimeout to check whether a timeout request occurs,
 *      if yes, call the handle-function
 *
 * INPUT:
 *      ticks -- system ticks from system started.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This api is for protection purpose, prevent DHCP_TASK call to lower function.
 */
void DHCP_MGR_CheckTimeout(UI32_T ticks)
{

    DHCP_FUNC   where;
    void        *what;
    DHCP_TIME   time_ticks = (DHCP_TIME) ticks;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* if time ticks reaches the maximum value,
     * all timeout list should minus this value to recount from 0
     */
    if (time_ticks < last_system_seconds)
    {
        DHCP_BD(EVENT, "time ticks flip, re-calculate");
        DHCP_TIME_ReCalTimeout(time_ticks);
    }

    while (DHCP_TIME_IsTimeout(time_ticks, &where, (void**)&what))
    {
        DHCP_BD(EVENT, "timer expired");
        (where)(what);
    }

    /* update last system seconds */
    last_system_seconds = time_ticks;


}   /*  end of DHCP_MGR_CheckTimeout    */



/*-----------------------------------
 *  CLI provision command function.
 *-----------------------------------
 *  1. all configuration commands are saved in DHCP_WA,
 *     not set to DHCP_OM.
 *  2. Interface Bind-checking is done in provision-command.
 *     eg. CLIENT, SERVER can not be coexisted on same interface.
 */

/* FUNCTION NAME : DHCP_MGR_SetIfRole
 * PURPOSE:
 *      Define interface role in DHCP; this info. is kept in DHCP_WA.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      role        -- one of { client | server | relay }
 *                      DHCP_TYPE_INTERFACE_BIND_CLIENT
 *                      DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully.
 *      DHCP_MGR_NO_IP -- interface does not have ip bind to it
 *      DHCP_MGR_DYNAMIC_IP -- the interface is acting as client (bootp/dhcp)
 *      DHCP_MGR_SERVER_ON -- the system is running as a dhcp server
 *      DHCP_MGR_FAIL   --  fail to set interface role.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 */
UI32_T DHCP_MGR_SetIfRole(UI32_T vid_ifIndex, UI32_T role)
{
    /*  check system stacking mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "Set vid_ifindex[%lu],role[%lu]", (unsigned long)vid_ifIndex, (unsigned long)role);

    if (!DHCP_WA_SetIfBindingRole(vid_ifIndex, role))
    {
        return DHCP_MGR_FAIL;
    }

    return DHCP_MGR_OK;

}   /*  end of DHCP_MGR_SetIfRole   */

UI32_T DHCP_MGR_SetIfStatus(UI32_T vid_ifIndex, UI32_T status)
{
    /*  check system stacking mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(EVENT, "vid_ifindex[%lu],status[%lu]", (unsigned long)vid_ifIndex, (unsigned long)status);

    if (status == DHCP_MGR_CLIENT_DOWN)
    {
        DHCP_MGR_DeleteClientDefaultGateway(vid_ifIndex);
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
        DHCP_MGR_DeleteClientNameServer(vid_ifIndex);
#endif /*#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)*/
        DHCP_TIME_ClearList();
    }

    return DHCP_MGR_OK;


}

/* FUNCTION NAME : DHCP_MGR_DeleteIfRole
 * PURPOSE:
 *      Delete interface role to WA
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be configured.
 *      role    -- client / relay
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully.
 *      DHCP_MGR_FAIL   --  fail to set the interface role.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DeleteIfRole(UI32_T vid_ifIndex, UI32_T role)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "vid_ifindex[%lu],role[%lu]", (unsigned long)vid_ifIndex, (unsigned long)role);

    ret = DHCP_WA_DeleteIfBindingRole(vid_ifIndex, role);

    if (ret == TRUE)
    {
        return DHCP_MGR_OK;
    }

    return DHCP_MGR_FAIL;

} /* end of DHCP_MGR_DeleteIfRole */

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCP_MGR_Destroy_If
 *------------------------------------------------------------------------------
 * PURPOSE: Destory DHCP interface via L3 interface destroy callback
 * INPUT :  vid_ifindex --  vlan id ifindex
 * OUTPUT:  None
 * RETURN:  DHCP_MGR_FAIL/DHCP_MGR_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_MGR_Destroy_If(UI32_T vid_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(EVENT, "vid_ifindex[%lu]", (unsigned long)vid_ifindex);

    DHCP_OM_DestroyIf(vid_ifindex);

    DHCP_WA_DestroyIfDhcpInfo(vid_ifindex);

    /* check if dhcp rule need to be cancelled
     */
    DHCP_ALGO_SetDhcpPacketRule();
    return DHCP_MGR_OK;

}

/*add by simon shih*/
BOOL_T DHCP_MGR_SetClientDefaultGateway(UI32_T vid_ifindex, UI8_T default_gateway[SYS_ADPT_IPV4_ADDR_LEN])
{
    UI8_T om_gateway[SYS_ADPT_IPV4_ADDR_LEN];
    L_INET_AddrIp_T gw_addr;
    UI32_T ret;
    UI32_T zeroaddr = 0;

    DHCP_BD(CONFIG, "Set vid_ifindex[%lu],gateway[%u.%u.%u.%u]",
            (unsigned long)vid_ifindex, L_INET_EXPAND_IP(default_gateway));

    /* for DHCP , there should be only one default gateway that can be set to netcfg */
    /* overrwrited mode: last config overwrite previous */
    memset(&gw_addr, 0, sizeof(gw_addr));
    gw_addr.type = L_INET_ADDR_TYPE_IPV4;
    gw_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

    if (memcmp(&zeroaddr, default_gateway, sizeof(UI32_T))==0)
    {
        return FALSE;
    }

    if (DHCP_OM_GetClientDefaultGateway(om_gateway))
    {
        if (memcmp(om_gateway, default_gateway, sizeof(om_gateway)) != 0)
        {
            memcpy(gw_addr.addr, om_gateway, SYS_ADPT_IPV4_ADDR_LEN);
            NETCFG_PMGR_ROUTE_DeleteDhcpDefaultGateway(&gw_addr);

            DHCP_OM_SetClientDefaultGateway(default_gateway);
            memcpy(gw_addr.addr, default_gateway, SYS_ADPT_IPV4_ADDR_LEN);
            NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway(&gw_addr);
        }
    }
    else
    {
        DHCP_OM_SetClientDefaultGateway(default_gateway);
        memcpy(gw_addr.addr, default_gateway, SYS_ADPT_IPV4_ADDR_LEN);

        if (NETCFG_TYPE_OK != (ret = NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway(&gw_addr)))
        {
            DHCP_BD(CONFIG, "Failed to set default gateway to netcfg_route");
            return FALSE;
        }
    }

    return TRUE;
}

BOOL_T DHCP_MGR_DeleteClientDefaultGateway(UI32_T vid_ifindex)
{
    L_INET_AddrIp_T gw_addr;

    struct interface_info   *if_pointer, *if_null;
    void *data_ptr;
    int i;
    BOOL_T in_use = FALSE;
    UI8_T om_gateway[SYS_ADPT_IPV4_ADDR_LEN];
    UI8_T zero_gateway[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T ret;

    DHCP_BD(CONFIG, "Set vid_ifindex[%lu]", (unsigned long)vid_ifindex);

    if (!DHCP_OM_GetClientDefaultGateway(om_gateway))
    {
        /* nothing to delete */
        return TRUE;
    }

    memset(&gw_addr, 0, sizeof(gw_addr));

    gw_addr.type = L_INET_ADDR_TYPE_IPV4;
    gw_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    memcpy(gw_addr.addr, om_gateway, 4);

    DHCP_BD(CONFIG, "Remove default gateway[%d.%d.%d.%d]", L_INET_EXPAND_IP(om_gateway));
    if_null = NULL;

    for (if_pointer = DHCP_OM_GetNextInterface(if_null);if_pointer != NULL;if_pointer = if_pointer->next)
    {
        if (if_pointer->vid_ifIndex == vid_ifindex)
            continue;

        if (!if_pointer->client)
            continue;

        if (!if_pointer->client->active)
            continue;

        for (i = 0;i < if_pointer->client->active->options[DHO_ROUTERS].len / 4;i++)
        {
            data_ptr = if_pointer->client->active->options[DHO_ROUTERS].data + i * 4;
            DHCP_BD(CONFIG, "Check %d.%d.%d.%d", L_INET_EXPAND_IP(data_ptr));

            if (memcmp(gw_addr.addr, data_ptr, SYS_ADPT_IPV4_ADDR_LEN) == 0)
            {
                DHCP_BD(CONFIG, "Gateway %d.%d.%d.%d used by vlan %ld",
                        L_INET_EXPAND_IP(gw_addr.addr), (unsigned long)if_pointer->vid_ifIndex);
                in_use = TRUE;
                break;
            }
        }
    }

    if (FALSE == in_use)
    {
        DHCP_BD(CONFIG, "Delet gateway[%d.%d.%d.%d]", L_INET_EXPAND_IP(gw_addr.addr));
        DHCP_OM_SetClientDefaultGateway(zero_gateway);

        if (NETCFG_TYPE_OK != (ret = NETCFG_PMGR_ROUTE_DeleteDhcpDefaultGateway(&gw_addr)))
        {
            DHCP_BD(CONFIG, "Failed to delete gateway from netcfg_route");
            return FALSE;
        }

    }

    return TRUE;
}




/* FUNCTION NAME : DHCP_MGR_RemoveSystemRole
 * PURPOSE:
 *      Remove system role (server / server + client / relay + client / relay / client )
 *      to OM.
 *
 * INPUT:
 *      role    -- the role to be removed from system
 *
 * OUTPUT:
 *
 * RETURN:
 *      DHCP_MGR_OK     -- Successfully set the system role
 *      DHCP_MGR_FALSE   -- Can't set the system role
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_MGR_RemoveSystemRole(UI32_T role)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "role[%lu]", (unsigned long)role);

    ret = DHCP_OM_RemoveSystemRole(role);

    if (ret == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "Fail to Remove System Role");
        return DHCP_MGR_FAIL;
    }
#if(SYS_CPNT_DHCP_SERVER == TRUE)
    /* Clear active pool list in wa */
    DHCP_WA_InitActivePool();
#endif
    return DHCP_MGR_OK;


}/* end of DHCP_MGR_RemoveSystemRole */

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/* FUNCTION NAME : DHCP_MGR_SetIfRelayServerAddress
 * PURPOSE:
 *      Set dhcp server ip address for relay agent to WA
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be configured.
 *      ip1, ip2, ip3, ip4, ip5 -- the ip list of servers
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   --  fail to set the relay server address.
 *
 * NOTES:
 *  1. The function will delete the previous setting for relay server address
 *      and add the newly configured relay server address
 *  2. The maximun numbers of server ip are 5. If user specifies (ip1, ip2,0,0,0) which means
 *      the user only specifies 2 DHCP Relay Server.
 *  3. If user specifies (ip1, 0,ip3, ip4, ip5), (ip1,0,0,0,0)will be set to DHCP
 *      Relay Server Address List
 *
 */
UI32_T DHCP_MGR_SetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T ip1, UI32_T ip2, UI32_T ip3, UI32_T ip4, UI32_T ip5)
{
    int i;
    UI32_T temp[5] = {0, 0, 0, 0, 0};
    UI32_T rv;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "vid_ifindex[%lu]\r\n"
            "ip1[%u.%u.%u.%u]\r\n"
            "ip2[%u.%u.%u.%u]\r\n"
            "ip3[%u.%u.%u.%u]\r\n"
            "ip4[%u.%u.%u.%u]\r\n"
            "ip5[%u.%u.%u.%u]",
            (unsigned long)vid_ifIndex,
            L_INET_EXPAND_IP(ip1),
            L_INET_EXPAND_IP(ip2),
            L_INET_EXPAND_IP(ip3),
            L_INET_EXPAND_IP(ip4),
            L_INET_EXPAND_IP(ip5));

    /* 2003-2-10, Penny: Reconfirm that server is not on before set relay server configuration
     *  to the system
     */
    rv = DHCP_MGR_CheckInterfaceCondition(vid_ifIndex, DHCP_MGR_BIND_RELAY);
    if (rv != DHCP_MGR_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Set Interface Relay Server Address");

        return rv;
    }

    temp[0] = ip1;
    temp[1] = ip2;
    temp[2] = ip3;
    temp[3] = ip4;
    temp[4] = ip5;

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
    {
        if (IP_LIB_IsIpInClassD((UI8_T *)&temp[i]) || IP_LIB_IsIpInClassE((UI8_T *)&temp[i]) ||
                IP_LIB_IsLoopBackIp((UI8_T *)&temp[i]))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     str);

            return (DHCP_MGR_FAIL);
        }
    }

    /* 1. Delete Relay Server Address */
    DHCP_MGR_DeleteAllRelayServerAddress(vid_ifIndex);

    /* 2. Add Relay Server Address */
    /* 2.1. Check if we need to clear out the rest of relay server */
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
    {
        if (temp[i] == 0)
        {
            /* clear out the rest */
            for (; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER;i++)
                temp[i] = 0;

            break;
        }
    }

    if (!DHCP_WA_AddIfRelayServerAddress(vid_ifIndex, temp))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 str);
        return DHCP_MGR_FAIL;
    }

    return DHCP_MGR_OK;


} /* end of DHCP_MGR_SetIfRelayServerAddress */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_GetIfRelayServerAddress
 *------------------------------------------------------------------------------
 * PURPOSE : Get relay server address for specified vlan interface
 * INPUT   : vid_ifIndex -- the interface vlan to be searched for the interface vlan's
 *                      relay server address.
 *           ip_array    -- the array for Relay server IP Address.
 * OUTPUT  : None
 * RETUEN  : DHCP_MGR_OK/DHCP_MGR_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_MGR_GetIfRelayServerAddress(
    UI32_T vid_ifIndex,
    UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER])
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    if (DHCP_WA_GetIfRelayServerAddress(vid_ifIndex, ip_array))
    {
        if (ip_array[0] == 0)
        {
            return DHCP_MGR_FAIL;
        }
        else
        {
            return DHCP_MGR_OK;
        }
    }

    return DHCP_MGR_FAIL;

} /* end of DHCP_MGR_GetRunningIfRelayServerAddress */

/* FUNCTION NAME : DHCP_MGR_DeleteAllRelayServerAddress
 * PURPOSE:
 *      Delete all Server addresses for DHCP Relay Agent in WA
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully.
 *      DHCP_MGR_FAIL   --  FAIL to delete all relay server addresses.
 *
 * NOTES:1. For add relay server address, we can add all server IPs at
 *          one time. If we modify the server address twice, the last update
 *          would replace the previous setting.
 *
 *       2. For delete relay server IP, we only allow delete all at a time
 *
 */
UI32_T DHCP_MGR_DeleteAllRelayServerAddress(UI32_T vid_ifIndex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "vid_ifindex[%lu]", (unsigned long)vid_ifIndex);

    if (!DHCP_WA_DeleteAllIfRelayServerAddress(vid_ifIndex))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Delete Interface Relay Server");
        return DHCP_MGR_FAIL;
    }

    return DHCP_MGR_OK;

} /* end of DHCP_MGR_DeleteAllRelayServerAddress */

#endif
/* FUNCTION NAME : DHCP_MGR_SetRestartObject
 * PURPOSE:
 *      Setup the restart object in WA and OM. Also check interface
 *      condition before actually restart DHCP.
 *
 * INPUT:
 *      restart_object -- specify the object needed to restart to system
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK         -- Successfully set the system role
 *      DHCP_MGR_FAIL       -- Can't set the system role
 *      DHCP_MGR_NO_IP      -- Interface does not have ip bind to it
 *      DHCP_MGR_DYNAMIC_IP -- The interface is acting as client (bootp/dhcp)
 *      DHCP_MGR_SERVER_ON  -- The system is running as a dhcp server
 *      DHCP_MGR_RELAY_ON   -- The system is running as a dhcp relay
 *
 * NOTES:
 *      1. If restart_object is server, return Error Code if current
 *      system has relay running
 *      2. If restart_object is relay, return Error Code if current
 *      system has server running.
 *
 */
UI32_T DHCP_MGR_SetRestartObject(UI32_T restart_object)
{
    UI32_T rv, ret;
    UI32_T address_mode;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "restart_object[%lu]", (unsigned long)restart_object);

    /* 1. Check system status (go thru all RIFs to check system role)*/
    switch (restart_object)
    {

        case DHCP_MGR_RESTART_SERVER:
            memset(&rif_config, 0, sizeof(rif_config));
            while (NETCFG_POM_IP_GetNextRifConfig(&rif_config) == NETCFG_TYPE_OK)
            {
                ret = NETCFG_POM_IP_GetIpAddressMode(rif_config.ifindex, &address_mode);
                if (ret != NETCFG_TYPE_OK)
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                             EH_TYPE_MSG_FAILED_TO_SET,
                                             SYSLOG_LEVEL_ERR,
                                             "Fail to Set Restart Object");
                    return DHCP_MGR_FAIL;
                }

                if (address_mode != NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
                    continue;

                rv = DHCP_MGR_CheckInterfaceCondition(rif_config.ifindex, DHCP_MGR_BIND_SERVER);

                if (rv != DHCP_MGR_OK)
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                             EH_TYPE_MSG_FAILED_TO_SET,
                                             SYSLOG_LEVEL_ERR,
                                             "Fail to Set Restart Object");
                    return rv;
                }
            }

            break;
        default:

            break;
    }

    /* 3. Set WA restart object */
    if (!DHCP_WA_SetRestartObject(restart_object))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Set Restart Object");
        return DHCP_MGR_FAIL;
    }

    return DHCP_MGR_OK;

}/* end of DHCP_MGR_SetRestartObject */

/* FUNCTION NAME : DHCP_MGR_C_SetClientId
 * PURPOSE:
 *      Define associated client identifier per interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      id_mode     -- the client-port of dhcp.
 *      id_len      -- the len of buffer.
 *      id_buf      -- the content of the cid.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- No more space in allocating memory
 *      DHCP_MGR_CID_EXCEED_MAX_SIZE -- cid exceeds the max size
 *
 * NOTES:
 *      1. Possible values of id_mode are {DHCP_TYPE_CID_HEX | DHCP_TYPE_CID_TEXT}.
 *
 */
UI32_T DHCP_MGR_C_SetClientId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    /* check null pointer */
    if (NULL == id_buf)
        return DHCP_MGR_FAIL;

    DHCP_BD(CONFIG, "Set vid_ifindex[%lu],id_mode[%lu],id_len[%lu],id_buf[%s]",
            (unsigned long)vid_ifIndex, (unsigned long)id_mode, (unsigned long)id_len, id_buf);

    DHCP_MGR_C_DeleteClientId(vid_ifIndex);
    ret = DHCP_WA_C_SetIfClientId(vid_ifIndex, id_mode, id_len, id_buf);
    if (ret == DHCP_WA_FAIL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Set Client ID for DHCP Client");
        return DHCP_MGR_FAIL;
    }
    else if (ret == DHCP_WA_CID_EXCEED_MAX_SIZE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Set Client ID for DHCP Client");
        return DHCP_MGR_CID_EXCEED_MAX_SIZE;
    }

    return DHCP_MGR_OK;

} /* end of DHCP_MGR_C_SetClientId */

/* FUNCTION NAME : DHCP_MGR_C_GetClientId
 * PURPOSE:
 *      Retrieve the associated client identifier for specified interface from OM.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      cid_p       -- the pointer of cid structure
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK   -- successfully.
 *      DHCP_MGR_FAIL -- failure.
 *      DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *      DHCP_MGR_NO_CID -- this interface has no CID configuration.
 *
 * NOTES:
 *      None.
 *
 */
UI32_T DHCP_MGR_C_GetClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    /* check null pointer */
    if (NULL == cid_p)
        return DHCP_MGR_FAIL;

    DHCP_BD(CONFIG, "Get vid_ifindex[%lu],id_mode[%lu],id_len[%lu],id_buf[%s]",
            (unsigned long)vid_ifIndex, (unsigned long)cid_p->id_mode, (unsigned long)cid_p->id_len, cid_p->id_buf);

    if (!DHCP_WA_C_GetIfClientId(vid_ifIndex, cid_p))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_GET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Get Client ID for DHCP Client");
        return DHCP_MGR_NO_SUCH_INTERFACE;
    }

    return DHCP_MGR_OK;
} /* end of DHCP_MGR_C_GetClientId */

/* FUNCTION NAME : DHCP_MGR_C_GetRunningClientId
 * PURPOSE:
 *      Get running config for CID in WA.
 *
 * INPUT:
 *      vid_ifIndex -- the key to identify ClientId
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_C_GetRunningClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }
    else
    {
        if (!DHCP_WA_C_GetIfClientId(vid_ifIndex, cid_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Client ID for DHCP Client");
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
        else
        {
            if ((cid_p->id_mode != DHCP_WA_DEFAULT_CID_MODE) ||
                    (cid_p->id_len != 0) ||
                    (cid_p->id_buf != 0))
            {
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }

            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }

    } /* end of DHCP_MGR_C_GetRunningClientId */
}
    /* FUNCTION NAME : DHCP_MGR_C_GetNextClientId
     * PURPOSE:
     *      Get the next client identifier for specified interface from OM.
     *
     * INPUT:
     *      vid_ifIndex -- the interface to be defined.
     *
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK   -- successfully.
     *      DHCP_MGR_FAIL -- failure.
     *      DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
     *      DHCP_MGR_NO_CID -- this interface has no CID configuration.
     *
     * NOTES:
     *      1. (vid_ifIndex = 0) to get 1st interface, for layer 2, get the mgmt vlan id
     *
     */
    UI32_T DHCP_MGR_C_GetNextClientId(UI32_T *vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
    {
        DHCP_WA_InterfaceDhcpInfo_T if_p;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == vid_ifIndex) || (NULL == cid_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get vid_ifindex[%lu]", (unsigned long)vid_ifIndex);

        if (!DHCP_WA_GetNextIfConfig(*vid_ifIndex, &if_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Client ID for DHCP Client");
            return DHCP_MGR_NO_SUCH_INTERFACE;
        }

        *vid_ifIndex =  if_p.ifIndex;

        memcpy(cid_p, &if_p.cid, sizeof(DHCP_MGR_ClientId_T));

        return DHCP_MGR_OK;



    } /* end of DHCP_MGR_C_GetNextClientId */

    /* FUNCTION NAME : DHCP_MGR_C_DeleteClientId
     * PURPOSE:
     *      Delete the client identifier for specified interface. (Restart DHCP needed)
     *
     * INPUT:
     *      vid_ifIndex -- the interface to be defined.
     *
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK   -- successfully.
     *      DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete cid
     *
     *
     * NOTES:
     *      1. This function will set Client_id structure to empty in WA. Until restart DHCP
     *      client to update WA info into OM
     *
     */
    UI32_T DHCP_MGR_C_DeleteClientId(UI32_T vid_ifIndex)
    {
        DHCP_WA_InterfaceDhcpInfo_T *ptr;
        char *string = NULL;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu]", (unsigned long)vid_ifIndex);

        ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
        if (ptr == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete CID for DHCP Client");
            return DHCP_MGR_FAIL;
        }
        else
        {
            DHCP_WA_C_SetIfClientId(vid_ifIndex, 0, 0, string);
            return DHCP_MGR_OK;
        }

    } /* end of DHCP_MGR_C_DeleteClientId */

    /* FUNCTION	NAME : DHCP_MGR_C_SetVendorClassId
     * PURPOSE:
     *		Define associated class identifier per interface.
     *
     * INPUT:
     *		vid_ifIndex -- the interface to be defined.
     *		id_mode 	-- the class id mode.
     *		id_len 		-- the len of buffer.
     *		id_buf		-- the content of the class id.
     *
     * OUTPUT:
     *		None.
     *
     * RETURN:
     *		DHCP_MGR_OK		-- successfully.
     *		DHCP_MGR_FAIL	-- No more space in allocating memory
     *		DHCP_MGR_CLASSID_EXCEED_MAX_SIZE -- class id exceeds the max size
     *
     * NOTES:
     *		1. Possible values of id_mode are {DHCP_TYPE_CLASSID_HEX | DHCP_TYPE_CLASSID_TEXT}.
     *
     */
    UI32_T DHCP_MGR_C_SetVendorClassId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
    {
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == id_buf)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu],id_mode[%lu],id_len[%lu],id_buf[%s]",
                (unsigned long)vid_ifIndex, (unsigned long)id_mode, (unsigned long)id_len, id_buf);

        ret = DHCP_WA_C_SetIfVendorClassId(vid_ifIndex, id_mode, id_len, id_buf);
        if (ret == DHCP_WA_FAIL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Vendor Class ID for DHCP Client");
            return DHCP_MGR_FAIL;
        }
        else if (ret == DHCP_WA_CLASSID_EXCEED_MAX_SIZE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Vendor Class ID for DHCP Client");
            return DHCP_MGR_CLASSID_EXCEED_MAX_SIZE;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_C_SetVendorClassId */

    /* FUNCTION	NAME : DHCP_MGR_C_GetVendorClassId
     * PURPOSE:
     *		Retrieve the associated class identifier for specified interface from OM.
     *
     * INPUT:
     *		vid_ifIndex -- the interface to be defined.
     *
     * OUTPUT:
     *		class_id_p	-- the pointer of class id structure
     *
     * RETURN:
     *		DHCP_MGR_OK	  -- successfully.
     *		DHCP_MGR_FAIL -- failure.
     *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
     *		DHCP_MGR_NO_CLASSID	-- this interface has no CLASSID configuration.
     *
     * NOTES:
     *		None.
     *
     */
    UI32_T DHCP_MGR_C_GetVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
    {

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == class_id_p)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get vid_ifindex[%lu]", (unsigned long)vid_ifIndex);

        if (!DHCP_WA_C_GetIfVendorClassId(vid_ifIndex, class_id_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Class ID for DHCP Client");
            return DHCP_MGR_NO_SUCH_INTERFACE;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_C_GetVendorClassId */

    /* FUNCTION	NAME : DHCP_MGR_C_GetRunningVendorClassId
     * PURPOSE:
     *		Get running config for CLASS ID in WA (Working Area).
     *
     * INPUT:
     *		vid_ifIndex -- the key to identify Vendor Class Id
     *
     *
     * OUTPUT:
     *		class_id_p	-- the pointer of class id structure
     *
     * RETURN:
     *		SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_C_GetRunningVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
    {
        UI8_T  model_name[SYS_ADPT_MAX_MODEL_NAME_SIZE+1] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (!DHCP_WA_C_GetIfVendorClassId(vid_ifIndex, class_id_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Class ID for DHCP Client");
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
        else
        {
            if (class_id_p->vendor_len == 0)
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;  /*no ip dhcp client class-id */
            else
            {
                if (TRUE == SYS_PMGR_GetModelName(0, model_name))
                {
                    if (0 == memcmp(class_id_p->vendor_buf, model_name, class_id_p->vendor_len))
                        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
                    else
                        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                }
                else
                    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
            }

        }

    } /* end of DHCP_MGR_C_GetRunningVendorClassId */

    /* FUNCTION	NAME : DHCP_MGR_C_GetNextVendorClassId
     * PURPOSE:
     *		Get the next class id for specified interface.
     *
     * INPUT:
     *		vid_ifIndex -- the current interface.
     *
     *
     * OUTPUT:
     *		vid_ifIndex --  the next active vlan interface.
     *      class_id_p	-- the pointer of class id structure
     *
     * RETURN:
     *		DHCP_MGR_OK
     *		DHCP_MGR_FAIL
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_C_GetNextVendorClassId(UI32_T *vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
    {
        DHCP_WA_InterfaceDhcpInfo_T if_p;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == vid_ifIndex) || (NULL == class_id_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get vid_ifindex[%lu]", (unsigned long)*vid_ifIndex);

        if (!DHCP_WA_GetNextIfConfig(*vid_ifIndex, &if_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Class ID for DHCP Client");
            return DHCP_MGR_FAIL;
        }

        *vid_ifIndex =  if_p.ifIndex;

        memcpy(class_id_p, &if_p.classid, sizeof(DHCP_MGR_Vendor_T));

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_C_GetNextVendorClassId */

    /* FUNCTION	NAME : DHCP_MGR_C_DeleteVendorClassId
     * PURPOSE:
     *		Delete the client identifier for specified interface, so that the
     *      vendor class option60 shall not be added into packet.
     *
     * INPUT:
     *		vid_ifIndex -- the interface to be defined.
     *
     *
     * OUTPUT:
     *		None.
     *
     * RETURN:
     *		DHCP_MGR_OK	  -- successfully.
     *		DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete class id
     *
     *
     * NOTES:
     *		1. This function will set class_id structure to empty in WA (Working Area).
     *
     */
    UI32_T DHCP_MGR_C_DeleteVendorClassId(UI32_T vid_ifIndex)
    {
        UI8_T  str[DHCP_MGR_CLASSID_BUF_MAX_SIZE+1] = {0};
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu]", (unsigned long)vid_ifIndex);

        ret = DHCP_WA_C_SetIfVendorClassId(vid_ifIndex, 0, 0, (char *)str);
        if (ret == DHCP_WA_FAIL)
        {

            return DHCP_MGR_FAIL;
        }
        else if (ret == DHCP_WA_CLASSID_EXCEED_MAX_SIZE)
        {
            return DHCP_MGR_CLASSID_EXCEED_MAX_SIZE;
        }
        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_C_DeleteVendorClassId */

    /* ------------------------------------------------------------------------
     * FUNCTION NAME - DHCP_MGR_UpdateUCData
     * ------------------------------------------------------------------------
     * PURPOSE  : This function notify DHCP to update UC data when UC size is changed
     * INPUT    : None
     * OUTPUT   : None
     * RETURN   : None
     * NOTE     : None
     * ------------------------------------------------------------------------
     */
    void DHCP_MGR_UpdateUCData(void)
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
            return;

        DHCP_OM_UpdateUCData();
        return;
    }

    /* FUNCTION NAME : DHCP_MGR_CheckInterfaceCondition
     * PURPOSE:
     *      Check interface condition before calling relay service
     *
     * INPUT:
     *      vid_ifIndex -- the interface to be checked.
     *      role -- the dhcp role of that vlan
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_NO_IP -- interface does not have ip bind to it
     *      DHCP_MGR_DYNAMIC_IP -- the interface is acting as client (bootp/dhcp)
     *      DHCP_MGR_SERVER_ON -- the system is running as a dhcp server
     *      DHCP_MGR_RELAY_ON -- the system is running as a dhcp relay
     *      DHCP_MGR_SHUT_DOWN_RELAY -- this interface should change binding role
     *                  from relay to client due to all relay server IPs are '0';
     *      DHCP_MGR_FAIL   --  fail to check the condition.
     *
     * NOTES: 1. In order to enable dhcp relay service, the following checking
     *          result should be a "YES" answer; if one of these is "NO", we can not
     *          activate relay service.
     *  a. having IP
     *  b. interface address method is user-defined
     *  c. current system dhcp server is off
     *
     *
     */
    static UI32_T DHCP_MGR_CheckInterfaceCondition(UI32_T vid_ifIndex, UI32_T role)
    {
        UI32_T ret;
#if (SYS_CPNT_ROUTING == TRUE)
        NETCFG_TYPE_InetRifConfig_T rif_config;
#endif
        UI32_T address_mode;

        DHCP_BD(CONFIG, "vid_ifindex[%lu],role[%lu]", (unsigned long)vid_ifIndex, (unsigned long)role);

        /* 1. Check if the system role, if not, continue the setting  */

        if (role == DHCP_MGR_BIND_RELAY)
        {
            /* 1. Check if all relay server IPs are '0', if so, need to notify back
            **  should not activate relay on
            */

            /* 2. Check if server is on ?? */
            if (DHCP_OM_IsServerOn())
                return DHCP_MGR_SERVER_ON;
        }
        else if (role == DHCP_MGR_BIND_SERVER)
        {
            if (DHCP_OM_IsRelayOn())
                return DHCP_MGR_RELAY_ON;
        }

        /* 2. Check ip validation */
#if (SYS_CPNT_ROUTING == TRUE)
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = vid_ifIndex;
        ret = NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config);
        if (ret != NETCFG_TYPE_OK)
            return DHCP_MGR_NO_IP;

        {
            UI8_T zero_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
            if (memcmp(rif_config.addr.addr , zero_ip, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                return DHCP_MGR_NO_IP;
        }
#endif
        /* 3. Check address mode = user-defined */
        ret = NETCFG_POM_IP_GetIpAddressMode(vid_ifIndex, &address_mode);
        if (ret != NETCFG_TYPE_OK)
            return DHCP_MGR_FAIL;
        if (address_mode != NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
            return DHCP_MGR_DYNAMIC_IP;

        return DHCP_MGR_OK;

    }

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    /* FUNCTION NAME : DHCP_MGR_AppendRelayServerAddress
     * PURPOSE:
     *      Append relay server addresses to current addresses.
     *
     * INPUT:
     *      number_of_relay_server          -- number of address
     *      address                         -- relay server addresses.
     *      current_number_of_relay_server  -- current number of address
     *      current_address                 -- pointer to current relay server addresses.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_FAIL               -- fail to append.
     *      DHCP_MGR_EXCEED_CAPACITY    -- exceed the maximum number of relay server address.
     *      DHCP_MGR OK                 -- append successfully.
     *
     * NOTES:
     *      None.
     */
    static UI32_T DHCP_MGR_AppendRelayServerAddress(UI32_T number_of_address,
            UI32_T address[],
            UI32_T current_number_of_address,
            UI32_T *current_address_p)
    {
        int number_index, current_number_index;
        BOOL_T is_duplicated;

        for (number_index = 0; number_index < number_of_address; number_index++)
        {
            if (address[number_index] == 0)
                continue;
            /* Confirm that the addresses follow the rule: No LoopBack, ClassD or ClassE IP */
            if (IP_LIB_IsIpInClassD((UI8_T *)&address[number_index]) ||
                    IP_LIB_IsIpInClassE((UI8_T *)&address[number_index]) ||
                    IP_LIB_IsLoopBackIp((UI8_T *)&address[number_index]))
            {
                return DHCP_MGR_FAIL;
            }

            /* append only if it is not duplicated from the previous list */
            is_duplicated = FALSE;
            for (current_number_index = 0; current_number_index < current_number_of_address; current_number_index++)
            {
                if (current_address_p[current_number_index] == 0)
                    break;
                if (address[number_index] == current_address_p[current_number_index])
                {
                    is_duplicated = TRUE;
                    break; /* we found it is dulplicated, no need to compare with others */
                }
            }
            if (is_duplicated == FALSE)
            {
                if (current_number_of_address < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER)
                {
                    current_address_p[current_number_of_address] = address[number_index];
                    current_number_of_address++;
                }
                else
                {
                    return DHCP_MGR_EXCEED_CAPACITY;
                }
            }
        } /* end of number_index loop */
        return DHCP_MGR_OK;

    } /* End of DHCP_MGR_AppendRelayServerAddress */

    /* FUNCTION NAME : DHCP_MGR_DeleteRelayServerAddress
     * PURPOSE:
     *      Delete relay server addresses from current addresses.
     *
     * INPUT:
     *      number_of_relay_server          -- number of address
     *      address                         -- relay server addresses.
     *      current_number_of_relay_server  -- current number of address
     *      current_address                 -- pointer to current relay server addresses.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_FAIL               -- fail to remove.
     *      DHCP_MGR OK                 -- remove successfully.
     *
     * NOTES:
     *      We set the address to be 0 as removing it.
     */
    static UI32_T DHCP_MGR_DeleteRelayServerAddress(UI32_T number_of_address,
            UI32_T address[],
            UI32_T current_number_of_address,
            UI32_T *current_address_p)
    {
        int number_index, current_number_index;

        for (number_index = 0; number_index < number_of_address; number_index++)
        {
            if (address[number_index] == 0)
                continue;
            /* Confirm that the addresses follow the rule: No LoopBack, ClassD or ClassE IP */
            if (IP_LIB_IsIpInClassD((UI8_T *)&address[number_index]) ||
                    IP_LIB_IsIpInClassE((UI8_T *)&address[number_index]) ||
                    IP_LIB_IsLoopBackIp((UI8_T *)&address[number_index]))
            {
                return DHCP_MGR_FAIL;
            }

            /* remove if it is in the current_address */
            for (current_number_index = 0; current_number_index < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; current_number_index++)
            {
                if (current_address_p[current_number_index] == address[number_index])
                {
                    current_address_p[current_number_index] = 0;
                }
            }
        }
        return DHCP_MGR_OK;

    } /* End of DHCP_MGR_DeleteRelayServerAddress */

    /* FUNCTION NAME : DHCP_MGR_ShiftRelayServerAddress
     * PURPOSE:
     *      Shift Relay Server Addresses for all non-zero values
     *
     * INPUT:
     *      *relay_server_address   -- pointer to relay server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      None.
     *
     * NOTES:
     *      It compares the non-zero address to the previous one.
     */
    static void DHCP_MGR_ShiftRelayServerAddress(UI32_T *relay_server_address_p)
    {
        int i, j;
        UI32_T temp_address;

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER - 1; i++)
        {
            for (j = 0; j < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER - 1 - i; j++)
            {
                if (relay_server_address_p[i+1] != 0 && relay_server_address_p[j] == 0)
                {
                    temp_address = relay_server_address_p[j];
                    relay_server_address_p[j] = relay_server_address_p[i+1];
                    relay_server_address_p[i+1] = temp_address;
                }
            } /* End of j loop */
        } /* End of i loop */

    } /* End of DHCP_MGR_ShiftRelayServerAddress */

    /* FUNCTION NAME : DHCP_MGR_SortRelayServerAddress
     * PURPOSE:
     *      Sort Relay Server Addresses
     *
     * INPUT:
     *      *relay_server_address   -- pointer to relay server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      None.
     *
     * NOTES:
     *      It compares the non-zero address to the previous one.
     */
    static void DHCP_MGR_SortRelayServerAddress(UI32_T *relay_server_address_p)
    {
        int i, j;
        UI32_T temp_address;

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER - 1; i++)
        {
            for (j = 0; j < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER - 1 - i; j++)
            {
                if (relay_server_address_p[j+1] != 0 &&
                        relay_server_address_p[j+1] < relay_server_address_p[j])
                {
                    temp_address = relay_server_address_p[j];
                    relay_server_address_p[j] = relay_server_address_p[j+1];
                    relay_server_address_p[j+1] = temp_address;
                }
            } /* End of j loop */
        } /* End of i loop */

    } /* End of DHCP_MGR_SortRelayServerAddress */

    /* ==========================
    ** The APIs for SNMP Use
    ** ==========================
    */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpRelayServerAddrServerIp
     * PURPOSE:
     *      Set relay server address item by array index.
     *
     * INPUT:
     *      vid_ifIndex     -- the interface to be configured.
     *      index           -- the index of the relay server array
     *      ip_address      -- the relay server address of the index to be set.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to set the relay server address.
     *
     * NOTES:
     *      1. The 1st relay ip's index is 1.
     *      2. If the ip_address = 0, it will clear out the rest of the relay server address
     *          the case of current index's address is 0 and the rest of the relay server IPs
     *          are not 0, we need to clear out the rest of the relay server address.
     *
     */
    UI32_T DHCP_MGR_SetDhcpRelayServerAddrServerIp(UI32_T vid_ifIndex, int index, UI32_T ip_address)
    {
        UI32_T rv;
        UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu],index[%d],ip_addr[%u.%u.%u.%u]",
                (unsigned long)vid_ifIndex, index, L_INET_EXPAND_IP(ip_address));

        /*check  second index*/

        if (index <= MIN_dhcpRelayServerAddrIndex || index > MAX_dhcpRelayServerAddrIndex)
        {
            return DHCP_MGR_FAIL;
        }

        rv = DHCP_MGR_CheckInterfaceCondition(vid_ifIndex, DHCP_MGR_BIND_RELAY);

        if (rv != DHCP_MGR_OK)
        {
            return rv;
        }

        if (!DHCP_WA_GetIfRelayServerAddress(vid_ifIndex, ip_array))
        {
            return DHCP_MGR_FAIL;
        }

        ip_array[index-1] = ip_address;

        DHCP_MGR_ShiftRelayServerAddress(ip_array);
        DHCP_MGR_SortRelayServerAddress(ip_array);

        if (ip_array[0] == 0)
        {
            if (!DHCP_WA_DeleteAllIfRelayServerAddress(vid_ifIndex))
                return DHCP_MGR_FAIL;
        }
        else if (!DHCP_WA_AddIfRelayServerAddress(vid_ifIndex, ip_array))
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDhcpRelayServerAddrServerIp */

    /* FUNCTION NAME : DHCP_MGR_AddInterfaceRelayServerAddress
     * PURPOSE:
     *      Add dhcp relay server ip addresses to the interface.
     *
     * INPUT:
     *      vid_ifIndex             -- the interface id
     *      number_of_relay_server  -- the number of dhcp relay server addresses
     *      relay_server_address[]  -- the list of dhcp relay server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
     *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
     *      DHCP_MGR_OK                 -- successfully
     *      DHCP_MGR_EXCEED_CAPACITY    -- exceed the maximum limit of dhcp relay server address
     *
     *
     * NOTES:
     *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
     *     - vid_ifIndex was invalid
     *
     *  2. The function returns DHCP_MGR_FAIL if
     *     - the interface is in slave mode
     *     - it fails to add addresses
     *
     *  3. The function returns DHCP_MGR_EXCEED_CAPACITY if
     *     - the total amount is bigger than SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER
     *     example1: (A,B,C) + (C,D,E) = (A,B,C,D,E) does not exceed the capacity.
     *     example2: (A,B) + (C,D,E,F) = (A,B,C,D,E,F) exceeds the capacity.
     *
     *  4. The function appends new relay server ip addresses right after the current
     *     relay server addresses.
     *
     */
    UI32_T DHCP_MGR_AddInterfaceRelayServerAddress(UI32_T vid_ifIndex,
            UI32_T number_of_relay_server,
            UI32_T relay_server_address[])
    {
        int current_number_index;
        UI32_T current_number_of_relay_server;
        UI32_T current_relay_server_address[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};
        UI32_T return_value;
        UI32_T rv;


        /* Confirm that it is not in slave mode */

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu],server num[%lu]\r\n"
                "ip1[%u.%u.%u.%u]\r\n"
                "ip2[%u.%u.%u.%u]\r\n"
                "ip3[%u.%u.%u.%u]\r\n"
                "ip4[%u.%u.%u.%u]\r\n"
                "ip5[%u.%u.%u.%u]",
                (unsigned long)vid_ifIndex, (unsigned long)number_of_relay_server,
                L_INET_EXPAND_IP(relay_server_address[0]),
                L_INET_EXPAND_IP(relay_server_address[1]),
                L_INET_EXPAND_IP(relay_server_address[2]),
                L_INET_EXPAND_IP(relay_server_address[3]),
                L_INET_EXPAND_IP(relay_server_address[4]));

        rv = DHCP_MGR_CheckInterfaceCondition(vid_ifIndex, DHCP_MGR_BIND_RELAY);

        if (rv != DHCP_MGR_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Interface Relay Server Address");
            return rv;
        }

        /* Confirm that number_of_relay_server does not exceed maximum capacity */
        if (number_of_relay_server > SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER)
        {
            return DHCP_MGR_EXCEED_CAPACITY;
        }

        /* get the current addresses from WA
         * 1. If the interface does not exist, it will create an interface with an
         *    empty list of relay server addresses.
         * 2. The function returns fail if the interface does not exist and it is
         *    not able to create the interface.
         */
        if (!DHCP_WA_GetIfRelayServerAddress(vid_ifIndex, current_relay_server_address))
        {
            return DHCP_MGR_FAIL;
        }

        /* get current number of non-zero address */
        current_number_of_relay_server = 0;

        for (current_number_index = 0; current_number_index < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; current_number_index++)
        {
            if (current_relay_server_address[current_number_index] != 0)
                current_number_of_relay_server++;
            else
                break; /* once zero appears we reach the end of the non-zero list */
        }

        /* append */
        return_value = DHCP_MGR_AppendRelayServerAddress(number_of_relay_server,
                       relay_server_address,
                       current_number_of_relay_server,
                       current_relay_server_address);

        if (return_value != DHCP_MGR_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Append Relay Server Address");
            return return_value;
        }

        /* sort */
        DHCP_MGR_SortRelayServerAddress(current_relay_server_address);

        /* set the current addresses to WA */
        if (!DHCP_WA_AddIfRelayServerAddress(vid_ifIndex, current_relay_server_address))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Interface Relay Server Address to WA");
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;

    } /* End of DHCP_MGR_AddInterfaceRelayServerAddress */


    /* FUNCTION NAME : DHCP_MGR_GetDhcpRelayServerAddrTable
     * PURPOSE:
     *      Get dhcp relay server ip address for relay agent to WA.
     *      (Get whole record function for SNMP)
     *
     * INPUT:
     *      vid_ifIndex     -- the interface to be configured.
     *      index           -- the index number to specify which
     *                          relay server address provided to SNMP
     *      ip_address_p    -- the relay server retrieved by above keys.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to get the relay server address.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetDhcpRelayServerAddrTable(UI32_T vid_ifIndex, UI32_T index, UI32_T *ip_address_p)
    {

        UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == ip_address_p)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get vid_ifindex[%lu],index[%lu]", (unsigned long)vid_ifIndex, (unsigned long)index);

        if (0 < index && index <= SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER)
        {
            if (!DHCP_WA_GetIfRelayServerAddress(vid_ifIndex, ip_array))
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                         EH_TYPE_MSG_FAILED_TO_GET,
                                         SYSLOG_LEVEL_ERR,
                                         "Fail to Get Relay Server");
                return DHCP_MGR_FAIL;
            }
            else
            {
                memcpy(ip_address_p, &ip_array[index-1], 4);
            }

            return DHCP_MGR_OK;
        }
        else
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Relay Server");
            return DHCP_MGR_FAIL;
        }

    } /* end of DHCP_MGR_GetDhcpRelayServerAddrTable */

    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpRelayServerAddrTable
     * PURPOSE:
     *      Get next dhcp relay server ip address for relay agent to WA.
     *
     * INPUT:
     *      vid_ifIndex_p   -- the pointer to the interface to be configured.
     *      index_p         -- the index number to specify which
     *                          relay server address provided to SNMP
     *      ip_address_p    -- the relay server next to above keys.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to get the relay server address.
     *
     * NOTES:
     *  1.              ---------> Get next
     * vid_ifIndex |        relay server address index
     *              [1]     |    [2]     |    [3]     |    [4]     |    [5]     |
     *          ------------+------------+------------+------------+------------+
     * 1001      10.1.1.1   | 10.1.1.2   | 10.1.1.3   | 10.1.1.4   | 10.1.1.5   |
     *          ------------+------------+------------+------------+------------+
     * 1002      10.1.2.1   | 10.1.2.2   | 10.1.2.3   | 10.1.2.4   | 10.1.2.5   |
     *          ------------+------------+------------+------------+------------+
     * 1003      10.1.3.1   | 10.1.3.2   | 10.1.3.3   | 10.1.3.4   | 10.1.3.5   |
     *          ------------+------------+------------+------------+------------+
     *  2. GetNext for (0,0, &ip_address) will get the 1st address of the 1st interface which in above
     *      example is 10.1.1.1;
     *  3. GetNext for (1001,5, &ip_address), the result will be ip_address = 10.1.2.1
     *
     *
     */
    UI32_T DHCP_MGR_GetNextDhcpRelayServerAddrTable(UI32_T *vid_ifIndex_p, UI32_T *index_p, UI32_T *ip_address_p)
    {

        DHCP_WA_InterfaceDhcpInfo_T if_config;
        UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (vid_ifIndex_p == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Relay Server");
            return DHCP_MGR_FAIL;
        }

        if (*index_p == SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER || *vid_ifIndex_p == 0)
        {
            if (!DHCP_WA_GetNextIfConfig(*vid_ifIndex_p, &if_config))
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                         EH_TYPE_MSG_FAILED_TO_GET,
                                         SYSLOG_LEVEL_ERR,
                                         "Fail to Get Next Relay Server");
                return DHCP_MGR_FAIL;
            }
            else
            {
                /* Get the 1st address of that vid_ifIndex */
                memcpy(vid_ifIndex_p, &if_config.ifIndex, 4);
                *index_p = 1;
                memcpy(ip_address_p, &if_config.relay_server[0], 4);
            }

            return DHCP_MGR_OK;
        }
        else
        {
            if (0 <= *index_p && *index_p < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER)
            {
                if (!DHCP_WA_GetIfRelayServerAddress(*vid_ifIndex_p, ip_array))
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                             EH_TYPE_MSG_FAILED_TO_GET,
                                             SYSLOG_LEVEL_ERR,
                                             "Fail to Get Next Relay Server");
                    return DHCP_MGR_FAIL;
                }
                else
                {
                    memcpy(ip_address_p, &ip_array[*index_p], 4);
                    *index_p = *index_p + 1;
                }

                return DHCP_MGR_OK;
            }

            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,

                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Relay Server");
            return DHCP_MGR_FAIL;

        }

    } /* end of DHCP_MGR_GetNextDhcpRelayServerAddrTable */

    /* FUNCTION NAME : DHCP_MGR_DeleteInterfaceRelayServerAddress
     * PURPOSE:
     *      Delete dhcp relay server ip addresses from the interface
     *
     * INPUT:
     *      vid_ifIndex             -- the interface id
     *      number_of_relay_server  -- the number of dhcp relay server addresses
     *      relay_server_address[]  -- the list of dhcp relay server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
     *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
     *      DHCP_MGR_OK                 -- successfully
     *
     * NOTES:
     *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
     *     - vid_ifIndex was invalid
     *
     *  2. The function returns DHCP_MGR_FAIL if
     *     - the interface is in slave mode.
     *     - it fails to set addresses.
     *
     *  3. The function deletes every dhcp relay server address before it
     *     returns DHCP_MGR_OK.
     *
     *  4. The function does not do any thing with an address which actually does not exist.
     *     If all the addresses do not exist, the function returns DHCP_MGR_OK.
     *
     */
    UI32_T DHCP_MGR_DeleteInterfaceRelayServerAddress(UI32_T vid_ifIndex,
            UI32_T number_of_relay_server,
            UI32_T relay_server_address[])
    {

        int current_number_index;
        UI32_T current_number_of_relay_server;
        UI32_T current_relay_server_address[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};
        UI32_T return_value;

        /* Confirm that it is not in slave mode */

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu],server num[%lu]\r\n"
                "ip1[%u.%u.%u.%u]\r\n"
                "ip2[%u.%u.%u.%u]\r\n"
                "ip3[%u.%u.%u.%u]\r\n"
                "ip4[%u.%u.%u.%u]\r\n"
                "ip5[%u.%u.%u.%u]",
                (unsigned long)vid_ifIndex, (unsigned long)number_of_relay_server,
                L_INET_EXPAND_IP(relay_server_address[0]),
                L_INET_EXPAND_IP(relay_server_address[1]),
                L_INET_EXPAND_IP(relay_server_address[2]),
                L_INET_EXPAND_IP(relay_server_address[3]),
                L_INET_EXPAND_IP(relay_server_address[4]));

        /*if wa not vlan interface. retun OK*/

        if (DHCP_WA_SearchIfInList(vid_ifIndex) == NULL)
        {
            return DHCP_MGR_OK;
        }

        /* get the current addresses from WA */
        if (!DHCP_WA_GetIfRelayServerAddress(vid_ifIndex, current_relay_server_address))
        {
            return DHCP_MGR_FAIL;
        }

        /* get current number of non-zero address */
        current_number_of_relay_server = 0;

        for (current_number_index = 0; current_number_index < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; current_number_index++)
        {
            if (current_relay_server_address[current_number_index] != 0)
                current_number_of_relay_server++;
            else
                break;
        }

        /* delete */
        return_value = DHCP_MGR_DeleteRelayServerAddress(number_of_relay_server,
                       relay_server_address,
                       current_number_of_relay_server,
                       current_relay_server_address);

        if (return_value != DHCP_MGR_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Relay Server Address");
            return return_value;
        }

        /* shift */
        DHCP_MGR_ShiftRelayServerAddress(current_relay_server_address);

        /* set the current addresses to WA */
        if (current_relay_server_address[0] == 0)
        {
            if (!DHCP_WA_DeleteAllIfRelayServerAddress(vid_ifIndex))
                return DHCP_MGR_FAIL;

        }
        else if (!DHCP_WA_AddIfRelayServerAddress(vid_ifIndex, current_relay_server_address))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Interface Relay Server Addresses to WA");
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;

    } /* End of DHCP_MGR_DeleteInterfaceRelayServerAddress */

#endif /* (SYS_CPNT_DHCP_RELAY == TRUE) */

    /* ==========================
    ** The APIs for DHCP Server
    ** ==========================
    */
#if (SYS_CPNT_DHCP_SERVER == TRUE)
/* FUNCTION NAME : DHCP_MGR_EnterPool
 * PURPOSE: This function checks the specified pool's existence. If not existed,
 *          created the pool with the specified pool_name; if existed, do nothing
 *          just return.
 *
 * INPUT:
 *      pool_name -- specify the pool name to be entered.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_EXCEED_MAX_POOL_NAME -- exceed max size of pool name
 *      DHCP_MGR_FAIL -- Fail in allocating memory for pool creation
 *      DHCP_MGR_OK
 *
 * NOTES:
 *      1. Check pool name exceeds the max pool name length.
 *      2. Check pool existence. If not, create one with pool name.
 *
 */
UI32_T DHCP_MGR_EnterPool(char *pool_name)
{
    UI32_T ret=0;
    UI32_T pool_length;
    DHCP_TYPE_PoolConfigEntry_T *pool;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    /* check null pointer */
    if (NULL == pool_name)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

    pool_length = strlen(pool_name);
    if (pool_length > DHCP_MGR_MAX_POOL_NAME_LEN)
    {
        ret = DHCP_MGR_EXCEED_MAX_POOL_NAME;
        goto exit;
    }

    pool = DHCP_WA_FindPoolByPoolName(pool_name);
    if (pool == NULL)
    {
        if(FALSE == DHCP_WA_CreatePool(pool_name))
        {
            ret = DHCP_MGR_FAIL;
            goto exit;
        }

    }

    ret = DHCP_MGR_OK;

exit:
    if(ret!=DHCP_MGR_OK)
    {
        char str[100]={0};
        snprintf(str, sizeof(str),"Fail to Enter DHCP pool: %s",pool_name);
        EH_MGR_Handle_Exception1(
            SYS_MODULE_DHCP,
            DHCP_TYPE_EH_Unknown,
            EH_TYPE_MSG_FAILED_TO_SET,
            SYSLOG_LEVEL_ERR,
            str);
    }

    return ret;

} /*  end of DHCP_MGR_EnterPool */
/* FUNCTION NAME : DHCP_MGR_DeletePool
 * PURPOSE:
 *      To delete pool config entry
 *
 * INPUT:
 *      pool_name -- the name of the pool which will be deleted.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to delete pool.
 *
 * NOTES:
 *      1. key is pool_name -- must be specified in order to retrieve
 *
 */
UI32_T DHCP_MGR_DeletePool(char *pool_name)
{
    UI32_T ret=0;
    DHCP_TYPE_PoolConfigEntry_T *pool;

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DHCP_MGR_FAIL;
    }

    /* check null pointer */
    if(NULL == pool_name)
    {
        return DHCP_MGR_FAIL;
    }

    DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

    pool = DHCP_WA_FindPoolByPoolName(pool_name);
    if (pool != NULL)
    {
        if(FALSE == DHCP_WA_DeletePool(pool))
        {
           ret = DHCP_MGR_FAIL;
           goto exit;
        }
    }

    ret = DHCP_MGR_OK;

exit:
    if(ret!=DHCP_MGR_OK)
    {
        char str[100]={0};
        snprintf(str, sizeof(str),"Fail to Delete DHCP pool: %s",pool_name);
        EH_MGR_Handle_Exception1(
            SYS_MODULE_DHCP,
            DHCP_TYPE_EH_Unknown,
            EH_TYPE_MSG_FAILED_TO_DELETE,
            SYSLOG_LEVEL_ERR,
            str);
    }

    return ret;
} /* end of DHCP_MGR_DeletePool */


    /* FUNCTION NAME : DHCP_MGR_SetExcludedIp
     * PURPOSE:
     *      To specify the excluded ip address (range).
     *
     * INPUT:
     *      low_address  -- the lowest ip in the excluded ip address range
     *      high_address -- the highest ip in the excluded ip address range
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to set excluded ip address.
     *      DHCP_MGR_IS_NETWORK_IP_OR_NETWORK_BROADCAST -- fail to set excluded ip as network address
     *                                                      or a network broadcast
     *      DHCP_MGR_EXCLUDE_ALL_IP_IN_SUBNET -- fail to set excluded ip range as (x.x.x.1 ~ x.x.x.254)
     *      DHCP_MGR_HIGH_IP_SMALLER_THAN_LOW_IP    -- fail to set excluded ip range that high ip smaller
     *                                              than low ip
     *      DHCP_MGR_EXCLUDED_IP_IN_OTHER_SUBNET    -- fail to set excluded ip in other subnet
     *
     * NOTES:
     *      1. To specify exluded ip address range, the high-address is required.
     *      Otherwise, only specify low-address is treated as excluding an address
     *      from available address range.
     *      2. (low_address != 0, high_address == 0) will treat low_address the only
     *      address to be excluded.
     */
    UI32_T DHCP_MGR_SetExcludedIp(UI32_T low_address, UI32_T high_address)
    {
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set low_addr[%u.%u.%u.%u],high_addr[%u.%u.%u.%u]",

                L_INET_EXPAND_IP(low_address), L_INET_EXPAND_IP(high_address));

        ret = DHCP_WA_SetExcludedIp(low_address, high_address);
        if (ret == DHCP_WA_FAIL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Excluded IP");
            return DHCP_MGR_FAIL;
        }
        else if (ret == DHCP_WA_IS_NETWORK_IP_OR_NETWORK_BROADCAST)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Excluded IP");
            return DHCP_MGR_IS_NETWORK_IP_OR_NETWORK_BROADCAST;
        }
        else if (ret == DHCP_WA_EXCLUDE_ALL_IP_IN_SUBNET)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Excluded IP");
            return DHCP_MGR_EXCLUDE_ALL_IP_IN_SUBNET;
        }
        else if (ret == DHCP_WA_HIGH_IP_SMALLER_THAN_LOW_IP)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Excluded IP");
            return DHCP_MGR_HIGH_IP_SMALLER_THAN_LOW_IP;
        }
        else if (ret == DHCP_WA_EXCLUDED_IP_IN_OTHER_SUBNET)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Excluded IP");
            return DHCP_MGR_EXCLUDED_IP_IN_OTHER_SUBNET;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetExcludedIp */

    /* FUNCTION NAME : DHCP_MGR_DelExcludedIp
     * PURPOSE:
     *      This function deletes the IP address(es) that the DHCP server
     *      should not assign to DHCP clients.
     *
     * INPUT:
     *      low_address  -- the lowest ip address of the excluded ip range
     *      high_address -- the highest ip address of the excluded ip range
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully delete the specified excluded ip.
     *      DHCP_MGR_FAIL -- FAIL to delete the specified excluded ip.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelExcludedIp(UI32_T low_address, UI32_T high_address)
    {
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set low_addr[%u.%u.%u.%u],high_addr[%u.%u.%u.%u]",

                L_INET_EXPAND_IP(low_address), L_INET_EXPAND_IP(high_address));

        ret = DHCP_WA_DelExcludedIp(low_address, high_address);

        if (ret == DHCP_WA_FAIL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Excluded IP");
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;
    } /*  end of DHCP_MGR_DelExcludedIp */


    /* FUNCTION NAME : DHCP_MGR_GetNextExcludedIp
     * PURPOSE:
     *      This function gets the next excluded IP address(es).
     *
     * INPUT:
     *      low_address  -- the lowest ip address of the excluded ip range
     *      high_address -- the highest ip address of the excluded ip range
     *
     * OUTPUT:
     *      low_address  -- the lowest ip address of the excluded ip range
     *      high_address -- the highest ip address of the excluded ip range
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully get the specified excluded ip.
     *      DHCP_MGR_FAIL -- FAIL to get the specified excluded ip.
     *
     * NOTES:
     *      1. low_address = 0 to get the 1st excluded IP entry.
     *      2. if return high_address = 0 which means there is no high address; this
     *          exclusion is only exclude an IP (low_address).
     */
    UI32_T DHCP_MGR_GetNextExcludedIp(UI32_T *low_address, UI32_T *high_address)
    {
        BOOL_T ret;
        DHCP_WA_IpExcluded_T  ip_address;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == low_address) || (NULL == high_address))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get low_addr[%u.%u.%u.%u],high_addr[%u.%u.%u.%u]",
                L_INET_EXPAND_IP(*low_address), L_INET_EXPAND_IP(*high_address));

        ip_address.low_excluded_address = *low_address;
        ip_address.high_excluded_address = *high_address;

        ret = DHCP_WA_GetNextIpExcluded(&ip_address);
        if (ret == FALSE)
        {
            *low_address = 0;
            *high_address = 0;

            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Excluded IP");
            return DHCP_MGR_FAIL;
        }
        else
        {
            memcpy(low_address, &ip_address.low_excluded_address, 4);
            memcpy(high_address, &ip_address.high_excluded_address, 4);
            return DHCP_MGR_OK;
        }

    } /* end of DHCP_MGR_GetNextExcludedIp */


    /* FUNCTION NAME : DHCP_MGR_SetNetworkToPoolConfigEntry
     * PURPOSE:
     *      Set network address to pool config entry (set by field) and link it to
     *      network sorted link list.
     *
     * INPUT:
     *      pool_name           -- pool name
     *      network_address     -- network ip address
     *      sub_netmask         -- subnet mask
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                         -- successfully.
     *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- pool has configured to host pool.
     *      DHCP_MGR_FAIL                       -- fail to set network to pool config
     *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
     *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
     *
     * NOTES:
     *      1. Network address can't be specified to '0x0'
     *      2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
     *          is 0x0, we use default subnet mask = 255.255.255.0
     *      3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
     *           pool->sub_netmask as subnet mask
     *      4. Add constraint checking for network pool:
     *          a). total numbers of network pool (SYS_ADPT_MAX_NBR_OF_DHCP_NETWORK_POOL)
     *          b). subnet mask checking for class C issue
     */
    UI32_T DHCP_MGR_SetNetworkToPoolConfigEntry(char *pool_name, UI32_T network_address, UI32_T sub_netmask)
    {
        UI32_T ret;
        DHCP_TYPE_PoolConfigEntry_T *pool;
        UI32_T  b_addr;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],network_addr[%u.%u.%u.%u],mask[%u.%u.%u.%u]",
                pool_name, L_INET_EXPAND_IP(network_address), L_INET_EXPAND_IP(sub_netmask));

        /*  network ID should not be NULL*/
        if (network_address == 0)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Network IP to pool");
            return DHCP_MGR_INVALID_IP;
        }

        /* can not allow a duplicated network ID setting */
        if (sub_netmask == 0)
        {
            /* get natural subnet mask */
            sub_netmask = DHCP_ALGO_GetNaturalSubnetMask(network_address);
            DHCP_BD(CONFIG, "Natural subnet mask[%u.%u.%u.%u]", L_INET_EXPAND_IP(sub_netmask));
        }

        if (DHCP_WA_GetNetworkPoolbyIpMask(network_address, sub_netmask) != NULL)
        {
            DHCP_BD(CONFIG, "Network already exist");
            return (DHCP_MGR_NETWORK_EXIST);
        }

        /* 2003.11.27, Jamescyl: Resolve EPR ES3626G-J6-00021 */
        /* Note: ip must fulfil the following conditions:
         * 1) must be either class A, class B, or class C
         * 2) can't be network b'cast ip or network id
         * 3) can't be loop back ip
         * 4) can't be b'cast ip
         * 5) can't be m'cast ip
         */
        /* class A, class B or class C */
        if ((IP_LIB_IsIpInClassA((UI8_T *)&network_address) == FALSE) && (IP_LIB_IsIpInClassB((UI8_T *)&network_address) == FALSE) &&
                (IP_LIB_IsIpInClassC((UI8_T *)&network_address) == FALSE) && ((network_address & sub_netmask) != 0))
        {
            return (DHCP_MGR_INVALID_IP);
        }

        /*  network b'cast ip   */
        if (IP_LIB_GetSubnetBroadcastIp((UI8_T *)&network_address, (UI8_T *)&sub_netmask, (UI8_T *)&b_addr) == IP_LIB_OK)
        {
            if (b_addr == network_address)
            {
                return (DHCP_MGR_INVALID_IP);
            }
        }

        /* loopback ip, b'cast ip, or m'cast ip */
        if ((IP_LIB_IsLoopBackIp((UI8_T *)&network_address) == TRUE) ||
                (IP_LIB_IsMulticastIp((UI8_T *)&network_address) == TRUE) ||
                (IP_LIB_IsBroadcastIp((UI8_T *)&network_address) == TRUE))
        {
            return (DHCP_MGR_INVALID_IP);
        }

        if (!IP_LIB_IsValidNetworkMask((UI8_T *)&sub_netmask))
        {
            return (DHCP_MGR_INVALID_MASK);
        }

        /* find pool by pool name. Check pool type if it is a host config pool */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Network IP to pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }


        if (pool->pool_type == DHCP_MGR_POOL_HOST)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Network IP to pool");
            return DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS;
        }


        if (pool->network_address != 0)
        {
            /* unlink this pool in network config sorted link list but don't free this link list */
            ret = DHCP_WA_UnLinkFromNetworkPoolSortedList(pool);
            if (ret != DHCP_WA_OK)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                         EH_TYPE_MSG_FAILED_TO_SET,
                                         SYSLOG_LEVEL_ERR,
                                         "Fail to Set Network IP to pool");
                return DHCP_MGR_FAIL;
            }
        }


        /* 2. set network address into pool */
        pool->network_address = network_address;
        pool->sub_netmask = sub_netmask;
        pool->pool_type = DHCP_MGR_POOL_NETWORK;

        /* 3. link to network sorted linked list */
        ret = DHCP_WA_LinkToNetworkPoolSortedList(pool);
        if (ret != DHCP_WA_OK)
        {
            pool->network_address = 0;
            pool->sub_netmask = 0;
            pool->pool_type = DHCP_MGR_POOL_NONE;
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Network IP to pool");
            return DHCP_MGR_FAIL;
        }
        return DHCP_MGR_OK;

    } /*  end of DHCP_MGR_SetNetworkToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelNetworkFromPoolConfigEntry
     * PURPOSE:
     *      Delete subnet number and mask from pool.
     *
     * INPUT:
     *      pool_name           -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to set network to pool config
     *      DHCP_MGR_NO_SUCH_POOL_NAME -- pool name not existed.
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelNetworkFromPoolConfigEntry(char *pool_name)
    {
        UI32_T ret;
        DHCP_TYPE_PoolConfigEntry_T *tmp_pool, *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Network IP from pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        /* 2. Unlink from network sorted link list */
        tmp_pool = DHCP_WA_GetNetworkPoolbyIpMask(pool->network_address, pool->sub_netmask);
        if (tmp_pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Network IP from pool");
            return DHCP_MGR_FAIL;
        }
        else
        {
            ret = DHCP_WA_UnLinkFromNetworkPoolSortedList(tmp_pool);

            if (ret != DHCP_WA_OK)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                         EH_TYPE_MSG_FAILED_TO_DELETE,
                                         SYSLOG_LEVEL_ERR,
                                         "Fail to Delete Network IP from pool");
                return DHCP_MGR_FAIL;
            }

            /* 3. Update pool config link list by pool_name: clear network address,
             *and subnet mask
             */
            pool->network_address = 0;
            pool->sub_netmask = 0;

            /* 4. Set pool type to DHCP_TYPE_POOL_NONE */
            pool->pool_type = DHCP_TYPE_POOL_NONE;
        }

        return DHCP_MGR_OK;

    } /* DHCP_MGR_DelNetworkFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetMacToPoolConfigEntry
     * PURPOSE:
     *      Set hardware address to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      mac       -- hardware address
     *      type      -- hardware address type  (DHCP_TYPE_HTYPE_ETHER / DHCP_TYPE_HTYPE_IEEE802)
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set pool config entry to WA.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- Can't find any pool by specified pool_name
     *      DHCP_MGR_MAC_OVERSIZE       -- MAC address len larger than requirment.
     *
     * NOTES:
     *      1. For type, if user types 'ethernet', type = DHCP_MGR_HTYPE_ETHER;
     *          if user types 'ieee802', type = DHCP_MGR_HTYPE_IEEE802. If user
     *          doesn't type anything, by default, type = DHCP_MGR_HTYPE_ETHER
     *
     */
    UI32_T DHCP_MGR_SetMacToPoolConfigEntry(char *pool_name, UI8_T *mac, UI32_T type)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) || (NULL == mac))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],mac[%02x-%02x-%02x-%02x-%02x-%02x],type[%lu]",
                pool_name, DHCP_EXPAND_MAC(mac), (unsigned long)type);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Mac to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        /* 2. set mac and mac type to the pool */
        pool->hardware_address.htype = type;
        pool->hardware_address.hlen = SYS_ADPT_MAC_ADDR_LEN;
        memcpy(pool->hardware_address.haddr, mac, pool->hardware_address.hlen);

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetMacToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelMacFromPoolConfigEntry
     * PURPOSE:
     *      Delete hardware address from pool config entry.
     *
     * INPUT:
     *      pool_name -- pool name.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to delete pool config entry to WA.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- Can't find any pool by specified pool_name
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelMacFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Mac from Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Mac from Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        memset(&pool->hardware_address, 0 , sizeof(struct hardware));

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_DelMacFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetCidToPoolConfigEntry
     * PURPOSE:
     *      Set client identifier to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      cid_mode  -- cid mode (text format or hex format)
     *      cid_len   -- length of cid
     *      buf       -- buffer that contains cid
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully
     *      DHCP_MGR_FAIL               -- fail to set Cid to pool config
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- cid_len exceeds max size defined in leaf_es3626a.h
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed
     *
     * NOTES:
     *      1. possible values of mode {DHCP_MGR_CID_HEX | DHCP_MGR_CID_TEXT}
     */
    UI32_T DHCP_MGR_SetCidToPoolConfigEntry(char *pool_name, UI32_T cid_mode, UI32_T cid_len, char *buf)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || buf == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Cid to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],cid_mode[%lu],cid_len[%lu],buf[%s]", pool_name, (unsigned long)cid_mode, (unsigned long)cid_len, buf);

        if (cid_len > DHCP_MGR_CID_BUF_MAX_SIZE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Cid to Pool");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Cid to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            pool->options.cid.id_mode = cid_mode;
            pool->options.cid.id_len  = cid_len;
            memcpy(pool->options.cid.id_buf, buf, cid_len);
            return DHCP_MGR_OK;
        }

    } /* end of DHCP_MGR_SetCidToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelCidToPoolConfigEntry
     * PURPOSE:
     *      Delete client identifier from pool config entry
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to remove cid in the pool.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelCidToPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Cid from pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Cid from pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            memset(&pool->options.cid, 0, sizeof(DHCP_MGR_ClientId_T));
            return DHCP_MGR_OK;
        }

    } /* end of DHCP_MGR_DelCidToPoolConfigEntry*/

    /* FUNCTION NAME : DHCP_MGR_SetDomainNameToPoolConfigEntry
     * PURPOSE:
     *      Set domain name to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name       -- pool name
     *      domain_name     -- domain name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set domain name
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- the length of domain name exceeds MAX size.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetDomainNameToPoolConfigEntry(char *pool_name, char *domain_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || domain_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Domain Name to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],domain_name[%s]", pool_name, domain_name);

        if (strlen(domain_name) > DHCP_WA_MAX_DOMAIN_NAME_LEN)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Domain Name to Pool");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Domain Name to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            strcpy(pool->options.domain_name, domain_name);

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDomainNameToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelDomainNameFromPoolConfigEntry
     * PURPOSE:
     *      Delete domain name from pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to set domain name
     *      DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelDomainNameFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Domain Name from Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Domain Name from Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(pool->options.domain_name, 0, DHCP_WA_MAX_DOMAIN_NAME_LEN);

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_DelDomainNameFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetDnsServerToPoolConfigEntry
     * PURPOSE:
     *      Set domain name server ip to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      ip_array  -- array that contains DNS server ip address(es)
     *      size      -- number of array elements
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set domain name
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- exceed max number of DNS server
     *
     * NOTES:
     *      1. The max numbers of dns server ip are 2.
     */
    UI32_T DHCP_MGR_SetDnsServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        int i;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || ip_array == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set DNS Server to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],size[d],ip1[%u.%u.%u.%u],ip2[%u.%u.%u.%u]",
                pool_name, size, L_INET_EXPAND_IP(ip_array[0]), L_INET_EXPAND_IP(ip_array[1]));

        if (size > DHCP_WA_MAX_NBR_OF_DNS_SERVER)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set DNS Server to Pool");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set DNS Server to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            for (i = 0; i < size; i++)
                pool->options.dns_server[i] = ip_array[i];
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDnsServerToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelDnsServerFromPoolConfigEntry
     * PURPOSE:
     *      Delete domain name server ip from pool config entry
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_FAIL   -- fail to remove dns server address(es)
     *
     * NOTES:
     *      1. The max numbers of dns server ip are 8.
     */
    UI32_T DHCP_MGR_DelDnsServerFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete DNS Server from Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete DNS Server from Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(pool->options.dns_server, 0, (DHCP_WA_MAX_NBR_OF_DNS_SERVER * sizeof(UI32_T)));

        return DHCP_MGR_OK;

    }/* end of DHCP_MGR_DelDnsServerFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetLeaseTimeToPoolConfigEntry
     * PURPOSE:
     *      Set lease time to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      day       -- day (0-365)
     *      hour      -- hour (0-23)
     *      minute    -- minute (0-59)
     *      is_infinite  -- infinite lease time (TRUE / FALSE)
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to set domain name
     *      DHCP_MGR_INVALID_ARGUMENT   -- argument is over boundary
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetLeaseTimeToPoolConfigEntry(char *pool_name, UI32_T lease_time , UI32_T infinite)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Lease Time");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],lease_time[%lu],infinite[%lu]",
                pool_name, (unsigned long)lease_time, (unsigned long)infinite);

        if (lease_time == 0 && infinite == 0)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Lease Time");
            return DHCP_MGR_INVALID_ARGUMENT;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Lease Time");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            if (infinite)
                lease_time = DHCP_WA_INFINITE_LEASE_TIME;


            pool->options.lease_time = lease_time;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetLeaseTimeToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelLeaseTimeFromPoolConfigEntry
     * PURPOSE:
     *      Delete lease time from pool config entry and restore default lease time.
     *
     * INPUT:
     *      pool_name -- pool name
     *
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to delete
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelLeaseTimeFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* Check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Lease from pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Lease from pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            memset(&pool->options.lease_time, 0, sizeof(UI32_T));
            pool->options.lease_time = DHCP_WA_DEFAULT_LEASE_TIME;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_DelLeaseTimeFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetDfltRouterToPoolConfigEntry
     * PURPOSE:
     *      Set default router address to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      ip_array  -- array that contains DNS server ip address(es)
     *      size      -- number of array elements
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set domain name
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- exceed max number of dflt router
     *
     * NOTES:
     *      1. The max numbers of dflt router are 2.
     */
    UI32_T DHCP_MGR_SetDfltRouterToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        int i;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || ip_array == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Dflt router to pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],size[%d],ip1[%u.%u.%u.%u],ip2[%u.%u.%u.%u]",
                pool_name, size, L_INET_EXPAND_IP(ip_array[0]), L_INET_EXPAND_IP(ip_array[1]));

        if (size > DHCP_WA_MAX_NBR_OF_DFLT_ROUTER)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Dflt router to pool");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Dflt router to pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            for (i = 0; i < size; i++)
                pool->options.default_router[i] = ip_array[i];
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDfltRouterToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelDfltRouterFromPoolConfigEntry
     * PURPOSE:
     *      Delete default router from pool config entry.
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_FAIL   -- fail to remove dflt router address(es)
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelDfltRouterFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Dflt router to pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Dflt router to pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(pool->options.default_router, 0, (DHCP_WA_MAX_NBR_OF_DFLT_ROUTER * sizeof(UI32_T)));

        return DHCP_MGR_OK;

    }/* end of DHCP_MGR_DelDfltRouterFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetBootfileToPoolConfigEntry
     * PURPOSE:
     *      Set bootfile name to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      bootfile  -- bootfile name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set bootfile name
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- the length of bootfile name exceeds MAX size.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetBootfileToPoolConfigEntry(char *pool_name, char *bootfile)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || bootfile == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set bootfile to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s], bootfile[%s]", pool_name, bootfile);

        if (strlen(bootfile) > DHCP_WA_MAX_BOOTFILE_LEN)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set bootfile to Pool");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set bootfile to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            memset(pool->options.bootfile, 0, DHCP_WA_MAX_BOOTFILE_LEN);
            memcpy(pool->options.bootfile, bootfile, strlen(bootfile));
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetBootfileToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelBootfileFromPoolConfigEntry
     * PURPOSE:
     *      Delete bootfile name from pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to remove bootfile name
     *      DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelBootfileFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete bootfile to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete bootfile to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(pool->options.bootfile, 0, DHCP_WA_MAX_BOOTFILE_LEN);

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_DelBootfileFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry
     * PURPOSE:
     *      Set netbios name server ip to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      ip_array  -- array that contains Netbios Name Server ip address(es)
     *      size      -- number of array elements
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set netbios name server ip
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- exceed max number of netbios name server
     *
     * NOTES:
     *      1. The max numbers of dns server ip are 5.
     */
    UI32_T DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        int i;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL || ip_array == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Netbios Name Server");
            return DHCP_MGR_FAIL;
        }

        if (size > DHCP_WA_MAX_NBR_OF_NETBIOS_NAME_SERVER)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Netbios Name Server");
            return DHCP_MGR_EXCEED_MAX_SIZE;
        }

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Netbios Name Server");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            for (i = 0; i < size; i++)
                pool->options.netbios_name_server[i] = ip_array[i];
        }

        return DHCP_MGR_OK;



    } /* end of DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry
     * PURPOSE:
     *      Delete netbios name server ip from pool config entry
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_FAIL   -- fail to remove netbios name server ip address(es)
     *
     * NOTES:
     *      1. The max numbers of dns server ip are 8.
     */
    UI32_T DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;


        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Netbios Name Server");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Netbios Name Server");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(pool->options.netbios_name_server, 0, (DHCP_WA_MAX_NBR_OF_NETBIOS_NAME_SERVER * sizeof(UI32_T)));

        return DHCP_MGR_OK;
    }


    /* FUNCTION NAME : DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry
     * PURPOSE:
     *      Set netbios node type to pool config entry (set by field).
     *
     * INPUT:
     *      pool_name -- pool name
     *      node_type -- netbios node type
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to set netbios node type
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *
     * NOTES:
     *      1. possible value of node_type:
     *      DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE -- broadcast
     *      DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE -- peer to peer
     *      DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE -- mixed
     *      DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE -- hybrid (recommended)
     */
    UI32_T DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry(char *pool_name, UI32_T node_type)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Netbios Node type");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],node_type[%lu]", pool_name, (unsigned long)node_type);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Netbios Node type");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            pool->options.netbios_node_type = node_type;

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelNetbiosNodeTypeFromPoolConfigEntry
     * PURPOSE:
     *      Delete netbios node type from pool config entry and restore default lease time.
     *
     * INPUT:
     *      pool_name -- pool name
     *
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to delete
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_DelNetbiosNodeTypeFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Netbios Node Type");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Netbios Node Type");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            memset(&pool->options.netbios_node_type, 0, sizeof(UI32_T));
            pool->options.netbios_node_type = DHCP_WA_NETBIOS_NODE_TYPE_NONE;
        }

        return DHCP_MGR_OK;
    }


    /* FUNCTION NAME : DHCP_MGR_SetNextServerToPoolConfigEntry
     * PURPOSE:
     *      Set next server ip to pool config entry (set by field)
     *
     * INPUT:
     *      pool_name -- pool name
     *      ip_array  -- array that contains next server ip address(es)
     *      size      -- number of array elements
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL               -- fail to set next server
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_EXCEED_MAX_SIZE    -- exceed max number of next server
     *
     * NOTES:
     *      1. The max numbers of next server ip are 5.
     */
    UI32_T DHCP_MGR_SetNextServerToPoolConfigEntry(char *pool_name, UI32_T ipaddr)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Next Server to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],ip[%u.%u.%u.%u]", pool_name, L_INET_EXPAND_IP(ipaddr));

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Next Server to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            pool->options.next_server = ipaddr;
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetNextServerToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelNextServerFromPoolConfigEntry
     * PURPOSE:
     *      Delete next server ip from pool config entry
     *
     * INPUT:
     *      pool_name -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
     *      DHCP_MGR_FAIL   -- fail to remove next server address(es)
     *
     * NOTES:
     *      1. The max numbers of next server ip are 8.
     */
    UI32_T DHCP_MGR_DelNextServerFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Next Server to Pool");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* find pool by pool name and set the value in */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Next Server to Pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
            memset(&pool->options.next_server, 0, 4);

        return DHCP_MGR_OK;

    }/* end of DHCP_MGR_DelNextServerFromPoolConfigEntry */


    /* FUNCTION NAME : DHCP_MGR_SetHostToPoolConfigEntry
     * PURPOSE:
     *      Set host address to pool config entry (set by field).
     *
     * INPUT:
     *      pool_name           -- pool name
     *      network_address     -- host ip address
     *      sub_netmask         -- subnet mask
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                         -- successfully.
     *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- pool has configured to network pool.
     *      DHCP_MGR_FAIL                       -- fail to set host to pool config
     *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
     *      DHCP_MGR_INVALID_IP                 -- the specified host address is invalid.
     *
     * NOTES:
     *      1. host address can't be specified to '0x0'
     *      2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
     *          is 0x0, we take natural subnet mask (class A, B or C).
     *      3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
     *           pool->sub_netmask as subnet mask
     *      4. Check total numbers of host config pool (SYS_ADPT_MAX_NBR_OF_DHCP_HOST_POOL)
     *      5. sub_netmask is useless in the host pool configuration,
     *         host pool is for manual binding.
     *         consider the original design, we keep this field but not use.
     */
    UI32_T DHCP_MGR_SetHostToPoolConfigEntry(char *pool_name, UI32_T host_address, UI32_T sub_netmask)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        DHCP_TYPE_PoolConfigEntry_T network_pool;
        DHCP_WA_IpExcluded_T exclude_ip;
        int num;
        UI32_T  b_addr;
        char	wa_pool_name[DHCP_WA_MAX_POOL_NAME_LEN+1] = {0};
        BOOL_T  valid_range = FALSE;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],host_addr[%u.%u.%u.%u],subnetmask[%u.%u.%u.%u]",
                pool_name, L_INET_EXPAND_IP(host_address), L_INET_EXPAND_IP(sub_netmask));

        /* 2003.11.27, Jamescyl: Resolve EPR ES3626G-J6-00019 */
        /* Note: ip must fulfil the following conditions:
         * 1) must be either class A, class B, or class C
         * 2) can't be network b'cast ip or network id
         * 3) can't be loop back ip
         * 4) can't be b'cast ip
         * 5) can't be m'cast ip
         */
        /* class A, class B or class C */
        if ((IP_LIB_IsIpInClassA((UI8_T *)&host_address) == FALSE) && (IP_LIB_IsIpInClassB((UI8_T *)&host_address) == FALSE) &&
                (IP_LIB_IsIpInClassC((UI8_T *)&host_address) == FALSE) && ((host_address & sub_netmask) != 0))
            return (DHCP_MGR_INVALID_IP);

        /*  network b'cast ip   */
        if (IP_LIB_GetSubnetBroadcastIp((UI8_T *)&host_address, (UI8_T *)&sub_netmask, (UI8_T *)&b_addr) == IP_LIB_OK)
        {
            if (b_addr == host_address)
                return (DHCP_MGR_INVALID_IP);
        }

        /*  network ID  */
        if (host_address != 0)
        {
            if ((host_address & sub_netmask) == host_address)
                return (DHCP_MGR_INVALID_IP);
        }
        /* loopback ip, b'cast ip, or m'cast ip */
        if ((IP_LIB_IsLoopBackIp((UI8_T *)&host_address) == TRUE) ||
                (IP_LIB_IsMulticastIp((UI8_T *)&host_address) == TRUE) ||
                (IP_LIB_IsBroadcastIp((UI8_T *)&host_address) == TRUE))
            return (DHCP_MGR_INVALID_IP);

        /* End of ES3626G-J6-00019 */

        /* 1. find pool by pool name. Check pool type if it is a network config pool.
        ** If user not specify subnet mask, use pool's subnet mask if any; otherwise,
        ** use natural subnet.
        */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Host Ip to pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        if (host_address == 0)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Host Ip to pool");
            return DHCP_MGR_INVALID_IP;
        }

        if (pool->pool_type == DHCP_MGR_POOL_NETWORK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Host Ip to pool");
            return DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS;
        }


        if (sub_netmask == 0)
        {
            if (pool->sub_netmask == 0)
            {
                /* get natural subnet mask */
                sub_netmask = DHCP_ALGO_GetNaturalSubnetMask(host_address);
                DHCP_BD(CONFIG, "Natural subnet mask[%u.%u.%u.%u]", L_INET_EXPAND_IP(sub_netmask));
            }
            else
            {
                sub_netmask = pool->sub_netmask;
            }
        }


        /* check if host pool is in the network range of network pool */
        memset(&network_pool, 0, sizeof(network_pool));
        while (DHCP_WA_GetNextPoolConfigEntry(wa_pool_name, &network_pool))
        {
            if (network_pool.pool_type != DHCP_MGR_POOL_NETWORK)
                continue;

            if ((network_pool.network_address & network_pool.sub_netmask) ==
                    (host_address & network_pool.sub_netmask))
            {
                valid_range = TRUE;
                break;
            }

            memcpy(wa_pool_name, network_pool.pool_name, sizeof(wa_pool_name));
        }

        /* check if host range is in the excluded list */
        memset(&exclude_ip, 0, sizeof(exclude_ip));
        while (DHCP_WA_GetNextIpExcluded(&exclude_ip))
        {
            if ((exclude_ip.low_excluded_address <= host_address) &&
                    (exclude_ip.high_excluded_address >= host_address))
            {
                valid_range = FALSE;
                break;
            }
        }

        if (FALSE == valid_range)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Host Ip to pool");
            return DHCP_MGR_FAIL;
        }

        /* Check total numbers of host config pool */
        DHCP_WA_GetNumbersOfHostPool(&num);
        if (num >= SYS_ADPT_MAX_NBR_OF_DHCP_HOST_POOL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Set Host Ip to pool");
            return DHCP_MGR_FAIL;
        }
        /* 2. set host address into pool */
        pool->host_address = host_address;
        pool->sub_netmask = sub_netmask;     /* this field is not used in host pool */
        pool->pool_type = DHCP_MGR_POOL_HOST;

        DHCP_WA_SetNumbersOfHostPool(1, TRUE);

        return DHCP_MGR_OK;
    } /*  end of DHCP_MGR_SetHostToPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_DelHostFromPoolConfigEntry
     * PURPOSE:
     *      Delete host address and mask from pool.
     *
     * INPUT:
     *      pool_name           -- pool name
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to remove host to pool config
     *      DHCP_MGR_NO_SUCH_POOL_NAME -- pool not existed
     *
     * NOTES:
     *      1. Check total number of host config pool.
     */
    UI32_T DHCP_MGR_DelHostFromPoolConfigEntry(char *pool_name)
    {
        DHCP_TYPE_PoolConfigEntry_T *tmp_pool;
        int num;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        tmp_pool = DHCP_WA_FindPoolByPoolName(pool_name);
        if (tmp_pool == NULL)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Delete Host Ip to pool");
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            /* Check total numbers of host config pool */
            DHCP_WA_GetNumbersOfHostPool(&num);

            if (num == 0)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                         EH_TYPE_MSG_FAILED_TO_DELETE,
                                         SYSLOG_LEVEL_ERR,
                                         "Fail to Delete Host Ip to pool");
                return DHCP_MGR_FAIL;
            }

            /* 2. Update pool config link list; clear host address */
            tmp_pool->host_address = 0;

            /* 3. Set pool type to DHCP_TYPE_POOL_NONE */
            tmp_pool->pool_type = DHCP_TYPE_POOL_NONE;

            /* Decrease numbers of host pool */
            DHCP_WA_SetNumbersOfHostPool(1, FALSE);
        }
        return DHCP_MGR_OK;
    } /* DHCP_MGR_DelHostFromPoolConfigEntry */

    /* FUNCTION NAME : DHCP_MGR_GetIpBinding
     * PURPOSE:
     *      Get the specified ip binding in server_om (memory.c)
     *
     * INPUT:
     *      ip_address -- the ip for corresponding binding information
     *
     * OUTPUT:
     *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
     *                    others: the pointer to the ip_binding lease in memory.c
     *
     *
     * RETURN:
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding)
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == ip_binding)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get ip_addr[%u.%u.%u.%u]", L_INET_EXPAND_IP(ip_address));

        if (DHCP_MEMORY_GetIpBindingLease(ip_address, (DHCP_MEMORY_Server_Lease_Config_T *)ip_binding))
        {
            return DHCP_MGR_OK;
        }
        else
        {

            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Ip binding");
            return DHCP_MGR_FAIL;
        }
    } /* end of  DHCP_MGR_GetIpBinding */

    /* FUNCTION NAME : DHCP_MGR_GetActiveIpBinding
     * PURPOSE:
     *      Get the active specified ip binding in server_om (memory.c)
     *
     * INPUT:
     *      ip_address -- the ip for corresponding binding information
     *
     * OUTPUT:
     *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
     *                    others: the pointer to the ip_binding lease in memory.c
     *
     *
     * RETURN:
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetActiveIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding)
    {
        UI32_T cur_time = 0;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == ip_binding)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get ip_addr[%u.%u.%u.%u]", L_INET_EXPAND_IP(ip_address));

        /* get system real time */
        SYS_TIME_GetRealTimeBySec(&cur_time);

        if (DHCP_MEMORY_GetIpBindingLease(ip_address, (DHCP_MEMORY_Server_Lease_Config_T *)ip_binding))
        {
            if ((cur_time - ip_binding->start_time) <= ip_binding->lease_time)
            {
                return DHCP_MGR_OK;
            }
            else
                return DHCP_MGR_FAIL;
        }
        else
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Ip binding");
            return DHCP_MGR_FAIL;
        }
    }

    /* FUNCTION NAME : DHCP_MGR_GetNextIpBinding
     * PURPOSE:
     *      Get the next specified ip binding in server_om (memory.c)
     *
     * INPUT:
     *      ip_address -- the ip for corresponding binding information
     *
     * OUTPUT:
     *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
     *                    others: the pointer to the next ip_binding lease
     *                      in memory.c
     *
     * RETURN:
     *
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get next ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetNextIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding)
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == ip_binding)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get ip_addr[%u.%u.%u.%u]", L_INET_EXPAND_IP(ip_address));

        if (DHCP_MEMORY_GetNextIpBindingLease(ip_address, (DHCP_MEMORY_Server_Lease_Config_T *)ip_binding))
        {
            return DHCP_MGR_OK;
        }
        else
        {

            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next Ip binding");
            return DHCP_MGR_FAIL;
        }

    } /* end of  DHCP_MGR_GetNextIpBinding */

    /* FUNCTION NAME : DHCP_MGR_GetNextActiveIpBinding
     * PURPOSE:
     *      Get the next active specified ip binding in server_om (memory.c)
     *
     * INPUT:
     *      ip_address -- the ip for corresponding binding information
     *
     * OUTPUT:
     *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
     *                    others: the pointer to the next ip_binding lease
     *                      in memory.c
     *
     * RETURN:
     *
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get next ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetNextActiveIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding)
    {
        UI32_T cur_time = 0;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == ip_binding)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get ip_addr[%u.%u.%u.%u]", L_INET_EXPAND_IP(ip_address));

        /* get system real time */
        SYS_TIME_GetRealTimeBySec(&cur_time);

        while (TRUE == DHCP_MEMORY_GetNextIpBindingLease(ip_address, (DHCP_MEMORY_Server_Lease_Config_T *)ip_binding))
        {
            if ((cur_time - ip_binding->start_time) <= ip_binding->lease_time)
            {
                return DHCP_MGR_OK;
            }
            else
            {
                /* ignore timeout lease, get next lease */
                ip_address = ip_binding->lease_ip;
                continue;
            }
        }

        return DHCP_MGR_FAIL;
    }


    /* FUNCTION NAME : DHCP_MGR_ClearIpBinding
     * PURPOSE:
     *      Clear the specified ip binding in server_om (memory.c)
     *
     * INPUT:
     *      ip_address -- the ip for marking available again in pool
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_FAIL   --  FAIL to clear ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_ClearIpBinding(UI32_T ip_address)
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set ip_addr[%u.%u.%u.%u]", L_INET_EXPAND_IP(ip_address));

        if (ip_address == 0)
        {
            return DHCP_MGR_FAIL;
        }

        if (DHCP_MEMORY_ClearIpBinding(ip_address))
        {
            return DHCP_MGR_OK;
        }
        else
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Clear Ip binding");
            return DHCP_MGR_FAIL;
        }

    } /* end of  DHCP_MGR_ClearIpBinding */

    /* FUNCTION NAME : DHCP_MGR_ClearAllIpBinding
     * PURPOSE:
     *      Clear all ip bindings in server_om (memory.c)
     *
     * INPUT:
     *      None.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_FAIL   --  FAIL to clear ip binding.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_ClearAllIpBinding(void)
    {
        UI32_T ip_address;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        ip_address = 0;
        if (DHCP_MEMORY_ClearIpBinding(ip_address))
        {
            return DHCP_MGR_OK;
        }
        else
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Clear All Ip bindings");
            return DHCP_MGR_FAIL;
        }

    } /* end of DHCP_MGR_ClearAllIpBinding */

    /* FUNCTION NAME : DHCP_MGR_GetPoolByPoolName
     * PURPOSE:
     *      Get pool config by pool name in WA.
     *
     * INPUT:
     *      pool_name -- the name of the pool to be searched.
     *
     * OUTPUT:
     *      pool_config -- the pointer to the pool config structure
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get.
     *
     * NOTES:
     *      1. pool_name can't be null string.
     *
     */
    UI32_T DHCP_MGR_GetPoolByPoolName(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config_p)
    {
        BOOL_T ret;
        DHCP_TYPE_PoolConfigEntry_T tmp_pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || pool_name[0] == 0)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to get pool name");
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_name);

        ret = DHCP_WA_GetPoolConfigEntryByPoolName(pool_name, &tmp_pool);
        if (ret == FALSE)
        {
            pool_config_p = NULL;
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to get pool name");
            return DHCP_MGR_FAIL;
        }
        else
        {
            memcpy(pool_config_p, &tmp_pool, sizeof(DHCP_TYPE_PoolConfigEntry_T));
            return DHCP_MGR_OK;
        }
    }

    /* FUNCTION NAME : DHCP_MGR_GetNextPoolConfig
     * PURPOSE:
     *      Get next config for pool in WA.
     *
     * INPUT:
     *      pool_config -- the pointer to the pool config structure
     *                  -- NULL to get the 1st pool config
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get.
     *
     * NOTES:
     *
     *
     */
    UI32_T DHCP_MGR_GetNextPoolConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config_p)
    {

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_config_p)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_config_p->pool_name);

        if (!DHCP_WA_GetNextPoolConfigEntry(pool_config_p->pool_name, pool_config_p))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get Next pool");
            return DHCP_MGR_FAIL;
        }
        else
        {
            return DHCP_MGR_OK;
        }

    } /* end of DHCP_MGR_GetNextPoolConfig */

    /* FUNCTION	NAME : DHCP_MGR_GetActivePool
     * PURPOSE:
     *		Get   active pool for WA. Search key is pool_name + pool_range.low_address
     *
     * INPUT:
     *		pool_name -- the pointer to the pool name
     *            pool_range_p.low_address --  pool_range to get next range. key is pool_range.low_address.
     * OUTPUT:
     *		pool_range_p.high_address
     *
     * RETURN:
     *		DHCP_MGR_OK	-- successfully.
     *		DHCP_MGR_FAIL	--  FAIL to get.
     *              DHCP_MGR_INVALID_ARGUMENT
     *              DHCP_MGR_NO_SUCH_POOL_NAME  -- can not find pool with giving pool name
     *
     * NOTES:
     *
     *
     */
    UI32_T DHCP_MGR_GetActivePool(char* pool_name, DHCP_MGR_ActivePoolRange_T *pool_range_p)
    {

        DHCP_TYPE_PoolConfigEntry_T tmp_pool;
        DHCP_WA_IpRange_T tmp_range;

        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE ||
                pool_name == NULL ||
                pool_range_p == NULL)
        {

            return DHCP_MGR_FAIL;
        }

        memset(&tmp_range, 0, sizeof(DHCP_WA_IpRange_T));

        while (DHCP_WA_GetNextActivePool(&tmp_range))
        {
            if (pool_range_p->low_address == tmp_range.low_address)
            {
                if (DHCP_WA_GetPoolConfigEntryByPoolName(pool_name, &tmp_pool)
                        && ((tmp_pool.network_address&tmp_pool.sub_netmask) == (pool_range_p->low_address & tmp_pool.sub_netmask)))
                {
                    pool_range_p->high_address = tmp_range.high_address;

                    return DHCP_MGR_OK;
                }
            }
        }


        return DHCP_MGR_FAIL;
    }



    /* FUNCTION	NAME : DHCP_MGR_GetNextActivePool
     * PURPOSE:
     *		Get next active pool for WA. Search key is pool_name + pool_range.low_address
     *
     * INPUT:
     *		pool_name -- the pointer to the pool name
     *                        -- get 1st pool if pool_name is null string
     *              pool_range_p --  pool_range to get next range. key is pool_range.low_address.
     *                           --  get 1st range if pool_range.low_address is 0
     * OUTPUT:
     *		pool_range_p.
     *      pool_name
     *
     * RETURN:
     *      DHCP_MGR_OK -- successfully.
     *      DHCP_MGR_FAIL   --  FAIL to get.
     *              DHCP_MGR_INVALID_ARGUMENT
     *
     * NOTES:
     *
     *
     */
    UI32_T DHCP_MGR_GetNextActivePool(char* pool_name, DHCP_MGR_ActivePoolRange_T *pool_range_p)
    {

        DHCP_TYPE_PoolConfigEntry_T tmp_pool;
        DHCP_WA_IpRange_T tmp_range;

        struct iaddr addr;

        struct subnet *subnet = NULL;
        BOOL_T ret = FALSE;


        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE ||
                pool_name == NULL ||
                pool_range_p == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s], low_addr[%u.%u.%u.%u], high_addr[%u.%u.%u.%u]",

                pool_name, L_INET_EXPAND_IP(pool_range_p->low_address),
                L_INET_EXPAND_IP(pool_range_p->high_address));

        tmp_range.low_address = pool_range_p->low_address;

        tmp_range.high_address = pool_range_p->high_address;

        if (strlen(pool_name) == 0)
        {
            tmp_pool.pool_name[0] = 0;
            memset(pool_range_p, 0, sizeof(DHCP_MGR_ActivePoolRange_T));
            memset(&tmp_range, 0, sizeof(DHCP_MGR_ActivePoolRange_T));
            ret = DHCP_WA_GetNextPoolConfigEntry(tmp_pool.pool_name, &tmp_pool);
        }
        else
        {
            strcpy(tmp_pool.pool_name, pool_name);

            if (!(ret = DHCP_WA_GetPoolConfigEntryByPoolName(tmp_pool.pool_name, &tmp_pool)))
            {

                ret = DHCP_WA_GetNextPoolConfigEntry(tmp_pool.pool_name, &tmp_pool);
            }

        }

        if (!ret)
        {

            return DHCP_MGR_FAIL;
        }

        /* check if server is on */
        /* if dhcp server is not enabled, we should not get active range of the pool */
        if (FALSE == DHCP_OM_IsServerOn())
        {
            return DHCP_MGR_FAIL;
        }

        do
        {
            while (DHCP_WA_GetNextActivePool(&tmp_range))
            {
                /* check if interface address is in the range of pool */
                memset(&addr, 0, sizeof(addr));
                addr.len = SYS_ADPT_IPV4_ADDR_LEN;
                memcpy(addr.iabuf, &(tmp_range.low_address), addr.len);
                subnet = find_subnet(addr);

                if ((!subnet) ||
                        (!subnet->interface))
                {
                    continue;
                }

                if ((tmp_range.low_address & tmp_pool.sub_netmask) == (tmp_pool.network_address & tmp_pool.sub_netmask))
                {
                    strcpy(pool_name, tmp_pool.pool_name);
                    pool_range_p->low_address = tmp_range.low_address;
                    pool_range_p->high_address = tmp_range.high_address;

                    return DHCP_MGR_OK;
                }
            }

            memset(&tmp_range, 0, sizeof(DHCP_MGR_ActivePoolRange_T));
        }
        while (DHCP_WA_GetNextPoolConfigEntry(tmp_pool.pool_name, &tmp_pool));


        return DHCP_MGR_FAIL;
    }


    /* FUNCTION NAME : DHCP_MGR_GetDhcpServerServiceStatus
     * PURPOSE:
     *      Get DHCP Server Service Status. (Enable / Disable)
     *
     * INPUT:
     *      None.
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      TRUE  -- Indicate DHCP server is enabled.
     *      FALSE -- Indicate DHCP server is disabled.
     *
     * NOTES:
     *      This API is for WEB to get Currently system running status for DHCP Server.
     *
     */
    BOOL_T DHCP_MGR_GetDhcpServerServiceStatus(void)
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return FALSE;
        }

        if (!DHCP_OM_IsServerOn())
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                     EH_TYPE_MSG_FAILED_TO_GET,
                                     SYSLOG_LEVEL_ERR,
                                     "Fail to Get dhcp server status");
            return FALSE;
        }
        else
        {
            return TRUE;
        }

    } /* end of DHCP_MGR_GetDhcpServerServiceStatus  */

    /* FUNCTION NAME : DHCP_MGR_GetDhcpPoolTable
     * PURPOSE:
     *      Get dhcp pool to WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
      *     pool_table_p -- the pointer to dhcp pool table.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to get dhcp pool table
     *
     * NOTES:
     */
    UI32_T DHCP_MGR_GetDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_table_p)
    {
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) || (NULL == pool_table_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_name);

        ret = DHCP_MGR_GetPoolByPoolName(pool_name, pool_table_p);

        if (ret != DHCP_MGR_OK)
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_GetDhcpPoolTable */

    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolTable
     * PURPOSE:
     *      Get next dhcp pool to WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
     *      pool_table_p -- the pointer to dhcp pool table.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to get next dhcp pool table.
     *
     * NOTES:
     *  1. *pool_name = 0 to get the 1st pool and returning pool_name in *pool_name.
     */
    UI32_T DHCP_MGR_GetNextDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_table_p)
    {
        UI32_T ret;
        DHCP_TYPE_PoolConfigEntry_T pool_config;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) || (NULL == pool_table_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_name);

        if (strlen(pool_name) > SYS_ADPT_DHCP_MAX_POOL_NAME_LEN)
        {
            return DHCP_MGR_FAIL;
        }

        memset(pool_config.pool_name, 0 , SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);

        strcpy(pool_config.pool_name, pool_name);
        ret = DHCP_MGR_GetNextPoolConfig(&pool_config);

        if (ret != DHCP_MGR_OK)
        {
            return ret;
        }

        strcpy(pool_name, pool_config.pool_name);

        memcpy(pool_table_p, &pool_config, sizeof(DHCP_TYPE_PoolConfigEntry_T));

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_GetNextDhcpPoolTable */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolPoolType
     * PURPOSE:
     *      Set pool type to the pool. (set by field)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
     *      pool_type       -- the type of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
     *      DHCP_MGR_FAIL               -- fail to set the pool type.
     *
     * NOTES:
     *      1. If pool has been configured to other pool_type, user must delete the original
     *          pool before setting new pool type.
     */
    UI32_T DHCP_MGR_SetDhcpPoolPoolType(char *pool_name, UI32_T pool_type)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],pool_type[%lu]", pool_name, (unsigned long)pool_type);

        /* 1. find pool by pool name. Check pool type if it is a host config pool */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        /* 2. Check if pool has previous configuration that was different from pool_type */
        if (pool->pool_type != DHCP_MGR_POOL_NONE && pool->pool_type != pool_type)
        {
            return DHCP_MGR_FAIL;
        }
        else
            pool->pool_type = pool_type;

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDhcpPoolPoolType */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolPoolAddress
     * PURPOSE:
     *      Set pool address to the pool. (set by field)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
     *      pool_address    -- the address of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
     *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- fail to set pool address due to user not yet
     *                                              specified pool type.
     *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
     *      DHCP_MGR_FAIL                       --  fail to set the pool address.
     *
     * NOTES:
     *      1. User needs to specified pool type first before specified pool address.
     */
    UI32_T DHCP_MGR_SetDhcpPoolPoolAddress(char *pool_name, UI32_T pool_address)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],pool_address[%u.%u.%u.%u]",
                pool_name, L_INET_EXPAND_IP(pool_address));

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        /* 1. Check if pool type has already been configured */
        if (pool->pool_type == DHCP_MGR_POOL_NONE)
        {
            return DHCP_MGR_FAIL;
        }
        else if (pool->pool_type == DHCP_MGR_POOL_NETWORK)
        {
            ret = DHCP_MGR_SetNetworkToPoolConfigEntry(pool_name, pool_address, pool->sub_netmask);

            if (ret != DHCP_MGR_OK)
            {
                return ret;
            }
        }
        else /* Host Pool */
        {
            ret = DHCP_MGR_SetHostToPoolConfigEntry(pool_name, pool_address, pool->sub_netmask);

            if (ret != DHCP_MGR_OK)
            {
                return ret;
            }
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolPoolAddress */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolMacAddress
     * PURPOSE:
     *      Set hardware mac to the pool. (set by field)
     *
     * INPUT:
     *      pool_name           -- specify the pool name of the table.
     *      pool_hardware_mac   -- the hardware mac of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
     *      DHCP_MGR_FAIL               -- fail to set the pool hardware mac.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetDhcpPoolMacAddress(char *pool_name, UI8_T *pool_hardware_mac)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        /* BODY */

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL || pool_hardware_mac == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],mac[%02x-%02x-%02x-%02x-%02x-%02x]",

                pool_name, DHCP_EXPAND_MAC(pool_hardware_mac));

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        if (DHCP_MGR_CheckPhysicalAddress(pool_hardware_mac) != TRUE)
        {
            return DHCP_MGR_INVALID_MAC_ADDRESS;
        }

        /* 2. set mac and mac type to the pool */
        pool->hardware_address.hlen = SYS_ADPT_MAC_ADDR_LEN;

        memcpy(pool->hardware_address.haddr, pool_hardware_mac, pool->hardware_address.hlen);

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDhcpPoolMacAddress */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolStatus
     * PURPOSE:
     *      Set pool status to the pool. (set by field)
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table.
     *      pool_status -- the status of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
     *      DHCP_MGR_FAIL               -- fail to set the pool status.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetDhcpPoolStatus(char *pool_name, UI32_T pool_status)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],pool_status[%lu]", pool_name, (unsigned long)pool_status);

        if (pool_status == VAL_dhcpPoolstatus_valid)
        {
            ret = DHCP_MGR_EnterPool(pool_name);

            if (ret != DHCP_MGR_OK)
            {
                return ret;
            }
        }
        else
        {

            /* 1. find pool by pool name. */
            pool = DHCP_WA_FindPoolByPoolName(pool_name);

            if (pool == NULL)
            {
                return DHCP_MGR_NO_SUCH_POOL_NAME;
            }

            ret = DHCP_MGR_DeletePool(pool_name);

            if (ret != DHCP_MGR_OK)
            {
                return ret;
            }
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDhcpPoolStatus */


    /* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptionTable
     * PURPOSE:
     *      Get dhcp option table from WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
      *     pool_option_p   -- the pointer to dhcp option table.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
     *      DHCP_MGR_FAIL               -- fail to get dhcp option table
     *
     * NOTES:
     */
    UI32_T DHCP_MGR_GetDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option_p)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_name);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        memcpy(pool_option_p, &pool->options, sizeof(DHCP_TYPE_ServerOptions_T));

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_GetDhcpPoolOptionTable */


    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptionTable
     * PURPOSE:
     *      Get next dhcp option table from WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table.
     *      pool_option_p   -- the pointer to dhcp option table.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL   --  fail to get dhcp option table
     *
     * NOTES:
     *  1. *pool_name = NULL to get the 1st pool option table and return 1st pool name in
     *      *pool_name.
     */
    UI32_T DHCP_MGR_GetNextDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option_p)
    {
        DHCP_TYPE_PoolConfigEntry_T tmp_pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) || (NULL == pool_option_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s]", pool_name);

        memset(&tmp_pool, 0 , sizeof(DHCP_TYPE_PoolConfigEntry_T));

        if (pool_name[0] != 0)
            strcpy(tmp_pool.pool_name, pool_name);

        ret = DHCP_MGR_GetNextPoolConfig(&tmp_pool);

        if (ret != DHCP_MGR_OK)
        {
            return ret;
        }

        memcpy(pool_option_p, &tmp_pool.options, sizeof(DHCP_TYPE_ServerOptions_T));

        strcpy(pool_name, tmp_pool.pool_name);

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_GetNextDhcpPoolOptionTable */


    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress
     * PURPOSE:
     *      Set the netbios server ip address.
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table
     *      index       -- netbios server index
     *      ip_addr     -- netbios server ip
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_FAIL   --  fail to set the value
     *
     * NOTES:
     *  1. 'index' is not array's index; instead, it is the index using in SNMP to
     *      specify either the 1st netbios server address or the second one.
     */
    UI32_T DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress(char *pool_name,
            UI32_T index,
            UI32_T ip_addr)
    {

        int i;
        UI32_T temp[SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER];
        DHCP_TYPE_PoolConfigEntry_T *pool;

        /* BODY */
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],index[%lu],ip[%u.%u.%u.%u]",
                pool_name, (unsigned long)index, L_INET_EXPAND_IP(ip_addr));
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL || index == 0)
        {
            return DHCP_MGR_FAIL;
        }

        pool->options.netbios_name_server[index-1] = ip_addr;

        memcpy(temp, pool->options.netbios_name_server, 4*SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER);

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER;i++)
        {
            if (temp[i] == 0)
            {
                /* clear out the rest */
                for (; i < SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER;i++)
                    temp[i] = 0;

                break;
            }
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress */

    /* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptNetbiosServerTable
     * PURPOSE:
     *      Get netbios server of dhcp pool from WA.
     *
     * INPUT:
     *      pool_name           -- specify the pool name of the table
     *      index               -- the address index
     *      netbios_server_p    -- the netbios server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetDhcpPoolOptNetbiosServerTable(char *pool_name,
            UI32_T index,
            UI32_T *netbios_server_p)
    {

        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) || (NULL == netbios_server_p))
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)index);

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        if (0 < index && index <= SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER)
        {
            memcpy(netbios_server_p, &pool->options.netbios_name_server[index-1], 4);
        }
        else
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_GetDhcpPoolOptNetbiosServerTable */

    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable
     * PURPOSE:
     *      Get next netbios server of dhcp pool from WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table
     *      index_p             -- the address index
     *      netbios_server_p    -- the netbios server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *  1.              ---------> Get next
     * pool_name|        address index
     *              [1]     |    [2]     |
     *          ------------+------------+
     * pool1     10.1.1.1   | 10.1.1.2   |
     *          ------------+------------+
     * pool2     10.1.2.1   | 10.1.2.2   |
     *          ------------+------------+
     *  2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
     *      ip_address = 10.1.1.1.
     *  3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
     */
    UI32_T DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable(char *pool_name,
            UI32_T *index_p,
            UI32_T *netbios_server_p)
    {
        DHCP_TYPE_PoolConfigEntry_T pool;
        UI32_T ret;


        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) ||
                (NULL == index_p) ||
                (NULL == netbios_server_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)*index_p);

        if (*index_p == SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER || *pool_name == 0)
        {
            memset(&pool, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));
            memcpy(pool.pool_name, pool_name, strlen(pool_name));
            ret = DHCP_MGR_GetNextPoolConfig(&pool);

            if (ret != DHCP_MGR_OK)
            {
                pool_name[0] = 0;
                *index_p = 0;
                return ret;
            }
            else
            {
                /* Get the 1st address of that pool */
                memcpy(pool_name, pool.pool_name, strlen(pool.pool_name));
                pool_name[strlen(pool.pool_name)] = '\0';
                *index_p = 1;
                memcpy(netbios_server_p, &pool.options.netbios_name_server[0], 4);
            }

            return DHCP_MGR_OK;
        }
        else
        {
            if (0 <= *index_p && *index_p < SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER)
            {
                ret =  DHCP_MGR_GetPoolByPoolName(pool_name, &pool);

                if (ret != DHCP_MGR_OK)
                {
                    pool_name[0] = 0;
                    *index_p = 0;
                    return ret;
                }
                else
                {
                    memcpy(netbios_server_p, &pool.options.netbios_name_server[*index_p], 4);
                    *index_p =  *index_p + 1 ;
                }

                return DHCP_MGR_OK;
            }
            else
            {
                return DHCP_MGR_FAIL;
            }

        }



    } /* end of DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable */


    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress
     * PURPOSE:
     *      Set the dns server ip address.
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table
     *      index       -- dns server index
     *      ip_addr     -- dns server ip
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   --  fail to set the value
     *
     * NOTES:
     *  1. 'index' is not array's index; instead, it is the index using in SNMP to
     *      specify either the 1st dns server address or the second one.
     *  2. If index == 1, and ip_addr[2] != 0 and ip_addr[1] == 0, this configuration will
     *      clear out all ip setting.
         */
    UI32_T DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress(char *pool_name,
            UI32_T index,
            UI32_T ip_addr)
    {
        int i;
        UI32_T temp[SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER];
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],index[%lu],ip[%u.%u.%u.%u]",
                pool_name, (unsigned long)index, L_INET_EXPAND_IP(ip_addr));

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL || index == 0)
        {
            return DHCP_MGR_FAIL;
        }

        pool->options.dns_server[index-1] = ip_addr;

        memcpy(temp, pool->options.dns_server, 4*SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER);

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER;i++)
        {
            if (temp[i] == 0)
            {
                /* clear out the rest */
                for (; i < SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER;i++)
                    temp[i] = 0;

                break;
            }
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress */

    /* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptionDnsSerTable
     * PURPOSE:
     *      Get dns server of dhcp pool from WA.
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table
     *      index           -- the index of the address
     *      dns_server_p    -- the dns server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     */
    UI32_T DHCP_MGR_GetDhcpPoolOptionDnsSerTable(char *pool_name,
            UI32_T index,
            UI32_T *dns_server_p)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)index);

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        if (0 < index && index <= SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER)
        {
            memcpy(dns_server_p, &pool->options.dns_server[index-1], 4);
            return DHCP_MGR_OK;
        }

        return DHCP_MGR_FAIL;


    } /* end of DHCP_MGR_GetDhcpPoolOptionDnsSerTable */

    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable
     * PURPOSE:
     *      Get next dns server table of dhcp pool from WA.
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table
     *      index_p         -- the index of the address
     *      dns_server_p    -- the array to store dns server addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *
     *  1.              ---------> Get next
     * pool_name|        address index
     *              [1]     |    [2]     |
     *          ------------+------------+
     * pool1     10.1.1.1   | 10.1.1.2   |
     *          ------------+------------+
     * pool2     10.1.2.1   | 10.1.2.2   |
     *          ------------+------------+
     *  2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
     *      ip_address = 10.1.1.1.
     *  3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
     */
    UI32_T DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable(char *pool_name,
            UI32_T *index_p,
            UI32_T *dns_server_p)
    {

        DHCP_TYPE_PoolConfigEntry_T pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) ||
                (NULL == index_p) ||
                (NULL == dns_server_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)*index_p);

        if (*index_p == SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER || *pool_name == 0)
        {
            memset(&pool, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));
            memcpy(pool.pool_name, pool_name, strlen(pool_name));
            ret = DHCP_MGR_GetNextPoolConfig(&pool);

            if (ret != DHCP_MGR_OK)
            {
                pool_name[0] = 0;
                *index_p = 0;
                return ret;
            }
            else
            {
                /* Get the 1st address of that pool */
                memcpy(pool_name, pool.pool_name, strlen(pool.pool_name));
                pool_name[strlen(pool.pool_name)] = '\0';
                *index_p = 1;
                memcpy(dns_server_p, &pool.options.dns_server[0], 4);
            }

            return DHCP_MGR_OK;
        }

        else
        {
            if (0 <= *index_p && *index_p < SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER)
            {
                ret =  DHCP_MGR_GetPoolByPoolName(pool_name, &pool);

                if (ret != DHCP_MGR_OK)
                {
                    pool_name[0] = 0;
                    *index_p = 0;
                    return ret;
                }
                else
                {
                    memcpy(dns_server_p, &pool.options.dns_server[*index_p], 4);
                    *index_p =  *index_p + 1;
                }

                return DHCP_MGR_OK;
            }
            else
            {
                return DHCP_MGR_FAIL;
            }

        }

    } /* end of DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable */


    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress
     * PURPOSE:
     *      Set the dflt router ip address.
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table
     *      index       -- dflt router address index
     *      ip_addr     -- dflt router ip
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  fail to set the value
     *
     * NOTES:
     *  1. 'index' is not array's index; instead, it is the index using in SNMP to
     *      specify either the 1st dflt router address or the second one.
     */
    UI32_T DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress(char *pool_name,
            UI32_T index,
            UI32_T ip_addr)
    {

        int i;
        UI32_T temp[SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER];
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }


        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],index[%lu],ip[%u.%u.%u.%u]",
                pool_name, (unsigned long)index, L_INET_EXPAND_IP(ip_addr));

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL || index == 0)
        {
            return DHCP_MGR_FAIL;
        }

        pool->options.default_router[index-1] = ip_addr;

        memcpy(temp, pool->options.default_router, 4*SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER);

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER;i++)
        {
            if (temp[i] == 0)
            {
                /* clear out the rest */
                for (; i < SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER;i++)
                    temp[i] = 0;

                break;
            }
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress */

    /* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptDefaultRouterTable
     * PURPOSE:
     *      Get dflt router of dhcp pool from WA.
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table
     *      index           -- the address index
     *      dflt_router_p   -- the dflt router addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetDhcpPoolOptDefaultRouterTable(char *pool_name,
            UI32_T index,
            UI32_T *dflt_router_p)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)index);

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        if (0 < index && index <= SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER)
        {
            memcpy(dflt_router_p, &pool->options.default_router[index-1], 4);
        }
        else
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_GetDhcpPoolOptDefaultRouterTable */

    /* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable
     * PURPOSE:
     *      Get next dflt router table of dhcp pool from WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      pool_name       -- specify the pool name of the table
     *      index_p         -- the address index
     *      dflt_router_p   -- the dflt router addresses
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *  1.              ---------> Get next
     * pool_name|        address index
     *              [1]     |    [2]     |
     *          ------------+------------+
     * pool1     10.1.1.1   | 10.1.1.2   |
     *          ------------+------------+
     * pool2     10.1.2.1   | 10.1.2.2   |
     *          ------------+------------+
     *  2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
     *      ip_address = 10.1.1.1.
     *  3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
     */
    UI32_T DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable(char *pool_name,
            UI32_T *index_p,
            UI32_T *dflt_router_p)
    {
        DHCP_TYPE_PoolConfigEntry_T pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if ((NULL == pool_name) ||
                (NULL == index_p) ||
                (NULL == dflt_router_p))
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Get pool_name[%s],index[%lu]", pool_name, (unsigned long)*index_p);

        if (*index_p == SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER || *pool_name == 0)
        {
            memset(&pool, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));
            memcpy(pool.pool_name, pool_name, strlen(pool_name));
            ret = DHCP_MGR_GetNextPoolConfig(&pool);

            if (ret != DHCP_MGR_OK)
            {
                pool_name[0] = 0;
                *index_p = 0;
                return ret;
            }
            else
            {
                /* Get the 1st address of that pool */
                memcpy(pool_name, pool.pool_name, strlen(pool.pool_name));
                pool_name[strlen(pool.pool_name)] = '\0';
                *index_p = 1;
                memcpy(dflt_router_p, &pool.options.default_router[0], 4);
            }

            return DHCP_MGR_OK;
        }
        else
        {
            if (0 <= *index_p && *index_p < SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER)
            {
                ret =  DHCP_MGR_GetPoolByPoolName(pool_name, &pool);

                if (ret != DHCP_MGR_OK)
                {
                    pool_name[0] = 0;
                    *index_p = 0;
                    return ret;
                }
                else
                {
                    memcpy(dflt_router_p, &pool.options.default_router[*index_p], 4);
                    *index_p = *index_p + 1;
                }

                return DHCP_MGR_OK;
            }
            else
            {
                return DHCP_MGR_FAIL;
            }

        }

    } /* end of DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionCidMode
     * PURPOSE:
     *      Set client identifier mode to the option table from WA.
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table.
     *      cid_mode    -- cid mode for this pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 --  successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  --  pool not exist.
     *      DHCP_MGR_FAIL               --  fail to set the value
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetDhcpPoolOptionCidMode(char *pool_name, UI32_T cid_mode)
    {

        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],cid_mode[%lu]", pool_name, (unsigned long)cid_mode);

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }
        else
        {
            if (cid_mode != VAL_dhcpPoolOptionCidMode_hex && cid_mode != VAL_dhcpPoolOptionCidMode_text)
            {
                return DHCP_MGR_FAIL;
            }
            else
                pool->options.cid.id_mode = cid_mode;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolOptionCidMode */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionCidBuffer
     * PURPOSE:
     *      Set client identifier in the buffer to the option table from WA.
     *
     * INPUT:
     *      pool_name   -- specify the pool name of the table.
     *      cid_buf     -- cid buffer for this pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK     --  successfully.
     *      DHCP_MGR_FAIL   --  fail to set the value
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_SetDhcpPoolOptionCidBuffer(char *pool_name, char *cid_buf)
    {

        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL || cid_buf == NULL)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],cid_buf[%s]", pool_name, cid_buf);

        if (strlen((char *)cid_buf) > DHCP_MGR_CID_BUF_MAX_SIZE)
            return DHCP_MGR_EXCEED_MAX_SIZE;

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        else
        {
            /* Check if pool type has been configured. cid_mode can not be
            **  VAL_dhcpPoolOptionCidMode_notSpecify
            */
            if (pool->options.cid.id_mode == VAL_dhcpPoolOptionCidMode_notSpecify)
                return DHCP_MGR_FAIL;

            pool->options.cid.id_len  = strlen(cid_buf);

            memcpy(pool->options.cid.id_buf, cid_buf, pool->options.cid.id_len);
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolOptionCidBuffer */


    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolSubnetMask
     * PURPOSE:
     *      Set subnet mask to the pool. (set by field)
     *
     * INPUT:
     *      pool_name           -- specify the pool name of the table.
     *      pool_subnet_mask    -- the subnet mask of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                         -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
     *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- fail to set pool address due to user not yet
     *                                              specified pool type.
     *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
     *      DHCP_MGR_FAIL                       -- fail to set the pool subnet mask.
     *
     * NOTES:
     *  1. If pool_subnet_mask is the same as what we previous configured, do nothing for it.
     */
    UI32_T DHCP_MGR_SetDhcpPoolSubnetMask(char *pool_name, UI32_T pool_subnet_mask)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;
        UI32_T ret;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* check null pointer */
        if (NULL == pool_name)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set pool_name[%s],mask[%u.%u.%u.%u]",
                pool_name, L_INET_EXPAND_IP(pool_subnet_mask));

        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        if (pool_subnet_mask != pool->sub_netmask)
        {
            if (pool->pool_type == DHCP_MGR_POOL_NETWORK)
            {
                ret = DHCP_MGR_SetNetworkToPoolConfigEntry(pool_name,
                        pool->network_address,
                        pool_subnet_mask);

                if (ret != DHCP_MGR_OK)
                {
                    return ret;
                }
            }
            else if (pool->pool_type == DHCP_MGR_POOL_HOST)
            {
                ret = DHCP_MGR_SetHostToPoolConfigEntry(pool_name,
                                                        pool->host_address,
                                                        pool_subnet_mask);

                if (ret != DHCP_MGR_OK)
                {
                    return ret;
                }
            }
            else
            {
                if (pool_subnet_mask != 0)
                    pool->sub_netmask = pool_subnet_mask;
            }

        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_SetDhcpPoolSubnetMask */

    /* FUNCTION NAME : DHCP_MGR_SetDhcpPoolHardwareType
     * PURPOSE:
     *      Set hardware type to the pool. (set by field)
     *
     * INPUT:
     *      pool_name           -- specify the pool name of the table.
     *      pool_hardware_type  -- the hardware type of the pool.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *      DHCP_MGR_OK                 -- successfully.
     *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
     *      DHCP_MGR_FAIL               -- fail to set the pool hardware type.
     *
     * NOTES:
     *  1. SNMP needs to change the hardware type from leaf constant value
     *      to the corresponding value presenting in DHCP_MGR.h
     */
    UI32_T DHCP_MGR_SetDhcpPoolHardwareType(char *pool_name, UI32_T pool_hardware_type)
    {
        DHCP_TYPE_PoolConfigEntry_T *pool;

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (pool_name == NULL)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set pool_name[%s],hardware_type[%lu]", pool_name, (unsigned long)pool_hardware_type);

        /* 1. find pool by pool name. */
        pool = DHCP_WA_FindPoolByPoolName(pool_name);

        if (pool == NULL)
        {
            return DHCP_MGR_NO_SUCH_POOL_NAME;
        }

        /* 2. set mac and mac type to the pool */
        if (pool_hardware_type != DHCP_MGR_HTYPE_NONE &&
                pool_hardware_type != DHCP_MGR_HTYPE_ETHER &&
                pool_hardware_type != DHCP_MGR_HTYPE_IEEE802 &&
                pool_hardware_type != DHCP_MGR_HTYPE_FDDI)
        {
            return DHCP_MGR_FAIL;
        }

        pool->hardware_address.htype = pool_hardware_type;

        return DHCP_MGR_OK;
    } /* end of DHCP_MGR_SetDhcpPoolHardwareType */

    /* FUNCTION NAME : DHCP_MGR_GetDhcpServerExcludedIpAddrTable
     * PURPOSE:
     *      Get exclude IP addr table of dhcp pool from WA. (Get by record function for SNMP)
     *
     * INPUT:
     *      low_address  -- the lowest ip address of the excluded ip range
     *      high_address -- the highest ip address of the excluded ip range
     *
     * OUTPUT:
     *      low_address  -- the lowest ip address of the excluded ip range
     *      high_address -- the highest ip address of the excluded ip range
     *
     * RETURN:
     *      DHCP_MGR_OK     -- successfully.
     *      DHCP_MGR_FAIL   -- fail to get the value.
     *
     * NOTES:
     *
     */
    UI32_T DHCP_MGR_GetDhcpServerExcludedIpAddrTable(UI32_T *low_address,
            UI32_T *high_address)
    {

        DHCP_WA_IpExcluded_T  ip_address;
        /* BODY */

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (low_address == NULL || high_address == NULL || *low_address == 0)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get low_addr[%u.%u.%u.%u], high_addr[%u.%u.%u.%u]",

                L_INET_EXPAND_IP(*low_address), L_INET_EXPAND_IP(*high_address));

        ip_address.low_excluded_address = *low_address;

        ip_address.high_excluded_address = *high_address;

        if (!DHCP_WA_GetIpExcluded(&ip_address))
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;

    } /* end of DHCP_MGR_GetDhcpServerExcludedIpAddrTable */

    /* FUNCTION NAME : DHCP_MGR_CheckPhysicalAddress
     * PURPOSE:check if DHCP physical address valid.
     *
     * INPUT:phy_addr
     *
     *
     * OUTPUT:None
     *
     *
     * RETURN:
     *    TRUE  --  valid
     *    FALSE --  invalid
     *
     * NOTES:ARP physical address should not be all-zero, broastcast,multicast,eaps address.
     *
     */
    static BOOL_T   DHCP_MGR_CheckPhysicalAddress(UI8_T *phy_addr)
    {
        UI8_T null_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
        UI8_T broadcast_addr[SYS_ADPT_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        UI8_T mutilcast_addr[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
        UI8_T eaps_addr[SYS_ADPT_MAC_ADDR_LEN] = {0x00, 0xe0, 0x2b, 0x00, 0x00, 0x04};

        if (phy_addr == NULL)
            return FALSE;

        if (memcmp(phy_addr, null_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 ||
                memcmp(phy_addr, broadcast_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 ||
                memcmp(phy_addr, mutilcast_addr, 1) == 0 ||
                memcmp(phy_addr, eaps_addr, SYS_ADPT_MAC_ADDR_LEN) == 0)
            return FALSE;
        else
            return TRUE;
    }
#endif
    /*-----------------------------------------------------------------------------
     * ROUTINE NAME: DHCP_MGR_HandleIPCReqMsg
     *-----------------------------------------------------------------------------
     * PURPOSE : Handle the ipc request message for DHCP MGR.
     *
     * INPUT   : msgbuf_p -- input request ipc message buffer
     *
     * OUTPUT  : msgbuf_p -- output response ipc message buffer
     *
     * RETURN  : TRUE  - there is a response required to be sent
     *           FALSE - there is no response required to be sent
     *
     * NOTES   : None.
     *-----------------------------------------------------------------------------
     */
    BOOL_T DHCP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
    {
        DHCP_MGR_IpcMsg_T *msg_p;

        if (msgbuf_p == NULL)
        {
            return FALSE;
        }

        msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

        /* Every ipc request will fail when operating mode is transition mode
         */
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            msg_p->type.ret_ui32 = DHCP_MGR_FAIL;
            msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
            return TRUE;
        }

        /* dispatch IPC message and call the corresponding DHCP_MGR function
         */
        switch (msg_p->type.cmd)
        {
            case DHCP_MGR_IPC_RESTART3:
                DHCP_MGR_Restart3(msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_REMOVESYSTEMROLE:
                msg_p->type.ret_ui32 = DHCP_MGR_RemoveSystemRole(msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RESTART:
                DHCP_MGR_Restart();
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_SETCLIENTID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_SetClientId(
                                           msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_2,
                                           msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_3,
                                           msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_cid);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_GETCLIENTID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetClientId(
                                           msg_p->data.arg_grp_ui32_clientid.arg_ui32,
                                           &msg_p->data.arg_grp_ui32_clientid.arg_client_id);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
                break;

            case DHCP_MGR_IPC_GETRUNNINGCLIENTID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetRunningClientId(
                                           msg_p->data.arg_grp_ui32_clientid.arg_ui32,
                                           &msg_p->data.arg_grp_ui32_clientid.arg_client_id);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
                break;

            case DHCP_MGR_IPC_GETNEXTCLIENTID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetNextClientId(
                                           &msg_p->data.arg_grp_ui32_clientid.arg_ui32,
                                           &msg_p->data.arg_grp_ui32_clientid.arg_client_id);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
                break;

            case DHCP_MGR_IPC_DELETECLIENTID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_DeleteClientId(msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_SETIFROLE:
                msg_p->type.ret_ui32 = DHCP_MGR_SetIfRole(
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_C_SETVENDORCLASSID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_SetVendorClassId(
                                           msg_p->data.arg_grp_vendor_class_id.vid,
                                           msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_mode,
                                           msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_len,
                                           msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_buf);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_C_DELETEVENDORCLASSID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_DeleteVendorClassId(
                                           msg_p->data.arg_grp_vendor_class_id.vid);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_C_GETVENDORCLASSID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetVendorClassId(
                                           msg_p->data.arg_grp_vendor_class_id.vid,
                                           &msg_p->data.arg_grp_vendor_class_id.vendordata);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
                break;

            case DHCP_MGR_IPC_C_GETRUNNINGVENDORCLASSID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetRunningVendorClassId(
                                           msg_p->data.arg_grp_vendor_class_id.vid,
                                           &msg_p->data.arg_grp_vendor_class_id.vendordata);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
                break;

            case DHCP_MGR_IPC_C_GETNEXTVENDORCLASSID:
                msg_p->type.ret_ui32 = DHCP_MGR_C_GetNextVendorClassId(
                                           &msg_p->data.arg_grp_vendor_class_id.vid,
                                           &msg_p->data.arg_grp_vendor_class_id.vendordata);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
                break;

            case DHCP_MGR_IPC_C_RELEASE_CLIENT_LEASE:
                msg_p->type.ret_ui32 = DHCP_MGR_ReleaseClientLease(msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
#if (SYS_CPNT_DHCP_SERVER == TRUE)
            case DHCP_MGR_IPC_S_CLEARALLIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_ClearAllIpBinding();
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_CLEARIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_ClearIpBinding(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETEXCLUDEDIP:
                msg_p->type.ret_ui32 = DHCP_MGR_SetExcludedIp(
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELEXCLUDEDIP:
                msg_p->type.ret_ui32 = DHCP_MGR_DelExcludedIp(
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_ENTERDHCPPOOL:
                msg_p->type.ret_ui32 = DHCP_MGR_EnterPool(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_S_DELDHCPPOOL:
                msg_p->type.ret_ui32 = DHCP_MGR_DeletePool(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETHOSTTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetHostToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELHOSTFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelHostFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETMACTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolMacAddress(
                                           msg_p->data.arg_grp_pool_config_phy_addr.pool_name,
                                           msg_p->data.arg_grp_pool_config_phy_addr.phy_addr);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETMACTYPETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolHardwareType(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELMACFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelMacFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETNETWORKTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetNetworkToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELNETWORKFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelNetworkFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDOMAINNAMETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDomainNameToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_domain_name.pool_name,
                                           msg_p->data.arg_grp_pool_domain_name.domain_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELDOMAINNAMEFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelDomainNameFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDNSSERVERTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDnsServerToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config_iparray.pool_name,
                                           msg_p->data.arg_grp_pool_config_iparray.ip_array.dns_server,
                                           msg_p->data.arg_grp_pool_config_iparray.size);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELDNSSERVERFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelDnsServerFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETLEASETIMETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetLeaseTimeToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELLEASETIMEFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelLeaseTimeFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDEFAULTROUTETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDfltRouterToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config_iparray.pool_name,
                                           msg_p->data.arg_grp_pool_config_iparray.ip_array.default_router,
                                           msg_p->data.arg_grp_pool_config_iparray.size);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELDEFAULTROUTEFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelDfltRouterFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETBOOTFILETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetBootfileToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_boot_file.pool_name,
                                           msg_p->data.arg_grp_pool_boot_file.bootfile);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELBOOTFILEFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelBootfileFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETNETBIOSNAMESERVERTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config_iparray.pool_name,
                                           msg_p->data.arg_grp_pool_config_iparray.ip_array.netbios_name_server,
                                           msg_p->data.arg_grp_pool_config_iparray.size);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELNETBIOSNAMESERVERFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETNETBIOSNODETYPETOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELNETBIOSNODETYPEFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelNetbiosNodeTypeFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name) ;
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETNEXTSERVERTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetNextServerToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELNEXTSERVERFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelNextServerFromPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name) ;
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETCLIENTIDENTIFIERTOPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_SetCidToPoolConfigEntry(
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.pool_name,
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_ui32_2,
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_cid);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_DELCLIENTIDENTIFIERFROMPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_DelCidToPoolConfigEntry(
                                           msg_p->data.arg_grp_pool_name.pool_name) ;
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_GETDHCPPOOLTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpPoolTable(
                                           msg_p->data.arg_grp_pool_config_entry.pool_name,
                                           &msg_p->data.arg_grp_pool_config_entry.pool_config);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPPOOLTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpPoolTable(
                                           msg_p->data.arg_grp_pool_config_entry.pool_name,
                                           &msg_p->data.arg_grp_pool_config_entry.pool_config);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
                break;
            case DHCP_MGR_IPC_S_GETDHCPPOOLOPTIONTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpPoolOptionTable(
                                           msg_p->data.arg_grp_pool_option.pool_name,
                                           &msg_p->data.arg_grp_pool_option.pool_option);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_option);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPPOOLOPTIONTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpPoolOptionTable(
                                           msg_p->data.arg_grp_pool_option.pool_name,
                                           &msg_p->data.arg_grp_pool_option.pool_option);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_option);
                break;
            case DHCP_MGR_IPC_S_GETDHCPPOOLDNSSERVERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpPoolOptionDnsSerTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDNSSERVERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETDHCPPOOLDEFAULTROUTERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpPoolOptDefaultRouterTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDEFAULTROUTERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETDHCPPOOLNETBIOSSERVERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpPoolOptNetbiosServerTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPPOOLNETBIOSSERVERTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           &msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
                break;
            case DHCP_MGR_IPC_S_GETDHCPSERVEREXCLUDEDIPTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpServerExcludedIpAddrTable(
                                           &msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
                break;
            case DHCP_MGR_IPC_S_GETNEXTDHCPSERVEREXCLUDEDIPTABLE:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextExcludedIp(
                                           &msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
                break;
            case DHCP_MGR_IPC_S_GETIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_GetIpBinding(
                                           msg_p->data.arg_grp_lease_entry.ipaddr,
                                           &msg_p->data.arg_grp_lease_entry.lease_entry);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
                break;
            case DHCP_MGR_IPC_S_GETACTIVEIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_GetActiveIpBinding(
                                           msg_p->data.arg_grp_lease_entry.ipaddr,
                                           &msg_p->data.arg_grp_lease_entry.lease_entry);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
                break;
            case DHCP_MGR_IPC_S_GETNEXTIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextIpBinding(
                                           msg_p->data.arg_grp_lease_entry.ipaddr,
                                           &msg_p->data.arg_grp_lease_entry.lease_entry);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
                break;
            case DHCP_MGR_IPC_S_GETNEXTACTIVEIPBINDING:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextActiveIpBinding(
                                           msg_p->data.arg_grp_lease_entry.ipaddr,
                                           &msg_p->data.arg_grp_lease_entry.lease_entry);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLSTATUS:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolStatus(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLOPTDNSSERIP:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLOPTDEFAULTROUTERIP:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLOPTNETBIOSSERVERIP:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_2);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDBUF:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolOptionCidBuffer(
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.pool_name,
                                           msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_cid);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDMODE:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolOptionCidMode(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLPOOLTYPE:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolPoolType(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLADDR:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolPoolAddress(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_S_SETDHCPPOOLSUBNETMASK:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpPoolSubnetMask(
                                           msg_p->data.arg_grp_pool_config.pool_name,
                                           msg_p->data.arg_grp_pool_config.arg_ui32_1);

                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_GETDHCPSERVERSERVICESTATUS:
                msg_p->type.ret_bool = DHCP_MGR_GetDhcpServerServiceStatus();
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
            case DHCP_MGR_IPC_R_ADDRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_AddInterfaceRelayServerAddress(
                                           msg_p->data.arg_grp_relay_server.ifindex,
                                           msg_p->data.arg_grp_relay_server.num,
                                           msg_p->data.arg_grp_relay_server.ip_array);

                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_R_DELALLRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_DeleteAllRelayServerAddress(
                                           msg_p->data.arg_grp_relay_server.ifindex);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_R_DELRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_DeleteInterfaceRelayServerAddress(
                                           msg_p->data.arg_grp_relay_server.ifindex,
                                           msg_p->data.arg_grp_relay_server.num,
                                           msg_p->data.arg_grp_relay_server.ip_array);

                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_R_GETIFRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_GetIfRelayServerAddress(
                                           msg_p->data.arg_grp_relay_server.ifindex,
                                           msg_p->data.arg_grp_relay_server.ip_array);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_relay_server);;
                break;
            case DHCP_MGR_IPC_R_GETRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_GetDhcpRelayServerAddrTable(
                                           msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
                                           &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);

                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
                break;
            case DHCP_MGR_IPC_R_GETNEXTRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextDhcpRelayServerAddrTable(
                                           &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
                                           &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
                                           &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);

                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
                break;
            case DHCP_MGR_IPC_R_SETRELAYSERVERADDRESS:
                msg_p->type.ret_ui32 = DHCP_MGR_SetDhcpRelayServerAddrServerIp(
                                           msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
                                           (int)msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
                                           msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);

                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
#endif
            case DHCP_MGR_IPC_SETRESTARTOBJECT:
                msg_p->type.ret_ui32 = DHCP_MGR_SetRestartObject(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_DELETEIFROLE:
                msg_p->type.ret_ui32 = DHCP_MGR_DeleteIfRole(
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                                           msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);

                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
            case DHCP_MGR_IPC_CHECKRESTARTOBJ:
                msg_p->type.ret_ui32 = DHCP_MGR_CheckRestartObj(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;
#if (SYS_CPNT_DHCP_SERVER == TRUE)
            case DHCP_MGR_IPC_S_GETNEXTPOOLCONFIG:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextPoolConfig(
                                           &(msg_p->data.arg_grp_pool_config_entry.pool_config));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
                break;

            case DHCP_MGR_IPC_S_GETNEXTACTIVEPOOL:
                msg_p->type.ret_ui32 = DHCP_MGR_GetNextActivePool(
                                           msg_p->data.arg_active_pool_range.pool_name,
                                           &(msg_p->data.arg_active_pool_range.pool_range));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_active_pool_range);
                break;
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
            case DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_STATUS:
                msg_p->type.ret_ui32 = DHCP_MGR_SetOption82Status(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_POLICY:
                msg_p->type.ret_ui32 = DHCP_MGR_SetOption82Policy(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_MODE:
                msg_p->type.ret_ui32 = DHCP_MGR_SetOption82RidMode(
                                           msg_p->data.arg_ui32);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_VALUE:
                msg_p->type.ret_ui32 = DHCP_MGR_SetOption82RidValue(
                                           msg_p->data.arg_grp_option82_rid_value.arg_rid_value);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_SUBTYPE_FORMAT:
                msg_p->type.ret_ui32 = DHCP_MGR_SetOption82Format(
                                           msg_p->data.arg_grp_option82_subtype_format.subtype_format);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER:

                msg_p->type.ret_ui32 = DHCP_MGR_SetRelayServerAddress(
                                           msg_p->data.arg_grp_option82_relay_server.arg_relay_server[0],
                                           msg_p->data.arg_grp_option82_relay_server.arg_relay_server[1],
                                           msg_p->data.arg_grp_option82_relay_server.arg_relay_server[2],
                                           msg_p->data.arg_grp_option82_relay_server.arg_relay_server[3],
                                           msg_p->data.arg_grp_option82_relay_server.arg_relay_server[4]);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER_FROM_SNMP:
                msg_p->type.ret_ui32 = DHCP_MGR_SetRelayServerAddressFromSnmp(
                                           msg_p->data.arg_grp_option82_relay_server_snmp.index,
                                           msg_p->data.arg_grp_option82_relay_server_snmp.server_ip);
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RELAY_SERVER_FROM_SNMP:
                msg_p->type.ret_ui32 = DHCP_MGR_GetRelayServerAddressFromSnmp(
                                           msg_p->data.arg_grp_option82_relay_server_snmp.index,
                                           &(msg_p->data.arg_grp_option82_relay_server_snmp.server_ip));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server_snmp);
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_DELETE_RELAY_SERVER:
                msg_p->type.ret_ui32 = DHCP_MGR_DeleteGlobalRelayServerAddress();
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_STATUS:
                msg_p->type.result_running_cfg = DHCP_MGR_GetRunningDhcpRelayOption82Status(
                                                     &(msg_p->data.arg_ui32));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_POLICY:
                msg_p->type.result_running_cfg = DHCP_MGR_GetRunningDhcpRelayOption82Policy(
                                                     &(msg_p->data.arg_ui32));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RID_MODE:
                msg_p->type.result_running_cfg = DHCP_MGR_GetRunningDhcpRelayOption82RidMode(
                                                     &(msg_p->data.arg_ui32));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_FORMAT:
                msg_p->type.result_running_cfg = DHCP_MGR_GetRunningDhcpRelayOption82Format(
                                                     &(msg_p->data.arg_grp_option82_subtype_format.subtype_format));
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_subtype_format);
                break;

            case DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RELAY_SERVER:
                msg_p->type.result_running_cfg = DHCP_MGR_GetRunningRelayServerAddress(
                                                     msg_p->data.arg_grp_option82_relay_server.arg_relay_server);
                msgbuf_p->msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server);
                break;
#endif

            default:
                SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
                msg_p->type.ret_ui32 = DHCP_MGR_FAIL;
                msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
        }

        return TRUE;
    } /* End of DHCP_MGR_HandleIPCReqMsg */





#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
    BOOL_T DHCP_MGR_SetClientNameServer(UI32_T vid_ifindex, UI8_T name_server[SYS_ADPT_IPV4_ADDR_LEN])
    {
        L_INET_AddrIp_T dns_addr;
        struct interface_info   *if_pointer, *if_null;
        memset(&dns_addr, 0, sizeof(dns_addr));
        int i;

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu],dns[%u.%u.%u.%u]",
                (unsigned long)vid_ifindex, L_INET_EXPAND_IP(name_server));

        dns_addr.type = L_INET_ADDR_TYPE_IPV4;
        dns_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        if_null = NULL;

        for (if_pointer = DHCP_OM_GetNextInterface(if_null);if_pointer != NULL;if_pointer = if_pointer->next)
        {
            if (if_pointer->vid_ifIndex == vid_ifindex)
                continue;
            if (!if_pointer->client)
                continue;
            if (!if_pointer->client->active)
                continue;

            for (i = 0;i < if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].len / 4;i++)
            {
                memcpy(dns_addr.addr, if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].data + i*4, 4);

                if (memcmp(dns_addr.addr, name_server, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                {
                    return TRUE;
                }
            }
        }

        memcpy(dns_addr.addr, name_server, SYS_ADPT_IPV4_ADDR_LEN);

        if (DNS_OK != DNS_PMGR_AddNameServer(&dns_addr))
        {
            DHCP_BD(CONFIG, "Failed to add dns server to DNS_PMGR");
            return FALSE;
        }

        if (TRUE != DNS_PMGR_EnableDomainLookup())
        {
            DHCP_BD(CONFIG, "Failed to enable domain lookup");
            return FALSE;
        }
        return TRUE;
    }

    BOOL_T DHCP_MGR_DeleteClientNameServer(UI32_T vid_ifindex)
    {
        L_INET_AddrIp_T dns_addr;
        struct interface_info   *if_pointer, *if_null;
        void* name_server_list, *data_ptr;
        memset(&dns_addr, 0, sizeof(dns_addr));
        int i, j;
        int name_server_count = 0;
        BOOL_T in_use = FALSE;
        dns_addr.type = L_INET_ADDR_TYPE_IPV4;
        dns_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        if_null = NULL;

        DHCP_BD(CONFIG, "Set vid_ifindex[%lu]", (unsigned long)vid_ifindex);
        /* gate name server list */

        for (if_pointer = DHCP_OM_GetNextInterface(if_null);if_pointer != NULL;if_pointer = if_pointer->next)
        {
            if (if_pointer->vid_ifIndex != vid_ifindex)
                continue;
            if (!if_pointer->client)
                continue;
            if (!if_pointer->client->active)
            {
                DHCP_BD(CONFIG, "No active client lease");
                return FALSE;
            }
            name_server_count = if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].len / 4;
            name_server_list = if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].data;
            break;
        }
        for (j = 0;j < name_server_count;j++)
        {
            in_use = FALSE;
            memcpy(dns_addr.addr, name_server_list + j*4, 4);
            for (if_pointer = DHCP_OM_GetNextInterface(if_null);if_pointer != NULL;if_pointer = if_pointer->next)
            {
                if (if_pointer->vid_ifIndex == vid_ifindex)
                    continue;
                if (!if_pointer->client)
                    continue;
                if (!if_pointer->client->active)
                    continue;
                for (i = 0;i < if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].len / 4;i++)
                {
                    data_ptr = if_pointer->client->active->options[DHO_DOMAIN_NAME_SERVERS].data + i * 4;

                    if (memcmp(dns_addr.addr, data_ptr, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                    {
                        DHCP_BD(CONFIG, "DNS server is used by vidifindex %ld", (unsigned long)if_pointer->vid_ifIndex);
                        in_use = TRUE;
                        break;
                    }
                }
            }
            if (FALSE == in_use)
            {
                if (DNS_OK != DNS_PMGR_DeleteNameServer(&dns_addr))
                {
                    DHCP_BD(CONFIG, "Failed to delete dns server to DNS_PMGR");
                }
            }
        }

        return TRUE;
    }

#endif /*#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)*/



#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)

    /*------------------------------------------------------------------------------
     * FUNCTION NAME - DHCP_MGR_SetOption82Status
     *------------------------------------------------------------------------------
     * PURPOSE  : Set Dhcp option82 status : enable/disable
     * INPUT    : status you want to set. 1 :DHCP_OPTION82_DISABLE, 2 :DHCP_OPTION82_ENABLE
     * OUTPUT   : none
     * RETURN   : DHCP_MGR_OK : If success
     *			  DHCP_MGR_FAIL:
     * NOTES    : none
     *------------------------------------------------------------------------------*/
    UI32_T DHCP_MGR_SetOption82Status(UI32_T status)
    {
        UI32_T MSG;
        UI32_T current_status;


        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set status[%lu]", (unsigned long)status);

        if (status == DHCP_OPTION82_DISABLE || status == DHCP_OPTION82_ENABLE)
        {
            if (status == DHCP_OPTION82_ENABLE)
            {
#if (SYS_CPNT_DHCPSNP == TRUE)
                {
                    UI8_T  dhcpsnp_option82_status;
                    DHCPSNP_PMGR_GetInformationOptionStatus(&dhcpsnp_option82_status);

                    if (dhcpsnp_option82_status == DHCPSNP_TYPE_OPTION82_ENABLED)
                    {
                        DHCP_BD(CONFIG, "dhcpsnp option82 is enabled.");
                        return DHCP_MGR_FAIL;
                    }
                }
#endif
            }

            if (DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Status(&current_status))
            {
                return DHCP_MGR_FAIL;
            }

            if (status != current_status)
            {
                if (DHCP_OM_OK != DHCP_OM_SetDhcpRelayOption82Status(status))
                {
                    return DHCP_MGR_FAIL;
                }

                /* DHCP relay information option 82 is enabled */
                if (DHCP_OPTION82_ENABLE == status)
                {
                    /* Enable to trap DHCP server and client packet to cpu */
                    if (FALSE == SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_L2_RELAY))
                    {
                        DHCP_BD(CONFIG, "Fail to enable dhcp trap");
                        return DHCP_MGR_FAIL;
                    }

#if (SYS_CPNT_DHCPSNP == TRUE)
                    {
                        if(DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetL2RelayForwarding(FALSE))
                        {
                            DHCP_BD(EVENT, "Failed to set l2 relay forwarding flag");
                            return DHCP_MGR_FAIL;
                        }
                    }
#endif  //SYS_CPNT_DHCPSNP
                }
                else
                {

                    /* Diable to trap DHCP server and client packet to cpu */
                    if (FALSE == SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_L2_RELAY))
                    {
                        DHCP_BD(CONFIG, "Fail to diable dhcp trap");
                        return DHCP_MGR_FAIL;
                    }

#if (SYS_CPNT_DHCPSNP == TRUE)
                    {
                        if(DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetL2RelayForwarding(TRUE))
                        {
                            DHCP_BD(EVENT, "Failed to set l2 relay forwarding flag");
                            return DHCP_MGR_FAIL;
                        }
                    }
#endif  //SYS_CPNT_DHCPSNP
                }

            }

            MSG = DHCP_MGR_OK;

        }
        else
        {
            MSG = DHCP_MGR_FAIL;
        }

        return MSG;
    }



    /*------------------------------------------------------------------------------
     * FUNCTION NAME - DHCP_MGR_SetOption82Policy
     *------------------------------------------------------------------------------
     * PURPOSE  : Set Dhcp option82 policy : drop/replace/keep
     * INPUT    : policy you want to set. 1 :DHCP_OPTION82_POLICY_DROP, 2 :DHCP_OPTION82_POLICY_REPLACE
                       3:DHCP_OPTION82_POLICY_KEEP
     * OUTPUT   : none
     * RETURN   : DHCP_MGR_OK : If success
     *			  DHCP_MGR_FAIL:
     * NOTES    : none
     *------------------------------------------------------------------------------*/
    UI32_T DHCP_MGR_SetOption82Policy(UI32_T policy)
    {
        UI32_T current_policy;


        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set policy[%lu]", (unsigned long)policy);

        if ((policy != DHCP_OPTION82_POLICY_DROP) &&
                (policy != DHCP_OPTION82_POLICY_REPLACE) &&
                (policy != DHCP_OPTION82_POLICY_KEEP))
        {
            return DHCP_MGR_INVALID_ARGUMENT;
        }

        if (DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Policy(&current_policy))
        {
            return DHCP_MGR_FAIL;
        }


        if (policy != current_policy)
        {
            if (DHCP_OM_OK != DHCP_OM_SetDhcpRelayOption82Policy(policy))
            {
                return DHCP_MGR_FAIL;
            }
        }

        return DHCP_MGR_OK;
    }



    /*------------------------------------------------------------------------------
     * FUNCTION NAME - DHCP_MGR_SetOption82RidMode
     *------------------------------------------------------------------------------
     * PURPOSE  : Set Dhcp option82 remote id mode
     * INPUT    : mode you want to set: DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP
     * OUTPUT   : none
     * RETURN   : DHCP_MGR_OK : If success
     *			  DHCP_MGR_FAIL:
     * NOTES    : none
     *------------------------------------------------------------------------------*/
    UI32_T DHCP_MGR_SetOption82RidMode(UI32_T mode)
    {
        UI32_T current_mode;

        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set mode[%lu]", (unsigned long)mode);

        if ((mode < DHCP_OPTION82_RID_MAC_HEX) || (mode > DHCP_OPTION82_RID_CONFIGURED_STRING))
        {
            return DHCP_MGR_INVALID_ARGUMENT;
        }

        if (DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82RidMode(&current_mode))
        {
            return DHCP_MGR_FAIL;
        }

        if (mode != current_mode)
        {
            if (DHCP_OM_OK != DHCP_OM_SetDhcpRelayOption82RidMode(mode))
            {
                return DHCP_MGR_FAIL;
            }

        }

        return DHCP_MGR_OK;
    }


    /*------------------------------------------------------------------------------
     * FUNCTION NAME - DHCP_MGR_SetOption82RidValue
     *------------------------------------------------------------------------------
     * PURPOSE  : Set Dhcp option82 remote id value
     * INPUT    : string   --  remote id string
     * OUTPUT   : none
     * RETURN   : TRUE : If success
     *			  FALSE:
     * NOTES    : max number of characters is 32.
     *------------------------------------------------------------------------------*/
    UI32_T DHCP_MGR_SetOption82RidValue(UI8_T *string)
    {

        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
            return DHCP_MGR_FAIL;

        if (string == NULL)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set rid[%s]", string);

        if (DHCP_OM_OK != DHCP_OM_SetDhcpRelayOption82RidValue(string))
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;
    }

    /*------------------------------------------------------------------------------
    * FUNCTION NAME - DHCP_MGR_SetOption82Format
    *------------------------------------------------------------------------------
    * PURPOSE  : Set Dhcp option82 encode format
    * INPUT    : subtype_format     -- Setting value
    * OUTPUT   : none
    * RETURN   : DHCP_MGR_OK : If success
    *			  DHCP_MGR_FAIL:
    * NOTES    : none
    *------------------------------------------------------------------------------*/
    UI32_T  DHCP_MGR_SetOption82Format(BOOL_T subtype_format)
    {
        BOOL_T current_format = FALSE;

        if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
            return DHCP_MGR_FAIL;

        DHCP_BD(CONFIG, "Set subtype[%s]", subtype_format ? "TRUE" : "FALSE");

        DHCP_OM_GetDhcpRelayOption82Format(&current_format);

        if (subtype_format == current_format)
        {
            return DHCP_MGR_OK;
        }

        if (DHCP_OM_OK != DHCP_OM_SetDhcpRelayOption82Format(subtype_format))
            return DHCP_MGR_FAIL;


        return DHCP_MGR_OK;
    }



    /* FUNCTION NAME : DHCP_MGR_SetRelayServerAddress
     * PURPOSE:
     *      Set global dhcp server ip address for L2 relay agent to WA
     *
     * INPUT:
     *		ip1, ip2, ip3, ip4, ip5	-- the ip list of servers
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *		DHCP_MGR_OK		-- successfully.
     *		DHCP_MGR_FAIL	--  fail to set the relay server address.
     *
     * NOTES:
     *	1. The function will delete the previous setting for relay server address
     * 		and add the newly configured relay server address
     *	2. The maximun numbers of server ip are 5. If user specifies (ip1, ip2,0,0,0) which means
     *		the user only specifies 2 DHCP Relay Server.
     *	3. If user specifies (ip1, 0,ip3, ip4, ip5), (ip1,0,0,0,0)will be set to DHCP
     *		Relay Server Address List
     *
     */
    UI32_T DHCP_MGR_SetRelayServerAddress(UI32_T ip1, UI32_T ip2, UI32_T ip3, UI32_T ip4, UI32_T ip5)
    {
        UI32_T i;
        UI32_T temp[5] = {0, 0, 0, 0, 0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set ip1[%u.%u.%u.%u]\r\n"

                "ip2[%u.%u.%u.%u]\r\n"
                "ip3[%u.%u.%u.%u]\r\n"
                "ip4[%u.%u.%u.%u]\r\n"
                "ip5[%u.%u.%u.%u]",
                L_INET_EXPAND_IP(ip1),
                L_INET_EXPAND_IP(ip2),
                L_INET_EXPAND_IP(ip3),
                L_INET_EXPAND_IP(ip4),
                L_INET_EXPAND_IP(ip5));

        temp[0] = ip1;
        temp[1] = ip2;
        temp[2] = ip3;
        temp[3] = ip4;
        temp[4] = ip5;

        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
        {

            if (IP_LIB_IsIpInClassD((UI8_T *)&temp[i]) || IP_LIB_IsIpInClassE((UI8_T *)&temp[i]) ||
                IP_LIB_IsLoopBackIp((UI8_T *)&temp[i]))
            {

                return DHCP_MGR_FAIL;
            }
        }

        if (DHCP_OM_OK != DHCP_OM_AddRelayServerAddress(temp))
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;
    } /* end of DHCP_MGR_SetRelayServerAddress */

    /* FUNCTION NAME : DHCP_MGR_SetRelayServerAddressFromSnmp
     * PURPOSE:
     *      Set global dhcp server ip address for L2 relay agent to WA from snmp
     *
     * INPUT:
     *      index      -- entry index
     *		server_ip  -- ip address of server
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *		DHCP_MGR_OK		-- successfully.
     *		DHCP_MGR_FAIL	--  fail to set the relay server address.
     *
     * NOTES:
     *	This function is only used for SNMP, it can set relay server address for specified index
     *  If there's zero ip address in entry whose index is lower than specified index, we can't let user to set from snmp.
     */
    UI32_T DHCP_MGR_SetRelayServerAddressFromSnmp(UI32_T index, UI32_T server_ip)
    {
        UI32_T relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Set index[%lu], server_ip[%u.%u.%u.%u]", (unsigned long)index, L_INET_EXPAND_IP(server_ip));

        /* index checking */
        if ((index < 1) || (index > SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER))
        {
            return DHCP_MGR_FAIL;
        }

//        server_ip = L_STDLIB_Ntoh32(server_ip);

        /* address validation checking */
        if (IP_LIB_IsIpInClassD((UI8_T*)&server_ip) ||
            IP_LIB_IsIpInClassE((UI8_T*)&server_ip) ||
            IP_LIB_IsLoopBackIp((UI8_T*)&server_ip))
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_OM_GetRelayServerAddress(relay_server);
        relay_server[index-1] = server_ip;

        /* set relay address to WA */
        if (DHCP_OM_OK != DHCP_OM_AddRelayServerAddress(relay_server))
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;
    }

    /* FUNCTION NAME : DHCP_MGR_GetRelayServerAddressFromSnmp
     * PURPOSE:
     *      Get global dhcp server ip address for L2 relay agent to WA from snmp
     *
     * INPUT:
     *      index      -- entry index
     *
     * OUTPUT:
     *      server_ip  -- ip address of server
     *
     * RETURN:
     *		DHCP_MGR_OK		-- successfully.
     *		DHCP_MGR_FAIL	--  fail to set the relay server address.
     *
     * NOTES:
     *	This function is only used for SNMP, it can get relay server address for specified index
     */
    UI32_T DHCP_MGR_GetRelayServerAddressFromSnmp(UI32_T index, UI32_T *server_ip)
    {
        UI32_T relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        /* null pointer checking */
        if (NULL == server_ip)
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_BD(CONFIG, "Get index[%lu]", (unsigned long)index);

        /* index checking */
        if ((index < 1) || (index > SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER))
        {
            return DHCP_MGR_FAIL;
        }

        DHCP_OM_GetRelayServerAddress(relay_server);

        *server_ip = relay_server[index-1];
        return DHCP_MGR_OK;
    }


    /* FUNCTION NAME : DHCP_MGR_DeleteGlobalRelayServerAddress
     * PURPOSE:
     *      Delete global Server addresses for DHCP L2 Relay Agent in WA
     *
     * INPUT:
     *      None.
     *
     * OUTPUT:
     *      None.
     *
     * RETURN:
     *		DHCP_MGR_OK	-- successfully.
     *		DHCP_MGR_FAIL	--  FAIL to delete all relay server addresses.
     *
     * NOTES:1. For add relay server address, we can add all server IPs at
     *			one time. If we modify the server address twice, the last update
     *			would replace the previous setting.
     *
     *		 2. For delete relay server IP, we only allow delete all at a time
     *
     */
    UI32_T DHCP_MGR_DeleteGlobalRelayServerAddress()
    {
        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            return DHCP_MGR_FAIL;
        }

        if (DHCP_OM_OK != DHCP_OM_DeleteGlobalRelayServerAddress())
        {
            return DHCP_MGR_FAIL;
        }

        return DHCP_MGR_OK;


    } /* end of DHCP_MGR_DeleteGlobalRelayServerAddress */

    /* FUNCTION NAME : DHCP_MGR_GetRunningDhcpRelayOption82Status
     * PURPOSE:
     *     Get running dhcp relay option 82 status
     *
     * INPUT:
     *      status_p
     *
     * OUTPUT:
     *      status_p  -- DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
     * RETURN:
     *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES: None.
     *
     */
    static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Status(UI32_T *status_p)
    {
        if (NULL == status_p)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        DHCP_OM_GetDhcpRelayOption82Status(status_p);

        if (SYS_DFLT_DHCP_OPTION82_STATUS == *status_p)
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    }

    /* FUNCTION NAME : DHCP_MGR_GetRunningDhcpRelayOption82Policy
     * PURPOSE:
     *     Get running dhcp relay option 82 policy
     *
     * INPUT:
     *      policy_p
     *
     * OUTPUT:
     *      policy_p  -- DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
     * RETURN:
     *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES: None.
     *
     */
    static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Policy(UI32_T *policy_p)
    {
        if (NULL == policy_p)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        DHCP_OM_GetDhcpRelayOption82Policy(policy_p);

        if (SYS_DFLT_DHCP_OPTION82_POLICY == *policy_p)
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    /* FUNCTION NAME : DHCP_MGR_GetRunningDhcpRelayOption82RidMode
     * PURPOSE:
     *     Get running dhcp relay option 82 remote id mode
     *
     * INPUT:
     *      mode_p
     *
     * OUTPUT:
     *      mode_p  -- DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
     * RETURN:
     *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES: None.
     *
     */
    static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82RidMode(UI32_T *mode_p)
    {
        if (NULL == mode_p)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        DHCP_OM_GetDhcpRelayOption82RidMode(mode_p);

        if (SYS_DFLT_DHCP_OPTION82_RID_MODE == *mode_p)
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    /* FUNCTION NAME : DHCP_MGR_GetRunningDhcpRelayOption82Format
     * PURPOSE:
     *     Get running dhcp relay option 82 subtype format
     *
     * INPUT:
     *      subtype_format_p
     *
     * OUTPUT:
     *      subtype_format_p  -- TRUE/FALSE
     * RETURN:
     *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES: None.
     *
     */
    static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningDhcpRelayOption82Format(BOOL_T *subtype_format_p)
    {
        if (NULL == subtype_format_p)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        DHCP_OM_GetDhcpRelayOption82Format(subtype_format_p);

        if (SYS_DLFT_DHCP_OPTION82_SUBTYPE_FORMAT == *subtype_format_p)
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;


    }

    /* FUNCTION NAME : DHCP_MGR_GetRunningRelayServerAddress
     * PURPOSE:
     *		Get running global relay server addresses.
     *
     * INPUT:
     *		relay_server -- the pointer to relay server
     *
     * OUTPUT:
     *		relay_server -- the pointer to relay server
     *
     * RETURN:
     *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
     *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
     *      SYS_TYPE_GET_RUNNING_CFG_FAIL
     *
     * NOTES: None.
     *
     */
    static SYS_TYPE_Get_Running_Cfg_T DHCP_MGR_GetRunningRelayServerAddress(UI32_T *relay_server)
    {
        if (NULL == relay_server)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        DHCP_OM_GetRelayServerAddress(relay_server);

        if (relay_server[0] == 0)
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;


    }

#endif

    /* FUNCTION	NAME : DHCP_MGR_ReleaseClientLease
     * PURPOSE:
     *		Release dhcp active client lease
     *
     * INPUT:
     *		ifindex         --  vlan interface index
     * OUTPUT:
     *		none
     *
     * RETURN:
     *		DHCP_MGR_OK	    --  successfully.
     *		DHCP_MGR_FAIL	--  fail.
     *
     *
     * NOTES:
     *		this api will free dhcp engine's active lease and send DHCPRELEASE packet to dhcp server.
     *
     */
    UI32_T DHCP_MGR_ReleaseClientLease(UI32_T ifindex)
    {
        DHCP_BD(CONFIG, "Set ifindex[%lu]", (unsigned long)ifindex);

        if (TRUE == DHCP_OM_SetIfMode(ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE))
        {

            if (FALSE == DHCP_ALGO_ReleaseClientLease(ifindex))
                return DHCP_MGR_FAIL;
            else
                return DHCP_MGR_OK;

        }
        else
            return DHCP_MGR_FAIL;
    }

    /* FUNCTION	NAME : DHCP_MGR_SignalRifDestroy
     * PURPOSE:
     *		ipcfg signal DHCP rif is destroyed
     *
     * INPUT:
     *		vid_ifindex     -- vlan interface index
     *      addr            -- rif information
     * OUTPUT:
     *		N/A
     *
     * RETURN:
     *		N/A
     *
     *
     * NOTES:
     *
     *
     */
    void DHCP_MGR_SignalRifDestroy(UI32_T vid_ifindex, L_INET_AddrIp_T *addr)
    {
        /* check null pointer */
        if (NULL == addr)
            return;

        DHCP_BD(EVENT, "vid_ifindex[%lu],ip[%u.%u.%u.%u]",
                (unsigned long)vid_ifindex, L_INET_EXPAND_IP(addr->addr));

#if (SYS_CPNT_DHCP_INFORM == TRUE)
        {
            struct interface_info *dhcp_if = NULL;
            /* check if rif is destroyed on DHCP interface which enabled DHCP inform,
             * if yes, restart DHCP client to re-build DHCP interface
             */
            dhcp_if = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifindex);
            if (NULL == dhcp_if)
            {
                /* DHCP doesn't have this interface, return */
                return;
            }

            if ((dhcp_if->dhcp_inform == TRUE) &&
                    (dhcp_if->primary_address != 0))
            {
                DHCP_MGR_Restart3(DHCP_TYPE_RESTART_CLIENT);
            }

        }
#endif
        return;
    }


    /* FUNCTION	NAME : DHCP_MGR_SignalRifUp
     * PURPOSE:
     *		ipcfg signal DHCP rif is active
     *
     * INPUT:
     *		vid_ifindex     -- vlan interface index
     *      addr            -- rif information
     * OUTPUT:
     *		N/A
     *
     * RETURN:
     *		N/A
     *
     *
     * NOTES:
     *
     *
     */
    void DHCP_MGR_SignalRifUp(UI32_T vid_ifindex, L_INET_AddrIp_T *addr)
    {

        /* check null pointer */
        if (NULL == addr)
            return;

        DHCP_BD(EVENT, "vid_ifindex[%lu],ip[%u.%u.%u.%u]",
                (unsigned long)vid_ifindex, L_INET_EXPAND_IP(addr->addr));

#if (SYS_CPNT_DHCP_INFORM == TRUE)
        {
            struct interface_info *dhcp_if = NULL;
            NETCFG_TYPE_L3_Interface_T l3_if;
            /* check if rif is active on DHCP interface which enabled DHCP inform,
             * if yes, restart DHCP client
             */
            dhcp_if = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifindex);
            if (NULL == dhcp_if)
            {
                memset(&l3_if, 0, sizeof(l3_if));
                l3_if.ifindex = vid_ifindex;
                if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&l3_if))
                {
                    return;
                }

                if (l3_if.dhcp_inform == TRUE)
                {
                    DHCP_MGR_Restart3(DHCP_TYPE_RESTART_CLIENT);
                }
            }

        }
#endif
        return;
    }



