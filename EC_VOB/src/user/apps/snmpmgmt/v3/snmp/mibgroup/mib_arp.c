#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "netcfg_pom_nd.h"
#include "netcfg_pmgr_nd.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "mib_arp.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"

#if (SYS_CPNT_PROXY_ARP == TRUE)
/********************************************
 ********************acctArpMgt******************
 ********************************************
 */

static BOOL_T header_arpProxyArpEntry(struct variable *vp,
                                               oid * name,
                                               size_t * length,
                                               int exact,
                                               UI32_T *index);

#endif


#if (SYS_CPNT_ARP == TRUE)

int
do_arpCacheTimeout(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo,
                            netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
              {
              UI32_T  value;

              if (NETCFG_POM_ND_GetIpNetToMediaTimeout(&value)!= TRUE)
                       return SNMP_ERR_GENERR;
                  long_return = value;
                  snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
             }
            break;
        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            {
           UI32_T value;

           if(requests->requestvb->type != ASN_INTEGER)
           {
               netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
               break;
           }
           if(requests->requestvb->val_len > sizeof(long))
           {
               netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
               break;
           }
           value = (*requests->requestvb->val.integer);
           if ((value<MIN_arpCacheTimeout)  ||    (value>MAX_arpCacheTimeout))
           {
               netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
               break;
           }
          }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
             {
                UI32_T value;
                value = (*requests->requestvb->val.integer);
                 if (NETCFG_PMGR_ND_SetIpNetToMediaTimeout(value) != NETCFG_TYPE_OK)
                       netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_arpStatRcvRequestPackets(netsnmp_mib_handler *handler,
                                         netsnmp_handler_registration *reginfo,
                                         netsnmp_agent_request_info *reqinfo,
                                         netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
           {
               NETCFG_TYPE_IpNetToMedia_Statistics_T stat;
               memset(&stat, 0, sizeof(NETCFG_TYPE_IpNetToMedia_Statistics_T));
               if (NETCFG_PMGR_ND_GetStatistics(&stat)!=NETCFG_TYPE_OK)
                   return SNMP_ERR_GENERR;
                long_return =  stat.in_request;
                snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));
            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_arpStatSendReplyPackets(netsnmp_mib_handler *handler,
                                       netsnmp_handler_registration *reginfo,
                                       netsnmp_agent_request_info *reqinfo,
                                       netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
           {
               NETCFG_TYPE_IpNetToMedia_Statistics_T stat;
               memset(&stat, 0, sizeof(NETCFG_TYPE_IpNetToMedia_Statistics_T));
               if (NETCFG_PMGR_ND_GetStatistics(&stat)!=NETCFG_TYPE_OK)
                   return SNMP_ERR_GENERR;
                long_return =  stat.out_reply;
                snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));
            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_arpStatRcvReplyPackets(netsnmp_mib_handler *handler,
                                     netsnmp_handler_registration *reginfo,
                                     netsnmp_agent_request_info *reqinfo,
                                     netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
           {
               NETCFG_TYPE_IpNetToMedia_Statistics_T stat;
               memset(&stat, 0, sizeof(NETCFG_TYPE_IpNetToMedia_Statistics_T));
               if (NETCFG_PMGR_ND_GetStatistics(&stat)!=NETCFG_TYPE_OK)
                   return SNMP_ERR_GENERR;
                long_return =  stat.in_reply;
                snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));
            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_arpStatSendRequestPackets(netsnmp_mib_handler *handler,
                                          netsnmp_handler_registration *reginfo,
                                          netsnmp_agent_request_info *reqinfo,
                                          netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
           {
               NETCFG_TYPE_IpNetToMedia_Statistics_T stat;
               memset(&stat, 0, sizeof(NETCFG_TYPE_IpNetToMedia_Statistics_T));
               if (NETCFG_PMGR_ND_GetStatistics(&stat)!=NETCFG_TYPE_OK)
                   return SNMP_ERR_GENERR;
                long_return =  stat.out_request;
                snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *)&long_return, sizeof(long_return));
            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_arpCacheDeleteAll(netsnmp_mib_handler *handler,
                             netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *reqinfo,
                             netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode) {

        case MODE_GET:
              {

                  long_return = VAL_arpCacheDeleteAll_noDelete;
                  snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
              }
            break;
        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            {
           UI32_T value;

           if(requests->requestvb->type != ASN_INTEGER)
           {
               netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
               break;
           }
           if(requests->requestvb->val_len > sizeof(long))
           {
               netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
               break;
           }
           value = (*requests->requestvb->val.integer);
           if ((value<VAL_arpCacheDeleteAll_delete)  ||    (value>VAL_arpCacheDeleteAll_noDelete))
           {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_WRONGVALUE);
                break;
           }
          }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */

            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
             {
                UI32_T value;
                value = (*requests->requestvb->val.integer);
                if (value == VAL_arpCacheDeleteAll_delete)
                {
                    if (NETCFG_PMGR_ND_DeleteAllDynamicIpv4NetToMediaEntry()!=NETCFG_TYPE_OK)
                    {
                         netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
                    }
                }
            }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */

            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */

            break;

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

#endif /* #if (SYS_CPNT_ARP == TRUE) */


#if (SYS_CPNT_PROXY_ARP == TRUE)

/* arpProxyArpTable*/

static BOOL_T header_arpProxyArpEntry(struct variable *vp,
                                               oid * name,
                                               size_t * length,
                                               int exact,
                                               UI32_T *index)
{
    int     result,len;

    result = snmp_oid_compare(name, *length, vp->name, (int)vp->namelen);

    if(exact)/*Get */
    {
        /* Check the length. */
        if (result < 0 || *length - vp->namelen != 1)
        {
            return FALSE;
        }
        /* Get index*/
        *index = name[vp->namelen];
        return TRUE;
    }
    else
    {
        if (result >= 0)
        {
            len = *length - vp->namelen;
            if(len == 0)
            {
                *index = 0;
            }
            else if(len == 1)
            {
                *index = name[vp->namelen];
            }
            else if(len > 1)
            {
                *index = name[vp->namelen];
            }
            return TRUE;
        }
        else
        {
            /* set the user's oid to be ours */
            memcpy (name, vp->name, ((int) vp->namelen) * sizeof (oid));
            return TRUE;
        }
    }
}

/*
 * var_arpProxyArpTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_arpProxyArpTable(struct variable *vp,
                             oid * name,
                             size_t * length,
                             int exact,
                             size_t * var_len, WriteMethod ** write_method)
{
    UI32_T index = 0;
    BOOL_T status;

   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch (vp->magic)
    {
       case ARPPROXYARPSTATUS:
        *write_method = write_arpProxyArpStatus;
      break;
      default:
         *write_method =0;
         break;
    }

    if (header_arpProxyArpEntry(vp, name, length, exact, &index) == FALSE)
        return NULL;


    if (exact)
    {
        if(NETCFG_POM_IP_GetIpNetToMediaProxyStatus(index, &status) != NETCFG_TYPE_OK)
        {
            return NULL;
        }
    }
    else
    {
        if(NETCFG_POM_IP_GetNextIpNetToMediaProxyStatus(&index, &status) != NETCFG_TYPE_OK)
        {
            return NULL;
        }
        name[vp->namelen] = index;
        *length = vp->namelen + 1;
    }

    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
      case ARPPROXYARPSTATUS:
        if(status == TRUE)
        {
            *var_len = sizeof(long_return);
            long_return = VAL_arpProxyArpStatus_enabled;
        }
        else
        {
            *var_len = sizeof(long_return);
            long_return = VAL_arpProxyArpStatus_disabled;
        }
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


int
write_arpProxyArpStatus(int action,
                                u_char * var_val,
                                u_char var_val_type,
                                size_t var_val_len,
                                u_char * statP, oid * name, size_t name_len)
{
    long value;
    UI32_T index;
    BOOL_T status;

    UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 6;

      /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  1 + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    index = name[oid_name_length];

    switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          value = *(long *)var_val;
          if ((value <VAL_arpProxyArpStatus_enabled) || (value >VAL_arpProxyArpStatus_disabled))
             return SNMP_ERR_WRONGVALUE;
          break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
        {

             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */

              value = * (long *) var_val;

              if (value == VAL_arpProxyArpStatus_enabled)
              {
                   status = TRUE;
                   if (NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(index,status) != NETCFG_TYPE_OK)
                   {
                       return SNMP_ERR_COMMITFAILED;
                   }
              }
              else if (value == VAL_arpProxyArpStatus_disabled)
              {
                   status = FALSE;
                   if (NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(index,status) != NETCFG_TYPE_OK)
                   {
                      return SNMP_ERR_COMMITFAILED;
                   }
              }
         }
          break;

        case UNDO:
             /* Back out any changes made in the ACTION case */
          break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
          break;
    }
    return SNMP_ERR_NOERROR;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

