/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_inet.h"
#include "bgp_type.h"
#include "bgp_pom.h"
#include "bgp_om.h"
#include "string.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define BGP_POM_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = BGP_OM_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    BGP_OM_IpcMsg_T *msg_p = (BGP_OM_IpcMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_BGP;\
    msgbuf_p->msg_size = msg_size;\

#define BGP_POM_SEND_WAIT_MSG_P() \
do{\
    if(SYSFUN_OK!=SYSFUN_SendRequestMsg(msgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,\
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))\
    {\
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);\
        return BGP_TYPE_RESULT_SEND_MSG_FAIL;\
    }\
}while(0)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T msgq_handle = 0;

/* EXPORTED FUNCTION SPECIFICATIONS
 */
BOOL_T BGP_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for PIM OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_BGP_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* Get running configuration functions
 */

UI32_T BGP_POM_GetConfigDataByField(BGP_TYPE_Config_Data_T *data_p)
{
    BGP_POM_DECLARE_MSG_P(config_data);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_CONFIG_DATA_BY_FIELD;

    memcpy(&msg_p->data.config_data, data_p, sizeof(*data_p));
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    memcpy(data_p, &msg_p->data.config_data, sizeof(*data_p));

    return msg_p->type.ret_ui32;
}

BOOL_T BGP_POM_GetNextRunCfgInstance(BGP_OM_RunCfgInstance_T *runcfg_instance_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_instance);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_INSTANCE;

    memcpy(&msg_p->data.runcfg_instance, runcfg_instance_p, sizeof(msg_p->data.runcfg_instance));
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    memcpy(runcfg_instance_p, &msg_p->data.runcfg_instance, sizeof(msg_p->data.runcfg_instance));

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetNextRunCfgConfederationPeer(UI32_T as_number, UI32_T *confederation_peer_p)
{
    BGP_POM_DECLARE_MSG_P(arg_grp_ui32x2);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_CONFEDERATION_PEER;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = *confederation_peer_p;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    *confederation_peer_p = msg_p->data.arg_grp_ui32x2.ui32_2;

    return msg_p->type.ret_bool;    
}

BOOL_T BGP_POM_GetNextRunCfgAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, BGP_OM_AggregateAddr_T *aggr_addr_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_aggr_addr);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_AGGREGATE_ADDR;

    msg_p->data.runcfg_aggr_addr.as_number  = as_number;
    msg_p->data.runcfg_aggr_addr.afi        = afi;
    msg_p->data.runcfg_aggr_addr.safi       = safi;
    msg_p->data.runcfg_aggr_addr.aggr_addr  = *aggr_addr_p;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    *aggr_addr_p = msg_p->data.runcfg_aggr_addr.aggr_addr;

    return msg_p->type.ret_bool;    
}

BOOL_T BGP_POM_GetNextRunCfgDistance(UI32_T as_number, BGP_OM_Distance_T *distance_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_distance);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_DISTANCE;

    msg_p->data.runcfg_distance.as_number  = as_number;
    msg_p->data.runcfg_distance.distance   = *distance_p;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    *distance_p = msg_p->data.runcfg_distance.distance;

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetNextRuncfgNeighbor(UI32_T as_number, BGP_OM_AfiSafiNeighbor_T *neighbor_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_neighbor);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_NEIGHBOR;

    msg_p->data.runcfg_neighbor.as_number  = as_number;
    msg_p->data.runcfg_neighbor.neighbor   = *neighbor_p;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    *neighbor_p = msg_p->data.runcfg_neighbor.neighbor;

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetNextRuncfgNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, BGP_OM_Network_T *network_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_network);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_NETWORK;

    msg_p->data.runcfg_network.as_number    = as_number;
    msg_p->data.runcfg_network.afi          = afi;
    msg_p->data.runcfg_network.safi         = safi;
    msg_p->data.runcfg_network.network      = *network_p;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    *network_p = msg_p->data.runcfg_network.network;

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetRuncfgPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], BGP_OM_AfiSafiNeighbor_T *config_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_peer_group);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_RUNCFG_PEER_GROUP;

    msg_p->data.runcfg_peer_group.as_number = as_number;
    memcpy(msg_p->data.runcfg_peer_group.group_name, group_name, BGP_TYPE_PEER_GROUP_NAME_LEN+1);
    msg_p->data.runcfg_peer_group.config.afi = config_p->afi;
    msg_p->data.runcfg_peer_group.config.safi = config_p->safi;

    BGP_POM_SEND_WAIT_MSG_P();

    memcpy(group_name, msg_p->data.runcfg_peer_group.group_name, BGP_TYPE_PEER_GROUP_NAME_LEN+1);
    memcpy(config_p, &msg_p->data.runcfg_peer_group.config, sizeof(BGP_OM_AfiSafiNeighbor_T));

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetNextRuncfgPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], BGP_OM_AfiSafiNeighbor_T *config_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_peer_group);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_PEER_GROUP;

    msg_p->data.runcfg_peer_group.as_number = as_number;
    memcpy(msg_p->data.runcfg_peer_group.group_name, group_name, BGP_TYPE_PEER_GROUP_NAME_LEN+1);
    msg_p->data.runcfg_peer_group.config.afi = config_p->afi;
    msg_p->data.runcfg_peer_group.config.safi = config_p->safi;
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    memcpy(group_name, msg_p->data.runcfg_peer_group.group_name, BGP_TYPE_PEER_GROUP_NAME_LEN+1);
    memcpy(config_p, &msg_p->data.runcfg_peer_group.config, sizeof(BGP_OM_AfiSafiNeighbor_T));

    return msg_p->type.ret_bool;
}

BOOL_T BGP_POM_GetNextRuncfgPeerGroupMember(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *ipaddr_p)
{
    BGP_POM_DECLARE_MSG_P(runcfg_group_member);
    msg_p->type.cmd = BGP_OM_IPCCMD_GET_NEXT_RUNCFG_PEER_GROUP_MEMBER;

    msg_p->data.runcfg_group_member.as_number = as_number;
    memcpy(msg_p->data.runcfg_group_member.group_name, group_name, BGP_TYPE_PEER_GROUP_NAME_LEN+1);
    memcpy(&(msg_p->data.runcfg_group_member.ipaddr), ipaddr_p, sizeof(L_INET_AddrIp_T));
	
    BGP_POM_SEND_WAIT_MSG_P();
	
    memcpy(ipaddr_p, &(msg_p->data.runcfg_group_member.ipaddr), sizeof(L_INET_AddrIp_T));

    return msg_p->type.ret_bool;
}

