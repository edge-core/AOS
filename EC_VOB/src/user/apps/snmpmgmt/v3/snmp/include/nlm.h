
#ifndef NLM_H
#define NLM_H

#include "snmp_api.h"
#include "nlm_mgr.h"

#define DEFAULT_NLMCONFIGENTRYLIMIT		256
#define DEFAULT_NLMCONFIGAGEOUT			1440
#define DEFAULT_NLMNOTIFICATIONLOGGED	0
#define DEFAULT_NLMNOTIFICATIONDUMPED	0

typedef enum
{
	ADMINSTATUS_ENABLED = 1,
	ADMINSTATUS_DISABLED
} NLM_ADMINSTATUS_T;

typedef enum
{
	OPERSTATUS_DISABLED = 1,
	OPERSTATUS_OPERATIONAL,
	OPERSTATUS_NOFILTER
} NLM_OPERSTATUS_T;

typedef enum
{
	STORAGETYPE_OTHER = 1,
	STORAGETYPE_VOLATILE,
	STORAGETYPE_NONVOLATILE,
	STORAGETYPE_PERMANENT,
	STORAGETYPE_READONLY
} NLM_STORAGETYPE_T;

typedef enum
{
	ENTRYSTATUS_ACTIVE = 1,
	ENTRYSTATUS_NOTINSERVICE,
	ENTRYSTATUS_NOTREADY,
	ENTRYSTATUS_CREATEANDGO,
	ENTRYSTATUS_CREATEANDWAIT,
	ENTRYSTATUS_DESTROY
} NLM_ENTRYSTATUS_T;

typedef struct NLM_LOG_VARIABLE_ENTRY_S NLM_LOG_VARIABLE_ENTRY_T ;
struct NLM_LOG_VARIABLE_ENTRY_S
{
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

	NLM_LOG_VARIABLE_ENTRY_T *next;
};

typedef struct NLM_LOG_ENTRY_S NLM_LOG_ENTRY_T ;
struct NLM_LOG_ENTRY_S
{
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
	NLM_LOG_VARIABLE_ENTRY_T *variable_entry_head;

	NLM_LOG_ENTRY_T *next;
};

void NLM_NotificationLog(UI32_T trap_type, UI32_T specific, UI32_T *enterprise, 
							UI32_T enterprise_length, netsnmp_variable_list *vars);
void NLM_InitiateDefaultSetting(void);
BOOL_T NLM_GetGlobalEntryLimit(UI32_T *entry_limit);
BOOL_T NLM_GetGlobalAgeOut(UI32_T *age_out);
BOOL_T NLM_SetGlobalAgeOut(UI32_T age_out);
BOOL_T NLM_GetConfigLogEntry(nlmConfigLog_T *entry);
BOOL_T NLM_SetConfigLogFilterName(UI8_T *index, UI8_T *filter_name);
BOOL_T NLM_SetConfigLogAdminStatus(UI8_T *index, UI32_T admin_status);
BOOL_T NLM_SetConfigLogStorageType(UI8_T *index, UI32_T storage_type);
BOOL_T NLM_GetGlobalNotificationsLogged(UI32_T *logged);
BOOL_T NLM_GetGlobalNotificationsBumped(UI32_T *bumped);
BOOL_T NLM_GetStatsLogEntry(nlmStatsLog_T *entry);
BOOL_T NLM_GetLogEntry(nlmLog_T *entry);
BOOL_T NLM_GetNextLogEntry(nlmLog_T *entry);
BOOL_T NLM_GetLogVariableEntry(nlmLogVariable_T *entry);
BOOL_T NLM_GetNextLogVariableEntry(nlmLogVariable_T *entry);
#endif
