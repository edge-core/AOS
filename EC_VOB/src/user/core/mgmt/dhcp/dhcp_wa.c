/* Module Name: DHCP_WA.C
 * Purpose:
 *      DHCP_WA holds all CLI configuration information. It is useful when DHCP server function enable.
 *      We must take complex consideration when CLI directly configuring DHCP_OM, but with DHCP_WA,
 *      we can restart DHCP_OM management and take overall consideration.
 *      Now, DHCP_WA includes : (for client)
 *          Set interface assigned ip, server ip, gateway ip.
 *          Set the role of DHCP with interface.
 *
 * Notes:
 *      1. Each interface binging status, Bootp, DHCP, or User-defined, is kept in NETCFG,
 *         because it's interface, needs to know by whole system CSC.
 *      2. For interface binding with DHCP, we keeps the role, Client, Server, or Relay Agent, in DHCP.
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *  0.2 2002.05.19  --  Penny ,  Revised  for DHCP Server / Relay
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "dhcp_type.h"

#include "sys_type.h"
#include "dhcp_wa.h"
#include "dhcp_algo.h"
#include "dhcp_error_print.h"
#include "memory.h"


#include "sysfun.h"
#include "l_mpool.h"
#include "l_sort_lst.h"
#include "tree.h"

#include "eh_mgr.h"
#include "eh_type.h"

#include "sys_module.h"
#if (SYS_CPNT_DHCP_CLIENT_CLASSID == TRUE)  
#include "sys_pmgr.h"
#endif
#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#include "vlan_lib.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef struct DHCP_WA_LCB_S
{
    UI32_T              managed_if_count;       /*  DHCP managed if amount              */
    UI32_T              object_mode;            /*  DHCP/BOOTP/USER_DEFINE  */
    L_MPOOL_HANDLE      if_config_pool;         /*  available if_config block pool      */
    DHCP_WA_InterfaceDhcpInfo_T *if_list;       /*  interface list          */
    int relay_service_count;                    /*  The numbers of interface having
                                                **  the relay server configuration */

    UI32_T restart_object;                      /*  BIT_0: DHCP_TYPE_RESTART_NONE, ???
                                                    BIT_1:DHCP_TYPE_RESTART_CLIENT,
                                                    BIT_2:DHCP_TYPE_RESTART_SERVER
                                                    BIT_3:DHCP_TYPE_RESTART_RELAY */
}DHCP_WA_LCB_T;

typedef struct DHCP_WA_SORTED_POOL_S
{
    char   pool_name[DHCP_WA_MAX_POOL_NAME_LEN+1];
    DHCP_TYPE_PoolConfigEntry_T *pool_p;
}DHCP_WA_SORTED_POOL_T;



/* LOCAL SUBPROGRAM DECLARATIONS
 */
static DHCP_WA_InterfaceDhcpInfo_T* DHCP_WA_CreateIfDhcpIf (UI32_T new_ifIndex);
#if (SYS_CPNT_DHCP_SERVER == TRUE)
/*static BOOL_T DHCP_WA_DestroyIfDhcpInfo (UI32_T ifIndex);*/
static int DHCP_WA_IpExcludedCompare(void *node_entry, void *input_entry);
static int DHCP_WA_RangeSetCompare(void *node_entry, void *input_entry);
static int DHCP_WA_ExcludeRangeSetCompare(void *node_entry, void *input_entry);
static int DHCP_WA_PoolNameCompare(void *node_entry, void *input_entry);
#endif



/* STATIC VARIABLE DECLARATIONS
 */
static  DHCP_WA_LCB_T               dhcp_wa_lcb;
static  DHCP_TYPE_PoolConfigEntry_T   *pool;           /* global pool config */
static  DHCP_TYPE_PoolConfigEntry_T   *network_pool;   /* network pool sorted list */
#if (SYS_CPNT_DHCP_SERVER == TRUE)
static  L_SORT_LST_List_T           dhcp_ip_excluded_list, range_set_list, pool_list, active_pool_list;
#endif
static  int                         network_pool_num, host_pool_num;


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : DHCP_WA_Init
 * PURPOSE:
 *      Initialize DHCP_WA.
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
void DHCP_WA_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    /*  Initialize control block    */
    memset (&dhcp_wa_lcb, 0, sizeof(DHCP_WA_LCB_T));
    pool = NULL;
    network_pool = NULL;
    network_pool_num = 0;
    host_pool_num = 0;
    //host_pool = NULL;

    /*  Allocate config-block pool for future usage */
    if ((dhcp_wa_lcb.if_config_pool = L_MPOOL_CreateBlockPool(sizeof(DHCP_WA_InterfaceDhcpInfo_T), DHCP_WA_MAX_IF_NBR)) == 0)
    {
        /*  log to system : No more space */
    }
#if (SYS_CPNT_DHCP_SERVER == TRUE)
    /* create sort list for dhcp_ip_excluded_list */
    L_SORT_LST_Create(&dhcp_ip_excluded_list, DHCP_WA_MAX_IP_EXCLUDED_ELEMENTS,
                         sizeof(DHCP_WA_IpExcluded_T), DHCP_WA_IpExcludedCompare);

    /* create sort list for range set */
    L_SORT_LST_Create(&range_set_list,
                    DHCP_WA_MAX_RANGE_SET_ELEMENTS,
                    sizeof(DHCP_WA_IpExcluded_T),
                    DHCP_WA_ExcludeRangeSetCompare);

    /* create sort list for pool */
    L_SORT_LST_Create(&pool_list,
                    SYS_ADPT_MAX_NBR_OF_DHCP_POOL,
                    sizeof(DHCP_WA_SORTED_POOL_T),
                    DHCP_WA_PoolNameCompare);

    /* create active range list */                    
    L_SORT_LST_Create(&active_pool_list,
                    SYS_ADPT_MAX_NBR_OF_DHCP_POOL+DHCP_WA_MAX_IP_EXCLUDED_ELEMENTS,
                    sizeof(DHCP_WA_IpRange_T),
                    DHCP_WA_RangeSetCompare);   
#endif

#if (SYS_CPNT_DHCP_CLIENT == TRUE)
           dhcp_wa_lcb.restart_object = DHCP_TYPE_RESTART_CLIENT;
#endif

    dhcp_wa_lcb.relay_service_count = 0;

    return;
}   /*  end of DHCP_WA_Init */


/* FUNCTION NAME : DHCP_WA_ReInit
 * PURPOSE:
 *      Release resource allocated.
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
void DHCP_WA_ReInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr=dhcp_wa_lcb.if_list;
    DHCP_WA_InterfaceDhcpInfo_T *next;
#if (SYS_CPNT_DHCP_SERVER == TRUE)
    DHCP_TYPE_PoolConfigEntry_T   *tmp_pool;
#endif

    /* BODY */
//Timon    SYSFUN_NonPreempty();
    network_pool_num = 0;
    host_pool_num = 0;


    /* Free pool allocation */
#if (SYS_CPNT_DHCP_SERVER == TRUE)
    tmp_pool = pool;
    while (tmp_pool)
    {
        if (DHCP_WA_DeletePool(tmp_pool))
            tmp_pool = pool;
        else
            break;
    }

    pool = NULL;
    network_pool = NULL;

    L_SORT_LST_Delete_All(&dhcp_ip_excluded_list);
    L_SORT_LST_Delete_All(&range_set_list);
    L_SORT_LST_Delete_All(&pool_list);
    L_SORT_LST_Delete_All(&active_pool_list );
#endif
    while (ptr != NULL)
    {
        next = ptr->next;
        L_MPOOL_FreeBlock(dhcp_wa_lcb.if_config_pool, ptr);
        ptr = next;
        dhcp_wa_lcb.managed_if_count--;
    }
    dhcp_wa_lcb.relay_service_count = 0;
    dhcp_wa_lcb.restart_object = DHCP_TYPE_RESTART_CLIENT;
    dhcp_wa_lcb.if_list = NULL;

//Timon    SYSFUN_Preempty();
    return;
}   /*  end of DHCP_WA_ReInit   */


/* FUNCTION NAME : DHCP_WA_SetIfBindingRole
 * PURPOSE:
 *      Define interface role in DHCP.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      role        -- one of { client | relay }
 *                      DHCP_TYPE_INTERFACE_BIND_CLIENT
 *                      DHCP_TYPE_INTERFACE_BIND_SERVER
 *                      DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully set the interace-role.
 *      FALSE   -- no such interface.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 */
BOOL_T DHCP_WA_SetIfBindingRole(UI32_T vid_ifIndex,UI32_T role)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */

            EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "Fail to Set Interface Role");
            return  FALSE;
        }
    }
    ptr->if_binding_role = role;
    return  TRUE;
}   /*  end of DHCP_WA_SetIfBindingRole */

/* FUNCTION NAME : DHCP_WA_DeleteIfBindingRole
 * PURPOSE:
 *      Remove interface role in DHCP. And set it to the default role.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      role        -- one of { client | relay }
 *                      DHCP_TYPE_INTERFACE_BIND_CLIENT
 *                      DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully set the interace-role.
 *      FALSE   -- no such interface.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 */
BOOL_T DHCP_WA_DeleteIfBindingRole(UI32_T vid_ifIndex,UI32_T role)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;

    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        /*  log message to system : no more space   */

        EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown,
                             EH_TYPE_MSG_FAILED_TO_DELETE,
                             SYSLOG_LEVEL_ERR,
                             "Fail to Delete Interface Role");
        return  FALSE;

    }
    ptr->if_binding_role = DHCP_WA_DEFAULT_ROLE;
    return  TRUE;
} /* DHCP_WA_DeleteIfBindingRole */

/* FUNCTION NAME : DHCP_WA_C_SetIfClientId
 * PURPOSE:
 *      Define interface CID for client.
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
 *      DHCP_WA_OK   -- Successful to set cid
 *      DHCP_WA_FAIL -- No more space in allocating memory
 *      DHCP_WA_CID_EXCEED_MAX_SIZE -- cid exceeds the max size
 *
 * NOTES:
 *
 */
UI32_T DHCP_WA_C_SetIfClientId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;

    /* BODY */
    /* Check if the length of the CID is larger than DHCP_TYPE_CID_BUF_MAX_SIZE(32) */
    if (id_len > DHCP_MGR_CID_BUF_MAX_SIZE)
        return DHCP_WA_CID_EXCEED_MAX_SIZE;
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /* log message to system : no more space */
            return  DHCP_WA_FAIL;
        }
    }
    ptr->cid.id_mode     = id_mode;
    ptr->cid.id_len      = id_len;
    if (id_len == 0)
    {
        memset(ptr->cid.id_buf, 0, MAXSIZE_dhcpcIfClientId);
    }
    else
        memcpy(ptr->cid.id_buf, id_buf, id_len);
    ptr->cid.id_buf[id_len]='\0';
    return  DHCP_WA_OK;
}   /* end of DHCP_WA_C_SetIfClientId */

/* FUNCTION NAME : DHCP_WA_C_GetIfClientId
 * PURPOSE:
 *      Retrieve the CID associated with interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      cid_p       -- the structure of CID
 *
 * RETURN:
 *      TRUE    -- successfully gotten
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_WA_C_GetIfClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    if (cid_p==NULL)
        return  FALSE;

    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
        return  FALSE;

    memcpy(cid_p, &ptr->cid, sizeof(DHCP_MGR_ClientId_T));
    return  TRUE;
}   /*  end of DHCP_WA_C_GetIfClientId  */

/* begin 2007-12, Joseph */
/* FUNCTION	NAME : DHCP_WA_C_SetIfVendorClassId
 * PURPOSE:
 *		Define interface CLASS ID for client.
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
 *		DHCP_WA_OK	 -- Successful to set class id
 *		DHCP_WA_FAIL -- No more space in allocating memory
 *		DHCP_WA_CLASSID_EXCEED_MAX_SIZE -- class id exceeds the max size
 *
 * NOTES:
 *		
 */
UI32_T DHCP_WA_C_SetIfVendorClassId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T	*ptr;
    
    /* BODY */
    /* Check if the length of the CLASS ID is larger than DHCP_MGR_CLASSID_BUF_MAX_SIZE */
    if (id_len > DHCP_MGR_CLASSID_BUF_MAX_SIZE)
    	return DHCP_WA_CLASSID_EXCEED_MAX_SIZE;
    	
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
    	if ((ptr = DHCP_WA_CreateIfDhcpIf (vid_ifIndex)) == NULL)
    	{
    		/* log message to system : no more space */
    		return	DHCP_WA_FAIL;
    	}
    }
     /* clear original setting */
    memset(&(ptr->classid), 0, sizeof(ptr->classid));
    ptr->classid.vendor_mode 	 = id_mode;
    ptr->classid.vendor_len 	 = id_len;

   memcpy(ptr->classid.vendor_buf, id_buf, id_len);

    return	DHCP_WA_OK;
}	/* end of DHCP_WA_C_SetIfVendorClassId */

/* FUNCTION	NAME : DHCP_WA_C_GetIfVendorClassId
 * PURPOSE:
 *		Retrieve the Class ID associated with interface.
 *
 * INPUT:
 *		vid_ifIndex     -- the interface to be defined.
 *
 * OUTPUT:
 *		class_id_p		-- the structure of Class ID 
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_WA_C_GetIfVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T	*ptr;
    /* BODY */
    if (class_id_p == NULL)
    	return	FALSE;
   
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    	return	FALSE;
    	
    memcpy(class_id_p, &ptr->classid, sizeof(DHCP_MGR_Vendor_T));
    return	TRUE;
}	/*	end of DHCP_WA_C_GetIfVendorClassId	*/

/* end 2007-12 */

#if (SYS_CPNT_DHCP_RELAY  == TRUE)
/* FUNCTION NAME : DHCP_WA_AddIfRelayServerAddress
 * PURPOSE:
 *      Add interface relay server address associated with interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      ip_list  -- server ip address list
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully configure
 *      FALSE   -- Fail to set server ip to interface
 *
 * NOTES:
 *      1. The maximum numbers of server ip can add-in is 5. If there are 5
 *      server IPs in interface info list, it would return FALSE to user
 *      while user attends to add one more server IP address.
 *
 */
BOOL_T DHCP_WA_AddIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T ip_list[])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_WA_InterfaceDhcpInfo_T    *if_ptr;
     int i = 0;
     UI8_T count=0;


    /* BODY */
    if_ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (if_ptr == NULL)
    {
        if ((if_ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }

    }

    /* clear relay server */
    memset(&(if_ptr->relay_server),0,sizeof(if_ptr->relay_server));
    
    /* Check for all 5 args are valid; if there is a '0'ip within 2 none '0' ip,
        shift up the none ip and put the '0' ip to the last  */
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
    {
        if (ip_list[i]!=0)
        {
             if_ptr->relay_server[count] = ip_list[i];
             count++;
        }
    }

    if(count!=0)
    {
        if(!(if_ptr->if_binding_role&DHCP_TYPE_BIND_RELAY))
        {
            if_ptr->if_binding_role |= DHCP_TYPE_BIND_RELAY;
            dhcp_wa_lcb.relay_service_count++;
#if (SYS_CPNT_DHCPSNP == TRUE)
            {
                UI32_T vid;
                VLAN_IFINDEX_CONVERTTO_VID(vid_ifIndex, vid);
                DHCPSNP_PMGR_SetL3RelayForwarding(vid, FALSE);
            }
#endif  //SYS_CPNT_DHCPSNP
        }
    }
    else
    {
        if(if_ptr->if_binding_role&DHCP_TYPE_BIND_RELAY)
        {
            if_ptr->if_binding_role &= ~DHCP_TYPE_BIND_RELAY;
            dhcp_wa_lcb.relay_service_count--;
#if (SYS_CPNT_DHCPSNP == TRUE)
            {
                UI32_T vid;
                VLAN_IFINDEX_CONVERTTO_VID(vid_ifIndex, vid);
                DHCPSNP_PMGR_SetL3RelayForwarding(vid, TRUE);
            }
#endif  //SYS_CPNT_DHCPSNP            
        }
    }

    return TRUE;

} /* end of DHCP_WA_AddIfRelayServerAddress */

/* FUNCTION NAME : DHCP_WA_GetRelayServiceCount
 * PURPOSE:
 *      Get all relay service count.
 *
 * INPUT:
 *      NONE.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      relay_service_count -- the numbers of relay configured in the system.
 *
 * NOTES:
 *      None.
 */
int DHCP_WA_GetRelayServiceCount(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */

    return dhcp_wa_lcb.relay_service_count;


} /* end of DHCP_WA_GetRelayServiceCount */

/* FUNCTION NAME : DHCP_WA_GetIfRelayServerAddress

 * PURPOSE:
 *      Get all interface relay server addresses associated with interface.
 *
 * INPUT:
 *      vid_ifIndex  -- the interface to be searched for relay server address.
 *      relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully get
 *      FALSE   -- Fail to get server ip to interface
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_WA_GetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T *relay_server)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_WA_InterfaceDhcpInfo_T    *if_ptr;

    /* BODY */
    if_ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (if_ptr == NULL)
    {
        /*
        relay_server = NULL;
        return FALSE;
        */

        if ((if_ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }

    }
    memcpy(relay_server, if_ptr->relay_server, (MAX_RELAY_SERVER* sizeof(UI32_T)));
    return TRUE;

} /* end of DHCP_WA_GetIfRelayServerAddress */

/* FUNCTION NAME : DHCP_WA_DeleteAllIfRelayServerAddress
 * PURPOSE:
 *      Delete all interface relay server addresses associated with interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully delete
 *      FALSE   -- Fail to delete server ip to interface
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_WA_DeleteAllIfRelayServerAddress(UI32_T vid_ifIndex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_WA_InterfaceDhcpInfo_T    *if_ptr;

    /* BODY */
    if_ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (if_ptr == NULL)
        return  TRUE;

    memset(if_ptr->relay_server, 0, sizeof(UI32_T)*MAX_RELAY_SERVER);

    if_ptr ->if_binding_role = DHCP_TYPE_BIND_CLIENT;
    if (dhcp_wa_lcb.relay_service_count > 0)
        dhcp_wa_lcb.relay_service_count--;

    return TRUE;
} /* end of DHCP_WA_DeleteAllIfRelayServerAddress */

#endif /* end of #if SYS_CPNT_DHCP_RELAY  == TRUE*/

/* FUNCTION NAME : DHCP_WA_C_SetIfGateway
 * PURPOSE:
 *      Save interface association setting.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      gate_iip    -- relay agent ip.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully saved
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfGateway (UI32_T vid_ifIndex, UI32_T gate_ip)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }
    }
    ptr->if_gateway = gate_ip;
    return  TRUE;
}   /*  end of DHCP_WA_C_SetIfGateway   */


/* FUNCTION NAME : DHCP_WA_C_SetIfServer
 * PURPOSE:
 *      Save interface association setting.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      svr_iip     -- dhcp server's ip.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully saved
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfServer(UI32_T vid_ifIndex, UI32_T svr_ip)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }
    }
    ptr->if_server_ip = svr_ip;
    return  TRUE;
}   /*  end of DHCP_WA_C_SetIfServer    */


/* FUNCTION NAME : DHCP_WA_C_SetIfIp
 * PURPOSE:
 *      Save interface association setting.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      if_ip       -- dhcp client's pre-assigned ip.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully saved
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfIp(UI32_T vid_ifIndex, UI32_T if_ip)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }
    }
    ptr->if_ip = if_ip;
    return  TRUE;
}   /*  end of DHCP_WA_C_SetIfIp    */


/* FUNCTION NAME : DHCP_WA_SetIfClientPort
 * PURPOSE:
 *      Save interface association setting.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      udp_port_no -- the client-port of dhcp.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully saved
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. As RFC specified, default client port is 68 if user not changed.
 *
 */
BOOL_T DHCP_WA_SetIfClientPort(UI32_T vid_ifIndex, UI32_T udp_port_no)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }
    }
    ptr->client_port = udp_port_no;
    return  TRUE;
}   /*  end of DHCP_WA_SetIfClientPort  */


/* FUNCTION NAME : DHCP_WA_SetIfServerPort
 * PURPOSE:
 *      Save interface association setting.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *      udp_port_no -- the server-port of dhcp.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully saved
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. As RFC specified, default client port is 67 if user not changed.
 */
BOOL_T DHCP_WA_SetIfServerPort(UI32_T vid_ifIndex, UI32_T udp_port_no)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        if ((ptr=DHCP_WA_CreateIfDhcpIf (vid_ifIndex))==NULL)
        {
            /*  log message to system : no more space   */
            return  FALSE;
        }
    }
    ptr->server_port = udp_port_no;
    return  TRUE;
}   /*  end of DHCP_WA_SetIfServerPort  */


/* FUNCTION NAME : DHCP_WA_GetIfPort
 * PURPOSE:
 *      Retrieve dhcp client and server port, udp port.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      client_port -- client-port associated with this interface.
 *      server_port -- server-port associated with this interface.
 *
 * RETURN:
 *      TRUE    -- successfully gotten
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. If not specified, use RFC definition (67,68)
 */
BOOL_T DHCP_WA_GetIfPort(UI32_T vid_ifIndex, UI32_T *client_port, UI32_T *server_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    if ((client_port==NULL)||(server_port==NULL))
        return  FALSE;
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
    {
        *client_port = DHCP_WA_DEFAULT_CLIENT_PORT;
        *server_port = DHCP_WA_DEFAULT_SERVER_PORT;
        return  FALSE;
    }
    *client_port = ptr->client_port;
    *server_port = ptr->server_port;
    return  TRUE;
}   /*  end of DHCP_WA_GetIfPort    */


/* FUNCTION NAME : DHCP_WA_GetIfConfig
 * PURPOSE:
 *      Retrieve assigned-ip, server-ip, and gateway-ip associated with
 *      interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      if_ip       --  interface ip.
 *      server_ip   --  server ip
 *      gate_ip     --  gateway ip.
 *
 * RETURN:
 *      TRUE    -- successfully gotten
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. If any ip is not specified, use 0.0.0.0 as specified
 */
BOOL_T DHCP_WA_GetIfConfig(UI32_T vid_ifIndex,DHCP_WA_InterfaceDhcpInfo_T* if_config_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T* temp_if_p;

    /* BODY */
    if (if_config_p==NULL)
    {
        return  FALSE;
    }
    temp_if_p = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == temp_if_p)
    {
        temp_if_p = DHCP_WA_CreateIfDhcpIf(vid_ifIndex);

        if (temp_if_p == NULL)
        {
            return FALSE;
        }
    }


    memcpy(if_config_p, temp_if_p, sizeof(DHCP_WA_InterfaceDhcpInfo_T));

    return  TRUE;
}   /*  end of DHCP_WA_GetIfConfig  */

/* FUNCTION NAME : DHCP_WA_GetNextIfConfig
 * PURPOSE:
 *      Retrieve assigned-ip, server-ip, and gateway-ip associated with
 *      interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      if_ip       --  interface ip.
 *      server_ip   --  server ip
 *      gate_ip     --  gateway ip.
 *
 * RETURN:
 *      TRUE    -- successfully gotten
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      1. If vid_ifIndex == 0, get the 1st if_config.
 */
BOOL_T DHCP_WA_GetNextIfConfig(UI32_T vid_ifIndex,DHCP_WA_InterfaceDhcpInfo_T* if_config_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;

    /* BODY */
    ptr = dhcp_wa_lcb.if_list;

    if (NULL == dhcp_wa_lcb.if_list)
    {
        return  FALSE;
    }

    if (vid_ifIndex == 0)
    {
        memcpy (if_config_p, ptr, sizeof(DHCP_WA_InterfaceDhcpInfo_T));
    }
    else
    {
        ptr = DHCP_WA_SearchIfInList(vid_ifIndex);

        if (ptr == NULL)
            return FALSE;
        else
        {
            //if_config_p = ptr->next;
            if (ptr->next != NULL)
                memcpy(if_config_p, ptr->next, sizeof(DHCP_WA_InterfaceDhcpInfo_T));
            else
            {
                if_config_p = NULL;
                return FALSE;
            }
        }
    }

    return TRUE;

}


/* FUNCTION NAME : DHCP_WA_GetIfBindingRole
 * PURPOSE:
 *      Retrieve the role associated with interface.
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *      role    -- binding role.
 *                  DHCP_TYPE_INTERFACE_BIND_CLIENT
 *                  DHCP_TYPE_INTERFACE_BIND_SERVER
 *                  DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * RETURN:
 *      TRUE    -- successfully gotten
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_WA_GetIfBindingRole(UI32_T vid_ifIndex, UI32_T *role)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    if (role==NULL)
    {
        *role = DHCP_WA_DEFAULT_ROLE;
        return  FALSE;
    }
    ptr = DHCP_WA_SearchIfInList(vid_ifIndex);
    if (NULL == ptr)
        return  FALSE;
    *role = ptr->if_binding_role;
    return  TRUE;
}   /*  end of DHCP_WA_GetIfBindingRole */


/* FUNCTION NAME : DHCP_WA_EnableDhcp
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
void    DHCP_WA_EnableDhcp (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    dhcp_wa_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_DHCP;
}   /*  end of DHCP_WA_EnableDhcp   */


/* FUNCTION NAME : DHCP_WA_EnableBootp
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
void    DHCP_WA_EnableBootp (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    dhcp_wa_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_BOOTP;
}   /*  end of DHCP_WA_EnableBootp  */



/* FUNCTION NAME : DHCP_WA_DisableDhcp
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
 *      TRUE    --  disable DHCP
 *      FALSE   --  previous setting is not DHCP, maybe BOOTP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable DHCP, the interface MUST be configured manually.
 *      2. These functions for DHCP/BOOTP disable/enable affecting whole CSC,
 *         not one interface. If DHCP and BOOTP are disable, even interface is
 *         indivial configured, still not activing auto-configure-interface.
 */
BOOL_T    DHCP_WA_DisableDhcp (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    if (dhcp_wa_lcb.object_mode != DHCP_TYPE_INTERFACE_MODE_DHCP)
        return  FALSE;
    dhcp_wa_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_USER_DEFINE;
    return  TRUE;
}   /*  end of DHCP_WA_DisableDhcp  */


/* FUNCTION NAME : DHCP_WA_DisableBootp
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
 *      TRUE    --  disable BOOTP
 *      FALSE   --  previous setting is not BOOTP, maybe DHCP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable BOOTP, the interface MUST be configured manually.
 *      2. These functions for DHCP/BOOTP disable/enable affecting whole CSC,
 *         not one interface. If DHCP and BOOTP are disable, even interface is
 *         indivial configured, still not activing auto-configure-interface.
 */
BOOL_T    DHCP_WA_DisableBootp (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    if (dhcp_wa_lcb.object_mode != DHCP_TYPE_INTERFACE_MODE_BOOTP)
        return  FALSE;
    dhcp_wa_lcb.object_mode = DHCP_TYPE_INTERFACE_MODE_USER_DEFINE;
    return  TRUE;
}   /*  end of DHCP_WA_DisableBootp */


/* FUNCTION NAME : DHCP_WA_GetObjectMode
 * PURPOSE:
 *      Retrieve object (DHCP) mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      One of { DHCP_TYPE_INTERFACE_MODE_USER_DEFINE | DHCP_TYPE_INTERFACE_MODE_BOOTP
 *      | DHCP_TYPE_INTERFACE_MODE_DHCP }
 *
 * NOTES:
 */
UI32_T DHCP_WA_GetObjectMode (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    return  dhcp_wa_lcb.object_mode;
}   /*  end of DHCP_WA_GetObjectMode    */

/* FUNCTION NAME : DHCP_WA_GetRestartObject
 * PURPOSE:
 *      Retrieve the object(s) that needed to restart
 *
 * INPUT:
 *      restart_object -- object needed to restart (none / server / client / relay)
 *      Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *      DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully retrieve
 *      FALSE   -- fail to get restart object
 *
 * NOTES:
 *      1. System default is running DHCP server. If no one config in WA yet, we
 *      only restart DHCP server.
 *
 */
BOOL_T DHCP_WA_GetRestartObject(UI32_T *restart_object)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    if (restart_object == 0)
        return FALSE;

    if (dhcp_wa_lcb.restart_object == 0)
        *restart_object = DHCP_TYPE_RESTART_NONE;
    else
        *restart_object = dhcp_wa_lcb.restart_object;

    return  TRUE;

}

/* FUNCTION NAME : DHCP_WA_SetRestartObject
 * PURPOSE:
 *      Retrieve the object(s) that needed to restart
 *
 * INPUT:
 *      restart_object -- object needed to restart (none/server / client / relay)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1.Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *      DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 */
BOOL_T DHCP_WA_SetRestartObject(UI32_T restart_object)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    if (restart_object == 0)
        return FALSE;

    dhcp_wa_lcb.restart_object |= restart_object;
    return TRUE;

}/* end of DHCP_WA_SetRestartObject*/

/* FUNCTION NAME : DHCP_WA_ClearRestartObject
 * PURPOSE:
 *      Clear the object that needed to restart
 *
 * INPUT:
 *      restart_object -- object needed to restart (none/server / client / relay)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *      DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 *
 * NOTES:
 *      1. This function will be called while during restart process; when one object
 *          has been restarted, it needs to clear out after that.
 */
void DHCP_WA_ClearRestartObject(UI32_T restart_object)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    dhcp_wa_lcb.restart_object ^= restart_object;

}/* end of DHCP_WA_ClearRestartObject*/

/* FUNCTION NAME : DHCP_WA_SearchIfInList
 * PURPOSE:
 *      Search this interface, check whether exist ?
 *
 * INPUT:
 *      ifIndex -- the interface to be searched.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NULL    --  the interface not in the list.
 *      others  --  the block associated with the interface in list.
 *
 * NOTES:
 *      None.
 */
DHCP_WA_InterfaceDhcpInfo_T* DHCP_WA_SearchIfInList(UI32_T ifIndex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *ptr;
    /* BODY */
    if (NULL == dhcp_wa_lcb.if_list)
    {
        /*
        ptr = (DHCP_WA_InterfaceDhcpInfo_T *) dhcp_malloc (sizeof(*ptr));
        memset(ptr, 0, sizeof(*ptr));
        dhcp_wa_lcb.if_list = ptr;
        */
        return  NULL;
    }
    ptr = dhcp_wa_lcb.if_list;

    while ((ptr != NULL)&&(ptr->ifIndex != ifIndex))
        ptr = ptr->next;

    return  ptr;
}   /*  end of DHCP_WA_SearchIfInList   */

/*********************
**** DHCP Server *****
*********************/
#if (SYS_CPNT_DHCP_SERVER == TRUE)
/* FUNCTION NAME : DHCP_WA_SetPoolConfigEntry
 * PURPOSE:
 *      Set entry to pool config. (Set by record)
 *
 * INPUT:
 *      pool -- pool config entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_WA_NO_SUCH_POOL_NAME
 *      DHCP_WA_FAIL --
 *
 * NOTES:
 *      Set by record function;
 */
#if 0
UI32_T DHCP_WA_SetPoolConfigEntry(DHCP_TYPE_PoolConfigEntry_T *pool)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool;
    /* BODY */

    /* 1. Check if pool created, if no, created */
    if (pool == NULL)
        return DHCP_WA_FAIL;

    if (!pool->pool_name)
        return DHCP_WA_FAIL;

    tmp_pool = DHCP_WA_FindPoolByPoolName(pool->pool_name);
    if (tmp_pool == NULL)
    {
        if (!DHCP_WA_CreatePool(pool->pool_name))
            return DHCP_WA_FAIL;
    }
    else
    {
        /* 2. Add entry to pool config table */
        /* 3. if it is a network config, set to network link list */
        /* 4. if it is a host config, set to host link list */
    }

} /*  end of DHCP_WA_SetPoolConfigEntry */
#endif

/* FUNCTION NAME : DHCP_WA_CreatePool
 * PURPOSE:
 *      Create a pool config, and link it to the pool config link list.
 *
 * INPUT:
 *      pool_name -- pool's name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE -- create successfully
 *      FALSE -- fail to create a pool
 *
 * NOTES:
 *
 */
BOOL_T DHCP_WA_CreatePool(char *pool_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool, *pool_config;
    DHCP_WA_SORTED_POOL_T sorted_pool;

    /* BODY */
    //pool_name[DHCP_WA_MAX_POOL_NAME_LEN+1]=0;

    /* 1. Allocate pool*/
    tmp_pool = (DHCP_TYPE_PoolConfigEntry_T *) dhcp_malloc(sizeof(DHCP_TYPE_PoolConfigEntry_T));
    if (NULL == tmp_pool)
    {
        /*  log msg : could not create a pool */
        return  FALSE;
    }

    /* 2. Clear pool */
    memset((char*)tmp_pool, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));

    /* 3. Set associated value*/
    memcpy(tmp_pool->pool_name, pool_name, strlen(pool_name));

    tmp_pool->options.cid.id_mode = DHCP_WA_DEFAULT_CID_MODE;
    tmp_pool->options.netbios_node_type = DHCP_WA_DEFAULT_NETBIOS_NODE_TYPE;;
    tmp_pool->options.lease_time = DHCP_WA_DEFAULT_LEASE_TIME;

    tmp_pool->pool_type = VAL_dhcpPoolPoolType_notSpecify;


    /* 4. Link to pool sorted list */
    memset(sorted_pool.pool_name, 0, DHCP_WA_MAX_POOL_NAME_LEN+1);
    memcpy(sorted_pool.pool_name, pool_name, strlen(pool_name));
    sorted_pool.pool_p = tmp_pool;

   if (!L_SORT_LST_Set(&pool_list, &sorted_pool))
    return FALSE;


    /* 5. Link to pool config link list */
    pool_config = pool;
    tmp_pool->next = pool_config;
	
	if(pool_config)    
		   pool_config->previous = tmp_pool;
    tmp_pool->clarified_next = NULL;
    pool = tmp_pool;


    return TRUE;

} /*  end of DHCP_WA_CreatePool */

/* FUNCTION NAME : DHCP_WA_DeletePool
 * PURPOSE:
 *      Delete a pool config.
 *
 * INPUT:
 *      pool_name -- pool's name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE -- create successfully
 *      FALSE -- fail to create a pool
 *
 * NOTES:
 *      1. Before deleting a pool, be sure that it is unlink from Network
 *          linked list if it is a network pool.
 *
 */
BOOL_T DHCP_WA_DeletePool(DHCP_TYPE_PoolConfigEntry_T *pool_config_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T ret;
    DHCP_WA_SORTED_POOL_T sorted_pool;

    /* BODY */
    if (pool_config_p->pool_type == DHCP_TYPE_POOL_NETWORK)
    {
        ret = DHCP_WA_UnLinkFromNetworkPoolSortedList(pool_config_p);
        if (ret != DHCP_WA_OK)
            return FALSE;
    }
    ret = DHCP_WA_UnlinkPoolFromPoolConfig(pool_config_p);
        if (ret != DHCP_WA_OK)
            return FALSE;

    /* delete this pool entry in sorted list */

    memset(sorted_pool.pool_name, 0, DHCP_WA_MAX_POOL_NAME_LEN+1);
    memcpy(sorted_pool.pool_name, pool_config_p->pool_name, strlen(pool_config_p->pool_name));

    if (!L_SORT_LST_Delete(&pool_list, &sorted_pool))
        return FALSE;


    dhcp_free(pool_config_p);
    pool_config_p = NULL;
    return TRUE;

} /* end of DHCP_WA_DeletePool */

/* FUNCTION NAME : DHCP_WA_SetExcludedIp
 * PURPOSE:
 *      To specify the excluded ip address (range) which server should not assign to client.
 *
 * INPUT:
 *      low_address  -- the lowest ip in the excluded ip address range
 *      high_address -- the highest ip in the excluded ip address range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_WA_OK      -- successfully.
 *      DHCP_WA_FAIL    -- fail to set excluded ip address.
 *      DHCP_WA_IS_NETWORK_IP_OR_NETWORK_BROADCAST -- fail to set excluded ip as network address
 *                                                      or a network broadcast
 *      DHCP_WA_EXCLUDE_ALL_IP_IN_SUBNET -- fail to set excluded ip range as (x.x.x.1 ~ x.x.x.254)
 *      DHCP_WA_HIGH_IP_SMALLER_THAN_LOW_IP -- fail to set excluded ip range that high ip smaller
 *                                              than low ip
 *      DHCP_WA_EXCLUDED_IP_IN_OTHER_SUBNET -- fail to set excluded ip in other subnet
 *
 * NOTES:
 *      1. To specify exluded ip address range, the high-address is required.
 *      Otherwise, only specify low-address is treated as excluding an address
 *      from available address range.
 *      2. (low_address != 0, high_address == 0) will treat low_address the only
 *      address to be excluded.
 *      3. In hornet, only can specify one ip excluded range, but here,
 *      we remove the restriction
 */
UI32_T DHCP_WA_SetExcludedIp(UI32_T low_address, UI32_T high_address)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    BOOL_T ret;
    DHCP_WA_IpExcluded_T  exclude_data;
    DHCP_TYPE_PoolConfigEntry_T  pool;

    if (high_address == 0x0)
        high_address = low_address;

    /* if ip is a class A/B/C network ip or network broad-cast, return false
    */
    if ((low_address << 24) == 0x00 ||(low_address  << 24) == 0xff000000 ||
        (high_address << 24) == 0x00 ||(high_address << 24) == 0xff000000)
      return DHCP_WA_IS_NETWORK_IP_OR_NETWORK_BROADCAST;

    /* prevent user setting range as (x.x.x.1 to x.x.x.254) */
    /*if (high_address)
    {
      if ( ((low_address << 24) == 0x01000000) &&
           ((high_address << 24) == 0xfe000000) )
         return DHCP_WA_EXCLUDE_ALL_IP_IN_SUBNET;
    }*/

    /* prevent high_excluded_address < low_excluded_address
    */
    if (high_address != 0)
    {
      if (high_address < low_address)
         return DHCP_WA_HIGH_IP_SMALLER_THAN_LOW_IP;
    }


    memset(&exclude_data, 0, sizeof(DHCP_WA_IpExcluded_T));
    memset(&pool, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));

#if 0
    /*prevent user set excluded_address not in the subnet*/
    while((tmp_pool = DHCP_WA_GetNextNetworkPoolbyIpMask(pool.network_address, pool.sub_netmask)) != NULL)
    {
        /*prevent set exclude address on the other subnet */
        if ( ((low_address & tmp_pool->sub_netmask)!=(tmp_pool->network_address & tmp_pool->sub_netmask)) ||
             ((high_address & tmp_pool->sub_netmask)!=(tmp_pool->network_address & tmp_pool->sub_netmask)) )
           return DHCP_WA_EXCLUDED_IP_IN_OTHER_SUBNET;

        memcpy(&pool, tmp_pool, sizeof(DHCP_TYPE_PoolConfigEntry_T));
    }
#endif

    exclude_data.low_excluded_address = low_address;
    exclude_data.high_excluded_address = high_address;
    ret = L_SORT_LST_Set(&dhcp_ip_excluded_list, &exclude_data);
    if (ret == FALSE)
        return DHCP_WA_FAIL;

    return DHCP_WA_OK;

} /* end of DHCP_WA_SetExcludedIp */

/* FUNCTION NAME : DHCP_WA_DelExcludedIp
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
 *      DHCP_WA_OK -- successfully delete the specified excluded ip.
 *      DHCP_WA_FAIL -- FAIL to delete the specified excluded ip.
 *
 * NOTES:
 *
 */
UI32_T DHCP_WA_DelExcludedIp(UI32_T low_address, UI32_T high_address)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    BOOL_T ret;
    DHCP_WA_IpExcluded_T  exclude_data;

    /* BODY */
	if (high_address == 0x0)
        high_address = low_address;
	
    exclude_data.low_excluded_address = low_address;
    exclude_data.high_excluded_address = high_address;

    ret = L_SORT_LST_Delete(&dhcp_ip_excluded_list, &exclude_data);
    if (ret == FALSE)
        return DHCP_WA_FAIL;
    else
        return DHCP_WA_OK;

} /*  end of DHCP_WA_DelExcludedIp */

/* FUNCTION NAME : DHCP_WA_GetNextIpExcluded
 * PURPOSE:
 *      This functin retrives the IP addresses that the DHCP server
 *      should not assign to DHCP clients.
 *
 * INPUT:
 *       ip_address  --low-address[high-address]
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *
 */
BOOL_T  DHCP_WA_GetNextIpExcluded(DHCP_WA_IpExcluded_T  *ip_address)
{
   if (ip_address -> low_excluded_address == 0)
      return (L_SORT_LST_Get_1st(&dhcp_ip_excluded_list, ip_address));
   else
      return (L_SORT_LST_Get_Next(&dhcp_ip_excluded_list, ip_address));

} /* end of DHCP_WA_GetNextIpExcluded */

/* FUNCTION NAME : DHCP_WA_GetIpExcluded
 * PURPOSE:
 *      This functin retrives the IP addresses that the DHCP server
 *      should not assign to DHCP clients.
 *
 * INPUT:
 *       ip_address  --low-address[high-address]
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *
 */
BOOL_T  DHCP_WA_GetIpExcluded(DHCP_WA_IpExcluded_T  *ip_address)
{
   if (ip_address -> low_excluded_address == 0)
      return FALSE;
   else
      return (L_SORT_LST_Get(&dhcp_ip_excluded_list, ip_address));

} /* end of DHCP_WA_GetIpExcluded */

/* FUNCTION NAME : DHCP_WA_LinkToNetworkPoolSortedList
 * PURPOSE:
 *
 *
 * INPUT:
 *      pool_config -- the pointer to the pool that will be added into
 *              network sorted list.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_WA_OK -- Successfully link to network sorted list
 *      DHCP_WA_FAIL -- Fail to link to network sorted list
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_WA_LinkToNetworkPoolSortedList(DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool, *tmp_network_pool;

    /* BODY */
    if (pool_config == NULL)
        return DHCP_WA_FAIL;

    /* Check total numbers of network pool */
    if (network_pool_num >= SYS_ADPT_MAX_NBR_OF_DHCP_NETWORK_POOL)
        return DHCP_WA_FAIL;

    /* reconfirm the pool name has already existed */
    tmp_pool = DHCP_WA_FindPoolByPoolName(pool_config->pool_name);
    if (tmp_pool == NULL)
        return DHCP_WA_FAIL;

    /* Search the position to insert into network config sorted list */
    tmp_network_pool = DHCP_WA_GetNextNetworkPoolbyIpMask(pool_config->network_address, pool_config->sub_netmask);

    if (tmp_network_pool == NULL)
    {
        /* there is no network_pool config yet */
        if (network_pool == NULL)
        {
            //network_pool->clarified_previous = NULL;
            network_pool = tmp_pool;
           // network_pool->clarified_next = NULL;
        }
        else /* insert to network_pool's tail */
        {
            tmp_pool = network_pool;
            while(tmp_pool->clarified_next != NULL )
            {
                tmp_pool = tmp_pool->clarified_next;
            }
            tmp_pool->clarified_next = pool_config;
            pool_config->clarified_previous = tmp_pool;
            tmp_pool->clarified_next->clarified_next = NULL;

        }
    }
    else
    {
        /* insert pool in front of tmp_network_pool */
        if (tmp_network_pool->clarified_previous != NULL)
        {
            pool_config->clarified_next = tmp_network_pool;
            pool_config->clarified_previous = tmp_network_pool->clarified_previous;
            tmp_network_pool->clarified_previous->clarified_next = pool_config;
            tmp_network_pool->clarified_previous = pool_config;
        }
        else
        {
            /* insert into network pool's head */
            pool_config->clarified_next = network_pool;
            pool_config->clarified_previous = NULL;
            pool_config->clarified_next->clarified_previous = pool_config;
            network_pool = pool_config;
        }
    }
    network_pool_num++;
    return DHCP_WA_OK;

} /*  end of DHCP_WA_LinkToNetworkPoolSortedList */

/* FUNCTION NAME : DHCP_WA_UnLinkFromNetworkPoolSortedList
 * PURPOSE:
 *
 *
 * INPUT:
 *      pool_config -- the pointer to the pool that will be unlinked from
 *              network sorted list.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_WA_OK -- Successfully link to network sorted list.
 *      DHCP_WA_FAIL -- Fail to link to network sorted list.
 *      DHCP_WA_POOL_NOT_EXISTED -- Pool is not existed.
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_WA_UnLinkFromNetworkPoolSortedList(DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool, *tmp_network_pool;

    /* BODY */
    if (pool_config == NULL)
        return DHCP_WA_FAIL;

    /* reconfirm the pool name has already existed */
    tmp_pool = DHCP_WA_FindPoolByPoolName(pool_config->pool_name);
    if (tmp_pool == NULL)
        return DHCP_WA_POOL_NOT_EXISTED;

    /* Get the pool from network sorted list */
    tmp_network_pool = DHCP_WA_GetNetworkPoolbyIpMask(pool_config->network_address, pool_config->sub_netmask);
    if (tmp_network_pool == NULL)
        return DHCP_WA_POOL_NOT_EXISTED;

    if (network_pool_num <= 0)
        return DHCP_WA_POOL_NOT_EXISTED;

    if (tmp_network_pool->clarified_next == NULL && tmp_network_pool->clarified_previous == NULL)
        network_pool = NULL;
    else
    {
        if (tmp_network_pool->clarified_next != NULL && tmp_network_pool->clarified_previous == NULL)
        {
            tmp_network_pool->clarified_next->clarified_previous = tmp_network_pool->clarified_previous;
            network_pool = tmp_network_pool->clarified_next;
        }
        else if (tmp_network_pool->clarified_next != NULL && tmp_network_pool->clarified_previous != NULL)
        {
            tmp_network_pool->clarified_next->clarified_previous = tmp_network_pool->clarified_previous;

        }

        if (tmp_network_pool->clarified_previous != NULL)
            tmp_network_pool->clarified_previous->clarified_next = tmp_network_pool->clarified_next;
    }

    network_pool_num--;
    return DHCP_WA_OK;

} /* end of DHCP_WA_UnLinkFromNetworkPoolSortedList */

/* FUNCTION NAME : DHCP_WA_FindPoolByPoolName
 * PURPOSE:
 *      Search pool config table by pool's name.
 *
 * INPUT:
 *      pool_name -- pool's name to be the key to search
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NULL -- Can't find any pool config by specified pool's name
 *      others -- the pointer pointed to the pool config table
 *
 * NOTES:
 *
 */
DHCP_TYPE_PoolConfigEntry_T *DHCP_WA_FindPoolByPoolName(char *pool_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */
    tmp_pool = pool;

    while (tmp_pool != NULL)
    {
        if (strcmp(tmp_pool->pool_name, pool_name) != 0)
            tmp_pool = tmp_pool->next;
        else
            break;
    }
    if (tmp_pool != NULL)
        return  tmp_pool;
    else
        return NULL;

} /*  end of DHCP_WA_FindPoolByPoolName */

/* FUNCTION NAME : DHCP_WA_UnlinkPoolFromPoolConfig
 * PURPOSE:
 *      Unlink Pool from pool config but don't free it
 *
 * INPUT:
 *      pool_config -- the specified pool for unlinking
 *
 * OUTPUT:
 *      DHCP_WA_OK  -- successfully unlink the specified pool.
 *      DHCP_WA_FAIL -- Fail to do so.
 *
 * RETURN:
 *      NULL -- Can't find any pool config by specified pool's name
 *      others -- the pointer pointed to the pool config table
 *
 * NOTES:
 *
 */
UI32_T DHCP_WA_UnlinkPoolFromPoolConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */


    /* BODY */
    if (pool_config == NULL)
        return DHCP_WA_FAIL;

    /* Check if this unlink pool is the 1st pool in the pool list */
    if (pool_config->previous == NULL)
    {
        pool =  pool_config->next;
    }
    else
    {
        pool_config->previous->next = pool_config->next;
    }

    if (pool_config->next != NULL)
        pool_config->next->previous = pool_config->previous;

    return DHCP_WA_OK;
}
/* FUNCTION NAME : DHCP_WA_GetNextPoolConfigEntry
 * PURPOSE:
 *      Get the next pool
 *
 * INPUT:
 *      pool_name   -- the pool name as a key to search in sorted list
 *      pool_config -- the pointer to the pool.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- Successfully get the value
 *      FALSE   -- Fail to get next
 *
 * NOTES:
 *
 *
 */
BOOL_T DHCP_WA_GetNextPoolConfigEntry(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_SORTED_POOL_T sorted_pool;

    /* BODY */

    memset(sorted_pool.pool_name, 0, DHCP_WA_MAX_POOL_NAME_LEN+1);
    strcpy(sorted_pool.pool_name, pool_name);



    /* Get the 1st pool if there is one */
    if (strlen(sorted_pool.pool_name) == 0)
    {

        if (!L_SORT_LST_Get_1st(&pool_list, &sorted_pool))
            return FALSE;

    }
    else
    {
        if (!L_SORT_LST_Get_Next(&pool_list, &sorted_pool))
            return FALSE;

    }

    memcpy(pool_config, sorted_pool.pool_p, sizeof(DHCP_TYPE_PoolConfigEntry_T));
    memcpy(pool_name, pool_config->pool_name, DHCP_WA_MAX_POOL_NAME_LEN+1);
    return TRUE;

}/* end of DHCP_WA_GetNextPoolConfigEntry */

/* FUNCTION NAME : DHCP_WA_GetPoolConfigEntryByPoolName
 * PURPOSE:
 *      Get the pool config by pool name.
 *
 * INPUT:
 *      pool_name -- the name of the pool to be searched.
 * OUTPUT:
 *      pool_config -- the pointer to the pool config structure
 *
 * RETURN:
 *      TRUE --  successfully.
 *
 * NOTES:
 *
 *
 */
BOOL_T DHCP_WA_GetPoolConfigEntryByPoolName(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool_p;
    /* BODY */
    tmp_pool_p = pool;

    if (pool_name == NULL)
        return FALSE;
    else
    {
        while (tmp_pool_p)
        {
            if (strcmp(pool_name, tmp_pool_p->pool_name))
                tmp_pool_p = tmp_pool_p->next;
            else
                break;
        }
    }

    if (tmp_pool_p)
    {
        memcpy(pool_config_p, tmp_pool_p, sizeof(*tmp_pool_p));
        return TRUE;
    }
    else
        return FALSE;

}/* end of DHCP_WA_GetPoolConfigEntryByPoolName */

/* FUNCTION NAME : DHCP_WA_GetParentPoolbyIpMaskForHost
 * PURPOSE:
 *      Get the parent pool (network pool) by host ip and subnet mask.
 *
 * INPUT:
 *      ip_address  -- the host IP Address
 *      sub_netmask -- the host sub_netmask
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      tmp_pool -- the pointer to network pool as a parent to the given host pool.
 *      NULL    -- can't find the pool.
 *
 * NOTES:
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetParentPoolbyIpMaskForHost(UI32_T ip_address, UI32_T sub_netmask)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */
    tmp_pool = network_pool;

    if (ip_address == 0 && sub_netmask == 0)
        return NULL;
    else
    {
        while (tmp_pool != NULL)
        {
            if ((ip_address & sub_netmask)<
                (tmp_pool->network_address & tmp_pool->sub_netmask))
                tmp_pool = tmp_pool->next;
            else
                break;
        }
        if (tmp_pool != NULL)
            return  tmp_pool;
        else
            return NULL;
    }

} /* end of DHCP_WA_GetParentPoolbyIpMaskForHost */

/* FUNCTION NAME : DHCP_WA_GetNextNetworkPoolbyIpMask
 * PURPOSE:
 *      Get the next network pool by ip and subnet mask.
 *
 * INPUT:
 *      ip_address
 *      sub_netmask
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      tmp_pool -- the pointer to the pool which (ip+mask)is smaller than the given (ip+mask)
 *      NULL    -- can't find the pool.
 *
 * NOTES:
 *      (ip_address, sub_netmask)=(0, 0) to get the 1st Network pool.
 *
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNextNetworkPoolbyIpMask(UI32_T ip_address, UI32_T sub_netmask)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */
    tmp_pool = network_pool;

    /* if (ip, mask) = (0,0), get the 1st network pool */
    if (ip_address == 0 && sub_netmask == 0)
    {
        if (tmp_pool != NULL)
            return  tmp_pool;
        else
            return NULL;
    }
    else
    {
        while (tmp_pool != NULL)
        {
            if ((ip_address & sub_netmask)<=
                (tmp_pool->network_address & tmp_pool->sub_netmask))
                tmp_pool = tmp_pool->clarified_next;
            else
                break;
        }
        if (tmp_pool != NULL)
            return  tmp_pool;
        else
            return NULL;
    }

} /* end of DHCP_WA_GetNextNetworkPoolbyIpMask */

/* FUNCTION NAME : DHCP_WA_GetNetworkPoolbyIpMask
 * PURPOSE:
 *      Get the network pool by ip and subnet mask.
 *
 * INPUT:
 *      ip_address
 *      sub_netmask
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      tmp_pool -- the pointer to the pool which (ip+mask)is equal to the given (ip+mask).
 *      NULL    -- can't find the pool.
 *
 * NOTES:
 *
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNetworkPoolbyIpMask(UI32_T ip_address, UI32_T sub_netmask)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */
    tmp_pool = network_pool;

    while (tmp_pool != NULL)
    {
        if ((ip_address & sub_netmask)!=
            (tmp_pool->network_address & tmp_pool->sub_netmask))
            tmp_pool = tmp_pool->clarified_next;
        else
            break;
    }
    if (tmp_pool != NULL)
        return  tmp_pool;
    else
        return NULL;

} /* DHCP_WA_GetNetworkPoolbyIpMask */

/* FUNCTION NAME : DHCP_WA_GetHostPoolbyPoolName
 * PURPOSE:
 *      Get the host pool by pool name
 *
 * INPUT:
 *      pool_name -- the name of the pool to search
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      tmp_pool -- the pointer to the pool according to the pool name
 *      NULL    -- can't find the pool.
 *
 * NOTES:
 *
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetHostPoolbyPoolName(char *pool_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */
    tmp_pool = pool;

    if (pool_name == NULL)
        return NULL;
    while (tmp_pool != NULL)
    {
        if (!memcmp(pool_name, tmp_pool->pool_name, sizeof(tmp_pool->pool_name)) &&
            tmp_pool->pool_type == DHCP_TYPE_POOL_HOST)
            break;
        else
            tmp_pool = tmp_pool->next;
    }
    if (tmp_pool != NULL)
        return  tmp_pool;
    else
        return NULL;


} /* end of DHCP_WA_GetHostPoolbyPoolName */

/* FUNCTION NAME : DHCP_WA_GetNextHostPoolbyPoolName
 * PURPOSE:
 *      Get the next host pool by pool name
 *
 * INPUT:
 *      pool_name -- the name of the pool;
 *                -- If pool_name = NULL, get the 1st host pool
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      tmp_pool -- the pointer to the next pool following the input pool.
 *      NULL    -- can't find the pool.
 *
 * NOTES:
 *
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNextHostPoolbyPoolName(char *pool_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
     DHCP_TYPE_PoolConfigEntry_T *tmp_pool;

    /* BODY */

    tmp_pool = pool;

    /* if pool_name is NULL, get the 1st host pool */
    if (pool_name == NULL)
    {
        while (tmp_pool != NULL)
        {
            if (tmp_pool->pool_type == DHCP_TYPE_POOL_HOST)
                break;
            else
                tmp_pool = tmp_pool->next;
        }

        if (tmp_pool != NULL)
            return  tmp_pool;
        else
            return NULL;
    }
    else
    {
        tmp_pool = DHCP_WA_GetHostPoolbyPoolName(pool_name);
        if (tmp_pool == NULL)
            return NULL;
        else
        {
            tmp_pool = tmp_pool->next;
            while (tmp_pool != NULL)
            {
                if (tmp_pool->pool_type == DHCP_TYPE_POOL_HOST)
                    break;
                else
                    tmp_pool = tmp_pool->next;
            }

            if (tmp_pool != NULL)
                return  tmp_pool;
            else
                return NULL;
        }
    }

} /* end of DHCP_WA_GetNextHostPoolbyPoolName */
#if 0
/* FUNCTION NAME : DHCP_WA_PoolInheritance
 * PURPOSE: Check option inheritance issue in pool config (WA)
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
 *      1. This function will be called each time DHCP Server Restart
 */
void DHCP_WA_PoolInheritance()
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_TYPE_PoolConfigEntry_T *host_pool, *parent_pool, *tmp_pool;

    /* BODY */

    if (network_pool == NULL)
        return;

    tmp_pool = network_pool;

    /*add by simon:when restart DHCP server, active_pool_list need to be refreshed*/
    L_SORT_LST_Delete_All(&active_pool_list );
    /* Get the last network pool entry */
    while(tmp_pool->next != NULL )
        tmp_pool = tmp_pool->next;

    /* 1. do option inheritance if needed for Network config pool */
    while (tmp_pool->previous != NULL)
    {
        tmp_pool = tmp_pool->previous;
        /* check if next pool on the same subnet as current or not */
        if ((tmp_pool->network_address & tmp_pool->sub_netmask) !=
            (tmp_pool->next->network_address & tmp_pool->next->sub_netmask))
            continue;

        /* check if all of the next pool's options are empty or not */
        if (tmp_pool->next->options.cid.id_len == 0 &&
            tmp_pool->next->options.default_router[0] == 0 &&
            tmp_pool->next->options.dns_server[0] == 0 &&
            tmp_pool->next->options.next_server == 0 &&
            tmp_pool->next->options.netbios_name_server[0] == 0 &&
            tmp_pool->next->options.netbios_node_type == 0 &&
            tmp_pool->next->options.domain_name[0] == 0 &&
            tmp_pool->next->options.bootfile[0] == 0 &&
            tmp_pool->next->options.lease_time == 0)
            continue;

        /* inherit from parent's options if one of the current options is empty */
        if (tmp_pool->options.cid.id_len == 0 &&
            tmp_pool->next->options.cid.id_len != 0)
            memcpy(&tmp_pool->options.cid, &tmp_pool->next->options.cid, sizeof(DHCP_TYPE_ClientId_T));

        if (tmp_pool->options.default_router[0] == 0 &&
            tmp_pool->next->options.default_router[0] != 0)
            memcpy(tmp_pool->options.default_router,
                    tmp_pool->next->options.default_router,
                    sizeof(tmp_pool->next->options.default_router));

        if (tmp_pool->options.dns_server[0] == 0 &&
            tmp_pool->next->options.dns_server[0] != 0)
            memcpy(tmp_pool->options.dns_server,
                    tmp_pool->next->options.dns_server,
                    sizeof(tmp_pool->next->options.dns_server));

        if (tmp_pool->options.next_server == 0 &&
            tmp_pool->next->options.next_server != 0)
            memcpy(&tmp_pool->options.next_server,
                    &tmp_pool->next->options.next_server,
                    sizeof(UI32_T));

        if (tmp_pool->options.netbios_name_server[0] == 0 &&
            tmp_pool->next->options.netbios_name_server[0] != 0)
            memcpy(tmp_pool->options.netbios_name_server,
                    tmp_pool->next->options.netbios_name_server,
                    sizeof(tmp_pool->next->options.netbios_name_server));

        if (tmp_pool->options.netbios_node_type == 0 &&
            tmp_pool->next->options.netbios_node_type != 0)
            tmp_pool->options.netbios_node_type = tmp_pool->next->options.netbios_node_type;

        if (tmp_pool->options.domain_name[0] == 0 &&
            tmp_pool->next->options.domain_name[0] != 0)
            memcpy(tmp_pool->options.domain_name,
                    tmp_pool->next->options.domain_name,
                    sizeof(tmp_pool->next->options.domain_name));

        if (tmp_pool->options.bootfile[0] == 0 &&
            tmp_pool->next->options.bootfile[0] != 0)
            memcpy(tmp_pool->options.bootfile,
                    tmp_pool->next->options.bootfile,
                    sizeof(tmp_pool->next->options.bootfile));

        if (tmp_pool->options.lease_time == 0 &&
            tmp_pool->next->options.lease_time != 0)
            tmp_pool->options.lease_time = tmp_pool->next->options.lease_time;

    } /* end of while (tmp_pool = tmp_pool->previous) */

    // 2. do option inheritance if needed for Host config pool
    host_pool = DHCP_WA_GetNextHostPoolbyPoolName(NULL);
    while (host_pool != NULL)
    {
        /* Get host pool's parent from network pool */
    //  parent_pool = DHCP_WA_GetNextNetworkPoolbyIpMask(host_pool->host_address, host_pool->sub_netmask);
        parent_pool = DHCP_WA_GetParentPoolbyIpMaskForHost(host_pool->host_address, host_pool->sub_netmask);
        if (parent_pool != NULL)
        {
            /* inherit from parent's options if one of the current options is empty */
            if (host_pool->options.cid.id_len == 0 &&
                parent_pool->options.cid.id_len != 0)
                memcpy(&host_pool->options.cid, &parent_pool->options.cid, sizeof(DHCP_TYPE_ClientId_T));

            if (host_pool->options.default_router[0] == 0 &&
                parent_pool->options.default_router[0] != 0)
                memcpy(host_pool->options.default_router,
                        parent_pool->options.default_router,
                        sizeof(parent_pool->options.default_router));

            if (host_pool->options.dns_server[0] == 0 &&
                parent_pool->options.dns_server[0] != 0)
                memcpy(host_pool->options.dns_server,
                        parent_pool->options.dns_server,
                        sizeof(parent_pool->options.dns_server));

            if (host_pool->options.next_server == 0 &&
                parent_pool->options.next_server != 0)
                memcpy(&host_pool->options.next_server,
                        &parent_pool->options.next_server,
                        sizeof(UI32_T));

            if (host_pool->options.netbios_name_server[0] == 0 &&
                parent_pool->options.netbios_name_server[0] != 0)
                memcpy(host_pool->options.netbios_name_server,
                        parent_pool->options.netbios_name_server,
                        sizeof(parent_pool->options.netbios_name_server));

            if (host_pool->options.netbios_node_type == 0 &&
                parent_pool->options.netbios_node_type != 0)
                host_pool->options.netbios_node_type = parent_pool->options.netbios_node_type;

            if (host_pool->options.domain_name[0] == 0 &&
                parent_pool->options.domain_name[0] != 0)
                memcpy(host_pool->options.domain_name,
                        parent_pool->options.domain_name,
                        sizeof(parent_pool->options.domain_name));

            if (host_pool->options.bootfile[0] == 0 &&
                parent_pool->options.bootfile[0] != 0)
                memcpy(host_pool->options.bootfile,
                        parent_pool->options.bootfile,
                        sizeof(parent_pool->options.bootfile));

            if (host_pool->options.lease_time == DHCP_WA_DEFAULT_LEASE_TIME &&
                parent_pool->options.lease_time != 0)
                host_pool->options.lease_time = parent_pool->options.lease_time;


        } /* end of if (parent_pool != NULL) */

        host_pool = DHCP_WA_GetNextHostPoolbyPoolName(host_pool->pool_name);

    } /* end of while (host_pool != NULL) */

} /*  end of DHCP_WA_PoolInheritance */
#endif

/* FUNCTION NAME : DHCP_WA_GetNetworkRange
 * PURPOSE:
 *      Calculate out the available range in a subnet by excluding user specified ip
 *      or ip range.
 *
 * INPUT:
 *      ip   -- ip address (either a host address or a network address)
 *      mask -- subnet mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void DHCP_WA_GetNetworkRange(struct subnet *subnet, UI32_T  network_address, UI32_T  sub_netmask)
{
    DHCP_WA_IpExcluded_T        input_range_value,  get_range_value, exclude_data;
    DHCP_WA_IpRange_T           active_range;
    struct iaddr low, high;
    int dynamic = 1;   /* 1 to enable bootp */

    /* calculate subnet range 
     */
    input_range_value.low_excluded_address = 
        (network_address & sub_netmask ) | 0x00000001;

    input_range_value.high_excluded_address = 
        (network_address & sub_netmask) | (~sub_netmask & 0xFFFFFFFE);    

    memset(&exclude_data,0,sizeof(exclude_data));
    while (DHCP_WA_GetNextIpExcluded(&exclude_data))
    {
        /* (1)|---- input range --- |
         *                            |--- exclude ----|
         *    range higher than input range, we don't need to handle it 
         */         
        if(exclude_data.low_excluded_address > input_range_value.high_excluded_address)
        {
            continue;
        }

        /* (2)                      |---- input range ----| 
         *     |---- exlude ---- |
         *     range lower than input range, we should get the next exlucde range 
         */
        if(exclude_data.high_excluded_address < input_range_value.low_excluded_address)
            continue;
            
        /* (3)     |---- input range ----|
         *       |----     exclude     ----|
         */
        if((exclude_data.low_excluded_address <= input_range_value.low_excluded_address)&&
           (exclude_data.high_excluded_address >= input_range_value.high_excluded_address))
        {
            input_range_value.low_excluded_address = 0;
            input_range_value.high_excluded_address = 0;
            break;
        }

        /* (4)    |---- input range ----| 
         *     |---- exclude ----|  
         */
        if((exclude_data.low_excluded_address<=input_range_value.low_excluded_address)&&
           (exclude_data.high_excluded_address<input_range_value.high_excluded_address)&&
           (exclude_data.high_excluded_address>=input_range_value.low_excluded_address))
        {
            input_range_value.low_excluded_address = exclude_data.high_excluded_address+1;
        }
                    
        /* (5)    |---- input range ----|
         *             |---- exclude ----|
         */
        if((exclude_data.low_excluded_address>input_range_value.low_excluded_address)&&
           (exclude_data.low_excluded_address<input_range_value.high_excluded_address)&&
           (exclude_data.high_excluded_address>=input_range_value.high_excluded_address))
        {
            input_range_value.high_excluded_address = exclude_data.low_excluded_address-1;
            /* no exclude range will cover the input range, break the loop
             */
            break;
        }
                    
        /* (6)    |---- input range ----| 
         *           |-- exclude --|
         */
        if((exclude_data.low_excluded_address>input_range_value.low_excluded_address)&&
           (exclude_data.high_excluded_address<input_range_value.high_excluded_address))
        {
            DHCP_WA_IpExcluded_T low_range;

            low_range.low_excluded_address = input_range_value.low_excluded_address;
            low_range.high_excluded_address = exclude_data.low_excluded_address-1;

            /* store the lower range after cut
             */
            L_SORT_LST_Set(&range_set_list, &low_range);
            input_range_value.low_excluded_address = exclude_data.high_excluded_address+1;
        }    
    }  

    /* store the result after cut all exlude range 
     */
    if((input_range_value.high_excluded_address >= input_range_value.low_excluded_address)&&
       (input_range_value.low_excluded_address != 0)&&
       (input_range_value.high_excluded_address != 0))
    {
        L_SORT_LST_Set(&range_set_list, &input_range_value);
    }  

    memset(&get_range_value,0,sizeof(get_range_value));
    
    /* get each range value and put it into server om by calling new_address_range */
    if(FALSE == L_SORT_LST_Get_1st(&range_set_list, &get_range_value))
        return;

      do
      {
        memcpy(low.iabuf, &get_range_value.low_excluded_address, sizeof(UI32_T));
        low.len = sizeof(UI32_T);
        memcpy(high.iabuf, &get_range_value.high_excluded_address, sizeof(UI32_T));
        high.len = sizeof(UI32_T);

        new_address_range(low, high, subnet, dynamic,&active_range.low_address,&active_range.high_address);
        
        if(active_range.low_address !=0)
        { 
            /* store active range */
            L_SORT_LST_Set(&active_pool_list,&active_range);
        }

      } while (L_SORT_LST_Get_Next(&range_set_list, &get_range_value));

    /* clear range set buffer list 
     */    
    L_SORT_LST_Delete_All(&range_set_list);

} /* end of DHCP_WA_GetNetworkRange() */

/* FUNCTION NAME :DHCP_WA_ParseOptionConfig
 * PURPOSE:
 *      Move all the options config in specified subnet to Server OM (memory.c)
 * INPUT:
 *      group -- group structure to hold options config.
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
void DHCP_WA_ParseOptionConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config, struct group *group)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    struct tree *tree = (struct tree *)0;

    /* BODY */
    /* 1. dflt Lease time  */
    group -> default_lease_time = pool_config->options.lease_time;

    /* 2. bootfile */
    if (strlen(pool_config->options.bootfile) != 0)
    {
        memcpy(group->filename, pool_config->options.bootfile, strlen(pool_config->options.bootfile));
    }
    /* 3. next_server */
    if (pool_config->options.next_server != 0)
    {
        /*memcpy(addr, &pool_config->options.next_server, sizeof(UI32_T));
        rv = tree_const(addr, 4);
        if (rv)
        {
            cache =tree_cache(rv);
            if (!tree_evaluate(cache))
                error ("\nnext-server is not known");

            group->next_server.len = 4;
            memcpy(group->next_server.iabuf, cache->value, group->next_server.len);
        }
        */
        /* Mask above reason, Penny: Due to in Group structure the next_server only
        ** can store 1 IP, it is not necessary to allocate more tree structure to
        ** hold the IP.
        */
        group->next_server.len = 4;
        memcpy(group->next_server.iabuf, &pool_config->options.next_server,
                group->next_server.len);

    }

    /* 4. domain name */
    if (strlen(pool_config->options.domain_name) != 0)
    {
        tree = parse_option_param(DHO_DOMAIN_NAME, pool_config);
        group->options[DHO_DOMAIN_NAME] = tree_cache (tree);
    }

    /* 5. netbios node type */
    if (pool_config->options.netbios_node_type != 0)
    {
        tree = parse_option_param(DHO_NETBIOS_NODE_TYPE, pool_config);
        group->options[DHO_NETBIOS_NODE_TYPE] = tree_cache (tree);
    }

    /* 6. netbios name server */
    if (pool_config->options.netbios_name_server[0] != 0)
    {
        tree = parse_option_param(DHO_NETBIOS_NAME_SERVERS, pool_config);
        group->options[DHO_NETBIOS_NAME_SERVERS] = tree_cache (tree);
    }
    /* 7. dns_server */
    if (pool_config->options.dns_server[0] != 0)
    {
        tree = parse_option_param(DHO_DOMAIN_NAME_SERVERS, pool_config);
        group->options[DHO_DOMAIN_NAME_SERVERS] = tree_cache (tree);
    }

    /* 8. default_router */
    if (pool_config->options.default_router[0] != 0)
    {
        tree = parse_option_param(DHO_ROUTERS, pool_config);
        group->options[DHO_ROUTERS] = tree_cache (tree);
    }

    /* 9. cid -- only host config pool contains this option */
    if (pool_config -> pool_type == DHCP_TYPE_POOL_HOST )
    {
        if (strlen((char *)pool_config->options.cid.id_buf) != 0)
        {
            tree = parse_option_param(DHO_DHCP_CLIENT_IDENTIFIER, pool_config);
            group->options[DHO_DHCP_CLIENT_IDENTIFIER] = tree_cache (tree);
        }
    }
} /* end of DHCP_WA_ParseOptionConfig */

/* FUNCTION NAME :DHCP_WA_GetNumbersOfHostPool
 * PURPOSE:
 *      Get the total numbers of host pool in WA.
 * INPUT:
 *      num -- the pointer to point the numbers of host pool.
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
void DHCP_WA_GetNumbersOfHostPool(int *num)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    *num = host_pool_num;
} /* end of DHCP_WA_GetNumbersOfHostPool */

/* FUNCTION NAME :DHCP_WA_SetNumbersOfHostPool
 * PURPOSE:
 *      Set the numbers of host pool in WA.
 * INPUT:
 *      num -- the numbers of host pool.
 *      increase -- TRUE: increase host pool number
 *                  FALSE: decrease host pool number
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
void DHCP_WA_SetNumbersOfHostPool(int num, BOOL_T increase)
{
    if (increase)
        host_pool_num = host_pool_num + num;
    else
        host_pool_num = host_pool_num - num;
} /* end of DHCP_WA_SetNumbersOfHostPool */

#endif /* #if   (SYS_CPNT_DHCP_SERVER == TRUE) */



/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */

/*---------------------------
 *  DHCP list management
 *---------------------------
 */
/* FUNCTION NAME : DHCP_WA_CreateIfDhcpIf
 * PURPOSE:
 *      Create a buffer to keep one interface configuration info. and
 *      link to the link list.
 *
 * INPUT:
 *      new_ifIndex -- the interface to be created.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NULL    -- Can't create the interface, no space or existed.
 *      others  -- the block new created, and linked to list.
 *
 * NOTES:
 *      None.
 */
static DHCP_WA_InterfaceDhcpInfo_T* DHCP_WA_CreateIfDhcpIf (UI32_T new_ifIndex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *if_ptr,*tmp_ptr,*last_ptr;
    /* BODY */
    /*  Check the interface is configured or not ?
     *  If already exist,
     *      return  NULL
     *  else
     *      allocate a config block,
     *      fill proper data
     *      link to the list
     */
    if_ptr = DHCP_WA_SearchIfInList(new_ifIndex);
    if (if_ptr)
        return  NULL;
    if (!L_MPOOL_AllocateBlock(dhcp_wa_lcb.if_config_pool, (void**)&if_ptr))
        return  NULL;
#ifdef UNIT_TEST_DHCP
    if_ptr = (DHCP_WA_InterfaceDhcpInfo_T *)dhcp_malloc(sizeof(DHCP_WA_InterfaceDhcpInfo_T));

#endif
    last_ptr=NULL;
    /*  Set the block initial value */
    memset(if_ptr, 0, sizeof(DHCP_WA_InterfaceDhcpInfo_T));
    if_ptr->client_port = DHCP_WA_DEFAULT_CLIENT_PORT;
    if_ptr->server_port = DHCP_WA_DEFAULT_SERVER_PORT;
    if_ptr->assigned_ip = FALSE;
    if_ptr->ifIndex = new_ifIndex;
    /* shumin.wang fix bug ES3628BT-FLF-ZZ-00439 */
    if_ptr->cid.id_mode = VAL_dhcpcIfClientIdMode_notSpecify;
    if_ptr->relay_server[0]=0x0;
    if_ptr->relay_server[1]=0x0;
    if_ptr->relay_server[2]=0x0;
    if_ptr->relay_server[3]=0x0;
    if_ptr->relay_server[4]=0x0;

    /* 2002-12-19, Penny */
    if_ptr->if_binding_role = DHCP_WA_DEFAULT_ROLE;
#if (SYS_CPNT_DHCP_CLIENT_CLASSID == TRUE)  
{
    UI32_T id_len=0;
    UI8_T  model_name[SYS_ADPT_MAX_MODEL_NAME_SIZE+1] = {0};

    if (TRUE == SYS_PMGR_GetModelName(0, model_name))
    {
        id_len = strlen((char *)model_name);

        if_ptr->classid.vendor_mode = DHCP_MGR_CLASSID_TEXT;
        if_ptr->classid.vendor_len = id_len;
 
    	memcpy(if_ptr->classid.vendor_buf, model_name, id_len);
    	
    }	  
}
#else
    if_ptr->classid.vendor_len = 0;        
    memset(if_ptr->classid.vendor_buf, 0, DHCP_MGR_CLASSID_BUF_MAX_SIZE);	
#endif

    /*  Link the new block to the sorted linked list, the largest ifIndex is the last node. */
    if(dhcp_wa_lcb.if_list == NULL)
    {
        if_ptr->next=NULL;
        if_ptr->previous=NULL;
        dhcp_wa_lcb.if_list = if_ptr;
    }
    else
    {
        tmp_ptr = dhcp_wa_lcb.if_list;
        while ((tmp_ptr != NULL)&&(tmp_ptr->ifIndex < new_ifIndex))
        {
            if(tmp_ptr->next == NULL)
                last_ptr=tmp_ptr;
            tmp_ptr = tmp_ptr->next;
        }

        if(tmp_ptr != NULL)
        {
            /* if_ptr is the first node */
            if(tmp_ptr->previous == NULL)
            {
                if_ptr->next = tmp_ptr;
                if_ptr->previous = NULL;
                tmp_ptr->previous = if_ptr;
                dhcp_wa_lcb.if_list = if_ptr;
            }
            /* if_ptr is the middle node */
            else
            {
                if_ptr->next = tmp_ptr;
                tmp_ptr->previous->next=if_ptr;
                if_ptr->previous = tmp_ptr->previous;
                tmp_ptr->previous = if_ptr;
            }
        }
        /* if_ptr is last node */
        else
        {
            last_ptr->next=if_ptr;
            if_ptr->previous=last_ptr;
            if_ptr->next=NULL;
        }

    }
    dhcp_wa_lcb.managed_if_count++;
    return  if_ptr;
}   /*  end of DHCP_WA_CreateIfDhcpIf   */

/* FUNCTION NAME : DHCP_WA_DestroyIfDhcpInfo
 * PURPOSE:
 *      Remove the interface from list.
 *
 * INPUT:
 *      ifIndex -- the interface to be removed.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- successfully removed
 *      FALSE   -- no this interface.
 *
 * NOTES:
 *      None.
 */
BOOL_T DHCP_WA_DestroyIfDhcpInfo (UI32_T ifIndex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    DHCP_WA_InterfaceDhcpInfo_T *if_ptr;
    /* BODY */
    /*  Check whether the interface exist ?
     *  If not found, then
     *      return FALSE
     *  else
     *      Remove the entry from list
     */
    if_ptr = DHCP_WA_SearchIfInList(ifIndex);
    if (if_ptr == NULL)
        return  FALSE;
    if (if_ptr->next)
        if_ptr->next->previous = if_ptr->previous;
    if (if_ptr->previous)
        if_ptr->previous->next = if_ptr->next;
    if(if_ptr == dhcp_wa_lcb.if_list)
    {
        dhcp_wa_lcb.if_list = if_ptr->next;
    }
    dhcp_wa_lcb.managed_if_count--;
    L_MPOOL_FreeBlock (dhcp_wa_lcb.if_config_pool, if_ptr);
    return  TRUE;
}   /*  end of DHCP_WA_DestroyIfDhcpInfo    */

#if (SYS_CPNT_DHCP_SERVER == TRUE)
static int DHCP_WA_IpExcludedCompare(void *node_entry, void *input_entry)
{
    DHCP_WA_IpExcluded_T  *node, *input;

    node = (DHCP_WA_IpExcluded_T *) node_entry;
    input = (DHCP_WA_IpExcluded_T  *) input_entry;

   if (node->low_excluded_address != input->low_excluded_address)
      return (node->low_excluded_address - input->low_excluded_address);
   else
      return (node->high_excluded_address - input->high_excluded_address);
}

/* PURPOSE: This functin compares network range.
 * INPUT:   node_entry, input_entry
 * OUTPUT:  none
 * RETURN:
 * NOTE:
 */
static int DHCP_WA_ExcludeRangeSetCompare(void *node_entry, void *input_entry)
{
    DHCP_WA_IpExcluded_T  *node, *input;

    node = (DHCP_WA_IpExcluded_T *) node_entry;
    input = (DHCP_WA_IpExcluded_T  *) input_entry;

   return (node->low_excluded_address - input->low_excluded_address);
}

static int DHCP_WA_RangeSetCompare(void *node_entry, void *input_entry)
{
	DHCP_WA_IpRange_T  *node, *input;

	node = (DHCP_WA_IpRange_T *) node_entry;
	input = (DHCP_WA_IpRange_T  *) input_entry;

   return (node->low_address - input->low_address);
}

/* PURPOSE: This functin compares pool_name length + pool_name.
 * INPUT:   node_entry, input_entry
 * OUTPUT:  none
 * RETURN:
 * NOTE:
 */
static int DHCP_WA_PoolNameCompare(void *node_entry, void *input_entry)
{
    DHCP_WA_SORTED_POOL_T *node, *input;

    node = (DHCP_WA_SORTED_POOL_T *) node_entry;
    input = (DHCP_WA_SORTED_POOL_T  *) input_entry;

    if (strlen(node->pool_name) != strlen(input->pool_name))
      return (strlen(node->pool_name) - strlen(input->pool_name));
   else
      return (strcmp(node->pool_name,input->pool_name));

}


/*add by simon*/
BOOL_T DHCP_WA_GetNextActivePool(DHCP_WA_IpRange_T * pool_range)
{

    if (pool_range->low_address == 0)
    {
      
      return (L_SORT_LST_Get_1st(&active_pool_list, pool_range));
    }
   else{
      
      return (L_SORT_LST_Get_Next(&active_pool_list, pool_range));
    }
}
void DHCP_WA_InitActivePool()
{
    L_SORT_LST_Delete_All(&active_pool_list);
}

#endif /* End of #if (SYS_CPNT_DHCP_SERVER == TRUE) */
