/* ------------------------------------------------------------------------
 * FILE NAME - AF_PMGR.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Ezio             14/03/2013      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2013
 * ------------------------------------------------------------------------
 */
#ifndef _AF_PMGR_H
#define _AF_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "af_type.h"
#include "af_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AF_PMGR_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of CDP packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_GetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of CDP packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_GetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of PVST packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_GetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of PVST packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_GetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status
);

#endif /* #ifndef _AF_PMGR_H */

