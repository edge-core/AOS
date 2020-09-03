/* Module Name: icmp_mgr.c
 * Purpose: Provide an interface for other components to access
 *          ICMP statistics. This component access the proc file
 *          In Linux syste, "/proc/net/snmp"
 *
 * Notes:
 *      1.The componenet access and parsing the Linux proc file "/proc/net/snmp"
 *        for retrieving the ICMP statistic value.
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *  0.1 2002.01.22  --  Czliao, 	Created
 *  0.2 2002.10.06  --  William,    Revise code.
 *  0.3 2007.06.12  --  Max Chen,   Porting to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 1999-2007.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "icmp_mgr.h"


/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
   ICMP_MGR_IcmpStatisticType_InMsgs = 0,
   ICMP_MGR_IcmpStatisticType_InErrors,
   ICMP_MGR_IcmpStatisticType_InDestUnreachs,
   ICMP_MGR_IcmpStatisticType_InTimeExcds,
   ICMP_MGR_IcmpStatisticType_InParmProbs,
   ICMP_MGR_IcmpStatisticType_InSrcQuenchs,
   ICMP_MGR_IcmpStatisticType_InRedirects,
   ICMP_MGR_IcmpStatisticType_InEchos,
   ICMP_MGR_IcmpStatisticType_InEchoReps,
   ICMP_MGR_IcmpStatisticType_InTimestamps,
   ICMP_MGR_IcmpStatisticType_InTimestampReps,
   ICMP_MGR_IcmpStatisticType_InAddrMasks,
   ICMP_MGR_IcmpStatisticType_InAddrMaskReps,
   ICMP_MGR_IcmpStatisticType_OutMsgs,
   ICMP_MGR_IcmpStatisticType_OutErrors,
   ICMP_MGR_IcmpStatisticType_OutDestUnreachs,
   ICMP_MGR_IcmpStatisticType_OutTimeExcds,
   ICMP_MGR_cmpStatisticType_OutParmProbs,
   ICMP_MGR_IcmpStatisticType_OutSrcQuenchs,
   ICMP_MGR_IcmpStatisticType_OutRedirects,
   ICMP_MGR_IcmpStatisticType_OutEchos,
   ICMP_MGR_IcmpStatisticType_OutEchoReps,
   ICMP_MGR_IcmpStatisticType_OutTimestamps,
   ICMP_MGR_IcmpStatisticType_OutTimestampReps,
   ICMP_MGR_IcmpStatisticType_OutAddrMasks,
   ICMP_MGR_IcmpStatisticType_OutAddrMaskReps,
   ICMP_MGR_IcmpStatisticType_MAX
}ICMP_MGR_IcmpStatisticType_T;

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static BOOL_T ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_T statistic_type, UI32_T *value);

/* STATIC VARIABLE DECLARATIONS 
 */
char *statistic_label[]={"InMsgs",
                         "InErrors",
                         "InDestUnreachs",
                         "InTimeExcds",
                         "InParmProbs",
                         "InSrcQuenchs",
                         "InRedirects",
                         "InEchos",
                         "InEchoReps",
                         "InTimestamps",
                         "InTimestampReps",
                         "InAddrMasks",
                         "InAddrMaskReps",
                         "OutMsgs",
                         "OutErrors",
                         "OutDestUnreachs",
                         "OutTimeExcds",
                         "OutParmProbs",
                         "OutSrcQuenchs",
                         "OutRedirects",
                         "OutEchos",
                         "OutEchoReps",
                         "OutTimestamps",
                         "OutTimestampReps",
                         "OutAddrMasks",
                         "OutAddrMaskReps"
                        };
                        
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : ICMP_MGR_Init
 * PURPOSE:
 *      Initialize.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void ICMP_MGR_Init()
{
    return;
}

/* FUNCTION	NAME : ICMP_MGR_Enter_Master_Mode
 * PURPOSE:
 *		Enter Master Mode; could handle ICMP management operation.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void ICMP_MGR_Enter_Master_Mode(void)
{
    return;
}

/* FUNCTION	NAME : ICMP_MGR_Enter_Slave_Mode
 * PURPOSE:
 *		Enter Slave	Mode; ignore any request.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void ICMP_MGR_Enter_Slave_Mode(void)
{
    return;
}

/* FUNCTION	NAME : ICMP_MGR_Enter_Transition_Mode
 * PURPOSE:
 *		Enter Transition Mode; release all resource of ICMP.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resources.
 */
void ICMP_MGR_Enter_Transition_Mode(void)
{
    return;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInMsgs
 * PURPOSE:
 *      Get (icmpInMsgs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_msgs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInMsgs).
 *      2. "The total number of ICMP messages which the entity
            received.  Note that this counter includes all those counted
            by icmpInErrors."
 */
BOOL_T  ICMP_MGR_GetIcmpInMsgs(UI32_T  *icmp_in_msgs)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InMsgs, icmp_in_msgs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInErrors
 * PURPOSE:
 *      Get (icmpInErrors) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_errors - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInErrors).
 *      2. "The number of ICMP messages which the entity received but
 *          determined as having ICMP-specific errors (bad ICMP
 *          checksums, bad length, etc.)."
 */
BOOL_T  ICMP_MGR_GetIcmpInErrors(UI32_T  *icmp_in_errors)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InErrors, icmp_in_errors))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInDestUnreachs
 * PURPOSE:
 *      Get (icmpInDestUnreachs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_dest_unreachs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInDestUnreachs).
 *      2. "The number of ICMP Destination Unreachable messages
 *          received."
 */
BOOL_T  ICMP_MGR_GetIcmpInDestUnreachs(UI32_T  *icmp_in_dest_unreachs)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InDestUnreachs, icmp_in_dest_unreachs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInTimeExcds
 * PURPOSE:
 *      Get (icmpInTimeExcds) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_time_excds - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInTimeExcds).
 *      2. "The number of ICMP Time Exceeded messages received."
 */
BOOL_T ICMP_MGR_GetIcmpInTimeExcds(UI32_T  *icmp_in_time_excds)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InTimeExcds, icmp_in_time_excds))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInParmProbs
 * PURPOSE:
 *      Get (icmpInParmProbs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_parm_probs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInParmProbs).
 *      2. "The number of ICMP Parameter Problem messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInParmProbs(UI32_T  *icmp_in_parm_probs)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InParmProbs, icmp_in_parm_probs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInSrcQuenchs
 * PURPOSE:
 *      Get (icmpInSrcQuenchs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_src_quenchs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInSrcQuenchs).
 *      2. "The number of ICMP Source Quench messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInSrcQuenchs(UI32_T  *icmp_in_src_quenchs)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InSrcQuenchs, icmp_in_src_quenchs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInRedirects
 * PURPOSE:
 *      Get (icmpInRedirects) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_redirects - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInRedirects).
 *      2. "The number of ICMP Redirect messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInRedirects(UI32_T  *icmp_in_redirects)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InRedirects, icmp_in_redirects))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInEchos
 * PURPOSE:
 *      Get (icmpInEchos) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_echos - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInEchos).
 *      2. "The number of ICMP Echo (request) messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInEchos(UI32_T  *icmp_in_echos)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InEchos, icmp_in_echos))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInEchoReps
 * PURPOSE:
 *      Get (icmpInEchoReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_echo_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInEchoReps).
 *      2. "The number of ICMP Echo Reply messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInEchoReps(UI32_T  *icmp_in_echo_reps)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InEchoReps, icmp_in_echo_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInTimestamps
 * PURPOSE:
 *      Get (icmpInTimestamps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_timestamps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInTimestamps).
 *      2. "The number of ICMP Timestamp Reply messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInTimestamps(UI32_T  *icmp_in_timestamps)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InTimestamps, icmp_in_timestamps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInTimestampReps
 * PURPOSE:
 *      Get (icmpInTimestampReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_timestamp_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInTimestampReps).
 *      2. "The number of ICMP Timestamp Reply messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInTimestampReps(UI32_T  *icmp_in_timestamp_reps)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InTimestampReps, icmp_in_timestamp_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInAddrMasks
 * PURPOSE:
 *      Get (icmpInAddrMasks) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_addr_masks - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInAddrMasks).
 *      2. "The number of ICMP Address Mask Request messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInAddrMasks(UI32_T  *icmp_in_addr_masks)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InAddrMasks, icmp_in_addr_masks))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpInAddrMaskReps
 * PURPOSE:
 *      Get (icmpInAddrMaskReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_in_addr_mask_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpInAddrMaskReps).
 *      2. "The number of ICMP Address Mask Reply messages received."
 */
BOOL_T  ICMP_MGR_GetIcmpInAddrMaskReps(UI32_T  *icmp_in_addr_mask_reps)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_InAddrMaskReps, icmp_in_addr_mask_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutMsgs
 * PURPOSE:
 *      Get (icmpOutMsgs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_msgs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutMsgs).
 *      2. "The total number of ICMP messages which this entity
 *          attempted to send.  Note that this counter includes all
 *          those counted by icmpOutErrors."
 */
BOOL_T  ICMP_MGR_GetIcmpOutMsgs(UI32_T  *icmp_out_msgs)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutMsgs, icmp_out_msgs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutErrors
 * PURPOSE:
 *      Get (icmpOutErrors) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_errors - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutErrors).
 *      2. "The number of ICMP messages which this entity did not send
 *          due to problems discovered within ICMP such as a lack of
 *          buffers.  This value should not include errors discovered
 *          outside the ICMP layer such as the inability of IP to route
 *          the resultant datagram.  In some implementations there may
 *          be no types of error which contribute to this counter's
 *          value."
 */
BOOL_T ICMP_MGR_GetIcmpOutErrors(UI32_T  *icmp_out_errors)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutErrors, icmp_out_errors))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutDestUnreachs
 * PURPOSE:
 *      Get (icmpOutDestUnreachs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_dest_unreachs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutDestUnreachs).
 *      2. "The number of ICMP Destination Unreachable messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutDestUnreachs(UI32_T  *icmp_out_dest_unreachs)
{
    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutDestUnreachs, icmp_out_dest_unreachs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutTimeExcds
 * PURPOSE:
 *      Get (icmpOutTimeExcds) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_time_excds - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutTimeExcds).
 *      2. "The number of ICMP Time Exceeded messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutTimeExcds(UI32_T  *icmp_out_time_excds)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutTimeExcds, icmp_out_time_excds))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutParmProbs
 * PURPOSE:
 *      Get (icmpOutParmProbs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_parm_probs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutParmProbs).
 *      2. "The number of ICMP Parameter Problem messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutParmProbs(UI32_T  *icmp_out_parm_probs)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_cmpStatisticType_OutParmProbs, icmp_out_parm_probs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutSrcQuenchs
 * PURPOSE:
 *      Get (icmpOutSrcQuenchs) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_src_quenchs - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutSrcQuenchs).
 *      2. "The number of ICMP Source Quench messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutSrcQuenchs(UI32_T  *icmp_out_src_quenchs)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutSrcQuenchs, icmp_out_src_quenchs))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutRedirects
 * PURPOSE:
 *      Get (icmpOutRedirects) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_redirects - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutRedirects).
 *      2. "The number of ICMP Redirect messages sent.  For a host,
 *          this object will always be zero, since hosts do not send
 *          redirects."
 */
BOOL_T  ICMP_MGR_GetIcmpOutRedirects(UI32_T  *icmp_out_redirects)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutRedirects, icmp_out_redirects))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutEchos
 * PURPOSE:
 *      Get (icmpOutEchos) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_echos - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutEchos).
 *      2. "The number of ICMP Echo (request) messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutEchos(UI32_T  *icmp_out_echos)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutEchos, icmp_out_echos))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutEchoReps
 * PURPOSE:
 *      Get (icmpOutEchoReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_echo_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutEchoReps).
 *      2. "The number of ICMP Echo Reply messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutEchoReps(UI32_T  *icmp_out_echo_reps)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutEchoReps, icmp_out_echo_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutTimestamps
 * PURPOSE:
 *      Get (icmpOutTimestamps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_timestamps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutTimestamps).
 *      2. "The number of ICMP Timestamp (request) messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutTimestamps(UI32_T  *icmp_out_timestamps)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutTimestamps, icmp_out_timestamps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutTimestampReps
 * PURPOSE:
 *      Get (icmpOutTimestampReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_timestamp_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutTimestampReps).
 *      2. "The number of ICMP Timestamp Reply messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutTimestampReps(UI32_T  *icmp_out_timestamp_reps)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutTimestampReps, icmp_out_timestamp_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutAddrMasks
 * PURPOSE:
 *      Get (icmpOutAddrMasks) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_addr_masks - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutAddrMasks).
 *      2. "The number of ICMP Address Mask Request messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutAddrMasks(UI32_T  *icmp_out_addr_masks)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutAddrMasks, icmp_out_addr_masks))
    {
	    return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME : ICMP_MGR_GetIcmpOutAddrMaskReps
 * PURPOSE:
 *      Get (icmpOutAddrMaskReps) counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp_out_addr_mask_reps - Ref. (Leaf_2011.h)
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      1. Ref. RFC 2011 about (icmpOutAddrMaskReps).
 *      2. "The number of ICMP Address Mask Reply messages sent."
 */
BOOL_T  ICMP_MGR_GetIcmpOutAddrMaskReps(UI32_T  *icmp_out_addr_mask_reps)
{

    if (TRUE == ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_OutAddrMaskReps, icmp_out_addr_mask_reps))
    {
	    return TRUE;
    }
    return FALSE;
}

/******** Statis Sub Program *********/
/* FUNCTION NAME : ICMP_MGR_GetIcmpStatisticByType
 * PURPOSE:
 *      Get counter of specified type.
 *
 * INPUT:
 *      satistic_type -- The specified statistic type.
 *         ICMP_MGR_IcmpStatisticType_InMsgs = 0,
 *         ICMP_MGR_IcmpStatisticType_InErrors,
 *         ICMP_MGR_IcmpStatisticType_InDestUnreachs,
 *         ICMP_MGR_IcmpStatisticType_InTimeExcds,
 *         ICMP_MGR_IcmpStatisticType_InParmProbs,
 *         ICMP_MGR_IcmpStatisticType_InSrcQuenchs,
 *         ICMP_MGR_IcmpStatisticType_InRedirects,
 *         ICMP_MGR_IcmpStatisticType_InEchos,
 *         ICMP_MGR_IcmpStatisticType_InEchoReps,
 *         ICMP_MGR_IcmpStatisticType_InTimestamps,
 *         ICMP_MGR_IcmpStatisticType_InTimestampReps,
 *         ICMP_MGR_IcmpStatisticType_InAddrMasks,
 *         ICMP_MGR_IcmpStatisticType_InAddrMaskReps,
 *         ICMP_MGR_IcmpStatisticType_OutMsgs,
 *         ICMP_MGR_IcmpStatisticType_OutErrors,
 *         ICMP_MGR_IcmpStatisticType_OutDestUnreachs,
 *         ICMP_MGR_IcmpStatisticType_OutTimeExcds,
 *         ICMP_MGR_cmpStatisticType_OutParmProbs,
 *         ICMP_MGR_IcmpStatisticType_OutSrcQuenchs,
 *         ICMP_MGR_IcmpStatisticType_OutRedirects,
 *         ICMP_MGR_IcmpStatisticType_OutEchos,
 *         ICMP_MGR_IcmpStatisticType_OutEchoReps,
 *         ICMP_MGR_IcmpStatisticType_OutTimestamps,
 *         ICMP_MGR_IcmpStatisticType_OutTimestampReps,
 *         ICMP_MGR_IcmpStatisticType_OutAddrMasks,
 *         ICMP_MGR_IcmpStatisticType_OutAddrMaskReps,
 *
 * OUTPUT:
 *      value - the statistic value of specified type
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available
 *
 * NOTES:
 *      None
 *
 */
static BOOL_T ICMP_MGR_GetIcmpStatisticByType(ICMP_MGR_IcmpStatisticType_T statistic_type, UI32_T *value)
{
    FILE *fp;  /* file descriptor for file operation */
    char buf1[512], buf2[512]; /*Buffer 1 for storing the lable, buffer 2 for storing value */
    char *label_p = NULL;
    char *value_p, *p; /* label_p for bufer 1 pointer, value_p for buffer 2, p is just for insert "\0" */
    BOOL_T ret=FALSE;
    BOOL_T is_found;
    int endflag;  

    if ( (statistic_type < 0) || (statistic_type >=ICMP_MGR_IcmpStatisticType_MAX) )
    {
        /* Invalid statistic type */
        return FALSE;  
    }
    
    fp = fopen("/proc/net/snmp", "r");
    if (NULL==fp) {
        return FALSE;
    }
    
    /* The file display as follows
       Icmp: InMsgs InErrors InDestUnreachs InTimeExcds InParmProbs ......
       Icmp: 79 3 60 16 0 0 0 0 0 0 0 0 0 43 0 43 0 0 0 0 0 0 0 0 0 0
     */
    is_found=FALSE;
    while (fgets(buf1, sizeof buf1, fp)) {
        /* Get first line -- label line */ 
	    if (!fgets(buf2, sizeof buf2, fp))
	    {
	        /* Get second line -- value line */
	        is_found = FALSE;
	        break;
	    }
	    
	    label_p = strchr(buf1, ':'); /* Move pointer to postion of ":" */
	    value_p = strchr(buf2, ':');
	    if (!label_p || !value_p)
	    {
	        /* If can not find ":", the format must be wrong, return FALSE */
	        is_found = FALSE;
	        break;
	    }
	    *label_p = '\0'; /*insert a end of string into buf1 -- label line */

        if (strcmp(buf1,"Icmp")==0)
        {
            /* We found the ICMP statistic, now buf1 store label, and buf2 store value*/
            is_found = TRUE;
            break; 
        }
    }
    fclose(fp);
    
    if (TRUE != is_found)
    {
        /* Can not find the ICMP statistic */
        return FALSE;
    }
    
    
    /* Now the line is ICMP statistic, start to parse the content */
    value_p++;
    label_p++;
    endflag = 0;
    ret=FALSE;
    while (!endflag) 
    {
        label_p += strspn(label_p, " \t\n"); /* skip next blank, move to start of next label */
        value_p += strspn(value_p, " \t\n"); /* skip next blank, move to start of next value*/
              
        p = label_p+strcspn(label_p, " \t\n"); /* move p to blank after end of label  */
        if (*p == '\0')
        {
            /* This is the last label we parsed */
            endflag = 1;
        }else{ 
            /* insert a end of string after label */
            *p = '\0';
        }
                     
        if (0==strcmp(label_p,statistic_label[statistic_type]))
        { 	
            /* We found the specified type, then copy value */
            *value=strtoul(value_p, &value_p, 10);
         	ret = TRUE;
            break;
        }
        label_p = p + 1;
    }
    return ret;
   
}


