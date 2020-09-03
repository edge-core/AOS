/* alloc.c

   Memory allocation... */

/*
 * Copyright (c) 1995, 1996, 1998 The Internet Software Consortium.
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
#include "dhcp_type.h"
#include "tree.h"
#include "hash.h"
#include "dhcp_error_print.h"
#include "alloc.h"
#include "sysfun.h"

/*Penny*/
struct dhcp_packet *dhcp_free_list;
struct packet *packet_free_list;

VOIDPTR dmalloc (size, name)
	int size;
	char *name;
{
	char  buff[128] = {0};
	VOIDPTR foo = (VOIDPTR)dhcp_malloc (size);
	if (!foo)
	{
		sprintf(buff,"\nDHCP>No memory for %s", name);
		DHCP_ERROR_Print(buff);
	}
	else
		memset (foo, 0, size);
	return foo;
}

void dfree (ptr, name)
	VOIDPTR ptr;
	char *name;
{
	if (!ptr) {
		DHCP_ERROR_Print("\nDHCP>dfree: free on null pointer.");
		return;
	}
	dhcp_free (ptr);
}

struct packet *new_packet (name)
	char *name;
{
	struct packet *rval;
	rval = (struct packet *)dmalloc (sizeof (struct packet), name);
	return rval;
}

struct dhcp_packet *new_dhcp_packet (name)
	char *name;
{
	struct dhcp_packet *rval;
	rval = (struct dhcp_packet *)dmalloc (sizeof (struct dhcp_packet),
					      name);
	return rval;
}

struct tree *new_tree (name)
	char *name;
{
	struct tree *rval = (struct tree *)dmalloc (sizeof (struct tree), name);
	return rval;
}

static struct tree_cache *free_tree_caches = NULL;

struct tree_cache *new_tree_cache (name)
	char *name;
{
	struct tree_cache *rval;
/*	UI32_T	prev_mask;*//*Timon*/

	if (free_tree_caches)
	{
		rval = free_tree_caches;
/*		prev_mask = SYSFUN_InterruptLock();*//*Timon*/
		free_tree_caches = (struct tree_cache *)(rval -> value);
/*		SYSFUN_InterruptUnlock(prev_mask);*//*Timon*/
	}
	else
	{
		rval = dmalloc (sizeof (struct tree_cache), name);
		if (!rval)
		{
		DHCP_ERROR_Print("\nDHCP>unable to allocate tree cache ");
			return NULL;
		}
	}
	/* Penny 2002-12-31: in order to clear out the content of tree cache to
	** 	prevent using tree cache in the recycle area
	*/
	memset(rval, 0, sizeof(*rval));
	return rval;
}

struct hash_table *new_hash_table (count, name)
	int count;
	char *name;
{
	struct hash_table *rval = dmalloc (sizeof (struct hash_table)
					   - (DEFAULT_HASH_SIZE
					      * sizeof (struct hash_bucket *))
					   + (count
					      * sizeof (struct hash_bucket *)),
					   name);
	rval -> hash_count = count;
	return rval;
}

struct hash_bucket *new_hash_bucket (name)
	char *name;
{
	struct hash_bucket *rval = dmalloc (sizeof (struct hash_bucket), name);
	return rval;
}

struct lease *new_leases (n, name)
	int n;
	char *name;
{
	struct lease *rval = dmalloc (n * sizeof (struct lease), name);
	return rval;
}

struct lease *new_lease (name)
	char *name;
{
	struct lease *rval = dmalloc (sizeof (struct lease), name);
	return rval;
}

struct subnet *new_subnet (name)
	char *name;
{
	struct subnet *rval = dmalloc (sizeof (struct subnet), name);
	return rval;
}

#if	1
struct class *new_class (name)
	char *name;
{
	struct class *rval = dmalloc (sizeof (struct class), name);
	return rval;
}
#endif
struct shared_network *new_shared_network (name)
	char *name;
{
	struct shared_network *rval =
		dmalloc (sizeof (struct shared_network), name);
	return rval;
}

struct group *new_group (name)
	char *name;
{
	struct group *rval =
		dmalloc (sizeof (struct group), name);
	return rval;
}

struct protocol *new_protocol (name)
	char *name;
{
	struct protocol *rval = dmalloc (sizeof (struct protocol), name);
	return rval;
}

static struct lease_state *free_lease_states = NULL;

struct lease_state *new_lease_state (name)
	char *name;
{
	struct lease_state *rval;

	if (free_lease_states) {
		rval = free_lease_states;
		free_lease_states =
			(struct lease_state *)(free_lease_states -> next);
	} else {
		rval = dmalloc (sizeof (struct lease_state), name);
	}

	/* Penny 2002-12-31: in order to clear out the content of lease state to
	** 	prevent using lease state in the recycle area
	*/
	//memset(rval, 0, sizeof(*rval));

	return rval;
}

struct domain_search_list *new_domain_search_list (name)
	char *name;
{
	struct domain_search_list *rval =
		dmalloc (sizeof (struct domain_search_list), name);
	return rval;
}

struct name_server *new_name_server (name)
	char *name;
{
	struct name_server *rval =
		dmalloc (sizeof (struct name_server), name);
	return rval;
}

void free_name_server (ptr, name)
	struct name_server *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_domain_search_list (ptr, name)
	struct domain_search_list *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_lease_state (ptr, name)
	struct lease_state *ptr;
	char *name;
{
	if (ptr -> prl)
		dfree (ptr -> prl, name);
	ptr -> next = free_lease_states;
	free_lease_states = ptr;
}

void free_protocol (ptr, name)
	struct protocol *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_group (ptr, name)
	struct group *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_shared_network (ptr, name)
	struct shared_network *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_class (ptr, name)
	struct class *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_subnet (ptr, name)
	struct subnet *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_lease (ptr, name)
	struct lease *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_hash_bucket (ptr, name)
	struct hash_bucket *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_hash_table (ptr, name)
	struct hash_table *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_tree_cache (ptr, name)
	struct tree_cache *ptr;
	char *name;
{
	/* Penny adds free tree */
	free_tree_node(ptr->tree, "tree for free_tree_cache");
	ptr->tree = NULL;
	/* end of Penny adds free tree */
	ptr -> value = (unsigned char *)free_tree_caches;
	free_tree_caches = ptr;
}

void free_packet (ptr, name)
	struct packet *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_dhcp_packet (ptr, name)
	struct dhcp_packet *ptr;
	char *name;
{
	dfree ((VOIDPTR)ptr, name);
}

void free_tree_node(struct tree *ptr, char *name)
{
	if (ptr==0)
	return;
	/* Penny modified: Need to free all tree structure element */
	switch (ptr->op)
	{
		case TREE_CONCAT:
			if (ptr->data.concat.left != 0 && ptr->data.concat.left->op == (TREE_CONST || TREE_CONCAT))
			{
				free_tree(ptr->data.concat.left, "free tree concat left");
				ptr->data.concat.left = 0;
			}
			if (ptr->data.concat.right != 0 && ptr->data.concat.right->op == (TREE_CONST || TREE_CONCAT))
			{
				free_tree(ptr->data.concat.right, "free tree concat right");
				ptr->data.concat.right = 0;
			}
			break;
		case TREE_CONST:
			if (ptr->data.const_val.data != 0)
			{
				dhcp_free(ptr->data.const_val.data);
				ptr->data.const_val.data = 0;
			}
			break;
		case TREE_HOST_LOOKUP:
			DHCP_ERROR_Print(" \nDHCP_Server>Tree host lookup temporary diabled.");
			break;
		default:
			DHCP_ERROR_Print("\nDHCP_Server>Bad node id in tree.");
			break;
		
	}		
	/*if (ptr->data.limit.tree != 0 && ptr->data.limit.tree->op != 0)
	{
		free_tree(ptr->data.limit.tree);
		ptr->data.limit.tree = 0;
	}*/
	dfree ((VOIDPTR)ptr, name);
	ptr = 0;
} /* end of free_tree_node */

void free_tree(ptr, name)
	struct tree *ptr;
	char *name;
{
	if (ptr==0)
	return;
	dfree ((VOIDPTR)ptr, name);
}
