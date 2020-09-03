#ifndef CLI_API_RSPAN_H
#define CLI_API_RSPAN_H


/*-----------------------------------------------------------
 * ROUTINE NAME - CLI_API_Show_Rspan
 *-----------------------------------------------------------
 * PURPOSE : This is the public handle to show the RSPAN session configuration.
 * INPUT   : cmd_idx - global command index.
 *         : arg - CLI argument pointer.
 *         : ctrl_P - CLI environment pointer
 * OUTPUT  : RSPAN session configuration.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : 3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
UI32_T CLI_API_Show_Rspan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*-----------------------------------------------------------
 * ROUTINE NAME - CLI_API_Rspan
 *-----------------------------------------------------------
 * PURPOSE : This is the public handle to execute the RSPAN configuration.
 * INPUT   : cmd_idx - global command index.
 *         : arg - CLI argument pointer.
 *         : ctrl_P - CLI environment pointer
 * OUTPUT  : RSPAN session configuration.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : 3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
UI32_T CLI_API_Rspan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif

