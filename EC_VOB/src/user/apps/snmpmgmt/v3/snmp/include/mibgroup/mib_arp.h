#ifndef ARP_SNMP_H
#define ARP_SNMP_H
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

/********************************************
 **********arpProxyArpTable******************
 ********************************************
 */
#define ARPPROXYARPIFINDEX              1
#define ARPPROXYARPSTATUS               2

/*arpMgt*/
void            init_arpMgt(void);
Netsnmp_Node_Handler do_arpCacheTimeout;
Netsnmp_Node_Handler get_arpStatRcvRequestPackets;
Netsnmp_Node_Handler get_arpStatSendReplyPackets;
Netsnmp_Node_Handler get_arpStatRcvReplyPackets;
Netsnmp_Node_Handler get_arpStatSendRequestPackets;
Netsnmp_Node_Handler do_arpCacheDeleteAll;

/*arpProxyArpTable*/
void            init_arpProxyArpTable(void);
FindVarMethod   var_arpProxyArpTable;
WriteMethod     write_arpProxyArpStatus;

#endif
