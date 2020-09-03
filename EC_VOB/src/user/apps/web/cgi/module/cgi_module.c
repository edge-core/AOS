#include "sys_bld.h"
#include "sys_cpnt.h"

#include "cgi.h"
#include "cgi_rest.h"
#include "cgi_request.h"
#include "cgi_response.h"
#include "jansson.h"

#include "l_stdlib.h"

#include "cgi_module.h"

typedef struct
{
    void (*init_fn)();
} CGI_MODULE_T;

/* ----------------------------------------------------------------------
 * Module List
 * ---------------------------------------------------------------------- */
#if (SYS_CPNT_AMTR == TRUE)
#include "cgi_module_mac_address.c"
#endif /* SYS_CPNT_AMTR */

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
#include "cgi_module_mac_notify.c"
#endif /* SYS_CPNT_AMTR_MAC_NOTIFY */

#if (SYS_CPNT_VLAN == TRUE)
#include "cgi_module_vlan.c"
#endif /* SYS_CPNT_VLAN */

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
#include "cgi_module_addrunconfig.c"
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

#include "cgi_module_device.c"
#include "cgi_module_system.c"
#include "cgi_module_interface.c"
#include "cgi_module_calendar.c"
#if (SYS_CPNT_LLDP == TRUE)
#include "cgi_module_lldp.c"
#endif /* SYS_CPNT_LLDP */

#include "cgi_module_stp.c"

#if (SYS_CPNT_OSPF == TRUE)
#include "cgi_module_ospf.c"
#endif /* SYS_CPNT_OSPF */

#if (SYS_CPNT_ARP == TRUE)
#include "cgi_module_arp.c"
#endif /* SYS_CPNT_ARP */

#if (SYS_CPNT_NMTR == TRUE)
#include "cgi_module_statistics.c"
#endif /* SYS_CPNT_NMTR */

#if (SYS_CPNT_MLAG == TRUE)
#include "cgi_module_mlag.c"
#endif /* SYS_CPNT_MLAG */

#if (SYS_CPNT_VXLAN == TRUE)
#include "cgi_module_vxlan.c"
#endif /* SYS_CPNT_VXLAN */

#if (SYS_CPNT_NTP == TRUE)
#include "cgi_module_ntp.c"
#endif /* SYS_CPNT_NTP */

#include "cgi_module_logging.c"

#include "cgi_module_port_channel.c"

#if (SYS_CPNT_IPV4_ROUTING == TRUE)
#include "cgi_module_route.c"
#endif /* SYS_CPNT_IPV4_ROUTING */

#if (SYS_CPNT_BGP_RESTAPI == TRUE)
#include "cgi_module_router.c"
#endif /* SYS_CPNT_BGP_RESTAPI */

#if (SYS_CPNT_PFC == TRUE)
#include "cgi_module_pfc.c"
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_SFLOW == TRUE)
#include "cgi_module_sflow.c"
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

#include "cgi_module_storm.c"

#if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE)
#include "cgi_module_cos.c"
#endif  /* #if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE) */

#if (SYS_CPNT_WRED == TRUE)
#include "cgi_module_ecn.c"
#endif  /* #if (SYS_CPNT_WRED == TRUE) */

#include "cgi_module_acl.c"
#include "cgi_module_monitor.c"

#if (SYS_CPNT_RSPAN == TRUE)
#include "cgi_module_rspan.c"
#endif  /* #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
#include "cgi_module_scheduler.c"
#endif  /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE) */

#if (SYS_CPNT_NSM_POLICY == TRUE)
#include "cgi_module_policy_route.c"
#endif  /* #if (SYS_CPNT_NSM_POLICY == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "cgi_module_dhcpsnp.c"
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

#if (SYS_CPNT_DHCP_RELAY == TRUE)
#include "cgi_module_dhcprelay.c"
#endif  /* #if (SYS_CPNT_DHCP_RELAY == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
#include "cgi_module_voice_vlan.c"
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "cgi_module_pppoe_ia.c"
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if(SYS_CPNT_NETACCESS == TRUE)
#include "cgi_module_netaccess.c"
#endif  /* #if (SYS_CPNT_NETACCESS == TRUE) */

#if(SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#include "cgi_module_pvlan.c"
#endif  /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if(SYS_CPNT_TIME_RANGE == TRUE)
#include "cgi_module_time_range.c"
#endif  /* #if (SYS_CPNT_TIME_RANGE == TRUE) */

#if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV)
#include "cgi_module_qos.c"
#endif  /* #if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV) */

#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
#include "cgi_module_led.c"
#endif /* #if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
#include "cgi_module_sfp.c"
#endif  /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */


static CGI_MODULE_T cgi_module[] =
{
#if (SYS_CPNT_AMTR == TRUE)
    {CGI_MODULE_MAC_ADDRESS_Init},
#endif /* SYS_CPNT_AMTR */
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    {CGI_MODULE_MAC_NOTIFY_Init},
#endif

#if (SYS_CPNT_VLAN == TRUE)
    {CGI_MODULE_VLAN_Init},
#endif /* SYS_CPNT_VLAN */

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    {CGI_MODULE_ADDRUNCONFIG_Init},
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

    {CGI_MODULE_DEVICE_Init},

    {CGI_MODULE_SYSTEM_Init},

    {CGI_MODULE_INTERFACE_Init},

    {CGI_MODULE_CALENDAR_Init},

#if (SYS_CPNT_LLDP == TRUE)
    {CGI_MODULE_LLDP_Init},
#endif /* SYS_CPNT_LLDP */

    {CGI_MODULE_STP_Init},

#if (SYS_CPNT_OSPF == TRUE)
    {CGI_MODULE_OSPF_Init},
#endif /* SYS_CPNT_OSPF */

#if (SYS_CPNT_ARP == TRUE)
    {CGI_MODULE_ARP_Init},
#endif /* SYS_CPNT_ARP */

#if (SYS_CPNT_NMTR == TRUE)
    {CGI_MODULE_STATISTICS_Init},
#endif /* SYS_CPNT_NMTR */

#if (SYS_CPNT_MLAG == TRUE)
    {CGI_MODULE_MLAG_Init},
#endif /* SYS_CPNT_MLAG */

#if (SYS_CPNT_VXLAN == TRUE)
    {CGI_MODULE_VXLAN_Init},
#endif /* SYS_CPNT_VXLAN */

#if (SYS_CPNT_NTP == TRUE)
    {CGI_MODULE_NTP_Init},
#endif /* SYS_CPNT_NTP */

    {CGI_MODULE_LOGGING_Init},

    {CGI_MODULE_PORT_CHANNEL_Init},

#if (SYS_CPNT_IPV4_ROUTING == TRUE)
    {CGI_MODULE_ROUTE_Init},
#endif /* SYS_CPNT_IPV4_ROUTING */

#if (SYS_CPNT_BGP_RESTAPI == TRUE)
    {CGI_MODULE_ROUTER_Init},
#endif /* SYS_CPNT_BGP_RESTAPI */

#if (SYS_CPNT_PFC == TRUE)
    {CGI_MODULE_PFC_Init},
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_SFLOW == TRUE)
    {CGI_MODULE_SFLOW_Init},
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

    {CGI_MODULE_STORM_Init},

#if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE)
    {CGI_MODULE_COS_Init},
#endif  /* #if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE) */

#if (SYS_CPNT_WRED == TRUE)
    {CGI_MODULE_ECN_Init},
#endif  /* #if (SYS_CPNT_WRED == TRUE) */

    {CGI_MODULE_ACL_Init},
    {CGI_MODULE_MONITOR_Init},

#if (SYS_CPNT_RSPAN == TRUE)
    {CGI_MODULE_RSPAN_Init},
#endif  /* #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
    {CGI_MODULE_SCHEDULER_Init},
#endif  /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE) */

#if (SYS_CPNT_NSM_POLICY == TRUE)
    {CGI_MODULE_POLICY_ROUTE_Init},
#endif  /* #if (SYS_CPNT_NSM_POLICY == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
    {CGI_MODULE_DHCPSNP_Init},
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    {CGI_MODULE_DHCPRELAY_Init},
#endif  /* #if (SYS_CPNT_DHCP_RELAY == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
    {CGI_MODULE_VOICE_VLAN_Init},
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
    {CGI_MODULE_PPPOE_IA_Init},
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if(SYS_CPNT_NETACCESS == TRUE)
    {CGI_MODULE_NETACCESS_Init},
#endif  /* #if (SYS_CPNT_NETACCESS == TRUE) */

#if(SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
    {CGI_MODULE_PVLAN_Init},
#endif  /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if(SYS_CPNT_TIME_RANGE == TRUE)
    {CGI_MODULE_TIME_RANGE_Init},
#endif  /* #if (SYS_CPNT_TIME_RANGE == TRUE) */

#if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV)
    {CGI_MODULE_QOS_Init},
#endif  /* #if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV) */

#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
    {CGI_MODULE_LOCATION_LED_Init},
#endif /* #if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    {CGI_MODULE_SFP_Init},
#endif  /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

    {NULL}
};

/* ----------------------------------------------------------------------
 * CGI_MODULE_Init: Module init function
 * ---------------------------------------------------------------------- */

void CGI_MODULE_Init()
{
    UI32_T i = 0;

    for (i = 0; i < _countof(cgi_module); ++ i)
    {
        CGI_MODULE_T *module = &cgi_module[i];

        if (module->init_fn != NULL)
        {
            module->init_fn();
        }
    }
}