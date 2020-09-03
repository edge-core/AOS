/* Module Name: STKMGMT_TYPE.H
 * Purpose: 
 * Notes:   
 * History:
 *    12/03/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 */
 
 
#ifndef	STKMGMT_TYPE_H
#define	STKMGMT_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */


/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
enum /* function number */
{
    STKMGMT_INIT_Create_Tasks_FunNo = 0,
    STKCTRL_TASK_Create_Tasks_FunNo,
    STKTPLG_TASK_Create_Tasks_FunNo,
    STKTPLG_OM_GetLocalMaxPortCapability_FunNo,
    STKTPLG_OM_GetPortType_FunNo,
    STKTPLG_OM_PortExist_FunNo,
    STKCTRL_TASK_StackState_CallBack_FunNo,
    STKCTRL_TASK_ReportSyslogMessage_FunNo
};


enum /* error code */
{
    STKTPLG_INIT_Create_Tasks_ErrNo = 0,
    STKCTRL_INIT_Create_Tasks_ErrNo,
    STKCTRL_TASK_Create_Tasks_ErrNo,
    STKTPLG_TASK_Create_Tasks_ErrNo,
    SYS_CALLBACK_TASK_Create_Tasks_ErrNo,   /*water_huang*/
    STKTPLG_OM_GetLocalMaxPortCapability_SwitchToDefault_ErrNo,
    STKTPLG_OM_GetPortType_SwitchToDefault_ErrNo,
    MainBoardType_SwitchToDefault_ErrNo,
    STKCTRL_TASK_StackState_CallBack_SwitchToDefault_ErrNo,
    STKCTRL_TASK_ReportSyslogMessage_SwitchToDefault_ErrNo
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 


/* MACRO FUNCTION DECLARATIONS
 */


#endif  /* STKMGMT_TYPE_H */

