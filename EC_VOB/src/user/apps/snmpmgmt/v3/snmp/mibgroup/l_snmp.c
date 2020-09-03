/* Copyright (C) 2001 - 2004 IP Infusion, Inc. All Rights Reserved. */




#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_api.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#include "l_snmp.h"

#ifndef UINT8_MAX
#define UINT8_MAX	0xFF
#endif


/* copies a byte array into an oid, converting to unsigned int values. */

void
oid_copy_bytes2oid (oid oid[], void *addr, signed int len)
{
  signed int i;
  unsigned char *pnt = (unsigned char *) addr;
  
  for (i = 0; i < len; i++)
    oid[i] = *pnt++;
}

/* copy oid values to an unsigned byte array. 
 */

int
oid2bytes (oid oid[], signed int len, void *addr)
{

  signed int i;
  unsigned char *pnt = (unsigned char *) addr;
  
  for (i = 0; i < len; i++)
  {
    *pnt++ = (oid[i] > UINT8_MAX ? UINT8_MAX : oid[i] );
  }
  return 0;
}


/*
 * bsearch_next
 *
 * performs a binary search and returns the next element in the table. 
 * returns NULL if no such element exists.
 */
void *                  
l_bsearch_next(const void *key, const void *base0, size_t nelements, 
             size_t element_size, int (*compar)(const void *, const void *))
{
  const char *base = base0;
  size_t lim;
  int cmp = 0;
  const void *p = NULL, *pnext;

  for (lim = nelements; lim != 0; lim >>= 1)
  {
    p = base + (lim >> 1) * element_size;
    cmp = (*compar)(key, p);
    if (cmp == 0)
      break;
    if (cmp > 0)
    {
      lim--;
      base = (const char *)p + element_size;
    }
  }
  
  if ( lim == 0 && cmp < 0 )
      return (void *)p;
  
  pnext = (const char *)p + element_size;
  if ( (const char *)pnext < (const char *)base0 + (nelements * element_size) )
    return (void *)pnext;
  else
    return NULL;
}

/* Utility function to get INTEGER index.  */
int
l_snmp_index_get (struct variable *v, oid * name, size_t * length,
                    unsigned int  * index, int exact)
{

  int result, len;

  *index = 0;

  if(*length < v->namelen)
   result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
   result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);

  if (exact)
    {
      /* Check the length. */
      if (result != 0 || *length - v->namelen != 1)
        return -1;

      *index = name[v->namelen];
      return 0;
    }
  else                          /* GETNEXT request */
    {
      /* return -1 if greater than our our table entry */
      if (result > 0)
        return -1;
      else if (result == 0)
        {
          /* use index value if present, otherwise 0. */
          len = *length - v->namelen;
          if (len >= 1)
            *index = name[v->namelen];
          return 0;
        }
      else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
    }
}

/* Utility function to set the object name and INTEGER index. */
void
l_snmp_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int  index)
{
  name[v->namelen] = index;
  *length = v->namelen + 1;
}

/* Utility function to get a MAC address index.  */
int
l_snmp_mac_addr_index_get (struct variable *v, oid *name, size_t *length, 
		    struct L_SNMP_MAC_ADDR *addr, int exact)
{
  unsigned int len;
  
  if (exact)
    {
      /* Check the length. */
      if (*length - v->namelen != sizeof (struct L_SNMP_MAC_ADDR))
      	return -1;

      if ( oid2bytes (name + v->namelen, sizeof (struct L_SNMP_MAC_ADDR), addr) != 0 )
        return -1;
      
      return 0;
    }
  else
    {
      memset( addr, 0, sizeof(struct L_SNMP_MAC_ADDR) );

      len = *length - v->namelen;
      if (len > sizeof (struct L_SNMP_MAC_ADDR)) len = sizeof (struct L_SNMP_MAC_ADDR);

      if ( oid2bytes (name + v->namelen, len, addr) != 0 )
        return -1;
      
      return 0;
    }
  return -1;
}

/* Utility function to set a mac address index.  */
void
l_snmp_mac_addr_index_set (struct variable *v, oid *name, size_t *length,
		    struct L_SNMP_MAC_ADDR *addr)
{
  memcpy (name, v->name, v->namelen * sizeof(oid) );
  
  oid_copy_bytes2oid (name + v->namelen, addr, sizeof (struct L_SNMP_MAC_ADDR));
  *length = v->namelen + sizeof (struct L_SNMP_MAC_ADDR);
}

/* Utility function to get a MAC address + integer index.  */
int
l_snmp_2_index_get (struct variable *v, oid *name, size_t *length, 
		    unsigned int  *index1, unsigned int  *index2, int exact)
{
  int result, len;

  *index1 = 0;
  *index2 = 0;
  
 // result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
  
  if(*length < v->namelen)
   result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
   result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
   
  if (exact)
    {
      /* Check the length. */
      if (result != 0 || *length - v->namelen != 2)
      	return -1;
      
      *index1 = name[v->namelen];
      *index2 = name[v->namelen+1];
      return 0;
    }
  else {
    /* return -1 if greater than our our table entry */
      if (result > 0)
        return -1;
      else if (result == 0)
        {
          /* use index value if present, otherwise 0. */
          len = *length - v->namelen;
          if (len >= 2) {
            *index1 = name[v->namelen];
            *index2 = name[v->namelen+1];
          }
          return 0;
        }
      else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
    }
}

/* Utility function to set the object name and INTEGER index. */
void
l_snmp_2_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int  index1, unsigned int  index2)
{
  name[v->namelen] = index1;
  name[v->namelen+1] = index2;
  *length = v->namelen + 2;
}

int
l_snmp_3_index_get (struct variable *v, oid *name, size_t *length,
		    UI32_T  *index1, UI32_T  *index2, UI32_T  *index3, int exact)
{
  int result, len;

  *index1 = 0;
  *index2 = 0;
  *index3 = 0;

  //result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
  if(*length < v->namelen)
   result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
   result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);

  if (exact)
    {
      /* Check the length. */
      if (result != 0 || *length - v->namelen != 3)
      	return -1;

      *index1 = name[v->namelen];
      *index2 = name[v->namelen+1];
      *index3 = name[v->namelen+2];
      return 0;
    }
  else {
    /* return -1 if greater than our our table entry */
      if (result > 0)
        return -1;
      else if (result == 0)
        {
          /* use index value if present, otherwise 0. */
          len = *length - v->namelen;
          if (len >= 3) {
            *index1 = name[v->namelen];
            *index2 = name[v->namelen+1];
            *index3 = name[v->namelen+2];
          }
          return 0;
        }
      else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
    }
}

void
l_snmp_3_index_set (struct variable *v, oid * name, size_t * length,
                    UI32_T  index1, UI32_T  index2, UI32_T  index3)
{
  name[v->namelen] = index1;
  name[v->namelen+1] = index2;
  name[v->namelen+2] = index3;
  *length = v->namelen + 3;
}
/* Utility function to get a MAC address + integer index.  */
int
l_snmp_mac_addr_int_index_get (struct variable *v, oid *name, size_t *length, 
		    struct L_SNMP_MAC_ADDR *addr, int *idx, int exact)
{
  int maclen, len;
  int result;


  if(*length < v->namelen)
   result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
   result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);

  if (exact){
      if (*length - v->namelen != sizeof (struct L_SNMP_MAC_ADDR) + 1)
      	return -1;

      if(oid2bytes (name + v->namelen, sizeof (struct L_SNMP_MAC_ADDR), addr) != 0 )
        return -1;
      
      *idx = name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR)];
      return 0;
      
  }else{
      memset( addr, 0, sizeof(struct L_SNMP_MAC_ADDR) );
      *idx = 0;
     if (result > 0)
        return -1;
     else if (result == 0){
        
        
       maclen = len = *length - v->namelen;

       if (maclen > sizeof (struct L_SNMP_MAC_ADDR)) 
          maclen = sizeof (struct L_SNMP_MAC_ADDR);
        
       if ( oid2bytes (name + v->namelen, maclen, addr) != 0 )
          return -1;
        
       len = len - maclen;
       if ( len > 0 )
          *idx = name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR)];
        
        return 0;
    
  }else{
  
        memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
        return 0;
  }
    
  }

  return -1;
}

/* Utility function to set a mac address + integer index.  */
void
l_snmp_mac_addr_int_index_set (struct variable *v, oid *name, size_t *length,
		    struct L_SNMP_MAC_ADDR *addr, int idx )
{
  memcpy (name, v->name, v->namelen * sizeof(oid) );
  
  oid_copy_bytes2oid (name + v->namelen, addr, sizeof (struct L_SNMP_MAC_ADDR));
  *length = v->namelen + sizeof (struct L_SNMP_MAC_ADDR);

  name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR)] = idx;
  *length += 1;

}


int
l_snmp_long_index_get (struct variable *v, oid * name, size_t * length,
                    unsigned int  *long_index, unsigned int  *int_index, int exact)
{
    int result, len;

    *int_index = 0;	
    *long_index = 0;

    //result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
    if(*length < v->namelen)
     result = snmp_oid_compare (name, *length, v->name, v->namelen);
    else
     result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
    

    if (exact)
    {
      /* Check the length. */
      if (result != 0 || *length - v->namelen != 2)
        return -1;

      *long_index = name[v->namelen];
      *int_index = name[v->namelen + 1];
      return 0;
    }
    else                          /* GETNEXT request */
    {
        /* return -1 if greater than our our table entry */
        if (result > 0)
          return -1;
        else if (result == 0)
        {
          /* use index value if present, otherwise 0. */
          len = *length - v->namelen;
          if (len >= 1)
          {
              *long_index = name[v->namelen];
              *int_index = name[v->namelen + 1];
          }
            
          return 0;
        }
        else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
      }
}

/* Utility function to set the object name and long+integer index. */
void
l_snmp_long_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int  long_index, unsigned int  int_index)
{
  name[v->namelen] = long_index;
  name[v->namelen + 1] = int_index;
  *length = v->namelen + 2;
}

/* Utility function to get a integer + MAC address.  */
int
l_snmp_int_mac_addr_index_get (struct variable *v, oid *name, size_t *length, int *id, struct L_SNMP_MAC_ADDR *addr, int exact)
{
  unsigned int maclen;
  
  if (exact)
  {
    /* Check the length. */
    if (*length - v->namelen != sizeof (struct L_SNMP_MAC_ADDR) + 1)
        return -1;

    *id = name[v->namelen];

    if ( oid2bytes (name + v->namelen + 1, sizeof (struct L_SNMP_MAC_ADDR), addr->addr) != 0 )
        return -1;

    return 0;    
  }
  else
  {
    memset( addr, 0, sizeof(struct L_SNMP_MAC_ADDR) );
    *id = 0;

    maclen = *length - v->namelen;
    /*EPR:NULL
     *problem: if have't check the length,the id may get the error value
     *so when mib table dot1dTpFdbTable(oid 1.3.6.1.2.1.17.4.3) get-next,it will fail
     *DanXie,Nov,3*/
    if(maclen>0)
        *id = name[v->namelen];
    
    if (maclen > sizeof (struct L_SNMP_MAC_ADDR)) 
      maclen = sizeof (struct L_SNMP_MAC_ADDR);

    if ( oid2bytes (name + v->namelen + 1, maclen, addr->addr) != 0 )
      return -1;
    
    return 0;
  }

  return -1;
}

/* Utility function to set a integer + mac address index.  */
void
l_snmp_int_mac_addr_index_set (struct variable *v, oid *name, size_t *length, int id, struct L_SNMP_MAC_ADDR *addr)
{
  memcpy (name, v->name, v->namelen * sizeof(oid) );

  name[v->namelen] = id;
  *length = v->namelen + 1;
  
  oid_copy_bytes2oid (name + v->namelen + 1, addr->addr, sizeof (struct L_SNMP_MAC_ADDR));
  *length += sizeof (struct L_SNMP_MAC_ADDR);

  return;
}

/* Utility function to get a integer + MAC address + integer index.  */
int
l_snmp_int_mac_addr_int_index_get (struct variable *v, oid *name, size_t *length, int *id, struct L_SNMP_MAC_ADDR *addr, int *idx, int exact)
{

  unsigned int maclen, len;
  int result;


  if(*length < v->namelen)
   result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
   result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);

  if (exact)
    {
      /* Check the length. */
      if (*length - v->namelen != sizeof (struct L_SNMP_MAC_ADDR) + 2)
          return -1;
      
      *id = name[v->namelen];
      
      if ( oid2bytes (name + v->namelen + 1, sizeof (struct L_SNMP_MAC_ADDR), addr->addr) != 0 )
          return -1;
      
      *idx = name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR) + 1];
      return 0; 

    }
  else                          /* GETNEXT request */
    {
      /* return -1 if greater than our our table entry */
      if (result > 0)
        return -1;
      else if (result == 0)
        {
          memset( addr, 0, sizeof(struct L_SNMP_MAC_ADDR) );
          *id = 0;
          *idx = 0;
          
          maclen = len = *length - v->namelen;
          /*EPR: ES4827G-20-2018
           *problem: if have't check the length,the id may get the error value
           *so when get-next,it will fail
           *DanXie,Nov,3*/
          if(len>0)
             *id = name[v->namelen];
          
          if (maclen > sizeof (struct L_SNMP_MAC_ADDR)) 
            maclen = sizeof (struct L_SNMP_MAC_ADDR);
          
          if ( oid2bytes (name + v->namelen + 1, maclen, addr->addr) != 0 )
            return -1;
          
          len = len - maclen;
          if ( len > 0 )
            *idx = name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR) + 1];
          
          return 0;

        }
      else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
    }
 return -1;

}

/* Utility function to set a integer + mac address + integer index.  */
void
l_snmp_int_mac_addr_int_index_set (struct variable *v, oid *name, size_t *length, int id, struct L_SNMP_MAC_ADDR *addr, int idx)
{
  memcpy (name, v->name, v->namelen * sizeof(oid) );

  name[v->namelen] = id;
  *length = v->namelen + 1;
  
  oid_copy_bytes2oid (name + v->namelen + 1, addr, sizeof (struct L_SNMP_MAC_ADDR));
  *length += sizeof (struct L_SNMP_MAC_ADDR);

  name[v->namelen + sizeof (struct L_SNMP_MAC_ADDR) +1] = idx;
  *length += 1;

  return;
}

/* Utility function to get a string index.  */
int l_snmp_str_index_get (struct variable *v, oid *name, size_t *length, 
                UI8_T *str, int exact)
{    
    int len;
    unsigned int str_len = name[v->namelen];
    int result ;
    //= snmp_oid_compare (name, v->namelen, v->name, v->namelen); 

    if(*length < v->namelen)
     result = snmp_oid_compare (name, *length, v->name, v->namelen);
    else
     result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
   
    if (exact)
    {
        /* Check the length. */
        if (result != 0 || *length-v->namelen != str_len + 1)
            return -1;

        /* get str form name */
        if ( oid2bytes (name + v->namelen + 1, str_len, str) != 0 )
            return -1;
        
        return 0;
    }
    else /* GETNEXT request */
    {
        if (result > 0)
            return -1;
        /*Digna modified 2006/9/11*/
        else if (result == 0)
        {
            /* use index value if present, otherwise 0. */
            len = *length - v->namelen - 1;
			
            if (len >=1)
                if ( oid2bytes (name + v->namelen + 1, len, str) != 0 )/*Digna modified 2006/9/11*/
                  return -1;   
            
            return 0;
        }
        else
        {
            /* set the user's oid to be ours */
            memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
            return 0;
        }
    }
}

/* Utility function to set a string index.  */
void l_snmp_str_index_set (struct variable *v, oid *name, size_t *length,
                UI8_T *str)
{
    int str_len = strlen((char*)str);
    
    memcpy(name, v->name, v->namelen * sizeof(oid));

    name[v->namelen] = str_len;/*Digna modified 2006/9/11*/
    
    oid_copy_bytes2oid(name + v->namelen + 1, str, str_len);
    
    *length = v->namelen + str_len +1;

    return;
}

/* Utility function to get a string + integer index.  */
int l_snmp_str_int_index_get (struct variable *v, oid *name, size_t *length, 
                UI8_T *str, int *idx, int exact)
{
    int len;
    unsigned int str_len = name[v->namelen];    
    int result ;
   // result= snmp_oid_compare (name, v->namelen, v->name, v->namelen);

    if(*length < v->namelen)
     result = snmp_oid_compare (name, *length, v->name, v->namelen);
    else
     result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
   

    if (exact)
    {
        /* Check the length. */
        if (result != 0 || *length-v->namelen != str_len+2)
            return -1;

        /* get str form name */
        if ( oid2bytes (name + v->namelen + 1, str_len, str) != 0 )
            return -1;  

        *idx = name[v->namelen + str_len + 1];    

        return 0; 
    }
    else /* GETNEXT request */
    { 
        if (result > 0)
            return -1;
        else if (result == 0)
        {
            /* use index value if present, otherwise 0. */
            len = *length - v->namelen - 2;/*Digna modified 2006/9/11*/
            if (len >= 1 )
            	{
                if ( oid2bytes (name + v->namelen + 1, len, str) != 0 )/*Digna modified 2006/9/11*/
                  return -1;   
                *idx = name[v->namelen + len + 1];/*Digna modified 2006/9/11*/
            	}
			
            return 0;
        }
        else
        {
            /* set the user's oid to be ours */
            memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
            return 0;
        }
    }
}

/* Utility function to set a string + integer index.  */
void l_snmp_str_int_index_set (struct variable *v, oid *name, size_t *length,
                UI8_T *str, int idx )
{
    int str_len = strlen((char *)str);    
    
    memcpy(name, v->name, v->namelen * sizeof(oid) );

    name[v->namelen] = str_len;
    
    oid_copy_bytes2oid(name + v->namelen + 1, str, str_len);
    
    name[v->namelen + 1 + str_len] = idx;
    
    *length = v->namelen + str_len + 2;
    
    return;
}




int
bitstring_count(char *bstring, int bstring_length)
{
  int i;
  char c;
  int n = 0;
 
  for (i=0; i<bstring_length; i++)
  {
    c = *bstring++;
    if (c)
      do
        n++;
      while((c = c & (c-1)));
  }
        
  return( n );
}

/* comparison function for sorting ports in the L2 port vlan table. Index
 * is the PORT.  DESCENDING ORDER 
 */
int 
l_snmp_port_descend_cmp(const void *e1, const void *e2)
{
  int vp1;  
  int vp2;

  if (e1 == e2)
    return 0;

  vp1 = *(int *)e1;
  vp2 = *(int *)e2;
  if ( vp1 < vp2 )
      return 1;
  else 
    if ( vp1 > vp2 )
      return -1;
    else
      return 0;
}

/* comparison function for sorting ports in the L2 port vlan table. Index
 * is the PORT.  ASCENDING ORDER 
 */
int 
l_snmp_port_ascend_cmp(const void *e1, const void *e2)
{
  int vp1;  
  int vp2;

  if (e1 == e2)
    return 0;

  vp1 = *(int *)e1;
  vp2 = *(int *)e2;

  if ( vp1 < vp2 )
      return -1;
  else 
    if ( vp1 > vp2 )
      return 1;
    else
      return 0;
}

/* comparison function for sorting VLANs in the L2 port vlan table. Index
 * is the PORT.  DESCENDING ORDER 
 */
int 
l_snmp_vlan_descend_cmp(const void *e1, const void *e2)
{
  struct L_SNMP_VLAN *vp1 = *(struct L_SNMP_VLAN **)e1;
  struct L_SNMP_VLAN *vp2 = *(struct L_SNMP_VLAN **)e2;

  /* if pointer is the same data is the same */
  if (vp1 == vp2)
    return 0;
  if (vp1 == 0)
    return 1;
  if (vp2 == 0)
    return -1;

  if ( vp1->vlanid < vp2->vlanid )
      return 1;
  else 
    if ( vp1->vlanid > vp2->vlanid )
      return -1;
    else
      return 0;
}

/* comparison function for sorting VLANs in the L2 port vlan table. Index
 * is the PORT.  ASCENDING ORDER 
 */
int 
l_snmp_vlan_ascend_cmp(const void *e1, const void *e2)
{
  struct L_SNMP_VLAN *vp1 = *(struct L_SNMP_VLAN **)e1;
  struct L_SNMP_VLAN *vp2 = *(struct L_SNMP_VLAN **)e2;
  
  /* if pointer is the same data is the same */
  if (vp1 == vp2)
    return 0;
  if (vp1 == 0)
    return 1;
  if (vp2 == 0)
    return -1;

  if ( vp1->vlanid < vp2->vlanid )
      return -1;
  else 
    if ( vp1->vlanid > vp2->vlanid )
      return 1;
    else
      return 0;
}



int l_snmp_inst_port_index_get (struct variable *v, oid *name, size_t *length, 
		    unsigned int  *inst, unsigned int  *port, int exact)
{
  int result, len;
  //result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
  if(*length < v->namelen)
     result = snmp_oid_compare (name, *length, v->name, v->namelen);
  else
     result = snmp_oid_compare (name, v->namelen, v->name, v->namelen);
   
  if (exact)
    {
      /* Check the length. */
      /*for Bug ES4827G-20-01448----DanXie,Sep,7*/
      if (result != 0 || *length - v->namelen != 2)
      	return -1;
      
      *inst = name[v->namelen];
      *port = name[v->namelen+1];
      return 0;
    }
  else {
    /* return -1 if greater than our our table entry */
      if (result > 0)
        return -1;
      else if (result == 0)
        {
          /* use index value if present, otherwise 0. */
          len = *length - v->namelen;
          if (len >= 2) {
            *inst = name[v->namelen];
            *port = name[v->namelen+1];
          }
          return 0;
        }
      else
        {
          /* set the user's oid to be ours */
          memcpy (name, v->name, ((int) v->namelen) * sizeof (oid));
          return 0;
        }
    }
}

/* Utility function to set the object name and INTEGER index. */
void
l_snmp_inst_port_index_set (struct variable *v, oid * name, size_t * length,
                    unsigned int  inst, unsigned int  port)
{
  name[v->namelen] = inst;
 *length = v->namelen + 1;/*inst size=1*/
  name[v->namelen+1] = port;
 *length += 1; /*port size=1*/
}


