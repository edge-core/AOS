#include "cgi_util.h"
#include "rule_type.h"
#include "l4_pmgr.h"

#define IS_EMPTY_MAC_ADDR_MASK(mask)         \
            (0    == (mask[0] | mask[1] | mask[2] |           \
                      mask[3] | mask[4] | mask[5]))
#define IS_FULL_MAC_ADDR_MASK(mask)          \
            (0xff == (mask[0] & mask[1] & mask[2] &           \
                      mask[3] & mask[4] & mask[5]))

enum CGI_MODULE_ACL_PARSEACE_CODE_E
{
    CGI_MODULE_ACL_PARSEACE_CODE_NONE = 0,
    CGI_MODULE_ACL_PARSEACE_CODE_ACTION,
    CGI_MODULE_ACL_PARSEACE_CODE_PKTFORMAT,
    CGI_MODULE_ACL_PARSEACE_CODE_SA,
    CGI_MODULE_ACL_PARSEACE_CODE_SAMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_DA,
    CGI_MODULE_ACL_PARSEACE_CODE_DAMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_VID,
    CGI_MODULE_ACL_PARSEACE_CODE_VIDMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_ETYPE,
    CGI_MODULE_ACL_PARSEACE_CODE_ETYPEMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_STDIP,
    CGI_MODULE_ACL_PARSEACE_CODE_SIP,
    CGI_MODULE_ACL_PARSEACE_CODE_SIPMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_DIP,
    CGI_MODULE_ACL_PARSEACE_CODE_DIPMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_PROTOCOL,
    CGI_MODULE_ACL_PARSEACE_CODE_PREC,
    CGI_MODULE_ACL_PARSEACE_CODE_DSCP,
    CGI_MODULE_ACL_PARSEACE_CODE_SPORT,
    CGI_MODULE_ACL_PARSEACE_CODE_SPORTMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_DPORT,
    CGI_MODULE_ACL_PARSEACE_CODE_DPORTMASK,
    CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAG,
    CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAGMASK
};

static int CGI_MODULE_ACL_ValidMacAceJson(json_t *ace_json_p);
static int CGI_MODULE_ACL_ValidIpAceJson(json_t *ace_json_p, RULE_TYPE_AclType_T type);
static void CGI_MODULE_ACL_ConstructMacAce(json_t *ace_json_p, RULE_TYPE_UI_Ace_Entry_T *ace_p);
static void CGI_MODULE_ACL_ConstructIpAce(json_t *ace_json_p, RULE_TYPE_AclType_T type, RULE_TYPE_UI_Ace_Entry_T *ace_p);
static void CGI_MODULE_ACL_GetOneInterfaceBind(UI32_T ifindex, RULE_TYPE_UI_AclEntry_T *acl_p, int direction, json_t *bind_obj_p);
static BOOL_T CGI_MODULE_ACL_GetAclType(const char *type_str_p, RULE_TYPE_AclType_T *acl_type_p);
static BOOL_T CGI_MODULE_ACL_GetAclDirection(const char *dir_str_p, BOOL_T *ingress_flag_p);
static CGI_STATUS_CODE_T CGI_MODULE_ACL_ReturnErrorMsg(HTTP_Response_T *http_response, int error_code);

/**----------------------------------------------------------------------
 * This API is used to get MAC ACL info.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *acls_p = json_array();
    RULE_TYPE_UI_AclEntry_T acl_entry;
    RULE_TYPE_UI_Ace_Entry_T ace;
    UI8_T  mac_ar[CGI_MACSTRLEN] = {0};
    UI32_T acl_index = 0;
    UI32_T ace_index = 0;

    json_object_set_new(result_p, "acls", acls_p);
    memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.acl_type = RULE_TYPE_MAC_ACL;

    while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))
    {
        json_t *acl_obj_p = json_object();
        json_t *aces_p = json_array();

        if (RULE_TYPE_MAC_ACL != acl_entry.acl_type)
        {
            continue;
        }

        ace_index = 0;
        memset(&ace, 0, sizeof(ace));
        json_object_set_new(acl_obj_p, "name", json_string(acl_entry.acl_name));
        json_object_set_new(acl_obj_p, "aces", aces_p);

        while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextUIAceByAcl(acl_index, &ace_index, &ace))
        {
            json_t *ace_obj_p = json_object();

            if (RULE_TYPE_ACE_PERMIT == ace.access)
            {
                json_object_set_new(ace_obj_p, "action", json_string("permit"));
            }
            else
            {
                json_object_set_new(ace_obj_p, "action", json_string("deny"));
            }

            switch (ace.pkt_format)
            {
                case VAL_diffServMacAcePktformat_untagged_Eth2:
                    json_object_set_new(ace_obj_p, "pktFormat", json_string("untagged-eth2"));
                    break;

                case VAL_diffServMacAcePktformat_untagged802Dot3:
                    json_object_set_new(ace_obj_p, "pktFormat", json_string("untagged-802.3"));
                    break;

                case VAL_diffServMacAcePktformat_tagggedEth2:
                    json_object_set_new(ace_obj_p, "pktFormat", json_string("tagged-eth2"));
                    break;

                case VAL_diffServMacAcePktformat_tagged802Dot3:
                    json_object_set_new(ace_obj_p, "pktFormat", json_string("tagged-802.3"));
                    break;

                case VAL_diffServMacAcePktformat_any:
                default:
                    json_object_set_new(ace_obj_p, "pktFormat", json_string("any"));
            }

            if (!IS_EMPTY_MAC_ADDR_MASK(ace.ether.sa.mask))
            {
                sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
                        ace.ether.sa.addr[0], ace.ether.sa.addr[1], ace.ether.sa.addr[2],
                        ace.ether.sa.addr[3], ace.ether.sa.addr[4], ace.ether.sa.addr[5]);
                json_object_set_new(ace_obj_p, "srcMac", json_string(mac_ar));
                sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
                        ace.ether.sa.mask[0], ace.ether.sa.mask[1], ace.ether.sa.mask[2],
                        ace.ether.sa.mask[3], ace.ether.sa.mask[4], ace.ether.sa.mask[5]);
                json_object_set_new(ace_obj_p, "srcMacMask", json_string(mac_ar));
            }

            if (!IS_EMPTY_MAC_ADDR_MASK(ace.ether.da.mask))
            {
                sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
                        ace.ether.da.addr[0], ace.ether.da.addr[1], ace.ether.da.addr[2],
                        ace.ether.da.addr[3], ace.ether.da.addr[4], ace.ether.da.addr[5]);
                json_object_set_new(ace_obj_p, "dstMac", json_string(mac_ar));
                sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
                        ace.ether.da.mask[0], ace.ether.da.mask[1], ace.ether.da.mask[2],
                        ace.ether.da.mask[3], ace.ether.da.mask[4], ace.ether.da.mask[5]);
                json_object_set_new(ace_obj_p, "dstMacMask", json_string(mac_ar));
            }

            if (ace.ether.ethertype.op == VAL_diffServMacAceEtherTypeOp_equal)
            {
                json_object_set_new(ace_obj_p, "etherType", json_integer(ace.ether.ethertype.u.s.data));
                json_object_set_new(ace_obj_p, "etherTypeMask", json_integer(ace.ether.ethertype.u.s.mask));
            }

            if (ace.ether.vid.op == VAL_diffServMacAceVidOp_equal)
            {
                json_object_set_new(ace_obj_p, "vid", json_integer(ace.ether.vid.u.s.data));
                json_object_set_new(ace_obj_p, "vidMask", json_integer(ace.ether.vid.u.s.mask));
            }

            json_array_append_new(aces_p, ace_obj_p);
        } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextUIAceByAcl(acl_index, &ace_index, &ace))

        json_array_append_new(acls_p, acl_obj_p);
    } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create MAC ACE entry.
 *
 * @param name (required, string) ACL name
 * @param aces (required, array)  ACEs
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *aces_p;
    json_t  *ace_p;
    const char *name_str_p;
    RULE_TYPE_UI_Ace_Entry_T ace;
    UI32_T acl_index = 0;
    int ace_num = 0, idx = 0;
    int parse_result = CGI_MODULE_ACL_PARSEACE_CODE_NONE;

    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }
    name_str_p = json_string_value(name_p);

    aces_p = CGI_REQUEST_GetBodyValue(http_request, "aces");

    if (NULL == aces_p) //create ACL name only
    {
        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, RULE_TYPE_MAC_ACL))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    //NULL != aces_p
    ace_num = json_array_size(aces_p);

    if (0 == ace_num)
    {
        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, RULE_TYPE_MAC_ACL))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    //validation
    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL != ace_p)
        {
            parse_result = CGI_MODULE_ACL_ValidMacAceJson(ace_p);

            if (CGI_MODULE_ACL_PARSEACE_CODE_NONE != parse_result)
            {
                return CGI_MODULE_ACL_ReturnErrorMsg(http_response, parse_result);
            }
        } //if (NULL != ace_p)
    } //for (idx = 0; idx < ace_num; idx ++)

    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL == ace_p)
        {
            continue;
        }

        CGI_MODULE_ACL_ConstructMacAce(ace_p, &ace);

        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, RULE_TYPE_MAC_ACL))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        if (RULE_TYPE_OK != L4_PMGR_ACL_SetUIAce2Acl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.SetUIAce2AclError", "Failed.");
        }
    } //for (idx = 0; idx < ace_num; idx ++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete all ACL.
 *
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    RULE_TYPE_UI_AclEntry_T acl_entry;
    UI32_T acl_index = 0;

    memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.acl_type = RULE_TYPE_MAC_ACL;

    while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))
    {
        if (RULE_TYPE_MAC_ACL != acl_entry.acl_type)
        {
            continue;
        }

        L4_PMGR_ACL_DelAclByNameAndAclType(acl_entry.acl_name, RULE_TYPE_MAC_ACL);
    } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update MAC ACE entry.
 *
 * @param id (required, string) ACL name
 * @param status (required, boolean) add or delete
 * @param aces (required, array)  ACEs
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *status_p;
    json_t  *aces_p;
    json_t  *ace_p;
    const char *name_str_p;
    RULE_TYPE_UI_Ace_Entry_T ace;
    UI32_T acl_index = 0;
    int ace_num = 0, idx = 0;
    int parse_result = CGI_MODULE_ACL_PARSEACE_CODE_NONE;

    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    aces_p = CGI_REQUEST_GetBodyValue(http_request, "aces");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (NULL == aces_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACEs.");
    }

    name_str_p = json_string_value(name_p);

    if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ACL does not exist.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    ace_num = json_array_size(aces_p);

    if (0 == ace_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "NULL ACEs.");
    }

    //validation
    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL != ace_p)
        {
            parse_result = CGI_MODULE_ACL_ValidMacAceJson(ace_p);

            if (CGI_MODULE_ACL_PARSEACE_CODE_NONE != parse_result)
            {
                return CGI_MODULE_ACL_ReturnErrorMsg(http_response, parse_result);
            }
        } //if (NULL != ace_p)
    } //for (idx = 0; idx < ace_num; idx ++)

    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL == ace_p)
        {
            continue;
        }

        CGI_MODULE_ACL_ConstructMacAce(ace_p, &ace);

        if (TRUE == json_boolean_value(status_p))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_SetUIAce2Acl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.SetUIAce2AclError", "Failed.");
            }
        }
        else
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_DelUIAceFromAcl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelUIAce2AclError", "Failed.");
            }
        }
    } //for (idx = 0; idx < ace_num; idx ++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_MAC_ID_Update

/**----------------------------------------------------------------------
 * This API is used to delete an ACL.
 *
 * @param id (required, string) ACL name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    const char *name_str_p;
    UI32_T acl_index = 0;

    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACL name.");
    }
    name_str_p = json_string_value(name_p);

    if (RULE_TYPE_OK == L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        if (RULE_TYPE_FAIL == L4_PMGR_ACL_DelAclByNameAndAclType(name_str_p, RULE_TYPE_MAC_ACL))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelAclByNameAndAclTypeError", "Failed.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

#if 0
/**----------------------------------------------------------------------
 * This API is used to delete an ACL.
 *
 * @param id (required, string) ACL name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_MAC_ID_ENTRY_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *entry_p;
    const char *name_str_p;
    char *entry_str_p;
    char *save_str_addr = NULL;
    char *action_str_p;
    char *sa_str_p;
    char *sa_mask_str_p;
    char *da_str_p;
    char *da_mask_str_p;
    char *etype_str_p;
    char *etype_mask_str_p;
    char *vid_str_p;
    char *vid_mask_str_p;
    char *end_p;
    RULE_TYPE_UI_Ace_Entry_T ace;

    memset(&ace, 0, sizeof(ace));
    ace.ace_type = RULE_TYPE_MAC_ACL;
    L4_PMGR_ACL_InitUIAce(&ace);
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACL name.");
    }
    name_str_p = json_string_value(name_p);

    entry_p = CGI_REQUEST_GetParamsValue(http_request, "entry");

    if (entry_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACL entry ID.");
    }
    entry_str_p = (char *) json_string_value(entry_p);
    action_str_p = strtok_r((char *)entry_str_p, "_", &save_str_addr);
    sa_str_p = strtok_r(NULL, "_", &save_str_addr);
    sa_mask_str_p = strtok_r(NULL, "_", &save_str_addr);
    da_str_p = strtok_r(NULL, "_", &save_str_addr);
    da_mask_str_p = strtok_r(NULL, "_", &save_str_addr);
    etype_str_p = strtok_r(NULL, "_", &save_str_addr);
    ace.ether.ethertype.u.s.data = strtol(etype_str_p, &end_p, 0);
    etype_mask_str_p = strtok_r(NULL, "_", &save_str_addr);
    ace.ether.ethertype.u.s.mask = strtol(etype_mask_str_p, &end_p, 0);
    vid_str_p = strtok_r(NULL, "_", &save_str_addr);
    ace.ether.vid.u.s.data = strtol(vid_str_p, &end_p, 0);
    vid_mask_str_p = strtok_r(NULL, "_", &save_str_addr);
    ace.ether.vid.u.s.mask = strtol(vid_mask_str_p, &end_p, 0);

    if (0 == strncmp(action_str_p, "deny", strlen("deny")))
    {
        ace.access = RULE_TYPE_ACE_DENY;
    }
    else if (0 == strncmp(action_str_p, "permit", strlen("permit")))
    {
        ace.access = RULE_TYPE_ACE_PERMIT;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid action.");
    }

    if (CGI_UTIL_MacStrToHex(sa_str_p, ace.ether.sa.addr) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source MAC format.");
    }

    if (CGI_UTIL_MacStrToHex(sa_mask_str_p, ace.ether.sa.mask) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source MAC mask format.");
    }

    if (CGI_UTIL_MacStrToHex(da_str_p, ace.ether.da.addr) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination MAC format.");
    }

    if (CGI_UTIL_MacStrToHex(da_mask_str_p, ace.ether.da.mask) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination MAC mask format.");
    }

    if (0 != ace.ether.ethertype.u.s.mask)
    {
        ace.ether.ethertype.op = VAL_diffServMacAceEtherTypeOp_equal;
    }

    if (0 != ace.ether.vid.u.s.mask)
    {
        ace.ether.vid.op = VAL_diffServMacAceVidOp_equal;
    }

    if (RULE_TYPE_FAIL == L4_PMGR_ACL_DelUIAceFromAcl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelUIAceFromAclError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}
#endif

/**----------------------------------------------------------------------
 * This API is used to get IP ACL entry.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_IP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *acls_p = json_array();
    RULE_TYPE_UI_AclEntry_T acl_entry;
    RULE_TYPE_UI_Ace_Entry_T ace;
    char ip_addr_ar[CGI_IPSTRLEN + 1] = {0};
    char ip_mask_ar[CGI_IPSTRLEN + 1] = {0};
    UI32_T acl_index = 0;
    UI32_T ace_index = 0;

    json_object_set_new(result_p, "acls", acls_p);
    memset(&acl_entry, 0, sizeof(acl_entry));

    acl_entry.acl_type = RULE_TYPE_IP_EXT_ACL;

    while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))
    {
        json_t *acl_obj_p = json_object();
        json_t *aces_p = json_array();

        if (RULE_TYPE_IP_EXT_ACL != acl_entry.acl_type)
        {
            continue;
        }

        ace_index = 0;
        memset(&ace, 0, sizeof(ace));
        json_object_set_new(acl_obj_p, "name", json_string(acl_entry.acl_name));
        json_object_set_new(acl_obj_p, "aces", aces_p);

        while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextUIAceByAcl(acl_index, &ace_index, &ace))
        {
            json_t *ace_obj_p = json_object();

            if (RULE_TYPE_ACE_PERMIT == ace.access)
            {
                json_object_set_new(ace_obj_p, "action", json_string("permit"));
            }
            else
            {
                json_object_set_new(ace_obj_p, "action", json_string("deny"));
            }

            L_INET_Ntop(AF_INET, ace.ipv4.sip.addr, ip_addr_ar, sizeof(ip_addr_ar));
            L_INET_Ntop(AF_INET, ace.ipv4.sip.mask, ip_mask_ar, sizeof(ip_mask_ar));
            json_object_set_new(ace_obj_p, "srcIp", json_string(ip_addr_ar));
            json_object_set_new(ace_obj_p, "srcIpMask", json_string(ip_mask_ar));

            L_INET_Ntop(AF_INET, ace.ipv4.dip.addr, ip_addr_ar, sizeof(ip_addr_ar));
            L_INET_Ntop(AF_INET, ace.ipv4.dip.mask, ip_mask_ar, sizeof(ip_mask_ar));
            json_object_set_new(ace_obj_p, "dstIp", json_string(ip_addr_ar));
            json_object_set_new(ace_obj_p, "dstIpMask", json_string(ip_mask_ar));

            if (RULE_TYPE_IPV4_PROTOCOL_OP_EQUAL == ace.ipv4.protocol.op)
            {
                json_object_set_new(ace_obj_p, "protocol", json_integer(ace.ipv4.protocol.u.s.data));
            }

            switch (ace.ipv4.dscp.op)
            {
                case RULE_TYPE_IPV4_DSCP_OP_TOS:
                    json_object_set_new(ace_obj_p, "tos", json_integer(ace.ipv4.dscp.u.tos.tos));
                    break;
                case RULE_TYPE_IPV4_DSCP_OP_PRECEDENCE:
                    json_object_set_new(ace_obj_p, "prec", json_integer(ace.ipv4.dscp.u.tos.precedence));
                    break;
                case RULE_TYPE_IPV4_DSCP_OP_DSCP:
                    json_object_set_new(ace_obj_p, "dscp", json_integer(ace.ipv4.dscp.u.ds));
                    break;
                case RULE_TYPE_IPV4_DSCP_OP_PRECEDENCE_AND_TOS:
                    json_object_set_new(ace_obj_p, "tos", json_integer(ace.ipv4.dscp.u.tos.tos));
                    json_object_set_new(ace_obj_p, "prec", json_integer(ace.ipv4.dscp.u.tos.precedence));
                    break;
                default:
                    break;
            }

            if (VAL_diffServIpAceSourcePortOp_equal == ace.l4_common.sport.op)
            {
                json_object_set_new(ace_obj_p, "srcPort", json_integer(ace.l4_common.sport.u.s.data));
                json_object_set_new(ace_obj_p, "srcPortMask", json_integer(ace.l4_common.sport.u.s.mask));
            }

            if (VAL_diffServIpAceDestPortOp_equal == ace.l4_common.dport.op)
            {
                json_object_set_new(ace_obj_p, "dstPort", json_integer(ace.l4_common.dport.u.s.data));
                json_object_set_new(ace_obj_p, "dstPortMask", json_integer(ace.l4_common.dport.u.s.mask));
            }

            if (0 != ace.tcp.flags.mask.u.code)
            {
                json_object_set_new(ace_obj_p, "ctrlFlag", json_integer(ace.tcp.flags.data.u.code));
                json_object_set_new(ace_obj_p, "ctrlFlagMask", json_integer(ace.tcp.flags.mask.u.code));
            }

            json_array_append_new(aces_p, ace_obj_p);
        } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextUIAceByAcl(acl_index, &ace_index, &ace))

        json_array_append_new(acls_p, acl_obj_p);
    } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_IP_Read

/**----------------------------------------------------------------------
 * This API is used to get IP ACL entry.
 *
 * @param name (required, string) ACL name
 * @param type (required, string) type
 * @param aces (required, array)  ACEs
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_IP_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *type_p;
    json_t  *aces_p;
    json_t  *ace_p;
    const char *name_str_p;
    const char *type_str_p;
    RULE_TYPE_UI_Ace_Entry_T ace;
    RULE_TYPE_AclType_T type = RULE_TYPE_IP_STD_ACL;
    UI32_T acl_index = 0;
    int ace_num = 0, idx = 0;
    int parse_result = CGI_MODULE_ACL_PARSEACE_CODE_NONE;

    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    type_p = CGI_REQUEST_GetBodyValue(http_request, "type");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (type_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get type.");
    }

    name_str_p = json_string_value(name_p);
    type_str_p = json_string_value(type_p);

    if (0 == strncmp(type_str_p, "standard", strlen("standard")))
    {
        type = RULE_TYPE_IP_STD_ACL;
    }
    else if (0 == strncmp(type_str_p, "extended", strlen("extended")))
    {
        type = RULE_TYPE_IP_EXT_ACL;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP type.");
    }

    aces_p = CGI_REQUEST_GetBodyValue(http_request, "aces");

    if (NULL == aces_p) //create ACL name only
    {
        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, type))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    //NULL != aces_p
    ace_num = json_array_size(aces_p);

    if (0 == ace_num)
    {
        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, type))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    //validation
    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL != ace_p)
        {
            parse_result = CGI_MODULE_ACL_ValidIpAceJson(ace_p, type);

            if (CGI_MODULE_ACL_PARSEACE_CODE_NONE != parse_result)
            {
                return CGI_MODULE_ACL_ReturnErrorMsg(http_response, parse_result);
            }
        } //if (NULL != ace_p)
    } //for (idx = 0; idx < ace_num; idx ++)

    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL == ace_p)
        {
            continue;
        }

        CGI_MODULE_ACL_ConstructIpAce(ace_p, type, &ace);

        if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_CreateAcl(name_str_p, type))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAclError", "Failed.");
            }
        }

        if (RULE_TYPE_OK != L4_PMGR_ACL_SetUIAce2Acl(name_str_p, type, &ace))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.SetUIAce2AclError", "Failed.");
        }
    } //for (idx = 0; idx < ace_num; idx ++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_IP_Create

/**----------------------------------------------------------------------
 * This API is used to delete all ACL.
 *
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_IP_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    RULE_TYPE_UI_AclEntry_T acl_entry;
    UI32_T acl_index = 0;

    memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.acl_type = RULE_TYPE_IP_EXT_ACL;

    while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))
    {
        if (RULE_TYPE_IP_EXT_ACL != acl_entry.acl_type)
        {
            continue;
        }

        L4_PMGR_ACL_DelAclByNameAndAclType(acl_entry.acl_name, RULE_TYPE_IP_EXT_ACL);
    } //while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAcl(&acl_index, &acl_entry))

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update IP ACE entry.
 *
 * @param id (required, string) ACL name
 * @param type (required, string) type
 * @param status (required, boolean) add or delete
 * @param aces (required, array)  ACEs
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_IP_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *type_p;
    json_t  *status_p;
    json_t  *aces_p;
    json_t  *ace_p;
    const char *name_str_p;
    const char *type_str_p;
    RULE_TYPE_UI_Ace_Entry_T ace;
    RULE_TYPE_AclType_T type = RULE_TYPE_IP_STD_ACL;
    UI32_T acl_index = 0;
    int ace_num = 0, idx = 0;
    int parse_result = CGI_MODULE_ACL_PARSEACE_CODE_NONE;

    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    type_p = CGI_REQUEST_GetBodyValue(http_request, "type");
    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    aces_p = CGI_REQUEST_GetBodyValue(http_request, "aces");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (NULL == aces_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACEs.");
    }

    name_str_p = json_string_value(name_p);

    if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ACL does not exist.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    type_str_p = json_string_value(type_p);

    if (0 == strncmp(type_str_p, "standard", strlen("standard")))
    {
        type = RULE_TYPE_IP_STD_ACL;
    }
    else if (0 == strncmp(type_str_p, "extended", strlen("extended")))
    {
        type = RULE_TYPE_IP_EXT_ACL;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP type.");
    }

    ace_num = json_array_size(aces_p);

    if (0 == ace_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "NULL ACEs.");
    }

    //validation
    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL != ace_p)
        {
            parse_result = CGI_MODULE_ACL_ValidIpAceJson(ace_p, type);

            if (CGI_MODULE_ACL_PARSEACE_CODE_NONE != parse_result)
            {
                return CGI_MODULE_ACL_ReturnErrorMsg(http_response, parse_result);
            }
        } //if (NULL != ace_p)
    } //for (idx = 0; idx < ace_num; idx ++)

    for (idx = 0; idx < ace_num; idx ++)
    {
        ace_p = json_array_get(aces_p, idx);

        if (NULL == ace_p)
        {
            continue;
        }

        CGI_MODULE_ACL_ConstructIpAce(ace_p, type, &ace);

        if (TRUE == json_boolean_value(status_p))
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_SetUIAce2Acl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.SetUIAce2AclError", "Failed.");
            }
        }
        else
        {
            if (RULE_TYPE_OK != L4_PMGR_ACL_DelUIAceFromAcl(name_str_p, RULE_TYPE_MAC_ACL, &ace))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelUIAce2AclError", "Failed.");
            }
        }
    } //for (idx = 0; idx < ace_num; idx ++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_IP_ID_Update

/**----------------------------------------------------------------------
 * This API is used to delete an ACL.
 *
 * @param id (required, string) ACL name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_IP_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    const char *name_str_p;
    UI32_T acl_index = 0;
    UI32_T delete_std = 0;
    UI32_T delete_ext = 0;

    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ACL name.");
    }

    name_str_p = json_string_value(name_p);

    if (RULE_TYPE_OK == L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        /* not know type of IP ACL, remove two types
         */
        delete_std = L4_PMGR_ACL_DelAclByNameAndAclType(name_str_p, RULE_TYPE_IP_STD_ACL);
        delete_ext = L4_PMGR_ACL_DelAclByNameAndAclType(name_str_p, RULE_TYPE_IP_EXT_ACL);

        if ((RULE_TYPE_OK != delete_std) && (RULE_TYPE_OK != delete_ext))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelAclByNameAndAclTypeError", "Failed.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_IP_ID_Delete

/**----------------------------------------------------------------------
 * This API is used to get IP ACL entry.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_INTF_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *binds_p = json_array();
    RULE_TYPE_UI_AclEntry_T acl_entry;
    RULE_TYPE_COUNTER_ENABLE_T counter_enable = RULE_TYPE_COUNTER_ENABLE;
    UI32_T pmax = 0, unit = 0, port = 0, ifindex = 0;
    UI32_T acl_index = 0, precedence = 0;
    UI16_T time_range_index = 0;

    json_object_set_new(result_p, "bindings", binds_p);

    STKTPLG_POM_GetNextUnit(&unit);
    pmax = SWCTRL_POM_UIGetUnitPortNumber(unit);

    for (port = 1; port <= pmax; port++)
    {
        SWCTRL_POM_UserPortToIfindex(unit, port, &ifindex);

        memset(&acl_entry, 0, sizeof(acl_entry));
        acl_index = 0;
        precedence = 0;
        time_range_index = RULE_TYPE_UNDEF_TIME_RANGE;

        while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAclByPort(ifindex, RULE_TYPE_INBOUND,
                &acl_index, &acl_entry, &precedence, &time_range_index, &counter_enable))
        {
            json_t *bind_obj_p = json_object();

            CGI_MODULE_ACL_GetOneInterfaceBind(ifindex, &acl_entry, RULE_TYPE_INBOUND, bind_obj_p);
            json_array_append_new(binds_p, bind_obj_p);
        } //L4_PMGR_ACL_GetNextAclByPort: RULE_TYPE_INBOUND

        memset(&acl_entry, 0, sizeof(acl_entry));
        acl_index = 0;
        precedence = 0;
        time_range_index = RULE_TYPE_UNDEF_TIME_RANGE;

        while (RULE_TYPE_OK == L4_PMGR_ACL_GetNextAclByPort(ifindex, RULE_TYPE_OUTBOUND,
                &acl_index, &acl_entry, &precedence, &time_range_index, &counter_enable))
        {
            json_t *bind_obj_p = json_object();

            CGI_MODULE_ACL_GetOneInterfaceBind(ifindex, &acl_entry, RULE_TYPE_OUTBOUND, bind_obj_p);
            json_array_append_new(binds_p, bind_obj_p);
        } //L4_PMGR_ACL_GetNextAclByPort: RULE_TYPE_OUTBOUND

    } //for (port =1; port <= pmax; port++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_INTF_Read

/**----------------------------------------------------------------------
 * This API is used to bind ACL entry.
 *
 * @param ifId (required, string) interface name
 * @param name (required, string) ACL name
 * @param type (required, string) ACL type
 * @param direction (required, string) direction
 * @param counter (optional, boolean) counter enable
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_INTF_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ifId_p;
    json_t  *name_p;
    json_t  *type_p;
    json_t  *direction_p;
    json_t  *counter_p;
    const char *ifid_str_p;
    const char *name_str_p;
    const char *type_str_p;
    const char *dir_str_p;
    RULE_TYPE_AclType_T acl_type = RULE_TYPE_MAC_ACL;
    RULE_TYPE_COUNTER_ENABLE_T counter_enable = RULE_TYPE_COUNTER_DISABLE;
    UI32_T acl_index = 0;
    UI32_T ifindex = 0;
    BOOL_T ingress_flag = FALSE;

    ifId_p = CGI_REQUEST_GetBodyValue(http_request, "ifId");
    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    type_p = CGI_REQUEST_GetBodyValue(http_request, "type");
    direction_p = CGI_REQUEST_GetBodyValue(http_request, "direction");
    counter_p = CGI_REQUEST_GetBodyValue(http_request, "counter");

    if (ifId_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
    }

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }

    if (type_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get type.");
    }

    if (direction_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get direction.");
    }

    ifid_str_p = json_string_value(ifId_p);
    name_str_p = json_string_value(name_p);
    type_str_p = json_string_value(type_p);
    dir_str_p = json_string_value(direction_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)ifid_str_p, &ifindex))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

    if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ACL name.");
    }

    if (TRUE != CGI_MODULE_ACL_GetAclType(type_str_p, &acl_type))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid type.");
    }

    if (TRUE != CGI_MODULE_ACL_GetAclDirection(dir_str_p, &ingress_flag))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid direction.");
    }

    if (NULL != counter_p)
    {
        if (TRUE != json_is_boolean(counter_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "counter is not boolean value.");
        }

        if (TRUE == json_boolean_value(counter_p))
        {
            counter_enable = RULE_TYPE_COUNTER_ENABLE;
        }
    }

    if (RULE_TYPE_OK != L4_PMGR_ACL_BindPort2Acl(ifindex,
            name_str_p, acl_type, ingress_flag, NULL, counter_enable))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.BindPort2AclError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_INTF_Create

/**----------------------------------------------------------------------
 * This API is used to unbind ACL entry.
 *
 * @param ifId (required, string) interface name
 * @param id (required, string) ACL name
 * @param type (required, string) ACL type
 * @param direction (required, string) direction
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ACL_INTF_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ifId_p;
    json_t  *name_p;
    json_t  *type_p;
    json_t  *direction_p;
    const char *ifid_str_p;
    const char *name_str_p;
    const char *type_str_p;
    const char *dir_str_p;
    RULE_TYPE_AclType_T acl_type = RULE_TYPE_MAC_ACL;
    UI8_T  port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    UI32_T acl_index = 0;
    UI32_T ifindex = 0;
    BOOL_T ingress_flag = FALSE;

    ifId_p = CGI_REQUEST_GetParamsValue(http_request, "ifId");
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    type_p = CGI_REQUEST_GetParamsValue(http_request, "type");
    direction_p = CGI_REQUEST_GetParamsValue(http_request, "direction");

    ifid_str_p = json_string_value(ifId_p);
    name_str_p = json_string_value(name_p);
    type_str_p = json_string_value(type_p);
    dir_str_p = json_string_value(direction_p);

    CGI_UTIL_UrlDecode(port_ar, ifid_str_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(port_ar, &ifindex))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ifId.");
    }

    if (TRUE != CGI_MODULE_ACL_GetAclType(type_str_p, &acl_type))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid type.");
    }

    if (TRUE != CGI_MODULE_ACL_GetAclDirection(dir_str_p, &ingress_flag))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid direction.");
    }

    if (RULE_TYPE_OK != L4_PMGR_ACL_GetAclIdByName(name_str_p, &acl_index))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    if (RULE_TYPE_OK != L4_PMGR_ACL_UnBindPortFromAcl(ifindex, name_str_p, acl_type, ingress_flag))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.unBindPort2AclError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_ACL_INTF_ID_Delete

static int CGI_MODULE_ACL_ValidMacAceJson(json_t *ace_json_p)
{
    json_t *action_p = json_object_get(ace_json_p, "action");
    json_t *pkt_p = json_object_get(ace_json_p, "pktFormat");
    json_t *sa_p = json_object_get(ace_json_p, "srcMac");
    json_t *sa_mask_p = json_object_get(ace_json_p, "srcMacMask");
    json_t *da_p = json_object_get(ace_json_p, "dstMac");
    json_t *da_mask_p = json_object_get(ace_json_p, "dstMacMask");
    json_t *vid_p = json_object_get(ace_json_p, "vid");
    json_t *vid_mask_p = json_object_get(ace_json_p, "vidMask");
    json_t *etype_p = json_object_get(ace_json_p, "etherType");
    json_t *etype_mask_p = json_object_get(ace_json_p, "etherTypeMask");
    const char *action_str_p;
    const char *pkt_str_p;
    const char *sa_str_p;
    const char *sa_mask_str_p;
    const char *da_str_p;
    const char *da_mask_str_p;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    if (action_p == NULL)
    {
        return CGI_MODULE_ACL_PARSEACE_CODE_ACTION;
    }

    action_str_p = json_string_value(action_p);

    if ((0 != strncmp(action_str_p, "deny", strlen("deny")))
            && (0 != strncmp(action_str_p, "permit", strlen("permit"))))
    {
        return CGI_MODULE_ACL_PARSEACE_CODE_ACTION;
    }

    if (pkt_p != NULL)
    {
        pkt_str_p = json_string_value(pkt_p);

        if ((0 != strncmp(pkt_str_p, "any", strlen("any")))
                && (0 != strncmp(pkt_str_p, "tagged-eth2", strlen("tagged-eth2")))
                && (0 != strncmp(pkt_str_p, "tagged-802.3", strlen("tagged-802.3")))
                && (0 != strncmp(pkt_str_p, "untagged-eth2", strlen("untagged-eth2")))
                && (0 != strncmp(pkt_str_p, "untagged-802.3", strlen("untagged-802.3"))))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_PKTFORMAT;
        }
    }

    if (sa_p != NULL)
    {
        sa_str_p = json_string_value(sa_p);

        if (CGI_UTIL_MacStrToHex(sa_str_p, mac_ar) == FALSE)
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_SA;
        }

        if (sa_mask_p != NULL)
        {
            sa_mask_str_p = json_string_value(sa_mask_p);

            if (CGI_UTIL_MacStrToHex(sa_mask_str_p, mac_ar) == FALSE)
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_SAMASK;
            }
        }
    } //if (sa_p != NULL)

    if (da_p != NULL)
    {
        da_str_p = json_string_value(da_p);

        if (CGI_UTIL_MacStrToHex(da_str_p, mac_ar) == FALSE)
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_DA;
        }

        if (da_mask_p != NULL)
        {
            da_mask_str_p = json_string_value(da_mask_p);

            if (CGI_UTIL_MacStrToHex(da_mask_str_p, mac_ar) == FALSE)
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_DAMASK;
            }
        }
    } //if (da_p != NULL)

    if (vid_p != NULL)
    {
        if (SYS_ADPT_MAX_VLAN_ID < json_integer_value(vid_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_VID;
        }
    } //if (vid_p != NULL)

    return CGI_MODULE_ACL_PARSEACE_CODE_NONE;
} //CGI_MODULE_ACL_ValidMacAceJson

static int CGI_MODULE_ACL_ValidIpAceJson(json_t *ace_json_p, RULE_TYPE_AclType_T type)
{
    json_t *action_p = json_object_get(ace_json_p, "action");
    json_t *sip_p = json_object_get(ace_json_p, "srcIp");
    json_t *sip_mask_p = json_object_get(ace_json_p, "srcIpMask");
    json_t *dip_p = json_object_get(ace_json_p, "dstIp");
    json_t *dip_mask_p = json_object_get(ace_json_p, "dstIpMask");
    json_t *protocol_p = json_object_get(ace_json_p, "protocol");
    json_t *prec_p = json_object_get(ace_json_p, "prec");
    json_t *dscp_p = json_object_get(ace_json_p, "dscp");
    json_t *sport_p = json_object_get(ace_json_p, "srcPort");
    json_t *sport_mask_p = json_object_get(ace_json_p, "srcPortMask");
    json_t *dport_p = json_object_get(ace_json_p, "dstPort");
    json_t *dport_mask_p = json_object_get(ace_json_p, "dstPortMask");
    json_t *ctrl_p = json_object_get(ace_json_p, "ctrlFlag");
    json_t *ctrl_mask_p = json_object_get(ace_json_p, "ctrlFlagMask");
    const char *action_str_p;
    const char *sip_str_p;
    const char *sip_mask_str_p;
    const char *dip_str_p;
    const char *dip_mask_str_p;
    UI32_T tmp_ip = 0;

    if (action_p == NULL)
    {
        return CGI_MODULE_ACL_PARSEACE_CODE_ACTION;
    }

    action_str_p = json_string_value(action_p);

    if ((0 != strncmp(action_str_p, "deny", strlen("deny")))
            && (0 != strncmp(action_str_p, "permit", strlen("permit"))))
    {
        return CGI_MODULE_ACL_PARSEACE_CODE_ACTION;
    }

    if (NULL != sip_p)
    {
        sip_str_p = json_string_value(sip_p);

        if (TRUE != CGI_UTIL_IpStrToInt(sip_str_p, &tmp_ip))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_SIP;
        }

        if (sip_mask_p != NULL)
        {
            sip_mask_str_p = json_string_value(sip_mask_p);

            if (TRUE != CGI_UTIL_IpStrToInt(sip_mask_str_p, &tmp_ip))
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_SIPMASK;
            }
        }
    } //if (NULL != sip_p)

    if (RULE_TYPE_IP_STD_ACL == type)
    {
        if ((NULL != dip_p) || (NULL != protocol_p) || (NULL != prec_p) || (NULL != dscp_p)
                || (NULL != sport_p) || (NULL != dport_p) || (NULL != ctrl_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_STDIP;
        }
    }

    if (NULL != dip_p)
    {
        dip_str_p = json_string_value(dip_p);

        if (TRUE != CGI_UTIL_IpStrToInt(dip_str_p, &tmp_ip))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_DIP;
        }

        if (dip_mask_p != NULL)
        {
            dip_mask_str_p = json_string_value(dip_mask_p);

            if (TRUE != CGI_UTIL_IpStrToInt(dip_mask_str_p, &tmp_ip))
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_DIPMASK;
            }
        }
    } //if (NULL != dip_p)

    if (NULL != protocol_p)
    {
        if (RULE_TYPE_MAX_OF_IP_PROTOCOL < json_integer_value(protocol_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_PROTOCOL;
        }
    } //if (NULL != protocol_p)

    if (NULL != prec_p)
    {
        if (RULE_TYPE_MAX_OF_IP_PRECEDENCE < json_integer_value(prec_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_PREC;
        }
    } //if (NULL != prec_p)

    if (NULL != dscp_p)
    {
        if (RULE_TYPE_MAX_OF_IP_DSCP < json_integer_value(dscp_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_DSCP;
        }
    } //if (NULL != dscp_p)

    if (NULL != sport_p)
    {
        if (MAX_diffServIpAceMaxSourcePort < json_integer_value(sport_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_SPORT;
        }

        if (sport_mask_p != NULL)
        {
            if (MAX_diffServIpAceSourcePortBitmask < json_integer_value(sport_mask_p))
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_SPORTMASK;
            }
        }
    } //if (NULL != sport_p)

    if (NULL != dport_p)
    {
        if (MAX_diffServIpAceMaxDestPort < json_integer_value(dport_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_DPORT;
        }

        if (dport_mask_p != NULL)
        {
            if (MAX_diffServIpAceDestPortBitmask < json_integer_value(dport_mask_p))
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_DPORTMASK;
            }
        }
    } //if (NULL != dport_p)

    if (NULL != ctrl_p)
    {
        if (RULE_TYPE_MAX_OF_IP_CONTROL_CODE < json_integer_value(ctrl_p))
        {
            return CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAG;
        }

        if (ctrl_mask_p != NULL)
        {
            if (MAX_diffServIpAceControlCodeBitmask < json_integer_value(ctrl_mask_p))
            {
                return CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAGMASK;
            }
        }
    } //if (NULL != ctrl_p)

    return CGI_MODULE_ACL_PARSEACE_CODE_NONE;
} //CGI_MODULE_ACL_ValidIpAceJson

static void CGI_MODULE_ACL_ConstructMacAce(json_t *ace_json_p, RULE_TYPE_UI_Ace_Entry_T *ace_p)
{
    json_t *action_p = json_object_get(ace_json_p, "action");
    json_t *pkt_p = json_object_get(ace_json_p, "pktFormat");
    json_t *sa_p = json_object_get(ace_json_p, "srcMac");
    json_t *sa_mask_p = json_object_get(ace_json_p, "srcMacMask");
    json_t *da_p = json_object_get(ace_json_p, "dstMac");
    json_t *da_mask_p = json_object_get(ace_json_p, "dstMacMask");
    json_t *vid_p = json_object_get(ace_json_p, "vid");
    json_t *vid_mask_p = json_object_get(ace_json_p, "vidMask");
    json_t *etype_p = json_object_get(ace_json_p, "etherType");
    json_t *etype_mask_p = json_object_get(ace_json_p, "etherTypeMask");
    const char *action_str_p;
    const char *pkt_str_p;
    const char *sa_str_p;
    const char *sa_mask_str_p;
    const char *da_str_p;
    const char *da_mask_str_p;

    memset(ace_p, 0, sizeof(RULE_TYPE_UI_Ace_Entry_T));
    ace_p->ace_type = RULE_TYPE_MAC_ACL;
    L4_PMGR_ACL_InitUIAce(ace_p);

    action_str_p = json_string_value(action_p);

    if (0 == strncmp(action_str_p, "deny", strlen("deny")))
    {
        ace_p->access = RULE_TYPE_ACE_DENY;
    }
    else if (0 == strncmp(action_str_p, "permit", strlen("permit")))
    {
        ace_p->access = RULE_TYPE_ACE_PERMIT;
    }

    if (pkt_p != NULL)
    {
        pkt_str_p = json_string_value(pkt_p);

        if (0 == strncmp(pkt_str_p, "any", strlen("any")))
        {
            ace_p->pkt_format = VAL_diffServMacAcePktformat_any;
        }
        else if (0 == strncmp(pkt_str_p, "tagged-eth2", strlen("tagged-eth2")))
        {
            ace_p->pkt_format = VAL_diffServMacAcePktformat_tagggedEth2;
        }
        else if (0 == strncmp(pkt_str_p, "tagged-802.3", strlen("tagged-802.3")))
        {
            ace_p->pkt_format = VAL_diffServMacAcePktformat_tagged802Dot3;
        }
        else if (0 == strncmp(pkt_str_p, "untagged-eth2", strlen("untagged-eth2")))
        {
            ace_p->pkt_format = VAL_diffServMacAcePktformat_untagged_Eth2;
        }
        else if (0 == strncmp(pkt_str_p, "untagged-802.3", strlen("untagged-802.3")))
        {
            ace_p->pkt_format = VAL_diffServMacAcePktformat_untagged802Dot3;
        }
    }

    if (sa_p != NULL)
    {
        sa_str_p = json_string_value(sa_p);
        CGI_UTIL_MacStrToHex(sa_str_p, ace_p->ether.sa.addr);
        memset(ace_p->ether.sa.mask, 0xff, sizeof(ace_p->ether.sa.mask));

        if (sa_mask_p != NULL)
        {
            sa_mask_str_p = json_string_value(sa_mask_p);
            CGI_UTIL_MacStrToHex(sa_mask_str_p, ace_p->ether.sa.mask);
        }
    } //if (sa_p != NULL)

    if (da_p != NULL)
    {
        da_str_p = json_string_value(da_p);
        CGI_UTIL_MacStrToHex(da_str_p, ace_p->ether.da.addr);
        memset(ace_p->ether.da.mask, 0xff, sizeof(ace_p->ether.da.mask));

        if (da_mask_p != NULL)
        {
            da_mask_str_p = json_string_value(da_mask_p);
            CGI_UTIL_MacStrToHex(da_mask_str_p, ace_p->ether.da.mask);
        }
    } //if (da_p != NULL)

    if (vid_p != NULL)
    {
        ace_p->ether.vid.u.s.data = json_integer_value(vid_p);
        ace_p->ether.vid.u.s.mask = MAX_diffServMacAceVidBitmask;
        ace_p->ether.vid.op = VAL_diffServMacAceVidOp_equal;

        if (vid_mask_p != NULL)
        {
            ace_p->ether.vid.u.s.mask = json_integer_value(vid_mask_p);
        }
    } //if (vid_p != NULL)

    if (etype_p != NULL)
    {
        ace_p->ether.ethertype.u.s.data = json_integer_value(etype_p);
        ace_p->ether.ethertype.u.s.mask = MAX_diffServMacAceEtherTypeBitmask;
        ace_p->ether.ethertype.op = VAL_diffServMacAceEtherTypeOp_equal;

        if (etype_mask_p != NULL)
        {
            ace_p->ether.ethertype.u.s.mask = json_integer_value(etype_mask_p);
        }
    }
} //CGI_MODULE_ACL_ConstructMacAce

static void CGI_MODULE_ACL_ConstructIpAce(json_t *ace_json_p, RULE_TYPE_AclType_T type, RULE_TYPE_UI_Ace_Entry_T *ace_p)
{
    json_t *action_p = json_object_get(ace_json_p, "action");
    json_t *sip_p = json_object_get(ace_json_p, "srcIp");
    json_t *sip_mask_p = json_object_get(ace_json_p, "srcIpMask");
    json_t *dip_p = json_object_get(ace_json_p, "dstIp");
    json_t *dip_mask_p = json_object_get(ace_json_p, "dstIpMask");
    json_t *protocol_p = json_object_get(ace_json_p, "protocol");
    json_t *prec_p = json_object_get(ace_json_p, "prec");
    json_t *dscp_p = json_object_get(ace_json_p, "dscp");
    json_t *sport_p = json_object_get(ace_json_p, "srcPort");
    json_t *sport_mask_p = json_object_get(ace_json_p, "srcPortMask");
    json_t *dport_p = json_object_get(ace_json_p, "dstPort");
    json_t *dport_mask_p = json_object_get(ace_json_p, "dstPortMask");
    json_t *ctrl_p = json_object_get(ace_json_p, "ctrlFlag");
    json_t *ctrl_mask_p = json_object_get(ace_json_p, "ctrlFlagMask");
    const char *action_str_p;
    const char *sip_str_p;
    const char *sip_mask_str_p;
    const char *dip_str_p;
    const char *dip_mask_str_p;

    memset(ace_p, 0, sizeof(RULE_TYPE_UI_Ace_Entry_T));
    ace_p->ace_type = type;
    L4_PMGR_ACL_InitUIAce(ace_p);

    action_str_p = json_string_value(action_p);

    if (0 == strncmp(action_str_p, "deny", strlen("deny")))
    {
        ace_p->access = RULE_TYPE_ACE_DENY;
    }
    else if (0 == strncmp(action_str_p, "permit", strlen("permit")))
    {
        ace_p->access = RULE_TYPE_ACE_PERMIT;
    }

    if (NULL != sip_p)
    {
        sip_str_p = json_string_value(sip_p);
        L_INET_Pton(L_INET_AF_INET, (char *)sip_str_p, ace_p->ipv4.sip.addr);
        memset(ace_p->ipv4.sip.mask, 0xff, sizeof(ace_p->ipv4.sip.mask));

        if (sip_mask_p != NULL)
        {
            sip_mask_str_p = json_string_value(sip_mask_p);
            L_INET_Pton(L_INET_AF_INET, (char *)sip_mask_str_p, ace_p->ipv4.sip.mask);
        }
    }

    if (RULE_TYPE_IP_STD_ACL == type)
    {
        return; //standard only support src ip
    }

    if (NULL != dip_p)
    {
        dip_str_p = json_string_value(dip_p);
        L_INET_Pton(L_INET_AF_INET, (char *)dip_str_p, ace_p->ipv4.dip.addr);
        memset(ace_p->ipv4.dip.mask, 0xff, sizeof(ace_p->ipv4.dip.mask));

        if (dip_mask_p != NULL)
        {
            dip_mask_str_p = json_string_value(dip_mask_p);
            L_INET_Pton(L_INET_AF_INET, (char *)dip_mask_str_p, ace_p->ipv4.dip.mask);
        }
    }

    if (NULL != protocol_p)
    {
        ace_p->ipv4.protocol.op = RULE_TYPE_IPV4_PROTOCOL_OP_EQUAL;
        ace_p->ipv4.protocol.u.s.data = json_integer_value(protocol_p);
        ace_p->ipv4.protocol.u.s.mask = RULE_TYPE_MAX_OF_IP_PROTOCOL;
    }

#if (SYS_CPNT_ACL_IP_EXT_DSCP == TRUE)
    if (NULL != dscp_p)
    {
        ace_p->ipv4.dscp.op = RULE_TYPE_IPV4_DSCP_OP_DSCP;
        ace_p->ipv4.dscp.u.ds = json_integer_value(dscp_p);
    }
#endif

#if (SYS_CPNT_ACL_IP_EXT_PREC == TRUE)
    if (NULL != prec_p)
    {
        ace_p->ipv4.dscp.op = RULE_TYPE_IPV4_DSCP_OP_PRECEDENCE;
        ace_p->ipv4.dscp.u.tos.precedence = json_integer_value(prec_p);
    }
#endif

    if (NULL != sport_p)
    {
        ace_p->l4_common.sport.op = VAL_diffServIpAceSourcePortOp_equal;
        ace_p->l4_common.sport.u.s.data = json_integer_value(sport_p);
        ace_p->l4_common.sport.u.s.mask = MAX_diffServIpAceSourcePortBitmask;

        if (sport_mask_p != NULL)
        {
            ace_p->l4_common.sport.u.s.mask = json_integer_value(sport_mask_p);
        }
    } //if (NULL != ctrl_p)

    if (NULL != dport_p)
    {
        ace_p->l4_common.dport.op = VAL_diffServIpAceDestPortOp_equal;
        ace_p->l4_common.dport.u.s.data = json_integer_value(dport_p);
        ace_p->l4_common.dport.u.s.mask = MAX_diffServIpAceDestPortBitmask;

        if (dport_mask_p != NULL)
        {
            ace_p->l4_common.dport.u.s.mask = json_integer_value(dport_mask_p);
        }
    } //if (NULL != ctrl_p)

    if (NULL != ctrl_p)
    {
        ace_p->tcp.flags.data.u.code = json_integer_value(ctrl_p);
        ace_p->tcp.flags.mask.u.code = MAX_diffServIpAceControlCodeBitmask;

        if (ctrl_mask_p != NULL)
        {
            ace_p->tcp.flags.mask.u.code = json_integer_value(ctrl_mask_p);
        }
    } //if (NULL != ctrl_p)
} //CGI_MODULE_ACL_ConstructIpAce

static void CGI_MODULE_ACL_GetOneInterfaceBind(UI32_T ifindex,
        RULE_TYPE_UI_AclEntry_T *acl_p, int direction, json_t *bind_obj_p)
{
    UI8_T  port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};

    CGI_UTIL_IfindexToRestPortStr(ifindex, port_ar);
    json_object_set_new(bind_obj_p, "ifId", json_string(port_ar));
    json_object_set_new(bind_obj_p, "name", json_string(acl_p->acl_name));

    if (RULE_TYPE_MAC_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("mac"));
    }
    else if (RULE_TYPE_IP_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ip"));
    }
    else if (RULE_TYPE_IP_STD_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ip-standard"));
    }
    else if (RULE_TYPE_IP_EXT_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ip-extended"));
    }
    else if (RULE_TYPE_IPV6_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ipv6"));
    }
    else if (RULE_TYPE_IPV6_STD_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ipv6-standard"));
    }
    else if (RULE_TYPE_IPV6_EXT_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("ipv6-extended"));
    }
#if (SYS_CPNT_DAI == TRUE)
    else if (RULE_TYPE_ARP_ACL == acl_p->acl_type)
    {
        json_object_set_new(bind_obj_p, "type", json_string("arp"));
    }
#endif
    else
    {
        json_object_set_new(bind_obj_p, "type", json_string("unknown"));
    }

    if (RULE_TYPE_INBOUND == direction)
    {
        json_object_set_new(bind_obj_p, "direction", json_string("in"));
    }
    else
    {
        json_object_set_new(bind_obj_p, "direction", json_string("out"));
    }
} //CGI_MODULE_ACL_GetOneInterfaceBind

static BOOL_T CGI_MODULE_ACL_GetAclType(const char *type_str_p, RULE_TYPE_AclType_T *acl_type_p)
{
    if (0 == strncmp(type_str_p, "mac", strlen("mac")))
    {
        *acl_type_p = RULE_TYPE_MAC_ACL;
    }
    else if (0 == strncmp(type_str_p, "ip-standard", strlen("ip-standard")))
    {
        *acl_type_p = RULE_TYPE_IP_STD_ACL;
    }
    else if (0 == strncmp(type_str_p, "ip-extended", strlen("ip-extended")))
    {
        *acl_type_p = RULE_TYPE_IP_EXT_ACL;
    }
    else if (0 == strncmp(type_str_p, "ipv6-standard", strlen("ipv6-standard")))
    {
        *acl_type_p = RULE_TYPE_IPV6_STD_ACL;
    }
    else if (0 == strncmp(type_str_p, "ipv6-extended", strlen("ipv6-extended")))
    {
        *acl_type_p = RULE_TYPE_IPV6_EXT_ACL;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
} //CGI_MODULE_ACL_GetAclType

static BOOL_T CGI_MODULE_ACL_GetAclDirection(const char *dir_str_p, BOOL_T *ingress_flag_p)
{
    if (0 == strncmp(dir_str_p, "in", strlen("in")))
    {
        *ingress_flag_p = TRUE;
    }
    else if (0 == strncmp(dir_str_p, "out", strlen("out")))
    {
        *ingress_flag_p = FALSE;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
} //CGI_MODULE_ACL_GetAclDirection

static CGI_STATUS_CODE_T CGI_MODULE_ACL_ReturnErrorMsg(HTTP_Response_T *http_response, int error_code)
{
    switch(error_code)
    {
        case CGI_MODULE_ACL_PARSEACE_CODE_ACTION:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid action.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SA:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source MAC.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SAMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source MAC mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DA:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination MAC.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DAMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination MAC mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_VID:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid VLAN ID.");

        case CGI_MODULE_ACL_PARSEACE_CODE_VIDMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid VLAN ID mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_ETYPE:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ether type.");

        case CGI_MODULE_ACL_PARSEACE_CODE_ETYPEMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ether type mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_STDIP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Standard IP ACE only support source IP.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SIP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source IP.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SIPMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source IP mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DIP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination IP.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DIPMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination IP mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_PROTOCOL:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid protocol.");

        case CGI_MODULE_ACL_PARSEACE_CODE_PREC:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid precedence.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DSCP:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid DSCP.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SPORT:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port.");

        case CGI_MODULE_ACL_PARSEACE_CODE_SPORTMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DPORT:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination port.");

        case CGI_MODULE_ACL_PARSEACE_CODE_DPORTMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination port mask.");

        case CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAG:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid control flag.");

        case CGI_MODULE_ACL_PARSEACE_CODE_CTRLFLAGMASK:
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid control flag mask.");

        default:
            break;
    }

    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "unknown error.");
} //CGI_MODULE_ACL_ReturnErrorMsg

void CGI_MODULE_ACL_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_ACL_MAC_Read;
    handlers.create_handler = CGI_MODULE_ACL_MAC_Create;
    handlers.delete_handler = CGI_MODULE_ACL_MAC_Delete;
    CGI_MAIN_Register("/api/v1/acls/mac", &handlers, 0);

    {   /* for "/api/v1/acls/mac/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_ACL_MAC_ID_Update;
        handlers.delete_handler = CGI_MODULE_ACL_MAC_ID_Delete;
        CGI_MAIN_Register("/api/v1/acls/mac/{id}", &handlers, 0);
    }

#if 0
    {   /* for "/api/v1/acls/mac/{id}/{entry}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_ACL_MAC_ID_ENTRY_Delete;
        CGI_MAIN_Register("/api/v1/acls/mac/{id}/{entry}", &handlers, 0);
    }
#endif

    {   /* for "/api/v1/acls/ip" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_ACL_IP_Read;
        handlers.create_handler = CGI_MODULE_ACL_IP_Create;
        handlers.delete_handler = CGI_MODULE_ACL_IP_Delete;
        CGI_MAIN_Register("/api/v1/acls/ip", &handlers, 0);
    }

    {   /* for "/api/v1/acls/ip/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_ACL_IP_ID_Update;
        handlers.delete_handler = CGI_MODULE_ACL_IP_ID_Delete;
        CGI_MAIN_Register("/api/v1/acls/ip/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/acls/interfaces" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_ACL_INTF_Read;
        handlers.create_handler = CGI_MODULE_ACL_INTF_Create;
        CGI_MAIN_Register("/api/v1/acls/interfaces", &handlers, 0);
    }

    {   /* for "/api/v1/acls/interfaces/{ifId}/{id}/{type}/{direction}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_ACL_INTF_ID_Delete;
        CGI_MAIN_Register("/api/v1/acls/interfaces/{ifId}/{id}/{type}/{direction}", &handlers, 0);
    }

}

static void CGI_MODULE_ACL_Init()
{
    CGI_MODULE_ACL_RegisterHandlers();
}
