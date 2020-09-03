/* MODULE NAME:  dying_gasp.c
 * PURPOSE:
 *     Dying gasp is a feature that will send trap just
 *     before power failure event occurs.
 *
 * NOTES:
 *     To simplify the call path, this CSC will be put
 *     in the snmp_proc process.
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    7/29/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2010
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "l_mm.h"
#include "syslog_pmgr.h"
#include "syslog_type.h"
#include "backdoor_mgr.h"
#include "dying_gasp_type.h"
#include "dying_gasp.h"
#include "trap_event.h"
#include "snmp_pmgr.h"
#include "sys_time.h"
#include "stktplg_board.h"
#include "stktplg_om.h"
#include "lan.h"
#include "oam_pom.h"
#include "swctrl_pom.h"
#include "leaf_4878.h"

#if (SYS_CPNT_EFM_OAM == TRUE)
#include "uc_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_EFM_OAM == TRUE)
#define OAM_FRAME_SA_OFFSET                12
#define OAM_FRAME_PDU_LEN                  46
#define OAM_FRAME_PROTOCOLS_SUBTYPE_OFFSET 0
#define OAM_FRAME_FLAGS_OFFSET             1
#define OAM_FRAME_ETHER_TYPE               0x8809

#define OAM_STATUS_POLLING_INTERVAL        500 /* 500 ticks = 5 sec */
#define DYING_GASP_TASK_TIMER_EVENT        BIT_0
#define DYING_GASP_TASK_BD_SUSPEND_EVENT   BIT_1 /* for backdoor use only */
#define DYING_GASP_TASK_ALL_EVENT          (DYING_GASP_TASK_TIMER_EVENT | DYING_GASP_TASK_BD_SUSPEND_EVENT)
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */

/* MACRO FUNCTION DECLARATIONS
 */
#if (SYS_CPNT_EFM_OAM == TRUE)
#define DYING_GASP_SET_PORT_PBMP(pbmp, port_id) (pbmp)[((port_id)-1)/8] |= (1<<(((port_id)-1)%8))
#define DYING_GASP_CLEAR_PORT_PBMP(pbmp, port_id) (pbmp)[((port_id)-1)/8] &= ~(1<<(((port_id)-1)%8))
#define DYING_GASP_PORT_PBMP_IS_SET(pbmp, port_id) (((pbmp)[((port_id)-1)/8] & (1<<(((port_id)-1)%8))) != 0)
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */

#define DYING_GASP_DBGMSG(fmtstr, arg...) if(dying_gasp_dbg_msg_flag==TRUE){BACKDOOR_MGR_Printf(fmtstr, ##arg);}

/* DATA TYPE DECLARATIONS
 */
enum {
    DYING_GASP_TRACE_ID_OAM_FRAME=0
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void DYING_GASP_INT_TASK_Main(void);
#if (SYS_CPNT_EFM_OAM == TRUE)
static void DYING_GASP_TASK_Main(void);
static void DYING_GASP_MacCalculation(UI8_T *mac, UI32_T array_index, UI32_T added_value);
static void DYING_GASP_UpdateOamStatus(void);
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
static void DYING_GASP_BACKDOOR_Main(void);

/* STATIC VARIABLE DECLARATIONS
 */

#if (SYS_CPNT_EFM_OAM == TRUE)
static UI32_T               dying_gasp_thread_id;
static L_MM_Mref_Handle_T*  oam_frames_mref[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
static UI8_T*               oam_frames_pktbuf[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
static UI8_T                oam_enabled_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
static UI8_T                cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
static const UI8_T          oam_frame_da[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
static BOOL_T               dying_gasp_dbg_msg_flag=FALSE;
static BOOL_T               dying_gasp_trap_enabled_flag=TRUE;

/* IMPORTED SUBPROGRAM BODIES
 */
extern BOOL_T SNMP_MGR_SendTrap(TRAP_EVENT_TrapData_T *trap_data);

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all process resource.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DYING_GASP_InitiateProcessResource(void)
{
#if (SYS_CPNT_EFM_OAM == TRUE)
    UC_MGR_Sys_Info_T sys_info;
    UI8_T  i;

    if(UC_MGR_InitiateProcessResources()==FALSE)
    {
        printf("%s(%d)UC_MGR_InitiateProcessResources failed.\r\n", __FUNCTION__, __LINE__);
    }

    memset(oam_frames_mref, 0, sizeof(oam_frames_mref));
    memset(oam_frames_pktbuf, 0, sizeof(oam_frames_pktbuf));
    memset(oam_enabled_pbmp, 0, sizeof(oam_enabled_pbmp));
    memset(cpu_mac, 0, sizeof(cpu_mac));

    /* get cpu mac
     */
    if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
    {
        printf("%s(%d)WARNING!!! Failed to get cpu mac.\r\n", __FUNCTION__, __LINE__);
    }
    else
    {
        memcpy(cpu_mac, sys_info.mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    }

    /* prepare tx packet buffer for all front ports on the device
     */
    for(i=0; i<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; i++)
    {
        oam_frames_mref[i]=L_MM_AllocateTxBuffer(OAM_FRAME_PDU_LEN, L_MM_USER_ID2(SYS_MODULE_DYING_GASP, DYING_GASP_TRACE_ID_OAM_FRAME));
        if(oam_frames_mref[i]==NULL)
        {
            printf("\r\nWARNING!!! Failed to allocate packet buffer for oam frames(index %hu). Dying gasp might not work.\r\n", i);
        }
    }

    /* fill data into tx packet buffer
     */
    for(i=0; i<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; i++)
    {
        UI32_T pdu_len/*lport*/;
        UI8_T* pdu;

        if(oam_frames_mref[i]==NULL)
            continue;

        /* get pdu from mref
         */
        pdu=L_MM_Mref_GetPdu(oam_frames_mref[i], &pdu_len);
        oam_frames_pktbuf[i]=pdu;
        if(pdu==NULL)
        {
            printf("\r\nWARNING!!! Failed to get pdu for port %hu. Dying gasp will not work.\r\n", i);
            return;
        }

        memset(pdu, 0, pdu_len);

        pdu[OAM_FRAME_PROTOCOLS_SUBTYPE_OFFSET]   = 0x03; /* OAM = 0x03 */
        pdu[OAM_FRAME_FLAGS_OFFSET]               = 0x00;
        pdu[OAM_FRAME_FLAGS_OFFSET+1]             = 0x02; /* Bit 1: Dying Gasp = TRUE */

        /* Put SA(port mac) for each port at pdu-OAM_FRAME_SA_OFFSET
         */
        /* Call SWCTRL_POM_GetPortMac() here will always failed because
         * all ipc requests are returning false in SWCTRL_OM_HandleIPCReqMsg()
         * when the csc is in transition mode. So evaluate port mac here directly.
         */
        memcpy(pdu-OAM_FRAME_SA_OFFSET, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
        DYING_GASP_MacCalculation(pdu-OAM_FRAME_SA_OFFSET, 5, i+1);
    }
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DYING_GASP_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("DYING_GASP",
                                                      SYS_BLD_SNMP_GROUP_IPCMSGQ_KEY,
                                                      DYING_GASP_BACKDOOR_Main);

}


/* FUNCTION NAME : DYING_GASP_CreateTask
 * PURPOSE:
 *      Spawn task for dying gasp.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      Highest real time priority will be
 *      assigned to this task.
 */
void DYING_GASP_CreateTask(void)
{
    UI32_T thread_id;
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    if(SYSFUN_SpawnThread(SYS_BLD_DYING_GASP_INT_THREAD_PRIORITY,
                          SYS_BLD_DYING_GASP_INT_THREAD_SCHED_POLICY,
                          SYS_BLD_DYING_GASP_INT_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          DYING_GASP_INT_TASK_Main,
                          NULL,
                          &thread_id)!=SYSFUN_OK)
    {
        owner_info.level        = SYSLOG_LEVEL_CRIT;
        owner_info.module_no    = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no  = 0;
        owner_info.error_no     = 0;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, SYS_BLD_DYING_GASP_INT_CSC_THREAD_NAME, 0, 0);
        printf("%s:SYSFUN_SpawnThread fail.(%s)\n", __FUNCTION__, SYS_BLD_DYING_GASP_INT_CSC_THREAD_NAME);
    }

#if (SYS_CPNT_EFM_OAM == TRUE)
    if(SYSFUN_SpawnThread(SYS_BLD_DYING_GASP_THREAD_PRIORITY,
                          SYS_BLD_DYING_GASP_THREAD_SCHED_POLICY,
                          SYS_BLD_DYING_GASP_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          DYING_GASP_TASK_Main,
                          NULL,
                          &dying_gasp_thread_id)!=SYSFUN_OK)
    {
        owner_info.level        = SYSLOG_LEVEL_CRIT;
        owner_info.module_no    = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no  = 0;
        owner_info.error_no     = 0;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, SYS_BLD_DYING_GASP_CSC_THREAD_NAME, 0, 0);
        printf("%s:SYSFUN_SpawnThread fail.(%s)\n", __FUNCTION__, SYS_BLD_DYING_GASP_CSC_THREAD_NAME);
    }
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
}

/* LOCAL SUBPROGRAM BODIES
 */
static void DYING_GASP_INT_TASK_Main(void)
{
#if (SYS_CPNT_EFM_OAM == TRUE)
    UI32_T my_unit_id=1;
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
    BOOL_T ret;

#if (SYS_CPNT_EFM_OAM == TRUE)
    if(STKTPLG_OM_GetMyUnitID(&my_unit_id)==FALSE)
    {
        printf("%s(%d) Failed to get my unit id.\r\n", __FUNCTION__, __LINE__);
    }
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */

#if defined(SYS_HWCFG_DYING_GASP_IRQ_NUM_GET_FROM_STKTPLG)
    ret=SYSFUN_Syscall(SYSFUN_SYSCALL_DYING_GASP, DYING_GASP_TYPE_SYSCALL_CMD_ENABLE_INTERRUPT, STKTPLG_BOARD_GetDyingGaspIrqNo(), 0, 0, 0);
    if(ret==FALSE)
    {
        printf("%s(%d): Failed to enable dying gasp irq.\r\n",
            __FUNCTION__, __LINE__);
        return;
    }
#endif

    while(1)
    {
        ret=SYSFUN_Syscall(SYSFUN_SYSCALL_DYING_GASP, DYING_GASP_TYPE_SYSCALL_CMD_WAIT_EVENT, 0, 0, 0, 0);
        if(ret==TRUE)
        {
            TRAP_EVENT_TrapData_T dying_gasp_trap_data;

#if (SYS_CPNT_EFM_OAM == TRUE)
            UI8_T                      array_idx, bit_idx, egress_port;
            UI8_T                      sa[SYS_ADPT_MAC_ADDR_LEN];

            /* send oam frames
             */

            for(array_idx=0; array_idx<SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; array_idx++)
            {
                /* check oam enabled status
                 */
                if(oam_enabled_pbmp[array_idx])
                {
                    for(bit_idx=0; bit_idx<8; bit_idx++)
                    {
                        if(oam_enabled_pbmp[array_idx] & (0x1<<bit_idx))
                        {
                            egress_port=8*array_idx + bit_idx + 1;
                            if(oam_frames_mref[egress_port-1]!=NULL)
                            {
                                memcpy(sa, oam_frames_pktbuf[egress_port-1]-OAM_FRAME_SA_OFFSET, SYS_ADPT_MAC_ADDR_LEN);
                                /* add mref ref count to prevent mref being freed
                                 * we will try to send as many oam dying gasp frames as possible
                                 */
                                L_MM_Mref_AddRefCount(oam_frames_mref[egress_port-1], 1);
                                LAN_SendPacket(oam_frames_mref[egress_port-1], (UI8_T*)oam_frame_da,
                                    sa, OAM_FRAME_ETHER_TYPE, 1, OAM_FRAME_PDU_LEN, my_unit_id, egress_port,
                                    FALSE, 3 /* highest priority */);
                            }
                        }
                    }
                }
            }
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */

            if(dying_gasp_trap_enabled_flag==TRUE)
            {
                /* send trap
                 */
                memset(&dying_gasp_trap_data, 0, sizeof(dying_gasp_trap_data));
                dying_gasp_trap_data.trap_type = TRAP_EVENT_DYING_GASP;
                dying_gasp_trap_data.community[0] = '\0';
                dying_gasp_trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;

#if 0 /* to measure elapsed time */
                printf("Dying gasp before send:%lu\n", SYSFUN_GetSysTick());
#endif
                SNMP_MGR_SendTrap(&dying_gasp_trap_data);

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_08)
                /* Dying Gasp trap, OAM MIB:
                 * special request from 08 to send as dot3OamThresholdEvent,
                 * instead of the original dot3OamNonThresholdEvent,
                 * and only fills two fields in the trap parameter structure
                 */
                memset(&dying_gasp_trap_data, 0, sizeof(dying_gasp_trap_data));
                dying_gasp_trap_data.community[0] = '\0';
                dying_gasp_trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;

                dying_gasp_trap_data.trap_type = TRAP_EVENT_DOT3_OAM_THRESHOLD;
                dying_gasp_trap_data.u.dot3_oam_threshold.dot3OamEventLogType = VAL_dot3OamEventLogType_dyingGaspEvent;
                dying_gasp_trap_data.u.dot3_oam_threshold.dot3OamEventLogLocation = VAL_dot3OamEventLogLocation_local;
                SNMP_MGR_SendTrap(&dying_gasp_trap_data);
#endif  /* (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_08) */
#endif  /* (SYS_CPNT_EFM_OAM == TRUE) */
            }
        }
    }
}

#if (SYS_CPNT_EFM_OAM == TRUE)
static void DYING_GASP_TASK_Main(void)
{
    void*  timer_handle;
    UI32_T recv_events;

    timer_handle=SYSFUN_PeriodicTimer_Create();

    if(timer_handle==NULL)
    {
        printf("\r\n Warning!!! Failed to create timer. Dying gasp might not work properly.\r\n");
        return;
    }

    if(SYSFUN_PeriodicTimer_Start(timer_handle, OAM_STATUS_POLLING_INTERVAL, DYING_GASP_TASK_TIMER_EVENT)==FALSE)
    {
        printf("\r\n Warning!!! Failed to start timer. Dying gasp might not work properly.\r\n");
        return;
    }

    while(TRUE)
    {

        if(SYSFUN_ReceiveEvent(DYING_GASP_TASK_ALL_EVENT, SYSFUN_EVENT_WAIT_ANY,
            SYSFUN_TIMEOUT_WAIT_FOREVER, &recv_events)==SYSFUN_OK)
        {
            if(recv_events&DYING_GASP_TASK_BD_SUSPEND_EVENT)
            {
                printf("Dying gasp task is going to be suspended.\r\n");
                SYSFUN_SuspendThreadSelf();
                printf("Dying gasp task is resumed.\r\n");
            }

            if(recv_events&DYING_GASP_TASK_TIMER_EVENT)
            {
                DYING_GASP_UpdateOamStatus();
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DYING_GASP_MacCalculation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will output the result of mac address after adding
 *           the specified value.
 * INPUT   : mac         --  the base mac address
 *           array_index --  the index of the mac address array for the specified
 *                           value being added to
 * OUTPUT  : mac
 * RETURN  :
 * NOTE    : This function might be called recursively.
 * -------------------------------------------------------------------------*/
static void DYING_GASP_MacCalculation(UI8_T *mac, UI32_T array_index, UI32_T added_value)
{
	if (array_index <= 0)
		return;
	if ((mac[array_index]+added_value) > 0xFF)
	{
		mac[array_index] = (UI8_T)(mac[array_index] + added_value - 0x100);
		DYING_GASP_MacCalculation(mac, --array_index, 1);
	}
	else
		mac[array_index] = mac[array_index] + (UI8_T) added_value;

	return;
}

static void DYING_GASP_UpdateOamStatus(void)
{
    static EFM_OAM_ENTITY_INFO  oam_entity_info; /* defined as static variable because sizeof(EFM_OAM_ENTITY_INFO) is big */
    static EFM_OAM_EVENT_CONF_T oam_event_conf;  /* defined as static variable because sizeof(EFM_OAM_EVENT_CONF_T) is big */

    UI32_T my_unit_id;
    UI32_T port_id;
    UI32_T lport;

    if(STKTPLG_OM_GetMyUnitID(&my_unit_id)==FALSE)
    {
        printf("%s(%d) Failed to get my unit id.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    for(port_id=1; port_id<=SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port_id++)
    {
        if(SWCTRL_POM_UserPortToLogicalPort(my_unit_id, port_id, &lport)==SWCTRL_LPORT_NORMAL_PORT)
        {
            if((EFM_OAM_POM_GetOAMEntry(lport, &oam_entity_info)==TRUE) &&
               (EFM_OAM_POM_GetEventConfig(lport, &oam_event_conf)==TRUE) &&
               (oam_entity_info.local_oam_enable==EFM_OAM_ENABLED) &&
               (oam_event_conf.dying_gasp_enable==EFM_OAM_DYING_GASP_ENABLED))
            {
                DYING_GASP_SET_PORT_PBMP(oam_enabled_pbmp, port_id);
            }
            else
            {
                DYING_GASP_CLEAR_PORT_PBMP(oam_enabled_pbmp, port_id);
            }

            DYING_GASP_DBGMSG("(%lu, %lu) local_oam_enabled=%s, dying_gasp_enabled=%s\r\n",
                my_unit_id, port_id, (oam_entity_info.local_oam_enable==EFM_OAM_ENABLED)?"yes":"no",
                (oam_event_conf.dying_gasp_enable==EFM_OAM_DYING_GASP_ENABLED)?"yes":"no");
        }
    }
}
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - DYING_GASP_BACKDOOR_Main
 * ---------------------------------------------------------------------
 * PURPOSE:  dying gasp back door main
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
static void DYING_GASP_BACKDOOR_Main(void)
{
#if (SYS_CPNT_EFM_OAM == TRUE)
    I32_T  port;
    char   buf[16];
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
    BOOL_T is_exit=FALSE;
    BOOL_T rc;
    char   ch;

    while(is_exit==FALSE)
    {
        BACKDOOR_MGR_Print("\r\n1. Emulate dying gasp interrupt\r\n");
        BACKDOOR_MGR_Print("2. Enable/Disable show timestamp to death\r\n");
#if (SYS_CPNT_EFM_OAM == TRUE)
        BACKDOOR_MGR_Print("3. Suspend/Resume dying gasp task\r\n");
        BACKDOOR_MGR_Print("4. Dump oam enabled port list\r\n");
        BACKDOOR_MGR_Print("5. Modify oam enabled port\r\n");
        BACKDOOR_MGR_Print("6. Dump oam frame pkt buf\r\n");
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
        BACKDOOR_MGR_Printf("7. Toggle debug message(%u)\r\n", dying_gasp_dbg_msg_flag);
        BACKDOOR_MGR_Printf("8. Toggle dying gasp trap(%u)\r\n", dying_gasp_trap_enabled_flag);
        BACKDOOR_MGR_Print("q. exit\r\n");

        ch=(char)BACKDOOR_MGR_GetChar();

        switch(ch)
        {
            case '1':
                rc=(BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_DYING_GASP, DYING_GASP_TYPE_SYSCALL_CMD_EMULATE_INTERRUPT, 0, 0, 0, 0);
                break;
            case '2':
                BACKDOOR_MGR_Print("1: Enable, 0: Disable ?");
                ch=(char)BACKDOOR_MGR_GetChar();
                rc=(BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_DYING_GASP, DYING_GASP_TYPE_SYSCALL_CMD_SHOW_TIMESTAMP_TO_DEATH, (ch=='1')?TRUE:FALSE, 0, 0, 0);
                break;
#if (SYS_CPNT_EFM_OAM == TRUE)
            case '3':
                BACKDOOR_MGR_Print("0:Suspend , 1: Resume ?");
                ch=(char)BACKDOOR_MGR_GetChar();
                if(ch=='0')
                {
                    SYSFUN_SendEvent(dying_gasp_thread_id, DYING_GASP_TASK_BD_SUSPEND_EVENT);
                }
                else
                {
                    SYSFUN_ResumeThread(dying_gasp_thread_id);
                }
                break;
            case '4':
            {
                BACKDOOR_MGR_Printf("OAM enabled ports: ");
                for(port=1; port<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
                {
                    if(DYING_GASP_PORT_PBMP_IS_SET(oam_enabled_pbmp, port))
                        BACKDOOR_MGR_Printf("%hu ", port);
                }
                BACKDOOR_MGR_Printf("\r\n");
            }
                break;
            case '5':
            {
                BACKDOOR_MGR_Printf("Which port? ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                port=atoi(buf);
                if(port<=0 || port >=SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
                {
                    BACKDOOR_MGR_Printf("Invalid port id: %d\r\n", port);
                    break;
                }
                BACKDOOR_MGR_Print("0:Disable , 1: Enable ?");
                ch=(char)BACKDOOR_MGR_GetChar();
                if(ch=='0')
                    DYING_GASP_CLEAR_PORT_PBMP(oam_enabled_pbmp, port);
                else
                    DYING_GASP_SET_PORT_PBMP(oam_enabled_pbmp, port);
            }
                break;
            case '6':
                BACKDOOR_MGR_Printf("Which port? ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                port=atoi(buf);
                if(port<=0 || port >SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
                {
                    BACKDOOR_MGR_Printf("Invalid port id: %d\r\n", port);
                    break;
                }
                if(oam_frames_pktbuf[port-1])
                {
                    BACKDOOR_MGR_DumpHex("Packet buffer", OAM_FRAME_PDU_LEN+14, oam_frames_pktbuf[port-1]-14);
                }
                else
                {
                    BACKDOOR_MGR_Printf("Packet buffer for port %ld is NULL\r\n", port);
                }
                break;
#endif /* end of #if (SYS_CPNT_EFM_OAM == TRUE) */
            case '7':
                dying_gasp_dbg_msg_flag=!dying_gasp_dbg_msg_flag;
                break;
            case '8':
                dying_gasp_trap_enabled_flag=!dying_gasp_trap_enabled_flag;
                break;
            case 'q':
                is_exit=TRUE;
                break;
            default:
                break;
        }
    }
}

