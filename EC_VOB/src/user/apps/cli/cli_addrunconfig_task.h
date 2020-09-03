/* MODULE NAME: cli_addrunconfig_task.h
 * PURPOSE:
 *  This file is a demonstration code for implementations of CLI addrunconfig task.
 *
 * NOTES:
 *  None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    05/18/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef _CLI_ADDRUNCONFIG_TASK_H_
#define _CLI_ADDRUNCONFIG_TASK_H_

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_ADDRUNCONFIG_CreateTask
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to create CLI addrunconfig task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_ADDRUNCONFIG_CreateTask(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_ADDRUNCONFIG_Task
 * ------------------------------------------------------------------------
 * PURPOSE  : CLI addrunconfig main task, it will process related event
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_ADDRUNCONFIG_Task(void);

#endif /* _CLI_ADDRUNCONFIG_TASK_H_ */
