/* Module Name: LEDDRV.H
 * Purpose: This moudle provides API to light port LEDs
 * Notes: 
 * History:
 *      21/June/2001    -- reconciled with other module and confirmed interface
 *      13/June/2001    -- Created by Jimmy Pai
 *      15/Oct/2002     -- Arden Chiu, Functions related to stacking added
 *
 * Copyright(C)      Accton Corporation, 2001
 */


#ifndef LEDDRV_H
#define LEDDRV_H



/* -------------------------------------------------- INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_mm.h"
#include "isc.h"

/* ------------------------------------------------- NAME CONSTANT DECLARATIONS
 */
#define LEDDRV_MAX_NUM_OF_PORTS (26)


/* ----------------------------------------------------- DATA TYPE DECLARATIONS
 */

/* port status */
typedef struct 
{
    UI8_T   admin           : 1;
    UI8_T   speed           : 2;    /* 10/100/1000/10G */
    UI8_T   duplex          : 1;    /* half/full */
    UI8_T   link            : 1;    /* up/down */
    UI8_T   active          : 1;    /* active/inactive */
    BOOL_T  sfp_in_use      : 1;    /* sfp in use       */
    BOOL_T  xfpmoduleinsert : 1;    /* for EIF8X10G Gbic transiver insert or not*/

#if (SYS_CPNT_POE == TRUE)
    UI8_T   led_mode	    : 1;    /* 1=poe mode, 0=nornal link mode*/
    UI8_T   poe_admin	    : 1;    /* 1=enable, 0 else*/
    UI8_T   poe_link	    : 1;    /* 1=PD found, 0 else*/
    UI8_T   poe_active	    : 1;    /* 1=PD drawing power, 0 else*/
    UI8_T   poe_overload    : 1;    /* 1=overload, 0 else*/ 
#endif

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
    BOOL_T  cable_diag_result : 1; /* for led to display cable diag fail result*/
#endif

} LEDDRV_Port_Status_T;

/* system status */
typedef struct
{
    UI8_T   poweron     : 2;    /* on/off */
    UI8_T   master      : 3;    /* master/slave unit */
    UI8_T   diag_ok     : 2;    /* ok/fail */
    UI8_T   stacking_link   : 2;    /* UP/DOWN stacking port status */
    UI8_T   reserve     : 2;
}   LEDDRV_System_Status_T;

/* system fault type
 * Note: Always add new type to the last existing type because some leddrv code
 *       depends on the order of each enum type definition
 */
typedef enum LEDDRV_System_Fault_Type_E
{
    LEDDRV_SYSTEM_FAULT_TYPE_FAN=0,
    LEDDRV_SYSTEM_FAULT_TYPE_THERMAL,
    LEDDRV_SYSTEM_FAULT_TYPE_PSU,
    LEDDRV_SYSTEM_FAULT_TYPE_MAX_NBR
} LEDDRV_System_Fault_Type_T;

enum LEDDRV_TYPE_RET_E
{
    LEDDRV_TYPE_RET_OK,
    LEDDRV_TYPE_RET_ERR_HW_FAIL,
    LEDDRV_TYPE_RET_ERR_NOT_SUPPORT,
};
typedef UI32_T LEDDRV_TYPE_RET_T;

#define LEDDRV_LINK_UP          1
#define LEDDRV_LINK_DOWN        0

#define LEDDRV_ADMIN_ENABLE     1
#define LEDDRV_ADMIN_DISABLE    0

#define LEDDRV_SFP_PLUGIN     1
#define LEDDRV_SFP_PLUGOUT     0
#define LEDDRV_SPEED_10M        0
#define LEDDRV_SPEED_100M       1
#define LEDDRV_SPEED_1000M      2
#define LEDDRV_SPEED_10G        3

#define LEDDRV_DUPLEX_HALF      0
#define LEDDRV_DUPLEX_FULL      1

#define LEDDRV_PORT_ACTIVE      1
#define LEDDRV_PORT_INACTIVE    0

#define LEDDRV_STACK_ARBITRATION 3
#define LEDDRV_STACK_OFF         2
#define LEDDRV_STACK_MASTER      1
#define LEDDRV_STACK_SLAVE       0
#define LEDDRV_STACK_MASTER_INIT 4

#define LEDDRV_MODULE_A          1
#define LEDDRV_MODULE_B          2
#define LEDDRV_MODULE_BUS_SW_ENABLE   0
#define LEDDRV_MODULE_BUS_SW_DISABLE  1
#define LEDDRV_STACKING_LINK_BOTH_UP            1
#define LEDDRV_STACKING_LINK_ONLY_TX_UP         2
#define LEDDRV_STACKING_LINK_ONLY_RX_UP         3
#define LEDDRV_STACKING_LINK_BOTH_DOWN          4
#define LEDDRV_STACKING_LINK_ONESIDE_UNCONNECT  5
#define LEDDRV_STACKING_LINK_NO_STACK           6      

#define LEDDRV_SYSTEM_NORMAL     0x0
#define LEDDRV_SYSTEM_FAN_FAIL   0x1
#define LEDDRV_SYSTEM_XFER       0x2
#define LEDDRV_SYSTEM_THERMAL    0x3

#define LEDDRV_COLOR_OFF              0
#define LEDDRV_COLOR_GREEN            1
#define LEDDRV_COLOR_AMBER            2
#define LEDDRV_COLOR_GREENFLASH       3
#define LEDDRV_COLOR_AMBERFLASH       4
#define LEDDRV_COLOR_GREENAMBER_FLASH 5

/*FOR POE LED*/
#define LEDDRV_POE_PORT_LINK_UP			1
#define LEDDRV_POE_PORT_LINK_DOWN		0
#define LEDDRV_POE_PORT_ACTIVE			1
#define LEDDRV_POE_PORT_INACTIVE			0
#define LEDDRV_POE_ADMIN_ENABLE			1
#define LEDDRV_POE_ADMIN_DISABLE			0
#define LEDDRV_POE_PORT_OVERLOAD			1
#define LEDDRV_POE_PORT_NOT_OVERLOAD	0

#define LEDDRV_POE_SYSTEM_NORMAL                0x0
#define LEDDRV_POE_SYSTEM_BUDGET_LIMIT          0x1
#define LEDDRV_POE_SYSTEM_THERMAL_THRESHOLD     0x2
#define LEDDRV_POE_SYSTEM_OFF                   0x2

#define LEDDRV_LED_MODE_NORMAL                  0
#define LEDDRV_LED_MODE_POE 	                1
#define LEDDRV_LED_MODE_UNKNOWN	                0xFF

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
#define LEDDRV_CABLE_DIAG_RESULT_NORMAL   0
#define LEDDRV_CABLE_DIAG_RESULT_FAIL     1
#endif

/* ----------------------------------------- EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME: LEDDRV_Initiate_System_Resources
 * PURPOSE: initializes all resources for LEDDRV
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
BOOL_T LEDDRV_InitiateSystemResources(void);

/* FUNCTION NAME: LEDDRV_AttachSystemResources
 * PURPOSE: Attach system resource for LEDDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LEDDRV_AttachSystemResources(void);

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
BOOL_T LEDDRV_Init(void);

/* FUNCTION NAME: LEDDRV_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *
 */
void LEDDRV_Create_InterCSC_Relation(void);

/* FUNCTION NAME: LEDDRV_Active
 * PURPOSE: Prepare display environment, Clear display-buffer, and do nothing.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 *
 */
BOOL_T LEDDRV_Active(void);

/* FUNCTION NAME: LEDDRV_Inactive
 * PURPOSE: Same as LEDDRV_Active but use another pattern.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 *      1. Depend on requirement for future.
 */
BOOL_T LEDDRV_Inactive(void);

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
void LEDDRV_SetTransitionMode(void);

/* FUNCTION NAME: LEDDRV_EnterTransitionMode
 * PURPOSE: set system in transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterTransitionMode (void);

/* FUNCTION NAME: LEDDRV_EnterMasterMode
 * PURPOSE: set system in master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterMasterMode (void);

/* FUNCTION NAME: LEDDRV_EnterSlaveMode
 * PURPOSE: set system in slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDDRV_EnterSlaveMode (void);

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
BOOL_T LEDDRV_ProvisionComplete(UI32_T unit, LEDDRV_System_Status_T  sys_status);

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
void LEDDRV_Display (void);

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
    BOOL_T LEDDRV_SetPortStatus(UI32_T unit, UI32_T port,BOOL_T is_stacking_port, LEDDRV_Port_Status_T *port_status);

/* FUNCTION NAME: LEDDRV_SetSystemStatus
 * PURPOSE: set system status light for a unit
 * INPUT:   unit - unit number
 *          port_status - status only, pattern determines in LEDDRV.
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module,
 *         for local LED control.
 */
BOOL_T LEDDRV_SetSystemStatus (UI32_T unit, LEDDRV_System_Status_T  sys_status);

/* FUNCTION NAME: LEDDRV_SetPoeLed
 * PURPOSE: Set PoE mainpower status light for a unit.
 * INPUT:   unit    -- Unit number
 *          status  -- POEDRV_TYPE_SYSTEM_OVERLOAD, the mainpower overload of PoE system
 *                     POEDRV_TYPE_SYSTEM_NORMAL, the mainpower normal of PoE system
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 */
BOOL_T LEDDRV_SetPoeLed(UI32_T unit, UI8_T status);

void LEDDRV_LightFrontPortLed(UI8_T unit, UI8_T port, UI8_T mode, BOOL_T poe_ack);
void LEDDRV_LightDiagPoELed(UI8_T mode, UI8_T  status);

/* FUNCTION NAME: LEDDRV_ShowUnitLED
 * PURPOSE: Shows the unit number on the LED panel
 * INPUT:   unit - unit number
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module
 */
BOOL_T LEDDRV_ShowUnitLED(UI32_T unit);

/* FUNCTION NAME: LEDDRV_ShowAllUnitLED
 * PURPOSE: Each unit in the stack shows its number on the LED panel
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function SHOULD be called from master LED management(LED_MGMT) module
 */
BOOL_T LEDDRV_ShowAllUnitLED(void);


/* FUNCTION NAME: LEDDRV_SetDiagLedStatus
 * PURPOSE: Set local Diag LED
 * INPUT:   led_status
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function set Diag led
 */
void LEDDRV_SetDiagLedStatus(UI32_T led_state);

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
BOOL_T LEDDRV_SetFaultStatus(UI32_T unit, LEDDRV_System_Fault_Type_T sys_fault_type, UI32_T index, BOOL_T is_fault);
#endif

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* FUNCTION NAME: LEDDRV_SetLoopbackFailStatus
 * PURPOSE: set led for loopback test fail for a unit
 * INPUT:   unit   : unit num
 *          port   : port num
 *          status : port status
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function set loopback test fail LED
 */
BOOL_T LEDDRV_SetLoopbackFailStatus(UI32_T unit, UI32_T port, LEDDRV_Port_Status_T *port_status);
#endif

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
BOOL_T LEDDRV_SetStackingLinkStatus(UI32_T unit, UI32_T status,BOOL_T stack_state,UI32_T up_port,UI8_T up_link_state,UI32_T down_port,UI8_T down_link_state);

BOOL_T LEDDRV_SetStackingPortLinkStatus(UI32_T unit,BOOL_T stack_state,UI32_T up_port,UI8_T up_link_state,UI32_T down_port,UI8_T down_link_state);

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
BOOL_T LEDDRV_SetSystemXferStatus(UI32_T unit, LEDDRV_System_Status_T  sys_status);

/* FUNCTION NAME: LEDDRV_SetHotSwapInsertion
 * PURPOSE: set Module light for Hot Insertion
 * INPUT:   unit  - unit num 
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetHotSwapInsertion(UI32_T unit);

/* FUNCTION NAME: LEDDRV_HotSwapRemoval
 * PURPOSE: set Module light for Hot Removal
 * INPUT:   unit  - unit num 
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none.
 */
BOOL_T LEDDRV_SetHotSwapRemoval(UI32_T unit);

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
BOOL_T LEDDRV_SetPowerStatus(UI32_T unit, UI32_T power, UI32_T status);

/* FUNCTION NAME: LEDDRV_PortShutDown
* PURPOSE: light off for a unit
* INPUT:   none
* OUTPUT:  none
* RETUEN:  none
* NOTES:   none.
*/
void LEDDRV_PortShutDown(void);

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
 * NOTES:   none.
 */
void LEDDRV_SetModuleLed(UI32_T unit, UI8_T color);

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
LEDDRV_TYPE_RET_T LEDDRV_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on);

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
LEDDRV_TYPE_RET_T LEDDRV_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p);

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
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
void LEDDRV_SetLocalModuleLed(UI8_T color);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LEDDRV_SetMiimBusSwitchPattern
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to set module's miim bus switch pattern.
 * INPUT   : module_index - which module
 *                          1 - module A
 *                          2 - module B
 *           pattern      - pattern to set
 *                          0 - enable
 *                          1 - disable
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : The module's MIIM Bus Switch register is located with the system led,
 *           so we put the api under LEDDRV though it's nothing to do with led lighting.
 *-------------------------------------------------------------------------
 */ 
void LEDDRV_SetMiimBusSwitchPattern(UI8_T module_index, UI8_T pattern);
#endif
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

#if SYS_CPNT_POE == TRUE
/* FUNCTION NAME: LEDDRV_SetPoELedStatus
 * PURPOSE: Set local PoE LED
 * INPUT:   led_status
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      1. The function set PoE led
 */
void LEDDRV_SetPoELedStatus(UI32_T led_state);
#endif

/* FUNCTION NAME: LEDDRV_ISC_Handler
 * PURPOSE: Call by isc_agent to handle ISC incoming packets
 * INPUT:   key           - ISC key
 *          mref_handle_p - mref handle of a incoming packet
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *       This function is called by ISC_Agent
 */
BOOL_T LEDDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

/* FUNCTION NAME: LEDDRV_SetMajorAlarmOutputLed
 * FUNCTION: This function will set the Major Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : none
 * RETUEN  : none
 * NOTES   : none
 */
void LEDDRV_SetMajorAlarmOutputLed(UI32_T unit, UI32_T status);

/* FUNCTION NAME: LEDDRV_SetMinorAlarmOutputLed
 * FUNCTION: This function will set the Minor Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : none
 * RETUEN  : none
 * NOTES   : none
 */
void LEDDRV_SetMinorAlarmOutputLed(UI32_T unit, UI32_T status);

#endif   /* LEDDRV_H */

