#ifndef _L_MATH_H
#define _L_MATH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* NAMING CONSTANT
 */

#define	INVALID_ACTIVE_PORT_NUMBER		0xffffffff

/* Minumum and maximum value(including) for offset argument of function
 * L_MATH_MacCalculation.
 */
#define L_MATH_MAC_CALCULATION_OFFSET_MIN -255
#define L_MATH_MAC_CALCULATION_OFFSET_MAX 255

/* MACRO DECLARATIONS
 */

/* the following two MACRO definitions work for gap between timeout_tick and current_tick is less than half of 2**32 or 2**16
 */
#define L_MATH_TimeOut32(current_tick, timeout_tick) ((I32_T)((UI32_T)(timeout_tick) - (UI32_T)(current_tick)) <= 0 )
#define L_MATH_TimeOut16(current_tick, timeout_tick) ((I16_T)((UI16_T)(timeout_tick) - (UI16_T)(current_tick)) <= 0 )

/* EXPORTED FUNCTIONS DECLARATION
 */
/* Lyn Yeh, 12/11/01, 11:26:07 */
UI32_T L_MATH_SelectBalancePort(UI32_T key, UI32_T nbr_of_active_port, UI32_T algorithm);

UI32_T L_MATH_CheckSum(void *start_addr, UI32_T length);

UI16_T L_MATH_CheckSum16 (UI16_T *ptr, UI32_T nbytes);

UI32_T L_MATH_Crc32(UI32_T crc, const I8_T *buf, UI32_T length);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - L_MATH_UdpCheckSumCalc
 * ---------------------------------------------------------------------
 * PURPOSE  : tell whether it is a digital string
 * INPUT    : UI16_T addr_len is address byte length
 *            UI16_T buff[] is an array containing all the octets in the UDP header and data.
 *            UI16_T len_udp is the length (number of octets) of the UDP header and data.
 *            UI16_T src_addr[4] and UI16_T dest_addr[4] are the IP source and destination address octets
 *  
 * OUTPUT   : none
 * RETURN   : TRUE  -- it is a digital string
 *            FALSE -- it is not a digital string
 * NOTES    : To calculate UDP checksum a "pseudo header" is added to the UDP header. This includes:
 *
 *            IP Source Address  4 bytes
 *            IP Destination Address 4 bytes
 *            Protocol  2 bytes
 *            UDP Length 2 bytes
 *              
 *            The checksum is calculated over all the octets of the pseudo header, UDP header and data. 
 *            If the data contains an odd number of octets a pad, zero octet is added to the end of data. 
 *            The pseudo header and the pad are not transmitted with the packet. 
 *             
 *            In the example code, 

 *            NOTE:
 *            if odd byte, it will use len_udp plus 1 byte to calculate, please make sure it won't overlap memory
 * ---------------------------------------------------------------------
 */
UI16_T L_MATH_UdpCheckSumCalc(UI16_T len_udp, UI16_T buff[], UI16_T addr_len, UI16_T src_addr[], UI16_T dest_addr[]);

double L_MATH_log2(double x);
double L_MATH_log10(double x);
double L_MATH_log(double x);
double L_MATH_sqrt(double x);
double L_MATH_pow(double x, double y);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L_MATH_MacCalculation
 * -------------------------------------------------------------------------
 * FUNCTION: This function evaluates the given mac address and the given
 *           offset value to come out a resulting mac address.
 * INPUT   : array_index - The index to the argument mac that the offset value
 *                         will be applied on. array_index 5 means mac[5]. This
 *                         value shall not be larger than 5.
 *           freeze_array_index
 *                       - The array index that is less then this index 
 *                         in mac shall not be changed. This function
 *                         returns FALSE when the elements in mac larger than
 *                         frezze_array_index is changed.
 *                         Specify 0 if all elements in mac are allowed to be
 *                         changed.
 *           offset      - The value to be applied to the given array_index
 *                         of mac. The value can be postive or negative.
 *                         The legal range of offset is
 *                         [L_MATH_MAC_CALCULATION_OFFSET_MIN,L_MATH_MAC_CALCULATION_OFFSET_MAX]
 *           mac_p       - base mac address, must point to a UI8_T array with
 *                         6 elements.
 * OUTPUT  : mac_p       - The resulting mac address.
 * RETURN  : TRUE        - The mac calculation perfoms without error.
 *           FALSE       - An error occurs during mac calculation.
 * NOTE    : 1. In usual, the first 3 bytes in mac belongs to OUI and should
 *              not be changed. Specify freeze_array_index as 3 to ensure that
 *              OUI is not changed.
 * -------------------------------------------------------------------------*/
BOOL_T L_MATH_MacCalculation(UI8_T array_index, UI8_T freeze_array_index, I32_T offset, UI8_T *mac_p);

#endif
