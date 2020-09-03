#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portauthsrvc_os.h"
#include "l_stdlib.h"
#include "netaccess_backdoor.h"

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
#include "swctrl.h"
#include "vlan_mgr.h"
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
#include "l4_pmgr.h"
#include "swctrl.h"
#include "pri_mgr.h"
#include "sys_module.h"
#include "sys_callback_mgr.h"
#endif

#define LOG(fmt, args...) \
    {                                       \
        if(debug()) {printf(fmt, ##args);}  \
    }

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
static
BOOL_T PORTAUTHSRVC_OS_Vlan_ValidityCheck(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    );

static
BOOL_T PORTAUTHSRVC_OS_Vlan_Apply(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    );

static
BOOL_T PORTAUTHSRVC_OS_Vlan_Restore(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    );
#else
static
BOOL_T PORTAUTHSRVC_OS_Vlan_Null(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    );
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
static
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

#if (SYS_CPNT_ACL_IPV6 == TRUE)
static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );
#endif /* #if (SYS_CPNT_ACL_IPV6 == TRUE) */

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    );

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    );
#else
static
BOOL_T PORTAUTHSRVC_OS_Qos_Null(
    UI32_T ifindex,
    const char *key,
    const char *val
    );
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

static PORTAUTHSRVC_OS_Vlan_Command_T portauthsrvc_os_dynamic_vlan_commands[] =
    {
    #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
        {
            PORTAUTHSRVC_OS_Vlan_ValidityCheck,
            PORTAUTHSRVC_OS_Vlan_Apply,
            PORTAUTHSRVC_OS_Vlan_Restore
        },
    #else
        {
            PORTAUTHSRVC_OS_Vlan_Null,
            PORTAUTHSRVC_OS_Vlan_Null,
            PORTAUTHSRVC_OS_Vlan_Null
        },
    #endif
    };

static PORTAUTHSRVC_OS_Qos_Command_T portauthsrvc_os_dynamic_qos_commands[] =
    {
    #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
        {
            "service-policy-in=",
            0,
            PORTAUTHSRVC_OS_Qos_DiffServ_Check,
            PORTAUTHSRVC_OS_Qos_DiffServ_Commit,
            PORTAUTHSRVC_OS_Qos_DiffServ_Restore
        },

        {
            "rate-limit-input=",
            0,
            PORTAUTHSRVC_OS_Qos_RateLimit_Check,
            PORTAUTHSRVC_OS_Qos_RateLimit_Commit,
            PORTAUTHSRVC_OS_Qos_RateLimit_Restore
        },

        {
            "switchport-priority-default=",
            0,
            PORTAUTHSRVC_OS_Qos_8021p_Check,
            PORTAUTHSRVC_OS_Qos_8021p_Commit,
            PORTAUTHSRVC_OS_Qos_8021p_Restore
        },

        {
            "ip-access-group-in=",
            0,
            PORTAUTHSRVC_OS_Qos_Acl_Ip_Check,
            PORTAUTHSRVC_OS_Qos_Acl_Ip_Commit,
            PORTAUTHSRVC_OS_Qos_Acl_Ip_Restore
        },

        #if (SYS_CPNT_ACL_IPV6 == TRUE)
        {
            "ipv6-access-group-in=",
            0,
            PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Check,
            PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Commit,
            PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Restore
        },
        #endif /* #if (SYS_CPNT_ACL_IPV6 == TRUE) */

        {
            "mac-access-group-in=",
            0,
            PORTAUTHSRVC_OS_Qos_Acl_Mac_Check,
            PORTAUTHSRVC_OS_Qos_Acl_Mac_Commit,
            PORTAUTHSRVC_OS_Qos_Acl_Mac_Restore
        },

        {
            "shutdown",
            0,
            PORTAUTHSRVC_OS_Qos_Shutdown_Check,
            PORTAUTHSRVC_OS_Qos_Shutdown_Commit,
            PORTAUTHSRVC_OS_Qos_Shutdown_Restore
        },
    #else
        {
            "(null)",
            0,
            PORTAUTHSRVC_OS_Qos_Null,
            PORTAUTHSRVC_OS_Qos_Null,
            PORTAUTHSRVC_OS_Qos_Null
        },
    #endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
    };

static UI32_T portauthsrvc_os_dbg_no;

void PORTAUTHSRVC_OS_Init()
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    L4_PMGR_Init();
#endif
    NETACCESS_BACKDOOR_Register("portauthsrvc_os", &portauthsrvc_os_dbg_no);
}

PORTAUTHSRVC_OS_Vlan_Command_T* PORTAUTHSRVC_OS_Vlan_Command()
{
    return &portauthsrvc_os_dynamic_vlan_commands[0];
}

PORTAUTHSRVC_OS_Qos_Command_T* PORTAUTHSRVC_OS_Qos_Command(
    int idx
    )
{
    return &portauthsrvc_os_dynamic_qos_commands[idx];
}

UI32_T PORTAUTHSRVC_OS_Qos_CommandNumber()
{
    return sizeof(portauthsrvc_os_dynamic_qos_commands)/sizeof(portauthsrvc_os_dynamic_qos_commands[0]);
}

static BOOL_T debug()
{
    //static int flag = 0;
    //return flag;
    return NETACCESS_BACKDOOR_IsOn(portauthsrvc_os_dbg_no);
}


#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
static
BOOL_T PORTAUTHSRVC_OS_Vlan_ValidityCheck(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    )
{
    UI32_T  unit, port, trunk_id;
    VLAN_OM_VLIST_T *p;
    VLAN_OM_Vlan_Port_Info_T port_info;
    VLAN_MGR_Dot1qVlanStaticEntry_T vlan;

    memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    if (   SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)
        || FALSE == VLAN_MGR_GetPortEntry(ifindex, &port_info)
       )
    {
        return FALSE;
    }

    if (VAL_vlanPortMode_hybrid != port_info.vlan_port_entry.vlan_port_mode)
    {
        LOG("%s port %lu is hybrid port mode\n", __FUNCTION__, ifindex);
        return FALSE;
    }

    for (p=untag_lst; p; p=p->next)
    {
        if (!VLAN_OM_GetDot1qVlanStaticEntry(p->vlan_id, &vlan))
        {
            LOG("%s vlan %lu is not exist\n", __FUNCTION__, p->vlan_id);
            return FALSE;
        }
    }

    for (p=untag_lst; p; p=p->next)
    {
        if (!VLAN_OM_GetDot1qVlanStaticEntry(p->vlan_id, &vlan))
        {
            LOG("%s vlan %lu is not exist\n", __FUNCTION__, p->vlan_id);
            return FALSE;
        }
    }

    LOG("%s OK\n", __FUNCTION__);
    return TRUE;
}

static
BOOL_T PORTAUTHSRVC_OS_Vlan_Apply(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    )
{
    BOOL_T rc = VLAN_MGR_SetAuthorizedVlanList(
        ifindex,
        (pvid==0)?VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID:pvid,
        tag_lst,
        untag_lst,
        FALSE
        );

    LOG("%s %s\n", __FUNCTION__, rc?"OK":"Failed");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Vlan_Restore(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    )
{
    BOOL_T rc = VLAN_MGR_SetAuthorizedVlanList(
        ifindex,
        VLAN_TYPE_DOT1Q_NULL_VLAN_ID,
        NULL,
        NULL,
        FALSE
        );

    LOG("%s %s\n", __FUNCTION__, rc?"OK":"Failed");

    return rc;
}
#else
static
BOOL_T PORTAUTHSRVC_OS_Vlan_Null(
    UI32_T ifindex,
    UI32_T pvid,
    VLAN_OM_VLIST_T *tag_lst,
    VLAN_OM_VLIST_T *untag_lst
    )
{
     LOG("%s\n", __FUNCTION__);

    return TRUE;
}
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */


#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T id;
    UI32_T rc = RULE_TYPE_OK;
    const char *name = val;

    if (strlen(val) <= SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH)
    {
        rc = L4_PMGR_QoS_GetPolicyMapIdByName(name, &id);
    }
    else
    {
        rc = RULE_TYPE_FAIL;
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc;
    const char *name = val;

    rc = L4_PMGR_QoS_BindPort2DynamicPolicyMap(ifindex, name, RULE_TYPE_INBOUND);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_DiffServ_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = L4_PMGR_QoS_UnBindPortFromDynamicPolicyMap(ifindex);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = TRUE;

    if (!L_STDLIB_StrIsDigit((char*)val))
    {
        rc = FALSE;
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = FALSE;
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    rc = SWCTRL_SetDynamicPortIngressRateLimit(ifindex, (UI32_T)atoi(val));

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");
#endif
#endif
    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_RateLimit_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = FALSE;
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    rc = SWCTRL_SetDynamicPortIngressRateLimit(ifindex, 0);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");
#endif
#endif
    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = TRUE;

    if (L_STDLIB_StrIsDigit((char*)val))
    {
        UI32_T priority = (UI32_T)atoi(val);

        if (priority < MIN_dot1dUserPriority || MAX_dot1dUserPriority < priority)
            rc = FALSE;
    }
    else
    {
        rc = FALSE;
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = PRI_MGR_SetDynamicDot1dPortDefaultUserPriority(ifindex, (UI32_T)atoi(val));

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_8021p_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc = PRI_MGR_SetDynamicDot1dPortDefaultUserPriority(ifindex, 255);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = RULE_TYPE_OK;
    const char *name = val;
    UI32_T acl_index;

    if (SYS_ADPT_ACL_MAX_NAME_LEN < strlen(val))
    {
        rc = RULE_TYPE_FAIL;
    }
    else if (L4_PMGR_ACL_GetAclIdByNameAndType(name, RULE_TYPE_IP_STD_ACL, &acl_index) != RULE_TYPE_OK)
    {
        if (L4_PMGR_ACL_GetAclIdByNameAndType(name, RULE_TYPE_IP_EXT_ACL, &acl_index) != RULE_TYPE_OK)
        {
            rc = RULE_TYPE_FAIL;
        }
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    const char *name = val;
    UI32_T rc = L4_PMGR_ACL_BindPort2DynamicAcl(ifindex, name, RULE_TYPE_IP_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ip_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = L4_PMGR_ACL_UnBindPortFromDynamicAcl(ifindex, RULE_TYPE_IP_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

#if (SYS_CPNT_ACL_IPV6 == TRUE)
static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = RULE_TYPE_OK;
    const char *name = val;
    UI32_T acl_index;

    if (SYS_ADPT_ACL_MAX_NAME_LEN < strlen(val))
    {
        rc = RULE_TYPE_FAIL;
    }
    else if (L4_PMGR_ACL_GetAclIdByNameAndType(name, RULE_TYPE_IPV6_STD_ACL, &acl_index) != RULE_TYPE_OK)
    {
        if (L4_PMGR_ACL_GetAclIdByNameAndType(name, RULE_TYPE_IPV6_EXT_ACL, &acl_index) != RULE_TYPE_OK)
        {
            rc = RULE_TYPE_FAIL;
        }
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;

}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    const char *name = val;
    UI32_T rc = L4_PMGR_ACL_BindPort2DynamicAcl(ifindex, name, RULE_TYPE_IPV6_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Ipv6_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = L4_PMGR_ACL_UnBindPortFromDynamicAcl(ifindex, RULE_TYPE_IPV6_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}
#endif /* #if (SYS_CPNT_ACL_IPV6 == TRUE) */

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = RULE_TYPE_OK;
    const char *name = val;
    UI32_T acl_index;

    if (SYS_ADPT_ACL_MAX_NAME_LEN < strlen(val))
    {
        rc = RULE_TYPE_FAIL;
    }
    else if (L4_PMGR_ACL_GetAclIdByNameAndType(name, RULE_TYPE_MAC_ACL, &acl_index) != RULE_TYPE_OK)
    {
        rc = RULE_TYPE_FAIL;
    }

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;

}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    const char *name = val;
    UI32_T rc = L4_PMGR_ACL_BindPort2DynamicAcl(ifindex, name, RULE_TYPE_MAC_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Acl_Mac_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T rc = L4_PMGR_ACL_UnBindPortFromDynamicAcl(ifindex, RULE_TYPE_MAC_ACL, TRUE);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == RULE_TYPE_OK)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == RULE_TYPE_OK)?TRUE:FALSE;

}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Check(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    UI32_T              unit, port, trunk_id;
    SWCTRL_Lport_Type_T rc;

    rc = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == SWCTRL_LPORT_NORMAL_PORT)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return (rc == SWCTRL_LPORT_NORMAL_PORT)?TRUE:FALSE;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Commit(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc;

    rc = SYS_CALLBACK_MGR_SetPortStatusCallback(
         SYS_MODULE_NETACCESS, ifindex,
         FALSE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_DYNAMIC_QOS);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == TRUE)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

static
BOOL_T PORTAUTHSRVC_OS_Qos_Shutdown_Restore(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    BOOL_T rc;

    rc = SYS_CALLBACK_MGR_SetPortStatusCallback(
         SYS_MODULE_NETACCESS, ifindex,
         TRUE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_DYNAMIC_QOS);

    LOG("%s %s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        (rc == TRUE)?"OK":"Failed",
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return rc;
}

#else
static
BOOL_T PORTAUTHSRVC_OS_Qos_Null(
    UI32_T ifindex,
    const char *key,
    const char *val
    )
{
    LOG("%s, ifindex=%lu key=\"%s\" val=\"%s\"\n",
        __FUNCTION__,
        ifindex,
        key?key:"(null)",
        val?val:"(null)");

    return TRUE;
}
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
