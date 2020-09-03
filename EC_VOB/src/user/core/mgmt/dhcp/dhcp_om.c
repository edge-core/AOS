/* Module Name: DHCP_OM.C
 * Purpose:
 *		DHCP_OM holds all data structures used in run-time, includes
 *			protocol-list, which keeps socket associating with interface.
 *			interface-list, which keeps interfaces managed by DHCP.
 *
 * Notes:
 *		None.
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "dhcp_type.h"

#ifdef UNIT_TEST_DHCP
#include "unit_test_dhcp.h"
#endif

#include "sys_type.h"
#include "l_mpool.h"
#include "dhcp_om.h"
#include "dhcp_algo.h"
#include "dhcp_wa.h"
#include "dhcp_backdoor.h"
#include "memory.h"
#include "netcfg_type.h"
#include "dhcp_time.h"
#include "sys_bld.h"
#include "uc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */



/* LOCAL SUBPROGRAM DECLARATIONS
 */
static struct interface_info *DHCP_OM_FindInterfaceByMac(UI8_T *mac);
static BOOL_T DHCP_OM_FreeCurrentInterface(struct interface_info *if_pointer);
static void DHCP_OM_FreeNextInterface(struct interface_info *if_pointer, struct interface_info *free_if);
static void DHCP_OM_FreeClientLeaseOptions(struct option_data *options);
#if (SYS_CPNT_DHCP_SERVER == TRUE)
static void DHCP_OM_ResetServerIfRole();
#endif
static struct interface_info *DHCP_OM_GetPreviousInterface(struct interface_info *if_ptr_addr);

/* STATIC VARIABLE DECLARATIONS
 */
/*
 *	interface list (interfaces)
 *	using 'interfaces' as name is for backword-checking, DHCP/IAD uses this name
 *	as link list head.
 */
static struct interface_info	*interfaces;
static DHCP_OM_LCB_T			dhcp_om_lcb;
static UI8_T dhcp_client_gateway[4];
static UI32_T                   dhcp_om_sem_id; /* DHCP_OM local semaphore id */
static DHCP_OM_UC_DATA_T  *dhcp_om_uc_data=0;    /* pointer to UC data */

#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)
static UI32_T           option_status;    /* show if Dhcp option82 ON or OFF */
static UI32_T           option_policy;    /*show how to process packet exist option82*/
static UI32_T           option_rid_mode;  /*remote id sub-option*/
static UI8_T            option_rid_value[SYS_ADPT_MAX_LENGTH_OF_RID + 1];
static BOOL_T           option_subtype_format;   /* CID and RID in option82 have subtype and sublength or not */
static UI32_T       	global_relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER]; 	/* the global server address for L2 relay agent */
#endif


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION	NAME : DHCP_OM_Init
 * PURPOSE:
 *		Initialize DHCP_OM software components at system starting.
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
void	DHCP_OM_Init(void)
{
    /*	Clear the link list to empty	*/
    interfaces = NULL;
    memset((char*) &dhcp_om_lcb, 0, sizeof(DHCP_OM_LCB_T));
    /*	set default mode as DHCP enable	*/
    dhcp_om_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_DHCP;

    /* set default system role to server */
    /* Will be enabled while implement Server  */
    dhcp_om_lcb.system_role = 0;
    dhcp_om_lcb.system_relay_count = 0;
    dhcp_om_lcb.system_client_count = 0;
    memset(dhcp_client_gateway, 0, sizeof(dhcp_client_gateway));
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    option_status = 0;
    option_rid_mode = 0;
    option_policy = 0;
    option_subtype_format = 0;
    memset(option_rid_value, 0, SYS_ADPT_MAX_LENGTH_OF_RID + 1);
#endif

    /* get lease data from UC, only record one interface's lease
     */
    dhcp_om_uc_data = UC_MGR_Allocate(UC_MGR_DHCP_INFO_INDEX, sizeof(DHCP_OM_UC_DATA_T), 4);

    if(!dhcp_om_uc_data)
    {
        SYSFUN_LogDebugMsg("Can't get UC data.\n");
    }
    else
    {
        /* check magic word, if it's not matched, init the UC memory
         */
        if(dhcp_om_uc_data->magic_word != DHCP_TYPE_UC_MAGIC_WORD)
        {
            memset(dhcp_om_uc_data, 0, sizeof(DHCP_OM_UC_DATA_T));
            dhcp_om_uc_data->magic_word = DHCP_TYPE_UC_MAGIC_WORD;
        }
    }

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &dhcp_om_sem_id) != SYSFUN_OK)
    {
        SYSFUN_LogDebugMsg("NETCFG_OM_IGMP_InitateProcessResources : Can't create semaphore. \n");
    }

}	/*	end of DHCP_OM_Init	*/


/* FUNCTION	NAME : DHCP_OM_ReInit
 * PURPOSE:
 *		Reinitialize DHCP_OM software components when DHCP must rebuild its working
 *		OM. It's similiar as a process restarting, but no process resource allocation
 *		be done. ie. Semaphore, memory pool, message queue all allocated in DHCP_OM_Init
 *		not in DHCP_OM_ReInit.
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
 *		1. No resource allocation in this function, just reset data structure to initial
 *		   value and following function, discover_interface, will fill out the proper data.
 */
void DHCP_OM_ReInit(UI32_T restart_object)
{

    struct interface_info	*if_pointer, *temp_pointer, *pre_pointer;

    DHCP_BD(DATABASE, "restart object[%lu]", (unsigned long)restart_object);

    /*	1. Clear out the interface which interface binding role is
    **	the same as the restart_object
    */

    if_pointer = interfaces;
    while (if_pointer != NULL)
    {
        if (restart_object & DHCP_TYPE_RESTART_CLIENT)
        {
            if (if_pointer->role & DHCP_TYPE_BIND_CLIENT)
            {

                pre_pointer = DHCP_OM_GetPreviousInterface(if_pointer);
                if (pre_pointer == NULL)
                {
                    /* if_pointer is the 1st interface in the interfaces list */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    interfaces = if_pointer;
                }
                else
                {
                    /* free the middle of the if in interfaces */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    pre_pointer->next = temp_pointer;
                }

            }
            else /* interface = relay */
            {
                if (if_pointer->next != NULL)
                {
                    if (if_pointer->next->role & DHCP_TYPE_BIND_CLIENT)
                    {
                        temp_pointer = if_pointer->next;
                        DHCP_OM_FreeNextInterface(if_pointer, temp_pointer);

                    }
                }
                if_pointer = if_pointer->next;
            }
            continue;
        } /* end of if (restart_object & DHCP_TYPE_RESTART_CLIENT) */
        if (restart_object & DHCP_TYPE_RESTART_RELAY)
        {
            if (if_pointer->role & DHCP_TYPE_BIND_RELAY)
            {
                pre_pointer = DHCP_OM_GetPreviousInterface(if_pointer);
                if (pre_pointer == NULL)
                {
                    /* if_pointer is the 1st interface in the interfaces list */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    interfaces = if_pointer;
                }
                else
                {
                    /* free the middle of the if in interfaces */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    pre_pointer->next = temp_pointer;
                }

            }
            else
            {
                if (if_pointer->next != NULL)
                {
                    if (if_pointer->next->role & DHCP_TYPE_BIND_RELAY)
                    {
                        temp_pointer = if_pointer->next;
                        DHCP_OM_FreeNextInterface(if_pointer, temp_pointer);

                    }
                }
                if_pointer = if_pointer->next;
            }
            continue;
        } /* end of restart_object = DHCP_TYPE_RESTART_RELAY */
        if (restart_object & DHCP_TYPE_RESTART_SERVER)
        {
            if (if_pointer->mode & NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
            {
                pre_pointer = DHCP_OM_GetPreviousInterface(if_pointer);
                if (pre_pointer == NULL)
                {
                    /* if_pointer is the 1st interface in the interfaces list */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    interfaces = if_pointer;
                }
                else
                {
                    /* free the middle of the if in interfaces */
                    temp_pointer = if_pointer->next;
                    DHCP_OM_FreeCurrentInterface(if_pointer);
                    if_pointer = temp_pointer;
                    pre_pointer->next = temp_pointer;
                }
            }
            else
            {
                if (if_pointer->next != NULL)
                {
                    if (if_pointer->next->mode & NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
                    {
                        temp_pointer = if_pointer->next;
                        DHCP_OM_FreeNextInterface(if_pointer, temp_pointer);
                        //interfaces = if_pointer;
                    }
                }
                if_pointer = if_pointer->next;
            }


            //dhcp_om_lcb.system_role ^= DHCP_TYPE_BIND_SERVER;
            continue;
        } /* end of else if (restart_object & DHCP_TYPE_RESTART_SERVER) */
        else
        {
            if_pointer = if_pointer->next;
        }

    } /* end of while loop*/

    /* Reinit system role base on restart object */
    if (restart_object & DHCP_TYPE_RESTART_CLIENT)
    {
        /* Reset Client count to 0 */

        dhcp_om_lcb.system_role &= ~DHCP_TYPE_BIND_CLIENT;
        dhcp_om_lcb.system_client_count = 0;
    }
    else if (restart_object & DHCP_TYPE_RESTART_RELAY)
    {
        /* Reset Relay count to 0 */
        dhcp_om_lcb.system_role &= ~DHCP_TYPE_BIND_RELAY;
        dhcp_om_lcb.system_relay_count = 0;
    }
    else
    {
        /* restart server */
        /* Comment out reason: Due to server pool configuration prior to discover_interface
        **	please refer to DHCP_MGR_ReactiveProcessing. If we clear system role here,
        **	this will cause DHCP Server not working in the system.
        */
        //dhcp_om_lcb.system_role ^= DHCP_TYPE_BIND_SERVER;
    }



}	/*	end of DHCP_OM_ReInit	*/

/* FUNCTION	NAME : DHCP_OM_ReInitAllDatabases
 * PURPOSE:
 *		Reinitialize DHCP_OM software components when DHCP must rebuild its working
 *		OM including server OM.
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
 *		1. No resource allocation in this function, just reset data structure to initial
 *		   value and following function, discover_interface, will fill out the proper data.
 */
void DHCP_OM_ReInitAllDatabases()
{
    /* 1. Clear DHCP Server OM */
#if (SYS_CPNT_DHCP_SERVER)
    DHCP_MEMORY_ReInit();
#endif
    /* 2. Clear all dhcp's interfaces */
    DHCP_OM_ReInit(DHCP_TYPE_RESTART_CLIENT);
    DHCP_OM_ReInit(DHCP_TYPE_RESTART_SERVER);
    DHCP_OM_ReInit(DHCP_TYPE_RESTART_RELAY);

    interfaces = NULL;

    /* 3. Put init value */
    dhcp_om_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_DHCP;
    dhcp_om_lcb.system_role = 0;
    dhcp_om_lcb.system_relay_count = 0;
    dhcp_om_lcb.system_client_count = 0;

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    option_status = 0;
    option_policy = 0;
    option_rid_mode = 0;
    memset(option_rid_value, 0, SYS_ADPT_MAX_LENGTH_OF_RID + 1);
#endif

} /* end of DHCP_OM_ReInitAllDatabases */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_OM_SetUCLeaseData
 * ------------------------------------------------------------------------
 * PURPOSE  : Set UC lease data
 * INPUT    : uc_data       --  UC data
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_OM_SetUCLeaseData(DHCP_OM_UC_LEASE_DATA_T *uc_data)
{
    UI32_T original_priority;

    if(!uc_data||!dhcp_om_uc_data)
        return FALSE;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    memcpy(&(dhcp_om_uc_data->lease_data), uc_data, sizeof(DHCP_OM_UC_LEASE_DATA_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_OM_GetUCLeaseData
 * ------------------------------------------------------------------------
 * PURPOSE  : Get UC lease data
 * INPUT    : uc_data       --  UC data
 * OUTPUT   : uc_data       --  UC data
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_OM_GetUCLeaseData(DHCP_OM_UC_LEASE_DATA_T *uc_data)
{
    UI32_T original_priority;

    if(!uc_data||!dhcp_om_uc_data)
        return FALSE;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    memcpy(uc_data, &(dhcp_om_uc_data->lease_data), sizeof(DHCP_OM_UC_LEASE_DATA_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_OM_UpdateUCData
 * ------------------------------------------------------------------------
 * PURPOSE  : This function notify DHCP to update UC data when UC size is changed
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DHCP_OM_UpdateUCData(void)
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);

    dhcp_om_uc_data = UC_MGR_Allocate(UC_MGR_DHCP_INFO_INDEX, sizeof(DHCP_OM_UC_DATA_T), 4);
    if(!dhcp_om_uc_data)
        goto exit;
#if 0    /* Temp marked off for compile error */
    /* copy UC data from snapshot to current UC memory
     */
    UC_MGR_GetOldUCData(UC_MGR_DHCP_INFO_INDEX, sizeof(DHCP_OM_UC_DATA_T), dhcp_om_uc_data);
#endif
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return;
}

/* FUNCTION	NAME : DHCP_OM_FreeNextInterface
 * PURPOSE:
 *		Free the pointer of next interface and un-link it
 *
 * INPUT:
 *		*if_pointer - pointer of the head of the interface
 * 		*free_if 	- pointer of next interface that will be free.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Success
 *		FALSE 	-- 	Fail
 *
 * NOTES:
 *
 */
static void DHCP_OM_FreeNextInterface(struct interface_info *if_pointer, struct interface_info *free_if)
{
    if_pointer->next = if_pointer->next->next;
    free_if->next = NULL;
    DHCP_OM_FreeCurrentInterface(free_if);
    free_if = NULL;
}

/* FUNCTION	NAME : DHCP_OM_FreeCurrentInterface
 * PURPOSE:
 *		Free the pointer of current interface
 *
 * INPUT:
 *		*if_pointer - pointer of the interface
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Success
 *		FALSE 	-- 	Fail
 *
 * NOTES:
 *
 */
static BOOL_T DHCP_OM_FreeCurrentInterface(struct interface_info *if_pointer)
{
    /* 1. Free other interface infos such as client_state, client_lease */
    if (if_pointer->client != NULL)
    {
        if (DHCP_OM_FreeClientState(if_pointer->client))
            if_pointer->client = NULL;
        else
        {
            // syslog free client state error
            return FALSE;
        }
    }
#if 0
    /* 2. Special condition: the interface's role was Relay or Server
    ** but user disables it now.
    ** SERVER -- if shared_network is not null which means it has been
    **		  -- server before, call Reinit Momory.c
    */

    if ((if_pointer -> shared_network != NULL) &&
            (if_pointer -> role == DHCP_TYPE_BIND_CLIENT))
    {
        DHCP_ALGO_SetDispatchFlag(FALSE);
        DHCP_ALGO_RemoveProtocol();
#if	 (SYS_CPNT_DHCP_SERVER == TRUE)
        DHCP_MEMORY_ReInit();
#endif
    }
#endif
    DHCP_TIME_Cancel_interface_timeout(if_pointer);
    if_pointer -> shared_network = NULL;

    dhcp_free(if_pointer);
    if_pointer = NULL;
    return TRUE;

}

#if 0
/* FUNCTION	NAME : DHCP_OM_FreeSharedNetwork
 * PURPOSE:
 *		Free the pointer of shared network
 *
 * INPUT:
 *		share -- the pointer to shared_network
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Success
 *		FALSE 	-- 	Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeSharedNetwork(struct shared_network *share)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    BOOL_T ret = TRUE;

    /* BODY */
    if (share == NULL)
        return ret;

    if (share->next != NULL)
    {
        if (DHCP_OM_FreeSharedNetwork(share->next))
            share->next = NULL;
        else
            ret = FALSE;
    }

    /* free subnet */
    DHCP_OM_FreeSubnets(share -> subnets);
    dhcp_free(share);
    share = NULL;

    return ret;

} /* end of DHCP_OM_FreeSharedNetwork */

/* FUNCTION	NAME : DHCP_OM_FreeSubnets
 * PURPOSE:
 *		Free the pointer of client state.
 *
 * INPUT:
 *		client -- client state
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Success
 *		FALSE 	-- 	Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeSubnets(struct subnet *subnet)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    BOOL_T ret = TRUE;

    /* BODY */

} /* end of DHCP_OM_FreeSubnets */
#endif

/* FUNCTION	NAME : DHCP_OM_FreeClientState
 * PURPOSE:
 *		Free the pointer of client state.
 *
 * INPUT:
 *		client -- client state
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Success
 *		FALSE 	-- 	Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeClientState(struct client_state *client)
{
    BOOL_T ret = TRUE;

    if (client == NULL)
        return ret;

    if (client->active != NULL)
    {
        if (DHCP_OM_FreeClientLease(client->active))
            client->active = NULL;
        else
            ret = FALSE;
    }

    if (client->new != NULL)
    {
        if (DHCP_OM_FreeClientLease(client->new))
            client->new = NULL;
        else
            ret = FALSE;
    }

    if (client->offered_leases != NULL)
    {
        if (DHCP_OM_FreeClientLease(client->offered_leases))
            client->offered_leases = NULL;
        else
            ret = FALSE;
    }

    if (client->leases != NULL)
    {
        if (DHCP_OM_FreeClientLease(client->leases))
            client->leases = NULL;
        else
            ret = FALSE;
    }

    if (client->alias != NULL)
    {
        if (DHCP_OM_FreeClientLease(client->alias))
            client->alias = NULL;
        else
            ret = FALSE;
    }

    if (client->config != NULL)
    {
        if (DHCP_OM_FreeClientConfig(client->config))
            client->config = NULL;
        else
            ret = FALSE;
    }

    dhcp_free(client);
    client = NULL;

    return ret;
} /* end of DHCP_OM_FreeClientState */

/* FUNCTION	NAME : DHCP_OM_FreeClientLease
 * PURPOSE:
 *		Free the pointer of client lease.
 *
 * INPUT:
 *		lease -- client lease
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- Success
 *		FALSE	-- Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeClientLease(struct client_lease *lease)
{
    BOOL_T ret = TRUE;

    if (lease == NULL)
        return ret;
    if (lease->next != NULL)
    {
        if (DHCP_OM_FreeClientLease(lease->next))
            lease->next = NULL;
        else
            ret = FALSE;
    }
    /* free all option in lease */
    DHCP_OM_FreeClientLeaseOptions(lease->options);

    if (lease -> server_name)
        dhcp_free(lease -> server_name);
    if (lease -> filename)
        dhcp_free(lease -> filename);

    dhcp_free(lease);
    lease = NULL;

    return ret;

} /* end of DHCP_OM_FreeClientLease */

/*
 *	Free option field in lease
 */
static void DHCP_OM_FreeClientLeaseOptions(struct option_data *options)
{
    int		i;
    /* Free the data associated with the options. */
    for (i = 0; i < 256; i++)
    {
        if (options[i].len && options[i].data)
            dhcp_free(options [i].data);
    }
    return;
}

/* FUNCTION	NAME : DHCP_OM_FreeClientConfig
 * PURPOSE:
 *		Free the pointer of client config.
 *
 * INPUT:
 *		lease -- client lease
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- Success
 *		FALSE	-- Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeClientConfig(struct client_config *config)
{
    BOOL_T ret = TRUE;

    if (config == NULL)
        return ret;
    if (config->reject_list != NULL)
    {
        if (DHCP_OM_FreeRejectList(config->reject_list))
            config->reject_list = NULL;
        else
            ret = FALSE;
    }
    dhcp_free(config);
    config = NULL;

    return ret;

} /* end of DHCP_OM_FreeClientConfig */

/* FUNCTION	NAME : DHCP_OM_FreeRejectList
 * PURPOSE:
 *		Free the pointer of reject list.
 *
 * INPUT:
 *		reject_list -- a list of IPs to be rejected
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- Success
 *		FALSE	-- Fail
 *
 * NOTES:
 *
 */
BOOL_T DHCP_OM_FreeRejectList(struct iaddrlist *reject_list)
{
    BOOL_T ret = TRUE;

    if (reject_list == NULL)
        return ret;
    if (reject_list->next != NULL)
    {
        if (DHCP_OM_FreeRejectList(reject_list->next))
            reject_list->next = NULL;
        else
            ret = FALSE;
    }
    dhcp_free(reject_list);
    reject_list = NULL;

    return ret;

} /* end of DHCP_OM_FreeClientConfig */


/* FUNCTION	NAME : DHCP_OM_CreateIf
 * PURPOSE:
 *		Create a interface working space and link to interface list.
 *
 * INPUT:
 *		ifIndex	-- interface ifIndex, possibly is vid_ifIndex.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- 	OK, the interface working space is created.
 *		FALSE	--	no more space for this interface management.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_CreateIf(UI32_T ifIndex)
{

    struct interface_info	*if_pointer;

    /*	Allocate working space	*/
    if_pointer = (struct interface_info	*) dhcp_malloc(sizeof(struct interface_info));
    if (NULL == if_pointer)
    {
        /*	log msg : could not create interface working space	*/
        return	FALSE;
    }

    DHCP_BD(DATABASE, "ifindex[%lu]", (unsigned long)ifIndex);

    /*	Clear working space 	*/
    memset((char*)if_pointer, 0, sizeof(struct interface_info));

    /*	Set interface associated ifIndex	*/
    if_pointer->vid_ifIndex	= ifIndex;
    if_pointer->client_port	= DHCP_WA_DEFAULT_CLIENT_PORT;
    if_pointer->server_port	= DHCP_WA_DEFAULT_SERVER_PORT;

#if (SYS_CPNT_DHCP_CLIENT_CLASSID == TRUE)
    {
        /* begin 2008-10, Joseph, default: PRODUCT MANUFACTURER */
        UI32_T id_len = 0;
        id_len = strlen(SYS_ADPT_PRODUCT_MANUFACTURER);

        if_pointer->classid.vendor_mode = DHCP_MGR_CLASSID_TEXT;
        if_pointer->classid.vendor_len = id_len;
        if (id_len == 0)
        {
            memset(if_pointer->classid.vendor_buf, 0, DHCP_MGR_CLASSID_BUF_MAX_SIZE);
        }
        else
        {
            memcpy(if_pointer->classid.vendor_buf, SYS_ADPT_PRODUCT_MANUFACTURER, id_len);
        }
        if_pointer->classid.vendor_buf[id_len] = '\0';
    }
#else
    if_pointer->classid.vendor_len = 0;
    memset(if_pointer->classid.vendor_buf, 0, DHCP_MGR_CLASSID_BUF_MAX_SIZE);
#endif
    /*	Link to interface list, interfaces, the name used in DHCP/IAD */
    if_pointer->next = interfaces;
    interfaces = if_pointer;

    return TRUE;

}	/*	end of DHCP_OM_CreateIf	*/


BOOL_T DHCP_OM_DestroyIf(UI32_T ifIndex)
{
    struct interface_info *if_pointer, *if_pointer_prev = NULL;

    if (NULL == interfaces)
    {
        return FALSE;
    }

    DHCP_BD(DATABASE, "ifindex[%lu]", (unsigned long)ifIndex);

    /* remove interface's role
     */
    DHCP_OM_SetIfBindingRole(ifIndex, DHCP_TYPE_BIND_NONE);

    if (interfaces->vid_ifIndex == ifIndex)
    {
        if_pointer = interfaces;
        interfaces = interfaces->next;
        DHCP_OM_FreeCurrentInterface(if_pointer);
        return TRUE;
    }

    if_pointer = interfaces;
    while (if_pointer)
    {
        if (if_pointer->vid_ifIndex == ifIndex)
        {
            if_pointer_prev->next = if_pointer ->next;
            DHCP_OM_FreeCurrentInterface(if_pointer);
            return TRUE;
        }
        if_pointer_prev = if_pointer;
        if_pointer = if_pointer->next;
    }
    return FALSE;
}



/* FUNCTION	NAME : DHCP_OM_SetIfHwAddress
 * PURPOSE:
 *		set hardware address info to interface info
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		hw_address  -- hardware address
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_SetIfHwAddress(UI32_T vid_ifIndex, struct hardware hw_address)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
        if_pointer->hw_address = hw_address;

    return;
}


/* FUNCTION	NAME : DHCP_OM_GetIfVlanByMac
 * PURPOSE:
 *
 *
 * INPUT:
 *		mac -- the interface's mac.
 *
 * OUTPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_GetIfVlanByMac(UI8_T *mac, UI32_T *vid_ifIndex)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByMac(mac);
    if (if_pointer != NULL)
        *vid_ifIndex = if_pointer->vid_ifIndex;
    else
        *vid_ifIndex = 0;

    return;
}	/*	end of DHCP_OM_GetIfVlanByMac	*/



/* FUNCTION	NAME : DHCP_OM_SetIfRbuf
 * PURPOSE:
 *		set read file descriptor info to interface info
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		rfdesc  	-- read file descriptor
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_SetIfRbuf(UI32_T vid_ifIndex, int rfdesc)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
        if_pointer->rfdesc = rfdesc;

    return;
}

/* FUNCTION	NAME : DHCP_OM_SetIfClientState
 * PURPOSE:
 *		set client_state info to interface info
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		client  	-- client_state struct
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_SetIfClientState(UI32_T vid_ifIndex, struct client_state *client)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
        if_pointer->client = client;

    return;
}


/* FUNCTION	NAME : DHCP_OM_SetIfPort
 * PURPOSE:
 *		Set client and server port of DHCP associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		client_port -- client-port associated with this interface.
 *		server_port	-- server-port associated with this interface.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. Do not check the interface, interface should be verified in MGR.
 */
void DHCP_OM_SetIfPort(UI32_T vid_ifIndex, UI32_T client_port, UI32_T server_port)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->client_port = client_port;
        if_pointer->server_port = server_port;
    }

    return;
}	/*	end of DHCP_OM_SetIfPort	*/

/* FUNCTION	NAME : DHCP_OM_SetIfBindingRole
 * PURPOSE:
 *		Define interface role in DHCP.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | server | relay }
 *						DHCP_TYPE_BIND_CLIENT
 *						DHCP_TYPE_BIND_RELAY
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 *		2. Do not check the interface, interface should be verified in MGR.
 */
void DHCP_OM_SetIfBindingRole(UI32_T vid_ifIndex, UI32_T role)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if(if_pointer != NULL)
    {
        if(if_pointer->role != role)
        {
            /* Remove old system role
             */
            if(if_pointer->role & DHCP_TYPE_BIND_RELAY)
            {
                DHCP_OM_RemoveSystemRole(DHCP_TYPE_BIND_RELAY);
            }

            if(if_pointer->role & DHCP_TYPE_BIND_CLIENT)
            {
                DHCP_OM_RemoveSystemRole(DHCP_TYPE_BIND_CLIENT);
            }

            if_pointer->role = role;

            /* Set system role
             */
            if (role == DHCP_TYPE_BIND_RELAY)
            {
                DHCP_OM_SetSystemRole(DHCP_TYPE_BIND_RELAY);
            }

            if (role == DHCP_TYPE_BIND_CLIENT)
            {
                DHCP_OM_SetSystemRole(DHCP_TYPE_BIND_CLIENT);
            }

            if (role == DHCP_TYPE_BIND_SERVER)
            {
                DHCP_OM_SetSystemRole(DHCP_TYPE_BIND_SERVER);
            }
        }
    }

    return;
}	/*	end of DHCP_OM_SetIfBindingRole	*/

/* FUNCTION	NAME : DHCP_OM_SetIfClientId
 * PURPOSE:
 *		Set cid associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		cid			-- the structure of CID
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_SetIfClientId(UI32_T vid_ifIndex, DHCP_TYPE_ClientId_T cid)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->cid = cid;
    }

    return;
}	/* end of DHCP_OM_SetIfClientId */

/* FUNCTION	NAME : DHCP_OM_GetIfClientId
 * PURPOSE:
 *		Get cid associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		cid_p		-- the address of the CID structure
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK -- Success
 *		DHCP_OM_INTERFACE_NOT_EXISTED -- the specified interface not existed in DHCP_OM table
 *
 * NOTES:
 *
 */
UI32_T DHCP_OM_GetIfClientId(UI32_T vid_ifIndex, DHCP_TYPE_ClientId_T *cid_p)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        memcpy(cid_p, &if_pointer->cid, sizeof(DHCP_MGR_ClientId_T));
        return DHCP_OM_OK;
    }
    else
        return DHCP_OM_INTERFACE_NOT_EXISTED;

}	/* end of DHCP_OM_SeDHCP_OM_GetIfClientIdtIfClientId */

/* FUNCTION	NAME : DHCP_OM_SetIfVendorClassId
 * PURPOSE:
 *		Set class id associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		classid		-- the structure of CLASS ID
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_OM_SetIfVendorClassId(UI32_T vid_ifIndex, DHCP_TYPE_Vendor_T classid)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->classid = classid;
    }

    return;
}	/* end of DHCP_OM_SetIfVendorClassId */


/* FUNCTION	NAME : DHCP_OM_SetIfRelayServerAddress
 * PURPOSE:
 *		Add interface relay server address associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		ip_list  -- server ip address list
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully configure
 *		FALSE	-- Fail to set server ip to interface
 *
 * NOTES:
 *		1. The maximum numbers of server ip can add-in is 5. If there are 5
 *		server IPs in interface info list, it would return FALSE to user
 *		while user attends to add one more server IP address.
 *
 */
BOOL_T DHCP_OM_SetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T *relay_server_list)
{

    struct interface_info	*if_pointer;
    int i;
    int j = 0;

    /* BODY */
    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        /* clear all relay server address then set them */
        for (i = 0; i < MAX_RELAY_SERVER; i++)
        {
            if_pointer->relay_server_list[i] = 0x0;
        }

        for (i = 0; i < MAX_RELAY_SERVER; i++)
        {
            if (relay_server_list[i] == 0)
                continue;

            if_pointer->relay_server_list[j] = relay_server_list[i];
            j++;
        }

        /* if j = 0 which means all relay server address are 0,
         * this interface binding role should change from relay
         * to client automatically.
        */
        if (j == 0)
        {
            DHCP_OM_SetIfBindingRole(if_pointer->vid_ifIndex, DHCP_TYPE_BIND_CLIENT);
        }
    }
    else
        return FALSE;

    return TRUE;

}/* end of DHCP_OM_SetIfRelayServerAddress */



/* FUNCTION	NAME : DHCP_OM_SetClientIfConfig
 * PURPOSE:
 *		Set assigned-ip, server-ip, and gateway-ip associated with
 *		interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		if_ip		--	interface ip.
 *		server_ip	--	server ip
 *		gate_ip		--	gateway ip.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. If any ip is not specified, use 0.0.0.0 as specified
 *		2. Do not check the interface, interface should be verified in MGR.
 */
void DHCP_OM_SetClientIfConfig(UI32_T vid_ifIndex, UI32_T if_ip, UI32_T server_ip, UI32_T gate_ip)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->primary_address = if_ip;
        if_pointer->server_ip		= server_ip;
        if_pointer->gateway_ip		= gate_ip;
    }

    return;
}	/*	end of DHCP_OM_SetClientIfConfig	*/


/* FUNCTION	NAME : DHCP_OM_GetIfPort
 * PURPOSE:
 *		Retrieve client and server port of interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		client_port -- client-port associated with this interface.
 *		server_port	-- server-port associated with this interface.
 *
 * RETURN:
 *		TRUE	--	Got interface information
 *		FALSE	--	The interface does not exist.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_GetIfPort(UI32_T vid_ifIndex, UI32_T *client_port, UI32_T *server_port)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        *client_port = if_pointer->client_port;
        *server_port = if_pointer->server_port;
        return	TRUE;
    }

    return FALSE;
}	/*	end of DHCP_OM_GetIfPort	*/

/* FUNCTION	NAME : DHCP_OM_SetIfIp
 * PURPOSE:
 *		Set interface ip address
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		ip_address	-- ip address to be bond to the interface
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Successfully set interface ip
 *		FALSE	--	Fail to set ip
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_SetIfIp(UI32_T vid_ifIndex, UI32_T ip_address)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->primary_address = ip_address;
        return	TRUE;
    }

    return FALSE;
}

/* FUNCTION	NAME : DHCP_OM_IsIfGotIp
 * PURPOSE:
 *		Retrieve client and server port of interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	Yes, the interface bound ip, not 0.0.0.0
 *		FALSE	--	No, the interface not configured yet.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_IsIfGotIp(UI32_T vid_ifIndex)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if (if_pointer->primary_address	==	0)
            return	FALSE;

        return	TRUE;
    }

    return FALSE;
}	/*	end of DHCP_OM_IsIfGotIp	*/


/* FUNCTION	NAME : DHCP_OM_FindInterfaceByVidIfIndex
 * PURPOSE:
 *		Search interface list to find out which interface associated
 *		with this vid_ifIndex.
 *
 * INPUT:
 *		vid_ifIndex -- the vlan interface to be searched.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL	--	no the interface_info.
 *		others	--	Pointer point to the interface.
 *
 * NOTES:
 *		1. Currently, switch, only one interface, so if can't find in the interface,
 *		   just return NULL.
 */

struct interface_info *DHCP_OM_FindInterfaceByVidIfIndex(UI32_T vid_ifIndex)
{

    struct interface_info	*if_pointer;

    if_pointer = interfaces;
    /*	Check interface list, is there a interface bound on the vlan ?
     *	if found, return the interface
     *	else	return	NULL
     */
    while (if_pointer != NULL)
    {
        if (if_pointer->vid_ifIndex != vid_ifIndex)
            if_pointer = if_pointer->next;
        else
            break;
    }
    if (if_pointer != NULL)
        return	if_pointer;
    else
        return NULL;
}	/*	end of DHCP_OM_FindInterfaceByVidIfIndex	*/


/* FUNCTION	NAME : DHCP_OM_GetNextInterface
 * PURPOSE:
 *		Get next interface in interface-list.
 *
 * INPUT:
 *		if_ptr_addr	--	Pointer point to interface which is current interface..
 *						NULL -- no current interface, want to get first interface.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL	--	No more next interface in interface list.
 *		others	--	Pointer point to the interface.
 *
 * NOTES:
 *		None.
 */
struct interface_info *DHCP_OM_GetNextInterface(struct interface_info *if_ptr_addr)
{

    if (if_ptr_addr == NULL)
        return	interfaces;
    else
    {
        return if_ptr_addr->next;
    }
}	/*	end of DHCP_OM_GetNextInterface	*/

BOOL_T DHCP_OM_GetClientDefaultGateway(UI8_T gateway[4])
{
    UI32_T zeroaddr = 0;

    if (memcmp(&zeroaddr, dhcp_client_gateway, sizeof(UI32_T)) == 0)
        return FALSE;

    memcpy(gateway, dhcp_client_gateway, sizeof(UI8_T)*4);

    DHCP_BD(DATABASE, "Gateway[%u.%u.%u.%u]", L_INET_EXPAND_IP(dhcp_client_gateway));

    return TRUE;
}
BOOL_T DHCP_OM_SetClientDefaultGateway(UI8_T gateway[4])
{
    memcpy(dhcp_client_gateway, gateway, sizeof(dhcp_client_gateway));
    return TRUE;
}
/* FUNCTION	NAME : DHCP_OM_GetPreviousInterface
 * PURPOSE:
 *		Get previous interface in interface-list.
 *
 * INPUT:
 *		if_ptr_addr	--	Pointer point to interface which is current interface..
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL	--	No more previous interface in interface list.
 *		others	--	Pointer point to the interface.
 *
 * NOTES:
 *		None.
 */

static struct interface_info *DHCP_OM_GetPreviousInterface(struct interface_info *if_ptr_addr)
{

    struct interface_info *tmp_ptr, *if_pointer;

    tmp_ptr = NULL;
    if_pointer = interfaces;

    if (if_ptr_addr == NULL)
        return	NULL;

    while (if_pointer != NULL)
    {
        if (if_pointer->vid_ifIndex != if_ptr_addr->vid_ifIndex)
        {
            tmp_ptr = if_pointer;
            if_pointer = if_pointer->next;
        }
        else
            break;
    }
    if (if_pointer != NULL)
    {
        return tmp_ptr;
    }
    else
        return NULL;


}	/*	end of DHCP_OM_GetNextInterface	*/


/* FUNCTION NAME : DHCP_OM_EnableDhcp
 * PURPOSE:
 *      Enable DHCP protocol to configure interface and
 *      Disable Bootp if BOOTP protocol is enable.
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
 *      1. Default Bootp is disable and Dhcp is enable. (by Dell request)
 */
void DHCP_OM_EnableDhcp(void)
{
    dhcp_om_lcb.object_mode	= DHCP_TYPE_INTERFACE_MODE_DHCP;
}	/*	end of DHCP_OM_EnableDhcp	*/


/* FUNCTION NAME : DHCP_OM_EnableBootp
 * PURPOSE:
 *      Enable BOOTP protocol to configure interface and
 *      Disable Dhcp if DHCP protocol is enable.
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
 *      1. Default Bootp is disable and Dhcp is enable. (by Dell request)
 */
void DHCP_OM_EnableBootp(void)
{
    dhcp_om_lcb.object_mode	= DHCP_TYPE_INTERFACE_MODE_BOOTP;
}	/*	end of DHCP_OM_EnableBootp	*/



/* FUNCTION NAME : DHCP_OM_DisableDhcp
 * PURPOSE:
 *      Disable DHCP protocol to configure interface, the interfaces should be configured
 *      by user, not by DHCP protocol
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE	--	disable DHCP
 *		FALSE	--	previous setting is not DHCP, maybe BOOTP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable DHCP, the interface MUST be configured manually.
 */
BOOL_T DHCP_OM_DisableDhcp(void)
{
    if (dhcp_om_lcb.object_mode	!= DHCP_TYPE_INTERFACE_MODE_DHCP)
        return	FALSE;
    dhcp_om_lcb.object_mode	= DHCP_TYPE_INTERFACE_MODE_USER_DEFINE;
    return	TRUE;
}	/*	end of DHCP_OM_DisableDhcp	*/


/* FUNCTION NAME : DHCP_OM_DisableBootp
 * PURPOSE:
 *      Disable BOOTP protocol to configure interface, the interfaces should be configured
 *      by user, not by BOOTP protocol
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE	--	disable BOOTP
 *		FALSE	--	previous setting is not BOOTP, maybe DHCP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable BOOTP, the interface MUST be configured manually.
 */
BOOL_T DHCP_OM_DisableBootp(void)
{
    if (dhcp_om_lcb.object_mode	!= DHCP_TYPE_INTERFACE_MODE_BOOTP)
        return	FALSE;
    dhcp_om_lcb.object_mode	= DHCP_TYPE_INTERFACE_MODE_USER_DEFINE;
    return	TRUE;
}	/*	end of DHCP_OM_DisableBootp	*/

/* FUNCTION NAME : DHCP_OM_GetMode
 * PURPOSE:
 *      Get the current DHCP Mode (DHCP/BOOTP/User-defined)
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP Mode(DHCP/BOOTP/User-defined)
 *
 * NOTES:
 *
 */
UI32_T	DHCP_OM_GetMode()
{
    return dhcp_om_lcb.object_mode;
}

/* FUNCTION NAME : DHCP_OM_GetIfMode
 * PURPOSE:
 *      Get the current DHCP Mode for that specified vid_ifIndex(DHCP/BOOTP/User-defined)
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      DHCP Address Mode(DHCP/BOOTP/User-defined)
 *
 * RETURN:
 *      TRUE 	-- Successfully get the address_mode for that specified vid_ifIndex
 *		FALSE   -- Can't get the address_mode
 *
 * NOTES: 2002/2/7: Penny created for Layer 3 multi-interface case.
 *
 */
BOOL_T DHCP_OM_GetIfMode(UI32_T vid_ifIndex, UI32_T *address_mode)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        *address_mode = if_pointer->mode;
        return TRUE;
    }
    else
        return FALSE; /* can't get address_mode for that vid_ifIndex */
}

/* FUNCTION NAME : DHCP_OM_SetIfMode
 * PURPOSE:
 *      Set the current DHCP Mode for that specified vid_ifIndex(DHCP/BOOTP/User-defined)
 *
 * INPUT:
 *      vid_ifIndex		-- the interface to be defined.
 *		address_mode	-- DHCP Address Mode(DHCP/BOOTP/User-defined)
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE 	-- Successfully set the address_mode for that specified vid_ifIndex
 *		FALSE   -- Can't set the address_mode
 *
 * NOTES: 2002/2/7: Penny created for Layer 3 multi-interface case.
 *
 */
BOOL_T DHCP_OM_SetIfMode(UI32_T vid_ifIndex, UI32_T address_mode)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->mode = address_mode;
        return TRUE;
    }
    else
        return FALSE; /* can't get address_mode for that vid_ifIndex */
}

/* FUNCTION NAME : DHCP_OM_SetSystemRole
 * PURPOSE:
 *      Set system role (server / server + client / relay + client / relay / client )
 *
 * INPUT:
 *      role	-- the system to be set
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE 	-- Successfully set the system role
 *		FALSE   -- Can't set the system role
 *
 * NOTES:
 *		None.
 *
 */
BOOL_T DHCP_OM_SetSystemRole(UI32_T role)
{

#if (SYS_CPNT_DHCP_RELAY)
    int relay_count = 0;

    if (role == DHCP_TYPE_BIND_RELAY)
    {
        if (DHCP_OM_IsServerOn())
            return FALSE;

        relay_count = DHCP_WA_GetRelayServiceCount();
        if (relay_count == 0)
        {
            dhcp_om_lcb.system_relay_count = 0;
            dhcp_om_lcb.system_role &= ~role;
            return FALSE;
        }

        dhcp_om_lcb.system_relay_count++;
    }
#endif
#if (SYS_CPNT_DHCP_SERVER)
    if (role == DHCP_TYPE_BIND_SERVER)
    {
        if (dhcp_om_lcb.system_relay_count > 0)
            return FALSE;
    }
#endif
    if (role == DHCP_TYPE_BIND_CLIENT)
        dhcp_om_lcb.system_client_count++;

    dhcp_om_lcb.system_role |= role;

    return TRUE;

} /* end of DHCP_OM_SetSystemRole */

/* FUNCTION NAME : DHCP_OM_GetSystemRole
 * PURPOSE:
 *      Get system role (server / server + client / relay + client / relay / client )
 *
 * INPUT:
 *      role	-- the system to be retrieved
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE 	-- Successfully get the system role
 *		FALSE   -- Can't get the system role
 *
 * NOTES: None.
 *
 */
BOOL_T DHCP_OM_GetSystemRole(UI32_T *role)
{
    if (role == NULL)
        return FALSE;
    else
    {
        *role = dhcp_om_lcb.system_role;
        return TRUE;
    }

} /* end of DHCP_OM_GetSystemRole */

/* FUNCTION NAME : DHCP_OM_RemoveSystemRole
 * PURPOSE:
 *      Remove system role (server / server + client / relay + client / relay / client )
 *
 * INPUT:
 *      role	-- the system to be removed
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE 	-- Successfully remove the specified system role
 *		FALSE   -- Can't remove the system role
 *
 * NOTES: None.
 *
 */
BOOL_T DHCP_OM_RemoveSystemRole(UI32_T role)
{
    if (role == 0)
        return FALSE;

    if(role == DHCP_TYPE_BIND_CLIENT)
    {
        if(dhcp_om_lcb.system_client_count > 0)
        {
            dhcp_om_lcb.system_client_count--;
        }

        if(dhcp_om_lcb.system_client_count == 0)
        {
            dhcp_om_lcb.system_role &= ~DHCP_TYPE_BIND_CLIENT;
        }
    }
#if (SYS_CPNT_DHCP_RELAY == TRUE)
    if(role == DHCP_TYPE_BIND_RELAY)
    {
        if(dhcp_om_lcb.system_relay_count > 0)
        {
            dhcp_om_lcb.system_relay_count--;
        }

        if(dhcp_om_lcb.system_relay_count == 0)
        {
            dhcp_om_lcb.system_role &= ~DHCP_TYPE_BIND_RELAY;
        }
    }
#endif

#if (SYS_CPNT_DHCP_SERVER == TRUE)
    if(role == DHCP_TYPE_BIND_SERVER)
    {
        DHCP_OM_ResetServerIfRole();
    }
#endif

    return TRUE;

} /* end of DHCP_OM_RemoveSystemRole */

/* FUNCTION	NAME : DHCP_OM_IsServerOn
 * PURPOSE:
 *		Check current system is running DHCP Server or not.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	the system is running as a DHCP Server
 *		FALSE	--	the system is not running as a DHCP Server
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_IsServerOn()
{
    UI32_T role;

    DHCP_OM_GetSystemRole(&role);
    if (role & DHCP_TYPE_BIND_SERVER)
        return TRUE;
    else
        return FALSE;

} /* end of DHCP_OM_IsServerOn */

/* FUNCTION	NAME : DHCP_OM_IsRelayOn
 * PURPOSE:
 *		Check current system is running DHCP relay or not.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	the system is running as a DHCP relay
 *		FALSE	--	the system is not running as a DHCP relay
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_OM_IsRelayOn()
{
    UI32_T role;

    DHCP_OM_GetSystemRole(&role);
    if ((role & DHCP_TYPE_BIND_RELAY) || dhcp_om_lcb.system_relay_count > 0)
        return TRUE;
    else
        return FALSE;

} /* end of DHCP_OM_IsRelayOn */


#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82Status
 * PURPOSE:
 *     Set dhcp relay option 82 status
 *
 * INPUT:
 *      status     --  DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
 *
 * OUTPUT:
 *      none
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82Status(UI32_T status)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    option_status = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82Status
 * PURPOSE:
 *     Get dhcp relay option 82 status
 *
 * INPUT:
 *      status
 *
 * OUTPUT:
 *      status  -- DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_GetDhcpRelayOption82Status(UI32_T *status)
{
    UI32_T original_priority;

    if (status == NULL)
        return DHCP_OM_FAIL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    *status = option_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);

    return DHCP_OM_OK;
}


/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82Policy
 * PURPOSE:
 *     Set dhcp relay option 82 policy
 *
 * INPUT:
 *      policy     --  DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
 *
 * OUTPUT:
 *      none
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82Policy(UI32_T policy)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    option_policy = policy;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82Policy
 * PURPOSE:
 *     Get dhcp relay option 82 policy
 *
 * INPUT:
 *      policy
 *
 * OUTPUT:
 *      policy  -- DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_GetDhcpRelayOption82Policy(UI32_T *policy)
{
    UI32_T original_priority;

    if (policy == NULL)
        return DHCP_OM_FAIL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    *policy = option_policy;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}


/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82RidMode
 * PURPOSE:
 *     Set dhcp relay option 82 remote id mode
 *
 * INPUT:
 *      mode     --  DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
 *
 * OUTPUT:
 *      none
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82RidMode(UI32_T mode)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    option_rid_mode = mode;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82RidMode
 * PURPOSE:
 *     Get dhcp relay option 82 remote id mode
 *
 * INPUT:
 *      mode
 *
 * OUTPUT:
 *      mode  -- DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_GetDhcpRelayOption82RidMode(UI32_T *mode)
{
    UI32_T original_priority;

    if (mode == NULL)
        return DHCP_OM_FAIL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    *mode = option_rid_mode;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82RidValue
 * PURPOSE:
 *     Set dhcp relay option 82 remote id value
 *
 * INPUT:
 *      string     --  configured string
 *
 * OUTPUT:
 *      none
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82RidValue(UI8_T *string)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    memcpy(option_rid_value, string, SYS_ADPT_MAX_LENGTH_OF_RID + 1);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82RidValue
 * PURPOSE:
 *     Get dhcp relay option 82 remote id value
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      string     --  configured string
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_GetDhcpRelayOption82RidValue(UI8_T *string)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    memcpy(string, option_rid_value, SYS_ADPT_MAX_LENGTH_OF_RID + 1);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}


/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82Format
 * PURPOSE:
 *     Set dhcp relay option 82 subtype format
 *
 * INPUT:
 *      subtype_format
 *
 * OUTPUT:
 *      none
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82Format(BOOL_T subtype_format)
{
    UI32_T original_priority;


    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    option_subtype_format = subtype_format;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82Format
 * PURPOSE:
 *     Get dhcp relay option 82 subtype format
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      subtype_format
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_GetDhcpRelayOption82Format(BOOL_T *subtype_format)
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    *subtype_format = option_subtype_format;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
}

/* FUNCTION	NAME : DHCP_OM_AddRelayServerAddress
 * PURPOSE:
 *		Add global relay server address.
 *
 * INPUT:
 *		ip_list  -- server ip address list
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully configure
 *		DHCP_OM_FAIL	-- Fail to set server ip to interface
 *
 * NOTES:
 *		1. The maximum numbers of server ip can add-in is 5. If there are 5
 *		server IPs in interface info list, it would return FALSE to user
 *		while user attends to add one more server IP address.
 *
 */
UI32_T DHCP_OM_AddRelayServerAddress(UI32_T ip_list[])
{
    UI32_T original_priority;
    UI32_T i = 0;
    UI32_T j;

    DHCP_OM_DeleteGlobalRelayServerAddress();
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);

    /* Check for all 5 args are valid; if there is a '0'ip within 2 none '0' ip,
    	shift up the none ip and put the '0' ip to the last  */
    j = 0;
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
    {
        if (ip_list[i] != 0)
        {
            global_relay_server[j] = ip_list[i];
            j++;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;

} /* end of DHCP_OM_AddRelayServerAddress */

/* FUNCTION	NAME : DHCP_OM_DeleteGlobalRelayServerAddress
 * PURPOSE:
 *		Delete global relay server addresses.
 *
 * INPUT:
 *      None.

 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully delete
 *		DHCP_OM_FAIL	-- Fail to delete server ip
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_OM_DeleteGlobalRelayServerAddress()
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    global_relay_server[0] = 0x0;
    global_relay_server[1] = 0x0;
    global_relay_server[2] = 0x0;
    global_relay_server[3] = 0x0;
    global_relay_server[4] = 0x0;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;
} /* end of DHCP_OM_DeleteGlobalRelayServerAddress */

/* FUNCTION	NAME : DHCP_OM_GetRelayServerAddress

 * PURPOSE:
 *		Get global relay server addresses.
 *
 * INPUT:
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		relay_server -- the pointer to relay server
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully get
 *		DHCP_OM_FAIL	-- Fail to get server ip to interface
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_OM_GetRelayServerAddress(UI32_T *relay_server)
{
    UI32_T original_priority;

    if (relay_server == NULL)
        return DHCP_OM_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcp_om_sem_id);
    memcpy(relay_server, global_relay_server, (SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER * sizeof(UI32_T)));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcp_om_sem_id, original_priority);
    return DHCP_OM_OK;

} /* end of DHCP_OM_GetRelayServerAddress */

#endif



#if (SYS_CPNT_DHCP_SERVER == TRUE)
/* FUNCTION NAME : DHCP_OM_ResetServerIfRole
 * PURPOSE:
 *     Reset interface role back to client if interface's
 *	   shared_network is not null (server has been on).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *
 * RETURN:
 *
 *
 * NOTES: None.
 *
 */
static void DHCP_OM_ResetServerIfRole()
{

    struct interface_info	*if_pointer;

    if_pointer = interfaces;

    while (if_pointer != NULL)
    {
        if (if_pointer -> shared_network != NULL)
            if_pointer -> role = DHCP_TYPE_BIND_CLIENT;

        if_pointer = if_pointer -> next;
    }


} /* end of DHCP_OM_ResetServerIfRole */
#endif

/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
/* FUNCTION	NAME : DHCP_OM_FindInterfaceByMac
 * PURPOSE:
 *		Search interface list to find out which interface associated
 *		with this mac.
 *
 * INPUT:
 *		mac -- the mac of interface to be searched.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL	--	no the interface_info.
 *		others	--	Pointer point to the interface.
 *
 * NOTES:
 *		1. Currently, switch, only one interface, so if can't find in the interface,
 *		   just return NULL.
 */

static struct interface_info *DHCP_OM_FindInterfaceByMac(UI8_T *mac)
{

    struct interface_info	*if_pointer;

    if_pointer = interfaces;
    /*	Check interface list, is there a interface bound on the vlan ?
     *	if found, return the interface
     *	else	return	NULL
     */

    while (if_pointer != NULL)
    {
        if (strncmp((char*)if_pointer->hw_address.haddr, (char*)mac, 6))
            if_pointer = if_pointer->next;
        else
            break;
    }
    if (if_pointer != NULL)
        return	if_pointer;
    else
        return NULL;
}	/*	end of DHCP_OM_FindInterfaceByMac	*/



/*------------------------------------------------------------------------------
 * ROUTINE NAME : DHCP_OM_GetLcbInformation
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get OM LCB information
 * INPUT:
 *    om_lcb  --  input lcb data buffer
 * OUTPUT:
 *    om_lcb  --  output lcb data
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    This API is for backdoor use
 *------------------------------------------------------------------------------
 */
BOOL_T DHCP_OM_GetLcbInformation(DHCP_OM_LCB_T *om_lcb)
{
    memcpy(om_lcb, &dhcp_om_lcb, sizeof(DHCP_OM_LCB_T));
    return TRUE;
}


#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION	NAME : DHCP_OM_SetIfDhcpInform
 * PURPOSE:
 *		Set dhcp inform to specified interface
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		do_enable   -- enable dhcp inform or not
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. Do not check the interface, interface should be verified in MGR.
 */
void DHCP_OM_SetIfDhcpInform(UI32_T vid_ifIndex, BOOL_T do_enable)
{

    struct interface_info	*if_pointer;

    if_pointer = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (if_pointer != NULL)
    {
        if_pointer->dhcp_inform = do_enable;
    }

    return;
}	/*	end of DHCP_OM_SetIfPort	*/

#endif /* SYS_CPNT_DHCP_INFORM */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DHCP_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T DHCP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if (ipcmsg_p == NULL)
        return FALSE;

    msg_data_p = (DHCP_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch (cmd)
    {

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
        case DHCP_OM_IPCCMD_GET_OPTION82_STATUS:
            msg_data_p->type.result_ui32 = DHCP_OM_GetDhcpRelayOption82Status(
                                               &(msg_data_p->data.ui32_v));
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(ui32_v);
            break;

        case DHCP_OM_IPCCMD_GET_OPTION82_POLICY:
            msg_data_p->type.result_ui32 = DHCP_OM_GetDhcpRelayOption82Policy(
                                               &(msg_data_p->data.ui32_v));
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(ui32_v);
            break;

        case DHCP_OM_IPCCMD_GET_OPTION82_RID_MODE:
            msg_data_p->type.result_ui32 = DHCP_OM_GetDhcpRelayOption82RidMode(
                                               &(msg_data_p->data.ui32_v));
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(ui32_v);
            break;

        case DHCP_OM_IPCCMD_GET_OPTION82_RID_VALUE:
            msg_data_p->type.result_ui32 = DHCP_OM_GetDhcpRelayOption82RidValue(
                                               msg_data_p->data.DHCP_Relay_Option82_Rid_Value_data.rid_value);
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(DHCP_Relay_Option82_Rid_Value_data);
            break;


        case DHCP_OM_IPCCMD_GET_RELAY_SERVER_ADDRESS:
            msg_data_p->type.result_ui32 = DHCP_OM_GetRelayServerAddress(
                                               msg_data_p->data.DHCP_Relay_Option82_Relay_Server_data.relay_server);
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(DHCP_Relay_Option82_Relay_Server_data);
            break;

        case DHCP_OM_IPCCMD_GET_OPTION82_FORMAT:
            msg_data_p->type.result_ui32 = DHCP_OM_GetDhcpRelayOption82Format(
                                               &(msg_data_p->data.bool_v));
            ipcmsg_p->msg_size = DHCP_OM_GET_MSG_SIZE(bool_v);
            break;

#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
        case DHCP_OM_IPCCMD_GET_IF_RELAY_SERVER_ADDRESS:
        {
            BOOL_T bool_ret;
            bool_ret = DHCP_WA_GetIfRelayServerAddress(
                msg_data_p->data.DHCP_Relay_If_Relay_Server_data.vid_ifindex,
                msg_data_p->data.DHCP_Relay_If_Relay_Server_data.ip_array);

            if(bool_ret)
                msg_data_p->type.result_ui32 = DHCP_OM_OK;
            else
                msg_data_p->type.result_ui32 = DHCP_OM_FAIL;
        }
            break;
#endif
        default:
            msg_data_p->type.result_ui32 = 0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;

    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if (cmd < DHCP_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


