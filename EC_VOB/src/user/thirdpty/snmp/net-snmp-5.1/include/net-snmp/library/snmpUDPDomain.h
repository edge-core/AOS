#ifndef _SNMPUDPDOMAIN_H
#define _SNMPUDPDOMAIN_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

netsnmp_transport *netsnmp_udp_transport(struct sockaddr_in *addr, int local);

#ifdef VXWORKS
typedef struct _com2SecEntry {
    char            community[34]; /* VACMSTRINGLEN defines as 34 in vacm.h*/
    unsigned long   network;
    unsigned long   mask;
    char            secName[34];  /* VACMSTRINGLEN defines as 34 in vacm.h*/
    struct _com2SecEntry *next;
    int     access_right;
} com2SecEntry;
#endif
/*
 * Convert a "traditional" peername into a sockaddr_in structure which is
 * written to *addr.  Returns 1 if the conversion was successful, or 0 if it
 * failed.  
 */

int             netsnmp_sockaddr_in(struct sockaddr_in *addr,
                                    const char *peername, int remote_port);


/*
 * Register any configuration tokens specific to the agent.  
 */

void            netsnmp_udp_agent_config_tokens_register(void);

#ifndef VXWORKS
void            netsnmp_udp_parse_security(const char *token, char *param);
#else
UI32_T            netsnmp_udp_parse_security(const char *token, char *param, int access_right);
#endif

int             netsnmp_udp_getSecName(void *opaque, int olength,
                                       const char *community,
                                       size_t community_len,
                                       char **secname);
#ifdef VXWORKS
/* kinghong add the following*/
void  netsnmp_udp_destroyViewEntry(const char *comm);
struct _com2SecEntry *netsnmp_udp_get_com2SecList(void);
struct _com2SecEntry *netsnmp_udp_getCom2SecEntry(const char *secName);
/* kinghong add the below*/
netsnmp_transport *
SNMPUDPDOMAIN_netsnmp_udp_create_ostring(const u_char * o, size_t o_len, int 
local);
#endif
/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_udp_ctor(void);

#ifdef __cplusplus
}
#endif
#endif/*_SNMPUDPDOMAIN_H*/
