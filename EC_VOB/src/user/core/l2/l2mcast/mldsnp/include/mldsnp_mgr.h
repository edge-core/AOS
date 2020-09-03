/* MODULE NAME: mldsnp_mgr.h
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/

#ifndef _MLDSNP_MGR_H
#define _MLDSNP_MGR_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_type.h"

/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct MLDSNP_MGR_Throttle_S
{
    /* Throttle status */
    /* IGMPSNP_MGR_THROTTLE_RUNNING_STATUS_FALSE */
    /* IGMPSNP_MGR_THROTTLE_RUNNING_STATUS_TRUE  */
    UI32_T throttle_status;

    /* the throttling action                */
    /* IGMPSNP_MGR_THROTTLE_ACTION_DENY     */
    /* IGMPSNP_MGR_THROTTLE_ACTION_REPLACE  */
    UI32_T  action;

    /* The max-group number */
    UI32_T  max_group_number;

    /* The current joined group count */
    UI32_T current_group_count;

} MLDSNP_MGR_Throttle_T;


typedef struct MDLSNP_MGR_Counter_S
{
    /*Group records*/
    UI32_T num_grecs;

    /*Send group reports*/
    UI32_T num_joins_send;

    /*Group reports */
    UI32_T num_joins;

    /*Succeed group reports*/
    UI32_T num_joins_succ;

    /*Send group leave*/
    UI32_T num_leaves_send;

    /*Group leave*/
    UI32_T num_leaves;

    /*Send general query*/
    UI32_T num_gq_send;

    /*Receive general query*/
    UI32_T num_gq_recv;

    /*Send specific query*/
    UI32_T num_sq_send;

    /*Receive specific query*/
    UI32_T num_sq_recv;

    /*Receive invalid igmp message*/
    UI32_T num_invalid_mld_recv;
    /*dropped by igmp filter*/
    UI32_T num_drop_by_filter;
    /*dropped by received on mvr source or mrouter port*/
    UI32_T num_drop_by_mroute_port;
    #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    /*dropped by rate limit*/
    UI32_T num_drop_by_rate_limit;
    #endif
    UI32_T num_drop_output;
}MDLSNP_MGR_Counter_T;

typedef struct MLDSNP_MGR_InfStat_S
{
    UI32_T                  active_group_cnt;
    UI32_T                  query_exptime;
    UI32_T                  query_uptime;
    UI32_T                  other_query_expire;
    UI32_T                  other_query_uptime;
    UI32_T                  unsolict_expire;
    UI8_T                   host_ip_addr[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T                   other_querier_ip_addr[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T                   self_querier_ip_addr[SYS_ADPT_IPV6_ADDR_LEN];
    MDLSNP_MGR_Counter_T    counter;
} MLDSNP_MGR_InfStat_T;


typedef struct
{
    UI32_T                  inf_id;
    MLDSNP_MGR_InfStat_T    statistics;
    BOOL_T                  is_vlan;
} MLDSNP_MGR_IPCMsg_InfStatistics;

typedef struct
{
    UI32_T port;
    UI32_T action;
} MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T;


typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI32_T port;
} MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI8_T  ip_begin[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T  ip_end[SYS_ADPT_IPV6_ADDR_LEN];
} MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI32_T profile_mode;
} MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
} MLDSNP_MGR_IPCMsg_MldSnoopProfile_T;

typedef struct
{
    UI32_T mldsnp_FilterStatus;
} MLDSNP_MGR_IPCMsg_MldSnoopFilter_T;

typedef struct
{
    UI32_T value1;
} MLDSNP_MGR_IPCMsg_GS1_T;

typedef struct
{
    UI32_T value1;
    UI32_T value2;
} MLDSNP_MGR_IPCMsg_GS2_T;

typedef struct
{
    UI32_T value1;
    UI32_T value2;
    UI32_T value3;
} MLDSNP_MGR_IPCMsg_GS3_T;


/* IPC message structure
 */
typedef struct MLDSNP_MGR_IPCMsg_S
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
            UI32_T  vid;
            UI8_T  port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        }router_portlist;

        struct
        {
            UI32_T  vid;
            UI32_T  lport;
            UI8_T   gip_ap[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ap[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
            MLDSNP_TYPE_CurrentMode_T mode;
        }port_join_group;

        struct
        {
            UI32_T  vid;
            UI8_T   port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T   gip_ap[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ap[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
        }static_portlist;

        MLDSNP_MGR_IPCMsg_GS1_T ui32_1;
        MLDSNP_MGR_IPCMsg_GS2_T ui32_2;
        MLDSNP_MGR_IPCMsg_GS3_T ui32_3;
        MLDSNP_MGR_IPCMsg_InfStatistics statistics;
    }data;

}MLDSNP_MGR_IPCMsg_T;
/* command used in IPC message
 */
enum MLDSNP_MGR_IPCMD_Command_E
{
    MLDSNP_MGR_IPCCMD_ADDPORTSTATICJOINGROUP,
    MLDSNP_MGR_IPCCMD_ADDSTATICROUTERPORT,
    MLDSNP_MGR_IPCCMD_DELETEPORTSTATICJOINGROUP,
    MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORT,
    MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVESTATUS,
    MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVEBYHOSTSTATUS,
    MLDSNP_MGR_IPCCMD_SETLASTLISTENERQUERYINTERVAL,
    MLDSNP_MGR_IPCCMD_SETMLDSNPVER,
    MLDSNP_MGR_IPCCMD_SETMLDSTATUS,
    MLDSNP_MGR_IPCCMD_SETQUERIERSTATUS,
    MLDSNP_MGR_IPCCMD_SETQUERYINTERVAL,
    MLDSNP_MGR_IPCCMD_SETQUERYRESPONSEINTERVAL,
    MLDSNP_MGR_IPCCMD_SETROBUSTNESSVALUE,
    MLDSNP_MGR_IPCCMD_SETROUTEREXPIRETIME,
    MLDSNP_MGR_IPCCMD_SETUNKNOWNFLOODBEHAVIOR,
    MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORTLIST,
    MLDSNP_MGR_IPCCMD_SET_UNSOLICITEDREPORTINTERVAL,
    MLDSNP_MGR_IPCCMD_SETSTATICROUTERPORTLIST,
    MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICJOINGROUP,
    MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICLEAVEGROUP,
    MLDSNP_MGR_IPCCMD_SETMROUTESTATUS,

    MLDSNP_MGR_IPCCMD_SET_MLD_FILTER_STATUS,
    MLDSNP_MGR_IPCCMD_SET_MLD_CREATE_PRIFILE,
    MLDSNP_MGR_IPCCMD_SET_MLD_DESTROY_PRIFILE,
    MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_MODE,
    MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_GROUP,
    MLDSNP_MGR_IPCCMD_DELETE_MLD_PRIFILE_GROUP,
    MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_PORT,
    MLDSNP_MGR_IPCCMD_REMOVE_MLD_PRIFILE_PORT,
    MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_PORT,
    MLDSNP_MGR_IPCCMD_SET_MLD_MAXGROUP_ACTION_PORT,
    MLDSNP_MGR_IPCCMD_GETNEXT_MLD_PRIFILE_PORT,
    MLDSNP_MGR_IPCCMD_SET_MLD_REPORT_LIMIT_PER_SECOND,
    MLDSNP_MGR_IPCCMD_GET_NEXT_MLD_REPORT_LIMIT_PER_SECOND,
    MLDSNP_MGR_IPCCMD_SET_QUERY_GUARD_STATUS,
    MLDSNP_MGR_IPCCMD_GETNEXT_QUERY_GUARD_STATUS,
    MLDSNP_MGR_IPCCMD_SET_MULTICAST_DATA_DROP_STATUS,
    MLDSNP_MGR_IPCCMD_GETNEXT_MULTICAST_DATA_DROP_STATUS,
    MLDSNP_MGR_IPCCMD_SET_PROXY_REPORTING,
    MLDSNP_MGR_IPCCMD_CLEAR_MLD_SNOOP_DYNAMIC_GROUP,
    MLDSNP_MGR_IPCCMD_CLEAR_IP_MLDSNOOPING_STATISTICS,
    MLDSNP_MGR_IPCCMD_GET_INF_STATISTICS,
    MLDSNP_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC, /*below is unsynchronization*/
};

#define MLDSNP_MGR_MSGBUF_TYPE_SIZE    sizeof(((MLDSNP_MGR_IPCMsg_T *)NULL)->type)

/*-------------------------------------------------------------------------
 * MACRO NAME - MLDSNP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of MLDSNP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of MLDSNP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define MLDSNP_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)(&((MLDSNP_MGR_IPCMsg_T *)0)->data) + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of MLDSNP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of MLDSNP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(((MLDSNP_MGR_IPCMsg_T *)NULL)->type)

/*-------------------------------------------------------------------------
 * MACRO NAME - MLDSNP_MGR_MSG_CMD
 *              MLDSNP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the MLDSNP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The MLDSNP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define MLDSNP_MGR_MSG_CMD(msg_p)    (((MLDSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define MLDSNP_MGR_MSG_RETVAL(msg_p) (((MLDSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result_ui32)

/*-------------------------------------------------------------------------
 * MACRO NAME - MLDSNP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define MLDSNP_MGR_MSG_DATA(msg_p)   ((void *)&((MLDSNP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* These value will be use by mgr handler to set msg.type.result
 *   MLDSNP_MGR_IPC_RESULT_OK   - only use when API has no return value
 *                                 and mgr deal this request.
 *   MLDSNP_MGR_IPC_RESULT_FAIL - it denote that mgr handler can't deal
 *                                 the request. (ex. in transition mode)
 */
#define MLDSNP_MGR_IPC_RESULT_OK    (0)
#define MLDSNP_MGR_IPC_RESULT_FAIL  (-1)

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterTransitionMode
*------------------------------------------------------------------------------
* Purpose: This function enter the transition mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterTransitionMode();

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterMasterMode
*------------------------------------------------------------------------------
* Purpose: This function enter the master mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterMasterMode();
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterSlaveMode
*------------------------------------------------------------------------------
* Purpose: This function enter the slave mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterSlaveMode();
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  MLDSNP_MGR_SetTransitionMode(void);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function gets operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T MLDSNP_MGR_GetOperationMode(void);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_InitiateProcessResources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void MLDSNP_MGR_InitiateProcessResources();

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetMldSnpStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMldSnpStatus(
                                                MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMldSnpVer(
                                                MLDSNP_TYPE_Version_T  ver);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQuerierStatus(
                                                    MLDSNP_TYPE_QuerierStatus_T querier_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetRobustnessValue(
                                                    UI16_T  value);
 /*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  : interval - the interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQueryInterval(
                                                    UI16_T  interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQueryResponseInterval(
                                                        UI16_T  interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetLastListenerQueryInterval(
                                                        UI16_T  interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function set the unsolicitedReportInterval
* INPUT  : interval  - the inteval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetUnsolicitedReportInterval(
    UI32_T  interval);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetRouterExpireTime(
                                                    UI16_T  exp_time);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  :  flood_behavior - the returned router port info
*          vlan_id        - vlan id
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetUnknownFloodBehavior(
    UI32_T vlan_id,
                                                    MLDSNP_TYPE_UnknownBehavior_T flood_behavior);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                       - the vlan id
*          immediate_leave_status - the returned router port info
*
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetImmediateLeaveStatus(
                                                     UI16_T                        vid,
                                                     MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave by-host-ip status
* INPUT  : vid                           - the vlan id
*          immediate_leave_byhost_status - the immediate leave by-host-ip
*
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetImmediateLeaveByHostStatus(
                                                    UI16_T                        vid,
                                                    MLDSNP_TYPE_ImmediateByHostStatus_T immediate_leave_byhost_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetPortListStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid            - the vlan id
 *           *gip_ap        - the group ip
 *           *sip_ap        - the source ip
 *           *port_list_ap  - the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetPortListStaticJoinGroup(
                                                    UI16_T  vid,
                                                    UI8_T   *gip_ap,
                                                    UI8_T   *sip_ap,
                                                    UI8_T   *port_list_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetPortListStaticLeaveGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid            - the vlan id
 *           *gip_ap        - the group ip
 *           *sip_ap        - the source ip
 *           *port_list_ap  - the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetPortListStaticLeaveGroup(
                                                     UI16_T  vid,
                                                     UI8_T   *gip_ap,
                                                     UI8_T   *sip_ap,
                                                     UI8_T   *port_list_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_AddPortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           lport        - the static port list
 *           rec_type       - the include or exclude mode
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddPortStaticJoinGroup(
                                           UI16_T  vid,
                                           UI8_T *gip_ap,
                                           UI8_T *sip_ap,
                                           UI16_T  lport,
                                           MLDSNP_TYPE_CurrentMode_T rec_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_DeletePortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete static port from group
 * INPUT   : vid     - the vlan id
 *           *gip_ap - the group ip
 *           *sip_ap - the source ip
 *           lport   - the logical port
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeletePortStaticJoinGroup(
                                                       UI16_T  vid,
                                                       UI8_T   *gip_ap,
                                                       UI8_T   *sip_ap,
                                                       UI16_T  lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_AddStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the static router port
 * INPUT   : vid   - the vlan id
 *           lport - the logical port
 * OUTPUT  : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddStaticRouterPort(
                                                 UI16_T  vid,
                                                 UI16_T  lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetStaticRouterPortlist(
                                                     UI16_T  vid,
                                                     UI8_T *port_list_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_DeleteStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete the static router port
 * INPUT   : vid		- the vlan id
 *           lport		- the logical port
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeleteStaticRouterPort(
                                                    UI16_T  vid,
                                                    UI16_T  lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_DeleteStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : vid    - the vlan id
*          *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  : SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeleteStaticRouterPortlist(
                                                        UI16_T  vid,
                                                        UI8_T   *port_list_ap);
/*------------------------------------------------------------------------------
* Function :
*------------------------------------------------------------------------------
* PURPOSE: Whenever Network Interface received a LACPDU packet,it calls
*          this function to handle the packet.
*INPUT   :
*          *mref_handle_p	-- packet buffer and return buffer function pointer.
*          *dst_mac	  -- the destination MAC address of this packet.
*          *src_mac	  -- the source MAC address of this packet.
*          tag_info	  -- tag information
*          type
*          pkt_length -- the length of the packet payload.
*          ip_ext_opt_len--the extension length in ip header
*          lport      -- the logical port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN:  None
* NOTE:
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_ProcessRcvdMldPdu(
                                  L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T              dst_mac[6],
                                  UI8_T               src_mac[6],
                                  UI16_T             tag_info,
                                  UI16_T             type,
                                  UI32_T             pkt_length,
                                  UI32_T             ip_ext_opt_len,
                                  UI32_T             lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_ProcessTimerTick
*------------------------------------------------------------------------------
* Purpose: This function process the timer
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_ProcessTimerTick();

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_HandleIPCReqMsg
*------------------------------------------------------------------------------
* Purpose: This function process the timer
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_HandleIPCReqMsg( SYSFUN_Msg_T* ipcmsg_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_LportNotOperUpCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the port not oeprate up
* INPUT  : lport  - the logical port
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_LportNotOperUpCallBack(
                                       UI32_T  lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberAdd1sCallback
*------------------------------------------------------------------------------
* Purpose: This function process the a port become trunk first member
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberAdd1sCallback(
                                           UI32_T  trunk_ifindex ,
                                           UI32_T  member_ifindex);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberAddCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port join the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberAddCallback(
                                         UI32_T  trunk_ifindex,
                                         UI32_T  member_ifindex);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberDeleteCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberDeleteCallback(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberLstDeleteCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port last leave the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberLstDeleteCallback(
                                UI32_T  trunk_ifindex,
                                UI32_T  member_ifindex);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanMemberDeletedCallBack
*------------------------------------------------------------------------------
* Purpose: This function the port remove from vlan
* INPUT  : vid  - the vlan id
*          lport- the lport will remove from the vlan
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanMemberDeletedCallBack(
                                            UI32_T  vid,
                                            UI32_T  lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanDestroyCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the vlan is destroy
* INPUT  : vlan_ifindex  - the vlan ifindex
*          vlan_status   - the vlan destroy way
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanDestroyCallBack(
                                      UI32_T  vlan_ifindex,
                                      UI32_T  vlan_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanCreatedCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the vlan created
* INPUT  : vlan_ifindex  - the vlan ifindex
*         vlan_status    - the vlan create way
* OUTPUT : None
* RETURN : TRUE  - Success
*          FALSE - Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanCreatedCallBack(
                                      UI32_T  vlan_ifindex,
                                      UI32_T  vlan_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_ProcessStpTC
*------------------------------------------------------------------------------
* Purpose: This function process topology change
* INPUT  : xstp_id - the instance id
*          lport - the enter forwarding port
*          is_root- this switch is root bridge or not
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :rfc 4541 2.1.1 4) General query may be send out on all active non-router port in order
*           to reduce network convergence
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_ProcessStpTC(UI32_T xstp_id, UI16_T lport, BOOL_T is_root);


/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetIMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set multicast routing satus
* INPUT  : is_enabled - mroute status is enabled or not
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetIMRouteStatus(BOOL_T is_enabled);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void MLDSNP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* -----------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void MLDSNP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
#endif/* End of mldsnp_mgr_H */



