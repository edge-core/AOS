/* Module Name: LED_MGR.H
 * Purpose: This moudle is responsible for the control of LED activity in
 *          response to system changes such as port link up or down.
 * Notes: 
 * History:
 *      23/Sep/2002     -- add enter transition mode MACRO.
 *      11/Sep/2001     -- master mode will spawn a task to handle status
 *                         change
 *      21/June/2001    -- reconciled with other modules to confirm the
 *                         interface
 *      12/June/2001    -- speed and duplex functions combined for extra
 *                         readability; 'unit' parameter added to accomondate
 *                         master/slave control machanism
 *      07/June/2001    -- First Draft created by Jimmy Pai
 *
 *      02/August/2007  -- Modified for linux platform by Echo Chen
 *
 * Copyright(C)      Accton Corporation, 2001,2007
 */
#ifndef LED_MGR_H
#define LED_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
#include "leddrv.h"
#endif

/* NAME CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - LED_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of LED message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of LED message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define LED_MGR_GET_MSGBUFSIZE(datatype_name) \
        ( sizeof(LED_MGR_IPCMsg_Header_T) + sizeof (datatype_name) ) 
        
/*-------------------------------------------------------------------------
 * MACRO NAME - LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of LED message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of LED message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_EmptyData_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - LED_MGR_MSG_CMD
 *              LED_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the LED command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The LED command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define LED_MGR_MSG_CMD(msg_p)    (((LED_MGR_IPCMsg_T *)msg_p->msg_buf)->header.cmd)
#define LED_MGR_MSG_RETVAL(msg_p) (((LED_MGR_IPCMsg_T *)msg_p->msg_buf)->header.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - LED_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define LED_MGR_MSG_DATA(msg_p)   ((void *)&((LED_MGR_IPCMsg_T *)msg_p->msg_buf)->data)

/* MGR handler will use this when it can't handle the message.
 *                                  (is in transition mode)
 */
#define LED_MGR_IPC_RESULT_FAIL  (-1)

enum
{
    LED_MGR_IPC_CMD_START_XFER,
    LED_MGR_IPC_CMD_STOP_XFER,
    LED_MGR_IPC_CMD_SET_MODULE_LED,
    LED_MGR_IPC_CMD_SET_LOCATION_LED,
    LED_MGR_IPC_CMD_GET_LOCATION_LED,
    LED_MGR_IPC_CMD_SET_POE_LED,
    LED_MGR_IPC_CMD_SET_POE_LED_CALLBACK,
    LED_MGR_IPC_CMD_SET_FAN_FAIL_LED,
    LED_MGR_IPC_CMD_SET_MAJOR_ALARM_OUTPUT_LED,
    LED_MGR_IPC_CMD_SET_MINOR_ALARM_OUTPUT_LED,
    LED_MGR_IPC_CMD_THERMAL_STATUS_CHANGED,
    LED_MGR_IPC_CMD_POWER_STATUS_CHANGED,
};

/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in led pmgr and mgr
 */
typedef struct
{
    UI32_T unit_id;
    UI32_T color ; 
} LED_MGR_IPCMsg_SetUnitColor_T;

#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
typedef struct
{
    UI32_T unit_id;
    BOOL_T is_led_on;
} LED_MGR_IPCMsg_SetLocationLED_T;

typedef struct
{
    UI32_T unit_id;
} LED_MGR_IPCMsg_GetLocationLED_T;
#endif

typedef struct
{
    UI32_T unit_id;
    UI32_T port_id;
    UI32_T status ; 
} LED_MGR_IPCMsg_SetPoELED_T;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef struct
{
    UI32_T unit_id;
    UI32_T fan_id;
    UI32_T status; 
} LED_MGR_IPCMsg_SetUnitFanstatus_T;
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef struct
{
    UI32_T unit_id;
    UI32_T thermal_id;
    BOOL_T is_abnormal; 
} LED_MGR_IPCMsg_ThermalStatusChanged_T;
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
typedef struct
{
    UI32_T unit_id;
    UI32_T power_id;
    UI32_T status;
} LED_MGR_IPCMsg_PowerStatusChanged_T;
#endif

typedef struct
{
    UI32_T unit_id;
    UI32_T status;
} LED_MGR_IPCMsg_SetAlarmOutputLed_T;

typedef struct
{
    /* empty struct.
     */
} LED_MGR_IPCMsg_EmptyData_T;

typedef union 
{
        UI32_T cmd;    /* for sending IPC request. LED_MGR_IPC_CMD1 ... */
        UI32_T result; /* for response */
}  LED_MGR_IPCMsg_Header_T;

typedef union 
{
    LED_MGR_IPCMsg_SetUnitColor_T      unitcolor;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    LED_MGR_IPCMsg_SetUnitFanstatus_T  fan_status;
#endif
    LED_MGR_IPCMsg_SetAlarmOutputLed_T alarm_output_led;
#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
    LED_MGR_IPCMsg_SetLocationLED_T    set_location_led;
    LED_MGR_IPCMsg_GetLocationLED_T    get_location_led;
#endif
    LED_MGR_IPCMsg_EmptyData_T         emptydata;
} LED_MGR_IPCMsg_Data_T;

typedef struct
{
    LED_MGR_IPCMsg_Header_T header;
    LED_MGR_IPCMsg_Data_T data;
} LED_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME: LED_MGR_Init
 * PURPOSE: Call LEDDRV_Init() to prepare display buffer space.
 *          register callback functions to SWCTRL, uport_linkup/linkdown...
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - if initialization sucessful
 *          FALSE - if initialization fail 
 * NOTES:
 *      The function will initilize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* at system startup.
 */
void  LED_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LED_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME: LED_MGR_EnterMasterMode
 * PURPOSE: Call LEDDRV_Active() to clear display buffer, lights master light.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - if initialization sucessful
 *          FALSE - if initialization fail 
 * NOTES:
 *      The function will initilize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* during stacking.
 *      This function SHOULD NOT be use in conjunction with
 *      LED_MGR_EnterSlaveMode().
 */
BOOL_T LED_MGR_EnterMasterMode (void);

/* FUNCTION NAME: LED_MGR_EnterSlaveMode
 * PURPOSE: Call LEDDRV_Active() to clear display buffer,spawn a task to recieve
 *          messages.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - if initialization sucessful
 *          FALSE - if initialization fail 
 * NOTES:
 *      The function will initilize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* during stacking.
 *      This function SHOULD NOT be use in conjunction with
 *      LED_MGR_EnterMasterMode().
 */
BOOL_T LED_MGR_EnterSlaveMode (void);


/* FUNCTION NAME: LED_MGR_Set_TransitionMode
 * PURPOSE: Set the working environment to the environment.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 */
void LED_MGR_Set_TransitionMode (void);

/* FUNCTION NAME: LED_MGR_EnterTransitionMode
 * PURPOSE: Change the working environment to the environment after calling LED_MGR_Init().
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 *          The function will try to light up LED is certain pattern for
 *          LED failiure checking.
 */
BOOL_T LED_MGR_EnterTransitionMode (void);

/* FUNCTION NAME: LED_MGR_Provision_Complete
 * PURPOSE: Lights up diag LED after provision has completed
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Provision_Complete(void);

/* FUNCTION NAME: LED_MGR_Task
 * PURPOSE: recieves incoming status change (port or system) messages then,
 *          reflect the change to LEDs.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 */
 
 
void LED_MGR_CreateTask(void);

/* FUNCTION NAME: LED_MGR_Start_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED blink green
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Start_Xfer(UI32_T unit);

/* FUNCTION NAME: LED_MGR_Stop_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED back to normal
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Stop_Xfer(UI32_T unit);

void LED_MGR_AdminEnable (UI32_T unit, UI32_T port);
void LED_MGR_AdminDisable (UI32_T unit, UI32_T port);
void LED_MGR_Linkup (UI32_T unit, UI32_T port);
void LED_MGR_Linkdown (UI32_T unit, UI32_T port);
void LED_MGR_SpeedDuplexChange(UI32_T unit, UI32_T port,UI32_T speed_duplex);
void LED_MGR_PortTypeChanged(UI32_T unit, UI32_T port, UI32_T port_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_HandleHotInsertion
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
void LED_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_HandleHotRemoval
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
void LED_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetModuleLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the module led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            color   --     LEDDRV_COLOR_OFF              off
 *                           LEDDRV_COLOR_GREEN            green
 *                           LEDDRV_COLOR_AMBER            amber
 *                           LEDDRV_COLOR_GREENFLASH       green flash
 *                           LEDDRV_COLOR_AMBERFLASH       amber flash
 *                           LEDDRV_COLOR_GREENAMBER_FLASH green amber flash
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
void LED_MGR_SetModuleLED(UI32_T unit_id, UI8_T color);

#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetLocationLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the location led status
 * INPUT    : unit_id   -- the unit desired to set
 *            led_is_on -- TRUE : Turn on Location LED
 *                         FALSE: Turn off Location LED
 * OUTPUT   : None
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_MGR_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_GetLocationLEDStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the location led status
 * INPUT    : unit_id     -- the unit desired to set
 * OUTPUT   : led_is_on_p -- TRUE : Location LED is on
 *                           FALSE: Location LED is off
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_MGR_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetPoeLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the PoE led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            status  -- POEDRV_TYPE_SYSTEM_OVEROAD, system overload
                         POEDRV_TYPE_SYSTEM_NORMAL, system normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
void LED_MGR_SetPoeLED(UI32_T unit_id, UI8_T status);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : LED_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for DEV_SWDRV mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void LED_MGR_Display(void);
BOOL_T LED_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
/* ------------------------------------------------ MACRO FUNCTION DECLARATIONS
 */
#endif  /* LED_MGR_H */

