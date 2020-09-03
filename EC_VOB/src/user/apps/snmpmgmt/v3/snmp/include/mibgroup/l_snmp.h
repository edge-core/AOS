/* Copyright (C) 2001 - 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _L_SNMP_H
#define _L_SNMP_H


struct L_SNMP_VLAN {
  int vlanid;
};
struct L_SNMP_MAC_ADDR
{
  unsigned char addr[6];
};

void oid_copy_bytes2oid (oid oid[], void *addr, signed int len);
int oid2bytes (oid oid[], signed int len, void *addr);
int l_snmp_index_get (struct variable *v, oid * name, size_t * length,
                    unsigned int * index, int exact);
int l_snmp_2_index_get (struct variable *v, oid * name, size_t * length,
                    unsigned int * port, unsigned int * vlan, int exact);
 int l_snmp_3_index_get (struct variable *v, oid * name, size_t * length,
                    UI32_T * index1, UI32_T * index2, UI32_T * index3, int exact);
void l_snmp_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int index);
void l_snmp_2_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int index1, unsigned int index2);
void l_snmp_3_index_set (struct variable *v, oid * name, size_t * length,
                    UI32_T index1, UI32_T index2, UI32_T index3);
int l_snmp_mac_addr_index_get (struct variable *v, oid *name, size_t *length, 
		    struct L_SNMP_MAC_ADDR *addr, int exact);
void l_snmp_mac_addr_index_set (struct variable *v, oid *name, size_t *length,
		    struct L_SNMP_MAC_ADDR *addr);
int l_snmp_mac_addr_int_index_get (struct variable *v, oid *name, size_t *length, 
		    struct L_SNMP_MAC_ADDR *addr, int *idx, int exact);
void l_snmp_mac_addr_int_index_set (struct variable *v, oid *name, size_t *length,
		    struct L_SNMP_MAC_ADDR *addr, int idx );


int l_snmp_str_index_get (struct variable *v, oid *name, size_t *length, 
                UI8_T *str, int exact);
void l_snmp_str_index_set (struct variable *v, oid *name, size_t *length,
                UI8_T *str);
int l_snmp_str_int_index_get (struct variable *v, oid *name, size_t *length, 
                UI8_T *str, int *idx, int exact);
void l_snmp_str_int_index_set (struct variable *v, oid *name, size_t *length,
                UI8_T *str, int idx );
int l_snmp_long_index_get (struct variable *v, oid * name, size_t * length,
                    unsigned int *long_index, unsigned int *int_index, int exact);
void l_snmp_long_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int long_index, unsigned int int_index);
int l_snmp_int_mac_addr_index_get (struct variable *v, oid *name, size_t *length,
                    int *id, struct L_SNMP_MAC_ADDR *addr, int exact);
void l_snmp_int_mac_addr_index_set (struct variable *v, oid *name, size_t *length,
                    int id, struct L_SNMP_MAC_ADDR *addr);
int l_snmp_int_mac_addr_int_index_get (struct variable *v, oid *name, size_t *length,
                    int *id, struct L_SNMP_MAC_ADDR *addr, int *idx, int exact);
void l_snmp_int_mac_addr_int_index_set (struct variable *v, oid *name, size_t *length,
                    int id, struct L_SNMP_MAC_ADDR *addr, int idx);


int l_snmp_vlan_descend_cmp(const void *e1, const void *e2);
int l_snmp_vlan_ascend_cmp(const void *e1, const void *e2);
int l_snmp_port_descend_cmp(const void *e1, const void *e2);
int l_snmp_port_ascend_cmp(const void *e1, const void *e2);

/* bit string utilities. SNMP bit strings start at the MSBit and move to the 
 * right for increasing bit numbers. There must enough bytes in the string to
 * contain each multiple of 8 bits.
 */


void *l_bsearch_next(const void *key, const void *base0, size_t nelements,
             size_t element_size, int (*compar)(const void *, const void *));

/* support layer2 private MIB - QingfengZhang, Tuesday, January 17, 2006  */
int l_snmp_inst_port_index_get (struct variable *v, oid *name, size_t *length, 
		    unsigned int *inst, unsigned int *port, int exact);


/* Utility function to set the object name and INTEGER index. */
void l_snmp_inst_port_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int inst, unsigned int port);
#endif



