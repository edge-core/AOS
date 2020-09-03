/* MODULE NAME:  onlpdrv_sfp.h
 * PURPOSE:
 *   This module implements the sfp(Small Form-factor Pluggable) related wrapper
 *   which call ONLP functions.
 *
 * NOTES:
 *
 * HISTORY
 *    10/30/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */
#ifndef ONLPDRV_SFP_H
#define ONLPDRV_SFP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ONLP_SFP_INVALID_PORT_ID (-1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    ONLPDRV_GBIC_INFO_TYPE_BASIC = 0,
    ONLPDRV_GBIC_INFO_TYPE_DDM,
}ONLPDRV_GBIC_INFO_TYPE_T;

typedef enum
{
    ONLPDRV_SFP_GBIC_ACCESS_TYPE_READ = 0,
    ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE, /* Not support yet */
}ONLPDRV_SFP_GBIC_ACCESS_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: ONLPDRV_SFP_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do initialization for this module.
 *-----------------------------------------------------------------------------
 * INPUT   : aos_uport_to_onlp_port - array for mapping aos uport id to onlp
 *                                    port id. All unused ports or non-sfp ports
 *                                    should be filled with ONLP_SFP_INVALID_PORT_ID.
 *                                    For example:
 *                                    If aos port id 1 is a sfp port and its
 *                                    corresponding onlp sfp port id is 0, then
 *                                    aos_uport_to_onlp_port[0] should be set as 0.
 *                                    If aos port id 2 is not a sfp port, then
 *                                    aos_uport_to_onlp_port[1] should be set as
 *                                    ONLP_SFP_INVALID_PORT_ID.
 *                                    If the maximum aos port id is 48, then all
 *                                    of the array elements after(including)
 *                                    aos_uport_to_onlp_port[48] should be set
 *                                    as ONLP_SFP_INVALID_PORT_ID when there is
 *                                    any unused elements in the tail of the array.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   :
 */
BOOL_T ONLPDRV_SFP_Init(I16_T aos_uport_to_onlp_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/* FUNCTION NAME: ONLPDRV_SFP_UpdatePortSfpTxDisable
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will configure the sfp tx enable setting kept in the
 *          shadow database to the hardware device.
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 * OUTPUT  : none
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. This function returns FALSE when the specified port is invalid or
 *              is not a sfp port.
 *           2. This function does not support stacking yet.
 */
BOOL_T ONLPDRV_SFP_UpdatePortSfpTxDisable(UI32_T unit, UI32_T port);

/* FUNCTION NAME: ONLPDRV_SFP_SetPortSfpTxDisable
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used set SFP TX_DISABLE state of the specified port
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 *           tx_disable  - the TX_DISABLE state to be set to the specified port
 *                         TRUE : turn on TX_DISABLE
 *                         FALSE: turn off TX_DISABLE
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : The operation is not allowed to perform on non-sfp port.
 */
BOOL_T ONLPDRV_SFP_SetPortSfpTxDisable(UI32_T unit, UI32_T port, BOOL_T tx_disable);

/* FUNCTION NAME: ONLPDRV_SFP_GetSfpPresentStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get sfp present status of the specified port
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 * OUTPUT  : is_present_p- TRUE : The SFP transceiver is present on the specified port.
 *                         FALSE: No SFP transceiver is detected on the specified port.
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. This function returns FALSE when the specified port is invalid or
 *              is not a sfp port.
 *           2. This function does not support stacking yet.
 */
BOOL_T ONLPDRV_SFP_GetSfpPresentStatus(UI32_T unit, UI32_T port, UI8_T *is_present_p);

/* FUNCTION NAME: ONLPDRV_SFP_AccessGbicInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the raw data of the specified offset and gbic info type from
 *          the specified port.
 *-----------------------------------------------------------------------------
 * INPUT   : unit            - unit id
 *           port            - aos port id
 *           gbic_info_type  - Valid types are listed below:
 *                             ONLPDRV_GBIC_INFO_TYPE_BASIC (Read through I2C address 0x50)
 *                             ONLPDRV_GBIC_INFO_TYPE_DDM   (Read through I2C address 0x51)
 *           offset          - data register offset address
 *           size            - size of read/written data
 *           gbic_access_type- Valid types are listed below:
 *                             ONLPDRV_SFP_GBIC_ACCESS_TYPE_READ
 *                             ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE (Not support yet)
 * OUTPUT  : info_p          - read/written data info
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. Does not support ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE because ONLP
 *              not support yet.
 */
BOOL_T ONLPDRV_SFP_AccessGbicInfo(UI32_T unit, UI32_T port, ONLPDRV_GBIC_INFO_TYPE_T gbic_info_type, UI16_T offset, UI8_T size, ONLPDRV_SFP_GBIC_ACCESS_TYPE_T gbic_access_type, UI8_T *info_p);

#endif    /* End of ONLPDRV_SFP_H */

