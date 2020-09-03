/* Module Name:	MEMORY.H
 * Purpose:
 *			Memory.c is DHCP Server database which contains lease and server configuration which
 *			currently running in the system
 *
 * Notes:
 *		This file is porting from Hornet IAD code.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2002.08.26	--	Penny, Created
 *
 * Copyright(C)		 Accton	Corporation, 2000, 2001, 2002.
 */



#ifndef		_MEMORY_H
#define		_MEMORY_H
#include "dhcp_type.h"
#include "hash.h"

#ifdef	SYS_CPNT_DHCP_SERVER
typedef struct DHCP_MEMORY_Server_Lease_Config_S
{
	struct DHCP_MEMORY_Server_Lease_Config_S *next;
	UI32_T   lease_ip;
	UI8_T    hardware_address[SYS_ADPT_MAC_ADDR_LEN];
	//UI8_T    client_hostname[SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN];
	UI32_T 	 lease_time; /* in sec */
	UI32_T	 start_time; /* in sec: show current system real time */
}DHCP_MEMORY_Server_Lease_Config_T;

void DHCP_MEMORY_Init();
/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_ReInit
 * ------------------------------------------------------------------------
 * FUNCTION : Reinitialize server OM software components when DHCP must
 *			  rebuild its working OM. It's similiar as a process
 *			  restarting, but no process resource allocation be done.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
void DHCP_MEMORY_ReInit();

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeHashBuckets
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to clear up all hash buckets
 *				in hash table.
 * INPUT    : hash bucket -- the pointer to the hash_bucket structure that
 *					needed to be freed.
 *			  type		  -- to indicate hash table whether for a host decl or
 *					a least structure.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeHashBuckets(struct hash_bucket *hash_bucket, int type);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeHashTable
 * ------------------------------------------------------------------------
 * FUNCTION : Free hash_table data structure.
 * INPUT    : hash_table -- the pointer to the hash_table structure that
 *					needed to be freed.
 *			  type		 -- to indicate hash table whether for a host decl or
 *					a least structure.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeHashTable(struct hash_table *hash_table, int type);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeLease
 * ------------------------------------------------------------------------
 * FUNCTION : Free lease data structure.
 * INPUT    : lease -- the pointer to the lease structure that
 *					needed to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeLease(struct lease *lease);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeLeaseState
 * ------------------------------------------------------------------------
 * FUNCTION : Free lease_state structure.
 * INPUT    : state -- the pointer to the lease_state structure that
 *					needed to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeLeaseState(struct lease_state *state);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeHostDecl
 * ------------------------------------------------------------------------
 * FUNCTION : Free host_decl structure.
 * INPUT    : host -- the pointer to the host_decl structure that
 *					needed to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeHostDecl(struct host_decl *host);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeGroup
 * ------------------------------------------------------------------------
 * FUNCTION : Free group data structure.
 * INPUT    : group -- the pointer to the group structure that
 *					needed to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeGroup(struct group *group);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeSharedNetworks
 * ------------------------------------------------------------------------
 * FUNCTION : Free shared_network data structure.
 * INPUT    : share -- the pointer to the shared_network structure that
 *					needed to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeSharedNetworks(struct shared_network *share);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_FreeSubnets
 * ------------------------------------------------------------------------
 * FUNCTION : Free subnet data structure.
 * INPUT    : subnet_to_be_free -- the subnet structure to be freed.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_FreeSubnets(struct subnet *subnet_to_be_free);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_WriteIpBindingLease
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to keep Ip binding lease in memory.c
 *				currently been called in dhcp_reply().
 * INPUT    : lease -- the pointer to the lease structure that will be
 *						saved in server OM for IP binding.

 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully write ip binding
 *			  FALSE -- Fail to write ip binding
 * NOTE     : None

 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_WriteIpBindingLease(struct lease *lease);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_GetIpBindingLease
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to get Ip binding lease in memory.c
 *
 * INPUT    : ip_address -- IP address as a key to search in ip binding
 *							lease.
 *			  binding	 -- ip lease binding.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully get the value
 *			  FALSE -- Fail to get the value
 * NOTE     : None

 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_GetIpBindingLease(UI32_T ip_address, DHCP_MEMORY_Server_Lease_Config_T *binding);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_GetNextIpBindingLease
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to get Ip binding lease in memory.c
 *
 * INPUT    : ip_address -- IP address as a key to search in ip binding
 *							lease; ip_address = 0 to get the 1st lease.
 *			  binding	 -- ip lease binding.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully get the value
 *			  FALSE -- Fail to get the value
 * NOTE     : None

 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_GetNextIpBindingLease(UI32_T ip_address, DHCP_MEMORY_Server_Lease_Config_T *binding);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_ClearIpBinding
 * ------------------------------------------------------------------------
 * FUNCTION : Free IP binding lease.
 * INPUT    : ip_address -- the ip to be searched as a key in IP binding
 *							if ip_address == 0, means to delete all IP
 *							bindings.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully free the pointer of the structure
 *			  FALSE -- Fail to free the pointer of the structure
 * NOTE     : This function just resets the date structure in server OM.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_ClearIpBinding(UI32_T ip_address);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_UpdateIpBindingForDhcpRelease
 * ------------------------------------------------------------------------
 * FUNCTION : Update Ip Binding when receiving dhcprelease packet.
 * INPUT    : ip_address -- the ip to be searched as a key in IP binding.
 * OUTPUT   : None
 * RETURN   : TRUE  -- Successfully update ip binding
 *			  FALSE -- Fail to update ip binding
 * NOTE     : This function will be called while dhcp server receive
 *				dhcprelease.
 * ------------------------------------------------------------------------
 */
BOOL_T DHCP_MEMORY_UpdateIpBindingForDhcpRelease(UI32_T ip_address);


void enter_host(struct host_decl *hd);
struct host_decl *find_hosts_by_haddr(int htype, unsigned char *haddr, int hlen);
struct host_decl *find_hosts_by_uid(unsigned char *data, int len);
struct subnet *find_host_for_network(struct host_decl **host, struct iaddr *addr, struct shared_network *share);
void new_address_range(struct iaddr low, struct iaddr high, struct subnet *subnet, int dynamic,UI32_T* low_active_ip, UI32_T* high_active_ip);
struct subnet *find_subnet(struct iaddr addr);
struct subnet *find_grouped_subnet(struct shared_network *share, struct iaddr addr);
int subnet_inner_than(struct subnet *subnet, struct subnet *scan, int warnp);
void enter_subnet(struct subnet *subnet);
void enter_shared_network(struct shared_network *share);
void enter_lease(struct lease *lease);
int supersede_lease(struct lease *comp, struct lease *lease, int commit);
void release_lease(struct lease *lease);
void abandon_lease(struct lease *lease, char *message);
struct lease *find_lease_by_ip_addr(struct iaddr addr);
struct lease *find_lease_by_uid(unsigned char *uid, int len);
struct lease *find_lease_by_hw_addr(unsigned char *hwaddr, int hwlen);
void uid_hash_add(struct lease *lease);
void uid_hash_delete(struct lease *lease);
void hw_hash_add(struct lease *lease);
void hw_hash_delete(struct lease *lease);
struct class *add_class(int type, char *name);
struct class *find_class(int type, unsigned char *name, int len);
struct group *clone_group(struct group *group, char *caller);
void write_leases();
int write_lease(struct lease *l);
int commit_leases ();
void dump_subnets();

#endif /* #ifdef	SYS_CPNT_DHCP_SERVER */

#endif /* _MEMORY_H */
