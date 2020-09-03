
/* static char SccsId[] = "+-<>?!NTP_MGR.C   22.1  22/04/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  NTP_MGR.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created
 * ------------------------------------------------------------------------
 *  Copyright(C)               Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */

 /* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <assert.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
/* For Exceptional Handler */
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "backdoor_mgr.h"
/* end For Exceptional Handler */
#include "leaf_es3626a.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "l_md5.h"
#include "ip_lib.h"
#include "sys_time.h"
#if(SYS_CPNT_SNTP == TRUE)
#include "sntp_pmgr.h"
#endif
#include "ntp_mgr.h"
#include "ntp_type.h"
#include "ntp_task.h"
#include "ntp_dbg.h" /* for debug use */
#include "ntp_recvbuff.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

#define NTP_MGR_LOCK()    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ntp_om_sem_id);
#define NTP_MGR_UNLOCK()  SYSFUN_OM_LEAVE_CRITICAL_SECTION(ntp_om_sem_id, orig_priority);

/* Stuff for extracting things from li_vn_mode */
#define PKT_MODE(li_vn_mode)    ((unsigned char)((li_vn_mode) & 0x7))
#define PKT_VERSION(li_vn_mode) ((unsigned char)(((li_vn_mode) >> 3) & 0x7))
#define PKT_LEAP(li_vn_mode)    ((unsigned char)(((li_vn_mode) >> 6) & 0x3))
#define NTOHL_FP(n, h)  do { (h)->Ul_i.Xl_ui = ntohl((n)->Ul_i.Xl_ui); \
                 (h)->Ul_f.Xl_uf = ntohl((n)->Ul_f.Xl_uf); } while (0)
#define L_CLR(v)    ((v)->Ul_i.Xl_ui = (v)->Ul_f.Xl_uf = 0)
#define HTONL_FP(h, n)  do { (n)->Ul_i.Xl_ui = htonl((h)->Ul_i.Xl_ui); \
                 (n)->Ul_f.Xl_uf = htonl((h)->Ul_f.Xl_uf); } while (0)

#define L_ABS(h, n) do { (n)->Ul_i.Xl_ui = htonl((h)->Ul_i.Xl_ui); \
                 (n)->Ul_f.Xl_uf = htonl((h)->Ul_f.Xl_uf); } while (0)
#if 0 /* QingfengZhang, 06 April, 2005 10:37:05 */
/* do {register UI32_T i_tmp; \
                 if((n)->Ul_f.Xl_uf > (h)->Ul_f.Xl_uf ){\
                        i_tmp = (h)->Ul_f.Xl_uf ;\
                        (h)->Ul_f.Xl_uf  = (n)->Ul_f.Xl_uf;\
                        (n)->Ul_f.Xl_uf = i_tmp;}\
                 } while (0) */
#endif /* #if 0 */

/*
 * Stuff for putting things back into li_vn_mode
 */
#define PKT_LI_VN_MODE(li, vn, md) \
    ((unsigned char)((((li) << 6) & 0xc0) | (((vn) << 3) & 0x38) | ((md) & 0x7)))


/*
 * Dealing with stratum.  0 gets mapped to 16 incoming, and back to 0
 * on output.
 */
#define PKT_TO_STRATUM(s)   ((unsigned char)(((s) == (STRATUM_PKT_UNSPEC)) ?\
                (STRATUM_UNSPEC) : (s)))

#define STRATUM_TO_PKT(s)   ((unsigned char)(((s) == (STRATUM_UNSPEC)) ?\
                (STRATUM_PKT_UNSPEC) : (s)))
#define MFPTOFP(x_i, x_f)   (((x_i) >= 0x00010000) ? 0x7fffffff : \
                    (((x_i) <= -0x00010000) ? 0x80000000 : \
                    (((x_i)<<16) | (((x_f)>>16)&0xffff))))
#define LFPTOFP(v)      MFPTOFP((v)->Ul_i.Xl_i, (v)->Ul_f.Xl_f)

#define UFPTOLFP(x, v) ((v)->Ul_i.Xl_ui = (UI32_T)(x)>>16, (v)->Ul_f.Xl_uf = (x)<<16)
#define FPTOLFP(x, v)  (UFPTOLFP((x), (v)), (x) < 0 ? (v)->Ul_i.Xl_ui -= 0x10000 : 0)

#define MAXLFP(v) ((v)->Ul_i.Xl_ui = 0x7fffffff, (v)->Ul_f.Xl_uf = 0xffffffff)
#define MINLFP(v) ((v)->Ul_i.Xl_ui = 0x80000000, (v)->Ul_f.Xl_uf = 0)
#define L_ISHIS(a, b)   ((a)->Ul_i.Xl_ui > (b)->Ul_i.Xl_ui || \
              ((a)->Ul_i.Xl_ui == (b)->Ul_i.Xl_ui && (a)->Ul_f.Xl_uf >= (b)->Ul_f.Xl_uf))

#define L_SUB(r, a) M_SUB((r)->Ul_i.Xl_ui, (r)->Ul_f.Xl_uf, (a)->Ul_i.Xl_ui, (a)->Ul_f.Xl_uf)
#define L_ADD(r, a) M_ADD((r)->Ul_i.Xl_ui, (r)->Ul_f.Xl_uf, (a)->Ul_i.Xl_ui, (a)->Ul_f.Xl_uf)
#define L_RSHIFT(v) M_RSHIFT((v)->Ul_i.Xl_ui, (v)->Ul_f.Xl_uf)
#define max(a,b)    (((a) > (b)) ? (a) : (b))

#define M_ADD(r_i, r_f, a_i, a_f)   /* r += a */ \
    do { \
        register UI32_T lo_tmp; \
        register UI32_T hi_tmp; \
        \
        lo_tmp = ((r_f) & 0xffff) + ((a_f) & 0xffff); \
        hi_tmp = (((r_f) >> 16) & 0xffff) + (((a_f) >> 16) & 0xffff); \
        if (lo_tmp & 0x10000) \
            hi_tmp++; \
        (r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
        \
        (r_i) += (a_i); \
        if (hi_tmp & 0x10000) \
            (r_i)++; \
    } while (0)

#define M_SUB(r_i, r_f, a_i, a_f)   /* r -= a */ \
    do { \
        register UI32_T lo_tmp; \
        register UI32_T hi_tmp; \
        \
        if ((a_f) == 0) { \
            (r_i) -= (a_i); \
        } else { \
            lo_tmp = ((r_f) & 0xffff) + ((-((I32_T)(a_f))) & 0xffff); \
            hi_tmp = (((r_f) >> 16) & 0xffff) \
                + (((-((I32_T)(a_f))) >> 16) & 0xffff); \
            if (lo_tmp & 0x10000) \
                hi_tmp++; \
            (r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
            \
            (r_i) += ~(a_i); \
            if (hi_tmp & 0x10000) \
                (r_i)++; \
        } \
    } while (0)
#define M_RSHIFT(v_i, v_f)      /* v >>= 1, v is signed */ \
            do { \
                (v_f) = (UI32_T)(v_f) >> 1; \
                if ((v_i) & 01) \
                    (v_f) |= 0x80000000; \
                if ((v_i) & 0x80000000) \
                    (v_i) = ((v_i) >> 1) | 0x80000000; \
                else \
                    (v_i) = (v_i) >> 1; \
            } while (0)


#define L_ISEQU(a, b)   M_ISEQU((a)->Ul_i.Xl_ui, (a)->Ul_f.Xl_uf, (b)->Ul_i.Xl_ui, (b)->Ul_f.Xl_uf)
#define M_ISEQU(a_i, a_f, b_i, b_f) /* a == b unsigned */ \
    ((a_i) == (b_i) && (a_f) == (b_f))

#define NTP_MGR_ONE_DAY 86400
#define NTP_MGR_PASS_BASE_NUM 9
#define NTP_MGR_PASS_BASE_MOVE_NUM 20
#define NTP_MGR_PASS_TOKEN_NUM   95

/* Add extra time before to send first packet to prevent packet loss in some
 * condition (ex: spanning tree is disabled).
 */
#define NTP_MGR_SEND_DELAY_TIME 10 /* seconds */

/* DATA TYPE DECLARATIONS
 */
typedef struct NTP_MGR_LCB_S
{
    UI32_T                      api_semaphore;  /* Semaphore used in API protection */
    NTP_TYPE_SYSTEM_STATE_T     system_state;
} NTP_MGR_LCB_T;

/* STATIC VARIABLE DECLARATIONS
 */
static NTP_MGR_SERVER_T  lastUpdat_server;
static UI32_T  ntp_om_sem_id;
static UI32_T  orig_priority;
static UI32_T  hz;
static UI32_T  local_timer;
static UI32_T  Fire;
static UI32_T  exponential_backoff_timer;
static UI32_T  sys_samples = DEFSAMPLES;   /* number of samples/server */
static UI32_T  sys_version = NTP_VERSION;  /* version to poll with */
static UI32_T  always_step = 0;
static UI32_T  never_step = 0;
static UI32_T  debug = 0;
static UI32_T  last_updateTick;
static UI32_T  DBG_NTP_TURN_MESSAGE_ON_OFF;
static I32_T   lastUpdate_time;
volatile UI32_T alarm_flag = 0;
unsigned long  sys_timeout = DEFTIMEOUT; /* timeout time, in TIMER_HZ units */
unsigned long  currentTime = 0;  /* The current internal time*/
static BOOL_T  server_done = FALSE;
static BOOL_T  DBG_NTP = FALSE;   /* dbg use */static void NTP_MGR_exchang_order(char *, int);
static int  NTP_MGR_int_reverse(int num);
static int  NTP_MGR_int_to_char(int num);
static int  NTP_MGR_char_to_int(char ch);
static BOOL_T is_rif_up = FALSE;
static BOOL_T is_port_forwarding = FALSE;
static UI32_T start_send_packet_time = 0;

/*--------------------------------------------------------------------------------------------
 * Begin of Data base type definition
 *--------------------------------------------------------------------------------------------
 */

typedef struct
{
    UI32_T current_server;
    UI32_T current_time;
    UI32_T current_tick;
    UI32_T last_update_server;
    UI32_T last_update_time;
} NTP_OM_VAR_STATE_T;

/* For Exceptional Handler */
enum NTP_MGR_FUN_NO_E
{
    NTP_MGR_STATUS_FUNC_NO = 1,
    NTP_MGR_SERVICE_MODE_FUNC_NO,
    NTP_MGR_POLL_TIME_FUNC_NO,
    NTP_MGR_INVALID_IP_FUNC_NO,
    NTP_MGR_INVALID_KEY_FUNC_NO,
    NTP_MGR_INVALID_VERSION_FUNC_NO,
    NTP_MGR_SERVER_IP_FUNC_NO,
    NTP_MGR_AUTH_STATUS_FUNC_NO,
    NTP_MGR_TIME_FUNC_NO,
    NTP_MGR_SERVER_FUNC_NO
};

/* STATIC VARIABLE DECLARATIONS
 */
/* Data base for NTP */
static NTP_OM_MIB_ENTRY_T NTP_TABLE;
static NTP_OM_VAR_STATE_T NTP_VAR_STATE;
/*--------------------------------------------------------------------------------------------
 * End of Data base type definition
 *--------------------------------------------------------------------------------------------
 */
/* STATIC LOCAL PROGRAM
 */
static BOOL_T NTP_MGR_PerformOperationMode(void);
static BOOL_T NTP_MGR_CheckServerIp(UI32_T ipaddress);
static BOOL_T NTP_MGR_ProcessRecvPacket(struct recvbuf *rbufp);
static BOOL_T NTP_MGR_Transmit_Packet(NTP_MGR_SERVER_T *server);
static BOOL_T NTP_MGR_ClockAdjust();
static BOOL_T NTP_MGR_AddServerData(NTP_MGR_SERVER_T *server,I32_T d, l_fp *c,UI32_T e);
static BOOL_T NTP_MGR_IsServerExist();
static NTP_MGR_SERVER_T * NTP_MGR_ClockSelect();
static void NTP_MGR_AlarmTimerRoutine(NTP_MGR_SERVER_T *server);
static void NTP_MGR_ClockFilter(register NTP_MGR_SERVER_T *server);
static void NTP_MGR_ClearGlobalData();
static void NTP_MGR_SetServersSendFlag(BOOL_T flag);


/* EXPORTED SUBPROGRAM BODIES
 */
SYSFUN_DECLARE_CSC

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize NTP_MGR used system resource, eg. protection semaphore.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_Init(void)
{
    local_timer = SYSFUN_GetSysTick();
    Fire = 0xffffffff;
    exponential_backoff_timer = 1;
    lastUpdate_time = 0;
    memset(&NTP_TABLE,0,sizeof(NTP_OM_MIB_ENTRY_T));
    memset(&lastUpdat_server,0, sizeof(NTP_MGR_SERVER_T));
    is_rif_up = FALSE;
    is_port_forwarding = FALSE;
    start_send_packet_time = 0;

    /* create semaphore */
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NTP_OM, &ntp_om_sem_id) != SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
    }

    NTP_Recvbuff_Init();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_EnterMasterMode(void)
{
    Fire = 0xffffffff;
    exponential_backoff_timer = 1;
    hz = SYS_BLD_TICKS_PER_SECOND;
    NTP_TABLE.polling_inteval = (1 << SYS_DFLT_NTP_POLL);
    NTP_TABLE.config_mode = NTP_DEFAULT_OPERATIONMODE;
    NTP_TABLE.service_status = NTP_DEFAULT_STATUS;
    NTP_TABLE.authenticate_status = NTP_DEFAULT_AUTHSTATUS;
    SYSFUN_ENTER_MASTER_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    local_timer = SYSFUN_GetSysTick();
    Fire = 0xffffffff;
    exponential_backoff_timer = 1;
    memset(&NTP_TABLE,0,sizeof(NTP_OM_MIB_ENTRY_T));
    is_rif_up = FALSE;
    is_port_forwarding = FALSE;
    start_send_packet_time = 0;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/

void NTP_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_GetOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return the NTP_MGR  mode. (slave/master/transition)
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T NTP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
* PURPOSE  : Handle the ipc request message for ntp mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if (ipcmsg_p == NULL)
    {
        return FALSE;
    }

    /* Every ipc request will fail when operating mode is transition mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        NTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch ((cmd = NTP_MGR_MSG_CMD(ipcmsg_p)))
    {
        case NTP_MGR_IPC_CMD_SET_STATUS:
            {
                NTP_MGR_IPCMsg_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_SetStatus(data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_GET_STATUS:
            {
                NTP_MGR_IPCMsg_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetStatus(&data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_RUNN_STATUS:
            {
                NTP_MGR_IPCMsg_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetRunningStatus(&data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_LAST_UPDATE_TIME:
            {
                NTP_MGR_IPCMsg_LastUpdateTime_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetLastUpdateTime(&data_p->time);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_LastUpdateTime_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_POLL_TIME:
            {
                NTP_MGR_IPCMsg_PollTime_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetPollTime(&data_p->poll_time);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_PollTime_T);
                break;
            }

        case NTP_MGR_IPC_CMD_SET_SV_OPMODE:
            {
                NTP_MGR_IPCMsg_ServMode_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_SetServiceOperationMode(data_p->serv_mode);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_GET_SV_OPMODE:
            {
                NTP_MGR_IPCMsg_ServMode_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetServiceOperationMode(&data_p->serv_mode);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE:
            {
                NTP_MGR_IPCMsg_ServMode_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetRunningServiceMode(&data_p->serv_mode);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T);
                break;
            }

        case NTP_MGR_IPC_CMD_ADD_SVR_IP:
            {
                NTP_MGR_IPCMsg_Server_T   *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_AddServerIp(data_p->ip_addr, data_p->version, data_p->keyid);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_DEL_SVR_IP:
            {
                NTP_MGR_IPCMsg_IpAddr_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_DeleteServerIp(data_p->ip_addr);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_DEL_ALL_SVR_IP:
            {
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_DeleteAllServerIp();
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_GET_NEXT_SVR:
            {
                NTP_MGR_IPCMsg_Server_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetNextServer(&data_p->ip_addr, &data_p->version, &data_p->keyid);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_LAST_UPDATE_SVR:
            {
                NTP_MGR_IPCMsg_Server_Entry_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetLastUpdateServer(&data_p->server_entry);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_Entry_T);
                break;
            }

        case NTP_MGR_IPC_CMD_FIND_SVR:
            {
                NTP_MGR_IPCMsg_Find_Server_Entry_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_FindServer(data_p->ip_addr, &data_p->server_entry);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T);
                break;
            }

        case NTP_MGR_IPC_CMD_FIND_NEXT_SVR:
            {
                NTP_MGR_IPCMsg_Find_Server_Entry_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_FindNextServer(data_p->ip_addr, &data_p->server_entry);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T);
                break;
            }

        case NTP_MGR_IPC_CMD_SET_AUTH_STATUS:
            {
                NTP_MGR_IPCMsg_Auth_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_SetAuthStatus(data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_GET_AUTH_STATUS:
            {
                NTP_MGR_IPCMsg_Auth_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetAuthStatus(&data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T);
                break;
            }

        case NTP_MGR_IPC_CMD_GET_RUNN_AUTH_STATUS:
            {
                NTP_MGR_IPCMsg_Auth_Status_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetRunningAuthStatus(&data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T);
                break;
            }

        case NTP_MGR_IPC_CMD_ADD_AUTH_KEY:
            {
                NTP_MGR_IPCMsg_Auth_Key_T   *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_AddAuthKey(data_p->keyid, data_p->auth_key);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_ADD_AUTH_KEY_ENCRYPTED:
            {
                NTP_MGR_IPCMsg_Auth_Key_Encrypted_T   *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_AddAuthKey_Encrypted(data_p->keyid, data_p->auth_key_encrypted);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_SET_AUTH_KEY_STATUS:
            {
                NTP_MGR_IPCMsg_Auth_Key_Status_T   *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_SetAuthKeyStatus(data_p->keyid, data_p->status);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_DEL_AUTH_KEY:
            {
                NTP_MGR_IPCMsg_Auth_Key_T  *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_DeleteAuthKey(data_p->keyid);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_DEL_ALL_AUTH_KEY:
            {
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_DeleteAllAuthKey();
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case NTP_MGR_IPC_CMD_GET_NEXT_KEY:
            {
                NTP_MGR_IPCMsg_Auth_Key_Entry_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_GetNextKey(&data_p->auth_entry);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Entry_T);
                break;
            }

        case NTP_MGR_IPC_CMD_FIND_KEY:
            {
                NTP_MGR_IPCMsg_Find_Key_Entry_T    *data_p = NTP_MGR_MSG_DATA(ipcmsg_p);
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = NTP_MGR_FindKey(data_p->keyid, &data_p->auth_entry);
                ipcmsg_p->msg_size = NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Key_Entry_T);
                break;
            }

        default:
            {
                NTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;

                if (DBG_NTP_TURN_MESSAGE_ON_OFF)
                {
                    printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
                }
                break;
            }
    } /* switch ipcmsg_p->cmd */

    if (NTP_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        if (DBG_NTP_TURN_MESSAGE_ON_OFF)
        {
            printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, (long)cmd);
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpStatus_enabled, 2 :VAL_ntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_SetStatus(UI32_T status)
{
    BOOL_T  ret;
    UI32_T  current_status;
    UI32_T  sntp_status;

#if(SYS_CPNT_SNTP == TRUE)/* ntp and sntp can't be enabled at same time - QingfengZhang, 01 April, 2005 4:53:38 */
    if(SNTP_MGR_GetStatus(&sntp_status)!= FALSE)
    {
        if((sntp_status == VAL_sntpStatus_enabled) &&(status == VAL_ntpStatus_enabled) )
            return FALSE;
    }
#endif

    if (status == VAL_ntpStatus_enabled || status == VAL_ntpStatus_disabled)
    {
        NTP_MGR_GetStatus(&current_status);

        if (status != current_status)
        {
            NTP_MGR_LOCK();
            Fire = 0xffffffff;
            exponential_backoff_timer = 1;
            NTP_TABLE.service_status = status;
            NTP_MGR_UNLOCK();
        }

        /* If client is enabled, set send_request flag TRUE;
         * else, set send_request FALSE.
         */
        if(status == VAL_ntpStatus_enabled)
        {
            NTP_MGR_SetServersSendFlag(TRUE);
            NTP_MGR_LOCK();
            last_updateTick = SYSFUN_GetSysTick();
            NTP_MGR_UNLOCK();
        }
        else
        {
            NTP_MGR_SetServersSendFlag(FALSE);
        }

        ret = TRUE;
    }
    else
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_STATUS_FUNC_NO, EH_TYPE_ret_FAILED_TO_SET, SYSLOG_LEVEL_ERR,"NTP status");
        ret = FALSE;
    }

    return (ret);
 }


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpStatus_enabled, 2: VAL_ntpStatus_disabled
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetStatus(UI32_T *status)
 {
    if (status == NULL)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_STATUS_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP status");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *status = NTP_TABLE.service_status;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }


/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpStatus_enabled/VAL_ntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
 SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningStatus(UI32_T *status)
 {
    NTP_MGR_LOCK();
    *status = NTP_TABLE.service_status;
    NTP_MGR_UNLOCK();

    if (*status == NTP_DEFAULT_STATUS) /* if status is default status */
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
 }


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP authenticate status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpAuthenticateStatus_enabled, 2 :VAL_ntpAuthenticateStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T NTP_MGR_SetAuthStatus(UI32_T status)
{
    BOOL_T  ret = FALSE;
    UI32_T  current_status;

    if (status == VAL_ntpAuthenticateStatus_enabled || status == VAL_ntpAuthenticateStatus_disabled)
    {
        NTP_MGR_GetAuthStatus(&current_status);

        if (status != current_status)
        {
            NTP_MGR_LOCK();
            NTP_TABLE.authenticate_status = status;
            NTP_MGR_UNLOCK();
        }

        ret = TRUE;
    }
    else
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_AUTH_STATUS_FUNC_NO, EH_TYPE_ret_FAILED_TO_SET, SYSLOG_LEVEL_ERR,"NTP Authenticate status");
        ret = FALSE;
    }

    return (ret);
 }

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP authenticate status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpAuthenticateStatus_enabled, 2: VAL_ntpAuthenticateStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_GetAuthStatus(UI32_T *status)
{
    if (status == NULL)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_AUTH_STATUS_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP status");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *status = NTP_TABLE.authenticate_status;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }


/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningAuthStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the authenticate status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpAuthenticateStatus_enabled/VAL_ntpAuthenticateStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningAuthStatus(UI32_T *status)
{
    NTP_MGR_LOCK();
    *status = NTP_TABLE.authenticate_status;
    NTP_MGR_UNLOCK();

    if (*status == NTP_DEFAULT_AUTHSTATUS) /* if status is default status */
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp_ service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_SetServiceOperationMode(UI32_T mode)
 {
    BOOL_T  ret;

    NTP_MGR_LOCK();

    switch (mode)
    {
        case VAL_ntpServiceMode_unicast:
            NTP_TABLE.config_mode = mode;
            ret = TRUE;
            break;
        case VAL_ntpServiceMode_broadcast:
        case VAL_ntpServiceMode_anycast: /* Not support now */
        default:
            ret = FALSE;
            EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_SERVICE_MODE_FUNC_NO, EH_TYPE_ret_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_ERR,"Server mode (1-2)");
          break;
    }

    NTP_MGR_UNLOCK();
    return (ret);
 }

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp_ service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetServiceOperationMode(UI32_T *mode)
 {
    if (NULL == mode)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_SERVICE_MODE_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP service mode");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *mode = NTP_TABLE.config_mode ;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }
/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_ntpServiceMode_unicast = 1
 *           VAL_ntpServiceMode_broadcast = 2
 *           VAL_ntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningServiceMode(UI32_T *servicemode)
{
    NTP_MGR_LOCK();
    *servicemode = NTP_TABLE.config_mode ;
    NTP_MGR_UNLOCK();

    if (*servicemode == NTP_DEFAULT_OPERATIONMODE) /* if service mode is default mode*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_GetPollTime(UI32_T *polltime)
{
    if (NULL == polltime)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_POLL_TIME_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP poll time");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *polltime = NTP_TABLE.polling_inteval;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a ntp_ server ip
 * INPUT    : 1. ip address 2. ntp version 3. key id
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_AddServerIp(UI32_T ipaddress, UI32_T version, UI32_T keyid)
{
    NTP_MGR_SERVER_T *tmpserver, server_tmp;
    NTP_MGR_SERVER_T *sp1,*sp2 = NULL;

    if (NTP_MGR_CheckServerIp(ipaddress) != TRUE)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_IP_FUNC_NO, EH_TYPE_ret_INVALID_IP, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    /* check version */
    if (version > 3 || version < 1)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_VERSION_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    /* check key id */
    if (keyid > MAX_ntpServerKeyId || keyid < 0)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    /* If ip alread exist , just modify version and keyid if change
     */
    if(NTP_MGR_FindServer(ipaddress,&server_tmp))
    {
        for(tmpserver=NTP_TABLE.server_entry; tmpserver != NULL;
                tmpserver=tmpserver->next_server)
        {
            if(ipaddress == tmpserver->srcadr.sin_addr.s_addr)
            {
                if(version != tmpserver->version || keyid != tmpserver->keyid)
                {
                    NTP_MGR_LOCK();
                    tmpserver->version = version;
                    tmpserver->keyid = keyid;
                    NTP_MGR_UNLOCK();
                }
            }
        }
    }
    /* If ip not exist , create a new server and add to the server list*/
    else
    {
        if(NTP_TABLE.num_servers >= NTP_MAXCLOCK)
        {
            return (FALSE);
        }

        tmpserver = (NTP_MGR_SERVER_T *)malloc(sizeof(NTP_MGR_SERVER_T));

        if(NULL == tmpserver)
        {
            return (FALSE);
        }

        NTP_MGR_LOCK();
        memset(tmpserver,0,sizeof(NTP_MGR_SERVER_T));
        memset(&tmpserver->srcadr,0,sizeof(tmpserver->srcadr));
        tmpserver->srcadr.sin_addr.s_addr = ipaddress;
        tmpserver->srcadr.sin_port = L_STDLIB_Hton16(NTP_PORT);
        tmpserver->version=version;
        tmpserver->keyid = keyid;
        tmpserver->ntpServerStatus = VAL_ntpServerStatus_valid;
        tmpserver->event_time = NTP_TABLE.num_servers+1;

        /* if the is flag is true,
         * send request packet immediately
         */
        tmpserver->send_request = TRUE;

        sp1=NTP_TABLE.server_entry;

        if(NTP_TABLE.server_entry == NULL)
        {
            NTP_TABLE.server_entry = tmpserver;
            tmpserver->next_server = NULL;
        }
        else
        {
            while((memcmp(&(tmpserver->srcadr.sin_addr.s_addr), &(sp1->srcadr.sin_addr.s_addr), sizeof(sp1->srcadr.sin_addr.s_addr)) > 0) && (sp1->next_server != NULL))
            {
                sp2=sp1;
                sp1=sp1->next_server;
            }
            if(memcmp(&(tmpserver->srcadr.sin_addr.s_addr), &(sp1->srcadr.sin_addr.s_addr), sizeof(sp1->srcadr.sin_addr.s_addr)) < 0)
            {
                if(sp1== NTP_TABLE.server_entry)
                {
                    NTP_TABLE.server_entry = tmpserver;
                    tmpserver->next_server = sp1;
                }
                else
                {
                    sp2->next_server = tmpserver;
                    tmpserver->next_server = sp1;
                }
            }
            else
            {
                sp1->next_server = tmpserver;
                tmpserver->next_server = NULL;
            }
        }

        NTP_TABLE.num_servers++;
        NTP_MGR_UNLOCK();
    }

    return (TRUE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip
 * INPUT    : index (MIN_ntpServerIndex <= index <= MAX_ntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : 1.Delete a designated server
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_DeleteServerIp(UI32_T ipaddress)
{
    NTP_MGR_SERVER_T *sp1,*sp2 = NULL,server_tmp;

    /* ip not exist*/
    if(!NTP_MGR_FindServer(ipaddress,&server_tmp))
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_IP_FUNC_NO, EH_TYPE_ret_INVALID_IP, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }
    else
    {
        sp1=NTP_TABLE.server_entry;

        while((ipaddress != sp1->srcadr.sin_addr.s_addr) && (sp1->next_server !=NULL))
        {
            sp2=sp1;
            sp1=sp1->next_server;
        }

        if(ipaddress == sp1->srcadr.sin_addr.s_addr)
        {
            if(sp1==NTP_TABLE.server_entry)
            {
                NTP_MGR_LOCK();
                NTP_TABLE.server_entry=sp1->next_server;
                NTP_MGR_UNLOCK();
                free(sp1);
                sp1 = NULL;
            }
            else
            {
                NTP_MGR_LOCK();
                sp2->next_server=sp1->next_server;
                NTP_MGR_UNLOCK();
                free(sp1);
                sp1 = NULL;
            }

            NTP_TABLE.num_servers--;
        }
    }

    return (TRUE);
 }

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of last update-time
 * INPUT    : buffer pointer stored time information
 * OUTPUT   : time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *        FALSE: NTP never get time from server.
 * NOTES    : Used in 'show ntp'
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetLastUpdateUTCTime(UI32_T *time)
{
    if (NULL == time)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_TIME_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"last update time");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *time = NTP_VAR_STATE.last_update_time;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetCurrentUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of GMT
 * INPUT    : Buffer of  UTC time
 * OUTPUT   : 1.time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *        FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetCurrentUTCTime(UI32_T *time)
{
    if (NULL == time)
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_TIME_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR, "current time");
        return FALSE;
    }

    NTP_MGR_LOCK();
    *time = NTP_VAR_STATE.current_time;
    NTP_MGR_UNLOCK();
    return (TRUE);
 }

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_InTimeServiceMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform time serice mode,e.g, unicast, broadcast mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by NTP_TASK
 *------------------------------------------------------------------------------*/
void NTP_MGR_InTimeServiceMode(void)
{
    BOOL_T ret;
    UI32_T service_mode;
    UI32_T service;
    UI32_T poll_time, poll_time_in_ticks;
    UI32_T currentTick,pollTick;
    UI32_T exponential_backoff_timer_in_ticks;
    UI32_T current_time = 0;

    if (0 == start_send_packet_time)
    {
        return;
    }

    SYS_TIME_GetRealTimeBySec(&current_time);
    if (current_time < start_send_packet_time)
    {
        return;
    }

    ret = NTP_MGR_GetStatus(&service);
    if (service != VAL_ntpStatus_enabled || ret != TRUE)
    {
        return;
    }

    ret = NTP_MGR_GetServiceOperationMode(&service_mode);

    if (ret != TRUE)
    {
        return;
    }

    /* Get poll time in second */
    ret = NTP_MGR_GetPollTime(&poll_time);

    if (ret != TRUE)
    {
        return;
    }

    poll_time_in_ticks = poll_time * SYS_BLD_TICKS_PER_SECOND;
    exponential_backoff_timer_in_ticks = exponential_backoff_timer * hz;

    switch (service_mode)
    {
        case VAL_ntpServiceMode_unicast:

            /* If there are one or more server exist,
             * or polling time reaches,
             * perform send request procedure.
             */
            if(TRUE == NTP_MGR_IsServerExist())
            {
                NTP_MGR_SERVER_T *server_p;
                BOOL_T server_perform = FALSE;

                for (server_p = NTP_TABLE.server_entry; server_p != NULL; server_p = server_p->next_server)
                {
                    if(TRUE == server_p->send_request)
                    {
                        server_perform = TRUE;
                        break;
                    }
                }

                /* To measue how much time is passed
                 */
                currentTick = SYSFUN_GetSysTick();
                pollTick = (currentTick- last_updateTick);

                /* If polling time is not zero, and no server need to send request,
                 * do nothing
                 */
                if(pollTick < poll_time_in_ticks)
                {
                    if(FALSE == server_perform)
                    {
                        break;
                    }
                }
                else
                {
                    /* polling time reaches, need to send request to every server.
                     */
                    NTP_MGR_SetServersSendFlag(TRUE);
                    last_updateTick = SYSFUN_GetSysTick();
                }

                /* Start to get time, and measure how much time is spent
                 */
                ret = NTP_MGR_PerformOperationMode();
            }
            else
            {
                /* no server exists
                 */
                break;
            }
            break;

        case VAL_ntpServiceMode_broadcast:

            /* If there are one or more server exist, perform send request procedure.
             */
            if( TRUE == NTP_MGR_IsServerExist() )
            {
                NTP_MGR_SetServersSendFlag(TRUE);
                ret =  NTP_MGR_PerformOperationMode();
            }
            /* Reset the timer */
            local_timer = SYSFUN_GetSysTick();
            Fire = 0;
            break;

        case VAL_ntpServiceMode_anycast:
            /* Reset the timer */
            local_timer = SYSFUN_GetSysTick();
            Fire = 0;
            break;

        default:
            assert(0);
            break;
    }

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_PerformOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform unicast or broadcast mode
 * INPUT    : ipaddress:if is zero then broadcast, or unicast
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by NTP_MGR_InTimeService
 *------------------------------------------------------------------------------
 */
static BOOL_T NTP_MGR_PerformOperationMode(void)
{
    struct recvbuf *rbuf;
    UI32_T was_alarmed;
    NTP_MGR_SERVER_T *sp;
    UI32_T eventTime = 0;
    UI32_T j;

    /* create ntp socket
     */
    if (NTP_TASK_CreateSocket() == FALSE)
    {
        if (DBG_NTP)
        {
            printf("NTP socket creates failed.\r\n");
        }
        return FALSE;
    }

    /* Before do the polling clear the global data first
     */
    NTP_MGR_ClearGlobalData();

    for (sp = NTP_TABLE.server_entry; sp != NULL; sp = sp->next_server)
    {
        if(TRUE != sp->send_request)
        {
            continue;
        }

        /* For Each polling ,the server event_time must be reset
         */
        NTP_MGR_LOCK();
        sp->event_time = ++eventTime;
        sp->filter_nextpt = 0;
        sp->xmtcnt = 0;

        /* use when Authenticate Enable
         */
        sp->trust = 0;

        for (j = 0; j < NTP_SHIFT; j++)
        {
            sp->filter_delay[j] = 0;
            L_CLR(&(sp->filter_offset[j]));
            sp->filter_soffset[j] = 0;
            sp->filter_error[j] = 0;
        }

        was_alarmed = 0;
        rbuf = NULL;
        server_done = FALSE;
        NTP_MGR_UNLOCK();

        /* when server complete procedure, server_done will be TRUE.
         */
        while (FALSE == server_done)
        {
            if (alarm_flag)
            {
                /* alarmed?
                 */
                was_alarmed = 1;
                NTP_MGR_LOCK();
                alarm_flag = 0;
                NTP_MGR_UNLOCK();
            }

            /* get received buffers
             */
            rbuf = NTP_RECVBUFF_GetFirstBuf();

            if (!was_alarmed && rbuf == NULL)
            {
                NTP_TASK_AddPkts_ToRecvbuff();

                if (alarm_flag)
                {
                    /* alarmed?
                     */
                    was_alarmed = 1;
                    NTP_MGR_LOCK();
                    alarm_flag = 0;
                    NTP_MGR_UNLOCK();
                }

                rbuf = NTP_RECVBUFF_GetFirstBuf();
            }

            while (rbuf != NULL)
            {
                NTP_MGR_ProcessRecvPacket(rbuf);
                free(rbuf);
                /* get received buffers
                 */
                rbuf = NTP_RECVBUFF_GetFirstBuf();
            }

            if (was_alarmed)
            {
                NTP_MGR_AlarmTimerRoutine(sp);
                was_alarmed = 0;
            }
        }  /* while (complete_servers < 1) */

        NTP_MGR_LOCK();
        sp->send_request = FALSE;
        NTP_MGR_UNLOCK();
    }

    /* Close ntp socket
     */
    NTP_TASK_CloseSocket();

    /* When we get here we've completed the polling of all servers. Adjust the clock,
     */
    return(NTP_MGR_ClockAdjust());
 }

/*----------------------
 * Internal function
 *----------------------
 */

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_CheckServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : check input address is in suitable range.
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T
NTP_MGR_CheckServerIp(
    UI32_T ipaddress)
{
    if (IP_LIB_IsValidForRemoteIp((UI8_T *)&ipaddress) != IP_LIB_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_DeleteAllServerIp(void)
{
    NTP_MGR_SERVER_T *sp;
    NTP_MGR_SERVER_T *spk;

    if(NTP_TABLE.server_entry == NULL)
    {
        return (TRUE);
    }

    NTP_MGR_LOCK();
    sp = NTP_TABLE.server_entry;

    while(sp!= NULL)
    {
        spk = sp->next_server;
        memset(sp,0,sizeof(NTP_MGR_SERVER_T));
        free(sp);
        sp = NULL;
        sp = spk;
    }

    NTP_TABLE.server_entry = NULL;
    NTP_TABLE.num_servers = 0;
    NTP_MGR_UNLOCK();
    return (TRUE);
}

/*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_MGR_FindServer
  *------------------------------------------------------------------------------
  * PURPOSE  : Get a server entry  from OM using ip address as index
  * INPUT    : ip address
  * OUTPUT   : buffer contain information
  * RETURN   : TRUE : If find
  *            FALSE: If not found
  * NOTES    : This is only used in cli
  *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_FindServer(UI32_T ipaddress, NTP_MGR_SERVER_T *server)
{
    NTP_MGR_SERVER_T *server_tmp;

    NTP_MGR_LOCK();

    for (server_tmp = NTP_TABLE.server_entry; server_tmp != NULL;
         server_tmp = server_tmp->next_server)
    {
        if (ipaddress == server_tmp->srcadr.sin_addr.s_addr)
        {
            memcpy(server,server_tmp, sizeof(NTP_MGR_SERVER_T));
            NTP_MGR_UNLOCK();
            return (TRUE);
        }
    }

    NTP_MGR_UNLOCK();
    return (FALSE);
}

/*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_MGR_GetServer
  *------------------------------------------------------------------------------
  * PURPOSE  : Get a server entry  from OM using ip address as index
  * INPUT    : ip address
  * OUTPUT   : buffer contain information
  * RETURN   :
  * NOTES    : This is only used in ntp task
  *------------------------------------------------------------------------------*/
NTP_MGR_SERVER_T * NTP_MGR_GetServer(UI32_T ipaddress)
{
    NTP_MGR_SERVER_T *server_tmp;

    NTP_MGR_LOCK();

    for(server_tmp=NTP_TABLE.server_entry; server_tmp != NULL;
        server_tmp=server_tmp->next_server)
    {
        if(ipaddress == server_tmp->srcadr.sin_addr.s_addr)
        {
            NTP_MGR_UNLOCK();
            return server_tmp;
        }
    }

    NTP_MGR_UNLOCK();
    return (NULL);
}


/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_AddAuthKey
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication key to the list
* INPUT    : 1.index 2. md5_key
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_AddAuthKey(UI32_T index, char *md5_key)
{
    NTP_MGR_AUTHKEY_T *sk,*sk1,*sk2 = NULL;
    NTP_MGR_AUTHKEY_T findKey;

    /* check key id */
    if (index > MAX_ntpServerKeyId || index < 0)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    /* check key context */
    if ((md5_key == NULL) || (strlen(md5_key) >MAXSIZE_ntpAuthKeyWord) )
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    /* If keyid alread exist , just modify md5_key */
    if(NTP_MGR_FindKey(index,&findKey))
    {
        sk = NTP_TABLE.authKey_entry[KEYHASH(index)];

        while (sk != 0)
        {
            if(index == sk->keyid)
            {
                NTP_MGR_LOCK();
                strncpy(sk->k.MD5_key, md5_key, sizeof(sk->k.MD5_key)-1);
                sk->k.MD5_key[sizeof(sk->k.MD5_key)-1] = '\0';
                sk->keylen = strlen(sk->k.MD5_key);
                sk->ntpAuthKeyStatus = VAL_ntpAuthKeyStatus_valid;

                if(sk->keylen > MAXSIZE_ntpAuthKeyWord)
                   sk->keylen = MAXSIZE_ntpAuthKeyWord;

                memset(sk->password, 0, sizeof(sk->password));

                if(NTP_MGR_Encode(7, md5_key, sk->password)!= TRUE)
                {
                    NTP_MGR_UNLOCK();
                    return (FALSE);
                }

                sk->flags |= KEY_MD5;
                NTP_MGR_UNLOCK();
                return (TRUE);
            }

            sk = sk->next;
        }
    }
    /* If key not exist , create a new authkeky and add to the Hash table*/
    else
    {
        if(NTP_TABLE.num_authkeys >= NTP_MAXAUTHKEY)
        {
            return (FALSE);
        }

        sk = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

        if(NULL == sk)
        {
            return (FALSE);
        }

        NTP_MGR_LOCK();
        strncpy(sk->k.MD5_key,(const char *)md5_key, sizeof(sk->k.MD5_key)-1);
        sk->k.MD5_key[sizeof(sk->k.MD5_key)-1] = '\0';
        sk->keylen = strlen(sk->k.MD5_key);

        if(sk->keylen > MAXSIZE_ntpAuthKeyWord)
              sk->keylen = MAXSIZE_ntpAuthKeyWord;

        memset(sk->password, 0, sizeof(sk->password));

        if(NTP_MGR_Encode(7, md5_key, sk->password)!= TRUE)
        {
            free(sk);
            NTP_MGR_UNLOCK();
            return (FALSE);
        }

        sk->keyid = index;
        sk->flags |= KEY_MD5;
        sk->lifetime = 0;
        sk->ntpAuthKeyStatus = VAL_ntpAuthKeyStatus_valid;

        sk1 = NTP_TABLE.authKey_entry[KEYHASH(index)];

        if(NTP_TABLE.authKey_entry[KEYHASH(index)] == NULL)
        {
            NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
            sk->next= NULL;
        }
        else
        {
            while((sk->keyid > sk1->keyid) && (sk1->next!= NULL))
            {
                sk2=sk1;
                sk1=sk1->next;
            }

            if(sk->keyid < sk1->keyid)
            {
                if(sk1== NTP_TABLE.authKey_entry[KEYHASH(index)])
                {
                    NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
                    sk->next= sk1;
                }
                else
                {
                    sk2->next = sk;
                    sk->next = sk1;
                }
            }
            else
            {
                sk1->next = sk;
                sk->next = NULL;
            }
        }

        NTP_TABLE.num_authkeys++;
        NTP_MGR_UNLOCK();
    }

    return (TRUE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteAllAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all Authenticaion key
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_DeleteAllAuthKey(void)
{
    UI32_T i;
    NTP_MGR_AUTHKEY_T *sk;
    NTP_MGR_AUTHKEY_T *skp;

    NTP_MGR_LOCK();

    for(i=0; i< HASHSIZE; i++)
    {
        sk=NTP_TABLE.authKey_entry[i];

        while(sk!=NULL && sk->keyid < MAX_ntpServerKeyId)
        {
            skp=sk->next;
            free(sk);
            sk = NULL;
            sk=skp;
        }

        NTP_TABLE.authKey_entry[i] = NULL;
    }

    NTP_TABLE.num_authkeys = 0;
    NTP_MGR_UNLOCK();
    return (TRUE);
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a designed Authentication key
 * INPUT    : keyid
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_DeleteAuthKey(UI32_T keyid)
{
    NTP_MGR_AUTHKEY_T *sk1,*sk2 = NULL,findKey;

    /* key not exist*/
    if(!NTP_MGR_FindKey(keyid,&findKey))
    {
        return (FALSE);
    }
    else
    {
        sk1 = NTP_TABLE.authKey_entry[KEYHASH(keyid)];

        while ((sk1->keyid != keyid) && sk1->next !=NULL)
        {
            sk2=sk1;
            sk1=sk1->next;
        }

        if (keyid == sk1->keyid)
        {
            if(sk1==NTP_TABLE.authKey_entry[KEYHASH(keyid)])
            {
                NTP_MGR_LOCK();
                NTP_TABLE.authKey_entry[KEYHASH(keyid)]=sk1->next;
                NTP_MGR_UNLOCK();
                free(sk1);
            }
            else
            {
                NTP_MGR_LOCK();
                sk2->next=sk1->next;
                NTP_MGR_UNLOCK();
                free(sk1);
                sk1 = NULL;
            }

            NTP_TABLE.num_authkeys--;
        }
    }

    return (TRUE);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_SetTrustedKey
*------------------------------------------------------------------------------
* PURPOSE  : Set an authentication key to be a trusted one
* INPUT    : keyid
             status
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    :
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_SetTrustedKey(UI32_T keyid, UI32_T status)
{
    NTP_MGR_AUTHKEY_T *sk;

    if(keyid > MAX_ntpServerKeyId || keyid < 0)
    {
        return (FALSE);
    }

    sk = NTP_TABLE.authKey_entry[KEYHASH(keyid)];

    while (sk != 0)
    {
        if (keyid == sk->keyid)
        {
            NTP_MGR_LOCK();
            if(status == NtpAuthkeyTrusted_enabled)
                sk->flags |= KEY_TRUSTED;
            else
                sk->flags &= ~KEY_TRUSTED;
            NTP_MGR_UNLOCK();
            return (TRUE);
        }

        sk = sk->next;
    }

    return (FALSE);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_FindKey
*------------------------------------------------------------------------------
* PURPOSE  : check whether the key exist
* INPUT    : keyid
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_FindKey(UI32_T keyid, NTP_MGR_AUTHKEY_T *authkey)
{
    NTP_MGR_AUTHKEY_T *sk;

    if(keyid > MAX_ntpServerKeyId || keyid < 0)
    {
        return (FALSE);
    }

    NTP_MGR_LOCK();
    sk = NTP_TABLE.authKey_entry[KEYHASH(keyid)];

    while (sk != 0)
    {
        if (keyid == sk->keyid)
        {
            memcpy(authkey,sk,sizeof(NTP_MGR_AUTHKEY_T));
            NTP_MGR_UNLOCK();
            return (TRUE);
        }

        sk = sk->next;
    }

    NTP_MGR_UNLOCK();
    return (FALSE);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_GetNextKey
*------------------------------------------------------------------------------
* PURPOSE  : Get next key
* INPUT    :
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_GetNextKey(NTP_MGR_AUTHKEY_T *authkey)
{
    NTP_MGR_AUTHKEY_T *sk;
    UI32_T next_keyid ;

    if(authkey->keyid > MAX_ntpServerKeyId || authkey->keyid < 0)
    {
        return (FALSE);
    }

    NTP_MGR_LOCK();

    for(next_keyid= authkey->keyid+1;next_keyid<=MAX_ntpServerKeyId;next_keyid++)
    {
        sk = NTP_TABLE.authKey_entry[KEYHASH(next_keyid)];

        while (sk != 0)
        {
            if ((next_keyid == sk->keyid) && (sk->ntpAuthKeyStatus !=VAL_ntpAuthKeyStatus_invalid))
            {
                memcpy(authkey,sk,sizeof(NTP_MGR_AUTHKEY_T));
                NTP_MGR_UNLOCK();
                return (TRUE);
            }
            sk = sk->next;
        }
    }

    NTP_MGR_UNLOCK();
    return (FALSE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetNextServer(UI32_T *ipadd, UI32_T *ver,UI32_T *key)
{
    NTP_MGR_SERVER_T    *sp;

    if (ipadd == 0)
    {
         EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_SERVER_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP server");
         return (FALSE);
    }

    NTP_MGR_LOCK();
    for(sp=NTP_TABLE.server_entry; sp != NULL;
        sp=sp->next_server)
    {
        if(memcmp(ipadd, &(sp->srcadr.sin_addr.s_addr), sizeof(sp->srcadr.sin_addr.s_addr)) < 0)
        {
            *ipadd = sp->srcadr.sin_addr.s_addr;
            *ver = sp->version;
            *key = sp->keyid;
            NTP_MGR_UNLOCK();
            return (TRUE);
        }
    }

    NTP_MGR_UNLOCK();
    return (FALSE);
}

/*
 * receive - receive and process an incoming frame
 */
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_ProcessRecvPacket
 *------------------------------------------------------------------------------
 * PURPOSE  : This function process one received data from the recvbuff.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_MGR_ProcessRecvPacket(struct recvbuf *rbufp)
{
    NTP_PACKET *rpkt;
    NTP_MGR_SERVER_T *server;
    I32_T di;
    l_fp t10, t23, tmp,t12,t03,org_t3;
    l_fp org;
    l_fp rec;
    l_fp ci;
    l_fp recv;
    int has_mac;
    int is_authentic;
    UI32_T authStatus;

    /*
     * Check to see if the packet basically looks like something
     * intended for us.
     */
    if (rbufp->recv_length == LEN_PKT_NOMAC)
        has_mac = 0;
    else if (rbufp->recv_length >= LEN_PKT_NOMAC)
        has_mac = 1;
    else {
        if (DBG_NTP)
            printf("receive: packet length %d\n",
               rbufp->recv_length);
        return FALSE;         /* funny length packet */
    }

    rpkt = &(rbufp->recv_pkt);

    if (PKT_VERSION(rpkt->li_vn_mode) < NTP_OLDVERSION ||
        PKT_VERSION(rpkt->li_vn_mode) > NTP_VERSION) {
        return FALSE;
    }

    if ((PKT_MODE(rpkt->li_vn_mode) != MODE_SERVER
         && PKT_MODE(rpkt->li_vn_mode) != MODE_PASSIVE)
        || rpkt->stratum >= STRATUM_UNSPEC) {
        if (DBG_NTP)
            printf("receive: mode %d stratum %d\n",
               PKT_MODE(rpkt->li_vn_mode), rpkt->stratum);
        return FALSE;
    }

    /*
     * So far, so good.  See if this is from a server we know.
     */
    server = NTP_MGR_GetServer(rbufp->recv_srcadr.sin_addr.s_addr);

    if(NULL == server)
    {
        if (DBG_NTP)
            printf("receive: server not found\n");
        return FALSE;
    }

    NTP_MGR_LOCK();
    /*
     * Decode the org timestamp and make sure we're getting a response
     * to our last request.
     */
    NTOHL_FP(&rpkt->org, &org);
    NTOHL_FP(&rpkt->org, &org_t3);

    if (!L_ISEQU(&org, &server->xmt)) {
        if (DBG_NTP)
            printf("receive: pkt.org and peer.xmt differ\n");
        NTP_MGR_UNLOCK();
        return FALSE;
    }

    NTP_MGR_UNLOCK();

    /*
     * Check out the authenticity if we're doing that.
     */
    if(NTP_MGR_GetAuthStatus(&authStatus) == FALSE)
    {
        return FALSE;
    }

    if (authStatus == VAL_ntpAuthenticateStatus_disabled)
        is_authentic = 1;
    else {
        is_authentic = 0;

        if (has_mac && (ntohl(rpkt->exten[0]) == server->keyid) && (rpkt->exten[0]!=0) )
            is_authentic = 1;
    }

    NTP_MGR_LOCK();
    server->trust <<= 1;

    if (!is_authentic)
        server->trust |= 1;

    /*
     * Looks good.  Record info from the packet.
     */
    server->leap= PKT_LEAP(rpkt->li_vn_mode);
    server->stratum = PKT_TO_STRATUM(rpkt->stratum);
    server->precision= rpkt->precision;
    server->rootdelay= ntohl(rpkt->rootdelay);
    server->rootdispersion= ntohl(rpkt->rootdispersion);
    server->refid= rpkt->refid;
    NTOHL_FP(&rpkt->reftime, &server->reftime);
    NTOHL_FP(&rpkt->rec, &rec);
    NTOHL_FP(&rpkt->xmt, &server->org);

    /*junying*/
/*    NTOHL_FP(&rbufp->recv_time, &recv);*/
    recv = rbufp->recv_time;
    NTP_MGR_UNLOCK();
#if 0
    /*
     * Make sure the server is at least somewhat sane.  If not, try
     * again.
     */
    if (L_ISZERO(&rec) || !L_ISHIS(&server->org, &rec)) {
        transmit(server);
        return;
    }
#endif

    /*
     * Calculate the round trip delay (di) and the clock offset (ci).
     * We use the equations (reordered from those in the spec):
     *
     * d = (t2 - t3) - (t1 - t0)
     * c = ((t2 - t3) + (t1 - t0)) / 2
     */
    t10 = server->org;      /* pkt.xmt == t1 */
    L_SUB(&t10, &rbufp->recv_time); /* recv_time == t0*/

    t23 = rec;          /* pkt.rec == t2 */
    L_SUB(&t23, &org);      /* pkt->org == t3 */

    /* now have (t2 - t3) and (t0 - t1).    Calculate (ci) and (di) */
    /*
     * Calculate (ci) = ((t1 - t0) / 2) + ((t2 - t3) / 2)
     * For large offsets this mayprevent an overflow on '+'
     */
    ci = t10;
    L_RSHIFT(&ci);
    tmp = t23;
    L_RSHIFT(&tmp);
    L_ADD(&ci, &tmp);

    /*
     * Calculate di in t23 in full precision, then truncate
     * to an s_fp.
     */
    /*L_SUB(&t23, &t10);*/
    /* switch can only get 0.01 sec , so special process here */
    t12 = server->org ;   /*t1-t2*/
    L_SUB(&t12, &rec);
    t03 = recv;
    if((t03.Ul_i.Xl_ui == org_t3.Ul_i.Xl_ui) && (t03.Ul_f.Xl_uf == org_t3.Ul_f.Xl_uf))
        t03.Ul_f.Xl_uf += 100000000 ;
    L_SUB(&t03,&org_t3);
    L_SUB(&t03, &t12);
    di = LFPTOFP(&t03);

/*    if (DBG_NTP)
        printf("offset: %s, delay %s\n", lfptoa(&ci, 6), fptoa(di, 5)); */

    di += (FP_SECOND >> (-(int)NTPDATE_PRECISION))
        + (FP_SECOND >> (-(int)server->precision)) + NTP_MAXSKW;
        /*+ t23.Ul_f.Xl_uf;*/

    if (di <= 0) {      /* value still too raunchy to use? */
        L_CLR(&ci);
        di = 0;
    } else {
        di = max(di, NTP_MINDIST);
    }

    /*
     * Shift this data in, then transmit again.
     */
    NTP_MGR_AddServerData(server,(I32_T)di, &ci, 0);
    NTP_MGR_Transmit_Packet(server);
    return TRUE;
}


 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_Transmit_Packet
 *------------------------------------------------------------------------------
 * PURPOSE  : transmit a packet to the given server, or mark it completed.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : This is called by the timeout routine and by the receive
 *        procedure.
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_MGR_Transmit_Packet(NTP_MGR_SERVER_T *server)
{
    NTP_PACKET xpkt;
    UI32_T authStatus;
    l_fp ts;
    I32_T socketid; /*dbg use   - QingfengZhang, 10 March, 2005 2:25:00 */
    NTP_MGR_AUTHKEY_T key;
    UI32_T time=0;

    memset(&xpkt,0,sizeof(NTP_PACKET));

    if (server->filter_nextpt < server->xmtcnt) {

        /*
         * Last message to this server timed out.  Shift
         * zeros into the filter.
         */
        L_CLR(&ts);
        if(NTP_MGR_AddServerData(server, 0, &ts, 0) == FALSE)
            return FALSE;
    }

    NTP_MGR_LOCK();

    if ((int)server->filter_nextpt >= sys_samples) {
        /*
         * Got all the data we need.  Mark this guy
         * completed and return.
         */
        server->event_time = 0;
        server_done = TRUE;
        NTP_MGR_UNLOCK();
        return FALSE;
    }

    NTP_MGR_UNLOCK();
    /*
     * If we're here, send another message to the server.    Fill in
     * the packet and let 'er rip.
     */
    xpkt.li_vn_mode = PKT_LI_VN_MODE(LEAP_NOTINSYNC,
                sys_version, MODE_CLIENT);
    xpkt.stratum = STRATUM_TO_PKT(STRATUM_UNSPEC);
    xpkt.ppoll = SYS_DFLT_NTP_POLL;
    xpkt.precision = NTPDATE_PRECISION;
    xpkt.rootdelay = htonl(NTPDATE_DISTANCE);
    xpkt.rootdispersion = htonl(NTPDATE_DISP);
    xpkt.refid = htonl(NTPDATE_REFID);
    L_CLR(&xpkt.reftime);
    L_CLR(&xpkt.org);
    L_CLR(&xpkt.rec);

    /*
     * Determine whether to authenticate or not.    If so,
     * fill in the extended part of the packet and do it.
     * If not, just timestamp it and send it away.
     */
    if(NTP_MGR_GetAuthStatus(&authStatus) == FALSE)
    {
        return FALSE;
    }

    SYS_TIME_GetUTC(&time);

    if (DBG_NTP)
    {
        printf("%s: time = %lu\n", __FUNCTION__, (unsigned long)time);
    }

    if (authStatus == VAL_ntpAuthenticateStatus_enabled)
    {
        NTP_MGR_LOCK();
        server->xmt.Ul_i.Xl_ui = time + NTP_1900_TO_1970_SECS;
        server->xmt.Ul_f.Xl_uf = (time%SYS_BLD_TICKS_PER_SECOND)* 100000000;
        HTONL_FP(&server->xmt, &xpkt.xmt);
        NTP_MGR_UNLOCK();
        /*L_ADDUF(&server->xmt, sys_authdelay); Here should add the delay of the MD5 Authencrypt*/
        if(NTP_MGR_FindKey(server->keyid,&key) == FALSE)
        {
            xpkt.exten[0] = 0;
            memset(&xpkt.mac,0,MAX_MAC_LEN);
        }
        else
        {
            if(key.ntpAuthKeyStatus ==VAL_ntpAuthKeyStatus_invalid)
            {
                xpkt.exten[0] = 0;
                memset(&xpkt.mac,0,MAX_MAC_LEN);
            }else
            {
                unsigned char digest[16];
                xpkt.exten[0] = htonl(server->keyid);
                L_MD5_MDStringAndPxt((UI8_T *)digest,(UI8_T *)key.k.MD5_key,key.keylen,(UI32_T *)&xpkt,LEN_PKT_NOMAC);
                memcpy(xpkt.mac,digest,MAX_MAC_LEN);
            }
        }
        if(NTP_TASK_Get_SocketId(&socketid) == TRUE)
        {
            sendto( socketid, (char *)&xpkt, LEN_PKT_NOMAC+MAX_MAC_LEN, 0,
            (struct sockaddr *)&(server->srcadr), sizeof(struct sockaddr));
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        NTP_MGR_LOCK();
        server->xmt.Ul_i.Xl_ui = time + NTP_1900_TO_1970_SECS;
        server->xmt.Ul_f.Xl_uf = (time%SYS_BLD_TICKS_PER_SECOND)* 100000000;
        HTONL_FP(&server->xmt, &xpkt.xmt);
        NTP_MGR_UNLOCK();

        if(NTP_TASK_Get_SocketId(&socketid) == TRUE)
        {
            sendto( socketid, (char *)&xpkt, LEN_PKT_NOMAC, 0,
            (struct sockaddr *)&(server->srcadr), sizeof(struct sockaddr));
        }
#if 0
        get_systime(&(server->xmt));
        HTONL_FP(&server->xmt, &xpkt.xmt);
        sendpkt(&(server->srcadr), &xpkt, LEN_PKT_NOMAC);

        if (debug > 1)
            printf("transmit to %s\n", stoa(&(server->srcadr)));
#endif
    }

    /*
     * Update the server timeout and transmit count
     */
    NTP_MGR_LOCK();
    server->event_time = currentTime + sys_timeout;
    server->xmtcnt++;
    NTP_MGR_UNLOCK();
    return TRUE;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_AddServerData
 *------------------------------------------------------------------------------
 * PURPOSE  : add a sample to the server's filter registers
 * INPUT    :
 * OUTPUT   :
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_MGR_AddServerData(
    NTP_MGR_SERVER_T *server,
    I32_T d,
    l_fp *c,
    UI32_T e
    )
{
    UI32_T i;
    NTP_MGR_LOCK();
    i = server->filter_nextpt;

    if (i < NTP_SHIFT) {
        server->filter_delay[i] = d;
        server->filter_offset[i] = *c;
        server->filter_soffset[i] = LFPTOFP(c);
        server->filter_error[i] = e;
        server->filter_nextpt = (unsigned short)(i + 1);
        NTP_MGR_UNLOCK();
        return TRUE;
    }

    NTP_MGR_UNLOCK();
    return FALSE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_IsServerExist
 *------------------------------------------------------------------------------
 * PURPOSE  : check if any server exists
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - one or more servers exist
 *            FALSE - no server exists
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_MGR_IsServerExist()
{
    if (NTP_TABLE.num_servers == 0)
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetServersSendFlag
 *------------------------------------------------------------------------------
 * PURPOSE  : set send_request flag of all servers
 * INPUT    : send_request flag
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void NTP_MGR_SetServersSendFlag(BOOL_T flag)
{
    NTP_MGR_SERVER_T *server_p;

    NTP_MGR_LOCK();

    for (server_p = NTP_TABLE.server_entry; server_p != NULL; server_p = server_p->next_server)
    {
        server_p->send_request = flag;
    }

    NTP_MGR_UNLOCK();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_ClearGlobalData
 *------------------------------------------------------------------------------
 * PURPOSE  : clear all global data
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 *
 * NOTES    :
 *------------------------------------------------------------------------------*/
static void NTP_MGR_ClearGlobalData()
{
    NTP_MGR_LOCK();
    debug = 0;                  /* Debugging flag */
    alarm_flag = 0;             /* Alarm flag.  Set when an alarm occurs */
    sys_version = NTP_VERSION;  /* version to poll with */
    currentTime = 0;            /* The current internal time */
    server_done = FALSE;
    always_step = 0;
    never_step = 0;
    NTP_MGR_UNLOCK();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_AlarmTimerRoutine
 *------------------------------------------------------------------------------
 * PURPOSE  : Process a timer to polling the specified server
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
static void NTP_MGR_AlarmTimerRoutine(NTP_MGR_SERVER_T  *server)
{
    /* Bump the current idea of the time
     */
    NTP_MGR_LOCK();
    currentTime++;
    NTP_MGR_UNLOCK();

    /* Search who's event timers have expired.
     * Give these to the transmit routine.
     */
    if (server->event_time != 0 && server->event_time <= currentTime)
    {
        NTP_MGR_Transmit_Packet(server);
    }
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_ClockAdjust
 *------------------------------------------------------------------------------
 * PURPOSE  : After polling all server , this routine will do clock adjust
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :Including clock_select and clock_filter.
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_MGR_ClockAdjust()
{
    register NTP_MGR_SERVER_T *sp, *server;
    I32_T absoffset;
    I32_T time;
    UI32_T sysTick;
    UI32_T current_time;

    for (sp = NTP_TABLE.server_entry; sp != NULL; sp = sp->next_server)
        NTP_MGR_ClockFilter(sp);
    server = NTP_MGR_ClockSelect();

#if 0
    if (debug || simple_query) {
        for (sp = sys_servers; sp != NULL; sp = sp->next_server)
          printserver(sp, stdout);
    }
#endif
    if (server == 0)
    {
        if(DBG_NTP)
        {
            printf("no server suitable for synchronization found \n");
        }
        return(FALSE);
    }

    memcpy(&lastUpdat_server,server,sizeof(NTP_MGR_SERVER_T));
    absoffset = server->soffset;

    if (absoffset < 0)
        absoffset = -absoffset;

    server->offset.Ul_i.Xl_i=server->offset.Ul_i.Xl_i+MAXDISTANCE;

    if(DBG_NTP)
    {
        UI32_T ip_address;
        UI8_T *p;
        ip_address = htonl(server->srcadr.sin_addr.s_addr);
        p = (UI8_T *)&ip_address;
        printf("Adjust time offset : %lu   Server: %d.%d.%d.%d\n",(unsigned long)server->offset.Ul_i.Xl_ui,p[0], p[1], p[2], p[3]);
    }

    sysTick = SYSFUN_GetSysTick();
    SYS_TIME_GetUTC(&current_time);
    time = current_time + server->offset.Ul_i.Xl_i;
    NTP_MGR_LOCK();
    lastUpdate_time = time;
    NTP_MGR_UNLOCK();

    if (DBG_NTP)
    {
        printf("%s: time = %lu, new time=%lu\n", __FUNCTION__, (unsigned long)current_time, (unsigned long)time);
    }

    SYS_TIME_SetRealTimeClockBySeconds(time, sysTick);
    return TRUE;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_ClockFilter
 *------------------------------------------------------------------------------
 * PURPOSE  : determine a server's delay, dispersion and offset
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
static void NTP_MGR_ClockFilter(register NTP_MGR_SERVER_T *server)
{
    register int i, j;
    UI32_T ord[NTP_SHIFT];

    /*
     * Sort indices into increasing delay order
     */
    for (i = 0; i < sys_samples; i++)
        ord[i] = i;

    for (i = 0; i < (sys_samples-1); i++)
    {
        for (j = i+1; j < sys_samples; j++)
        {
            if (server->filter_delay[ord[j]] == 0)
                continue;

            if (server->filter_delay[ord[i]] == 0
              || (server->filter_delay[ord[i]] > server->filter_delay[ord[j]]))
            {
                register UI32_T tmp;
                tmp = ord[i];
                ord[i] = ord[j];
                ord[j] = tmp;
            }
        }
    }

    /*
     * Now compute the dispersion, and assign values to delay and
     * offset.  If there are no samples in the register, delay and
     * offset go to zero and dispersion is set to the maximum.
     */
    if (server->filter_delay[ord[0]] == 0)
    {
        server->delay = 0;
        L_CLR(&server->offset);
        server->soffset = 0;
        server->dispersion = PEER_MAXDISP;
    }
    else
    {
        register I32_T d;
        server->delay = server->filter_delay[ord[0]];
        server->offset = server->filter_offset[ord[0]];
        server->soffset = LFPTOFP(&server->offset);
        server->dispersion = 0;

        for (i = 1; i < sys_samples; i++)
        {
            if (server->filter_delay[ord[i]] == 0)
                d = PEER_MAXDISP;
            else
            {
                d = server->filter_soffset[ord[i]] - server->filter_soffset[ord[0]];

                if (d < 0)
                    d = -d;

                if (d > PEER_MAXDISP)
                    d = PEER_MAXDISP;
            }
            /*
             * XXX This *knows* PEER_FILTER is 1/2
             */
            server->dispersion += (UI32_T)(d) >> i;
        }
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_ClockSelect
 *------------------------------------------------------------------------------
 * PURPOSE  : select the pick-of-the-litter clock from the samples
 *      we've got.
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :Including clock_select and clock_filter.
 *------------------------------------------------------------------------------*/
static NTP_MGR_SERVER_T * NTP_MGR_ClockSelect()
{
    register NTP_MGR_SERVER_T *server;
    register UI32_T i;
    register UI32_T nlist;
    register I32_T d;
    register UI32_T j;
    register UI32_T n;
    I32_T local_threshold;
    NTP_MGR_SERVER_T *server_list[NTP_MAXCLOCK];
    UI32_T server_badness[NTP_MAXCLOCK];
    NTP_MGR_SERVER_T *sys_server;

    /*
     * This first chunk of code is supposed to go through all
     * servers we know about to find the NTP_MAXLIST servers which
     * are most likely to succeed.  We run through the list
     * doing the sanity checks and trying to insert anyone who
     * looks okay.  We are at all times aware that we should
     * only keep samples from the top two strata and we only need
     * NTP_MAXLIST of them.
     */
    nlist = 0;  /* none yet */

    for (server = NTP_TABLE.server_entry; server != NULL; server = server->next_server)
    {
        if (server->delay == 0)
        {
            if (DBG_NTP)
                printf("exit from server->delay =0 \n");
            continue;   /* no data */
        }

        if (server->stratum > NTP_INFIN)
        {
            if (DBG_NTP)
                printf("exit from server->stratum\n");
            continue;   /* stratum no good */
        }

        if (server->delay > NTP_MAXWGT)
        {
            if (DBG_NTP)
                printf("exit from server->delay > NTP_MAXWGT\n");
            continue;   /* too far away */
        }

        if (server->leap == LEAP_NOTINSYNC)
        {
            if (DBG_NTP)
                printf("exit from server->leap\n");
            continue;   /* he's in trouble */
        }

        if (!L_ISHIS(&server->org, &server->reftime))
        {
            if (DBG_NTP)
                printf("exit from server->org\n");
            continue;   /* very broken host */
        }

        if ((server->org.Ul_i.Xl_ui- server->reftime.Ul_i.Xl_ui) >= NTP_MAXAGE)
        {
            if (DBG_NTP)
                printf("exit from server->reftime\n");
            continue; /* too long without sync */
        }

        if (server->trust != 0)
        {
            if (DBG_NTP)
               printf("exit from server->trust\n");
            continue;
        }

        /*
         * This one seems sane.  Find where he belongs
         * on the list.
         */
        d = server->dispersion + server->dispersion;

        for (i = 0; i < nlist; i++)
        {
            if (server->stratum <= server_list[i]->stratum)
                break;
        }

        for ( ; i < nlist; i++)
        {
            if (server->stratum < server_list[i]->stratum)
                break;

            if (d < (I32_T) server_badness[i])
                break;
        }

        /*
         * If i points past the end of the list, this
         * guy is a loser, else stick him in.
         */
        if (i >= NTP_MAXLIST)
            continue;

        for (j = nlist; j > i; j--)
        {
            if (j < NTP_MAXLIST)
            {
                server_list[j] = server_list[j-1];
                server_badness[j] = server_badness[j-1];
            }
        }

        server_list[i] = server;
        server_badness[i] = d;

        if (nlist < NTP_MAXLIST)
            nlist++;
    }

    /*
     * Got the five-or-less best.  Cut the list where the number of
     * strata exceeds two.
     */
    j = 0;

    for (i = 1; i < nlist; i++)
    {
        if (server_list[i]->stratum > server_list[i-1]->stratum)
        {
            if (++j == 2)
            {
                nlist = i;
                break;
            }
        }
    }

    /*
     * Whew!  What we should have by now is 0 to 5 candidates for
     * the job of syncing us.  If we have none, we're out of luck.
     * If we have one, he's a winner.  If we have more, do falseticker
     * detection.
     */

    if (nlist == 0)
    {
        sys_server = 0;
    }
    else if (nlist == 1)
    {
        sys_server = server_list[0];
    }
    else
    {
        /*
         * Re-sort by stratum, bdelay estimate quality and
         * server.delay.
         */
        for (i = 0; i < nlist-1; i++)
        {
            for (j = i+1; j < nlist; j++)
            {
                if (server_list[i]->stratum < server_list[j]->stratum)
                    break;  /* already sorted by stratum */

                if (server_list[i]->delay < server_list[j]->delay)
                    continue;

                server = server_list[i];
                server_list[i] = server_list[j];
                server_list[j] = server;
            }
        }

        /*
         * Calculate the fixed part of the dispersion limit
         */
        local_threshold = (FP_SECOND >> (-(int)NTPDATE_PRECISION))+ NTP_MAXSKW;

        /*
         * Now drop samples until we're down to one.
         */
        while (nlist > 1)
        {
            for (n = 0; n < nlist; n++)
            {
                 server_badness[n] = 0;

                 for (j = 0; j < nlist; j++)
                 {
                     if (j == n) /* with self? */
                         continue;

                     d = server_list[j]->soffset - server_list[n]->soffset;

                     if (d < 0)  /* absolute value */
                         d = -d;

                     /*
                      * XXX This code *knows* that
                      * NTP_SELECT is 3/4
                      */

                     for (i = 0; i < j; i++)
                         d = (d>>1) + (d>>2);

                      server_badness[n] += d;
                 }
            }

            /*
             * We now have an array of nlist badness
             * coefficients.  Find the badest.  Find
             * the minimum precision while we're at
             * it.
             */
            i = 0;
            n = server_list[0]->precision;

            for (j = 1; j < nlist; j++)
            {
              if (server_badness[j] >= server_badness[i])
                  i = j;

              if (n > server_list[j]->precision)
                  n = server_list[j]->precision;
            }

            /*
             * i is the index of the server with the worst
             * dispersion.  If his dispersion is less than
             * the threshold, stop now, else delete him and
             * continue around again.
             */
            if ((I32_T) server_badness[i] < (local_threshold + (FP_SECOND >> (-n))))
                break;

            for (j = i + 1; j < nlist; j++)
                server_list[j-1] = server_list[j];

            nlist--;
        }

        /*
         * What remains is a list of less than 5 servers.
         * Take the best.
         */
        sys_server = server_list[0];
    }

    return sys_server;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_Alarming
 *------------------------------------------------------------------------------
 * PURPOSE  : when interrutp occur , set the flag
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    : every 20 ticks ,the function will be called.
 *------------------------------------------------------------------------------*/
void NTP_MGR_Alarming()
{
    NTP_MGR_LOCK();
    alarm_flag++;
    NTP_MGR_UNLOCK();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update server for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetLastUpdateServer(NTP_MGR_SERVER_T *serv)
{
    if (NULL == serv)
    {
         EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_SERVER_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP server");
         return (FALSE);
    }

    NTP_MGR_LOCK();
    memcpy(serv,&lastUpdat_server,sizeof(NTP_MGR_SERVER_T));
    NTP_MGR_UNLOCK();
    return (TRUE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update time for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetLastUpdateTime(I32_T *time)
{
    if (NULL == time)
    {
         EH_MGR_Handle_Exception1 (SYS_MODULE_NTP, NTP_MGR_SERVER_FUNC_NO, EH_TYPE_ret_FAILED_TO_GET, SYSLOG_LEVEL_ERR,"NTP server");
         return (FALSE);
    }

    NTP_MGR_LOCK();
    *time = lastUpdate_time;
    NTP_MGR_UNLOCK();
    return (TRUE);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_AddAuthKey_Encrypted
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication encrypted key to the list ,use in provison
* INPUT    : 1.index 2. encryptedkey
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_AddAuthKey_Encrypted(UI32_T index, char *encryptedkey)
{
    char M_key[MAXSIZE_ntpAuthKeyWord+1]={0};
    NTP_MGR_AUTHKEY_T *sk,*sk1,*sk2 = NULL;

    /* check key id */
    if (index > MAX_ntpServerKeyId || index < 0)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    if(NTP_TABLE.num_authkeys >= NTP_MAXAUTHKEY)
    {
        return (FALSE);
    }

    if(NTP_MGR_Decode(encryptedkey, M_key)!= TRUE)
    {
        return (FALSE);
    }

    sk = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

    if(NULL == sk)
    {
        return (FALSE);
    }

    NTP_MGR_LOCK();
    memset(sk,0,sizeof(NTP_MGR_AUTHKEY_T));
    strncpy(sk->k.MD5_key, M_key, sizeof(sk->k.MD5_key)-1);
    sk->k.MD5_key[sizeof(sk->k.MD5_key)-1] = '\0';
    strncpy(sk->password, encryptedkey, sizeof(sk->password)-1);
    sk->password[sizeof(sk->password)-1] = '\0';
    sk->keylen = strlen(sk->k.MD5_key);
    if(sk->keylen > MAXSIZE_ntpAuthKeyWord)
       sk->keylen = MAXSIZE_ntpAuthKeyWord;
    sk->keyid = index;
    sk->flags |= KEY_MD5;
    sk->lifetime = 0;
    sk->ntpAuthKeyStatus = VAL_ntpAuthKeyStatus_valid;

    sk1=NTP_TABLE.authKey_entry[KEYHASH(index)];

    if(NTP_TABLE.authKey_entry[KEYHASH(index)] == NULL)
    {
        NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
        sk->next= NULL;
    }
    else
    {
        while((sk->keyid > sk1->keyid) && (sk1->next!= NULL))
        {
            sk2=sk1;
            sk1=sk1->next;
        }

        if(sk->keyid < sk1->keyid)
        {
            if(sk1== NTP_TABLE.authKey_entry[KEYHASH(index)])
            {
                NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
                sk->next= sk1;
            }
            else
            {
                sk2->next = sk;
                sk->next = sk1;
            }
        }
        else
        {
            sk1->next = sk;
            sk->next = NULL;
        }
    }

    NTP_TABLE.num_authkeys++;
    NTP_MGR_UNLOCK();
    return (TRUE);
}
/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_Show_Delay
*------------------------------------------------------------------------------
* PURPOSE  : show all the servers delay in backdoor
* INPUT    :
* OUTPUT   :
* RETURN   :
* NOTES    : debug use
*------------------------------------------------------------------------------*/
void NTP_MGR_Show_Delay()
{
    register NTP_MGR_SERVER_T *server;
    UI32_T ip_address;
    UI8_T *p;

    for (server = NTP_TABLE.server_entry; server != NULL; server = server->next_server)
    {
        ip_address = htonl(server->srcadr.sin_addr.s_addr);
        p = (UI8_T *)&ip_address;
        BACKDOOR_MGR_Printf("\nserver: %d.%d.%d.%d  delay: %lu\n",p[0], p[1], p[2], p[3],(unsigned long)server->delay);
    }
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_Debug_Enable
*------------------------------------------------------------------------------
* PURPOSE  : show Debug message
* INPUT    :
* OUTPUT   :
* RETURN   :
* NOTES    : debug use
*------------------------------------------------------------------------------*/
void NTP_MGR_Debug_Enable()
{
    DBG_NTP = TRUE;
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_Debug_Enable
*------------------------------------------------------------------------------
* PURPOSE  : show Debug message
* INPUT    :
* OUTPUT   :
* RETURN   :
* NOTES    : debug use
*------------------------------------------------------------------------------*/
void NTP_MGR_Debug_Disable()
{
    DBG_NTP = FALSE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_FindNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_FindNextServer(UI32_T ipadd, NTP_MGR_SERVER_T *serv)
{
    NTP_MGR_SERVER_T    *sp;

    NTP_MGR_LOCK();

    for(sp=NTP_TABLE.server_entry; sp != NULL;
        sp=sp->next_server)
    {
        if(memcmp(&ipadd, &(sp->srcadr.sin_addr.s_addr), sizeof(sp->srcadr.sin_addr.s_addr)) < 0)
        {
            memcpy(serv,sp, sizeof(NTP_MGR_SERVER_T));
            NTP_MGR_UNLOCK();
            return (TRUE);
        }
    }

    NTP_MGR_UNLOCK();
    return (FALSE);
}


/*
  note:out_buf' space must >= buf 3*len+4
       out_buf must memset 0
   - QingfengZhang, 30 March, 2005 2:06:23 */
BOOL_T NTP_MGR_Encode(int flag,char *buf,char *encode_passwd)
{
    int i;
    int len;
    int r_num;
    int pass_num;
    int value;
    int temp;
    int result[MAXSIZE_ntpAuthKeyWord+1];
    char password[MAXSIZE_ntpAuthKeyWord+1] = "\0";

    if (buf == NULL)
        return FALSE;

    if(flag !=0 && flag !=7)
        return FALSE;

    len = strlen(buf);

    if(len < MINSIZE_ntpAuthKeyWord)
        return FALSE;
    else if(len > MAXSIZE_ntpAuthKeyWord)
        return FALSE;

    for(i=0; i<len; i++)
    {
        if(*(buf+i)>'~' || *(buf+i)<' ')
            return FALSE;
    }

    strcpy(password, buf);

    if(encode_passwd == NULL)
        return FALSE;

    if(flag == NTP_MGR_PASS_TAG_NO)
    {
        strcpy(encode_passwd, password);
    }
    else if(flag == NTP_MGR_PASS_TAG_YES)
    {
        int head;
        int r_rest_num;

        r_num = rand();
        r_rest_num = r_num % NTP_MGR_PASS_BASE_NUM;
        head = NTP_MGR_int_to_char(r_num % (4*NTP_MGR_PASS_BASE_NUM));
        NTP_MGR_exchang_order(password, r_rest_num);
        pass_num = (*password-NTP_MGR_PASS_BASE_MOVE_NUM)*NTP_MGR_PASS_BASE_NUM+r_rest_num;
        result[0] = pass_num;
        value = NTP_MGR_int_reverse(result[0]);
        temp = (value%10) + (value/10%10) + (value/100%10) + 'A' -1;
        sprintf(encode_passwd, "%c%02d%c", head, value/10, temp);

        for(i=1; i<len; i++)
        {
            char tmp_str[4] = "\0";
            int  num;

            if(result[i-1]%2)
            {
                num = rand();
                pass_num = (*(password+i) - NTP_MGR_PASS_BASE_MOVE_NUM)*NTP_MGR_PASS_BASE_NUM + num%NTP_MGR_PASS_BASE_NUM;
                result[i] = pass_num;
                value = NTP_MGR_int_reverse(result[i]);
                temp = (value%10) + (value/10%10) + (value/100%10) + 'A' -1;
                sprintf(tmp_str, "%02d%c", value/10, temp);
                strcat(encode_passwd, tmp_str);
            }
            else
            {
                pass_num = *(password+i) - ' ';
                pass_num = NTP_MGR_PASS_TOKEN_NUM - pass_num;
                result[i] = pass_num + result[i-1]%10/2;
                value = result[i];
                sprintf(tmp_str, "%02d", value);
                strcat(encode_passwd, tmp_str);
            }
        }

        NTP_MGR_exchang_order(encode_passwd, r_rest_num);
    }

    return TRUE;
}

BOOL_T NTP_MGR_Decode(char *buf,char *passwd)
{
    int i,j;
    int len;
    int result[MAXSIZE_ntpAuthKeyWord+1];
    int temp0, temp1, temp2;
    char symbol[MAXSIZE_ntpAuthKeyWord+1] = "\0";
    char password[3*MAXSIZE_ntpAuthKeyWord+2] = "\0";

    if(buf == NULL)
          return FALSE;

    len=strlen(buf);
    strcpy(password, buf);
    NTP_MGR_exchang_order(password, NTP_MGR_char_to_int(*password) % 9);
    temp0 = *(password+1) - '0';
    temp1 = *(password+2) - '0';
    temp2 = (*(password+3)-'A'+1) - temp1 - temp0;
    result[0] = temp2*100 + temp1*10 + temp0*1;
    symbol[0] = result[0]/NTP_MGR_PASS_BASE_NUM + NTP_MGR_PASS_BASE_MOVE_NUM;
    i = 4;
    j = 1;

    while(i < len)
    {
        if(result[j-1]%2)
        {
            temp0 = *(password+i+0) - '0';
            temp1 = *(password+i+1) - '0';
            temp2 = (*(password+i+2)-'A'+1) -temp1 -temp0;
            result[j] = temp2*100 + temp1*10 + temp0*1;
            symbol[j] = result[j]/NTP_MGR_PASS_BASE_NUM + NTP_MGR_PASS_BASE_MOVE_NUM;
            i += 3;
        }

        else
        {
            result[j] = (*(password+i+0)-'0')*10 + (*(password+i+1)-'0')*1;
            symbol[j] = result[j];
            symbol[j] -= result[j-1]%10/2;
            symbol[j] = NTP_MGR_PASS_TOKEN_NUM-symbol[j];
            symbol[j] += ' ';
            i += 2;
        }

        j++;
    }

    for(i=0; i<(int)strlen(symbol); i++)
    {
        if(*(symbol+i)>'~' || *(symbol+i)<' ')
            return FALSE;
    }


    NTP_MGR_exchang_order(symbol, NTP_MGR_char_to_int(*password) % 9);

    if(passwd == NULL)
        return FALSE;

    strcpy(passwd, symbol);
    return TRUE;
}

static void NTP_MGR_exchang_order(char *str, int num)
{
    int i, j, k;
    char ch;
    int len = strlen(str) ;

    assert(str != NULL);

    if(num < 2)
        return ;

    if(num > len - 1)
        num = len - 1;

    for(i=1; i<len; i+=num)
    {
        if(i + num > len )
            num = len - i ;

        for(j=0, k=num-1; j<k; j++, k--)
        {
            ch = *(str + i + j);
            *(str + i + j) = *(str + i + k);
            *(str + i + k) = ch;
        }
    }
}

static int NTP_MGR_int_reverse(int num)
{
    int i;
    int result;
    int digit;

    for(digit=10; num/digit!=0; digit*=10);
    result = 0;

    for(i=digit/10; i!=0; i/=10)
    {
        result += (num%10)*i;
        num /= 10;
    }

    return result;
}

static int NTP_MGR_int_to_char(int num)
{
    int head = num ;

    if(head >= 0 && head <= 9)
        head += '0';
    else if(head >= 10 && head <= 35)
        head += 'A' - 10;

    return head;
}


static int NTP_MGR_char_to_int(char ch)
{
    int head = ch ;

    if(head >= '0' && head <= '9')
        head -= '0';
    else if(head >= 'A' && head <= 'Z')
        head -= ('A' - 10);

    return head;
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_SetAuthKeyStatus
*------------------------------------------------------------------------------
* PURPOSE  : Set an authentication key status
* INPUT    : 1.index 2. status : VAL_ntpAuthKeyStatus_valid/VAL_ntpAuthKeyStatus_invalid
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : only used for snmp do set authentication key status
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_SetAuthKeyStatus(UI32_T index, UI32_T status)
{
    NTP_MGR_AUTHKEY_T *sk,*sk1,*sk2 = NULL;
    NTP_MGR_AUTHKEY_T findKey;

    /* check key id */
    if (index > MAX_ntpServerKeyId || index < 0)
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    if (status == VAL_ntpAuthKeyStatus_invalid)
    {
        if (NTP_MGR_DeleteAuthKey(index) != TRUE)
        {
            EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
            return (FALSE);
        }
    }
    else if (status == VAL_ntpAuthKeyStatus_valid)
    {
        /* If keyid alread exist , status already is valid */
        if(NTP_MGR_FindKey(index,&findKey))
        {
            EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
            return (TRUE);
        }
        /* If key not exist , create a new authkeky and add to the Hash table*/
        else
        {
            if(NTP_TABLE.num_authkeys >= NTP_MAXAUTHKEY)
            {
                return (FALSE);
            }

            sk = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

            if(NULL == sk)
            {
                return (FALSE);
            }

            NTP_MGR_LOCK();
            memset(sk,0,sizeof(NTP_MGR_AUTHKEY_T));
            sk->keyid = index;
            sk->flags |= KEY_MD5;
            sk->lifetime = 0;
            sk->ntpAuthKeyStatus = VAL_ntpAuthKeyStatus_invalid;
            sk1=NTP_TABLE.authKey_entry[KEYHASH(index)];

            if(NTP_TABLE.authKey_entry[KEYHASH(index)] == NULL)
            {
                NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
                sk->next= NULL;
            }
            else
            {
                while((sk->keyid > sk1->keyid) && (sk1->next!= NULL))
                {
                    sk2=sk1;
                    sk1=sk1->next;
                }

                if(sk->keyid < sk1->keyid)
                {
                    if(sk1== NTP_TABLE.authKey_entry[KEYHASH(index)])
                    {
                        NTP_TABLE.authKey_entry[KEYHASH(index)] = sk;
                        sk->next= sk1;
                    }
                    else
                    {
                        sk2->next = sk;
                        sk->next = sk1;
                    }
                }
                else
                {
                    sk1->next = sk;
                    sk->next = NULL;
                }
            }

            NTP_TABLE.num_authkeys++;
            NTP_MGR_UNLOCK();
        }
    }
    else
    {
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_MGR_INVALID_KEY_FUNC_NO, EH_TYPE_ret_INVALID_VALUE, SYSLOG_LEVEL_INFO);
        return (FALSE);
    }

    return (TRUE);
}

/* Digests a string and prints the result. Add for NTP - QingfengZhang, 06 April, 2005 2:19:08 */
/*  FUNCTION NAME : L_MD5_MDStringAndPxt
 *  PURPOSE:
 *      Encode a string and packet by MD5.
 *  INPUT:
 *      string       -- input string for digest
 *      stringlen    -- input string length
 *      pkt          -- input packet for digest
 *      len          -- input packet length
 *  OUTPUT:
 *      digest -- output digest string.
 *
 *  RETURN:
 *      None
 *
 *  NOTES:
 *      This routine is used in NTP.
 */
void L_MD5_MDStringAndPxt (UI8_T *digest, UI8_T *string,UI32_T stringlen,UI32_T *pkt,UI32_T len)
{
    UI8_T *string_ar[] = {string, (UI8_T *)pkt};
    UI32_T len_ar[] = {stringlen, len};

    L_MD5_MDString_N(digest, 2, string_ar, len_ar);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_RifUp_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : RIF up callback handler.
 * INPUT    : ifindex   -- The ifindex of active rif.
 *            addr_p    -- The IP address of active rif.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_RifUp_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    NTP_MGR_LOCK();
    is_rif_up = TRUE;

    if (   (TRUE == is_port_forwarding)
        && (0 == start_send_packet_time)
        )
    {
        SYS_TIME_GetRealTimeBySec(&start_send_packet_time);
        start_send_packet_time += NTP_MGR_SEND_DELAY_TIME;
    }
    NTP_MGR_UNLOCK();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_LPortEnterForwarding_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : Lport enters forwarding callback handler.
 * INPUT    : xstid -- Index of the spanning tree.
 *            lport -- Logical port number.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_LPortEnterForwarding_Callback(UI32_T xstp_id, UI32_T lport)
{
    NTP_MGR_LOCK();
    is_port_forwarding = TRUE;

    if (   (TRUE == is_rif_up)
        && (0 == start_send_packet_time)
        )
    {
        SYS_TIME_GetRealTimeBySec(&start_send_packet_time);
        start_send_packet_time += NTP_MGR_SEND_DELAY_TIME;
    }
    NTP_MGR_UNLOCK();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_LPortLeaveForwarding_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : Lport leaves forwarding callback handler.
 * INPUT    : xstid -- Index of the spanning tree.
 *            lport -- Logical port number.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_LPortLeaveForwarding_Callback(UI32_T xstp_id, UI32_T lport)
{
}