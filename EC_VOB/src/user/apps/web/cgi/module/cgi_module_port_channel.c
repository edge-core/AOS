#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "trk_pmgr.h"
#include "swctrl.h"

static BOOL_T CGI_MODULE_PORT_CHANNEL_GetOne(UI32_T trunk_id, json_t *trunk_obj_p);

/**----------------------------------------------------------------------
 * This API is used to create static port cahnnel and then set name and member(s).
 *
 * @param id (required, number) port cahnnel ID
 * @param members (optional, array) set port member(s)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *members_p;
    json_t  *member_p;
    UI32_T  trunk_id = 0;
    UI32_T  unit = 0, port = 0, lport = 0;
    UI32_T  idx = 0, member_num = 0;
    BOOL_T  is_static = FALSE;

    id_p = CGI_REQUEST_GetBodyValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port channel ID.");
    }
    trunk_id = json_integer_value(id_p);

    if (TRUE != TRK_PMGR_IsTrunkExist(trunk_id, &is_static))
    {
        if (TRUE != TRK_PMGR_CreateTrunk(trunk_id))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "trunk.ereateTrunkError", "Failed to create port channel.");
        }

        if (TRUE != TRK_PMGR_SetTrunkStatus(trunk_id, VAL_trunkStatus_valid))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "trunk.setTrunkStatusError", "Failed to set port channel valid.");
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
            lport = json_integer_value(member_p);

            if (TRK_MGR_SUCCESS != TRK_PMGR_AddTrunkMember(trunk_id, lport))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "trunk.addTrunkMemberError", "Failed to add port to port channel.");
            }
        }
    }

    json_object_set_new(result_p, "id", id_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all port cahnnels.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *trunks_p = json_array();
    UI32_T  trunk_id = 0;

    json_object_set_new(result_p, "portChannels", trunks_p);

    while(TRK_PMGR_GetNextTrunkId(&trunk_id))
    {
        json_t *trunk_obj_p = json_object();

        if (TRUE == CGI_MODULE_PORT_CHANNEL_GetOne(trunk_id, trunk_obj_p))
        {
            json_array_append_new(trunks_p, trunk_obj_p);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a port cahnnel.
 *
 * @param id (required, number) port cahnnel ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    //json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  trunk_id = 0;
    BOOL_T  is_static = FALSE;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    trunk_id = json_integer_value(id_p);

    if (TRUE != TRK_PMGR_IsTrunkExist(trunk_id, &is_static))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    if ( (TRUE != TRK_PMGR_SetTrunkStatus(trunk_id, VAL_trunkStatus_invalid))
        || (TRUE != TRK_PMGR_DestroyTrunk(trunk_id))
       )
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "trunk.destroyTrunkError", "Failed to delete port channel.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read a port cahnnel.
 *
 * @param id (required, number) Unique port cahnnel ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  trunk_id = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port channel ID.");
    }
    trunk_id = json_integer_value(id_p);

    if (TRUE != CGI_MODULE_PORT_CHANNEL_GetOne(trunk_id, result_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such port channel ID.");

    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to add member to a port cahnnel.
 *
 * @param id       (required, number) Unique port cahnnel ID
 * @param ifId     (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_Members_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *member_id_p;
    UI32_T  trunk_id = 0;
    UI32_T  lport = 0;
    BOOL_T  is_static = FALSE;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port channel ID.");
    }
    trunk_id = json_integer_value(id_p);

    member_id_p = CGI_REQUEST_GetBodyValue(http_request, "member");
    if (member_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get member ID.");
    }
    lport = json_integer_value(member_id_p);

    if (TRUE != TRK_PMGR_IsTrunkExist(trunk_id, &is_static))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such Port channel. Please create it first.");
    }

    if (TRUE != TRK_PMGR_AddTrunkMember(trunk_id, lport))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
            "trunk.addTrunkMemberError", "Failed to add port to port channel.");
    }

    json_object_set_new(result_p, "id", member_id_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete member to a port cahnnel.
 *
 * @param id       (required, number) Unique port cahnnel ID
 * @param memberId (required, string) Unique port cahnnel member ID.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PORT_CHANNEL_Members_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    //json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *member_id_p;
    UI32_T  trunk_id = 0;
    UI32_T  lport = 0;
    BOOL_T  is_static = FALSE;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port channel ID.");
    }
    trunk_id = json_integer_value(id_p);

    member_id_p = CGI_REQUEST_GetParamsValue(http_request, "memberId");
    if (member_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get memberId.");
    }
    lport = json_integer_value(member_id_p);

    if (TRUE != TRK_PMGR_IsTrunkExist(trunk_id, &is_static))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    if (TRUE != TRK_PMGR_DeleteTrunkMember(trunk_id, lport))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
            "trunk.deleteTrunkMemberError", "Failed to remove port from port channel.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_PORT_CHANNEL_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_PORT_CHANNEL_Read;
    handlers.create_handler = CGI_MODULE_PORT_CHANNEL_Create;
    CGI_MAIN_Register("/api/v1/port-channels", &handlers, 0);

    {   /* for "/api/v1/port-channels/{trunk_id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_PORT_CHANNEL_ID_Read;
        handlers.delete_handler = CGI_MODULE_PORT_CHANNEL_ID_Delete;
        CGI_MAIN_Register("/api/v1/port-channels/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/port-channels/{trunk_id}/members" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_PORT_CHANNEL_Members_Create;
        CGI_MAIN_Register("/api/v1/port-channels/{id}/members", &handlers, 0);
    }

    {   /* for "/api/v1/port-channels/{id}/members/{memberId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_PORT_CHANNEL_Members_Delete;
        CGI_MAIN_Register("/api/v1/port-channels/{id}/members/{memberId}", &handlers, 0);
    }
}

static void CGI_MODULE_PORT_CHANNEL_Init()
{
    CGI_MODULE_PORT_CHANNEL_RegisterHandlers();
}

static BOOL_T CGI_MODULE_PORT_CHANNEL_GetOne(UI32_T trunk_id, json_t *trunk_obj_p)
{
    TRK_MGR_TrunkEntry_T trunk_entry;
    UI32_T ifindex = 0;

    memset(&trunk_entry, 0, sizeof(trunk_entry));
    trunk_entry.trunk_index = trunk_id;

    if (TRUE != TRK_PMGR_GetTrunkEntry(&trunk_entry)) {
        return FALSE;
    }

    json_object_set_new(trunk_obj_p, "id", json_integer(trunk_id));
    SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &ifindex);
    json_object_set_new(trunk_obj_p, "ifindex", json_integer(ifindex));

    /* members
     */
    {
        json_t  *members = json_array();
        UI32_T  i = 0;

        json_object_set_new(trunk_obj_p, "members", members);

        for (i = 1; i <= ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST)*8) ; i++)
        {
            if(trunk_entry.trunk_ports[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                json_array_append_new(members, json_integer(i));
            }
        }
    }

    return TRUE;
}