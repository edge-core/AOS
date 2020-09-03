/* MODULE NAME:  telnet_pmgr.h
 * PURPOSE:
 *    PMGR implement for telnet.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/02/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef TELNET_PMGR_H
#define TELNET_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "telnet_mgr.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
BOOL_T TELNET_PMGR_CurrentTelnetSession(UI16_T remote_port, char *myopts, char *hisopts);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_InitiateProcessResource
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
BOOL_T TELNET_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdPort through the IPC msgq.
 * INPUT:
 *    port -- tcp port for telnet
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TELNET_TYPE_RESULT_OK   -- Success
 *    TELNET_TYPE_RESULT_FAIL -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdPort(UI32_T port);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdPort through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    port  -- tcp port for telnet
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdPort(UI32_T *pPort);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TELNET_TYPE_RESULT_OK   -- Success
 *    TELNET_TYPE_RESULT_FAIL -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdStatus(TELNET_State_T state);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdStatus through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    pState -- telnet server status
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdStatus(TELNET_State_T *pState);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdMaxSessions through the IPC msgq.
 *
 * INPUT:
 *    maxSessions -- max number of telnet sessions
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdMaxSession(UI32_T maxSession);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdStatus through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    pState -- telnet server status
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdMaxSession(UI32_T *pMaxSession);

#if (SYS_CPNT_CLUSTER == TRUE)

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTelnetRelaying
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTelnetRelaying through the IPC msgq.
 *
 * INPUT:
 *    UI32_T task_id, BOOL_T bRelaying, UI32_T memberId
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_ClusterToMemberFromUART
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_ClusterToMemberFromUART through the IPC msgq.
 *
 * INPUT:
 *    UI8_T member_id --member id.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_ClusterToMemberFromUART(UI8_T member_id);

#endif  /* #if (SYS_CPNT_CLUSTER == TRUE) */
#endif    /* End of CSCA_PMGR_H */

