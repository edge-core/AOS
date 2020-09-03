#ifndef CLI_API_DEBUG_H
#define CLI_API_DEBUG_H

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Aaa
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of aaa
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Aaa(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Arp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of arp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Cluster
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of cluster
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Cluster(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Dhcp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of dhcp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Dot1x
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of dot1x
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Dot1x(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Ipdhcpsnp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of ip dhcp snooping
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Ipdhcpsnp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Lacp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of lacp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Lacp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Portsecurity
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of port security
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Portsecurity(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Radius
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of radius
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Radius(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Tacacs
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of tacacs
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Tacacs(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Telnet
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of telnet
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Telnet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Spanningtree
 *------------------------------------------------------------------------------
 * PURPOSE  :debug spanning-tree {all|root|bpdu|events} at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_mldsnp
 *------------------------------------------------------------------------------
 * PURPOSE  :debug mldsnp {all|rx|tx|event} at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Mldsnp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Debug
 *------------------------------------------------------------------------------
 * PURPOSE  :show debug at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_SyncE
 *------------------------------------------------------------------------------
 * PURPOSE  :debug synce {all|rx|tx} at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Vxlan
 *------------------------------------------------------------------------------
 * PURPOSE  :debug VXLAN {all|database|event|vni|vtep} at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/

UI32_T CLI_API_Debug_Vxlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

