/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>
#ifndef VXWORKS
#include <stdlib.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#endif /*endof #ifndef VXWORKS*/

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/instance.h>
#include <net-snmp/agent/serialize.h>
#include <net-snmp/agent/read_only.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif
#include "sys_cpnt.h"
#if (SYS_CPNT_DBSYNC_TXT == TRUE)
#include "snmp_mgr.h"
#include "dbsync_txt_mgr.h"
#endif

/** @defgroup instance instance: process individual MIB instances easily.
 *  @ingroup leaf
 *  @{
 */
netsnmp_mib_handler *
netsnmp_get_instance_handler(void)
{
    return netsnmp_create_handler("instance",
                                  netsnmp_instance_helper_handler);
}

int
netsnmp_register_instance(netsnmp_handler_registration *reginfo)
{
    netsnmp_inject_handler(reginfo, netsnmp_get_instance_handler());
    return netsnmp_register_serialize(reginfo);
}

int
netsnmp_register_read_only_instance(netsnmp_handler_registration *reginfo)
{
    netsnmp_inject_handler(reginfo, netsnmp_get_instance_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_read_only_handler());
    return netsnmp_register_serialize(reginfo);
}

static
netsnmp_handler_registration *
get_reg(const char *name,
        const char *ourname,
        oid * reg_oid, size_t reg_oid_len,
        void *it,
        int modes,
        Netsnmp_Node_Handler * scalarh, Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;
    netsnmp_mib_handler *myhandler;

    if (subhandler) {
        myreg =
            netsnmp_create_handler_registration(name,
                                                subhandler,
                                                reg_oid, reg_oid_len,
                                                modes);
        myhandler = netsnmp_create_handler(ourname, scalarh);
        myhandler->myvoid = (void *) it;
        netsnmp_inject_handler(myreg, myhandler);
    } else {
        myreg =
            netsnmp_create_handler_registration(name,
                                                scalarh,
                                                reg_oid, reg_oid_len,
                                                modes);
        myreg->handler->myvoid = (void *) it;
    }
    return myreg;
}

int
netsnmp_register_read_only_ulong_instance(const char *name,
                                          oid * reg_oid,
                                          size_t reg_oid_len, u_long * it,
                                          Netsnmp_Node_Handler *
                                          subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "ulong_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RONLY, netsnmp_instance_ulong_handler,
                    subhandler);
    return netsnmp_register_read_only_instance(myreg);
}

int
netsnmp_register_ulong_instance(const char *name,
                                oid * reg_oid, size_t reg_oid_len,
                                u_long * it,
                                Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "ulong_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RWRITE, netsnmp_instance_ulong_handler,
                    subhandler);
    return netsnmp_register_instance(myreg);
}

int
netsnmp_register_read_only_counter32_instance(const char *name,
                                              oid * reg_oid,
                                              size_t reg_oid_len,
                                              u_long * it,
                                              Netsnmp_Node_Handler *
                                              subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "counter32_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RWRITE, netsnmp_instance_counter32_handler,
                    subhandler);
    return netsnmp_register_read_only_instance(myreg);
}

int
netsnmp_register_read_only_long_instance(const char *name,
                                         oid * reg_oid, size_t reg_oid_len,
                                         long *it,
                                         Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "long_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RONLY, netsnmp_instance_long_handler,
                    subhandler);
    return netsnmp_register_read_only_instance(myreg);
}

int
netsnmp_register_long_instance(const char *name,
                               oid * reg_oid, size_t reg_oid_len,
                               long *it, Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "long_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RWRITE, netsnmp_instance_long_handler,
                    subhandler);
    return netsnmp_register_instance(myreg);
}

int
netsnmp_register_read_only_int_instance(const char *name,
                                oid * reg_oid, size_t reg_oid_len,
                                int *it, Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "int_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RONLY, netsnmp_instance_int_handler,
                    subhandler);
    return netsnmp_register_read_only_instance(myreg);
}

  /*
   * Compatibility with earlier (inconsistently named) routine
   */
int
register_read_only_int_instance(const char *name,
                                oid * reg_oid, size_t reg_oid_len,
                                int *it, Netsnmp_Node_Handler * subhandler)
{
  return netsnmp_register_read_only_int_instance(name,
                                reg_oid, reg_oid_len,
                                it, subhandler);
}

int
netsnmp_register_int_instance(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              int *it, Netsnmp_Node_Handler * subhandler)
{
    netsnmp_handler_registration *myreg;

    myreg = get_reg(name, "int_handler", reg_oid, reg_oid_len, it,
                    HANDLER_CAN_RWRITE, netsnmp_instance_int_handler,
                    subhandler);
    return netsnmp_register_instance(myreg);
}

int
netsnmp_instance_ulong_handler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{

    u_long         *it = (u_long *) handler->myvoid;
    u_long         *it_save;

    DEBUGMSGTL(("netsnmp_instance_ulong_handler", "Got request:  %d\n",
                reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
                                 (u_char *) it, sizeof(*it));
        break;

        /*
         * SET requests.  Should only get here if registered RWRITE 
         */
    case MODE_SET_RESERVE1:
        if (requests->requestvb->type != ASN_UNSIGNED)
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGTYPE);
        break;

    case MODE_SET_RESERVE2:
        /*
         * store old info for undo later 
         */
        memdup((u_char **) & it_save, (u_char *) it, sizeof(u_long));
        if (it_save == NULL) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests,
                                      netsnmp_create_data_list
                                      (INSTANCE_HANDLER_NAME, it_save,
                                       free));
        break;

    case MODE_SET_ACTION:
        /*
         * update current 
         */
        DEBUGMSGTL(("testhandler", "updated u_long %ul -> %ul\n", *it,
                    *(requests->requestvb->val.integer)));
        *it = *(requests->requestvb->val.integer);
        break;

    case MODE_SET_UNDO:
        *it =
            *((u_long *) netsnmp_request_get_list_data(requests,
                                                       INSTANCE_HANDLER_NAME));
        break;

    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
        /*
         * nothing to do 
         */
        break;
    }

    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);

    return SNMP_ERR_NOERROR;
}

int
netsnmp_instance_counter32_handler(netsnmp_mib_handler *handler,
                                   netsnmp_handler_registration *reginfo,
                                   netsnmp_agent_request_info *reqinfo,
                                   netsnmp_request_info *requests)
{

    u_long         *it = (u_long *) handler->myvoid;

    DEBUGMSGTL(("netsnmp_instance_counter32_handler",
                "Got request:  %d\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                 (u_char *) it, sizeof(*it));
        break;

        /*
         * SET requests.  Should only get here if registered RWRITE 
         */
    default:
        snmp_log(LOG_ERR,
                 "netsnmp_instance_counter32_handler: illegal mode\n");
        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
        return SNMP_ERR_NOERROR;
    }
    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);
    return SNMP_ERR_NOERROR;
}

int
netsnmp_instance_long_handler(netsnmp_mib_handler *handler,
                              netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo,
                              netsnmp_request_info *requests)
{

    long           *it = (long *) handler->myvoid;
    long           *it_save;

    DEBUGMSGTL(("netsnmp_instance_long_handler", "Got request:  %d\n",
                reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 (u_char *) it, sizeof(*it));
        break;

        /*
         * SET requests.  Should only get here if registered RWRITE 
         */
    case MODE_SET_RESERVE1:
        if (requests->requestvb->type != ASN_INTEGER)
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGTYPE);
        break;

    case MODE_SET_RESERVE2:
        /*
         * store old info for undo later 
         */
        memdup((u_char **) & it_save, (u_char *) it, sizeof(u_long));
        if (it_save == NULL) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests,
                                      netsnmp_create_data_list
                                      (INSTANCE_HANDLER_NAME, it_save,
                                       free));
        break;

    case MODE_SET_ACTION:
        /*
         * update current 
         */
        DEBUGMSGTL(("testhandler", "updated u_long %ul -> %ul\n", *it,
                    *(requests->requestvb->val.integer)));
        *it = *(requests->requestvb->val.integer);
        break;

    case MODE_SET_UNDO:
        *it =
            *((u_long *) netsnmp_request_get_list_data(requests,
                                                       INSTANCE_HANDLER_NAME));
        break;

    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
        /*
         * nothing to do 
         */
        break;
    }
    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);
    return SNMP_ERR_NOERROR;
}

int
netsnmp_instance_int_handler(netsnmp_mib_handler *handler,
                             netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *reqinfo,
                             netsnmp_request_info *requests)
{

    int *it = (int *) handler->myvoid;
    int *it_save;
    long tmp_it;
    
    DEBUGMSGTL(("netsnmp_instance_int_handler", "Got request:  %d\n",
                reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
	/*
	 * Use a long here, otherwise on 64 bit use of an int would fail
	 */
	tmp_it = *it;
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 (u_char *) &tmp_it, sizeof(long));
        break;

        /*
         * SET requests.  Should only get here if registered RWRITE 
         */
    case MODE_SET_RESERVE1:
        if (requests->requestvb->type != ASN_INTEGER)
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGTYPE);
        break;

    case MODE_SET_RESERVE2:
        /*
         * store old info for undo later 
         */
        memdup((u_char **) & it_save, (u_char *) it, sizeof(u_long));
        if (it_save == NULL) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests,
                                      netsnmp_create_data_list
                                      (INSTANCE_HANDLER_NAME, it_save,
                                       free));
        break;

    case MODE_SET_ACTION:
        /*
         * update current 
         */
        DEBUGMSGTL(("testhandler", "updated int %d -> %l\n", *it,
                    *(requests->requestvb->val.integer)));
        *it = (int) *(requests->requestvb->val.integer);
        break;

    case MODE_SET_UNDO:
        *it =
            *((u_int *) netsnmp_request_get_list_data(requests,
                                                      INSTANCE_HANDLER_NAME));
        break;

    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
        /*
         * nothing to do 
         */
        break;
    }
    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);
    return SNMP_ERR_NOERROR;
}

int
netsnmp_instance_helper_handler(netsnmp_mib_handler *handler,
                                netsnmp_handler_registration *reginfo,
                                netsnmp_agent_request_info *reqinfo,
                                netsnmp_request_info *requests)
{

    netsnmp_variable_list *var = requests->requestvb;

    int             ret, cmp;

    DEBUGMSGTL(("helper:instance", "Got request:\n"));
    cmp = snmp_oid_compare(requests->requestvb->name,
                           requests->requestvb->name_length,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(("helper:instance", "  oid:", cmp));
    DEBUGMSGOID(("helper:instance", var->name, var->name_length));
    DEBUGMSG(("helper:instance", "\n"));

    switch (reqinfo->mode) {
    case MODE_GET:
        if (cmp != 0) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_NOSUCHINSTANCE);
            return SNMP_ERR_NOERROR;
        } else {
            return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                             requests);
        }
        break;

    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_UNDO:
    case MODE_SET_FREE:
        if (cmp != 0) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_NOCREATION);
            return SNMP_ERR_NOERROR;
        } else {
          #if   (SYS_CPNT_DBSYNC_TXT == TRUE) 
/* kinghong added for autoave, before Set an variable,
 * we need to set the dirty bit to 1
 */
BOOL_T  tmpDirty;
if (reqinfo->mode == MODE_SET_ACTION)
{
if (!SNMP_MGR_SetTmpDirty(1))
    printf("autoSave   Fail\n");
}
#endif   
            ret= netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                             requests);

#if   (SYS_CPNT_DBSYNC_TXT == TRUE)         
           /*kinghong add for autosave, after set the variable, we will
            *check the dirty bit, if the dirty bit is TRUE, we need to
            *call DBSYNC for autosave
            */
            if (reqinfo->mode == MODE_SET_ACTION)
            {
                if(!SNMP_MGR_GetTmpDirty(&tmpDirty))
                    printf("autoSave   Fail\n");
                if (tmpDirty)
                   DBSYNC_TXT_MGR_SetDirty(TRUE);
            }
        /*  end of  kinghong added*/      
#endif               
        return ret;   
        }
        break;

    case MODE_GETNEXT:
        if (cmp < 0 || (cmp == 0 && requests->inclusive)) {
            reqinfo->mode = MODE_GET;
            snmp_set_var_objid(requests->requestvb, reginfo->rootoid,
                               reginfo->rootoid_len);
            ret =
                netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                          requests);
            reqinfo->mode = MODE_GETNEXT;
            return ret;
        } else {
            return SNMP_ERR_NOERROR;
        }
        break;
    }
    /*
     * got here only if illegal mode found 
     */
    return SNMP_ERR_GENERR;
}

/*
 * @} 
 */
