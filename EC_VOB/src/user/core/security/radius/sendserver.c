#include "radiusclient.h"
#include "radius_om.h"
#include "l_stdlib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "unistd.h"
#define AF_INET4 AF_INET

#include <string.h>
#include "l_md5.h"
#include "l_inet.h"
#include "sys_time.h"
#include "nmtr_pmgr.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#include "syslog_pmgr.h"
#include "mib2_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef SOCKET_ERROR
#define SOCKET_ERROR        (-1)
#endif

/* following constants defined in RFC2866 */
#define RADIUS_PACKET_CODE_ACCOUNTING_REQUEST       4
#define RADIUS_PACKET_CODE_ACCOUNTING_RESPONSE      5

#define RADIUS_SOCKET_PORT_BINDING_RETRY_TIMES      5
#define RADIUS_SOCKET_PORT_SENDING_RETRY_TIMES      20

//#define SENDSERVER_DEBUG_MODE

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef SENDSERVER_DEBUG_MODE
#ifndef _MSC_VER
#define SENDSERVER_TRACE(fmt, args...)  printf("[%s] "fmt"\r\n", __FUNCTION__, ##args)
#else
#define SENDSERVER_TRACE(fmt, ...)      printf("[%s] "fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#endif
#else
#define SENDSERVER_TRACE
#endif /* #ifdef SENDSERVER_DEBUG_MODE */

#define INC(X)  { if (X < 1812) X = 1812; (X) = (X) + 1; if (X > 2000) X = 1812; }

//static BOOL_T debug_sendserver = FALSE;

static int  msg_auth_offset ;
static UI32_T radius_local_port;
static int rc_check_reply(AUTH_HDR *auth, char *secret_r, UI8_T *vector,
    UI8_T seq_nbr, int length,
    RADIUSCLIENT_ResponsePacketErrorType_T *error_p);

#if (SYS_CPNT_IGMPAUTH == TRUE)
static int SENDSERVER_BuildIgmpAuthPacket(UI32_T request_index,
    RADIUS_OM_IGMPAuthRequestData_T *request_data_p,
    VALUE_PAIR **avps_pp);
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */
static int SENDSERVER_BuildDot1xAuthPacket(UI32_T request_index,
    RADIUS_OM_Dot1xAuthRequestData_T *request_data_p,
    VALUE_PAIR **avps_pp);
static int SENDSERVER_BuildRadaAuthPacket(UI32_T request_index,
    RADIUS_OM_RadaAuthRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp);
static int SENDSERVER_BuildUserAuthPacket(UI32_T request_index,
    RADIUS_OM_UserAuthRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp);
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static int SENDSERVER_BuildAccountingPacket(UI32_T request_index,
    RADIUS_OM_AccountingRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp);
static int SENDSERVER_BuildAcctRequestPacketHeader(const UI32_T identifier,
    char *secret_key_p, VALUE_PAIR *avps_p, UI8_T *packet_p,
    UI32_T *packet_len_p, UI8_T *packet_vector_p);
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
static int SENDSERVER_BuildAuthRequestPacketHeader(
    const RADIUS_OM_RadiusRequestType_T type, const UI32_T identifier,
    char *secret_key_p, VALUE_PAIR *avps_p, UI8_T *packet_p,
    UI32_T *packet_len_p, UI8_T *packet_vector_p);
static int SENDSERVER_BuildRequestPacket(const UI32_T request_index,
    RADIUS_Server_Host_T *host_p, UI8_T *packet_p, UI32_T *packet_len_p,
    UI8_T *packet_vector_p);
static int SENDSERVER_SendPacket(const int socket_id,
    const UI32_T server_ip, const UI32_T server_port, const UI8_T *packet_p,
    const UI32_T packet_len);
static BOOL_T SENDSERVER_GetPrivilegeFromCiscoAVPair(VALUE_PAIR *pair,
    I32_T *privilege);
static int SENDSERVER_ExtractDot1xAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p);
static int SENDSERVER_ExtractRadaAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p);
static int SENDSERVER_ExtractUserAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p);
static int SENDSERVER_ExtractAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p);

/*
 * Function: rc_pack_list
 *
 * Purpose: Packs an attribute value pair list into a buffer.
 *
 * Returns: Number of octets packed.
 *
 */

 int rc_pack_list(VALUE_PAIR *vp, char *secret_s, AUTH_HDR *auth, UI32_T *msg_auth_offset_p)
{
    int             length, i, pc, secretlen, padded_length;
    int             total_length = 0;
    UI32_T           lvalue;
    UI8_T   passbuf[MAX(AUTH_PASS_LEN, CHAP_VALUE_LENGTH)];
    UI8_T   md5buf[256];
    UI8_T   *buf, *vector;

    buf = auth->data;

    while (vp != (VALUE_PAIR *) NULL)
    {
        *buf++ = vp->attribute;

        switch (vp->attribute)
        {
        case PW_USER_PASSWORD:

            /* Encrypt the password */

            /* Chop off password at AUTH_PASS_LEN */
            length = vp->lvalue;
            if (length > AUTH_PASS_LEN)
                length = AUTH_PASS_LEN;

            /* Calculate the padded length */
            padded_length = (length+(AUTH_VECTOR_LEN-1)) & ~(AUTH_VECTOR_LEN-1);

            /* Record the attribute length */
            *buf++ = padded_length + 2;

            /* Pad the password with zeros */
            memset ((char *) passbuf, '\0', AUTH_PASS_LEN);
            memcpy ((char *) passbuf, vp->strvalue, length);

            secretlen = strlen (secret_s);
            vector = auth->vector;

            for(i = 0; i < padded_length; i += AUTH_VECTOR_LEN)
            {
                /* Calculate the MD5 digest*/

                strncpy ((char *) md5buf, secret_s, sizeof(md5buf));
                memcpy ((char *) md5buf + secretlen, vector, AUTH_VECTOR_LEN);
                L_MD5_MDString(buf, md5buf, secretlen + AUTH_VECTOR_LEN);

                /* Remeber the start of the digest */
                vector = buf;

                /* Xor the password into the MD5 digest */
                for (pc = i; pc < (i + AUTH_VECTOR_LEN); pc++)
                {
                    *buf++ ^= passbuf[pc];
                }
            }

            total_length += padded_length + 2;
            break;

        default:

            if (   (PW_MESSAGE_AUTHENTICATOR == vp->attribute)
                && (NULL != msg_auth_offset_p)
                )
            {
                *msg_auth_offset_p = total_length;
            }

            switch (vp->type)
            {
            case PW_TYPE_STRING:
            case PW_TYPE_IP6ADDR:
                length = vp->lvalue;
                *buf++ = length + 2;
                memcpy (buf, vp->strvalue, length);
                buf += length;
                total_length += length + 2;
                break;

            case PW_TYPE_INTEGER:
                *buf++ = sizeof (UI32_T) + 2;
                lvalue = L_STDLIB_Hton32(vp->lvalue);
                memcpy (buf, (char *) &lvalue, sizeof (UI32_T));
                buf += sizeof (UI32_T);
                total_length += sizeof (UI32_T) + 2;
                break;

            case PW_TYPE_IPADDR:
                *buf++ = sizeof (UI32_T) + 2;
                /* ipaddress already is network order */
                lvalue = vp->lvalue;
                memcpy (buf, (char *) &lvalue, sizeof (UI32_T));
                buf += sizeof (UI32_T);
                total_length += sizeof (UI32_T) + 2;
                break;

            default:
                break;
            }/* switch (vp->type) */

            break;
        } /* switch (vp->attribute) */

        vp = vp->next;
    } /* while (vp != (VALUE_PAIR *) NULL) */

    return total_length;
}

/*
 * Function: rc_check_reply
 *
 * Purpose: verify items in returned packet.
 *
 * Returns: OK_RC       -- upon success,
 *      BADRESP_RC  -- if anything looks funny.
 *
 */

static int rc_check_reply(AUTH_HDR *auth, char *secret_r, UI8_T *vector,
    UI8_T seq_nbr, int length,
    RADIUSCLIENT_ResponsePacketErrorType_T *error_p)
{
    int             secretlen;
    int             totallen;
    UI8_T   calc_digest[AUTH_VECTOR_LEN];
    UI8_T   reply_digest[AUTH_VECTOR_LEN];

    if (NULL == error_p)
    {
        *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_NONE;
        return BADRESP_RC;
    }

    /* ES3550MO-PoE-FLF-AA-00053
     * use CmnLib APIs to replace socket ntohs(), htons()... function
     */
    totallen = L_STDLIB_Ntoh16(auth->length);

    /* Verify that id (seq. number) matches what we sent */
    if (auth->id != seq_nbr)
    {
        SENDSERVER_TRACE("auth->id=%u, seq_nbr=%u", auth->id, seq_nbr);
        *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_SEQ_NBR;
        return (BADRESP_RC);
    }

    /* Verify the length.
    */
    if (totallen != length)
    {
        *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_LENGTH;
        return (BADRESP_RC);
    }

    /* Verify the reply digest */
    memcpy((char *)reply_digest, (char *)auth->vector, AUTH_VECTOR_LEN);
    memcpy((char *)auth->vector, (char *)vector, AUTH_VECTOR_LEN);
    secretlen = strlen(secret_r);
    memcpy((char *)auth + totallen, secret_r, secretlen);
    L_MD5_MDString(calc_digest, (UI8_T*)auth, totallen + secretlen);

    if (memcmp((char *)reply_digest, (char *)calc_digest, AUTH_VECTOR_LEN) != 0)
    {
#ifdef RADIUS_116
        /* the original Livingston radiusd v1.16 seems to have
           a bug in digest calculation with accounting requests,
           authentication request are ok. i looked at the code
           but couldn't find any bugs. any help to get this
           kludge out are welcome. preferably i want to
           reproduce the calculation bug here to be compatible
           to stock Livingston radiusd v1.16.   -lf, 03/14/96
           */
        if (auth->code == PW_ACCOUNTING_RESPONSE)
            return (OK_RC);
#endif
        SENDSERVER_TRACE("bad reply digest");
        *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_BAD_AUTHENTICATOR;
        return (BADRESP_RC);
    }

    return (OK_RC);
}

#if (SYS_CPNT_IGMPAUTH == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_BuildIgmpAuthPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Generate a/v paisr for IGMP authentication request.
 * INPUT    : request_index     - Request index
 *            request_data_p    - Request data.
 * OUTPUT   : avps_pp           - Generated a/v pairs.
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildIgmpAuthPacket(UI32_T request_index,
    RADIUS_OM_IGMPAuthRequestData_T *request_data_p,
    VALUE_PAIR **avps_pp)
{
    UI32_T server_index;
    L_INET_AddrIp_T ip_addr;
    L_INET_AddrIp_T inet_server_ip;
    RADIUS_Server_Host_T server_host;
    char mac_string[20];

    if (   (NULL == request_data_p)
        || (NULL == avps_pp)
        )
    {
        return ERROR_RC;
    }

    sprintf(mac_string,
        "%02X-%02X-%02X-%02X-%02X-%02X",
        request_data_p->auth_mac[0],
        request_data_p->auth_mac[1],
        request_data_p->auth_mac[2],
        request_data_p->auth_mac[3],
        request_data_p->auth_mac[4],
        request_data_p->auth_mac[5]);

    if (NULL == rc_avpair_add(avps_pp, PW_USER_NAME, mac_string, 0))
    {
        goto undo;
    }

    if (NULL == rc_avpair_add(avps_pp, PW_USER_PASSWORD, mac_string, 0))
    {
        goto undo;
    }

    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT,
        &request_data_p->auth_port, 0))
    {
        goto undo;
    }

    if (NULL == rc_avpair_add(avps_pp, PW_FRAMED_IP_ADDRESS,
        &request_data_p->ip_address, 0))
    {
        goto undo;
    }

    memset(&inet_server_ip, 0, sizeof(inet_server_ip));
    inet_server_ip.type = L_INET_ADDR_TYPE_IPV4;
    inet_server_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestServerIndex(request_index,
            &server_index))
        || (FALSE == RADIUS_OM_Get_Server_Host(server_index, &server_host))
        )
    {
        goto undo;
    }
    memcpy(inet_server_ip.addr, &server_host.server_ip, SYS_ADPT_IPV4_ADDR_LEN);
    ip_addr.type = L_INET_ADDR_TYPE_UNKNOWN;
    get_local_ip(&inet_server_ip, &ip_addr);

    if (ip_addr.type == L_INET_ADDR_TYPE_IPV4 ||
        ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IP_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else if (ip_addr.type == L_INET_ADDR_TYPE_IPV6 ||
        ip_addr.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IPV6_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else
    {
        goto undo;
    }

    return OK_RC;

undo:
    rc_avpair_free(*avps_pp);
    *avps_pp = NULL;

    return ERROR_RC;
}
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_BuildDot1xAuthPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Generate a/v paisr for 802.1X authentication request.
 * INPUT    : request_index     - Request index
 *            request_data_p    - Request data.
 * OUTPUT   : avps_pp           - Generated a/v pairs.
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildDot1xAuthPacket(UI32_T request_index,
    RADIUS_OM_Dot1xAuthRequestData_T *request_data_p,
    VALUE_PAIR **avps_pp)
{
    UI8_T port_str[4];
    UI8_T mac_str[20];
    UI8_T sys_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    UI32_T nas_port_type;
    UI8_T msg_auth[AUTH_VECTOR_LEN];
    L_INET_AddrIp_T ip_addr, inet_server_ip;

    if (   (NULL == request_data_p)
        || (NULL == avps_pp)
        )
    {
        return ERROR_RC;
    }

    /* Fill in User-Name
     */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_NAME,
        request_data_p->username, 0))
    {
        goto undo;
    }

    /* Fill in NAS-Port
     */
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT,
        &request_data_p->src_port, 0))
    {
        goto undo;
    }

    /* Fill in NAS-Port-Id
     */
    sprintf((char *)port_str, "%lu", request_data_p->src_port);
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT_ID_STRING, port_str, 0))
    {
        goto undo;
    }

    /* Fill in Calling-Station-Id
     */
    sprintf((char *)mac_str, "%02X-%02X-%02X-%02X-%02X-%02X",
        request_data_p->src_mac[0], request_data_p->src_mac[1],
        request_data_p->src_mac[2], request_data_p->src_mac[3],
        request_data_p->src_mac[4], request_data_p->src_mac[5]);
    if (NULL == rc_avpair_add(avps_pp, PW_CALLING_STATION_ID, mac_str, 0))
    {
        goto undo;
    }

    /* Fill in EAP-Message
     */
    if (NULL == rc_avpair_add(avps_pp, PW_EAP_MESSAGE,
        request_data_p->eap_data, request_data_p->eap_data_len))
    {
        goto undo;
    }

    /* Fill in State
     */
    if (NULL == rc_avpair_add(avps_pp, PW_STATE,
        request_data_p->state_data, request_data_p->state_data_len))
    {
        goto undo;
    }

    /* Fill in NAS-IP-Address or NAS-IPv6-Address
     */
    memset(&inet_server_ip, 0, sizeof(inet_server_ip));
    inet_server_ip.type = L_INET_ADDR_TYPE_IPV4;
    inet_server_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    ip_addr.type = L_INET_ADDR_TYPE_UNKNOWN;
    get_local_ip(&inet_server_ip, &ip_addr);
    if (   (ip_addr.type == L_INET_ADDR_TYPE_IPV4)
        || (ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
        )
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IP_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IPV6_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Identifier
     */
    memset(sys_name, '\0', SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN);
    if (MIB2_POM_GetSysName(sys_name) == TRUE)
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IDENTIFIER, sys_name, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Port-Type
     */
    nas_port_type = PW_ETHERNET;
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT_TYPE, &nas_port_type, 0))
    {
        goto undo;
    }

    /* Fill in Message-Authenticator
     */
    memset(msg_auth, 0, sizeof(msg_auth));
    if (NULL == rc_avpair_add(avps_pp, PW_MESSAGE_AUTHENTICATOR, msg_auth, sizeof(msg_auth)))
    {
        goto undo;
    }

    return OK_RC;

undo:
    rc_avpair_free(*avps_pp);
    *avps_pp = NULL;

    return ERROR_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_BuildRadaAuthPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Generate a/v paisr for RADA authentication request.
 * INPUT    : request_index     - Request index
 *            request_data_p    - Request data.
 * OUTPUT   : avps_pp           - Generated a/v pairs.
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildRadaAuthPacket(UI32_T request_index,
    RADIUS_OM_RadaAuthRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp)
{
    UI8_T port_str[4];
    UI8_T mac_str[20];
    UI8_T sys_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    UI32_T nas_port_type;
    L_INET_AddrIp_T ip_addr, inet_server_ip;

    if (   (NULL == request_data_p)
        || (NULL == avps_pp)
        )
    {
        return ERROR_RC;
    }

    /* Fill in User-Name
    */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_NAME,
        request_data_p->username, 0))
    {
        goto undo;
    }

    /* Fill in User-Password
    */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_PASSWORD,
        request_data_p->password, 0))
    {
        goto undo;
    }

    /* Fill in NAS-Port
    */
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT,
        &request_data_p->src_port, 0))
    {
        goto undo;
    }

    /* Fill in NAS-Port-Id
    */
    sprintf((char *)port_str, "%lu", request_data_p->src_port);
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT_ID_STRING, port_str, 0))
    {
        goto undo;
    }

    /* Fill in Calling-Station-Id
    */
    sprintf((char *)mac_str, "%02X-%02X-%02X-%02X-%02X-%02X",
        request_data_p->src_mac[0], request_data_p->src_mac[1], request_data_p->src_mac[2],
        request_data_p->src_mac[3], request_data_p->src_mac[4], request_data_p->src_mac[5]);
    if (NULL == rc_avpair_add(avps_pp, PW_CALLING_STATION_ID, mac_str, 0))
    {
        goto undo;
    }

    /* Fill in NAS-IP-Address or NAS-IPv6-Address
    */
    memset(&inet_server_ip, 0, sizeof(inet_server_ip));
    inet_server_ip.type = L_INET_ADDR_TYPE_IPV4;
    inet_server_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    ip_addr.type = L_INET_ADDR_TYPE_UNKNOWN;
    get_local_ip(&inet_server_ip, &ip_addr);
    if ((ip_addr.type == L_INET_ADDR_TYPE_IPV4)
        || (ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
        )
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IP_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IPV6_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Identifier
    */
    memset(sys_name, '\0', SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN);
    if (MIB2_POM_GetSysName(sys_name) == TRUE)
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IDENTIFIER, sys_name, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Port-Type
    */
    nas_port_type = PW_ETHERNET;
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT_TYPE, &nas_port_type, 0))
    {
        goto undo;
    }

    return OK_RC;

undo:
    rc_avpair_free(*avps_pp);
    *avps_pp = NULL;

    return ERROR_RC;
}

/*---------------------------------------------------------------------------
* ROUTINE NAME - SENDSERVER_BuildUserAuthPacket
*---------------------------------------------------------------------------
* PURPOSE  : Generate a/v paisr for user authentication request.
* INPUT    : request_index     - Request index
*            request_data_p    - Request data.
* OUTPUT   : avps_pp           - Generated a/v pairs.
* RETURN   : OK_RC/ERROR_RC
* NOTES    : None
*---------------------------------------------------------------------------
*/
static int SENDSERVER_BuildUserAuthPacket(UI32_T request_index,
    RADIUS_OM_UserAuthRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp)
{
    UI8_T sys_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    L_INET_AddrIp_T ip_addr, inet_server_ip;

    if (   (NULL == request_data_p)
        || (NULL == avps_pp)
        )
    {
        return ERROR_RC;
    }

    /* Fill in User-Name
    */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_NAME,
        request_data_p->username, 0))
    {
        goto undo;
    }

    /* Fill in User-Password
    */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_PASSWORD,
        request_data_p->password, 0))
    {
        goto undo;
    }

    /* Fill in NAS-IP-Address or NAS-IPv6-Address
    */
    memset(&inet_server_ip, 0, sizeof(inet_server_ip));
    inet_server_ip.type = L_INET_ADDR_TYPE_IPV4;
    inet_server_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    ip_addr.type = L_INET_ADDR_TYPE_UNKNOWN;
    get_local_ip(&inet_server_ip, &ip_addr);
    if (   (ip_addr.type == L_INET_ADDR_TYPE_IPV4)
        || (ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
        )
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IP_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IPV6_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Identifier
    */
    memset(sys_name, '\0', SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN);
    if (MIB2_POM_GetSysName(sys_name) == TRUE)
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IDENTIFIER, sys_name, 0))
        {
            goto undo;
        }
    }

    return OK_RC;

undo:
    rc_avpair_free(*avps_pp);
    *avps_pp = NULL;

    return ERROR_RC;
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_BuildAccountingPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Generate a/v paisr for accounting request.
 * INPUT    : request_index     - Request index
 *            request_data_p    - Request data.
 * OUTPUT   : avps_pp           - Generated a/v pairs.
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildAccountingPacket(UI32_T request_index,
    RADIUS_OM_AccountingRequestData_T *request_data_p,
    VALUE_PAIR ** avps_pp)
{
    UI8_T port_str[4];
    UI8_T mac_str[20];
    UI8_T sys_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    UI8_T byte_buffer[32];
    UI32_T server_index;
    UI32_T four_bytes_value;
    L_INET_AddrIp_T ip_addr, inet_server_ip;
    RADACC_UserInfo_T user_info;
    SWDRV_IfTableStats_T if_table_stats;
    RADIUS_Server_Host_T server_host;

    if (   (NULL == request_data_p)
        || (NULL == avps_pp)
        )
    {
        return ERROR_RC;
    }

    user_info.user_index = request_data_p->user_index;
    if (FALSE == RADIUS_OM_GetAccUserEntry(&user_info))
    {
        return ERROR_RC;
    }

    /* Fill in User-Name
     */
    if (NULL == rc_avpair_add(avps_pp, PW_USER_NAME, user_info.user_name,
        strlen((char *)user_info.user_name)))
    {
        goto undo;
    }

    /* Fill in NAS-IP-Address or NAS-IPv6-Address
     */
    memset(&inet_server_ip, 0, sizeof(inet_server_ip));
    inet_server_ip.type = L_INET_ADDR_TYPE_IPV4;
    inet_server_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    ip_addr.type = L_INET_ADDR_TYPE_UNKNOWN;
    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestServerIndex(request_index,
            &server_index))
        || (FALSE == RADIUS_OM_Get_Server_Host(server_index, &server_host))
        )
    {
        goto undo;
    }
    memcpy(inet_server_ip.addr, &server_host.server_ip, SYS_ADPT_IPV4_ADDR_LEN);
    get_local_ip(&inet_server_ip, &ip_addr);
    if (   (ip_addr.type == L_INET_ADDR_TYPE_IPV4)
        || (ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
        )
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IP_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }
    else
    {
        if (NULL == rc_avpair_add(avps_pp, PW_NAS_IPV6_ADDRESS, ip_addr.addr, 0))
        {
            goto undo;
        }
    }

    /* Fill in NAS-Port-Type
     */
    four_bytes_value = PW_ETHERNET;
    if (NULL == rc_avpair_add(avps_pp, PW_NAS_PORT_TYPE, &four_bytes_value, 0))
    {
        goto undo;
    }

    /* Fill in Acct-Session-Id
     */
    if (FALSE == RADIUS_OM_GetAccUserEntrySessionId(request_data_p->user_index,
        byte_buffer, sizeof(byte_buffer)))
    {
        goto undo;
    }
    if (NULL == rc_avpair_add(avps_pp, PW_ACCT_SESSION_ID, byte_buffer,
        strlen((char *)byte_buffer)))
    {
        goto undo;
    }

    /* Fill in Acct-Status-Type
     */
    switch (request_data_p->request_type)
    {
    case RADACC_START:
        four_bytes_value = PW_STATUS_START;
        break;

    case RADACC_STOP:
        four_bytes_value = PW_STATUS_STOP;
        break;

    case RADACC_InterimUpdate:
        four_bytes_value = PW_STATUS_ALIVE;
        break;

    default:
        goto undo;
    }
    if (NULL == rc_avpair_add(avps_pp, PW_ACCT_STATUS_TYPE, &four_bytes_value, 0))
    {
        goto undo;
    }

    /* Fill in Acct-Authentic
     */
    switch (user_info.auth_by_whom)
    {
    case AAA_ACC_AUTH_BY_RADIUS:
        four_bytes_value = PW_RADIUS;
        break;

    case AAA_ACC_AUTH_BY_LOCAL:
        four_bytes_value = PW_LOCAL;
        break;

    case AAA_ACC_AUTH_BY_TACACS_PLUS:
    case AAA_ACC_AUTH_BY_UNKNOWN:
    default:
        four_bytes_value = PW_REMOTE;
        break;
    }
    if (NULL == rc_avpair_add(avps_pp, PW_ACCT_AUTHENTIC, &four_bytes_value, 0))
    {
        goto undo;
    }

    /* Fill in Acct-Delay-Time
     */
    SYS_TIME_GetRealTimeBySec(&four_bytes_value);
    four_bytes_value = four_bytes_value - request_data_p->created_time;
    if (NULL == rc_avpair_add(avps_pp, PW_ACCT_DELAY_TIME, &four_bytes_value, 0))
    {
        goto undo;
    }

    /* Fill in Calling-Station-Id
     */
    sprintf((char *)mac_str, "%02X-%02X-%02X-%02X-%02X-%02X",
        user_info.auth_mac[0], user_info.auth_mac[1], user_info.auth_mac[2],
        user_info.auth_mac[3], user_info.auth_mac[4], user_info.auth_mac[5]);
    if (NULL == rc_avpair_add(avps_pp, PW_CALLING_STATION_ID, mac_str, 0))
    {
        goto undo;
    }

    if (RADACC_STOP == request_data_p->request_type)
    {
        /* Only DOT1X need to send statistic_info.
         */
        if (   (AAA_CLIENT_TYPE_DOT1X == user_info.client_type)
            && (TRUE == NMTR_PMGR_GetIfTableStats(user_info.ifindex,
                &if_table_stats))
            )
        {
            /* Fill in Acct-Input-Octets
             */
            four_bytes_value = if_table_stats.ifInOctets -
                user_info.statistic_info.ifInOctets;
            if (NULL == rc_avpair_add(avps_pp, PW_ACCT_INPUT_OCTETS,
                &four_bytes_value, 0))
            {
                goto undo;
            }

            /* Fill in Acct-Output-Octets
             */
            four_bytes_value = if_table_stats.ifOutOctets -
                user_info.statistic_info.ifOutOctets;
            if (NULL == rc_avpair_add(avps_pp, PW_ACCT_OUTPUT_OCTETS,
                &four_bytes_value, 0))
            {
                goto undo;
            }

            /* Fill in Acct-Input-Packets
             */
            four_bytes_value = if_table_stats.ifInUcastPkts -
                user_info.statistic_info.ifInUcastPkts;
            if (NULL == rc_avpair_add(avps_pp, PW_ACCT_INPUT_PACKETS,
                &four_bytes_value, 0))
            {
                goto undo;
            }

            /* Fill in Acct-Output-Packets
             */
            four_bytes_value = if_table_stats.ifOutUcastPkts -
                user_info.statistic_info.ifOutUcastPkts;
            if (NULL == rc_avpair_add(avps_pp, PW_ACCT_OUTPUT_PACKETS,
                &four_bytes_value, 0))
            {
                goto undo;
            }

        }

        /* Fill in Acct-Session-Time
        */
        SENDSERVER_TRACE("Acct-Session-Time: create_time(%lu), start-time(%lu)",
            request_data_p->created_time, user_info.session_start_time);

        four_bytes_value = request_data_p->created_time -
            user_info.session_start_time;
        if (NULL == rc_avpair_add(avps_pp, PW_ACCT_SESSION_TIME, &four_bytes_value, 0))
        {
            goto undo;
        }

        /* Fill in Acct-Terminate-Cause
         */
        four_bytes_value = (AAA_ACC_TERM_BY_UNKNOWN == user_info.terminate_cause) ?
            AAA_ACC_TERM_BY_NAS_REQUEST : user_info.terminate_cause;
        if (NULL == rc_avpair_add(avps_pp, PW_ACCT_TERMINATE_CAUSE, &four_bytes_value, 0))
        {
            goto undo;
        }
    }

    return OK_RC;

undo:
    rc_avpair_free(*avps_pp);
    *avps_pp = NULL;

    return ERROR_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - SENDSERVER_BuildAcctRequestPacketHeader
 *---------------------------------------------------------------------------
 * PURPOSE  : Build the header of the accounting request packet.
 * INPUT    : identifier        - Identifier in the packet
 *            secret_key_p      - Secret key of RADIUS server
 *            avps_p            - A/v pairs of the packet
 * OUTPUT   : packet_p          - The built packet
 *            packet_len_p      - Length of the built packet
 *            packet_vector_p   - The vector in the packet
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildAcctRequestPacketHeader(const UI32_T identifier,
    char *secret_key_p, VALUE_PAIR *avps_p, UI8_T *packet_p,
    UI32_T *packet_len_p, UI8_T *packet_vector_p)
{
    AUTH_HDR *header_p;
    UI32_T msg_auth_av_pair_offset;
    UI32_T packed_av_pair_len;
    UI32_T secret_key_len;

    if (   (NULL == secret_key_p)
        || (NULL == avps_p)
        || (NULL == packet_p)
        || (NULL == packet_len_p)
        || (NULL == packet_vector_p)
        )
    {
        return ERROR_RC;
    }

    header_p = (AUTH_HDR *)packet_p;
    header_p->code = PW_ACCOUNTING_REQUEST;
    header_p->id = identifier;

    memset(header_p->vector, 0, sizeof(header_p->vector));

    packed_av_pair_len = rc_pack_list(avps_p, secret_key_p, header_p, NULL);

    *packet_len_p = AUTH_HDR_LEN + packed_av_pair_len;
    header_p->length = L_STDLIB_Hton16(*packet_len_p);

    secret_key_len = strlen(secret_key_p);
    memcpy(packet_p + *packet_len_p, secret_key_p, secret_key_len);
    L_MD5_MDString(packet_vector_p, packet_p, *packet_len_p + secret_key_len);
    memcpy(header_p->vector, packet_vector_p, sizeof(header_p->vector));

    return OK_RC;
}
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - SENDSERVER_BuildAuthRequestPacketHeader
 *---------------------------------------------------------------------------
 * PURPOSE  : Build the header of the authentication request packet.
 * INPUT    : type              - Request type
 *            identifier        - Identifier in the packet
 *            secret_key_p      - Secret key of RADIUS server
 *            avps_p            - A/v pairs of the packet
 * OUTPUT   : packet_p          - The built packet
 *            packet_len_p      - Length of the built packet
 *            packet_vector_p   - The vector in the packet
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildAuthRequestPacketHeader(
    const RADIUS_OM_RadiusRequestType_T type, const UI32_T identifier,
    char *secret_key_p, VALUE_PAIR *avps_p, UI8_T *packet_p,
    UI32_T *packet_len_p, UI8_T *packet_vector_p)
{
    AUTH_HDR *header_p;
    UI32_T msg_auth_av_pair_offset;
    UI32_T packed_av_pair_len;

    if (   (NULL == secret_key_p)
        || (NULL == avps_p)
        || (NULL == packet_p)
        || (NULL == packet_len_p)
        || (NULL == packet_vector_p)
        )
    {
        return ERROR_RC;
    }

    header_p = (AUTH_HDR *)packet_p;
    header_p->code = PW_ACCESS_REQUEST;
    header_p->id = identifier;

    rc_random_vector(header_p->vector);
    memcpy(packet_vector_p, header_p->vector, AUTH_VECTOR_LEN);

    packed_av_pair_len = rc_pack_list(avps_p, secret_key_p, header_p,
        &msg_auth_av_pair_offset);

    *packet_len_p = AUTH_HDR_LEN + packed_av_pair_len;
    header_p->length = L_STDLIB_Hton16(*packet_len_p);

    if (RADIUS_REQUEST_TYPE_DOT1X_AUTH == type)
    {
        UI8_T vector[AUTH_VECTOR_LEN];

        memset(header_p->data + msg_auth_av_pair_offset + 2, 0,
            AUTH_VECTOR_LEN);
        L_MD5_HMAC_MD5((UI8_T *)header_p, *packet_len_p, (UI8_T *)secret_key_p,
            strlen(secret_key_p), vector);
        memcpy(header_p->data + msg_auth_av_pair_offset + 2, vector,
            sizeof(vector));
    }

    return OK_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - SENDSERVER_BuildRequestPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Build the request packet according to the type.
 * INPUT    : request_index     - The request index
 *            host_p            - The RADIUS server
 * OUTPUT   : packet_p          - The built packet
 *            packet_len_p      - Length of the built packet
 *            packet_vector_p   - The vector in the packet
 * RETURN   : OK_RC/ERROR_RC
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_BuildRequestPacket(const UI32_T request_index,
    RADIUS_Server_Host_T *host_p, UI8_T *packet_p, UI32_T *packet_len_p,
    UI8_T *packet_vector_p)
{
    RADIUS_OM_RadiusRequestType_T type;
    RADIUS_OM_RequestEntry_T request_data;
    UI32_T identifier;
    VALUE_PAIR *avps_p = NULL;
    int ret = ERROR_RC;

    if (   (NULL == host_p)
        || (NULL == packet_vector_p)
        || (NULL == packet_p)
        || (NULL == packet_len_p)
        )
    {
        return ERROR_RC;
    }

    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index, &type))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestData(request_index, &request_data))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestIdentifier(request_index, &identifier))
        )
    {
        return ERROR_RC;
    }

    switch (type)
    {
#if (SYS_CPNT_IGMPAUTH == TRUE)
    case RADIUS_REQUEST_TYPE_IGMPAUTH:
        if (OK_RC != SENDSERVER_BuildIgmpAuthPacket(request_index,
            &request_data.igmp_auth_data, &avps_p))
        {
            return ERROR_RC;
        }
        break;
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

    case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
        if (OK_RC != SENDSERVER_BuildDot1xAuthPacket(request_index,
            &request_data.dot1x_auth_data, &avps_p))
        {
            return ERROR_RC;
        }
        break;

    case RADIUS_REQUEST_TYPE_RADA_AUTH:
        if (OK_RC != SENDSERVER_BuildRadaAuthPacket(request_index,
            &request_data.rada_auth_data, &avps_p))
        {
            return ERROR_RC;
        }
        break;

    case RADIUS_REQUEST_TYPE_WEB_AUTH:
    case RADIUS_REQUEST_TYPE_USER_AUTH:
        if (OK_RC != SENDSERVER_BuildUserAuthPacket(request_index,
            &request_data.user_auth_data, &avps_p))
        {
            return ERROR_RC;
        }
        break;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    case RADIUS_REQUEST_TYPE_ACCOUNTING:
        if (OK_RC != SENDSERVER_BuildAccountingPacket(request_index,
            &request_data.accounting_data, &avps_p))
        {
            return ERROR_RC;
        }
        break;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    default:
        return ERROR_RC;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        ret = SENDSERVER_BuildAuthRequestPacketHeader(type, identifier,
            (char *)host_p->secret, avps_p, packet_p, packet_len_p,
            packet_vector_p);
    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    else
    {
        ret = SENDSERVER_BuildAcctRequestPacketHeader(identifier,
            (char *)host_p->secret, avps_p, packet_p, packet_len_p,
            packet_vector_p);
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    rc_avpair_free(avps_p);

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_SendPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  Send the specified packet.
 * INPUT:    socket_id  - Socket ID
 *           host_p     - RADIUS server host
 *           packet_p   - Packet data
 *           packet_len - Length of packet data
 * OUTPUT:   None
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     Note
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_SendPacket(const int socket_id,
    const UI32_T server_ip, const UI32_T server_port, const UI8_T *packet_p,
    const UI32_T packet_len)
{
    int ret;
    struct sockaddr_in *sin_p;
    struct sockaddr saremote;

    if (   (0 > socket_id)
        || (NULL == packet_p)
        || (0 == packet_len)
        )
    {
        return ERROR_RC;
    }

    sin_p = (struct sockaddr_in *)&saremote;
    memset(&saremote, '\0', sizeof(saremote));
    sin_p->sin_family = AF_INET;
    sin_p->sin_addr.s_addr = server_ip;
    sin_p->sin_port = L_STDLIB_Hton16(server_port);

    ret = sendto(socket_id,
        (char *)packet_p, packet_len, 0,
        (struct sockaddr *)sin_p, sizeof(*sin_p));

    if (ret < 0)
    {
        SENDSERVER_TRACE("Failed to send packet (ret = %d)", ret);

        return ERROR_RC;
    }

    SENDSERVER_TRACE("Send packet %d bytes", ret);

    return OK_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_SendRequestPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  Send the request packet.
 * INPUT:    request_index  - Request index
 *           socket_id      - Socket ID
 * OUTPUT:   vector_p       - Vector data in sent packet
 *           secret_key_p   - Secret key of the remote RADIUS server
 *           timeout_p      - Timeout of the remote RADIUS server
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
int SENDSERVER_SendRequestPacket(const UI32_T request_index,
    const int socket_id, UI8_T *vector_p, char *secret_key_p,
    UI32_T *timeout_p)
{
    UI8_T packet[BUFFER_LEN];
    UI32_T packet_len = 0;
    UI32_T server_index;
    UI32_T server_ip;
    UI32_T server_port = 0;
    RADIUS_Server_Host_T host;
    RADIUS_OM_RadiusRequestType_T type;

    if (   (0 > socket_id)
        || (NULL == vector_p)
        || (NULL == secret_key_p)
        || (NULL == timeout_p)
        )
    {
        return ERROR_RC;
    }

    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestServerIndex(
            request_index, &server_index))
        || (FALSE == RADIUS_OM_Get_Server_Host(server_index, &host))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index,
            &type))
        )
    {
        return ERROR_RC;
    }

    if (OK_RC != SENDSERVER_BuildRequestPacket(request_index, &host, packet,
        &packet_len, vector_p))
    {
        return ERROR_RC;
    }

    server_ip = host.server_ip;
    if (RADIUS_REQUEST_TYPE_ACCOUNTING != type)
    {
        server_port = host.server_port;
    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    else
    {
        server_port = host.acct_port;
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    SENDSERVER_TRACE("Send packet to server index %lu, IP = 0x%lX, port = %lu",
        server_index, server_ip, server_port);

    if (OK_RC != SENDSERVER_SendPacket(socket_id, server_ip, server_port, packet,
        packet_len))
    {
        return ERROR_RC;
    }

    strncpy((char *)secret_key_p, (char *)host.secret, sizeof(host.secret) - 1);
    secret_key_p[sizeof(host.secret) - 1] = '\0';

    *timeout_p = host.timeout;

    return OK_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ProcessReceivedPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  Process the received packet.
 * INPUT:    request_index  - Request index
 *           packet_p       - Packet
 *           packet_len     - Packet length
 * OUTPUT:   code_p         - Code in the packet
 *           error_p        - Error type
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
int SENDSERVER_ProcessReceivedPacket(const UI32_T request_index,
    UI8_T *packet_p, int packet_len, UI32_T *code_p,
    RADIUSCLIENT_ResponsePacketErrorType_T *error_p)
{
    RADIUS_OM_RequestResult_T result_data;
    AUTH_HDR *recv_auth_p;
    VALUE_PAIR *vp_p;
    char secret_key[MAXSIZE_radiusServerGlobalKey + 1];
    UI8_T vector[AUTH_VECTOR_LEN];
    UI32_T identifier;
    int result;
    int ret;
    RADIUS_OM_RadiusRequestType_T type;

    SENDSERVER_TRACE("");

    if (   (NULL == packet_p)
        || (NULL == code_p)
        || (NULL == error_p)
        )
    {
        return ERROR_RC;
    }

    if (   (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestResultData(request_index,
            &result_data))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestSecretKey(request_index,
            secret_key))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestVector(request_index,
            vector))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestIdentifier(request_index,
            &identifier))
        || (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index, &type))
        )
    {
        return ERROR_RC;
    }

    recv_auth_p = (AUTH_HDR *)packet_p;

    if (OK_RC != rc_check_reply(recv_auth_p, (char *)secret_key, vector, identifier,
        packet_len, error_p))
    {
        SENDSERVER_TRACE("Fail in rc_check_reply (error = %d)", *error_p);

        return ERROR_RC;
    }

    switch (type)
    {
    case RADIUS_REQUEST_TYPE_IGMPAUTH:
    case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
    case RADIUS_REQUEST_TYPE_RADA_AUTH:
    case RADIUS_REQUEST_TYPE_WEB_AUTH:
    case RADIUS_REQUEST_TYPE_USER_AUTH:
        switch (recv_auth_p->code)
        {
        case PW_ACCESS_ACCEPT:
        case PW_ACCESS_CHALLENGE:
        case PW_ACCESS_REJECT:
            break;

        default:
            SENDSERVER_TRACE("Unknown code (%d)", recv_auth_p->code);

            *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE;
            return ERROR_RC;
        }

        break;

    case RADIUS_REQUEST_TYPE_ACCOUNTING:
        switch (recv_auth_p->code)
        {
        case PW_ACCOUNTING_RESPONSE:
            break;

        default:
            SENDSERVER_TRACE("Unknown code (%d)", recv_auth_p->code);

            *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE;
            return ERROR_RC;
        }

        break;

    default:
        SENDSERVER_TRACE("Unknown type (%d)", type);

        *error_p = RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE;
        return ERROR_RC;
    }

    vp_p = rc_avpair_gen(recv_auth_p);

    if (   (NULL != vp_p)
        && (ERROR_RC == SENDSERVER_ExtractAvPair(request_index, &result_data, vp_p))
        )
    {
        SENDSERVER_TRACE("Fail to extract a/v pair (vp = %p)", vp_p);

        rc_avpair_free(vp_p);
        return ERROR_RC;
    }

    rc_avpair_free(vp_p);

    *code_p = recv_auth_p->code;

    RADIUS_OM_SetRequestResultData(request_index, &result_data);

    SENDSERVER_TRACE("Packet is ok");

    return OK_RC;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_GetPrivilegeFromCiscoAVPair
 *---------------------------------------------------------------------------
 * PURPOSE: Get the privilege level for Cisco AVPair
 * INPUT  : The list of AVPair
 * OUTPUT : privilege, AUTH_LOGIN~AUTH_ADMINISTRATIVE
 * RETURN : TRUE  - SUCCESS
 *          FALSE - FAILURE
 * NOTE   : This function would get the privilege level from the cisco-avpair(1)
 *          of Cisco(9) on the vendor specific(26) AV-Pair.
 *          If the RADIUS server returns login(1) in service-type(6), the privilege
 *          level would be take from cisco-avpair.
 *          If the RADIUS server returns without service-type(6) or cisco-avpair(1),
 *          the funtion would return FALSE.
 *---------------------------------------------------------------------------
 */
static BOOL_T SENDSERVER_GetPrivilegeFromCiscoAVPair(VALUE_PAIR *pair,
    I32_T *privilege)
{
    VALUE_PAIR *vp, *got_vp;

    got_vp = pair;

    vp = got_vp;

    if ((vp = rc_avpair_get(vp, PW_SERVICE_TYPE)))
    {
        if (vp->lvalue != PW_LOGIN)
        {
            /* no service type configured
             */
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    vp = got_vp;

    if ((vp = rc_avpair_get(vp, PW_CISCO_AVPAIR)))
    {
        I32_T priv_lvl;
        sscanf(vp->strvalue, "shell:priv-lvl=%ld", &priv_lvl);

        if ((priv_lvl < AUTH_LOGIN) || (priv_lvl > AUTH_ADMINISTRATIVE))
        {
            return FALSE;
        }

        *privilege = priv_lvl;

        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ExtractDot1xAuthAvPair
 *---------------------------------------------------------------------------
 * PURPOSE:  Extract the A/V pairs for dot1x authentication response packet.
 * INPUT:    request_index  - Request index
 *           result_data_p  - Result
 *           vp_p           - A/V pairs
 * OUTPUT:   result_data_p  - Result
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_ExtractDot1xAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p)
{
    int ret = OK_RC;
    VALUE_PAIR *tmp_vp_p;

    if (   (NULL == result_data_p)
        || (NULL == vp_p)
        )
    {
        return ERROR_RC;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_FILTER_ID);
    if (NULL != tmp_vp_p)
    {
        strncpy(result_data_p->dot1x_auth.filter_id,
            tmp_vp_p->strvalue,
            sizeof(result_data_p->dot1x_auth.filter_id) - 1);
        result_data_p->dot1x_auth.filter_id[
            sizeof(result_data_p->dot1x_auth.filter_id) - 1] = '\0';
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_TYPE);
    if (NULL != tmp_vp_p)
    {
        result_data_p->dot1x_auth.tunnel_type = tmp_vp_p->lvalue;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_MEDIUM_TYPE);
    if (NULL != tmp_vp_p)
    {
        result_data_p->dot1x_auth.tunnel_medium_type = tmp_vp_p->lvalue;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_PRIVATE_GROUP);
    if (NULL != tmp_vp_p)
    {
        char *real_value_p = tmp_vp_p->strvalue;

        if (tmp_vp_p->strvalue[0] <= 0x1F)
        {
            real_value_p = tmp_vp_p->strvalue + 1;
        }

        strncpy(result_data_p->dot1x_auth.tunnel_private_group_id, real_value_p,
            sizeof(result_data_p->dot1x_auth.tunnel_private_group_id) - 1);
        result_data_p->dot1x_auth.tunnel_private_group_id[
            sizeof(result_data_p->dot1x_auth.tunnel_private_group_id) - 1] = '\0';
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_SESSION_TIMEOUT);
    if (NULL != tmp_vp_p)
    {
        result_data_p->dot1x_auth.session_timeout = tmp_vp_p->lvalue;
    }

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ExtractRadaAuthAvPair
 *---------------------------------------------------------------------------
 * PURPOSE:  Extract the A/V pairs for RADA authentication response packet.
 * INPUT:    request_index  - Request index
 *           result_data_p  - Result
 *           vp_p           - A/V pairs
 * OUTPUT:   result_data_p  - Result
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_ExtractRadaAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p)
{
    int ret = OK_RC;
    VALUE_PAIR *tmp_vp_p;

    if (   (NULL == result_data_p)
        || (NULL == vp_p)
        )
    {
        return ERROR_RC;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_FILTER_ID);
    if (NULL != tmp_vp_p)
    {
        strncpy(result_data_p->rada_auth.filter_id,
            tmp_vp_p->strvalue,
            sizeof(result_data_p->rada_auth.filter_id) - 1);
        result_data_p->rada_auth.filter_id[
            sizeof(result_data_p->rada_auth.filter_id) - 1] = '\0';
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_TYPE);
    if (NULL != tmp_vp_p)
    {
        result_data_p->rada_auth.tunnel_type = tmp_vp_p->lvalue;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_MEDIUM_TYPE);
    if (NULL != tmp_vp_p)
    {
        result_data_p->rada_auth.tunnel_medium_type = tmp_vp_p->lvalue;
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_TUNNEL_PRIVATE_GROUP);
    if (NULL != tmp_vp_p)
    {
        char *real_value_p = tmp_vp_p->strvalue;

        if (tmp_vp_p->strvalue[0] <= 0x1F)
        {
            real_value_p = tmp_vp_p->strvalue + 1;
        }

        strncpy(result_data_p->rada_auth.tunnel_private_group_id, real_value_p,
            sizeof(result_data_p->rada_auth.tunnel_private_group_id) - 1);
        result_data_p->rada_auth.tunnel_private_group_id[
            sizeof(result_data_p->rada_auth.tunnel_private_group_id) - 1] = '\0';
    }

    tmp_vp_p = rc_avpair_get(vp_p, PW_SESSION_TIMEOUT);
    if (NULL != tmp_vp_p)
    {
        result_data_p->rada_auth.session_timeout = tmp_vp_p->lvalue;
    }

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ExtractUserAuthAvPair
 *---------------------------------------------------------------------------
 * PURPOSE:  Extract the A/V pairs for user authentication response packet.
 * INPUT:    request_index  - Request index
 *           result_data_p  - Result
 *           vp_p           - A/V pairs
 * OUTPUT:   result_data_p  - Result
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_ExtractUserAuthAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p)
{
    int ret = OK_RC;

    if (   (NULL == result_data_p)
        || (NULL == vp_p)
        )
    {
        return ERROR_RC;
    }

    result_data_p->user_auth.privilege = AUTH_LOGIN_ERROR;

    if (FALSE == SENDSERVER_GetPrivilegeFromCiscoAVPair(vp_p,
        &result_data_p->user_auth.privilege))
    {
        SENDSERVER_TRACE("Failed to get privilege from Cisco a/v pair");

        if (SYS_ADPT_RADIUS_USE_SERVICE_TYPE_AS_PRIVILEGE == TRUE)
        {
            VALUE_PAIR *tmp_vp_p;

            SENDSERVER_TRACE("Get privilege from Service-Type a/v pair");

            tmp_vp_p = rc_avpair_get(vp_p, PW_SERVICE_TYPE);
            if (NULL != tmp_vp_p)
            {
                SENDSERVER_TRACE("Service type as privilege (%lu)",
                    tmp_vp_p->lvalue);

                result_data_p->user_auth.privilege = tmp_vp_p->lvalue;
            }
        }

        if (SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE == TRUE)
        {
            VALUE_PAIR *tmp_vp_p;

            SENDSERVER_TRACE("Get privilege from Filter-Id a/v pair");

            tmp_vp_p = rc_avpair_get(vp_p, PW_FILTER_ID);
            if (NULL != tmp_vp_p)
            {
                if (NULL != strstr(tmp_vp_p->strvalue,
                    SYS_ADPT_RADIUS_FIELD_ID_FOR_ADMIN_PRIVILEGE))
                {
                    SENDSERVER_TRACE("Field ID = %s",
                        SYS_ADPT_RADIUS_FIELD_ID_FOR_ADMIN_PRIVILEGE);

                    result_data_p->user_auth.privilege = AUTH_ADMINISTRATIVE;
                }
                else if (NULL != strstr(tmp_vp_p->strvalue,
                    SYS_ADPT_RADIUS_FIELD_ID_FOR_SUPPER_PRIVILEGE))
                {
                    SENDSERVER_TRACE("Field ID = %s",
                        SYS_ADPT_RADIUS_FIELD_ID_FOR_SUPPER_PRIVILEGE);

                    result_data_p->user_auth.privilege = AUTH_ADMINISTRATIVE;
                }
                else if (NULL != strstr(tmp_vp_p->strvalue,
                    SYS_ADPT_RADIUS_FIELD_ID_FOR_GUEST_PRIVILEGE))
                {
                    SENDSERVER_TRACE("Field ID = %s",
                        SYS_ADPT_RADIUS_FIELD_ID_FOR_GUEST_PRIVILEGE);

                    result_data_p->user_auth.privilege = AUTH_LOGIN;
                }
                else
                {
                    SENDSERVER_TRACE("Field ID is unknown");

                    result_data_p->user_auth.privilege = AUTH_LOGIN_ERROR;
                    ret = ERROR_RC;
                }
            }
        }
    }

    SENDSERVER_TRACE("ret = %d", ret);

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ExtractAvPair
 *---------------------------------------------------------------------------
 * PURPOSE:  Extract the A/V pairs based on request type.
 * INPUT:    request_index  - Request index
 *           result_data_p  - Result
 *           vp_p           - A/V pairs
 * OUTPUT:   result_data_p  - Result
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static int SENDSERVER_ExtractAvPair(UI32_T request_index,
    RADIUS_OM_RequestResult_T *result_data_p, VALUE_PAIR *vp_p)
{
    RADIUS_OM_RadiusRequestType_T type;

    SENDSERVER_TRACE("");

    if (   (NULL == result_data_p)
        || (NULL == vp_p)
        )
    {
        return ERROR_RC;
    }

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetRequestType(request_index, &type))
    {
        return ERROR_RC;
    }

    switch (type)
    {
#if (SYS_CPNT_IGMPAUTH == TRUE)
    case RADIUS_REQUEST_TYPE_IGMPAUTH:
        return OK_RC;
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

    case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
        return SENDSERVER_ExtractDot1xAuthAvPair(request_index,
            result_data_p, vp_p);

    case RADIUS_REQUEST_TYPE_RADA_AUTH:
        return SENDSERVER_ExtractRadaAuthAvPair(request_index,
            result_data_p, vp_p);

    case RADIUS_REQUEST_TYPE_WEB_AUTH:
    case RADIUS_REQUEST_TYPE_USER_AUTH:
        return SENDSERVER_ExtractUserAuthAvPair(request_index,
            result_data_p, vp_p);

    default:
        SENDSERVER_TRACE("Unknown type (%d)", type);

        return ERROR_RC;
    }

    SENDSERVER_TRACE("Unknown type (%d)", type);

    return ERROR_RC;
}