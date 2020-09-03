/*-----------------------------------------------------------------------------
 * Module   : led_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide APIs to access LED_MGR.
 *    
 * History: 
 *          Date         Modifier,        Reason
 * ------------------------------------------------------------------------
 *          2007.08.01   Echo Chen        Created   
 *-----------------------------------------------------------------------------
 * Copyright(C) Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "led_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void LED_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);
#if 0
static void LED_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size);

#endif
/* STATIC VARIABLE DECLARATIONS 
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: LED_PMGR_InitiateProcessResource
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:
 *          TRUE  --  Sucess
 *          FALSE --  Error
 * NOTES:
 */
BOOL_T LED_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME: LED_PMGR_Start_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED blink green
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
BOOL_T LED_PMGR_Start_Xfer(UI32_T unit)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetUnitColor_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_START_XFER,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)LED_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: LED_PMGR_Stop_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED back to normal
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
BOOL_T LED_PMGR_Stop_Xfer(UI32_T unit)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetUnitColor_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_STOP_XFER,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)LED_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME - LED_PMGR_SetModuleLED
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
 */
BOOL_T LED_PMGR_SetModuleLED(UI32_T unit_id, UI8_T color)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetUnitColor_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit_id;
    data_p->color  = color;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_MODULE_LED,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)LED_MGR_MSG_RETVAL(msg_p);
}

#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_SetLocationLED
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
LEDDRV_TYPE_RET_T LED_PMGR_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetLocationLED_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetLocationLED_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id   = unit_id;
    data_p->is_led_on = led_is_on;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_LOCATION_LED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetLocationLED_T),
                     LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T)FALSE);

    return (LEDDRV_TYPE_RET_T)LED_MGR_MSG_RETVAL(msg_p);

}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_GetLocationLEDStatus
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
LEDDRV_TYPE_RET_T LED_PMGR_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_GetLocationLED_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_GetLocationLED_T *data_p;

    if(led_is_on_p==NULL)
        return FALSE;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id   = unit_id;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_GET_LOCATION_LED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_GetLocationLED_T),
                     LED_MGR_GET_MSGBUFSIZE(BOOL_T),
                     (UI32_T)FALSE);

    *led_is_on_p = *((BOOL_T*)LED_MGR_MSG_DATA(msg_p));
    return (LEDDRV_TYPE_RET_T)LED_MGR_MSG_RETVAL(msg_p);
}

#endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

/* FUNCTION NAME - LED_PMGR_SetPoeLED
 * PURPOSE  : This function will set the poe led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            status  -- POEDRV_TYPE_SYSTEM_OVEROAD, system overload
                         POEDRV_TYPE_SYSTEM_NORMAL, system normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 */
BOOL_T LED_PMGR_SetPoeLED(UI32_T unit_id, UI8_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetUnitColor_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit_id;
    data_p->color  = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_POE_LED,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitColor_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)LED_MGR_MSG_RETVAL(msg_p);
}

void LED_PMGR_SetPOELed_callback(UI32_T unit_id, UI32_T port_id, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetPoELED_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetPoELED_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit_id;
    data_p->port_id  = port_id;
    data_p->status  = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_POE_LED_CALLBACK,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetPoELED_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    /*return (BOOL_T)LED_MGR_MSG_RETVAL(msg_p);*/
    return;
}

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetFanFailLED
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the FAN fail led
 * INPUT   : unit   -- in which unit
 *           fan    -- which fan id
 *           status -- fan status(VAL_switchFanStatus_ok/VAL_switchFanStatus_failure)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetFanFailLED(UI32_T unit, UI32_T fan, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitFanstatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetUnitFanstatus_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;
    data_p->fan_id = fan;
    data_p->status = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_FAN_FAIL_LED,
                         msg_p,
                         LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetUnitFanstatus_T),
                         LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return ;
}
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_PMGR_ThermalStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Light the LED according to the given thermal sensor status
 * INPUT    : unit          - unit number
 *            thermal       - thermal sensor number (Starts from 1)
 *            is_abnormal   - TRUE  - the temperature of the given thermal sensor falls in the
 *                                    abnormal region (overheating or undercooling)
 *                            FALSE - the temperature of the given thermal sensor falls in the
 *                                    normal region
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void LED_PMGR_ThermalStatusChanged(UI32_T unit, UI32_T thermal, BOOL_T is_abnormal)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_ThermalStatusChanged_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_ThermalStatusChanged_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;
    data_p->thermal_id = thermal;
    data_p->is_abnormal = is_abnormal;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_THERMAL_STATUS_CHANGED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_ThermalStatusChanged_T),
                     LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T)FALSE);

    return;
}

#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_PMGR_PowerStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Light the LED according to the given psu status
 * INPUT    : unit          - unit number
 *            power_id      - psu id (Starts from 1)
 *            status        - VAL_swIndivPowerStatus_notPresent
 *                            VAL_swIndivPowerStatus_green
 *                            VAL_swIndivPowerStatus_red
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void LED_PMGR_PowerStatusChanged(UI32_T unit, UI32_T power_id, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_PowerStatusChanged_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_PowerStatusChanged_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;
    data_p->power_id = power_id;
    data_p->status = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_POWER_STATUS_CHANGED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_PowerStatusChanged_T),
                     LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T)FALSE);

    return;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetMajorAlarmOutputLed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the Major Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetMajorAlarmOutputLed(UI32_T unit, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetAlarmOutputLed_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetAlarmOutputLed_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;
    data_p->status = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_MAJOR_ALARM_OUTPUT_LED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetAlarmOutputLed_T),
                     LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T)FALSE);

    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetMinorAlarmOutputLed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the Minor Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetMinorAlarmOutputLed(UI32_T unit, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetAlarmOutputLed_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    LED_MGR_IPCMsg_SetAlarmOutputLed_T *data_p;

    data_p = LED_MGR_MSG_DATA(msg_p);
    data_p->unit_id  = unit;
    data_p->status = status;

    LED_PMGR_SendMsg(LED_MGR_IPC_CMD_SET_MINOR_ALARM_OUTPUT_LED,
                     msg_p,
                     LED_MGR_GET_MSGBUFSIZE(LED_MGR_IPCMsg_SetAlarmOutputLed_T),
                     LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T)FALSE);

    return;
}

/* LOCAL SUBPROGRAM BODIES 
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the SYS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SYS request message.
 *           res_size  - the size of SYS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : If the message is for a MGR API and has neigher output para. or
 *           return value, use SYS_PMGR_SendMsgWithoutWaittingResponse()
 *           instead.
 *-------------------------------------------------------------------------
 */
static void LED_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_LEDMGMT;
    msg_p->msg_size = req_size;

    LED_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK ||
        LED_MGR_MSG_RETVAL(msg_p) == LED_MGR_IPC_RESULT_FAIL)
        LED_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_SendMsgWithoutWaittingResponse
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message without waitting response.
 * INPUT   : cmd       - the SYS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SYSrequest message.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This is only used for the message that is for a MGR API and
 *           has neigher output para. or return value.
 *-------------------------------------------------------------------------
 */
#if 0
static void LED_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size)
{
    msg_p->cmd = SYS_MODULE_LEDMGMT;
    msg_p->msg_size = req_size;

    LED_MGR_MSG_CMD(msg_p) = cmd;

    SYSFUN_SendRequestMsg(ipcmsgq_handle,
                          msg_p,
                          SYSFUN_TIMEOUT_WAIT_FOREVER,
                          SYSFUN_SYSTEM_EVENT_IPCMSG,
                          0,
                          NULL);
}
#endif

/* End of this file */

