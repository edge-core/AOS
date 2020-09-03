
#include "sys_cpnt.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "rfc_3014.h"
#include "nlm_mgr.h"
#include "snmp_mgr.h"

/********************************************
 ***********nlmConfigGlobalEntryLimit*******
 ********************************************
 */
int do_nlmConfigGlobalEntryLimit(netsnmp_mib_handler		  *handler,
						   				netsnmp_handler_registration  *reginfo,
						   				netsnmp_agent_request_info	  *reqinfo,
						   				netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            UI32_T limit_value;

            if (!NLM_MGR_GetGlobalEntryLimit(&limit_value))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = limit_value;
            snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
            if (requests->requestvb->type != ASN_GAUGE)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
        {
            UI32_T value;

            if (TRUE != NLM_MGR_GetGlobalEntryLimit(&value))
            {
                return SNMP_ERR_GENERR;
            }

            /* not support set action now. can set to the original value only, or return badValue (ref. RFC1369)
             */
            if (value != *requests->requestvb->val.integer)
            {
                return SNMP_ERR_BADVALUE;
            }
			break;
        }

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

/********************************************
 ***********nlmConfigGlobalAgeOut************
 ********************************************
 */
int do_nlmConfigGlobalAgeOut(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            UI32_T ageout_value;

            if (!NLM_MGR_GetGlobalAgeOut(&ageout_value))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = ageout_value;
            snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
		{
			UI32_T value;

            if (requests->requestvb->type != ASN_GAUGE)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

			value = (*requests->requestvb->val.integer);

			if ((value < VAL_MIN_NLMCONFIGAGEOUT) || (value > VAL_MAX_NLMCONFIGAGEOUT))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
			}

			break;
		}

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
		{
			UI32_T value;

			value = (*requests->requestvb->val.integer);

			if (!NLM_MGR_SetGlobalAgeOut(value))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
			}

			break;
		}

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

/********************************************
 *************nlmConfigLogTable***************
 ********************************************
 */
int do_nlmConfigLogFilterName(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            strcpy((char *)return_buf, (char *)entry.filter_name);
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) return_buf, strlen((char *)return_buf));
            break;
		}

		case MODE_SET_RESERVE1:
		{
            if (requests->requestvb->type != ASN_OCTET_STR)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

            if (requests->requestvb->val_len < 0 || requests->requestvb->val_len > 32)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
				break;
        	}
		}
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
		{
			UI8_T filter_name[MAXSIZE_snmpNotifyFilterProfileName+1];
			UI8_T index[32];

			index[0] = '\0';
            memcpy(filter_name, requests->requestvb->val.string, requests->requestvb->val_len);
			filter_name[requests->requestvb->val_len] = '\0';

			if (!NLM_MGR_SetConfigLogFilterName(index, filter_name))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
			}

			break;
		}

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmConfigLogEntryLimit(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.entry_limit;
            snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
            if (requests->requestvb->type != ASN_GAUGE)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
        {
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            /* not support set action now. can set to the original value only, or return badValue (ref. RFC1369)
             */
            if (entry.entry_limit != *requests->requestvb->val.integer)
            {
                return SNMP_ERR_BADVALUE;
            }
			break;
        }

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmConfigLogAdminStatus(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.admin_status;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *)&long_return, sizeof(long_return));
            break;
		}

		case MODE_SET_RESERVE1:
		{
			UI32_T value;

            if (requests->requestvb->type != ASN_INTEGER)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

			value = (*requests->requestvb->val.integer);

			if ((value < VAL_MIN_NLMCONFIGADMINSTATUS) || (value > VAL_MAX_NLMCONFIGADMINSTATUS))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
			}

			break;
		}

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
		{
			UI8_T value;
			UI8_T index[32];

			index[0] = '\0';
			value = (*requests->requestvb->val.integer);

			if (!NLM_MGR_SetConfigLogAdminStatus(index, value))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
			}

			break;
		}

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmConfigLogOperStatus(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.oper_status;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmConfigLogStorageType(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.storage_type;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *)&long_return, sizeof(long_return));
            break;
		}

		case MODE_SET_RESERVE1:
		{
			UI32_T value;

            if (requests->requestvb->type != ASN_INTEGER)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

			value = (*requests->requestvb->val.integer);

			if ((value < VAL_MIN_NLMCONFIGSTORAGETYPE) || (value > VAL_MAX_NLMCONFIGSTORAGETYPE))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
			}

			break;
		}

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
		{
			UI8_T value;
			UI8_T index[32];

			index[0] = '\0';
			value = (*requests->requestvb->val.integer);

			if (!NLM_MGR_SetConfigLogStorageType(index, value))
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
			}

			break;
		}

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmConfigLogEntryStatus(netsnmp_mib_handler		  *handler,
						   			netsnmp_handler_registration  *reginfo,
						   			netsnmp_agent_request_info	  *reqinfo,
						   			netsnmp_request_info 		  *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmConfigLog_T entry;

            memset(&entry, 0, sizeof(nlmConfigLog_T));

            if (!NLM_MGR_GetConfigLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.entry_status;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
            if (requests->requestvb->type != ASN_INTEGER)
        	{
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				break;
        	}

		case MODE_SET_RESERVE2:
            if ((*requests->requestvb->val.integer) < 1 || (*requests->requestvb->val.integer) > 6)
            {
                return SNMP_ERR_WRONGVALUE;
            }
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

void
init_nlmConfigLogTable(void)
{
	static oid	nlmConfigLogFilterName_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 2, 0};
	static oid	nlmConfigLogEntryLimit_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 3, 0};
	static oid	nlmConfigLogAdminStatus_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 4, 0};
	static oid	nlmConfigLogOperStatus_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 5, 0};
	static oid	nlmConfigLogStorageType_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 6, 0};
	static oid	nlmConfigLogEntryStatus_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 3, 1, 7, 0};

	netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogEntryLimit",
                               do_nlmConfigLogEntryLimit,
                               nlmConfigLogEntryLimit_oid,
                               OID_LENGTH(nlmConfigLogEntryLimit_oid),
                               HANDLER_CAN_RWRITE));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogFilterName",
                               do_nlmConfigLogFilterName,
                               nlmConfigLogFilterName_oid,
                               OID_LENGTH(nlmConfigLogFilterName_oid),
                               HANDLER_CAN_RWRITE));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogAdminStatus",
                               do_nlmConfigLogAdminStatus,
                               nlmConfigLogAdminStatus_oid,
                               OID_LENGTH(nlmConfigLogAdminStatus_oid),
                               HANDLER_CAN_RWRITE));

	netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogOperStatus",
                               do_nlmConfigLogOperStatus,
                               nlmConfigLogOperStatus_oid,
                               OID_LENGTH(nlmConfigLogOperStatus_oid),
                               HANDLER_CAN_RONLY));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogStorageType",
                               do_nlmConfigLogStorageType,
                               nlmConfigLogStorageType_oid,
                               OID_LENGTH(nlmConfigLogStorageType_oid),
                               HANDLER_CAN_RWRITE));

	netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigLogEntryStatus",
                               do_nlmConfigLogEntryStatus,
                               nlmConfigLogEntryStatus_oid,
                               OID_LENGTH(nlmConfigLogEntryStatus_oid),
                               HANDLER_CAN_RWRITE));
}

//static oid	nlmConfigLogTable_variables_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1};

struct variable3 nlmConfigLogTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
	{NLMLOGNAME, 				ASN_OCTET_STR,	NOACCESS,	var_nlmConfigLogTable,
	 3, {3, 1, 1}},
#endif
    {NLMCONFIGLOGFILTERNAME, 	ASN_OCTET_STR,	RWRITE,     var_nlmConfigLogTable,
     3, {3, 1, 2}},
    {NLMCONFIGLOGENTRYLIMIT, 	ASN_UNSIGNED,	RWRITE,     var_nlmConfigLogTable,
     3, {3, 1, 3}},
    {NLMCONFIGLOGADMINSTATUS, 	ASN_INTEGER,	RWRITE,     var_nlmConfigLogTable,
     3, {3, 1, 4}},
    {NLMCONFIGLOGOPERSTATUS, 	ASN_INTEGER,	RONLY,     	var_nlmConfigLogTable,
     3, {3, 1, 5}},
    {NLMCONFIGLOGSTORAGETYPE, 	ASN_INTEGER,	RWRITE,     var_nlmConfigLogTable,
     3, {3, 1, 6}},
    {NLMCONFIGLOGENTRYSTATUS, 	ASN_INTEGER,	RWRITE,     var_nlmConfigLogTable,
     3, {3, 1, 7}},
};

static int
header_nlmConfigLogTable(struct variable *vp,
               					oid * name, size_t *length,
               					int exact, size_t *var_len,
               					WriteMethod **write_method,
               					UI8_T *index, UI32_T *index_len)
{
	oid		newname[MAX_OID_LEN];
	int		result;
    int		return_val;
	oid		next_inst[1+32];
	int 	i;
	nlmConfigLog_T  entry;
	UI32_T	oid_name_length = 12;

	memcpy((char *)newname, (char *)vp->name,
           (int)vp->namelen * sizeof(oid));

    memset(&entry, 0 , sizeof(entry));

	while ((return_val = NLM_MGR_GetNextConfigLogEntry(&entry)))
	{
		next_inst[0] = strlen((char *)entry.name);
		for (i=0; i<next_inst[0]; i++)
		{
 			next_inst[i+1] = entry.name[i];
		}

        memcpy((char*)&newname[oid_name_length], (char*)next_inst, (1+strlen((char *)entry.name))*sizeof(oid));

        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen+1+strlen((char *)entry.name));
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
	}

	if (!return_val)
	{
        return MATCH_FAILED;
    }

    memcpy((char *)name, (char *)newname,
           ((int)vp->namelen + 1 + strlen((char *)entry.name)) * sizeof(oid));
    *length = vp->namelen + 1 + strlen((char *)entry.name);
	*var_len = sizeof(long);

	strcpy((char *)index, (char *)entry.name);
	*index_len = strlen((char *)index);

    return MATCH_SUCCEEDED;
}

unsigned char *
var_nlmConfigLogTable(struct variable *vp, oid *name,
							 size_t	*length, int exact,
							 size_t	*var_len, WriteMethod **write_method)
{
    UI32_T index_len = 0, retval;
    UI8_T index[32];

    nlmConfigLog_T entry;

	memset(&entry, 0, sizeof(entry));

	switch(vp->magic)
	{
		case NLMCONFIGLOGFILTERNAME:
			*write_method = write_nlmConfigLogFilterName;
			break;

		case NLMCONFIGLOGADMINSTATUS:
			*write_method = write_nlmConfigLogAdminStatus;
			break;

		case NLMCONFIGLOGSTORAGETYPE:
			*write_method = write_nlmConfigLogStorageType;
			break;

		default:
			*write_method = 0;
	}

	retval = header_nlmConfigLogTable(vp, name, length, exact, var_len, write_method, index, &index_len);

    if (retval == MATCH_FAILED)
        return NULL;

    memcpy(entry.name, index, index_len);

    if (!NLM_MGR_GetConfigLogEntry(&entry))
		return NULL;

    switch (vp->magic)
	{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
		case NLMLOGNAME:
        	strcpy((char *)return_buf, (char *)index);
        	*var_len = strlen((char *)return_buf);
        	return (u_char *)return_buf;
#endif

		case NLMCONFIGLOGFILTERNAME:
        	strcpy((char *)return_buf, (char *)entry.filter_name);
        	*var_len = strlen((char *)return_buf);
        	return (u_char *)return_buf;

		case NLMCONFIGLOGENTRYLIMIT:
        	long_return = entry.entry_limit;
        	return (u_char *)&long_return;

		case NLMCONFIGLOGADMINSTATUS:
        	long_return = entry.entry_limit;
        	return (u_char *)&long_return;

		case NLMCONFIGLOGOPERSTATUS:
        	long_return = entry.entry_limit;
        	return (u_char *)&long_return;

		case NLMCONFIGLOGSTORAGETYPE:
        	long_return = entry.entry_limit;
        	return (u_char *)&long_return;

		case NLMCONFIGLOGENTRYSTATUS:
        	long_return = entry.entry_limit;
        	return (u_char *)&long_return;

		default:
        	ERROR_MSG("");
    }

    return NULL;
}

int
write_nlmConfigLogFilterName(int	action,
									u_char	*var_val,
									u_char	var_val_type,
									size_t	var_val_len,
									u_char	*statP,
									oid 	*name,
									size_t	name_len)
{
	int size,i;
    UI8_T index[32], filter_name[MAXSIZE_snmpNotifyFilterProfileName+1];
    UI32_T oid_name_length = 12;

    for (i=0; i<name[oid_name_length]; i++)
    {
		index[i] = name[oid_name_length+1+i];
    }
    index[name[oid_name_length]] = '\0';

	switch (action)
	{
		case RESERVE1:
			if (var_val_type != ASN_OCTET_STR)
			{
            	return SNMP_ERR_WRONGTYPE;
			}

			if (var_val_len > 32 * sizeof(UI8_T))
			{
            	return SNMP_ERR_WRONGLENGTH;
			}

          	break;

        case RESERVE2:
			break;

        case FREE:
        	break;

        case ACTION:
        	size = var_val_len;
        	memcpy(filter_name, var_val, size);
			filter_name[size] = '\0';

			if (!NLM_MGR_SetConfigLogFilterName(index, filter_name))
			{
				return SNMP_ERR_COMMITFAILED;
			}

			break;

        case UNDO:
        	break;

        case COMMIT:
        	break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_nlmConfigLogAdminStatus(int	  action,
									  u_char  *var_val,
									  u_char  var_val_type,
									  size_t  var_val_len,
									  u_char  *statP,
									  oid     *name,
									  size_t  name_len)
{
	int i;
    UI8_T index[32];
	UI32_T value;
    UI32_T oid_name_length = 12;

    for (i=0; i<name[oid_name_length]; i++)
    {
		index[i] = name[oid_name_length+1+i];
    }
    index[name[oid_name_length]] = '\0';

	switch (action)
	{
		case RESERVE1:
			if (var_val_type != ASN_INTEGER)
			{
            	return SNMP_ERR_WRONGTYPE;
			}

			if (var_val_len > sizeof(long))
			{
            	return SNMP_ERR_WRONGLENGTH;
			}

          	break;

        case RESERVE2:
			break;

        case FREE:
        	break;

        case ACTION:
        	value = *(long *)var_val;
			if (!NLM_MGR_SetConfigLogAdminStatus(index, value))
			{
				return SNMP_ERR_COMMITFAILED;
			}

			break;

        case UNDO:
        	break;

        case COMMIT:
        	break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_nlmConfigLogStorageType(int   action,
   									  u_char  *var_val,
   									  u_char  var_val_type,
   									  size_t  var_val_len,
   									  u_char  *statP,
   									  oid     *name,
   									  size_t  name_len)
{
	int i;
    UI8_T index[32];
	UI32_T value;
    UI32_T oid_name_length = 12;

    for (i=0; i<name[oid_name_length]; i++)
    {
		index[i] = name[oid_name_length+1+i];
    }
    index[name[oid_name_length]] = '\0';

	switch (action)
	{
		case RESERVE1:
			if (var_val_type != ASN_INTEGER)
			{
            	return SNMP_ERR_WRONGTYPE;
			}

			if (var_val_len > sizeof(long))
			{
            	return SNMP_ERR_WRONGLENGTH;
			}

          	break;

        case RESERVE2:
			break;

        case FREE:
        	break;

        case ACTION:
        	value = *(long *)var_val;
			if (!NLM_MGR_SetConfigLogStorageType(index, value))
			{
				return SNMP_ERR_COMMITFAILED;
			}

			break;

        case UNDO:
        	break;

        case COMMIT:
        	break;
    }
    return SNMP_ERR_NOERROR;
}

/********************************************
 ******nlmStatsGlobalNotificationsLogged*****
 ********************************************
 */
int do_nlmStatsGlobalNotificationsLogged(netsnmp_mib_handler		    *handler,
									   			  netsnmp_handler_registration  *reginfo,
									   			  netsnmp_agent_request_info	*reqinfo,
									   			  netsnmp_request_info 		    *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            UI32_T logged;

            if (!NLM_MGR_GetGlobalNotificationsLogged(&logged))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = logged;
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

/********************************************
 ******nlmStatsGlobalNotificationsBumped*****
 ********************************************
 */
int do_nlmStatsGlobalNotificationsBumped(netsnmp_mib_handler		     *handler,
									   			   netsnmp_handler_registration  *reginfo,
									   			   netsnmp_agent_request_info	 *reqinfo,
									   			   netsnmp_request_info 		 *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            UI32_T Bumped;

            if (!NLM_MGR_GetGlobalNotificationsBumped(&Bumped))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = Bumped;
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

/********************************************
 *****************nlmStatsLogTable***********
 ********************************************
 */
int do_nlmStatsLogNotificationsLogged(netsnmp_mib_handler	*handler,
									  netsnmp_handler_registration  *reginfo,
									  netsnmp_agent_request_info	*reqinfo,
									  netsnmp_request_info 		    *requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmStatsLog_T entry;

			memset(&entry, 0, sizeof(nlmStatsLog_T));

            if (!NLM_MGR_GetStatsLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.logged;
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

int do_nlmStatsLogNotificationsBumped(netsnmp_mib_handler		*handler,
									   	  netsnmp_handler_registration  *reginfo,
									   	  netsnmp_agent_request_info	*reqinfo,
									   	  netsnmp_request_info 		 	*requests)
{
	switch(reqinfo->mode)
	{
		case MODE_GET:
		{
            nlmStatsLog_T entry;

			memset(&entry, 0, sizeof(nlmStatsLog_T));

            if (!NLM_MGR_GetStatsLogEntry(&entry))
            {
                return SNMP_ERR_GENERR;
            }

            long_return = entry.bumped;
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));

            break;
		}

		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			break;

		case MODE_SET_FREE:
			break;

		case MODE_SET_ACTION:
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}

void
init_nlmStatsLogTable(void)
{
	static oid	nlmStatsLogNotificationsLogged_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 2, 3, 1, 1, 0};
	static oid	nlmStatsLogNotificationsBumped_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 2, 3, 1, 2, 0};

	netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                              ("nlmStatsLogNotificationsLogged",
                               do_nlmStatsLogNotificationsLogged,
                               nlmStatsLogNotificationsLogged_oid,
                               OID_LENGTH(nlmStatsLogNotificationsLogged_oid),
                               HANDLER_CAN_RONLY));

	netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                              ("nlmStatsLogNotificationsBumped",
                               do_nlmStatsLogNotificationsBumped,
                               nlmStatsLogNotificationsBumped_oid,
                               OID_LENGTH(nlmStatsLogNotificationsBumped_oid),
                               HANDLER_CAN_RONLY));
}

//static oid	nlmStatsLogTable_variables_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 2};

struct variable3 nlmStatsLogTable_variables[] =
{
    {NLMSTATSLOGNOTIFICATIONSLOGGED,	ASN_COUNTER,	RONLY,	var_nlmStatsLogTable,
     3, {3, 1, 1}},
    {NLMSTATSLOGNOTIFICATIONSBUMPED,	ASN_COUNTER,	RONLY,	var_nlmStatsLogTable,
     3, {3, 1, 2}},
};

static int
header_nlmStatsLogTable(struct variable *vp,
               				    oid *name, size_t *length,
               					int exact, size_t *var_len,
               					WriteMethod **write_method,
               					UI8_T *index, UI32_T *index_len)
{
	oid		newname[MAX_OID_LEN];
	int		result;
    int		return_val;
	oid		next_inst[1+32];
	int 	i;
	nlmStatsLog_T  entry;
	UI32_T	oid_name_length = 12;

	memcpy((char *)newname, (char *)vp->name,
           (int)vp->namelen * sizeof(oid));

    memset(&entry, 0 , sizeof(entry));

	while ((return_val = NLM_MGR_GetNextStatsLogEntry(&entry)))
	{
		next_inst[0] = strlen((char *)entry.name);
		for (i=0; i<next_inst[0]; i++)
		{
 			next_inst[i+1] = entry.name[i];
		}

        memcpy((char*)&newname[oid_name_length], (char*)next_inst, (1+strlen((char *)entry.name))*sizeof(oid));

        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen+1+strlen((char *)entry.name));
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
	}

	if (!return_val)
	{
        return MATCH_FAILED;
    }

    memcpy((char *)name, (char *)newname,
           ((int)vp->namelen + 1 + strlen((char *)entry.name)) * sizeof(oid));
    *length = vp->namelen + 1 + strlen((char *)entry.name);
	*var_len = sizeof(long);

	strcpy((char *)index, (char *)entry.name);
	*index_len = strlen((char *)index);

    return TRUE;
}

unsigned char *
var_nlmStatsLogTable(struct variable *vp, oid *name,
						   size_t *length, int exact,
						   size_t *var_len, WriteMethod **write_method)
{
    UI32_T index_len = 0, retval;
    UI8_T index[32];

    nlmStatsLog_T entry;

	memset(&entry, 0, sizeof(entry));

	retval = header_nlmStatsLogTable(vp, name, length, exact, var_len, write_method, index, &index_len);

    if (retval == MATCH_FAILED)
        return NULL;

    memcpy(entry.name, index, index_len);

    if (!NLM_MGR_GetStatsLogEntry(&entry))
		return NULL;

    switch (vp->magic)
	{
		case NLMSTATSLOGNOTIFICATIONSLOGGED:
        	long_return = entry.logged;
        	return (u_char *)&long_return;

		case NLMSTATSLOGNOTIFICATIONSBUMPED:
        	long_return = entry.bumped;
        	return (u_char *)&long_return;

		default:
        	ERROR_MSG("");
    }

    return NULL;
}

/********************************************
 ******************nlmLogTable***************
 ********************************************
 */
static oid	nlmLogTable_variables_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 3};

struct variable3 nlmLogTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
	{NLMLOGINDEX, 				ASN_UNSIGNED,	NOACCESS,	var_nlmLogTable,
	 3, {1, 1, 1}},
#endif
    {NLMLOGTIME, 				ASN_TIMETICKS,	RONLY,		var_nlmLogTable,
     3, {1, 1, 2}},
    {NLMLOGENGINEID, 			ASN_OCTET_STR,	RONLY,		var_nlmLogTable,
     3, {1, 1, 4}},
    {NLMLOGENGINETADDRESS, 		ASN_OCTET_STR,	RONLY,		var_nlmLogTable,
     3, {1, 1, 5}},
    {NLMLOGENGINETDOMAIN, 		ASN_OBJECT_ID,	RONLY,		var_nlmLogTable,
     3, {1, 1, 6}},
    {NLMLOGCONTEXTENGINEID, 	ASN_OCTET_STR,	RONLY,		var_nlmLogTable,
     3, {1, 1, 7}},
    {NLMLOGCONTEXTNAME, 		ASN_OCTET_STR,	RONLY,		var_nlmLogTable,
     3, {1, 1, 8}},
    {NLMLOGNOTIFICATIONID, 		ASN_OBJECT_ID,	RONLY,		var_nlmLogTable,
     3, {1, 1, 9}},
};

void
init_nlmLogTable(void)
{
    REGISTER_MIB("nlmLogTable",			nlmLogTable_variables,			variable3,
                 nlmLogTable_variables_oid);
}

static int
header_nlmLogTable(struct variable *vp,
               			oid * name, size_t *length,
               			int exact, size_t *var_len,
               			WriteMethod **write_method,
               			UI8_T *index1, UI32_T *index2,
               			UI32_T *index1_len)
{
	oid		newname[MAX_OID_LEN];
	int		result;
    int		return_val;
	nlmLog_T  entry;

#if 0
	memcpy((char *)newname, (char *)vp->name,
           (int)vp->namelen * sizeof(oid));

    memset(&entry, 0 , sizeof(entry));
	while ((return_val = NLM_MGR_GetNextLogEntry(&entry)))
	{
		newname[vp->namelen] = 0;
     	newname[vp->namelen+1] = entry.index;

        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen+1+strlen(entry.name)+1);
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
	}

	if (!return_val)
	{
        return MATCH_FAILED;
    }

    memcpy((char *)name, (char *)newname,
           ((int)vp->namelen+1+1) * sizeof(oid));
    *length = vp->namelen + 1 + 1;
	*var_len = sizeof(long);

	strcpy(index1, entry.name);
	*index1_len = strlen(index1);
	*index2 = entry.index;
#else
	memset(&entry, 0, sizeof(entry));

	result = snmp_oid_compare(name, vp->namelen, vp->name, vp->namelen);

	if (exact == 1)
	{
		if ((result != 0) || (*length != (vp->namelen + 2)))
			return MATCH_FAILED;

		index1[0] = name[vp->namelen];
		*index1_len = 0;
		*index2 = name[vp->namelen + 1];
	}
	else
	{
		if (result == 0)
		{
			if (*length >= (vp->namelen + 2))
			{
				entry.name[0] = 0;
				entry.index = name[vp->namelen + 1];
			}
		}

		memcpy(newname, vp->name, vp->namelen * sizeof(oid));

		while ((return_val = NLM_MGR_GetNextLogEntry(&entry)))
		{
			newname[vp->namelen] = 0;
	     	newname[vp->namelen+1] = entry.index;

	        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen+1+1);
	        if (result < 0)
	            break;
		}

		if (!return_val)
		{
	        return MATCH_FAILED;
	    }

	    memcpy((char *)name, (char *)newname,
	           ((int)vp->namelen+1+1) * sizeof(oid));
	    *length = vp->namelen + 1 + 1;

		strcpy((char *)index1, (char *)entry.name);
		*index1_len = strlen((char *)index1);
		*index2 = entry.index;
	}
#endif
	*var_len = sizeof(long);

    return MATCH_SUCCEEDED;
}

unsigned char *
var_nlmLogTable(struct variable *vp, oid *name,
					 size_t	*length, int exact,
					 size_t	*var_len, WriteMethod **write_method)
{
    UI32_T index1_len, retval;
    UI8_T index1[32];
	UI32_T index2 = 0;
    nlmLog_T entry;

	memset(&entry, 0, sizeof(entry));

	retval = header_nlmLogTable(vp, name, length, exact, var_len, write_method, index1, &index2, &index1_len);

    if (retval == MATCH_FAILED)
    {
        return NULL;
    }

    entry.name[0] = '\0';
	entry.index = index2;
    if (!NLM_MGR_GetLogEntry(&entry))
    {
		return NULL;
    }

    switch (vp->magic)
	{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
		case NLMLOGINDEX:
        	long_return = entry.index;
        	return (u_char *)&long_return;
#endif

		case NLMLOGTIME:
        	long_return = entry.time;
        	return (u_char *) &long_return;

		case NLMLOGENGINEID:
        	memcpy(return_buf, entry.engineid, entry.engineid_len);
        	*var_len = entry.engineid_len;
        	return (u_char *)return_buf;

		case NLMLOGENGINETADDRESS:
			/* a TAddress is 6 octets long */
        	memcpy(return_buf, entry.taddress, 6);
        	*var_len = 6;
        	return (u_char *)return_buf;

		case NLMLOGENGINETDOMAIN:
            *var_len = entry.tdomain.oid_len * sizeof(UI32_T);
            memcpy(oid_return, entry.tdomain.oid, *var_len);
        	return (u_char *)oid_return;

		case NLMLOGCONTEXTENGINEID:
        	memcpy(return_buf, entry.contextengineid, entry.contextengineid_len);
        	*var_len = entry.contextengineid_len;
        	return (u_char *)return_buf;

		case NLMLOGCONTEXTNAME:
        	strcpy((char *)return_buf, (char *)entry.contextname);
        	*var_len = strlen((char *)return_buf);
        	return (u_char *)return_buf;

		case NLMLOGNOTIFICATIONID:
            *var_len = entry.notificationid.oid_len * sizeof(UI32_T);
            memcpy(oid_return, entry.notificationid.oid, *var_len);
        	return (u_char *)oid_return;

		default:
        	ERROR_MSG("");
    }

    return NULL;
}

/********************************************
 ************nlmLogVariableTable**************
 ********************************************
 */
static oid	nlmLogVariableTable_variables_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 3};

struct variable3 nlmLogVariableTable_variables[] =
{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
	{NLMLOGVARIABLEINDEX,			ASN_UNSIGNED,	NOACCESS,	var_nlmLogVariableTable,
	 3, {2, 1, 1}},
#endif
    {NLMLOGVARIABLEID, 				ASN_OBJECT_ID,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 2}},
    {NLMLOGVARIABLEVALUETYPE,		ASN_INTEGER,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 3}},
    {NLMLOGVARIABLECOUNTER32VAL,	ASN_COUNTER,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 4}},
    {NLMLOGVARIABLEUNSIGNED32VAL,	ASN_UNSIGNED,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 5}},
    {NLMLOGVARIABLETIMETICKSVAL,	ASN_TIMETICKS,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 6}},
    {NLMLOGVARIABLEINTEGER32VAL, 	ASN_INTEGER,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 7}},
    {NLMLOGVARIABLEOCTETSTRINGVAL,	ASN_OCTET_STR,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 8}},
    {NLMLOGVARIABLEIPADDRESSVAL,	ASN_IPADDRESS,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 9}},
    {NLMLOGVARIABLEOIDVAL,			ASN_OBJECT_ID,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 10}},
    {NLMLOGVARIABLECOUNTER64VAL,	ASN_COUNTER64,	RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 11}},
    {NLMLOGVARIABLEOPAQUEVAL,		ASN_OPAQUE,		RONLY,		var_nlmLogVariableTable,
     3, {2, 1, 12}},
};

void
init_nlmLogVariableTable(void)
{
    REGISTER_MIB("nlmLogVariableTable", nlmLogVariableTable_variables,	variable3,
                 nlmLogVariableTable_variables_oid);
}

static int
header_nlmLogVariableTable(struct variable *vp,
               			oid * name, size_t *length,
               			int exact, size_t *var_len,
               			WriteMethod **write_method,
               			UI8_T *index1, UI32_T *index2,
               			UI32_T *index3, UI32_T *index1_len)
{
	oid		newname[MAX_OID_LEN];
	int		result;
    int		return_val;
	nlmLogVariable_T  entry;

#if 0
	oid		next_inst[3];
	UI32_T	oid_name_length = 12;

	memcpy((char *)newname, (char *)vp->name,
           (int)vp->namelen * sizeof(oid));

    memset(&entry, 0 , sizeof(entry));

	while ((return_val = NLM_MGR_GetNextLogVariableEntry(&entry)))
	{
		next_inst[0] = '\0';
     	next_inst[1] = entry.index;
		next_inst[2] = entry.var_index;

        memcpy((char*)&newname[oid_name_length], (char*)next_inst, 3 * sizeof(oid));

        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen + 1 + 2);
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
	}

	if (!return_val)
	{
        return MATCH_FAILED;
    }

    memcpy((char *)name, (char *)newname,
           ((int)vp->namelen + 3) * sizeof(oid));
    *length = vp->namelen + 1 + 2;
	*var_len = sizeof(long);

	strcpy(index1, entry.name);
	*index1_len = 0;
	*index2 = entry.index;
	*index3 = entry.var_index;
#else
	memset(&entry, 0, sizeof(entry));

	result = snmp_oid_compare(name, vp->namelen, vp->name, vp->namelen);

	if (exact == 1)
	{
		if ((result != 0) || (*length != (vp->namelen + 3)))
			return MATCH_FAILED;

		index1[0] = name[vp->namelen];
		*index1_len = 0;
		*index2 = name[vp->namelen + 1];
		*index3 = name[vp->namelen + 2];
	}
	else
	{
		if (result == 0)
		{
			if (*length >= (vp->namelen + 3))
			{
				entry.name[0] = name[vp->namelen];
				entry.index = name[vp->namelen + 1];
				entry.var_index = name[vp->namelen + 2];
			}
		}

		memcpy(newname, vp->name, vp->namelen * sizeof(oid));

		while ((return_val = NLM_MGR_GetNextLogVariableEntry(&entry)))
		{
			newname[vp->namelen] = 0;
	     	newname[vp->namelen+1] = entry.index;
	     	newname[vp->namelen+2] = entry.var_index;

	        result = snmp_oid_compare(name, *length, newname, (int)vp->namelen+1+strlen((char *)entry.name)+2);
	        if (result < 0)
	            break;
		}

		if (!return_val)
		{
	        return MATCH_FAILED;
	    }

	    memcpy((char *)name, (char *)newname,
	           ((int)vp->namelen+1+2) * sizeof(oid));
	    *length = vp->namelen + 1 + 2;

		strcpy((char *)index1, (char *)entry.name);
		*index1_len = strlen((char *)index1);
		*index2 = entry.index;
		*index3 = entry.var_index;
	}
#endif
	*var_len = sizeof(long);

    return MATCH_SUCCEEDED;
}

unsigned char *
var_nlmLogVariableTable(struct variable *vp, oid *name,
					 size_t	*length, int exact,
					 size_t	*var_len, WriteMethod **write_method)
{
    UI32_T index1_len, retval;
    UI8_T index1[32];
	UI32_T index2 = 0, index3 = 0;

    nlmLogVariable_T entry;

	memset(&entry, 0, sizeof(entry));

	retval = header_nlmLogVariableTable(vp, name, length, exact, var_len, write_method,
		index1, &index2, &index3, &index1_len);

    if (retval == MATCH_FAILED)
    {
        return NULL;
    }

    entry.name[0] = '\0';
	entry.index = index2;
	entry.var_index = index3;

    if (!NLM_MGR_GetLogVariableEntry(&entry))
    {
		return NULL;
    }

    switch (vp->magic)
	{
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
		case NLMLOGVARIABLEINDEX:
        	long_return = entry.var_index;
        	return (u_char *)&long_return;
#endif

		case NLMLOGVARIABLEID:
            *var_len = entry.var_id->oid_len * sizeof(UI32_T);
            memcpy(oid_return, entry.var_id->oid, *var_len);
        	return (u_char *)oid_return;

		case NLMLOGVARIABLEVALUETYPE:
        	switch (entry.var_type)
        	{
				case ASN_COUNTER:
        			long_return = NLM_COUNTER32;
					break;
				case ASN_UNSIGNED:
        			long_return = NLM_UINTEGER32;
					break;
				case ASN_TIMETICKS:
        			long_return = NLM_TIMETICKS;
					break;
				case ASN_INTEGER:
        			long_return = NLM_INTEGER32;
					break;
				case ASN_IPADDRESS:
        			long_return = NLM_IPADDRESS;
					break;
				case ASN_OCTET_STR:
        			long_return = NLM_OCTETSTRING;
					break;
				case ASN_OBJECT_ID:
        			long_return = NLM_OBJECTID;
					break;
				case ASN_COUNTER64:
        			long_return = NLM_COUNTER64;
					break;
				case ASN_OPAQUE:
        			long_return = NLM_OPAQUE;
					break;
				default:
					return NULL;
        	}

        	return (u_char *)&long_return;

		case NLMLOGVARIABLECOUNTER32VAL:
			if (entry.var_type != ASN_COUNTER)
			{
				long_return = 0;
			}
			else
			{
        		long_return = entry.u.counter32;
			}

        	return (u_char *)&long_return;

		case NLMLOGVARIABLEUNSIGNED32VAL:
			if (entry.var_type != ASN_UNSIGNED)
			{
				long_return = 0;
			}
			else
			{
        		long_return = entry.u.unsigned32;
			}
        	return (u_char *)&long_return;

		case NLMLOGVARIABLETIMETICKSVAL:
			if (entry.var_type != ASN_TIMETICKS)
			{
				long_return = 0;
			}
			else
			{
        		long_return = entry.u.timeticks;
			}

        	return (u_char *)&long_return;

		case NLMLOGVARIABLEINTEGER32VAL:
			if (entry.var_type != ASN_INTEGER)
			{
				long_return = 0;
			}
			else
			{
        		long_return = entry.u.integer32;
			}

        	return (u_char *)&long_return;

		case NLMLOGVARIABLEIPADDRESSVAL:
            *var_len = sizeof(ipaddr_return);
			if (entry.var_type != ASN_IPADDRESS)
			{
				ipaddr_return = 0;
			}
			else
			{
        		ipaddr_return = entry.u.ipaddress;
			}

        	return (u_char *)&ipaddr_return;

		case NLMLOGVARIABLEOCTETSTRINGVAL:
			if (entry.var_type != ASN_OCTET_STR)
			{
				return_buf[0] = '\0';
        		*var_len = 0;
			}
			else
			{
        		memcpy(return_buf, entry.u.octetstring, entry.var_len);
				return_buf[entry.var_len] = '\0';
        		*var_len = entry.var_len;
			}

        	return (u_char *)return_buf;

		case NLMLOGVARIABLEOIDVAL:
			if (entry.var_type != ASN_OBJECT_ID)
			{
				*var_len = 0;
				oid_return[0] = '\0';
			}
			else
			{
           		*var_len = entry.u.objectid.oid_len * sizeof(oid);
            	memcpy(oid_return, entry.u.objectid.oid, *var_len);
			}

        	return (u_char *)oid_return;

		case NLMLOGVARIABLECOUNTER64VAL:
			if (entry.var_type != ASN_COUNTER64)
			{
				memset(&long64_return, 0, 8);
			}
			else
			{
                SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.u.counter64);
//        		long64_return = entry.u.counter64;
			}

			*var_len = sizeof(long64_return);
        	return (u_char *)&long64_return;

		case NLMLOGVARIABLEOPAQUEVAL:
			if (entry.var_type != ASN_OPAQUE)
			{
				return_buf[0] = '\0';
        		*var_len = 0;
			}
			else
			{
        		memcpy(return_buf, entry.u.opaque, entry.var_len);
				return_buf[entry.var_len] = '\0';
        		*var_len = entry.var_len;
			}

        	return (u_char *)return_buf;

		default:
        	ERROR_MSG("");
    }

    return NULL;
}

void
init_NotificationLogMIB(void)
{
    static oid	nlmConfigGlobalEntryLimit_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 1, 0};
    static oid	nlmConfigGlobalAgeOut_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 1, 2, 0};
    static oid	nlmStatsGlobalNotificationsLogged_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 2, 1, 0};
    static oid	nlmStatsGlobalNotificationsBumped_oid[] = {SNMP_OID_NOTIFICATIONLOGMIB, 1, 2, 2, 0};

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigGlobalEntryLimit",
                               do_nlmConfigGlobalEntryLimit,
                               nlmConfigGlobalEntryLimit_oid,
                               OID_LENGTH(nlmConfigGlobalEntryLimit_oid),
                               HANDLER_CAN_RWRITE));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("nlmConfigGlobalAgeOut",
                               do_nlmConfigGlobalAgeOut,
                               nlmConfigGlobalAgeOut_oid,
                               OID_LENGTH(nlmConfigGlobalAgeOut_oid),
                               HANDLER_CAN_RWRITE));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                              ("nlmStatsGlobalNotificationsLogged",
                               do_nlmStatsGlobalNotificationsLogged,
                               nlmStatsGlobalNotificationsLogged_oid,
                               OID_LENGTH(nlmStatsGlobalNotificationsLogged_oid),
                               HANDLER_CAN_RONLY));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                              ("nlmStatsGlobalNotificationsBumped",
                               do_nlmStatsGlobalNotificationsBumped,
                               nlmStatsGlobalNotificationsBumped_oid,
                               OID_LENGTH(nlmStatsGlobalNotificationsBumped_oid),
                               HANDLER_CAN_RONLY));
}

