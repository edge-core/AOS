/* Module Name: SWDRV_LIB.H
 * Purpose:
 *         This file provides utility routines which are related
 *         to switch driver, such as translation between SWDRV
 *         constant and corresponding string.
 * Notes:
 *
 * History:
 *       Date        Modifier        Reason
 *       2011/12/12  Kenneth Tzeng   Create this file
 *
 * Copyright(C)      EdgeCore Corporation, 2011
 */

#ifndef SWDRV_LIB_H
#define SWDRV_LIB_H

/* INCLUDE FILE DECLARATIONS
 */
#include "swctrl_om.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* recommended output buffer length for SWDRV_LIB_MapSpeedDuplex()
 */
#define SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN 16

/* recommended output buffer length for SWDRV_LIB_MapPortType()
 */
#define SWDRV_LIB_PORT_TYPE_STR_MAX_LEN 16

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: SWDRV_LIB_MapSFPEEPROMConnector
 * PURPOSE: map connector swdrv constant to corresponding string
 * INPUT: connector -- Connector [Address A0h, Byte 2]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapSFPEEPROMConnector(UI8_T connector, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapSFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: fiber_type -- Fibre Channel Transmission Media (fiber type)
 *        length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapSFPEEPROMTransceiverMedia(UI8_T fiber_type, UI8_T length_km, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapSFPEEPROMTransceiverCompCode
 * PURPOSE: map transceiver compliance code
 *          swdrv constant to corresponding string
 * INPUT: eth_comp -- Ethernet Compliance Codes
 *        teng_eth_comp -- 10G Ethernet Compliance Codes
 *        lengh_km -- Length (single mode)-km [Address A0h, Byte 14] 
 *        length_100m -- Length (single mode)-(100's)m [Address A0h, Byte 15] 
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapSFPEEPROMTransceiverCompCode(UI8_T eth_comp, UI8_T teng_eth_comp, UI8_T length_km, UI8_T length_100m, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapXFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapXFPEEPROMTransceiverMedia(UI8_T length_km, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapXFPEEPROMTransceiverCompCode
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: teng_eth_comp -- 10G Ethernet Compliance Codes
 *        length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapXFPEEPROMTransceiverCompCode(UI8_T teng_eth_comp, UI8_T length_km, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapQSFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: fiber_type -- Fibre Channel Transmission Media (fiber type)
 *        length_km -- Length (single mode)-km [Page 00h Address 142]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapQSFPEEPROMTransceiverMedia(UI8_T fiber_type, UI8_T length_km, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: eth_comp -- 10G Ethernet Compliance Codes
 *        length_km -- Length (single mode)-km [Page 00h Address 142]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode(UI8_T eth_comp, UI8_T length_km, char* data_p);

/* FUNCTION NAME: SWDRV_LIB_MapSpeedDuplex
 * PURPOSE: Map speed duplex constant to the corresponding string
 * INPUT:  speed_duplex -- speed duplex enumeration value
 *                         valid constants are defined in leaf_es4626a.h which
 *                         precede with "VAL_portSpeedDpxCfg_".
 *         buff_len     -- length of buff
 * OUTPUT: buff  -- corresponding string
 * RETURN: TRUE  -- successfully
 *         FALSE -- failure
 * NOTES:  1. Suggests to use SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN as length of buff
 *         2. If buffer length is not large enough, the string in buff will be
 *            truncated.
 */
BOOL_T SWDRV_LIB_MapSpeedDuplex(UI32_T speed_duplex, UI32_T buff_len, char* buff);

/* FUNCTION NAME: SWDRV_LIB_MapPortType
 * PURPOSE: Map port type constant to the corresponding string
 * INPUT:  port_type    -- port type enumeration value
 *                         valid constants are defined in leaf_es4626a.h which
 *                         precede with "VAL_portType_".
 *         buff_len     -- length of buff
 * OUTPUT: buff  -- corresponding string
 * RETURN: TRUE  -- successfully
 *         FALSE -- failure
 * NOTES:  1. Suggests to use SWDRV_LIB_PORT_TYPE_STR_MAX_LEN as length of buff
 *         2. If buffer length is not large enough, the string in buff will be
 *            truncated.
 */
BOOL_T SWDRV_LIB_MapPortType(UI32_T port_type, UI32_T buff_len, char* buff);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_LIB_GetCustomizedSfpTransceiverTypeStr
 * -------------------------------------------------------------------------
 * FUNCTION: To get transceiver type str
 * INPUT   : unit
 *           sfp_index
 *           buf_size
 * OUTPUT  : buf
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_LIB_GetCustomizedSfpTransceiverTypeStr(UI32_T unit, UI32_T sfp_index, UI32_T buf_size, char *buf);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_LIB_ChangeSfpDdmThresholdToInteger
 * -------------------------------------------------------------------------
 * FUNCTION: Change the unit of SFP DDM threshold by multiplying by 100 and
 *           convert its data type from float to integer.
 * INPUT   : threshold_float_p
 * OUTPUT  : threshold_int_p
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_LIB_ChangeSfpDdmThresholdToInteger(SWCTRL_OM_SfpDdmThreshold_T *threshold_flaot_p, SWCTRL_OM_SfpDdmThresholdInteger_T *threshold_int_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_LIB_CalcSFPChecksum
 * -------------------------------------------------------------------------
 * FUNCTION: Calculate the checksum according to cc_type
 * INPUT   : cc_type
 *             (SWDRV_TYPE_SFP_CHECKSUM_CC_BASE,
 *             SWDRV_TYPE_SFP_CHECKSUM_CC_EXT,
 *             SWDRV_TYPE_SFP_CHECKSUM_CC_DMI)
 *           info_p
 * OUTPUT  : calc_cksum_p
 *           real_cksum_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_LIB_CalcSFPChecksum(UI8_T cc_type, UI8_T *info_p, UI8_T *calc_cksum_p, UI8_T *real_cksum_p);

#endif /* End of SWDRV_LIB_H */
