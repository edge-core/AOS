#ifndef  CLI_API_L3_H
#define  CLI_API_L3_H

#include "sys_hwcfg.h"
#include "cli_def.h"
#include "sys_cpnt.h"

typedef enum{
   DATABASE_TYPE_ADV_ROUTER = 0,
   DATABASE_TYPE_ASBR_SUMMARY,
   DATABASE_TYPE_DATABASE_SUMMARY,
   DATABASE_TYPE_EXTERNAL,
   DATABASE_TYPE_NETWORK,
   DATABASE_TYPE_NSSA_EXTERNAL,
   DATABASE_TYPE_ROUTER,
   DATABASE_TYPE_SELF_ORIGINATE,
   DATABASE_TYPE_SUMMARY,
   DATABASE_TYPE_ALL
}CLI_API_OspfDatabaseType_T;


#define AFLAG_V  4
#define AFLAG_E  2
#define AFLAG_B  1

/*--------------------arp rip static route----------------------------------------*/
UI32_T CLI_API_L3_Show_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Clear_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Host_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Clear_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/* move to cli_api_system.c, UI32_T CLI_API_L3_Show_Ip_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P); */
UI32_T CLI_API_L3_Arp_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Routing(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_SW_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_L3_Ip_Proxy(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*----------------------------dhcp-------------------------------------------------------*/
UI32_T CLI_API_L3_DHCP_RELAY_SERVER(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* L2 ---------> L3 */
/*#if SYS_CPNT_SUPPORTING_ROUTING*/
UI32_T CLI_API_IP_Dhcp_Restart(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Ip_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Multipath_Number(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_Defaultgateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*#endif*/

/*----------------------------------ospf-----------------------------------------*/


UI32_T CLI_API_L3_ROUTEROSPF_AREA(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Router_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Clear_Ip_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Compatible(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


UI32_T CLI_API_L3_ROUTEROSPF_Default_information(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Default_Metric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Autocost(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Routerid(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Summary_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_ROUTEROSPF_Timers_Spf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Authentication(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Authentication_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Cost(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Deadinterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Hellointerval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Message(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Retransmitinterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf_Transmitdelay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_L3_Show_Ip_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*---------------------------------dhcp_server-------------------------------------*/
UI32_T CLI_API_L3_Clear_Ip_Dhcp_Binding(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Dhcp_Excluded_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Dhcp_Pool(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Hardware_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Domain_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Dns_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Lease(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Default_Router(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Boot_File(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Netbios_Name_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Netbios_Node_Type(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_Pool_Next_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Service_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Dhcp_ClientIdentifier(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Dhcp_Binding(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Dhcp_Pool(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/* management vlan */
UI32_T CLI_API_MANAGEMENT_Vlan (UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Management_Vlan (UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_MANAGEMENT_Vlan_DefaultGateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);











/*Lin.Li,for porting RIP start*/

UI32_T CLI_API_L3_Router_Rip(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Default_Metric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Distribute_List(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Maximum_Prefix(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Timers(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Authentication_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Authentication_String(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Receive_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Receive_Packet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_RipSend_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Send_Packet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Split_Horizon(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Passive_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Distance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Originate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Exec_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Rip_Global_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Rip_Debugging(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Rip_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Rip(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Clear_Ip_Rip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T show_ip_protocols_rip(UI32_T *line_num_p);
/*Lin.Li, for porting RIP end*/

/*fuzhimin,20090212*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
UI32_T CLI_API_Ip_Address_Eth0(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
/*fuzhimin,20090212,end*/

/*Lin.Li, OSPF*/
UI32_T CLI_API_L3_Ospf_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_RouterId(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_Timer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_DefaultMetric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_PassiveInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_CompatibleRfc1583(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf_Area(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Ip_Helper(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_helper_address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Forward_Protocol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_helper_status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Mroute_Dense(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Mroute_Sparse(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T show_ip_protocols_ospf(UI32_T *line_num_p);

/* show ip protocols [bgp| ospf| rip] */
UI32_T CLI_API_L3_Show_Ip_Protocols(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* SYS_CPNT_ECMP_BALANCE_MODE */
UI32_T CLI_API_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /*end of #ifndef  CLI_API_L3_H */

