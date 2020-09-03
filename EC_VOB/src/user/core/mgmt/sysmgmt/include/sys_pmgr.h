#ifndef SYS_PMGR_H
#define SYS_PMGR_H
/* ------------------------------------------------------------------------
 *  FILE NAME  -  sys_pmgr.h
 * ------------------------------------------------------------------------
 * PURPOSE:
 *
 *  History
 *
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_InitiateProcessResource(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetConsoleCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of console setting
 * INPUT:   None
 * OUTPUT:  console setting(by record)
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetConsoleCfg(SYS_MGR_Console_T *consoleCfg);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningPasswordThreshold(UI32_T *passwordThreshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetPasswordThreshold(UI32_T passwordThreshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value   - nactive time out
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleExecTimeOut(UI32_T *time_out_value);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleExecTimeOut(UI32_T time_out_value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleSilentTime(UI32_T *silent_time);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleSilentTime(UI32_T silent_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetSilentTime(UI32_T *silent_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetVtySilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetSilentTime(UI32_T silent_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetTelnetCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of Telnet setting
 * INPUT    : None
 * OUTPUT   : console setting(by record)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetTelnetCfg(SYS_MGR_Telnet_T *telnetCfg);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetPasswordThreshold(UI32_T *passwordThreshold);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Telnet password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetPasswordThreshold(UI32_T passwordThreshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetExecTimeOut(UI32_T *time_out_value);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetExecTimeOut(UI32_T time_out_value);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the parameters of local console UART.
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetUartParameters(SYS_MGR_Uart_Cfg_T *uartCfg);

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetUartOperBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the operating baudrate of local console UART .
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetUartOperBaudrate(SYS_MGR_Uart_BaudRate_T *uart_operbaudrate);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Get_Autobaudrate_Switch
 * ---------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   :
 *
 *
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_Get_Autobaudrate_Switch();
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default uart setting is successfully retrieved.
 *          For UART parameters, any one is changed will be thought
 *          that the setting is different.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *        4. We have a rule that getrunning should be getrunning by field,
 *           but for CLI there is a command "terminal" which can set
 *           all the uart config by one command, so we can getrunning
 *           by record for this uart parameter.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningUartParameters(SYS_MGR_Uart_RunningCfg_T *uart_cfg);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set baudrate of serial port
 * INPUT    : SYS_MGR_Uart_BaudRate_T Baudrate
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartBaudrate(SYS_MGR_Uart_BaudRate_T baudrate);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartParity
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Parity of serial port
 * INPUT    : SYS_MGR_Console_Parity_T Parity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartParity(SYS_MGR_Uart_Parity_T parity);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartDataBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set DataLength of serial port
 * INPUT    : SYS_MGR_Uart_Parity_T DataLength
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartDataBits(SYS_MGR_Uart_Parity_T data_bits);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartStopBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set StopBits of serial port
 * INPUT    : SYS_MGR_Uart_Stop_Bits_T stop_bits
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartStopBits(SYS_MGR_Uart_Stop_Bits_T stop_bits);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysInfo
 * ---------------------------------------------------------------------
 * PURPOSE: get system info for specified unit
 * INPUT    : unit      - key to specifiy a unique uit in the stack
 * OUTPUT   : sys_info  - all the system info of specified unit
 * RETURN   : TRUE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
 /* No component to call this funtion */
BOOL_T SYS_PMGR_GetSysInfo(UI32_T unit, SYS_MGR_Info_T *sys_info);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : *prompt_string -- prompt string with max length 32
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPromptString(UI8_T *prompt_string);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetPromptString(UI8_T *prompt_string);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get running system prompt string
 * INPUT    : *prompt_string -- prompt string address
 * OUTPUT   : *prompt_string -- prompt string with max length 32
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningPromptString(UI8_T *prompt_string);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	 - SYS_PMGR_LogWatchDogExceptionInfo
 * ---------------------------------------------------------------------
 * PURPOSE:	This function will set watch dog timer exception information
 *			to SYS_MGR_WATCHDOG_TIMER_FILE
 * INPUT	: SYS_MGR_WatchDogExceptionInfo_T *wd_own
 * OUTPUT	: None
 * RETURN	: TRUE/FALSE
 * NOTES	: Call by NMI exception	0x500 handler rtcInt() in usrconfig.c
 *			  and backdoor only!!
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_LogWatchDogExceptionInfo(SYS_MGR_WatchDogExceptionInfo_T	*wd_own);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific unit.
 * INPUT  : power_status->sw_unit_index   --- Which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPowerStatus(SYS_MGR_PowerStatus_T *power_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetNextPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some unit.
 * INPUT  : power_status->sw_unit_index   --- Next to which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
 /* No component to call this funtion */
BOOL_T SYS_PMGR_GetNextPowerStatus(SYS_MGR_PowerStatus_T *power_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Which unit.
 *          indiv_power->sw_indiv_power_index      --- Which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Next to which unit.
 *          indiv_power->sw_indiv_power_index      --- Next to which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 *          sw_alarm->sw_alarm_unit_input_index  --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name        --- description of alarm input
 *          sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetNextSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of next alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name  --- description of alarm input
 *          sw_alarm->sw_alarm_status      --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : Used for SNMP.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInputStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 * OUTPUT : sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInputStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_SetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to set name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 *          sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetRunningAlarmInputName
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get name of alarm input if it's different
 *          from default value
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT:  sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetMajorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get major alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMajorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetMinorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMinorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get major and minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *         	sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetNextAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get next major and minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *         	sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure

 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
  /* No component to call this funtion */
BOOL_T SYS_PMGR_GetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
  /* No component to call this funtion */
BOOL_T SYS_PMGR_GetNextFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To set the next fan speed.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_SetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to set force fan speed full
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetFanSpeedForceFull(BOOL_T mode);

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetFanSpeedForceFull(BOOL_T *mode);

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetRunningFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get running force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode  --- TRUE/FALSE.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningFanSpeedForceFull(BOOL_T *mode);
#endif

void	SYS_PMGR_FanStatusChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T status);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_thermal_status->switch_unit_index     --- Which unit
 *            switch_thermal_status->switch_thermal_index  --- Which thermal
 * OUTPUT   : switch_thermal_status->switch_thermal_status --- The temperature of
 *            the given thermal
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_thermal_status->switch_unit_index --- Next to which unit
 *            switch_thermal_status->switch_thermal_index  --- Next to which thermal
 * OUTPUT   : switch_thermal_status->switch_thermal_status --- The temperature of
 *            the given thermal
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDefaultSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the default switch thermal action entry.
 * INPUT    : *entry --- default entry pointer
 * OUTPUT   : *entry --- default entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetDefaultSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get next switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action rising threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            rising_threshold -- rising threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T rising_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action falling threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            falling_threshold -- falling threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T falling_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry action.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            action -- entry action
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry status.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            status -- entry status
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T status);

void	SYS_PMGR_ThermalStatusChanged_CallBack(UI32_T unit, UI32_T thermal, UI32_T status);

#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set console login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleLoginTimeOut(UI32_T time_out_value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set telnet login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetLoginTimeOut(UI32_T time_out_value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value   - nactive time out
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleLoginTimeOut(UI32_T *time_out_value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetLoginTimeOut(UI32_T *time_out_value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysDescr
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system description.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- system description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_sysDescr+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSysDescr(UI32_T unit_id, UI8_T *sys_descrption);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysObjectID
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system object ID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_oid -- system OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSysObjectID(UI32_T unit_id, UI8_T *sys_oid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetProductName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- product name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdName+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductName(UI32_T unit_id, UI8_T *product_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDeviceName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get device name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : device_name    -- device name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_DEVICE_NAME_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetDeviceName(UI32_T unit_id, UI8_T *device_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPrivateMibRoot
 * ---------------------------------------------------------------------
 * PURPOSE  : Get MIB root OID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : mib_root -- MIB root OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPrivateMibRoot(UI32_T unit_id, UI8_T *mib_root);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetModelName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get model name by board ID.
 * INPUT    : unit_id     -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                           means local unit.
 * OUTPUT   : model_name  -- model name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_MODEL_NAME_SIZE + 1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetModelName(UI32_T unit_id, UI8_T *model_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetProductManufacturer
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product manufacturer.
 * INPUT    : none.
 * OUTPUT   : prod_manufacturer -- product manufacturer.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdManufacturer+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductManufacturer(char *prod_manufacturer);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetProductDescription
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product description.
 * INPUT    : none.
 * OUTPUT   : prod_descrption -- product description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdDescription+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductDescription(char *prod_description);

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : The current CPU utilization in percent in the past 5 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_p       -- cpu usage (X%)
 *            cpu_util_float_p -- fractional part of cpu usage (X * 0.001%)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePercentage(UI32_T *cpu_util_p, UI32_T *cpu_util_float_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsageMaximum
 * ---------------------------------------------------------------------
 * PURPOSE  : The maximum CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_max_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsageMaximum(UI32_T *cpu_util_max_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsageAverage
 * ---------------------------------------------------------------------
 * PURPOSE  : The average CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_avg_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsageAverage(UI32_T *cpu_util_avg_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePeakTime
 * ---------------------------------------------------------------------
 * PURPOSE  : The start time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_time -- peak start time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePeakTime(UI32_T *cpu_util_peak_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePeakDuration
 * ---------------------------------------------------------------------
 * PURPOSE  : The duration time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_duration -- peak duration time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePeakDuration(UI32_T *cpu_util_peak_duration);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of CPU utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilAlarmStatus(BOOL_T *alarm_status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of CPU utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuUtilRisingThreshold(UI32_T rising_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilRisingThreshold(UI32_T *rising_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningCpuUtilRisingThreshold(UI32_T *rising_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set falling threshold of CPU utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuUtilFallingThreshold(UI32_T falling_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilFallingThreshold(UI32_T *falling_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningCpuUtilFallingThreshold(UI32_T *falling_threshold_p);

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set high watermark.
 * INPUT    : watermark.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardHighWatermark(UI32_T watermark);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set low watermark.
 * INPUT    : watermark.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardLowWatermark(UI32_T watermark);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set max threshold.
 * INPUT    : threshold.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardMaxThreshold(UI32_T threshold);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set min threshold.
 * INPUT    : threshold.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardMinThreshold(UI32_T threshold);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardStatus(BOOL_T status);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard trap status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardTrapStatus(BOOL_T status);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuGuardInfo
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get cpu guard information.
 * INPUT    : None.
 * OUTPUT   : info - cpu guard information.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuGuardInfo(SYS_MGR_CpuGuardInfo_T *info);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get high watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardHighWatermark(UI32_T *value);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get low watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardLowWatermark(UI32_T *value);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get max threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardMaxThreshold(UI32_T *value);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get min threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardMinThreshold(UI32_T *value);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardStatus(BOOL_T *value);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardTrapStatus(BOOL_T *value);
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilByName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get task CPU util info
 * INPUT    : cpu_util_info_p->task_name
 *            get_next
 * OUTPUT   : cpu_util_info_p
 * RETURN   : TRUE/FALSE
 * NOTES    : To get the first entry by task_name[0] = 0 and get_next = TRUE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilByName(
    SYS_MGR_TaskCpuUtilInfo_T *cpu_util_info_p,
    BOOL_T get_next);
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE) */
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
/*------------------------------------------------------------------------------
 * Function : SYS_PMGR_GetMemoryUtilizationBrief
 *------------------------------------------------------------------------------
 * Purpose  : This function will get memory utilization
 * INPUT    : none
 * OUTPUT   : sys_mem_brief_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 *-----------------------------------------------------------------------------*/
BOOL_T  SYS_PMGR_GetMemoryUtilizationBrief(SYS_MGR_MemoryUtilBrief_T *sys_mem_brief_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilTotalMemory
 * ---------------------------------------------------------------------
 * PURPOSE  : Get total memory capability
 * INPUT    : None.
 * OUTPUT   : total_memory_p -- total size of usable memory (bytes)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilTotalMemory(MEM_SIZE_T *total_memory_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilFreePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : Get free memory percentage
 * INPUT    : None.
 * OUTPUT   : free_percentage_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilFreePercentage(UI32_T *free_percentage_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of memory utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilAlarmStatus(BOOL_T *alarm_status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetMemoryUtilRisingThreshold(UI32_T rising_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilRisingThreshold(UI32_T *rising_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningMemoryUtilRisingThreshold(UI32_T *rising_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetMemoryUtilFallingThreshold(UI32_T falling_threshold);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilFallingThreshold(UI32_T *falling_threshold_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningMemoryUtilFallingThreshold(UI32_T *falling_threshold_p);
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadInTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T remain_seconds
 * OUTPUT   : SYS_TIME_DST time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadInTime(I32_T remain_seconds, SYS_TIME_DST *next_reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST reload_at
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadAtTime(
    SYS_RELOAD_OM_RELOADAT_DST reload_at,
    SYS_TIME_DST *next_reload_time,
    BOOL_T *function_active
    );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadRegularityTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload reload_regularity date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 *            ( 1<= reload_regularity.day_of_month <= 31)
 *            ( 1<= reload_regularity.day_of_week <= 7)
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadRegularityTime(
    SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity,
    SYS_TIME_DST *next_reload_time,
    BOOL_T *function_active
    );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadTimeInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get remain time and reload time.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 * RETURN   : UI32_T
 * NOTES    : return value
 *              0 : Get value success. (remaining_reload_time is continue countdown)
 *              1 : Get value fail
 *              2 : No reload function on. (reload-in, reload-at, reload-regularity are all off)
 *              3 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (The System time is nerver changed by user or sntp)
 *              4 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (Reload at time passed)
 *              "3" and "4" may happen together, but we return "3" advanced.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetReloadTimeInfo(I32_T *remain_seconds, SYS_TIME_DST *reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetReloadIn
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T minute
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadIn(UI32_T minute);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST *reload_at_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadInInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : UI32_T        *remain_seconds
 *            SYS_TIME_DST  *next_reload_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadInInfo(I32_T *remain_seconds, SYS_TIME_DST *next_reload_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadAtInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetRunningReloadAtInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : value
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningReloadAtInfo(
    SYS_RELOAD_OM_RELOADAT_DST *reload_at
    );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadRegularityInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_GetRunningReloadRegularityInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningReloadRegularityInfo(
    SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
    );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_ReloadInCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-in function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadInCancel();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_ReloadAtCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-at function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadAtCancel();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_ReloadRegularityCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-regularity function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadRegularityCancel();

#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */


#endif /* End of SYS_PMGR_H */
