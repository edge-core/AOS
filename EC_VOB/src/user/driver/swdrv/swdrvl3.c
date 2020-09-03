/*******************************************************************
 *
 *    DESCRIPTION:
 *       Layer 3 Driver Layer API Specifications
 *
 *    AUTHOR:
 *
 *    HISTORY:
 *
 *   By              Date     Ver.   Modification Description
 *   --------------- -------- -----  ---------------------------------------
 *   Anderson                        Created
 *   Ted           01/24/2002        1. Clarify the API
 *                                   2. Change the IPMC related APIs
 *   Ted           07/09/2002        Fixed SWDRVL3_DeleteHostRoute() input parameter "dst_vid" data type
 *   Garfield      06/12/2003        1. Remove parameter next_hop_gateway_ip
 *                                      in api SWDRVL3_AddNetRoute()
 *                                   2. Add two apis SWDRVL3_EnableUnicastStormProtect()
 *                                      and SWDRVL3_DisableUnicastStormProtect()
 *   Garfield      05/03/2004        1. Use new mechanism to solve the performance problem:
 *                                      Problem: When adding or deleting a net route or host route entry
 *                                               from AMTRL3 directly, AMTRL3 takes too long time to complete
 *                                               the whole action, and hence, produces performance issue.
 *                                      New Mechanism: Spawn a task in SWDRVL3. Use this task to add and create
 *                                               net routes or host routes. The FSM is in the L_HASH library.
 *                                   2. For SWDRVL3_DeleteHostRouteByPort, swdrvl3 must translate unit+port to
 *                                      lport. This makes L_HASH can delete all host routes according to lport.
 *                                      SWDRVL3 also needs to take over trunk issue.
 *   Garfield      05/24/2004        1. Add several new APIs:
 *                                      SWDRVL3_AddNetRouteWithNextHopIp().
 *                                      SWDRVL3_SlaveAddNetRouteWithNextHopIp().
 *                                      SWDRVL3_LocalAddNetRouteWithNextHopIp()
 *                                      These APIs are for adding net route with next hop IP.
 *                                      They are needed by SDK4.2.3
 *   Garfield      06/08/2004        1. Add backdoor counters for system performance analysis.
 *   Garfield      06/17/2004        1. Add one new API:
 *                                      SWDRVL3_SetDefaultRouteWithNextHopIp()
 *                                      This API is for adding default route with next hop IP.
 *                                      The next hop IP is needed by SDK4.2.3
 *   Garfield      06/28/2004        1. Add hash function checking :
 *                                      Each time core layer (AMTRL3) tries to add a net route,
 *                                      check the DEFIP Table first. If the DEFIP table is full, returns
 *                                      SWDRVL3_L3_BUCKET_FULL error message.
 *                                      By the same way, each time core layer (AMTRL3) tries to add a host
 *                                      route, check the l3 table first. If the l3 table is full, returns
 *                                      SWDRVL3_L3_BUCKET_FULL error message.
 *******************************************************************
 */
#define SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES          FALSE
#define SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER    FALSE
#define SWDRVL3_ENABLE_DROP_UNKNOWN_IP               FALSE

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "stktplg_om.h"
#include "stktplg_pom.h"
#include "dev_swdrvl3.h"
#include "dev_swdrvl3_pmgr.h"
#include "stdio.h"
#include "string.h"
#include "isc.h"
#include "l_hash.h"
#include "l_cvrt.h"
#include "syslog_type.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "syslog_pmgr.h"
#include "backdoor_mgr.h"
#include "swdrv_cache_mgr.h"
#include "swdrvl3.h"
#include "swdrvl3_type.h"

#if (SYS_CPNT_HRDRV == TRUE)
/* kelin-pttch */
#include "hrdrv.h"
#endif

#if (SYS_CPNT_SWDRVL3_CACHE == TRUE)
    #include "swdrvl3_cache.h"
#endif

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
#include "iml_mgr.h"
#include "amtrl3_mgr.h"
#include "lan.h"
#include "l2mux_mgr.h"
#endif

#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
#include "netif_mgr.h"
#include "amtrl3_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define SWDRVL3_TASK_Create_Tasks_FunNo 0
#define SWDRVL3_TASK_Create_Tasks_ErrNo 0

#define SWDRVL3_BACKDOOR_TIME_TICK_INTERVAL  100    /* timer tick every 1 seconds */
#define SWDRVL3_TIMER_EVENT                 BIT_0
#define SWDRVL3_UPDATE_EVENT                BIT_1
#define SWDRVL3_TUNNEL_TIMER_EVENT          BIT_2     /* for tunnel check net route hit bit*/
#define SWDRVL3_TUNNEL_TIME_TICK_INTERVAL  (SYS_BLD_TICKS_PER_SECOND * 20)  /* timer tick every 20 second */
#define SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS  50      /* Each time SWDRVL3 task processes 50 entries */
#define SWDRVL3_ALL_DEVICE              -1          /* broadcom StrataSwitch */
#define SWDRVL3_ALL_EVENTS          (SWDRVL3_UPDATE_EVENT | SWDRVL3_TUNNEL_TIMER_EVENT)


/* The following two definitions are for ISC */
#define SWDRVL3_TIME_OUT                1500         /* time to wait for ISC reply */
#define SWDRVL3_TRY_TIMES               4

/* The following two definitions are for route entries process*/
#define ADD_ROUTE                       1
#define DEL_ROUTE                       2

/* The following definition is for L3 hash bucket size */
#define L3_BUCKET_SIZE                      512

#define DEF_ROUT_EPORT                      0x1f
#define DEF_ROUT_EVID                       1
#define DEF_IS_TRUNK                        FALSE
#define DEF_TRUNK_ID                        0
#define DEF_TAGGED_FRAME                    FALSE

/* Backdoor parameter
 */
#define SDL3C_BACKDOOR_CMD_MAXLEN       16
#define KEY_ESC                         27
#define KEY_BACKSPACE                   8
#define KEY_EOS                         0

#define SWDRVL3_POOL_ID_ISC_SEND         0
#define SWDRVL3_POOL_ID_ISC_REPLY        1

/* MACRO FUNCTION DECLARATIONS
 */

#define ERRMSG(str)                 printf("\r\n%s(): %s", __FUNCTION__, str )

#define SWDRVL3_RETURN_WITH_VALUE(ret)        { return ret;}
#define SWDRVL3_RETURN_WITHOUT_VALUE()        { return;}

#define SWDRVL3_SIZEOF(type, field)           sizeof (_##type->field)
#define SWDRVL3_OFFSET(offset, type, field)   { type v; offset=(UI32_T)&v.##field - (UI32_T)&v; }

#define SWDRVL3_NET_ROUTE_HASH_BUFFER_OFFSET         sizeof(SWDRVL3_ShmemData_T)
#define SWDRVL3_HOST_ROUTE_HASH_BUFFER_OFFSET        (SWDRVL3_NET_ROUTE_HASH_BUFFER_OFFSET + shmem_data_p->SWDRVL3_net_route_hash_sz)
#define SWDRVL3_HOST_TUNNEL_ROUTE_HASH_BUFFER_OFFSET (SWDRVL3_HOST_ROUTE_HASH_BUFFER_OFFSET + shmem_data_p->SWDRVL3_host_route_hash_sz)
#define SWDRVL3_NET_TUNNEL_ROUTE_SORT_LST_BUFFER_OFFSET (SWDRVL3_HOST_TUNNEL_ROUTE_HASH_BUFFER_OFFSET + shmem_data_p->SWDRVL3_host_tunnel_route_hash_sz)

#define SWDRVL3_EnterCriticalSection()        SYSFUN_TakeSem(swdrvl3_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SWDRVL3_LeaveCriticalSection()        SYSFUN_GiveSem(swdrvl3_sem_id)

#define    SWDRVL3_TUNNEL_INIT_TO_DEVDRV(_devdrv_init_, _swdrv_init_) do{\
    _devdrv_init_.l3_intf_id = _swdrv_init_.l3_intf_id;\
    _devdrv_init_.vid= _swdrv_init_.vid;\
    memcpy(_devdrv_init_.src_mac,  _swdrv_init_.src_mac, sizeof(_devdrv_init_.src_mac));\
    _devdrv_init_.tunnel_type= _swdrv_init_.tunnel_type;\
    _devdrv_init_.ttl= _swdrv_init_.ttl;\
    memcpy(_devdrv_init_.nexthop_mac,  _swdrv_init_.nexthop_mac, sizeof(_devdrv_init_.nexthop_mac));\
    _devdrv_init_.sip= _swdrv_init_.sip;\
    _devdrv_init_.dip= _swdrv_init_.dip;\
}while(0)

#define    SWDRVL3_TUNNEL_TERM_TO_DEVDRV(_devdrv_term_, _swdrv_term_) do{\
    _devdrv_term_.fib_id = _swdrv_term_.fib_id;\
    _devdrv_term_.tunnel_type= _swdrv_term_.tunnel_type;\
    memcpy(_devdrv_term_.lport, _swdrv_term_.lport, sizeof(_devdrv_term_.lport));\
    _devdrv_term_.sip= _swdrv_term_.sip;\
    _devdrv_term_.dip= _swdrv_term_.dip;\
}while(0)
  


#if (SYS_CPNT_SWDRVL3 == FALSE)
#define L_HASH_SHMEM_ACQUIRE_TYPE_SWDRVL3_HOSTROUTE 1
#define L_HASH_SHMEM_ACQUIRE_TYPE_SWDRVL3_NETROUTE  1
#endif

/* DATA TYPE DECLARATIONS
 */



typedef BOOL_T (*SWDRVL3_ServiceFunc_t)(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);

/* Backdoor parameter
 */



/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     SWDRVL3_TASK_Main(void);
static BOOL_T   SWDRVL3_EventHandler(void);

/* slave callback function */
static BOOL_T SWDRVL3_SlaveEnableIpForwarding		(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDisableIpForwarding		(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveEnableRouting              (ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDisableRouting             (ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveCreateL3Interface(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteL3Interface(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddL3Mac(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteL3Mac(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveSetL3Bit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveUnSetL3Bit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveSetInetHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddInetNetRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetNetRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
/*static BOOL_T SWDRVL3_SlaveAddInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);*/
/*static BOOL_T SWDRVL3_SlaveDeleteInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);*/
static BOOL_T SWDRVL3_SlaveAddTunnelInitiator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteTunnelInitiator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveTunnelUpdateTtl(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddTunnelTerminator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteTunnelTerminator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddInetMyIpHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetMyIpHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddInetECMPRouteMultiPath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetECMPRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddInetECMPRouteOnePath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteInetECMPRouteOnePath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveSetSpecialDefaultRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveDeleteSpecialDefaultRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveReadAndClearNetRouteEntryHitBit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveClearHostRouteHWInfo(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveClearNetRouteHWInfo(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_TunnelNetRouteHitBitChange(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveAddTunnelIntfL3(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);
static BOOL_T SWDRVL3_SlaveSetEcmpBalanceMode(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p);

static void SWDRVL3_InitStackInfo();

#if (SWDRVL3_ENABLE_DROP_UNKNOWN_IP == TRUE)
static void SWDRVL3_SetDefaultValueToChip();
#endif

static BOOL_T SWDRVL3_LocalEnableIpForwarding(UI32_T flag, UI32_T vr_id);
static BOOL_T SWDRVL3_LocalDisableIpForwarding(UI32_T flag, UI32_T vr_id);
static BOOL_T SWDRVL3_LocalEnableRouting                (void);
static BOOL_T SWDRVL3_LocalDisableRouting               (void);
static UI32_T SWDRVL3_LocalCreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T *hw_info);
static BOOL_T SWDRVL3_LocalDeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info);
static UI32_T SWDRVL3_LocalAddL3Mac(UI8_T *vlan_mac, UI32_T vid);       
static BOOL_T SWDRVL3_LocalDeleteL3Mac(UI8_T *vlan_mac, UI32_T vid);
static BOOL_T SWDRVL3_LocalSetL3Bit(UI8_T *vlan_mac, UI32_T vid);       
static BOOL_T SWDRVL3_LocalUnSetL3Bit(UI8_T *vlan_mac, UI32_T vid);
static BOOL_T SWDRVL3_LocalSetInetHostRoute(SWDRVL3_Host_T *host);
static BOOL_T SWDRVL3_LocalDeleteInetHostRoute(SWDRVL3_Host_T *host);
static BOOL_T SWDRVL3_LocalAddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info);
static BOOL_T SWDRVL3_LocalDeleteInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info);
static BOOL_T SWDRVL3_LocalAddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host);
static BOOL_T SWDRVL3_LocalDeleteInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host);
static BOOL_T SWDRVL3_LocalAddTunnelInitiator(SWDRVL3_TunnelInitiator_T *tunnel);
static BOOL_T SWDRVL3_LocalDeleteTunnelInitiator(UI32_T l3_intf_id);
static BOOL_T SWDRVL3_LocalTunnelUpdateTtl(UI32_T l3_intf_id, UI8_T ttl);
static BOOL_T SWDRVL3_LocalAddTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel);
static BOOL_T SWDRVL3_LocalDeleteTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel);
static UI32_T SWDRVL3_LocalAddInetMyIpHostRoute(SWDRVL3_Host_T *host);
static UI32_T SWDRVL3_LocalDeleteInetMyIpHostRoute(SWDRVL3_Host_T *host);
static UI32_T SWDRVL3_LocalAddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count);
static UI32_T SWDRVL3_LocalDeleteInetECMPRoute(SWDRVL3_Route_T *route);
static UI32_T SWDRVL3_LocalAddInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first);
static BOOL_T SWDRVL3_LocalDeleteInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last);
static BOOL_T SWDRVL3_LocalSetSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action);
static BOOL_T SWDRVL3_LocalDeleteSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action);
static BOOL_T SWDRVL3_LocalReadAndClearHostRouteEntryHitBit(SWDRVL3_Host_T *host, UI32_T *hit);
static BOOL_T SWDRVL3_LocalClearHostRouteHWInfo(SWDRVL3_Host_T *host);
static BOOL_T SWDRVL3_LocalClearNetRouteHWInfo(SWDRVL3_Route_T *route);
static BOOL_T SWDRVL3_LocalReadAndClearNetRouteEntryHitBit(SWDRVL3_Route_T *tunnel_route, UI32_T *hit);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
static BOOL_T SWDRVL3_LocalSetEcmpBalanceMode(UI32_T mode);
#endif

/* amytu remove 2004-07-19
 * Reason: These local functions are not in used.
 */
#if 0
static BOOL_T SWDRVL3_LocalSetAgeTimer                  (UI32_T second);
static BOOL_T SWDRVL3_LocalDeleteHostRouteBySubnet      (UI32_T subnet, UI32_T prefix_length);
static UI32_T SWDRVL3_LocalAgeoutHostTable              (UI32_T scan_count, void (*CallBackFun)(UI32_T ip));
#endif
#if 0
static BOOL_T SWDRVL3_LocalDeleteHostRouteByPort        (UI32_T unit, UI32_T port, UI32_T trunk_id, BOOL_T is_trunk);
static BOOL_T SWDRVL3_LocalClearAllHostRouteTable       (void);
static BOOL_T SWDRVL3_LocalReadAndClearHostRouteHitBit  (UI32_T dst_ip, UI32_T *hit);
#endif

static void SWDRVL3_InitHashHostTable(void);
static void SWDRVL3_InitHashNetTable(void);
static void SWDRVL3_InitHashHostTunnelTable(void);
static void SWDRVL3_InitLinkListNetTunnelTable(void);

static BOOL_T SWDRVL3_ProcessNetRouteQueue(void);
static BOOL_T SWDRVL3_ProcessHostRouteQueue(void);
static BOOL_T SWDRVL3_ProcessTunnelHostRouteQueue(void);
static BOOL_T SWDRVL3_TunnelNetRouteHandler(void);
/* Backdoor parameter
 */
static void   SDL3C_BACKDOOR_Main(void);
static void   SDL3C_BACKDOOR_Engine(void);
static void   SDL3C_BACKDOOR_ParseCmd(UI8_T *cmd_buf, UI8_T ch);
static void   SDL3C_BACKDOOR_ExecuteCmd(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_TerminateCmd(UI8_T *cmd_buf);

static void   SDL3C_BACKDOOR_PrintMenu(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ShowDebugFlag(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DisplayHostCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DisplayNetCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_SetDebugFlag(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_SetNumMaxProcess(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DisplayLhashCounter(UI8_T *cmd_buf);

static void   SDL3C_BACKDOOR_SetSpecialDefaultRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddNetRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DeleteNetRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddHostRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ReadHostRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DeleteHostRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ClearAllHostRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ReadAndClearHostRouteHitBit(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ReadAndClearNetRouteHitBit(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ReadAndClearTunnelNetRouteHitBit(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddTunnelInitiator(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DeleteTunnelInitiator(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddTunnelTerminator(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DeleteTunnelTerminator(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddTunnelIntfL3(UI8_T *cmd_buf);

static void   SDL3C_BACKDOOR_AddHostTunnelRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_DeleteHostTunnelRoute(UI8_T *cmd_buf);

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
static void   SDL3C_BACKDOOR_EnableSystemCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ShowSystemCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ClearSystemCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ShowProcessCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ClearProcessCounter(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_ShowSystemTick(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_EnableSystemTicker(UI8_T *cmd_buf);
static void   SWDRVL3_CreateBackdoorTask(void);
static void   SWDRVL3_Backdoor_Task_Main(void);
static void   SDL3C_BACKDOOR_GetCounters(void);
static void   SDL3C_Clear_SystemPerformanceCounter(UI8_T *cmd_buf);
#endif

#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
static void   SDL3C_BACKDOOR_EnableAutoHostRoute(UI8_T *cmd_buf);
static void   SDL3C_BACKDOOR_AddHostRouteAuto(void);
static void   SWDRVL3_CreateBackdoorATask(void);
static void   SWDRVL3_Backdoor_Auto_Task_Main(void);
#endif
#if (SYS_CPNT_IP_TUNNEL == TRUE)
static void   SDL3C_BACKDOOR_ReadAndClearTunnelNetRouteHitBit(UI8_T *cmd_buf);
#endif
static BOOL_T SDL3C_Debug(UI32_T flag);
static void   SDL3C_SetDebugFlag(UI32_T flag);
static void   SDL3C_GetDebugFlag(UI32_T *flag);

static int    SDL3C_BACKDOOR_GetLine(char s[], int lim);
static UI32_T SDL3C_BACKDOOR_AtoUl(UI8_T *s, int radix);
static void   SDL3C_BACKDOOR_GetMac(UI8_T * mac);
static int   SDL3C_BACKDOOR_AtoIP4(UI8_T *s, UI8_T *ip);
static void   SDL3C_BACKDOOR_GetIP4(UI32_T * IPaddr);
static void     SDL3C_BACKDOOR_GetIP6(UI8_T *IP6addr);
static int    SDL3C_BACKDOOR_AHtoI(char *buffer);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
static void SDL3C_BACKDOOR_SetEcmpBalanceMode(UI8_T *cmd_buf);
#endif

/*  for removal warning */
#if 0
static void     SDL3C_BACKDOOR_DisplayMac(UI8_T *mac);
static void     SDL3C_BACKDOOR_DisplayIP6(UI8_T *ipaddr);
static void     SDL3C_BACKDOOR_DisplayIP4(UI32_T ipaddr);
#endif
static void SWDRVL3_Init();


/* STATIC VARIABLE DECLARATIONS
 */

static UI32_T   swdrvl3_sem_id; /* Semaphore ID */

static HostRoute_T    *_HostRoute_T;
static NetRoute_T     *_NetRoute_T;
static HostTunnelRoute_T     *_HostTunnelRoute_T;


#if (SWDRVL3_ENABLE_DROP_UNKNOWN_IP == TRUE)
static  UI32_T                       default_route_next_hop = 0xffffffff;
static  char                         default_route_mac[SYS_ADPT_MAC_ADDR_LEN] = "\x00\x30\xF1\x00\x00\x00";
#endif
/* static  UI32_T                       l3_host_bucket[512]; */


/* STATIC VARIABLE FOR BACKDOOR */

#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
static UI32_T               add_host_route_index = 0x32010102;
static UI8_T                add_host_route_mac_index[SYS_ADPT_MAC_ADDR_LEN] = "\x00\x01\x02\x00\x00\x02";
#endif

/* Backdoor parameter
 */
static  SDL3C_BACKDOOR_CommandStruct_T SDL3C_BACKDOOR_CommandTable[] =
{
    /*  cmd_str,    cmd_title,              cmd_descritption,                           cmd_function            */
    /*  ------------------------------------------------------------------------------------------------------  */
    {   "",         "SDL3C",                "Engineering Mode Main Menu",               SDL3C_BACKDOOR_PrintMenu},
    {   "n",        "Function Verification","To verify swdrvl3 APIs",                   SDL3C_BACKDOOR_PrintMenu},
    {   "d",        "Debug Flag",           "To set the debug flag",                    SDL3C_BACKDOOR_PrintMenu},
    {   "dd",       "Set Debug Flag",       "To set the debug flag",                    SDL3C_BACKDOOR_SetDebugFlag},
    {   "dm",       "Show L_hash counter",  "To show L_HASH counter",                   SDL3C_BACKDOOR_DisplayLhashCounter},
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    {   "dp",       "Show num of process",  "To show the number of process",            SDL3C_BACKDOOR_ShowProcessCounter},
    {   "da",       "Clear num of process", "To clear the number of process",           SDL3C_BACKDOOR_ClearProcessCounter},
    {   "db",       "Show system tick",     "To show the system tick",                  SDL3C_BACKDOOR_ShowSystemTick},
    {   "df",       "Enable system tick",   "To enable the system tick",                SDL3C_BACKDOOR_EnableSystemTicker},
    {   "de",       "Enable Sys Counter",   "To enable System Counter",                 SDL3C_BACKDOOR_EnableSystemCounter},
    {   "dg",       "Show Sys Counter",     "To show system counter",                   SDL3C_BACKDOOR_ShowSystemCounter},
    {   "dc",       "Clear Sys Counter",    "To clear system counter",                  SDL3C_BACKDOOR_ClearSystemCounter},
    {   "dt",       "Clear Sys_Cnt Table",  "To clear system counter table",            SDL3C_Clear_SystemPerformanceCounter},
#endif
#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
    {   "dh",       "Enable auto host route", "To enable auto write host route",        SDL3C_BACKDOOR_EnableAutoHostRoute},
#endif

    {   "na",       "Set Special Default Route", "To set a default route which can DROP/TRAP2CPU", SDL3C_BACKDOOR_SetSpecialDefaultRoute},
    {   "nb",       "Add Net Route",        "To add a routing entry",                   SDL3C_BACKDOOR_AddNetRoute},
    {   "nc",       "Delete Net Route",     "To delete a routing entry",                SDL3C_BACKDOOR_DeleteNetRoute},
    {   "nd",       "Add Host Route",       "To add a host entry",                      SDL3C_BACKDOOR_AddHostRoute},
    {   "ne",       "Read Host Route",      "To read a host entry",                     SDL3C_BACKDOOR_ReadHostRoute},
    {   "nf",       "Delete Host Route",    "To delete a host entry",                   SDL3C_BACKDOOR_DeleteHostRoute},
    {   "ng",       "Clear All Host Route", "To delete all host entries",               SDL3C_BACKDOOR_ClearAllHostRoute},
    {   "nh",       "Read and Clear Host Hit Bit",  "To read and clear hit bit of a host entry", SDL3C_BACKDOOR_ReadAndClearHostRouteHitBit},
    {   "ni",       "Read and Clear Net Hit Bit",    "To read and clear hit bit of a  net entry", SDL3C_BACKDOOR_ReadAndClearNetRouteHitBit},
    {   "nj",       "Add Tunnel Initiator",        "To add a tunnel initiator",         SDL3C_BACKDOOR_AddTunnelInitiator},
    {   "nk",       "Delete Tunnel Initiator",     "To delete a tunnel initiator",      SDL3C_BACKDOOR_DeleteTunnelInitiator},
    {   "nl",       "Add Tunnel Terminator",        "To add a tunnel terminator",         SDL3C_BACKDOOR_AddTunnelTerminator},
    {   "nm",       "Delete Tunnel Terminator",     "To delete a tunnel terminator",      SDL3C_BACKDOOR_DeleteTunnelTerminator},
    {   "nn",       "Add Host Tunnel Route",        "To add a host route with tunnel",    SDL3C_BACKDOOR_AddHostTunnelRoute},
    {   "no",       "Delete Host Tunnel Route",     "To delete a host route with tunnel", SDL3C_BACKDOOR_DeleteHostTunnelRoute},       
    {   "np",       "Add Tunnel Intf L3",   "To add a tunnel l3 intf",                  SDL3C_BACKDOOR_AddTunnelIntfL3},
    {   "nq",       "Show Net counter",     "To show add/delete net counter",           SDL3C_BACKDOOR_DisplayNetCounter},
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    {   "nr",       "Set ECMP balance mode", "To set ECMP load-balance mode",           SDL3C_BACKDOOR_SetEcmpBalanceMode},
#endif
    {   "ds",       "Show Debug Flag",      "To show the debug flag",                   SDL3C_BACKDOOR_ShowDebugFlag},
    {   "hc",       "Show host counter",    "To show add/delete host counter",          SDL3C_BACKDOOR_DisplayHostCounter},
    {   "dn",       "Set num of write",     "To set the number of write",               SDL3C_BACKDOOR_SetNumMaxProcess},
    {   "\377",     "End of Table",         "End of the Command Table",                 SDL3C_BACKDOOR_PrintMenu}
};

static  SDL3C_BACKDOOR_DebugStruct_T  SDL3C_BACKDOOR_DebugFlagTable[] =
{
    {   SDL3C_DEBUG_FLAG_NONE,          "SDL3C_DEBUG_FLAG_NONE"         },
    {   SDL3C_DEBUG_FLAG_CALLIN,        "SDL3C_DEBUG_FLAG_CALLIN"       },
    {   SDL3C_DEBUG_FLAG_CALLOUT,       "SDL3C_DEBUG_FLAG_CALLOUT"      },
    {   SDL3C_DEBUG_FLAG_ERRMSG,        "SDL3C_DEBUG_FLAG_ERRMSG"       },
    {   SDL3C_DEBUG_FLAG_ALL,           "SDL3C_DEBUG_FLAG_ALL"          }
};

static  UI8_T   SDL3C_BACKDOOR_ClearScreen[] =
{ KEY_ESC, '[', '2', 'J', KEY_EOS };




/* service function table
 */
static SWDRVL3_ServiceFunc_t SWDRVL3_func_tab[] =
{
    SWDRVL3_SlaveEnableRouting,                 /* 0 SWDRVL3_ENABLE_ROUTING                       */
    SWDRVL3_SlaveDisableRouting,                /* 1 SWDRVL3_DISABLE_ROUTING                      */
    SWDRVL3_SlaveEnableIpForwarding,            /* 2 SWDRVL3_ENABLE_IP_FORWARDING                 */
    SWDRVL3_SlaveDisableIpForwarding,           /* 3 SWDRVL3_DISABLE_IP_FORWARDING                */
    SWDRVL3_SlaveAddL3Mac,                      /* 4 SWDRVL3_ADD_L3_MAC                           */
    SWDRVL3_SlaveDeleteL3Mac,                   /* 5 SWDRVL3_DELETE_L3_MAC                        */
    SWDRVL3_SlaveSetL3Bit,                      /* 6 SWDRVL3_SET_L3_BIT                           */
    SWDRVL3_SlaveUnSetL3Bit,                    /* 7 SWDRVL3_UNSET_L3_BIT                         */    
    SWDRVL3_SlaveCreateL3Interface,             /* 8 SWDRVL3_CREATE_L3_INTERFACE                  */
    SWDRVL3_SlaveDeleteL3Interface,             /* 9 SWDRVL3_DELETE_L3_INTERFACE                  */
    SWDRVL3_SlaveSetInetHostRoute,              /* 10 SWDRVL3_SET_INET_HOST_ROUTE                 */
    SWDRVL3_SlaveDeleteInetHostRoute,           /* 11 SWDRVL3_DELETE_INET_HOST_ROUTE              */
    SWDRVL3_SlaveAddInetNetRoute,               /* 12 SWDRVL3_ADD_INET_NET_ROUTE                  */
    SWDRVL3_SlaveDeleteInetNetRoute,            /* 13 SWDRVL3_DELETE_INET_NET_ROUTE               */
    SWDRVL3_SlaveAddInetMyIpHostRoute,          /* 14 SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE           */
    SWDRVL3_SlaveDeleteInetMyIpHostRoute,       /* 15 SWDRVL3_DELETE_INET_MY_IP_HOST_ROUTE        */
    SWDRVL3_SlaveAddInetECMPRouteMultiPath,     /* 16 SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH       */
    SWDRVL3_SlaveDeleteInetECMPRoute,           /* 17 SWDRVL3_DELETE_INET_ECMP_ROUTE_MULTIPATH    */
    SWDRVL3_SlaveAddInetECMPRouteOnePath,       /* 18 SWDRVL3_ADD_INET_ECMP_ROUTE_ONE_PATH        */
    SWDRVL3_SlaveDeleteInetECMPRouteOnePath,    /* 19 SWDRVL3_DELETE_INET_ECMP_ROUTE_ONE_PATH     */
    SWDRVL3_SlaveSetSpecialDefaultRoute,        /* 20 SWDRVL3_SET_SPECIAL_DEFAULT_ROUTE           */
    SWDRVL3_SlaveDeleteSpecialDefaultRoute,     /* 21 SWDRVL3_DELETE_SPECIAL_DEFAULT_ROUTE        */
    SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit, /* 22 SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT */
    SWDRVL3_SlaveClearHostRouteHWInfo,          /* 23 SWDRVL3_CLEAR_HOST_ROUTE_HW_INFO            */
    SWDRVL3_SlaveClearNetRouteHWInfo,           /* 24 SWDRVL3_CLEAR_NET_ROUTE_HW_INFO             */
    SWDRVL3_SlaveAddTunnelInitiator,            /* 25 SWDRVL3_ADD_TUNNEL_INITIATOR                */
    SWDRVL3_SlaveDeleteTunnelInitiator,         /* 26 SWDRVL3_DELETE_TUNNEL_INITIATOR             */
    SWDRVL3_SlaveAddTunnelTerminator,           /* 27 SWDRVL3_ADD_TUNNEL_TERMINATOR               */
    SWDRVL3_SlaveDeleteTunnelTerminator,        /* 28 SWDRVL3_DELETE_TUNNEL_INITIATOR             */
    SWDRVL3_SlaveAddInetHostTunnelRoute,        /* 29 SWDRVL3_SET_INET_HOST_TUNNEL_ROUTE          */
    SWDRVL3_SlaveDeleteInetHostTunnelRoute,     /* 30 SWDRVL3_DELETE_INET_HOST_TUNNEL_ROUTE       */
    SWDRVL3_SlaveReadAndClearNetRouteEntryHitBit, /* 31 SWDRVL3_READ_AND_CLEAR_NET_ROUTE_HIT_BIT    */
    SWDRVL3_TunnelNetRouteHitBitChange,           /* 32 SWDRVL3_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE    */
    SWDRVL3_SlaveTunnelUpdateTtl,                 /* 33 SWDRVL3_TUNNEL_UPDATE_TTL*/
    SWDRVL3_SlaveAddTunnelIntfL3,                 /* 34 SWDRVL3_ADD_TUNNEL_INTF_L3 */
    SWDRVL3_SlaveSetEcmpBalanceMode,              /* 35 SWDRVL3_SET_ECMP_BALANCE_MODE */
};



static SWDRVL3_ShmemData_T *shmem_data_p;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: SWDRVL3_AttachSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init share memory semaphore
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_AttachSystemResources(void)
{
    shmem_data_p = (SWDRVL3_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWDRVL3_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_SWDRVL3_OM, &swdrvl3_sem_id);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_ProvisionComplete
 *---------------------------------------------------------------------------------
 * PURPOSE: All provision commands are settle down.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRVL3_ProvisionComplete(void)
{
    return;
}

/* FUNCTION NAME: SWDRVL3_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init L3 switch driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_InitiateSystemResources(void)
{
    shmem_data_p = (SWDRVL3_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWDRVL3_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_SWDRVL3_OM, &swdrvl3_sem_id);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    SWDRVL3_Init();
    SWDRVL3_InitHashNetTable();
    SWDRVL3_InitHashHostTable();
    SWDRVL3_InitHashHostTunnelTable();
    SWDRVL3_InitLinkListNetTunnelTable();

    /* initalization for BACKDOOR */
    SDL3C_SetDebugFlag((UI32_T)SDL3C_DEBUG_FLAG_NONE);

    L_HASH_Init();

    return;
} /* SWDRVL3_Inititate_System_Resources() */

void SWDRVL3_Init()
{
#if (SWDRVL3_ENABLE_DROP_UNKNOWN_IP == TRUE)
    shmem_data_p->default_route_inited = FALSE;
#endif
    shmem_data_p->max_num_process = SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS;
    shmem_data_p->host_added_counter = 0;   /* debug purpose */
    shmem_data_p->host_deleted_counter = 0; /* debug purpose */
    shmem_data_p->net_added_counter = 0;    /* debug purpose */
    shmem_data_p->net_deleted_counter = 0;  /* debug purpose */
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_performance_counters_index = 0;
    shmem_data_p->system_counter_enable = FALSE;
    shmem_data_p->host_added_in_chip_counter = 0;
    shmem_data_p->process_counter1 = 0;
    shmem_data_p->process_counter2 = 0;
    shmem_data_p->system_ticker1_1 = 0;
    shmem_data_p->system_ticker1_2 = 0;
    shmem_data_p->system_ticker2_1 = 0;
    shmem_data_p->system_ticker2_2 = 0;
    shmem_data_p->system_tick_usage_index = 0;
    shmem_data_p->system_ticker_enable = FALSE;
#endif
#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
    shmem_data_p->auto_host_route_enable = FALSE;
#endif
}

/* FUNCTION NAME: SWDRVL3_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_Create_InterCSC_Relation(void)
{
   BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sd3c", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, SDL3C_BACKDOOR_Main);
} /* End of SWDRVL3_Create_InterCSC_Relation() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_CreateTask
 *------------------------------------------------------------------------
 * FUNCTION: This function will create address management task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWDRVL3_CreateTask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    if (SYSFUN_SpawnThread (SYS_BLD_SWDRVL3_TASK_PRIORITY,
			                SYSFUN_SCHED_DEFAULT,
			                SYS_BLD_SWDRVL3_TASK,
                            SYS_BLD_TASK_COMMON_STACK_SIZE,
                            SYSFUN_TASK_NO_FP,
                            SWDRVL3_TASK_Main,
                            0,
                            &shmem_data_p->swdrvl3_task_tid))
    {
        owner_info.level = SYSLOG_LEVEL_CRIT;
        owner_info.module_no = SYS_MODULE_SWDRVL3;
        owner_info.function_no = SWDRVL3_TASK_Create_Tasks_FunNo;
        owner_info.error_no = SWDRVL3_TASK_Create_Tasks_ErrNo;
#if (SYS_CPNT_SYSLOG == TRUE)
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, SYS_BLD_SWDRVL3_TASK, 0, 0);
#endif
        return;
    } /* end of if */

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    SWDRVL3_CreateBackdoorTask();
#endif

#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
    SWDRVL3_CreateBackdoorATask();
#endif

    return;
} /* end of SWDRVL3_CreateTask() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterMasterMode(void)
{
    UI32_T previous_unit_id = 0, index = 1;

    SWDRVL3_InitStackInfo();

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl[0] = shmem_data_p->swdrvl3_stack_info.my_unit_id;
    SWDRVL3_LeaveCriticalSection();

    while (STKTPLG_POM_GetNextUnit(&previous_unit_id) == TRUE)
    {
        if (previous_unit_id != shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl[0])
        {
           SWDRVL3_EnterCriticalSection();
           shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl[index] = previous_unit_id;
           index++;
           SWDRVL3_LeaveCriticalSection();
        }
    }

    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);

    /* Removed by Garfield, 05/24/2004, requested by Wuli */
    /*SWDRVL3_SetDefaultValueToChip();*/
} /* SWDRVL3_EnterMasterMode() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterSlaveMode(void)
{
    SWDRVL3_InitStackInfo();

    SWDRVL3_EnterCriticalSection();
    memset(&shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl, 0, sizeof(UI32_T) * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    SWDRVL3_LeaveCriticalSection();
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);

} /* SWDRVL3_EnterSlaveMode() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterTransitionMode(void)
{
    SWDRVL3_EnterCriticalSection();

    memset((UI8_T*)&shmem_data_p->swdrvl3_stack_info, 0, sizeof(SWDRVL3_StackInfo_T));

    /* delete all host route records */
    L_HASH_ShMem_DeleteAll(&shmem_data_p->SWDRVL3_host_route_hash_desc);

    /* delete all net route records */
    L_HASH_ShMem_DeleteAll(&shmem_data_p->SWDRVL3_net_route_hash_desc);

    /* delete all tunnel host route records */
    L_HASH_ShMem_DeleteAll(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc);
    
    /* delete all tunnel net route records */
    L_SORT_LST_ShMem_Delete_All(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc);     
    SWDRVL3_LeaveCriticalSection();

    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);

} /* SWDRVL3_EnterTransitionMode() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           set transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
} /* SWDRVL3_SetTransitionMode() */


UI32_T SWDRVL3_GetShMemInfo(void )
{
    L_HASH_ShMem_Desc_T swdrvl3_host_route_hash_desc;
    L_HASH_ShMem_Desc_T swdrvl3_net_route_hash_desc;
    L_HASH_ShMem_Desc_T swdrvl3_host_tunnel_route_hash_desc;
    HostRoute_T         tmp_host;
    NetRoute_T          tmp_route;
    HostTunnelRoute_T   tmp_host_tunnel_route;
    UI32_T              shmem_buffer_size = sizeof(SWDRVL3_ShmemData_T);
    
    /* 1. Initialize Route hash table */
    memset(&swdrvl3_net_route_hash_desc,0,sizeof(swdrvl3_net_route_hash_desc));
    swdrvl3_net_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_NET_ROUTE;
    swdrvl3_net_route_hash_desc.nbr_of_hash_bucket = 512;
    swdrvl3_net_route_hash_desc.key_offset[0] = (UI8_T *)&tmp_route.dst_ip - (UI8_T *)&tmp_route;
    swdrvl3_net_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(NetRoute_T, dst_ip);
    swdrvl3_net_route_hash_desc.key_offset[1] = (UI8_T *)&tmp_route.prefix_length - (UI8_T *)&tmp_route;
    swdrvl3_net_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(NetRoute_T, prefix_length);
    swdrvl3_net_route_hash_desc.element_of_query_group[0] = (SYS_ADPT_TOTAL_NBR_OF_LPORT + 1);
    swdrvl3_net_route_hash_desc.acquire_element_index_of_query_group_fun_id = L_HASH_SHMEM_ACQUIRE_TYPE_SWDRVL3_NETROUTE;
    swdrvl3_net_route_hash_desc.record_size = sizeof(NetRoute_T);
    swdrvl3_net_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;
  
    /* plus the size of hash table */
    shmem_buffer_size += L_HASH_ShMem_GetWorkingBufferRequiredSize(&swdrvl3_net_route_hash_desc);

    /* 2. Initialize Host hash table */
    memset(&swdrvl3_host_route_hash_desc,0,sizeof(swdrvl3_host_route_hash_desc));
    swdrvl3_host_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_HOST_ROUTE;
    swdrvl3_host_route_hash_desc.nbr_of_hash_bucket = 512;
    swdrvl3_host_route_hash_desc.key_offset[0] = (UI8_T *)&tmp_host.ip_addr - (UI8_T *)&tmp_host;
    swdrvl3_host_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(HostRoute_T, ip_addr);
    swdrvl3_host_route_hash_desc.key_offset[1] = (UI8_T *)&tmp_host.dst_vid - (UI8_T *)&tmp_host;
    swdrvl3_host_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(HostRoute_T, dst_vid);
    swdrvl3_host_route_hash_desc.element_of_query_group[0] = (SYS_ADPT_TOTAL_NBR_OF_LPORT + 1);
    swdrvl3_host_route_hash_desc.acquire_element_index_of_query_group_fun_id = L_HASH_SHMEM_ACQUIRE_TYPE_SWDRVL3_HOSTROUTE;
    swdrvl3_host_route_hash_desc.record_size = sizeof(HostRoute_T);
    swdrvl3_host_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;
  
    /* plus the size of hash table */
    shmem_buffer_size += L_HASH_ShMem_GetWorkingBufferRequiredSize(&swdrvl3_host_route_hash_desc);

    /* 3. Initialize Host Tunnel Route hash table */
    memset(&swdrvl3_host_tunnel_route_hash_desc, 0, sizeof(swdrvl3_host_tunnel_route_hash_desc));
    swdrvl3_host_tunnel_route_hash_desc.nbr_of_hash_bucket = L3_BUCKET_SIZE;
    swdrvl3_host_tunnel_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_HOST_ROUTE;
    swdrvl3_host_tunnel_route_hash_desc.key_offset[0] = (UI8_T *)&tmp_host_tunnel_route.dst_vid - (UI8_T *)&tmp_host_tunnel_route;
    swdrvl3_host_tunnel_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(HostTunnelRoute_T, dst_vid);
    swdrvl3_host_tunnel_route_hash_desc.key_offset[1] = (UI8_T *)&tmp_host_tunnel_route.ip_addr - (UI8_T *)&tmp_host_tunnel_route;
    swdrvl3_host_tunnel_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(HostTunnelRoute_T, ip_addr);
    swdrvl3_host_tunnel_route_hash_desc.record_size = sizeof(HostTunnelRoute_T);
    swdrvl3_host_tunnel_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;

    shmem_buffer_size += L_HASH_ShMem_GetWorkingBufferRequiredSize(&swdrvl3_host_tunnel_route_hash_desc);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    /* 4. Initialize Net Tunnel Route sort list */
    shmem_buffer_size += L_SORT_LST_SHMEM_PT_BUFFER_REQUIRED_SZ(SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_NET_CACHE_ENTRY, sizeof(NetRoute_T));
#endif
    
    return shmem_buffer_size;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_InitStackInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will initialize the information for stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRVL3_InitStackInfo()
{
    SWDRVL3_EnterCriticalSection();
    memset((UI8_T*)&shmem_data_p->swdrvl3_stack_info, 0, sizeof(SWDRVL3_StackInfo_T));
    SWDRVL3_LeaveCriticalSection();
    STKTPLG_POM_GetMyUnitID(&(shmem_data_p->swdrvl3_stack_info.my_unit_id));
    STKTPLG_POM_GetNumberOfUnit(&shmem_data_p->swdrvl3_stack_info.num_of_units);
    STKTPLG_POM_GetMasterUnitId(&shmem_data_p->swdrvl3_stack_info.master_unit_id);
} /* SWDRVL3_InitStackInfo() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION: This function will update ethernet address table database periodically.
 *           SWDRVL3 Task monitor:
 *               1. Update Event
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1. In current desing, SWDRVL3_TASK_Main uses one event(SWDRVL3_UPDATE_EVENT)
 *              to handle the routing table access:
 *              1.1 If there is NO event happends, waits forever.
 *              1.2 If there is event happends, check the current state.
 *                  If the current state is NOT in master mode NOR slave mode,
 *                  clear event, sleep one second, and continue waiting next event.
 *              1.3 If the current state is in master mode or slave mode,
 *                  call SWDRVL3_EventHandler() to add or delete route entries.
 *                  1.3.1 If all the route entries are processed(return TRUE),
 *                        clear event bit. This makes SYSFUN_ReceiveEvent() continue
 *                        to wait next event.
 *                  1.3.2 If NOT all the route entries are processed(return FALSE),
 *                        sleep 0.1 second. Then SYSFUN_ReceiveEvent() continues
 *                        to process the remaining route entries.
 * -------------------------------------------------------------------------*/
static void SWDRVL3_TASK_Main(void)
{
    UI32_T      wait_events = 0, all_events = 0;
    UI32_T      current_state;
    BOOL_T      rc;


#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if((shmem_data_p->swdrvl3_tunnel_task_tid = SYSFUN_PeriodicTimer_Create()) == NULL)
        printf("%s: create timer (0.5 second interval) fail.\n", __FUNCTION__);

    /* This timer sends event every 3 minute (the same as amtrl3 check time in master unit)*/
    SYSFUN_PeriodicTimer_Start(shmem_data_p->swdrvl3_tunnel_task_tid, SWDRVL3_TUNNEL_TIME_TICK_INTERVAL, SWDRVL3_TUNNEL_TIMER_EVENT);
#endif



    /* Task Body */
    while (TRUE)
    {
        /* wait event */
        if ((SYSFUN_ReceiveEvent (SWDRVL3_ALL_EVENTS,
                             SYSFUN_EVENT_WAIT_ANY,
                             (all_events)? SYSFUN_TIMEOUT_NOWAIT:SYSFUN_TIMEOUT_WAIT_FOREVER,
                             &wait_events)) != SYSFUN_OK)
        {
            /*
             * Now no new event come.
             * If all_event != 0, we should continue to process L_Hash queue for old queued jobs.
             * If all_event == 0, there is no queued job in L_Hash. Go to wait new event.
             */
            if(all_events == 0){
                SYSFUN_Sleep(10);
                continue;
            }
        }

        all_events |= wait_events;

        current_state = SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
        if ((current_state != SYS_TYPE_STACKING_MASTER_MODE) && (current_state != SYS_TYPE_STACKING_SLAVE_MODE))
        {
            all_events = 0;
            SYSFUN_Sleep(100);
            continue;
        }

        if (all_events & SWDRVL3_UPDATE_EVENT)
        {
            rc = SWDRVL3_EventHandler();
            if (rc == TRUE)
            {
                all_events ^= SWDRVL3_UPDATE_EVENT;
            }
            else
            {
                SYSFUN_Sleep(10);
            }
        } /* end of if */
  
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        if(all_events & SWDRVL3_TUNNEL_TIMER_EVENT)
        {
            if (current_state == SYS_TYPE_STACKING_SLAVE_MODE)
            {
                /* we should get slave unit's hit bit and store it,
                 * if there's a change, we should use ISC to inform master unit,
                 * and then sys_callback to amtrl3 
                 */
                if(TRUE == SWDRVL3_TunnelNetRouteHandler())
                {
                    all_events ^= SWDRVL3_TUNNEL_TIMER_EVENT;
                }
                else
                {
                    SYSFUN_Sleep(10);
                }
            }
            else
            {
                all_events ^= SWDRVL3_TUNNEL_TIMER_EVENT;
            }
        }
#endif        

    } /* end of while(TRUE) */

    return;
} /* end of SWDRVL3_TASK_Main() */

/*******************************************************************************
 * SWDRVL3_EnableIpForwarding
 *
 * Purpose: This function will enable IPv4/IPv6 fowarding function.
 * Inputs:
 *          flags: Indicates either IPv4 or IPv6 forwarding function is enabled
 *	    vr_id: Virtual Router ID
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T SWDRVL3_EnableIpForwarding(UI32_T flags, UI32_T vr_id)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_CreateL3Interface");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ENABLE_IP_FORWARDING));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_ENABLE_IP_FORWARDING;
        isc_buf_p->info.ip_forwarding_op.flags = flags;
        isc_buf_p->info.ip_forwarding_op.vr_id = vr_id;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalEnableIpForwarding(flags, vr_id);
    return ret;
}

/*******************************************************************************
 * SWDRVL3_DisableIpForwarding
 *
 * Purpose: This function will Disable IPv4/IPv6 fowarding function.
 * Inputs:
 *          flags: Indicates either IPv4 or IPv6 forwarding function is Disabled
 *	    vr_id: Virtual Router ID
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T SWDRVL3_DisableIpForwarding(UI32_T flags, UI32_T vr_id)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_CreateL3Interface");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DISABLE_IP_FORWARDING));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_DISABLE_IP_FORWARDING;
        isc_buf_p->info.ip_forwarding_op.flags = flags;
        isc_buf_p->info.ip_forwarding_op.vr_id   = vr_id;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalDisableIpForwarding(flags, vr_id);
    return ret;
}


/*******************************************************************************
 * SWDRVL3_CreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 *          hw_info_p : hw info (index) of the L3 interface
 * Outputs: 
 *          hw_info_p : hw info (index) of the l3 interface
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
UI32_T  SWDRVL3_CreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T *hw_info_p)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_CreateL3Interface");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return SWDRVL3_L3_OTHERS;
        }

        isc_buf_p->ServiceID = SWDRVL3_CREATE_L3_INTERFACE;
        isc_buf_p->info.l3_interface.fib_id = fib_id;
        isc_buf_p->info.l3_interface.vid   = vid;
        isc_buf_p->info.l3_interface.hw_info = *hw_info_p;
        
        memcpy(isc_buf_p->info.l3_interface.mac, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return SWDRVL3_L3_OTHERS;
        }
    }

    return SWDRVL3_LocalCreateL3Interface(fib_id, vid, vlan_mac, hw_info_p);
}


/*******************************************************************************
 * SWDRVL3_HotInsertCreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 *          hw_info : hw info (index) of the L3 interface
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
BOOL_T SWDRVL3_HotInsertCreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertCreateL3Interface");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_CREATE_L3_INTERFACE;
        isc_buf_p->info.l3_interface.fib_id  = fib_id;
        isc_buf_p->info.l3_interface.vid     = vid;
        isc_buf_p->info.l3_interface.hw_info = hw_info;
        memcpy(isc_buf_p->info.l3_interface.mac, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    return TRUE;
}


/*******************************************************************************
 * SWDRVL3_DeleteL3Interface
 *
 * Purpose: This function will delete an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 *          hw_info : hw info (index) of the L3 interface
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteL3Interface");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_L3_INTERFACE;
        isc_buf_p->info.l3_interface.fib_id = fib_id;
        isc_buf_p->info.l3_interface.vid   = vid;
        isc_buf_p->info.l3_interface.hw_info = hw_info;
        memcpy(isc_buf_p->info.l3_interface.mac, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalDeleteL3Interface(fib_id, vid, vlan_mac, hw_info);
    return ret;
}


/*******************************************************************************
 * SWDRVL3_AddL3Mac
 *
 * Purpose: Add one MAC address with L3 bit On(For vrrp HSRP)
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddL3Mac(UI8_T *l3_mac, UI32_T vid)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddL3Mac");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return SWDRVL3_L3_OTHERS;
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_L3_MAC;
        isc_buf_p->info.l3_interface.vid   = vid;
        memcpy(isc_buf_p->info.l3_interface.mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return SWDRVL3_L3_OTHERS;
        }
    }

    ret=SWDRVL3_LocalAddL3Mac(l3_mac, vid);
    return ret;
}


/*******************************************************************************
 * SWDRVL3_DeleteL3Mac
 *
 * Purpose: Remove one L3 MAC address that belonging to one vlan interface.
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *          This function is non-applicable for Intel solution
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteL3Mac");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_L3_MAC;
        isc_buf_p->info.l3_interface.vid   = vid;
        memcpy(isc_buf_p->info.l3_interface.mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalDeleteL3Mac(l3_mac, vid);
    return ret;
}


/*******************************************************************************
 * SWDRVL3_SetL3Bit
 *
 * Purpose: Turn on the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: None
 *******************************************************************************
 */
UI32_T SWDRVL3_SetL3Bit(UI8_T *l3_mac, UI32_T vid)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_SetL3Bit");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return SWDRVL3_L3_OTHERS;
        }

        isc_buf_p->ServiceID = SWDRVL3_SET_L3_BIT;
        isc_buf_p->info.l3_interface.vid   = vid;
        memcpy(isc_buf_p->info.l3_interface.mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return SWDRVL3_L3_OTHERS;
        }
    }

    ret=SWDRVL3_LocalAddL3Mac(l3_mac, vid);
    return ret;
}


/*******************************************************************************
 * SWDRVL3_UnSetL3Bit
 *
 * Purpose: Turn off the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T SWDRVL3_UnSetL3Bit(UI8_T *l3_mac, UI32_T vid)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_UnSetL3Bit");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CREATE_L3_INTERFACE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_UNSET_L3_BIT;
        isc_buf_p->info.l3_interface.vid   = vid;
        memcpy(isc_buf_p->info.l3_interface.mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalUnSetL3Bit(l3_mac, vid);
    return ret;
}

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/*******************************************************************************
 * SWDRVL3_SetRouterAdditionalCtrlReg
 *
 * Purpose: set router addtition control register for route (Set: meter work, Unset: static route work)
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T SWDRVL3_SetRouterAdditionalCtrlReg(UI32_T value)
{
    return DEV_SWDRVL3_PMGR_SetRouterAdditionalCtrlReg(value);
}
#endif  /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/

/*******************************************************************************
 * SWDRVL3_SetInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_SetInetHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_SetInetHostRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
    {
        SWDRVL3_EnterCriticalSection();
        shmem_data_p->host_added_counter++;
    	SWDRVL3_LeaveCriticalSection();
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_SET_INET_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_SET_INET_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.dst_vid              = host->vid;
        isc_buf_p->info.host_route.unit                 = host->unit;
        isc_buf_p->info.host_route.port                 = host->port;
        isc_buf_p->info.host_route.trunk_id             = host->trunk_id;
        isc_buf_p->info.host_route.hw_info              = host->hw_info;
        memcpy (isc_buf_p->info.host_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.flags = host->flags;
    dev_swdrvl3_host_route.fib_id = host->fib_id;
    dev_swdrvl3_host_route.vid = host->vid;
    dev_swdrvl3_host_route.unit = host->unit;
    dev_swdrvl3_host_route.port = host->port;
    dev_swdrvl3_host_route.trunk_id = host->trunk_id;
    dev_swdrvl3_host_route.hw_info = host->hw_info;
    memcpy(dev_swdrvl3_host_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_host_route.mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_SetInetHostRoute(&dev_swdrvl3_host_route);
    host->hw_info = dev_swdrvl3_host_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}

/*******************************************************************************
 * SWDRVL3_AddInetHostTunnelRoute
 *
 * Purpose: This function will add a host entry as well as associated tunnel initiator/terminator.
 * Inputs:
 *          host  : host entry with tunneling information
 * Outputs: 
 *          host->hw_info
 *	    host->tnl_init.l3_intf_id
 *
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_HostTunnel_T dev_swdrvl3_host_tunnel_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_SetInetHostTunnelRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

//    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
//    {
//        SWDRVL3_EnterCriticalSection();
//        shmem_data_p->host_added_counter++;
//    	SWDRVL3_LeaveCriticalSection();
//    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_HOST_TUNNEL_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_HOST_TUNNEL_ROUTE;
        isc_buf_p->info.host_tunnel_route.flags                = host->flags;
        isc_buf_p->info.host_tunnel_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_tunnel_route.dst_vid              = host->vid;
        isc_buf_p->info.host_tunnel_route.unit                 = host->unit;
        isc_buf_p->info.host_tunnel_route.port                 = host->port;
        isc_buf_p->info.host_tunnel_route.trunk_id             = host->trunk_id;
        isc_buf_p->info.host_tunnel_route.hw_info              = host->hw_info;
        memcpy (isc_buf_p->info.host_tunnel_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_tunnel_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        SWDRVL3_TUNNEL_INIT_TO_DEVDRV(isc_buf_p->info.host_tunnel_route.tnl_init, host->tnl_init);
        SWDRVL3_TUNNEL_TERM_TO_DEVDRV(isc_buf_p->info.host_tunnel_route.tnl_term, host->tnl_term);

        if (isc_buf_p->info.host_tunnel_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_tunnel_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_tunnel_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_host_tunnel_route, 0, sizeof(DEV_SWDRVL3_HostTunnel_T));
    dev_swdrvl3_host_tunnel_route.flags = host->flags;
    dev_swdrvl3_host_tunnel_route.fib_id = host->fib_id;
    dev_swdrvl3_host_tunnel_route.vid = host->vid;
    dev_swdrvl3_host_tunnel_route.unit = host->unit;
    dev_swdrvl3_host_tunnel_route.port = host->port;
    dev_swdrvl3_host_tunnel_route.trunk_id = host->trunk_id;
    dev_swdrvl3_host_tunnel_route.hw_info = host->hw_info;
    memcpy(dev_swdrvl3_host_tunnel_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_host_tunnel_route.mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);

    SWDRVL3_TUNNEL_INIT_TO_DEVDRV(dev_swdrvl3_host_tunnel_route.tnl_init, host->tnl_init);
    SWDRVL3_TUNNEL_TERM_TO_DEVDRV(dev_swdrvl3_host_tunnel_route.tnl_term, host->tnl_term);

  
    if (dev_swdrvl3_host_tunnel_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_tunnel_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_tunnel_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_AddInetHostTunnelRoute(&dev_swdrvl3_host_tunnel_route);
    host->hw_info = dev_swdrvl3_host_tunnel_route.hw_info;
    host->tnl_init.l3_intf_id = dev_swdrvl3_host_tunnel_route.tnl_init.l3_intf_id;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}




/*******************************************************************************
 * SWDRVL3_HotInsertAddInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddInetHostRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_SET_INET_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_SET_INET_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.dst_vid              = host->vid;
        isc_buf_p->info.host_route.unit                 = host->unit;
        isc_buf_p->info.host_route.port                 = host->port;
        isc_buf_p->info.host_route.trunk_id             = host->trunk_id;
        isc_buf_p->info.host_route.hw_info              = host->hw_info;
        memcpy (isc_buf_p->info.host_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_NO_ERROR);
}

/*******************************************************************************
 * SWDRVL3_HotInsertAddInetHostTunnelRoute
 *
 * Purpose: This function will add or update a tunnel host entry.
 * Inputs:
 *          tunnel_host  : tunnel host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *tunnel_host)
{

    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddInetHostRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((tunnel_host->mac == NULL) || (tunnel_host->src_mac == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_HOST_TUNNEL_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_HOST_TUNNEL_ROUTE;
        isc_buf_p->info.host_tunnel_route.flags                = tunnel_host->flags;
        isc_buf_p->info.host_tunnel_route.fib_id               = tunnel_host->fib_id;
        isc_buf_p->info.host_tunnel_route.dst_vid              = tunnel_host->vid;
        isc_buf_p->info.host_tunnel_route.unit                 = tunnel_host->unit;
        isc_buf_p->info.host_tunnel_route.port                 = tunnel_host->port;
        isc_buf_p->info.host_tunnel_route.trunk_id             = tunnel_host->trunk_id;
        isc_buf_p->info.host_tunnel_route.hw_info              = tunnel_host->hw_info;
        memcpy (isc_buf_p->info.host_tunnel_route.dst_mac, tunnel_host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_tunnel_route.src_mac, tunnel_host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        SWDRVL3_TUNNEL_INIT_TO_DEVDRV(isc_buf_p->info.host_tunnel_route.tnl_init, tunnel_host->tnl_init);
        SWDRVL3_TUNNEL_TERM_TO_DEVDRV(isc_buf_p->info.host_tunnel_route.tnl_term, tunnel_host->tnl_term);
        
        if (isc_buf_p->info.host_tunnel_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_tunnel_route.ip_addr.ipv6_addr, tunnel_host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_tunnel_route.ip_addr.ipv4_addr = tunnel_host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_NO_ERROR);
}

/*******************************************************************************
 * SWDRVL3_TunnelUpdateTtl
 *
 * Purpose: This function will update a tunnel initiator's ttl.
 * Inputs:
 *          tunnel_entry->tnl_init.l3_intf_id -- l3 interface id bind with tunnel initiator
 *          tunnel_entry->tnl_init.ttl        -- tunnel ttl in tunnel initiator
 * Outputs: N/A
 *          
 * Return:  
 *          
 * Note:    
 *******************************************************************************
 */
UI32_T SWDRVL3_TunnelUpdateTtl(SWDRVL3_HostTunnel_T *tunnel_entry)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    DEV_SWDRVL3_TunnelInitiator_T dev_tunnel_initiator;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T unit_index, pdu_len;
    UI32_T ret = SWDRVL3_L3_NO_ERROR;
    UI16_T dst_bmp = 0, isc_ret_val;
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_UpdateTunnelTtl");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* for ISC Send Multicast Reliable */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_TUNNEL_UPDATE_TTL));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_TUNNEL_UPDATE_TTL;
        isc_buf_p->info.tunnel_initiator.l3_intf_id = tunnel_entry->tnl_init.l3_intf_id;
        isc_buf_p->info.tunnel_initiator.ttl = tunnel_entry->tnl_init.ttl;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* End of Send Multicast Reliable */

    memset(&dev_tunnel_initiator, 0, sizeof(dev_tunnel_initiator));
    dev_tunnel_initiator.l3_intf_id = tunnel_entry->tnl_init.l3_intf_id;
    dev_tunnel_initiator.ttl = tunnel_entry->tnl_init.ttl;
    ret = DEV_SWDRVL3_PMGR_UpdateTunnelTtl(&dev_tunnel_initiator);
   

    SWDRVL3_RETURN_WITH_VALUE(ret);
    
}

/*******************************************************************************
 * SWDRVL3_DeleteInetHostTunnelRoute
 *
 * Purpose: This function will delete a host entry and associated tunnel initiator/terminator.
 * Inputs:
 *          host  : host entry with tunneling information
 * Outputs: 
 *          host->hw_info
 *	    host->tnl_init.l3_intf_id
 *
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_DeleteInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_HostTunnel_T dev_swdrvl3_host_tunnel_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteInetHostTunnelRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

//    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
//    {
//        SWDRVL3_EnterCriticalSection();
//        shmem_data_p->host_added_counter++;
//    	SWDRVL3_LeaveCriticalSection();
//    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_HOST_TUNNEL_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_HOST_TUNNEL_ROUTE;
        isc_buf_p->info.host_tunnel_route.flags                = host->flags;
        isc_buf_p->info.host_tunnel_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_tunnel_route.dst_vid              = host->vid;
        isc_buf_p->info.host_tunnel_route.unit                 = host->unit;
        isc_buf_p->info.host_tunnel_route.port                 = host->port;
        isc_buf_p->info.host_tunnel_route.trunk_id             = host->trunk_id;
        isc_buf_p->info.host_tunnel_route.hw_info              = host->hw_info;
        memcpy (isc_buf_p->info.host_tunnel_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_tunnel_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        isc_buf_p->info.host_tunnel_route.tnl_init=host->tnl_init;
        isc_buf_p->info.host_tunnel_route.tnl_term = host->tnl_term;
        if (isc_buf_p->info.host_tunnel_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_tunnel_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_tunnel_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_host_tunnel_route, 0, sizeof(DEV_SWDRVL3_HostTunnel_T));
    dev_swdrvl3_host_tunnel_route.flags = host->flags;
    dev_swdrvl3_host_tunnel_route.fib_id = host->fib_id;
    dev_swdrvl3_host_tunnel_route.vid = host->vid;
    dev_swdrvl3_host_tunnel_route.unit = host->unit;
    dev_swdrvl3_host_tunnel_route.port = host->port;
    dev_swdrvl3_host_tunnel_route.trunk_id = host->trunk_id;
    dev_swdrvl3_host_tunnel_route.hw_info = host->hw_info;
    memcpy(dev_swdrvl3_host_tunnel_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_host_tunnel_route.mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    SWDRVL3_TUNNEL_INIT_TO_DEVDRV(dev_swdrvl3_host_tunnel_route.tnl_init, host->tnl_init);
    SWDRVL3_TUNNEL_TERM_TO_DEVDRV(dev_swdrvl3_host_tunnel_route.tnl_term, host->tnl_term);

    if (dev_swdrvl3_host_tunnel_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_tunnel_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_tunnel_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetHostTunnelRoute(&dev_swdrvl3_host_tunnel_route);
    host->hw_info = dev_swdrvl3_host_tunnel_route.hw_info;
    host->tnl_init.l3_intf_id = dev_swdrvl3_host_tunnel_route.tnl_init.l3_intf_id;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}




/*******************************************************************************
 * SWDRVL3_DeleteInetHostRoute
 *
 * Purpose: This function will delete a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteInetHostRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL) || (host == NULL))
        SWDRVL3_RETURN_WITH_VALUE(FALSE);


    SWDRVL3_EnterCriticalSection();
    shmem_data_p->host_deleted_counter++;
	SWDRVL3_LeaveCriticalSection();

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.dst_vid              = host->vid;
        isc_buf_p->info.host_route.unit                 = host->unit;
        isc_buf_p->info.host_route.port                 = host->port;
        isc_buf_p->info.host_route.trunk_id             = host->trunk_id;
        isc_buf_p->info.host_route.hw_info              = host->hw_info;
        memcpy (isc_buf_p->info.host_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.flags = host->flags;
    dev_swdrvl3_host_route.fib_id = host->fib_id;
    dev_swdrvl3_host_route.vid = host->vid;
    dev_swdrvl3_host_route.unit = host->unit;
    dev_swdrvl3_host_route.port = host->port;
    dev_swdrvl3_host_route.trunk_id = host->trunk_id;
    dev_swdrvl3_host_route.hw_info = host->hw_info;
    memcpy(dev_swdrvl3_host_route.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_host_route.mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetHostRoute(&dev_swdrvl3_host_route);
    host->hw_info = dev_swdrvl3_host_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_ClearHostRouteHWInfo
 *
 * Purpose: This function will clear HW information of the host.
 * Inputs:
 *          host->hw_info  : Host HW information
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    According to the design, the host HW information SHOULD BE cleared
 *          while deleting the Host Route entry.
 *******************************************************************************
 */
BOOL_T SWDRVL3_ClearHostRouteHWInfo(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = FALSE;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_ClearHostRouteHWInfo\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(FALSE);
    }

    if (host == NULL)
        SWDRVL3_RETURN_WITH_VALUE(FALSE);

    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
        SWDRVL3_RETURN_WITH_VALUE(TRUE);

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CLEAR_HOST_ROUTE_HW_INFO));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_CLEAR_HOST_ROUTE_HW_INFO;
        isc_buf_p->info.host_route.hw_info              = host->hw_info;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret = SWDRVL3_LocalClearHostRouteHWInfo(host);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}

/*******************************************************************************
 * SWDRVL3_AddTunnelInitiator
 *
 * Purpose: This function will add a tunnel initiator.
 * Inputs:
 *          tunnel : Tunnel Initiator information
 * Outputs: tunnel->l3_intf_id : L3 Interface Id associated with this tunnel 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelInitiator(SWDRVL3_TunnelInitiator_T *tunnel)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_TunnelInitiator_T dev_swdrvl3_tunnel_initiator;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n SWDRVL3_AddTunnelInitiator");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((tunnel->sip.addr == NULL) || (tunnel->dip.addr == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_TUNNEL_INITIATOR));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_TUNNEL_INITIATOR;
        isc_buf_p->info.tunnel_initiator.vid		      = tunnel->vid;
        isc_buf_p->info.tunnel_initiator.tunnel_type	      = tunnel->tunnel_type;
        isc_buf_p->info.tunnel_initiator.ttl		      = tunnel->ttl;
        memcpy (&(isc_buf_p->info.tunnel_initiator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
        memcpy (&(isc_buf_p->info.tunnel_initiator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));
        memcpy (isc_buf_p->info.tunnel_initiator.src_mac, tunnel->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.tunnel_initiator.nexthop_mac, tunnel->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
 
        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_tunnel_initiator, 0, sizeof(DEV_SWDRVL3_TunnelInitiator_T));

    dev_swdrvl3_tunnel_initiator.vid		      = tunnel->vid;
    dev_swdrvl3_tunnel_initiator.tunnel_type	      = tunnel->tunnel_type;
    dev_swdrvl3_tunnel_initiator.ttl		      = tunnel->ttl;
    memcpy (&(dev_swdrvl3_tunnel_initiator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
    memcpy (&(dev_swdrvl3_tunnel_initiator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));
    memcpy (dev_swdrvl3_tunnel_initiator.src_mac, tunnel->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (dev_swdrvl3_tunnel_initiator.nexthop_mac, tunnel->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);

    ret = DEV_SWDRVL3_PMGR_AddTunnelInitiator(&dev_swdrvl3_tunnel_initiator);
    tunnel->l3_intf_id = dev_swdrvl3_tunnel_initiator.l3_intf_id;

    SWDRVL3_RETURN_WITH_VALUE(ret);

}

/*******************************************************************************
 * SWDRVL3_AddTunnelIntfL3
 *
 * Purpose: This function will add/del a tunnel l3 interface.
 * Inputs:
 *          tl3_p  : Tunnel Initiator information
 *          tl3_p->l3_intf_id for del
 *          tl3_p->vid        for add
 *          tl3_p->src_mac    for add
 *          is_add : TRUE if it's to add
 * Outputs: tl3_p->l3_intf_id : L3 Interface Id for add
 * Return:  SWDRVL3_L3_NO_ERROR/SWDRVL3_L3_OTHERS
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelIntfL3(
    SWDRVL3_TunnelIntfL3_T *tl3_p)
{
#if (SYS_CPNT_VXLAN == TRUE)
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_TunnelIntfL3_T dsl3_tl3;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n SWDRVL3_AddTunnelInitiator");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

     /* new code for ISC Send Multicast Reliable    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T),
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_TUNNEL_INTF_L3));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_TUNNEL_INTF_L3;
        isc_buf_p->info.tl3  = *tl3_p;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    memset(&dsl3_tl3, 0, sizeof(dsl3_tl3));

    dsl3_tl3.vid	    = tl3_p->vid;
    dsl3_tl3.l3_intf_id	= tl3_p->l3_intf_id;
    memcpy(dsl3_tl3.src_mac, tl3_p->src_mac, sizeof(dsl3_tl3.src_mac));

    ret = DEV_SWDRVL3_PMGR_AddTunnelIntfL3(&dsl3_tl3, tl3_p->is_add);

    tl3_p->l3_intf_id = dsl3_tl3.l3_intf_id;

    SWDRVL3_RETURN_WITH_VALUE(ret);
#else

    return SWDRVL3_L3_OTHERS;

#endif
}

/*******************************************************************************
 * SWDRVL3_DeleteTunnelInitiator
 *
 * Purpose: This function will delete a tunnel initiator.
 * Inputs:
 *          l3_intf_id: The L3 Interface ID associated with the tunnel initiator 
 * Outputs: None 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_DeleteTunnelInitiator(UI32_T l3_intf_id)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n SWDRVL3_DeleteTunnelInitiator");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if (l3_intf_id == 0)
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_TUNNEL_INITIATOR));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_TUNNEL_INITIATOR;
        isc_buf_p->info.interface_num			      = l3_intf_id;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */

    ret = DEV_SWDRVL3_PMGR_DeleteTunnelInitiator(l3_intf_id);

    SWDRVL3_RETURN_WITH_VALUE(ret);


}

/*******************************************************************************
 * SWDRVL3_AddTunnelTerminator
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : Tunnel Terminator information
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_TunnelTerminator_T dev_swdrvl3_tunnel_terminator;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n SWDRVL3_AddTunnelTerminator");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((tunnel->sip.addr == NULL) || (tunnel->dip.addr == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_TUNNEL_TERMINATOR));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_TUNNEL_TERMINATOR;
        isc_buf_p->info.tunnel_terminator.fib_id	      = tunnel->fib_id;
        isc_buf_p->info.tunnel_terminator.tunnel_type	      = tunnel->tunnel_type;
        memcpy (isc_buf_p->info.tunnel_terminator.lport, tunnel->lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memcpy (&(isc_buf_p->info.tunnel_terminator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
        memcpy (&(isc_buf_p->info.tunnel_terminator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));
 
        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_tunnel_terminator, 0, sizeof(DEV_SWDRVL3_TunnelTerminator_T));

    dev_swdrvl3_tunnel_terminator.fib_id		= tunnel->fib_id;
    dev_swdrvl3_tunnel_terminator.tunnel_type	        = tunnel->tunnel_type;
    memcpy (dev_swdrvl3_tunnel_terminator.lport, tunnel->lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy (&(dev_swdrvl3_tunnel_terminator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
    memcpy (&(dev_swdrvl3_tunnel_terminator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));

    ret = DEV_SWDRVL3_PMGR_AddTunnelTerminator(&dev_swdrvl3_tunnel_terminator);

    SWDRVL3_RETURN_WITH_VALUE(ret);


}

/*******************************************************************************
 * SWDRVL3_DeleteTunnelTerminator
 *
 * Purpose: This function will delete a tunnel terminator.
 * Inputs:
 *          tunnel : Tunnel Terminator information
 * Outputs: None 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_DeleteTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_TunnelTerminator_T dev_swdrvl3_tunnel_terminator;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n SWDRVL3_DeleteTunnelTerminator");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((tunnel->sip.addr == NULL) || (tunnel->dip.addr == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_TUNNEL_TERMINATOR));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_TUNNEL_TERMINATOR;
        isc_buf_p->info.tunnel_terminator.fib_id	      = tunnel->fib_id;
        isc_buf_p->info.tunnel_terminator.tunnel_type	      = tunnel->tunnel_type;
        memcpy (isc_buf_p->info.tunnel_terminator.lport, tunnel->lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memcpy (&(isc_buf_p->info.tunnel_terminator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
        memcpy (&(isc_buf_p->info.tunnel_terminator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));
 
        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_tunnel_terminator, 0, sizeof(DEV_SWDRVL3_TunnelTerminator_T));

    dev_swdrvl3_tunnel_terminator.fib_id		= tunnel->fib_id;
    dev_swdrvl3_tunnel_terminator.tunnel_type	        = tunnel->tunnel_type;
    memcpy (dev_swdrvl3_tunnel_terminator.lport, tunnel->lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy (&(dev_swdrvl3_tunnel_terminator.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
    memcpy (&(dev_swdrvl3_tunnel_terminator.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));

    ret = DEV_SWDRVL3_PMGR_DeleteTunnelTerminator(&dev_swdrvl3_tunnel_terminator);

    SWDRVL3_RETURN_WITH_VALUE(ret);

}

/*******************************************************************************
 * SWDRVL3_AddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddInetNetRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);


    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_NET_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_NET_ROUTE;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_hw_info[0]        = nh_hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(isc_buf_p->info.net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
            isc_buf_p->info.net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(dev_swdrvl3_net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        dev_swdrvl3_net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_AddInetNetRoute(&dev_swdrvl3_net_route, nh_hw_info);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddInetNetRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_NET_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_NET_ROUTE;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_hw_info[0]        = nh_hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(isc_buf_p->info.net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
            isc_buf_p->info.net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_NO_ERROR);
}


/*******************************************************************************
 * SWDRVL3_DeleteInetNetRoute
 *
 * Purpose: This function will delete a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : The HW information of the nexthop
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteInetNetRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_deleted_counter++;
	SWDRVL3_LeaveCriticalSection();

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_NET_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_NET_ROUTE;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_hw_info[0]        = nh_hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(isc_buf_p->info.net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
            isc_buf_p->info.net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(dev_swdrvl3_net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        dev_swdrvl3_net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetNetRoute(&dev_swdrvl3_net_route, nh_hw_info);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_ClearNetRouteHWInfo
 *
 * Purpose: This function will clear HW information of the route.
 * Inputs:
 *          route->hw_info  : Route HW information
 * Outputs: 
 *          route->hw_info
 * Return:  TRUE/FALSE
 * Note:    1. According to the design, the host HW information SHOULD BE cleared
 *             while deleting the NEXT HOP Host Route entry.
 *          2. For ECMP Route, the HW information SHOULD be cleared while deleting
 *             the ECMP Route.
 *******************************************************************************
 */
BOOL_T SWDRVL3_ClearNetRouteHWInfo(SWDRVL3_Route_T *route)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = FALSE;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_ClearNetRouteHWInfo\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(FALSE);
    }

    if (route == NULL)
        SWDRVL3_RETURN_WITH_VALUE(FALSE);

    if (route->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
        SWDRVL3_RETURN_WITH_VALUE(TRUE);

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_CLEAR_NET_ROUTE_HW_INFO));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_CLEAR_NET_ROUTE_HW_INFO;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret = SWDRVL3_LocalClearNetRouteHWInfo(route);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_SetSpecialDefaultRoute
 *
 * Purpose:  This function will add an default route entry for special purpose.
 * Inputs:
 *          default_route  : Default Route entry information
 *          action : Indicate the special purpose of this Default Route entry
 *              The "action" are:
 *                  SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *                  SWDRVL3_ACTION_DROP Drop packet if match
 *
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    The implementation will not accept "SWDRVL3_ACTION_ROUTE".
 *******************************************************************************
 */
BOOL_T SWDRVL3_SetSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_SetSpecialDefaultRoute\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((default_route == NULL) || (action == SWDRVL3_ACTION_ROUTE))
        return FALSE;

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_SET_SPECIAL_DEFAULT_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_SET_SPECIAL_DEFAULT_ROUTE;
        isc_buf_p->info.net_route.action               = action;
        isc_buf_p->info.net_route.flags                = default_route->flags;
        isc_buf_p->info.net_route.fib_id               = default_route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = default_route->dst_vid;
        isc_buf_p->info.net_route.unit                 = default_route->unit;
        isc_buf_p->info.net_route.port                 = default_route->port;
        isc_buf_p->info.net_route.trunk_id             = default_route->trunk_id;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret=SWDRVL3_LocalSetSpecialDefaultRoute(default_route, action);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_DeleteSpecialDefaultRoute
 *
 * Purpose:  This function will delete the special default route entry.
 * Inputs:
 *          default_route  : Default Route entry information
 *          action : Indicate the special purpose of this Default Route entry
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteSpecialDefaultRoute\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((default_route == NULL) || (action == SWDRVL3_ACTION_ROUTE))
        return FALSE;

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_SPECIAL_DEFAULT_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_SPECIAL_DEFAULT_ROUTE;
        isc_buf_p->info.net_route.action               = action;
        isc_buf_p->info.net_route.flags                = default_route->flags;
        isc_buf_p->info.net_route.fib_id               = default_route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = default_route->dst_vid;
        isc_buf_p->info.net_route.unit                 = default_route->unit;
        isc_buf_p->info.net_route.port                 = default_route->port;
        isc_buf_p->info.net_route.trunk_id             = default_route->trunk_id;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret=SWDRVL3_LocalDeleteSpecialDefaultRoute(default_route, action);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 *  SWDRVL3_ReadAndClearHostRouteHitBit
 *
 * Purpose: Read and Clear hit bit of a host entry.
 * Inputs:
 *          host  : Host entry information
 * Outputs: 
 *          hit : The read Hit bit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_ReadAndClearHostRouteEntryHitBit(SWDRVL3_Host_T *host, UI32_T *hit)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = FALSE;
    UI32_T              hit_bit = 0;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_ReadAndClearHostRouteEntryHitBit\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL) ||
        (host == NULL) || (hit == NULL))
        return FALSE;

    *hit = 0;
    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.hit                  = 0;
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret = SWDRVL3_LocalReadAndClearHostRouteEntryHitBit(host, &hit_bit);
    *hit |= hit_bit;
    SWDRVL3_RETURN_WITH_VALUE(ret);
}



/*******************************************************************************
 *  SWDRVL3_ReadAndClearNetRouteHitBit
 *
 * Purpose: Read and Clear hit bit of a net entry.
 * Inputs:
 *          net_route  : net entry information
 * Outputs: 
 *          hit : The read Hit bit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_ReadAndClearNetRouteHitBit(SWDRVL3_Route_T *net_route, UI32_T *hit)
{
#if 0
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
#endif    
    UI32_T              ret = FALSE;
    UI32_T              hit_bit = 0;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_ReadAndClearTunnelNetRouteHitBit\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    /* anything else need to check here ?*/
    if ((net_route->nexthop_mac == NULL) || (net_route->src_mac == NULL) ||
        (net_route == NULL) || (hit == NULL))
        return FALSE;
 
    *hit = 0;

#if 0     /* we don't send ISC to slave units */
    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
   
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_READ_AND_CLEAR_NET_ROUTE_HIT_BIT;
        isc_buf_p->info.net_route.flags                = net_route->flags;
        isc_buf_p->info.net_route.fib_id               = net_route->fib_id;
        isc_buf_p->info.net_route.prefix_length        = net_route->prefix_length;
        isc_buf_p->info.net_route.hit                  = 0;
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, net_route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = net_route->dst_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }
#endif    
#if 0
    /* temp print net route information here */
    printf("%s[%d]:net route inforamtion\r\n",__FUNCTION__,__LINE__);
    printf("Fib ID:%lu\r\n",net_route->fib_id);
    printf("Flags: 0x%lx\r\n",net_route->flags);
    printf("Destination:%lx:%lx:%lx:%lx/%lu\r\n",
        ((UI32_T *)(net_route->dst_ip.ipv6_addr))[0],
        ((UI32_T *)(net_route->dst_ip.ipv6_addr))[1],
        ((UI32_T *)(net_route->dst_ip.ipv6_addr))[2],
        ((UI32_T *)(net_route->dst_ip.ipv6_addr))[3],
        net_route->prefix_length);
    /* end */
#endif    
    ret = SWDRVL3_LocalReadAndClearNetRouteEntryHitBit(net_route, &hit_bit);
    (*hit) |= hit_bit;
    /*printf("hit=%lu,hit_bit=%lu\r\n",*hit,hit_bit);*/
    SWDRVL3_RETURN_WITH_VALUE(ret);
}



/*******************************************************************************
 * SWDRVL3_AddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetMyIpHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddInetMyIpHostRoute\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.dst_vid              = host->vid;
        memcpy (isc_buf_p->info.host_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret = SWDRVL3_LocalAddInetMyIpHostRoute(host);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetMyIpHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddInetMyIpHostRoute\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        isc_buf_p->info.host_route.dst_vid              = host->vid;
        memcpy (isc_buf_p->info.host_route.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_NO_ERROR);
}


/*******************************************************************************
 * SWDRVL3_DeleteInetMyIpHostRoute
 *
 * Purpose: This function will delete a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetMyIpHostRoute(SWDRVL3_Host_T *host)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddInetMyIpHostRoute");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_MY_IP_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_MY_IP_HOST_ROUTE;
        isc_buf_p->info.host_route.flags                = host->flags;
        isc_buf_p->info.host_route.fib_id               = host->fib_id;
        if (isc_buf_p->info.host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(FALSE);
        }
    }

    if (SWDRVL3_L3_NO_ERROR != SWDRVL3_LocalDeleteInetMyIpHostRoute(host))
        SWDRVL3_RETURN_WITH_VALUE(FALSE);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_AddInetECMPRouteOnePath
 *
 * Purpose: This function will add a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 *          is_first
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If is_first is TRUE, this function will create a multipath nexthop
 *             handler according to the input nh_hw_info and set to route->hw_info.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddInetECMPRouteOnePath\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info == NULL))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

    if (is_first == TRUE)
    {
        SWDRVL3_EnterCriticalSection();
        shmem_data_p->net_added_counter++;
    	SWDRVL3_LeaveCriticalSection();
    }

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_ECMP_ROUTE_ONE_PATH));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_ECMP_ROUTE_ONE_PATH;
        isc_buf_p->info.net_route.flags                = route->flags;
        if (is_first == TRUE)
            isc_buf_p->info.net_route.flags           |= DEV_SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_hw_info[0]        = nh_hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(isc_buf_p->info.net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
            isc_buf_p->info.net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    ret = SWDRVL3_LocalAddInetECMPRouteOnePath(route, nh_hw_info, is_first);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_DeleteInetECMPRouteOnePath
 *
 * Purpose: This function will add or update a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 *          is_last
 * Outputs: None
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteInetECMPRouteOnePath\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info == NULL))
        return FALSE;

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_ECMP_ROUTE_ONE_PATH));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_ECMP_ROUTE_ONE_PATH;
        isc_buf_p->info.net_route.flags                = route->flags;
        if (is_last == TRUE)
            isc_buf_p->info.net_route.flags           |= DEV_SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_hw_info[0]        = nh_hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(isc_buf_p->info.net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
            isc_buf_p->info.net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret = SWDRVL3_LocalDeleteInetECMPRouteOnePath(route, nh_hw_info, is_last);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_AddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Number of nexthops
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_AddInetECMPRouteMultiPath\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info_array == NULL) ||
        (count > SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);


    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_count             = count;
        memcpy(isc_buf_p->info.net_route.nh_hw_info, nh_hw_info_array, (sizeof(void*) * count));
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_host_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_AddInetECMPRouteMultiPath(&dev_swdrvl3_net_route, nh_hw_info_array, count);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop to all slaves.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Number of nexthops
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;
    /*DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;*/

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddInetECMPRouteMultiPath\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL) || (nh_hw_info_array == NULL) ||
        (count > SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE))
        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        isc_buf_p->info.net_route.nh_count             = count;
        memcpy(isc_buf_p->info.net_route.nh_hw_info, nh_hw_info_array, (sizeof(void*) * count));
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_DeleteInetECMPRoute
 *
 * Purpose: This function will delete an ECMP route.
 * Inputs:
 *          route  : Route entry information
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None.
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetECMPRoute(SWDRVL3_Route_T *route)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = TRUE;
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;
    

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DeleteInetECMPRoute\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((route->src_mac == NULL) || (route->nexthop_mac == NULL) || 
        (route == NULL))
        return FALSE;


    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_deleted_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
            L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DELETE_INET_ECMP_ROUTE_MULTIPATH));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID = SWDRVL3_DELETE_INET_ECMP_ROUTE_MULTIPATH;
        isc_buf_p->info.net_route.flags                = route->flags;
        isc_buf_p->info.net_route.fib_id               = route->fib_id;
        isc_buf_p->info.net_route.dest_vid             = route->dst_vid;
        isc_buf_p->info.net_route.unit                 = route->unit;
        isc_buf_p->info.net_route.port                 = route->port;
        isc_buf_p->info.net_route.trunk_id             = route->trunk_id;
        isc_buf_p->info.net_route.prefix_length        = route->prefix_length;
        isc_buf_p->info.net_route.hw_info              = route->hw_info;
        memcpy (isc_buf_p->info.net_route.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            isc_buf_p->info.net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        }

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    /* amytu add 2004-08-06
     * Modification: Master unit shall not depends on L_HASH to complete add_route action
     * because AMTRL3 needs immediate reply reflecting actual operation result. Slave unit can
     * still uses L_HASH mechanism to complete write to driver operation by SWDRVL3 Task.
     */
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetECMPRoute(&dev_swdrvl3_net_route);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


/*******************************************************************************
 * SWDRVL3_EnableRouting
 *
 * Purpose: This function enables the L3 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T SWDRVL3_EnableRouting(void)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_EnableRouting\n");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* new code for ISC Send Multicast Reliable
     */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ENABLE_ROUTING));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_ENABLE_ROUTING;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalEnableRouting();
    return ret;
} /* SWDRVL3_EnableRouting() */


/*******************************************************************************
 * SWDRVL3_EnableUnicastStormProtect
 *
 * Purpose: to prevent the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T SWDRVL3_EnableUnicastStormProtect(UI32_T unit, UI32_T port)
{
    BOOL_T ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_EnableUnicastStormProtect");
    }


    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    ret=DEV_SWDRVL3_PMGR_EnableUnicastStormProtect(unit, port);
    return ret;
} /* SWDRVL3_EnableUnicastStormProtect() */


/*******************************************************************************
 * SWDRVL3_DisableUnicastStormProtect
 * Purpose: to allow the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T SWDRVL3_DisableUnicastStormProtect(UI32_T unit, UI32_T port)
{
    BOOL_T ret;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DisableUnicastStormProtect");
    }


    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    ret=DEV_SWDRVL3_PMGR_DisableUnicastStormProtect(unit, port);
    return ret;
} /* SWDRVL3_DisableUnicastStormProtect() */


/*******************************************************************************
 * SWDRVL3_DisableRouting
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T SWDRVL3_DisableRouting(void)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_DisableRouting");
    }


    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
       return FALSE;
    }

   /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index))
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_DISABLE_ROUTING));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL3_DISABLE_ROUTING;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return FALSE;
        }
    }

    ret=SWDRVL3_LocalDisableRouting();
    SWDRVL3_RETURN_WITH_VALUE(ret);
} /* SWDRVL3_DisableRouting() */


#if 0
/*******************************************************************************
 *  SWDRVL3_HotInsertAddHostRouteToModule
 *
 * Purpose: Write unicast host entry to Optional Module (for Hot Insert used)
 * Inputs:
 *   dst_ip           - destination IP.
 *   dst_mac          - destination MAC.
 *   src_mac          - source MAC (router MAC)
 *   dst_vid          - destination Vid (used for tagged egress port)
 *   unit             - unit number
 *   port             - port number
 *   trunk_id         - trunk group id
 *   is_trunk         - indicate whether the egress port is trunking port
 *   is_static        - static entry
 *   tagged_frame     - ture if tagged frame.
 * Outputs: None
 * Return:
 *   SWDRVL3_L3_BUCKET_FULL    1
 *   SWDRVL3_L3_NO_ERROR       2
 *   SWDRVL3_L3_OTHERS         3
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddHostRouteToModule(UI32_T    dst_ip,
                              UI8_T     *dst_mac,
                              UI8_T     *src_mac,
                              UI32_T    dst_vid,
                              UI32_T    unit,
                              UI32_T    port,
                              UI32_T    trunk_id,
                              BOOL_T    is_trunk,
                              BOOL_T    is_static,
                              BOOL_T    tagged_frame)
{
#if (SYS_CPNT_SWDRVL3_CACHE == TRUE)
    BOOL_T hit_bit_on = TRUE;
#endif
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, drv_unit, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    UI32_T              ret = SWDRVL3_L3_NO_ERROR;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddHostRouteToModule");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Error checking */
    if ((dst_mac == 0) || (src_mac == 0))
       SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

	SWDRVL3_EnterCriticalSection();
    shmem_data_p->host_added_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* Just Add Entry to Module Board, Not to MainBoard.
     */
    /* new code for ISC Send Multicast Reliable
     */
    for (unit_index = 0; unit_index < shmem_data_p->swdrvl3_stack_info.num_of_units; unit_index++)
    {
        if( STKTPLG_POM_OptionModuleIsExist(shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl[unit_index],
                                           &drv_unit) == TRUE )
            dst_bmp |= BIT_VALUE(drv_unit-1);
        else
            continue;
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_HOST_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }

        isc_buf_p->ServiceID                      = SWDRVL3_ADD_HOST_ROUTE;
        isc_buf_p->info.host_route.dst_ip         = dst_ip;
        isc_buf_p->info.host_route.dst_vid        = dst_vid;
        isc_buf_p->info.host_route.unit           = unit;
        isc_buf_p->info.host_route.port           = port;
        isc_buf_p->info.host_route.trunk_id       = trunk_id;
        isc_buf_p->info.host_route.is_trunk       = is_trunk;
        isc_buf_p->info.host_route.is_static      = is_static;
        isc_buf_p->info.host_route.tagged_frame   = tagged_frame;
        memcpy (isc_buf_p->info.host_route.dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (isc_buf_p->info.host_route.src_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

#if (SYS_CPNT_SWDRVL3_CACHE == TRUE)
    ret = SWDRVL3_CACHE_AddHostRouteEntry(dst_ip, dst_mac, src_mac, dst_vid, unit, port, trunk_id, is_trunk, is_static, tagged_frame, hit_bit_on);
    if(ret==FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */
        SYSFUN_Debug_Printf("\r\n"__FUNCTION__"():SWDRVL3_CACHE_AddHostRouteEntry");
    }
#endif
    SWDRVL3_RETURN_WITH_VALUE(ret);
}

/*******************************************************************************
 *  SWDRVL3_HotInsertAddNetRouteToModule
 *
 * Purpose: Set IP gateway entry to Optional Module (for Hot Insert used).
 * Inputs:
 *   dest_ip            - destination ip addr.
 *   prefix_length      - subnet mask length
 *   next_hop_mac_addr  - destination MAC,
 *   next_host_gateway_ip - the ip of the next hop gateway
 *   src_mac            - source MAC (router MAC)
 *   dst_vid            - destination VID (used for tagged egress port)
 *   unit               - unit number
 *   port               - port number
 *   trunk_id           - tunk group id
 *   is_trunk           - indicate whether the egress port is trunking port
 *   is_local_connected - indicate whether the route is direct connected
 *   tagged_frame       - ture if tagged frame.
 * Outputs: None
 * Return:
 *   SWDRVL3_L3_BUCKET_FULL    1
 *   SWDRVL3_L3_NO_ERROR       2
 *   SWDRVL3_L3_OTHERS         3
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddNetRouteToModule(UI32_T   dest_ip,
                           UI32_T   prefix_length,
                           UI32_T   next_hop_gateway_ip,
                           UI8_T    *next_hop_mac_addr,
                           UI8_T    *src_mac,
                           UI32_T   dst_vid,
                           UI32_T   unit,
                           UI32_T   port,
                           UI32_T   trunk_id,
                           BOOL_T   is_trunk,
                           BOOL_T   is_local_connected,
                           BOOL_T   tagged_frame)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, drv_unit, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_HotInsertAddNetRouteToModule");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((next_hop_mac_addr == 0) || (src_mac == 0))
       SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);

	SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* Just Add Entry to Module Board, Not to MainBoard.
     */
    /* new code for ISC Send Multicast Reliable
     */
    for (unit_index = 0; unit_index < shmem_data_p->swdrvl3_stack_info.num_of_units; unit_index++)
    {
        if( STKTPLG_POM_OptionModuleIsExist(shmem_data_p->swdrvl3_stack_info.stack_unit_id_tbl[unit_index],
                                           &drv_unit) == TRUE )
            dst_bmp |= BIT_VALUE(drv_unit-1);
        else
            continue;
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_ADD_NET_ROUTE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }


        isc_buf_p->ServiceID           = SWDRVL3_ADD_NET_ROUTE;
        isc_buf_p->info.net_route.dest_ip         = dest_ip;
        isc_buf_p->info.net_route.prefix_length   = prefix_length;
        isc_buf_p->info.net_route.next_hop_gateway_ip   = next_hop_gateway_ip;
        isc_buf_p->info.net_route.egress_unit     = unit;
        isc_buf_p->info.net_route.egress_port     = port;
        isc_buf_p->info.net_route.is_local_connected  = is_local_connected;
        isc_buf_p->info.net_route.dest_vid        = dst_vid;
        isc_buf_p->info.net_route.trunk_id        = trunk_id;
        isc_buf_p->info.net_route.is_trunk        = is_trunk;
        isc_buf_p->info.net_route.tagged_frame    = tagged_frame;
        memcpy(isc_buf_p->info.net_route.src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(isc_buf_p->info.net_route.next_hop_mac, next_hop_mac_addr, SYS_ADPT_MAC_ADDR_LEN);

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
        }
    }

    SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_NO_ERROR);
} /* SWDRVL3_HotInsertAddNetRouteToModule */
#endif

/* LOCAL SUBPROGRAM BODIES
 */

static BOOL_T SWDRVL3_LocalEnableIpForwarding(UI32_T flag, UI32_T vr_id)
{
    UI32_T enable_flag = 1;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_EnableRouting");
    }

    if(flag == SWDRVL3_FLAG_IPV4)
    {
	    if(DEV_SWDRVL3_PMGR_EnableIPv4Routing(enable_flag, vr_id) == FALSE)
	        return FALSE;
    }
    else if(flag == SWDRVL3_FLAG_IPV6)
    {
	    if(DEV_SWDRVL3_PMGR_EnableIPv6Routing(enable_flag, vr_id) == FALSE)
	        return FALSE;
    }
    else
	return FALSE;

    return TRUE;
}

static BOOL_T SWDRVL3_LocalDisableIpForwarding(UI32_T flag, UI32_T vr_id)
{
    UI32_T disable_flag = 0;


    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_DisableRouting");
    }

     if(flag == SWDRVL3_FLAG_IPV4)
    {
	    if(DEV_SWDRVL3_PMGR_DisableIPv4Routing(disable_flag, vr_id) == FALSE)
	        return FALSE;
    }
    else if(flag == SWDRVL3_FLAG_IPV6)
    {
	    if(DEV_SWDRVL3_PMGR_DisableIPv6Routing(disable_flag, vr_id) == FALSE)
	        return FALSE;
    }
    else
	return FALSE;

    return TRUE;
}

static BOOL_T SWDRVL3_LocalEnableRouting(void)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_EnableRouting");
    }

    if(DEV_SWDRVL3_PMGR_EnableRouting(0, 0) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T SWDRVL3_LocalDisableRouting(void)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_DisableRouting");
    }

    if(DEV_SWDRVL3_PMGR_DisableRouting(0, 0) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

static UI32_T SWDRVL3_LocalCreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T *hw_info)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_CreateL3Interface");
    }

    return DEV_SWDRVL3_PMGR_CreateL3Interface(fib_id, vid, vlan_mac, hw_info);
}

static BOOL_T SWDRVL3_LocalDeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_DeleteL3Interface");
    }

    if (DEV_SWDRVL3_PMGR_DeleteL3Interface(fib_id, vid, vlan_mac, hw_info) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

static UI32_T SWDRVL3_LocalAddL3Mac(UI8_T *vlan_mac, UI32_T vid)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_AddL3Mac");
    }

    return DEV_SWDRVL3_PMGR_AddL3Mac(vid, vlan_mac);
}
        
static BOOL_T SWDRVL3_LocalDeleteL3Mac(UI8_T *vlan_mac, UI32_T vid)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteL3Mac");
    }

    if (DEV_SWDRVL3_PMGR_DeleteL3Mac(vid, vlan_mac) == TRUE)
    {
        return TRUE;
    }
    
    return FALSE;
}

static BOOL_T SWDRVL3_LocalSetL3Bit(UI8_T *vlan_mac, UI32_T vid)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_SetL3Bit");
    }

    return DEV_SWDRVL3_PMGR_SetL3Bit(vid, vlan_mac);
}
        
static BOOL_T SWDRVL3_LocalUnSetL3Bit(UI8_T *vlan_mac, UI32_T vid)
{
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_UnSetL3Bit");
    }

    if (DEV_SWDRVL3_PMGR_UnSetL3Bit(vid, vlan_mac) == TRUE)
    {
        return TRUE;
    }
    
    return FALSE;
}

static BOOL_T SWDRVL3_LocalSetInetHostRoute(SWDRVL3_Host_T *host)
{
    BOOL_T       ret = TRUE;
    HostRoute_T  host_record;
    L_HASH_Index_T record_index;

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        return FALSE;

    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
    {
        SWDRVL3_EnterCriticalSection();
        shmem_data_p->host_added_counter++;
    	SWDRVL3_LeaveCriticalSection();
    }

    /* Initialize net_record structure */
    memset(&host_record, 0, sizeof(HostRoute_T));

    /* Fill the parameters into the host route record */
    host_record.flags = host->flags;
    host_record.fib_id = host->fib_id;
    memcpy (host_record.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (host_record.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    host_record.dst_vid = host->vid;
    host_record.unit = host->unit;
    host_record.port = host->port;
    host_record.trunk_id = host->trunk_id;
    host_record.hw_info = host->hw_info;
    if (host_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host_record.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host_record.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_SetRecord(&shmem_data_p->SWDRVL3_host_route_hash_desc, (UI8_T*)&host_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_SetRecord in SWDRVL3_LocalSetInetHostRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for AddHostRoute!!");
       return FALSE;
    }

    return TRUE;
}

static BOOL_T SWDRVL3_LocalDeleteInetHostRoute(SWDRVL3_Host_T *host)
{
    BOOL_T       ret = TRUE;
    HostRoute_T  host_record;
    L_HASH_Index_T record_index;

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->host_deleted_counter++;
	SWDRVL3_LeaveCriticalSection();
    
    /* Initialize net_record structure */
    memset(&host_record, 0, sizeof(HostRoute_T));

    host_record.flags = host->flags;
    host_record.fib_id = host->fib_id;
    memcpy (host_record.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (host_record.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    host_record.dst_vid = host->vid;
    host_record.unit = host->unit;
    host_record.port = host->port;
    host_record.trunk_id = host->trunk_id;
    host_record.hw_info = host->hw_info;
    if (host_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host_record.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host_record.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_DeleteRecord(&shmem_data_p->SWDRVL3_host_route_hash_desc, (UI8_T*)&host_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_DeleteRecord in SWDRVL3_LocalDeleteInetHostRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger deleting host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for DeleteHostRoute!!");
       return FALSE;
    }

    return TRUE;
}

static BOOL_T SWDRVL3_LocalAddTunnelInitiator(SWDRVL3_TunnelInitiator_T *tunnel)
{
    BOOL_T       ret = TRUE;
    TunnelInitiator_T  tunnel_record;
    L_HASH_Index_T record_index;

    /* Error checking */
    if ((tunnel->sip.addr == NULL) || (tunnel->dip.addr == NULL))
        return FALSE;

    /* Initialize net_record structure */
    memset(&tunnel_record, 0, sizeof(TunnelInitiator_T));

    /* Fill the parameters into the host route record */
    tunnel_record.vid		= tunnel->vid;
    tunnel_record.tunnel_type   = tunnel->tunnel_type;
    tunnel_record.ttl 		= tunnel->ttl;

    memcpy (tunnel_record.src_mac, tunnel->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (tunnel_record.nexthop_mac, tunnel->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (&(tunnel_record.sip), &(tunnel->sip), sizeof(L_INET_AddrIp_T));
    memcpy (&(tunnel_record.dip), &(tunnel->dip), sizeof(L_INET_AddrIp_T));


    SWDRVL3_EnterCriticalSection();

    /* Set the tunnel initiator record into OM */
    ret = L_HASH_ShMem_SetRecord(&shmem_data_p->SWDRVL3_tunnel_initiator_hash_desc, (UI8_T*)&tunnel_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_SetRecord in SWDRVL3_LocalAddTunnelInitiator Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for AddTunnelInitiator!!");
       return FALSE;
    }

    return TRUE;

}

static BOOL_T SWDRVL3_LocalDeleteTunnelInitiator(UI32_T l3_intf_id)
{return TRUE;}

static BOOL_T SWDRVL3_LocalAddTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
{return TRUE;}

static BOOL_T SWDRVL3_LocalDeleteTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
{return TRUE;}

static BOOL_T SWDRVL3_LocalTunnelUpdateTtl(UI32_T l3_intf_id, UI8_T ttl)
{
    DEV_SWDRVL3_TunnelInitiator_T dev_tunnel_initiator;
    UI32_T ret = SWDRVL3_L3_NO_ERROR;
    
    memset(&dev_tunnel_initiator, 0, sizeof(dev_tunnel_initiator));
    dev_tunnel_initiator.l3_intf_id = l3_intf_id;
    dev_tunnel_initiator.ttl = ttl;
    ret = DEV_SWDRVL3_PMGR_UpdateTunnelTtl(&dev_tunnel_initiator);
    SWDRVL3_RETURN_WITH_VALUE(ret);
}

static BOOL_T SWDRVL3_LocalAddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host)
{
    BOOL_T       ret = TRUE;
    HostTunnelRoute_T  host_record;
    L_HASH_Index_T record_index;

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        return FALSE;

//    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
//    {
//        SWDRVL3_EnterCriticalSection();
//        shmem_data_p->host_added_counter++;
//    	SWDRVL3_LeaveCriticalSection();
//    }

    /* Initialize net_record structure */
    memset(&host_record, 0, sizeof(HostTunnelRoute_T));

    /* Fill the parameters into the host route record */
    host_record.flags = host->flags;
    host_record.fib_id = host->fib_id;
    memcpy (host_record.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (host_record.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    host_record.dst_vid = host->vid;
    host_record.unit = host->unit;
    host_record.port = host->port;
    host_record.trunk_id = host->trunk_id;
    host_record.hw_info = host->hw_info;
    host_record.hit = 0;    
    if (host_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host_record.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host_record.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    memcpy (&(host_record.tnl_init), &(host->tnl_init), sizeof(TunnelInitiator_T));
    memcpy (&(host_record.tnl_term), &(host->tnl_term), sizeof(TunnelTerminator_T));


    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_SetRecord(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, (UI8_T*)&host_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_SetRecord in SWDRVL3_LocalAddInetHostTunnelRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for AddHostRoute!!");
       return FALSE;
    }

    return TRUE;

}

static BOOL_T SWDRVL3_LocalDeleteInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host)
{
    BOOL_T       ret = TRUE;
    HostTunnelRoute_T  host_record;
    L_HASH_Index_T record_index;

    /* Error checking */
    if ((host->mac == NULL) || (host->src_mac == NULL))
        return FALSE;

//    if (host->hw_info == (void *)SWDRVL3_HW_INFO_INVALID)
//    {
//        SWDRVL3_EnterCriticalSection();
//        shmem_data_p->host_added_counter++;
//    	SWDRVL3_LeaveCriticalSection();
//    }

    /* Initialize net_record structure */
    memset(&host_record, 0, sizeof(HostTunnelRoute_T));

    /* Fill the parameters into the host route record */
    host_record.flags = host->flags;
    host_record.fib_id = host->fib_id;
    memcpy (host_record.dst_mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (host_record.src_mac, host->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    host_record.dst_vid = host->vid;
    host_record.unit = host->unit;
    host_record.port = host->port;
    host_record.trunk_id = host->trunk_id;
    host_record.hw_info = host->hw_info;
    if (host_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host_record.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host_record.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    memcpy (&(host_record.tnl_init), &(host->tnl_init), sizeof(TunnelInitiator_T));
    memcpy (&(host_record.tnl_term), &(host->tnl_term), sizeof(TunnelTerminator_T));


    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_DeleteRecord(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, (UI8_T*)&host_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_DeleteRecord in SWDRVL3_LocalDeleteInetHostTunnelRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for AddHostRoute!!");
       return FALSE;
    }

    return TRUE;


}


static BOOL_T SWDRVL3_LocalAddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
{
    BOOL_T       ret = TRUE;
    NetRoute_T  net_record;
    L_HASH_Index_T record_index;

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* Initialize net_record structure */
    memset(&net_record, 0, sizeof(NetRoute_T));

    /* Fill the parameters into the host route record */
    net_record.flags = route->flags;
    net_record.fib_id = route->fib_id;
    net_record.dest_vid = route->dst_vid;
    net_record.unit = route->unit;
    net_record.port = route->port;
    net_record.trunk_id = route->trunk_id;
    net_record.hw_info = route->hw_info;
    net_record.prefix_length = route->prefix_length;
    net_record.nh_hw_info[0] = nh_hw_info;
    memcpy(net_record.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(net_record.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (net_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(net_record.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(net_record.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        net_record.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        net_record.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_SetRecord(&shmem_data_p->SWDRVL3_net_route_hash_desc, (UI8_T*)&net_record,&record_index);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    /* add tunnel net route to link-list */
    {
        if (net_record.flags & SWDRVL3_FLAG_DYNAMIC_TUNNEL)
        {  
            ret |= L_SORT_LST_ShMem_Set(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc,&net_record);     
        }
    }
#endif

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_SetRecord in SWDRVL3_LocalAddInetNetRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for SWDRVL3_LocalAddInetNetRoute!!");
       return FALSE;
    }

    return TRUE;
}

static BOOL_T SWDRVL3_LocalDeleteInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
{
    BOOL_T       ret = TRUE;
    NetRoute_T  net_record;
    L_HASH_Index_T record_index;

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();
    
    /* Initialize net_record structure */
    memset(&net_record, 0, sizeof(NetRoute_T));

    /* Fill the parameters into the net route record */
    net_record.flags = route->flags;
    net_record.dest_vid = route->dst_vid;
    net_record.unit = route->unit;
    net_record.port = route->port;
    net_record.trunk_id = route->trunk_id;
    net_record.hw_info = route->hw_info;
    net_record.prefix_length = route->prefix_length;
    net_record.nh_hw_info[0] = nh_hw_info;
    memcpy(net_record.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(net_record.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (net_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(net_record.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        net_record.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the net route record into OM */
    ret = L_HASH_ShMem_DeleteRecord(&shmem_data_p->SWDRVL3_net_route_hash_desc, (UI8_T*)&net_record,&record_index);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    /* delete tunnel net route to link-list */
    {
        if (net_record.flags & SWDRVL3_FLAG_DYNAMIC_TUNNEL)
        {
            ret |= L_SORT_LST_ShMem_Delete(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc,&net_record);     
        }
    }
#endif 

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_DeleteRecord in SWDRVL3_LocalDeleteInetNetRoute Error");
        }
       return FALSE;
    }

    /* Send event to task to trigger deleting host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for SWDRVL3_LocalDeleteInetNetRoute!!");
       return FALSE;
    }

    return TRUE;
}

static UI32_T SWDRVL3_LocalAddInetMyIpHostRoute(SWDRVL3_Host_T *host)
{
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;
    UI32_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_LocalAddInetMyIpHostRoute\n");
    }
    
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.flags = host->flags;
    dev_swdrvl3_host_route.fib_id = host->fib_id;
    dev_swdrvl3_host_route.vid = host->vid;
    memcpy (dev_swdrvl3_host_route.mac, host->mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    ret = DEV_SWDRVL3_PMGR_AddInetMyIpHostRoute(&dev_swdrvl3_host_route);
    host->hw_info = dev_swdrvl3_host_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}

static UI32_T SWDRVL3_LocalDeleteInetMyIpHostRoute(SWDRVL3_Host_T *host)
{
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;
    UI32_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_LocalDeleteInetMyIpHostRoute");
    }
    
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.flags = host->flags;
    dev_swdrvl3_host_route.fib_id = host->fib_id;
    dev_swdrvl3_host_route.hw_info = host->hw_info;
    if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetMyIpHostRoute(&dev_swdrvl3_host_route);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


static UI32_T SWDRVL3_LocalAddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count)
{
    BOOL_T       ret = TRUE;
    NetRoute_T  net_record;
    L_HASH_Index_T record_index;

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_added_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* Initialize net_record structure */
    memset(&net_record, 0, sizeof(NetRoute_T));

    /* Fill the parameters into the host route record */
    net_record.flags = route->flags;
    net_record.fib_id = route->fib_id;
    net_record.dest_vid = route->dst_vid;
    net_record.unit = route->unit;
    net_record.port = route->port;
    net_record.trunk_id = route->trunk_id;
    net_record.hw_info = route->hw_info;
    if(count>SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE)
    {
        printf("%s(%d):Illegal count value=%lu\r\n", __FUNCTION__, __LINE__, (unsigned long)count);
        count=SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE;/* workaround */
        /* debug dump */
        printf("%s(%d): fib_id=%d dest_vid=%d unit=%d, port=%d, trunk_id=%d\r\n", __FUNCTION__, __LINE__, (int)(route->fib_id),(int)(route->dst_vid),(int)(route->unit),(int)(route->port),(int)(route->trunk_id));
    }
    net_record.prefix_length = route->prefix_length;
    net_record.nh_count = count;
    memcpy(net_record.nh_hw_info, nh_hw_info_array, (sizeof(UI32_T) * count));
    memcpy(net_record.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(net_record.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (net_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(net_record.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(net_record.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        net_record.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        net_record.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_SetRecord(&shmem_data_p->SWDRVL3_net_route_hash_desc, (UI8_T*)&net_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_SetRecord in SWDRVL3_LocalAddInetECMPRouteMultiPath Error\n");
        }
       return SWDRVL3_L3_OTHERS;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for SWDRVL3_LocalAddInetECMPRouteMultiPath!!");
       return SWDRVL3_L3_OTHERS;
    }

    return SWDRVL3_L3_NO_ERROR;
}


static UI32_T SWDRVL3_LocalDeleteInetECMPRoute(SWDRVL3_Route_T *route)
{
    BOOL_T       ret = TRUE;
    NetRoute_T  net_record;
    L_HASH_Index_T record_index;

    SWDRVL3_EnterCriticalSection();
    shmem_data_p->net_deleted_counter++;
	SWDRVL3_LeaveCriticalSection();

    /* Initialize net_record structure */
    memset(&net_record, 0, sizeof(NetRoute_T));

    /* Fill the parameters into the host route record */
    net_record.flags = route->flags;
    net_record.fib_id = route->fib_id;
    net_record.dest_vid = route->dst_vid;
    net_record.unit = route->unit;
    net_record.port = route->port;
    net_record.trunk_id = route->trunk_id;
    net_record.hw_info = route->hw_info;
    net_record.prefix_length = route->prefix_length;
    memcpy(net_record.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(net_record.next_hop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (net_record.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(net_record.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(net_record.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        net_record.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        net_record.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }

    SWDRVL3_EnterCriticalSection();

    /* Set the host route record into OM */
    ret = L_HASH_ShMem_DeleteRecord(&shmem_data_p->SWDRVL3_net_route_hash_desc, (UI8_T*)&net_record,&record_index);

    SWDRVL3_LeaveCriticalSection();

    if (ret == FALSE)
    {
        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
        {
            printf("\r\n L_HASH_DeleteRecord in SWDRVL3_LocalDeleteInetECMPRoute Error\n");
        }
       return FALSE;
    }

    /* Send event to task to trigger adding host route entry into chips */
    if (SYSFUN_SendEvent(shmem_data_p->swdrvl3_task_tid, SWDRVL3_UPDATE_EVENT) != SYSFUN_OK)
    {
       SYSFUN_Debug_Printf("\r\nSWDRVL3 SYSFUN_SendEvent error for %s!!", __FUNCTION__);
       return FALSE;
    }

    return TRUE;
}


static UI32_T SWDRVL3_LocalAddInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first)
{
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;
    UI32_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_LocalAddInetECMPRouteOnePath\n");
    }

    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(dev_swdrvl3_net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        dev_swdrvl3_net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_AddInetECMPRouteOnePath(&dev_swdrvl3_net_route, nh_hw_info, is_first);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


static BOOL_T SWDRVL3_LocalDeleteInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last)
{
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;
    BOOL_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_LocalDeleteInetECMPRouteOnePath\n");
    }
    
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.fib_id = route->fib_id;
    dev_swdrvl3_net_route.dst_vid = route->dst_vid;
    dev_swdrvl3_net_route.prefix_length = route->prefix_length;
    dev_swdrvl3_net_route.unit = route->unit;
    dev_swdrvl3_net_route.port = route->port;
    dev_swdrvl3_net_route.trunk_id = route->trunk_id;
    dev_swdrvl3_net_route.hw_info = route->hw_info;
    memcpy(dev_swdrvl3_net_route.src_mac, route->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(dev_swdrvl3_net_route.nexthop_mac, route->nexthop_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(dev_swdrvl3_net_route.next_hop_ip.ipv6_addr, route->next_hop_ip.ipv6_addr,
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = route->dst_ip.ipv4_addr;
        dev_swdrvl3_net_route.next_hop_ip.ipv4_addr = route->next_hop_ip.ipv4_addr;
    }
    ret = DEV_SWDRVL3_PMGR_DeleteInetECMPRouteOnePath(&dev_swdrvl3_net_route, nh_hw_info, is_last);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


static BOOL_T SWDRVL3_LocalSetSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
{
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_SetSpecialDefaultRoute\n");
    }

    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = default_route->flags;
    dev_swdrvl3_net_route.fib_id = default_route->fib_id;
    dev_swdrvl3_net_route.dst_vid = default_route->dst_vid;
    dev_swdrvl3_net_route.unit = default_route->unit;
    dev_swdrvl3_net_route.port = default_route->port;
    dev_swdrvl3_net_route.trunk_id = default_route->trunk_id;
    
    return DEV_SWDRVL3_PMGR_SetSpecialDefaultRoute(&dev_swdrvl3_net_route, action);
}


static BOOL_T SWDRVL3_LocalDeleteSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
{
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteSpecialDefaultRoute\n");
    }

    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = default_route->flags;
    dev_swdrvl3_net_route.fib_id = default_route->fib_id;
    dev_swdrvl3_net_route.dst_vid = default_route->dst_vid;
    dev_swdrvl3_net_route.unit = default_route->unit;
    dev_swdrvl3_net_route.port = default_route->port;
    dev_swdrvl3_net_route.trunk_id = default_route->trunk_id;
    
    return DEV_SWDRVL3_PMGR_DeleteSpecialDefaultRoute(&dev_swdrvl3_net_route, action);
}


static BOOL_T SWDRVL3_LocalReadAndClearHostRouteEntryHitBit(SWDRVL3_Host_T *host, UI32_T *hit)
{
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;
    UI32_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_ReadAndClearHostRouteEntryHitBit\n");
    }
    
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.flags = host->flags;
    dev_swdrvl3_host_route.fib_id = host->fib_id;
    dev_swdrvl3_host_route.hw_info = host->hw_info;
    if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host->ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_host_route.ip_addr.ipv4_addr = host->ip_addr.ipv4_addr;
    }

    ret = DEV_SWDRVL3_PMGR_ReadAndClearHostRouteEntryHitBit(&dev_swdrvl3_host_route, hit);

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


static BOOL_T SWDRVL3_LocalReadAndClearNetRouteEntryHitBit(SWDRVL3_Route_T *net_route, UI32_T *hit)
{

    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;
    UI32_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_ReadAndClearHostRouteEntryHitBit\n");
    }
    
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = net_route->flags;
    dev_swdrvl3_net_route.fib_id = net_route->fib_id;
    dev_swdrvl3_net_route.prefix_length = net_route->prefix_length;
    if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, net_route->dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        dev_swdrvl3_net_route.dst_ip.ipv4_addr = net_route->dst_ip.ipv4_addr;
    }

    ret = DEV_SWDRVL3_PMGR_ReadAndClearNetRouteEntryHitBit(&dev_swdrvl3_net_route, hit);


    SWDRVL3_RETURN_WITH_VALUE(ret);

}



static BOOL_T SWDRVL3_LocalClearHostRouteHWInfo(SWDRVL3_Host_T *host)
{
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;
    BOOL_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_ClearHostRouteHWInfo\n");
    }
    
    memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
    dev_swdrvl3_host_route.hw_info = host->hw_info;

    ret = DEV_SWDRVL3_PMGR_ClearHostRouteHWInfo(&dev_swdrvl3_host_route);
    host->hw_info = dev_swdrvl3_host_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}


static BOOL_T SWDRVL3_LocalClearNetRouteHWInfo(SWDRVL3_Route_T *route)
{
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;
    BOOL_T ret;
    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
    {
       printf("\r\n Enter DEV_SWDRVL3_PMGR_ClearNetRouteHWInfo\n");
    }
    
    memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
    dev_swdrvl3_net_route.flags = route->flags;
    dev_swdrvl3_net_route.hw_info = route->hw_info;

    ret = DEV_SWDRVL3_PMGR_ClearNetRouteHWInfo(&dev_swdrvl3_net_route);
    route->hw_info = dev_swdrvl3_net_route.hw_info;

    SWDRVL3_RETURN_WITH_VALUE(ret);
}

static BOOL_T SWDRVL3_SlaveEnableIpForwarding(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalEnableIpForwarding(buf_p->info.ip_forwarding_op.flags, 
                                      buf_p->info.ip_forwarding_op.vr_id) == TRUE)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveDisableIpForwarding(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalDisableIpForwarding(buf_p->info.ip_forwarding_op.flags,
                                      buf_p->info.ip_forwarding_op.vr_id) == TRUE)
    {
       status = TRUE;
    }
    
	return status;
}

static BOOL_T SWDRVL3_SlaveEnableRouting(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    BOOL_T status = TRUE;

    if(SWDRVL3_LocalEnableRouting() == FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalEnableRouting fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveDisableRouting(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    BOOL_T status = TRUE;

    if(SWDRVL3_LocalDisableRouting() == FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDisableRouting fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveCreateL3Interface(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalCreateL3Interface(buf_p->info.l3_interface.fib_id, 
                                      buf_p->info.l3_interface.vid,
                                      buf_p->info.l3_interface.mac,
                                      &buf_p->info.l3_interface.hw_info) == SWDRVL3_L3_NO_ERROR)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveDeleteL3Interface(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalDeleteL3Interface(buf_p->info.l3_interface.fib_id,
                                      buf_p->info.l3_interface.vid,
                                      buf_p->info.l3_interface.mac,
                                      buf_p->info.l3_interface.hw_info) == TRUE)
    {
       status = TRUE;
    }
    
	return status;
}

static BOOL_T SWDRVL3_SlaveAddL3Mac(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalAddL3Mac(buf_p->info.l3_interface.mac,
                                buf_p->info.l3_interface.vid) == SWDRVL3_L3_NO_ERROR)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveDeleteL3Mac(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalDeleteL3Mac(buf_p->info.l3_interface.mac,
                                buf_p->info.l3_interface.vid) == TRUE)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveSetL3Bit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalSetL3Bit(buf_p->info.l3_interface.mac,
                              buf_p->info.l3_interface.vid) == TRUE)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveUnSetL3Bit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
	UI8_T status = FALSE;

	if (SWDRVL3_LocalUnSetL3Bit(buf_p->info.l3_interface.mac,
                                buf_p->info.l3_interface.vid) == TRUE)
    {
       status = TRUE;
    }

	return status;
}

static BOOL_T SWDRVL3_SlaveAddInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_HostTunnel_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddHostRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_HostTunnel_T));

    host.flags          = buf_p->info.host_tunnel_route.flags;
    host.fib_id         = buf_p->info.host_tunnel_route.fib_id;
    host.vid            = buf_p->info.host_tunnel_route.dst_vid;
    host.unit           = buf_p->info.host_tunnel_route.unit;
    host.port           = buf_p->info.host_tunnel_route.port;
    host.trunk_id       = buf_p->info.host_tunnel_route.trunk_id;
    host.hw_info        = buf_p->info.host_tunnel_route.hw_info;
    memcpy(host.mac, buf_p->info.host_tunnel_route.dst_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(host.src_mac, buf_p->info.host_tunnel_route.src_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(&(host.tnl_init), &(buf_p->info.host_tunnel_route.tnl_init), 
                sizeof(SWDRVL3_TunnelInitiator_T));
    memcpy(&(host.tnl_term), &(buf_p->info.host_tunnel_route.tnl_term), 
                sizeof(SWDRVL3_TunnelTerminator_T));
 

    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_tunnel_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_tunnel_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalAddInetHostTunnelRoute (&host)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddHostRoute fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveDeleteInetHostTunnelRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_HostTunnel_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteInetHostTunnelRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_HostTunnel_T));

    host.flags          = buf_p->info.host_tunnel_route.flags;
    host.fib_id         = buf_p->info.host_tunnel_route.fib_id;
    host.vid            = buf_p->info.host_tunnel_route.dst_vid;
    host.unit           = buf_p->info.host_tunnel_route.unit;
    host.port           = buf_p->info.host_tunnel_route.port;
    host.trunk_id       = buf_p->info.host_tunnel_route.trunk_id;
    host.hw_info        = buf_p->info.host_tunnel_route.hw_info;
    memcpy(host.mac, buf_p->info.host_tunnel_route.dst_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(host.src_mac, buf_p->info.host_tunnel_route.src_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(&(host.tnl_init), &(buf_p->info.host_tunnel_route.tnl_init), 
                sizeof(SWDRVL3_TunnelInitiator_T));
    memcpy(&(host.tnl_term), &(buf_p->info.host_tunnel_route.tnl_term), 
                sizeof(SWDRVL3_TunnelTerminator_T));
 

    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_tunnel_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_tunnel_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalDeleteInetHostTunnelRoute (&host)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddHostRoute fails", __FUNCTION__);
    }

    return status;
}



static BOOL_T SWDRVL3_SlaveSetInetHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Host_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddHostRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    host.flags          = buf_p->info.host_route.flags;
    host.fib_id         = buf_p->info.host_route.fib_id;
    host.vid            = buf_p->info.host_route.dst_vid;
    host.unit           = buf_p->info.host_route.unit;
    host.port           = buf_p->info.host_route.port;
    host.trunk_id       = buf_p->info.host_route.trunk_id;
    host.hw_info        = buf_p->info.host_route.hw_info;
    memcpy(host.mac, buf_p->info.host_route.dst_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(host.src_mac, buf_p->info.host_route.src_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalSetInetHostRoute (&host)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddHostRoute fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveDeleteInetHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Host_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddHostRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    host.flags          = buf_p->info.host_route.flags;
    host.fib_id         = buf_p->info.host_route.fib_id;
    host.vid            = buf_p->info.host_route.dst_vid;
    host.unit           = buf_p->info.host_route.unit;
    host.port           = buf_p->info.host_route.port;
    host.trunk_id       = buf_p->info.host_route.trunk_id;
    host.hw_info        = buf_p->info.host_route.hw_info;
    memcpy(host.mac, buf_p->info.host_route.dst_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(host.src_mac, buf_p->info.host_route.src_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    
    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalDeleteInetHostRoute (&host)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddHostRoute fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveAddTunnelInitiator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_TunnelInitiator_T tunnel;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddTunnelInitiator");
    }

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelInitiator_T));

    tunnel.vid            = buf_p->info.tunnel_initiator.vid;
    tunnel.tunnel_type	  = buf_p->info.tunnel_initiator.tunnel_type;
    tunnel.ttl		  = buf_p->info.tunnel_initiator.ttl;
    memcpy(tunnel.src_mac, buf_p->info.tunnel_initiator.src_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    memcpy(tunnel.nexthop_mac, buf_p->info.tunnel_initiator.nexthop_mac, 
                SYS_ADPT_MAC_ADDR_LEN);

    memcpy(&(tunnel.sip), &(buf_p->info.tunnel_initiator.sip), sizeof(L_INET_AddrIp_T));
    memcpy(&(tunnel.dip), &(buf_p->info.tunnel_initiator.dip), sizeof(L_INET_AddrIp_T));
  
    if(SWDRVL3_LocalAddTunnelInitiator(&tunnel)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddTunnelInitiator fails", __FUNCTION__);
    }

    return status;

}

static BOOL_T SWDRVL3_SlaveDeleteTunnelInitiator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    UI32_T l3_intf_id;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteTunnelInitiator");
    }

    l3_intf_id		  = buf_p->info.interface_num;
 
    if(SWDRVL3_LocalDeleteTunnelInitiator(l3_intf_id)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteTunnelInitiator fails", __FUNCTION__);
    }

    return status;

}


static BOOL_T SWDRVL3_SlaveTunnelUpdateTtl(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    UI32_T l3_intf_id;
    UI8_T  ttl;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteTunnelInitiator");
    }

    l3_intf_id		  = buf_p->info.tunnel_initiator.l3_intf_id;
    ttl               = buf_p->info.tunnel_initiator.ttl;
    
    if(SWDRVL3_LocalTunnelUpdateTtl(l3_intf_id, ttl)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteTunnelInitiator fails", __FUNCTION__);
    }

    return status;

}

static BOOL_T SWDRVL3_SlaveAddTunnelTerminator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_TunnelTerminator_T tunnel;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddTunnelTerminator");
    }

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    tunnel.fib_id	  = buf_p->info.tunnel_terminator.fib_id;
    tunnel.tunnel_type	  = buf_p->info.tunnel_terminator.tunnel_type;
    memcpy(tunnel.lport, buf_p->info.tunnel_terminator.lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(&(tunnel.sip), &(buf_p->info.tunnel_terminator.sip), sizeof(L_INET_AddrIp_T));
    memcpy(&(tunnel.dip), &(buf_p->info.tunnel_terminator.dip), sizeof(L_INET_AddrIp_T));
  
    if(SWDRVL3_LocalAddTunnelTerminator(&tunnel)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddTunnelTerminator fails", __FUNCTION__);
    }

    return status;

}

static BOOL_T SWDRVL3_SlaveDeleteTunnelTerminator(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_TunnelTerminator_T tunnel;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteTunnelTerminator");
    }

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    tunnel.fib_id	  = buf_p->info.tunnel_terminator.fib_id;
    tunnel.tunnel_type	  = buf_p->info.tunnel_terminator.tunnel_type;
    memcpy(tunnel.lport, buf_p->info.tunnel_terminator.lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(&(tunnel.sip), &(buf_p->info.tunnel_terminator.sip), sizeof(L_INET_AddrIp_T));
    memcpy(&(tunnel.dip), &(buf_p->info.tunnel_terminator.dip), sizeof(L_INET_AddrIp_T));
  
    if(SWDRVL3_LocalDeleteTunnelTerminator(&tunnel)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteTunnelTerminator fails", __FUNCTION__);
    }

    return status;

}

static BOOL_T SWDRVL3_SlaveAddInetNetRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;
    void *nh_hw_info = (void *)SWDRVL3_HW_INFO_INVALID;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddInetNetRoute ");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    nh_hw_info = (void *)(uintptr_t)buf_p->info.net_route.nh_hw_info[0];
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }
    
    if(SWDRVL3_LocalAddInetNetRoute (&route, nh_hw_info)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddInetNetRoute fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveDeleteInetNetRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;
    void *nh_hw_info = (void *)SWDRVL3_HW_INFO_INVALID;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteInetNetRoute ");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    nh_hw_info = (void *)(uintptr_t)buf_p->info.net_route.nh_hw_info[0];
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }
    
    if(SWDRVL3_LocalDeleteInetNetRoute (&route, nh_hw_info)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteInetNetRoute fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveAddInetMyIpHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Host_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddInetMyIpHostRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    host.flags          = buf_p->info.host_route.flags;
    host.fib_id         = buf_p->info.host_route.fib_id;
    host.vid            = buf_p->info.host_route.dst_vid;
    memcpy(host.mac, buf_p->info.host_route.dst_mac, 
                SYS_ADPT_MAC_ADDR_LEN);
    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalAddInetMyIpHostRoute (&host) != SWDRVL3_L3_NO_ERROR)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddInetMyIpHostRoute fails", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveDeleteInetMyIpHostRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Host_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteInetMyIpHostRoute ");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    host.flags          = buf_p->info.host_route.flags;
    host.fib_id         = buf_p->info.host_route.fib_id;
    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalDeleteInetMyIpHostRoute (&host) != SWDRVL3_L3_NO_ERROR)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteInetMyIpHostRoute fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveAddInetECMPRouteMultiPath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;
    void * nh_hw_info_array;
    UI32_T nh_count;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddInetECMPRouteMultiPath\n");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    nh_count = buf_p->info.net_route.nh_count;
    nh_hw_info_array = (void *)buf_p->info.net_route.nh_hw_info;
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }
    
    if(SWDRVL3_LocalAddInetECMPRouteMultiPath(&route, nh_hw_info_array, nh_count) != 
                SWDRVL3_L3_NO_ERROR);
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddInetECMPRouteMultiPath fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveDeleteInetECMPRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteInetECMPRoute\n");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }
    
    if(SWDRVL3_LocalDeleteInetECMPRoute(&route) != SWDRVL3_L3_NO_ERROR)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteInetECMPRoute fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveAddInetECMPRouteOnePath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{    
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;
    void *nh_hw_info = (void *)SWDRVL3_HW_INFO_INVALID;
    BOOL_T is_first = FALSE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddInetECMPRouteOnePath\n");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    nh_hw_info = (void *)(uintptr_t)buf_p->info.net_route.nh_hw_info[0];
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }

    if (route.flags & SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE)
        is_first = TRUE;
    
    if(SWDRVL3_LocalAddInetECMPRouteOnePath(&route, nh_hw_info, is_first)!= 
                SWDRVL3_L3_NO_ERROR);
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalAddInetECMPRouteOnePath fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveDeleteInetECMPRouteOnePath(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T route;
    void *nh_hw_info = (void *)SWDRVL3_HW_INFO_INVALID;
    BOOL_T is_last = FALSE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteInetECMPRouteOnePath\n");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags = buf_p->info.net_route.flags;
    route.fib_id = buf_p->info.net_route.fib_id;
    route.dst_vid = buf_p->info.net_route.dest_vid;
    route.unit = buf_p->info.net_route.unit;
    route.port = buf_p->info.net_route.port;
    route.trunk_id = buf_p->info.net_route.trunk_id;
    route.prefix_length = buf_p->info.net_route.prefix_length;
    route.hw_info = buf_p->info.net_route.hw_info;
    nh_hw_info = (void *)(uintptr_t)buf_p->info.net_route.nh_hw_info[0];
    memcpy(route.nexthop_mac, buf_p->info.net_route.next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(route.src_mac, buf_p->info.net_route.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    
    if (route.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(route.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
        memcpy(route.next_hop_ip.ipv6_addr, buf_p->info.net_route.next_hop_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        route.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
        route.next_hop_ip.ipv4_addr = buf_p->info.net_route.next_hop_ip.ipv4_addr;
    }

    if (route.flags & SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE)
        is_last = TRUE;
    
    if(SWDRVL3_LocalDeleteInetECMPRouteOnePath(&route, nh_hw_info, is_last)!= 
                SWDRVL3_L3_NO_ERROR);
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteInetECMPRouteOnePath fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveSetSpecialDefaultRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T default_route;
    UI32_T action;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveSetSpecialDefaultRoute\n");
    }

    memset(&default_route, 0, sizeof(SWDRVL3_Route_T));
    default_route.flags = buf_p->info.net_route.flags;
    default_route.fib_id = buf_p->info.net_route.fib_id;
    default_route.dst_vid = buf_p->info.net_route.dest_vid;
    default_route.unit = buf_p->info.net_route.unit;
    default_route.port = buf_p->info.net_route.port;
    default_route.trunk_id = buf_p->info.net_route.trunk_id;
    action = buf_p->info.net_route.action;
    
    if(SWDRVL3_LocalSetSpecialDefaultRoute(&default_route, action)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalSetSpecialDefaultRoute fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveDeleteSpecialDefaultRoute(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T default_route;
    UI32_T action;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveDeleteSpecialDefaultRoute\n");
    }

    memset(&default_route, 0, sizeof(SWDRVL3_Route_T));
    default_route.flags = buf_p->info.net_route.flags;
    default_route.fib_id = buf_p->info.net_route.fib_id;
    default_route.dst_vid = buf_p->info.net_route.dest_vid;
    default_route.unit = buf_p->info.net_route.unit;
    default_route.port = buf_p->info.net_route.port;
    default_route.trunk_id = buf_p->info.net_route.trunk_id;
    action = buf_p->info.net_route.action;
    
    if(SWDRVL3_LocalDeleteSpecialDefaultRoute(&default_route, action)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteSpecialDefaultRoute fails", __FUNCTION__);
    }

    return status;
}


static BOOL_T SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Host_T host;
    UI32_T hit;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit\n");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    host.flags          = buf_p->info.host_route.flags;
    host.fib_id         = buf_p->info.host_route.fib_id;
    hit                 = buf_p->info.host_route.hit;
    if (host.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(host.ip_addr.ipv6_addr, buf_p->info.host_route.ip_addr.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        host.ip_addr.ipv4_addr = buf_p->info.host_route.ip_addr.ipv4_addr;
    }
    
    if(SWDRVL3_LocalReadAndClearHostRouteEntryHitBit (&host, &hit)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalReadAndClearHostRouteEntryHitBit fails\n", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveReadAndClearNetRouteEntryHitBit(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    UI8_T status = TRUE;
    SWDRVL3_Route_T net;
    UI32_T hit;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit\n");
    }

    memset(&net, 0, sizeof(SWDRVL3_Route_T));

    net.flags          = buf_p->info.net_route.flags;
    net.fib_id         = buf_p->info.net_route.fib_id;
    net.prefix_length  = buf_p->info.net_route.prefix_length;
    hit                 = buf_p->info.net_route.hit;
    if (net.flags & SWDRVL3_FLAG_IPV6)
    {
        memcpy(net.dst_ip.ipv6_addr, buf_p->info.net_route.dst_ip.ipv6_addr, 
                    SYS_ADPT_IPV6_ADDR_LEN);
    }
    else
    {
        net.dst_ip.ipv4_addr = buf_p->info.net_route.dst_ip.ipv4_addr;
    }
    
    if(SWDRVL3_LocalReadAndClearNetRouteEntryHitBit (&net, &hit)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalReadAndClearHostRouteEntryHitBit fails\n", __FUNCTION__);
    }

    return status;
}

static BOOL_T SWDRVL3_SlaveClearHostRouteHWInfo(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    SWDRVL3_Host_T host;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveClearHostRouteHWInfo\n");
    }

    memset(&host, 0, sizeof(SWDRVL3_Host_T));
    host.hw_info        = buf_p->info.host_route.hw_info;
    
    return SWDRVL3_LocalClearHostRouteHWInfo(&host);
}


static BOOL_T SWDRVL3_SlaveClearNetRouteHWInfo(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
    SWDRVL3_Route_T route;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveClearNetRouteHWInfo\n");
    }

    memset(&route, 0, sizeof(SWDRVL3_Route_T));
    route.flags          = buf_p->info.net_route.flags;
    route.hw_info        = buf_p->info.net_route.hw_info;
    
    return SWDRVL3_LocalClearNetRouteHWInfo(&route);
}

static BOOL_T SWDRVL3_SlaveAddTunnelIntfL3(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI8_T status = TRUE;
    DEV_SWDRVL3_TunnelIntfL3_T dsl3_tl3;
    SWDRVL3_TunnelIntfL3_T  *tl3_p;

    
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveAddTunnelIntfL3");
    }

    memset(&dsl3_tl3, 0, sizeof(dsl3_tl3));

    tl3_p = &buf_p->info.tl3;

    dsl3_tl3.vid	    = tl3_p->vid;
    dsl3_tl3.l3_intf_id	= tl3_p->l3_intf_id;
    memcpy(dsl3_tl3.src_mac, tl3_p->src_mac, sizeof(dsl3_tl3.src_mac));
    
    if(DEV_SWDRVL3_PMGR_AddTunnelIntfL3(&dsl3_tl3, tl3_p->is_add)!=DEV_SWDRVL3_L3_NO_ERROR)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalDeleteTunnelInitiator fails", __FUNCTION__);
    }

    tl3_p->l3_intf_id = dsl3_tl3.l3_intf_id;

    return status;
#else

    return TRUE;

#endif

}

static BOOL_T SWDRVL3_SlaveSetEcmpBalanceMode(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    UI8_T status = TRUE;
    UI32_T mode;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveSetEcmpBalanceMode");
    }

    mode = buf_p->info.mode;
    
    if(SWDRVL3_LocalSetEcmpBalanceMode(mode)==FALSE)
    {
        status = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL3_LocalSetEcmpBalanceMode", __FUNCTION__);
    }

    return status;
#else
    return TRUE;
#endif
}

static BOOL_T SWDRVL3_TunnelNetRouteHitBitChange(ISC_Key_T *key, SWDRVL3_RxIscBuf_T *buf_p)
{

    UI8_T dst_addr[SYS_ADPT_IPV6_ADDR_LEN] ={0};
    UI32_T unit_id = 0;
    UI32_T preflen = 0;
    UI32_T fib_id = 0;
    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
        printf("\r\n Enter SWDRVL3_SlaveReadAndClearHostRouteEntryHitBit\n");
    }
    fib_id             = buf_p->info.net_route.fib_id;
    preflen            = buf_p->info.net_route.prefix_length;
    unit_id            = buf_p->info.net_route.unit;
    memcpy(dst_addr, buf_p->info.net_route.dst_ip.ipv6_addr, SYS_ADPT_IPV6_ADDR_LEN);

    
    /* callback to amtrl3 to update stacking unit's net route hit bit state */
#if (SYS_CPNT_IP_TUNNEL == TRUE)    
    SYS_CALLBACK_MGR_TunnelNetRouteHitBitChangeCallback(SYS_MODULE_SWDRVL3,fib_id, dst_addr, preflen, unit_id);
#endif
    return TRUE;
}


/* -------------------------------------------------------------------------
 * Function : SWDRVL3_GetElementByGroupHostEntry
 * -------------------------------------------------------------------------
 * Purpose  : This callback function is used by L_Hash to get the port number
 *            through the group ID and the record.
 * INPUT    : group    -- type of the group
 *            *record  -- the pointer to the record which saved the Host Entry
 * OUTPUT   : *port -- port number
 * RETURN   : TRUE/FALSE
 * NOTE     : 1. If the record can be found in the specific group, this api returns
 *               TRUE and return the port number.
 *            2. If the record cannot be found in the specific group, this api
 *               returns FALSE.
 *            3. If group is 0, get the port number by the specific record.
 *            4. If group is 1, get the vid by the specific record.
 *            5. Logically speaking, swdrvl3 shall not call SWCTRL API.
 *            6. The port number for L_HASH is from 0 rather than 1.
 *               So the port number must be ifindex and 0 means local.
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRVL3_GetElementByGroupHostEntry(UI32_T group, void *record, UI32_T *element)
{
    if (group >= L_HASH_MAX_QUERY_GROUP)
       return FALSE;
    if ((record == 0) || (element == 0))
       return FALSE;

    switch(group)
    {
        case 0:
            if ((((HostRoute_T *)record)->unit < 1) ||
                (((HostRoute_T *)record)->unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
                (((HostRoute_T *)record)->port == 0) ||
                (((HostRoute_T *)record)->port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
                *element = 0;
            else if (((HostRoute_T *)record)->flags & SWDRVL3_FLAG_TRUNK_EGRESS_PORT)
                *element = STKTPLG_OM_TRUNKID_TO_IFINDEX(((HostRoute_T *)record)->trunk_id);
            else
                *element = STKTPLG_OM_UPORT_TO_IFINDEX(((HostRoute_T *)record)->unit,
                                                      ((HostRoute_T *)record)->port);
            break;
        case 1:
            *element = ((HostRoute_T *)record)->dst_vid;
            break;
        default:
            break;
    }

    return TRUE;
} /* SWDRVL3_GetElementByGroupHostEntry() */


/* -------------------------------------------------------------------------
 * Function : SWDRVL3_GetElementByGroupNetEntry
 * -------------------------------------------------------------------------
 * Purpose  : This callback function is used by L_Hash to get the port number
 *            through the group ID and the record.
 * INPUT    : group    -- type of the group
 *            *record  -- the pointer to the record which saved the Net Entry
 * OUTPUT   : *port -- port number
 * RETURN   : TRUE/FALSE
 * NOTE     : 1. If the record can be found in the specific group, this api returns
 *               TRUE and return the port number.
 *            2. If the record cannot be found in the specific group, this api
 *               returns FALSE.
 *            3. If group is 0, get the port number by the specific record.
 *            4. If group is 1, get the vid by the specific record.
 *            5. Logically speaking, swdrvl3 shall not call SWCTRL API.
 *            6. The port number for L_HASH is from 0 rather than 1.
 *               So the port number must be ifindex and 0 means local.
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRVL3_GetElementByGroupNetEntry(UI32_T group, void *record, UI32_T *element)
{
    if (group >= L_HASH_MAX_QUERY_GROUP)
       return FALSE;
    if ((record == 0) || (element == 0))
       return FALSE;

    switch(group)
    {
        case 0:
            if ((((NetRoute_T *)record)->unit < 1) ||
                (((NetRoute_T *)record)->unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
                (((NetRoute_T *)record)->port == 0) ||
                (((NetRoute_T *)record)->port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
                *element = 0;
            else if (((NetRoute_T *)record)->flags & SWDRVL3_FLAG_TRUNK_EGRESS_PORT)
                *element = STKTPLG_OM_TRUNKID_TO_IFINDEX(((NetRoute_T *)record)->trunk_id);
            else
                *element = STKTPLG_OM_UPORT_TO_IFINDEX(((NetRoute_T *)record)->unit,
                                                      ((NetRoute_T *)record)->port);
            break;
        case 1:
            *element = ((NetRoute_T *)record)->dest_vid;
            break;
        default:
            break;
    }

    return TRUE;
} /* SWDRVL3_GetElementByGroupNetEntry() */


/* -------------------------------------------------------------------------
 * Function : SWDRVL3_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of SWDRVL3 via ISC
 * INPUT    : *key      -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_Agent
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRVL3_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    SWDRVL3_RxIscBuf_T *buf_p;
    UI32_T             service;
    BOOL_T             return_val = FALSE;

    buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &service); /* service is used as dummy here */
    if(buf_p==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
        return FALSE;
    }

    service = buf_p->ServiceID;
    if (service<SWDRVL3_MAX_SERVICE_ID && SWDRVL3_func_tab[service] != NULL)
    {
        return_val = SWDRVL3_func_tab[service](key, buf_p);
    }
    else
    {
        SYSFUN_Debug_Printf("\r\n%s():Invalid service ID(%lu)", __FUNCTION__, service);
    }

    L_MM_Mref_Release(&mref_handle_p);

    return return_val;
} /* SWDRVL3_ISC_Handler() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_InitHashHostTable
 * -------------------------------------------------------------------------
 * PURPOSE: Init Host Table with proper OFFSET and Field size
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None
 * NOTES:   1. There are two keys for hash table of host route:
 *             1.1 Destination IP address
 *             1.2 Destination VLAN
 * -------------------------------------------------------------------------*/
static void SWDRVL3_InitHashHostTable(void)
{
    HostRoute_T tmp;

    memset(&shmem_data_p->SWDRVL3_host_route_hash_desc, 0, sizeof(L_HASH_ShMem_Desc_T));

    shmem_data_p->SWDRVL3_host_route_hash_desc.nbr_of_hash_bucket = L3_BUCKET_SIZE;
    shmem_data_p->SWDRVL3_host_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_HOST_ROUTE;
    shmem_data_p->SWDRVL3_host_route_hash_desc.key_offset[0] = (UI8_T *)&tmp.ip_addr - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_host_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(HostRoute_T, ip_addr);
    
    shmem_data_p->SWDRVL3_host_route_hash_desc.key_offset[1] = (UI8_T *)&tmp.dst_vid - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_host_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(HostRoute_T, dst_vid);
    shmem_data_p->SWDRVL3_host_route_hash_desc.record_size = sizeof(HostRoute_T);
    shmem_data_p->SWDRVL3_host_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;

    shmem_data_p->SWDRVL3_host_route_hash_desc.buffer_offset = 
            SWDRVL3_HOST_ROUTE_HASH_BUFFER_OFFSET - L_CVRT_GET_OFFSET(shmem_data_p, &shmem_data_p->SWDRVL3_host_route_hash_desc);

    shmem_data_p->SWDRVL3_host_route_hash_sz = L_HASH_ShMem_GetWorkingBufferRequiredSize(&shmem_data_p->SWDRVL3_host_route_hash_desc);

    L_HASH_ShMem_Create(&shmem_data_p->SWDRVL3_host_route_hash_desc);
} /* SWDRVL3_InitHashHostTable() */


/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_InitHashNetTable
 * -------------------------------------------------------------------------
 * PURPOSE: Init Host Table with proper OFFSET and Field size
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None
 * NOTES:   1. There are two keys for hash table of net route:
 *             1.1 Destination IP address
 *             1.2 Prefix length
 * -------------------------------------------------------------------------*/
static void SWDRVL3_InitHashNetTable(void)
{
    NetRoute_T tmp;

    memset(&shmem_data_p->SWDRVL3_net_route_hash_desc, 0, sizeof(L_HASH_ShMem_Desc_T));

    shmem_data_p->SWDRVL3_net_route_hash_desc.nbr_of_hash_bucket = L3_BUCKET_SIZE;
    shmem_data_p->SWDRVL3_net_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_NET_ROUTE;
    shmem_data_p->SWDRVL3_net_route_hash_desc.key_offset[0] = (UI8_T *)&tmp.dst_ip - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_net_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(NetRoute_T, dst_ip);

    shmem_data_p->SWDRVL3_net_route_hash_desc.key_offset[1] = (UI8_T *)&tmp.prefix_length - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_net_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(NetRoute_T, prefix_length);
    shmem_data_p->SWDRVL3_net_route_hash_desc.record_size = sizeof(NetRoute_T);
    shmem_data_p->SWDRVL3_net_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;

    shmem_data_p->SWDRVL3_net_route_hash_desc.buffer_offset = 
            SWDRVL3_NET_ROUTE_HASH_BUFFER_OFFSET - L_CVRT_GET_OFFSET(shmem_data_p, &shmem_data_p->SWDRVL3_net_route_hash_desc);
    shmem_data_p->SWDRVL3_net_route_hash_sz = L_HASH_ShMem_GetWorkingBufferRequiredSize(&shmem_data_p->SWDRVL3_net_route_hash_desc);

    L_HASH_ShMem_Create(&shmem_data_p->SWDRVL3_net_route_hash_desc);
} /* SWDRVL3_InitHashHostTable() */


/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_InitHashHostTunnelTable
 * -------------------------------------------------------------------------
 * PURPOSE: Init Host Tunnel Table with proper OFFSET and Field size
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None
 * NOTES:   1. There are two keys for hash table of host tunnel route:
 *             1.1 L3 Interface ID assoicated with the tunnel initiator
 *             1.2 HW Info in the host route entry
 * -------------------------------------------------------------------------*/
static void SWDRVL3_InitHashHostTunnelTable(void)
{
    HostTunnelRoute_T tmp;

    memset(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, 0, sizeof(L_HASH_ShMem_Desc_T));

    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.nbr_of_hash_bucket = L3_BUCKET_SIZE;
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_HOST_ROUTE;
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.key_offset[0] = (UI8_T *)&tmp.dst_vid - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.key_size[0] = SWDRVL3_SIZEOF(HostTunnelRoute_T, dst_vid);
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.key_offset[1] = (UI8_T *)&tmp.ip_addr - (UI8_T *)&tmp;
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.key_size[1] = SWDRVL3_SIZEOF(HostTunnelRoute_T, ip_addr);
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.record_size = sizeof(HostTunnelRoute_T);
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;

    shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc.buffer_offset = 
            SWDRVL3_HOST_TUNNEL_ROUTE_HASH_BUFFER_OFFSET - L_CVRT_GET_OFFSET(shmem_data_p, &shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc);
    shmem_data_p->SWDRVL3_host_tunnel_route_hash_sz = L_HASH_ShMem_GetWorkingBufferRequiredSize(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc);
    L_HASH_ShMem_Create(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc);
} /* SWDRVL3_InitHashHostTable() */



/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_InitLinkListNetTunnelTable
 * -------------------------------------------------------------------------
 * PURPOSE: Init net Tunnel Table 
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None
 * NOTES:   1. There are two keys for hash table of host tunnel route:
 *             1.1 L3 Interface ID assoicated with the tunnel initiator
 *             1.2 HW Info in the host route entry
 * -------------------------------------------------------------------------*/
static void SWDRVL3_InitLinkListNetTunnelTable(void)
{

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    memset(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc, 0, sizeof(L_SORT_LST_ShMem_List_T));

    L_SORT_LST_ShMem_Create(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc,
                            ((UI8_T *)shmem_data_p + SWDRVL3_NET_TUNNEL_ROUTE_SORT_LST_BUFFER_OFFSET),
                            SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_NET_CACHE_ENTRY,
                            sizeof(NetRoute_T),
                            L_SORT_LST_SHMEM_COMPARE_FUNC_ID_SWDRVL3_TUNNEL_NET_ROUTE);
    shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_sz = L_SORT_LST_SHMEM_PT_BUFFER_REQUIRED_SZ(SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_NET_CACHE_ENTRY, sizeof(NetRoute_T));
   
#endif   

} /* SWDRVL3_InitHashHostTable() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_EventHandler
 * -------------------------------------------------------------------------
 * PURPOSE: Process Event
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE--done, FALSE--not all done.
 * NOTES:   1. Process SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS net route entries first
 *          2. Process SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS host route entries then.
 *          3. Process SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS tunnel host route entries at last
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRVL3_EventHandler(void)
{
    BOOL_T       rc_net, rc_host, rc_tunnel, rc_all;
    

    SWDRVL3_EnterCriticalSection();
    /* Process net route entries */
    rc_net = SWDRVL3_ProcessNetRouteQueue();


    
    /* Process host route entries */
    rc_host = SWDRVL3_ProcessHostRouteQueue();
    
    /* Process tunnel host route entries */
    rc_tunnel = SWDRVL3_ProcessTunnelHostRouteQueue();

    rc_all = rc_tunnel&rc_net&rc_host;
    
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    /* Save tick information for backdoor */
    if ((shmem_data_p->system_ticker_enable != FALSE) && (shmem_data_p->system_tick_usage_index < 200))
    {
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][0] = shmem_data_p->system_ticker1_1;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][1] = shmem_data_p->system_ticker1_2;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][2] = shmem_data_p->system_ticker2_1;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][3] = shmem_data_p->system_ticker2_2;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][4] = shmem_data_p->system_ticker1_2 - shmem_data_p->system_ticker1_1;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][5] = shmem_data_p->system_ticker2_2 - shmem_data_p->system_ticker2_1;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][6] = shmem_data_p->process_counter1;
       shmem_data_p->system_tick_usage[shmem_data_p->system_tick_usage_index][7] = shmem_data_p->process_counter2;
       shmem_data_p->system_tick_usage_index++;
    }
#endif
    SWDRVL3_LeaveCriticalSection();

    /*
     * If all net route entries and host route entries are processed, return TRUE.
     * Otherwise need swdrvl3 task process remain entries in next loop.
     */
    return (rc_all);
} /* SWDRVL3_EventHandler() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_TunnelNetRouteHandler
 * -------------------------------------------------------------------------
 * PURPOSE: Process Tunnel net route event 
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE--done, FALSE--not all done.
 * NOTES:   
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRVL3_TunnelNetRouteHandler(void)
{

    NetRoute_T  tunnel_net_route;
    SWDRVL3_Route_T  sd3c_net_route;
    UI32_T hit=0;
    UI16_T dst_bmp = 0;

    memset(&tunnel_net_route, 0, sizeof(tunnel_net_route));
    SWDRVL3_EnterCriticalSection();
    dst_bmp |= BIT_VALUE(shmem_data_p->swdrvl3_stack_info.master_unit_id-1);
    /* Every 20 sec read all tunnel net route's hit bit */
    while(L_SORT_LST_ShMem_Get_Next(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc, &tunnel_net_route))
    {
        memset(&sd3c_net_route, 0, sizeof(sd3c_net_route));
        sd3c_net_route.flags = tunnel_net_route.flags;
        sd3c_net_route.fib_id = tunnel_net_route.fib_id;
        sd3c_net_route.prefix_length = tunnel_net_route.prefix_length;
        if(sd3c_net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(sd3c_net_route.dst_ip.ipv6_addr, tunnel_net_route.dst_ip.ipv6_addr,SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            sd3c_net_route.dst_ip.ipv4_addr = tunnel_net_route.dst_ip.ipv4_addr;
        }
        
        if(TRUE == SWDRVL3_LocalReadAndClearNetRouteEntryHitBit(&sd3c_net_route, &hit))
        {
            /* check if hit bit state is changed,
             * if yes, we send ISC to master unit to update amtrl3's hit bit
             */
            
            L_MM_Mref_Handle_T* mref_handle_p=NULL;
            SWDRVL3_RxIscBuf_T* isc_buf_p=NULL;
            UI32_T pdu_len=0;
            UI32_T isc_ret_val=0;
            UI32_T hit_bit=0;
            
            /* check if hit bit state is changed */
            if(tunnel_net_route.hit ^ hit)
            {
            
                /* send ISC to master unit */
                if(dst_bmp!=0)
                {
                    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), 
                        L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE));
                    isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
                    if(isc_buf_p==NULL)
                    {
                        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                        SWDRVL3_RETURN_WITH_VALUE(SWDRVL3_L3_OTHERS);
                    }

                    isc_buf_p->ServiceID = SWDRVL3_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE;          
                    isc_buf_p->info.net_route.flags                = sd3c_net_route.flags;
                    isc_buf_p->info.net_route.fib_id               = sd3c_net_route.fib_id;
                    isc_buf_p->info.net_route.prefix_length        = sd3c_net_route.prefix_length;
                    isc_buf_p->info.net_route.hit                  = hit;
                    isc_buf_p->info.net_route.unit                 = shmem_data_p->swdrvl3_stack_info.my_unit_id;
                    if (isc_buf_p->info.net_route.flags & SWDRVL3_FLAG_IPV6)
                    {
                        memcpy(isc_buf_p->info.net_route.dst_ip.ipv6_addr, sd3c_net_route.dst_ip.ipv6_addr, 
                                    SYS_ADPT_IPV6_ADDR_LEN);
                    }
                    else
                    {
                        isc_buf_p->info.net_route.dst_ip.ipv4_addr = sd3c_net_route.dst_ip.ipv4_addr;
                    }

                    isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                                      mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                      SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

                    if(isc_ret_val!=0)
                    {
                        /* If some of the stacking units fails, data among units will be
                         * different. May need to design a error handling mechanism for this
                         * condition
                         */
            
                        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                        {
                            printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
                        } 
                        SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
                    
                    }
                }

                /* update shmem's hit bit state */
                tunnel_net_route.hit = hit;
                if(FALSE == L_SORT_LST_ShMem_Set(&shmem_data_p->SWDRVL3_net_tunnel_route_sort_lst_desc,&tunnel_net_route))
                {
            
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n%s():L_SORT_LST_ShMem_Set() fails", __FUNCTION__);
                    } 
                }
            }
            
   
        }
    }
    SWDRVL3_LeaveCriticalSection();

    return TRUE;
}


static BOOL_T SWDRVL3_ProcessNetRouteQueue(void)
{
    BOOL_T      l_hash_result = FALSE;
    BOOL_T      finished_processed = FALSE;
    UI32_T      process_number=0;
    NetRoute_T  *net_record;
    UI8_T       action;
    BOOL_T      event_type, ret = FALSE;
    UI32_T      return_buf=0;
    L_HASH_Index_T record_index;
    DEV_SWDRVL3_Route_T dev_swdrvl3_net_route;

    /* BODY
     */
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_ticker1_1 = SYSFUN_GetSysTick();

#endif
    while (!finished_processed)
    {
        l_hash_result = L_HASH_ShMem_DequeueJobList(&shmem_data_p->SWDRVL3_net_route_hash_desc, (UI8_T**)&net_record, &action,&record_index);

        if (l_hash_result == FALSE)
        {
            if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
            {
                printf("\r\n L_HASH_DequeueJobList return FALSE");
            }
            /* If there is no job in queue, we think the work is finished, therefore, we should return TRUE */
            ret=TRUE;
            break;
        }

        memset(&dev_swdrvl3_net_route, 0, sizeof(DEV_SWDRVL3_Route_T));
        dev_swdrvl3_net_route.flags = net_record->flags;
        dev_swdrvl3_net_route.fib_id = net_record->fib_id;
        dev_swdrvl3_net_route.dst_vid = net_record->dest_vid;
        dev_swdrvl3_net_route.prefix_length = net_record->prefix_length;
        dev_swdrvl3_net_route.unit = net_record->unit;
        dev_swdrvl3_net_route.port = net_record->port;
        dev_swdrvl3_net_route.trunk_id = net_record->trunk_id;
        dev_swdrvl3_net_route.hw_info = net_record->hw_info;
        memcpy(dev_swdrvl3_net_route.src_mac, net_record->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(dev_swdrvl3_net_route.nexthop_mac, net_record->next_hop_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (dev_swdrvl3_net_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(dev_swdrvl3_net_route.dst_ip.ipv6_addr, net_record->dst_ip.ipv6_addr, 
                        SYS_ADPT_IPV6_ADDR_LEN);
            memcpy(dev_swdrvl3_net_route.next_hop_ip.ipv6_addr, net_record->next_hop_ip.ipv6_addr,
                        SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            dev_swdrvl3_net_route.dst_ip.ipv4_addr = net_record->dst_ip.ipv4_addr;
            dev_swdrvl3_net_route.next_hop_ip.ipv4_addr = net_record->next_hop_ip.ipv4_addr;
        }

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
        shmem_data_p->process_counter1++;
#endif

        process_number++;
        switch(action)
        {
           case L_HASH_ACTION_ADD:
                return_buf = SWDRVL3_L3_NO_ERROR;

                if (net_record->flags & SWDRVL3_FLAG_ECMP)
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                    {
                       printf("\r\n Enter DEV_SWDRVL3_PMGR_AddInetECMPRouteMultiPath\n");
                    }
                    return_buf = DEV_SWDRVL3_PMGR_AddInetECMPRouteMultiPath(&dev_swdrvl3_net_route, 
                                        net_record->nh_hw_info, net_record->nh_count);
                }
                else
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                    {
                       printf("\r\n Enter DEV_SWDRVL3_PMGR_AddInetNetRoute\n");
                    }
                    return_buf = DEV_SWDRVL3_PMGR_AddInetNetRoute(&dev_swdrvl3_net_route, (void *)(uintptr_t)net_record->nh_hw_info[0]);
                }
                
                /* The return value may not be 1, therefore, we tranfer the return value to TRUE or FALSE */
                if (return_buf == SWDRVL3_L3_NO_ERROR)
                {
                    ret=TRUE;
                }else{
                    ret=FALSE;
                }
                if (ret == FALSE)
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n DEV_SWDRVL3_AddNetRoute Error\n");
                    }
                }
                net_record->hw_info = dev_swdrvl3_net_route.hw_info;
                break;
           case L_HASH_ACTION_DEL:
                if (net_record->flags & SWDRVL3_FLAG_ECMP)
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                    {
                       printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteInetECMPRoute\n");
                    }
                    ret = DEV_SWDRVL3_PMGR_DeleteInetECMPRoute(&dev_swdrvl3_net_route);
                }
                else
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                    {
                       printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteInetNetRoute\n");
                    }
                    ret = DEV_SWDRVL3_PMGR_DeleteInetNetRoute(&dev_swdrvl3_net_route, (void *)(uintptr_t)net_record->nh_hw_info[0]);
                }
                
                if (ret == FALSE)
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n DEV_SWDRVL3_DeleteNetRoute Error\n");
                    }
                }
                net_record->hw_info = dev_swdrvl3_net_route.hw_info;
                break;
           default:
                break;
        }

        if (ret == TRUE)
        {
           if(action == L_HASH_ACTION_ADD)
           {
               event_type = L_HASH_SET_SUCCESS_EV;
           }
           else
           {
               event_type = L_HASH_DEL_SUCCESS_EV;
           }
        }
        else
        {
           event_type = L_HASH_FAIL_EV;
        }

        if (L_HASH_ShMem_OperationResult(&shmem_data_p->SWDRVL3_net_route_hash_desc, event_type, (UI8_T *)net_record) == FALSE)
        {
           SYSFUN_Debug_Printf("\n\r SWDRVL3: L_HASH_OperationResult for net route fails");
           /*Not finished all entries, so return value should be FALSE*/
           ret = FALSE;
           break;
        }

        /*if (process_number > SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS)*/
        if (process_number > shmem_data_p->max_num_process)
        {
           ret = FALSE;
           break;
        }
    } /* end of while */

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_ticker1_2 = SYSFUN_GetSysTick();
#endif

    return ret;
} /* end of SWDRVL3_ProcessNetRouteQueue() */

static BOOL_T SWDRVL3_ProcessHostRouteQueue(void)
{
    BOOL_T      l_hash_result = FALSE;
    BOOL_T      finished_processed = FALSE;
    UI32_T      process_number=0, event_type;
    BOOL_T      ret = FALSE;
    UI8_T       action;
    UI32_T      return_buf=0;
    HostRoute_T *host_record;
    L_HASH_Index_T record_index;
    DEV_SWDRVL3_Host_T dev_swdrvl3_host_route;

    /* BODY
     */
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_ticker2_1 = SYSFUN_GetSysTick();

#endif
    while (!finished_processed)
    {
        /* Process host route entries */
        l_hash_result = L_HASH_ShMem_DequeueJobList(&shmem_data_p->SWDRVL3_host_route_hash_desc, (UI8_T**)&host_record, &action,&record_index);
        if (l_hash_result == FALSE)
        {
            if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
            {
                printf("\r\n L_HASH_DequeueJobList return FALSE");
            }
            /* If there is no job in queue, we think the work is finished, therefore, we should return TRUE */
            ret=TRUE;
            break;
        } /* end of if */

        memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_Host_T));
        dev_swdrvl3_host_route.flags = host_record->flags;
        dev_swdrvl3_host_route.fib_id = host_record->fib_id;
        dev_swdrvl3_host_route.vid = host_record->dst_vid;
        dev_swdrvl3_host_route.unit = host_record->unit;
        dev_swdrvl3_host_route.port = host_record->port;
        dev_swdrvl3_host_route.trunk_id = host_record->trunk_id;
        dev_swdrvl3_host_route.hw_info = host_record->hw_info;
        memcpy(dev_swdrvl3_host_route.src_mac, host_record->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(dev_swdrvl3_host_route.mac, host_record->dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host_record->ip_addr.ipv6_addr,
                SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            dev_swdrvl3_host_route.ip_addr.ipv4_addr = host_record->ip_addr.ipv4_addr;
        }

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
        shmem_data_p->process_counter2++;
#endif
        process_number++;
        switch(action)
        {
           case L_HASH_ACTION_ADD:
                return_buf = SWDRVL3_L3_NO_ERROR;
                
                if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                {
                   printf("\r\n Enter DEV_SWDRVL3_PMGR_SetInetHostRoute");
                }
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
                shmem_data_p->host_added_in_chip_counter++;

#endif
                return_buf = DEV_SWDRVL3_PMGR_SetInetHostRoute(&dev_swdrvl3_host_route);
                if (return_buf != SWDRVL3_L3_NO_ERROR)
                {
                    ret = FALSE;
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n DEV_SWDRVL3_PMGR_SetInetHostRoute Error");
                    }
                }
                else
                {
                    ret = TRUE;
                }
                host_record->hw_info = dev_swdrvl3_host_route.hw_info;
                break;
           case L_HASH_ACTION_DEL:
                if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                {
                   printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteInetHostRoute");
                }
                
                ret = DEV_SWDRVL3_PMGR_DeleteInetHostRoute(&dev_swdrvl3_host_route);
                if (ret == FALSE)
                {
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n DEV_SWDRVL3_PMGR_DeleteInetHostRoute Error");
                    }
                }
                host_record->hw_info = dev_swdrvl3_host_route.hw_info;
                break;
           default:
                break;
        } /* end of switch */

        if (ret == TRUE)
        {
           if(action == L_HASH_ACTION_ADD)
           {
               event_type = L_HASH_SET_SUCCESS_EV;
           }
           else
           {
               event_type = L_HASH_DEL_SUCCESS_EV;
           }
        }
        else
        {
           event_type = L_HASH_FAIL_EV;
        }


        if (L_HASH_ShMem_OperationResult(&shmem_data_p->SWDRVL3_host_route_hash_desc, event_type, (UI8_T *)host_record) == FALSE)
        {
           SYSFUN_Debug_Printf("\n\r SWDRVL3: L_HASH_OperationResult for host route fails");
           /*Not finished all entries, so return value should be FALSE*/
           ret = FALSE;
           break;
        }

        /*if (process_number > SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS)*/
        if (process_number > shmem_data_p->max_num_process)
        {
           ret = FALSE;
           break;
        }
    } /* end of while */
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_ticker2_2 = SYSFUN_GetSysTick();
#endif

    return ret;
} /* end of SWDRVL3_ProcessHostRouetQueue() */



static BOOL_T SWDRVL3_ProcessTunnelHostRouteQueue(void)
{
    BOOL_T      l_hash_result = FALSE;
    BOOL_T      finished_processed = FALSE;
    UI32_T      process_number=0, event_type;
    BOOL_T      ret = FALSE;
    UI8_T       action;
    UI32_T      return_buf=DEV_SWDRVL3_L3_NO_ERROR;
    HostTunnelRoute_T *host_record;
    L_HASH_Index_T record_index;
    DEV_SWDRVL3_HostTunnel_T dev_swdrvl3_host_route;

    /* BODY
     */

    while (!finished_processed)
    {
        /* Process host route entries */
        l_hash_result = L_HASH_ShMem_DequeueJobList(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, (UI8_T**)&host_record, &action,&record_index);
        if (l_hash_result == FALSE)
        {
            if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
            {
                printf("\r\n L_HASH_DequeueJobList return FALSE");
            }
            /* If there is no job in queue, we think the work is finished, therefore, we should return TRUE */
            ret=TRUE;
            break;
        } /* end of if */

        memset(&dev_swdrvl3_host_route, 0, sizeof(DEV_SWDRVL3_HostTunnel_T));
        dev_swdrvl3_host_route.flags = host_record->flags;
        dev_swdrvl3_host_route.fib_id = host_record->fib_id;
        dev_swdrvl3_host_route.vid = host_record->dst_vid;
        dev_swdrvl3_host_route.unit = host_record->unit;
        dev_swdrvl3_host_route.port = host_record->port;
        dev_swdrvl3_host_route.trunk_id = host_record->trunk_id;
        dev_swdrvl3_host_route.hw_info = host_record->hw_info;
        memcpy(dev_swdrvl3_host_route.src_mac, host_record->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(dev_swdrvl3_host_route.mac, host_record->dst_mac, SYS_ADPT_MAC_ADDR_LEN);

        
        SWDRVL3_TUNNEL_INIT_TO_DEVDRV(dev_swdrvl3_host_route.tnl_init, host_record->tnl_init);
        SWDRVL3_TUNNEL_TERM_TO_DEVDRV(dev_swdrvl3_host_route.tnl_term, host_record->tnl_term);
        
        if (dev_swdrvl3_host_route.flags & SWDRVL3_FLAG_IPV6)
        {
            memcpy(dev_swdrvl3_host_route.ip_addr.ipv6_addr, host_record->ip_addr.ipv6_addr,
                SYS_ADPT_IPV6_ADDR_LEN);
        }
        else
        {
            dev_swdrvl3_host_route.ip_addr.ipv4_addr = host_record->ip_addr.ipv4_addr;
        }

#if 0
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
        shmem_data_p->process_counter2++;
#endif
#endif
        process_number++;
        switch(action)
        {
           
           case L_HASH_ACTION_ADD:
                
                
                if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                {
                   printf("\r\n Enter DEV_SWDRVL3_PMGR_SetInetHostRoute");
                }
#if 0                
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
                shmem_data_p->host_added_in_chip_counter++;

#endif
#endif
                return_buf = DEV_SWDRVL3_PMGR_AddInetHostTunnelRoute(&dev_swdrvl3_host_route);
                if (return_buf != DEV_SWDRVL3_L3_NO_ERROR)
                {
                    ret = FALSE;
                    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                    {
                        printf("\r\n DEV_SWDRVL3_PMGR_SetInetHostRoute Error");
                    }
                }
                else
                {
                    ret = TRUE;
                }
                host_record->hw_info = dev_swdrvl3_host_route.hw_info;
                host_record->tnl_init.l3_intf_id = dev_swdrvl3_host_route.tnl_init.l3_intf_id;
                break;
           case L_HASH_ACTION_DEL:
                if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLOUT) == TRUE)
                {
                   printf("\r\n Enter DEV_SWDRVL3_PMGR_DeleteInetHostRoute");
                }
                
                return_buf = DEV_SWDRVL3_PMGR_DeleteInetHostTunnelRoute(&dev_swdrvl3_host_route);
                if(return_buf != DEV_SWDRVL3_L3_NO_ERROR)
                {
                    if (ret == FALSE)
                    {
                        if (SDL3C_Debug(SDL3C_DEBUG_FLAG_ERRMSG) == TRUE)
                        {
                            printf("\r\n DEV_SWDRVL3_PMGR_DeleteInetHostRoute Error");
                        }
                    }
                }
                else
                {   
                    ret = TRUE;
                }
               
                host_record->hw_info = dev_swdrvl3_host_route.hw_info;
                host_record->tnl_init.l3_intf_id = dev_swdrvl3_host_route.tnl_init.l3_intf_id;
                break;
           default:
                break;
        } /* end of switch */

        if (ret == TRUE)
        {
           if(action == L_HASH_ACTION_ADD)
           {   
               event_type = L_HASH_SET_SUCCESS_EV;
           }
           else
           {   
               event_type = L_HASH_DEL_SUCCESS_EV;
           }
        }
        else
        {  
           event_type = L_HASH_FAIL_EV;
        }


        if (L_HASH_ShMem_OperationResult(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, event_type, (UI8_T *)host_record) == FALSE)
        {
           SYSFUN_Debug_Printf("\n\r SWDRVL3: L_HASH_OperationResult for host route fails");
           /*Not finished all entries, so return value should be FALSE*/
           ret = FALSE;
           break;
        }

        /*if (process_number > SWDRVL3_NUMBER_OF_ENTRY_TO_PROCESS)*/
        if (process_number > shmem_data_p->max_num_process)
        {
           ret = FALSE;
           break;
        }
    } /* end of while */
#if 0    
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    shmem_data_p->system_ticker2_2 = SYSFUN_GetSysTick();
#endif
#endif
    return ret;
} /* end of SWDRVL3_ProcessTunnelHostRouteQueue() */


/* Backdoor parameter/functions
 */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SDL3C_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void SDL3C_BACKDOOR_Main(void)
{

    SDL3C_BACKDOOR_Engine();

    return;
} /* End of SDL3C_BACKDOOR_Main */


static void SDL3C_BACKDOOR_Engine(void)
{
    BOOL_T  engine_continue;
    UI8_T   ch, cmd_index, cmd_buf_index;
    UI8_T   cmd_buf[SDL3C_BACKDOOR_CMD_MAXLEN + 1];

    engine_continue = TRUE;
    cmd_buf[0]      = 0;
    cmd_index       = 0;
    cmd_buf_index   = 0;
    while (engine_continue)
    {
        SDL3C_BACKDOOR_ExecuteCmd(cmd_buf);
        ch = getchar();

        switch (ch)
        {
            case KEY_ESC:       /* Go to the main menu */
                cmd_buf[0]      = 0;
                break;

            case KEY_BACKSPACE: /* Go to the up level menu */
                if (cmd_buf[0] != 0)
                    cmd_buf[strlen((char *)cmd_buf) - 1] = 0;
                break;

            case 'q':           /* Quit the engineering mode */
            case 'Q':
                engine_continue = FALSE;
                break;

            default:
                printf("%c", ch);
                SDL3C_BACKDOOR_ParseCmd(cmd_buf, ch);
                break;
        }

    }

    return;
} /* End of SDL3C_BACKDOOR_Engine */


/*
 * If { cmd_buf[] | {ch} } exists in the SDL3C_BACKDOOR_CommandTable
 * then cmd_buf[] |= ch;
 */
static void SDL3C_BACKDOOR_ParseCmd(UI8_T *cmd_buf, UI8_T ch)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen((char *)cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SDL3C_BACKDOOR_CMD_MAXLEN)
        return;

    cmd_buf[cmd_length]     = ch;
    cmd_buf[cmd_length + 1] = 0;

    while ((!cmd_found) && (SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str[0] != 255))
    {
        if (strcmp((char *)cmd_buf, SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (!cmd_found)
        cmd_buf[cmd_length] = 0;

    return;
} /* End of SDL3C_BACKDOOR_ParseCmd */


static void SDL3C_BACKDOOR_ExecuteCmd(UI8_T *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;
    BOOL_T  cmd_found;

    cmd_found   = FALSE;
    cmd_length  = (UI8_T)strlen((char *)cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SDL3C_BACKDOOR_CMD_MAXLEN)
        return;

    while ((!cmd_found) && (SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str[0]!=255))
    {
        if (strcmp((char *)cmd_buf, SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str) == 0)
            cmd_found = TRUE;
        else
        {
            cmd_index++;
        }
    }

    if (cmd_found)
    {
        printf("%s", SDL3C_BACKDOOR_ClearScreen);
        /* Print Title and Description */
        printf("\n\r%s    %s",  SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_title,
                                SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_description);
        printf("\n\r------------------------------------------------------------\n");
        SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_function(cmd_buf);
    }

    return;
} /* End of SDL3C_BACKDOOR_ExecuteCmd */


static void SDL3C_BACKDOOR_TerminateCmd(UI8_T *cmd_buf)
{
    /* Terminating */
    cmd_buf[strlen((char *)cmd_buf)-1] = 0;
    printf("\n\r");
    printf("\n\r[Esc]       : Main Menu");
    printf("\n\r[Back_Space]: Up Menu");
    printf("\n\rQ/q         : To quit");
    printf("\n\rPress any key to continue ...");

    return;
} /* End of SDL3C_BACKDOOR_TerminateCmd */


static void SDL3C_BACKDOOR_PrintMenu(UI8_T *cmd_buf)
{
    UI8_T   cmd_length, cmd_index;

    cmd_length  = (UI8_T)strlen((char *)cmd_buf);
    cmd_index   = 0;

    if (cmd_length >= SDL3C_BACKDOOR_CMD_MAXLEN)
        return;

    printf("\n\r");
    while ( SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str[0] != 255)
    {
        if (    ((UI8_T)(strlen(SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str) ) == (cmd_length + 1))
             && (strncmp((char *)SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str, (char *)cmd_buf, cmd_length) == 0)
           )
        {
            printf("\n\r%c : %-24s -- %-40s",
                    SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_str[cmd_length],
                    SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_title,
                    SDL3C_BACKDOOR_CommandTable[cmd_index].cmd_description
                  );
        }
        cmd_index++;
    }

    printf("\n\r");
    printf("\n\r%c : %-24s -- %-40s",'Q', "Quit", "To exit the SWDRVL3 engineering mode");
    printf("\n\r%c : %-24s -- %-40s",'q', "Quit", "To exit the SWDRVL3 engineering mode");
    printf("\n\r[Esc]       : Main Menu");
    printf("\n\r[Back_Space]: Up Menu");
    printf("\n\r");
    printf("\n\rSelect : ");

    return;
} /* End of SDL3C_BACKDOOR_PrintMenu */


/* ------------------------------------------------------------------------
 * Command Function
 */
static void   SDL3C_BACKDOOR_SetSpecialDefaultRoute(UI8_T *cmd_buf)
{
    SWDRVL3_Route_T default_route;
    UI32_T  action = 0;
    UI32_T  flags;
    UI8_T   buffer[10] = {0};
    BOOL_T  is_trunk = FALSE;
    BOOL_T  tagged_vlan = FALSE;

    memset(&default_route, 0, sizeof(SWDRVL3_Route_T));

    printf("\n\r Input Action: (1 = TRAP2CPU, 2 = DROP)  ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        action = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    if (action == 1)
        action = SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU;
    else if (action == 2)
        action = SYS_CPNT_DEFAULT_ROUTE_ACTION_DROP;
    else
        return;

    memset(buffer, 0, 10);

    printf("\n\r Input FIB ID: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        default_route.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0 ,10);

    printf("\n\r Input dst_vid: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        default_route.dst_vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    printf("\n\r Input TAGGED FRAME: (1 = tagged, 0 = untagged)");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        tagged_vlan = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    if (tagged_vlan == 1)
    {
        default_route.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    }

    memset(buffer, 0, 10);

    printf("\n\r Input unit: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        default_route.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    if(is_trunk == TRUE)
    {
        default_route.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        printf("\n\r Input trunk_id: ");
        if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
        {
            default_route.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 10);
    }
    else
    {
        printf("\n\r Input port: ");
        if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
        {
            default_route.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 10);
    }

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            default_route.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input next hop gateway IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&default_route.next_hop_ip.ipv4_addr);
        }
        else if (flags == 2)
        {
            default_route.flags |= SWDRVL3_FLAG_IPV6;
            
            printf("\n\r Input next hop gateway IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(default_route.next_hop_ip.ipv6_addr);
        }
        else
            return;
    }

    printf("\n\r Input next_hop_mac: ");
    SDL3C_BACKDOOR_GetMac(default_route.nexthop_mac);

    printf("\n\r Input src_mac: ");
    SDL3C_BACKDOOR_GetMac(default_route.src_mac);

    if (SWDRVL3_SetSpecialDefaultRoute(&default_route, action) == FALSE)
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "FALSE");
    }
    else
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "Success");
    }

    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_AddNetRoute(UI8_T *cmd_buf)
{
    SWDRVL3_Route_T     route;
    UI32_T  flags;
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[20] = {0};
    BOOL_T  is_trunk = FALSE;
    BOOL_T  tagged_vlan = FALSE;
    BOOL_T  is_local_connected = FALSE;
    BOOL_T  is_ecmp = FALSE;
    void * nh_hw_info;

    memset(&route, 0, sizeof(SWDRVL3_Route_T));

    memset(buffer, 0, 20);

    printf("\n\r Input virtual router ID: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    { 
        route.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0 ,20);

    printf("\n\r Input TAGGED FRAME: (1 = tagged, 0 = untagged)  ");

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        tagged_vlan = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    if (tagged_vlan == 1)
    {
        route.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    }

    memset(buffer, 0, 20);

    printf("\n\r Input dst_vid: ");

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.dst_vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    printf("\n\r Input is local connected route: (1 = YES, 0 = NO)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        if (SDL3C_BACKDOOR_AtoUl(buffer, 10) == 1)
            is_local_connected = TRUE;
    }

    if (is_local_connected == TRUE)
    {
        route.flags |= SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE;
        goto add_route;
    }

    memset(buffer, 0, 20);

    printf("\n\r Input is ECMP route: (1 = YES, 0 = NO)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_ecmp = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    if (is_ecmp == TRUE)
    {
        route.flags |= SWDRVL3_FLAG_ECMP;
    }

    memset(buffer, 0, 20);

    printf("\n\r Input unit: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    if(is_trunk == TRUE)
    {
        route.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        printf("\n\r Input trunk_id: ");
	BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            route.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }
    else
    {
        printf("\n\r Input port: ");
	BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            route.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }

    printf("\n\r Input next_hop_mac: ");
    SDL3C_BACKDOOR_GetMac(route.nexthop_mac);

    printf("\n\r Input src_mac: ");
    SDL3C_BACKDOOR_GetMac(route.src_mac);

add_route:
    memset(buffer, 0, 20);
    
    printf("\n\r Input Prefix_length: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.prefix_length = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    
    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            route.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input destination IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&route.dst_ip.ipv4_addr);
            printf("\n\r Input next hop gateway IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&route.next_hop_ip.ipv4_addr);
        }
        else if (flags == 2)
        {
            route.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input destination IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(route.dst_ip.ipv6_addr);
            printf("\n\r Input next hop gateway IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(route.next_hop_ip.ipv6_addr);
        }
        else
            return;
    }

    printf("\n\r Input nh_hw_info in hexidecimal format: ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    nh_hw_info = (void*)(uintptr_t)SDL3C_BACKDOOR_AtoUl(buffer, 16);
 
    ret = SWDRVL3_AddInetNetRoute(&route, nh_hw_info);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");
        printf("%s: Successful\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");
        printf("%s: FAILED (bucket full)\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");
        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");
        printf("%s: Others\n", __FUNCTION__);
    }
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_DeleteNetRoute(UI8_T *cmd_buf)
{
    SWDRVL3_Route_T     route;
    UI32_T  flags;
    UI8_T   buffer[20] = {0};
    BOOL_T  is_ecmp = FALSE;
    BOOL_T  is_trunk = FALSE;

    memset(&route, 0, sizeof(SWDRVL3_Route_T));

    memset(buffer, 0, 20);

    printf("\n\r Input virtual router ID: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    { 
        route.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0, 20);

    printf("\n\r Input is ECMP route: (1 = YES, 0 = NO)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_ecmp = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    if (is_ecmp == TRUE)
    {
        route.flags |= SWDRVL3_FLAG_ECMP;
    }
    else
    {
        goto delete_route;
    }

    memset(buffer, 0, 20);

    printf("\n\r Input dst_vid: ");

   BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.dst_vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    printf("\n\r Input unit: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    if(is_trunk == TRUE)
    {
        route.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        printf("\n\r Input trunk_id: ");
	BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            route.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }
    else
    {
        printf("\n\r Input port: ");
	BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            route.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }

    printf("\n\r Input next_hop_mac: ");
    SDL3C_BACKDOOR_GetMac(route.nexthop_mac);

delete_route:
    memset(buffer, 0, 20);
    
    printf("\n\r Input Prefix_length: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        route.prefix_length = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    
    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            route.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input destination IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&route.dst_ip.ipv4_addr);
            printf("\n\r Input next hop gateway IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&route.next_hop_ip.ipv4_addr);
        }
        else if (flags == 2)
        {
            route.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input destination IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(route.dst_ip.ipv6_addr);
            printf("\n\r Input next hop gateway IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(route.next_hop_ip.ipv6_addr);
        }
        else
            return;
    }

    if (SWDRVL3_DeleteInetNetRoute(&route, (void *)SWDRVL3_HW_INFO_INVALID) == FALSE)
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "FALSE");
    }
    else
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "Success");
    }

    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_AddHostRoute(UI8_T *cmd_buf)
{
    SWDRVL3_Host_T     host;
    UI32_T  flags;
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[10] = {0};
    BOOL_T  is_trunk = FALSE;
    BOOL_T  tagged_vlan = FALSE;

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    memset(buffer, 0, 10);

    BACKDOOR_MGR_Printf("\n\r Input virtual router ID: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    { 
        host.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0, 10);

    BACKDOOR_MGR_Printf("\n\r Input TAGGED FRAME: (1 = tagged, 0 = untagged)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    {
        tagged_vlan = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    if (tagged_vlan == 1)
    {
        host.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    }

    memset(buffer, 0, 10);

    BACKDOOR_MGR_Printf("\n\r Input dst_vid: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    {
        host.vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    BACKDOOR_MGR_Printf("\n\r Input unit: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    {
        host.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    BACKDOOR_MGR_Printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 10);

    if(is_trunk == TRUE)
    {
        host.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        BACKDOOR_MGR_Printf("\n\r Input trunk_id: ");
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
        {
            host.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 10);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Input port: ");
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
        {
            host.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 10);
    }

    BACKDOOR_MGR_Printf("\n\r Input next_hop_mac: ");
    SDL3C_BACKDOOR_GetMac(host.mac);

    BACKDOOR_MGR_Printf("\n\r Input src_mac: ");
    SDL3C_BACKDOOR_GetMac(host.src_mac);

    BACKDOOR_MGR_Printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            BACKDOOR_MGR_Printf("\n\r Input destination IPv4 address: ");
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            BACKDOOR_MGR_Printf("\n\r Input destination IPv6 address: ");
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }

    host.hw_info = (void *)SWDRVL3_HW_INFO_INVALID;

    ret = SWDRVL3_SetInetHostRoute(&host);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("%s: Successful\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("%s: FAILED (bucket full)\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("%s: FAILED (entry already exists)\n", __FUNCTION__);
    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("%s: Others\n", __FUNCTION__);
    }
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_ReadHostRoute(UI8_T *cmd_buf)
{
#if 0
    SWDRVL3_Host_T      host;
    UI32_T  flags;
    UI8_T   buffer[10] = {0};

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    printf("\n\r Input virtual router ID: ");
    if (SDL3C_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        host.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    
    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    if (SDL3C_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input IPv4 Host address: ");
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input IPv6 Host address: ");
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }

    if (SWDRVL3_ReadInetHostRoute(&host) == FALSE)
    {
        printf("\n\r");
        printf(__FUNCTION__": %s\n", "FALSE");
    }

    printf("\n      RESULT:\n ------------------------------------\n");
    printf("    dst_mac: ");
    SDL3C_BACKDOOR_DisplayMac(host.mac);
    printf("    src_mac: ");
    SDL3C_BACKDOOR_DisplayMac(host.src_mac);
    printf("    virtual router ID: %ld\n", host.vir_rt_id);
    printf("    Vlan ID:%ld\n", host.vid);

    if (host.flags & SWDRVL3_FLAG_TRUNK_EGRESS_PORT)
        printf("    trunk_id: %ld\n", host.trunk_id);
    else
    {
        printf("    unit: %ld\n", host.unit);
        printf("    port: %ld\n", host.port);
    }

    if (host.flags & SWDRVL3_FLAG_STATIC)
        printf("    is_static: YES\n");
    else
        printf("    is_static: NO\n");

    if (host.flags & SWDRVL3_FLAG_TAGGED_EGRESS_VLAN)
        printf("    is_tagged_vlan: YES\n");
    else
        printf("    is_tagged_vlan: NO\n");

    printf("    Host IP address: ");

    if (host.flags & SWDRVL3_FLAG_IPV6)
        SDL3C_BACKDOOR_DisplayIP6(host.ip_addr.ipv6_addr);
    else
        SDL3C_BACKDOOR_DisplayIP4(host.ip_addr.ipv4_addr);
#endif
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_DeleteHostRoute(UI8_T *cmd_buf)
{
    SWDRVL3_Host_T     host;
    UI32_T  flags;
    UI8_T   buffer[10] = {0};


    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    memset(buffer, 0, 10);

    printf("\n\r Input virtual router ID: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    { 
        host.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0, 10);

    printf("\n\r Input dst_vid: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        host.vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input IPv4 Host address: ");
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input IPv6 Host address: ");
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }

    if (SWDRVL3_DeleteInetHostRoute(&host) == FALSE)
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "FALSE");
    }
    else
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "Success");
    }

    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_ClearAllHostRoute(UI8_T *cmd_buf)
{
#if 0
    SWDRVL3_Host_T      host;
    UI32_T  flags, param_flags;
    UI8_T   buffer[10] = {0};

    memset(&host, 0, sizeof(SWDRVL3_Host_T));

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    if (SDL3C_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            param_flags |= SWDRVL3_FLAG_IPV4;
        }
        else if (flags == 2)
        {
            param_flags |= SWDRVL3_FLAG_IPV6;
        }
        else
            return;
    }

    if (SWDRVL3_ClearAllInetHostRoute(param_flags) == FALSE)
    {
        printf("\n\r");
        printf(__FUNCTION__": %s\n", "FALSE");
    }
    else
    {
        printf("\n\r");
        printf(__FUNCTION__": %s\n", "Success");
    }

#endif
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_ReadAndClearHostRouteHitBit(UI8_T *cmd_buf)
{
    SWDRVL3_Host_T      host;
    UI32_T  flags;
    UI32_T  hit_bit = 0;
    UI8_T   buffer[10] = {0};

    memset(&host, 0, sizeof(SWDRVL3_Host_T));
    memset(buffer, 0, 10);

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input IPv4 Host address: ");
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input IPv6 Host address: ");
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }

    if (SWDRVL3_ReadAndClearHostRouteEntryHitBit(&host, &hit_bit) == FALSE)
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "FALSE");
    }
    else
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "Success");
    }


    printf("%s: value of hit bit is %ld\n", __FUNCTION__, (long)hit_bit);
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}


static void   SDL3C_BACKDOOR_ReadAndClearNetRouteHitBit(UI8_T *cmd_buf)
{
    SWDRVL3_Route_T      net_route;
    UI32_T  flags;
    UI32_T  hit_bit = 0;
    UI8_T   buffer[10] = {0};

    memset(&net_route, 0, sizeof(SWDRVL3_Route_T));
    memset(buffer, 0, 10);

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            net_route.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input IPv4 Destination address: ");
            SDL3C_BACKDOOR_GetIP4(&net_route.dst_ip.ipv4_addr);
            
        }
        else if (flags == 2)
        {
            net_route.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input IPv6 Destination address: ");
            SDL3C_BACKDOOR_GetIP6(net_route.dst_ip.ipv6_addr);
        }
        else
            return;
    }

    printf("\n\r Input Prefix length: ");
    if (SDL3C_BACKDOOR_GetLine((char *)buffer, 10) > 0)
    {
        net_route.prefix_length = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    if (SWDRVL3_ReadAndClearNetRouteHitBit(&net_route, &hit_bit) == FALSE)
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "FALSE");
    }
    else
    {
        printf("\n\r");
        printf("%s: %s\n", __FUNCTION__, "Success");
    }


    printf("%s: value of hit bit is %ld\n", __FUNCTION__, (long)hit_bit);
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

#define SDL3C_BACKDOOR_UPORT_TO_IFINDEX(unit, port)         ( ((unit)-1) * (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) + (port)  )
static UI32_T SDL3C_BACKDOOR_OctetToLPort(UI8_T * octet, UI32_T comparing_length)

{

    UI32_T      index;

    UI32_T      value;

    UI8_T       tmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];



    memset(tmp, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (memcmp(tmp, octet, comparing_length) == 0)

        return 0;



    memcpy(tmp, octet, comparing_length);

    for (index = 0; index < comparing_length; index++)

    {

      value = index*8+1;

      while (!(tmp[index] & 0x80) && (tmp[index] != 0x00) )

      {

         value++;

         tmp [index] = (UI8_T) (tmp[index] << 1);

      }



      if (tmp[index] & 0x80)

         return (value);

   }



   return 0;



}

static void   SDL3C_BACKDOOR_AddHostTunnelRoute(UI8_T *cmd_buf)
{

  // 1. input host route information
  // 2. input tunnel initiator information
    SWDRVL3_HostTunnel_T     host;
    SWDRVL3_TunnelInitiator_T     tunnel;
    SWDRVL3_TunnelTerminator_T     tunnel_term;

    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[20] = {0};
    UI32_T  flags;
    BOOL_T  is_trunk = FALSE;
    BOOL_T  tagged_vlan = FALSE;
    UI32_T local_ifindex;

    memset(&host, 0, sizeof(SWDRVL3_HostTunnel_T));
    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelInitiator_T));
    memset(&tunnel_term, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r ================= Host Route ================");fflush(stdout);


    BACKDOOR_MGR_Printf("\n\r Input virtual router ID: ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    { 
        host.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r Input TAGGED FRAME: (1 = tagged, 0 = untagged)  ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        tagged_vlan = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }
    if (tagged_vlan == 1)
    {
        host.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    }

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r Input dst_vid: ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        host.vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r Input unit of the user port(uport): ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        host.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)  ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    if(is_trunk == TRUE)
    {
        host.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        BACKDOOR_MGR_Printf("\n\r Input trunk_id of the user port(uport): ");fflush(stdout);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            host.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Input port number of the user port(uport): ");fflush(stdout);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            host.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }

    BACKDOOR_MGR_Printf("\n\r Input next_hop_mac: ");fflush(stdout);
    SDL3C_BACKDOOR_GetMac(host.mac);

    BACKDOOR_MGR_Printf("\n\r Input src_mac: ");fflush(stdout);
    SDL3C_BACKDOOR_GetMac(host.src_mac);

    BACKDOOR_MGR_Printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input destination IPv4 address: ");fflush(stdout);
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input destination IPv6 address: ");fflush(stdout);
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }


    BACKDOOR_MGR_Printf("\n\r ================= Tunnel Initiator ================");fflush(stdout);

//    printf("\n\r Input next_hop_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(tunnel.nexthop_mac);
//
//    printf("\n\r Input src_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(tunnel.src_mac);
//
//    printf("\n\r Input Vlan ID: ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel.vid = (UI8_T)atoi(buffer);
//    memset(buffer, 0 ,20);
    memcpy(tunnel.nexthop_mac, host.mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(tunnel.src_mac, host.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    tunnel.vid = (UI8_T)(host.vid);

    BACKDOOR_MGR_Printf("\n\r Input TTL value: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel.ttl = (UI8_T)atoi((char*)buffer);

    memset(buffer, 0 ,20);

    BACKDOOR_MGR_Printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel.tunnel_type = (UI8_T)atoi((char*)buffer);

    memset(buffer, 0 ,20);

    BACKDOOR_MGR_Printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.sip.addr));

    BACKDOOR_MGR_Printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.dip.addr));

    memcpy(&(host.tnl_init), &tunnel, sizeof(SWDRVL3_TunnelInitiator_T));
   
  // 3. input tunnel temrinator information

    BACKDOOR_MGR_Printf("\n\r ================= Tunnel Terminator ================");

    memset(&tunnel_term, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 20);

//    printf("\n\r Input FIB ID: ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel_term.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 20);
    tunnel_term.fib_id = host.fib_id;

    memset(buffer, 0 ,20);

    local_ifindex = SDL3C_BACKDOOR_UPORT_TO_IFINDEX(host.unit, host.port);

    BACKDOOR_MGR_Printf("%s[%d]: local_ifindex=%ld\n",__FUNCTION__, __LINE__, (long)local_ifindex);
    memset(tunnel_term.lport+(local_ifindex-1)/8, 0x80 >> ((local_ifindex+7) % 8), 1);
    memset(tunnel_term.lport+(local_ifindex-1)/8+1, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST - (local_ifindex-1)/8 -1);

   UI32_T tmp = SDL3C_BACKDOOR_OctetToLPort(tunnel_term.lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    BACKDOOR_MGR_Printf("ifindex in port_list=%ld\n",(long)tmp);
//    printf("\n\r Input %d hexidecimal characters as port_bitmap: ",SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)lport,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
//    memcpy(tunnel_term.lport, lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    memset(buffer, 0 ,20);

//    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel_term.tunnel_type = (UI8_T)SDL3C_BACKDOOR_AtoUl(buffer, 20);
    tunnel_term.tunnel_type = tunnel.tunnel_type;

    memset(buffer, 0 ,20);

//    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);
//
//    SDL3C_BACKDOOR_GetIP4(tunnel_term.sip.addr);
    memcpy(tunnel_term.sip.addr, tunnel.dip.addr, SYS_ADPT_IPV4_ADDR_LEN);

    BACKDOOR_MGR_Printf("\n\r Input Tunnel Source IPv4 Prefix len:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel_term.sip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,20);

//    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);
//
//    SDL3C_BACKDOOR_GetIP4(tunnel_term.dip.addr);
    memcpy(tunnel_term.dip.addr, tunnel.sip.addr, SYS_ADPT_IPV4_ADDR_LEN);

    BACKDOOR_MGR_Printf("\n\r Input Tunnel Destination IPv4 Prefix len:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel_term.dip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,20);

    memcpy(&(host.tnl_term), &tunnel_term, sizeof(SWDRVL3_TunnelTerminator_T));

   ret = SWDRVL3_AddInetHostTunnelRoute(&host);

   if(ret == SWDRVL3_L3_NO_ERROR)
    {
        BACKDOOR_MGR_Printf("\n\r");fflush(stdout);
        BACKDOOR_MGR_Printf("%s: Successful with hw_info=%#lx and l3_intf_id=%ld\n", __FUNCTION__, (unsigned long)(uintptr_t)(host.hw_info), (long)host.tnl_init.l3_intf_id);fflush(stdout);
    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        BACKDOOR_MGR_Printf("\n\r");fflush(stdout);
        BACKDOOR_MGR_Printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);
    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        BACKDOOR_MGR_Printf("\n\r");fflush(stdout);
        BACKDOOR_MGR_Printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);
    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        BACKDOOR_MGR_Printf("\n\r");fflush(stdout);
        BACKDOOR_MGR_Printf("%s: Others\n", __FUNCTION__);fflush(stdout);
    }
 
   
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_DeleteHostTunnelRoute(UI8_T *cmd_buf)
{

  // 1. input host route information
  // 2. input tunnel initiator information
    SWDRVL3_HostTunnel_T     host;
    SWDRVL3_TunnelInitiator_T     tunnel;
    SWDRVL3_TunnelTerminator_T     tunnel_term;

    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[20] = {0};
    UI32_T  flags, local_ifindex;
    BOOL_T  is_trunk = FALSE;

    memset(&host, 0, sizeof(SWDRVL3_HostTunnel_T));
    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelInitiator_T));
    memset(&tunnel_term, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 20);

    printf("\n\r ================= Host Route ================");fflush(stdout);


    printf("\n\r Input virtual router ID: ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    { 
        host.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    } 

    memset(buffer, 0, 20);

//    printf("\n\r Input TAGGED FRAME: (1 = tagged, 0 = untagged)  ");fflush(stdout);
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    {
//        tagged_vlan = SDL3C_BACKDOOR_AtoUl(buffer, 10);
//    }
//    if (tagged_vlan == 1)
//    {
//        host.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
//    }
//
//    memset(buffer, 0, 20);
//
//    printf("\n\r Input dst_vid: ");fflush(stdout);
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    {
//        host.vid = SDL3C_BACKDOOR_AtoUl(buffer, 10);
//    }
//
//    memset(buffer, 0, 20);
//
    printf("\n\r Input unit of the user port(uport): ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        host.unit = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    printf("\n\r Input trunk or normal port: (1 = TRUNK, 0 = NORMAL)  ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        is_trunk = SDL3C_BACKDOOR_AtoUl(buffer, 10);
    }

    memset(buffer, 0, 20);

    if(is_trunk == TRUE)
    {
        host.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

        printf("\n\r Input trunk_id of the user port(uport): ");fflush(stdout);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            host.trunk_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }
    else
    {
        printf("\n\r Input port number of the user port(uport): ");fflush(stdout);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        {
            host.port = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        }

        memset(buffer, 0, 20);
    }

//    printf("\n\r Input next_hop_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(host.mac);
//
//    printf("\n\r Input src_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(host.src_mac);

    printf("\n\r Input type of IP: (1 = IPv4, 2 = IPv6)  ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        flags = SDL3C_BACKDOOR_AtoUl(buffer, 10);
        if (flags == 1)
        {
            host.flags |= SWDRVL3_FLAG_IPV4;

            printf("\n\r Input destination IPv4 address: ");fflush(stdout);
            SDL3C_BACKDOOR_GetIP4(&host.ip_addr.ipv4_addr);
        }
        else if (flags == 2)
        {
            host.flags |= SWDRVL3_FLAG_IPV6;

            printf("\n\r Input destination IPv6 address: ");fflush(stdout);
            SDL3C_BACKDOOR_GetIP6(host.ip_addr.ipv6_addr);
        }
        else
            return;
    }

    printf("\n\r Input hw_info in hexidecimal format: ");fflush(stdout);
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    host.hw_info = (void*)(uintptr_t)SDL3C_BACKDOOR_AtoUl(buffer, 16);
 
    printf("\n\r ================= Tunnel Initiator ================");fflush(stdout);

//    printf("\n\r Input next_hop_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(tunnel.nexthop_mac);
//
//    printf("\n\r Input src_mac: ");fflush(stdout);
//    SDL3C_BACKDOOR_GetMac(tunnel.src_mac);
//
//    printf("\n\r Input Vlan ID: ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel.vid = (UI8_T)atoi(buffer);
//    memset(buffer, 0 ,20);
//    memcpy(tunnel.nexthop_mac, host.mac, SYS_ADPT_MAC_ADDR_LEN);
//    memcpy(tunnel.src_mac, host.src_mac, SYS_ADPT_MAC_ADDR_LEN);
//    tunnel.vid = (UI8_T)(host.vid);
//
//    printf("\n\r Input TTL value: ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel.ttl = (UI8_T)atoi(buffer);
//
//    memset(buffer, 0 ,20);
//
//    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel.tunnel_type = (UI8_T)atoi(buffer);
//
//    memset(buffer, 0 ,20);
//
//    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);
//
//    SDL3C_BACKDOOR_GetIP4(tunnel.sip.addr);
//
//    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);
//
//    SDL3C_BACKDOOR_GetIP4(tunnel.dip.addr);
//
//    memcpy(&(host.tnl_init), &tunnel, sizeof(SWDRVL3_TunnelInitiator_T));

    printf("\n\r Input ID of the L3 interface the tunnel initiator is associated with: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    host.tnl_init.l3_intf_id  = SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0, 20);

   
  // 3. input tunnel temrinator information

    BACKDOOR_MGR_Printf("\n\r ================= Tunnel Terminator ================");

    memset(&tunnel_term, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 20);

//    printf("\n\r Input FIB ID: ");fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
//    tunnel_term.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 20);
    tunnel_term.fib_id = host.fib_id;

    memset(buffer, 0 ,20);

    local_ifindex = SDL3C_BACKDOOR_UPORT_TO_IFINDEX(host.unit, host.port);
    printf("%s[%d]: local_ifindex=%ld\n",__FUNCTION__, __LINE__, (long)local_ifindex);
    memset(tunnel_term.lport+(local_ifindex-1)/8, 0x80 >> ((local_ifindex+7) % 8), 1);
    memset(tunnel_term.lport+(local_ifindex-1)/8+1, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST - (local_ifindex-1)/8 -1);
 
//    printf("\n\r Input %d hexidecimal characters as port_bitmap: ",SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);fflush(stdout);
//
//    BACKDOOR_MGR_RequestKeyIn((char*)lport,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
//    memcpy(tunnel_term.lport, lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);


    memset(buffer, 0 ,20);

    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel_term.tunnel_type = (UI8_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0 ,20);

    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel_term.sip.addr));

    printf("\n\r Input Tunnel Source IPv4 Mask:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel_term.sip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,20);

    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel_term.dip.addr));

    printf("\n\r Input Tunnel Destination IPv4 Mask:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel_term.dip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,20);

    memcpy(&(host.tnl_term), &tunnel_term, sizeof(SWDRVL3_TunnelTerminator_T));


    ret = SWDRVL3_DeleteInetHostTunnelRoute(&host);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Host Tunnel Route deletion Successful\n", __FUNCTION__);fflush(stdout);


    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Others\n", __FUNCTION__);fflush(stdout);

    }
 
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void SDL3C_BACKDOOR_AddTunnelIntfL3(UI8_T *cmd_buf)
{
    SWDRVL3_TunnelIntfL3_T  tl3;
    UI32_T                  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T                   buffer[20] = {0};

    memset(&tl3, 0, sizeof(tl3));

    BACKDOOR_MGR_Printf("\n\r Input src_mac: ");
    SDL3C_BACKDOOR_GetMac(tl3.src_mac);

    BACKDOOR_MGR_Printf("\n\r Input Vlan ID: ");
    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tl3.vid = (UI8_T)atoi((char*)buffer);
    tl3.is_add = TRUE;

    ret = SWDRVL3_AddTunnelIntfL3(&tl3);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        BACKDOOR_MGR_Printf("%s: Successful with L3 Interface ID %ld\n",
            __FUNCTION__, (long)tl3.l3_intf_id);
    }
    else
    {
        BACKDOOR_MGR_Printf("%s: Others\n", __FUNCTION__);
    }

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
}

static void   SDL3C_BACKDOOR_AddTunnelInitiator(UI8_T *cmd_buf)
{
    SWDRVL3_TunnelInitiator_T     tunnel;
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[20] = {0};

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelInitiator_T));

    memset(buffer, 0, 20);

    BACKDOOR_MGR_Printf("\n\r Input next_hop_mac: ");
    SDL3C_BACKDOOR_GetMac(tunnel.nexthop_mac);

    BACKDOOR_MGR_Printf("\n\r Input src_mac: ");
    SDL3C_BACKDOOR_GetMac(tunnel.src_mac);

    printf("\n\r Input Vlan ID: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel.vid = (UI8_T)atoi((char*)buffer);
    memset(buffer, 0 ,20);

    printf("\n\r Input TTL value: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel.ttl = (UI8_T)atoi((char*)buffer);

    memset(buffer, 0 ,20);

    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,20);
    tunnel.tunnel_type = (UI8_T)atoi((char*)buffer);

    memset(buffer, 0 ,20);

    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)tunnel.sip.addr);

    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)tunnel.dip.addr);

    ret = SWDRVL3_AddTunnelInitiator(&tunnel);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Successful with L3 Interface of %ld\n", __FUNCTION__,(long)tunnel.l3_intf_id);fflush(stdout);


    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Others\n", __FUNCTION__);fflush(stdout);

    }
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_DeleteTunnelInitiator(UI8_T *cmd_buf)
{
    UI32_T  l3_intf_id;
    UI8_T   buffer[10] = {0};
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;

    memset(buffer, 0, 10);

    printf("\n\r Input ID of the L3 interface the tunnel initiator is associated with: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    l3_intf_id  = SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0, 10);

    ret = SWDRVL3_DeleteTunnelInitiator(l3_intf_id);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Successful with L3 Interface of %ld\n", __FUNCTION__, (long)l3_intf_id);fflush(stdout);


    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Others\n", __FUNCTION__);fflush(stdout);

    }
 
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_AddTunnelTerminator(UI8_T *cmd_buf)
{
    SWDRVL3_TunnelTerminator_T     tunnel;
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[10] = {0};
    UI8_T lport[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0}; 

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 10);

    printf("\n\r Input FIB ID: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0 ,10);

    printf("\n\r Input %d hexidecimal characters as port_bitmap: ",SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)lport,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(tunnel.lport, lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.tunnel_type = (UI8_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.sip.addr));

    printf("\n\r Input Tunnel Source IPv4 Prefix len:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.sip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.dip.addr));

    printf("\n\r Input Tunnel Destination IPv4 Prefix len:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.dip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,10);

    ret = SWDRVL3_AddTunnelTerminator(&tunnel);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Successful\n", __FUNCTION__);fflush(stdout);


    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Others\n", __FUNCTION__);fflush(stdout);

    }
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;

}

static void   SDL3C_BACKDOOR_DeleteTunnelTerminator(UI8_T *cmd_buf)
{
    SWDRVL3_TunnelTerminator_T     tunnel;
    UI32_T  ret = SWDRVL3_L3_NO_ERROR;
    UI8_T   buffer[10] = {0};
    UI8_T lport[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0}; 

    memset(&tunnel, 0, sizeof(SWDRVL3_TunnelTerminator_T));

    memset(buffer, 0, 10);

    printf("\n\r Input FIB ID: ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.fib_id = SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0 ,10);

    printf("\n\r Input %d hexidecimal characters as port_bitmap: ",SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)lport,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(tunnel.lport, lport, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);


    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Type : (1 = Manual, 2 = 6To4, 3 = ISATAP)  ");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.tunnel_type = (UI8_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);

    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Source IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.sip.addr));

    printf("\n\r Input Tunnel Source IPv4 Mask:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.sip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,10);

    printf("\n\r Input Tunnel Destination IPv4 Address:");fflush(stdout);

    SDL3C_BACKDOOR_GetIP4((UI32_T*)(tunnel.dip.addr));

    printf("\n\r Input Tunnel Destination IPv4 Mask:");fflush(stdout);

    BACKDOOR_MGR_RequestKeyIn((char*)buffer,10);
    tunnel.dip.preflen = (UI16_T)SDL3C_BACKDOOR_AtoUl(buffer, 10);
    memset(buffer, 0 ,10);

    ret = SWDRVL3_DeleteTunnelTerminator(&tunnel);

    if(ret == SWDRVL3_L3_NO_ERROR)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Successful\n", __FUNCTION__);fflush(stdout);


    }
    else if(ret == SWDRVL3_L3_BUCKET_FULL)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (bucket full)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_EXISTS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: FAILED (entry already exists)\n", __FUNCTION__);fflush(stdout);

    }
    else if(ret == SWDRVL3_L3_OTHERS)
    {
        printf("\n\r");fflush(stdout);

        printf("%s: Others\n", __FUNCTION__);fflush(stdout);

    }
    
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;


}



// =======================================================================

static void SDL3C_BACKDOOR_ShowDebugFlag(UI8_T *cmd_buf)
{
    UI32_T  flag;

    SDL3C_GetDebugFlag(&flag);
    printf("\n\rDebugFlag = %08lx", (unsigned long)flag);

    /* Terminating */
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);

    return;
} /* End of SDL3C_BACKDOOR_ShowDebugFlag */

static void SDL3C_BACKDOOR_DisplayHostCounter(UI8_T* cmd_buf)
{
    printf("\n\rhost counter (add:%d, delete:%d)", (int)shmem_data_p->host_added_counter, (int)shmem_data_p->host_deleted_counter);
    /* Terminating */
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}   /* end of SDL3C_BACKDOOR_DisplayHostCounter */

static void SDL3C_BACKDOOR_DisplayNetCounter(UI8_T* cmd_buf)
{
    printf("\n\rnet counter (add:%d, delete:%d)",(int) shmem_data_p->net_added_counter, (int)shmem_data_p->net_deleted_counter);
    /* Terminating */
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}   /* end of SDL3C_BACKDOOR_DisplayNetCounter */

static void   SDL3C_BACKDOOR_DisplayLhashCounter(UI8_T *cmd_buf)
{
    UI8_T     temp_buffer[20] = {0};
    UI32_T    record_count = 0;

    printf("\n\r Enter Number of Host route in L_Hash: ");
    if (SDL3C_BACKDOOR_GetLine((char *)temp_buffer, 20) > 0)
    {
        record_count = SDL3C_BACKDOOR_AtoUl(temp_buffer, 10);
    }
    L_HASH_ShMem_GetNRecord(&shmem_data_p->SWDRVL3_host_route_hash_desc, record_count);

    record_count = 0;

    printf("\n\r Enter Number of Net route in L_Hash: ");
    if (SDL3C_BACKDOOR_GetLine((char *)temp_buffer, 20) > 0)
    {
        record_count = SDL3C_BACKDOOR_AtoUl(temp_buffer, 10);
    }
    L_HASH_ShMem_GetNRecord(&shmem_data_p->SWDRVL3_net_route_hash_desc, record_count);

    printf("\n\r Enter Number of Tunnel Host route in L_Hash: ");
    if (SDL3C_BACKDOOR_GetLine((char *)temp_buffer, 20) > 0)
    {
        record_count = SDL3C_BACKDOOR_AtoUl(temp_buffer, 10);
    }
    L_HASH_ShMem_GetNRecord(&shmem_data_p->SWDRVL3_host_tunnel_route_hash_desc, record_count);
    /* Terminating */
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}   /* end of SDL3C_BACKDOOR_DisplayLhashCounter */


static void SDL3C_BACKDOOR_SetDebugFlag(UI8_T *cmd_buf)
{
    UI8_T   i, select, value, ch;
    UI32_T  flag, set_flag;
    UI8_T   *status;

    i = 1;
    SDL3C_GetDebugFlag(&flag);
    while (SDL3C_BACKDOOR_DebugFlagTable[i].flag < (UI32_T)SDL3C_DEBUG_FLAG_ALL)
    {
        /* Print the menu and the current status */
        if ((flag & SDL3C_BACKDOOR_DebugFlagTable[i].flag) != 0)
        {
            status = (UI8_T *)"ON";
        }
        else
        {
            status = (UI8_T *)"OFF";
        }
        printf("\n\r%2d -- %-32s : %-3s", i, SDL3C_BACKDOOR_DebugFlagTable[i].flag_string, status);
        i++;
    }
    printf("\n\r");
    printf("\n\rSelect (0 = end) : ");
    ch = getchar();
    printf("%c", ch);

    select = (UI8_T)(ch - '0');
    set_flag = (UI32_T)SDL3C_BACKDOOR_DebugFlagTable[select].flag;
    if (select != 0 && select < i)
    {
        printf("          Set (1 = ON, 0 = OFF) : ");
        ch = getchar();
        printf("%c", ch);
        value = (UI8_T)(ch - '0');
        if (value)
        {
            SDL3C_SetDebugFlag(flag | set_flag);
        }
        else
        {
            SDL3C_SetDebugFlag(flag & ~set_flag);
        }
    }
    else
    {
        /* Terminating */
        SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    }

    return;
} /* End of SDL3C_BACKDOOR_SetDebugFlag */


static int SDL3C_BACKDOOR_GetLine(char s[], int lim)
{
    int c, i;

    /*for (i=0; i<lim-1 && (c=getchar()) != 0 && c!='\n'; ++i)*/
    for (i=0; i<lim-1 && (c=getchar()) != 0 && c!='\r'; ++i)
    {
        s[i] = c;
        printf("%c", c);
    }
    s[i] = '\0';
    return i;
} /* End of SDL3C_BACKDOOR_GetLine */

/*****************************************/
/* Convert string 's' to unsigned long,  */
/* with 'radix' as base.                 */
/*****************************************/

static UI32_T SDL3C_BACKDOOR_AtoUl(UI8_T *s, int radix)
{
    int i;
    unsigned long n = 0;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
        ;       /* skip white space */

    if (s[i] == '+' || s[i] == '-')
    {
        i++;    /* skip sign */
    }

    if (radix == 10)
    {
        for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
        {
            n = 10 * n + s[i] - '0';
        }
    }
    else if (radix == 16)
    {
        if ( (s[i] == '0') && (s[i+1] == 'x' || s[i+1] == 'X') ) /* Charles,*/
           i=i+2;                                                /* To skip the "0x" or "0X" */


        for (n = 0;
            (s[i] >= '0' && s[i] <= '9') ||
            (s[i] >= 'A' && s[i] <= 'F') ||
            (s[i] >= 'a' && s[i] <= 'f');
            i++)
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                n = 16 * n + s[i] - '0';
            }
            else if (s[i] >= 'A' && s[i] <= 'F')
            {
                n = 16 * n + s[i] - 'A'+ 10;
            }
            else if (s[i] >= 'a' && s[i] <= 'f')
            {
                n = 16 * n + s[i] - 'a'+ 10;
            }
        }
    }
    return (n);
}


static void SDL3C_BACKDOOR_GetMac(UI8_T * mac)
{
    int i;
    UI8_T   buffer[10] = {0};

    for(i = 0; i < 5; i++)
    {
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 3);
        {
            *mac = SDL3C_BACKDOOR_AtoUl(buffer, 16);
        }
        BACKDOOR_MGR_Printf(":");
        mac++;
    }

    memset(buffer, 0, 10);
    
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 3);
        *mac = SDL3C_BACKDOOR_AtoUl(buffer, 16);
    BACKDOOR_MGR_Printf("\n");

    return;
}

static int SDL3C_BACKDOOR_AtoIP4(UI8_T *s, UI8_T *ip)
{
        UI8_T token[20];
        int   i,j;  /* i for s[]; j for token[] */
        int   k;    /* k for ip[] */
    
        UI8_T temp[4];
    
        i = 0;
        j = 0;
        k = 0;
    
        while (s[i] != '\0')
        {
            if (s[i] == '.')
            {
                token[j] = '\0';
                if (strlen((char *)token) < 1 || strlen((char *)token) > 3 ||
                    atoi((char *)token) < 0 || atoi((char *)token) > 255)
                {
                    return 0;
                }
                else if (k >= 4)
                {
                    return 0;
                }
                else
                {
                    temp[k++] =(UI8_T)atoi((char *)token);
                    i++; j = 0;
                }
            }
            else if (!(s[i] >= '0' && s[i] <= '9'))
            {
                return 0;
            }
            else
            {
                token[j++] = s[i++];
            }
    
        } /* while */
    
        token[j] = '\0';
        if (strlen((char *)token) < 1 || strlen((char *)token) > 3 ||
            atoi((char *)token) < 0 || atoi((char *)token) > 255)
        {
            return 0;
        }
        else if (k != 3)
        {
            return 0;
        }
    
        temp[k]=(UI8_T)atoi((char *)token);
        
        ip[0] = temp[0];
        ip[1] = temp[1];
        ip[2] = temp[2];
        ip[3] = temp[3];
    
        return 1;

}

static void SDL3C_BACKDOOR_GetIP4(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[20] = {0};

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        ret = SDL3C_BACKDOOR_AtoIP4(buffer, temp_ip);
    }

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered a invalid IP address\n");
        return;
    }

    BACKDOOR_MGR_Printf("\n");

    return;
}

// Convert Hexidecimal ASCII digits to integer
static int SDL3C_BACKDOOR_AHtoI(char *token)
{
  int result=0, value_added=0, i=0;

   do {
   if((*(token+i) >= '0') && (*(token+i) <= '9'))
    value_added = (int) (*(token+i) - 48);
   else if((*(token+i) >= 'a') && (*(token+i) <= 'f'))
    value_added = (int) (*(token+i) - 87);
   else if((*(token+i) >= 'A') && (*(token+i) <= 'F'))
    value_added = (int) (*(token+i) - 55);
   else
    return -1;
   result = result * 16 + value_added;
   i++;
  } while(*(token+i) != '\0');

   if(result < 0 || result > 255)
    return -1;
   return result;
}

static int SDL3C_BACKDOOR_AtoIPV6(UI8_T *s, UI8_T *ip)
{
        UI8_T token[50];
        int   i,j;  /* i for s[]; j for token[] */
        int   k,l;  /* k for ip[]; l for copying coutner */
    
        UI8_T temp[20];
    
        i = 0;
        j = 0;
        k = 0;
    	l = 0;

        while (s[i] != '\0')
    {
            if ((s[i] == ':') || (j == 2))
        {
		 token[j] = '\0';
        
                if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
                    SDL3C_BACKDOOR_AHtoI((char *)token) < 0 || SDL3C_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
                    return 0;
                else if (k >= 16)  // Too many digits
                    return 0;
                else // token is ready
        {
                    temp[k++] =(UI8_T)SDL3C_BACKDOOR_AHtoI((char *)token);
		    if(s[i] == ':')
                     i++; 
            j = 0;
        }
    }
            else if ((s[i] < '0' || s[i] > '9') && (s[i] < 'a' || s[i] > 'f') && (s[i] < 'A' || s[i] > 'F'))
                return 0;
            else
                token[j++] = s[i++];
        } /* while */
    
        token[j] = '\0';

        if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
            SDL3C_BACKDOOR_AHtoI((char *)token) < 0 || SDL3C_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
            return 0;
        else if (k >= 16)  // Too many digits
            return 0;
 
   
        temp[k]=(UI8_T)SDL3C_BACKDOOR_AHtoI((char *)token);
        
        for(l=0;l<16;l++)
         ip[l] = temp[l];
    
        return 1;

}

static void SDL3C_BACKDOOR_GetIP6(UI8_T * IPaddr)
{
    int ret = 0;
//    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[50] = {0};

    //if(AMTRL3_BACKDOOR_GetLine((char*)buffer, 20) > 0)
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 50);

    if(strlen((char*)buffer) != 39)
     ret = 0;
    else
     ret = SDL3C_BACKDOOR_AtoIPV6(buffer, IPaddr);

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IPv6 address\n");
        return;
    }
    
    //BACKDOOR_MGR_Printf("\n");

    return;
}



//static void SDL3C_BACKDOOR_GetIP6(UI8_T *IP6addr)
//{
//    int i;
//    int j = 0;
//    UI8_T   buffer[20] = {0};
//
//    for(i = 0; i < 15; i++)
//    {
//        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 3);
//        {
//            *IP6addr = SDL3C_BACKDOOR_AtoUl(buffer, 16);
//        }
//        
//        IP6addr++;
//        j += 1;
//
//        if ((j % 2) == 0)
//        {
//            j = 0;
//            BACKDOOR_MGR_Printf(":");
//        }
//    }
//
//    memset(buffer, 0, 20);
//    
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 3);
//        *IP6addr = SDL3C_BACKDOOR_AtoUl(buffer, 16);
//    BACKDOOR_MGR_Printf("\n");
//
//    return;
//}

/* for removal warnig */
#if 0
static void SDL3C_BACKDOOR_DisplayMac(UI8_T *mac)
{
    int i;

    for(i = 0; i < 5; i++)
        printf("%2.2x:", mac[i]);

    printf("%2.2x\n", mac[5]);

    return;
}

static void SDL3C_BACKDOOR_DisplayIP6(UI8_T *ipaddr)
{
    int i;
    UI16_T  *ipaddr_f;

    ipaddr_f = (UI16_T *)ipaddr;

    for (i = 0; i < 7; i++)
        printf("%4.4x:", ipaddr_f[i]);

    printf("%4.4x\n", ipaddr_f[7]);

    return;
}
#endif

/* for removal warning */
#if 0
static void SDL3C_BACKDOOR_DisplayIP4(UI32_T ipaddr)
{
    int i;
    UI8_T *ipaddr_f;

    ipaddr_f = (UI8_T *)&ipaddr;

    for(i = 0; i < 3; i++)
        printf("%d.", ipaddr_f[i]);

    printf("%d\n", ipaddr_f[3]);

    return;
}
#endif

static void   SDL3C_BACKDOOR_SetNumMaxProcess(UI8_T *cmd_buf)
{
#if 1
    UI8_T     temp_buffer[20] = {0};

    printf("\n\r Enter Number of Max Process Entries: ");
    if (SDL3C_BACKDOOR_GetLine((char *)temp_buffer, 20) > 0)
    {
    	SWDRVL3_EnterCriticalSection();
        shmem_data_p->max_num_process = SDL3C_BACKDOOR_AtoUl(temp_buffer, 10);
		SWDRVL3_LeaveCriticalSection();
    }
    printf("\n\r Enter Number of Max Process Entries: %d ", (int)shmem_data_p->max_num_process);
#endif
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* End of SDL3C_BACKDOOR_SetNumMaxProcess */

static BOOL_T SDL3C_Debug(UI32_T flag)
{
    return ( (shmem_data_p->SDL3C_DebugFlag & flag) != 0);
} /* End of SDL3C_Debug() */


static void   SDL3C_SetDebugFlag(UI32_T flag)
{
    SWDRVL3_EnterCriticalSection();
    shmem_data_p->SDL3C_DebugFlag    = flag;
    SWDRVL3_LeaveCriticalSection();
    return;
} /* End of SDL3C_SetDebugFlag() */


static void   SDL3C_GetDebugFlag(UI32_T *flag)
{
    *flag   = shmem_data_p->SDL3C_DebugFlag;
    return;
} /* End of SDL3C_GetDebugFlag() */

#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)

static void SWDRVL3_CreateBackdoorTask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    if (SYSFUN_SpawnTask (SYS_BLD_SWDRVL3_BACKDOOR_TASK,
                          39,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          SWDRVL3_Backdoor_Task_Main,
                          0,
                          &shmem_data_p->swdrvl3_task_backdoor_tid))
    {
        owner_info.level = SYSLOG_LEVEL_CRIT;
        owner_info.module_no = SYS_MODULE_SWDRVL3;
        owner_info.function_no = SWDRVL3_TASK_Create_Tasks_FunNo;
        owner_info.error_no = SWDRVL3_TASK_Create_Tasks_ErrNo;
#if(SYS_CPNT_SYSLOG == TRUE)
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, SYS_BLD_SWDRVL3_BACKDOOR_TASK, 0, 0);
#endif
        return;
    } /* end of if */
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_Backdoor_Task_Main
 * -------------------------------------------------------------------------
 * FUNCTION: This function will collect counters from LAN, IML, AMTRL3 and,
 *           SWDRVL3 periodically.
 *           SWDRVL3 Backdoor Task monitors: Timer Event.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void SWDRVL3_Backdoor_Task_Main(void)
{
    UI32_T      wait_events, all_events = 0;
    UI32_T      current_state;

    /* start periodic timer
     */
    if (SYSFUN_StartPeriodicTimerEvent (SWDRVL3_BACKDOOR_TIME_TICK_INTERVAL, SWDRVL3_TIMER_EVENT, 0, &shmem_data_p->swdrvl3_backdoor_task_tmid)
        != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\n\r SWDRVL3: create periodic swdrvl3 backdoor timer error.");
    } /* end of if */

    /* Task Body
     */
    while (TRUE)
    {
        /* wait event */
        SYSFUN_ReceiveEvent (SWDRVL3_TIMER_EVENT,
                             SYSFUN_EVENT_WAIT_ANY,
                             (all_events)? SYSFUN_TIMEOUT_NOWAIT:SYSFUN_TIMEOUT_WAIT_FOREVER, /* timeout */
                             &wait_events);

        all_events |= wait_events;

        if ( all_events == 0 )
            continue;

        current_state = SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p); 

        if ((current_state != SYS_TYPE_STACKING_MASTER_MODE) && (current_state != SYS_TYPE_STACKING_SLAVE_MODE))
        {
            all_events = 0;
            continue;
        }

        /* periodic timer event arrival
         */
        if (wait_events & SWDRVL3_TIMER_EVENT)
        {
           if ((shmem_data_p->system_counter_enable != FALSE) && (shmem_data_p->system_performance_counters_index < 200))
           {
              SDL3C_BACKDOOR_GetCounters();
           }

           all_events ^= SWDRVL3_TIMER_EVENT;
        } /* end of if */
    } /* end of while(TRUE) */

    return;
} /* end of SWDRVL3_Backdoor_Task_Main() */

static void SDL3C_BACKDOOR_GetCounters(void)
{
    UI32_T    al3_add_host_in_counters = 0;
    UI32_T    al3_add_host_out_counters = 0;
    UI32_T    ip_packet_from_lan = 0;
    UI32_T    ip_packet_to_p2 = 0;
    UI32_T    arp_packet_from_lan = 0;
    UI32_T    arp_packet_to_p2 = 0;
    UI32_T    arp_packet_to_amtrl3 = 0;
    UI32_T    queue_full_drop = 0;
    UI32_T    lan_in_pkts_counter = 0;
    UI32_T    lan_drop_pkts_counter = 0;
    UI32_T    lan_announced_counter = 0;
    UI32_T    l2mux_announce_pkts_counter = 0;

    /* Get counters from LAN */
    LAN_GetSystemPerformanceCounter(&lan_in_pkts_counter, &lan_drop_pkts_counter, &lan_announced_counter);

    /* Get counters from L2_MUX */
    L2MUX_MGR_GetCounter(&l2mux_announce_pkts_counter);

    /* Get counters from IML */
    IML_MGR_GetSystemPerformanceCounter(&ip_packet_from_lan, &ip_packet_to_p2,
                                        &arp_packet_from_lan, &arp_packet_to_p2,
                                        &arp_packet_to_amtrl3, &queue_full_drop);

    /* Get counters from AMTRL3 */
    al3_add_host_in_counters = AMTRL3_MGR_GetAL3AddHostCounter();
    al3_add_host_out_counters = AMTRL3_MGR_GetSwdrvL3AddHostCounter();

    /* Get counters from SWDRVL3_CACHE */


	SWDRVL3_EnterCriticalSection();
    /* Put the variable into the system_performance_counters table */
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][0] = lan_in_pkts_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][1] = lan_drop_pkts_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][2] = lan_announced_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][3] = l2mux_announce_pkts_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][4] = ip_packet_from_lan;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][5] = ip_packet_to_p2;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][6] = arp_packet_from_lan;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][7] = arp_packet_to_p2;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][8] = arp_packet_to_amtrl3;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][9] = queue_full_drop;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][10] = al3_add_host_in_counters;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][11] = al3_add_host_out_counters;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][12] = shmem_data_p->host_added_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][13] = shmem_data_p->host_added_in_chip_counter;
    shmem_data_p->system_performance_counters[shmem_data_p->system_performance_counters_index][14] = shmem_data_p->host_added_in_chip_counter;

    shmem_data_p->system_performance_counters_index++;
	SWDRVL3_LeaveCriticalSection();
    return;
} /* SDL3C_BACKDOOR_GetCounters */


static void SDL3C_BACKDOOR_ClearSystemCounter(UI8_T *cmd_buf)
{
    /* Clear counters in LAN */
    LAN_ClearSystemPerformanceCounter();

    /* Clear counters in L2_MUX */
    L2MUX_MGR_ClearCounter();

    /* Clear counters in IML */
    IML_MGR_ClearSystemPerformanceCounter();

    /* Clear counters in AMTRL3 */
    AMTRL3_MGR_ClearAddHostDebugCounter();

    /* Clear counters in SWDRVL3_CACHE */
	SWDRVL3_EnterCriticalSection();
    shmem_data_p->host_added_counter = 0;
    shmem_data_p->host_added_in_chip_counter = 0;
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* SDL3C_BACKDOOR_ClearSystemCounter */


static void   SDL3C_BACKDOOR_ShowSystemCounter(UI8_T *cmd_buf)
{
    int i;
    printf("\n\r  LAN   |                  IML                   |       AL3         |       SD3C       |");
    printf("\n\r  Ip |  Drop |  Ann |      IP       |    ARP         |  Drop |   In    |   Out   |   In   |   Out   |");
    printf("\n\r        |  In   |  Out  |   In   |  Out  |       |         |         |        |         |");
    for (i=0 ; i < shmem_data_p->system_performance_counters_index; i++)
    {
        printf("\n\r  %8d | %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d |", shmem_data_p->system_performance_counters[i][0],
                        shmem_data_p->system_performance_counters[i][1],
                        shmem_data_p->system_performance_counters[i][2],
                        shmem_data_p->system_performance_counters[i][3],
                        shmem_data_p->system_performance_counters[i][4],
                        shmem_data_p->system_performance_counters[i][5],
                        shmem_data_p->system_performance_counters[i][6],
                        shmem_data_p->system_performance_counters[i][8],
                        shmem_data_p->system_performance_counters[i][9],
                        shmem_data_p->system_performance_counters[i][10],
                        shmem_data_p->system_performance_counters[i][11],
                        shmem_data_p->system_performance_counters[i][12],
                        shmem_data_p->system_performance_counters[i][13]);
    }

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* End of SDL3C_BACKDOOR_ShowSystemCounter */

static void SDL3C_BACKDOOR_EnableSystemCounter(UI8_T *cmd_buf)
{
	SWDRVL3_EnterCriticalSection();
    shmem_data_p->system_counter_enable = ~shmem_data_p->system_counter_enable;

    if (shmem_data_p->system_counter_enable == TRUE)
       printf("\n\r System Counter Enable");
    else if (shmem_data_p->system_counter_enable == FALSE)
       printf("\n\r System Counter Disable");
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* End of SDL3C_BACKDOOR_EnableSystemCounter */


static void SDL3C_Clear_SystemPerformanceCounter(UI8_T *cmd_buf)
{
    int i, j;

	SWDRVL3_EnterCriticalSection();
    shmem_data_p->system_performance_counters_index = 0;

    for (i = 0; i<200; i++)
        for (j = 0; j < 15; j++)
            shmem_data_p->system_performance_counters[i][j] = 0;
	SWDRVL3_LeaveCriticalSection()

    return;
}

static void   SDL3C_BACKDOOR_ShowProcessCounter(UI8_T *cmd_buf)
{
	SWDRVL3_EnterCriticalSection();
    printf("\n\rProcess Counter for net route: %d", shmem_data_p->process_counter1);
    printf("\n\rProcess Counter for host route: %d", shmem_data_p->process_counter2);
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_ClearProcessCounter(UI8_T *cmd_buf)
{
	SWDRVL3_EnterCriticalSection();
    shmem_data_p->process_counter1 = 0;
    shmem_data_p->process_counter2 = 0;
	SWDRVL3_LeaveCriticalSection();

    printf("\n\rProcess Counter has been cleared!");
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void   SDL3C_BACKDOOR_ShowSystemTick(UI8_T *cmd_buf)
{
    int i;

    printf("\n\r Net system tick | Host system tick | Net counter | Host counter ");
	SWDRVL3_EnterCriticalSection();
    for (i=0 ; i < shmem_data_p->system_tick_usage_index; i++)
    {
        printf("\n\r %7d | %7d | %7d | %7d | %7d | %7d | %7d | %7d", system_tick_usage[i][0],
                        shmem_data_p->system_tick_usage[i][1],
                        shmem_data_p->system_tick_usage[i][2],
                        shmem_data_p->system_tick_usage[i][3],
                        shmem_data_p->system_tick_usage[i][4],
                        shmem_data_p->system_tick_usage[i][5],
                        shmem_data_p->system_tick_usage[i][6],
                        shmem_data_p->system_tick_usage[i][7]);
    }
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
}

static void SDL3C_BACKDOOR_EnableSystemTicker(UI8_T *cmd_buf)
{
	SWDRVL3_EnterCriticalSection();
    shmem_data_p->system_ticker_enable = ~shmem_data_p->system_ticker_enable;

    if (shmem_data_p->system_ticker_enable != FALSE)
       printf("\n\r System Ticker Enable");
    else if (shmem_data_p->system_ticker_enable == FALSE)
       printf("\n\r System Ticker Disable");
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* End of SDL3C_BACKDOOR_EnableSystemTicker */

#endif /* SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER */
#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)

static void SWDRVL3_CreateBackdoorATask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    if (SYSFUN_SpawnTask (SYS_BLD_SWDRVL3_BACKDOORA_TASK,
                          135,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          SWDRVL3_Backdoor_Auto_Task_Main,
                          0,
                          &shmem_data_p->swdrvl3_task_backdoora_tid))
    {
        owner_info.level = SYSLOG_LEVEL_CRIT;
        owner_info.module_no = SYS_MODULE_SWDRVL3;
        owner_info.function_no = SWDRVL3_TASK_Create_Tasks_FunNo;
        owner_info.error_no = SWDRVL3_TASK_Create_Tasks_ErrNo;
#if(SYS_CPNT_SYSLOG == TRUE)
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, SYS_BLD_SWDRVL3_BACKDOORA_TASK, 0, 0);
#endif
        return;
    } /* end of if */
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_Backdoor_Auto_Task_Main
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add n host routes into chip from AMTRL3 automatically.
 *           SWDRVL3 Backdoor Auto Task monitors: Timer Event.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void SWDRVL3_Backdoor_Auto_Task_Main(void)
{
    UI32_T      wait_events, all_events = 0;
    UI32_T      current_state;

    /* start periodic timer
     */
    if (SYSFUN_StartPeriodicTimerEvent (SWDRVL3_BACKDOOR_TIME_TICK_INTERVAL, SWDRVL3_TIMER_EVENT, 0, &shmem_data_p->swdrvl3_backdoora_task_tmid)
        != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\n\r SWDRVL3: create periodic swdrvl3 backdoor auto timer error.");
    } /* end of if */

    /* Task Body
     */
    while (TRUE)
    {
        /* wait event */
        SYSFUN_ReceiveEvent (SWDRVL3_TIMER_EVENT,
                             SYSFUN_EVENT_WAIT_ANY,
                             (all_events)? SYSFUN_TIMEOUT_NOWAIT:SYSFUN_TIMEOUT_WAIT_FOREVER, /* timeout */
                             &wait_events);

        all_events |= wait_events;

        if ( all_events == 0 )
            continue;

        current_state = SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p); 

        if ((current_state != SYS_TYPE_STACKING_MASTER_MODE) && (current_state != SYS_TYPE_STACKING_SLAVE_MODE))
        {
            all_events = 0;
            continue;
        }

        /* periodic timer event arrival
         */
        if (wait_events & SWDRVL3_TIMER_EVENT)
        {
           if ((shmem_data_p->auto_host_route_enable != FALSE) && (add_host_route_index <= 0x320108D1))
           {
               SDL3C_BACKDOOR_AddHostRouteAuto();
           }

           all_events ^= SWDRVL3_TIMER_EVENT;
        } /* end of if */
    } /* end of while(TRUE) */

    return;
} /* end of SWDRVL3_Backdoor_Auto_Task_Main() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SDL3C_BACKDOOR_AddHostRouteAuto
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add n host routes into chip from AMTRL3 automatically.
 *           After adding n host routes, call SYSFUN_BackDoor_Spy() to print CPU utilization.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1. Add 2000 host routes from 50.1.1.2 to 50.1.8.209
 *              on Vlan 2 on port 19.
 *           2.
 * -------------------------------------------------------------------------*/
static void SDL3C_BACKDOOR_AddHostRouteAuto(void)
{
#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
    UI32_T i;
    UI32_T rif_num  = 0;
    UI32_T dest_port  = 19;

    printf("\n\r Enter SDL3C_BACKDOOR_AddHostRouteAuto: %x", add_host_route_index);

    NETIF_MGR_GetRifFromIp (0x32010102,  &rif_num);

    for (i = 0; i < 70; i++, add_host_route_index++, (add_host_route_mac_index[5])++)
    {
         AMTRL3_MGR_AddHostRoute(add_host_route_index, add_host_route_mac_index, rif_num, dest_port, VAL_ipNetToMediaExtType_dynamic);

         if ((add_host_route_mac_index[5]) == 0xff)
         {
             (add_host_route_mac_index[4])++;
             (add_host_route_mac_index[5]) = 0x0;
         }
    }

    printf("\n\r Leave SDL3C_BACKDOOR_AddHostRouteAuto: %x", add_host_route_index);

    if (add_host_route_index >= 0x320108D1)
       SYSFUN_BackDoor_Spy();
/*    if (add_host_route_index == 0x32010102)
    {
        SYSFUN_Sleep (100);
        printf("Start IP address: 0x32010102");
        for (i = 0; i <= 253; i++, add_host_route_index++, (add_host_route_mac_index[5])++)
        {
             AMTRL3_MGR_AddHostRoute(add_host_route_index, add_host_route_mac_index, rif_num, dest_port, VAL_ipNetToMediaExtType_dynamic);
             if ((add_host_route_mac_index[5]) == 0xff)
             {
                (add_host_route_mac_index[4])++;
                (add_host_route_mac_index[5]) = 0x0;
             }
        }
        printf("End IP address: %x", add_host_route_index);
    }
    else if (add_host_route_index == 0x32010800)
    {
        SYSFUN_Sleep (100);
        printf("Start IP address: %x", add_host_route_index);
        for (i = 0; i <= 209; i++, add_host_route_index++, (add_host_route_mac_index[5])++)
        {
             AMTRL3_MGR_AddHostRoute(add_host_route_index, add_host_route_mac_index, rif_num, dest_port, VAL_ipNetToMediaExtType_dynamic);
        }
        printf("End IP address: %x", add_host_route_index);
    }
    else
    {
        SYSFUN_Sleep (100);
        printf("Start IP address: %x", add_host_route_index);
        for (i = 0; i <= 255; i++, add_host_route_index++, (add_host_route_mac_index[5])++)
        {
             AMTRL3_MGR_AddHostRoute(add_host_route_index, add_host_route_mac_index, rif_num, dest_port, VAL_ipNetToMediaExtType_dynamic);
             if ((add_host_route_mac_index[5]) == 0xff)
             {
                (add_host_route_mac_index[4])++;
                (add_host_route_mac_index[5]) = 0x0;
             }
        }
        printf("End IP address: %x", add_host_route_index);
    }
*/
#endif
    return;
} /* SDL3C_BACKDOOR_AddHostRouteAuto */


static void SDL3C_BACKDOOR_EnableAutoHostRoute(UI8_T *cmd_buf)
{
	SWDRVL3_EnterCriticalSection();
    shmem_data_p->auto_host_route_enable = ~shmem_data_p->auto_host_route_enable;

    if (shmem_data_p->auto_host_route_enable != FALSE)
       printf("\n\r Auto Host Route Enable");
    else if (shmem_data_p->auto_host_route_enable == FALSE)
       printf("\n\r Auto Host Route Disable");
	SWDRVL3_LeaveCriticalSection();

    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    return;
} /* End of SDL3C_BACKDOOR_EnableAutoHostRoute */

#endif /* ENABLE_SYSTEM_PERFORMANCE_COUNTER */

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
static void SDL3C_BACKDOOR_SetEcmpBalanceMode(UI8_T *cmd_buf)
{
    UI8_T   ch;
    UI8_T   input;
    UI32_T  mode = 0;

    printf("\r\nECMP balance mode:");
    printf("\r\n    1. Dst-ip + L4-port");
    printf("\r\nEnter ECMP balance mode:");

    ch = getchar();
    
    input = (UI8_T)(ch - '0');

    if (input == 1)
    {
        mode |= SWDRVL3_ECMP_DST_IP;
        mode |= SWDRVL3_ECMP_L4_PORT;
    }
    else
    {
        /* Terminating */
        SDL3C_BACKDOOR_TerminateCmd(cmd_buf);
    }
      
    SWDRVL3_SetEcmpBalanceMode(mode);

    /* Terminating */
    SDL3C_BACKDOOR_TerminateCmd(cmd_buf);

    return;
}
#endif

/* FUNCTION NAME : SWDRVL3_CompareTunnelNetRouteByIfindexAddr
 * PURPOSE:
 *      Provide function for L_SORT module to compare NetRoute_T elements
 *
 * INPUT:
 *      elm1_p        element1 for comparing
 *      elm2_p        element2 for comparing
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      if elm1_p is better than elm2_p, reture value is greater than zero.
 *      if elm1_p is same as elm2_p, return value is zero.
 *      if elm1_p is worse than elm2_p, return value is less than zero
 *
 * NOTES:
 *      The first key is dst_vid, the second key is dst_ip, the third key is prefix length
 */
int SWDRVL3_CompareTunnelNetRouteByIfindexAddr(void *elm1_p,void *elm2_p)
{
    NetRoute_T *element1_p, *element2_p;
    
    element1_p = (NetRoute_T *)elm1_p;
    element2_p = (NetRoute_T *)elm2_p;
   
    /* Destination vlan index */
    if (element1_p->dest_vid > element2_p->dest_vid)
        return 1;
    else if (element1_p->dest_vid < element2_p->dest_vid)
        return -1;

    /* Destination ip */
    if (memcmp(element1_p->dst_ip.ipv6_addr, element1_p->dst_ip.ipv6_addr, SYS_ADPT_IPV6_ADDR_LEN) > 0)
        return 1;
    else if (memcmp(element1_p->dst_ip.ipv6_addr, element1_p->dst_ip.ipv6_addr, SYS_ADPT_IPV6_ADDR_LEN) < 0)
        return -1;

    /* Prefix length */
    if (element1_p->prefix_length > element2_p->prefix_length)
        return 1;
    else if (element1_p->prefix_length < element2_p->prefix_length)
        return -1;
    
    return 0;
}

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_LocalSetEcmpBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set ECMP balance mode
 * INPUT   : balance_mode bitmap - SWDRVL3_ECMP_DST_IP
 *                                 SWDRVL3_ECMP_L4_PORT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRVL3_LocalSetEcmpBalanceMode(UI32_T mode)
{
    if (TRUE != DEV_SWDRVL3_PMGR_SetEcmpBalanceMode(mode))
    {
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_SetEcmpBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set ECMP balance mode
 * INPUT   : balance_mode bitmap - SWDRVL3_ECMP_DST_IP
 *                                 SWDRVL3_ECMP_L4_PORT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRVL3_SetEcmpBalanceMode(UI32_T mode)
{
    SWDRVL3_RxIscBuf_T* isc_buf_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              unit_index, pdu_len;
    UI16_T              dst_bmp = 0, isc_ret_val;
    BOOL_T              ret = TRUE;

    if (SDL3C_Debug(SDL3C_DEBUG_FLAG_CALLIN) == TRUE)
    {
       printf("\r\n Enter SWDRVL3_SetL3Bit");
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SWDRVL3_L3_OTHERS;
    }

     /* new code for ISC Send Multicast Reliable
    */
    unit_index=0;
    while(STKTPLG_POM_GetNextDriverUnit(&unit_index)==TRUE)
    {
        if ( unit_index == shmem_data_p->swdrvl3_stack_info.my_unit_id )
            continue;
        else
            dst_bmp |= BIT_VALUE(unit_index-1);
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL3_RxIscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL3, SWDRVL3_POOL_ID_ISC_SEND, SWDRVL3_SET_ECMP_BALANCE_MODE));
        isc_buf_p = (SWDRVL3_RxIscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return SWDRVL3_L3_OTHERS;
        }

        isc_buf_p->ServiceID = SWDRVL3_SET_ECMP_BALANCE_MODE;
        isc_buf_p->info.mode = mode;

        isc_ret_val=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL3_SID,
                                          mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                          SWDRVL3_TRY_TIMES, SWDRVL3_TIME_OUT, FALSE);

        if(isc_ret_val!=0)
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable() fails(ret_val=0x%x)", __FUNCTION__, isc_ret_val);
            return SWDRVL3_L3_OTHERS;
        }
    }

    ret=SWDRVL3_LocalSetEcmpBalanceMode(mode);
    return ret;
}
#endif /*#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)*/


