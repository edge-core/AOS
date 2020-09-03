/*--------------------------------------------------------------------------+ 
 * FILE NAME - nmtr_mgr.h                                                       +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file defines Network Monitor APIs and Task                +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *   arthur         07/24/2001       creation                               +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * -------------------------------------------------------------------------*/
/* NOTES:
 * 1. RFC 1493 MIB contains the following group:
 *    a) the dot1dBase   group (dot1dBridge 1)  -- implemented in sta_mgr
 *    b) the dot1dStp    group (dot1dBridge 2)  -- implemented in sta_mgr
 *    c) the dot1dSr     group (dot1dBridge 3)  -- not implemented
 *    d) the dot1dTp     group (dot1dBridge 4)  -- implemented partial, partial in amtr_mgr
 *    e) the dot1dStatic group (dot1dBridge 5)  -- implemented here
 */ 

#ifndef NMTR_PMGR_H
#define NMTR_PMGR_H

/*--------------------------- 
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "sys_type.h"   
#include "swdrv_type.h"
#include "nmtr_mgr.h"


/* TYPE DEFINITIONS  
 */

/* NAMING CONSTANT
 */

/*---------------------------- 
 * EXPORTED SUBPROGRAM BODIES
 *----------------------------*/
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_InitiateProcessResource                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : TRUE  -- success
 *           FALSE -- fail
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_InitiateProcessResource(void);

/*-----------------
 * MIB2 Statistics 
 *-----------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the iftable statistic of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : SWDRV_IfTableStats_T   *if_table_stats - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats_p);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T   *if_table_stats - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats_p);



/*--------------------------
 * Extended MIB2 Statistics                                 
 *--------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfXTableStats                                 
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended iftable a interface           
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfXTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p);



/*---------------------------
 * Ether-like MIB Statistics 
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p);
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p);



/*---------------------------
 * Ether-like MIB Pause Stats
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_SetEtherLikePauseAdminMode
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will set the ether-like pause function
 * INPUT   : UI32_T ifindex                            - interface index
 *           mode                                      - VAL_dot3PauseAdminMode_disabled
 *                                                       VAL_dot3PauseAdminMode_enabledXmit
 *                                                       VAL_dot3PauseAdminMode_enabledRcv
 *                                                       VAL_dot3PauseAdminMode_enabledXmitAndRcv
 * OUTPUT  : None
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetEtherLikePauseAdminMode(UI32_T ifindex, UI32_T mode);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like pause statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetNextEtherLikePause                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p);



/*---------------------
 * RMON MIB Statistics 
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics of a interface                
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats_p);
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index 
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats_p);


#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetPfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextPfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */


#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats);
#endif /* (SYS_CPNT_CN == TRUE) */


/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 4 
 *         (Port Table for Transparent Bridges)                          */
/*
 *          dot1dTpPortTable OBJECT-TYPE
 *              SYNTAX  SEQUENCE OF Dot1dTpPortEntry
 *              ::= { dot1dTp 4 }
 *          dot1dTpPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortTable 1 }
 *          Dot1dTpPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPort             INTEGER,
 *                  dot1dTpPortMaxInfo      INTEGER,
 *                  dot1dTpPortInFrames     Counter,
 *                  dot1dTpPortOutFrames    Counter,
 *                  dot1dTpPortInDiscards   Counter
 *              }
 */              
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry); 


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- the next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetNextDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry); 


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 5 ) -- dot1dTp 5
 *          dot1dTpHCPortTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpHCPortEntry
 *              ::= { dot1dTp 5 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpHCPortTable 1 }
 *          Dot1dTpHCPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpHCPortInFrames   Counter64,
 *                  dot1dTpHCPortOutFrames  Counter64,
 *                  dot1dTpHCPortInDiscards Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetNextDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry);


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 6 ) -- dot1dTp 6
 *          dot1dTpPortOverflowTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpPortOverflowEntry
 *              ::= { dot1dTp 6 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortOverflowTable 1 }
 *          Dot1dTpPortOverflowEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPortInOverflowFrames     Counter32,
 *                  dot1dTpPortOutOverflowFrames    Counter32,
 *                  dot1dTpPortInOverflowDiscards   Counter32
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetNextDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry);


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value   
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetPortUtilization(UI32_T ifindex, NMTR_MGR_Utilization_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get next utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextPortUtilization(UI32_T *ifindex, NMTR_MGR_Utilization_T *utilization_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwideStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide counters of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
void NMTR_PMGR_ClearSystemwideStats(UI32_T ifindex);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide PFC counters of a interface
 * INPUT   : ifindex - interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_PMGR_ClearSystemwidePfcStats(UI32_T ifindex);
#endif /* (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide iftable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended systemwide iftable a interface
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide pause statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetNextSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 * OUTPUT  : UI32_T *ifindex                           - the next interface index
 *           SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats_p);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats_p);

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwidePfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwidePfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats);
#endif /* (SYS_CPNT_CN == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: get the utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetPortUtilization300secs(UI32_T ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_PMGR_GetNextPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 * OUTPUT  : ifindex            -- interface index
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetNextPortUtilization300secs(UI32_T *ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetEtherStatsHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified ether statistics
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   es_hc_entry->ether_stats_index          -- (key)
 * OUTPUT   :   es_hc_entry         -- Ether stats high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherStatsHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetEtherStatsHighCapacityEntry(NMTR_MGR_EtherStatsHighCapacityEntry_T *es_hc_entry);

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_PrintPortAllUtilization300secs                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
#if 0
BOOL_T NMTR_MGR_PrintPortAllUtilization300secs(UI32_T ifindex, UI32_T select);
#endif

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetDefaultHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetDefaultHistoryCtrlEntry(UI32_T ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetHistoryCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : ctrl_p->ctrl_idx
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_DestroyHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_DestroyHistoryCtrlEntryByIfindex(UI32_T ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get a ctrl entry with specified data source
 * INPUT   : entry_p->ctrl_idx   - for get_next = TRUE
 *           ctrl_p->name        - for get_next = FALSE
 *           ctrl_p->data_source
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCurrentEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get current entry
 * INPUT   : entry_p->ctrl_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCurrentEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryPreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryPreviousEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextHistoryPreviousEntryByCtrlIdx
 *-----------------------------------------------------------------
 * FUNCTION: This function will get next previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(NMTR_TYPE_HistSampleEntry_T *entry_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextRemovedDefaultHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get removed default ctrl entry
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextRemovedDefaultHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextUserCfgHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get user configured ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextUserCfgHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

#endif /* NMTR_PMGR_H */

