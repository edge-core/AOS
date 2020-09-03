#ifndef		_ALLOC_H
#define		_ALLOC_H

#include "dhcp_type.h"

VOIDPTR dmalloc(int, char *);
void dfree (void *, char *);
struct packet *new_packet (char *);
struct dhcp_packet *new_dhcp_packet (char *);
struct tree *new_tree (char *);
struct tree_cache *new_tree_cache (char *);
struct hash_table *new_hash_table (int, char *);
struct hash_bucket *new_hash_bucket (char *);
struct lease *new_lease (char *);
struct lease *new_leases (int, char *);
struct subnet *new_subnet (char *);
struct class *new_class (char *);
struct shared_network *new_shared_network (char *);
struct group *new_group(char *);
struct protocol *new_protocol(char *);
struct lease_state *new_lease_state(char *);
struct domain_search_list *new_domain_search_list(char *);
struct name_server *new_name_server (char *);
void free_name_server (struct name_server *, char *);
void free_domain_search_list (struct domain_search_list *, char *);
void free_lease_state(struct lease_state *, char *);
void free_protocol (struct protocol *, char *);
void free_group (struct group *, char *);
void free_shared_network(struct shared_network *, char *);
//void free_class(struct class *, char *);
void free_subnet (struct subnet *, char *);
void free_lease (struct lease *, char *);
void free_hash_bucket (struct hash_bucket *, char *);
void free_hash_table (struct hash_table *, char *);
void free_tree_cache(struct tree_cache *, char *);
void free_packet(struct packet *, char *);
void free_dhcp_packet(struct dhcp_packet *, char *);
void free_tree_node(struct tree *ptr, char *name);
void free_tree (struct tree *, char *);

#endif /* end of alloc.h */
