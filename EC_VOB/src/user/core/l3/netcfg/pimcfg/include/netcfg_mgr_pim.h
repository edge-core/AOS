/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_PIM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/05/18     --- Hongliang, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_MGR_PIM_H
#define NETCFG_MGR_PIM_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "l_radix.h"

#define NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_PIM_IPCMsg_Type_U)
#define INTERFACE_NAMSIZ 20

/* command used in IPC message
 */
enum
{
    NETCFG_MGR_PIM_IPC_ENABLE_DENSE_MODE,  
    NETCFG_MGR_PIM_IPC_DISABLE_DENSE_MODE,
    NETCFG_MGR_PIM_IPC_SET_HELLO_INTERVAL,
    NETCFG_MGR_PIM_IPC_SET_HELLO_HOLD_INTERVAL,
    NETCFG_MGR_PIM_IPC_SET_PIM_NEIGHBOR_FILTER,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_NEIGHBOR_FILTER,
    NETCFG_MGR_PIM_IPC_PIM_EXCLUDE_GENID,
    NETCFG_MGR_PIM_IPC_SET_PIM_DR_PRIORITY,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_DR_PRIORITY,
    NETCFG_MGR_PIM_IPC_ENABLE_SPARSE_MODE,
    NETCFG_MGR_PIM_IPC_DISABLE_SPARSE_MODE,
    NETCFG_MGR_PIM_IPC_ENABLE_STATE_REFRESH,
    NETCFG_MGR_PIM_IPC_DISABLE_STATE_REFRESH,
    NETCFG_MGR_PIM_IPC_SET_PIM_STATE_REFRESH_INTERVAL,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_STATE_REFRESH_INTERVAL,
    NETCFG_MGR_PIM_IPC_SET_PIM_ACCEPT_REGISTER_LIST,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_ACCEPT_REGISTER_LIST,
    NETCFG_MGR_PIM_IPC_ENABLE_CRP_PREFIX,
    NETCFG_MGR_PIM_IPC_DISABLE_CRP_PREFIX,
    NETCFG_MGR_PIM_IPC_IGNORE_RP_PRIORITY,
    NETCFG_MGR_PIM_IPC_NO_IGNORE_RP_PRIORITY,
    NETCFG_MGR_PIM_IPC_SET_PIM_JOIN_PRUNE_INTERVAL,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_JOIN_PRUNE_INTERVAL,
    NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_RATE_LIMIT,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_RATE_LIMIT,
    NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_SUPPRESSION_TIME,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_SUPPRESSION_TIME,
    NETCFG_MGR_PIM_IPC_SET_PIM_RP_ADDRESS,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_RP_ADDRESS,
    NETCFG_MGR_PIM_IPC_SET_PIM_KAT_TIME,
    NETCFG_MGR_PIM_IPC_UNSET_PIM_KAT_TIME,
    NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_CHECKSUM,
    NETCFG_MGR_PIM_IPC_ENABLE_REGISTER_CHECKSUM,
    NETCFG_MGR_PIM_IPC_DISABLE_REGISTER_CHECKSUM,
    NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE,
    NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE_GROUP_ADDR,
    NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE,
    NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE_GROUP_ADDR,
    NETCFG_MGR_PIM_IPC_ENABLE_BSR_CANDIDATE,    
    NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_HASH,
    NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_PRIORITY,    
    NETCFG_MGR_PIM_IPC_DISABLE_BSR_CANDIDATE,
    NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY,
    NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY_WITH_GROUP_LIST,
    NETCFG_MGR_PIM_IPC_UNSET_SPT_THRESHOLD_INFINITY
};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_MGR_PIM_IPCMsg_T.data
 */
#define NETCFG_MGR_PIM_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_PIM_IPCMsg_T*)0)->data.field_name))




typedef struct
{
    union NETCFG_MGR_PIM_IPCMsg_Type_U
    {
        UI32_T cmd;                /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T  result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T        ui32_v;
        int           int_v;
        char          char_v[20];
        struct pal_in4_addr     addr_v;
        struct
        {
            UI32_T  arg1;
            UI32_T  arg2;
        } arg_grp1;          
        struct
        {
            UI32_T  arg1;
            UI8_T   arg2[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
        } arg_grp2;   
        struct
        {
            UI32_T  arg1;
            BOOL_T  arg2;
        } arg_grp3;   
        struct
        {
            UI8_T   arg1[INTERFACE_NAMSIZ + 1];
            UI32_T  arg2;
            UI32_T  arg3;
        } arg_grp4;  

    } data;
}NETCFG_MGR_PIM_IPCMsg_T;

BOOL_T NETCFG_MGR_PIM_InitiateProcessResources(void);
void NETCFG_MGR_PIM_EnterMasterMode (void); 
void NETCFG_MGR_PIM_ProvisionComplete(void); 
void NETCFG_MGR_PIM_EnterSlaveMode (void);
void NETCFG_MGR_PIM_SetTransitionMode(void);
void NETCFG_MGR_PIM_EnterTransitionMode (void);   
BOOL_T NETCFG_MGR_PIM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p); 
UI32_T NETCFG_MGR_PIM_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);
UI32_T NETCFG_MGR_PIM_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);
UI32_T NETCFG_MGR_PIM_SignalInterfaceDelete(UI32_T ifindex);
UI32_T NETCFG_MGR_PIM_SignalInterfaceAdd(UI32_T ifindex);
BOOL_T NETCFG_MGR_PIM_UnSetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr);
BOOL_T NETCFG_MGR_PIM_SetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr);
BOOL_T NETCFG_MGR_PIM_UnSetPimRpCandidate(UI8_T *ifName);
BOOL_T NETCFG_MGR_PIM_SetPimRpCandidate(UI8_T *ifName);
BOOL_T NETCFG_MGR_PIM_SetCandidateBsrPriority(UI8_T *ifName, UI32_T prioriy);
BOOL_T NETCFG_MGR_PIM_SetCandidateBsrHash(UI8_T *ifName, UI32_T hash);
BOOL_T NETCFG_MGR_PIM_DisableCandidateBsr(void);
BOOL_T NETCFG_MGR_PIM_EnableCandidateBsr(UI8_T *ifName);
BOOL_T NETCFG_MGR_PIM_UnSetPimSptThresholdInfinity(void);
BOOL_T NETCFG_MGR_PIM_SetPimSptThresholdInfinity(void);
BOOL_T NETCFG_MGR_PIM_SetPimSptThresholdInfinityGroupList(UI8_T *list);
BOOL_T NETCFG_MGR_PIM_DisableRegisterChecksum(void);
BOOL_T NETCFG_MGR_PIM_EnableRegisterChecksum(void);
BOOL_T NETCFG_MGR_PIM_SetRegisterChecksumGroupList(UI8_T *list);
BOOL_T NETCFG_MGR_PIM_SetKat(UI32_T time);
BOOL_T NETCFG_MGR_PIM_SetRegisterSuppressionTime(UI32_T time);
BOOL_T NETCFG_MGR_PIM_SetRegisterRateLimit(UI32_T limit);
BOOL_T NETCFG_MGR_PIM_SetJoinPruneInterval(UI32_T interval);
BOOL_T NETCFG_MGR_PIM_NoIgnoreRpPriority(void);
BOOL_T NETCFG_MGR_PIM_IgnoreRpPriority(void);
BOOL_T NETCFG_MGR_PIM_IgnoreRpPriority(void);
BOOL_T NETCFG_MGR_PIM_EnableCrpPrefix(void);
BOOL_T NETCFG_MGR_PIM_SetAcceptRegisterList(UI8_T *flt);
BOOL_T NETCFG_MGR_PIM_UnSetAcceptRegisterList(void);
BOOL_T NETCFG_MGR_PIM_SetDRPriority(UI32_T ifindex, UI32_T priority);
BOOL_T NETCFG_MGR_PIM_SetExcludeGenID(UI32_T ifindex, BOOL_T exclude_b);
BOOL_T NETCFG_MGR_PIM_SetNeighborFilter(UI32_T ifindex, UI8_T *flt);
BOOL_T NETCFG_MGR_PIM_UnSetNeighborFilter(UI32_T ifindex);
BOOL_T NETCFG_MGR_PIM_SetHelloHoldInterval(UI32_T ifindex, UI32_T interval);
BOOL_T NETCFG_MGR_PIM_SetHelloInterval(UI32_T ifindex, UI32_T interval);
BOOL_T NETCFG_MGR_PIM_DisableSparseMode(UI32_T ifindex);
BOOL_T NETCFG_MGR_PIM_EnableSparseMode(UI32_T ifindex);
BOOL_T NETCFG_MGR_PIM_DisableDenseMode(UI32_T ifindex);
BOOL_T NETCFG_MGR_PIM_EnableDenseMode(UI32_T ifindex);  

#endif /* NETCFG_MGR_PIM_H */

