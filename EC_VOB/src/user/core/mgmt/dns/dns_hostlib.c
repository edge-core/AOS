/* MODULE NAME: dns_hostlib.c
 * PURPOSE:
 *       This module provide functions to deal with local host table
 *
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-09-06    -- Wiseway , created
 *       2002-10-31    -- Wiseway   modified for convention
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <string.h>
#include <stdlib.h>
#include <stdio.h> /*maggie liu remove warning*/

#include "l_inet.h"
#include "sys_module.h"
#include "l_mm.h"

#include "dns.h"
#include "dns_hostlib.h"
#include "dns_type.h"

extern I32_T dns_inet_addr(register const char *cp);

//#include "semLib.h"
/* data type declarations */

static HostEntry_PTR DNS_HOSTLIB_SearchName(const char * name);
static HostEntry_PTR DNS_HOSTLIB_SearchAddr(L_INET_AddrIp_T *netAddr_p);

static void DNS_HOSTLIB_AddDnsHostEntryToList(HostEntry_PTR src_p);
static void DNS_HOSTLIB_DelDnsHostEntryFromList(HostEntry_PTR src_p);
static BOOL_T DNS_HOSTLIB_AddIpToDnsHostEntry(
    HostEntry_PTR src_p, L_INET_AddrIp_T *addr_p);

static BOOL_T DNS_HOSTLIB_DelIpFromDnsHostEntry(
    HostEntry_PTR src_p, L_INET_AddrIp_T *addr_p);

static BOOL_T DNS_HOSTLIB_GetNextInOneDnsHostAddrEntry(
    UI32_T host_idx, BOOL_T get_first, L_INET_AddrIp_T *addr_p);

static BOOL_T DNS_HOSTLIB_AddDnsHostEntryToArray(HostEntry_PTR src_p);
static BOOL_T DNS_HOSTLIB_DelDnsHostEntryFromArray(HostEntry_PTR src_p);
static BOOL_T DNS_HOSTLIB_IsDnsHostEntryValid(HostEntry_PTR src_p);
static BOOL_T DNS_HOSTLIB_IsAddrTypeValid(UI32_T addr_type);
static void DNS_HOSTLIB_LocalGetDefHostName(char *host_name_p);

/* STATIC VARIABLE DECLARATIONS
 */
static HostEntry_PTR hostList = NULL;
static HostEntry_PTR    hostList_par[SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE] = {NULL};
static UI32_T   number_of_host_entry =   0;
static I8_T targetName[MAXHOSTNAMELEN + 1];

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : DNS_HOSTLIB_HostTblInit
 *
 * PURPOSE:
 *      Initialize th local host table. Set hostList with two default host entries.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 */
void DNS_HOSTLIB_HostTblInit(void)
{
    L_INET_AddrIp_T ip;
    char *my_addr = "127.0.0.1";

    memset(&ip, 0 ,sizeof(ip));
    L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, my_addr, (L_INET_Addr_T *)&ip, sizeof(ip));

    DNS_HOSTLIB_HostAdd("localhost",&ip);
}

/* FUNCTION NAME :DNS_HOSTLIB_HostAdd
 *
 * PURPOSE:
 *      add a host to the host table
 *      This routine adds a host name to the local host table.
 *      The host table has one entry per Internet address.
 *      More than one name may be used for an address.
 *      Additional host names are added as aliases.
 *
 * INPUT:
 *      I8_T * -- host name
 *      I8_T * -- host addr in standard Internet format
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if the host table is full, the host name is already entered,
 *      the parameters are invalid, or memory is insufficient.
 *
 * NOTES:
 *      none
 */
int DNS_HOSTLIB_HostAdd(char *hostName, L_INET_AddrIp_T *hostAddr_p)
{
    HostEntry_PTR   nameHostent_P;
    HostEntry_PTR   addrHostent_P;
    int size = MIN(strlen((char *)hostName), MAXHOSTNAMELEN);  /*commented by wiseway 2002-11-01 */

    if(strlen((char *)hostName) > MAXHOSTNAMELEN)
        return DNS_ERROR;

    if(hostName == NULL || hostAddr_p == NULL)
    {
        return DNS_ERROR;
    }


    if( hostAddr_p->addrlen == 0 )
    {
        return DNS_ERROR;
    }

    nameHostent_P = DNS_HOSTLIB_SearchName(hostName);
    addrHostent_P = DNS_HOSTLIB_SearchAddr(hostAddr_p);

    if(nameHostent_P != NULL && addrHostent_P != NULL)  /* all already in the list */
    {
        return DNS_ERROR;
    }   /* only ip in the list, then add alias */
    else if(nameHostent_P == NULL && addrHostent_P != NULL)
    {
        return DNS_ERROR;
/* at presetnt, does not support alias,
 *   need to return error for this case
 */
#if 0
        if( addrHostent_P->hostName[MAXHOSTNAMENUM-1].name[0] == '\0' )
        {
            int j;
            int i;

            for(i = 1; i < MAXHOSTNAMENUM; i++)
            {
                if(addrHostent_P->hostName[i].name[0] == '\0')
                {
                    strncpy((char *)(addrHostent_P->hostName[i].name), (char *)hostName, size);
                    addrHostent_P->hostName[i].name[size] = '\0';
                    return DNS_OK;
                }
                else if( (strlen((char *)hostName)<strlen((char *)(addrHostent_P->hostName[i].name
                    ))) )
                {
                    for( j=MAXHOSTNAMENUM-1 ; j>i ; j-- )
                    {
                        strcpy((char *)addrHostent_P->hostName[j].name, (char *)addrHostent_P->hostName[j-1].name);
                    }
                    strncpy((char *)addrHostent_P->hostName[i].name, (char *)hostName, size);
                    addrHostent_P->hostName[i].name[size] = '\0';
                    return DNS_OK;
                }
                else if( (strlen((char *)hostName)==strlen((char *)addrHostent_P->hostName[i].name)) && (strncmp((char *)hostName, (char *)addrHostent_P->hostName[i].name, size)<0) )
                {
                    for( j=MAXHOSTNAMENUM-1 ; j>i ; j-- )
                    {
                        strcpy((char *)addrHostent_P->hostName[j].name, (char *)addrHostent_P->hostName[j-1].name);
                    }
                    strncpy((char *)addrHostent_P->hostName[i].name, (char *)hostName, size);
                    addrHostent_P->hostName[i].name[size] = '\0';
                    return DNS_OK;
                }
            }
        }
#endif

    }   /* only name in the list, then add the ip */
    else if(nameHostent_P != NULL && addrHostent_P == NULL)
    {
        if (FALSE == DNS_HOSTLIB_AddIpToDnsHostEntry(nameHostent_P, hostAddr_p))
                    {
            return DNS_ERROR;
        }

                return DNS_OK;
            }

    else /* none is in the list, so we creat a new entry */
    {
        if( number_of_host_entry < MAX_NBR_OF_HOST_TABLE_SIZE )/*isiah*/
        {
            nameHostent_P = (HostEntry_T *)L_MM_Malloc(sizeof(HostEntry_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_HOSTLIB_HOSTADD));
            if(nameHostent_P == NULL)
            {
                return DNS_ERROR;
            }

            if (FALSE == DNS_HOSTLIB_AddDnsHostEntryToArray(nameHostent_P))
            {
                L_MM_Free(nameHostent_P);
                return DNS_ERROR;
            }

            /* init the entry */
            memset(nameHostent_P, 0, sizeof(HostEntry_T));
            /* assign the val */
            strncpy((char *)nameHostent_P->hostName[0].name, (char *)hostName, size);
            nameHostent_P->hostName[0].name[size] = '\0';
            memcpy(&nameHostent_P->netAddr[0], hostAddr_p, sizeof(L_INET_AddrIp_T));

            /* link into the list */

            DNS_HOSTLIB_AddDnsHostEntryToList(nameHostent_P);
            return DNS_OK;
        }
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_HostShow
 * PURPOSE:
 *      This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 */
void DNS_HOSTLIB_HostShow(void)
{
    HostEntry_PTR hostentry_P;
    int i, j;
    char ip[L_INET_MAX_IPADDR_STR_LEN+1];

    printf("hostname\t\tinet address   \t\talias   \n");
    printf("--------\t\t------------   \t\t-----   \n\n");
    for(hostentry_P = hostList; hostentry_P != 0; hostentry_P = hostentry_P->next_P)
    {
        j = 1;
        printf("%-8s\t\t", hostentry_P->hostName[0].name);
        for(i = 0; i < MAXHOSTIPNUM; i++)
        {
            if(hostentry_P->netAddr[i].addrlen!= 0)
            {
                if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&hostentry_P->netAddr[i],
                                                                   ip,
                                                                   sizeof(ip)))
                {
                    continue;
                }

                printf("%-15s\t\t", ip );

                for( ; j < MAXHOSTNAMENUM; j++)
                {
                    if(hostentry_P->hostName[j].name[0]!='\0')
                    {
                        printf("%s\n        \t\t", hostentry_P->hostName[j].name);
                        j++;
                        break;
                    }
                }
                if(j == MAXHOSTNAMENUM)     /*MAXHOSTNAMENUM - 1 to MAXHOSTNAMENUM ,wiseway 2002-11-01*/
                {
                    printf("\n        \t\t");
                }
            }
            if(i == MAXHOSTIPNUM - 1)
            {
                for( ; j < MAXHOSTNAMENUM; j++)
                {
                    if(hostentry_P->hostName[j].name[0]!='\0')
                    {
                        printf("               \t\t%s\n        \t\t", hostentry_P->hostName[j].name);
                    }
                }
                printf("\n");
            }
        }   /* for i < MAXHOSTIPNUM end */
    }
}

/* FUNCTION NAME : DNS_HOSTLIB_HostDelete
 *
 * PURPOSE:
 *      This routine deletes a <name,ip address> from the local host table.  If <name> is
 *      a host name, only the ip address is deleted.  If <name> is a host name alias,
 *      the alias and ip address are deleted.
 *
 * INPUT:
 *      I8_T * -- host name or alias
 *      I8_T * -- host addr in standard Internet format
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *      Afeter this operation, this cases may occurs:
 *      only hostname or hostname&alias exists ,the ip addresses have all been deleted.
 *      none
 */
int DNS_HOSTLIB_HostDelete(char *name, L_INET_AddrIp_T *addr_p)
{
    HostEntry_PTR   hostentry_P;
    int name_index, ip_index, j;

    if(name == NULL || addr_p == NULL)
    {
        return DNS_ERROR;
    }

    /* search inet address */
    hostentry_P = DNS_HOSTLIB_SearchAddr(addr_p);
    if(hostentry_P == NULL)
    {
        return DNS_ERROR;
    }

    /* delete the alias name */

    /* for each host name entry
     */
    for(name_index = 0; name_index < MAXHOSTNAMENUM; name_index++)
    {
        /* if name is non-empty
         */
        if(hostentry_P->hostName[name_index].name[0]!= '\0')
        {
            /* if name matches
             */
            if(strcmp((char *)hostentry_P->hostName[name_index].name, (char *)name) == 0)
            {
                /* if "name_index" in not at 0, delete
                 */
                if (name_index!=0)
                {
                    hostentry_P->hostName[name_index].name[0] = '\0';

                    /* isiah, shift data */
                    for (j = name_index+1; j < MAXHOSTNAMENUM; j++)
                    {
                        strcpy((char *)hostentry_P->hostName[j-1].name, (char *)hostentry_P->hostName[j].name);
                    }

                    strcpy((char *)hostentry_P->hostName[MAXHOSTNAMENUM-1].name, "");
                }

                /* for each IP address
                 */
                for (ip_index = 0; ip_index < MAXHOSTIPNUM; ip_index++)
                {
                    /* if IP address matches, delete
                     */
                    if (L_INET_CompareInetAddr((L_INET_Addr_T *) & hostentry_P->netAddr[ip_index],
                        (L_INET_Addr_T *) addr_p, 0) == 0)
                    {
                        memset(&hostentry_P->netAddr[ip_index], 0, sizeof(L_INET_AddrIp_T));

                        for (j = ip_index+1; j < MAXHOSTIPNUM; j++)
                        {
                            memcpy(&hostentry_P->netAddr[j-1], &hostentry_P->netAddr[j], sizeof(L_INET_AddrIp_T));
                        }

                        memset(&hostentry_P->netAddr[MAXHOSTIPNUM-1], 0, sizeof(L_INET_AddrIp_T));
                    }
                }

                /*isiah.2003-01-14. avoid follow status, only hostname or hostname&alias exists ,the ip addresses have all been deleted.*/
				if( FALSE == DNS_HOSTLIB_IsDnsHostEntryValid(hostentry_P) )
                {
                    DNS_HOSTLIB_DelDnsHostEntryFromArray(hostentry_P);
				    DNS_HOSTLIB_DelDnsHostEntryFromList(hostentry_P);
                    L_MM_Free(hostentry_P);
                    hostentry_P = NULL;
                }
                return DNS_OK;
            }
        }
    }
    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_HostsClear
 *
 * PURPOSE:
 *      This routine clears all the entries in the hostList .
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 */
void DNS_HOSTLIB_HostsClear(void)
{
    HostEntry_PTR hostEntry_p=NULL;

       while  (NULL!=hostList)
      {
        hostEntry_p=hostList;
        hostList=hostList->next_P;
        L_MM_Free(hostEntry_p);
        hostEntry_p = NULL;
       }

    number_of_host_entry = 0;
    memset(hostList_par, 0, sizeof(hostList_par));
}

/* FUNCTION NAME : DNS_HOSTLIB_HostNameDelete
 *
 * PURPOSE:
 *      This routine deletes a host name from the local host table.  If <name> is
 *      a host name, the host entry is deleted.  If <name> is a host name alias,
 *      only the alias is deleted.
 *
 * INPUT:
 *      I8_T * -- host name or alias
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *      none
 */
int DNS_HOSTLIB_HostNameDelete(char * name)
{
    HostEntry_PTR   hostentry_P;
    HostEntry_PTR   preHostentry_P;
    int i;

    if (name == NULL)
    {
//      errnoSet(S_hostLib_INVALID_PARAMETER);
        return DNS_ERROR;
    }

    /* convert from string to int format
     */

    hostentry_P = DNS_HOSTLIB_SearchName(name);

//  semTake (&hostListSem, WAIT_FOREVER);

    /* search inet address
     */
    if(hostentry_P == NULL)
    {
        return DNS_ERROR;
    }

    /* delete the hostname and the entry
     */
    if (strcmp((char *)hostentry_P->hostName[0].name, (char *)name) == 0)
    {
        if (hostentry_P == hostList)
        {
            hostList = hostentry_P->next_P;
        }
        else
        {
            preHostentry_P = hostList;

            while(preHostentry_P->next_P != hostentry_P)
            {
                preHostentry_P = preHostentry_P->next_P;
            }

            preHostentry_P->next_P = hostentry_P->next_P;
        }

        /* delete from array
         */
        DNS_HOSTLIB_DelDnsHostEntryFromArray(hostentry_P);
        DNS_HOSTLIB_DelDnsHostEntryFromList(hostentry_P);

        L_MM_Free(hostentry_P);
        hostentry_P = NULL;
        number_of_host_entry -= 1;
        return DNS_OK;
    }

    /* delete the alias name
     */
    for(i = 1; i < MAXHOSTNAMENUM; i++)
    {
        if (hostentry_P->hostName[i].name[0] != '\0')
        {
            if (strcmp((char *)hostentry_P->hostName[i].name, (char *)name) == 0)
            {
                hostentry_P->hostName[i].name[0] = '\0';
                return DNS_OK;
            }
        }
    }

    return DNS_ERROR;
}




/* FUNCTION NAME : DNS_HOSTLIB_HostTblSearchByName
 *
 * PURPOSE:
 *      This routine returns a list of the Internet address of a host that has
 *      been added to the host table by DNS_MGR_HostAdd(), and store it in the
 *      addr[].
 *
 * INPUT:
 *      const I8_T *   -- host name to be searched.
 *      struct in_addr -- Ip addr to be searched.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if not find the name int the hostList.
 *
 * NOTES:
 *      This function is called by called by hostGetByName.
 */
int DNS_HOSTLIB_HostTblSearchByName(const char *name, L_INET_AddrIp_T addr_ar[], int *addr_count_p)
{
    HostEntry_PTR hostentry_P;
    int i;

    *addr_count_p=0;
    hostentry_P = DNS_HOSTLIB_SearchName(name);

    if(hostentry_P != NULL)
    {
        for(i = 0; i < MAXHOSTIPNUM; i++)
        {
            memcpy(&addr_ar[i], &hostentry_P->netAddr[i], sizeof(L_INET_AddrIp_T));

            if (addr_ar[i].addrlen != 0)
            {
                (*addr_count_p)++;
            }
        }
        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_HostGetByName
 *
 * PURPOSE:
 *      This routine returns a list of the Internet address of a host that has
 *      been added to the host table by DNS_MGR_HostAdd(), and store it in the
 *      addr[].
 *
 * INPUT:
 *      const I8_T *   -- host name or alias.
 *
 * OUTPUT:
 *      L_INET_AddrIp_T addr_ar[] --Ip addr array where the searched IP addrs will be put.
 *
 * RETURN:
 *      int
 *
 * NOTES:
 *      none
 */
int DNS_HOSTLIB_HostGetByName(const char *name, L_INET_AddrIp_T addr_ar[])        /* name of host */
{
    int status;
    int addr_count=0;

    if(name == NULL)
    {
        return DNS_ERROR;
    }

    /* Search the host table using the host name as the key */
    status = DNS_HOSTLIB_HostTblSearchByName(name, addr_ar,&addr_count);
    if (addr_count==0)
    {
        status=DNS_ERROR;
    }

    return status;
}


/* FUNCTION NAME : DNS_HOSTLIB_HostTblSearchByAddr
 *
 * PURPOSE:
 *      This routine finds the host name by its Internet address and copies it to
 *      <name>.  The buffer <name> should be preallocated with (MAXHOSTNAMELEN + 1)
 *      bytes of memory and is NULL-terminated unless insufficient space is
 *      provided.
 *
 * INPUT:
 *      struct in_addr -- inet address of host
 *
 * OUTPUT:
 *      I8_T * --  buffer to hold name
 *
 * RETURN:
 *      DNS_OK , or DNS_ERROR if not find.
 *
 * NOTES:
 *      Host name aliases will not be returned.
 *      none
 */
int DNS_HOSTLIB_HostTblSearchByAddr(L_INET_AddrIp_T *netAddr_p, I8_T *name)
{
    HostEntry_PTR   hostentry_P;
    int     n;

    /* search for internet address */
    hostentry_P = DNS_HOSTLIB_SearchAddr(netAddr_p);

    if(hostentry_P != NULL)
    {
        n = strlen((char *)hostentry_P->hostName[0].name);
        strncpy((char *)name, (char *)hostentry_P->hostName[0].name, MIN(n, MAXHOSTNAMELEN));
        name[MIN(n, MAXHOSTNAMELEN)] = '\0';
        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_HostGetByAddr
 *
 * PURPOSE:
 *      This routine finds the host name by its Internet address and copies it to
 *      <name>.  The buffer <name> should be preallocated with (MAXHOSTNAMELEN + 1)
 *      bytes of memory and is NULL-terminated unless insufficient space is
 *      provided.
 *      This routine does not look for aliases.  Host names are limited to
 *      MAXHOSTNAMELEN (from hostLib.h) characters.
 *
 * INPUT:
 *      const I8_T * addr -- inet address of host.
 *
 * OUTPUT:
 *      I8_T *name     -- buffer to hold name..
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if buffer is invalid or the host is unknown
 *
 * NOTES:
 *       none
 */
int DNS_HOSTLIB_HostGetByAddr(const I8_T * addr,I8_T * name)
{
    int status;
    L_INET_AddrIp_T netAddr;

    if(name == NULL)
    {
        return DNS_ERROR;
    }

    memset(&netAddr, 0, sizeof(netAddr));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
        (char *)addr, (L_INET_Addr_T *)&netAddr, sizeof(netAddr)))
    {
        return DNS_ERROR;
    }

    /* Search the host table using the host address as the key */

    status = DNS_HOSTLIB_HostTblSearchByAddr(&netAddr, name);

    return status;
}


/* FUNCTION NAME : DNS_HOSTLIB_SetHostName
 *
 * PURPOSE:
 *      This routine sets the target machine's symbolic name, which can be used
 *      for identification.
 *
 * INPUT:
 *      const I8_T * -- machine name
 *      int          -- length of name
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if nameLen is larger then MAXHOSTNAMELEN
 *
 * NOTES:
 *      none
 */
int DNS_HOSTLIB_SetHostName(const I8_T * name,  int nameLen)
{
    if(name != NULL && (unsigned)nameLen <= MAXHOSTNAMELEN)
    {
        strcpy((char *)targetName, (char *)name);
        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME :  DNS_HOSTLIB_GetHostName
 * PURPOSE:
 *      This routine gets the target machine's symbolic name, which can be used
 *      for identification.
 *
 * INPUT:
 *      int     -- length of name
 *
 * OUTPUT:
 *      I8_T * -- buffer to hold machine name .
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if nameLen is smaller then the lenth of targetName.
 *
 * NOTES:
 *      none
 */
int DNS_HOSTLIB_GetHostName(I8_T * name,int nameLen)
{
    if(name != NULL && strlen((char *)targetName) <= (unsigned)nameLen)
    {
        strcpy((char *)name, (char *)targetName);
        return DNS_OK;
    }

    return DNS_ERROR;
}


/* FUNCTION NAME : DNS_HOSTLIB_AddHostEntry
 * PURPOSE:
 *      This funciton add the specified struct to the local host table.
 *
 *
 * INPUT:
 *      HostEntry_PTR -- a pointer to a struct to be added to the local host table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *       This function will be called by OM module.
 */
int DNS_HOSTLIB_AddHostEntry(HostEntry_PTR host_entry_t_p)
{
    int i=0;
    HostEntry_PTR   hostentry_P;
    if (NULL==host_entry_t_p)
        return DNS_ERROR;
    hostentry_P = (HostEntry_T *)L_MM_Malloc(sizeof(HostEntry_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_HOSTLIB_ADDHOSTENTRY));
    if(hostentry_P == NULL)
        {
            return DNS_ERROR;
        }
        /* init the entry */
    for(i = 0; i < MAXHOSTNAMENUM; i++)
        {
            hostentry_P->hostName[i].name[0] = '\0';
        }
    for(i = 0; i < MAXHOSTIPNUM; i++)
        {
            memset(&hostentry_P->netAddr[i], 0, sizeof(L_INET_AddrIp_T));
        }
    memcpy(hostentry_P,host_entry_t_p,sizeof(HostEntry_T)); /* link into the list */
    hostentry_P->next_P = hostList;
    hostList = hostentry_P;

    return DNS_OK;
}


/* FUNCTION NAME : DNS_HOSTLIB_GetNextHostEntry
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      int *host_index_p -- previous active index of HostEntry_T struct.
 *
 * OUTPUT:
 *      int *host_index_p -- current active index of HostEntry_T struct.
 *      HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by OM module.
 *      The initial value is -1.
 */
int DNS_HOSTLIB_GetNextHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    HostEntry_PTR hostentry_P;
    int i=0;
    if ((NULL==host_index_p)||(NULL==dns_host_entry_t_p)||((*host_index_p)<-1))
        return DNS_ERROR;
    hostentry_P = hostList;
    if (NULL==hostentry_P)
        return DNS_ERROR;
    (*host_index_p)++;
    while (hostentry_P)
        {
        /* if dnsHostEntry does not have valid ip address,
         * only SNMP can see this entry..
         */
	    if (FALSE == DNS_HOSTLIB_IsDnsHostEntryValid(hostentry_P))
	    {
            hostentry_P=hostentry_P->next_P;
            continue;
	    }

            if ((*host_index_p)==i)
                {
                    memcpy(dns_host_entry_t_p,hostentry_P,sizeof(HostEntry_T));
                    return DNS_OK;
                }
            hostentry_P=hostentry_P->next_P;
            i++;
        }
    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_HOSTLIB_GetHostEntryBySnmp
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format
 *
 * OUTPUT:
 *      none.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsHostEntryBySnmp
 */
int DNS_HOSTLIB_GetHostEntryBySnmp(char *hostname_p, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR hostentry_p;
    int i=0;
    L_INET_AddrIp_T netAddr;

    /* BODY */
    if((NULL==hostname_p)||(NULL==hostaddr_p))
    {
        return DNS_ERROR;
    }

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
        hostaddr_p, (L_INET_Addr_T *)&netAddr, sizeof(netAddr)))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    if(NULL==hostentry_p)
    {
        return DNS_ERROR;
    }

    while(hostentry_p)
    {
        if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) == 0 )
        {
            for( i=0 ; i<MAXHOSTIPNUM ; i++ )
            {
                if (L_INET_CompareInetAddr((L_INET_Addr_T *) & hostentry_p->netAddr[i],
                    (L_INET_Addr_T *) &netAddr, 0) == 0)
                {
                    return DNS_OK;
                }
            }
            return DNS_ERROR;
        }
        else
        {
            hostentry_p=hostentry_p->next_P;
        }
    }
    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_HOSTLIB_GetNextHostEntryBySnmp
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *      I8_T   *hostname_p   --  next active host name.
 *      I8_T   *hostaddr_p   --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsNextHostEntryBySnmp
 */
int DNS_HOSTLIB_GetNextHostEntryBySnmp(UI8_T *hostname_p, UI8_T *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR hostentry_p;
    int i=0;
    L_INET_AddrIp_T netAddr;

    /* BODY */
    if((NULL==hostname_p)||(NULL==hostaddr_p))
    {
        return DNS_ERROR;
    }

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
        (char*)hostaddr_p, (L_INET_Addr_T *)&netAddr, sizeof(netAddr)))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    if(NULL==hostentry_p)
    {
        return DNS_ERROR;
    }

        while(hostentry_p)
        {
            if( strlen((char *)hostentry_p->hostName[0].name) == strlen((char *)hostname_p) )
            {
                if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) >= 0 )
                {
                    if( netAddr.addrlen != 0 )
                    {
                        for( i=0 ; i<MAXHOSTIPNUM ; i++ )
                        {
                            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & hostentry_p->netAddr[i],
                                (L_INET_Addr_T *) &netAddr, 0) == 0)
                            {
                                if( (i+1) < MAXHOSTIPNUM && (hostentry_p->netAddr[i+1].addrlen !=0) )
                                {
                                    L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[i+1], (char *)hostaddr_p, sizeof(hostaddr_p));
                                        return DNS_OK;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[0], (char *)hostaddr_p, sizeof(hostaddr_p));
                            return DNS_OK;
                        }

                    hostentry_p = hostentry_p->next_P;
                    if( hostentry_p )
                    {
                        strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                        L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[0], (char *)hostaddr_p, sizeof(hostaddr_p));
                        return DNS_OK;
                    }
                    else
                    {
                        return DNS_ERROR;
                    }

                }
            }
            else if( strlen((char *)hostentry_p->hostName[0].name) > strlen((char *)hostname_p) )
            {
                strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[0], (char *)hostaddr_p, sizeof(hostaddr_p));
                return DNS_OK;
            }
            hostentry_p = hostentry_p->next_P;
        }
        return DNS_ERROR;
}



/* FUNCTION NAME : DNS_HOSTLIB_GetAliasNameBySnmp
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *      none.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsAliasNameBySnmp
 */
int DNS_HOSTLIB_GetAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR hostentry_p;
    int i=0;

    /* BODY */
    if((NULL==hostname_p)||(NULL==aliasname_p))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    if(NULL==hostentry_p)
    {
        return DNS_ERROR;
    }

    while(hostentry_p)
    {
        if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) == 0 )
        {
            for( i=1 ; i<MAXHOSTNAMENUM ; i++ )
            {
                if( strcmp((char *)hostentry_p->hostName[i].name, (char *)aliasname_p) == 0 )
                {
                    return DNS_OK;
                }
            }
            return DNS_ERROR;
        }
        else
        {
            hostentry_p=hostentry_p->next_P;
        }
    }
    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_HOSTLIB_GetNextAliasNameBySnmp
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *      I8_T   *hostname_p   --  next active host name.
 *      I8_T   *aliasname_p  --  next active alias name.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetNextDnsAliasNameBySnmp
 */
int DNS_HOSTLIB_GetNextAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR hostentry_p;
    int i=0;

    /* BODY */
    if((NULL==hostname_p)||(NULL==aliasname_p))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    if(NULL==hostentry_p)
    {
        return DNS_ERROR;
    }

    while(hostentry_p)
    {
        if( strlen((char *)hostentry_p->hostName[0].name) == strlen((char *)hostname_p) )
        {
            if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) >= 0 )
            {
                for( i=1 ; i<MAXHOSTNAMENUM ; i++ )
                {
                    if( strlen((char *)hostentry_p->hostName[i].name) == strlen((char *)aliasname_p) )
                    {
                        if( strcmp((char *)hostentry_p->hostName[i].name, (char *)aliasname_p) > 0 )
                        {
                            strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                            strcpy((char *)aliasname_p, (char *)hostentry_p->hostName[i].name);
                            return DNS_OK;
                        }
                    }
                    else if( strlen((char *)hostentry_p->hostName[i].name) > strlen((char *)aliasname_p) )
                    {
                        strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                        strcpy((char *)aliasname_p, (char *)hostentry_p->hostName[i].name);
                        return DNS_OK;
                    }
                }
            }
        }
        else if( strlen((char *)hostentry_p->hostName[0].name) > strlen((char *)hostname_p) )
        {
            if( hostentry_p->hostName[1].name[0] != '\0' )
            {
                strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                strcpy((char *)aliasname_p, (char *)hostentry_p->hostName[1].name);
                return DNS_OK;
            }
        }
        hostentry_p = hostentry_p->next_P;
    }
    return DNS_ERROR;
}


/* PENDING:
 * This seems to be a linked list which sorts name length first,
 * and then name value, possibly used by SNMP.
 * This code causes a PGRelief warning pgr0060 on the variable "prevHostent_P",
 * as it may be NULL but is used to point (->) to its content.
 * If this code for sure will not be used, it will be removed from the code.
 */
#if 0  /*!*/ /* PENDING: DNS_HOSTLIB_*DnsHostEntryByNameAndIndex; suspected unused code */
/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostEntryByNameAndIndex
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T    *hostname_p --  current active host name.
 *      I32_T   *index_p    --  current active index of host addr for host name.
 *      I8_T    *hostaddr_p --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *      I8_T    *hostname_p --  next active host name.
 *      I32_T   *index_p    --  next active index of host addr for host name.
 *      I8_T    *hostaddr_p --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetNextDnsHostEntryByNameAndIndex
 */
int DNS_HOSTLIB_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR hostentry_p;
    L_INET_AddrIp_T netAddr;
    I32_T   l_index;

    /* BODY */
    if((NULL==hostname_p)||(NULL==hostaddr_p)||(NULL==index_p))
    {
        return DNS_ERROR;
    }

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
        hostaddr_p, (L_INET_Addr_T *)&netAddr, sizeof(netAddr)))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    if(NULL==hostentry_p)
    {
        return DNS_ERROR;
    }

    while(hostentry_p)
    {
        if( strlen((char *)hostentry_p->hostName[0].name) == strlen((char *)hostname_p) )
        {
            if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) >= 0 && ((*index_p)<SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP))
            {
/*isiah.2003-11-10*/
/* for don't sort ip address when input */
                l_index = *index_p/* + 1*/;
                if( hostentry_p->netAddr[l_index].addrlen != 0 )
                {
                    L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[l_index], hostaddr_p, sizeof(hostaddr_p));
                    *index_p = l_index + 1;
                    return DNS_OK;
                }
                hostentry_p = hostentry_p->next_P;
                if( hostentry_p )
                {
                        strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
                        L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[0], hostaddr_p, sizeof(hostaddr_p));
                        *index_p = 1;
                       return DNS_OK;
                }
                else
                {
                    return DNS_ERROR;
                }

            }
        }
        else if( strlen((char *)hostentry_p->hostName[0].name) > strlen((char *)hostname_p) )
        {
               strcpy((char *)hostname_p, (char *)hostentry_p->hostName[0].name);
               L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[0], hostaddr_p, sizeof(hostaddr_p));
               *index_p = 1;
               return DNS_OK;
        }
        hostentry_p = hostentry_p->next_P;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function adds a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      *hostname_p --  host name.
 *      index       --  index of host addr for host name.
 *      ip_addr     --  ip addr will be added as a host name.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, L_INET_AddrIp_T *addr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR   hostentry_p;
    HostEntry_PTR   nameHostent_P;
    HostEntry_PTR   addrHostent_P;
    HostEntry_PTR   prevHostent_P  = NULL;

    int i=0;
    int size = MIN(strlen((char *)hostname_p), MAXHOSTNAMELEN);

    /* BODY */
    if((NULL==hostname_p))
    {
        return DNS_ERROR;
    }

    /* start from head
     */
    hostentry_p = hostList;

    /* while each host, which is not NULL
     */
    while(hostentry_p)
    {
        /* if name matches
         */
        if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) == 0 )
        {
            /* if address matches
             * range searched: 0-based array index from 0 to less that (index - 1);
             *                 1-based user index from 1 to (index - 1)
             */
            for( i=0 ; i<(index-1) ; i++ )
            {
                if( (L_INET_CompareInetAddr((L_INET_Addr_T *) &h ostentry_p->netAddr[i],
                        (L_INET_Addr_T *) addr_p, 0) == 0)
                    || (hostentry_p->netAddr[i].addrlen == 0) )
                {
                    return DNS_ERROR;
                }
            }
            memcpy(&hostentry_p->netAddr[index-1], addr_p, sizeof(L_INET_AddrIp_T));
            return DNS_OK;
        }

        /* next
         */
        hostentry_p = hostentry_p->next_P;
    }

    /* none is in the list, so we creat a new entry */
    nameHostent_P = DNS_HOSTLIB_SearchName(hostname_p);

    /* PENDING:
     * It is not known why we must input index as 1 for insersion.
     * The call chain is as follows, but no one calls the PMGR.
     * So, this section cannot be tested for correctness or "pgr".
     *
     * DNS_PMGR_SetDnsHostEntryByNameAndIndex
     * .. DNS_MGR_SetDnsHostEntryByNameAndIndex
     *    -> DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex
     */

    /* if index is 1, and not found
     */
    if( (index == 1) && (nameHostent_P == NULL) )
    {
        /* if not full
         */
        if( number_of_host_entry < MAX_NBR_OF_HOST_TABLE_SIZE )  /*isiah*/
        {
            nameHostent_P = (HostEntry_T *)L_MM_Malloc(sizeof(HostEntry_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_HOSTLIB_SETDNSHOSTENTRYBYNAMEANDINDEX));
            if(nameHostent_P == NULL)
            {
                return DNS_ERROR;
            }

            /* init the entry */
            memset(nameHostent_P, 0, sizeof(HostEntry_T));

            /* assign the val */
            strncpy((char *)nameHostent_P->hostName[0].name, (char *)hostname_p, size);
            nameHostent_P->hostName[0].name[size] = '\0';
            memcpy(&nameHostent_P->netAddr[0], addr_p, sizeof(L_INET_AddrIp_T));

            /* link into the list */
            /*isiah*/

            /* if list is empty
             */
            if( hostList == NULL )
            {
                nameHostent_P->next_P = hostList;
                hostList = nameHostent_P;
                number_of_host_entry += 1;
                return DNS_OK;
            }

            /* list not empty, start from head
             */
            addrHostent_P = hostList;
            i = 0;

            /* while current pointer
             */
            while(addrHostent_P)
            {
                /* if new name is shorter
                 */
                if( (strlen((char *)hostname_p) < strlen((char *)addrHostent_P->hostName[0].name)) )
                {
                    if( i==0 )
                    {
                        nameHostent_P->next_P = addrHostent_P;
                        hostList = nameHostent_P;
                        number_of_host_entry += 1;
                        return DNS_OK;
                    }
                    else
                    {
                        nameHostent_P->next_P = addrHostent_P;
                        prevHostent_P->next_P = nameHostent_P;  /* prevHostend_P: pending solution to pgr0060 */
                        number_of_host_entry += 1;
                        return DNS_OK;
                    }
                }

                /* if new name has same length, and content is "less"
                 */
                else if( (strlen((char *)hostname_p) == strlen((char *)addrHostent_P->hostName[0].name))
                    && (strncmp((char *)hostname_p, (char *)addrHostent_P->hostName[0].name, size) < 0) )
                {
                    if( i==0 )
                    {
                        nameHostent_P->next_P = addrHostent_P;
                        hostList = nameHostent_P;
                        number_of_host_entry += 1;
                        return DNS_OK;
                    }
                    else
                    {
                        nameHostent_P->next_P = addrHostent_P;
                        prevHostent_P->next_P = nameHostent_P;  /* prevHostend_P: pending solution to pgr0060 */
                        number_of_host_entry += 1;
                        return DNS_OK;
                    }
                }

                /* if new name is "greater"
                 */
                prevHostent_P = addrHostent_P;  /* prevHostend_P: assigned here */
                addrHostent_P = addrHostent_P->next_P;
                i++;
            }

            /*isiah*/
            /* while loop ended, append to end
             */
            prevHostent_P->next_P = nameHostent_P;  /* prevHostend_P: pending solution to pgr0060 */
            number_of_host_entry += 1;
            return DNS_OK;
        }
    }

    return DNS_ERROR;
}

/*maggie liu, ES4827G-FLF-ZZ-00243*/
/* FUNCTION NAME : DNS_HOSTLIB_DeleteDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function delete a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      *hostname_p --  host name.
 *      index       --  index of host addr for host name.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_DeleteDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR   hostentry_p;

    int i=0;

    /* BODY */
    if((NULL==hostname_p))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    while(hostentry_p)
    {
        if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) == 0 )
        {
            /*maggie liu, ES4827G-FLF-ZZ-00243*/
            if(hostentry_p->netAddr[index-1].addrlen != 0)
            {
                for( i=(index-1) ; i<MAXHOSTIPNUM ; i++ ) /*maggie liu, ES3628BT-FLF-ZZ-00147*/
                {
                    memcpy(&hostentry_p->netAddr[i], &hostentry_p->netAddr[i+1], sizeof(L_INET_AddrIp_T));
                }

                memset(&hostentry_p->netAddr[MAXHOSTIPNUM-1], 0, sizeof(L_INET_AddrIp_T));
            }

            if( hostentry_p->netAddr[0].addrlen == 0 )
            {
                DNS_HOSTLIB_HostNameDelete(hostentry_p->hostName[0].name);
            }
            return DNS_OK;
        }
        hostentry_p = hostentry_p->next_P;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function get a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      I8_T    *hostname_p --  host name.
 *      UI32_T  index       --  index of host addr for host name.
 *
 * OUTPUT:
 *      I8_T    *hostaddr_p --  host addr in standard Internet format.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_GetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    HostEntry_PTR   hostentry_p;

    /* BODY */
    if((NULL==hostname_p)||(NULL==hostaddr_p))
    {
        return DNS_ERROR;
    }

    hostentry_p = hostList;
    while(hostentry_p)
    {
        if( strcmp((char *)hostentry_p->hostName[0].name, (char *)hostname_p) == 0 )
        {
            L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[index-1], hostaddr_p, sizeof(hostaddr_p));
            return DNS_OK;
        }
        hostentry_p = hostentry_p->next_P;
    }

    return DNS_ERROR;
}
#endif  /*!*/ /* 0; PENDING: DNS_HOSTLIB_*DnsHostEntryByNameAndIndex; suspected unused code */

/* FUNCTION NAME : DNS_HOSTLIB_CreateDnsHostEntry
 * PURPOSE: To create a new host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'hostname1', 'hostname2', 'hostname3'...
 */
BOOL_T DNS_HOSTLIB_CreateDnsHostEntry(UI32_T host_idx)
{
    HostEntry_PTR   cur_p;
    BOOL_T          ret = FALSE;

    if ((host_idx < 1) || (host_idx > SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        return ret;
    }

    if (hostList_par[host_idx -1] == NULL)
    {
        cur_p = (HostEntry_T *)L_MM_Malloc(sizeof(HostEntry_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_HOSTLIB_SETDNSHOSTENTRYBYNAMEANDINDEX));
        if(cur_p != NULL)
        {
            memset(cur_p, 0, sizeof(HostEntry_T));
            DNS_HOSTLIB_LocalGetDefHostName(cur_p->hostName[0].name);
            hostList_par [host_idx -1] = cur_p;
            DNS_HOSTLIB_AddDnsHostEntryToList(cur_p);
            ret = TRUE;
        }
    }
    else
    {
        /* old entry exists, just return false
         */
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_DestroyDnsHostEntry(UI32_T host_idx)
{
    BOOL_T          ret = FALSE;

    if ((host_idx < 1) || (host_idx > SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        return ret;
    }

    ret = TRUE;

    if (hostList_par[host_idx -1] != NULL)
    {
        DNS_HOSTLIB_DelDnsHostEntryFromList(hostList_par [host_idx -1]);
        L_MM_Free(hostList_par [host_idx -1]);
        hostList_par [host_idx -1] = NULL;
    }
    else
    {
        /* old entry does not exist, return true for delete
         * bcz row_status is allowed to set destroy on unexist entry
         */
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostEntry
 * PURPOSE: To modify a hostname to the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 *          hostname_p -- pointer to hostname content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. hostname_p[0] == '\0' is not valid
 */
BOOL_T DNS_HOSTLIB_SetDnsHostEntry(UI32_T host_idx, char *hostname_p)
{
    HostEntry_PTR   cur_p;
    BOOL_T          ret = FALSE;

    if ((NULL == hostname_p) ||
        (strlen(hostname_p) > SYS_ADPT_DNS_MAX_NAME_LENGTH) ||
        (hostname_p[0] == '\0') ||
        (host_idx < 1) ||
        (host_idx > SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        return ret;
    }

    /* check if old entry exist ?
     */
    if (hostList_par [host_idx -1] != NULL)
    {
        /* same as old name ?
         */
        if (strcmp(hostname_p, hostList_par[host_idx -1]->hostName[0].name) == 0)
        {
            ret = TRUE;
        }
        else
        {
            /* to check:
             *   same host name can not be added to different host entry
             */
            cur_p = DNS_HOSTLIB_SearchName(hostname_p);
            if ((NULL != cur_p) && (cur_p != hostList_par [host_idx -1]))
            {
                return ret;
            }

            DNS_HOSTLIB_DelDnsHostEntryFromList(hostList_par [host_idx -1]);

            strcpy(hostList_par [host_idx -1]->hostName[0].name, hostname_p);
            DNS_HOSTLIB_AddDnsHostEntryToList(hostList_par [host_idx -1]);

            ret = TRUE;
        }
    }
    else
    {
        /* old entry does not exist, return false
         */
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostEntry
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetDnsHostEntry(UI32_T host_idx, char *hostname_p)
{
    HostEntry_PTR   cur_p;
    BOOL_T          ret = FALSE;

    if ((NULL != hostname_p) && (0 < host_idx) && (host_idx <= MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        cur_p = hostList_par[host_idx-1];

        if (cur_p != NULL)
        {
            memcpy(hostname_p, cur_p->hostName[0].name, sizeof(cur_p->hostName[0]));
            ret = TRUE;
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostEntry
 * PURPOSE: To get next entry from the dnsHostEntry table.
 * INPUT  : host_idx_p   -- index of dnsHostEntry
 *                          (1-based, 0 to get the first,
 *                           key to search the entry)
 * OUTPUT : host_idx_p   -- next index of dnsHostEntry
 *          hostname_p   -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetNextDnsHostEntry(UI32_T *host_idx_p, char *hostname_p)
{
    HostEntry_PTR   cur_p;
    UI32_T          cur_idx;
    BOOL_T          ret = FALSE;

    if ((NULL != host_idx_p) && (NULL != hostname_p))
    {
        cur_idx = *host_idx_p +1;

        for ( ;cur_idx <= MAX_NBR_OF_HOST_TABLE_SIZE; cur_idx++)
        {
            cur_p = hostList_par[cur_idx-1];

            if (cur_p != NULL)
            {
                *host_idx_p = cur_idx;
                memcpy(hostname_p, cur_p->hostName[0].name, sizeof(cur_p->hostName[0]));
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostAddrEntry
 * PURPOSE: To add/delete an ip address to/from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to operate with (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_p are
 *                        keys to search the entry)
 *		    is_add    -- true to add/ false to delete
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 *          4. 0.0.0.0 is not a valid input.
 *          5. one addr can not be added to two different host entry.
 */
BOOL_T DNS_HOSTLIB_SetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add)
{
    HostEntry_PTR   cur_p;
    BOOL_T          ret = FALSE;

    if ((NULL == addr_p) || (host_idx < 1) ||
        (host_idx > SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        return ret;
    }

    switch (addr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
            break;

#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
            break;

        case L_INET_ADDR_TYPE_IPV6Z:
            break;
#endif

        default:
            return ret;
    }

    if ((TRUE == DNS_HOSTLIB_IsAddrTypeValid(addr_p->type)) &&
        (0 != addr_p->addrlen))
    {
        cur_p = DNS_HOSTLIB_SearchAddr(addr_p);
        if (TRUE == is_add)
        {
            /* addr already exists should not add again,
             * so only add addr when host entry can not be found
             */
            if (NULL == cur_p)
            {
                ret = DNS_HOSTLIB_AddIpToDnsHostEntry(hostList_par[host_idx-1], addr_p);
            }
        }
        else
        {
            if ((NULL != cur_p) && (cur_p == hostList_par[host_idx-1]))
            {
                ret = DNS_HOSTLIB_DelIpFromDnsHostEntry(hostList_par[host_idx-1], addr_p);
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostAddrEntry
 * PURPOSE: To get entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to get from (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_p are
 *                        keys to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p)
{
    HostEntry_PTR   cur_p;
    UI32_T          i;
    BOOL_T          ret = FALSE;

    if ((NULL != addr_p) && (TRUE == DNS_HOSTLIB_IsAddrTypeValid(addr_p->type)) &&
        (0 < host_idx) && (host_idx <= MAX_NBR_OF_HOST_TABLE_SIZE))
    {
        cur_p = hostList_par[host_idx-1];

        if (cur_p != NULL)
        {
            for (i = 0; i < MAXHOSTIPNUM; i++)
            {
                if (L_INET_CompareInetAddr((L_INET_Addr_T *) addr_p, (L_INET_Addr_T *) & cur_p->netAddr[i],
                    0) == 0)
                {
                    ret = TRUE;
                    break;
                }
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostAddrEntry
 * PURPOSE: To get next entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx_p  -- index of dnsHostAddrEntry to get from
 *                         (1-based, 0 to get the first)
 *		    addr_p      -- ip address content
 *                         (host_idx_p, addr_p are
 *                          keys to search the entry)
 * OUTPUT : host_idx_p  -- next index of dnsHostAddrEntry
 *          addr_p      -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p)
{
    UI32_T  cur_idx;
    BOOL_T  ret = FALSE, get_first = FALSE;

    if ((NULL != host_idx_p) && (NULL != addr_p))
    {
        cur_idx = *host_idx_p;

        if (cur_idx == 0)
        {
            cur_idx = 1;
            get_first = TRUE;
        }

        for ( ; cur_idx <= MAX_NBR_OF_HOST_TABLE_SIZE; cur_idx++)
        {
            ret = DNS_HOSTLIB_GetNextInOneDnsHostAddrEntry
                            (cur_idx, get_first, addr_p);

            if (TRUE == ret)
            {
                *host_idx_p = cur_idx;
                break;
            }

            get_first = TRUE;
        }
    }

    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME : DNS_HOSTLIB_SearchName
 *
 * PURPOSE:
 *		This function find the entry which has a hostname or alias "name"
 *
 * INPUT:
 *		const I8_T * -- host name
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		HostEntry_PTR
 *
 * NOTES:
 *		none
 */
static HostEntry_PTR DNS_HOSTLIB_SearchName(const char * name)
{
	HostEntry_PTR hostentry_P;
	int i;

	for(hostentry_P = hostList; hostentry_P != NULL; hostentry_P = hostentry_P->next_P)
	{
		for(i = 0; i < MAXHOSTNAMENUM; i++)
		{
			if(hostentry_P->hostName[i].name != NULL)
			{
				if(strcmp(hostentry_P->hostName[i].name, name) == 0)
				{
					return hostentry_P;
				}
			}
		}
	}

	return NULL;
}


/* FUNCTION NAME : DNS_HOSTLIB_SearchAddr
 *
 * PURPOSE:
 *		This function find the entry which has an ip addr "netAddr"
 *
 *
 * INPUT:
 *		struct in_addr -- Ip address
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		HostEntry_PTR  -- a pointer to the found host entry which has an ip addr "netAddr"
 *
 * NOTES:
 *		 none.
 */
static HostEntry_PTR DNS_HOSTLIB_SearchAddr(L_INET_AddrIp_T *netAddr_p)
{
    HostEntry_PTR hostentry_P;
    int i;

    for(hostentry_P = hostList; hostentry_P != NULL; hostentry_P = hostentry_P->next_P)
    {
        for(i = 0; i < MAXHOSTIPNUM; i++)
        {
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & hostentry_P->netAddr[i],
                (L_INET_Addr_T *) netAddr_p, 0) == 0)
            {
                return hostentry_P;
            }
        }
    }
    return NULL;
}


/* FUNCTION NAME : DNS_HOSTLIB_AddDnsHostEntryToList
 * PURPOSE: To add an entry to hostList.
 * INPUT  : src_p -- entry pointer to be added
 * OUTPUT : none.
 * RETURN : none.
 * NOTES  : sorted by 1. length of hotname
 *                    2. hostname
 */
static void DNS_HOSTLIB_AddDnsHostEntryToList(HostEntry_PTR src_p)
{
    HostEntry_PTR   cur_p, prv_p;

    if (hostList == NULL)
    {
        src_p->next_P = hostList;
        hostList = src_p;
        number_of_host_entry += 1;
    }
    else
    {
        prv_p = NULL;
        cur_p = hostList;

        /* insert into correct position
         */
        while (cur_p)
        {
/* bcz new dnsHostEntry didn't use HostName as index,
 * it's not necessary to sort the HostName by length
 */
#if 0
            cur_name_len = strlen(cur_p->hostName[0].name);
            if (src_name_len < cur_name_len)
            {
                break;
            }
            else if ((src_name_len == cur_name_len) &&
                     (strcmp(src_p->hostName[0].name, cur_p->hostName[0].name)<0))
#endif
            if(strcmp(src_p->hostName[0].name, cur_p->hostName[0].name)<0)
            {
                break;
            }

            prv_p = cur_p;
            cur_p = cur_p->next_P;
        }

        if (NULL == prv_p)
        {
            src_p->next_P = hostList;
            hostList = src_p;
        }
        else
        {
            src_p->next_P = prv_p->next_P;
            prv_p->next_P = src_p;
        }

        number_of_host_entry += 1;
    }
}

/* FUNCTION NAME : DNS_HOSTLIB_DelDnsHostEntryFromList
 * PURPOSE: To delete an entry from hostList.
 * INPUT  : src_p -- entry pointer to be deleted
 * OUTPUT : none.
 * RETURN : none.
 * NOTES  : none.
 */
static void DNS_HOSTLIB_DelDnsHostEntryFromList(HostEntry_PTR src_p)
{
    HostEntry_PTR   tmp_p;

    if (NULL != src_p)
    {
        if (hostList == src_p)
        {
            hostList = hostList->next_P;
            number_of_host_entry -= 1;
        }
        else
        {
            tmp_p = hostList;
            while (NULL != tmp_p)
            {
                if (tmp_p->next_P == src_p)
                {
                    tmp_p->next_P = src_p->next_P;
                    number_of_host_entry -= 1;
                    break;
                }

                tmp_p = tmp_p->next_P;
            }
        }

        src_p->next_P = NULL;
    }
}

/* FUNCTION NAME : DNS_HOSTLIB_AddIpToDnsHostEntry
 * PURPOSE: To add an ip address to the HostEntry.
 * INPUT  : src_p     -- entry pointer to add ip address to
 *		    addr_p    -- ip address content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. don't do valid checking for the arguments....
 */
static BOOL_T DNS_HOSTLIB_AddIpToDnsHostEntry(
    HostEntry_PTR src_p, L_INET_AddrIp_T *addr_p)
{
    UI32_T  i, j;
    BOOL_T  ret = FALSE;

    if (NULL != src_p)
    {
        if (TRUE == DNS_HOSTLIB_IsAddrTypeValid(addr_p->type))
        {
            /* find the position to insert
             */
            for (i = 0; i < MAXHOSTIPNUM; i++)
            {
                if ( (src_p->netAddr[i].addrlen == 0)  /* no more */
                    || (addr_p->type < src_p->netAddr[i].type)  /* or, type is less */
                    || ( (addr_p->type == src_p->netAddr[i].type)  /* or, type is equal */
                        && (memcmp(addr_p->addr, src_p->netAddr[i].addr,
                            src_p->netAddr[i].addrlen) < 0) )  /* and address is less */
                    || ( (addr_p->type == src_p->netAddr[i].type)  /* or, type is equal */
                        && (memcmp(addr_p->addr, src_p->netAddr[i].addr,
                            src_p->netAddr[i].addrlen) == 0)  /* and address is equal */
                        && (addr_p->zoneid < src_p->netAddr[i].zoneid) ) /* and zone ID is less */
                    )
                {
                    break;
                }
            }

            if (i < MAXHOSTIPNUM)
            {
                if (src_p->netAddr[i].addrlen != 0)
                {
                    if (src_p->netAddr[MAXHOSTIPNUM-1].addrlen == 0)
                    {
                        /* move other entry to tail
                         */
                        for (j = MAXHOSTIPNUM-1; j > i; j--)
                        {
                            memcpy(&src_p->netAddr[j], &src_p->netAddr[j-1], sizeof(src_p->netAddr[j]));
                        }

                        memset(&src_p->netAddr[i], 0, sizeof(src_p->netAddr[i]));
                    }
                }

                /* insert the new ip if a empty space is found
                 */
                if (src_p->netAddr[i].addrlen == 0)
                {
                    memcpy (&src_p->netAddr[i], addr_p, sizeof(src_p->netAddr[i]));
                    ret = TRUE;
                }
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_DelIpFromDnsHostEntry
 * PURPOSE: To delete an ip address from the HostEntry.
 * INPUT  : src_p     -- entry pointer to delete ip address from
 *		    addr_p    -- ip address content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. don't do valid checking for the arguments....
 */
static BOOL_T DNS_HOSTLIB_DelIpFromDnsHostEntry(
    HostEntry_PTR src_p, L_INET_AddrIp_T *addr_p)
{
    UI32_T  i, j;
    BOOL_T  ret = FALSE;

    if (NULL != src_p)
    {
        if (TRUE == DNS_HOSTLIB_IsAddrTypeValid(addr_p->type))
        {
            /* find the position to delete
             */
            for (i = 0; i < MAXHOSTIPNUM; i++)
            {
                if (L_INET_CompareInetAddr((L_INET_Addr_T *) addr_p, (L_INET_Addr_T *) & src_p->netAddr[i],
                    0) == 0)
                {
                    for(j = i+1; j < MAXHOSTIPNUM; j++)
                    {
                        memcpy(&src_p->netAddr[j-1], &src_p->netAddr[j], sizeof(L_INET_AddrIp_T));
                    }

                    memset(&src_p->netAddr[MAXHOSTIPNUM-1], 0, sizeof(L_INET_AddrIp_T));
                    ret = TRUE;
                    break;
                }
            }
        }
    }
    else
    {
        /* old entry does not exist, return true for delete
         * bcz row_status is allowed to set destroy on unexist entry
         */
        ret = TRUE;
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_GetNextInOneDnsHostAddrEntry
 * PURPOSE: To get next ip address from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to get from
 *		    get_first -- true to get the first ip address
 *		    addr_p    -- ip address content
 * OUTPUT : addr_p    -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 */
static BOOL_T DNS_HOSTLIB_GetNextInOneDnsHostAddrEntry(
    UI32_T host_idx, BOOL_T get_first, L_INET_AddrIp_T *addr_p)
{
    HostEntry_PTR   cur_p;
    UI32_T          i;
    BOOL_T          ret = FALSE;

    cur_p = hostList_par[host_idx-1];
    if (NULL != cur_p)
    {
        if (TRUE == get_first)
        {
            if (cur_p->netAddr[0].addrlen != 0)
            {
                memcpy(addr_p, &cur_p->netAddr[0], sizeof(L_INET_AddrIp_T));
                ret = TRUE;
            }
        }
        else
        {
            for (i = 0; i < MAXHOSTIPNUM; i++)
            {
                if (L_INET_CompareInetAddr((L_INET_Addr_T *) addr_p, (L_INET_Addr_T *) & cur_p->netAddr[i],
                    0) == 0)
                {
                    if ((i+1 < MAXHOSTIPNUM) &&
                        (cur_p->netAddr[i+1].addrlen != 0))
                    {
                        memcpy(addr_p, &cur_p->netAddr[i+1], sizeof(L_INET_AddrIp_T));
                        ret = TRUE;
                    }

                    break;
                }
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_AddDnsHostEntryToArray
 * PURPOSE: To add a HostEntry to hostList_par.
 * INPUT  : src_p -- entry pointer to be added
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : none.
 */
static BOOL_T DNS_HOSTLIB_AddDnsHostEntryToArray(HostEntry_PTR src_p)
{
    UI32_T  ar_idx;
    BOOL_T  ret = FALSE;

    if (NULL != src_p)
    {
        for (ar_idx=0; ar_idx <MAX_NBR_OF_HOST_TABLE_SIZE; ar_idx++)
        {
            if (NULL == hostList_par[ar_idx])
            {
                hostList_par[ar_idx] = src_p;
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_DelDnsHostEntryFromArray
 * PURPOSE: To delete a HostEntry from hostList_par.
 * INPUT  : src_p -- entry pointer to be deleted
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : none.
 */
static BOOL_T DNS_HOSTLIB_DelDnsHostEntryFromArray(HostEntry_PTR src_p)
{
    UI32_T  idx;
    BOOL_T  ret = FALSE;

    if (NULL != src_p)
    {
        for (idx=0; idx <MAX_NBR_OF_HOST_TABLE_SIZE; idx++)
        {
            if (src_p == hostList_par[idx])
            {
                hostList_par[idx] = NULL;
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_IsDnsHostEntryValid
 * PURPOSE: To check if the specified HostEntry is valid
 * INPUT  : src_p -- entry pointer to be checked
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. the HostEntry is valid if it has at least one non-zero ip address
 *             (zero ip address means 0.0.0.0).
 *          2. only support ipv4 now.
 */
static BOOL_T DNS_HOSTLIB_IsDnsHostEntryValid(HostEntry_PTR src_p)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    if (NULL != src_p)
    {
        for (i = 0; i < MAXHOSTIPNUM; i++)
        {
            if (src_p->netAddr[i].addrlen != 0)
            {
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_HOSTLIB_LocalGetDefHostName
 * PURPOSE: To get an available default host name for the new entry.
 * INPUT  : none.
 * OUTPUT : host_name_p -- pointer to the host name content.
 * RETURN : none.
 * NOTES  : none.
 */
static void DNS_HOSTLIB_LocalGetDefHostName(char *host_name_p)
{
    UI32_T  used_idx =1;

    if (NULL != host_name_p)
    {
        /* Use default name: 'HostName1' ,'HostName2', 'HostName3', ...
         */
        do
        {
            sprintf(host_name_p, DNS_TYPE_DEFAULT_HOST_NAME_FORMAT, (long)used_idx++);
        } while (NULL != DNS_HOSTLIB_SearchName(host_name_p));
    }
}

/* FUNCTION NAME : DNS_HOSTLIB_IsAddrTypeValid
 * PURPOSE: To check if the specified address type is valid
 * INPUT  : addr_type -- address type to be checked
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. only support ipv4 now.
 */
static BOOL_T DNS_HOSTLIB_IsAddrTypeValid(UI32_T addr_type)
{
    BOOL_T  ret;

    switch(addr_type)
    {
    case L_INET_ADDR_TYPE_IPV4:
    case L_INET_ADDR_TYPE_IPV6:
    case L_INET_ADDR_TYPE_IPV6Z:
        ret = TRUE;
        break;

    default:
        ret = FALSE;
        break;
    }

    return ret;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

