/* Module Name: SWDRV_LIB.C
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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "swdrv_type.h"
#include "swctrl.h"

#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SWDRV_LIB_FloatToInt(float float_val, I16_T *val_p);

/* STATIC VARIABLE DEFINITIONS
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
BOOL_T SWDRV_LIB_MapSFPEEPROMConnector(UI8_T connector, char* data_p)
{
    switch (connector)
    {
        case SWDRV_TYPE_GBIC_CONNECTOR_SC:
            sprintf(data_p, "SC");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_FIBRE_CHANNEL_STYLE_1:
            sprintf(data_p, "Fibre Channel Style 1 copper connector");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_FIBRE_CHANNEL_STYLE_2:
            sprintf(data_p, "Fibre Channel Style 2 copper connector");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_BNC_TNC:
            sprintf(data_p, "BNC/TNC");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_FIBRE_CHANNEL_COAXIAL:
            sprintf(data_p, "Fibre Channel coaxial headers");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_FIBRE_JACK:
            sprintf(data_p, "FiberJack");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_LC:
            sprintf(data_p, "LC");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_MT_RJ:
            sprintf(data_p, "MT-RJ");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_MU:
            sprintf(data_p, "MU");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_SG:
            sprintf(data_p, "SG");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_OPTICAL_PIGTAIL:
            sprintf(data_p, "Optical pigtail");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_MPO_PARALLEL_OPTIC:
            sprintf(data_p, "MPO Parallel Optic");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_HSSDC_II:
            sprintf(data_p, "HSSDC II");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_COPPER_PIGTAIL:
            sprintf(data_p, "Copper pigtail");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_RJ45:
            sprintf(data_p, "RJ45");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_NO_SEPARABLE_CONNECTOR:
            sprintf(data_p, "No Separable Connector");
            break;
        case SWDRV_TYPE_GBIC_CONNECTOR_UNKNOWN:
        default:
            sprintf(data_p, "unknown");
            break;
    }
    return TRUE;
}

/* FUNCTION NAME: SWDRV_LIB_MapSFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: fiber_type -- Fibre Channel Transmission Media (fiber type)
 *        length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapSFPEEPROMTransceiverMedia(UI8_T fiber_type, UI8_T length_km, char* data_p)
{
    char *temp = data_p;

    if (fiber_type == 0)
    {
        if (length_km)
        {
            temp += sprintf(temp, "Single Mode (SM), ");
        }
        else
        {
            temp += sprintf(temp, "Multimode Mode, ");
        }
    }
    else
    {
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_SINGLEMODE)
            temp += sprintf(temp, "Single Mode (SM), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_MULTIMODE_5)
            temp += sprintf(temp, "Multimode 50um (M5), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_MULTIMODE_6)
            temp += sprintf(temp, "Multimode 62.5um (M6), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_VIDEO_COAX)
            temp += sprintf(temp, "Video Coax (TV), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_MINIATURE_COAX)
            temp += sprintf(temp, "Miniature Coax (MI), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_TWISTED_PAIR)
            temp += sprintf(temp, "Twisted Pair (TP), ");
        if (fiber_type & SWDRV_TYPE_GBIC_FIBER_TYPE_TWIN_AXIAL_PAIR)
            temp += sprintf(temp, "Twin Axial Pair (TW), ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", fiber_type);

    return TRUE;
}

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
BOOL_T SWDRV_LIB_MapSFPEEPROMTransceiverCompCode(UI8_T eth_comp, UI8_T teng_eth_comp, UI8_T length_km, UI8_T length_100m, char* data_p)
{
    char *temp = data_p;

    /* for special case */
    if (eth_comp == 0 && teng_eth_comp == 0)
    {
        if (((length_km == 0x1e) || (length_km == 0x28)) && (length_100m == 0xff))
            temp += sprintf(temp, "1000BASE-LHX, ");
    }
    else
    {
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_1000BASE_SX)
            temp += sprintf(temp, "1000BASE-SX, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_1000BASE_LX)
        {
            /* This is a workaround for edgecore because transceiver vendor write 0x02 at byte#6 */
            if (((length_km == 0x46) || (length_km == 0x50)) && (length_100m == 0xff))
                temp += sprintf(temp, "1000BASE-ZX, ");
            else if (((length_km == 0x1e) || (length_km == 0x28)) && (length_100m == 0xff))
                temp += sprintf(temp, "1000BASE-LHX, ");
            else
                temp += sprintf(temp, "1000BASE-LX, ");
        }
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_1000BASE_CX)
            temp += sprintf(temp, "1000BASE-CX, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_1000BASE_T)
            temp += sprintf(temp, "1000BASE-T, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_100BASE_LX_LX10)
            temp += sprintf(temp, "100BASE-LX/LX10, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_100BASE_FX)
            temp += sprintf(temp, "100BASE-FX, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_BASE_BX10)
            temp += sprintf(temp, "BASE-BX10, ");
        if (eth_comp & SWDRV_TYPE_GBIC_ETH_COMP_CODES_BASE_PX)
            temp += sprintf(temp, "BASE-PX, ");

        if (teng_eth_comp & SWDRV_TYPE_GBIC_10G_ETH_COMP_CODES_10GBASE_SR)
            temp += sprintf(temp, "10GBASE-SR, ");
        if (teng_eth_comp & SWDRV_TYPE_GBIC_10G_ETH_COMP_CODES_10GBASE_LR)
            temp += sprintf(temp, "10GBASE-LR, ");
        if (teng_eth_comp & SWDRV_TYPE_GBIC_10G_ETH_COMP_CODES_10GBASE_LRM)
            temp += sprintf(temp, "10GBASE-LRM, ");
        if (teng_eth_comp & SWDRV_TYPE_GBIC_10G_ETH_COMP_CODES_10GBASE_ER)
            temp += sprintf(temp, "10GBASE-ER, ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", eth_comp);

    return TRUE;
}

/* FUNCTION NAME: SWDRV_LIB_MapXFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapXFPEEPROMTransceiverMedia(UI8_T length_km, char* data_p)
{
    char *temp = data_p;

    if (length_km)
    {
        temp += sprintf(temp, "Single Mode (SM), ");
    }
    else
    {
        temp += sprintf(temp, "Multimode Mode, ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", length_km);

    return TRUE;
}

/* FUNCTION NAME: SWDRV_LIB_MapXFPEEPROMTransceiverCompCode
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: teng_eth_comp -- 10G Ethernet Compliance Codes
 *        length_km -- Length (single mode)-km [Address A0h, Byte 14]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapXFPEEPROMTransceiverCompCode(UI8_T teng_eth_comp, UI8_T length_km, char* data_p)
{
    char *temp = data_p;

    /* for special case */
    if (teng_eth_comp == 0)
    {
        if (length_km == 80)
            temp += sprintf(temp, "1000BASE-ZR/ZW, ");
    }
    else
    {
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_SR)
            temp += sprintf(temp, "10GBASE-SR, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_LR)
            temp += sprintf(temp, "10GBASE-LR, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_ER)
            temp += sprintf(temp, "10GBASE-ER, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_LRM)
            temp += sprintf(temp, "10GBASE-LRM, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_SW)
            temp += sprintf(temp, "10GBASE-SW, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_LW)
            temp += sprintf(temp, "10GBASE-LW, ");
        if (teng_eth_comp & SWDRV_TYPE_XFP_10G_ETH_COMP_CODES_10GBASE_EW)
            temp += sprintf(temp, "10GBASE-EW, ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", teng_eth_comp);

    return TRUE;
}

/* FUNCTION NAME: SWDRV_LIB_MapQSFPEEPROMTransceiverMedia
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: fiber_type -- Fibre Channel Transmission Media (fiber type)
 *        length_km -- Length (single mode)-km [Page 00h Address 142]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapQSFPEEPROMTransceiverMedia(UI8_T fiber_type, UI8_T length_km, char* data_p)
{
    char *temp = data_p;

    if (fiber_type == 0)
    {
        if (length_km)
        {
            temp += sprintf(temp, "Single Mode (SM), ");
        }
        else
        {
            temp += sprintf(temp, "Multimode Mode, ");
        }
    }
    else
    {
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_SINGLEMODE)
            temp += sprintf(temp, "Single Mode (SM), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_MULTIMODE_3)
            temp += sprintf(temp, "Multimode 50um (OM3), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_MULTIMODE_5)
            temp += sprintf(temp, "Multimode 50m (M5), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_MULTIMODE_6)
            temp += sprintf(temp, "Multimode 62.5m (M6), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_VIDEO_COAX)
            temp += sprintf(temp, "Video Coax (TV), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_MINIATURE_COAX)
            temp += sprintf(temp, "Miniature Coax (MI), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_TWISTED_PAIR)
            temp += sprintf(temp, "Twisted Pair (TP), ");
        if (fiber_type & SWDRV_TYPE_QSFP_FIBER_TYPE_TWIN_AXIAL_PAIR)
            temp += sprintf(temp, "Twin Axial Pair (TW), ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", fiber_type);

    return TRUE;
}

/* FUNCTION NAME: SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode
 * PURPOSE: map transceiver media  swdrv constant to corresponding string
 * INPUT: eth_comp -- 10G Ethernet Compliance Codes
 *        length_km -- Length (single mode)-km [Page 00h Address 142]
 * OUTPUT: data_p -- corresponding string
 * RETURN: TRUE -- successfully
 * NOTES:
 */
BOOL_T SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode(UI8_T eth_comp, UI8_T length_km, char* data_p)
{
    char *temp = data_p;

    /* for special case */
    if (eth_comp != 0)
    {
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_10GBASE_LRM)
            temp += sprintf(temp, "10GBASE-LRM, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_10GBASE_LR)
            temp += sprintf(temp, "10GBASE-LR, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_10GBASE_SR)
            temp += sprintf(temp, "10GBASE-SR, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_40GBASE_CR4)
            temp += sprintf(temp, "40GBASE-CR4, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_40GBASE_SR4)
            temp += sprintf(temp, "40GBASE-SR4, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_40GBASE_LR4)
            temp += sprintf(temp, "40GBASE-LR4, ");
        if (eth_comp & SWDRV_TYPE_QSFP_40G_ETH_COMP_CODES_40G_ACTIVE_CABLE)
            temp += sprintf(temp, "40G Active Cable (XLPPI), ");
    }

    if (temp != data_p)
        *(temp - 2) = 0;
    else
        sprintf( data_p, "[0x%02x]", eth_comp);

    return TRUE;
}

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
BOOL_T SWDRV_LIB_MapSpeedDuplex(UI32_T speed_duplex, UI32_T buff_len, char* buff)
{
    const char *map_str_p;

    switch (speed_duplex)
    {
        case VAL_portSpeedDpxCfg_halfDuplex10:
            map_str_p = "10half";
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10:
            map_str_p = "10full";
            break;

        case VAL_portSpeedDpxCfg_halfDuplex100:
            map_str_p = "100half";
            break;

        case VAL_portSpeedDpxCfg_fullDuplex100:
            map_str_p = "100full";
            break;

        case VAL_portSpeedDpxCfg_halfDuplex1000:
            map_str_p = "1000half";
            break;

        case VAL_portSpeedDpxCfg_fullDuplex1000:
            map_str_p = "1000full";
            break;

        case VAL_portSpeedDpxCfg_halfDuplex10g:
            map_str_p = "10Ghalf";
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10g:
            map_str_p = "10Gfull";
            break;

        case VAL_portSpeedDpxCfg_fullDuplex25g:
            map_str_p = "25Gfull";
            break;
        case VAL_portSpeedDpxCfg_halfDuplex40g:
            map_str_p = "40Ghalf";
            break;
        case VAL_portSpeedDpxCfg_fullDuplex40g:
            map_str_p = "40Gfull";
            break;
        case VAL_portSpeedDpxCfg_fullDuplex100g:
            map_str_p = "100Gfull";
            break;

        default:
            return FALSE;
            break;
    }
    snprintf(buff, buff_len, "%s", map_str_p);
    return TRUE;
}

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
BOOL_T SWDRV_LIB_MapPortType(UI32_T port_type, UI32_T buff_len, char* buff)
{
    const char *map_str_p;

    switch (port_type)
    {
        case VAL_portType_other:
            map_str_p = "Other";
            break;
        case VAL_portType_hundredBaseTX:
            map_str_p = "100BASE-TX";
            break;            
        case VAL_portType_hundredBaseFX:
            map_str_p = "100BASE-FX";
            break;            
        case VAL_portType_thousandBaseSX:
            map_str_p = "1000BASE-SX";
            break;
        case VAL_portType_thousandBaseLX:
            map_str_p = "1000BASE-LX";
            break;
        case VAL_portType_thousandBaseT:
            map_str_p = "1000BASE-T";
            break;
        case VAL_portType_thousandBaseGBIC:
            map_str_p = "1000 GBIC";
            break;
        case VAL_portType_thousandBaseSfp:
            map_str_p = "1000BASE SFP";
            break;
        case VAL_portType_tenG:
            map_str_p = "10GBASE";
            break;
        case VAL_portType_tenGBaseT:
            map_str_p = "10GBASE-T";
            break;
        case VAL_portType_tenGBaseXFP:
            map_str_p = "10GBASE XFP";
            break;
        case VAL_portType_tenGBaseSFP:
            map_str_p = "10GBASE SFP+";
            break;
        case VAL_portType_twentyFiveGBaseSFP:
            map_str_p = "25GBASE SFP+";
            break;
        case VAL_portType_fortyGBaseQSFP:
            map_str_p = "40GBASE QSFP";
            break;
        case VAL_portType_hundredGBaseQSFP:
            map_str_p = "100GBASEQSFP";
            break;
        case VAL_portType_hundredBaseFxScSingleMode:
            map_str_p = "FxSc single";
            break;
        case VAL_portType_hundredBaseFxScMultiMode:
            map_str_p = "FxSc multi";
            break;
        default:
            return FALSE;
            break;
    }
    snprintf(buff, buff_len, "%s", map_str_p);
    return TRUE;
}

#if (SYS_CPNT_SFP_INFO_FOR_BROCADE == TRUE)
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
BOOL_T SWDRV_LIB_GetCustomizedSfpTransceiverTypeStr(UI32_T unit, UI32_T sfp_index, UI32_T buf_size, char *buf)
{
    static struct {
        UI8_T sonet_compliance_code_b1; /* Address A0h, Byte 5 */
        UI8_T eth_compliance_code;      /* Address A0h, Byte 6 */
        UI16_T wavelength;              /* Address A0h, Byte 60-61 */
        UI8_T link_length_km;           /* Address A0h, Byte 14 */
        char *transceiver_type_str;
    }
    brocade_sfp_table[] =
    {
        /* 33003-000 */     { 0x00, 0x10, 0x060e, 0x50, "1000BASE-LHA" },
        /* 33004-000 */     { 0x00, 0x20, 0x060e, 0x50, "1000BASE-LHB" },
        /* 00600-291 */     { 0x00, 0x02, 0x05be, 0x50, "1000BASE-CWDM" },
        /* 00600-292 */     { 0x00, 0x02, 0x05d2, 0x50, "1000BASE-CWDM" },
        /* 00600-293 */     { 0x00, 0x02, 0x05e6, 0x50, "1000BASE-CWDM" },
        /* 00600-294 */     { 0x00, 0x02, 0x05fa, 0x50, "1000BASE-CWDM" },
        /* 00600-295 */     { 0x00, 0x02, 0x060e, 0x50, "1000BASE-CWDM" },
        /* 00600-296 */     { 0x00, 0x02, 0x0622, 0x50, "1000BASE-CWDM" },
        /* 00600-297 */     { 0x00, 0x02, 0x0636, 0x50, "1000BASE-CWDM" },
        /* 00600-298 */     { 0x00, 0x02, 0x064a, 0x50, "1000BASE-CWDM" },
        /* 33005-000 */     { 0x00, 0x42, 0x05d2, 0x0a, "1000BASE-BXD" },
        /* 33006-000 */     { 0x00, 0x42, 0x051e, 0x0a, "1000BASE-BXU" },
        /* 33225-100 */     { 0x02, 0x00, 0x051e, 0x0f, "100BASE-FX-IR" },
        /* 33226-100 */     { 0x04, 0x00, 0x051e, 0x28, "100BASE-FX-LR" },
        /* 57-0000194-01 */ { 0x00, 0x10, 0x060e, 0x58, "1000BASE-LHA" },
    };

    SWCTRL_OM_SfpInfo_T sfp_info;
    UI8_T i;

    if(!SWCTRL_GetPortSfpInfo(unit, sfp_index, &sfp_info))
    {
        return FALSE;
    }

    if(strncmp(sfp_info.vendor_name, "BROCADE", sizeof("BROCADE")-1) != 0)
    {
        return FALSE;
    }

    for(i = 0; i < sizeof(brocade_sfp_table)/sizeof(*brocade_sfp_table); i++)
    {
        if (sfp_info.transceiver[2] == brocade_sfp_table[i].sonet_compliance_code_b1 &&
            sfp_info.transceiver[3] == brocade_sfp_table[i].eth_compliance_code &&
            sfp_info.wavelength == brocade_sfp_table[i].wavelength &&
            sfp_info.link_length_support_km == brocade_sfp_table[i].link_length_km)
        {
            strncpy(buf, brocade_sfp_table[i].transceiver_type_str, buf_size);
            buf[buf_size-1] = 0;
            return TRUE;
        }
    }

    return FALSE;
}/* End of SWDRV_LIB_GetSfpTransceiverTypeStr */
#endif /* (SYS_CPNT_SFP_INFO_FOR_BROCADE == TRUE) */

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
BOOL_T SWDRV_LIB_ChangeSfpDdmThresholdToInteger(SWCTRL_OM_SfpDdmThreshold_T *threshold_flaot_p, SWCTRL_OM_SfpDdmThresholdInteger_T *threshold_int_p)
{
    if(threshold_flaot_p == NULL || threshold_int_p == NULL)
        return FALSE;

    SWDRV_LIB_FloatToInt(threshold_flaot_p->temp_high_alarm*100, &threshold_int_p->temp_high_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->temp_low_alarm*100, &threshold_int_p->temp_low_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->temp_high_warning*100, &threshold_int_p->temp_high_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->temp_low_warning*100, &threshold_int_p->temp_low_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->voltage_high_alarm*100, (I16_T *)&threshold_int_p->voltage_high_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->voltage_low_alarm*100, (I16_T *)&threshold_int_p->voltage_low_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->voltage_high_warning*100, (I16_T *)&threshold_int_p->voltage_high_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->voltage_low_warning*100, (I16_T *)&threshold_int_p->voltage_low_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->bias_high_alarm*100, (I16_T *)&threshold_int_p->bias_high_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->bias_low_alarm*100, (I16_T *)&threshold_int_p->bias_low_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->bias_high_warning*100, (I16_T *)&threshold_int_p->bias_high_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->bias_low_warning*100, (I16_T *)&threshold_int_p->bias_low_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->tx_power_high_alarm*100, &threshold_int_p->tx_power_high_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->tx_power_low_alarm*100, &threshold_int_p->tx_power_low_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->tx_power_high_warning*100, &threshold_int_p->tx_power_high_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->tx_power_low_warning*100, &threshold_int_p->tx_power_low_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->rx_power_high_alarm*100, &threshold_int_p->rx_power_high_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->rx_power_low_alarm*100, &threshold_int_p->rx_power_low_alarm);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->rx_power_high_warning*100, &threshold_int_p->rx_power_high_warning);
    SWDRV_LIB_FloatToInt(threshold_flaot_p->rx_power_low_warning*100, &threshold_int_p->rx_power_low_warning);
    //printf("%s-%d temp_high_alarm:%f temp_high_alarm*100:%f temp_high_alarm:%d\r\n", __FUNCTION__, __LINE__, threshold_flaot_p->temp_high_alarm, threshold_flaot_p->temp_high_alarm*100, threshold_int_p->temp_high_alarm);

    return TRUE;
}

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
BOOL_T SWDRV_LIB_CalcSFPChecksum(UI8_T cc_type, UI8_T *info_p, UI8_T *calc_cksum_p, UI8_T *real_cksum_p)
{
    UI8_T i = 0, start = 0, end = 0;

    if(info_p == NULL || calc_cksum_p == NULL || real_cksum_p == NULL)
    {
        BACKDOOR_MGR_Printf("%s-%d NULL pointer\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(cc_type == SWDRV_TYPE_SFP_CHECKSUM_CC_BASE)
    {
        start = 0;
        end = 63;
    }
    else if (cc_type == SWDRV_TYPE_SFP_CHECKSUM_CC_EXT)
    {
        start = 64;
        end = 95;
    }
    else
    {
        return FALSE;
    }

    (*calc_cksum_p) = 0;
    for(i = start; i < end; i++)
    {
        (*calc_cksum_p) += info_p[i];
    }
    (*calc_cksum_p) &= 0xff;
    (*real_cksum_p) = info_p[end];

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_LIB_FloatToInt
 * -------------------------------------------------------------------------
 * FUNCTION: change from float to int
 * INPUT   : float_val
 * OUTPUT  : val_p
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_LIB_FloatToInt(float float_val, I16_T *val_p)
{
    double int_part, fract_part;

    if(val_p == NULL)
        return;

    fract_part = modf(float_val, &int_part);

    if(float_val > 0)
    {
        if(fract_part > 0.99)
            *val_p = ceil(float_val);
        else
            *val_p = floor(float_val);
    }
    else
    {
        if(fract_part < -0.99)
            *val_p = floor(float_val);
        else
            *val_p = ceil(float_val);
    }
}

