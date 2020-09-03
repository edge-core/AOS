/* Module Name: icmpcfg.h
 * Purpose: Provide an interface for other components to access
 *          ICMP statistics.
 *
 * Notes:
 *      1.The statistic can be read only. And can not be reset or modified currently.
 *      2.In Linux platform, these statistics are maintained by kernel.
 *      3.Max, because the IRDP(ICMP Router Discovery, Router advertisement) are not supported in Linux kernel,
 *        and currently are not used by Mercury platform, so in the porting, the related
 *        APIs are removed. If need, please refer to rdisc package which is included in "iputils" package for Linux
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *  0.1 2002.01.22  --  Czliao, 	Created
 *  0.2 2002.10.06  --  William,    Revised, removing unreasonable code and seq.
 *  0.3 2007.05.25  --  Max Chen,   Porting to Linux and only support ICMP statistics read only. 
 *                                  Router advertisement APIs are removed.
 *
 * Copyright(C)      Accton Corporation, 1999-2007.
 */

#ifndef _ICMPCFG_H
#define _ICMPCFG_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
/* For Exceptional Handler */
enum ICMPCFG_FUN_NO_E
{
    ICMPCFG_FUNCTION_NO = 0,
    
    
}; 
/* For Exceptional Handler */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : ICMPCFG_InitiateProcessResources
 * PURPOSE: Init the necessary resource for ICMPCFG.
 *
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
 *      None.
 */
void ICMPCFG_InitiateProcessResources(void);

/* FUNCTION NAME : ICMPCFG_Create_InterCSC_Relation
 * PURPOSE:This function initializes all function pointer registration operations.
 *
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
 *      None.
 */
void ICMPCFG_Create_InterCSC_Relation(void);

/* FUNCTION NAME : ICMPCFG_GetIcmpInMsgs
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
BOOL_T  ICMPCFG_GetIcmpInMsgs(UI32_T  *icmp_in_msgs);

/* FUNCTION NAME : ICMPCFG_GetIcmpInErrors
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
BOOL_T  ICMPCFG_GetIcmpInErrors(UI32_T  *icmp_in_errors);

/* FUNCTION NAME : ICMPCFG_GetIcmpInDestUnreachs
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
BOOL_T  ICMPCFG_GetIcmpInDestUnreachs(UI32_T  *icmp_in_dest_unreachs);

/* FUNCTION NAME : ICMPCFG_GetIcmpInTimeExcds
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
BOOL_T  ICMPCFG_GetIcmpInTimeExcds(UI32_T  *icmp_in_time_excds);

/* FUNCTION NAME : ICMPCFG_GetIcmpInParmProbs
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
BOOL_T  ICMPCFG_GetIcmpInParmProbs(UI32_T  *icmp_in_parm_probs);

/* FUNCTION NAME : ICMPCFG_GetIcmpInSrcQuenchs
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
BOOL_T  ICMPCFG_GetIcmpInSrcQuenchs(UI32_T  *icmp_in_src_quenchs);

/* FUNCTION NAME : ICMPCFG_GetIcmpInRedirects
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
BOOL_T  ICMPCFG_GetIcmpInRedirects(UI32_T  *icmp_in_redirects);

/* FUNCTION NAME : ICMPCFG_GetIcmpInEchos
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
BOOL_T  ICMPCFG_GetIcmpInEchos(UI32_T  *icmp_in_echos);

/* FUNCTION NAME : ICMPCFG_GetIcmpInEchoReps
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
BOOL_T  ICMPCFG_GetIcmpInEchoReps(UI32_T  *icmp_in_echo_reps);

/* FUNCTION NAME : ICMPCFG_GetIcmpInTimestamps
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
BOOL_T  ICMPCFG_GetIcmpInTimestamps(UI32_T  *icmp_in_timestamps);

/* FUNCTION NAME : ICMPCFG_GetIcmpInTimestampReps
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
BOOL_T  ICMPCFG_GetIcmpInTimestampReps(UI32_T  *icmp_in_timestamp_reps);

/* FUNCTION NAME : ICMPCFG_GetIcmpInAddrMasks
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
BOOL_T  ICMPCFG_GetIcmpInAddrMasks(UI32_T  *icmp_in_addr_masks);

/* FUNCTION NAME : ICMPCFG_GetIcmpInAddrMaskReps
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
BOOL_T  ICMPCFG_GetIcmpInAddrMaskReps(UI32_T  *icmp_in_addr_mask_reps);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutMsgs
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
BOOL_T  ICMPCFG_GetIcmpOutMsgs(UI32_T  *icmp_out_msgs);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutErrors
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
BOOL_T  ICMPCFG_GetIcmpOutErrors(UI32_T  *icmp_out_errors);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutDestUnreachs
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
BOOL_T  ICMPCFG_GetIcmpOutDestUnreachs(UI32_T  *icmp_out_dest_unreachs);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutTimeExcds
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
BOOL_T  ICMPCFG_GetIcmpOutTimeExcds(UI32_T  *icmp_out_time_excds);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutParmProbs
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
BOOL_T  ICMPCFG_GetIcmpOutParmProbs(UI32_T  *icmp_out_parm_probs);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutSrcQuenchs
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
BOOL_T  ICMPCFG_GetIcmpOutSrcQuenchs(UI32_T  *icmp_out_src_quenchs);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutRedirects
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
BOOL_T  ICMPCFG_GetIcmpOutRedirects(UI32_T  *icmp_out_redirects);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutEchos
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
BOOL_T  ICMPCFG_GetIcmpOutEchos(UI32_T  *icmp_out_echos);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutEchoReps
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
BOOL_T  ICMPCFG_GetIcmpOutEchoReps(UI32_T  *icmp_out_echo_reps);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutTimestamps
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
BOOL_T  ICMPCFG_GetIcmpOutTimestamps(UI32_T  *icmp_out_timestamps);


/* FUNCTION NAME : ICMPCFG_GetIcmpOutTimestampReps
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
BOOL_T  ICMPCFG_GetIcmpOutTimestampReps(UI32_T  *icmp_out_timestamp_reps);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutAddrMasks
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
BOOL_T  ICMPCFG_GetIcmpOutAddrMasks(UI32_T  *icmp_out_addr_masks);

/* FUNCTION NAME : ICMPCFG_GetIcmpOutAddrMaskReps
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
BOOL_T  ICMPCFG_GetIcmpOutAddrMaskReps(UI32_T  *icmp_out_addr_mask_reps);

#endif /* _ICMPCFG_H */
