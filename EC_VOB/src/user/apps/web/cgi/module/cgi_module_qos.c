
#include "l4_pmgr.h"

enum CGI_MODULE_QOS_ERROR_CODE_E
{
    CGI_MODULE_QOS_ERROR_NONE = 0,
    CGI_MODULE_QOS_ERROR_CALSS_EXCEED_ELEMENTS,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_TYPE,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_ACL,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_COS,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPPRECEDENCE,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPDSCP,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_VLAN,
    CGI_MODULE_QOS_ERROR_CALSS_INVALID_SRCPORT,
    CGI_MODULE_QOS_ERROR_CALSS_NOT_ALLOW_MIX_SELECTOR,
    CGI_MODULE_QOS_ERROR_CALSS_FREE_SELECTOR_UNAVAILABLE,
    CGI_MODULE_QOS_ERROR_CALSS_FAIL,
    CGI_MODULE_QOS_ERROR_POLICY_INVALID_CLASS,
    CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION,
    CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION_VAL,
    CGI_MODULE_QOS_ERROR_POLICY_INVALID_PHB,
    CGI_MODULE_QOS_ERROR_POLICY_MISS_METER,
    CGI_MODULE_QOS_ERROR_POLICY_METER_MODEL,
    CGI_MODULE_QOS_ERROR_POLICY_METER_FIELD,
    CGI_MODULE_QOS_ERROR_POLICY_METER_RATE,
    CGI_MODULE_QOS_ERROR_POLICY_METER_BURST,
    CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_RATE,
    CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_BURST,
    CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_SRTCM_FIELD,
    CGI_MODULE_QOS_ERROR_POLICY_METER_SRTCM_UNSUPPORT_FIELD,
    CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_TRTCM_FIELD,
    CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_EXCEED_ACT,
    CGI_MODULE_QOS_ERROR_POLICY_METER_REMARK_DSCP,
    CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS,
    CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_ACTION,
    CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_METER,
    CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS,
    CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_ACTION,
    CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_METER
};

static BOOL_T CGI_MODULE_QOS_GetOneClassMap(UI32_T cmap_id, RULE_TYPE_UI_ClassMap_T *cmap_p, json_t *cmap_obj_p);
static BOOL_T CGI_MODULE_QOS_GetOnePolicyMap(UI32_T pmap_id, RULE_TYPE_UI_PolicyMap_T *pmap_p, json_t *pmap_obj_p);
static int CGI_MODULE_QOS_ValidClassMapElements(json_t *elements_p);
static int CGI_MODULE_QOS_ValidPolicyMapElements(json_t *elements_p);
static int CGI_MODULE_QOS_SetClassMapElements(const char *name_p, json_t *elements_p, BOOL_T add);
static int CGI_MODULE_QOS_SetPolicyMapElements(const char *name_p, json_t *elements_p, BOOL_T add);
static BOOL_T CGI_MODULE_QOS_ValidPolicyMapName(const char *pname_p);
static CGI_STATUS_CODE_T CGI_MODULE_QOS_ReturnErrorMsg(HTTP_Response_T *http_response, int error_code);

/**----------------------------------------------------------------------
 * This API is used to get class-map info.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_CLASSMAP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *cmaps_p = json_array();
    RULE_TYPE_UI_ClassMap_T cmap;
    UI32_T cmap_id = 0;

    memset(&cmap, 0, sizeof(cmap));
    json_object_set_new(result_p, "classMaps", cmaps_p);

    while (RULE_TYPE_OK == L4_PMGR_QoS_GetNextClassMap(&cmap_id, &cmap)
            && (cmap.row_status == VAL_diffServClassMapStatus_active))
    {
        json_t *cmap_obj_p = json_object();

        if (TRUE == CGI_MODULE_QOS_GetOneClassMap(cmap_id, &cmap, cmap_obj_p))
        {
            json_array_append_new(cmaps_p, cmap_obj_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_CLASSMAP_Read

/**----------------------------------------------------------------------
 * This API is used to create a class-map.
 *
 * @param name (required, string) class-map name
 * @param description (required, string) description
 * @param elements (required, array) class-map match
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_CLASSMAP_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    json_t  *desc_p = CGI_REQUEST_GetBodyValue(http_request, "description");
    json_t  *elements_p = CGI_REQUEST_GetBodyValue(http_request, "elements");
    const char *name_str_p;
    const char *desc_str_p;
    RULE_TYPE_UI_ClassMap_T cmap;
    int ret = 0;

    memset(&cmap, 0, sizeof(cmap));

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_ValidClassMapElements(elements_p);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    name_str_p = json_string_value(name_p);
    memcpy(cmap.class_map_name, name_str_p, sizeof(cmap.class_map_name)-1);
    cmap.class_map_name[sizeof(cmap.class_map_name)-1] = '\0';

    if (RULE_TYPE_OK != L4_PMGR_QoS_GetClassMapByName(&cmap))
    {
        if (RULE_TYPE_OK != L4_PMGR_QoS_CreateClassMapByName(name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.CreateClassMapByNameError", "Failed.");
        }
    }

    if (NULL != desc_p)
    {
        desc_str_p = json_string_value(desc_p);

        if (RULE_TYPE_OK != L4_PMGR_QoS_GetClassMapByName(&cmap))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.GetClassMapByNameError", "Failed.");
        }

        if (0 != strcmp(cmap.desc, desc_str_p))
        {
            strncpy(cmap.desc, desc_str_p, sizeof(cmap.desc)-1);
            cmap.desc[sizeof(cmap.desc)-1] = '\0';

            if (RULE_TYPE_OK != L4_PMGR_QoS_SetClassMap(&cmap))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetClassMapDescError", "Failed.");
            }
        }
    } //if (NULL != desc_p)

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_SetClassMapElements(name_str_p, elements_p, TRUE);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_CLASSMAP_Create

/**----------------------------------------------------------------------
 * This API is used to get specified class-map info.
 *
 * @param name (required, string) class-map name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_CLASSMAP_NAME_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    const char *name_str_p;
    RULE_TYPE_UI_ClassMap_T cmap;
    UI32_T cmap_id = 0;

    memset(&cmap, 0, sizeof(cmap));

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    name_str_p = json_string_value(name_p);
    strcpy(cmap.class_map_name, name_str_p);

    if (RULE_TYPE_OK == L4_PMGR_QoS_GetClassMapByName(&cmap))
    {
        if (RULE_TYPE_OK == L4_PMGR_QoS_GetClassMapIdByName(name_str_p, &cmap_id))
        {
            CGI_MODULE_QOS_GetOneClassMap(cmap_id, &cmap, result_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_CLASSMAP_NAME_Read

/**----------------------------------------------------------------------
 * This API is used to update a class-map.
 *
 * @param name (required, string) class-map name
 * @param status (required, boolean) status
 * @param description (required, string) description
 * @param match (required, array) class-map match
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_CLASSMAP_NAME_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    json_t  *status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    json_t  *desc_p = CGI_REQUEST_GetBodyValue(http_request, "description");
    json_t  *elements_p = CGI_REQUEST_GetBodyValue(http_request, "elements");
    const char *name_str_p;
    const char *desc_str_p;
    RULE_TYPE_UI_ClassMap_T cmap;
    BOOL_T status = FALSE;
    int ret = 0;

    memset(&cmap, 0, sizeof(cmap));

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_ValidClassMapElements(elements_p);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    name_str_p = json_string_value(name_p);
    memcpy(cmap.class_map_name, name_str_p, sizeof(cmap.class_map_name)-1);
    cmap.class_map_name[sizeof(cmap.class_map_name)-1] = '\0';

    if (RULE_TYPE_OK != L4_PMGR_QoS_GetClassMapByName(&cmap))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid class-map name.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        status = TRUE;
    }

    if (NULL != desc_p)
    {
        desc_str_p = json_string_value(desc_p);

        if (TRUE == status)
        {
            if (0 != strcmp(cmap.desc, desc_str_p))
            {
                strncpy(cmap.desc, desc_str_p, sizeof(cmap.desc)-1);
                cmap.desc[sizeof(cmap.desc)-1] = '\0';
            }
        }
        else
        {
            cmap.desc[0] = '\0';
        }

        if (RULE_TYPE_OK != L4_PMGR_QoS_SetClassMap(&cmap))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetClassMapDescError", "Failed.");
        }
    } //if (NULL != desc_p)

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_SetClassMapElements(name_str_p, elements_p, status);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_CLASSMAP_NAME_Update

/**----------------------------------------------------------------------
 * This API is used to delete specified class-map info.
 *
 * @param name (required, string) class-map name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_CLASSMAP_NAME_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    UI32_T  ret = RULE_TYPE_OK;

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    ret = L4_PMGR_QoS_DelClassMap(json_string_value(name_p));

    if ((RULE_TYPE_OK != ret) && (RULE_TYPE_CLASS_MAP_NONEXISTED != ret))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.DelClassMapError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_CLASSMAP_NAME_Delete

/**----------------------------------------------------------------------
 * This API is used to get policy-map info.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_POLICYMAP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pmaps_p = json_array();
    RULE_TYPE_UI_PolicyMap_T pmap;
    UI32_T pmap_id = 0;

    memset(&pmap, 0, sizeof(pmap));
    json_object_set_new(result_p, "policyMaps", pmaps_p);

    while (RULE_TYPE_OK == L4_PMGR_QoS_GetNextPolicyMap(&pmap_id, &pmap)
            && (pmap.row_status == VAL_diffServPolicyMapStatus_active))
    {
        json_t *pmap_obj_p = json_object();

        if (TRUE == CGI_MODULE_QOS_GetOnePolicyMap(pmap_id, &pmap, pmap_obj_p))
        {
            json_array_append_new(pmaps_p, pmap_obj_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_POLICYMAP_Read

/**----------------------------------------------------------------------
 * This API is used to create a policy-map.
 *
 * @param name (required, string) policy-map name
 * @param description (required, string) description
 * @param elements (required, array) policy-map elements
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_POLICYMAP_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    json_t  *desc_p = CGI_REQUEST_GetBodyValue(http_request, "description");
    json_t  *elements_p = CGI_REQUEST_GetBodyValue(http_request, "elements");
    const char *name_str_p;
    UI32_T rule_ret = RULE_TYPE_OK;
    int ret = 0;

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_ValidPolicyMapElements(elements_p);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    name_str_p = json_string_value(name_p);
    rule_ret = L4_PMGR_QoS_CreatePolicyMapByName(name_str_p);

    if ((RULE_TYPE_OK != rule_ret) && (RULE_TYPE_POLICY_MAP_EXISTED != rule_ret))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.CreatePolicyMapByNameError", "Failed.");
    }

    if (NULL != desc_p)
    {
        if (RULE_TYPE_OK != L4_PMGR_QoS_SetPolicyMapDescByName(name_str_p, json_string_value(desc_p)))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetPolicyMapDescByNameError", "Failed.");
        }
    } //if (NULL != desc_p)

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_SetPolicyMapElements(name_str_p, elements_p, TRUE);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_POLICYMAP_Create

/**----------------------------------------------------------------------
 * This API is used to get specified policy-map info.
 *
 * @param name (required, string) policy-map name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_POLICYMAP_NAME_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    const char *name_str_p;
    RULE_TYPE_UI_PolicyMap_T pmap;
    UI32_T pmap_id = 0;

    memset(&pmap, 0, sizeof(pmap));

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    name_str_p = json_string_value(name_p);

    while (RULE_TYPE_OK == L4_PMGR_QoS_GetNextPolicyMap(&pmap_id, &pmap)
            && (pmap.row_status == VAL_diffServPolicyMapStatus_active))
    {
        if (0 != strcmp(name_str_p, pmap.name))
        {
            continue;
        }

        CGI_MODULE_QOS_GetOnePolicyMap(pmap_id, &pmap, result_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_POLICYMAP_NAME_Read

/**----------------------------------------------------------------------
 * This API is used to update a policy-map.
 *
 * @param name (required, string) policy-map name
 * @param status (required, boolean) status
 * @param elements (required, array) policy-map elements
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_POLICYMAP_NAME_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    json_t  *status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    json_t  *desc_p = CGI_REQUEST_GetBodyValue(http_request, "description");
    json_t  *elements_p = CGI_REQUEST_GetBodyValue(http_request, "elements");
    const char *name_str_p;
    const char *desc_str_p;
    char desc_ar[SYS_ADPT_DIFFSERV_MAX_DESCRIPTION_LENGTH + 1] = {0};
    BOOL_T status = FALSE;
    int ret = 0;

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_ValidPolicyMapElements(elements_p);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    name_str_p = json_string_value(name_p);

    if (TRUE != CGI_MODULE_QOS_ValidPolicyMapName(name_str_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid policy-map name.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        status = TRUE;
    }

    if (NULL != desc_p)
    {
        desc_str_p = json_string_value(desc_p);

        if (TRUE == status)
        {
            strncpy(desc_ar, desc_str_p, sizeof(desc_ar)-1);
            desc_ar[sizeof(desc_ar)-1] = '\0';
        }
        else
        {
            desc_ar[0] = '\0';
        }

        if (RULE_TYPE_OK != L4_PMGR_QoS_SetPolicyMapDescByName(name_str_p, desc_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetPolicyMapDescByNameError", "Failed.");
        }
    } //if (NULL != desc_p)

    if (NULL != elements_p)
    {
        ret = CGI_MODULE_QOS_SetPolicyMapElements(name_str_p, elements_p, status);

        if (CGI_MODULE_QOS_ERROR_NONE != ret)
        {
            return CGI_MODULE_QOS_ReturnErrorMsg(http_response, ret);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_POLICYMAP_NAME_Update

/**----------------------------------------------------------------------
 * This API is used to delete specified policy-map info.
 *
 * @param name (required, string) policy-map name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_POLICYMAP_NAME_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p = CGI_REQUEST_GetParamsValue(http_request, "name");
    UI32_T  ret = RULE_TYPE_OK;

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    ret = L4_PMGR_QoS_DelPolicyMapByName(json_string_value(name_p));

    if ((RULE_TYPE_OK != ret) && (RULE_TYPE_POLICY_MAP_NONEXISTED != ret))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.DelPolicyMapByNameError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_POLICYMAP_NAME_Delete

/**----------------------------------------------------------------------
 * This API is used to get port binding info.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_SERVICE_INTF_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *bindings_p = json_array();
    char    pname_ar[SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH + 1] = {0};
    UI32_T  lport = 0;

    json_object_set_new(result_p, "bindings", bindings_p);

    while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_GetNextLogicalPort(&lport))
    {
        if (RULE_TYPE_OK == L4_PMGR_QoS_GetPolicyMapNameByPort(lport, RULE_TYPE_INBOUND, pname_ar))
        {
            json_t *bind_obj_p = json_object();

            json_object_set_new(bind_obj_p, "port", json_integer(lport));
            json_object_set_new(bind_obj_p, "direction", json_string("input"));
            json_object_set_new(bind_obj_p, "policyMap", json_string(pname_ar));
            json_array_append_new(bindings_p, bind_obj_p);
        }

        if (RULE_TYPE_OK == L4_PMGR_QoS_GetPolicyMapNameByPort(lport, RULE_TYPE_OUTBOUND, pname_ar))
        {
            json_t *bind_obj_p = json_object();

            json_object_set_new(bind_obj_p, "port", json_integer(lport));
            json_object_set_new(bind_obj_p, "direction", json_string("output"));
            json_object_set_new(bind_obj_p, "policyMap", json_string(pname_ar));
            json_array_append_new(bindings_p, bind_obj_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_SERVICE_INTF_Read

/**----------------------------------------------------------------------
 * This API is used to bind a port to policy service.
 *
 * @param port (required, number) port ifindex
 * @param direction (required, string) "input" or "output"
 * @param policyMap (required, string) policy-map name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_SERVICE_INTF_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p = CGI_REQUEST_GetBodyValue(http_request, "port");
    json_t  *direct_p = CGI_REQUEST_GetBodyValue(http_request, "direction");
    json_t  *policy_p = CGI_REQUEST_GetBodyValue(http_request, "policyMap");
    const char *direct_str_p;
    const char *policy_str_p;
    RULE_TYPE_InOutDirection_T  direction = RULE_TYPE_INBOUND;
    UI32_T lport = 0;
    UI32_T unit = 0, port = 0, trunk = 0;

    if ((NULL == port_p) || (NULL == direct_p) || (NULL == policy_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port/direction/policyMap.");
    }

    lport = json_integer_value(port_p);
    direct_str_p = json_string_value(direct_p);
    policy_str_p = json_string_value(policy_p);

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port.");
    }

    if (0 == memcmp(direct_str_p, "input", strlen("input")))
    {
        direction = RULE_TYPE_INBOUND;
    }
    else if (0 == memcmp(direct_str_p, "output", strlen("output")))
    {
        direction = RULE_TYPE_OUTBOUND;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid direction.");
    }

    if (TRUE != CGI_MODULE_QOS_ValidPolicyMapName(policy_str_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid policy-map name.");
    }

    if (RULE_TYPE_OK != L4_PMGR_QoS_BindPort2PolicyMap(lport, policy_str_p, direction))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.BindPort2PolicyMapError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_SERVICE_INTF_Create

/**----------------------------------------------------------------------
 * This API is used to get specified port binding.
 *
 * @param id (required, number) port ifindex
 * @param direction (required, string) "input" or "output"
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_SERVICE_INTF_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    json_t  *direct_p = CGI_REQUEST_GetParamsValue(http_request, "direction");
    const char *direct_str_p;
    const char *policy_str_p;
    char  pname_ar[SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH + 1] = {0};
    RULE_TYPE_InOutDirection_T  direction = RULE_TYPE_INBOUND;
    UI32_T lport = 0;
    UI32_T unit = 0, port = 0, trunk = 0;

    if ((NULL == port_p) || (NULL == direct_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id/direction.");
    }

    lport = json_integer_value(port_p);
    direct_str_p = json_string_value(direct_p);

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port.");
    }

    if (0 == memcmp(direct_str_p, "input", strlen("input")))
    {
        direction = RULE_TYPE_INBOUND;
    }
    else if (0 == memcmp(direct_str_p, "output", strlen("output")))
    {
        direction = RULE_TYPE_OUTBOUND;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid direction.");
    }

    if (RULE_TYPE_OK == L4_PMGR_QoS_GetPolicyMapNameByPort(lport, direction, pname_ar))
    {
        json_object_set_new(result_p, "port", json_integer(lport));
        json_object_set_new(result_p, "direction", json_string(direct_str_p));
        json_object_set_new(result_p, "policyMap", json_string(pname_ar));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_SERVICE_INTF_ID_Read

/**----------------------------------------------------------------------
 * This API is used to get specified port binding.
 *
 * @param id (required, number) port ifindex
 * @param direction (required, string) "input" or "output"
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_QOS_SERVICE_INTF_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    json_t  *direct_p = CGI_REQUEST_GetParamsValue(http_request, "direction");
    const char *direct_str_p;
    const char *policy_str_p;
    char  pname_ar[SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH + 1] = {0};
    RULE_TYPE_InOutDirection_T  direction = RULE_TYPE_INBOUND;
    UI32_T lport = 0;
    UI32_T unit = 0, port = 0, trunk = 0;

    if ((NULL == port_p) || (NULL == direct_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id/direction.");
    }

    lport = json_integer_value(port_p);
    direct_str_p = json_string_value(direct_p);

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port.");
    }

    if (0 == memcmp(direct_str_p, "input", strlen("input")))
    {
        direction = RULE_TYPE_INBOUND;
    }
    else if (0 == memcmp(direct_str_p, "output", strlen("output")))
    {
        direction = RULE_TYPE_OUTBOUND;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid direction.");
    }

    if (RULE_TYPE_OK == L4_PMGR_QoS_GetPolicyMapNameByPort(lport, direction, pname_ar))
    {
        if (RULE_TYPE_OK != L4_PMGR_QoS_UnBindPortFromPolicyMap(lport, pname_ar, direction))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.UnBindPortFromPolicyMapError", "Failed.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_QOS_SERVICE_INTF_ID_Delete


static BOOL_T CGI_MODULE_QOS_GetOneClassMap(UI32_T cmap_id, RULE_TYPE_UI_ClassMap_T *cmap_p, json_t *cmap_obj_p)
{
    json_t *elements_p = json_array();
    RULE_TYPE_UI_ClassMapElement_T rec;
    char value_ar[4] = {0};
    UI32_T  cmap_eid = 0;

    memset(&rec, 0, sizeof(rec));

    json_object_set_new(cmap_obj_p, "name", json_string(cmap_p->class_map_name));
    json_object_set_new(cmap_obj_p, "description", json_string(cmap_p->desc));
    json_object_set_new(cmap_obj_p, "elements", elements_p);

    while (RULE_TYPE_OK == L4_PMGR_QoS_GetNextElementFromClassMap(cmap_id, &cmap_eid, &rec))
    {
        json_t *element_obj_p = json_object();

        if (RULE_TYPE_CLASS_ACL == rec.class_type)
        {
            json_object_set_new(element_obj_p, "type", json_string("access-list"));
            json_object_set_new(element_obj_p, "value", json_string(rec.element.acl_name));
        }
        else if (RULE_TYPE_CLASS_MF == rec.class_type)
        {
            switch (rec.element.mf_entry.mf_type)
            {
                case RULE_TYPE_MF_PRECEDENCE:
                    json_object_set_new(element_obj_p, "type", json_string("ip-precedence"));
                    break;

                case RULE_TYPE_MF_DSCP:
                    json_object_set_new(element_obj_p, "type", json_string("ip-dscp"));
                    break;

                case RULE_TYPE_MF_VLAN:
                    json_object_set_new(element_obj_p, "type", json_string("vlan"));
                    break;

#if (SYS_CPNT_IPV6 == TRUE)
                case RULE_TYPE_MF_DSCP_IPV6:
                    json_object_set_new(element_obj_p, "type", json_string("ipv6-dscp"));
                    break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

                case RULE_TYPE_MF_COS:
                    json_object_set_new(element_obj_p, "type", json_string("cos"));
                    break;

                case RULE_TYPE_MF_SOURCE_PORT:
                    json_object_set_new(element_obj_p, "type", json_string("source-port"));

                    break;

                default:
                    json_object_set_new(element_obj_p, "type", json_string("unknown"));
            } //switch (rec.element.mf_entry.mf_type)

            sprintf(value_ar, "%d", rec.element.mf_entry.mf_value);
            json_object_set_new(element_obj_p, "value", json_string(value_ar));
        }

        json_array_append_new(elements_p, element_obj_p);
    } //L4_PMGR_QoS_GetNextElementFromClassMap

    return TRUE;
} //CGI_MODULE_QOS_GetOneClassMap

static BOOL_T CGI_MODULE_QOS_GetOnePolicyMap(UI32_T pmap_id, RULE_TYPE_UI_PolicyMap_T *pmap_p, json_t *pmap_obj_p)
{
    json_t *elements_p = json_array();
    RULE_TYPE_UI_PolicyElement_T rec;
    char value_ar[4] = {0};
    UI32_T  cmap_eid = 0;

    memset(&rec, 0, sizeof(rec));

    json_object_set_new(pmap_obj_p, "name", json_string(pmap_p->name));
    json_object_set_new(pmap_obj_p, "description", json_string(pmap_p->desc));
    json_object_set_new(pmap_obj_p, "elements", elements_p);

    while ((RULE_TYPE_OK == L4_PMGR_QoS_GetNextElementFromPolicyMap(pmap_id, &rec))
            && (VAL_diffServPolicyMapElementStatus_active == rec.row_status))
    {
        json_t *element_obj_p = json_object();

        json_object_set_new(element_obj_p, "classMap", json_string(rec.class_map_name));

        switch (rec.action_entry.in_action_type)
        {
            case IN_ACTION_NEW_COS:
                json_object_set_new(element_obj_p, "actionType", json_string("cos"));
                json_object_set_new(element_obj_p, "actionValue",
                        json_integer((unsigned long)rec.action_entry.in_action_value));
                break;

            case IN_ACTION_NEW_IP_DSCP:
                json_object_set_new(element_obj_p, "actionType", json_string("ip-dscp"));
                json_object_set_new(element_obj_p, "actionValue",
                        json_integer((unsigned long)rec.action_entry.in_action_value));
                break;

            case IN_ACTION_NEW_IP_PRECEDENCE:
                json_object_set_new(element_obj_p, "actionType", json_string("ip-precedence"));
                json_object_set_new(element_obj_p, "actionValue",
                        json_integer((unsigned long)rec.action_entry.in_action_value));
                break;

            case IN_ACTION_NEW_PHB:
                json_object_set_new(element_obj_p, "actionType", json_string("phb"));
                json_object_set_new(element_obj_p, "actionValue",
                        json_integer((unsigned long)rec.action_entry.in_action_value));
                break;

            default:
                break;
        }

        if (VAL_diffServMeterStatus_active != rec.meter_entry.row_status)
        {
            json_array_append_new(elements_p, element_obj_p);
            continue;
        } //VAL_diffServMeterStatus_active

        if (RULE_TYPE_METER_MODE_FLOW == rec.meter_entry.meter_model)
        {
            json_object_set_new(element_obj_p, "meterModel", json_string("flow"));
            json_object_set_new(element_obj_p, "meterRate", json_integer((unsigned long)rec.meter_entry.rate));
            json_object_set_new(element_obj_p, "meterBurstSize", json_integer((unsigned long)rec.meter_entry.burst_size));
        }
        else if (RULE_TYPE_METER_MODE_TRTCM_COLOR_BLIND == rec.meter_entry.meter_model)
        {
            json_object_set_new(element_obj_p, "meterModel", json_string("trtcm-color-blind"));
            json_object_set_new(element_obj_p, "meterRate", json_integer((unsigned long)rec.meter_entry.rate));
            json_object_set_new(element_obj_p, "meterBurstSize", json_integer((unsigned long)rec.meter_entry.burst_size));
            json_object_set_new(element_obj_p, "meterPeakRate", json_integer((unsigned long)rec.meter_entry.peak_rate));
            json_object_set_new(element_obj_p, "meterPeakBurstSize", json_integer((unsigned long)rec.meter_entry.peak_burst_size));
        }
        else if (RULE_TYPE_METER_MODE_TRTCM_COLOR_AWARE == rec.meter_entry.meter_model)
        {
            json_object_set_new(element_obj_p, "meterModel", json_string("trtcm-color-aware"));
            json_object_set_new(element_obj_p, "meterRate", json_integer((unsigned long)rec.meter_entry.rate));
            json_object_set_new(element_obj_p, "meterBurstSize", json_integer((unsigned long)rec.meter_entry.burst_size));
            json_object_set_new(element_obj_p, "meterPeakRate", json_integer((unsigned long)rec.meter_entry.peak_rate));
            json_object_set_new(element_obj_p, "meterPeakBurstSize", json_integer((unsigned long)rec.meter_entry.peak_burst_size));
        }
        else if (RULE_TYPE_METER_MODE_SRTCM_COLOR_BLIND == rec.meter_entry.meter_model)
        {
            json_object_set_new(element_obj_p, "meterModel", json_string("srtcm-color-blind"));
            json_object_set_new(element_obj_p, "meterRate", json_integer((unsigned long)rec.meter_entry.rate));
            json_object_set_new(element_obj_p, "meterBurstSize", json_integer((unsigned long)rec.meter_entry.burst_size));
            json_object_set_new(element_obj_p, "meterPeakBurstSize", json_integer((unsigned long)rec.meter_entry.peak_burst_size));
        }
        else if (RULE_TYPE_METER_MODE_SRTCM_COLOR_AWARE == rec.meter_entry.meter_model)
        {
            json_object_set_new(element_obj_p, "meterModel", json_string("srtcm-color-aware"));
            json_object_set_new(element_obj_p, "meterRate", json_integer((unsigned long)rec.meter_entry.rate));
            json_object_set_new(element_obj_p, "meterBurstSize", json_integer((unsigned long)rec.meter_entry.burst_size));
            json_object_set_new(element_obj_p, "meterPeakBurstSize", json_integer((unsigned long)rec.meter_entry.peak_burst_size));
        }

        switch(rec.action_entry.confirm_action_type)
        {
            case CONFIRM_ACTION_TRANSMIT:
                json_object_set_new(element_obj_p, "conformAction", json_string("transmit"));
                break;

            case CONFIRM_ACTION_REMARK_DSCP_TRANSMIT:
                sprintf(value_ar, "%d", rec.action_entry.confirm_action_value);
                json_object_set_new(element_obj_p, "conformAction", json_string(value_ar));
                break;

            default:
                break;
        }

        switch(rec.action_entry.exceed_action_type)
        {
            case EXCEED_ACTION_DROP:
                json_object_set_new(element_obj_p, "exceedAction", json_string("drop"));
                break;

            case EXCEED_ACTION_REMARK_DSCP_TRANSMIT:
                sprintf(value_ar, "%d", rec.action_entry.exceed_action_value);
                json_object_set_new(element_obj_p, "exceedAction", json_string(value_ar));
                break;

            default:
                break;
        }

        switch(rec.action_entry.violate_action_type)
        {
            case VIOLATE_ACTION_DROP:
                json_object_set_new(element_obj_p, "violateAction", json_string("drop"));
                break;

            case VIOLATE_ACTION_REMARK_DSCP_TRANSMIT:
                sprintf(value_ar, "%d", rec.action_entry.violate_action_value);
                json_object_set_new(element_obj_p, "violateAction", json_string(value_ar));
                break;

            default:
                break;
        }

        json_array_append_new(elements_p, element_obj_p);
    } //L4_PMGR_QoS_GetNextElementFromPolicyMap

    return TRUE;
} //CGI_MODULE_QOS_GetOnePolicyMap

static int CGI_MODULE_QOS_ValidClassMapElements(json_t *elements_p)
{
    json_t *element_p;
    json_t *type_p;
    json_t *value_p;
    const char *type_str_p;
    const char *value_str_p;
    UI32_T acl_index = 0;
    int element_num = 0, idx = 0;
    int value = 0;

    element_num = json_array_size(elements_p);

    if (0 == element_num)
    {
        return CGI_MODULE_QOS_ERROR_NONE;
    }

    if (SYS_ADPT_DIFFSERV_MAX_CLASS_NBR_OF_CLASS_MAP < element_num)
    {
        return CGI_MODULE_QOS_ERROR_CALSS_EXCEED_ELEMENTS;
    }

    for (idx = 0; idx < element_num; idx ++)
    {
        element_p = json_array_get(elements_p, idx);

        if (NULL == element_p)
        {
            continue;
        } //if (NULL != element_p)

        type_p = json_object_get(element_p, "type");
        value_p = json_object_get(element_p, "value");

        if ((NULL == type_p) || (NULL == value_p))
        {
            return CGI_MODULE_QOS_ERROR_CALSS_INVALID_TYPE;
        }

        type_str_p = json_string_value(type_p);
        value_str_p = json_string_value(value_p);

        if (0 == memcmp(type_str_p, "cos", strlen("cos")))
        {
            value = atoi(value_str_p);

            if ((MIN_diffServMacAceMinCos > value) || (MAX_diffServMacAceMinCos < value))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_COS;
            }
        }
        else if (0 == memcmp(type_str_p, "vlan", strlen("vlan")))
        {
            value = atoi(value_str_p);

            if ((1 > value) || (SYS_DFLT_DOT1QMAXVLANID < value))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_VLAN;
            }
        }
        else if (0 == memcmp(type_str_p, "ip-dscp", strlen("ip-dscp")))
        {
            value = atoi(value_str_p);

            if ((MIN_DSCP_VAL > value) || (MAX_DSCP_VAL < value))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPDSCP;
            }
        }
        else if (0 == memcmp(type_str_p, "ipv6-dscp", strlen("ipv6-dscp")))
        {
            value = atoi(value_str_p);

            if ((MIN_DSCP_VAL > value) || (MAX_DSCP_VAL < value))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPDSCP;
            }
        }
        else if (0 == memcmp(type_str_p, "access-list", strlen("access-list")))
        {
            if (SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH < strlen(value_str_p))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_ACL;
            }

            if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(value_str_p, &acl_index))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_ACL;
            }
        }
        else if (0 == memcmp(type_str_p, "ip-precedence", strlen("ip-precedence")))
        {
            value = atoi(value_str_p);

            if ((MIN_PRE_VAL > value) || (MAX_PRE_VAL < value))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPPRECEDENCE;
            }
        }
        else if (0 == memcmp(type_str_p, "source-port", strlen("source-port")))
        {
            UI32_T  unit = 0, port = 0, trunk = 0;

            value = atoi(value_str_p);

            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(value, &unit, &port, &trunk))
            {
                return CGI_MODULE_QOS_ERROR_CALSS_INVALID_SRCPORT;
            }
        }
        else
        {
            return CGI_MODULE_QOS_ERROR_CALSS_INVALID_TYPE;
        }
    } //for (idx = 0; idx < element_num; idx ++)

    return CGI_MODULE_QOS_ERROR_NONE;
} //CGI_MODULE_QOS_ValidClassMapElements

static int CGI_MODULE_QOS_ValidPolicyMapElements(json_t *elements_p)
{
    json_t *element_p;
    json_t *class_p;
    json_t *action_p;
    json_t *action_val_p;
    json_t *meter_p;
    json_t *rate_p;
    json_t *burst_p;
    json_t *peak_rate_p;
    json_t *peak_burst_p;
    json_t *conform_p;
    json_t *exceed_p;
    json_t *violate_p;
    const char *class_str_p;
    const char *action_str_p;
    const char *meter_str_p;
    const char *conform_str_p;
    const char *violate_str_p;
    UI32_T cmap_idx = 0;
    int element_num = 0, idx = 0;
    int value = 0;
    int rate = 0, burst = 0, peak_rate = 0, peak_burst = 0;

    element_num = json_array_size(elements_p);

    if (0 == element_num)
    {
        return CGI_MODULE_QOS_ERROR_NONE;
    }

    if (SYS_ADPT_DIFFSERV_MAX_CLASS_MAP_NBR_OF_POLICY_MAP < element_num)
    {
        return CGI_MODULE_QOS_ERROR_CALSS_EXCEED_ELEMENTS;
    }

    for (idx = 0; idx < element_num; idx ++)
    {
        element_p = json_array_get(elements_p, idx);

        if (NULL == element_p)
        {
            continue;
        } //if (NULL != element_p)

        class_p = json_object_get(element_p, "classMap");

        if (NULL == class_p)
        {
            continue; //do nothing
        }

        class_str_p = json_string_value(class_p);

        if (RULE_TYPE_OK != L4_PMGR_QoS_GetClassMapIdByName(class_str_p, &cmap_idx))
        {
            return CGI_MODULE_QOS_ERROR_POLICY_INVALID_CLASS;
        }

        action_p = json_object_get(element_p, "actionType");
        action_val_p = json_object_get(element_p, "actionValue");

        if (NULL != action_p)
        {
            if (NULL == action_val_p)
            {
                return CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION_VAL;
            }

            action_str_p = json_string_value(action_p);
            value = json_integer_value(action_val_p);

            if (0 == memcmp(action_str_p, "cos", strlen("cos")))
            {
                if ((MIN_diffServMacAceMinCos > value) || (MAX_diffServMacAceMinCos < value))
                {
                    return CGI_MODULE_QOS_ERROR_CALSS_INVALID_COS;
                }
            }
            else if (0 == memcmp(action_str_p, "phb", strlen("cos")))
            {
                if ((MIN_PHB_VAL > value) || (MAX_PHB_VAL < value))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_INVALID_PHB;
                }
            }
            else if (0 == memcmp(action_str_p, "ip-dscp", strlen("ip-dscp")))
            {
                if ((MIN_DSCP_VAL > value) || (MAX_DSCP_VAL < value))
                {
                    return CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPDSCP;
                }
            }
            else if (0 == memcmp(action_str_p, "ip-precedence", strlen("ip-precedence")))
            {
                if ((MIN_PRE_VAL > value) || (MAX_PRE_VAL < value))
                {
                    return CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPPRECEDENCE;
                }
            }
            else
            {
                return CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION;
            }
        } //if (NULL != action_p)

        meter_p = json_object_get(element_p, "meterModel");
        rate_p = json_object_get(element_p, "meterRate");
        burst_p = json_object_get(element_p, "meterBurstSize");
        peak_rate_p = json_object_get(element_p, "meterPeakRate");
        peak_burst_p = json_object_get(element_p, "meterPeakBurstSize");
        conform_p = json_object_get(element_p, "conformAction");
        exceed_p = json_object_get(element_p, "exceedAction");
        violate_p = json_object_get(element_p, "violateAction");

        if (NULL != meter_p)
        {
            if ((NULL == rate_p) || (NULL == burst_p) || (NULL == conform_p) || (NULL == violate_p))
            {
                return CGI_MODULE_QOS_ERROR_POLICY_MISS_METER;
            }

            meter_str_p = json_string_value(meter_p);
            rate = json_integer_value(rate_p);
            burst = json_integer_value(burst_p);

            if ((SYS_ADPT_DIFFSERV_MIN_POLICE_RATE > rate) || (SYS_ADPT_DIFFSERV_MAX_POLICE_RATE < rate))
            {
                return CGI_MODULE_QOS_ERROR_POLICY_METER_RATE;
            }

            if ((SYS_ADPT_DIFFSERV_MIN_POLICE_BURST > burst) || (SYS_ADPT_DIFFSERV_MAX_POLICE_BURST < burst))
            {
                return CGI_MODULE_QOS_ERROR_POLICY_METER_BURST;
            }

            if (NULL != peak_rate_p)
            {
                peak_rate = json_integer_value(peak_rate_p);

                if ((SYS_ADPT_DIFFSERV_MIN_POLICE_RATE > peak_rate) || (SYS_ADPT_DIFFSERV_MAX_POLICE_RATE < peak_rate))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_RATE;
                }
            }

            if (NULL != peak_burst_p)
            {
                peak_burst = json_integer_value(peak_burst_p);

                if ((SYS_ADPT_DIFFSERV_MIN_POLICE_BURST > peak_burst) || (SYS_ADPT_DIFFSERV_MAX_POLICE_BURST < peak_burst))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_BURST;
                }
            }

            conform_str_p = json_string_value(conform_p);
            violate_str_p = json_string_value(violate_p);

            if (0 != memcmp(conform_str_p, "transmit", strlen("transmit")))
            {
                if ((2 < strlen(conform_str_p)) || (MAX_DSCP_VAL < atoi(conform_str_p))) //2 is max length of dscp
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_REMARK_DSCP;
                }
            }

            if (0 != memcmp(violate_str_p, "drop", strlen("drop")))
            {
                if ((2 < strlen(violate_str_p)) || (MAX_DSCP_VAL < atoi(violate_str_p))) //2 is max length of dscp
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_REMARK_DSCP;
                }
            }

            if (NULL != exceed_p)
            {
                const char *exceed_str_p = json_string_value(exceed_p);

                if (0 != memcmp(exceed_str_p, "drop", strlen("drop")))
                {
                    if ((2 < strlen(exceed_str_p)) || (MAX_DSCP_VAL < atoi(exceed_str_p))) //2 is max length of dscp
                    {
                        return CGI_MODULE_QOS_ERROR_POLICY_METER_REMARK_DSCP;
                    }
                }
            }

            if (0 == memcmp(meter_str_p, "flow", strlen("flow")))
            {
                if ((NULL != peak_rate_p) || (NULL != peak_burst_p))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_FIELD;
                }
            }
            else if ((0 == memcmp(meter_str_p, "srtcm-color-blind", strlen("srtcm-color-blind")))
                    || (0 == memcmp(meter_str_p, "srtcm-color-aware", strlen("srtcm-color-aware"))))
            {
                if (NULL != peak_rate_p)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_SRTCM_UNSUPPORT_FIELD;
                }

                if (NULL == peak_burst_p)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_SRTCM_FIELD;
                }

                if (NULL == exceed_p)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_EXCEED_ACT;
                }
            }
            else if ((0 == memcmp(meter_str_p, "trtcm-color-blind", strlen("trtcm-color-blind")))
                    || (0 == memcmp(meter_str_p, "trtcm-color-aware", strlen("trtcm-color-aware"))))
            {
                if ((NULL == peak_rate_p) || (NULL == peak_burst_p))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_TRTCM_FIELD;
                }

                if (NULL == exceed_p)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_EXCEED_ACT;
                }
            }
            else
            {
                return CGI_MODULE_QOS_ERROR_POLICY_METER_MODEL;
            }
        } //if (NULL != meter_p)
    } //for (idx = 0; idx < element_num; idx ++)

    return CGI_MODULE_QOS_ERROR_NONE;
} //CGI_MODULE_QOS_ValidPolicyMapElements

static int CGI_MODULE_QOS_SetClassMapElements(const char *name_p, json_t *elements_p, BOOL_T add)
{
    json_t *element_p;
    json_t *type_p;
    json_t *value_p;
    const char *type_str_p;
    const char *value_str_p;
    RULE_TYPE_UI_ClassMapElement_T rec;
    UI32_T ret = RULE_TYPE_OK;
    int element_num = 0, idx = 0;

    element_num = json_array_size(elements_p);

    if (0 == element_num)
    {
        return CGI_MODULE_QOS_ERROR_NONE;
    }

    for (idx = 0; idx < element_num; idx ++)
    {
        element_p = json_array_get(elements_p, idx);

        if (NULL == element_p)
        {
            continue;
        } //if (NULL != element_p)

        memset(&rec, 0, sizeof(rec));
        rec.class_type = RULE_TYPE_CLASS_MF;
        type_p = json_object_get(element_p, "type");
        value_p = json_object_get(element_p, "value");
        type_str_p = json_string_value(type_p);
        value_str_p = json_string_value(value_p);

        if (0 == memcmp(type_str_p, "cos", strlen("cos")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_COS;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }
        else if (0 == memcmp(type_str_p, "vlan", strlen("vlan")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_VLAN;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }
        else if (0 == memcmp(type_str_p, "ip-dscp", strlen("ip-dscp")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_DSCP;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }
        else if (0 == memcmp(type_str_p, "ipv6-dscp", strlen("ipv6-dscp")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_DSCP_IPV6;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }
        else if (0 == memcmp(type_str_p, "access-list", strlen("access-list")))
        {
            rec.class_type = RULE_TYPE_CLASS_ACL;
            strcpy(rec.element.acl_name, value_str_p);
        }
        else if (0 == memcmp(type_str_p, "ip-precedence", strlen("ip-precedence")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_PRECEDENCE;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }
        else if (0 == memcmp(type_str_p, "source-port", strlen("source-port")))
        {
            rec.element.mf_entry.mf_type = RULE_TYPE_MF_SOURCE_PORT;
            rec.element.mf_entry.mf_value = atoi(value_str_p);
        }

        if (TRUE == add)
        {
            ret = L4_PMGR_QoS_AddElement2ClassMap(name_p, &rec);
        }
        else
        {
            ret = L4_PMGR_QoS_DelElementFromClassMap(name_p, &rec);
        }

        switch(ret)
        {
            case RULE_TYPE_OK:
                break;

            case RULE_TYPE_NOT_ALLOW_MIX_SELECTOR:
                return CGI_MODULE_QOS_ERROR_CALSS_NOT_ALLOW_MIX_SELECTOR;

            case RULE_TYPE_FREE_SELECTOR_UNAVAILABLE:
                return CGI_MODULE_QOS_ERROR_CALSS_FREE_SELECTOR_UNAVAILABLE;

            default:
                return CGI_MODULE_QOS_ERROR_CALSS_FAIL;
                break;
        }
    } //for (idx = 0; idx < element_num; idx ++)

    return CGI_MODULE_QOS_ERROR_NONE;
} //CGI_MODULE_QOS_SetClassMapElements

static int CGI_MODULE_QOS_SetPolicyMapElements(const char *name_p, json_t *elements_p, BOOL_T add)
{
    json_t *element_p;
    json_t *class_p;
    json_t *action_p;
    json_t *action_val_p;
    json_t *meter_p;
    json_t *rate_p;
    json_t *burst_p;
    json_t *peak_rate_p;
    json_t *peak_burst_p;
    json_t *conform_p;
    json_t *exceed_p;
    json_t *violate_p;
    const char *class_str_p;
    const char *action_str_p;
    const char *meter_str_p;
    const char *conform_str_p;
    const char *exceed_str_p;
    const char *violate_str_p;
    UI32_T rule_ret = RULE_TYPE_OK;
    int element_num = 0, idx = 0;
    int value = 0;
    int rate = 0, burst = 0, peak_rate = 0, peak_burst = 0;

    element_num = json_array_size(elements_p);

    if (0 == element_num)
    {
        return CGI_MODULE_QOS_ERROR_NONE;
    }

    for (idx = 0; idx < element_num; idx ++)
    {
        RULE_TYPE_UI_Action_T action;
        RULE_TYPE_TBParamEntry_T meter;

        memset(&action, 0, sizeof(action));
        memset(&meter, 0, sizeof(meter));

        //initial action
        action.in_action_type = IN_ACTION_INVALID;
        action.in_action_value = 0;
        action.out_action_type = OUT_ACTION_INVALID;
        action.out_action_value = 0;
        action.confirm_action_type = CONFIRM_ACTION_INVALID;
        action.confirm_action_value = 0;
        action.exceed_action_type = EXCEED_ACTION_INVALID;
        action.exceed_action_value = 0;
        action.violate_action_type = VIOLATE_ACTION_INVALID;
        action.violate_action_value = 0;
        action.class_action_type = CLASS_ACTION_INVALID;
        action.class_action_value = 0;

        meter.row_status = VAL_diffServMeterStatus_active;

        element_p = json_array_get(elements_p, idx);

        if (NULL == element_p)
        {
            continue;
        } //if (NULL != element_p)

        class_p = json_object_get(element_p, "classMap");
        action_p = json_object_get(element_p, "actionType");
        meter_p = json_object_get(element_p, "meterModel");

        if (NULL == class_p)
        {
            continue; //do nothing
        }

        class_str_p = json_string_value(class_p);

        if ((NULL == action_p) && (NULL == meter_p))
        {
            if (TRUE != add) //delete class, no need to parse other fields
            {
                rule_ret = L4_PMGR_QoS_DelElementFromPolicyMap(name_p, class_str_p);

                if ((RULE_TYPE_OK != rule_ret) && (RULE_TYPE_CLASS_MAP_NONEXISTED != rule_ret))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS;
                }
            }
            else
            {
                if (RULE_TYPE_OK != L4_PMGR_QoS_AddElement2PolicyMap(name_p, class_str_p, NULL, NULL))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS;
                }
            }

            continue;  //delete class, no need to parse other fields
        }

        if (NULL != action_p)
        {
            action_val_p = json_object_get(element_p, "actionValue");
            action_str_p = json_string_value(action_p);

            if (0 == memcmp(action_str_p, "cos", strlen("cos")))
            {
                action.in_action_type = IN_ACTION_NEW_COS;
            }
            else if (0 == memcmp(action_str_p, "phb", strlen("cos")))
            {
                action.in_action_type = IN_ACTION_NEW_PHB;
            }
            else if (0 == memcmp(action_str_p, "ip-dscp", strlen("ip-dscp")))
            {
                action.in_action_type = IN_ACTION_NEW_IP_DSCP;
            }
            else if (0 == memcmp(action_str_p, "ip-precedence", strlen("ip-precedence")))
            {
                action.in_action_type = IN_ACTION_NEW_IP_PRECEDENCE;
            }

            action.in_action_value= json_integer_value(action_val_p);

            if (TRUE != add)
            {
                if (RULE_TYPE_OK != L4_PMGR_QoS_DelElementActionFromPolicyMap(name_p, class_str_p, &action))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_ACTION;
                }
            }
            else
            {
                if (RULE_TYPE_OK != L4_PMGR_QoS_AddElement2PolicyMap(name_p, class_str_p, NULL, &action))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_ACTION;
                }
            }
        } //if (NULL != action_p)

        meter_str_p = json_string_value(meter_p);
        rate_p = json_object_get(element_p, "meterRate");
        burst_p = json_object_get(element_p, "meterBurstSize");
        peak_rate_p = json_object_get(element_p, "meterPeakRate");
        peak_burst_p = json_object_get(element_p, "meterPeakBurstSize");
        conform_p = json_object_get(element_p, "conformAction");
        exceed_p = json_object_get(element_p, "exceedAction");
        violate_p = json_object_get(element_p, "violateAction");

        if (NULL != meter_p)
        {
            meter.rate = json_integer_value(rate_p);
            meter.burst_size = json_integer_value(burst_p);

            if (NULL != peak_rate_p)
            {
                meter.peak_rate = json_integer_value(peak_rate_p);
            }

            if (NULL != peak_burst_p)
            {
                meter.peak_burst_size = json_integer_value(peak_burst_p);
            }

            conform_str_p = json_string_value(conform_p);
            violate_str_p = json_string_value(violate_p);

            if (0 == memcmp(conform_str_p, "transmit", strlen("transmit")))
            {
                action.confirm_action_type = CONFIRM_ACTION_TRANSMIT;
            }
            else
            {
                action.confirm_action_type = CONFIRM_ACTION_REMARK_DSCP_TRANSMIT;
                action.confirm_action_value = atoi(conform_str_p);
            }

            if (0 == memcmp(violate_str_p, "drop", strlen("drop")))
            {
                action.violate_action_type = VIOLATE_ACTION_DROP;
            }
            else
            {
                action.violate_action_type = VIOLATE_ACTION_REMARK_DSCP_TRANSMIT;
                action.violate_action_value = atoi(violate_str_p);
            }

            if (NULL != exceed_p)
            {
                exceed_str_p = json_string_value(exceed_p);

                if (0 == memcmp(exceed_str_p, "drop", strlen("drop")))
                {
                    action.exceed_action_type = EXCEED_ACTION_DROP;
                }
                else
                {
                    action.exceed_action_type = EXCEED_ACTION_REMARK_DSCP_TRANSMIT;
                    action.exceed_action_value = atoi(exceed_str_p);
                }
            }

            if (0 == memcmp(meter_str_p, "flow", strlen("flow")))
            {
                meter.meter_model = RULE_TYPE_METER_MODE_FLOW;
            }
            else if (0 == memcmp(meter_str_p, "srtcm-color-blind", strlen("srtcm-color-blind")))
            {
                meter.meter_model = RULE_TYPE_METER_MODE_SRTCM_COLOR_BLIND;
            }
            else if (0 == memcmp(meter_str_p, "srtcm-color-aware", strlen("srtcm-color-aware")))
            {
                meter.meter_model = RULE_TYPE_METER_MODE_SRTCM_COLOR_AWARE;
            }
            else if (0 == memcmp(meter_str_p, "trtcm-color-blind", strlen("trtcm-color-blind")))
            {
                meter.meter_model = RULE_TYPE_METER_MODE_TRTCM_COLOR_BLIND;
            }
            else if(0 == memcmp(meter_str_p, "trtcm-color-aware", strlen("trtcm-color-aware")))
            {
                meter.meter_model = RULE_TYPE_METER_MODE_TRTCM_COLOR_AWARE;
            }

            if (TRUE != add)
            {
                if (RULE_TYPE_OK != L4_PMGR_QoS_DelElementActionFromPolicyMap(name_p, class_str_p, &action))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_ACTION;
                }

                if (RULE_TYPE_OK != L4_PMGR_QoS_DelElementMeterFromPolicyMap(name_p, class_str_p, &meter))
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_METER;
                }
            }
            else
            {
                rule_ret = L4_PMGR_QoS_AddElement2PolicyMap(name_p, class_str_p, &meter, &action);

                if (RULE_TYPE_METER_FAIL == rule_ret)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_METER;
                }
                else if (RULE_TYPE_ACTION_FAIL == rule_ret)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_ACTION;
                }
                else if (RULE_TYPE_OK != rule_ret)
                {
                    return CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS;
                }
            }
        } //if (NULL != meter_p)

    } //for (idx = 0; idx < element_num; idx ++)

    return CGI_MODULE_QOS_ERROR_NONE;
} //CGI_MODULE_QOS_SetPolicyMapElements

static BOOL_T CGI_MODULE_QOS_ValidPolicyMapName(const char *pname_p)
{
    RULE_TYPE_UI_PolicyMap_T pmap;
    UI32_T pmap_id = 0;

    memset(&pmap, 0, sizeof(pmap));

    while (RULE_TYPE_OK == L4_PMGR_QoS_GetNextPolicyMap(&pmap_id, &pmap)
            && (pmap.row_status == VAL_diffServPolicyMapStatus_active))
    {
        if (0 == strcmp(pname_p, pmap.name))
        {
            return TRUE;
        }
    }

    return FALSE;
} //CGI_MODULE_QOS_ValidPolicyMapName

static CGI_STATUS_CODE_T CGI_MODULE_QOS_ReturnErrorMsg(HTTP_Response_T *http_response, int error_code)
{
    switch(error_code)
    {
        case CGI_MODULE_QOS_ERROR_CALSS_EXCEED_ELEMENTS:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Exceed max elements.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_TYPE:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid element type.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_ACL:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ACL name.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_COS:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "CoS out of range.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPPRECEDENCE:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "IP precedence out of range.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_IPDSCP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "IP DSCP out of range.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_VLAN:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "VLAN ID out of range.");

        case CGI_MODULE_QOS_ERROR_CALSS_INVALID_SRCPORT:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port.");

        case CGI_MODULE_QOS_ERROR_CALSS_NOT_ALLOW_MIX_SELECTOR:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetClassElementError", "Not allow mix selector.");

        case CGI_MODULE_QOS_ERROR_CALSS_FREE_SELECTOR_UNAVAILABLE:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.SetClassElementError", "No free selector.");

        case CGI_MODULE_QOS_ERROR_CALSS_FAIL:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.AddElement2ClassMapError", "Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_INVALID_CLASS:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid class-map.");

        case CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid action.");

        case CGI_MODULE_QOS_ERROR_POLICY_INVALID_ACTION_VAL:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid action value.");

        case CGI_MODULE_QOS_ERROR_POLICY_INVALID_PHB:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid PHB value.");

        case CGI_MODULE_QOS_ERROR_POLICY_MISS_METER:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Miss meter field.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_MODEL:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid meter model.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_FIELD:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Include invalid meter field.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_RATE:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid meter rate.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_BURST:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid meter burst size.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_RATE:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid meter peak rate.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_PEAK_BURST:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid meter peak burst size.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_SRTCM_FIELD:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Miss SRTCM field.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_SRTCM_UNSUPPORT_FIELD:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "SRTCM does not support peak rate.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_TRTCM_FIELD:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Miss TRTCM field.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_MISS_EXCEED_ACT:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Miss exceed action.");

        case CGI_MODULE_QOS_ERROR_POLICY_METER_REMARK_DSCP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid DSCP of remark action.");

        case CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.AddElement2PolicyMapError", "Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_ACTION:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.AddElement2PolicyMapError", "Action Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_ADD_CLASS_METER:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.AddElement2PolicyMapError", "Meter Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.DelElementFromPolicyMapError", "Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_ACTION:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.DelElementFromPolicyMapError", "Action Failed.");

        case CGI_MODULE_QOS_ERROR_POLICY_DEL_CLASS_METER:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "QoS.DelElementFromPolicyMapError", "MeterFailed.");

        default:
            break;
    }

    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "unknown error.");
} //CGI_MODULE_QOS_ReturnErrorMsg

void CGI_MODULE_QOS_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_QOS_CLASSMAP_Read;
    handlers.create_handler = CGI_MODULE_QOS_CLASSMAP_Create;
    CGI_MAIN_Register("/api/v1/qos/class-maps", &handlers, 0);

    {   /* for "/api/v1/qos/class-maps/{name}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_QOS_CLASSMAP_NAME_Read;
        handlers.update_handler = CGI_MODULE_QOS_CLASSMAP_NAME_Update;
        handlers.delete_handler = CGI_MODULE_QOS_CLASSMAP_NAME_Delete;
        CGI_MAIN_Register("/api/v1/qos/class-maps/{name}", &handlers, 0);
    }

    {   /* for "/api/v1/qos/policy-maps" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_QOS_POLICYMAP_Read;
        handlers.create_handler = CGI_MODULE_QOS_POLICYMAP_Create;
        CGI_MAIN_Register("/api/v1/qos/policy-maps", &handlers, 0);
    }

    {   /* for "/api/v1/qos/policy-maps/{name}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_QOS_POLICYMAP_NAME_Read;
        handlers.update_handler = CGI_MODULE_QOS_POLICYMAP_NAME_Update;
        handlers.delete_handler = CGI_MODULE_QOS_POLICYMAP_NAME_Delete;
        CGI_MAIN_Register("/api/v1/qos/policy-maps/{name}", &handlers, 0);
    }

    {   /* for "/api/v1/qos/service-policy/interfaces" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_QOS_SERVICE_INTF_Read;
        handlers.create_handler = CGI_MODULE_QOS_SERVICE_INTF_Create;
        CGI_MAIN_Register("/api/v1/qos/service-policy/interfaces", &handlers, 0);
    }

    {   /* for "/api/v1/qos/service-policy/interfaces/{id}/direction/{direction}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_QOS_SERVICE_INTF_ID_Read;
        handlers.delete_handler = CGI_MODULE_QOS_SERVICE_INTF_ID_Delete;
        CGI_MAIN_Register("/api/v1/qos/service-policy/interfaces/{id}/direction/{direction}", &handlers, 0);
    }
}

static void CGI_MODULE_QOS_Init()
{
    CGI_MODULE_QOS_RegisterHandlers();
}

