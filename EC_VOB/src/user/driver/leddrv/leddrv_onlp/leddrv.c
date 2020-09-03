/* MODULE NAME: leddrv.c
 * PURPOSE: This moudle provides API to light the system-type LEDs through ONLP
 *          library.
 * NOTES:
 * REASON:
 * Description:
 * CREATOR:      Chiourung Huang
 * HISTORY
 *    17/5/2016 - Chiourung Huang, Created
 *
 * Copyright(C)      Accton Corporation, 2016
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "sysfun.h"

#include "leddrv.h"
#include "leddrv_type.h"
#include "leaf_es3626a.h"
#include "leaf_sys.h"

#include "stktplg_pom.h"
#include "uc_mgr.h"
#include "sysrsc_mgr.h"

#include "isc.h"

#define BACKDOOR_ENABLE

#ifdef BACKDOOR_ENABLE
#include "backdoor_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    #define LEDDRV_REMOTE_SERVICE_BASE  0x00
    #define LEDDRV_REMOTE_SET_PORT      (LEDDRV_REMOTE_SERVICE_BASE)
    #define LEDDRV_REMOTE_SET_SYSTEM     (LEDDRV_REMOTE_SERVICE_BASE+1)
    #define LEDDRV_REMOTE_SET_HOTSWAPIN  (LEDDRV_REMOTE_SERVICE_BASE+2)
    #define LEDDRV_REMOTE_SET_NOHOTSWAP  (LEDDRV_REMOTE_SERVICE_BASE+3)
    #define LEDDRV_REMOTE_SET_MODULELED  (LEDDRV_REMOTE_SERVICE_BASE+4)
    #define LEDDRV_REMOTE_SERVICE_LAST   LEDDRV_REMOTE_SET_MODULELED

    #define LEDDRV_ISC_TRY_TIME         2
    #define LEDDRV_ISC_TIMEOUT          300
    #define LEDDRV_POOL_ID_ISC_SEND     0
    #define LEDDRV_POOL_ID_ISC_REPLY    1
#endif /* #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

#ifdef BACKDOOR_ENABLE
    #define LEDDRV_DEBUG_FLAG_ERRMSG        1
    #define LEDDRV_DEBUG_FLAG_DBGMSG        2
    #define LEDDRV_DEBUG_FLAG_DUMP_TXPKT    4
    #define LEDDRV_DEBUG_FLAG_DUMP_RXPKT    8
    #define LEDDRV_WORD_WRAP                16
    #define MAXLINE                         32
#endif /* End of BACKDOOR_ENABLE */

/* DATA TYPE DECLARATIONS
 */
typedef struct LEDDRV_Position_Offset_S
{
    UI8_T   chip_id;
    UI8_T   port_id;
} __attribute__((packed)) LEDDRV_Position_Offset_T;

typedef struct LEDDRV_Packet_S
{
    UI16_T  service_id;
    UI16_T  return_value;
    union
    {
        struct
        {
            LEDDRV_System_Status_T status;
        }sys_info;
        struct
        {
            UI32_T port;
            LEDDRV_Port_Status_T status;
        }port_info;
        struct
        {
            UI32_T color;
        }color_info;
    }data;
} __attribute__((packed)) LEDDRV_Packet_T;

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
typedef BOOL_T (*LEDDRV_Remote_Function_T)(ISC_Key_T *key, LEDDRV_Packet_T *packet);
#endif
typedef enum LEDDRV_Error_Type_S
{
    LEDDRV_ERROR_NONE           = 0,
    LEDDRV_ERROR_NOT_MASTER,
    LEDDRV_ERROR_PORT_NOT_EXIST,
    LEDDRV_ERROR_TRANSITION_MODE
} LEDDRV_Error_Type_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    static BOOL_T   LEDDRV_Slave_SetPortStatus(ISC_Key_T *key, LEDDRV_Packet_T *packet);
    static BOOL_T   LEDDRV_Slave_SetSystemStatus(ISC_Key_T *key, LEDDRV_Packet_T *packet);
    static BOOL_T   LEDDRV_Slave_SetModuleLed(ISC_Key_T *key, LEDDRV_Packet_T *packet);
    static BOOL_T   LEDDRV_Slave_SetHotSwapInsertion(ISC_Key_T *key, LEDDRV_Packet_T *packet);
    static BOOL_T   LEDDRV_Slave_SetHotSwapRemoval(ISC_Key_T *key, LEDDRV_Packet_T *packet);
#endif /* End of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

#ifdef BACKDOOR_ENABLE
    static BOOL_T   LEDDRV_Debug(UI32_T flag);
    static void     LEDDRV_SetDebug(UI32_T flag);
    static void     LEDDRV_GetDebug(UI32_T *flag);
    static void     LEDDRV_INIT_BackdoorInfo_CallBack(void);

    static void LEDDRV_PrintHexChar(UI8_T *string, UI16_T char_count);
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    static void LEDDRV_PrintPacket(UI8_T *packet, UI16_T size);
#endif
    static void LEDDRV_BACKDOOR_SetDebug(void);
#endif /* End of BACKDOOR_ENABLE */

/* STATIC VARIABLE DECLARATIONS
 */

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
static LEDDRV_Remote_Function_T LEDDRV_remote_service_table[] =
    {
        LEDDRV_Slave_SetPortStatus,
        LEDDRV_Slave_SetSystemStatus,
        LEDDRV_Slave_SetHotSwapInsertion,  /* LEDDRV_REMOTE_SET_HOTSWAPIN */
        LEDDRV_Slave_SetHotSwapRemoval,    /* LEDDRV_REMOTE_SET_NOHOTSWAP */
        LEDDRV_Slave_SetModuleLed,
    };

#endif  /* #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

#ifdef BACKDOOR_ENABLE
    static UI32_T debug_flag;
    static char *leddrv_error_messages[] =
    {   "Operation completed successfully",
        "Not in master mode",
        "Specified port does not exist",
        "Unit is in transition mode"
    };
    static  UI8_T   LEDDRV_BACKDOOR_ClearScreen[] = { "\033[2J" };
#endif
static LEDDRV_Shmem_Data_T *leddrv_shmem_data_p;

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef BACKDOOR_ENABLE
#define ERRMSG(x)           if(LEDDRV_Debug(LEDDRV_DEBUG_FLAG_ERRMSG))              \
                                printf("%s() Error: %s\r\n", __FUNCTION__, leddrv_error_messages[(x)] )

#define DBGMSG(fmtstr, arg...) if(LEDDRV_Debug(LEDDRV_DEBUG_FLAG_DBGMSG))                          \
                               {                                                                   \
                                   printf("%s(%d):" fmtstr "\r\n", __FUNCTION__, __LINE__, ##arg); \
                               }

#define DUMP_TXPKT(x)       if(LEDDRV_Debug(LEDDRV_DEBUG_FLAG_DUMP_TXPKT) )         \
                            {                                                       \
                                printf("%s() Send:\r\n", __FUNCTION__);             \
                                LEDDRV_PrintPacket( (UI8_T *) &(x), sizeof((x)) );  \
                            }

#define DUMP_RXPKT(x)       if(LEDDRV_Debug(LEDDRV_DEBUG_FLAG_DUMP_RXPKT) )         \
                            {                                                       \
                                printf("%s() Received:\r\n", __FUNCTION__);         \
                                LEDDRV_PrintPacket( (UI8_T *) &(x), sizeof((x)) );  \
                            }
#else
#define ERRMSG(x)       ;
#define DBGMSG(x)       ;
#define DUMP_TXPKT(x)   ;
#define DUMP_RXPKT(x)   ;
#endif /* BACKDOOR_ENABLE */

#define LEDDRV_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(leddrv_shmem_data_p->sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define LEDDRV_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(leddrv_shmem_data_p->sem_id)

SYSFUN_DECLARE_CSC;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: LEDDRV_Initiate_System_Resources
 * PURPOSE: initializes all resources for LEDDRV
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
BOOL_T LEDDRV_InitiateSystemResources(void)
{
    leddrv_shmem_data_p = (LEDDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_LEDDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(leddrv_shmem_data_p);
    LEDDRV_Init();
    return (TRUE);
}

/* FUNCTION NAME: LEDDRV_AttachSystemResources
 * PURPOSE: Attach system resource for LEDDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LEDDRV_AttachSystemResources(void)
{
    leddrv_shmem_data_p = (LEDDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_LEDDRV_SHMEM_SEGID);

    return;
}

/* FUNCTION NAME: LEDDRV_init
 * PURPOSE: Initialize LED driver, allocate display-buffer space...
 *          and register LEDDRV_Display() function.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - if initialization sucessful
 *          FALSE - if initialization fail
 * NOTES:
 *      1. The function will register LEDDRV_display function to the timer,
 *         IMC functions for setting port status and LED pattern, and initialize
 *         variables to default state.
 *      2. It SHOULD be called *ONCE ONLY* at system startup by LED management
 *         module and *SHOULD NOT* be used as a reset machanism.
 *      3. In this function, call STK_TPLG api to get my-board-type.
 *         So, STK_TPLG must reply this request at any mode.
 */
BOOL_T LEDDRV_Init(void)
{
    UC_MGR_Sys_Info_T uc_sys_info;
    UI32_T rc;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("%s(%d)Get UC System Information Fail.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    leddrv_shmem_data_p->my_board_id=uc_sys_info.board_id;

    rc=SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &(leddrv_shmem_data_p->sem_id));
    if (rc!=SYSFUN_OK)
    {
        printf("%s(%d)Failed to create semaphore.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *
 */
void LEDDRV_Create_InterCSC_Relation(void)
{
#ifdef BACKDOOR_ENABLE
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("leddrv",SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,LEDDRV_INIT_BackdoorInfo_CallBack);
#endif
}

/* FUNCTION NAME: LEDDRV_EnterTransitionMode
 * PURPOSE: set system in transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterTransitionMode(void)
{
    STKTPLG_POM_GetMyUnitID(&(leddrv_shmem_data_p->my_unit_id));

    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(leddrv_shmem_data_p);
}

/* FUNCTION NAME: LEDDRV_SetTransitionMode(void)
 *----------------------------------------------------------------------------------
 * PURPOSE: Tell LEDDRV to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void LEDDRV_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(leddrv_shmem_data_p);
}

/* FUNCTION NAME: LEDDRV_EnterMasterMode
 * PURPOSE: set system in master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterMasterMode(void)
{
    STKTPLG_POM_GetMyUnitID(&(leddrv_shmem_data_p->my_unit_id));
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(leddrv_shmem_data_p);
}

/* FUNCTION NAME: LEDDRV_EnterSlaveMode
 * PURPOSE: set system in slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterSlaveMode(void)
{
    STKTPLG_POM_GetMyUnitID(&(leddrv_shmem_data_p->my_unit_id));
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(leddrv_shmem_data_p);
}

/* FUNCTION NAME: LEDDRV_Display
 * PURPOSE: Copy display-buffer pattern to LED I/O buffer.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. This is a callback function, registered to timer at initialization and
 *         executed periodically to change LED state.
 *      2. Display will have no 'if statement'.
 */
void LEDDRV_Display (void)
{
}

/* FUNCTION NAME: LEDDRV_ShowUnitLED
 * PURPOSE: Shows the unit number on the LED panel
 * INPUT:   unit - unit number
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module
 */
BOOL_T LEDDRV_ShowUnitLED(UI32_T unit)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_ShowAllUnitLED
 * PURPOSE: Each unit in the stack shows its number on the LED panel
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module
 */
BOOL_T LEDDRV_ShowAllUnitLED(void)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_SetPortStatus
 * PURPOSE: set port status light pattern for a specific port
 * INPUT:   unit - unit number
 *          port - user port number
 *          port_status - status only, pattern determines in LEDDRV.
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module,
 *         for local LED control.
 */
BOOL_T LEDDRV_SetPortStatus(UI32_T unit, UI32_T port,BOOL_T is_stacking_port, LEDDRV_Port_Status_T *port_status)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_PortShutDown
* PURPOSE: light off for a unit
* INPUT:   none
* OUTPUT:  none
* RETUEN:  none
* NOTES:   none.
*/
void LEDDRV_PortShutDown(void)
{
}

/* FUNCTION NAME: LEDDRV_SetSystemStatus
 * PURPOSE: set system status light for a unit
 * INPUT:   unit - unit number
 *          sys_status - status only, pattern determines in LEDDRV.
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module,
 *         for local LED control.
 */
BOOL_T LEDDRV_SetSystemStatus(UI32_T unit, LEDDRV_System_Status_T  sys_status)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_SetPowerStatus
 * PURPOSE: set power status light for a unit
 * INPUT:   unit   - unit number
 *          power  - internal
 *          status - VAL_swIndivPowerStatus_notPresent
 *                   VAL_swIndivPowerStatus_green
 *                   VAL_swIndivPowerStatus_red
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetPowerStatus(UI32_T unit, UI32_T power, UI32_T status)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_SetModuleLed
 * PURPOSE: set internal power LED pattern
 * INPUT    : unit_id -- the unit desired to set
 *           color   -- LEDDRV_COLOR_OFF              off
 *                      LEDDRV_COLOR_GREEN            green
 *                      LEDDRV_COLOR_AMBER            amber
 *                      LEDDRV_COLOR_GREENFLASH       green flash
 *                      LEDDRV_COLOR_AMBERFLASH       amber flash
 *                      LEDDRV_COLOR_GREENAMBER_FLASH green amber flash
 *
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   This function have implement ISC remote call mechanism, since the module Led
 *          is set after master unit collect all units' module information and triggered
 *          by master unit.
 */
void LEDDRV_SetModuleLed(UI32_T unit, UI8_T color)
{
    return;
}

/* FUNCTION NAME: LEDDRV_SetHotSwapInsertion
 * PURPOSE: set Module light for Hot Insertion
 * INPUT:   unit  - unit num
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetHotSwapInsertion(UI32_T unit)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_HotSwapRemoval
 * PURPOSE: set Module light for Hot Removal
 * INPUT:   unit  - unit num
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetHotSwapRemoval(UI32_T unit)
{
    return TRUE;
}

/* FUNCTION NAME: LEDDRV_SetStackingLinkStatus
 * PURPOSE: set stacking cable status light for a unit
 * INPUT:   unit   - unit number
 *          status - LEDDRV_STACKING_LINK_BOTH_UP
 *                   LEDDRV_STACKING_LINK_ONLY_TX_UP
 *                   LEDDRV_STACKING_LINK_ONLY_RX_UP
 *                   LEDDRV_STACKING_LINK_BOTH_DOWN
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetStackingLinkStatus(UI32_T unit, UI32_T status,BOOL_T stack_state,UI32_T up_port,UI8_T up_link_state,UI32_T down_port,UI8_T down_link_state)
{
    return TRUE;
}

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
BOOL_T LEDDRV_SetStackingPortLinkStatus(UI32_T unit,BOOL_T stack_state,UI32_T up_port,UI8_T up_link_state,UI32_T down_port,UI8_T down_link_state)
{
    return TRUE;
}
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

#if ((SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) || (SYS_CPNT_THERMAL_DETECT == TRUE) || \
     (SYS_CPNT_POWER_DETECT == TRUE))
/* FUNCTION NAME: LEDDRV_SetFaultStatus
 * PURPOSE: Set led for system fault status on a unit
 * INPUT:   unit           - unit id
 *          sys_fault_type - The type of the system fault. Valid values are listed below:
 *                           LEDDRV_SYSTEM_FAULT_TYPE_FAN
 *                           LEDDRV_SYSTEM_FAULT_TYPE_THERMAL
 *                           LEDDRV_SYSTEM_FAULT_TYPE_PSU
 *          index          - The index for the type of the system fault.
 *                           The value of the index starts from 1 and never be 0.
 *                           For example, sys_fault_type=LEDDRV_SYSTEM_FAULT_TYPE_FAN
 *                           and index=1 indicates the fan index 1 fault status.
 *          is_fault       - TRUE - the indicated system fault occurs.
 *                           FALSE- the indicated system fault does not occur.
 * OUTPUT:  none
 * RETUEN:  TRUE           - OK
 *          FALSE          - Failed
 * NOTES:
 *      1. The function sets Fault status and update the Fault LED accordingly
 */
BOOL_T LEDDRV_SetFaultStatus(UI32_T unit, LEDDRV_System_Fault_Type_T sys_fault_type, UI32_T index, BOOL_T is_fault)
{
    return TRUE;
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */

/*==================================
 * Functions needed only in stacking
 *==================================*/

/* FUNCTION NAME: LEDDRV_ISC_Handler
 * PURPOSE: Call by isc_agent to handle ISC incoming packets
 * INPUT:   key           - ISC key
 *          mref_handle_p - mref handle of a incoming packet
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *       This function is called by ISC_Agent
 */
BOOL_T LEDDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    BOOL_T  ret = TRUE;

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    LEDDRV_Packet_T *packet;
    UI32_T  pdu_len;


    packet = (LEDDRV_Packet_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if(packet==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
        return FALSE;
    }

    /*
     * Check to abort operation if callback service id(opcode) is more then
     * number of callback service on this drive.
     */
    if(packet->service_id > LEDDRV_REMOTE_SERVICE_LAST || LEDDRV_remote_service_table[packet->service_id]==NULL)
    {
        L_MM_Mref_Release (&mref_handle_p);
        printf("\r\nLEDDRV: Service ID is invalid!\r\n");
        return TRUE;
    }

    DUMP_RXPKT(*packet);
    ret = LEDDRV_remote_service_table[packet->service_id](key, packet);
    L_MM_Mref_Release(&mref_handle_p);

#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */
    return ret;
}

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
static BOOL_T LEDDRV_Slave_SetPortStatus(ISC_Key_T *key, LEDDRV_Packet_T *packet)
{
    return LEDDRV_SetPortStatus(leddrv_shmem_data_p->my_unit_id, packet->data.port_info.port, FALSE,&(packet->data.port_info.status));
}

static BOOL_T LEDDRV_Slave_SetSystemStatus(ISC_Key_T *key, LEDDRV_Packet_T *packet)
{
    return LEDDRV_SetSystemStatus(leddrv_shmem_data_p->my_unit_id, packet->data.sys_info.status);
}

static BOOL_T LEDDRV_Slave_SetModuleLed(ISC_Key_T *key, LEDDRV_Packet_T *packet)
{
    LEDDRV_SetModuleLed(leddrv_shmem_data_p->my_unit_id, packet->data.color_info.color);
    return TRUE;
}
static BOOL_T LEDDRV_Slave_SetHotSwapInsertion(ISC_Key_T *key, LEDDRV_Packet_T *packet)
{
    UI32_T  my_unit_id;

    if (!STKTPLG_POM_GetMyUnitID(&my_unit_id))
    {
        return FALSE;
    }

    return TRUE ;
}

static BOOL_T LEDDRV_Slave_SetHotSwapRemoval(ISC_Key_T *key, LEDDRV_Packet_T *packet)
{
    UI32_T  my_unit_id;

    if (!STKTPLG_POM_GetMyUnitID(&my_unit_id))
    {
        return FALSE;
    }
    return LEDDRV_SetHotSwapRemoval(my_unit_id);
}

#endif /* End of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

/*=============================
 *Backdoor function definitions
 *=============================*/
#ifdef BACKDOOR_ENABLE
static BOOL_T LEDDRV_Debug(UI32_T flag)
{
    return (debug_flag & flag)? TRUE: FALSE;
}

static void LEDDRV_SetDebug(UI32_T flag)
{
    debug_flag = flag;
}

static void LEDDRV_GetDebug(UI32_T *flag)
{
    *flag = debug_flag;
}

static void LEDDRV_PrintHexChar(UI8_T *string, UI16_T char_count)
{
    UI8_T index;
    BACKDOOR_MGR_Printf(" ");
    for(index = 0; index < char_count; index++)
        BACKDOOR_MGR_Printf("%02X ", string[index]);

    for(; index < LEDDRV_WORD_WRAP; index++)
            BACKDOOR_MGR_Printf("   ");

    BACKDOOR_MGR_Printf("| ");
    for(index = 0; index < char_count; index++)
    {
        if(string[index] < ' ' || string[index] > '~')
            BACKDOOR_MGR_Printf(".");
        else
            BACKDOOR_MGR_Printf("%c", string[index]);
    }
    BACKDOOR_MGR_Printf("\r\n");
}

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
static void LEDDRV_PrintPacket(UI8_T *packet, UI16_T size)
{
    char title_bar[] = {
" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | 0123456789ABCDEF\r\n\
-------------------------------------------------+------------------"
};

    UI16_T data_remain;
    data_remain = size;

    printf("\r\nPacket Content:\r\n%s\r\n", title_bar);
    for(;data_remain > LEDDRV_WORD_WRAP;)
    {
        LEDDRV_PrintHexChar(packet, LEDDRV_WORD_WRAP);
        packet += LEDDRV_WORD_WRAP;
        data_remain -= LEDDRV_WORD_WRAP;
    }
    LEDDRV_PrintHexChar(packet, data_remain);
    return;
}
#endif

static void LEDDRV_BACKDOOR_SetDebug(void)
{
    UI32_T  flag;
    UI32_T  temp_flag;
    UI8_T   choice;

    BACKDOOR_MGR_Printf("%s", LEDDRV_BACKDOOR_ClearScreen);
    LEDDRV_GetDebug(&flag);

    BACKDOOR_MGR_Printf("\r\nCurrent debug status");
    BACKDOOR_MGR_Printf("\r\n====================");
    BACKDOOR_MGR_Printf("\r\n1. Show Error Messages:     %s",
        (flag & LEDDRV_DEBUG_FLAG_ERRMSG)? "On":"Off");
    BACKDOOR_MGR_Printf("\r\n2. Show Debug Messages:     %s",
        (flag & LEDDRV_DEBUG_FLAG_DBGMSG)? "On":"Off");
    BACKDOOR_MGR_Printf("\r\n3. Show Sent Packets:       %s",
        (flag & LEDDRV_DEBUG_FLAG_DUMP_TXPKT)? "On":"Off");
    BACKDOOR_MGR_Printf("\r\n4. Show Received Packets:   %s",
        (flag & LEDDRV_DEBUG_FLAG_DUMP_RXPKT)? "On":"Off");
    BACKDOOR_MGR_Printf("\r\n====================");
    BACKDOOR_MGR_Printf("\r\nEnter your choice(1~4): ");

    choice = BACKDOOR_MGR_GetChar();
    if(choice < '1' || choice > '4')
    {
        BACKDOOR_MGR_Printf("\r\nInput Error");
        return;
    }

    switch(choice)
    {
        case '1':
            BACKDOOR_MGR_Printf("\r\nLEDDRV_DEBUG_FLAG_ERRMSG (0:OFF, 1:ON): ");
            temp_flag = LEDDRV_DEBUG_FLAG_ERRMSG;
            break;
        case '2':
            BACKDOOR_MGR_Printf("\r\nLEDDRV_DEBUG_FLAG_DBGMSG (0:OFF, 1:ON): ");
            temp_flag = LEDDRV_DEBUG_FLAG_DBGMSG;
            break;
        case '3':
            BACKDOOR_MGR_Printf("\r\nLEDDRV_DEBUG_FLAG_DUMP_TXPKT (0:OFF, 1:ON): ");
            temp_flag = LEDDRV_DEBUG_FLAG_DUMP_TXPKT;
            break;
        default:
            BACKDOOR_MGR_Printf("\r\nLEDDRV_DEBUG_FLAG_DUMP_RXPKT (0:OFF, 1:ON): ");
            temp_flag = LEDDRV_DEBUG_FLAG_DUMP_RXPKT;
            break;
    }

    choice = BACKDOOR_MGR_GetChar();
    if(choice < '0' || choice > '1')
    {
        BACKDOOR_MGR_Printf("\r\nInput Error");
        return;
    }

    /* Turn off a debug flag */
    if(choice == '0')
    {
        temp_flag   = ~temp_flag;
        debug_flag &= temp_flag;
        LEDDRV_SetDebug(debug_flag);
    }
    else
    {
        debug_flag |= temp_flag;
        LEDDRV_SetDebug(debug_flag);
    }
}

static void LEDDRV_INIT_BackdoorInfo_CallBack(void)
{
    int     select_value = 0;
    BOOL_T  exitbackdoor = FALSE;
    UI8_T   in_str0[32];
    UI32_T  max_len=sizeof(in_str0)-1;

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n=================================");
        BACKDOOR_MGR_Printf("\r\n  LED Driver Menu        ");
        BACKDOOR_MGR_Printf("\r\n=================================");
        BACKDOOR_MGR_Printf("\r\n [1].Set Debug Flag");
        BACKDOOR_MGR_Printf("\r\n=================================");
        BACKDOOR_MGR_Printf("\r\n [0].Exit");
        BACKDOOR_MGR_Printf("\r\n Press <enter> continue");
        BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

        select_value = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\r\n", (char)select_value);
        switch(select_value)
        {
            case '0':
                exitbackdoor = TRUE;
                break;
            case '1':
                LEDDRV_BACKDOOR_SetDebug();
                break;
            default:
                exitbackdoor = TRUE;
        }
        BACKDOOR_MGR_Printf("\r\nPress any key to continue");
        BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%s", LEDDRV_BACKDOOR_ClearScreen);

        if(exitbackdoor == TRUE)
            break;
    }
}
#endif /* BACKDOOR_ENABLE */
/* FUNCTION NAME: LEDDRV_SetDiagLedStatus
 * PURPOSE: Set local Diag LED
 * INPUT:   led_status, 1: Green; 0:amber  2:  flash green
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function set Diag led
 */
void LEDDRV_SetDiagLedStatus(UI32_T led_state)
{
}

/* FUNCTION NAME: LEDDRV_ProvisionComplete
 * PURPOSE: set system status light for a unit
 * INPUT:   unit - unit number
 *          sys_status - status only, pattern determines in LEDDRV.
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module,
 *         for local LED control.
 */
BOOL_T LEDDRV_ProvisionComplete(UI32_T unit, LEDDRV_System_Status_T  sys_status)
{
    LEDDRV_ShowUnitLED(leddrv_shmem_data_p->my_unit_id);

    return TRUE;
}

/* FUNCTION NAME: LEDDRV_SetSystemXferStatus
 * PURPOSE: set xfer system status light for a unit
 * INPUT:   unit - unit number
 *          port_status - xfer status only, pattern determines in LEDDRV.
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module,
 *         for local LED control.
 */
BOOL_T LEDDRV_SetSystemXferStatus(UI32_T unit, LEDDRV_System_Status_T  sys_status)
{
    return TRUE;
}

#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LEDDRV_SetLocalModuleLed
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to set module led pattern.
 * INPUT   : color - LEDDRV_COLOR_OFF               off
 *                   LEDDRV_COLOR_GREEN             green
 *                   LEDDRV_COLOR_AMBER             amber
 *                   LEDDRV_COLOR_GREENFLASH        green flash
 *                   LEDDRV_COLOR_AMBERFLASH        amber flash
 *                   LEDDRV_COLOR_GREENAMBER_FLASH  green amber flash
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *-------------------------------------------------------------------------
 */
void LEDDRV_SetLocalModuleLed(UI8_T color)
{
    return;
}

#endif

#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LEDDRV_SetLocationLED
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to set location LED status.
 * INPUT   : unit_id   - the unit desired to set
 *           led_is_on - TRUE : Location LED is on
 *                       FALSE: Location LED is off
 * OUTPUT  : None.
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE    : unit_id is ignored in this project because it does not support
 *           stacking.
 *-------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LEDDRV_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on)
{
    const char* shell_cmd_tmp ="/usr/bin/onlp_ledutil -l";
    char shell_cmd[64];
    int status;

    if (led_is_on)
    {
        sprintf(shell_cmd,"%s 1", shell_cmd_tmp);
    }
    else
    {
        sprintf(shell_cmd,"%s 0", shell_cmd_tmp);
    }

    if (SYSFUN_ExecuteSystemShell(shell_cmd)!=SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, shell_cmd);
        return LEDDRV_TYPE_RET_ERR_HW_FAIL;
    }
	return LEDDRV_TYPE_RET_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LEDDRV_GetLocationLEDStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to get location LED status.
 * INPUT   : unit_id   - the unit desired to set
 *           led_is_on - TRUE : Location LED is on
 *                       FALSE: Location LED is off
 * OUTPUT  : None.
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE    : unit_id is ignored in this project because it does not support
 *           stacking.
 *-------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LEDDRV_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p)
{
    const char* shell_cmd_p ="/usr/bin/onlp_ledutil -s";
    char output_buf[32];
    int status;
    UI32_T output_buf_size=sizeof(output_buf);

    if (SYSFUN_GetExecuteCmdOutput(shell_cmd_p, &output_buf_size, output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, shell_cmd_p);
        return 0;
    }
    status=atoi(output_buf);

    if (status == 1)
    {
        *led_is_on_p=TRUE;
    }
    else
    {
        *led_is_on_p=FALSE;
    }
    return LEDDRV_TYPE_RET_OK;
}

#endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

