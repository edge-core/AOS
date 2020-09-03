/* ------------------------------------------------------------------------
 * FILE NAME - AF_OM.H
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
#ifndef _AF_OM_H
#define _AF_OM_H

#include "sysrsc_mgr.h"
#include "af_type.h"

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void AF_OM_InitiateSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void AF_OM_AttachSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetShMemInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-----------------------------------------------------------------------------
 */
void
AF_OM_GetShMemInfo(
    SYSRSC_MGR_SEGID_T *segid_p,
    UI32_T *seglen_p
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init OM resouce
 * INPUT  : use_default -- set with default value
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AF_OM_Init(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of CDP packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetPortCdptStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of CDP packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_GetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status_p
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of PVST packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of PVST packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_GetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status_p
);

#endif /* #ifndef _AF_OM_H */

