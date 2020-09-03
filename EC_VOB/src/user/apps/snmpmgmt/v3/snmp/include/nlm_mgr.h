
#ifndef NLM_MGR_H
#define NLM_MGR_H

#include "sys_type.h"
#include "leaf_3411.h"
#include "leaf_3413n.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "trap_event.h"

#define VAL_MIN_NLMCONFIGAGEOUT			0
#define VAL_MAX_NLMCONFIGAGEOUT			4294967295U
#define VAL_MIN_NLMCONFIGADMINSTATUS	1
#define VAL_MAX_NLMCONFIGADMINSTATUS	2
#define VAL_MIN_NLMCONFIGSTORAGETYPE	1
#define VAL_MAX_NLMCONFIGSTORAGETYPE	5

typedef struct
{
	UI32_T		oid[30];
	UI32_T		oid_len;
} NLM_OID_T;

typedef struct
{
	UI8_T		name[32];
	UI8_T		filter_name[MAXSIZE_snmpNotifyFilterProfileName+1];
	UI32_T		entry_limit;
	UI32_T		admin_status;
	UI32_T		oper_status;
	UI32_T		storage_type;
	UI32_T		entry_status;
} nlmConfigLog_T;

typedef struct
{
	UI8_T		name[32];
	UI32_T		logged;
	UI32_T		bumped;
} nlmStatsLog_T;

typedef struct
{
	UI8_T		name[32];
	UI32_T		index;
	UI32_T		time;
	UI8_T		engineid[MAXSIZE_snmpEngineID];
	UI32_T		engineid_len;
	UI8_T		taddress[6];
	NLM_OID_T	tdomain;
	UI8_T		contextengineid[MAXSIZE_snmpEngineID];
	UI32_T		contextengineid_len;
	UI8_T		contextname[SYS_ADPT_MAX_COMM_STR_NAME_LEN];
	NLM_OID_T	notificationid;
} nlmLog_T;

typedef struct
{
	UI8_T		name[32];
	UI32_T		index;
	UI32_T		var_index;
	NLM_OID_T	*var_id;
	UI32_T		var_type;
	UI32_T		var_len;

	union
	{
		UI32_T		counter32;
		UI32_T		unsigned32;
		UI32_T		timeticks;
		UI32_T		integer32;
		UI8_T		octetstring[128];
		UI32_T		ipaddress;
		NLM_OID_T	objectid;
		UI64_T		counter64;
		UI8_T		opaque[128];
	} u;
} nlmLogVariable_T;

void NLM_MGR_NotificationLog(UI32_T trap_type, UI32_T specific,	UI32_T *enterprise, 
							UI32_T enterprise_length, struct variable_list *vars);
void NLM_MGR_InitiateSystemResources(void);
BOOL_T NLM_MGR_GetGlobalEntryLimit(UI32_T *entry_limit);
BOOL_T NLM_MGR_GetGlobalAgeOut(UI32_T *age_out);
BOOL_T NLM_MGR_SetGlobalAgeOut(UI32_T age_out);
BOOL_T NLM_MGR_GetConfigLogEntry(nlmConfigLog_T *entry);
BOOL_T NLM_MGR_SetConfigLogFilterName(UI8_T *index, UI8_T *filter_name);
BOOL_T NLM_MGR_SetConfigLogAdminStatus(UI8_T *index, UI32_T admin_status);
BOOL_T NLM_MGR_SetConfigLogStorageType(UI8_T *index, UI32_T storage_type);
BOOL_T NLM_MGR_GetNextConfigLogEntry(nlmConfigLog_T *entry);
BOOL_T NLM_MGR_GetGlobalNotificationsLogged(UI32_T *logged);
BOOL_T NLM_MGR_GetGlobalNotificationsBumped(UI32_T *bumped);
BOOL_T NLM_MGR_GetStatsLogEntry(nlmStatsLog_T *entry);
BOOL_T NLM_MGR_GetNextStatsLogEntry(nlmStatsLog_T *entry);
BOOL_T NLM_MGR_GetLogEntry(nlmLog_T *entry);
BOOL_T NLM_MGR_GetNextLogEntry(nlmLog_T *entry);
BOOL_T NLM_MGR_GetLogVariableEntry(nlmLogVariable_T *entry);
BOOL_T NLM_MGR_GetNextLogVariableEntry(nlmLogVariable_T *entry);
#endif
