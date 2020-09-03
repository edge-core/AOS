/*system*/
#include "sys_cpnt.h"
#if (SYS_CPNT_ROUTING == TRUE)

#include "sysfun.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>

//#include "skt_vx.h"
//#include "socket.h"

#include "sys_dflt.h"

#include "sys_pmgr.h"    /* 2007-12, Joseph */

/*cli internal*/
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
#include "cli_type.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"
#include "cli_msg.h"
#include "cli_cmd.h"
#include "cli_auth.h"
#include "cli_runcfg.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"

#if (SYS_CPNT_OSPF6 == TRUE)
#include "ospf6_pmgr.h"
#include "ospf6_type.h"
#include "ospf6_mgr.h"
#include "cli_api_ospf6.h"

#endif

#include "vlan_lib.h"

#define OSPF_ABR_TYPE_UNKNOWN			0
#define OSPF_ABR_TYPE_STANDARD			1
#define OSPF_ABR_TYPE_CISCO			    2
#define OSPF_ABR_TYPE_IBM			    3
#define OSPF_ABR_TYPE_SHORTCUT			4

/* Route types. */
#define OSPF_ROUTE_DEFAULT                0
#define OSPF_ROUTE_KERNEL                 1
#define OSPF_ROUTE_CONNECT                2
#define OSPF_ROUTE_STATIC                 3
#define OSPF_ROUTE_RIP                    4
#define OSPF_ROUTE_RIPNG                  5
#define OSPF_ROUTE_OSPF                   6
#define OSPF_ROUTE_OSPF6                  7
#define OSPF_ROUTE_BGP                    8
#define OSPF_ROUTE_ISIS                   9
#define OSPF_ROUTE_MAX                   10

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))
#define SN_LAN                          1

#ifndef	MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef	MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#endif

#define AS_EXTERNAL_LSA_BIT_T		(1 << 0)
#define AS_EXTERNAL_LSA_BIT_F		(1 << 1)
#define AS_EXTERNAL_LSA_BIT_E		(1 << 2)

#define OSPF_AREA_DEFAULT			0
#define OSPF_AREA_STUB				1
#define OSPF_AREA_NSSA				2
#define OSPF_AREA_TYPE_MAX			3

char *show_ipv6_database_desc[] =
{
  "unknown",
  "Router-LSA",
  "Network-LSA",
  "Inter-Area-Prefix-LSA",
  "Inter-Area-Router-LSA",
  "AS-external-LSA",
  "Group Membership-LSA",
  "NSSA-external-LSA",
  "Link-LSA",
  "Intra-Area-Prefix-LSA",
#ifdef HAVE_OSPF6_TE
  "Intra-Area-Te-LSA",
#endif /* HAVE_OSPF6_TE */
};

static char *ospf6_abr_type_descr_str[] =
{
  "Unknown",
  "Standard (RFC2328)",
  "Alternative Cisco (RFC3509)",
  "Alternative IBM (RFC3509)",
  "Alternative Shortcut"
};


#define SHOW_IPV6_OSPF_HEADER_COMMON                                          \
  "Link State ID   ADV Router      Age  Seq#       CkSum"

char *show_ipv6_database_header[] =
{
  "",
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
  SHOW_IPV6_OSPF_HEADER_COMMON,
#ifdef HAVE_OSPF6_TE
  SHOW_IPV6_OSPF_HEADER_COMMON,
#endif /* HAVE_OSPF6_TE */
};

#if (SYS_CPNT_OSPF6 == TRUE)

static UI32_T CLI_API_L3_Ospf6_AbrTypeFromString(char *type)
{
	/* cisco type */
	if ( type[0] == 'c' || type[0] == 'C' )
		return OSPF_ABR_TYPE_CISCO;
	/* ibm type */
	else if ( type[0] == 'I' || type[0] == 'i' )
		return OSPF_ABR_TYPE_IBM;
	/* standard type */
	else if ( type[1] == 't' || type[0] == 'T' )
		return OSPF_ABR_TYPE_STANDARD;
	/* shortcut type */
	else if ( type[1] == 'h' || type[0] == 'H' )
		return OSPF_ABR_TYPE_SHORTCUT;

	return OSPF_ABR_TYPE_UNKNOWN;
}

/* 0    -- success
 * -1   -- fail
 */
static int CLI_API_L3_Ospf6_Str2Addr(char *str, struct pal_in4_addr *addr)
{
    UI8_T addr_tmp[SYS_ADPT_IPV4_ADDR_LEN]= {0};

    if(0 == CLI_LIB_AtoIp(str, addr_tmp))
        return -1;
    memcpy(&(addr->s_addr), addr_tmp, sizeof(UI32_T));
    return 0;
}
#if 0
static BOOL_T CLI_API_L3_GetIfindexFromIfname(char *ifname, UI32_T *ifindex)
{
    char    *ptr;
    UI32_T  vlan_id = 0;

    if(strncasecmp("vlan",ifname,4)!= 0)
      return FALSE;
    for(ptr = ifname + 4; *ptr != '\0'; ++ptr)
      if ((*ptr -'9' > 0) || (*ptr -'0'< 0))
        return FALSE;

    sscanf(ifname+4, "%ld", &vlan_id);
    if(VLAN_OM_ConvertToIfindex(vlan_id, ifindex) == TRUE)
        return TRUE;
    else
        return FALSE;
}
#endif
static BOOL_T CLI_API_L3_GetIfnameFromIfindex(UI32_T ifindex,char *ifname)
{
    UI32_T vid;
    if(VLAN_OM_ConvertFromIfindex(ifindex, &vid) != TRUE)
    {
        return FALSE;
    }
    sprintf(ifname, "VLAN %lu",(unsigned long)vid);
    return TRUE ;
}

static void CLI_API_L3_Str2Addr(char *str, struct pal_in4_addr *addr)
{
    UI8_T addr_tmp[SYS_ADPT_IPV4_ADDR_LEN]= {0};

    CLI_LIB_AtoIp(str, addr_tmp);
    memcpy(&(addr->s_addr), addr_tmp, sizeof(UI32_T));
}

#endif

UI32_T CLI_API_L3_Router_Ospf6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T ret;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    char   tag[10] = {};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_ROUTER_IPV6_OSPF:
            if(arg[0] == NULL)
                strcpy(tag, "1");
            else
                strcpy(tag, arg[0]);

            ret = OSPF6_PMGR_RouterOspfSet(vr_id, tag);
            if(ret != OSPF6_TYPE_RESULT_SUCCESS)
            {
                CLI_LIB_PrintStr("The Router Ospf6 setting failed.\r\n");
            }
            else
            {
                strncpy(ctrl_P->CMenu.tag, tag, strlen(tag));
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_ROUTEROSPF6_MODE;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_ROUTER_IPV6_OSPF:
            if(arg[0] == NULL)
                strcpy(tag, "1");
            else
                strcpy(tag, arg[0]);

            ret = OSPF6_PMGR_RouterOspfUnset(vr_id, tag);
            if( ret != OSPF6_TYPE_RESULT_SUCCESS)
            {
                CLI_LIB_PrintStr("The Router Ospf6 unsetting failed.\r\n");
            }
            break;

        default:
            return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;
}

#if 0
UI32_T CLI_API_L3_Ip_Ospf6_RouterInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
	int i;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T ifindex = ctrl_P->CMenu.vlan_ifindex;
    UI32_T instance_id = 0;
    UI32_T format = OSPF_AREA_ID_FORMAT_DEFAULT;
    struct pal_in4_addr area_id;
    UI32_T ret;
	char   *tag = NULL;

    /* Get argument  */
    for (i = 0; arg[i];)
    {
        switch (arg[i][0])
        {
            /* Get area id */
            case 'a':
            case 'A':
                if(CLI_API_L3_Ospf6_Str2AreaId(arg[i+1], &area_id, &format))
                {
                    CLI_LIB_PrintStr ("Invalid OSPF area ID\n");
                    return CLI_NO_ERROR;
                }
                i += 2;
                break;
            /* Get tag information */
            case 't':
            case 'T':
                tag = arg[i+1];
                i += 2;
                break;
            /* Get instance id */
            case 'i':
            case 'I':
                instance_id = atoi(arg[i+1]);
                i += 2;
                break;
            default:
                break;

        }
    }
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ROUTER_OSPF:
            ret = OSPF6_PMGR_IfRouterSet(vr_id, tag, ifindex, area_id.s_addr, format, instance_id);
            if ( ret != OSPF6_TYPE_RESULT_SUCCESS )
                CLI_LIB_PrintStr ("Setting router failed.\r\n");
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ROUTER_OSPF:
            ret = OSPF6_PMGR_IfRouterUnset(vr_id, tag, ifindex, area_id.s_addr, instance_id, TRUE);
            if ( ret != OSPF6_TYPE_RESULT_SUCCESS )
                CLI_LIB_PrintStr ("Unsetting router failed.\r\n");
            break;
        default:
            return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}
#endif

/* cmd
ipv6 ospf [process-id] area area-id [instance instance-id]
no ipv6 ospf [process-id] area area-id [instance instance-id]

and

ipv6 ospf {cost | dead-interval | hello-interval | priority | retransmit-interval | transmit-delay } config-value [instance instance-id]
no ipv6 ospf {cost | dead-interval | hello-interval | priority | retransmit-interval | transmit-delay } [instance instance-id]

*/

UI32_T CLI_API_L3_Ip_Ospf6_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_AbrType(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T type;
	char   *tag = NULL;

    tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;
    switch(cmd_idx)
    {
    	case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_ABRTYPE:
			type = CLI_API_L3_Ospf6_AbrTypeFromString(arg[0]);
    		if(OSPF6_PMGR_ABRTypeSet(vr_id, tag, type) != OSPF6_TYPE_RESULT_SUCCESS)
    		{
    			CLI_LIB_PrintStr("Setting ABR type failed.\r\n");
    		}
    	    break;

    	case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_ABRTYPE:
    		if( OSPF6_PMGR_ABRTypeUnset(vr_id, tag) != OSPF6_TYPE_RESULT_SUCCESS )
    		{
    			CLI_LIB_PrintStr("Unsetting ABR type failed.\r\n");
    		}
    	    break;

    	default:
    		return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_Area(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf6_DefaultMetric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T metric = 0;
	char   *tag = NULL;

    tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;

    if(arg[0])
    {
        metric = atoi(arg[0]);
    }

    switch(cmd_idx)
    {
    	case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_DEFAULTMETRIC:
    		if(OSPF6_PMGR_DefaultMetricSet(vr_id, tag, metric) != OSPF6_TYPE_RESULT_SUCCESS)
    		{
    			CLI_LIB_PrintStr("Setting default metric failed.\r\n");
    		}
    	    break;

    	case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_DEFAULTMETRIC:
    		if( OSPF6_PMGR_DefaultMetricUnset(vr_id, tag) != OSPF6_TYPE_RESULT_SUCCESS )
    		{
    			CLI_LIB_PrintStr("Unsetting default metric failed.\r\n");
    		}
    	    break;

    	default:
    		return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_MaxCurrentDD(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
	UI32_T vr_id = SYS_DFLT_VR_ID;
	UI32_T max_current_dd = 0;
	char   *tag = NULL;

	tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;

	if(arg[0])
	{
		max_current_dd = atoi(arg[0]);
	}

	switch(cmd_idx)
	{
		case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_MAXIMUMCONCURRENTDD:
			if(OSPF6_PMGR_ConcurrentDDSet(vr_id, tag, max_current_dd) != OSPF6_TYPE_RESULT_SUCCESS)
			{
				CLI_LIB_PrintStr("Setting max concurrent dd failed.\r\n");
			}
			break;

		case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_MAXIMUMCONCURRENTDD:
			if( OSPF6_PMGR_ConcurrentDDUnset(vr_id, tag) != OSPF6_TYPE_RESULT_SUCCESS )
			{
				CLI_LIB_PrintStr("Unsetting max concurrent dd failed.\r\n");
			}
			break;

		default:
			return CLI_ERR_CMD_INVALID;
	}
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_PassiveInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
	UI32_T vr_id = SYS_DFLT_VR_ID;
	char   *tag = NULL;
	UI32_T ifindex;
	UI32_T ret;
	UI32_T vid=0;
	tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;

    vid = CLI_LIB_AtoUl(arg[1],10);
    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);


	switch(cmd_idx)
	{
		 case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_PASSIVEINTERFACE:
			 ret = OSPF6_PMGR_PassiveIfSet(vr_id, tag, ifindex);
			 if(ret != OSPF6_TYPE_RESULT_SUCCESS)
			 {
				 if(ret == OSPF6_TYPE_RESULT_IF_NOT_EXIST)
				 {
					 CLI_LIB_PrintStr("Please specify an existing interface.\r\n");
				 }
				 else
				 {
					 CLI_LIB_PrintStr("The OSPF passive interface setting failed.\r\n");
				 }
			 }
			 break;

		 case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_PASSIVEINTERFACE:
			 ret = OSPF6_PMGR_PassiveIfUnset(vr_id, tag, ifindex);
			 if(ret != OSPF6_TYPE_RESULT_SUCCESS)
			 {
				 if(ret == OSPF6_TYPE_RESULT_IF_NOT_EXIST)
				 {
					 CLI_LIB_PrintStr("Please specify an existing interface.\r\n");
				 }
				 else
				 {
					 CLI_LIB_PrintStr("The OSPF passive interface setting failed.\r\n");
				 }
			 }
			 break;

		 default:
			 break;
	}
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    char   *tag = NULL;
    UI32_T proto = 0;
    int mtype = 0;
    int metric = 0;
    BOOL_T is_mtype = FALSE, is_metric = FALSE; /*metric-type or metric are configured or not */    
    int i = 0;
    UI32_T ret;

    tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;

    for(i=0; arg[i]; )
    {
        if(!strncmp(arg[i], "connected", 3))
        {
            proto = OSPF_ROUTE_CONNECT;
            i+=1;
        }
        else if(!strncmp(arg[i], "static", 3))
        {
            proto = OSPF_ROUTE_STATIC;
            i+=1;
        }
        else if(!strncmp(arg[i], "ripng", 3))
        {
            proto = OSPF_ROUTE_RIPNG;
            i+=1;
        }
        else if(!strncmp(arg[i], "metric-type", 7))
        {
            is_mtype = TRUE;
            if(cmd_idx == PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_REDISTRIBUTE)
            {
                mtype = atoi(arg[i+1]);
                i+=2;
            }
            else
            {
                i+=1;
            }
        }
        else if(!strncmp(arg[i], "metric", 6))
        {
            is_metric = TRUE;
            if(cmd_idx == PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_REDISTRIBUTE)
            {
                metric = atoi(arg[i+1]);
                i+=2;
            }
            else
            {
                i+=1;
            }
        }
    } /* for */

    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_REDISTRIBUTE:
            ret = OSPF6_PMGR_RedistributeProtoSet(vr_id, tag, proto);
            if(ret != OSPF6_TYPE_RESULT_SUCCESS)
            {
                CLI_LIB_PrintStr("Redistributing failed.\r\n");
                return CLI_NO_ERROR;
            }
            if(is_mtype)
            {
                ret = OSPF6_PMGR_RedistributeMetricTypeSet(vr_id, tag, proto, mtype);
                if (ret != OSPF6_TYPE_RESULT_SUCCESS)
                {
                    CLI_LIB_PrintStr("Failed to set metric type.\r\n");
                    return CLI_NO_ERROR;
                }
            }
            if(is_metric)
            {
                ret = OSPF6_PMGR_RedistributeMetricSet(vr_id, tag, proto, metric);
                if (ret != OSPF6_TYPE_RESULT_SUCCESS)
                {
                    CLI_LIB_PrintStr("Failed to set metric.\r\n");
                    return CLI_NO_ERROR;
                }
            }
            break;

        case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_REDISTRIBUTE:
            if((proto != 0) && !is_mtype && !is_metric)
            {
                ret = OSPF6_PMGR_RedistributeProtoUnset(vr_id, tag, proto);
                if(ret != OSPF6_TYPE_RESULT_SUCCESS)
                {
                    CLI_LIB_PrintStr("Unredistributing failed.\r\n");
                }
                return CLI_NO_ERROR;
            }
            if(is_mtype)
            {
                ret = OSPF6_PMGR_RedistributeMetricTypeUnset(vr_id, tag, proto);
                if(ret != OSPF6_TYPE_RESULT_SUCCESS)
                {
                    CLI_LIB_PrintStr("Failed to unset metric type.\r\n");
                    return CLI_NO_ERROR;
                }
            }
            if(is_metric)
            {
                ret = OSPF6_PMGR_RedistributeMetricUnset(vr_id, tag, proto);
                if (ret != OSPF6_TYPE_RESULT_SUCCESS)
                {
                    CLI_LIB_PrintStr("Failed to unset metric.\r\n");
                    return CLI_NO_ERROR;
                }
            }
            break;

         default:
            break;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_RouterId(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    char   *tag = NULL;
    struct pal_in4_addr router_id;

    tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;

    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W1_ROUTERID:
             if(0 != CLI_API_L3_Ospf6_Str2Addr(arg[0], &router_id))
             {
                 CLI_LIB_Printf("Invalid router ID.\r\n");
                 break;
             }
             if(OSPF6_PMGR_RouterIdSet(vr_id, tag, router_id.s_addr) != OSPF6_TYPE_RESULT_SUCCESS)
             {
                 CLI_LIB_PrintStr("The OSPF Router Id setting failed.\r\n");
             }
             break;

         case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_NO_ROUTERID:
             if(OSPF6_PMGR_RouterIdUnset(vr_id, tag)!= OSPF6_TYPE_RESULT_SUCCESS)
             {
                 CLI_LIB_PrintStr("The OSPF Router Id unsetting failed.\r\n");
             }
             break;

         default:
             break;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Ospf6_Timer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    char   *tag = NULL;
    UI32_T delay_timer = 0, hold_timer = 0;

    tag = ctrl_P->CMenu.tag[0] == 0 ? NULL : ctrl_P->CMenu.tag;
    if(arg[0] && arg[1])
    {
        delay_timer = atoi(arg[0]);
        hold_timer = atoi(arg[1]);
    }
    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W2_TIMERS_SPF:
             if(OSPF6_PMGR_DelayTimerSet(vr_id, tag, delay_timer)!= OSPF6_TYPE_RESULT_SUCCESS)
             {
                 CLI_LIB_PrintStr("The OSPF delay timer setting is failed.\r\n");
             }
             if(OSPF6_PMGR_HoldTimerSet(vr_id, tag, hold_timer)!= OSPF6_TYPE_RESULT_SUCCESS)
             {
                 CLI_LIB_PrintStr("The OSPF hold timer setting is failed.\r\n");
             }
             break;

         case PRIVILEGE_CFG_ROUTEROSPF6_CMD_W3_NO_TIMERS_SPF:
             if( OSPF6_PMGR_TimerUnset(vr_id, tag)!= OSPF6_TYPE_RESULT_SUCCESS)
             {
                 CLI_LIB_PrintStr("The OSPF timer unsetting failed.\r\n");
             }
             break;

         default:
             break;
    }
#endif
    return CLI_NO_ERROR;
}
#if (SYS_CPNT_OSPF6 == TRUE)
/* command: show ip ospf neighbor */
static UI32_T show_ospf6_neighbor(char *arg)
{
#define IFSM_DROther			5
#define IFSM_Backup			    6
#define IFSM_DR				    7
   UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   char   tmpBuf[100] = {0};
   UI32_T line_num = 0;
   char   ifname[10] = {};
   UI8_T  ID_address[30] = {0};
   char   state[30] = {0};
   OSPF6_MGR_NBR_ENTRY_T  ospf6_neighbor_entry;
   OSPF6_MGR_VNBR_ENTRY_T ospf6_virt_neighbor_entry;
   OSPF6_MGR_OSPF_ENTRY_T entry;
   UI32_T ret;

   memset(&ospf6_neighbor_entry, 0, sizeof(OSPF6_MGR_NBR_ENTRY_T));
   memset(&ospf6_virt_neighbor_entry, 0, sizeof(OSPF6_MGR_VNBR_ENTRY_T));
   CLI_LIB_PrintStr("\r\n       ID         Pri        State       Interface ID     Interface\r\n");
   CLI_LIB_PrintStr("--------------- ------ ---------------- --------------- --------------\r\n");

   /* Neighbor */
   memset(&entry, 0, sizeof(OSPF6_MGR_OSPF_ENTRY_T));
   entry.is_first = TRUE;
   while( TRUE )
   {
       if ( arg )
       {
          memset(&entry, 0, sizeof(OSPF6_MGR_OSPF_ENTRY_T));
          strncpy(entry.tag, arg, strlen(arg));
          if ( OSPF6_PMGR_Process_Get(&entry) != OSPF6_TYPE_RESULT_SUCCESS )
          {
              break;
          }
       }
       else
       {
           if ( OSPF6_PMGR_Process_GetNext(&entry) != OSPF6_TYPE_RESULT_SUCCESS )
               break;
       }
      /* Copy the tag information */
      strncpy(ospf6_neighbor_entry.tag, entry.tag, strlen(entry.tag));
      while( TRUE )
      {
          if ( OSPF6_PMGR_Neighbor_GetNext(&ospf6_neighbor_entry) != OSPF6_TYPE_RESULT_SUCCESS )
            break;
          L_INET_Ntoa(ospf6_neighbor_entry.nbr_rtr_id, ID_address);
          switch(ospf6_neighbor_entry.nbr_state)
          {
             case VAL_ospfNbrState_down:
             strcpy(state,"DOWN");
             break;

             case VAL_ospfNbrState_attempt:
             strcpy(state,"ATTEMPT");
             break;

             case VAL_ospfNbrState_init:
             strcpy(state,"INIT");
             break;

             case VAL_ospfNbrState_twoWay:
             strcpy(state,"2WAY");
             break;

             case VAL_ospfNbrState_exchangeStart:
             strcpy(state,"EX START");
             break;

             case VAL_ospfNbrState_exchange:
             strcpy(state,"EXCHANGE");
             break;

             case VAL_ospfNbrState_loading:
             strcpy(state,"LOADING");
             break;

             case VAL_ospfNbrState_full:
             strcpy(state,"FULL");
             break;

             default:
             break;
          }
          switch(ospf6_neighbor_entry.router_role)
          {
             case IFSM_DR:
             strcat(state,"/DR");
             break;

             case IFSM_Backup:
             strcat(state,"/BDR");
             break;

             case IFSM_DROther:
             strcat(state,"/DROTHER");
             break;

             default:
             break;
          }
          CLI_API_L3_GetIfnameFromIfindex(ospf6_neighbor_entry.ifindex, ifname);
          sprintf(tmpBuf, "%15s %6d %16s %15lu %14s\r\n",
                          ID_address,
                          ospf6_neighbor_entry.nbr_priority,
                          state,
                          (unsigned long)ospf6_neighbor_entry.nbr_ifid,
                          ifname);
          PROCESS_MORE(tmpBuf);
      }
      if ( arg )
        break;
   }
   /* virtual neighbor */
   memset(&entry, 0, sizeof(OSPF6_MGR_OSPF_ENTRY_T));
   entry.is_first = TRUE;
   while( TRUE )
   {
       if ( arg )
       {
          memset(&entry, 0, sizeof(OSPF6_MGR_OSPF_ENTRY_T));
          strncpy(entry.tag, arg, strlen(arg));
          if ( OSPF6_PMGR_Process_Get(&entry) != OSPF6_TYPE_RESULT_SUCCESS )
              break;
       }
       else
       {
           if ( OSPF6_PMGR_Process_GetNext(&entry) != OSPF6_TYPE_RESULT_SUCCESS )
               break;
       }
      /* Copy the tag information */
      strncpy(ospf6_virt_neighbor_entry.tag, entry.tag, strlen(entry.tag));
      while( TRUE )
      {
          ret = OSPF6_PMGR_VirtNeighbor_GetNext(&ospf6_virt_neighbor_entry);
          if (  ret != OSPF6_TYPE_RESULT_SUCCESS )
              break;
          memset(ID_address, 0, sizeof(ID_address));
          L_INET_Ntoa(ospf6_virt_neighbor_entry.nbr_rtr_id.s_addr, ID_address);
          switch(ospf6_virt_neighbor_entry.nbr_state)
          {
             case VAL_ospfNbrState_down:
             strcpy(state,"DOWN");
             break;

             case VAL_ospfNbrState_attempt:
             strcpy(state,"ATTEMPT");
             break;

             case VAL_ospfNbrState_init:
             strcpy(state,"INIT");
             break;

             case VAL_ospfNbrState_twoWay:
             strcpy(state,"2WAY");
             break;

             case VAL_ospfNbrState_exchangeStart:
             strcpy(state,"EX START");
             break;

             case VAL_ospfNbrState_exchange:
             strcpy(state,"EXCHANGE");
             break;

             case VAL_ospfNbrState_loading:
             strcpy(state,"LOADING");
             break;

             case VAL_ospfNbrState_full:
             strcpy(state,"FULL");
             break;

             default:
             break;
          }
          /* Virtual neighbor has no DR/BDR */
          strcat(state,"/-");
          sprintf(tmpBuf, "%15s %6lu %16s %15lu %8s\r\n",ID_address, (unsigned long)ospf6_virt_neighbor_entry.nbr_priority,state,
                           (unsigned long)ospf6_virt_neighbor_entry.nbr_ifid, ospf6_virt_neighbor_entry.name);
          PROCESS_MORE(tmpBuf);
      }
      if ( arg )
        break;
   }
        return CLI_NO_ERROR;
}

char *
ospf6_options_string (u_int32_t options, char *buf)
{
  snprintf (buf, OSPF6_OPTIONS_STR_MAXLEN, "%s|%s|%s|%s|%s|%s",
	     CHECK_FLAG (options, OSPFV3_OPTION_DC) ? "DC" : "-",
	     CHECK_FLAG (options, OSPFV3_OPTION_R)  ? "R"  : "-",
	     CHECK_FLAG (options, OSPFV3_OPTION_N)  ? "N"  : "-",
	     CHECK_FLAG (options, OSPFV3_OPTION_MC) ? "MC" : "-",
	     CHECK_FLAG (options, OSPFV3_OPTION_E)  ? "E"  : "-",
	     CHECK_FLAG (options, OSPFV3_OPTION_V6) ? "V6" : "-");

  return buf;
}

char *
ospf6_router_lsa_bits_string (u_char bits, char *buf)
{
  snprintf (buf, OSPF6_ROUTER_LSA_BITS_STR_MAXLEN, "%s|%s|%s|%s",
	     CHECK_FLAG (bits, ROUTER_LSA_BIT_W) ? "W" : "-",
	     CHECK_FLAG (bits, ROUTER_LSA_BIT_V) ? "V" : "-",
	     CHECK_FLAG (bits, ROUTER_LSA_BIT_E) ? "E" : "-",
	     CHECK_FLAG (bits, ROUTER_LSA_BIT_B) ? "B" : "-");
  return buf;
}

// copy from ospf6_debug.c 
char *
ospf6_lsa_prefix_options_string (OSPF6_TYPE_LSA_Prefix_T *p, char *buf)
{
  if (!p)
    return "-";

  snprintf (buf, OSPF6_PREFIX_OPTIONS_STRING_MAXLEN, "%s|%s|%s|%s",
	     CHECK_FLAG (p->options, OSPFV3_PREFIX_OPTION_P) ? "P" : "-",
	     CHECK_FLAG (p->options, OSPFV3_PREFIX_OPTION_MC) ? "MC" : "-",
	     CHECK_FLAG (p->options, OSPFV3_PREFIX_OPTION_LA) ? "LA" : "-",
	     CHECK_FLAG (p->options, OSPFV3_PREFIX_OPTION_NU) ? "NU" : "-");

  return buf;
}

char *ospf6_link_type_desc[] =
{
  "(null)",
  "another Router (point-to-point)",
  "a Transit Network",
  "(Reserved)",
  "a Virtual Link",
};

/* ref. ospf6_cli_show_lsa_header */
static int
cli_api_ospf6_show_lsa_header (OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p, UI32_T *line_num_p)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num;

    line_num = *line_num_p;

    OSPF6_MGR_LSA_HEADER_T *header;
    UI8_T arr_id[4] = {};

    if(!lsa_entry_p)
        return -1;
    header = (OSPF6_MGR_LSA_HEADER_T *) lsa_entry_p->advertisement; 
    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "LS age: %d\r\n", header->ls_age);
    PROCESS_MORE(buff);

    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "LSA type: 0x%04X\r\n", L_STDLIB_Ntoh16(header->type));
    PROCESS_MORE(buff);

    memcpy(arr_id, &header->id.s_addr, 4);
    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "Link State ID: %d.%d.%d.%d\r\n", arr_id[0], arr_id[1], arr_id[2],arr_id[3]);
    PROCESS_MORE(buff);

    memcpy(arr_id, &header->adv_router.s_addr, 4);
    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "Advertising Router: %d.%d.%d.%d\r\n", arr_id[0], arr_id[1], arr_id[2],arr_id[3]);
    PROCESS_MORE(buff);

    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "LS Seq Number: 0x%08lX\r\n", (long)L_STDLIB_Ntoh32(header->ls_seqnum));
    PROCESS_MORE(buff);
    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "Checksum: 0x%04X\r\n", L_STDLIB_Ntoh16(header->checksum));
    PROCESS_MORE(buff);
    snprintf(buff, CLI_DEF_MAX_BUFSIZE, "Length: %d\r\n", L_STDLIB_Ntoh16(header->length));
    PROCESS_MORE(buff);

    *line_num_p = line_num;
    return 0;
}

/* copy from ospf6_lsa.c */
u_int16_t ospfv3_lsa_code2type[] =
{
  0x0000,       /* Dummy. */
  0x2001,       /* Router-LSA. */
  0x2002,       /* Netwrok-LSA. */
  0x2003,       /* Inter-Area-Prefix-LSA. */
  0x2004,       /* Inter-Area-Router-LSA. */
  0x4005,       /* AS-external-LSA. */
  0x0000,       /* Group-Membership-LSA. */
  0x0000,       /* NSSA-LSA. */
  0x0008,       /* Link-LSA. */
  0x2009,       /* Intra-Prefix-LSA. */
#ifdef HAVE_OSPF6_TE
  0xa00a,	/* Intra-Area-TE-LSA. */
#endif /* HAVE_OSPF6_TE */
};
#endif

UI32_T CLI_API_Ospf6_Clear_Ospf6_Process(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF6 == TRUE)
    int ret;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    char *tag = NULL;

    if(arg[0] && strncmp(arg[0], "process", 7))
    {
       tag = arg[0];
    }
    ret = OSPF6_PMGR_ClearOspf6Process(vr_id, tag);
    if ( ret != OSPF6_TYPE_RESULT_SUCCESS )
    {
        CLI_LIB_PrintStr("Failed to clear ospfv3 process.\r\n");
    }
#endif
	
   return CLI_NO_ERROR;
}


UI32_T CLI_API_L3_Show_Ip_Ospf6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}
#endif

