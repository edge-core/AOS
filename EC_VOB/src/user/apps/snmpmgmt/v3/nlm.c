
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "l_inet.h"
#include "trap_event.h"
#include "snmp_mgr.h"
#include "sys_time.h"
#include "nlm.h"

static NLM_LOG_ENTRY_T *log_list;
static UI32_T log_index;
static UI32_T current_age_out_minutes;
static char  current_filter_name[128];
static NLM_ADMINSTATUS_T current_admin_status;
static NLM_STORAGETYPE_T current_storage_type;
static NLM_ENTRYSTATUS_T current_entry_status;
static UI32_T current_notification_logged_counter;
static UI32_T current_notification_bumped_counter;

static NLM_LOG_VARIABLE_ENTRY_T* NLM_LoggedVariable(netsnmp_variable_list *vars, UI32_T *var_index)
{
    NLM_LOG_VARIABLE_ENTRY_T *log_vars_entry = NULL;

    log_vars_entry = (NLM_LOG_VARIABLE_ENTRY_T *)malloc(sizeof(NLM_LOG_VARIABLE_ENTRY_T));
    if (log_vars_entry == NULL)
    {
        return NULL;
    }

    log_vars_entry->var_index = ++(*var_index);

    log_vars_entry->var_id = NULL;
    log_vars_entry->var_id = (NLM_OID_T *)malloc(sizeof(NLM_OID_T));
    if (log_vars_entry->var_id == NULL)
    {
        free(log_vars_entry);
        return NULL;
    }
    memcpy(log_vars_entry->var_id->oid, vars->name, vars->name_length * sizeof(UI32_T));
    log_vars_entry->var_id->oid_len = vars->name_length;

    log_vars_entry->var_type = vars->type;
    log_vars_entry->var_len = vars->val_len;

    switch (log_vars_entry->var_type)
    {
        case ASN_COUNTER:
            log_vars_entry->u.counter32 = *(vars->val.integer);
            break;
        case ASN_UNSIGNED:
            log_vars_entry->u.unsigned32 = *(vars->val.integer);
            break;
        case ASN_TIMETICKS:
            log_vars_entry->u.timeticks = *(vars->val.integer);
            break;
        case ASN_INTEGER:
            log_vars_entry->u.integer32 = *(vars->val.integer);
            break;
        case ASN_OCTET_STR:
            memcpy(log_vars_entry->u.octetstring, vars->val.string, vars->val_len * sizeof(UI8_T));
            log_vars_entry->u.octetstring[vars->val_len] = '\0';
            break;
        case ASN_IPADDRESS:
            log_vars_entry->u.ipaddress = *(vars->val.integer);
            break;
        case ASN_OBJECT_ID:
            memcpy(log_vars_entry->u.objectid.oid, vars->val.objid, vars->val_len);
            log_vars_entry->u.objectid.oid_len = vars->val_len / sizeof(UI32_T);
            break;
        case ASN_COUNTER64:
            memcpy(&log_vars_entry->u.counter64, vars->val.counter64, 8);
            break;
        case ASN_OPAQUE:
            memcpy(log_vars_entry->u.opaque, vars->val.string, vars->val_len * sizeof(UI8_T));
            log_vars_entry->u.opaque[vars->val_len] = '\0';
            break;
        default:
            return NULL;
    }

    return log_vars_entry;
}

static NLM_LOG_ENTRY_T*
NLM_Logged(
    netsnmp_variable_list *vars,
    UI32_T trap_type,
    UI32_T specific,
    UI32_T *enterprise,
    UI32_T enterprise_length,
    SNMP_MGR_SnmpNotifyFilterProfileEntry_T *filter_profile_entry)
{
    NLM_LOG_VARIABLE_ENTRY_T *log_vars_entry = NULL;
    NLM_LOG_VARIABLE_ENTRY_T *cur, *next;
    SNMP_MGR_SnmpTargetAddrEntry_T tar_addr_entry;
    SNMP_MGR_SnmpCommunityEntry_T comm_entry;
    SNMP_MGR_SnmpRemoteEngineID_T remote_entry;
    SNMP_MGR_TrapDestEntry_T trap_dest_entry;
    netsnmp_variable_list *vars_entry = NULL;
    NLM_LOG_ENTRY_T *log_entry = NULL;
    UI32_T engineid_len;
    UI32_T var_index = 0;

    log_entry = (NLM_LOG_ENTRY_T *)malloc(sizeof(NLM_LOG_ENTRY_T));

    if (log_entry == NULL)
    {
        return NULL;
    }

    log_index++;
    log_entry->index = log_index;

    SYS_TIME_GetSystemUpTimeByTick(&log_entry->time);

    /* get trap destination entry
     */
    memset(&trap_dest_entry, 0, sizeof(trap_dest_entry));
    memcpy(&trap_dest_entry.trap_dest_address,
        &filter_profile_entry->snmp_notify_filter_profile_ip,
        sizeof(trap_dest_entry.trap_dest_address));

    if (SNMP_MGR_GetTrapReceiver(&trap_dest_entry) != SNMP_MGR_ERROR_OK)
    {
        free(log_entry);
        return NULL;
    }

    /* get target address entry
     */
    memset(&tar_addr_entry, 0, sizeof(tar_addr_entry));
    strncpy(tar_addr_entry.snmp_target_addr_name,
        filter_profile_entry->snmp_target_params_name,
        sizeof(tar_addr_entry.snmp_target_addr_name));
    tar_addr_entry.snmp_target_addr_name[sizeof(tar_addr_entry.snmp_target_addr_name)-1] = '\0';

    if (SNMP_MGR_GetSnmpTargetAddrTable(&tar_addr_entry) != SNMP_MGR_ERROR_OK)
    {
        free(log_entry);
        return NULL;
    }

    memcpy(log_entry->taddress, tar_addr_entry.taddr.snmp_target_addr_taddress, tar_addr_entry.snmp_target_addr_len);
    memcpy(log_entry->tdomain.oid, tar_addr_entry.snmp_target_addr_tdomain,
        tar_addr_entry.snmp_target_addr_tdomain_len * sizeof(UI32_T));
    log_entry->tdomain.oid_len = tar_addr_entry.snmp_target_addr_tdomain_len;

    if (trap_dest_entry.trap_dest_version == 3)
    {
        if (trap_dest_entry.trap_dest_type == VAL_snmpNotifyType_inform)
        {
            memset(&remote_entry, 0, sizeof(SNMP_MGR_SnmpRemoteEngineID_T));
            memcpy(&remote_entry.snmp_remote_engineID_host,
                &trap_dest_entry.trap_dest_address,
                sizeof(remote_entry.snmp_remote_engineID_host));

            if (SNMP_MGR_GetSnmpRemoteEngineIDEntry(&remote_entry) != SNMP_MGR_ERROR_OK)
            {
                free(log_entry);
                return NULL;
            }
            memcpy(log_entry->contextengineid, remote_entry.snmp_remote_engineID,
                remote_entry.snmp_remote_engineIDLen);
            log_entry->contextengineid_len = remote_entry.snmp_remote_engineIDLen;
        }
        else
        {
            if (SNMP_MGR_GetEngineID(log_entry->contextengineid, &engineid_len) != SNMP_MGR_ERROR_OK)
            {
                free(log_entry);
                return NULL;
            }

            log_entry->contextengineid_len = engineid_len;
        }

        memset(&comm_entry, 0, sizeof(SNMP_MGR_SnmpCommunityEntry_T));
        strcpy(comm_entry.snmp_community_index, "public");
        if (SNMP_MGR_GetSnmpCommunityEntry(&comm_entry) != SNMP_MGR_ERROR_OK)
        {
            free(log_entry);
            return NULL;
        }

        strncpy((char *)log_entry->contextname, (char *)comm_entry.snmp_community_context_name, sizeof(log_entry->contextname));
        log_entry->contextname[sizeof(log_entry->contextname)-1] = '\0';
    }
    else
    {
        log_entry->contextengineid[0] = '\0';
        log_entry->contextengineid_len = 0;
        strncpy((char *)log_entry->contextname, trap_dest_entry.trap_dest_community, sizeof(log_entry->contextname));
        log_entry->contextname[sizeof(log_entry->contextname)-1] = '\0';
    }

    if (trap_dest_entry.trap_dest_version == 1)
    {
        log_entry->engineid[0] = '\0';
        log_entry->engineid_len= 0;
    }
    else
    {
        if (SNMP_MGR_GetEngineID(log_entry->engineid, &engineid_len) != SNMP_MGR_ERROR_OK)
        {
            free(log_entry);
            return NULL;
        }

        log_entry->engineid_len= engineid_len;
    }
    if (specific == 0)
    {
        memcpy(log_entry->notificationid.oid, enterprise, enterprise_length * sizeof(UI32_T));
        log_entry->notificationid.oid_len = enterprise_length;
    }
    else
    {
        memcpy(log_entry->notificationid.oid, enterprise, enterprise_length * sizeof(UI32_T));
        log_entry->notificationid.oid[enterprise_length] = 0;
        log_entry->notificationid.oid[enterprise_length+1] = specific;
        log_entry->notificationid.oid_len = enterprise_length + 2;
    }

    /* log variables binded */
    log_entry->variable_entry_head = NULL;
    for (vars_entry=vars; vars_entry; vars_entry = vars_entry->next_variable)
    {
        log_vars_entry = NLM_LoggedVariable(vars_entry, &var_index);
        if (log_vars_entry == NULL)
        {
            for (cur=log_entry->variable_entry_head; cur; cur=next)
            {
                next = cur->next;
                free(cur->var_id);
                free(cur);
            }

            free(log_entry);

            return NULL;
        }

        log_vars_entry->next = log_entry->variable_entry_head;
        log_entry->variable_entry_head = log_vars_entry;
    }

    return log_entry;
}

static BOOL_T NLM_Bumped(NLM_LOG_ENTRY_T *log_entry)
{
    NLM_LOG_VARIABLE_ENTRY_T *cur, *next;

    if (log_entry == NULL)
        return FALSE;

    /* delete variable entry */
    for (cur=log_entry->variable_entry_head; cur; cur=next)
    {
        next = cur->next;
        free((void*)cur->var_id);
        free((void*)cur);
    }

    free((void*)log_entry);
    log_entry = NULL;

    return TRUE;
}

static BOOL_T NLM_AgeOutBumped(void)
{
    NLM_LOG_ENTRY_T *log_entry = NULL;
    NLM_LOG_ENTRY_T *cur = NULL;
    NLM_LOG_ENTRY_T *pre = NULL;
    UI32_T      current_time;

    /* check if log is time out */
    for (log_entry=log_list; log_entry; log_entry=log_entry->next)
    {
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        if (current_time > ((current_age_out_minutes * 6000) + log_entry->time))
        {
            if (pre == NULL)
            {
                log_list = NULL;
            }
            else
            {
                pre->next = NULL;
            }

            break;
        }
        pre = log_entry;
    }

    while (log_entry)
    {
        cur = log_entry;
        log_entry = log_entry->next;

        if (!NLM_Bumped(cur))
            return FALSE;

        current_notification_logged_counter--;
    }

    return TRUE;
}

void NLM_NotificationLog(UI32_T trap_type, UI32_T specific, UI32_T *enterprise,
                            UI32_T enterprise_length, netsnmp_variable_list *vars)
{
    SNMP_MGR_SnmpNotifyFilterEntry_T filter_entry;
    SNMP_MGR_SnmpNotifyFilterProfileEntry_T filter_profile_entry;
    NLM_LOG_ENTRY_T *log_entry = NULL;
    NLM_LOG_ENTRY_T *pre;

    memset(&filter_entry, 0, sizeof(SNMP_MGR_SnmpNotifyFilterEntry_T));

    if (current_admin_status == ADMINSTATUS_DISABLED)
        return;

    if (current_filter_name[0] == '\0')
        return;

    memset(&filter_profile_entry, 0, sizeof(filter_profile_entry));
    strncpy(filter_profile_entry.snmp_notify_filter_profile_name,
        current_filter_name,
        sizeof(filter_profile_entry.snmp_notify_filter_profile_name));
    filter_profile_entry.snmp_notify_filter_profile_name[sizeof(filter_profile_entry.snmp_notify_filter_profile_name)-1] = '\0';

    if (SNMP_MGR_ERROR_OK != SNMP_MGR_GetSnmpNotifyFilterProfileTableByProfileName(&filter_profile_entry))
    {
        return;
    }

    /* check a name if identify an existing entry in snmpNotifyFilterTable */
    if (!SNMP_MGR_CheckExistNotifyFilter(trap_type, specific, enterprise,
            enterprise_length, filter_profile_entry.snmp_target_params_name))
    {
        return;
    }
    /* check if log exceed entry limit */
    if (current_notification_logged_counter == DEFAULT_NLMCONFIGENTRYLIMIT)
    {
        if (log_list == NULL)
        {
            return;
        }

        pre = NULL;
        for (log_entry=log_list; log_entry->next; log_entry=log_entry->next)
        {
            pre = log_entry;
        }

        if (pre == NULL)
        {
            log_list = NULL;
        }
        else
        {
            pre->next = NULL;
        }

        if (!NLM_Bumped(log_entry))
        {
            return;
        }

        current_notification_logged_counter--;
        current_notification_bumped_counter++;
    }

    if (!NLM_AgeOutBumped())
    {
        return;
    }

    log_entry = NLM_Logged(vars, trap_type, specific, enterprise, enterprise_length, &filter_profile_entry);

    if (log_entry == NULL)
    {
        return;
    }

    log_entry->next = log_list;
    log_list = log_entry;
    current_notification_logged_counter++;
}


void NLM_InitiateDefaultSetting(void)
{
    log_list = NULL;
    log_index = 0;
    current_age_out_minutes = DEFAULT_NLMCONFIGAGEOUT;
    current_filter_name[0] = '\0';
    current_admin_status = ADMINSTATUS_ENABLED;
    current_storage_type = STORAGETYPE_VOLATILE;
    current_entry_status = ENTRYSTATUS_ACTIVE;
    current_notification_logged_counter = 0;
    current_notification_bumped_counter = 0;
}

BOOL_T NLM_GetGlobalEntryLimit(UI32_T *entry_limit)
{
    *entry_limit = DEFAULT_NLMCONFIGENTRYLIMIT;

    return TRUE;
}
BOOL_T NLM_GetGlobalAgeOut(UI32_T *age_out)
{
    *age_out = current_age_out_minutes;

    if (!NLM_AgeOutBumped())
        return FALSE;

    return TRUE;
}

BOOL_T NLM_SetGlobalAgeOut(UI32_T age_out)
{
    current_age_out_minutes = age_out;

    if (!NLM_AgeOutBumped())
        return FALSE;

    return TRUE;
}

BOOL_T NLM_GetConfigLogEntry(nlmConfigLog_T *entry)
{
    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    strcpy((char *)entry->filter_name, (char *)current_filter_name);
    entry->entry_limit = DEFAULT_NLMCONFIGENTRYLIMIT;
    entry->admin_status = current_admin_status;

    if (current_admin_status == ADMINSTATUS_DISABLED)
    {
        entry->oper_status = OPERSTATUS_DISABLED;
    }
    else if (SNMP_MGR_CheckNotifyFilterName(current_filter_name)
        && (current_filter_name[0] != '\0'))
    {
        entry->oper_status = OPERSTATUS_OPERATIONAL;
    }
    else
    {
        entry->oper_status = OPERSTATUS_NOFILTER;
    }

    entry->storage_type = current_storage_type;
    entry->entry_status = current_entry_status;

    return TRUE;
}

BOOL_T NLM_SetConfigLogFilterName(UI8_T *index, UI8_T *filter_name)
{
    /* only default entry for unnamed log */
    if (index[0] != '\0')
        return FALSE;

    if (current_admin_status == ADMINSTATUS_DISABLED)
        return FALSE;

    strcpy((char *)current_filter_name, (char *)filter_name);

    return TRUE;
}

BOOL_T NLM_SetConfigLogAdminStatus(UI8_T *index, UI32_T admin_status)
{
    /* only default entry for unnamed log */
    if (index[0] != '\0')
        return FALSE;

    current_admin_status = admin_status;

    return TRUE;
}

BOOL_T NLM_SetConfigLogStorageType(UI8_T *index, UI32_T storage_type)
{
    /* Only default entry for unnamed log */
    if (index[0] != '\0')
        return FALSE;

    if (current_admin_status == ADMINSTATUS_DISABLED)
        return FALSE;

    /* Only keep in memory */
    if (storage_type != current_storage_type)
        return FALSE;
    else
        return TRUE;
}

BOOL_T NLM_GetGlobalNotificationsLogged(UI32_T *logged)
{
    *logged = current_notification_logged_counter;

    if (!NLM_AgeOutBumped())
        return FALSE;

    return TRUE;
}

BOOL_T NLM_GetGlobalNotificationsBumped(UI32_T *bumped)
{
    if (!NLM_AgeOutBumped())
        return FALSE;

    *bumped = current_notification_bumped_counter;

    return TRUE;
}

BOOL_T NLM_GetStatsLogEntry(nlmStatsLog_T *entry)
{
    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    if (!NLM_AgeOutBumped())
        return FALSE;

    entry->logged = current_notification_logged_counter;
    entry->bumped = current_notification_bumped_counter;

    return TRUE;
}

BOOL_T NLM_GetLogEntry(nlmLog_T *entry)
{
    NLM_LOG_ENTRY_T *log_entry = NULL;

    if (!NLM_AgeOutBumped())
        return FALSE;

    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    if (current_admin_status == ADMINSTATUS_DISABLED)
        return FALSE;

    for (log_entry=log_list; log_entry; log_entry=log_entry->next)
    {
        if (log_entry->index == entry->index)
            break;
    }

    if (log_entry == NULL)
        return FALSE;

    entry->time = log_entry->time;
    memcpy(entry->engineid, log_entry->engineid, log_entry->engineid_len);
    entry->engineid_len = log_entry->engineid_len;
    /* a TAddress is 6 octets long */
    memcpy(entry->taddress, log_entry->taddress, 6);
    memcpy(&entry->tdomain, &log_entry->tdomain, sizeof(NLM_OID_T));
    memcpy(entry->contextengineid, log_entry->contextengineid,
        log_entry->contextengineid_len);
    entry->contextengineid_len = log_entry->contextengineid_len;
    strcpy((char *)entry->contextname, (char *)log_entry->contextname);
    memcpy(&entry->notificationid, &log_entry->notificationid, sizeof(NLM_OID_T));

    return TRUE;
}

BOOL_T NLM_GetNextLogEntry(nlmLog_T *entry)
{
    NLM_LOG_ENTRY_T *log_entry = NULL;

    if (!NLM_AgeOutBumped())
        return FALSE;

    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    if (log_list == NULL)
        return FALSE;

    if (entry->index == 0)
    {
        /* fisrt entry */
        for (log_entry=log_list; log_entry->next; log_entry=log_entry->next);
    }
    else
    {
        /* check if get to last entry */
        if (log_list->index == entry->index)
            return FALSE;

        /* getnext */
        for (log_entry=log_list; log_entry->next; log_entry=log_entry->next)
        {
            if (log_entry->next->index == entry->index)
                break;
        }

        if (log_entry->next == NULL)
            return FALSE;
    }

    if (log_entry == NULL)
        return FALSE;

    entry->index = log_entry->index;

    return TRUE;
}

BOOL_T NLM_GetLogVariableEntry(nlmLogVariable_T *entry)
{
    NLM_LOG_VARIABLE_ENTRY_T *log_var_entry = NULL;
    NLM_LOG_ENTRY_T *log_entry = NULL;
    if (!NLM_AgeOutBumped())
        return FALSE;

    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    if (current_admin_status == ADMINSTATUS_DISABLED)
        return FALSE;

    for (log_entry=log_list; log_entry; log_entry=log_entry->next)
    {
        if (log_entry->index == entry->index)
        {
            for (log_var_entry=log_entry->variable_entry_head; log_var_entry;
                log_var_entry=log_var_entry->next)
            {
                if (log_var_entry->var_index == entry->var_index)
                    break;
            }

            break;
        }
    }

    if ((log_entry == NULL) || (log_var_entry == NULL))
        return FALSE;

    entry->var_id = log_var_entry->var_id;
    entry->var_type = log_var_entry->var_type;
    entry->var_len = log_var_entry->var_len;

    switch (log_var_entry->var_type)
    {
        case ASN_COUNTER:
            entry->u.counter32 = log_var_entry->u.counter32;
            return TRUE;
        case ASN_UNSIGNED:
            entry->u.unsigned32 = log_var_entry->u.unsigned32;
            return TRUE;
        case ASN_TIMETICKS:
            entry->u.timeticks = log_var_entry->u.timeticks;
            return TRUE;
        case ASN_INTEGER:
            entry->u.integer32 = log_var_entry->u.integer32;
            return TRUE;
        case ASN_OCTET_STR:
            memcpy(entry->u.octetstring, log_var_entry->u.octetstring, log_var_entry->var_len);
            entry->u.octetstring[log_var_entry->var_len] = '\0';
            return TRUE;
        case ASN_IPADDRESS:
            entry->u.ipaddress = log_var_entry->u.ipaddress;
            return TRUE;
        case ASN_OBJECT_ID:
            memcpy(&entry->u.objectid, &log_var_entry->u.objectid, sizeof(NLM_OID_T));
            return TRUE;
        case ASN_COUNTER64:
            memcpy(&entry->u.counter64, &log_var_entry->u.counter64, 8);
            return TRUE;
        case ASN_OPAQUE:
            memcpy(entry->u.opaque, log_var_entry->u.opaque, log_var_entry->var_len);
            entry->u.opaque[log_var_entry->var_len] = '\0';
            return TRUE;
        default:
            return FALSE;
    }
}

BOOL_T NLM_GetNextLogVariableEntry(nlmLogVariable_T *entry)
{
    NLM_LOG_VARIABLE_ENTRY_T *log_var_entry = NULL;
    NLM_LOG_ENTRY_T *log_entry = NULL;

    if (!NLM_AgeOutBumped())
        return FALSE;

    /* only default entry for unnamed log */
    if (entry->name[0] != '\0')
        return FALSE;

    if (log_list == NULL)
        return FALSE;

    if ((entry->index == 0) && (entry->var_index == 0))
    {
        /* fisrt entry */
        for (log_entry=log_list; log_entry->next; log_entry=log_entry->next);

        if (log_entry->variable_entry_head == NULL)
            return FALSE;

        for (log_var_entry=log_entry->variable_entry_head; log_var_entry->next;
        log_var_entry=log_var_entry->next);

        entry->index = log_entry->index;
        entry->var_index = log_var_entry->var_index;
    }
    else
    {
        /* get next */
        for (log_entry=log_list; log_entry; log_entry=log_entry->next)
        {
            if (log_entry->index == entry->index)
            {
                for (log_var_entry=log_entry->variable_entry_head; log_var_entry;
                    log_var_entry=log_var_entry->next)
                {
                    if (log_var_entry->var_index == (entry->var_index + 1))
                        break;
                }

                break;
            }
        }

        if (log_var_entry == NULL)
        {
            for (log_entry=log_list; log_entry; log_entry=log_entry->next)
            {
                if (log_entry->index == (entry->index + 1))
                    break;
            }

            if (log_entry == NULL)
                return FALSE;

            entry->index = log_entry->index;
            entry->var_index = 1;
        }
        else
        {
            entry->index = log_entry->index;
            entry->var_index = log_var_entry->var_index;
        }

    }

    return TRUE;
}

