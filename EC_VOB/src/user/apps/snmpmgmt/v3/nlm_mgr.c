
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "sys_type.h"
#include "trap_event.h"
#include "nlm_mgr.h"
#include "nlm.h"

void NLM_MGR_InitiateSystemResources(void)
{
	NLM_InitiateDefaultSetting();
}

void NLM_MGR_NotificationLog(UI32_T trap_type, UI32_T specific,
					UI32_T *enterprise, UI32_T enterprise_length, netsnmp_variable_list *vars)
{
	NLM_NotificationLog(trap_type, specific, enterprise, enterprise_length, vars);
}
	
BOOL_T NLM_MGR_GetGlobalEntryLimit(UI32_T *entry_limit)
{
	UI32_T ret;

	ret = NLM_GetGlobalEntryLimit(entry_limit);
	return ret;
}

BOOL_T NLM_MGR_GetGlobalAgeOut(UI32_T *age_out)
{
	UI32_T ret;

	ret = NLM_GetGlobalAgeOut(age_out);
	return ret;
}

BOOL_T NLM_MGR_SetGlobalAgeOut(UI32_T age_out)
{
	UI32_T ret;

	if ((age_out < VAL_MIN_NLMCONFIGAGEOUT) || (age_out > VAL_MAX_NLMCONFIGAGEOUT))
		return FALSE;
		
	ret = NLM_SetGlobalAgeOut(age_out);
	return ret;
}

BOOL_T NLM_MGR_GetConfigLogEntry(nlmConfigLog_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetConfigLogEntry(entry);
	return ret;
}

BOOL_T NLM_MGR_SetConfigLogFilterName(UI8_T *index, UI8_T *filter_name)
{
	UI32_T ret;
	
	if ((index == NULL) || (filter_name == NULL))
		return FALSE;
	
	ret = NLM_SetConfigLogFilterName(index, filter_name);
	return ret;
}

BOOL_T NLM_MGR_SetConfigLogAdminStatus(UI8_T *index, UI32_T admin_status)
{
	UI32_T ret;

	if ((index == NULL) || (admin_status < VAL_MIN_NLMCONFIGADMINSTATUS) || (admin_status > VAL_MAX_NLMCONFIGADMINSTATUS))
		return FALSE;

	ret = NLM_SetConfigLogAdminStatus(index, admin_status);
	return ret;
}

BOOL_T NLM_MGR_SetConfigLogStorageType(UI8_T *index, UI32_T storage_type)
{
	UI32_T ret;

	if ((index == NULL) || (storage_type < VAL_MIN_NLMCONFIGSTORAGETYPE) || (storage_type > VAL_MAX_NLMCONFIGSTORAGETYPE))
		return FALSE;

	ret = NLM_SetConfigLogStorageType(index, storage_type);
	return ret;
}
BOOL_T NLM_MGR_GetNextConfigLogEntry(nlmConfigLog_T *entry)
{
	if (entry == NULL)
		return FALSE;

	/* for unnamed log */
	entry->name[0] = '\0';
	
	return TRUE;
}

BOOL_T NLM_MGR_GetGlobalNotificationsLogged(UI32_T *logged)
{
	UI32_T ret;

	ret = NLM_GetGlobalNotificationsLogged(logged);
	return ret;
}

BOOL_T NLM_MGR_GetGlobalNotificationsBumped(UI32_T *bumped)
{
	UI32_T ret;

	ret = NLM_GetGlobalNotificationsBumped(bumped);
	return ret;
}

BOOL_T NLM_MGR_GetStatsLogEntry(nlmStatsLog_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetStatsLogEntry(entry);
	return ret;
}

BOOL_T NLM_MGR_GetNextStatsLogEntry(nlmStatsLog_T *entry)
{
	if (entry == NULL)
		return FALSE;

	/* for unnamed log */
	entry->name[0] = '\0';
	
	return TRUE;
}

BOOL_T NLM_MGR_GetLogEntry(nlmLog_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetLogEntry(entry);
	return ret;
}

BOOL_T NLM_MGR_GetNextLogEntry(nlmLog_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetNextLogEntry(entry);
	return ret;
}

BOOL_T NLM_MGR_GetLogVariableEntry(nlmLogVariable_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetLogVariableEntry(entry);
	return ret;
}

BOOL_T NLM_MGR_GetNextLogVariableEntry(nlmLogVariable_T *entry)
{
	UI32_T ret;

	if (entry == NULL)
		return FALSE;

	ret = NLM_GetNextLogVariableEntry(entry);
	return ret;
}

