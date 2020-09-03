
#ifndef NOTIFICATIONLOGMIB_H
#define NOTIFICATIONLOGMIB_H

#define SNMP_OID_NOTIFICATIONLOGMIB		1, 3, 6, 1, 2, 1, 92

/********************************************
 ***************nlmConfigLogTable************
 ********************************************
 */
#define NLMLOGNAME					1
#define NLMCONFIGLOGFILTERNAME		2
#define NLMCONFIGLOGENTRYLIMIT		3
#define NLMCONFIGLOGADMINSTATUS		4
#define NLMCONFIGLOGOPERSTATUS		5
#define NLMCONFIGLOGSTORAGETYPE		6
#define NLMCONFIGLOGENTRYSTATUS		7

/********************************************
 ***************nlmStatsLogTable************
 ********************************************
 */
#define NLMSTATSLOGNOTIFICATIONSLOGGED	1
#define NLMSTATSLOGNOTIFICATIONSBUMPED	2

/********************************************
 ***************nlmLogTable******************
 ********************************************
 */
#define NLMLOGINDEX					1
#define NLMLOGTIME					2
#define NLMLOGDATEANDTIME			3
#define NLMLOGENGINEID				4
#define NLMLOGENGINETADDRESS		5
#define NLMLOGENGINETDOMAIN			6
#define NLMLOGCONTEXTENGINEID		7
#define NLMLOGCONTEXTNAME			8
#define NLMLOGNOTIFICATIONID		9

/********************************************
 ***************nlmLogVariableTable***********
 ********************************************
 */
#define NLMLOGVARIABLEINDEX				1
#define NLMLOGVARIABLEID				2
#define NLMLOGVARIABLEVALUETYPE			3
#define NLMLOGVARIABLECOUNTER32VAL		4
#define NLMLOGVARIABLEUNSIGNED32VAL		5
#define NLMLOGVARIABLETIMETICKSVAL		6
#define NLMLOGVARIABLEINTEGER32VAL		7
#define NLMLOGVARIABLEIPADDRESSVAL		8
#define NLMLOGVARIABLEOCTETSTRINGVAL	9
#define NLMLOGVARIABLEOIDVAL			10
#define NLMLOGVARIABLECOUNTER64VAL		11
#define NLMLOGVARIABLEOPAQUEVAL			12

enum LOG_VARIABLE_VALUE_TYPE_E
{
    NLM_COUNTER32 = 1,
	NLM_UINTEGER32,
	NLM_TIMETICKS,
	NLM_INTEGER32,
	NLM_IPADDRESS,
	NLM_OCTETSTRING,
	NLM_OBJECTID,
	NLM_COUNTER64,
	NLM_OPAQUE
};

Netsnmp_Node_Handler do_nlmConfigGlobalEntryLimit;
Netsnmp_Node_Handler do_nlmConfigGlobalAgeOut;
Netsnmp_Node_Handler do_nlmStatsGlobalNotificationsLogged;
Netsnmp_Node_Handler do_nlmStatsGlobalNotificationsBumped;

/* nlmConfigLogTable */
FindVarMethod	var_nlmConfigLogTable;
WriteMethod 	write_nlmConfigLogFilterName;
WriteMethod 	write_nlmConfigLogAdminStatus;
WriteMethod 	write_nlmConfigLogStorageType;
#if 1
Netsnmp_Node_Handler do_nlmConfigLogFilterName;
Netsnmp_Node_Handler do_nlmConfigLogEntryLimit;
Netsnmp_Node_Handler do_nlmConfigLogAdminStatus;
Netsnmp_Node_Handler do_nlmConfigLogOperStatus;
Netsnmp_Node_Handler do_nlmConfigLogStorageType;
Netsnmp_Node_Handler do_nlmConfigLogEntryStatus;
#endif

/* nlmStatsLogTable */
FindVarMethod	var_nlmStatsLogTable;
#if 1
Netsnmp_Node_Handler do_nlmStatsLogNotificationsLogged;
Netsnmp_Node_Handler do_nlmStatsLogNotificationsBumped;
#endif


/* nlmLogTable */
FindVarMethod	var_nlmLogTable;

/* nlmLogVariableTable */
FindVarMethod	var_nlmLogVariableTable;

void init_NotificationLogMIB(void);
void init_nlmConfigLogTable(void);
void init_nlmStatsLogTable(void);
void init_nlmLogTable(void);
void init_nlmLogVariableTable(void);
#endif		/* NOTIFICATIONLOGMIB_H */
