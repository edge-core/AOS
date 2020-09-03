/*--------------------------------------------------------------------------+
 * FILE NAME - DEV_NMTRDRV.h                                                 +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file defines Network Monitor driver APIs                  +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *   Arthur        07/23/2001        New Creation                           +
 *   Ted           01/24/2002        Clarify the API                        +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * ------------------------------------------------------------------------*/

#ifndef DEV_NMTRDRV_H
#define DEV_NMTRDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "swdrv_type.h"

/* NAMING CONSTANT
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_ClearPortCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T port    -   port number
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing the port counter after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_ClearPortCounter(UI32_T unit, UI32_T port);


#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_ClearVlanCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the vlan counter
 * INPUT   : UI32_T vid     -   VID; 0 to clear all vlan
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_ClearVlanCounter(UI32_T vid);
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the all counters in whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_ClearAllCounters(void);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetIfTableStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetIfTableStats(UI32_T unit,
                                   UI32_T start_port,               /* <01302002> */
                                   UI32_T end_port,                 /* <01302002> */
                                   SWDRV_IfTableStats_T  *stats); /* <01302002> */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetIfXTableStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfXTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetIfXTableStats(UI32_T unit,
                                    UI32_T start_port,               /* <01302002> */
                                    UI32_T end_port,                 /* <01302002> */
                                    SWDRV_IfXTableStats_T  *stats);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetRmonStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetRmonStats(UI32_T unit,
                                UI32_T start_port,               /* <01302002> */
                                UI32_T end_port,                 /* <01302002> */
                                SWDRV_RmonStats_T  *stats);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetEtherLikeStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the EtherLike statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetEtherLikeStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_EtherlikeStats_T  *stats);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetEtherLikePause
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the EtherLike pause statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetEtherLikePause(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_EtherlikePause_T  *stats);

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetIfPerQStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the statistics for each CoS queue
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetIfPerQStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_IfPerQStats_T  *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetPfcStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetPfcStats(
    UI32_T unit,
    UI32_T start_port,
    UI32_T end_port,
    SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetQcnStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetQcnStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_QcnStats_T  *stats);
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_GetIfXTableStatsForVlan
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfXTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_vid       -   start vid
 *           UI32_T end_vid         -   end vid
 *           *stats                 -   statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_GetIfXTableStatsForVlan(UI32_T unit,
                                   UI32_T start_vid,
                                   UI32_T end_vid,
                                   SWDRV_IfXTableStats_T  *stats);
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DEV_NMTRDRV_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for dev_nmtrdrv.
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
BOOL_T DEV_NMTRDRV_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p) ;

void DEV_NMTRDRV_Create_InterCSC_Relation(void);

#endif  /* DEV_NMTRDRV_H */

