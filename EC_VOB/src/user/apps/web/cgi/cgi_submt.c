/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_submt.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * This is the file for handling the CGI submit.

 * CLASSES AND FUNCTIONS:

 * Basic services:

 * HISTORY:
 * 1999-03-23 (Tue): Created by Zhong Qiyao.
 * 2005-08-11 (Thu): Correct and delete some error messages by Wind Lai

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "sys_adpt.h"
#include "cgi.h"
#include "cgi_coretype.h"
#include "cgi_auth.h"
#include "cgi_res.h"
#include "cgi_util.h"
#include "cgi_submt.h"
#include "cgi_multi.h"
#include "cgi_lib.h"
#include "cli_mgr.h"
#include "cli_cmd.h"
#include "cli_pmgr.h"
#include "cli_def.h"
#include "dhcp_mgr.h"
#include "l_cvrt.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "l_md5.h"
#include "l_base64.h"
#include "leaf_sys.h"
#include "leaf_vbridge.h"
#include "mib2_pmgr.h"
//#include "netcfg_mgr.h"
#include "netcfg_type.h"
#include "vlan_pmgr.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "sys_mgr.h"
#include "sys_pmgr.h"
#include "amtr_mgr.h"
#include "amtr_pmgr.h"
#include "sysfun.h"
#include "telnet_mgr.h"
#include "telnet_pmgr.h"
#include "fs.h"
#include "fs_type.h"
#include "xfer_mgr.h"
#include "xfer_pmgr.h"
#include "xfer_buf_mgr.h"
#include "amtr_mgr.h"
#include "http_mgr.h"
#include "syslog_mgr.h"
#include "stkctrl_task.h"
#include "stkctrl_pmgr.h"
#include "trk_mgr.h"
#include "trk_pmgr.h"
#include "lacp_mgr.h"
#include "lacp_pmgr.h"
#include "sys_time.h"
#include "buffer_mgr.h"
#include "cmgr.h"
#include "nmtr_pmgr.h"
#include "ip_lib.h"

#if (SYS_CPNT_ROUTING == TRUE)
#include "netcfg_pmgr_route.h"
#include "nsm_pmgr.h"
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_type.h"
#include "netcfg_pmgr_rip.h"
#endif /* #if (SYS_CPNT_RIP == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
#include "sys_reload_mgr.h"
#endif /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */

#if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))
#include "xstp_type.h"
#include "xstp_mgr.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#endif /* #if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)) */

#if (SYS_CPNT_COS == TRUE)
#include "pri_pmgr.h"
#include "cos_vm.h"
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_QOS_V2 == TRUE)
#include "l4_mgr.h"
#include "l4_pmgr.h"
#else
#include "l4_cos_mgr.h"
#include "l4_cos_pmgr.h"
#include "l4_ds_mgr.h"
#include "l4_ds_pmgr.h"
#endif /* #if (SYS_CPNT_QOS_V2 == TRUE) */

#if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV)
#include "rule_type.h"
#endif /* #if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV) */

#if (SYS_CPNT_ADD == TRUE)
#include "add_mgr.h"
#include "add_pmgr.h"
#endif /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#include "aaa_pmgr.h"
#endif /* #if (SYS_CPNT_AAA == TRUE) */

#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_mgr.h"
#include "radius_pmgr.h"
#endif /* #if (SYS_CPNT_RADIUS == TRUE) */

#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_mgr.h"
#include "tacacs_pmgr.h"
#endif /* #if (SYS_CPNT_TACACS == TRUE) */

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_pmgr.h"
#include "webauth_type.h"
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_mgr.h"
#include "1x_pmgr.h"
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if(SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_type.h"
#include "netaccess_mgr.h"
#include "netaccess_pmgr.h"
#endif /* #if(SYS_CPNT_NETACCESS == TRUE) */

#if (SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
#include "sshd_mgr.h"
#include "sshd_pmgr.h"
#include "keygen_mgr.h"
#include "keygen_pmgr.h"
#include "keygen_type.h"
#endif /* #if (SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
#include "dai_pmgr.h"
#include "dai_type.h"
#endif  /* #if (SYS_CPNT_DAI == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_mgr.h"
#include "psec_pmgr.h"
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

#if ((SYS_CPNT_SYSLOG == TRUE) || (SYS_CPNT_REMOTELOG == TRUE))
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#endif /* #if ((SYS_CPNT_SYSLOG == TRUE) || (SYS_CPNT_REMOTELOG == TRUE)) */

#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_mgr.h"
#endif /* #if (SYS_CPNT_SMTP == TRUE) */

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pmgr.h"
#include "cluster_pom.h"
#include "cluster_type.h"
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_SNTP == TRUE)
#include "sntp_mgr.h"
#include "sntp_pmgr.h"
#endif /* #if (SYS_CPNT_SNTP == TRUE) */

#if (SYS_CPNT_NTP == TRUE)
#include "ntp_pmgr.h"
#endif /* #if (SYS_CPNT_NTP == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#include "cfm_type.h"
#endif /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
#include "oam_mgr.h"
#include "oam_om.h"
//#include "oam_type.h"
#include "leaf_4878.h"
#endif /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_POE == TRUE)
#include "poe_pmgr.h"
#endif /* #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_SNMP == TRUE)
#include "snmp_mgr.h"
#include "snmp_pmgr.h"
#include "trap_mgr.h"
#include "if_mgr.h"
#include "userauth.h"
#if (SYS_CPNT_SNMP_VERSION == 3)
#include "leaf_3415.h"
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
#endif /* #if (SYS_CPNT_SNMP == TRUE) */

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_mgr.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "lldp_type.h"
#include "leaf_ieeelldp.h"
#endif /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_TRACEROUTE == TRUE)
#include "traceroute_pmgr.h"
#include "traceroute_pom.h"
#endif /* #if (SYS_CPNT_TRACEROUTE == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif /* #if (SYS_CPNT_DHCPSNP == TRUE) */

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pmgr.h"
#endif

#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_pmgr.h"
#include "dhcp_type.h"
#endif /* #if (SYS_CPNT_DHCP == TRUE) */

#include "netcfg_pmgr_arp.h"

#if (SYS_CPNT_IPV6 == TRUE)
#include "netcfg_pmgr_nd.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "leaf_4001.h"
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_pmgr.h"
#endif  /* #if (SYS_CPNT_DHCPV6 == TRUE) */

#include "vlan_pom.h"

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
#include "ospf_pmgr.h"
#include "ospf_type.h"
#include "netcfg_pmgr_ospf.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#include "dns_type.h"
#endif /* #if (SYS_CPNT_DNS == TRUE) */

#if (SYS_CPNT_QOS_V2 == TRUE)
#include "l4_pmgr.h"
#include "cos_vm.h"
#endif  /* #if (SYS_CPNT_QOS_V2 == TRUE) */

#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#include "udphelper_type.h"
#endif /* #if (SYS_CPNT_UDP_HELPER == TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
#include "vrrp_type.h"
#include "vrrp_pmgr.h"
#endif /* #if (SYS_CPNT_VRRP == TRUE) */

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "dhcpsnp_pmgr.h"
#endif /* #if (SYS_CPNT_IP_SOURCE_GUARD == TRUE) */

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
#include "ip6sg_pmgr.h"
#include "ip6sg_pom.h"
#endif /* #if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE) */

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#include "rspan_om.h"
#endif /* #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
#include "netcfg_pmgr_ip.h"
#endif

#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#include "dos_om.h"
#endif /* #if (SYS_CPNT_DOS == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#include "pppoe_ia_type.h"
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#include "nmtr_type.h"
#endif /* #if (SYS_CPNT_NMTR_HISTORY == TRUE) */

#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pmgr.h"
#include "mldsnp_type.h"
#endif /* #if (SYS_CPNT_MLDSNP == TRUE) */

#if (SYS_CPNT_ATC_STORM == TRUE)
#include "leaf_es3626a.h"
#endif /* #if (SYS_CPNT_ATC_STORM == TRUE) */

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE)
#include "if_pmgr.h"
#endif

#ifndef ASSERT
#define ASSERT(eq)
#endif /* ASSERT */

#ifndef _countof
#define _countof(ar)    (sizeof(ar)/sizeof(*ar))
#endif /* _countof */

/* ----------------------------------------------------------------------
 * External
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */

/* For Exceptional Handler */


enum CGI_ERROR_CODE_E
{
    CGI_ERR_NOT_ENOUGH_PRIVILEGE = CGI_ERR_NOT_ENOUGH_PRIVILEGE_VALUE,
    CGI_ERR_ANALYZE,
    CGI_ERR_MST_INSTANCE_EXIST,
    CGI_ERR_MST_INSTANCE_ACTIVE,

    /* ERROR_CODE count, must be at the last one */
    CGI_ERR_MAX_ERROR_CODE,
};

typedef int (*P_SUBMITFUNC_T) (HTTP_Connection_T*, int, int, envcfg_t *, envcfg_t *);

typedef struct tagCGI_SUBMIT_TABLE_S {
    const char      *szName;
    P_SUBMITFUNC_T  pFunc;
    UI16_T          privilege_mode;
    UI16_T          cmd_idx;

} CGI_SUBMIT_TABLE_T;


/* ----------------------------------------------------------------------
 * Global constants.
 * ---------------------------------------------------------------------- */

/* back.htm */
const char *cgi_szBackHtm =
"<html><body onLoad=\"history.go(-1)\"></body></html>\n";


/* table for submit */
/* MUST BE IN ASCENDING ALPHABETICAL ORDER */

CGI_SUBMIT_TABLE_T cgi_submit_table [] =
{
    {"", NULL, NORMAL_EXEC_MODE, 0}
};

/************************************************************************/
/* Error message                                                        */
/*                                                                      */
/* Entry sequence must conform with CGI_ERROR_CODE_E enumeration which  */
/* is defined in cgi_submt.h                                            */
/************************************************************************/
static const char  *cgi_err_msg [CGI_ERR_MAX_ERROR_CODE] =
{
    [CGI_ERR_NOT_ENOUGH_PRIVILEGE] = "User privileges are not enough to perform this operation.",
    [CGI_ERR_ANALYZE]              = "Data is invalid."
};

static const char *cgi_cos_vm_err_msg[COS_TYPE_E_COUNT] =
{
    [COS_TYPE_E_UNKNOWN] = "Failed to change scheduling method and set queue mode.",
    [COS_TYPE_E_SCHEDULING_METHOD] = "Failed to change scheduling method.",
    [COS_TYPE_E_PARAMETER_STRICT_QUEUE_PRIORITY] = "Not acceptable, all strict queues priority must be higher than WRR queues.",
    [COS_TYPE_E_STRICT_QUEUE_CONFIG] = "Failed to set queue as strict priority."
};

/* ---------------------------------------------------------------- *
 * cgi_send_reloadhtm - build & send reloadhtm (PathInfo+QureryStr) *
 *                                                                  *
 * INPUT - reloadhtm: buffer to store reload HTML/JavaScript;       *
 *         initiated with the query string for reload page, if any  *
 * OUTPUT - reloadhtm: buffer stored with complete reload HTML/JS   *
 * RETURN -  1: reload page is found                                *
 *          -1: reload page is NOT found                            *
 * ---------------------------------------------------------------- */
static
int cgi_send_reloadhtm (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, const char *reloadhtm)
{
    const char *reload_fmt = "<html><body><script>location.href='%s%s'</script></body></html>\n";
    const char *err_reload_fmt = "<html><body><script>alert('%s'); location.href='%s%s'</script></body></html>\n";

    const char *uri;
    char  szQueryStr[LOCATE_MEM_SIZE] = {0}, *href;

    CGI_RESPONSE_USER_CONTEXT_PTR_T ctx;

    CGI_VERIFY_HTTP_CONNECTION(http_connection);

    if (http_connection == NULL || http_connection->res == NULL || http_connection->res->user_ctx == NULL)
    {
        return -1;
    }

    ctx = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_connection->res->user_ctx;

    uri = get_env(envcfg, "URI");

    ASSERT(uri);

    if (uri != NULL)
    {
        href = (char *)(strrchr(uri, '/') + 1);

        if (reloadhtm[0] != '\0')
        {
            //
            // FIXME: Need to detected if source string is too long
            // if too long, return error message or redirect to index page
            //
            strncpy(szQueryStr, (char *) reloadhtm, sizeof(szQueryStr)-1);
            szQueryStr[sizeof(szQueryStr)-1] = '\0';
        }
        else
            szQueryStr[0] = '\0';

        if (ctx->have_error)
        {
            // TODO: Check the modification in here !!
            sprintf((char *) reloadhtm, err_reload_fmt, ctx->error_str, href, szQueryStr);
        }
        else
        {
            sprintf((char *) reloadhtm, reload_fmt, href, szQueryStr);
        }

        cgi_SendHeader(http_connection, sock, -1, envcfg);
        cgi_SendText(http_connection, sock, reloadhtm);
        cgi_response_end(http_connection);

        return 1;
    }

    return -1;
}


/* ----------------------------------------------------------------------
 * cgi_submit_compar: Global comparison function for binary search.
 * ---------------------------------------------------------------------- */
int cgi_submit_compar(const void *key, const void *member)
{
    char               *szKey = (char *) key;
    CGI_SUBMIT_TABLE_T  *pTableMember = (CGI_SUBMIT_TABLE_T *) member;

    return strcmp (szKey, (char *) pTableMember->szName);
}

int cgi_submit_ErrorState(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, UI32_T errorCode, UI32_T *send)
{
    char   *strBuf;
    const char *begin = "<html><body><script type=\"text/javascript\" language=JavaScript>\n alert(\"",
               *end = "\")</script></body></html>\n";
    BOOL_T  isDebugErrorMessageFlag = FALSE;
    int     len;

    ASSERT(http_connection != NULL);

    {
        if (http_connection && http_connection->res && http_connection->res->user_ctx)
        {
            CGI_RESPONSE_USER_CONTEXT_PTR_T ctx = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_connection->res->user_ctx;

            ctx->have_error = 1;

            // TODO: Check the modification in here !!
            ASSERT(errorCode < _countof(cgi_err_msg));

            errorCode = (ctx->error_code < _countof(cgi_err_msg)) ? errorCode : CGI_ERR_ANALYZE;

            snprintf(ctx->error_str, CGI_MAX_ERROR_STR + 1, "%s", cgi_err_msg[errorCode]);
            ctx->error_str[CGI_MAX_ERROR_STR] = '\0';
            *send = 1;
            return 0;
        }
    }

#if (SYS_CPNT_WEB_AUTOSAVE_CONFIG==TRUE)
    web_autosave_submit_result  = FALSE;
#endif

    ASSERT(errorCode < _countof(cgi_err_msg));

    if (_countof(cgi_err_msg) <= errorCode)
    {
        errorCode = CGI_ERR_ANALYZE;
    }

    if ((strBuf = (char *)malloc (1024)) == NULL)
    {
        return 0;
    }

    strcpy (strBuf, begin);
    len = strlen (begin);

#if (SYS_CPNT_EH == TRUE)
    {
    /* for error handler */
    UI32_T module_id = 0;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  msg[256] = "";
    EH_PMGR_Get_Exception_Info(&module_id,&function_no,&msg_flag, msg, sizeof(msg));
    /* for compatabile for old codes */
    if (module_id == 0) /* EH_PMGR_Get_Exception_Info get error, so we use old error message */
    {
        strcpy (&strBuf [len], cgi_err_msg [errorCode]);
        len += strlen (cgi_err_msg[errorCode]);
    }
    else if (EH_PMGR_IS_FOR_DEBUG_MSG_PURPOSE(msg_flag))
    {
        isDebugErrorMessageFlag = TRUE;
    }
    else
    {
        strcpy (&strBuf [len], msg);
        len += strlen (msg);
    }
    }
#else
    strcpy (&strBuf [len], cgi_err_msg [errorCode]);
    len += strlen (cgi_err_msg[errorCode]);
#endif
    strcpy (&strBuf [len], end);
    len += strlen (end);
    strcpy (&strBuf [len], "");

    if (*send == 0 && isDebugErrorMessageFlag == FALSE)
    {
        cgi_SendHeader(http_connection, sock, -1, envcfg);
        cgi_SendText(http_connection, sock, strBuf);
        *send = 1;
    }

    free (strBuf);
    return 0;
}

/* ----------------------------------------------------------------------
 * cgi_submit_ErrorStateByStr : send error message to user
 * Input: sock    : Socket identifier.
 *        envcfg  : Environment configuration.
 *        errorStr: error string
 * Output: send: 0: no send screen
 *               1: had send screen
 * ---------------------------------------------------------------------- */
int cgi_submit_ErrorStateByStr(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, const char* errorStr, UI32_T *send)
{
    char   *strBuf;
    const char *begin = "<html><body><script type=\"text/javascript\" language=JavaScript>\n alert(\"",
               *end = "\")</script></body></html>\n";
    int     strBufSize = 1024, len = 0;

#if (SYS_CPNT_WEB_AUTOSAVE_CONFIG==TRUE)
    web_autosave_submit_result  = FALSE;
#endif

    // TODO: Check the modification code in here
    {
        if (http_connection && http_connection->res && http_connection->res->user_ctx)
        {
            CGI_RESPONSE_USER_CONTEXT_PTR_T ctx = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_connection->res->user_ctx;

            ctx->have_error = 1;

            snprintf(ctx->error_str, CGI_MAX_ERROR_STR + 1, "%s", errorStr);
            ctx->error_str[CGI_MAX_ERROR_STR] = '\0';
            *send = 1;
            return 0;
        }
    }

    if ((strBuf = (char *)malloc (strBufSize)) == NULL)
    {
        return 0;
    }

    memset(strBuf, 0, strBufSize);

    strncpy (strBuf, begin, strBufSize);
    len = strlen (begin);

    strncpy (&strBuf [len], errorStr, strBufSize - len);
    len += strlen (errorStr);

    strncpy (&strBuf [len], end, strBufSize - len);
    len += strlen (end);

    if (len >= strBufSize)
    {
        return 0;
    }

    strcpy (&strBuf [len], "");

    if (*send == 0)
    {
        cgi_SendHeader(http_connection, sock, -1, envcfg);
        cgi_SendText(http_connection, sock, strBuf);
        *send = 1;
    }

    free (strBuf);
    return 0;
}

static CGI_SUBMIT_TABLE_T * CGI_SUBMIT_Find(const char *page_name, CGI_SUBMIT_TABLE_T *table, size_t entry_count)
{
    return (CGI_SUBMIT_TABLE_T *) bsearch(page_name,
                                          table,
                                          entry_count,
                                          sizeof (CGI_SUBMIT_TABLE_T),
                                          &cgi_submit_compar);
}

/* ----------------------------------------------------------------------
 * cgi_submit: Handle CGI submit.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.

 * Return:
 * 1: Have sent screen.
 * 0: Have not sent screen.
 * -1: Error.
 * ---------------------------------------------------------------------- */
int cgi_submit(HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery)
{
    char                *pPageName;
    char                *pValue;
    CGI_SUBMIT_TABLE_T  *pFound;
    int                 nRet;
    int                 page_auth;

    /* get which page to submit */
    if ((pPageName = get_env (envQuery, "page")) == NULL)
    {
        /* no page to submit */
        return 0;
    }

    pFound = CGI_SUBMIT_Find(pPageName, cgi_submit_table, sizeof (cgi_submit_table) / sizeof (CGI_SUBMIT_TABLE_T));

    {
        CGI_SUBMIT_TABLE_T  *pFound_temp;
        pFound_temp = (CGI_SUBMIT_TABLE_T *) bsearch (pPageName,
                                                      & cgi_submit_table [0],
                                                      sizeof (cgi_submit_table) / sizeof (CGI_SUBMIT_TABLE_T),
                                                      sizeof (CGI_SUBMIT_TABLE_T),
                                                      &cgi_submit_compar);

        ASSERT(pFound == pFound_temp);
    }

    /* use binary search */
    if (pFound == NULL)
    {
        /* not found */
        return 0;
    }

    /* for support multi privilege, this cli function will check cmd privilege and
     * its mode, if not support, return value decided by cli
     */
    page_auth = CLI_PMGR_Get_Max_Command_Privilege(pFound->cmd_idx,  pFound->privilege_mode);

/* for user submit /webauth/ pages doesn't need auth */
// TODO: 1. Only need to get "URI" from envcfg once, and check if NULL at the beginning of this function.
//       2. Use one function to check if this request needs authorized or not,
//          for the check of webauth uri, also move to the function.
#if (SYS_CPNT_WEBAUTH == TRUE)
    if ((pValue = get_env (envcfg, "URI")) == NULL)
    {
        return -1;
    }

    if ((auth < page_auth)&&
       (strncmp(pValue, "/webauth/", 9) != 0))
#else
    if (auth < page_auth)
#endif
    {
        UI32_T  i = 0;
        char   *reloadhtm = NULL;
        char   *pPath, *pQuery;

        reloadhtm = (char *)calloc(LOCATE_MEM_SIZE, 1);

        if (reloadhtm != NULL)
        {
            memset(reloadhtm, 0, LOCATE_MEM_SIZE);

            /* send error alert */
            cgi_submit_ErrorState(http_connection, sock, envcfg, CGI_ERR_NOT_ENOUGH_PRIVILEGE, &i);

            /* send reload htm */
            if ((pValue = get_env(envcfg, "URI")) != NULL)
            {
                pPath = (char *)(strrchr(pValue, '/') + 1);

                if (((pValue = get_env(envcfg,"REFERER")) != NULL) &&
                    ((pQuery = strstr(pValue, "?")) != NULL))
                {
                    strncpy(reloadhtm, pQuery, LOCATE_MEM_SIZE - 1);
                    reloadhtm[LOCATE_MEM_SIZE - 1]='\0';
                }
                else
                {
                    reloadhtm[0] = '\0';
                }

                cgi_send_reloadhtm(http_connection, sock, envcfg, reloadhtm);
                nRet = 1;
            }

            free(reloadhtm);
        }
    }
    else
    {
        /* dispatch for submit */
        nRet = (* pFound->pFunc)(http_connection, sock, auth, envcfg, envQuery);
    }

    /* return */
    return (nRet);
}


