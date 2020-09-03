/* Module Name:	DHCP_WA.H
 * Purpose:
 *		DHCP_WA holds all CLI configuration information. It is useful when DHCP server function enable.
 *		We must take complex consideration when CLI directly configuring DHCP_OM, but with DHCP_WA,
 *		we can restart DHCP_OM management and take overall consideration.
 *		Now, DHCP_WA includes : (for client)
 *			Set interface assigned ip, server ip, gateway ip.
 *			Set the role of DHCP with interface.
 *
 * Notes:
 *		1. Each interface binging status, Bootp, DHCP, or User-defined, is kept in NETCFG,
 *		   because it's interface, needs to know by whole system CSC.
 *		2. For interface binding with DHCP, we keeps the role, Client, Server, or Relay Agent, in DHCP.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.24	--	William, Created
 *  0.2 2002.06.12	--  Penny, added APIs for relay configuration
 *  0.3 2002.07.09	--  Penny, added APIs for server configuration
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001, 2002.
 */



#ifndef		_DHCP_WA_H
#define		_DHCP_WA_H


/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"
#include "dhcp_type.h"
#include "dhcp_mgr.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

/* NAME	CONSTANT DECLARATIONS
 */
#define	DHCP_WA_MAX_IF_NBR						256		/*	SYS_BLD_MAX_IF_NBR	*/
#define DHCP_WA_MAX_POOL_NAME_LEN				SYS_ADPT_DHCP_MAX_POOL_NAME_LEN 
#define DHCP_WA_MAX_IP_EXCLUDED_ELEMENTS  		SYS_ADPT_DHCP_MAX_IP_EXCLUDED_ELEMENTS
#define DHCP_WA_MAX_RANGE_SET_ELEMENTS   		SYS_ADPT_DHCP_MAX_RANGE_SET_ELEMENTS 
#define DHCP_WA_MAX_NBR_OF_RELAY_SERVER 		SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER
#define DHCP_WA_MAX_NBR_OF_DFLT_ROUTER  		SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER 
#define DHCP_WA_MAX_NBR_OF_DNS_SERVER 			SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER 
//#define DHCP_WA_MAX_NBR_OF_NEXT_SERVER  		SYS_ADPT_DHCP_MAX_NBR_OF_NEXT_SERVER
#define DHCP_WA_MAX_NBR_OF_NETBIOS_NAME_SERVER  SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER 

#define DHCP_WA_MAX_DOMAIN_NAME_LEN     		SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN
#define DHCP_WA_MAX_BOOTFILE_LEN     			SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN 
#define DHCP_WA_MAX_CLIENT_HOSTNAME_LEN			SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN
#define	DHCP_WA_DEFAULT_CLIENT_PORT				SYS_DFLT_DHCP_CLIENT_PORT
#define	DHCP_WA_DEFAULT_SERVER_PORT				SYS_DFLT_DHCP_SERVER_PORT
#define DHCP_WA_DEFAULT_LEASE_TIME				SYS_DFLT_DHCP_LEASE_TIME 
#define	DHCP_WA_DEFAULT_ROLE					DHCP_TYPE_BIND_CLIENT
#define DHCP_WA_DEFAULT_CID_MODE				SYS_DFLT_DHCP_CID_MODE 
#define DHCP_WA_DEFAULT_NETBIOS_NODE_TYPE		SYS_DFLT_DHCP_NETBIOS_NODE_TYPE 
#define DHCP_WA_DEFAULT_CLASSID_MODE            SYS_DFLT_DHCP_CLASSID_MODE

#define DHCP_WA_INFINITE_LEASE_TIME           	0xFFFFFFFF
#define DHCP_WA_MAX_NUMS_OF_OPTIONS				9


#define DHCP_WA_NETBIOS_NODE_TYPE_NONE        	0
#define DHCP_WA_NETBIOS_NODE_TYPE_B_NODE      	1 
#define DHCP_WA_NETBIOS_NODE_TYPE_P_NODE      	2 
#define DHCP_WA_NETBIOS_NODE_TYPE_M_NODE      	4 
#define DHCP_WA_NETBIOS_NODE_TYPE_H_NODE      	8           

/* return value */
#define DHCP_WA_OK										0x0
#define DHCP_WA_FAIL									0x80000001
#define DHCP_WA_IS_NETWORK_IP_OR_NETWORK_BROADCAST		0x80000002
#define DHCP_WA_EXCLUDE_ALL_IP_IN_SUBNET				0x80000003
#define DHCP_WA_HIGH_IP_SMALLER_THAN_LOW_IP				0x80000004
#define DHCP_WA_EXCLUDED_IP_IN_OTHER_SUBNET				0x80000005
#define DHCP_WA_POOL_NOT_EXISTED						0x80000006
#define DHCP_WA_CID_EXCEED_MAX_SIZE	   					0x80000007
#if (SYS_CPNT_DHCP_HOSTNAME == TRUE)
#define DHCP_WA_HOSTNAME_EXCEED_MAX_SIZE	   			0x80000008      /* 2006-07, Joseph */
#endif
#define DHCP_WA_CLASSID_EXCEED_MAX_SIZE	   			    0x80000009      /* 2007-12, Joseph */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */
typedef	struct DHCP_WA_InterfaceDhcpInfo_S
{
	struct DHCP_WA_InterfaceDhcpInfo_S	*next;		/*	link list	*/
	struct DHCP_WA_InterfaceDhcpInfo_S	*previous;
	UI32_T	ifIndex;						/*	DHCP managed Interface index	*/
	BOOL_T	assigned_ip;					/*	TRUE--ip assigned, FALSE--no ip configured	*/
	UI32_T	if_binding_role;				/*	Client/Relay		*/
	UI32_T	if_ip;							/*	0.0.0.0 -- not assigned	*/
	UI32_T	if_server_ip;					/*	0.0.0.0 -- no server ip */
	UI32_T	if_gateway;						/*	0.0.0.0 -- no relay specified	*/
	UI16_T	client_port;					/*	default udp port : 68	*/
	UI16_T	server_port;					/*	default udp port - 67	*/
	UI32_T	relay_server[DHCP_WA_MAX_NBR_OF_RELAY_SERVER]; 	/* the server address for relay agent */	
	DHCP_MGR_ClientId_T	cid;
    DHCP_MGR_Vendor_T   classid;            /* 2007-12, Joseph */
}DHCP_WA_InterfaceDhcpInfo_T; 
 
/* DHCP Server */
typedef struct DHCP_WA_IpExcludeed_S
{
  //struct DHCP_WA_IpExcludeed_S *next;	
   UI32_T  low_excluded_address;            /* the lowest address in the excluded range */
   UI32_T  high_excluded_address;           /* the highest address in the excluded range */
}  DHCP_WA_IpExcluded_T;

#if (SYS_CPNT_DHCP_SERVER == TRUE)
typedef struct DHCP_WA_IpRange_S
{
   UI32_T  low_address;            /* the lowest address in the  range */
   UI32_T  high_address;           /* the highest address in the  range */
}  DHCP_WA_IpRange_T;
#endif

 
/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION	NAME : DHCP_WA_Init
 * PURPOSE:
 *		Initialize DHCP_WA.
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
void	DHCP_WA_Init (void);


/* FUNCTION	NAME : DHCP_WA_ReInit
 * PURPOSE:
 *		Release all resource allocated.
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
void	DHCP_WA_ReInit (void);

#ifdef	SYS_CPNT_DHCP_SERVER
/* FUNCTION	NAME : DHCP_WA_CreatePool
 * PURPOSE:  
 *		Create a pool config, and link it to the pool config link list.
 *
 * INPUT:
 *		pool_name -- pool's name 
 *		
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE -- create successfully
 *		FALSE -- fail to create a pool
 *	
 * NOTES:
 *		
 */
BOOL_T DHCP_WA_CreatePool(char *pool_name);

/* FUNCTION	NAME : DHCP_WA_DeletePool
 * PURPOSE:  
 *		Delete a pool config.
 *
 * INPUT:
 *		pool_name -- pool's name 
 *		
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE -- create successfully
 *		FALSE -- fail to create a pool
 *	
 * NOTES:
 *		1. Before deleting a pool, be sure that it is unlink from Network 
 *			linked list if it is a network pool.
 *		
 */
BOOL_T DHCP_WA_DeletePool(DHCP_TYPE_PoolConfigEntry_T *pool);

/* FUNCTION NAME : DHCP_WA_SetExcludedIp
 * PURPOSE:
 *      To specify the excluded ip address (range) which server should not assign to client.
 *
 * INPUT:
 *      low_address  -- the lowest ip in the excluded ip address range
 * 		high_address -- the highest ip in the excluded ip address range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_WA_OK		-- successfully.
 *		DHCP_WA_FAIL	-- fail to set excluded ip address.	
 *		DHCP_WA_IS_NETWORK_IP_OR_NETWORK_BROADCAST -- fail to set excluded ip as network address
 *												   		or a network broadcast
 *		DHCP_WA_EXCLUDE_ALL_IP_IN_SUBNET -- fail to set excluded ip range as (x.x.x.1 ~ x.x.x.254)
 *		DHCP_WA_HIGH_IP_SMALLER_THAN_LOW_IP	-- fail to set excluded ip range that high ip smaller
 *												than low ip
 *		DHCP_WA_EXCLUDED_IP_IN_OTHER_SUBNET	-- fail to set excluded ip in other subnet
 *
 * NOTES:
 *		1. To specify exluded ip address range, the high-address is required.
 *		Otherwise, only specify low-address is treated as excluding an address 
 *		from available address range.
 *		2. (low_address != 0, high_address == 0) will treat low_address the only
 *		address to be excluded.
 *		3. In hornet, only can specify one ip excluded range, but here, 
 *		we remove the restriction
 */
UI32_T DHCP_WA_SetExcludedIp(UI32_T low_address, UI32_T high_address);

/* FUNCTION	NAME : DHCP_WA_DelExcludedIp
 * PURPOSE:
 *		This function deletes the IP address(es) that the DHCP server 
 *		should not assign to DHCP clients. 
 *
 * INPUT:
 *		low_address  -- the lowest ip address of the excluded ip range
 *		high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_WA_OK -- successfully delete the specified excluded ip.
 *		DHCP_WA_FAIL -- FAIL to delete the specified excluded ip.
 *
 * NOTES:
 *		
 */
UI32_T DHCP_WA_DelExcludedIp(UI32_T low_address, UI32_T high_address);

/* FUNCTION	NAME : DHCP_WA_GetNextIpExcluded
 * PURPOSE:
 *		This functin retrives the IP addresses that the DHCP server 
 *		should not assign to DHCP clients. 
 *
 * INPUT:
 *		 ip_address  --low-address[high-address]
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE/FALSE
 *
 * NOTES:
 *		
 */
BOOL_T  DHCP_WA_GetNextIpExcluded(DHCP_WA_IpExcluded_T  *ip_address);

/* FUNCTION	NAME : DHCP_WA_GetIpExcluded
 * PURPOSE:
 *		This functin retrives the IP addresses that the DHCP server
 *		should not assign to DHCP clients.
 *
 * INPUT:
 *		 ip_address  --low-address[high-address]
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE/FALSE
 *
 * NOTES:
 *
 */
BOOL_T  DHCP_WA_GetIpExcluded(DHCP_WA_IpExcluded_T  *ip_address);

/* FUNCTION	NAME : DHCP_WA_FindPoolByPoolName
 * PURPOSE: 
 *		Search pool config table by pool's name.
 *
 * INPUT:
 *		pool_name -- pool's name to be the key to search		
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL -- Can't find any pool config by specified pool's name
 *		others -- the pointer pointed to the pool config table 
 *
 * NOTES:
 *		
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_FindPoolByPoolName(char *pool_name);

/* FUNCTION	NAME : DHCP_WA_GetPoolConfigEntryByPoolName
 * PURPOSE:
 *		Get the pool config by pool name.
 *
 * INPUT:
 *		pool_name -- the name of the pool to be searched.
 * OUTPUT:
 *		pool_config -- the pointer to the pool config structure
 *
 * RETURN:
 *		TRUE --  successfully.
 *
 * NOTES:
 * 		
 *
 */
BOOL_T DHCP_WA_GetPoolConfigEntryByPoolName(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config_p);

/* FUNCTION	NAME : DHCP_WA_GetParentPoolbyIpMaskForHost
 * PURPOSE:
 *		Get the parent pool (network pool) by host ip and subnet mask.
 *
 * INPUT:
 *		ip_address  -- the host IP Address
 *		sub_netmask -- the host sub_netmask 
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		tmp_pool -- the pointer to network pool as a parent to the given host pool.
 *		NULL	-- can't find the pool.
 *
 * NOTES:
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetParentPoolbyIpMaskForHost(UI32_T ip_address, UI32_T sub_netmask);

/* FUNCTION	NAME : DHCP_WA_GetNextNetworkPoolbyIpMask
 * PURPOSE:
 *		Get the next network pool by ip and subnet mask.
 *
 * INPUT:
 *		ip_address
 *		sub_netmask
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		tmp_pool -- the pointer to the pool which (ip+mask)is smaller than the given (ip+mask)
 *		NULL	-- can't find the pool.
 *
 * NOTES:
 *		
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNextNetworkPoolbyIpMask(UI32_T ip_address, UI32_T sub_netmask);

/* FUNCTION	NAME : DHCP_WA_GetNetworkPoolbyIpMask
 * PURPOSE:
 *		Get the network pool by ip and subnet mask.
 *
 * INPUT:
 *		ip_address
 *		sub_netmask
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		tmp_pool -- the pointer to the pool which (ip+mask)is equal to the given (ip+mask).
 *		NULL	-- can't find the pool.
 *
 * NOTES:
 *		
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNetworkPoolbyIpMask(UI32_T ip_address, UI32_T sub_netmask);

/* FUNCTION	NAME : DHCP_WA_GetNextPoolConfigEntry
 * PURPOSE:
 *		Get the next pool
 *
 * INPUT:
 *		pool_name	-- the pool name as a key to search in sorted list
 *		pool_config	-- the pointer to the pool.
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- Successfully get the value
 *		FALSE	-- Fail to get next
 *
 * NOTES:
 * 		
 *
 */
BOOL_T DHCP_WA_GetNextPoolConfigEntry(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config);

/* FUNCTION	NAME : DHCP_WA_GetHostPoolbyPoolName
 * PURPOSE:
 *		Get the host pool by pool name
 *
 * INPUT:
 *		pool_name -- the name of the pool to search
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		tmp_pool -- the pointer to the pool according to the pool name 
 *		NULL	-- can't find the pool.
 *
 * NOTES:
 *		
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetHostPoolbyPoolName(char *pool_name);

/* FUNCTION	NAME : DHCP_WA_GetNextHostPoolbyPoolName
 * PURPOSE:
 *		Get the next host pool by pool name
 *
 * INPUT:
 *		pool_name -- the name of the pool; If pool_name = NULL, get the 1st host pool
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		tmp_pool -- the pointer to the next pool following the input pool. 
 *		NULL	-- can't find the pool.
 *
 * NOTES:
 *		
 */
DHCP_TYPE_PoolConfigEntry_T* DHCP_WA_GetNextHostPoolbyPoolName(char *pool_name);
#if 0
/* FUNCTION	NAME : DHCP_WA_PoolInheritance
 * PURPOSE: Check option inheritance issue in pool config (WA)
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
 *		1. This function will be called each time DHCP Server Restart
 */
void DHCP_WA_PoolInheritance();
#endif
/* FUNCTION	NAME : DHCP_WA_GetNetworkRange
 * PURPOSE:
 *		Calculate out the available range in a subnet by excluding user specified ip 
 *		or ip range.
 *
 * INPUT:
 *		ip   -- ip address (either a host address or a network address)
 *		mask -- subnet mask
 * 
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 * NOTES:
 *		None.
 */ 
void DHCP_WA_GetNetworkRange(struct subnet *subnet, UI32_T  network_address, UI32_T  sub_netmask);

/* FUNCTION NAME :DHCP_WA_ParseOptionConfig
 * PURPOSE:
 *		Move all the options config in specified subnet to Server OM (memory.c)
 * INPUT:
 *		group -- group structure to hold options config.
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
void DHCP_WA_ParseOptionConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config, struct group *group);

/* FUNCTION	NAME : DHCP_WA_UnLinkFromNetworkPoolSortedList
 * PURPOSE:
 *
 *
 * INPUT:
 *		pool_config -- the pointer to the pool that will be unlinked from
 *				network sorted list.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_WA_OK -- Successfully link to network sorted list.
 *		DHCP_WA_FAIL -- Fail to link to network sorted list.
 *		DHCP_WA_POOL_NOT_EXISTED -- Pool is not existed.
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_WA_UnLinkFromNetworkPoolSortedList(DHCP_TYPE_PoolConfigEntry_T *pool_config);

/* FUNCTION	NAME : DHCP_WA_UnlinkPoolFromPoolConfig
 * PURPOSE: 
 *		Unlink Pool from pool config but don't free it
 *
 * INPUT:
 *		pool_config -- the specified pool for unlinking
 *
 * OUTPUT:
 *		DHCP_WA_OK	-- successfully unlink the specified pool.
 *		DHCP_WA_FAIL -- Fail to do so.
 *
 * RETURN:
 *		NULL -- Can't find any pool config by specified pool's name
 *		others -- the pointer pointed to the pool config table 
 *
 * NOTES:
 *		
 */
UI32_T DHCP_WA_UnlinkPoolFromPoolConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config);

/* FUNCTION	NAME : DHCP_WA_LinkToNetworkPoolSortedList
 * PURPOSE:
 *
 *
 * INPUT:
 *		pool_config -- the pointer to the pool that will be added into
 *				network sorted list.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_WA_OK -- Successfully link to network sorted list
 *		DHCP_WA_FAIL -- Fail to link to network sorted list
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_WA_LinkToNetworkPoolSortedList(DHCP_TYPE_PoolConfigEntry_T *pool_config);

/* FUNCTION NAME :DHCP_WA_GetNumbersOfHostPool
 * PURPOSE:
 *		Get the total numbers of host pool in WA.
 * INPUT:
 *		num -- the pointer to point the numbers of host pool.
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
void DHCP_WA_GetNumbersOfHostPool(int *num);

/* FUNCTION NAME :DHCP_WA_SetNumbersOfHostPool
 * PURPOSE:
 *		Set the numbers of host pool in WA.
 * INPUT:
 *		num -- the numbers of host pool.
 *		increase -- TRUE: increase host pool number
 *					FALSE: decrease host pool number
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
void DHCP_WA_SetNumbersOfHostPool(int num, BOOL_T increase);

#endif /* end of #ifdef	SYS_CPNT_DHCP_SERVER */

/* FUNCTION	NAME : DHCP_WA_SetIfBindingRole
 * PURPOSE:
 *		Define interface role in DHCP.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | relay }
 *						DHCP_TYPE_INTERFACE_BIND_CLIENT
 *						DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully set the interace-role.
 *		FALSE 	-- no such interface.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 */
BOOL_T DHCP_WA_SetIfBindingRole(UI32_T vid_ifIndex,UI32_T role);

/* FUNCTION	NAME : DHCP_WA_DeleteIfBindingRole
 * PURPOSE:
 *		Remove interface role in DHCP. And set it to the default role.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | relay }
 *						DHCP_TYPE_INTERFACE_BIND_CLIENT
 *						DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully set the interace-role.
 *		FALSE 	-- no such interface.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 */
BOOL_T DHCP_WA_DeleteIfBindingRole(UI32_T vid_ifIndex,UI32_T role);

/* FUNCTION	NAME : DHCP_WA_C_SetIfClientId
 * PURPOSE:
 *		Define interface CID for client.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		id_mode 	-- the client-port of dhcp.
 *		id_len 		-- the len of buffer.
 *		id_buf		-- the content of the cid.	
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_WA_OK	 -- Successful to set cid
 *		DHCP_WA_FAIL -- No more space in allocating memory
 *		DHCP_WA_CID_EXCEED_MAX_SIZE -- cid exceeds the max size
 *
 * NOTES:
 *		
 */
UI32_T DHCP_WA_C_SetIfClientId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf);

/* FUNCTION	NAME : DHCP_WA_C_GetIfClientId
 * PURPOSE:
 *		Retrieve the CID associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		cid_p		-- the structure of CID 
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_WA_C_GetIfClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/* FUNCTION	NAME : DHCP_WA_AddIfRelayServerAddress
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
BOOL_T DHCP_WA_AddIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T ip_list[]);

/* FUNCTION	NAME : DHCP_WA_GetRelayServiceCount 
 * PURPOSE:
 *		Get all relay service count.
 *
 * INPUT:
 *		NONE.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		relay_service_count -- the numbers of relay configured in the system.
 *
 * NOTES:
 *		None.
 */
int DHCP_WA_GetRelayServiceCount(void);

/* FUNCTION	NAME : DHCP_WA_GetIfRelayServerAddress
 
 * PURPOSE:
 *		Get all interface relay server addresses associated with interface.
 *
 * INPUT:
 *		vid_ifIndex  -- the interface to be searched for relay server address.
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully get
 *		FALSE	-- Fail to get server ip to interface
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_WA_GetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T *relay_server);

/* FUNCTION	NAME : DHCP_WA_DeleteAllIfRelayServerAddress
 * PURPOSE:
 *		Delete all interface relay server addresses associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully delete
 *		FALSE	-- Fail to delete server ip to interface
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_WA_DeleteAllIfRelayServerAddress(UI32_T vid_ifIndex);
#endif /* end of #if (SYS_CPNT_DHCP_RELAY == TRUE)*/

/* FUNCTION	NAME : DHCP_WA_C_SetIfGateway
 * PURPOSE:
 *		Save interface association setting.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		gate_iip	-- relay agent ip.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully saved
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfGateway (UI32_T vid_ifIndex, UI32_T gate_ip);


/* FUNCTION	NAME : DHCP_WA_C_SetIfServer
 * PURPOSE:
 *		Save interface association setting.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		svr_iip		-- dhcp server's ip.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully saved
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfServer(UI32_T vid_ifIndex, UI32_T svr_ip);



/* FUNCTION	NAME : DHCP_WA_C_SetIfIp
 * PURPOSE:
 *		Save interface association setting.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		if_ip		-- dhcp client's pre-assigned ip.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully saved
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 *
 */
BOOL_T DHCP_WA_C_SetIfIp(UI32_T vid_ifIndex, UI32_T if_ip);


/* FUNCTION	NAME : DHCP_WA_SetIfClientPort
 * PURPOSE:
 *		Save interface association setting.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		udp_port_no -- the client-port of dhcp.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully saved
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. As RFC specified, default client port is 68 if user not changed.
 *
 */
BOOL_T DHCP_WA_SetIfClientPort(UI32_T vid_ifIndex, UI32_T udp_port_no);


/* FUNCTION	NAME : DHCP_WA_SetIfServerPort
 * PURPOSE:
 *		Save interface association setting.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		udp_port_no -- the server-port of dhcp.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	-- successfully saved
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. As RFC specified, default client port is 67 if user not changed.
 */
BOOL_T DHCP_WA_SetIfServerPort(UI32_T vid_ifIndex, UI32_T if_ip);


/* FUNCTION	NAME : DHCP_WA_GetIfPort
 * PURPOSE:
 *		Retrieve dhcp client and server port, udp port.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		client_port -- client-port associated with this interface.
 *		server_port	-- server-port associated with this interface.
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. If not specified, use RFC definition (67,68)
 */
BOOL_T DHCP_WA_GetIfPort(UI32_T vid_ifIndex, UI32_T *client_port, UI32_T *server_port);


/* FUNCTION	NAME : DHCP_WA_GetIfConfig
 * PURPOSE:
 *		Retrieve assigned-ip, server-ip, and gateway-ip associated with
 *		interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		if_config	-- the interface info for that specific vid_ifIndex
 *
 * OUTPUT:
 *		if_config	-- the interface info for that specific vid_ifIndex		
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		
 */
BOOL_T DHCP_WA_GetIfConfig(UI32_T vid_ifIndex, DHCP_WA_InterfaceDhcpInfo_T* if_config_p);

/* FUNCTION	NAME : DHCP_WA_GetNextIfConfig
 * PURPOSE:
 *		Retrieve assigned-ip, server-ip, and gateway-ip associated with
 *		interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		if_ip		--	interface ip.
 *		server_ip	--	server ip
 *		gate_ip		--	gateway ip.
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		1. If vid_ifIndex == 0, get the 1st if_config.
 */
BOOL_T DHCP_WA_GetNextIfConfig(UI32_T vid_ifIndex,DHCP_WA_InterfaceDhcpInfo_T* if_config_p);

/* FUNCTION	NAME : DHCP_WA_GetIfBindingRole
 * PURPOSE:
 *		Retrieve the role associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		role	-- binding role.
 *					DHCP_TYPE_INTERFACE_BIND_CLIENT
 *					DHCP_TYPE_INTERFACE_BIND_SERVER
 *					DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * RETURN:
 *		TRUE	-- successfully gotten
 *		FALSE	-- no this interface.
 *
 * NOTES:
 *		None.
 */
BOOL_T DHCP_WA_GetIfBindingRole(UI32_T vid_ifIndex, UI32_T *role);

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
void    DHCP_WA_EnableDhcp (void);


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
void    DHCP_WA_EnableBootp (void);


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
 *      TRUE	--	disable DHCP
 *		FALSE	--	previous setting is not DHCP, maybe BOOTP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable DHCP, the interface MUST be configured manually.
 */
BOOL_T    DHCP_WA_DisableDhcp (void);


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
 *      TRUE	--	disable BOOTP
 *		FALSE	--	previous setting is not BOOTP, maybe DHCP or USER-DEFINE.
 *
 * NOTES:
 *      1. After disable BOOTP, the interface MUST be configured manually.
 */
BOOL_T    DHCP_WA_DisableBootp (void);


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
 *		| DHCP_TYPE_INTERFACE_MODE_DHCP }
 *
 * NOTES:
 */
UI32_T DHCP_WA_GetObjectMode (void);

/* FUNCTION NAME : DHCP_WA_GetRestartObject
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
 *		TRUE /FALSE
 *
 * NOTES:
 *		Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *		DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 */
BOOL_T DHCP_WA_GetRestartObject(UI32_T *restart_object);

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
 *		None.
 *
 * NOTES:Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *		DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 *		1. 
 */
BOOL_T DHCP_WA_SetRestartObject(UI32_T restart_object);

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
 *		None.
 *
 * NOTES:
 *		1. This function will be called while during restart process; when one object
 *			has been restarted, it needs to clear out after that.
 *		2. Possible restart values:
 *      DHCP_TYPE_RESTART_NONE | DHCP_TYPE_RESTART_CLIENT
 *		DHCP_TYPE_RESTART_SERVER | DHCP_TYPE_RESTART_RELAY
 */
void DHCP_WA_ClearRestartObject(UI32_T restart_object);

/* FUNCTION	NAME : DHCP_WA_SearchIfInList
 * PURPOSE:
 *		Search this interface, check whether exist ?
 *
 * INPUT:
 *		ifIndex -- the interface to be searched.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		NULL	--	the interface not in the list.
 *		others	--	the block associated with the interface in list.
 *
 * NOTES:
 *		None.
 */
DHCP_WA_InterfaceDhcpInfo_T* DHCP_WA_SearchIfInList(UI32_T ifIndex);

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
UI32_T DHCP_WA_C_SetIfVendorClassId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf);

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
BOOL_T DHCP_WA_C_GetIfVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p);
BOOL_T DHCP_WA_DestroyIfDhcpInfo (UI32_T ifIndex);

#if (SYS_CPNT_DHCP_SERVER == TRUE)
/*add by simon*/
BOOL_T DHCP_WA_GetNextActivePool(DHCP_WA_IpRange_T * pool_range);
void DHCP_WA_InitActivePool();
#endif

/* end 2007-12 */
#endif	 /*	_DHCP_WA_H */
