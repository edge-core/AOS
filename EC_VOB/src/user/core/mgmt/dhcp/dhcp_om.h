/* Module Name:	DHCP_OM.H
 * Purpose:
 *		DHCP_OM holds all data structures used in run-time, includes
 *			protocol-list, which keeps socket associating with interface.
 *			interface-list, which keeps interfaces managed by DHCP.
 *
 * Notes:
 *		None.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.24	--	William, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */



#ifndef		_DHCP_OM_H
#define		_DHCP_OM_H


/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"
#include "dhcp_type.h"



#define DHCP_OM_MSGBUF_TYPE_SIZE sizeof(union DHCP_OM_IPCMsg_Type_U)

#define DHCP_OM_GET_MSG_SIZE(field_name)                       \
            (DHCP_OM_MSGBUF_TYPE_SIZE +                        \
            sizeof(((DHCP_OM_IPCMsg_T*)0)->data.field_name))

typedef struct
{
    union DHCP_OM_IPCMsg_Type_U
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        I32_T result_i32;  /*respond ui32 return*/ 
 
    }type;
    
    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];

        struct 
        {
            UI32_T relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER];
        }DHCP_Relay_Option82_Relay_Server_data;

        struct
        {
            UI8_T  rid_value[SYS_ADPT_MAX_LENGTH_OF_RID + 1];
        }DHCP_Relay_Option82_Rid_Value_data;

#if (SYS_CPNT_DHCP_RELAY == TRUE)
        struct 
        {
            UI32_T vid_ifindex;
            UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER];
        }DHCP_Relay_If_Relay_Server_data;
#endif
    } data; /* contains the supplemntal data for the corresponding cmd */
}DHCP_OM_IPCMsg_T;

typedef	struct DHCP_OM_LCB_S
{
	UI32_T	object_mode;			/*	whole CSC mode -- DHCP/BOOTP/USER_DEFINE	*/
	UI32_T system_role;				/*  Server, Server+Client, Relay+Client */
	int system_relay_count;
	int system_client_count;
}	DHCP_OM_LCB_T;

typedef struct DHCP_OM_Iaddr_S
{
    UI8_T  iabuf[16];
    UI32_T len;
}DHCP_OM_Iaddr_T;

typedef struct DHCP_OM_UC_Lease_Data_S
{
    DHCP_OM_Iaddr_T address;    /* lease address*/
    UI32_T vid_ifindex;         /* vlan id ifindex */
    UI32_T expiry;              /* expiry time */
    UI32_T renewal;             /* renew time */
    UI32_T rebind;              /* rebind time */
    BOOL_T is_bootp;            /* is bootp lease or not */
}DHCP_OM_UC_LEASE_DATA_T;

typedef struct DHCP_OM_UC_DATA_S
{
    UI32_T magic_word;                  /* DHCP UC magic word */
    DHCP_OM_UC_LEASE_DATA_T lease_data; /* DHCP lease data */
}DHCP_OM_UC_DATA_T;

enum
{
    DHCP_OM_IPCCMD_GET_OPTION82_STATUS,
    DHCP_OM_IPCCMD_GET_OPTION82_POLICY,
    DHCP_OM_IPCCMD_GET_OPTION82_RID_MODE,
    DHCP_OM_IPCCMD_GET_OPTION82_RID_VALUE,
    DHCP_OM_IPCCMD_GET_RELAY_SERVER_ADDRESS,
    DHCP_OM_IPCCMD_GET_OPTION82_FORMAT,
#if (SYS_CPNT_DHCP_RELAY == TRUE)
    DHCP_OM_IPCCMD_GET_IF_RELAY_SERVER_ADDRESS,
#endif
    DHCP_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC
};


/* NAME	CONSTANT DECLARATIONS
 */

/*  Function returned value */
#define DHCP_OM_OK							0
#define DHCP_OM_INTERFACE_NOT_EXISTED      	0x80000001
#define DHCP_OM_INVALID_ARG                 0x80000002
#define DHCP_OM_FAIL                        0x80000008

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */
typedef	struct	DHCP_OM_INTERFACE_INFO_S
{
	struct	DHCP_OM_INTERFACE_INFO_S	*next;
	UI32_T	ifIndex;	/*	which interface associated	*/
	UI32_T	rif_num;	/*	associated network interface*/
	UI32_T	flags;		/*	Request, Automatic	*/
	void	*rx_mref_list;
}	DHCP_OM_INTERFACE_INFO_T;


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION	NAME : DHCP_OM_Init
 * PURPOSE:
 *		Initialize DHCP_OM software components at system starting..
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
void	DHCP_OM_Init (void);


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
void	DHCP_OM_ReInit (UI32_T restart_object);

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
void DHCP_OM_ReInitAllDatabases();

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
BOOL_T DHCP_OM_FreeClientState(struct client_state *client);

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
BOOL_T DHCP_OM_FreeClientLease(struct client_lease *lease);

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
BOOL_T DHCP_OM_FreeClientConfig(struct client_config *config);

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
BOOL_T DHCP_OM_FreeRejectList(struct iaddrlist *reject_list);

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
BOOL_T	DHCP_OM_CreateIf(UI32_T ifIndex);

BOOL_T DHCP_OM_DestroyIf(UI32_T ifIndex);


/* FUNCTION	NAME : DHCP_OM_SetIfFlags
 * PURPOSE:
 *		set flags to interface info
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		flags -- INTERFACE_REQUESTED 1
 			  -- INTERFACE_AUTOMATIC 2 
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		
 
void DHCP_OM_SetIfFlags(UI32_T vid_ifIndex, UI32_T flags);
*/

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

void DHCP_OM_SetIfHwAddress(UI32_T vid_ifIndex, struct hardware hw_address);

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
void DHCP_OM_GetIfVlanByMac(UI8_T *mac, UI32_T *vid_ifIndex);


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
void DHCP_OM_SetIfRbuf(UI32_T vid_ifIndex, int rfdesc);

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
void DHCP_OM_SetIfClientState(UI32_T vid_ifIndex, struct client_state *client);

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
void	DHCP_OM_SetIfPort (UI32_T vid_ifIndex, UI32_T client_port, UI32_T server_port);

/* FUNCTION	NAME : DHCP_OM_SetIfBindingRole
 * PURPOSE:
 *		Define interface role in DHCP.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | server | relay }
 *						DHCP_TYPE_INTERFACE_BIND_CLIENT
 *						DHCP_TYPE_INTERFACE_BIND_SERVER
 *						DHCP_TYPE_INTERFACE_BIND_RELAY
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
void DHCP_OM_SetIfBindingRole(UI32_T vid_ifIndex,UI32_T role);

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
void DHCP_OM_SetIfClientId(UI32_T vid_ifIndex, DHCP_TYPE_ClientId_T cid);

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
UI32_T DHCP_OM_GetIfClientId(UI32_T vid_ifIndex, DHCP_TYPE_ClientId_T *cid_p);

/* begin 2007-12, Joseph */
/* FUNCTION	NAME : DHCP_OM_SetIfVendorClassId
 * PURPOSE:
 *		Set class id associated with interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		classid			-- the structure of CLASS ID
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
void DHCP_OM_SetIfVendorClassId(UI32_T vid_ifIndex, DHCP_TYPE_Vendor_T classid);

/* end 2007-12 */

/* FUNCTION	NAME : DHCP_OM_SetIfRelayServer
 * PURPOSE:
 *		Set DHCP server IP address list for relay agent
 *
 * INPUT:
 *		vid_ifIndex 	-- the interface to be defined.
 *		relay_server	-- a list of IP address for dhcp server
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE / FALSE
 *
 * NOTES:
 *		
 */
BOOL_T DHCP_OM_SetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T *relay_server_list);


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
void DHCP_OM_SetClientIfConfig(UI32_T vid_ifIndex, UI32_T if_ip, UI32_T server_ip, UI32_T gate_ip);


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
BOOL_T DHCP_OM_GetIfPort (UI32_T vid_ifIndex, UI32_T *client_port, UI32_T *server_port);

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
BOOL_T DHCP_OM_SetIfIp(UI32_T vid_ifIndex, UI32_T ip_address);

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
BOOL_T DHCP_OM_IsIfGotIp (UI32_T vid_ifIndex);


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
struct interface_info *DHCP_OM_FindInterfaceByVidIfIndex(UI32_T vid_ifIndex);


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
struct interface_info *DHCP_OM_GetNextInterface(struct interface_info *if_ptr_addr);

BOOL_T DHCP_OM_GetClientDefaultGateway(UI8_T gateway[4]);
BOOL_T DHCP_OM_SetClientDefaultGateway(UI8_T gateway[4]);
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
void    DHCP_OM_EnableDhcp (void);


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
void    DHCP_OM_EnableBootp (void);



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
BOOL_T    DHCP_OM_DisableDhcp (void);


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
BOOL_T    DHCP_OM_DisableBootp (void);

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
UI32_T	DHCP_OM_GetMode();

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
BOOL_T DHCP_OM_GetIfMode(UI32_T vid_ifIndex, UI32_T *address_mode);

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
BOOL_T DHCP_OM_SetIfMode(UI32_T vid_ifIndex, UI32_T address_mode);

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
 * NOTES: None.
 *      
 */
BOOL_T DHCP_OM_SetSystemRole(UI32_T role);

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
BOOL_T DHCP_OM_GetSystemRole(UI32_T *role);

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
 *      TRUE 	-- Successfully remove the system role
 *		FALSE   -- Can't remove the system role
 *
 * NOTES: None.
 *      
 */
BOOL_T DHCP_OM_RemoveSystemRole(UI32_T role);

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
BOOL_T DHCP_OM_IsServerOn();

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
BOOL_T DHCP_OM_IsRelayOn();

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
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_OM_SetDhcpRelayOption82Status(UI32_T status);

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
UI32_T DHCP_OM_GetDhcpRelayOption82Status(UI32_T *status);

/* FUNCTION NAME : DHCP_OM_SetDhcpRelayOption82Policy
 * PURPOSE:
 *     Set dhcp relay option 82 policy
 *
 * INPUT:
 *      status     --  DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
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
UI32_T DHCP_OM_SetDhcpRelayOption82Policy(UI32_T policy);

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82Policy
 * PURPOSE:
 *     Get dhcp relay option 82 policy
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
UI32_T DHCP_OM_GetDhcpRelayOption82Policy(UI32_T *policy);

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
UI32_T DHCP_OM_SetDhcpRelayOption82RidMode(UI32_T mode);

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
UI32_T DHCP_OM_GetDhcpRelayOption82RidMode(UI32_T *mode);

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
UI32_T DHCP_OM_SetDhcpRelayOption82RidValue(UI8_T *string);

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
UI32_T DHCP_OM_GetDhcpRelayOption82RidValue(UI8_T *string);

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
UI32_T DHCP_OM_SetDhcpRelayOption82Format(BOOL_T subtype_format);

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
UI32_T DHCP_OM_GetDhcpRelayOption82Format(BOOL_T *subtype_format);


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
UI32_T DHCP_OM_AddRelayServerAddress(UI32_T ip_list[]);

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
UI32_T DHCP_OM_DeleteGlobalRelayServerAddress();

/* FUNCTION	NAME : DHCP_OM_GetRelayServerAddress
 
 * PURPOSE:
 *		Get global relay server addresses.
 *
 * INPUT:
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully get
 *		DHCP_OM_FAIL	-- Fail to get server ip to interface
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_OM_GetRelayServerAddress(UI32_T *relay_server);
#endif
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
BOOL_T DHCP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

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
BOOL_T DHCP_OM_GetLcbInformation(DHCP_OM_LCB_T* om_lcb);

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
BOOL_T DHCP_OM_SetUCLeaseData(DHCP_OM_UC_LEASE_DATA_T *uc_data);

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
BOOL_T DHCP_OM_GetUCLeaseData(DHCP_OM_UC_LEASE_DATA_T *uc_data);

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
void DHCP_OM_UpdateUCData(void);

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
void DHCP_OM_SetIfDhcpInform(UI32_T vid_ifIndex, BOOL_T do_enable);
#endif /* SYS_CPNT_DHCP_INFORM */

#endif	 /*	_DHCP_OM_H */
