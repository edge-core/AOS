#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl.h"
#include "swdrv.h"
#include "swdrv_lib.h"
#include "swdrv_om.h"


static BOOL_T CGI_MODULE_SFP_DMM_GetOneInterface(UI32_T unit, UI32_T port, json_t *sfp_obj_p);

/**----------------------------------------------------------------------
 * This API is used to read all SFP DMM modules.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SFP_DMM_Interfaces_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *intfs_p = json_array();
    UI32_T unit = 0, max_port_num = 0, port = 0, sfp_index = 0;

    json_object_set_new(result_p, "interfaces", intfs_p);

    STKTPLG_POM_GetNextUnit(&unit);
    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);

    for (port = 1; port <= max_port_num; port++)
    {
        json_t *intf_obj_p = json_object();

        /* check in CGI_MODULE_SFP_DMM_GetOneInterface
        if (FALSE == STKTPLG_OM_UserPortToSfpIndex(unit, port, &sfp_index))
        {
            continue;
        }
        */

        if (TRUE == CGI_MODULE_SFP_DMM_GetOneInterface(unit, port, intf_obj_p))
        {
            json_array_append_new(intfs_p, intf_obj_p);
        }
    } //loop port

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_SFP_DMM_Interfaces_Read

/**----------------------------------------------------------------------
 * This API is used to read a SFP DMM.
 *
 * @param id (required, string) Unique port ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SFP_DMM_Interfaces_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char *id_str_p;
    UI32_T  unit = 0, port = 0;
    UI8_T  port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }

    id_str_p = json_string_value(id_p);
    CGI_UTIL_UrlDecode(port_ar, id_str_p);

    if (TRUE != CGI_UTIL_InterfaceIdToEth((const char *) port_ar, &unit, &port))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Interface ID.");
    }

    if (TRUE == CGI_MODULE_SFP_DMM_GetOneInterface(unit, port, result_p))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No SFP info of the port.");
    }
} //CGI_MODULE_SFP_DMM_Interfaces_ID_Read


void CGI_MODULE_SFP_RegisterHandlers()
{
    {
        CGI_API_HANDLER_SET_T handlers = {0};

        handlers.read_handler = CGI_MODULE_SFP_DMM_Interfaces_Read;
        CGI_MAIN_Register("/api/v1/sfp/dmm/interfaces", &handlers, 0);
    }

    {   /* for "/api/v1/sfp/dmm/interfaces/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_SFP_DMM_Interfaces_ID_Read;
        CGI_MAIN_Register("/api/v1/sfp/dmm/interfaces/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_SFP_Init()
{
    CGI_MODULE_SFP_RegisterHandlers();
}

static BOOL_T CGI_MODULE_SFP_DMM_GetOneInterface(UI32_T unit, UI32_T port, json_t *sfp_obj_p)
{
    SWCTRL_OM_SfpInfo_T sfp_info;
    SWCTRL_OM_SfpDdmInfoMeasured_T sfp_ddm_info_measured;
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    SWCTRL_OM_SfpDdmThresholdEntry_T sfp_ddm_threshold_entry;
#endif
    char id_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    char connector_ar[CGI_MAX_VARIABLE_STR] = {0};
    char media_ar[CGI_MAX_VARIABLE_STR] = {0};
    char eth_ar[CGI_MAX_VARIABLE_STR] = {0};
    char oui_ar[10] = {0};
    char date_ar[10] = {0};
    UI32_T sfp_index = 0, lport = 0;
    UI8_T  date_code_year = 0, date_code_month = 0, date_code_day = 0;
    BOOL_T is_present = FALSE;

    if (FALSE == STKTPLG_OM_UserPortToSfpIndex(unit, port, &sfp_index))
    {
        return FALSE;
    }

    memset(&sfp_info, 0, sizeof(sfp_info));
    memset(&sfp_ddm_info_measured, 0, sizeof(sfp_ddm_info_measured));
    memset(&sfp_ddm_threshold_entry, 0, sizeof(sfp_ddm_threshold_entry));
    SWCTRL_POM_UserPortToIfindex(unit, port, &lport);

    if ((FALSE == SWCTRL_POM_GetPortSfpPresent(unit, sfp_index, &is_present))
            || (FALSE == is_present)
            || (FALSE == SWCTRL_POM_GetPortSfpInfo(unit, sfp_index, &sfp_info)))
    {
        return FALSE;
    }

    if (TRUE == sfp_info.is_invalid)
    {
        return FALSE;
    }

    sprintf(id_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);

    SWDRV_LIB_MapSFPEEPROMConnector(sfp_info.connector, connector_ar);

    switch (sfp_info.identifier)
    {
        case SWDRV_TYPE_GBIC_ID_SFP:
            SWDRV_LIB_MapSFPEEPROMTransceiverMedia(
                    sfp_info.transceiver[6], sfp_info.link_length_support_km, media_ar);
            SWDRV_LIB_MapSFPEEPROMTransceiverCompCode(sfp_info.transceiver[3],
                                                      sfp_info.transceiver[0],
                                                      sfp_info.link_length_support_km,
                                                      sfp_info.link_length_support_100m, eth_ar);
#if (SYS_CPNT_SFP_INFO_FOR_BROCADE == TRUE)
            SWDRV_LIB_GetCustomizedSfpTransceiverTypeStr(unit, sfp_index, sizeof(eth_ar), eth_ar);
#endif
            break;

        case SWDRV_TYPE_GBIC_ID_XFP:
            SWDRV_LIB_MapXFPEEPROMTransceiverMedia(sfp_info.link_length_support_km, media_ar);
            SWDRV_LIB_MapXFPEEPROMTransceiverCompCode(sfp_info.transceiver[0],
                                                    sfp_info.link_length_support_km, eth_ar);
            break;

        case SWDRV_TYPE_GBIC_ID_QSFP:
        case SWDRV_TYPE_GBIC_ID_QSFP_PLUS:
            SWDRV_LIB_MapQSFPEEPROMTransceiverMedia(
                    sfp_info.transceiver[6], sfp_info.link_length_support_km, media_ar);
            SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode(sfp_info.transceiver[0],
                                                   sfp_info.link_length_support_km, eth_ar);
            break;

        default:
            sprintf(media_ar, "%s", "unknown");
            sprintf(eth_ar, "%s", "unknown");
    }

    sprintf(oui_ar, "%02X-%02X-%02X", sfp_info.vendor_oui[0], sfp_info.vendor_oui[1], sfp_info.vendor_oui[2]);
    date_code_year  = (UI32_T) ((sfp_info.date_code[0]-'0')*10 + (sfp_info.date_code[1]-'0'));
    date_code_month = (UI32_T) ((sfp_info.date_code[2]-'0')*10 + (sfp_info.date_code[3]-'0'));
    date_code_day   = (UI32_T) ((sfp_info.date_code[4]-'0')*10 + (sfp_info.date_code[5]-'0'));
    sprintf(date_ar, "%02d-%02d-%02d", date_code_year, date_code_month, date_code_day);

    json_object_set_new(sfp_obj_p, "id", json_string(id_ar));
    json_object_set_new(sfp_obj_p, "connectorType", json_string(connector_ar));
    json_object_set_new(sfp_obj_p, "fiberType", json_string(media_ar));
    json_object_set_new(sfp_obj_p, "ethCompliance", json_string(eth_ar));
    json_object_set_new(sfp_obj_p, "baudRate", json_integer(sfp_info.bitrate * 100));
    json_object_set_new(sfp_obj_p, "vendorOui", json_string(oui_ar));
    json_object_set_new(sfp_obj_p, "vendorName", json_string(sfp_info.vendor_name));
    json_object_set_new(sfp_obj_p, "vendorPn", json_string(sfp_info.vendor_pn));
    json_object_set_new(sfp_obj_p, "vendorRev", json_string(sfp_info.vendor_rev));
    json_object_set_new(sfp_obj_p, "vendorSn", json_string(sfp_info.vendor_sn));
    json_object_set_new(sfp_obj_p, "dateCode", json_string(date_ar));
    json_object_set_new(sfp_obj_p, "txDisableCap", json_boolean(TRUE == sfp_info.support_tx_disable));

    if ((TRUE == sfp_info.support_ddm)
            && (TRUE == SWCTRL_POM_GetPortSfpDdmInfoMeasured(unit, sfp_index, &sfp_ddm_info_measured)))
    {
        json_t *dmm_obj_p = json_object();

        json_object_set_new(dmm_obj_p, "temperature", json_real((double) sfp_ddm_info_measured.temperature));
        json_object_set_new(dmm_obj_p, "vcc", json_real((double) sfp_ddm_info_measured.voltage));

        if ((SWDRV_TYPE_GBIC_ID_SFP == sfp_info.identifier) || (SWDRV_TYPE_GBIC_ID_XFP == sfp_info.identifier))
        {
            json_object_set_new(dmm_obj_p, "biasCh1", json_real((double) sfp_ddm_info_measured.tx_bias_current));
            json_object_set_new(dmm_obj_p, "biasCh2", json_integer(0));
            json_object_set_new(dmm_obj_p, "biasCh3", json_integer(0));
            json_object_set_new(dmm_obj_p, "biasCh4", json_integer(0));
            json_object_set_new(dmm_obj_p, "txPower", json_real((double) sfp_ddm_info_measured.tx_power));
            json_object_set_new(dmm_obj_p, "rxPowerCh1", json_real((double) sfp_ddm_info_measured.rx_power));
            json_object_set_new(dmm_obj_p, "rxPowerCh2", json_integer(0));
            json_object_set_new(dmm_obj_p, "rxPowerCh3", json_integer(0));
            json_object_set_new(dmm_obj_p, "rxPowerCh4", json_integer(0));
        }
        else if ((SWDRV_TYPE_GBIC_ID_QSFP == sfp_info.identifier) || (SWDRV_TYPE_GBIC_ID_QSFP_PLUS == sfp_info.identifier))
        {
            json_object_set_new(dmm_obj_p, "biasCh1", json_real((double) sfp_ddm_info_measured.tx_bias_current));
            json_object_set_new(dmm_obj_p, "biasCh2", json_real((double) sfp_ddm_info_measured.tx_bias_current_2));
            json_object_set_new(dmm_obj_p, "biasCh3", json_real((double) sfp_ddm_info_measured.tx_bias_current_3));
            json_object_set_new(dmm_obj_p, "biasCh4", json_real((double) sfp_ddm_info_measured.tx_bias_current_4));
            json_object_set_new(dmm_obj_p, "rxPowerCh1", json_real((double) sfp_ddm_info_measured.rx_power));
            json_object_set_new(dmm_obj_p, "rxPowerCh2", json_real((double) sfp_ddm_info_measured.rx_power_2));
            json_object_set_new(dmm_obj_p, "rxPowerCh3", json_real((double) sfp_ddm_info_measured.rx_power_3));
            json_object_set_new(dmm_obj_p, "rxPowerCh4", json_real((double) sfp_ddm_info_measured.rx_power_4));
        }

        json_object_set_new(sfp_obj_p, "dmm", dmm_obj_p);
    } //if (TRUE == sfp_info.support_ddm)

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    if (TRUE == SWCTRL_POM_GetPortSfpDdmThresholdEntry(lport, &sfp_ddm_threshold_entry))
    {
        json_t *thresh_obj_p = json_object();
        json_t *temp_obj_p = json_object();
        json_t *volt_obj_p = json_object();
        json_t *current_obj_p = json_object();
        json_t *txpower_obj_p = json_object();
        json_t *rxpower_obj_p = json_object();

        json_object_set_new(thresh_obj_p, "monitor", json_boolean(TRUE == sfp_ddm_threshold_entry.trap_enable));
        json_object_set_new(thresh_obj_p, "auto", json_boolean(TRUE == sfp_ddm_threshold_entry.auto_mode));

        json_object_set_new(temp_obj_p, "lowAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.temp_low_alarm));
        json_object_set_new(temp_obj_p, "lowWarning", json_real((double) sfp_ddm_threshold_entry.threshold.temp_low_warning));
        json_object_set_new(temp_obj_p, "highWarning", json_real((double) sfp_ddm_threshold_entry.threshold.temp_high_warning));
        json_object_set_new(temp_obj_p, "highAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.temp_high_alarm));
        json_object_set_new(thresh_obj_p, "temperature", temp_obj_p);

        json_object_set_new(volt_obj_p, "lowAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.voltage_low_alarm));
        json_object_set_new(volt_obj_p, "lowWarning", json_real((double) sfp_ddm_threshold_entry.threshold.voltage_low_warning));
        json_object_set_new(volt_obj_p, "highWarning", json_real((double) sfp_ddm_threshold_entry.threshold.voltage_high_warning));
        json_object_set_new(volt_obj_p, "highAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.voltage_high_alarm));
        json_object_set_new(thresh_obj_p, "voltage", volt_obj_p);

        json_object_set_new(current_obj_p, "lowAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.bias_low_alarm));
        json_object_set_new(current_obj_p, "lowWarning", json_real((double) sfp_ddm_threshold_entry.threshold.bias_low_warning));
        json_object_set_new(current_obj_p, "highWarning", json_real((double) sfp_ddm_threshold_entry.threshold.bias_high_warning));
        json_object_set_new(current_obj_p, "highAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.bias_high_alarm));
        json_object_set_new(thresh_obj_p, "current", current_obj_p);

        if ((SWDRV_TYPE_GBIC_ID_SFP == sfp_info.identifier) || (SWDRV_TYPE_GBIC_ID_XFP == sfp_info.identifier))
        {
            json_object_set_new(txpower_obj_p, "lowAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.tx_power_low_alarm));
            json_object_set_new(txpower_obj_p, "lowWarning", json_real((double) sfp_ddm_threshold_entry.threshold.tx_power_low_warning));
            json_object_set_new(txpower_obj_p, "highWarning", json_real((double) sfp_ddm_threshold_entry.threshold.tx_power_high_warning));
            json_object_set_new(txpower_obj_p, "highAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.tx_power_high_alarm));
            json_object_set_new(thresh_obj_p, "txPower", txpower_obj_p);
        }

        json_object_set_new(rxpower_obj_p, "lowAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.rx_power_low_alarm));
        json_object_set_new(rxpower_obj_p, "lowWarning", json_real((double) sfp_ddm_threshold_entry.threshold.rx_power_low_warning));
        json_object_set_new(rxpower_obj_p, "highWarning", json_real((double) sfp_ddm_threshold_entry.threshold.rx_power_high_warning));
        json_object_set_new(rxpower_obj_p, "highAlarm", json_real((double) sfp_ddm_threshold_entry.threshold.rx_power_high_alarm));
        json_object_set_new(thresh_obj_p, "rxPower", rxpower_obj_p);

        json_object_set_new(sfp_obj_p, "thresholds", thresh_obj_p);
    } //thresholds
#endif  /* #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */

    return TRUE;
} //CGI_MODULE_SFP_DMM_GetOneInterface
