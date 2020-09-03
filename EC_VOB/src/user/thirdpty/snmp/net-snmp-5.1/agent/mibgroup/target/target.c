#include <net-snmp/net-snmp-config.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifndef VXWORKS
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#endif /*end of #ifndef VXWORKS*/

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmpTargetAddrEntry.h"
#include "snmpTargetParamsEntry.h"
#include "target.h"

#ifdef VXWORKS
#include "leaf_3413t.h"
#endif

#include <time.h>

#ifndef VXWORKS
#define MAX_TAGS 128
#else
#define MAX_TAGS 5
#define MAX_TAG_LEN 32
#endif

netsnmp_session *
#ifdef VXWORKS
get_target_sessions(char *taglist, int tagtype, char* targetparamsname,
		    TargetFilterFunction * filterfunct,
#else
get_target_sessions(char *taglist, TargetFilterFunction * filterfunct,
#endif
                    void *filterArg)
{
    netsnmp_session *ret = NULL, thissess;
    struct targetAddrTable_struct *targaddrs;
    char            buf[SPRINT_MAX_LEN];
#ifdef VXWORKS
    char            tags[MAX_TAGS][MAX_TAG_LEN], *cp;
#else
    char            tags[MAX_TAGS][SPRINT_MAX_LEN], *cp;
#endif
    int             numtags = 0, i;
    static struct targetParamTable_struct *param;

    DEBUGMSGTL(("target_sessions", "looking for: %s\n", taglist));
    for (cp = taglist; cp && numtags < MAX_TAGS;) {
        cp = copy_nword(cp, tags[numtags], sizeof(tags[numtags]));
        DEBUGMSGTL(("target_sessions", " for: %d=%s\n", numtags,
                    tags[numtags]));
        numtags++;
    }

    for (targaddrs = get_addrTable(); targaddrs;
         targaddrs = targaddrs->next) {

        /*
         * legal row?
         */
        if (targaddrs->tDomain == NULL ||
            targaddrs->tAddress == NULL ||
            targaddrs->rowStatus != SNMP_ROW_ACTIVE) {
            DEBUGMSGTL(("target_sessions", "  which is not ready yet\n"));
            continue;
        }

        if (netsnmp_tdomain_support
            (targaddrs->tDomain, targaddrs->tDomainLen, NULL, NULL) == 0) {
            snmp_log(LOG_ERR,
                     "unsupported domain for target address table entry %s\n",
                     targaddrs->name);
        }

        /*
         * check tag list to see if we match
         */
        if (targaddrs->tagList) {
            /*
             * loop through tag list looking for requested tags
             */
            for (cp = targaddrs->tagList; cp;) {
                cp = copy_nword(cp, buf, sizeof(buf));
                for (i = 0; i < numtags; i++) {
                    if (strcmp(buf, tags[i]) == 0) {
                        /*
                         * found a valid target table entry
                         */
                        DEBUGMSGTL(("target_sessions", "found one: %s\n",
                                    tags[i]));

                        if (targaddrs->params) {
                            param = get_paramEntry(targaddrs->params);
                            if (!param
                                || param->rowStatus != SNMP_ROW_ACTIVE) {
                                /*
                                 * parameter entry must exist and be active
                                 */
                                continue;
                            }
                        } else {
                            /*
                             * parameter entry must be specified
                             */
                            continue;
                        }

                        /*
                         * last chance for caller to opt-out.  Call
                         * filtering function
                         */
                        #ifdef VXWORKS
                        strncpy(targetparamsname, targaddrs->params, MAXSIZE_snmpTargetParamsName);
                        targetparamsname[MAXSIZE_snmpTargetParamsName]=0;
                        #endif

                        if (filterfunct &&
                            (*(filterfunct)) (targaddrs, param,
                                              filterArg)) {
                            continue;
                        }

                        if (targaddrs->storageType != ST_READONLY &&
                            targaddrs->sess &&
                            param->updateTime >=
                            targaddrs->sessionCreationTime) {
                            /*
                             * parameters have changed, nuke the old session
                             */
                            snmp_close_sess_only(targaddrs->sess);
                            targaddrs->sess = NULL;
                        }
                        /* kinghong, 2004/4/29, inform request support will no need to call this function*/
                        #if 0
                        #ifdef VXWORKS
                          SNMPUDPDOMAIN_CreateTrapSocket();
                        #endif
                        #endif
                        /*
                         * target session already exists?
                         */
                        if (targaddrs->sess == NULL) {
                            /*
                             * create an appropriate snmp session and add
                             * it to our return list
                             */
                            netsnmp_transport *t = NULL;

                            #ifdef VXWORKS
                            t = SNMP_TRANSPORT_netsnmp_tdomain_transport_oid(targaddrs->
                                                              tDomain,
                                                              targaddrs->
                                                              tDomainLen,
                                                              targaddrs->
                                                              tAddress,
                                                              targaddrs->
                                                              tAddressLen,
                                                              0);
                            #else
                            t = netsnmp_tdomain_transport_oid(targaddrs->
                                                              tDomain,
                                                              targaddrs->
                                                              tDomainLen,
                                                              targaddrs->
                                                              tAddress,
                                                              targaddrs->
                                                              tAddressLen,
                                                              0);
                            #endif
                            if (t == NULL) {
                                DEBUGMSGTL(("target_sessions",
                                            "bad dest \""));
                                DEBUGMSGOID(("target_sessions",
                                             targaddrs->tDomain,
                                             targaddrs->tDomainLen));
                                DEBUGMSG(("target_sessions", "\", \""));
                                DEBUGMSGHEX(("target_sessions",
                                             targaddrs->tAddress,
                                             targaddrs->tAddressLen));
                                DEBUGMSG(("target_sessions", "\n"));
                                continue;
                            } else {
                              #ifdef VXWORKS
                              char           *dst_str=NULL;
			                  if (t->f_fmtaddr != NULL)
				              dst_str =  t->f_fmtaddr(t, NULL, 0);
                              #else
                                char           *dst_str =
                                    t->f_fmtaddr(t, NULL, 0);
                              #endif
                                if (dst_str != NULL) {
                                    DEBUGMSGTL(("target_sessions",
                                                "  to: %s\n", dst_str));
                                    free(dst_str);
                                }
                            }
                            memset(&thissess, 0, sizeof(thissess));
                            /* kinghong fixed, the units of timeout is 0.01 seconds, should time 10000*/
                           /* thissess.timeout = (targaddrs->timeout) * 1000;*/

                            thissess.timeout = (targaddrs->timeout) * 10000;  /* the unit of timeout should be us (defined in struct snmp_session in snmp_api.h) */
                            thissess.retries = targaddrs->retryCount;
                            DEBUGMSGTL(("target_sessions",
                                        "timeout: %d -> %d\n",
                                        targaddrs->timeout,
                                        thissess.timeout));

                            if (param->mpModel == SNMP_VERSION_3 &&
                                param->secModel != 3) {
                                snmp_log(LOG_ERR,
                                         "unsupported model/secmodel combo for target %s\n",
                                         targaddrs->name);
                                /*
                                 * XXX: memleak
                                 */
                                netsnmp_transport_free(t);
                                continue;
                            }
                            thissess.version = param->mpModel;
                            if (param->mpModel == SNMP_VERSION_3) {
                                thissess.securityName = param->secName;
                                thissess.securityNameLen =
                                    strlen(thissess.securityName);
                                thissess.securityLevel = param->secLevel;

				thissess.securityModel = param->secModel;
				if (tagtype == 1 /* is SNMPNOTIFYTYPE_TRAP*/) {
				    /* if this is a trap, this agent is
				     * the authorative agent so don't
				     * send the probe for reports from
				     * the manager
				     */
				    thissess.flags |= SNMP_FLAGS_DONT_PROBE;
				} /* end */
                            } else {
                                thissess.community =
                                    (u_char *) strdup(param->secName);
                                thissess.community_len =
                                    strlen((char *) thissess.community);
                            }

                            targaddrs->sess = snmp_add(&thissess, t,
                                                       NULL, NULL);
                            targaddrs->sessionCreationTime = time(NULL);
                        }
                        if (targaddrs->sess) {
                            if (ret) {
                                targaddrs->sess->next = ret;
                            }
                            ret = targaddrs->sess;
                        } else {
                            snmp_sess_perror("target session", &thissess);
                        }
                    }
                }
            }
        }
    }
    return ret;
}
