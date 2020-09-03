#include "ctype.h"
#include "l_inet.h"
#include "sys_cpnt.h"
#include "cli_api.h"

#if (SYS_CPNT_DNS == TRUE)
#include "dns_mgr.h"
#include "dns_pmgr.h"
#include "dns_pom.h"
#include "dns_type.h"
#endif

#define CLI_DNS_TEST 0

UI32_T CLI_API_IP_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
    L_INET_AddrIp_T addr;
   UI32_T i = 0;

   for (i = 1; arg[i] != NULL; i++)
   {
        if(arg[i] != NULL)
        {
            memset(&addr, 0, sizeof(addr));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                               arg[i],
                                                               (L_INET_Addr_T *)&addr,
                                                               sizeof(addr)))
            {
                return CLI_ERR_INTERNAL;
            }
        }

      switch(cmd_idx)
      {
      case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_HOST:
#if(SYS_CPNT_IPV6==TRUE)
      case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_HOST:
#endif
         if (DNS_PMGR_HostAdd(arg[0], &addr) == DNS_ERROR)
         {
            CLI_LIB_PrintStr_1("Failed to add hostname with IP address %s\r\n", arg[i]);
         }
         break;

      case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_HOST:
#if(SYS_CPNT_IPV6==TRUE)
      case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_HOST:
#endif

         if (DNS_PMGR_HostDelete(arg[0], &addr) == DNS_ERROR)
         {
            CLI_LIB_PrintStr_1("Failed to delete hostname with IP address %s\r\n", arg[i]);
         }
         break;

      default:
         return CLI_ERR_INTERNAL;
      }
   }
#endif
   return CLI_NO_ERROR;
}


UI32_T CLI_API_IP_DomainList(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_DOMAINLIST:
      if (DNS_PMGR_AddDomainNameToList(arg[0]) == DNS_ERROR)
      {
         CLI_LIB_PrintStr((char *)"Failed to add domain-name list\r\n");
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_DOMAINLIST:
      if (DNS_PMGR_DeleteDomainNameFromList(arg[0]) == DNS_ERROR)
      {
         CLI_LIB_PrintStr((char *)"Failed to delete domain-name list\r\n");
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
   return CLI_NO_ERROR;
}



UI32_T CLI_API_IP_DomainLookup(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_DOMAINLOOKUP:
      if (DNS_PMGR_EnableDomainLookup() == FALSE)
      {
         CLI_LIB_PrintStr((char *)"Failed to enable domain-name lookup\r\n");
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_DOMAINLOOKUP:
      if (DNS_PMGR_DisableDomainLookup() == FALSE)
      {
         CLI_LIB_PrintStr((char *)"Failed to disable domain-name lookup\r\n");
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_DomainName(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_DOMAINNAME:
      if (DNS_PMGR_AddDomainName(arg[0]) == DNS_ERROR)
      {
         CLI_LIB_PrintStr((char *)"Failed to set domain-name\r\n");
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_DOMAINNAME:
      if (DNS_PMGR_DeleteDomainName() == DNS_ERROR)
      {
         CLI_LIB_PrintStr((char *)"Failed to delete domain-name\r\n");
      }
      break;

   default:
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_NameServer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
    L_INET_AddrIp_T addr;
   UI32_T i = 0;

   for (i = 0; i<6; i++)
   {
        if(arg[i] != NULL)
        {
            memset(&addr, 0, sizeof(addr));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                               arg[i],
                                                               (L_INET_Addr_T *)&addr,
                                                               sizeof(addr)))
            {
                return CLI_ERR_INTERNAL;
            }
        }

      switch(cmd_idx)
      {
      case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_NAMESERVER:
         if(arg[i] != NULL)
         {
                    if (DNS_PMGR_AddNameServer(&addr) == DNS_ERROR)
             {
                CLI_LIB_PrintStr_1("Failed to add name-server with IP address %s\r\n", arg[i]);
             }
         }
         else
         {
             i = 6;
             break;
         }
         break;

      case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_NAMESERVER:
         if (arg[0]==NULL)
         {
             if(DNS_PMGR_DeleteAllNameServer()==DNS_ERROR)
             {
                 CLI_LIB_PrintStr((char *)"Failed to delete all name-server. \r\n");
             }

                 i=6;
             break;
         }
         else
         {
             if(arg[i] != NULL)
             {
                        if (DNS_PMGR_DeleteNameServer(&addr) == DNS_ERROR)
                 {
                    CLI_LIB_PrintStr_1("Failed to delete name-server with IP address %s\r\n", arg[i]);
                 }
             }
             else
             {
             	 i =6;
                 break;
             }
         }

         break;

      default:
         break;
      }
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Clear_DNS_Cache(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   if (DNS_PMGR_ClearDnsCache() == DNS_ERROR)
   {
      CLI_LIB_PrintStr((char *)"Failed to clear DNS cache records\r\n");
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Clear_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   if (arg[0][0] == '*')
   {
        if (DNS_PMGR_ClearDnsCache() == DNS_ERROR)
        {
         CLI_LIB_PrintStr((char *)"Failed to clear DNS cache records\r\n");
        }
   }
   else
   {
        if (DNS_PMGR_DeleteDnsCacheRR((I8_T *)arg[0]) == DNS_ERROR)
      {
            CLI_LIB_PrintStr_1("Failed to clear cache %s records\r\n",arg[0]);
      }
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_SHOW_DNS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   I8_T    dns_ip_domain_name[DNS_MAX_NAME_LENGTH+1]={0};
   L_INET_AddrIp_T ip;
   char  ip_addr[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

   PROCESS_MORE((char *)"Domain Lookup Status:\r\n");
   SYSFUN_Sprintf(buff, "    DNS %s\r\n",(DNS_PMGR_GetDnsStatus() == DNS_ENABLE) ? ("Enabled"):("Disabled"));
   PROCESS_MORE(buff);
   DNS_PMGR_GetDnsIpDomain((char *)dns_ip_domain_name);
   PROCESS_MORE((char *)"Default Domain Name:\r\n");
   SYSFUN_Sprintf(buff, "    %s\r\n",dns_ip_domain_name);
   PROCESS_MORE(buff);
   PROCESS_MORE((char *)"Domain Name List:\r\n");
   memset(dns_ip_domain_name, 0, sizeof(dns_ip_domain_name));
   while(DNS_PMGR_GetNextDomainNameList(dns_ip_domain_name) == TRUE)
   {
      SYSFUN_Sprintf(buff, "    %s\r\n",dns_ip_domain_name);
      PROCESS_MORE(buff);
   }
   PROCESS_MORE((char *)"Name Server List:\r\n");
   /* EPR: ASF4526B-FLF-P5-00107
    * Problem: the command "show dns" can't show name servers configured
    * Root cause: the initial value of ip is incorrect
    */
   memset((void *)&ip, 0, sizeof (ip));
   while(DNS_POM_GetNextNameServerList(&ip) == TRUE)
   {
        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&ip,
                                                           ip_addr,
                                                           sizeof(ip_addr)))
        {
            continue;
        }

        SYSFUN_Sprintf(buff, "    %s\r\n",ip_addr);
        PROCESS_MORE(buff);
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_SHOW_DNS_CACHE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)
    UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  temp[CLI_DEF_MAX_BUFSIZE/2] = {0};
    UI32_T line_num = 0;
    I32_T   index = -1;
    DNS_CacheRecord_T   cache_entry;
    char   ip[L_INET_MAX_IPADDR_STR_LEN+1];

    PROCESS_MORE((char *)"No.     Flag    Type    IP Address      TTL     Host\r\n");
    PROCESS_MORE((char *)"------- ------- ------- --------------- ------- --------\r\n");
    while ( DNS_PMGR_GetNextCacheEntry(&index, &cache_entry) == TRUE )
    {
        SYSFUN_Sprintf((char *)buff, "%7lu %7d ", (unsigned long)index,cache_entry.flag);

        if (DNS_RRT_A == cache_entry.type || DNS_RRT_AAAA == cache_entry.type)
        {
            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&cache_entry.ip,
                                                               ip,
                                                               sizeof(ip)))
            {
                continue;
            }

            SYSFUN_Sprintf((char *)temp, "%-7s %-15s ", "Host", ip);
            strcat((char *)buff, (char *)temp);
        }
        else
        {
            UI32_T idx;
            memcpy(&idx, cache_entry.ip.addr, 4);
            SYSFUN_Sprintf((char *)temp, "%-7s POINTER TO:%-4ld\t", "CNAME", (long)idx);
            strcat((char *)buff, (char *)temp);
        }

        SYSFUN_Sprintf((char *)temp, "%7lu ",(unsigned long)cache_entry.ttl);
        strcat((char *)buff, (char *)temp);
        SYSFUN_Sprintf((char *)temp, "%s\r\n", cache_entry.name);
        strcat((char *)buff, (char *)temp);
        PROCESS_MORE((char*)buff);
    }
#endif
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_DNS == TRUE)
/* show cache or host entry information
 * format is:
 *
 * No.  Flag Type    IP Address           TTL  Host
 * ---- ---- ------- -------------------- ---- ----------------------------------
 */
static UI32_T show_hosts_info(UI32_T host_num, HostEntry_T *hostentry_p, UI32_T cache_idx, DNS_CacheRecord_T *cache_entry_p)
{
    enum
    {
        SHOW_IP_LEN = sizeof("--------------------")-1,
        SHOW_HOST_LEN = sizeof("----------------------------------")-1
    };

    char   ip[L_INET_MAX_IPADDR_STR_LEN+1]={0};
    UI8_T  host[L_INET_MAX_DNS_STR_LEN+1]={0};
    UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T host_len = 0;
    UI32_T ip_len = 0;

    if(hostentry_p != NULL)
    {
            UI32_T i;
            host_len = strlen((char*)hostentry_p->hostName);
            strcpy((char*)host, (char*)hostentry_p->hostName);

            for(i = 0; i < MAXHOSTIPNUM; i++)
            {
                if(hostentry_p->netAddr[i].addrlen != 0)
                {
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&hostentry_p->netAddr[i],
                                                                       ip,
                                                                       sizeof(ip)))
                    {
                        continue;
                    }

                    ip_len = strlen((char*)ip);

                    SYSFUN_Sprintf((char*)buff, "%4lu    2 Address %-*.*s        %.*s\r\n",
                                          (unsigned long)host_num++,
                                          SHOW_IP_LEN ,SHOW_IP_LEN, ip,
                                          SHOW_HOST_LEN, host);
                    PROCESS_MORE((char*)buff);
                }
            }
    }

    if(cache_entry_p!=NULL)
    {
        UI8_T  temp[CLI_DEF_MAX_BUFSIZE/2] = {0};
        host_len = strlen(cache_entry_p->name);

        strcpy((char*)host, (char*)cache_entry_p->name);

        SYSFUN_Sprintf((char*)buff, "%4lu %4d ", (unsigned long)cache_idx+host_num,cache_entry_p->flag);

        if(DNS_RRT_A == cache_entry_p->type || DNS_RRT_AAAA == cache_entry_p->type)
        {
            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&cache_entry_p->ip,
                                                               ip,
                                                               sizeof(ip)))
            {
                return CLI_ERR_INTERNAL;
            }

            ip_len=strlen((char*)ip);
            SYSFUN_Sprintf((char*)temp, "%-7s %-*.*s  ", "Address", SHOW_IP_LEN, SHOW_IP_LEN, ip);
            strcat((char*)buff, (char*)temp);
        }
        else
        {
            UI32_T idx = 0;
            memcpy(&idx, cache_entry_p->ip.addr, 4);
            SYSFUN_Sprintf((char*)temp, "%-7s POINTER TO:%-4ld\t", "CNAME", (long)idx+host_num);
            strcat((char*)buff, (char*)temp);
        }

        SYSFUN_Sprintf((char*)temp, "%5lu ",(unsigned long)cache_entry_p->ttl);
        strcat((char*)buff, (char*)temp);
        SYSFUN_Sprintf((char*)temp, "%.*s\r\n", SHOW_HOST_LEN, host);
        strcat((char*)buff, (char*)temp);
        PROCESS_MORE((char*)buff);


    }

    /* IP or domain name too long to show in show hosts format.
     */
    if(ip_len > SHOW_IP_LEN && host_len <= SHOW_HOST_LEN)
    {
        SYSFUN_Sprintf((char*)buff, "                  %.*s\r\n",
                             SHOW_IP_LEN, ip+SHOW_IP_LEN);
        PROCESS_MORE((char*)buff);
    }
    else if( ip_len <= SHOW_IP_LEN && host_len > SHOW_HOST_LEN)
    {
        SYSFUN_Sprintf((char*)buff, "                                             %.*s\r\n",
                             SHOW_HOST_LEN, host+SHOW_HOST_LEN);
        PROCESS_MORE((char*)buff);
    }
    else if(ip_len > SHOW_IP_LEN && host_len > SHOW_HOST_LEN)
    {
        SYSFUN_Sprintf((char*)buff, "                  %-*.*s       %.*s\r\n",
                             SHOW_IP_LEN, SHOW_IP_LEN, ip+SHOW_IP_LEN,
                             SHOW_HOST_LEN, host+SHOW_HOST_LEN);
        PROCESS_MORE((char*)buff);
    }

    return host_num;

}
#endif /* #if (SYS_CPNT_DNS == TRUE) */

UI32_T CLI_API_SHOW_HOSTS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DNS == TRUE)

   UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   UI32_T host_num = 0;

   PROCESS_MORE("No.  Flag Type    IP Address            TTL   Host\r\n");
   PROCESS_MORE("---- ---- ------- --------------------  ----- ----------------------------------\r\n");

    /*show host table
     */
    {
        HostEntry_T hostentry;
        int host_index = -1;

        memset(&hostentry, 0, sizeof(hostentry));

        while( DNS_OK == DNS_PMGR_GetNextDnsHostEntry(&host_index, &hostentry) )
        {
            host_num = show_hosts_info(host_num, &hostentry, 0, NULL);
        }
    }

    /*show cache
     */
      {
        I32_T   index = -1;
        DNS_CacheRecord_T   cache_entry;

        while ( DNS_PMGR_GetNextCacheEntry(&index, &cache_entry) == TRUE )
         {
            show_hosts_info(host_num, NULL, index, &cache_entry);
}
      }

#endif
   return CLI_NO_ERROR;
}

