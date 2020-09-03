#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "vlan_type.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "swctrl.h"

static BOOL_T CGI_MODULE_VLAN_GetOne(UI32_T vid, json_t *vlan_obj_p);

/**----------------------------------------------------------------------
 * This API is used to create static VLAN and then set name and member(s).
 *
 * @param vid (required, number) VLAN ID
 * @param name (optional, string) set vlan name
 * @param members (optional, array) set port member(s)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *name_p;
    json_t  *members_p;
    json_t  *member_p;
    json_t  *if_id_p;
    json_t  *is_tagged_p;
    const char*   if_id_str_p;
    const char*   vlan_name_str_p;
    UI32_T  vid = 0;
    BOOL_T  is_tagged = FALSE;
    UI32_T  lport = 0;
    UI32_T  idx = 0, member_num = 0;

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");
    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");

    if (VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.createError", "Failed to create VLAN.");
    }

    if (VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active) != TRUE)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.createSetAdminStatusError", "Failed to set admin status on VLAN.");
    }

    if (name_p != NULL)
    {
        vlan_name_str_p = json_string_value(name_p);

        if (!VLAN_PMGR_SetDot1qVlanStaticName(vid, (char*)vlan_name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.createSetNameError", "Failed to set VLAN name.");
        }
    }

    members_p = CGI_REQUEST_GetBodyValue(http_request, "members");
    if (members_p != NULL)
    {
        /* get port from members array
         */
        member_num = json_array_size(members_p);

        for (idx=0; idx <member_num; idx++)
        {
            member_p = json_array_get(members_p, idx);
            if (NULL == member_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a member.");
            }

            if_id_p = json_object_get(member_p, "ifId");
            if (NULL == if_id_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
            }
            if_id_str_p = json_string_value(if_id_p);

            is_tagged_p = json_object_get(member_p, "isTagged");
            if (NULL == is_tagged_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get isTagged.");
            }
            if (TRUE != json_is_boolean(is_tagged_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The isTagged is not boolean type.");
            }
            is_tagged = json_boolean_value(is_tagged_p);

            if (TRUE != CGI_UTIL_InterfaceIdToLport(if_id_str_p, &lport))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
            }

            if (TRUE == is_tagged)
            {
                if (TRUE != VLAN_PMGR_AddEgressPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
                }
            }
            else
            {
                if (TRUE != VLAN_PMGR_AddUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
                }
            }
        }
    }

    json_object_set_new(result_p, "id", vid_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to set pvid.
 *
 * @param pvid (required, number) native VLAN ID
 * @param ifId (required, string) port
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pvid_p;
    json_t  *if_id_p;
    const char*   if_id_str_p;
    UI32_T  pvid = 0;
    UI32_T  lport = 0;

    pvid_p = CGI_REQUEST_GetBodyValue(http_request, "pvid");
    if (pvid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get pvid.");
    }
    pvid = json_integer_value(pvid_p);

    if_id_p = CGI_REQUEST_GetBodyValue(http_request, "ifId");
    if (if_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
    }
    if_id_str_p = json_string_value(if_id_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(if_id_str_p, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)
    if (!VLAN_PMGR_SetNativeVlanAgent(lport, pvid))
#else
    if (!VLAN_PMGR_SetDot1qPvid(lport, pvid))
#endif
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.setPvidError", "Failed to set PVID.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all VLANs.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vlans_p = json_array();
    UI32_T  time_mark = 0, vid = 0;

    json_object_set_new(result_p, "vlans", vlans_p);
    while(VLAN_POM_GetNextVlanId(time_mark, &vid))
    {
        json_t *vlan_obj_p = json_object();
        if (TRUE == CGI_MODULE_VLAN_GetOne(vid, vlan_obj_p))
        {
            json_array_append_new(vlans_p, vlan_obj_p);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a VLAN.
 *
 * @param vid (required, number) VLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    UI32_T  vid = 0;

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    vid = json_integer_value(vid_p);

    if (   (VLAN_POM_IsVlanExisted(vid))
        && (VLAN_PMGR_DeleteNormalVlan(vid, VAL_dot1qVlanStatus_permanent) != TRUE)
       )
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.deleteNormalVlanError", "Failed to delete VLAN.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read a VLAN.
 *
 * @param id (required, number) Unique VLAN ID, same as vid
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    UI32_T  vid = 0;

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    vid = json_integer_value(vid_p);
    if (TRUE == CGI_MODULE_VLAN_GetOne(vid, result_p))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such VLAN ID.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to add member to a VLAN.
 *
 * @param id       (required, number) Unique VLAN ID, same as vid
 * @param ifId     (required, string) Interface ID
 * @param isTagged (required, boolean) Tagged/Untagged member
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Members_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *if_id_p;
    json_t  *is_tagged_p;
    const char*   if_id_str_p;
    UI32_T  vid = 0, vid_ifindex = 0;
    BOOL_T  is_tagged = FALSE;
    UI32_T  lport = 0;

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    if_id_p = CGI_REQUEST_GetBodyValue(http_request, "ifId");
    if (if_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
    }
    if_id_str_p = json_string_value(if_id_p);

    is_tagged_p = CGI_REQUEST_GetBodyValue(http_request, "isTagged");
    if (NULL == is_tagged_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get isTagged.");
    }
    if (TRUE != json_is_boolean(is_tagged_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The isTagged is not boolean type.");
    }
    is_tagged = json_boolean_value(is_tagged_p);

    if(!VLAN_POM_IsVlanExisted(vid))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such VLAN. Please create it first.");
    }

    if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.convertToIfindexError", "Failed to get VLAN interface value.");

    if (TRUE != CGI_UTIL_InterfaceIdToLport(if_id_str_p, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

    if (TRUE ==is_tagged)
    {
        if (VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, lport)) /* untagged member to tagged member */
        {
            if (TRUE != VLAN_PMGR_DeleteUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.deleteUntagPortMemberError", "Failed to add port to VLAN.");
            }
        }
        else
        {
            if (TRUE != VLAN_PMGR_AddEgressPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
            }
        }
    }
    else
    {
        if (TRUE != VLAN_PMGR_AddUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
        }
    }

    json_object_set_new(result_p, "id", if_id_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete member to a VLAN.
 *
 * @param id       (required, number) Unique VLAN ID, same as vid
 * @param memberId (required, string) Unique VLAN member ID, same as ifId.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Members_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *member_id_p;
    UI32_T  vid=0, vid_ifindex=0;
    UI32_T  unit=0, port=0, lport=0;
    const char    *member_id_str_p;
    char    buffer[20] = {0};

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    member_id_p = CGI_REQUEST_GetParamsValue(http_request, "memberId");
    if (member_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get memberId.");
    }
    member_id_str_p = json_string_value(member_id_p);
    CGI_UTIL_UrlDecode(buffer, member_id_str_p);

    if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.convertToIfindexError", "Failed to get VLAN interface value.");

    if (TRUE != CGI_UTIL_InterfaceIdToLport((const char *) buffer, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error memberId.");
    }

    if (    (TRUE == VLAN_POM_IsPortVlanMember(vid_ifindex, lport))
        &&  (FALSE == VLAN_PMGR_DeleteEgressPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
       )
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.deleteEgressPortMemberError", "Failed to remove port from VLAN.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to add/delete members to list VLANs.
 *
 * @param vids   (required, string) VLAN ID list
 * @param status (required, boolean) add or delete
 * @param ifId     (required, string) Interface ID
 * @param isTagged (required, boolean) Tagged/Untagged member
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_Members_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vids_p;
    json_t  *status_p;
    json_t  *if_id_p;
    json_t  *is_tagged_p;
    const char*   vids_str_p;
    const char*   if_id_str_p;
    char   *op_ptr;
    UI32_T  vid = 0, vid_ifindex = 0;
    BOOL_T  status = FALSE;
    BOOL_T  is_tagged = FALSE;
    UI32_T  lport = 0;
    UI32_T  err_idx = 0, lower_val = 0, upper_val = 0;
    //UI8_T vlan_list_ar[VLAN_TYPE_VLAN_LIST_SIZE] = {0};
    char  token_ar[SYS_ADPT_CLI_MAX_BUFSIZE + 1] = {0};
    char  delemiters[2] = {0};

    vids_p = CGI_REQUEST_GetBodyValue(http_request, "vids");
    if_id_p = CGI_REQUEST_GetBodyValue(http_request, "ifId");
    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    is_tagged_p = CGI_REQUEST_GetBodyValue(http_request, "isTagged");

    if ((NULL == vids_p) || (NULL == if_id_p) || (NULL == status_p) || (NULL == is_tagged_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get fields.");
    }

    vids_str_p = json_string_value(vids_p);
    if_id_str_p = json_string_value(if_id_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(if_id_str_p, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }
    status = json_boolean_value(status_p);

    if (TRUE != json_is_boolean(is_tagged_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The isTagged is not boolean type.");
    }
    is_tagged = json_boolean_value(is_tagged_p);

    delemiters[0] = ',';
    op_ptr = (char *) vids_str_p;

    do //pars vid list
    {
        memset(token_ar, 0, sizeof(token_ar));
        op_ptr = CGI_UTIL_GetToken(op_ptr, token_ar, delemiters);

        if (!CGI_UTIL_GetLowerUpperValue(token_ar, &lower_val, &upper_val, &err_idx))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Cannot parse vids.");
        }

        for (vid = lower_val; vid <= upper_val; vid++)
        {
            if (VLAN_POM_IsVlanExisted(vid) == FALSE)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such VLAN. Please create it first.");
            }

            if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.convertToIfindexError", "Failed to get VLAN interface value.");
            }

            //L_CVRT_ADD_MEMBER_TO_PORTLIST(vlan_list_ar, vid);
        }
    } while (op_ptr != 0 && !isspace(*op_ptr));

    op_ptr = (char *) vids_str_p;

    if (TRUE == status)  //add
    {
        do
        {
            memset(token_ar, 0, sizeof(token_ar));
            op_ptr = CGI_UTIL_GetToken(op_ptr, token_ar, delemiters);
            CGI_UTIL_GetLowerUpperValue(token_ar, &lower_val, &upper_val, &err_idx);

            for (vid = lower_val; vid <= upper_val; vid++)
            {
                VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);

                if (TRUE == is_tagged)
                {
                    if (VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, lport)) /* untagged member to tagged member */
                    {
                        if (TRUE != VLAN_PMGR_DeleteUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                        {
                            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                    "vlan.deleteUntagPortMemberError", "Failed to add port to VLAN.");
                        }
                    }
                    else
                    {
                        if (TRUE != VLAN_PMGR_AddEgressPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                        {
                            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                    "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
                        }
                    }
                }
                else
                {
                    if (TRUE != VLAN_PMGR_AddUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                    {
                        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                "vlan.addEgressPortMemberError", "Failed to add port to VLAN.");
                    }
                }
            }
        } while (op_ptr != 0 && !isspace(*op_ptr));
    } //add
    else //delete
    {
        do
        {
            memset(token_ar, 0, sizeof(token_ar));
            op_ptr = CGI_UTIL_GetToken(op_ptr, token_ar, delemiters);
            CGI_UTIL_GetLowerUpperValue(token_ar, &lower_val, &upper_val, &err_idx);

            for (vid = lower_val; vid <= upper_val; vid++)
            {
                VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);

                if (    (TRUE == VLAN_POM_IsPortVlanMember(vid_ifindex, lport))
                    &&  (FALSE == VLAN_PMGR_DeleteEgressPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
                   )
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "vlan.deleteEgressPortMemberError", "Failed to remove port from VLAN.");
                }
            }
        } while (op_ptr != 0 && !isspace(*op_ptr));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VLAN_Members_Update

#if (SYS_CPNT_MAC_VLAN == TRUE)
#if 0
/**----------------------------------------------------------------------
 * This API is used to read a MAC base VLAN.
 *
 * @param id (required, number) Unique VLAN ID, same as vid
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_MAC_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *addrs_p = json_array();
    UI32_T  vid = 0;
    VLAN_TYPE_MacVlanEntry_T  mac_vlan_entry;

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }

    vid = json_integer_value(vid_p);
    memset(&mac_vlan_entry, 0, sizeof(mac_vlan_entry));


    while (TRUE == VLAN_OM_GetNextMacVlanEntry(&mac_vlan_entry))
    {
        char mac_addr_ar[20] = {0}; //xx-xx-xx-xx-xx-xx/xx
        int mask_prefix = 0;

        if (mac_vlan_entry.vid != vid)
        {
            continue;
        }

        mask_prefix = CGI_UTIL_MaskToPrefix(mac_vlan_entry.mask);
        sprintf(mac_addr_ar, "%02X-%02X-%02X-%02X-%02X-%02X/%lu",
                mac_vlan_entry.mac_address[0],
                mac_vlan_entry.mac_address[1],
                mac_vlan_entry.mac_address[2],
                mac_vlan_entry.mac_address[3],
                mac_vlan_entry.mac_address[4],
                mac_vlan_entry.mac_address[5], mask_prefix);
        json_array_append_new(addrs_p, json_string(mac_addr_ar));
    }

    json_object_set_new(result_p, "id", json_integer(vid));
    json_object_set_new(result_p, "vid", json_integer(vid));
    json_object_set_new(result_p, "macBasedAddrs", addrs_p);

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}
#endif //#if 0

/**----------------------------------------------------------------------
 * This API is used to add MAC base to a VLAN.
 *
 * @param vid         (required, number) Unique VLAN ID, same as vid
 * @param macBaseAddr (required, string) MAC base addr and mask
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_MAC_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *mac_base_p;
    char*   mac_base_str_p;
    char    *save_str_addr = NULL;
    char    *mac_str_p;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T   mask_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    char    mac_id_ar[24] = {0}; //xx-xx-xx-xx-xx-xx/xx
    UI32_T  vid = 0;
    UI32_T  mask_prefix = 0;

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    mac_base_p = CGI_REQUEST_GetBodyValue(http_request, "macBaseAddr");

    if (mac_base_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get macBaseAddr.");
    }
    mac_base_str_p = (char *) json_string_value(mac_base_p);
    mac_str_p = strtok_r(mac_base_str_p, "/", &save_str_addr);

    if (sscanf (save_str_addr, "%lu", &mask_prefix) != 1)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse prefix.");
    }

    if (CGI_UTIL_MacStrToHex(mac_str_p, mac_ar) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse source MAC.");
    }

    CGI_UTIL_PrefixToMask(mask_prefix, mask_ar);

    if(!VLAN_POM_IsVlanExisted(vid))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such VLAN. Please create it first.");
    }

    if (TRUE != VLAN_PMGR_SetMacVlanEntry(mac_ar, mask_ar, vid, SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.SetMacVlanEntry", "Failed to set VLAN MAC.");
    }

    sprintf(mac_id_ar, "%02X-%02X-%02X-%02X-%02X-%02X_%lu",
            mac_ar[0], mac_ar[1], mac_ar[2], mac_ar[3], mac_ar[4], mac_ar[5], mask_prefix);
    json_object_set_new(result_p, "id", json_string(mac_id_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete MAC to a VLAN.
 *
 * @param id            (required, number) Unique VLAN ID, same as vid
 * @param macBaseAddrId (required, string) Unique mac ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_MAC_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    if (TRUE != VLAN_PMGR_DeleteAllMacVlanEntry())
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "vlan.DeleteAllMacVlanEntry", "Failed to remove all MAC from VLAN.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete MAC to a VLAN.
 *
 * @param id            (required, number) Unique VLAN ID, same as vid
 * @param macBaseAddrId (required, string) Unique mac ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VLAN_MAC_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *mac_base_p;
    UI32_T  mask_prefix = 0;
    char    *mac_base_str_p;
    char    *save_str_addr = NULL;
    char    *mac_str_p;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T   mask_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    mac_base_p = CGI_REQUEST_GetParamsValue(http_request, "macBaseAddrId");

    if (mac_base_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get macBaseAddrId.");
    }
    mac_base_str_p = (char *) json_string_value(mac_base_p);
    mac_str_p = strtok_r(mac_base_str_p, "_", &save_str_addr);

    if (sscanf (save_str_addr, "%lu", &mask_prefix) != 1)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse prefix.");
    }

    if (CGI_UTIL_MacStrToHex(mac_str_p, mac_ar) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse source MAC.");
    }

    CGI_UTIL_PrefixToMask(mask_prefix, mask_ar);

    if (TRUE != VLAN_PMGR_DeleteMacVlanEntry(mac_ar, mask_ar))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "vlan.DeleteMacVlanEntry", "Failed to remove MAC from VLAN.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}
#endif /* #if (SYS_CPNT_MAC_VLAN == TRUE) */

void CGI_MODULE_VLAN_RegisterHandlers()
{
    {
        CGI_API_HANDLER_SET_T handlers = {0};

        handlers.read_handler = CGI_MODULE_VLAN_Read;
        handlers.create_handler = CGI_MODULE_VLAN_Create;
        handlers.update_handler = CGI_MODULE_VLAN_Update;
        CGI_MAIN_Register("/api/v1/vlans", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_VLAN_ID_Read;
        handlers.delete_handler = CGI_MODULE_VLAN_ID_Delete;
        CGI_MAIN_Register("/api/v1/vlans/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/{id}/members" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_VLAN_Members_Create;
        CGI_MAIN_Register("/api/v1/vlans/{id}/members", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/{id}/members/{memberId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VLAN_Members_Delete;
        CGI_MAIN_Register("/api/v1/vlans/{id}/members/{memberId}", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/members" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_VLAN_Members_Update;
        CGI_MAIN_Register("/api/v1/vlans/members", &handlers, 0);
    }

#if (SYS_CPNT_MAC_VLAN == TRUE)
    {   /* for "/api/v1/vlans/{id}/macs" */
        CGI_API_HANDLER_SET_T handlers = {0};
        //handlers.read_handler = CGI_MODULE_VLAN_MAC_Read;
        handlers.create_handler = CGI_MODULE_VLAN_MAC_Create;
        handlers.delete_handler = CGI_MODULE_VLAN_MAC_Delete;
        CGI_MAIN_Register("/api/v1/mac-vlans", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/{id}/macs/{macBaseAddrId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VLAN_MAC_ID_Delete;
        CGI_MAIN_Register("/api/v1/mac-vlans/{macBaseAddrId}", &handlers, 0);
    }
#endif
}

static void CGI_MODULE_VLAN_Init()
{
    CGI_MODULE_VLAN_RegisterHandlers();
}

static BOOL_T CGI_MODULE_VLAN_GetOne(UI32_T vid, json_t *vlan_obj_p)
{
    UI32_T  ret=FALSE;
    UI32_T  time_mark = 0;
    UI32_T  vid_ifindex = 0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    if(VLAN_POM_GetDot1qVlanCurrentEntry(time_mark, vid, &vlan_info))
    {
        json_object_set_new(vlan_obj_p, "id", json_integer(vid));
        json_object_set_new(vlan_obj_p, "vid", json_integer(vid));
        json_object_set_new(vlan_obj_p, "name", json_string(vlan_info.dot1q_vlan_static_name));

        /* members
         */
        {
            json_t  *members = json_array();
            UI32_T  unit=0, port=0, lport=0, trunk_id=0;
            UI8_T   id[20] = {0};
            SWCTRL_Lport_Type_T lport_type;

            json_object_set_new(vlan_obj_p, "members", members);
            for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
            {
                memset(id, 0, sizeof(id));
                if (VLAN_POM_IsPortVlanMember(vid_ifindex, lport))
                {
                    json_t *member_obj_p = json_object();
                    BOOL_T is_tagged = FALSE;

                    lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

                    if (vlan_info.dot1q_vlan_current_untagged_ports[(UI32_T)( (lport - 1)/8 )] & (1 << ( 7 - ( (lport - 1) % 8) ) ) )
                    {
                        is_tagged = FALSE;
                    }
                    else
                    {
                        is_tagged = TRUE;
                    }

                    if (lport_type == SWCTRL_LPORT_NORMAL_PORT)
                    {
                        sprintf(id, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
                    }
                    else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
                    {
                        sprintf(id, "trunk%lu", (unsigned long)trunk_id);
                    }

                    json_object_set_new(member_obj_p, "id", json_string(id));
                    json_object_set_new(member_obj_p, "ifId", json_string(id));
                    json_object_set_new(member_obj_p, "isTagged", json_boolean(is_tagged));
                    json_array_append_new(members, member_obj_p);
                }
            }
        }
        ret = TRUE;
    }
    return ret;
}
