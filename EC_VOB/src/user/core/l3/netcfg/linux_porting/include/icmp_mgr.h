/* Module Name: icmp_mgr.h
 * Purpose: Provide an interface for other components to access
 *          ICMP statistics. This component access the Linux statistic proc file
 *          "/proc/net/snmp" as netstat doing.
 *
 * Notes:
 *      None
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *  0.1 2002.01.22  --  Czliao, 	Created
 *  0.2 2007.05.29  --  Max Chen,   Porting to Linux platform. Also remove IRDP and Redirect functionality.
 *                                  They are no longer used in Mercury while porting.
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002.
 */

#ifndef _ICMP_MGR_H
#define _ICMP_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

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
void ICMP_MGR_Init();

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
void ICMP_MGR_Enter_Master_Mode(void);

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
void ICMP_MGR_Enter_Slave_Mode(void);

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
void ICMP_MGR_Enter_Transition_Mode(void);

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
BOOL_T  ICMP_MGR_GetIcmpInMsgs(UI32_T  *icmp_in_msgs);

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
BOOL_T  ICMP_MGR_GetIcmpInErrors(UI32_T  *icmp_in_errors);

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
BOOL_T  ICMP_MGR_GetIcmpInDestUnreachs(UI32_T  *icmp_in_dest_unreachs);

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
BOOL_T  ICMP_MGR_GetIcmpInTimeExcds(UI32_T  *icmp_in_time_excds);

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
BOOL_T  ICMP_MGR_GetIcmpInParmProbs(UI32_T  *icmp_in_parm_probs);

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
BOOL_T  ICMP_MGR_GetIcmpInSrcQuenchs(UI32_T  *icmp_in_src_quenchs);

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
BOOL_T  ICMP_MGR_GetIcmpInRedirects(UI32_T  *icmp_in_redirects);

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
BOOL_T  ICMP_MGR_GetIcmpInEchos(UI32_T  *icmp_in_echos);

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
BOOL_T  ICMP_MGR_GetIcmpInEchoReps(UI32_T  *icmp_in_echo_reps);

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
BOOL_T  ICMP_MGR_GetIcmpInTimestamps(UI32_T  *icmp_in_timestamps);

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
BOOL_T  ICMP_MGR_GetIcmpInTimestampReps(UI32_T  *icmp_in_timestamp_reps);

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
BOOL_T  ICMP_MGR_GetIcmpInAddrMasks(UI32_T  *icmp_in_addr_masks);

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
BOOL_T  ICMP_MGR_GetIcmpInAddrMaskReps(UI32_T  *icmp_in_addr_mask_reps);

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
BOOL_T  ICMP_MGR_GetIcmpOutMsgs(UI32_T  *icmp_out_msgs);

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
BOOL_T  ICMP_MGR_GetIcmpOutErrors(UI32_T  *icmp_out_errors);

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
BOOL_T  ICMP_MGR_GetIcmpOutDestUnreachs(UI32_T  *icmp_out_dest_unreachs);

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
BOOL_T  ICMP_MGR_GetIcmpOutTimeExcds(UI32_T  *icmp_out_time_excds);

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
BOOL_T  ICMP_MGR_GetIcmpOutParmProbs(UI32_T  *icmp_out_parm_probs);

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
BOOL_T  ICMP_MGR_GetIcmpOutSrcQuenchs(UI32_T  *icmp_out_src_quenchs);

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
BOOL_T  ICMP_MGR_GetIcmpOutRedirects(UI32_T  *icmp_out_redirects);

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
BOOL_T  ICMP_MGR_GetIcmpOutEchos(UI32_T  *icmp_out_echos);

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
BOOL_T  ICMP_MGR_GetIcmpOutEchoReps(UI32_T  *icmp_out_echo_reps);

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
BOOL_T  ICMP_MGR_GetIcmpOutTimestamps(UI32_T  *icmp_out_timestamps);


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
BOOL_T  ICMP_MGR_GetIcmpOutTimestampReps(UI32_T  *icmp_out_timestamp_reps);

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
BOOL_T  ICMP_MGR_GetIcmpOutAddrMasks(UI32_T  *icmp_out_addr_masks);

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
BOOL_T  ICMP_MGR_GetIcmpOutAddrMaskReps(UI32_T  *icmp_out_addr_mask_reps);

#endif /* _ICMP_MGR_H */
