/*-----------------------------------------------------------------------------
 * Module Name: lacp_util.h
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the declaration of lacp_util.c
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. This header file provides the declarations functions in lacp utilities.
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 * Modification History:
 *   By      Date        Ver.   Modification Description
 *   ------- ----------  -----  -----------------------------------------------
 *   ckhsu   2001/12/10         Create for Mercury.
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */
#ifndef _LACP_UTIL_H_
#define _LACP_UTIL_H_

#include "lacp_mgr.h"
#include "lacp_type.h"

#if(SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#include "debug_type.h"
#include "sys_time.h"
#endif

#if(SYS_CPNT_DEBUG == TRUE)
#define DEBUG_HEADER()                                                              \
    ({                                                                              \
        static int year, month, day, hour, minute, second;                          \
        SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);    \
        sprintf(DEBUG_LineBuffer, "\r\n%d:%d:%d: LACP: ", hour, minute, second);    \
        DEBUG_LineBuffer;                                                           \
    })
#define DEBUG_STRCAT(strbuffer,format,args...)      \
    do                                              \
    {                                               \
        sprintf(DEBUG_LineBuffer, format, ##args);  \
        strcat(strbuffer, DEBUG_LineBuffer);        \
    } while(0)
#define DEBUG_CONFIG(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_CONFIG,0,format,##args)
#define DEBUG_EVENT(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_EVENT,0,format,##args)
#define DEBUG_PACKET(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_PACKET,0,format,##args)
#define DEBUG_PACKET_DETAIL(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_PACKET_DETAIL,0,format,##args)
#define DEBUG_STATE_MACHING(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_STATE_MACHING,0,format,##args)
#define DEBUG_DATABASE(format,args...)  DEBUG_MGR_Printf(DEBUG_TYPE_LACP,DEBUG_TYPT_MATCH_ALL,DEBUG_TYPE_LACP_DATABASE,0,format,##args)
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define SPRINT_MEMORY(str,mptr,size)                        \
    do                                                      \
    {                                                       \
        int i;                                              \
        unsigned char *ptr = (unsigned char *)mptr;         \
        str[0] = 0;                                         \
        for (i = 0; i < size; i++)                          \
        {                                                   \
            if (i % DUMP_MEMORY_BYTES_PER_LINE == 0)        \
            {                                               \
                if (i > 0) {strcat(str, "\r\n");}           \
                sprintf(DEBUG_LineBuffer, "\t%04xh ", i);    \
                strcat(str, DEBUG_LineBuffer);              \
            }                                               \
            sprintf(DEBUG_LineBuffer, "%02x", *ptr++);      \
            strcat(str,DEBUG_LineBuffer);                   \
        }                                                   \
        if (i > 0) {strcat(str,"\r\n");}                    \
    } while(0)
#else /* #if(SYS_CPNT_DEBUG == TRUE) */
#define DEBUG_HEADER()
#define DEBUG_STRCAT(strbuffer,format,args...)
#define DEBUG_CONFIG(format,args...)
#define DEBUG_EVENT(format,args...)
#define DEBUG_PACKET(format,args...)
#define DEBUG_PACKET_DETAIL(format,args...)
#define DEBUG_STATE_MACHING(format,args...)
#define DEBUG_DATABASE(format,args...)
#define SPRINT_MEMORY(str,mptr,size)
#endif/* #if(SYS_CPNT_DEBUG == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordActorDefault
 *------------------------------------------------------------------------
 * FUNCTION: This function will record default actor information to
 *           related port.
 * INPUT   : port -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordActorDefault( LACP_PORT_T *port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordDefault
 *------------------------------------------------------------------------
 * FUNCTION: This function will record default partner information to
 *           related port.
 * INPUT   : port -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordDefault( LACP_PORT_T *port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Detach_Mux_From_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function will detach the port from the aggregator.
 * INPUT   : pPort -- pointer to port which want to be detached.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Detach_Mux_From_Aggregator( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Disable_Collecting_Distributing
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable the collecting and distributing as
 *           describe in 802.3ad.
 * INPUT   : pPort -- pointer to port which want to be disabled.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Disable_Collecting_Distributing( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Attach_Mux_To_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function will attach port to aggregator.
 * INPUT   : pPort -- pointer to port which want to be attached.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, this is a funny problem. Whether we should let the
 *           logical interface to be up at this time or when port get into
 *           ENABLE_COLLECTING_DISTRIBUTING state?
 *------------------------------------------------------------------------*/
void LACP_Attach_Mux_To_Aggregator( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Collecting_Distributing
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable collecting and distributing of the
 *           port.
 * INPUT   : pPort -- pointer to port which want to be enabled.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, what should we do here? From the standard, we should
 *           enable/disable the collecting/distributing here, but due to
 *           we don't have this function in chip, we can only do nothing
 *           here and let the STP to handle all things. This is the same
 *           as HP's implementation.
 *------------------------------------------------------------------------*/
void LACP_Enable_Collecting_Distributing( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_EnablePort
 *------------------------------------------------------------------------
 * FUNCTION: This function should be called when outside function try to
 *           enable/disable of a port.
 * INPUT   : pPort  -- pointer to port which want to be enabled/disabled.
 *           enable -- TRUE/FALSE for enable/disable a port.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, we have to think in MC2 and Foxfire about this case.
 *           When port is enabled/disabled, the port should try to take
 *           care of a logical interface since it is mixed in MC2/foxfire.
 *------------------------------------------------------------------------*/
void LACP_EnablePort( LACP_PORT_T *pPort, BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the current_while_timer
 * INPUT   : pPort   -- pointer to port which want to be enabled/disabled.
 *           timeout -- timer tick type (long/short)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Current_While_Timer( LACP_PORT_T *pPort, LACP_TIMEOUT_E timeout);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the current_while_timer
 * INPUT   : pPort   -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Current_While_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Current_While_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the current_while_timer status
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Current_While_Timer_Enabled( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the current_while_timer tick
 * INPUT   : pPort -- pointer to port which want to be decreased.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Current_While_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Current_While_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the current_while_timer expired
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Current_While_Timer_Expired( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the periodic timer of a port.
 * INPUT   : pPort   -- pointer to port which want to be started.
 *           timeout -- timeout type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Periodic_Timer( LACP_PORT_T *pPort, LACP_TIMEOUT_E timeout);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the periodic timer of a port.
 * INPUT   : pPort   -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Periodic_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Periodic_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the periodic timer of a port is
 *           enabled or not.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Periodic_Timer_Enabled( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the periodic timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Periodic_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Periodic_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the periodic timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Periodic_Timer_Expired( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be start.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Wait_While_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Wait_While_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Wait_While_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Wait_While_Timer_Enabled( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Wait_While_Timer( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Wait_While_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Wait_While_Timer_Expired( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Generate_PDU
 *------------------------------------------------------------------------
 * FUNCTION: This function will generate a 802.3ad PDU according to type
 * INPUT   : pPort -- pointer to the port which want to generate PDU.
 *           type  -- Pdu type to be generated
 * OUTPUT  : pPdu  -- pointer to variable which will take Pdu
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Generate_PDU( LACP_PORT_T *pPort, LACP_PDU_U  *pPdu, LACP_PDU_TYPE_E type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Check_PDU
 *------------------------------------------------------------------------
 * FUNCTION: This function will check a 802.3ad PDU according to type
 * INPUT   : pPdu -- pointer to the Pdu which want to be checked.
 *           type -- Pdu type to be generated
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE for a correct/incorrect type of Pdu.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Check_PDU( LACP_PDU_U  *pPdu, LACP_PDU_TYPE_E type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_Default_Selected
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the default
 *           SELECTED value of a port.
 * INPUT   : pPort -- pointer to the port which want to update.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_Default_Selected( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_Selected
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the
 *           SELECTED value of a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to update.
 *           pPdu  -- pointer to the Pdu which will be used to compare.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_Selected( LACP_PORT_T *pPort, LACP_PDU_U *pPdu);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_NTT
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the
 *           NTT value of a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to update.
 *           pPdu  -- pointer to the Pdu which will be used to compare.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_NTT( LACP_PORT_T *pPort, LACP_PDU_U *pPdu);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordPDU
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will record the
 *           Pdu content to a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to set.
 *           pPdu  -- pointer to the Pdu which will be used to record.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordPDU( LACP_PORT_T *pPort, LACP_PDU_U *pPdu);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Choose_Matched
 *------------------------------------------------------------------------
 * FUNCTION: This function is not mentioned in 802.3ad but it is necessary.
 *           Because this is the residue to LACP_recordPDU.
 * INPUT   : pPort -- pointer to the port which want to set.
 *           pPdu  -- pointer to the Pdu which will be used to check.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Choose_Matched( LACP_PORT_T *pPort, LACP_PDU_U *pPdu);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Information
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : slot -- slot
 *           port -- port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_Information( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Info
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : info -- pointer to a LAC_INFO_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_Info( LAC_INFO_T *info);

#if 0   /* Allenc */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Find_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to find an appropiate port to aggregate
 *           with.
 * INPUT   : pPort -- pointer to a LACP_PORT_T to search.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Find_AggregatorPort( LACP_PORT_T *pPort);
#endif  /* Allenc */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_AllocateAggregator
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns a pointer of available aggregator, or
 *            NULL if no aggregator is available.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to the aggregator
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
LACP_AGGREGATOR_T*  LACP_UTIL_AllocateAggregator(LACP_PORT_T *port_ptr);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SortAggregatorMembers
 * ------------------------------------------------------------------------
 * FUNCTION : This function will sort the members attached to the specified
 *            aggregator
 * INPUT    : agg_ptr   -- aggregator
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : prev_port this_port               test_port
 *                  |   |                       |
 *                  V   V                       V
 *          O-->O-->O-->@-->O-->O--> ....... -->O-->#-->O--> ... -->O-->||
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_SortAggregatorMembers(LACP_AGGREGATOR_T *agg_ptr);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_GetAggregator
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns TRUE if the aggregator of the specified
 *            port is existing, or FALSE if not available.
 * INPUT    : port_ptr  -- port
 * OUTPUT   : agg_ptr   -- aggregator
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_GetAggregator(LACP_PORT_T *port_ptr, LACP_AGGREGATOR_T *agg_ptr);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_RefreshOperStatusAndAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  : Refresh the operation status and the LACP admin key for the
 *            specified port.
 * INPUT    : port_ptr  -- port pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_RefreshOperStatusAndAdminKey(LACP_PORT_T *port_ptr);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_RefreshPortSpeedDuplex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to refresh the speed duplex mode for
 *           a port.
 * INPUT   : pPort -- pointer to the port we want to refresh.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void    LACP_UTIL_RefreshPortSpeedDuplex(LACP_PORT_T *port_ptr);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetFirst_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get first aggregator port in system.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_GetFirst_AggregatorPort(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next aggregator port in system.
 * INPUT   : pPrev -- previous port
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_GetNext_AggregatorPort( LACP_PORT_T *pPrev);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Message( BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Trace_Route_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message to trace the
 *           calling route.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Trace_Route_Message( BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Pdu
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug Pdu message in system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Pdu( BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Should_Select_Same_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to check ports should select the same
 *           aggregator.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Should_Select_Same_Aggregator( LACP_PORT_T *pA, LACP_PORT_T *pB);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Add_To_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to add port to an aggregator
 *           aggregator.
 * INPUT   : pAgg  -- Aggregator to add port
 *           pPort -- port to be removed
 * OUTPUT  : None
 * RETURN  : Index of port sequence number in aggregator.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Add_To_Aggregator( LACP_AGGREGATOR *pAgg, LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Remove_From_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to remove port to an aggregator
 *           aggregator.
 * INPUT   : pAgg  -- Aggregator to remove port
 *           pPort -- port to be removed
 * OUTPUT  : None
 * RETURN  : Index of port sequence number in aggregator.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Remove_From_Aggregator( LACP_AGGREGATOR *pAgg, LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_First_Port_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get first port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : First port in the aggregator pointed by pass in port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Get_First_Port_In_Aggregator( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Next_Port_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : Next port in the aggregator pointed by pass in port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Get_Next_Port_In_Aggregator( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Port_Index_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : 0     -- Not found.
 *           other -- index of port in aggregator
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Get_Port_Index_In_Aggregator( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Recalculate_CollectingDistributing_Ports
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to recalculate the ports in Collecting/
 *           Distributing state of the aggregator which this port belong
 *           to.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Recalculate_CollectingDistributing_Ports( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_CollectingDistributing
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to check port is in Collecting/
 *           Distributing state or not.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_CollectingDistributing( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Trunk_Member_Set
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to be compatible with Foxfire/MC2 to
 *           know whether this port is a trunk member port or not.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Is a trunk port member.
 *           FALSE -- Is not a trunk port member.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Trunk_Member_Set( UI32_T slot, UI32_T port, UI32_T *pTrkId);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Trunk_Member_Set
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to be compatible with Foxfire/MC2 to
 *           know whether this port is a trunk member port or not.
 * INPUT   : slot -- slot number
 *           port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE  -- Is a trunk port member.
 *           FALSE -- Is not a trunk port member.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Get_Available_TrunkId( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TX_SendPDU
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to send packet out from port.
 * INPUT   : mref -- LReference to data structure.
 *           slot -- slot number.
 *           port -- port number.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_TX_SendPDU( L_MM_Mref_Handle_T *mref_handle_p, UI32_T slot, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_RX_HandlePacket
 *-------------------------------------------------------------------------
 * FUNCTION : This function really handles the incoming LACP packets.
 * INPUT    : LACP_MSG_T *lacp_msg_ptr: the pointer points to
 *                              a message containing packet information.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE :
 *      It allocate a memory buffer via standard C malloc() function and
 *      copy the LACP packet payload to the new buffer. It then free the
 *      packet buffer by calling the free function pointed by the
 *      pktFrtn.free_func function pointer.
 *      Finally it packages these information (packet content buffer,
 *      port number) into a message and sends it to the LACP input queue.
 *      When the LACP received the message from the input queue, it processes
 *      the packet content and modifies its protocol state if necessary.
 *-------------------------------------------------------------------------
 */
void LACP_RX_HandlePacket(LACP_MSG_T *lacp_msg_ptr);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Empty_Trunk
 *------------------------------------------------------------------------
 * FUNCTION: This function is understand whethe this trunk id is empty or
 *           a existing id.
 * INPUT   : mref -- LReference to data structure.
 *           slot -- slot number.
 *           port -- port number.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Empty_Trunk( UI32_T trunk_id);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Print_PDU_Info
 *------------------------------------------------------------------------
 * FUNCTION: This function is used when debug.
 * INPUT   : pPDU   -- pointer to data structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Print_PDU_Info( LACP_PDU_U *pPDU);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_SetInterface
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to set the interface as up or down.
 * INPUT   : slot -- slot number.
 *           port -- port number.
 *           up_or_down -- TRUE/FALSE for Up/Down.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_SetInterface( UI32_T slot, UI32_T port, BOOL_T up_or_down);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggMACAddress
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator MAC address
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggMACAddress( UI32_T agg_id, UI8_T *pMAC);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Priority
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorSystemPriority( UI32_T agg_id, UI16_T *pPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Priority
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorSystemID( UI32_T agg_id, UI8_T *pMAC);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggAggregateOrIndividual
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator aggregate ability
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pAggregatable -- aggregate ability of the aggregator.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggAggregateOrIndividual( UI32_T agg_id, UI8_T *pAggregatable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator admin key
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pAdminKey -- administration key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorAdminKey( UI32_T agg_id, UI16_T *pAdminKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator operational key
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorOperKey( UI32_T agg_id, UI16_T *pOperKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner ID.
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerSystemID( UI32_T agg_id, UI8_T *pMAC);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner priority.
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerSystemPriority( UI32_T agg_id, UI16_T *pPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner operational
 *           key.
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerOperKey( UI32_T agg_id, UI16_T *pOperKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggCollectorMaxDelay
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner operational
 *           key.
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pDelay    -- MaxDelay of Collector.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggCollectorMaxDelay( UI32_T agg_id, UI16_T *pDelay);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorTimeout
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Timeout
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pTimeout -- timeout of the aggregator Actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorTimeout( UI32_T agg_id, UI32_T *pTimeout);

/*int LACP_GetLine(char s[], int lim);*/
#define LACP_GetLine(buf, lim) (BACKDOOR_MGR_RequestKeyIn((buf), (lim)), strlen(buf))


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortListPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a port list related to the
 *           aggregator which the pass in ifIndex associated with.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortListPorts( UI32_T agg_index, UI8_T *pList);

/*------------------------------------------------------------------------
 * ROUTINE NAME - dot3adAggPortActorSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get actor system priority of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorSystemPriority( UI32_T agg_index, UI16_T *pPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a actor system id of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorSystemID( UI32_T agg_index, UI8_T *pSystemID);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get actor admin key of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorAdminKey( UI32_T agg_index, UI16_T *pAdminKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get oper. key of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorOperKey( UI32_T agg_index, UI16_T *pOperKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin system priority of
 *           a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminSystemPriority( UI32_T agg_index, UI16_T *pPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner oper. system priority.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperSystemPriority( UI32_T agg_index, UI16_T *pPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner admin system id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminSystemID( UI32_T agg_index, UI8_T *pMAC);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner Oper. system id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperSystemID( UI32_T agg_index, UI8_T *pMAC);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin key of a physical
 *           port.
 *           key.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminKey( UI32_T agg_index, UI16_T *pOperKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner oper. key of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperKey( UI32_T agg_index, UI16_T *pOperKey);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortSelectedAggID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a selected aggregator id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortSelectedAggID( UI32_T agg_index, UI32_T *pSelectedAggID);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortAttachedAggID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a attach aggregator of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortAttachedAggID( UI32_T agg_index, UI32_T *pAttachedAggID);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Actor port number.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorPort( UI32_T agg_index, UI16_T *pPortNo);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Actor port priority of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorPortPriority( UI32_T agg_index, UI16_T *pPortPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin port number of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortNo  -- admin port number of partner.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminPort( UI32_T agg_index, UI16_T *pPortNo);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to oper. value of partner port number of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortNo   -- oper. value of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperPort( UI32_T agg_index, UI16_T *pPortNo);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a partner admin port priority of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortPriority  -- admin priority of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminPortPriority( UI32_T agg_index, UI16_T *pPortPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get oper. value of port priority of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortPriority  -- oper. value of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperPortPriority( UI32_T agg_index, UI16_T *pPortPriority);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorAdminState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get admin actor state of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAdminState  -- admin actor state.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorAdminState( UI32_T agg_index, UI8_T *pAdminState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorOperState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a actor oper. port state of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperState  -- oper. state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorOperState( UI32_T agg_index, UI8_T *pOperState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner admin port state.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAdminState -- partner admin port state.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminState( UI32_T agg_index, UI8_T *pAdminState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner oper. port state of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperState -- partner oper. state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperState( UI32_T agg_index, UI8_T *pOperState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortAggregateOrIndividual
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get aggregate ability of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAggregatable -- aggregate ability of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortAggregateOrIndividual( UI32_T agg_index, UI8_T *pAggregatable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsLACPDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsLACPDUsRx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerPDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerPDUsRx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerResponsePDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerResponsePDUsRx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsUnknownRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsUnknownRx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsIllegalRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsIllegalRx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsLACPDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsLACPDUsTx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerPDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerPDUsTx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerResponsePDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerResponsePDUsTx( UI32_T agg_index, UI32_T *pCounter);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugRxState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pRxState  -- Rx state of port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugRxState( UI32_T agg_index, UI8_T *pRxState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugLastRxTime
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pRxTime   -- RxTime of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugLastRxTime( UI32_T agg_index, UI32_T *pRxTime);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugMuxState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMuxState -- Mux state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugMuxState( UI32_T agg_index, UI8_T *pMuxState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugMuxReason
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of the port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMuxReason  -- Mux reason of this port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugMuxReason( UI32_T agg_index, UI8_T *pMuxReason);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChurnState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of Actor Churn state
 *           machine
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pACState  -- state of actor churn state machine.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChurnState( UI32_T agg_index, UI8_T *pACState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChurnState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of partner churn
 *           machine
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPCState  -- state of partner churn state machine.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChurnState( UI32_T agg_index, UI8_T *pPCState);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChurnCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChurnCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChurnCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChurnCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorSyncTransitionCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorSyncTransitionCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerSyncTransitionCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerSyncTransitionCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChangeCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChangeCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChangeCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChangeCount( UI32_T agg_index, UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adTablesLastChanged
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get last change of the table.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adTablesLastChanged( UI32_T *pCount);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_Appropriate_ifIndex_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate next ifIndex from the
 *           pass in interface index.
 * INPUT   : agg_index -- index of aggregator to get next
 * OUTPUT  : None
 * RETURN  : 0 -- Not found
 *           other -- next interface index.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_GetNext_Appropriate_ifIndex_From_ifIndex( UI32_T agg_index);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate ifIndex from the
 *           pass in interface index.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( UI32_T agg_index);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_OperPortParameters
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show oper. protocol parameters of
 *           a port.
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_OperPortParameters( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_AdminPortParameters
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show admin protocol parameters of
 *           a port.
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_AdminPortParameters( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_PortProtocolState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set partner admin port state
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_PortProtocolState( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_PortStatistics
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set partner admin port state
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_PortStatistics( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Reset_All_PortStatistics
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to reset all statistics of all ports.
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Reset_All_PortStatistics(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Reset_PortStatistics
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to reset port statistics of all ports.
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Reset_PortStatistics( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_TRK_MGR_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in debug calling
 *           trunk system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_TRK_MGR_Message( BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_All_Ports_In_Agg
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show all ports in the aggregator.
 * INPUT   : slot : slot
 *           port : port
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_All_Ports_In_Agg( UI32_T slot, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Try_To_Attach_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in debug calling
 *           Try_To_Attach.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Try_To_Attach_Message( BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Print_TablesLastChanged
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to print last changed time.
 * INPUT   : lag_mib_objects of MIB object
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Print_TablesLastChanged( LACP_MGR_LagMibObjects_T *lag_mib_objects);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Print_Fail_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to print fail message to add to trunk.
 * INPUT   : err_msg: error message
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Print_Fail_Message( UI32_T err_msg);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Migrate_Ports
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to migrate ports to appropiate Aggregator.
 * INPUT   : pPort -- port to be migrate
 * OUTPUT  : None
 * RETURN  : TRUE  -- SUCCESS
 *           FALSE -- FAIL
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Migrate_Port( LACP_PORT_T *pPort);


/*===========================================================================*/
/* Functions for setting MIB entry                                           */
/*===========================================================================*/


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SetDot3adAggActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set related aggregator admin key
 * INPUT   : agg_id -- index of the aggregator.
 *           admin_key  -- administration key of actor.
 *           is_default -- whether it is to set default value
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : 1. The admin key which is set to the aggregator is set to all
 *              the trunk member
 *           2. The static_admin_key flag is disabled if is_default is TRUE,
 *              else enabled.
 *------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_SetDot3adAggActorAdminKey(UI32_T agg_id, UI16_T admin_key, BOOL_T is_default);


BOOL_T LACP_UTIL_SetDot3adAggActorSystemPriority( UI32_T agg_id, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_ClearStaticAdminKeyFlag
 * ------------------------------------------------------------------------
 * PURPOSE  :   Clear the static admin key flag
 * INPUT    :   UI32_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_ClearStaticAdminKeyFlag(UI32_T port_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3AggPortActorLacp_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   the periodic time out value. short time out or long time out
 * INPUT    :   UI32_T  port_index  -- the port ifindex
 *              UI32_T  timeout   -- the long or short timeout value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- set success
 *              FALSE -- set fail
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_UTIL_SetDot3AggPortActorLacp_Timeout(UI32_T port_index, UI32_T timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_GetDot3AggPortActorLacp_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the periodic time value.
 * INPUT    :   UI32_T  port_index  -- the port ifindex
 *              UI16_T  *timeout    -- the output field
 * OUTPUT   :   *timeout
 * RETURN   :   TRUE  -- get success
 *              FALSE -- get fail
 * NOTE     :   None
 * REF      :
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_UTIL_GetDot3AggPortActorLacp_Timeout(UI32_T port_index, UI32_T *timeout);

BOOL_T LACP_UTIL_SetDot3AggLastChange_Time();

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SetDot3adAggActorTimeout
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set related aggregator timeout
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : timeout -- timeout of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_SetDot3adAggActorTimeout( UI32_T agg_id, UI32_T timeout);

#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_AddStaticLacpTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a static LACP trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *               pPort     -- which port to add
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_AddStaticLacpTrunkMember(UI32_T trunk_id, LACP_PORT_T *pPort);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_DeleteStaticLacpTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a static LACP trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *               pPort     -- which port to delete
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_DeleteStaticLacpTrunkMember(UI32_T trunk_id, LACP_PORT_T *pPort);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is matched with  port admin key, 
                     then add a member to a static LACP trunking port
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember(UI32_T  trunk_id, UI16_T admin_key);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsMatchPortKeyAndDeleteStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is matched with  port admin key, 
                     then delete a member from a static LACP trunking port
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckAggKeyIsMatchPortKeyAndDeleteStaticTrunkMember(UI32_T  trunk_id, UI16_T admin_key);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check port admin key is matched with LAG admin key, 
                     then add a member to a static LACP trunking port
 * INPUT   : port_ptr -- port
 *               admin_key     -- port admin key 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember(LACP_PORT_T *port_ptr, UI16_T admin_key);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check port admin key is matched with  LAG admin key, 
                     then delete a member from a static LACP trunking port
 * INPUT   : port_ptr -- port
 *               admin_key     -- port admin key 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember(LACP_PORT_T *port_ptr, UI16_T admin_key);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsSameWithOtherAgg
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is same with ohter LAG 
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_CheckAggKeyIsSameWithOtherAgg(UI32_T agg_id, UI16_T admin_key);
#endif

#endif /* _LACP_UTIL_H_ */
