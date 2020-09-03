/* memory.c

   Memory-resident database... */

/*
 * Copyright (c) 1995, 1996, 1997, 1998 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */


//#include "dhcpd.h"

#include "memory.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "hash.h"
#include "dhcp_type.h"
#include "dhcp_algo.h"
#include "alloc.h"
#include "sys_time.h"
#include "l_sort_lst.h"
#include "sys_cpnt.h"
#include "tree.h"
//#include "time.h"
#include "dhcp_error_print.h"

#if	(SYS_CPNT_DHCP_SERVER == TRUE) 

/* DATA TYPE DECLARATIONS
 */
typedef	struct DHCP_MEMORY_AddressRangeBlock_S
{
	int lease_count;
	struct lease *address_range_p;
}DHCP_MEMORY_AddressRangeBlock_T;

/* STATIC VARIABLE DECLARATIONS
 */
static struct subnet *subnets;
static struct shared_network *shared_networks;
static struct hash_table *host_hw_addr_hash;
static struct hash_table *host_uid_hash;
static struct hash_table *lease_uid_hash;
static struct hash_table *lease_ip_addr_hash;
static struct hash_table *lease_hw_addr_hash;
static struct hash_table *vendor_class_hash;
static struct hash_table *user_class_hash;
static struct lease *dangling_leases;
//static DHCP_MEMORY_Server_Lease_Config_T *ip_binding_p;
static	L_SORT_LST_List_T  			dhcp_ip_binding_list;

static DHCP_MEMORY_AddressRangeBlock_T range_block[SYS_ADPT_DHCP_MAX_RANGE_SET_ELEMENTS];
static int block_index;
static int total_num_of_available_ip;


/* LOCAL SUBPROGRAM	DECLARATIONS
 */
static int DHCP_MEMORY_IpBindingCompare(void *node_entry, void *input_entry);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_MEMORY_Init
 * ------------------------------------------------------------------------
 * FUNCTION : Initialize DHCP Server OM (memory.c)
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DHCP_MEMORY_Init()
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
	subnets 			= NULL;
	shared_networks 	= NULL;
	host_hw_addr_hash 	= NULL;
	host_uid_hash 		= NULL;
	lease_uid_hash		= NULL;
	lease_ip_addr_hash	= NULL;
	lease_hw_addr_hash	= NULL;
	dangling_leases		= NULL;
	vendor_class_hash	= NULL;
	user_class_hash		= NULL;	

	block_index = 0;
	total_num_of_available_ip = 0;
	/* Create sort-list for IP binding */
	L_SORT_LST_Create(&dhcp_ip_binding_list, SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL,
						 sizeof(DHCP_MEMORY_Server_Lease_Config_T), DHCP_MEMORY_IpBindingCompare);
		
} /* end of DHCP_MEMORY_Init */

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
void DHCP_MEMORY_ReInit()
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */   
     int i;
    /* BODY */
    
    /* Delete all IP bindings */
	L_SORT_LST_Delete_All(&dhcp_ip_binding_list);
	
	DHCP_MEMORY_FreeHashTable(host_hw_addr_hash, DHCP_TYPE_HASH_HOST_DECL);
	DHCP_MEMORY_FreeHashTable(host_uid_hash, DHCP_TYPE_HASH_HOST_DECL);
	DHCP_MEMORY_FreeHashTable(lease_uid_hash, DHCP_TYPE_HASH_LEASE);
	DHCP_MEMORY_FreeHashTable(lease_ip_addr_hash, DHCP_TYPE_HASH_LEASE);
	DHCP_MEMORY_FreeHashTable(lease_hw_addr_hash, DHCP_TYPE_HASH_LEASE);
	
	DHCP_MEMORY_FreeSubnets(subnets);
	DHCP_MEMORY_FreeSharedNetworks(shared_networks);	
	//DHCP_MEMORY_FreeHashTable(vendor_class_hash);
	//DHCP_MEMORY_FreeHashTable(user_class_hash);
	//DHCP_MEMORY_FreeLease(dangling_leases);
	
	/* Free address range block */
	if (block_index != 0)
	{
		for ( i = 0; i < block_index; i++)
		{
			range_block[i].lease_count = 0;
			dhcp_free(range_block[i].address_range_p);
		}
	}	
	
	subnets 			= NULL;
	shared_networks 	= NULL;
	host_hw_addr_hash 	= NULL;
	host_uid_hash 		= NULL;
	lease_uid_hash		= NULL;
	lease_ip_addr_hash	= NULL;
	lease_hw_addr_hash	= NULL;
	dangling_leases		= NULL;
	vendor_class_hash	= NULL;
	user_class_hash		= NULL;	
  
  	block_index = 0;
	total_num_of_available_ip = 0;
	
} /* end of DHCP_MEMORY_ReInit */


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
BOOL_T DHCP_MEMORY_FreeHashBuckets(struct hash_bucket *hash_bucket, int type)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	BOOL_T ret = TRUE;

	if (hash_bucket->next != NULL)
	{
		DHCP_MEMORY_FreeHashBuckets(hash_bucket->next, type);
		hash_bucket->next = NULL;
	}
	
	hash_bucket->name = NULL;
	if (type == DHCP_TYPE_HASH_HOST_DECL)
	{
		ret = DHCP_MEMORY_FreeHostDecl((struct host_decl *)hash_bucket->value);
	}
	else if (type == DHCP_TYPE_HASH_LEASE)
	{
		//ret = DHCP_MEMORY_FreeLease((struct lease *)hash_table->buckets[i]->value);
	}
	else
	{
		DHCP_ERROR_Print("\nHash Table type is wrong!!! ");	
	}
	
	if (ret == FALSE)
		DHCP_ERROR_Print("\nHash Table free is error!!! ");	
		
	hash_bucket->value = NULL;
    	
	dhcp_free(hash_bucket);
	hash_bucket = NULL;
	
	return ret;
 	
 
} /* end of DHCP_MEMORY_FreeHashBuckets */

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
BOOL_T DHCP_MEMORY_FreeHashTable(struct hash_table *hash_table, int type)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	int i;   
	BOOL_T ret = TRUE;

    if (hash_table == NULL)
    {
    	return TRUE;
    }
    	
    for ( i=0; i < DEFAULT_HASH_SIZE; i++ )
    {
    	if (hash_table->buckets[i]== 0)
    		continue;
    	ret = DHCP_MEMORY_FreeHashBuckets(hash_table->buckets[i], type); 
    	hash_table->buckets[i] = NULL;  		
    }
    
    dhcp_free(hash_table);
    hash_table = NULL;
    
    return ret;
       
} /* end of DHCP_MEMORY_FreeHashTable */

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
BOOL_T DHCP_MEMORY_FreeLease(struct lease *lease)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	   
    /* BODY */
	if (lease == NULL)
    	return TRUE;
  
//	SYSFUN_OM_ENTER_CRITICAL_SECTION();
  
    /* Free linked list of lease if any */	
    if (lease->prev != NULL)
    {
    	if (DHCP_MEMORY_FreeLease(lease->prev))
    		lease->prev = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    lease->next = NULL;
    
    /* Free n_uid lease if any */
    if (lease->n_uid != NULL)
    {
    	if (DHCP_MEMORY_FreeLease(lease->n_uid))
    		lease->n_uid = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* Free n_hw lease if any */
    if (lease->n_hw != NULL)
    {
    	if (DHCP_MEMORY_FreeLease(lease->n_hw))
    		lease->n_hw = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* Free waitq_next lease if any */
    if (lease->waitq_next != NULL)
    {
    	if (DHCP_MEMORY_FreeLease(lease->waitq_next))
    		lease->waitq_next= NULL;
    	else
    	{
 //   		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    if (lease->uid != NULL)
    	dhcp_free (lease->uid);
    
    lease -> hostname 		 = NULL;
    lease -> client_hostname = NULL;
    
    /* Free host_decl */
    if (lease->host != NULL)
    {
    	if (DHCP_MEMORY_FreeHostDecl(lease->host))
    		lease->host= NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    lease -> subnet 		 = NULL;
    lease -> shared_network  = NULL;
    
    /* Free lease_state */
    if (lease->state != NULL)
    {
    	/*if (DHCP_MEMORY_FreeLeaseState(lease->state))
		 		lease->state = NULL;
		  else
    		return FALSE;		
		*/
    	/* Mask off reason, Penny: We are not really going to free  
    	** lease_state. Instead, it is just like tree_cache, we keep
    	** the unwanted memory in DHCP. Next time if we need to allocate,
    	** we get it from here not from system
    	*/
    		free_lease_state(lease->state, "DHCP_MEMORY_FreeLease");
    		lease->state = NULL;
    
    }
	/* Penny: We won't free lease here; THis is because we allocate a large 
	** block (address range), we will free whole block later.
	*/
    lease = NULL;
//    SYSFUN_OM_LEAVE_CRITICAL_SECTION();
	return TRUE;
} /* end of DHCP_MEMORY_FreeLease */

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
BOOL_T DHCP_MEMORY_FreeLeaseState(struct lease_state *state)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int i;
	
    /* BODY */
    if (state == NULL)
   		return TRUE;
   	
//   	SYSFUN_OM_ENTER_CRITICAL_SECTION();
   	
    if (state->next != NULL)
    {
    	if (DHCP_MEMORY_FreeLeaseState(state->next))
    		state->next = NULL;
    	else
    	{
 //   		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    state->ip = NULL;
    
    for (i = 0; i < 256; i++) 
    {
		if (state -> options [i])
			free_tree_cache (state -> options [i], "DHCP_MEMORY_FreeLeaseState");
	}
	state->server_name 	  = NULL;
	if (state->prl != NULL)
		dhcp_free(state->prl);
		
	state->prl 		   	  = NULL;
	state->shared_network = NULL; 
    
    dhcp_free(state);
    state = NULL;
    
//    SYSFUN_OM_LEAVE_CRITICAL_SECTION();
	return TRUE;
    
    
} /* end of DHCP_MEMORY_FreeLeaseState */

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
BOOL_T DHCP_MEMORY_FreeHostDecl(struct host_decl *host)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
   	if (host == NULL)
   		return TRUE;
   	
//   	SYSFUN_OM_ENTER_CRITICAL_SECTION();	
   		
    if (host->n_ipaddr != NULL)
    {
    	if (DHCP_MEMORY_FreeHostDecl(host->n_ipaddr))
    		host->n_ipaddr = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }

	if(host->encoded_hw_addr && host->encoded_cid)
    {
        /* the entry in host_hw and uid hash table, 
           if free two times, system cause exception.           
         */
        host->encoded_hw_addr = FALSE;
        host->encoded_cid = FALSE;
        return FALSE;
    }
	
    host->name = NULL;
   
   	if (host ->fixed_addr != NULL)
   	{
    	free_tree_cache(host ->fixed_addr, "DHCP_MEMORY_FreeHostDecl");
    	host->fixed_addr = NULL;
    }
    
    if (host ->group != NULL)
    {
    	DHCP_MEMORY_FreeGroup(host ->group);
    	host ->group = NULL;
    }
    dhcp_free(host);
    host = NULL;
    
//    SYSFUN_OM_LEAVE_CRITICAL_SECTION();
	return TRUE;   
    
} /* end of DHCP_MEMORY_FreeHostDecl */

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
BOOL_T DHCP_MEMORY_FreeGroup(struct group *group)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
	int i;
    /* BODY */
	if (group == NULL)
    	return TRUE;
    
//    SYSFUN_OM_ENTER_CRITICAL_SECTION();	
    
    /* Free linked list of lease if any */	
    if (group->next != NULL)
    {
    	if (DHCP_MEMORY_FreeGroup(group->next))
    		group->next = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* NOT free subnet and shared_network here */
    group->subnet 			= NULL;
    group->shared_network 	= NULL;
    
   /* if (group->filename != NULL)
    {
    	dhcp_free(group->filename);
    	group->filename = NULL;
    }
    
    if (group->server_name != NULL)
    {
    	dhcp_free(group->server_name);
    	group->server_name = NULL;
    }
	*/
	group->server_name = NULL;    
    for (i = 0; i < 256; i++) 
    {
		if (group -> options [i])
			free_tree_cache (group -> options [i], "DHCP_MEMORY_FreeGroup");
	}
	
	dhcp_free(group);
	group = NULL;
	
//	SYSFUN_OM_LEAVE_CRITICAL_SECTION();
	return TRUE;
}

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
BOOL_T DHCP_MEMORY_FreeSharedNetworks(struct shared_network *share)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
  
    /* BODY */
    if (share == NULL)
    	return TRUE;
    
//    SYSFUN_OM_ENTER_CRITICAL_SECTION();	
    
    /* Free linked list of shared_network if any */	
    if (share->next != NULL)
    {
    	if (DHCP_MEMORY_FreeSharedNetworks(share->next))
    		share->next = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* Free subnet if any */
    /* We don't free subnet in free shared_network */
/*    if (share->subnets != NULL)
    {
    	if (DHCP_MEMORY_FreeSubnets(share->subnets))
    		share->subnets = NULL;
    	else
    		return FALSE;
    }
*/
	share->subnets = NULL;
    
    /* Just assign NULL to interface pointer, we will
    ** do the free job in DHCP_OM_FreeCurrentInterface
    */
    share->interface = NULL;
    
    /* Free lease */
    share->last_lease = NULL;
    /* Free Group */
    if (share->group != NULL)
    {
    	if (DHCP_MEMORY_FreeGroup(share->group))
    		share->group = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    dhcp_free(share);
	share = NULL;
	
//	SYSFUN_OM_LEAVE_CRITICAL_SECTION();
	return TRUE;
    
} /* end of DHCP_MEMORY_FreeSharedNetworks */

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
BOOL_T DHCP_MEMORY_FreeSubnets(struct subnet *subnet_to_be_free)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
 
	   
    /* BODY */
    if (subnet_to_be_free == NULL)
    	return TRUE;
    
//    SYSFUN_OM_ENTER_CRITICAL_SECTION();	
    
    /* Free next_subnet */	
    if (subnet_to_be_free->next_subnet != NULL)
    {
    	if (DHCP_MEMORY_FreeSubnets(subnet_to_be_free->next_subnet))
    		subnet_to_be_free->next_subnet = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* Free next_sibling */	
    if (subnet_to_be_free->next_sibling != NULL)
    {
    	if (DHCP_MEMORY_FreeSubnets(subnet_to_be_free->next_sibling))
    		subnet_to_be_free->next_sibling = NULL;
    	else
    	{
 //   		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    /* Free shared_network */	
/*    if (subnet_to_be_free->shared_network != NULL)
    {
    	if (DHCP_MEMORY_FreeSharedNetworks(subnet_to_be_free->shared_network))
    		subnet_to_be_free->shared_network = NULL;
    	else
    		return FALSE;
    }
*/
	subnet_to_be_free->shared_network = NULL;
    
    /* Just assign NULL to interface pointer, we will
    ** do the free job in DHCP_OM_FreeCurrentInterface
    */
    subnet_to_be_free->interface = NULL;
    
    /* Free group */	
    if (subnet_to_be_free->group != NULL)
    {
    	if (DHCP_MEMORY_FreeGroup(subnet_to_be_free->group))
    		subnet_to_be_free->group = NULL;
    	else
    	{
//    		SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    		return FALSE;
    	}
    }
    
    dhcp_free(subnet_to_be_free);
	subnet_to_be_free = NULL;
    
//    SYSFUN_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
    
} /* end of DHCP_MEMORY_FreeSubnets */

void enter_host(struct host_decl *hd)
{
	struct host_decl *hp = (struct host_decl *)0;
	struct host_decl *np = (struct host_decl *)0;
    
	
	hd -> n_ipaddr = (struct host_decl *)0;
	
    	
	
	if (hd -> interface.hlen) {
		if (!host_hw_addr_hash)
			host_hw_addr_hash = new_hash ();
		else
			hp = (struct host_decl *)hash_lookup (host_hw_addr_hash, hd -> interface.haddr, hd -> interface.hlen);

		/* If there isn't already a host decl matching this
		   address, add it to the hash table. */
		if (!hp)
			add_hash (host_hw_addr_hash,
				  hd -> interface.haddr, hd -> interface.hlen,
				  (unsigned char *)hd);
		    hd->encoded_hw_addr = TRUE;
	}

	/* If there was already a host declaration for this hardware
	   address, add this one to the end of the list. */

	if (hp) {
		for (np = hp; np -> n_ipaddr; np = np -> n_ipaddr)
			;
		np -> n_ipaddr = hd;
	}

	if (hd -> group -> options [DHO_DHCP_CLIENT_IDENTIFIER]) {
		if (!tree_evaluate (hd -> group -> options
				    [DHO_DHCP_CLIENT_IDENTIFIER]))
		{
			
			return;
		}
			
		/* If there's no uid hash, make one; otherwise, see if
		   there's already an entry in the hash for this host. */
		if (!host_uid_hash) {
			host_uid_hash = new_hash ();
			hp = (struct host_decl *)0;
		} else
			hp = (struct host_decl *) hash_lookup(host_uid_hash,
								hd -> group -> options[DHO_DHCP_CLIENT_IDENTIFIER] -> value,
								hd -> group -> options[DHO_DHCP_CLIENT_IDENTIFIER] -> len);

		/* If there's already a host declaration for this
		   client identifier, add this one to the end of the
		   list.  Otherwise, add it to the hash table. */
		if (hp) {
			/* Don't link it in twice... */
			if (!np) {
				for (np = hp; np -> n_ipaddr;
				     np = np -> n_ipaddr)
					;
				np -> n_ipaddr = hd;
			}
		} else {
			add_hash (host_uid_hash,
				  hd -> group -> options
				  [DHO_DHCP_CLIENT_IDENTIFIER] -> value,
				  hd -> group -> options
				  [DHO_DHCP_CLIENT_IDENTIFIER] -> len,
				  (unsigned char *)hd);
			hd->encoded_cid = TRUE;
		}
	}
	
	
}

struct host_decl *find_hosts_by_haddr(int htype, unsigned char *haddr, int hlen)
{
	struct host_decl *foo;
    
		
	foo = (struct host_decl *)hash_lookup (host_hw_addr_hash,
					       haddr, hlen);
	
	return foo;
}

struct host_decl *find_hosts_by_uid(unsigned char *data, int len)
{
	struct host_decl *foo;
	
		
	foo = (struct host_decl *)hash_lookup (host_uid_hash, data, len);
	
	
	return foo;
}

/* More than one host_decl can be returned by find_hosts_by_haddr or
   find_hosts_by_uid, and each host_decl can have multiple addresses.
   Loop through the list of hosts, and then for each host, through the
   list of addresses, looking for an address that's in the same shared
   network as the one specified.    Store the matching address through
   the addr pointer, update the host pointer to point at the host_decl
   that matched, and return the subnet that matched. */

struct subnet *find_host_for_network(struct host_decl **host, struct iaddr *addr, struct shared_network *share)
{
	int i;
	struct subnet *subnet;
	struct iaddr ip_address;
	struct host_decl *hp;

	
		

	for (hp = *host; hp; hp = hp -> n_ipaddr) {
		if (!hp -> fixed_addr || !tree_evaluate (hp -> fixed_addr))
			continue;
		for (i = 0; i < hp ->fixed_addr->len; i += 4) {
			ip_address.len = 4;
			memcpy(ip_address.iabuf,
				hp -> fixed_addr -> value + i, 4);
			subnet = find_grouped_subnet (share, ip_address);
			if (subnet) {
				*addr = ip_address;
				*host = hp;
				
				return subnet;
			}
		}
	}
	
	
	return (struct subnet *)0;
}

void new_address_range(struct iaddr low, struct iaddr high, struct subnet *subnet, int dynamic,UI32_T* low_active_ip, UI32_T* high_active_ip)
{
	struct lease *address_range, *lp, *plp;
	struct iaddr net, addr, temp_addr;
	UI32_T min, max, i, ip_leftout;
	char lowbuf [16], highbuf [16], netbuf [16],str[90];
	struct shared_network *share = subnet -> shared_network;
//	struct hostent *h;
	//struct in_addr ia;
	
	
	ip_leftout = 0;
	
		

	/* All subnets should have attached shared network structures. */
	if (!share) {
	/*	strcpy (netbuf, piaddr (subnet -> net));
		error ("No shared network for network %s (%s)",
		       netbuf, piaddr (subnet -> netmask));*/
		sprintf(str,"\nDHCP>No shared network for network, %s", subnet->net.iabuf);
		DHCP_ERROR_Print(str);
		
		return;
	}

	/* Initialize the hash table if it hasn't been done yet. */
	if (!lease_uid_hash)
		lease_uid_hash = new_hash ();
	if (!lease_ip_addr_hash)
		lease_ip_addr_hash = new_hash ();
	if (!lease_hw_addr_hash)
		lease_hw_addr_hash = new_hash ();

	/* Make sure that high and low addresses are in same subnet. */
	net = subnet_number(low, subnet -> netmask);
	if (!addr_eq (net, subnet_number (high, subnet -> netmask))) {
		strcpy (lowbuf, piaddr (low));
		strcpy (highbuf, piaddr (high));
		strcpy (netbuf, piaddr (subnet -> netmask));
		sprintf(str, "Address range spans %s!","multiple subnets");
		DHCP_ERROR_Print(str);
		
		return;
	}

	/* Make sure that the addresses are on the correct subnet. */
	if (!addr_eq (net, subnet -> net)) {
		strcpy (lowbuf, piaddr (low));
		strcpy (highbuf, piaddr (high));
		strcpy (netbuf, piaddr (subnet -> netmask));
		sprintf(str,"Not on net %s/%s!",
		       piaddr (subnet -> net), netbuf);
		DHCP_ERROR_Print(str);
		
		return;
	}

	/* Get the high and low host addresses... */
	max = host_addr (high, subnet -> netmask);
	min = host_addr (low, subnet -> netmask);

	/* Allow range to be specified high-to-low as well as low-to-high. */
	if (min > max) {
		max = min;
		min = host_addr (high, subnet -> netmask);
	}

	/* 2003-1-13, Penny: Add Max numbers of available IPs control */
	if (total_num_of_available_ip >=  SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL)
	{
		DHCP_ERROR_Print("\nReach maximum numbers of IPs we allowed");
		*low_active_ip=*high_active_ip= 0;
		return;
	}
	
	if (total_num_of_available_ip + (max - min + 1) > SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL)
	{
		DHCP_ERROR_Print("\nSplit IP range in this pool becoming");
		//ip_leftout = total_num_of_available_ip + (max - min + 1)- SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL;
		ip_leftout = SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL - total_num_of_available_ip;
				
		max = min + ip_leftout - 1;		
		high = ip_addr(subnet -> net, subnet -> netmask, max);
		
		strcpy (lowbuf, piaddr (low));
		strcpy (highbuf, piaddr (high));
		strcpy (netbuf, piaddr (subnet -> netmask));
		sprintf(str,"\nAddress range %s to %s on net %s!\n",
		       lowbuf, highbuf, piaddr (subnet -> net));
		DHCP_ERROR_Print(str);
	}

	total_num_of_available_ip += (max - min + 1);
    temp_addr =  ip_addr(subnet -> net, subnet -> netmask, min);
    memcpy(low_active_ip,temp_addr.iabuf,sizeof(UI32_T));
    temp_addr =  ip_addr(subnet -> net, subnet -> netmask, max);
    memcpy(high_active_ip,temp_addr.iabuf,sizeof(UI32_T));
	/* Get a lease structure for each address in the range. */
	address_range = new_leases (max - min + 1, "new_address_range");
	if (!address_range) 
	{
		strcpy (lowbuf, piaddr (low));
		strcpy (highbuf, piaddr (high));
		sprintf("No memory for range %s-%s.", lowbuf, highbuf);
		DHCP_ERROR_Print(str);
		
		return;
	}
	
	range_block[block_index].address_range_p = address_range;
	range_block[block_index].lease_count = max - min + 1;
	block_index++;
	
	memset (address_range, 0, (sizeof *address_range) * (max - min + 1));

	/* Fill in the last lease if it hasn't been already... */
	if (!share -> last_lease) {
		share -> last_lease = &address_range [0];
	}

	/* Fill out the lease structures with some minimal information. */
	for (i = 0; i < max - min + 1; i++) 
	{
		temp_addr =  ip_addr(subnet -> net, subnet -> netmask, i + min);
		memcpy(&address_range [i].ip_addr, temp_addr.iabuf, 4);
		//memcpy(&address_range [i].ip_addr, subnet -> net.iabuf, subnet->net.len);
		address_range [i].starts = address_range [i].timestamp = MIN_TIME;
		address_range [i].ends = MIN_TIME;
		address_range [i].subnet = subnet;
		address_range [i].shared_network = share;
		address_range [i].flags = dynamic ? DYNAMIC_BOOTP_OK : 0;

		//memcpy (&ia.s_addr, &address_range[i].ip_addr, 4);
		
		
		/* 08/28/02, PENNY, DHCP_MODIFICATION */
		#if 0
		if (subnet -> group -> get_lease_hostnames) {
			h = gethostbyaddr ((char *)&ia, sizeof ia, AF_INET);
			if (!h)
				warn("\n No hostname.");
				/*warn ("No hostname for %s", inet_ntoa (ia));*/
			else {
				address_range [i].hostname =
					malloc (strlen (h -> h_name) + 1);
				if (!address_range [i].hostname)
					error ("no memory for hostname %s.",
					       h -> h_name);
				strcpy (address_range [i].hostname,
					h -> h_name);
			}
		}
		#endif
		/* end of 08/28/02, PENNY DHCP_MODIFICATION */

		/* Link this entry into the list. */
		address_range [i].next = share -> leases;
		address_range [i].prev = (struct lease *)0;
		share -> leases = &address_range [i];
		if (address_range [i].next)
			address_range [i].next -> prev = share -> leases;
		/*add_hash (lease_ip_addr_hash,
			  address_range [i].ip_addr.iabuf,
			  address_range [i].ip_addr.len,
			  (unsigned char *)&address_range [i]);*/
		add_hash (lease_ip_addr_hash,
			  (unsigned char *)&address_range [i].ip_addr,
			  4,
			  (unsigned char *)&address_range [i]);
	}

	/* Find out if any dangling leases are in range... */
	plp = (struct lease *)0;
	for (lp = dangling_leases; lp; lp = lp -> next) {
		struct iaddr lnet, ia;
		int lhost;
		
		memcpy(addr.iabuf, &lp -> ip_addr, 4);
		addr.len =4;
		lnet = subnet_number(addr, subnet -> netmask);
		memcpy(ia.iabuf, &lp->ip_addr, 4);
		ia.len = 4;
		lhost = host_addr(ia, subnet -> netmask);

		/* If it's in range, fill in the real lease structure with
		   the dangling lease's values, and remove the lease from
		   the list of dangling leases. */
		if (addr_eq (lnet, subnet -> net) &&
		    lhost >= i && lhost <= max) {
			if (plp) {
				plp -> next = lp -> next;
			} else {
				dangling_leases = lp -> next;
			}
			lp -> next = (struct lease *)0;
			address_range [lhost - i].hostname = lp -> hostname;
			address_range [lhost - i].client_hostname =
				lp -> client_hostname;
			supersede_lease (&address_range [lhost - i], lp, 0);
			free_lease (lp, "new_address_range");
		} else
			plp = lp;
	}
	
	
}

struct subnet *find_subnet(struct iaddr addr)
{
	struct subnet *rv;
	
	
		

	for (rv = subnets; rv; rv = rv -> next_subnet) {
		if (addr_eq (subnet_number (addr, rv -> netmask), rv -> net))
		{
			
			return rv;
		}
	}
	
	return (struct subnet *)0;
}

struct subnet *find_grouped_subnet(struct shared_network *share, struct iaddr addr)
{
	struct subnet *rv;

	
		
	
	for (rv = share -> subnets; rv; rv = rv -> next_sibling) {
		if (addr_eq (subnet_number (addr, rv -> netmask), rv -> net))
		{
			
			return rv;
		}
	}
	
	return (struct subnet *)0;
}

int subnet_inner_than(struct subnet *subnet, struct subnet *scan, int warnp)
{
	
		
	
	if (addr_eq (subnet_number (subnet -> net, scan -> netmask),
		     scan -> net) ||
	    addr_eq (subnet_number (scan -> net, subnet -> netmask),
		     subnet -> net)) {
		char n1buf [16];
		int i, j;
		for (i = 0; i < 32; i++)
			if (subnet -> netmask.iabuf [3 - (i >> 3)]
			    & (1 << (i & 7)))
				break;
		for (j = 0; j < 32; j++)
			if (scan -> netmask.iabuf [3 - (j >> 3)] &
			    (1 << (j & 7)))
				break;
		strcpy (n1buf, piaddr (subnet -> net));
		if (warnp)
			DHCP_ERROR_Print("\nsubnet conflicts.");
			/*warn ("%ssubnet %s/%d conflicts with subnet %s/%d",
			      "Warning: ", n1buf, 32 - i,
			      piaddr (scan -> net), 32 - j);*/
		if (i < j)
		{
			
			return 1;
		}
	}
	
	return 0;
}

/* Enter a new subnet into the subnet list. */

void enter_subnet(struct subnet *subnet)
{
	struct subnet *scan, *prev = (struct subnet *)0;
	
	
		

	/* Check for duplicates... */
	for (scan = subnets; scan; scan = scan -> next_subnet) {
		/* When we find a conflict, make sure that the
		   subnet with the narrowest subnet mask comes
		   first. */
		if (subnet_inner_than (subnet, scan, 1)) 
		{
			if (prev) {
				prev -> next_subnet = subnet; 
			} else
				subnets = subnet;
			subnet -> next_subnet = scan;
			
			return;
		}
		prev = scan;
	}

	/* XXX use the BSD radix tree code instead of a linked list. */
	subnet -> next_subnet = subnets;
	subnets = subnet;
	
}
	
/* Enter a new shared network into the shared network list. */

void enter_shared_network(struct shared_network *share)
{
	
		
	/* XXX Sort the nets into a balanced tree to make searching quicker. */
	share -> next = shared_networks;
	shared_networks = share;
	
}
	
/* Enter a lease into the system.   This is called by the parser each
   time it reads in a new lease.   If the subnet for that lease has
   already been read in (usually the case), just update that lease;
   otherwise, allocate temporary storage for the lease and keep it around
   until we're done reading in the config file. */

void enter_lease(struct lease *lease)
{
	struct iaddr addr;
	struct lease *comp;
	
	
		
	memcpy(addr.iabuf, &lease->ip_addr, 4);
	addr.len = 4;
	comp = find_lease_by_ip_addr (addr);
	/* If we don't have a place for this lease yet, save it for
	   later. */
	if (!comp) {
		comp = new_lease ("enter_lease");
		if (!comp) {
			error("\nNo memory for lease");
		}
		*comp = *lease;
		comp -> next = dangling_leases;
		comp -> prev = (struct lease *)0;
		dangling_leases = comp;
	} else {
		/* Record the hostname information in the lease. */
		comp -> hostname = lease -> hostname;
		comp -> client_hostname = lease -> client_hostname;
		supersede_lease (comp, lease, 0);
	}
	
	
}

/* Replace the data in an existing lease with the data in a new lease;
   adjust hash tables to suit, and insertion sort the lease into the
   list of leases by expiry time so that we can always find the oldest
   lease. */

int supersede_lease(struct lease *comp, struct lease *lease, int commit)
{
	int enter_uid = 0;
	int enter_hwaddr = 0;
	struct lease *lp;
	TIME cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
	
	/* Static leases are not currently kept in the database... */
	if (lease -> flags & STATIC_LEASE)
		return 1;
	
	
		

	/* If the existing lease hasn't expired and has a different
	   unique identifier or, if it doesn't have a unique
	   identifier, a different hardware address, then the two
	   leases are in conflict.  If the existing lease has a uid
	   and the new one doesn't, but they both have the same
	   hardware address, and dynamic bootp is allowed on this
	   lease, then we allow that, in case a dynamic BOOTP lease is
	   requested *after* a DHCP lease has been assigned. */

	if (!(lease -> flags & ABANDONED_LEASE) &&
	    comp -> ends > cur_time &&
	    (((comp -> uid && lease -> uid) &&
	      (comp -> uid_len != lease -> uid_len ||
	       memcmp (comp -> uid, lease -> uid, comp -> uid_len))) ||
	     (!comp -> uid &&
	      ((comp -> hardware_addr.htype !=
		lease -> hardware_addr.htype) ||
	       (comp -> hardware_addr.hlen !=
		lease -> hardware_addr.hlen) ||
	       memcmp (comp -> hardware_addr.haddr,
		       lease -> hardware_addr.haddr,
		       comp -> hardware_addr.hlen))))) {
		/* warn ("Lease conflict at %s",
		      piaddr (comp -> ip_addr)); */
		    warn("\n Lease conflict");
		
		return 0;
	} else {
		/* If there's a Unique ID, dissociate it from the hash
		   table and free it if necessary. */
		if (comp -> uid) {
			uid_hash_delete (comp);
			enter_uid = 1;
			if (comp -> uid != &comp -> uid_buf [0]) {
				dhcp_free (comp -> uid);
				comp -> uid_max = 0;
				comp -> uid_len = 0;
			}
			comp -> uid = (unsigned char *)0;
		} else
			enter_uid = 1;

		if (comp -> hardware_addr.htype &&
		    ((comp -> hardware_addr.hlen !=
		      lease -> hardware_addr.hlen) ||
		     (comp -> hardware_addr.htype !=
		      lease -> hardware_addr.htype) ||
		     memcmp (comp -> hardware_addr.haddr,
			     lease -> hardware_addr.haddr,
			     comp -> hardware_addr.hlen))) {
			hw_hash_delete (comp);
			enter_hwaddr = 1;
		} else if (!comp -> hardware_addr.htype)
			enter_hwaddr = 1;

		/* Copy the data files, but not the linkages. */
		comp -> starts = lease -> starts;
		if (lease -> uid) {
			if (lease -> uid_len < sizeof (lease -> uid_buf)) {
				memcpy (comp -> uid_buf,
					lease -> uid, lease -> uid_len);
				comp -> uid = &comp -> uid_buf [0];
				comp -> uid_max = sizeof comp -> uid_buf;
			} else if (lease -> uid != &lease -> uid_buf [0]) {
				comp -> uid = lease -> uid;
				comp -> uid_max = lease -> uid_max;
				lease -> uid = (unsigned char *)0;
				lease -> uid_max = 0;
			} else {
				error ("corrupt lease uid."); /* XXX */
				
				return 0;
			}
		} else {
			comp -> uid = (unsigned char *)0;
			comp -> uid_max = 0;
		}
		comp -> uid_len = lease -> uid_len;
		comp -> host = lease -> host;
		comp -> hardware_addr = lease -> hardware_addr;
		comp -> flags = ((lease -> flags & ~PERSISTENT_FLAGS) |
				 (comp -> flags & ~EPHEMERAL_FLAGS));

		/* Record the lease in the uid hash if necessary. */
		if (enter_uid && lease -> uid) {
			uid_hash_add (comp);
		}

		/* Record it in the hardware address hash if necessary. */
		if (enter_hwaddr && lease -> hardware_addr.htype) {
			hw_hash_add (comp);
		}

		/* Remove the lease from its current place in the 
		   timeout sequence. */
		if (comp -> prev) {
			comp -> prev -> next = comp -> next;
		} else {
			comp -> shared_network -> leases = comp -> next;
		}
		if (comp -> next) {
			comp -> next -> prev = comp -> prev;
		}
		if (comp -> shared_network -> last_lease == comp) {
			comp -> shared_network -> last_lease = comp -> prev;
		}

		/* Find the last insertion point... */
		if (comp == comp -> shared_network -> insertion_point ||
		    !comp -> shared_network -> insertion_point) {
			lp = comp -> shared_network -> leases;
		} else {
			lp = comp -> shared_network -> insertion_point;
		}

		if (!lp) {
			/* Nothing on the list yet?    Just make comp the
			   head of the list. */
			comp -> shared_network -> leases = comp;
			comp -> shared_network -> last_lease = comp;
		} else if (lp -> ends > lease -> ends) {
			/* Skip down the list until we run out of list
			   or find a place for comp. */
			while (lp -> next && lp -> ends > lease -> ends) {
				lp = lp -> next;
			}
			if (lp -> ends > lease -> ends) {
				/* If we ran out of list, put comp
				   at the end. */
				lp -> next = comp;
				comp -> prev = lp;
				comp -> next = (struct lease *)0;
				comp -> shared_network -> last_lease = comp;
			} else {
				/* If we didn't, put it between lp and
				   the previous item on the list. */
				if ((comp -> prev = lp -> prev))
					comp -> prev -> next = comp;
				comp -> next = lp;
				lp -> prev = comp;
			}
		} else {
			/* Skip up the list until we run out of list
			   or find a place for comp. */
			while (lp -> prev && lp -> ends < lease -> ends) {
				lp = lp -> prev;
			}
			if (lp -> ends < lease -> ends) {
				/* If we ran out of list, put comp
				   at the beginning. */
				lp -> prev = comp;
				comp -> next = lp;
				comp -> prev = (struct lease *)0;
				comp -> shared_network -> leases = comp;
			} else {
				/* If we didn't, put it between lp and
				   the next item on the list. */
				if ((comp -> next = lp -> next))
					comp -> next -> prev = comp;
				comp -> prev = lp;
				lp -> next = comp;
			}
		}
		comp -> shared_network -> insertion_point = comp;
		comp -> ends = lease -> ends;
	}

	/* Return zero if we didn't commit the lease to permanent storage;
	   nonzero if we did. */
	/*return commit && write_lease (comp) && commit_leases (); */
	
	
	return 1;
} /* end of supersede_lease */

/* Release the specified lease and re-hash it as appropriate. */

void release_lease(struct lease *lease)
{
	int ret = 0;
	struct lease lt;
	TIME cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

	
		
	
	lt = *lease;
    lease->ends = cur_time;   
    memset(&(lt.hardware_addr), 0, sizeof(lt.hardware_addr));
    lt.uid = (unsigned char *)0;
   	lt.uid_len = 0;
	ret = supersede_lease (lease, &lt, 1);
	if (ret)
		DHCP_MEMORY_UpdateIpBindingForDhcpRelease(lease->ip_addr);

	
	
}

/* Abandon the specified lease (set its timeout to infinity and its
   particulars to zero, and re-hash it as appropriate. */

void abandon_lease(struct lease *lease, char *message)
{
	struct lease lt;
	TIME cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

	
		
	
	lease -> flags |= ABANDONED_LEASE;
	lt = *lease;
	lt.ends = cur_time;
	/*warn ("Abandoning IP address %s: %s",
	      piaddr (lease -> ip_addr), message);*/
	lt.hardware_addr.htype = 0;
	lt.hardware_addr.hlen = 0;
	lt.uid = (unsigned char *)0;
	lt.uid_len = 0;
	supersede_lease (lease, &lt, 1);
	
	
}

/* Locate the lease associated with a given IP address... */

struct lease *find_lease_by_ip_addr(struct iaddr addr)
{
	struct lease *lease = (struct lease *)hash_lookup (lease_ip_addr_hash,
							   addr.iabuf,
							   addr.len);
	return lease;
}

struct lease *find_lease_by_uid(unsigned char *uid, int len)
{
	
	struct lease *lease = (struct lease *)hash_lookup (lease_uid_hash,
							   uid, len);
	
	return lease;
}

struct lease *find_lease_by_hw_addr(unsigned char *hwaddr, int hwlen)
{
	struct lease *lease = (struct lease *)hash_lookup (lease_hw_addr_hash,
							   hwaddr, hwlen);
	return lease;
}

/* Add the specified lease to the uid hash. */

void uid_hash_add(struct lease *lease)
{
	struct lease *head =
		find_lease_by_uid (lease -> uid, lease -> uid_len);
	struct lease *scan;

#ifdef DEBUG
	if (lease -> n_uid)
		abort ();
#endif

	
		

	/* If it's not in the hash, just add it. */
	if (!head)
		add_hash (lease_uid_hash, lease -> uid,
			  lease -> uid_len, (unsigned char *)lease);
	else {
		/* Otherwise, attach it to the end of the list. */
		for (scan = head; scan -> n_uid; scan = scan -> n_uid)
#ifdef DEBUG
			if (scan == lease)
				abort ()
#endif
					;
		scan -> n_uid = lease;
	}
	
	
}

/* Delete the specified lease from the uid hash. */

void uid_hash_delete(struct lease *lease)
{
	struct lease *head =
		find_lease_by_uid (lease -> uid, lease -> uid_len);
	struct lease *scan;

	
		

	/* If it's not in the hash, we have no work to do. */
	if (!head) {
		lease -> n_uid = (struct lease *)0;
		
		return;
	}

	/* If the lease we're freeing is at the head of the list,
	   remove the hash table entry and add a new one with the
	   next lease on the list (if there is one). */
	if (head == lease) {
		delete_hash_entry (lease_uid_hash,
				   lease -> uid, lease -> uid_len);
		if (lease -> n_uid)
			add_hash (lease_uid_hash,
				  lease -> n_uid -> uid,
				  lease -> n_uid -> uid_len,
				  (unsigned char *)(lease -> n_uid));
	} else {
		/* Otherwise, look for the lease in the list of leases
		   attached to the hash table entry, and remove it if
		   we find it. */
		for (scan = head; scan -> n_uid; scan = scan -> n_uid) {
			if (scan -> n_uid == lease) {
				scan -> n_uid = scan -> n_uid -> n_uid;
				break;
			}
		}
	}
	lease -> n_uid = (struct lease *)0;
	
}

/* Add the specified lease to the hardware address hash. */

void hw_hash_add(struct lease *lease)
{
	struct lease *head =
		find_lease_by_hw_addr (lease -> hardware_addr.haddr,
				       lease -> hardware_addr.hlen);
	struct lease *scan;

	
		
	
	/* If it's not in the hash, just add it. */
	if (!head)
		add_hash (lease_hw_addr_hash,
			  lease -> hardware_addr.haddr,
			  lease -> hardware_addr.hlen,
			  (unsigned char *)lease);
	else {
		/* Otherwise, attach it to the end of the list. */
		for (scan = head; scan -> n_hw; scan = scan -> n_hw)
			;
		scan -> n_hw = lease;
	}
	
}

/* Delete the specified lease from the hardware address hash. */

void hw_hash_delete(struct lease *lease)
{
	struct lease *head =
		find_lease_by_hw_addr (lease -> hardware_addr.haddr,
				       lease -> hardware_addr.hlen);
	struct lease *scan;
	
	
		

	/* If it's not in the hash, we have no work to do. */
	if (!head) {
		lease -> n_hw = (struct lease *)0;
		
		return;
	}

	/* If the lease we're freeing is at the head of the list,
	   remove the hash table entry and add a new one with the
	   next lease on the list (if there is one). */
	if (head == lease) {
		delete_hash_entry (lease_hw_addr_hash,
				   lease -> hardware_addr.haddr,
				   lease -> hardware_addr.hlen);
		if (lease -> n_hw)
			add_hash (lease_hw_addr_hash,
				  lease -> n_hw -> hardware_addr.haddr,
				  lease -> n_hw -> hardware_addr.hlen,
				  (unsigned char *)(lease -> n_hw));
	} else {
		/* Otherwise, look for the lease in the list of leases
		   attached to the hash table entry, and remove it if
		   we find it. */
		for (scan = head; scan -> n_hw; scan = scan -> n_hw) {
			if (scan -> n_hw == lease) {
				scan -> n_hw = scan -> n_hw -> n_hw;
				break;
			}
		}
	}
	lease -> n_hw = (struct lease *)0;
	
}


struct class *add_class(int type, char *name)
{
	struct class *class_p = new_class ("add_class");
	char *tname = (char *)malloc (strlen (name) + 1);

	if (!vendor_class_hash)
		vendor_class_hash = new_hash ();
	if (!user_class_hash)
		user_class_hash = new_hash ();

	if (!tname || !class_p || !vendor_class_hash || !user_class_hash)
		return (struct class *)0;

	memset(class_p, 0, sizeof(*class_p));
	strcpy(tname, name);
	class_p -> name = tname;

	if (type)
		add_hash (user_class_hash,
			  (unsigned char *)tname, strlen (tname),
			  (unsigned char *)class_p);
	else
		add_hash (vendor_class_hash,
			  (unsigned char *)tname, strlen (tname),
			  (unsigned char *)class_p);
	return class_p;
}

struct class *find_class(int type, unsigned char *name, int len)
{
	struct class *class =
		(struct class *)hash_lookup (type
					     ? user_class_hash
					     : vendor_class_hash, name, len);
	return class;
}	

struct group *clone_group(struct group *group, char *caller)
{
	struct group *g = new_group(caller);
	if (!g)
		DHCP_ERROR_Print("\nCan't allocate new group.");
		/* error ("%s: can't allocate new group", caller); */
	*g = *group;
	return g;
}

/* Write all interesting leases to permanent storage. */

void write_leases()
{
	struct lease *l;
	struct shared_network *s;


	for (s = shared_networks; s; s = s -> next) {
		for (l = s -> leases; l; l = l -> next) {
			if (l -> hardware_addr.hlen ||
			    l -> uid_len ||
			    (l -> flags & ABANDONED_LEASE))
				if (!write_lease (l))
					error ("Can't rewrite lease database");
		}
	}
	if (!commit_leases ())
		error ("Can't commit leases to new database: %m");
	

}

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
BOOL_T DHCP_MEMORY_GetIpBindingLease(UI32_T ip_address, DHCP_MEMORY_Server_Lease_Config_T *binding)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */  
    DHCP_MEMORY_Server_Lease_Config_T list_elm;  
    
    /* BODY */
    if (ip_address == 0)
    	return FALSE;
    
    list_elm.lease_ip = ip_address;
      
	if (!L_SORT_LST_Get( &dhcp_ip_binding_list, &list_elm))
    	return FALSE;
    else
    {
    	memcpy(binding, &list_elm, sizeof(DHCP_MEMORY_Server_Lease_Config_T));
    	return TRUE;
    }
        
} /* end of DHCP_MEMORY_GetIpBindingLease */

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
BOOL_T DHCP_MEMORY_GetNextIpBindingLease(UI32_T ip_address, DHCP_MEMORY_Server_Lease_Config_T *binding)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */  
     DHCP_MEMORY_Server_Lease_Config_T list_elm;  
     
    /* BODY */
    list_elm.lease_ip = ip_address;
    
    /* get 1st lease */
    if (ip_address == 0)
    {
    	if (!L_SORT_LST_Get_1st(&dhcp_ip_binding_list, &list_elm))
    		return FALSE;
    	else
    	{
    		memcpy(binding, &list_elm, sizeof(DHCP_MEMORY_Server_Lease_Config_T));
    		return TRUE;	
    	}
  
    }
    else
    {
    	if (!L_SORT_LST_Get_Next (&dhcp_ip_binding_list, &list_elm))
    		return FALSE;
    	else
    	{
    		memcpy(binding, &list_elm, sizeof(DHCP_MEMORY_Server_Lease_Config_T));
    		return TRUE;		
    	}
    }
    
} /* end of DHCP_MEMORY_GetNextIpBindingLease */

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
BOOL_T DHCP_MEMORY_ClearIpBinding(UI32_T ip_address)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
 	DHCP_MEMORY_Server_Lease_Config_T         list_elm;  
 	struct lease *lease;
	struct iaddr cip;
 	
    /* BODY */
    
    list_elm.lease_ip = ip_address;
    
    /* 1. Delete all ip bindings */
    if (ip_address == 0)
    {
    	if (!L_SORT_LST_Get_1st (&dhcp_ip_binding_list, &list_elm))
    		return FALSE;
    	else
    	{
    		do
    		{
    			/* 1. release the IP */ 
			    cip.len = 4;
				memcpy (cip.iabuf, &list_elm.lease_ip, 4);
				lease = find_lease_by_ip_addr (cip);
			    
			    if (lease)
			    	release_lease(lease);
    			
    		} while (L_SORT_LST_Get_Next(&dhcp_ip_binding_list, &list_elm));	
    	}
    	
    	
    	return TRUE;	
    }
    else /* only delete one IP binding */
    {
    	if (!L_SORT_LST_Get( &dhcp_ip_binding_list, &list_elm))
        	return FALSE;
        else
        {
        	/* 1. release the IP */ 
		    cip.len = 4;
			memcpy (cip.iabuf, &list_elm.lease_ip, 4);
			lease = find_lease_by_ip_addr (cip);
		    
		    if (lease)
		    {
		    	release_lease(lease);		    	
		    }
    		return TRUE;
    	}
    }
    
} /* end of DHCP_MEMORY_ClearIpBinding */

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
BOOL_T DHCP_MEMORY_UpdateIpBindingForDhcpRelease(UI32_T ip_address)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
 	DHCP_MEMORY_Server_Lease_Config_T         list_elm;  
 	
    /* BODY */
    
    list_elm.lease_ip = ip_address;
    
    
	if (!L_SORT_LST_Get( &dhcp_ip_binding_list, &list_elm))
    	return FALSE;
    else
    {
		return (L_SORT_LST_Delete (&dhcp_ip_binding_list, &list_elm));
	}
   
   
    
} /* end of DHCP_MEMORY_UpdateIpBindingForDhcpRelease */    

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
BOOL_T DHCP_MEMORY_WriteIpBindingLease(struct lease *lease)
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */  
    UI32_T seconds;
	DHCP_MEMORY_Server_Lease_Config_T binding;
	
    
    /* BODY */
    /* It's a bootp lease or static lease */
    if (lease != NULL &&
        ((lease->flags & STATIC_LEASE)||
         (lease->flags & BOOTP_LEASE)))
    {
		memcpy(&binding.lease_ip, &lease->ip_addr, 4);
		memcpy(binding.hardware_address, lease->hardware_addr.haddr, SYS_ADPT_MAC_ADDR_LEN);
		binding.lease_time = DHCP_MGR_INFINITE_LEASE_TIME; /* infinite lease */
		SYS_TIME_GetRealTimeBySec(&seconds); /* real time */ 
		binding.start_time = seconds;
		
		if (!L_SORT_LST_Set(&dhcp_ip_binding_list, &binding))
        	return FALSE;
       	
    }
    else if (lease == NULL || lease->state == NULL || lease->state->offer!=DHCPACK )
    	return FALSE;
    else
    {	
	    memcpy(&binding.lease_ip, &lease->ip_addr, 4);
		memcpy(binding.hardware_address, lease->hardware_addr.haddr, SYS_ADPT_MAC_ADDR_LEN);
		binding.lease_time = lease->state->expiry;
		SYS_TIME_GetRealTimeBySec(&seconds); /* real time */ 
		binding.start_time = seconds;
		
		if (!L_SORT_LST_Set(&dhcp_ip_binding_list, &binding))
        	return FALSE;
        	
	}
	return TRUE;
} /* end of DHCP_MEMORY_WriteIpBindingLease */

int write_lease(struct lease *lease)
{
#if 0
   DHCP_API_Dhcp_Server_Lease_Config_T lease_data;
   strcpy(lease_data.lease_ip, piaddr (lease -> ip_addr));
   strcpy(lease_data.hardware_address, print_hw_addr (lease -> hardware_addr.htype,
					                                       lease -> hardware_addr.hlen,
					                                       lease -> hardware_addr.haddr));
   strcpy(lease_data.client_hostname, lease -> client_hostname); 
   RS_DHCPAPI_Set_LeaseConfigTable(&lease_data);
   return 1;
#endif
	

return 1;

#if 0
	struct tm *t;
	
	char tbuf [64];
	int errors = 0;
	int i;

	if (counting)
		++count;
	errno = 0;
	fprintf (db_file, "lease %s {\n", piaddr (lease -> ip_addr));
	if (errno) {
		++errors;
	}
	
	t = gmtime (&lease -> starts);
	sprintf (tbuf, "%d %d/%02d/%02d %02d:%02d:%02d;",
		 t -> tm_wday, t -> tm_year + 1900,
		 t -> tm_mon + 1, t -> tm_mday,
		 t -> tm_hour, t -> tm_min, t -> tm_sec);
	errno = 0;
	fprintf (db_file, "\tstarts %s\n", tbuf);
	if (errno) {
		++errors;
	}

	t = gmtime (&lease -> ends);
	sprintf (tbuf, "%d %d/%02d/%02d %02d:%02d:%02d;",
		 t -> tm_wday, t -> tm_year + 1900,
		 t -> tm_mon + 1, t -> tm_mday,
		 t -> tm_hour, t -> tm_min, t -> tm_sec);
	errno = 0;
	fprintf (db_file, "\tends %s", tbuf);
	if (errno) {
		++errors;
	}

	if (lease -> hardware_addr.hlen) {
		errno = 0;
		fprintf (db_file, "\n\thardware %s %s;",
			 hardware_types [lease -> hardware_addr.htype],
			 print_hw_addr (lease -> hardware_addr.htype,
					lease -> hardware_addr.hlen,
					lease -> hardware_addr.haddr));
		if (errno) {
			++errors;
		}
	}
	if (lease -> uid_len) {
		int i;
		errno = 0;
		fprintf (db_file, "\n\tuid %2.2x", lease -> uid [0]);
		if (errno) {
			++errors;
		}
		for (i = 1; i < lease -> uid_len; i++) {
			errno = 0;
			fprintf (db_file, ":%2.2x", lease -> uid [i]);
			if (errno) {
				++errors;
			}
		}
		putc (';', db_file);
	}
	if (lease -> flags & BOOTP_LEASE) {
		errno = 0;
		fprintf (db_file, "\n\tdynamic-bootp;");
		if (errno) {
			++errors;
		}
	}
	if (lease -> flags & ABANDONED_LEASE) {
		errno = 0;
		fprintf (db_file, "\n\tabandoned;");
		if (errno) {
			++errors;
		}
	}
	if (lease -> client_hostname) {
		for (i = 0; lease -> client_hostname [i]; i++)
			if (lease -> client_hostname [i] < 33 ||
			    lease -> client_hostname [i] > 126)
				goto bad_client_hostname;
		errno = 0;
		fprintf (db_file, "\n\tclient-hostname \"%s\";",
			 lease -> client_hostname);
		if (errno) {
			++errors;
		}
	}
       bad_client_hostname:
	if (lease -> hostname) {
		for (i = 0; lease -> hostname [i]; i++)
			if (lease -> hostname [i] < 33 ||
			    lease -> hostname [i] > 126)
				goto bad_hostname;
		errno = 0;
		errno = 0;
		fprintf (db_file, "\n\thostname \"%s\";",
			 lease -> hostname);
		if (errno) {
			++errors;
		}
	}
       bad_hostname:
	errno = 0;
	fputs ("\n}\n", db_file);
	if (errno) {
		++errors;
	}
	if (errors)
		note ("write_lease: unable to write lease %s",
		      piaddr (lease -> ip_addr));
	return !errors;
#endif
}

int commit_leases ()
{
	return 1;
}

void dump_subnets()
{
	struct lease *l;
	struct shared_network *s;
	struct subnet *n;

	//note ("Subnets:");
	for (n = subnets; n; n = n -> next_subnet) {
	//	debug ("  Subnet %s", piaddr (n -> net));
	//	debug ("     netmask %s",
//		       piaddr (n -> netmask));
	}
//	note ("Shared networks:");
	for (s = shared_networks; s; s = s -> next) {
//		note ("  %s", s -> name);
		for (l = s -> leases; l; l = l -> next) {
//			print_lease (l);
		}
		if (s -> last_lease) {
//			debug ("    Last Lease:");
//			print_lease (s -> last_lease);
		}
	}
}


static int DHCP_MEMORY_IpBindingCompare(void *node_entry, void *input_entry)
{
	DHCP_MEMORY_Server_Lease_Config_T  *node, *input;

	node = (DHCP_MEMORY_Server_Lease_Config_T *) node_entry;
	input = (DHCP_MEMORY_Server_Lease_Config_T  *) input_entry;


    return (node->lease_ip - input->lease_ip);

}

#endif /* end of #if (SYS_CPNT_DHCP_SERVER == TRUE) */
