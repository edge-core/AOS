/* Module Name:	DCLI.H
 * Purpose:
 *		DCLI, Debug CLI, is a testing task for verifying Telnet valid or not.
 *		After telnet is connectable, this task could be used as debug CLI, for
 *		developer writes some codes, inspecting the status of whole task.
 *
 * Notes:
 *
 * History:
 *		 Date		--  Modifier,  Reason
 *	0.0	2001.11.17	--	William, 	Created.
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */

#ifndef		_DCLI_H
#define		_DCLI_H

/* INCLUDE FILE	DECLARATIONS
 */
#include	"sys_type.h"


/* NAME	CONSTANT DECLARATIONS
 */
#define MAX_USERNAME_LEN		10
#define CLI_SESS_TELNET         1       /* session connection type: telnet */
/*------------------------------------------------------------------------|
 *                           DEFINITIONS                                  |
 *------------------------------------------------------------------------*/
#define ESC_DETECTED			1
#define ESC_NOT_DETECTED		0

#define MAX_INSTRING_LEN		255    /* max length for cli_GetStr */

#define AUTO_LOGOUT_TIME		15		/* 30 minutes for autologout  */
#define INCREASE_COUNTER		0		/* used by adjTnSessCounter, plus 1 */
#define DECREASE_COUNTER		1		/* used by adjTnSessCounter, minus 1 */
#define MAX_PATHPROMPT_LEN		300			/* max length for path prompt */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */
typedef	UI32_T	*jmp_buf;

/********************************* 
 * cli session control structure * 
 *********************************/
typedef struct CLI_CTRL_S
{
	UI8_T is_empty;					/* flag to check if this structure is empty or not 	*/
	UI8_T user_security_level;		/* user security level         */
	UI8_T sess_type;				/* session type: CLI_SESS_UART or CLI_SESS_TELNET  	*/
	UI8_T	sess_id;				/*	2001.11.17, keep the index of this workspace	*/
	UI32_T cli_tid;
	UI32_T swno;					/* current switch(unit) number */
	UI32_T model;					/* 12 or 24 port model         */
	UI8_T user_name[MAX_USERNAME_LEN+1];
	UI8_T *send_buf;
	UI8_T	*cmd_buf;				/*	2001.11.18, William, for Client Cmd buffer	*/
	I32_T socket;
	jmp_buf	env_backup;
}	CLI_CTRL_T;


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/*	ROUTINE NAME : DCLI_SessionInit
 *	FUNCTION	 : initialize session workspace
 *	INPUT		 : None.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 :
 */
void	DCLI_SessionInit(void);


/*	ROUTINE NAME : DCLI_AllocSessionWorkspace
 *	FUNCTION	 : Allocate one cli session workspace from pool.
 *	INPUT		 : None.
 *	OUTPUT		 : None.
 *	RETURN		 : NULL	--	no more available workspace.
 *					other-	the starting pointer of workspace.
 *	NOTES		 :
 *			1. Session Workspace is managed by processing routine, CLI or DCLI.
 */
UI32_T	*DCLI_AllocSessionWorkspace ();


/*	ROUTINE NAME : DCLI_FreeSessionWorkspace
 *	FUNCTION	 : Free workspace to cli session pool.
 *	INPUT		 : work_space -- Pointer point to workspace.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 :
 *			1. Session Workspace is managed by processing routine, CLI or DCLI.
 */
void	DCLI_FreeSessionWorkspace(UI32_T	*work_space);


/*	ROUTINE NAME : DCLI_SetSessionContext
 *	FUNCTION	 : Set cli session workspace, initailized context.
 *	INPUT		 : work_space	-- Pointer point to starting address of workspace.
 *				   tid			-- task-id own this workspace.
 *				   socket_id	-- socket-id, this session handling.
 *				   sess_type	-- cli session type, maybe CLI_SESS_TELNET, UART,...
 *	OUTPUT		 : None.
 *	RETURN		 : TRUE	-- successfully settle down
 *				   FALSE-- invalid workspace.
 *	NOTES		 :
 *			1. Session Workspace is managed by processing routine, CLI or DCLI.
 *			2. processing routine should keep these information, tid, socket_id, and
 *			   session_type in work-space for processing usage.
 */
BOOL_T	DCLI_SetSessionContext(UI32_T *work_space, UI32_T tid, int socket_id, UI32_T sess_type);


/*	ROUTINE NAME : DCLI_SessionMain
 *	FUNCTION	 : Handle all requests from client.
 *	INPUT		 : work_space -- Pointer point to workspace.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 :
 *			1. Handling routine, Session-Main, use information in work-space to
 *			   process all requests.
 *			2. After processing all requests, free all resources allocated in this function,
 *			   but not include work-space, because workspace is allocated in TNSHD_ChildTask().
 *			   Then, just return to caller, ie. TNSHD_ChildTask.
 */
void	DCLI_SessionMain (UI32_T *work_space);


#endif	 /*	DCLI_H */
