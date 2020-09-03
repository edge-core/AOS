/* Module Name: STKTPLG_BACKDOOR.C
 * Purpose: This file contains the debugging information of stack topology:
 *
 * Notes:
 *
 * History:
 *    04/11/02       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999-2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>

#include "sys_module.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_adpt.h"

#include "sys_callback_mgr.h"
#include "stktplg_om.h"
#include "stktplg_engine.h"
#include "stktplg_backdoor.h"
#include "stktplg_shom.h"
#include "stktplg_board.h" 
#include "stktplg_tx.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#include <string.h>

#include "sys_pmgr.h"
#endif /* SYS_CPNT_WATCHDOG_TIMER */
#include "sys_time.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define HBT0_1_PAYLOAD_OFFSET ((uintptr_t)(((STKTPLG_OM_HBT_0_1_T *)0)->payload))

#if defined(AOS5700_54X)
#define STKTPLG_BACKDOOR_HAS_STKTPLG_BOARD_BACKDOOR TRUE
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if 0 /* JinhuaWei, 03 August, 2008 10:13:15 */
static void STKTPLG_BACKDOOR_ShowTopoState();
static void STKTPLG_BACKDOOR_ClearTopoState();
static void STKTPLG_BACKDOOR_SaveStateToDatabase(char *str,UI8_T lenth);
#endif /* #if 0 */
static void STKTPLG_BACKDOOR_Main (void);

static void STKTPLG_BACKDOOR_ShowLocalDeviceInfo(void);
static void STKTPLG_BACKDOOR_ShowEntityMIBEntries(void);
static void STKTPLG_BACKDOOR_ShowHbt01PacketInfo(UI8_T *pHbtPayloadHdr);
static void STKTPLG_BACKDOOR_ShowHbt2PacketInfo(UI8_T *pHbtPayloadHdr);
static BOOL_T STKTPLG_BACKDOOR_IsValidMAC(UI8_T * mac_addr);
static void STKTPLG_BACKDOOR_CounterProcess(void);
static void STKTPLG_BACKDOOR_ClearTxCounter();
static void STKTPLG_BACKDOOR_ClearTxDetailCounter();
static void STKTPLG_BACKDOOR_ShowTxCounter();
static void STKTPLG_BACKDOOR_ShowTxDetailCounter();
static void STKTPLG_BACKDOOR_ShowRxCounter();
static void STKTPLG_BACKDOOR_ClearRxCounter();
static void STKTPLG_BACKDOOR_ClearMaxProcessTime();
static void STKTPLG_BACKDOOR_ShowMaxProcessTime();
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
static void STKTPLG_BACKDOOR_LedProcess(void);
static void STKTPLG_BACKDOOR_LedMenu(void);
#endif
static void STKTPLG_BACKDOOR_ToggleDebugProcess(void);
static void STKTPLG_BACKDOOR_ToggleDebugMenu(void);
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
static void STKTPLG_BACKDOOR_WatchDogMenu(void);
static void STKTPLG_BACKDOOR_WatchDogProcess(void);
#endif
static void STKTPLG_BACKDOOR_PerformanceMenu(void);
static void STKTPLG_BACKDOOR_PerformanceProcess(void);
static void STKTPLG_BACKDOOR_LocalDBMenu(void);
static void STKTPLG_BACKDOOR_ShowLocalDBProcess(void);
static void STKTPLG_BACKDOOR_ToggleOMDebugFlag(UI8_T toggle_bits);

extern void *memset(void *b, int c, size_t len);

#define STKTPLG_BD_OPEN
/* STATIC VARIABLE DECLARATIONS
 */
#define MAX_COUNTER 20
static UI32_T tx_counter[STKTPLG_HBT_TYPE_MAX+1];/*pkt sent out counters*/
static UI32_T tx_detail_counter[STKTPLG_HBT_TYPE_MAX+1][MAX_COUNTER]; /*pkt detail send*/
static UI32_T rx_counter[STKTPLG_HBT_TYPE_MAX+1];
static UI32_T timeout_counter[STKTPLG_TIMER_MAX+1];

static BOOL_T buttonstate = FALSE;
static BOOL_T capture_HBT_received = FALSE;
static BOOL_T capture_HBT_transmitted = FALSE;
static BOOL_T HBT_ShowLevel = FALSE;
#define STATE_MAX_LEN 100
static char stktplg_state[STATE_MAX_LEN];
static UI8_T state_max_len;
static UI32_T process_time[4];/*process_time[0] is time,process_time[1] is state,process_time[2],event*/

static const char *PktType[STKTPLG_HBT_TYPE_MAX+1] = {"STKTPLG_HBT_TYPE_0",
                                                    "STKTPLG_HBT_TYPE_1",
                                                    "STKTPLG_HBT_TYPE_2",
                                                    "STKTPLG_TCN",
                                                    "STKTPLG_HELLO_TYPE_0",
                                                    "STKTPLG_HELLO_TYPE_1_ENQ",
                                                    "STKTPLG_HELLO_TYPE_1_RDY",
                                                    "STKTPLG_HBT_TYPE_0_REBOUND",
                                                    "STKTPLG_HBT_TYPE_1_REBOUND",
                                                    "STKTPLG_EXP_INFO_TO_SLAVE",
                                                    "STKTPLG_EXP_INFO_TO_MASTER",
                                                    "STKTPLG_CLOSED_LOOP_TCN",
                                                    "STKTPLG_HBT_TYPE_0_UP",
                                                    "STKTPLG_HBT_TYPE_0_DOWN",
                                                    "STKTPLG_HBT_TYPE_1_DOWN",
                                                    "STKTPLG_HBT_TYPE_2_DOWN",
                                                    "STKTPLG_HBT_TYPE_0_ACK",
                                                    "STKTPLG_HBT_TYPE_HALT",
                                                    "STKTPLG_EXP_NOTIFY",
                                                    "STKTPLG_TPLG_SYNC",
                                                    "STKTPLG_ERROR_PKT"};

const static char *sktplgstateName[STKTPLG_STATE_MAX] = {"STKTPLG_STATE_INIT",
                                                    "STKTPLG_STATE_ARBITRATION",
                                                    "STKTPLG_STATE_STANDALONE",
                                                    "STKTPLG_STATE_MASTER_SYNC",
                                                    "STKTPLG_STATE_GET_TOPOLOGY_INFO",
                                                    "STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT",
                                                    "STKTPLG_STATE_MASTER",
                                                    "STKTPLG_STATE_SLAVE",
                                                    "STKTPLG_STATE_HALT",
                                                    "STKTPLG_STATE_PRE_STANDALONE"};

/* EXPORTED SUBPROGRAM BODIES
 */
#if (STKTPLG_BACKDOOR_HAS_STKTPLG_BOARD_BACKDOOR==TRUE)
extern void STKTPLG_BOARD_BACKDOOR_Main(void);
#endif

/* FUNCTION NAME : STKTPLG_BACKDOOR_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_BACKDOOR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("stktplg",
        SYS_BLD_STKTPLG_GROUP_IPCMSGQ_KEY, STKTPLG_BACKDOOR_Main);
    #if (STKTPLG_BACKDOOR_HAS_STKTPLG_BOARD_BACKDOOR==TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("stktplg_board",
        SYS_BLD_STKTPLG_GROUP_IPCMSGQ_KEY, STKTPLG_BOARD_BACKDOOR_Main);
    #endif

    memset(tx_counter,0,sizeof(tx_counter));
    memset(rx_counter,0,sizeof(rx_counter));
    memset(tx_detail_counter,0,sizeof(tx_detail_counter));
    memset(stktplg_state,0,sizeof(stktplg_state));
    memset(process_time,0,sizeof(process_time));
    state_max_len = 0;
}

/* FUNCTION NAME : STKTPLG_BACKDOOR_ShowHbtPacket
 * PURPOSE: Show HBT packet
 * INPUT:   rx_tx         --
 *          current_state --
 *          pHbtPacket    --
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_BACKDOOR_ShowHbtPacket(UI32_T rx_tx, STKTPLG_OM_State_T current_state, UI8_T* pHbtPacket)
{
    STKTPLG_OM_HBT_Header_T *pHbtPktHeader = (STKTPLG_OM_HBT_Header_T *)pHbtPacket;
    UI32_T cur_ticks = SYS_TIME_GetSystemTicksBy10ms();

    /* Should we display the HBT packet?
     */
    switch(rx_tx)
    {
        case STKTPLG_BACKDOOR_RXHBT:
            STKTPLG_BACKDOOR_IncRxCounter(pHbtPktHeader->type);
            if(!capture_HBT_received)
                return;
            BACKDOOR_MGR_Printf("\n======= Receive HBT Packet in %s @ time: %ld =======\n", sktplgstateName[current_state], cur_ticks);
            break;

        case STKTPLG_BACKDOOR_TXHBT:
            STKTPLG_BACKDOOR_IncTxCounter(pHbtPktHeader->type);
            if(!capture_HBT_transmitted)
                return;
            BACKDOOR_MGR_Printf("\n<<<<<<< Transmit HBT Packet in %s @ time: %ld>>>>>>>>>\n", sktplgstateName[current_state], cur_ticks);
            break;

        default:
            BACKDOOR_MGR_Printf("STKMGMT_BACKDOOR_ShowHbtPacket's first argument error!\n");
            return;
    }

    BACKDOOR_MGR_Printf("Ver:%d Type:%d NextU:%d Seq:%d MLI:0x%04x\n", pHbtPktHeader->version, pHbtPktHeader->type, pHbtPktHeader->next_unit, pHbtPktHeader->seq_no, pHbtPktHeader->masters_location);

    if(HBT_ShowLevel)
    {
        switch(pHbtPktHeader->type)
        {
            case STKTPLG_HBT_TYPE_0:
            case STKTPLG_HBT_TYPE_1:
                STKTPLG_BACKDOOR_ShowHbt01PacketInfo((UI8_T *)pHbtPktHeader + HBT0_1_PAYLOAD_OFFSET);
                break;

            case STKTPLG_HBT_TYPE_2:
                STKTPLG_BACKDOOR_ShowHbt2PacketInfo((UI8_T *)&pHbtPktHeader[1]);
                break;
            case STKTPLG_TCN:
                BACKDOOR_MGR_Print("STKTPLG_TCN\n");
                break;

            case STKTPLG_HELLO_TYPE_0:
                BACKDOOR_MGR_Print("STKTPLG_HELLO_TYPE_0\n");
                break;

            case STKTPLG_HBT_TYPE_0_REBOUND:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_0_REBOUND\n");
                STKTPLG_BACKDOOR_ShowHbt01PacketInfo((UI8_T *)pHbtPktHeader + HBT0_1_PAYLOAD_OFFSET);
                break;

            case STKTPLG_HBT_TYPE_1_REBOUND:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_1_REBOUND\n");
                STKTPLG_BACKDOOR_ShowHbt01PacketInfo((UI8_T *)pHbtPktHeader + HBT0_1_PAYLOAD_OFFSET);
                break;

            case STKTPLG_CLOSED_LOOP_TCN:
                BACKDOOR_MGR_Print("STKTPLG_CLOSED_LOOP_TCN\n");
                break;

            case STKTPLG_HBT_TYPE_0_DOWN:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_0_DOWN\n");
                break;

            case STKTPLG_HBT_TYPE_1_DOWN:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_1_DOWN\n");
                break;

            case STKTPLG_HBT_TYPE_2_DOWN:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_2_DOWN\n");
                break;

            case STKTPLG_HBT_TYPE_HALT:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_HALT\n");
                break;

            case STKTPLG_TPLG_SYNC:
                BACKDOOR_MGR_Print("STKTPLG_TPLG_SYNC\n");
                break;
            case STKTPLG_TCN_TYPE_1:
                BACKDOOR_MGR_Printf("STKTPLG_TCN_TYPE_1:ttl=%hu\n", ((STKTPLG_OM_TCN_TYPE_1_T*)pHbtPacket)->ttl);
                break;

            case STKTPLG_HBT_TYPE_MAX:
                BACKDOOR_MGR_Print("STKTPLG_HBT_TYPE_MAX\n");
                break;
            default:
                BACKDOOR_MGR_Print("STKMGMT_BACKDOOR_ShowHbtPacket's second argument error!\n");
                return;
        }
    }
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: STKTPLG_BACKDOOR_Main
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void STKTPLG_BACKDOOR_Main (void)
{
#define MAXLINE 255
    char line_buffer[MAXLINE+1];
    int  select_value = 0;

    while(1)
    {

        BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
        BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE);
        BACKDOOR_MGR_Printf("\r\n===========================================");
        BACKDOOR_MGR_Printf("\r\n  Stack Topology Engineer Menu 2001/10/26  ");
        BACKDOOR_MGR_Printf("\r\n===========================================");
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
        BACKDOOR_MGR_Printf("\r\n [1] Set LED status");
#endif
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
        BACKDOOR_MGR_Printf("\r\n [2] Set watchdog timer");
#endif
        BACKDOOR_MGR_Printf("\r\n [3] Toggle Debug\r\n");
        BACKDOOR_MGR_Printf("\r\n [4] Show Counter Info\r\n");
        BACKDOOR_MGR_Printf("\r\n [5] Show Performace Info\r\n");
        BACKDOOR_MGR_Printf("\r\n [6] Show Stacking Info\r\n");
        BACKDOOR_MGR_Printf("\r\n [99] Exit Stack Topology Engineer Menu!!");
        BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

        if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
        {
            select_value = atoi(line_buffer);
            BACKDOOR_MGR_Printf("\r\nSelect value is %d", select_value); /* Debug message */
        }

        switch(select_value)
        {
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
            case 1:
                STKTPLG_BACKDOOR_LedProcess();
                break;
#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
           case 2:
               STKTPLG_BACKDOOR_WatchDogProcess();
               break;
#endif
           case 3:
               STKTPLG_BACKDOOR_ToggleDebugProcess();
               break;
           case 4:
                STKTPLG_BACKDOOR_CounterProcess();
                break;
           case 5:
                STKTPLG_BACKDOOR_PerformanceProcess();
                break;
            case 6:
                STKTPLG_BACKDOOR_ShowLocalDBProcess();
                break;
            case 99:
                BACKDOOR_MGR_Printf("\r\n Exit Stack Topology Engineer Menu");
                return;
        }
    }

} /* End of STKTPLG_BACKDOOR_Main */

/* FUNCTION NAME: STKTPLG_BACKDOOR_ShowLocalDeviceInfo
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void STKTPLG_BACKDOOR_ShowLocalDeviceInfo(void)
{
    STKTPLG_OM_Info_T board_info;
    UI8_T             module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T             module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI32_T            port_num;

    if (!STKTPLG_OM_GetSysInfo(1, &board_info))
        return;

    if (!STKTPLG_OM_GetDeviceModulePresented(1, module_presented))
        return;

    if (!STKTPLG_OM_GetDeviceModuleType(1, module_type))
        return;

    if (!STKTPLG_OM_GetMaxPortCapability(1, &port_num))
        return;

    BACKDOOR_MGR_Printf("\r\n=========== Dump unit 1 information ======================");
    /*printf("\r\n Main Board Type is (%d).", device_info.main_board_type);*/
    BACKDOOR_MGR_Printf("\r\n\r");
    BACKDOOR_MGR_Printf("\r\n Mac Address is (%2x-%2x-%2x-%2x-%2x-%2x).", board_info.mac_addr[0],
        board_info.mac_addr[1],
        board_info.mac_addr[2],
        board_info.mac_addr[3],
        board_info.mac_addr[4],
        board_info.mac_addr[5]);
    BACKDOOR_MGR_Printf("\r\n Serial No is (%s).", board_info.serial_no);
    BACKDOOR_MGR_Printf("\r\n Mainborad H/W version is (%s).", board_info.mainboard_hw_ver);
    BACKDOOR_MGR_Printf("\r\n Agent H/W version is (%s).", board_info.agent_hw_ver);
    BACKDOOR_MGR_Printf("\r\n Loader version is (%s).", board_info.loader_ver);
    BACKDOOR_MGR_Printf("\r\n Post version is (%s).", board_info.post_ver);
    BACKDOOR_MGR_Printf("\r\n Runtime version is (%s).", board_info.runtime_sw_ver);

    BACKDOOR_MGR_Printf("\r\n\r");

    BACKDOOR_MGR_Printf("\r\n Module Present is %d.", module_presented[0]);
	
    BACKDOOR_MGR_Printf("\r\n Module Type is %d.", module_type[0]);

    BACKDOOR_MGR_Printf("\r\n Total ports is (%d).", port_num);

    return;
} /* End of STKTPLG_BACKDOOR_ShowLocalDeviceInfo */


/* FUNCTION NAME: STKTPLG_BACKDOOR_ShowEntityMIBEntries
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void STKTPLG_BACKDOOR_ShowEntityMIBEntries(void)
{
    STKTPLG_OM_EntPhysicalEntry_T ent_physical_entry;
    UI32_T  i;

    ent_physical_entry.ent_physical_index = -1;     /* Initialize value */

    while (STKTPLG_OM_GetNextEntPhysicalEntry(&ent_physical_entry))
    {
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_index(%ld)", ent_physical_entry.ent_physical_index);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_vendor_type(");
        for (i=0; i<ent_physical_entry.ent_physical_vendor_type.num_components; i++)
        {
            BACKDOOR_MGR_Printf("%lu", ent_physical_entry.ent_physical_vendor_type.component_list[i]);
            if (i != ent_physical_entry.ent_physical_vendor_type.num_components-1)
                BACKDOOR_MGR_Printf(".");
        }
        BACKDOOR_MGR_Printf(")");
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_contained_in(%ld)", ent_physical_entry.ent_physical_contained_in);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_class(%ld)", ent_physical_entry.ent_physical_class);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_parent_rel_pos(%ld)", ent_physical_entry.ent_physical_parent_rel_pos);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_name(%s)", ent_physical_entry.ent_physical_name);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_hardware_rev(%s)", ent_physical_entry.ent_physical_hardware_rev);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_firmware_rev(%s)", ent_physical_entry.ent_physical_firmware_rev);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_software_rev(%s)", ent_physical_entry.ent_physical_software_rev);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_entry_rw.ent_physical_serial_num(%s)", ent_physical_entry.ent_physical_entry_rw.ent_physical_serial_num);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_mfg_name(%s)", ent_physical_entry.ent_physical_mfg_name);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_model_name(%s)", ent_physical_entry.ent_physical_model_name);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_entry_rw.ent_physical_alias(%s)", ent_physical_entry.ent_physical_entry_rw.ent_physical_alias);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_entry_rw.ent_physical_asset_id(%s)", ent_physical_entry.ent_physical_entry_rw.ent_physical_asset_id);
        BACKDOOR_MGR_Printf("\r\n ent_physical_entry.ent_physical_is_fru(%d)", ent_physical_entry.ent_physical_is_fru);
        BACKDOOR_MGR_Printf("\n\r\n ..................");
    }

    return;
} /*End of STKTPLG_BACKDOOR_ShowEntityMIBEntries*/

static  void STKTPLG_BACKDOOR_ShowHbt01PacketInfo(UI8_T *pHbtPayloadHdr)
{
    STKTPLG_OM_HBT_0_1_Payload_T    *pPayload = (STKTPLG_OM_HBT_0_1_Payload_T *)pHbtPayloadHdr;
    UI8_T   index = 0;

    while(STKTPLG_BACKDOOR_IsValidMAC(pPayload->mac_addr) && index < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        index++;

        BACKDOOR_MGR_Printf("unit %d#", index);
        BACKDOOR_MGR_Printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x ", pPayload->mac_addr[0], pPayload->mac_addr[1],
            pPayload->mac_addr[2], pPayload->mac_addr[3],
            pPayload->mac_addr[4], pPayload->mac_addr[5]);
        BACKDOOR_MGR_Printf("ImageT:%d NumC:%d StartMId:%d BoradI:%d SlaveRdy:", pPayload->image_type, pPayload->chip_nums,
            pPayload->start_module_id, pPayload->board_id);

        if(pPayload->slave_ready)
            BACKDOOR_MGR_Printf("TRUE\n");
        else
            BACKDOOR_MGR_Printf("FALSE\n");

        pPayload++;
    }
}

static  void STKTPLG_BACKDOOR_ShowHbt2PacketInfo(UI8_T *pHbtPayloadHdr)
{
    /* STKTPLG_OM_HBT_2_Payload_T *pPayload = (STKTPLG_OM_HBT_2_Payload_T *)pHbtPayloadHdr; */
    BACKDOOR_MGR_Printf("HBT2\n");
}

static  BOOL_T STKTPLG_BACKDOOR_IsValidMAC(UI8_T * mac_addr)
{
    return((mac_addr[0])||
            (mac_addr[1])||
            (mac_addr[2])||
            (mac_addr[3])||
            (mac_addr[4])||
            (mac_addr[5]));
}

BOOL_T STKTPLG_BACKDOOR_IncTxCounter(UI8_T type)
{
    if(type >= STKTPLG_HBT_TYPE_MAX)  /*modified by Jinhua Wei,because UI8_T type can't be smaller than 0*/
    {
        tx_counter[STKTPLG_HBT_TYPE_MAX]++;
        return FALSE;
    }
    tx_counter[type]++;
    return TRUE;
}

static void STKTPLG_BACKDOOR_ClearTxCounter()
{
    memset(tx_counter,0,sizeof(tx_counter));
}
static void STKTPLG_BACKDOOR_ShowTxCounter()
{
    UI8_T i;
    UI32_T allcounter = 0;
    printf("\r\nALL STKTPLG SEND PKT \n");
    for(i =0 ; i< STKTPLG_HBT_TYPE_MAX+1; i++)
    {
        if(tx_counter[i] != 0)
        {
            printf("%20s     %lu\n",PktType[i],tx_counter[i]);
            allcounter = allcounter +tx_counter[i];
        }
    }
    printf("ALL Counter %lu\n",allcounter);
    printf("=========================================\n");
}
static void STKTPLG_BACKDOOR_ClearTxDetailCounter()
{
    memset(tx_detail_counter,0,sizeof(tx_detail_counter));
}
static void STKTPLG_BACKDOOR_ShowTxDetailCounter()
{
    UI8_T i,j;
    UI32_T allcounter = 0;

    printf("=========================================\n");

    printf("\r\n Detail STKTPLG PKT \n");


    for(i =0 ; i< STKTPLG_HBT_TYPE_MAX+1; i++)
    {
        allcounter = 0;
        for(j=0;j<MAX_COUNTER;j++)
        {
            if(tx_detail_counter[i][j] != 0)
            {
                printf("TYPE %-20s   %d     %lu\n",PktType[i],j,tx_detail_counter[i][j]);
                allcounter = allcounter +tx_detail_counter[i][j];
            }
        }
        if(allcounter !=0)
        {
            printf("All counter is    %lu\n",allcounter);
            printf("*************************************************\n");
        }
    }

}

BOOL_T STKTPLG_BACKDOOR_IncSendCounter(UI8_T type,UI8_T place)
{
    tx_detail_counter[type][place]++;
    return TRUE;
}


static void STKTPLG_BACKDOOR_ClearRxCounter()
{
    memset(rx_counter,0,sizeof(rx_counter));
}
static void STKTPLG_BACKDOOR_ShowRxCounter()
{
    UI8_T i;
    UI32_T allcounter = 0;
    printf("type                          counter\n");
    printf("\r\n=========================================\n");
    for(i =0 ; i< STKTPLG_HBT_TYPE_MAX+1; i++)
    {
        if(rx_counter[i] != 0)
        {
            printf("%20s     %lu\n",PktType[i],rx_counter[i]);
            allcounter = allcounter +rx_counter[i];
        }
    }
    printf("=========================================\n");
    printf("All counter is    %lu\n",allcounter);
}

BOOL_T STKTPLG_BACKDOOR_IncRxCounter(UI8_T type)
{
    if(type >= STKTPLG_HBT_TYPE_MAX)  /*modified by Jinhua Wei,because UI8_T type can't be smaller than 0*/
    {
        rx_counter[STKTPLG_HBT_TYPE_MAX]++;
        return FALSE;
    }
    rx_counter[type]++;
    return TRUE;
}

static void STKTPLG_BACKDOOR_ClearTimeOutCounter()
{
    memset(timeout_counter,0,sizeof(timeout_counter));
}
static void STKTPLG_BACKDOOR_ShowTimeOutCounter()
{
    UI8_T i;
    UI32_T allcounter = 0;
    printf("type                          counter\n");
    printf("\r\n=========================================\n");
    for(i =0 ; i< STKTPLG_TIMER_MAX; i++)
    {
        if(timeout_counter[i] != 0)
        {
            printf("%20s     %lu\n",PktType[i],timeout_counter[i]);
            allcounter = allcounter +timeout_counter[i];
        }
    }
    printf("=========================================\n");
    printf("All counter is    %lu\n",allcounter);
}

BOOL_T STKTPLG_BACKDOOR_IncTimeOutCounter(UI8_T type)
{
    if(type >= STKTPLG_TIMER_MAX)  /*modified by Jinhua Wei,because UI8_T type can't be smaller than 0*/
    {
        timeout_counter[STKTPLG_TIMER_MAX]++;
        return FALSE;
    }
    timeout_counter[type]++;
    return TRUE;
}

void STKTPLG_BACKDOOR_ShowTxCounterType(UI8_T type)
{
    if(type >= STKTPLG_HBT_TYPE_MAX)  /*modified by Jinhua Wei,because UI8_T type can't be smaller than 0*/
    {
        printf("\r\n %20s     %lu\n",PktType[STKTPLG_HBT_TYPE_MAX],tx_counter[STKTPLG_HBT_TYPE_MAX]);
        return ;
    }
    printf("\r\nShowTxCounterType %20s     %ld\n",PktType[type],tx_counter[type]);
    return ;
}




BOOL_T STKTPLG_BACKDOOR_GetButtonState()
{
    return buttonstate ;
}

void STKTPLG_BACKDOOR_SaveTopoState(UI8_T state)
{/*
 switch(state)
 {
  case  STKTPLG_STATE_INIT :
        STKTPLG_BACKDOOR_SaveStateToDatabase("I-",2);
      break;
  case  STKTPLG_STATE_ARBITRATION:
      STKTPLG_BACKDOOR_SaveStateToDatabase("A-",2);
      break;
  case  STKTPLG_STATE_STANDALONE:
      STKTPLG_BACKDOOR_SaveStateToDatabase("SM-",3);
      break;
  case  STKTPLG_STATE_MASTER_SYNC:
      STKTPLG_BACKDOOR_SaveStateToDatabase("MS-",3);
      break;
  case  STKTPLG_STATE_GET_TOPOLOGY_INFO:
      STKTPLG_BACKDOOR_SaveStateToDatabase("T-",2);
      break;
  case  STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT:
      STKTPLG_BACKDOOR_SaveStateToDatabase("SA-",3);
      break;
  case  STKTPLG_STATE_MASTER:
      STKTPLG_BACKDOOR_SaveStateToDatabase("M-",2);
      break;
  case  STKTPLG_STATE_SLAVE:
      STKTPLG_BACKDOOR_SaveStateToDatabase("S-",2);
      break;
  case  STKTPLG_STATE_HALT:
      STKTPLG_BACKDOOR_SaveStateToDatabase("H-",2);
      break;
  case  STKTPLG_STATE_PRE_STANDALONE:
      break;
  }*/
}

#if 0 /* JinhuaWei, 03 August, 2008 10:12:01 */
static void STKTPLG_BACKDOOR_SaveStateToDatabase(char *str,UI8_T lenth)
{
  /*int i=0;
  while(i<=lenth)
  {
     if(state_max_len >= STATE_MAX_LEN)
     {
       state_max_len = 0;
     }
     stktplg_state[state_max_len]=*str;
     str++;
     state_max_len++;
  }
  if(state_max_len <STATE_MAX_LEN)
   stktplg_state[state_max_len] = '\0';
  else
  {
     stktplg_state[0] = '\0';
   }*/
}

static void STKTPLG_BACKDOOR_ClearTopoState()
{
    memset(stktplg_state,0,sizeof(stktplg_state));
    state_max_len = 0;
}

static void STKTPLG_BACKDOOR_ShowTopoState()
{
    char *str;
    str = stktplg_state;
    printf("topo is %s\n",str);
    return;
}
#endif /* #if 0 */
void STKTPLG_BACKDOOR_SetMaxProcessTime(UI32_T time,UI8_T state,BOOL_T event,UI8_T type)
{
    if(time > process_time[0])
    {
        process_time[0]=time;
        process_time[1]=state;
        process_time[2]=event;
        if(!event)
            process_time[3] = type;
    }
}
static void STKTPLG_BACKDOOR_ClearMaxProcessTime()
{
    memset(process_time,0,sizeof(process_time));
}

static void STKTPLG_BACKDOOR_ShowMaxProcessTime()
{
    printf("\r\n=========================================================\n");
    printf("\r\n Time ,State ,Time Event,Pkt Type\n");
    printf("%ld,%s,%lu,%lu",process_time[0],sktplgstateName[process_time[1]],process_time[2],process_time[3]);
    printf("\r\n=========================================================\n");
}

static void STKTPLG_BACKDOOR_CounterMenu(void)
{
    printf("\r\n [1].Show RX Counter");
    printf("\r\n [2].Clear RX Counter");
    printf("\r\n [3].Show TX Counter");
    printf("\r\n [4].Clear TX Counter");
    printf("\r\n [5].Show TX Detail Counter");
    printf("\r\n [6].Clear TX Detail Counter");
    printf("\r\n [7].Show TimeOut Counter");
    printf("\r\n [8].Clear TimeOut Counter");
    printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

}

static void STKTPLG_BACKDOOR_CounterProcess(void)
{
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_CounterMenu();
        ch = getchar();
        switch(ch)
        {
            case '1':/*Show RX Counter*/
                STKTPLG_BACKDOOR_ShowRxCounter();
                break;
            case '2':/*Clear RX Counter*/
                STKTPLG_BACKDOOR_ClearRxCounter();
                break;
            case '3':/*Show TX Counter*/
                STKTPLG_BACKDOOR_ShowTxCounter();
                break;
            case '4':/*Clear TX Counter*/
                STKTPLG_BACKDOOR_ClearTxCounter();
                break;
            case '5':/*Show TX Detail  Counter*/
                STKTPLG_BACKDOOR_ShowTxDetailCounter();
                break;
            case '6':/*Clear TX Detail  Counter*/
                STKTPLG_BACKDOOR_ClearTxDetailCounter();
                break;
            case '7':/*Show TimeOut*/
                STKTPLG_BACKDOOR_ShowTimeOutCounter();
                break;
            case '8':/*Clear Timeout Counter*/
                STKTPLG_BACKDOOR_ClearTimeOutCounter();
                break;
            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
static void STKTPLG_BACKDOOR_LedMenu(void)
{
    BACKDOOR_MGR_Printf("\r\n [4] Set Stack Status LED(Master).");
    BACKDOOR_MGR_Printf("\r\n [5] Set Stack Status LED(Second Unit).");
    BACKDOOR_MGR_Printf("\r\n [6] Set Stack Status LED(Others Unit).");
    BACKDOOR_MGR_Printf("\r\n [7] Set Stack Link LED(Up).");
    BACKDOOR_MGR_Printf("\r\n [8] Set Stack Link LED(Down/not stack).");
    BACKDOOR_MGR_Printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");


}

static void STKTPLG_BACKDOOR_LedProcess(void)
{
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_LedMenu();
        ch = getchar();
        switch(ch)
        {
            case '4':
                STKTPLG_MGR_SetBaseLed(SYS_HWCFG_SYSTEM_BASE_MASTER_UNIT);
                break;
            case '5':
                STKTPLG_MGR_SetBaseLed(SYS_HWCFG_SYSTEM_BASE_SECOND_UNIT);
                break;
            case '6':
                STKTPLG_MGR_SetBaseLed(SYS_HWCFG_SYSTEM_BASE_OTHER_UNIT);
                break;
            case '7':
                STKTPLG_MGR_SetStackLinkLed(SYS_HWCFG_SYSTEM_STACK_LINK_OK);
                break;
            case '8':
                STKTPLG_MGR_SetStackLinkLed(SYS_HWCFG_SYSTEM_STACK_LINK_DOWN_OR_NOSTACK);
                break;
            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)

static void STKTPLG_BACKDOOR_WatchDogMenu(void)
{
    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n  Stack Topology WatchDog Menu 2008/12/30  ");
    BACKDOOR_MGR_Printf("\r\n===========================================");

    BACKDOOR_MGR_Printf("\r\n [1] Enable Watch Dog.");
    BACKDOOR_MGR_Printf("\r\n [2] Disable Watch Dog.");
    BACKDOOR_MGR_Printf("\r\n [3] Kick Off Watch Dog (Must Enable Watch Dog First).");
    BACKDOOR_MGR_Printf("\r\n [4] Kick On Watch Dog (Must Enable Watch Dog First).");
    BACKDOOR_MGR_Printf("\r\n [5] Kick Counter.");
    BACKDOOR_MGR_Printf("\r\n [6] Watch Dog Test Task Create.");
    BACKDOOR_MGR_Printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

}

static void STKTPLG_BACKDOOR_WatchDogProcess(void)
{
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_WatchDogMenu();
        ch = getchar();
        switch(ch)
        {
             case '1':
                 SYS_TIME_EnableWatchDogTimer();
                 break;
             case '2':
                 SYS_TIME_DisableWatchDogTimer();
                 break;
             case '3':
                 SYS_TIME_KickWatchDogTimer();
                 SYS_TIME_SetKick(FALSE);
                 break;
             case '4':
                 SYS_TIME_SetKick(TRUE);
                 SYS_TIME_KickWatchDogTimer();
                 break;
             case '5':
                 {
                     UI32_T counter;

                     SYS_TIME_GetKickCounter(&counter);
                     BACKDOOR_MGR_Printf("\r\n");
                     BACKDOOR_MGR_Printf("\nWatch Dog Kick   = %ld]\r\n", counter);

                 }
                 break;
             case '6':
                 {
         #define WATCH_DOG_TEST_STRING   "Watch Dog Test Task"
                     SYS_MGR_WatchDogExceptionInfo_T wd_own;

                     memset(&wd_own, 0, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
                     memcpy(wd_own.name, WATCH_DOG_TEST_STRING, sizeof(WATCH_DOG_TEST_STRING));
                     wd_own.pStackBase  = 1;   /* points to bottom of stack */
                     wd_own.pStackLimit = 2;   /* points to stack limit */
                     wd_own.pStackEnd   = 3;   /* points to init stack limit */

                     wd_own.priority    = 4;   /* task priority       */
                     wd_own.status      = 5;   /* task status         */
                     wd_own.delay       = 6;   /* delay/timeout ticks */

                     wd_own.stackSize    = 7;  /* size of stack in bytes */
                     wd_own.stackCurrent = 8;  /* current stack usage in bytes */
                     wd_own.stackMargin  = 9;  /*  used as Program Counter */

                     if (SYS_PMGR_LogWatchDogExceptionInfo(&wd_own) == TRUE)
                         BACKDOOR_MGR_Printf("\n\nWatch Dog Log Success ==> Warm Start System\n\n");
                 }

                 break;

            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}
#endif /* SYS_CPNT_WATCHDOG_TIMER */


static void STKTPLG_BACKDOOR_ToggleDebugMenu(void)
{
    BOOL_T  stktplg_engine_debug_mode;
    BOOL_T  stktplg_engine_TCN_mode;
    UI8_T   stktplg_om_debug_mode;

    STKTPLG_ENGINE_GetDebugMode(&stktplg_engine_debug_mode);
    STKTPLG_ENGINE_GetTCNMode(&stktplg_engine_TCN_mode);
    STKTPLG_OM_GetDebugMode(&stktplg_om_debug_mode);


    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n  Stack Topology Toggle Debug Menu 2008/12/30  ");
    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n [1] %s Rx HBTx packet", capture_HBT_received ? "Hide" : "Show");
    BACKDOOR_MGR_Printf("\r\n [2] %s Tx HBTx packet", capture_HBT_transmitted ? "Hide" : "Show");
    BACKDOOR_MGR_Printf("\r\n [3] Display %s Level", HBT_ShowLevel ? "Low" : "High");
    BACKDOOR_MGR_Printf("\r\n [4] %s STKTPLG_ENGINE debug message", stktplg_engine_debug_mode ? "Hide" : "Show");
    BACKDOOR_MGR_Printf("\r\n [5] %s TCN", stktplg_engine_TCN_mode ? "Disable" : "Enable");
    BACKDOOR_MGR_Printf("\r\n [6] Toggle STKTPLG_OM debug message(%s)", (stktplg_om_debug_mode&STKTPLG_OM_DEBUG_MODE_FLAG_DEBUG_MSG)?"Show":"Hide");
    BACKDOOR_MGR_Printf("\r\n [7] %s set master button state",buttonstate?"Not press":"Press");
    BACKDOOR_MGR_Printf("\r\n [8] Current mode to master mode");
    BACKDOOR_MGR_Printf("\r\n [9] Current mode to slave mode");
    BACKDOOR_MGR_Printf("\r\n [a] %s STKTPLG_BOARD debug message", STKTPLG_BOARD_GetDebugMode() ? "Hide" : "Show");
    BACKDOOR_MGR_Printf("\r\n [b] Toggle STKTPLG_TX debug flag(%hu)", STKTPLG_TX_GetDebugMode());
    BACKDOOR_MGR_Printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

}

static void STKTPLG_BACKDOOR_ToggleDebugProcess(void)
{
    int   ich;
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;
    char  buf[16];

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_ToggleDebugMenu();
        ich = (BACKDOOR_MGR_GetChar());
        if(ich==EOF)
            continue;
        ch=(UI8_T)ich;
        switch(ch)
        {

            case '1':
                capture_HBT_received = !capture_HBT_received;
                break;
            case '2':
                capture_HBT_transmitted = !capture_HBT_transmitted;
                break;
            case '3':
                HBT_ShowLevel= !HBT_ShowLevel;
                break;
            case '4':
                STKTPLG_ENGINE_ToggleDebugMode();
                break;
            case '5':
                STKTPLG_ENGINE_ToggleTCNMode();
                break;
            case '6':
                STKTPLG_BACKDOOR_ToggleOMDebugFlag(STKTPLG_OM_DEBUG_MODE_FLAG_DEBUG_MSG);
                break;
            case '7':
                buttonstate = !buttonstate;
                break;
            case '8':
                SYS_CALLBACK_MGR_StackStateCallBack(SYS_MODULE_STKTPLG, STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE);
                break;
            case '9':
                SYS_CALLBACK_MGR_StackStateCallBack(SYS_MODULE_STKTPLG, STKTPLG_MASTER_LOSE_MSG);
                break;
            case 'a':
                STKTPLG_BOARD_SetDebugMode(!STKTPLG_BOARD_GetDebugMode());
                break;
            case 'b':
                STKTPLG_TX_ToggleDebugMode();
                break;

            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}

static void STKTPLG_BACKDOOR_PerformanceMenu(void)
{
    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n  Stack Topology Performance Menu 2008/12/30  ");
    BACKDOOR_MGR_Printf("\r\n===========================================");

    BACKDOOR_MGR_Printf("\r\n [1] show max process time ");
    BACKDOOR_MGR_Printf("\r\n [2] show detail max process time ");
    BACKDOOR_MGR_Printf("\r\n [3] clear max time");
    BACKDOOR_MGR_Printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

}

static void STKTPLG_BACKDOOR_PerformanceProcess(void)
{
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_PerformanceMenu();
        ch = getchar();
        switch(ch)
        {
            case '1':
                STKTPLG_BACKDOOR_ShowMaxProcessTime();
                break;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            case '3':
                STKTPLG_ENGINE_BD_ClearTick();
                STKTPLG_BACKDOOR_ClearMaxProcessTime();
                break;
           case '2' :
                STKTPLG_ENGINE_BD_GetTick();
                break;
#endif

            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}

static void STKTPLG_BACKDOOR_LocalDBMenu(void)
{

    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n  Stack Topology Local DB Menu 2008/12/30  ");
    BACKDOOR_MGR_Printf("\r\n===========================================");
    BACKDOOR_MGR_Printf("\r\n [1] Show local unit device information.");
    BACKDOOR_MGR_Printf("\r\n [2] Show all entity MIB index.");
    BACKDOOR_MGR_Printf("\r\n [3] Current state\r\n");
    BACKDOOR_MGR_Printf("\r\n [q].QUIT");
    BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

}

static void STKTPLG_BACKDOOR_ShowLocalDBProcess(void)
{
    UI8_T ch=0;
    BOOL_T  backdoor_continue=TRUE;

    while(backdoor_continue)
    {
        STKTPLG_BACKDOOR_LocalDBMenu();
        ch = getchar();
        switch(ch)
        {
           case '1':
                STKTPLG_BACKDOOR_ShowLocalDeviceInfo();
                break;
            case '2':
                STKTPLG_BACKDOOR_ShowEntityMIBEntries();
                break;

            case '3':
                STKTPLG_ENGINE_ShowStktplgInfo();
                break;
            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid input!\n");
                break;
        }
    }
    return;
}

static void STKTPLG_BACKDOOR_ToggleOMDebugFlag(UI8_T toggle_bits)
{
    UI8_T debug_mode=0, i, mask;

    STKTPLG_OM_GetDebugMode(&debug_mode);
    for (i=0; i<sizeof(toggle_bits)*8; i++)
    {
        mask=1<<i;
        if (toggle_bits&mask)
        {
            if (debug_mode&mask)
                debug_mode &= ~(mask);
            else
                debug_mode |= mask;
        }
    }

    STKTPLG_OM_SetDebugMode(debug_mode);
}

