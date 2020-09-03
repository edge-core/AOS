#if((SYS_CPNT_A3COM0458_PORT_SECURITY_MIB==TRUE) &&(SYS_CPNT_NETWORK_ACCESS==TRUE))
#include "sys_cpnt.h"
#include "sys_type.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "snmp_mgr.h"
#include "networkaccess_mgr.h"
#include "leaf_a3com0458PortSecurity.h"
#include "A3com0458PortSecurity.h"

#define securePortOidNameLength 9

void
init_securePort(void)
{
    static oid      secureStop_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 3, 0 };
    static oid      secureRadaDefaultSessionTime_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 1, 0 };
    static oid      securePortVlanMembershipList_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 6, 0 };
    static oid      secureRadaHoldoffTime_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 2, 0 };
    static oid      secureRadaAuthPassword_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 6, 0 };
    static oid      secureRadaAuthMode_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 4, 0 };
    static oid      secureRadaReauthenticate_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 3, 0 };
    static oid      secureRadaAuthUsername_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 7, 5, 0 };
    static oid      securePortSecurityControl_oid[] =
        { 1, 3, 6, 1, 4, 1, 43, 10, 22, 4, 0 };

    DEBUGMSGTL(("securePort", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("secureStop",
                                         get_secureStop,
                                         secureStop_oid,
                                         OID_LENGTH(secureStop_oid),
                                         HANDLER_CAN_RONLY));

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("secureRadaDefaultSessionTime",
                               do_secureRadaDefaultSessionTime,
                               secureRadaDefaultSessionTime_oid,
                               OID_LENGTH
                               (secureRadaDefaultSessionTime_oid),
                               HANDLER_CAN_RWRITE));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("securePortVlanMembershipList",
                                         get_securePortVlanMembershipList,
                                         securePortVlanMembershipList_oid,
                                         OID_LENGTH
                                         (securePortVlanMembershipList_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("secureRadaHoldoffTime",
                               do_secureRadaHoldoffTime,
                               secureRadaHoldoffTime_oid,
                               OID_LENGTH(secureRadaHoldoffTime_oid),
                               HANDLER_CAN_RWRITE));
    netsnmp_register_instance(netsnmp_create_handler_registration
                                        ("secureRadaAuthPassword",
                                         do_secureRadaAuthPassword,
                                         secureRadaAuthPassword_oid,
                                         OID_LENGTH
                                         (secureRadaAuthPassword_oid),
                                         HANDLER_CAN_RWRITE));
    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("secureRadaAuthMode", do_secureRadaAuthMode,
                               secureRadaAuthMode_oid,
                               OID_LENGTH(secureRadaAuthMode_oid),
                               HANDLER_CAN_RWRITE));
    netsnmp_register_instance(netsnmp_create_handler_registration
                                        ("secureRadaReauthenticate",
                                         do_secureRadaReauthenticate,
                                         secureRadaReauthenticate_oid,
                                         OID_LENGTH
                                         (secureRadaReauthenticate_oid),
                                         HANDLER_CAN_RWRITE));
    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("secureRadaAuthUsername",
                               do_secureRadaAuthUsername,
                               secureRadaAuthUsername_oid,
                               OID_LENGTH(secureRadaAuthUsername_oid),
                               HANDLER_CAN_RWRITE));
    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("securePortSecurityControl",
                               do_securePortSecurityControl,
                               securePortSecurityControl_oid,
                               OID_LENGTH(securePortSecurityControl_oid),
                               HANDLER_CAN_RWRITE));
}


oid             securePortTable_variables_oid[] =
    { 1, 3, 6, 1, 4, 1, 43, 10, 22};

/*
 * variable3 securePortTable_variables:
 *   this   defines function callbacks and type return information
 *   for the  mib section
 */
#define SECURESLOTINDEX 1
#define SECUREPORTINDEX 2
#define SECUREPORTMODE 3
#define SECURENEEDTOKNOWMODE 4
#define SECUREINTRUSIONACTION 5
#define SECURENUMBERADDRESSES 6
#define SECURENUMBERADDRESSESSTORED 7
#define SECUREMAXIMUMADDRESSES 8



struct variable3 securePortTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {SECURESLOTINDEX, ASN_INTEGER, RONLY, var_securePortTable, 3,
     {1, 1, 1}},
    {SECUREPORTINDEX, ASN_INTEGER, RONLY, var_securePortTable, 3,
     {1, 1, 2}},
    {SECUREPORTMODE, ASN_INTEGER, RWRITE, var_securePortTable, 3,
     {1, 1, 3}},
    {SECURENEEDTOKNOWMODE, ASN_INTEGER, RWRITE, var_securePortTable, 3,
     {1, 1, 4}},
    {SECUREINTRUSIONACTION, ASN_INTEGER, RWRITE, var_securePortTable, 3,
     {1, 1, 5}},
    {SECURENUMBERADDRESSES, ASN_INTEGER, RWRITE, var_securePortTable, 3,
     {1, 1, 6}},
    {SECURENUMBERADDRESSESSTORED, ASN_INTEGER, RONLY, var_securePortTable,
     3, {1, 1, 7}},
    {SECUREMAXIMUMADDRESSES, ASN_INTEGER, RONLY, var_securePortTable, 3,
     {1, 1, 8}},
};

void
init_securePortTable(void)
{

    DEBUGMSGTL(("securePortTable", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("securePortTable", securePortTable_variables, variable3,
                 securePortTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

oid             secureAddressTable_variables_oid[] =
    { 1, 3, 6, 1, 4, 1, 43, 10, 22};

/*
 * variable3 secureAddressTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
#define SECUREADDRSLOTINDEX 1
#define SECUREADDRPORTINDEX 2
#define SECUREADDRMAC 3
#define SECUREADDRROWSTATUS 4

struct variable3 secureAddressTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {SECUREADDRSLOTINDEX, ASN_INTEGER, RONLY, var_secureAddressTable, 3,
     {2, 1, 1}},
    {SECUREADDRPORTINDEX, ASN_INTEGER, RONLY, var_secureAddressTable, 3,
     {2, 1, 2}},
    {SECUREADDRMAC, ASN_OCTET_STR, RONLY, var_secureAddressTable, 3,
     {2, 1, 3}},
    {SECUREADDRROWSTATUS, ASN_INTEGER, RWRITE, var_secureAddressTable, 3,
     {2, 1, 4}},
};

/** Initializes the secureAddressTable module */
void
init_secureAddressTable(void)
{

    DEBUGMSGTL(("secureAddressTable", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("secureAddressTable", secureAddressTable_variables,
                 variable3, secureAddressTable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

oid             secureOUITable_variables_oid[] =
    { 1, 3, 6, 1, 4, 1, 43, 10, 22 };

/*
 * variable3 secureOUITable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
#define SECUREOUISLOTINDEX 1
#define SECUREOUI 2
#define SECUREOUIROWSTATUS 3

struct variable3 secureOUITable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {SECUREOUISLOTINDEX, ASN_INTEGER, RONLY, var_secureOUITable, 3,
     {5, 1, 1}},
    {SECUREOUI, ASN_OCTET_STR, RONLY, var_secureOUITable, 3, {5, 1, 2}},
    {SECUREOUIROWSTATUS, ASN_INTEGER, RWRITE, var_secureOUITable, 3,
     {5, 1, 3}},
};

/** Initializes the secureOUITable module */
void
init_secureOUITable(void)
{

    DEBUGMSGTL(("secureOUITable", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("secureOUITable", secureOUITable_variables, variable3,
                 secureOUITable_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}


int get_secureStop(netsnmp_mib_handler             *handler,
                   netsnmp_handler_registration    *reginfo,
                   netsnmp_agent_request_info      *reqinfo,
                   netsnmp_request_info            *requests)
{
    switch (reqinfo->mode)
    {
        case MODE_GET:
        {
            UI32_T secure_stop;

            if (!NETWORKACCESS_MGR_GetSecureStopStatus(&secure_stop))
                return SNMP_ERR_GENERR;

            long_return = (UI32_T) secure_stop;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));

            break;
        }

        default:
            /* we should never get here, so this is a really bad error */
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int
do_securePortSecurityControl(netsnmp_mib_handler *handler,
                             netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *reqinfo,
                             netsnmp_request_info *requests)
{
   BOOL_T b_value;
    /*
     * We are never called for a GETNEXT if it's registered as a
     * "instance", as it's "magically" handled for us.
     */

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one.
     */
    switch (reqinfo->mode) {

    case MODE_GET:

        if(NETWORKACCESS_MGR_GetSecurePortSecurityControl(&b_value)==TRUE)
        {
            if(b_value==TRUE)
            {
                long_return = VAL_securePortSecurityControl_enabled;
            }
            else if(b_value==FALSE)
            {
                long_return = VAL_securePortSecurityControl_disabled;
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
        }
      else
        {
                return SNMP_ERR_GENERR;
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
            value = (*requests->requestvb->val.integer);
            if ((value< VAL_securePortSecurityControl_enabled) || (value>VAL_securePortSecurityControl_disabled))
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
        }
            break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
        {
            UI32_T value;
            value = (*requests->requestvb->val.integer);
            if(value == VAL_securePortSecurityControl_enabled)
            {
                b_value=TRUE;
            }
            else if( value == VAL_securePortSecurityControl_disabled)
            {
                b_value=FALSE;
            }
            else
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
            if (NETWORKACCESS_MGR_SetSecurePortSecurityControl(b_value) != TRUE)
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }

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


int
get_securePortVlanMembershipList(netsnmp_mib_handler *handler,
                                 netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo,
                                 netsnmp_request_info *requests)
{
    /*
     * We are never called for a GETNEXT if it's registered as a
     * "instance", as it's "magically" handled for us.
     */

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one.
     */

    switch (reqinfo->mode) {

    case MODE_GET:

     if(NETWORKACCESS_MGR_GetSecurePortVlanMembershipList(return_buf))
     {
         snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) return_buf, strlen(return_buf));
     }
     else
     {
         return SNMP_ERR_GENERR;
     }
     break;


    default:
        /*
         * we should never get here, so this is a really bad error
         */
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
do_secureRadaDefaultSessionTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info *reqinfo,
                          netsnmp_request_info *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

    switch(reqinfo->mode)
    {
        case MODE_GET:
            if( NETWORKACCESS_MGR_GetSecureRadaDefaultSessionTime(&long_return))
            {
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
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
            value = (*requests->requestvb->val.integer);
            if ((value< MIN_secureRadaDefaultSessionTime) || (value>MAX_secureRadaDefaultSessionTime))
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
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
                 if (NETWORKACCESS_MGR_SetSecureRadaDefaultSessionTime(value) != TRUE)
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
do_secureRadaHoldoffTime(netsnmp_mib_handler *handler,
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
            if( NETWORKACCESS_MGR_GetSecureRadaHoldoffTime(&long_return))
            {
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
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
           value = (*requests->requestvb->val.integer);
          if ((value< MIN_secureRadaHoldoffTime) || (value>MAX_secureRadaHoldoffTime))
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
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
            if (NETWORKACCESS_MGR_SetSecureRadaHoldoffTime(value) != TRUE)
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
do_secureRadaReauthenticate(netsnmp_mib_handler *handler,
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
            strcpy(return_buf,"");
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) return_buf, strlen(return_buf));
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            if ((requests->requestvb->val_len!=SIZE_secureRadaReauthenticate) )
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
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
                UI8_T  buf[SIZE_secureRadaReauthenticate+1];
                memcpy(buf, requests->requestvb->val.string, requests->requestvb->val_len);
                buf[requests->requestvb->val_len]= '\0';
                if (NETWORKACCESS_MGR_SetSecureRadaReauthenticate(buf) != TRUE)
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
do_secureRadaAuthMode(netsnmp_mib_handler *handler,
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
            if( NETWORKACCESS_MGR_GetSecureRadaAuthMode(&long_return))
            {
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &long_return, sizeof(long_return));
            }
            else
            {
                return SNMP_ERR_GENERR;
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
           value = (*requests->requestvb->val.integer);
            switch(value)
            {
                case VAL_secureRadaAuthMode_papUsernameAsMacAddress:
                case VAL_secureRadaAuthMode_papUsernameFixed:
                   break;
                default:
                    netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
                   break;
            }
            break;
        }
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
            if (NETWORKACCESS_MGR_SetSecureRadaAuthMode(value) != TRUE)
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
do_secureRadaAuthUsername(netsnmp_mib_handler *handler,
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
            if(NETWORKACCESS_MGR_GetSecureRadaAuthUsername(return_buf))
            {
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) return_buf, strlen(return_buf));
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            if ((requests->requestvb->val_len<MINSIZE_secureRadaAuthUsername)||(requests->requestvb->val_len>MAXSIZE_secureRadaAuthUsername) )
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
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
                UI8_T  buf[MAXSIZE_secureRadaAuthUsername+1];
                memcpy(buf, requests->requestvb->val.string, requests->requestvb->val_len);
                buf[requests->requestvb->val_len]= '\0';
                if (NETWORKACCESS_MGR_SetSecureRadaAuthUsername(buf) != TRUE)
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
do_secureRadaAuthPassword(netsnmp_mib_handler *handler,
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
            strcpy(return_buf,"");
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) return_buf, strlen(return_buf));
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            if ((requests->requestvb->val_len<MINSIZE_secureRadaAuthPassword)||(requests->requestvb->val_len>MAXSIZE_secureRadaAuthPassword) )
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_INCONSISTENTVALUE);
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
                UI8_T  buf[MAXSIZE_secureRadaAuthPassword+1];
                memcpy(buf, requests->requestvb->val.string, requests->requestvb->val_len);
                buf[requests->requestvb->val_len]= '\0';
                if (NETWORKACCESS_MGR_SetSecureRadaAuthPassword(buf) != TRUE)
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

#define securePortEntry_INSTANCE_LEN  2
static BOOL_T securePortTable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                NETWORKACCESS_SecurePortEntry_T   *data)
{
    UI32_T slot_index;
    UI32_T port_index;

    if (compc !=securePortEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    slot_index = (UI32_T)compl[0];
    port_index = (UI32_T)compl[1];
    if (NETWORKACCESS_MGR_GetSecurePortEntry(slot_index,port_index ,data)!=TRUE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T securePortTable_next(int      compc,
                                 oid     *compl,
                                 UI32_T  *index,
                                 NETWORKACCESS_SecurePortEntry_T    *data)
{
    oid tmp_compl[securePortEntry_INSTANCE_LEN];
    UI32_T slot_index;
    UI32_T port_index;
    /* Generate the instance of each table entry and find the
    * smallest instance that's larger than compc/compl.
    *
    * Step 1: Verify and extract the input key from "compc" and "compl"
    * Note: The number of input key is defined by "compc".
    *       The key for the specified instance is defined in compl.
    */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,securePortEntry_INSTANCE_LEN, tmp_compl);

    slot_index = (UI32_T)tmp_compl[0];
    port_index = (UI32_T)tmp_compl[1];
    if (compc<securePortEntry_INSTANCE_LEN)
    {
        if (NETWORKACCESS_MGR_GetSecurePortEntry(slot_index, port_index, data)!=TRUE)
        {
            if (NETWORKACCESS_MGR_GetNextSecurePortEntry( &slot_index, &port_index, data)!=TRUE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (NETWORKACCESS_MGR_GetNextSecurePortEntry( &slot_index,&port_index, data)!=TRUE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

unsigned char  *
var_securePortTable(struct variable *vp,
                    oid * name,
                    size_t * length,
                    int exact,
                    size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later
     */
    UI32_T compc=0;
    UI32_T idx1;
    oid compl[securePortEntry_INSTANCE_LEN];
    oid best_inst[securePortEntry_INSTANCE_LEN];
    NETWORKACCESS_SecurePortEntry_T  data;
    switch (vp->magic) {
      case SECURESLOTINDEX:
        *write_method = 0;
        break;
      case SECUREPORTINDEX:
        *write_method = 0;
        break;
      case SECUREPORTMODE:
        *write_method = write_securePortMode;
        break;
      case SECURENEEDTOKNOWMODE:
        *write_method = write_secureNeedToKnowMode;
        break;
      case SECUREINTRUSIONACTION:
        *write_method = write_secureIntrusionAction;
        break;
      case SECURENUMBERADDRESSES:
        *write_method = write_secureNumberAddresses;
        break;
      case SECURENUMBERADDRESSESSTORED:
       *write_method = 0;
        break;
      case SECUREMAXIMUMADDRESSES:
       *write_method = 0;
        break;
    default:
        *write_method = 0;
        break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, securePortEntry_INSTANCE_LEN);

     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!securePortTable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!securePortTable_next(compc, compl, &idx1, &data))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0]=data.slot_index;
    best_inst[1]=data.port_index;
    memcpy(name + vp->namelen, best_inst, securePortEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +securePortEntry_INSTANCE_LEN;

    /* default give len = 4, should need to modify in switch case if not equal to 4*/
    *var_len = 4;


    switch (vp->magic) {
      case SECURESLOTINDEX:
        long_return = data.slot_index;
        return (u_char*) &long_return;
      case SECUREPORTINDEX:
        long_return = data.port_index;
        return (u_char*) &long_return;
      case SECUREPORTMODE:
        long_return = data.port_mode;
        return (u_char*) &long_return;
     case SECURENEEDTOKNOWMODE:
        long_return = data.need_to_know_mode;
        return (u_char*) &long_return;
      case SECUREINTRUSIONACTION:
        long_return = data.intrusion_action;
        return (u_char*) &long_return;
      case SECURENUMBERADDRESSES:
        long_return = data.number_addresses;
        return (u_char*) &long_return;
      case SECURENUMBERADDRESSESSTORED:
        long_return = data.number_addresses_stored;
        return (u_char*) &long_return;
      case SECUREMAXIMUMADDRESSES:
        long_return = data.maximum_addresses;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

int
write_securePortMode(int action,
                     u_char * var_val,
                     u_char var_val_type,
                     size_t var_val_len,
                     u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  securePortEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_securePortMode_noRestrictions:
            case VAL_securePortMode_continuousLearning:
            case VAL_securePortMode_autoLearn:
            case VAL_securePortMode_secure:
            case VAL_securePortMode_userLogin:
            case VAL_securePortMode_userLoginSecure:
            case VAL_securePortMode_userLoginWithOUI:
            case VAL_securePortMode_macAddressWithRadius:
            case VAL_securePortMode_macAddressOrUserLoginSecure:
            case VAL_securePortMode_macAddressElseUserLoginSecure:
                break;
            default:
               return SNMP_ERR_WRONGVALUE;
        }
                break;
    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T unit;
        UI32_T port;
        unit = (UI32_T)name[oid_name_length];
        port = (UI32_T)name[oid_name_length+1];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecurePortMode( unit, port,value  )!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;
    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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

int
write_secureNeedToKnowMode(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  securePortEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_secureNeedToKnowMode_notAvailable:
            case VAL_secureNeedToKnowMode_disabled:
            case VAL_secureNeedToKnowMode_needToKnowOnly:
            case VAL_secureNeedToKnowMode_needToKnowWithBroadcastsAllowed:
            case VAL_secureNeedToKnowMode_needToKnowWithMulticastsAllowed:
            case VAL_secureNeedToKnowMode_permanentNeedToKnowOnly:
            case VAL_secureNeedToKnowMode_permanentNeedToKnowWithBroadcastsAllowed:
            case VAL_secureNeedToKnowMode_permanentNeedToKnowWithMulticastsAllowed:
                break;
            default:
                return SNMP_ERR_WRONGVALUE;
        }
        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T unit;
        UI32_T port;
        unit = (UI32_T)name[oid_name_length];
        port = (UI32_T)name[oid_name_length+1];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecureNeedToKnowMode( unit, port,value  )!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;
    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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

int
write_secureIntrusionAction(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  securePortEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_secureIntrusionAction_notAvailable:
            case VAL_secureIntrusionAction_noAction:
            case VAL_secureIntrusionAction_disablePort:
            case VAL_secureIntrusionAction_disablePortTemporarily:
            case VAL_secureIntrusionAction_allowDefaultAccess:
            case VAL_secureIntrusionAction_blockMacAddress:
                break;
            default:
                return SNMP_ERR_WRONGVALUE;
        }
        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T unit;
        UI32_T port;
        unit = (UI32_T)name[oid_name_length];
        port = (UI32_T)name[oid_name_length+1];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecureIntrusionAction(unit, port,value)!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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

int
write_secureNumberAddresses(int action,
                            u_char * var_val,
                            u_char var_val_type,
                            size_t var_val_len,
                            u_char * statP, oid * name, size_t name_len)
{
    long value;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  securePortEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T unit;
        UI32_T port;
        unit = (UI32_T)name[oid_name_length];
        port = (UI32_T)name[oid_name_length+1];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecureNumberAddresses(unit, port,value)!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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

#define secureAddressEntry_INSTANCE_LEN 8
static BOOL_T secureAddressTable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                NETWORKACCESS_SecureAddressEntry_T   *data)
{
    UI8_T addr_MAC[SIZE_secureAddrMAC];
    UI32_T slot_index;
    UI32_T port_index;

    if (compc !=secureAddressEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    slot_index = (UI32_T)compl[0];
    port_index = (UI32_T)compl[1];
    addr_MAC[0]= (UI8_T)compl[2];
    addr_MAC[1]= (UI8_T)compl[3];
    addr_MAC[2]= (UI8_T)compl[4];
    addr_MAC[3]= (UI8_T)compl[5];
    addr_MAC[4]= (UI8_T)compl[6];
    addr_MAC[5]= (UI8_T)compl[7];

    if (NETWORKACCESS_MGR_GetSecureAddressEntry(slot_index,port_index ,addr_MAC,data)!=TRUE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T ssecureAddressTable_next(int      compc,
                                 oid     *compl,
                                 UI32_T  *index,
                                 NETWORKACCESS_SecureAddressEntry_T    *data)
{
    oid tmp_compl[secureAddressEntry_INSTANCE_LEN];
    UI8_T addr_MAC[SIZE_secureAddrMAC];
    UI32_T slot_index;
    UI32_T port_index;

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,secureAddressEntry_INSTANCE_LEN, tmp_compl);

    slot_index = (UI32_T)compl[0];
    port_index = (UI32_T)compl[1];
    addr_MAC[0]= (UI8_T)compl[2];
    addr_MAC[1]= (UI8_T)compl[3];
    addr_MAC[2]= (UI8_T)compl[4];
    addr_MAC[3]= (UI8_T)compl[5];
    addr_MAC[4]= (UI8_T)compl[6];
    addr_MAC[5]= (UI8_T)compl[7];

    if (compc<secureAddressEntry_INSTANCE_LEN)
    {
        if (NETWORKACCESS_MGR_GetSecureAddressEntry(slot_index, port_index,addr_MAC, data)!=TRUE)
        {
            if (NETWORKACCESS_MGR_GetNextSecureAddressEntry(&slot_index,&port_index, addr_MAC,data)!=TRUE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (NETWORKACCESS_MGR_GetNextSecureAddressEntry( &slot_index,&port_index, addr_MAC,data)!=TRUE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

unsigned char  *
var_secureAddressTable(struct variable *vp,
                       oid * name,
                       size_t * length,
                       int exact,
                       size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later
     */
    UI32_T compc=0;
    UI32_T idx1;
    oid compl[secureAddressEntry_INSTANCE_LEN];
    oid best_inst[secureAddressEntry_INSTANCE_LEN];
    NETWORKACCESS_SecureAddressEntry_T  data;

    switch (vp->magic) {
      case SECUREADDRSLOTINDEX:
        *write_method = 0;
        break;
      case SECUREADDRPORTINDEX:
        *write_method = 0;
        break;
      case SECUREADDRMAC:
        *write_method = 0;
        break;
      case SECUREADDRROWSTATUS:
        *write_method = write_secureAddrRowStatus;
        break;
    default:
        *write_method = 0;
        break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, secureAddressEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!secureAddressTable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!ssecureAddressTable_next(compc, compl, &idx1, &data))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    best_inst[0]=data.addr_slot_index;
    best_inst[1]=data.addr_port_index;
    best_inst[2]=data.addr_MAC[0];
    best_inst[3]=data.addr_MAC[1];
    best_inst[4]=data.addr_MAC[2];
    best_inst[5]=data.addr_MAC[3];
    best_inst[6]=data.addr_MAC[4];
    best_inst[7]=data.addr_MAC[5];

    memcpy(name + vp->namelen, best_inst, secureAddressEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +secureAddressEntry_INSTANCE_LEN;

    /* default give len = 4, should need to modify in switch case if not equal to 4*/
    *var_len = 4;


    /*
     * * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
      case SECUREADDRSLOTINDEX:
        long_return = data.addr_slot_index;
        return (u_char*) &long_return;
      case SECUREADDRPORTINDEX:
        long_return = data.addr_port_index;
        return (u_char*) &long_return;
      case SECUREADDRMAC:
      {
        memcpy( return_buf,data.addr_MAC, SIZE_secureAddrMAC);
        *var_len = SIZE_secureAddrMAC;
        return (u_char*) return_buf;
      }
      case SECUREADDRROWSTATUS:
        long_return = data.addr_row_status;
        return (u_char*) &long_return;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


int
write_secureAddrRowStatus(int action,
                          u_char * var_val,
                          u_char var_val_type,
                          size_t var_val_len,
                          u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  secureAddressEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_secureAddrRowStatus_active:
            case VAL_secureAddrRowStatus_notInService:
            case VAL_secureAddrRowStatus_notReady:
            case VAL_secureAddrRowStatus_createAndGo:
            case VAL_secureAddrRowStatus_createAndWait:
            case VAL_secureAddrRowStatus_destroy:
                break;
            default:
                return SNMP_ERR_WRONGVALUE;
        }
        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T unit;
        UI32_T port;
        UI8_T addr_MAC[SIZE_secureAddrMAC];
        unit = (UI32_T)name[oid_name_length];
        port = (UI32_T)name[oid_name_length+1];
        addr_MAC[0]= (UI8_T)name[oid_name_length+2];
        addr_MAC[1]= (UI8_T)name[oid_name_length+3];
        addr_MAC[2]= (UI8_T)name[oid_name_length+4];
        addr_MAC[3]= (UI8_T)name[oid_name_length+5];
        addr_MAC[4]= (UI8_T)name[oid_name_length+6];
        addr_MAC[5]= (UI8_T)name[oid_name_length+7];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecureAddrRowStatus(unit, port,addr_MAC,value)!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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

#define secureOUIEntry_INSTANCE_LEN 4

static BOOL_T secureOUITable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                NETWORKACCESS_SecureOUIEntry_T   *data)
{
    oid tmp_compl[secureOUIEntry_INSTANCE_LEN];
    UI32_T secureOUISlotIndex;
    UI8_T secureOUI[SIZE_secureOUI];

    if (compc !=secureOUIEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    secureOUISlotIndex = (UI32_T)tmp_compl[0];
    secureOUI[0] = (UI8_T)tmp_compl[1];
    secureOUI[1] = (UI8_T)tmp_compl[2];
    secureOUI[2] = (UI8_T)tmp_compl[3];
    if (NETWORKACCESS_MGR_GetSecureOUIEntry(secureOUISlotIndex,secureOUI,data)!=TRUE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T secureOUITable_next(int      compc,
                                 oid     *compl,
                                 UI32_T  *index,
                                 NETWORKACCESS_SecureOUIEntry_T    *data)
{
    oid tmp_compl[secureOUIEntry_INSTANCE_LEN];
    UI32_T secureOUISlotIndex;
    UI8_T secureOUI[SIZE_secureOUI];
    /* Generate the instance of each table entry and find the
    * smallest instance that's larger than compc/compl.
    *
    * Step 1: Verify and extract the input key from "compc" and "compl"
    * Note: The number of input key is defined by "compc".
    *       The key for the specified instance is defined in compl.
    */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,secureOUIEntry_INSTANCE_LEN, tmp_compl);

    secureOUISlotIndex = (UI32_T)tmp_compl[0];
    secureOUI[0] = (UI8_T)tmp_compl[1];
    secureOUI[1] = (UI8_T)tmp_compl[2];
    secureOUI[2] = (UI8_T)tmp_compl[3];


    if (compc<secureOUIEntry_INSTANCE_LEN)
    {
        if (NETWORKACCESS_MGR_GetSecureOUIEntry(secureOUISlotIndex, secureOUI, data)!=TRUE)
        {
            if (NETWORKACCESS_MGR_GetNextSecureOUIEntry( secureOUISlotIndex, secureOUI, data)!=TRUE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (NETWORKACCESS_MGR_GetNextSecureOUIEntry( secureOUISlotIndex,secureOUI, data)!=TRUE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

unsigned char  *
var_secureOUITable(struct variable *vp,
                   oid * name,
                   size_t * length,
                   int exact,
                   size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later
     */
    UI32_T compc=0;
    UI32_T idx1;
    oid compl[secureOUIEntry_INSTANCE_LEN];
    oid best_inst[secureOUIEntry_INSTANCE_LEN];
    NETWORKACCESS_SecureOUIEntry_T  data;

    switch (vp->magic) {
      case SECUREOUISLOTINDEX:
        *write_method = 0;
        break;
      case SECUREOUI:
        *write_method = 0;
        break;
      case SECUREOUIROWSTATUS:
        *write_method = write_secureOUIRowStatus;
        break;
    default:
        *write_method = 0;
        break;
    }


    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, secureOUIEntry_INSTANCE_LEN);
     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!secureOUITable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!secureOUITable_next(compc, compl, &idx1, &data))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    best_inst[0]=data.secureOUISlotIndex;
    best_inst[1]=data.secureOUI[0];
    best_inst[3]=data.secureOUI[1];
    best_inst[4]=data.secureOUI[2];


    memcpy(name + vp->namelen, best_inst, secureOUIEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +secureOUIEntry_INSTANCE_LEN;

    /* default give len = 4, should need to modify in switch case if not equal to 4*/
    *var_len = 4;

    switch (vp->magic) {
        case SECUREOUISLOTINDEX:
          long_return = data.secureOUISlotIndex;
           return (u_char*) &long_return;
        case SECUREOUI:
        {
            memcpy( return_buf,data.secureOUI, SIZE_secureOUI);
            *var_len = SIZE_secureOUI;
            return (u_char*) return_buf;
        }
        case SECUREOUIROWSTATUS:
           long_return = data.secureOUIRowStatus;
           return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }
    return NULL;
}


int
write_secureOUIRowStatus(int action,
                         u_char * var_val,
                         u_char var_val_type,
                         size_t var_val_len,
                         u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = securePortOidNameLength +3;

    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  secureOUIEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch (action) {
    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_secureOUIRowStatus_active:
            case VAL_secureOUIRowStatus_notInService:
            case VAL_secureOUIRowStatus_notReady:
            case VAL_secureOUIRowStatus_createAndGo:
            case VAL_secureOUIRowStatus_createAndWait:
            case VAL_secureOUIRowStatus_destroy:
                break;
            default:
                return SNMP_ERR_WRONGVALUE;
        }
        break;

    case FREE:
        /*
         * Release any resources that have been allocated
         */
        break;

    case ACTION:
    {
        UI32_T secureOUISlotIndex;
        UI8_T secureOUI[SIZE_secureOUI];
        secureOUISlotIndex = (UI32_T)name[oid_name_length];
        secureOUI[0] = (UI8_T)name[oid_name_length+1];
        secureOUI[1] = (UI8_T)name[oid_name_length+2];
        secureOUI[2] = (UI8_T)name[oid_name_length+3];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetSecureOUIRowStatus(secureOUISlotIndex, secureOUI,value)!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
    }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
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
#endif
