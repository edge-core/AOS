#include "sys_cpnt.h"
#if (SYS_CPNT_LLDP == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "l_stdlib.h"
#include "l_cvrt.h"
#include "ieee_lldp.h"
#include "leaf_ieeelldp.h"
#include "lldp_type.h"
#include "lldp_mgr.h"
#include "snmp_mgr.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"

/* an array of bit masks in SNMP bit order (*MSBit first) */
static char bitstring_mask[] = { 1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1<<0 };
static oid null_oid[] = { 0, 0 };

void
lldp_bitstring_setbit(char *bstring, int number)
{
        bstring += number / 8;
        *bstring |= bitstring_mask[(number % 8)];
}

void
lldp_bitstring_clearbit(char *bstring, int number)
{
        bstring += number / 8;
        *bstring &= ~bitstring_mask[(number % 8)];
}
int
lldp_bitstring_testbit(char *bstring, int number)
{
        bstring += number / 8;
        return ((*bstring & bitstring_mask[(number % 8)]) != 0);
}
/** Initializes the lldpConfiguration module */
void
init_lldpConfiguration(void)
{
    static oid lldpMessageTxInterval_oid[] = { 1,0,8802,1,1,2,1,1,1,0 };
    static oid lldpMessageTxHoldMultiplier_oid[] = { 1,0,8802,1,1,2,1,1,2,0 };
    static oid lldpReinitDelay_oid[] = { 1,0,8802,1,1,2,1,1,3,0 };
    static oid lldpTxDelay_oid[] = { 1,0,8802,1,1,2,1,1,4,0 };
    static oid lldpNotificationInterval_oid[] = { 1,0,8802,1,1,2,1,1,5,0 };

  DEBUGMSGTL(("lldpConfiguration", "Initializing\n"));

  netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpMessageTxInterval",
                               do_lldpMessageTxInterval,
                               lldpMessageTxInterval_oid,
                               OID_LENGTH(lldpMessageTxInterval_oid),
                               HANDLER_CAN_RWRITE));
  netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpMessageTxHoldMultiplier",
                               do_lldpMessageTxHoldMultiplier,
                               lldpMessageTxHoldMultiplier_oid,
                               OID_LENGTH(lldpMessageTxHoldMultiplier_oid),
                               HANDLER_CAN_RWRITE));
  netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpReinitDelay",
                               do_lldpReinitDelay,
                               lldpReinitDelay_oid,
                               OID_LENGTH(lldpReinitDelay_oid),
                               HANDLER_CAN_RWRITE));
  netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpTxDelay",
                               do_lldpTxDelay,
                               lldpTxDelay_oid,
                               OID_LENGTH(lldpTxDelay_oid),
                               HANDLER_CAN_RWRITE));
  netsnmp_register_instance(netsnmp_create_handler_registration
                              ("lldpNotificationInterval",
                               do_lldpNotificationInterval,
                               lldpNotificationInterval_oid,
                               OID_LENGTH(lldpNotificationInterval_oid),
                               HANDLER_CAN_RWRITE));
}

int
do_lldpMessageTxInterval(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_SysConfigEntry_T config_entry;

            if (LLDP_POM_GetSysConfigEntry(&config_entry)==TRUE)
            {
                long_return = config_entry.message_tx_interval;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
            }

            break;
        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer< MIN_lldpMessageTxInterval ||*requests->requestvb->val.integer > MAX_lldpMessageTxInterval)
            {
               netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetMsgTxInterval(*requests->requestvb->val.integer) != LLDP_TYPE_RETURN_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
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
do_lldpMessageTxHoldMultiplier(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_SysConfigEntry_T config_entry;

            if (LLDP_POM_GetSysConfigEntry(&config_entry)==TRUE)
            {
                long_return = config_entry.message_tx_hold_multiplier;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
            }

            break;
        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer< MIN_lldpMessageTxHoldMultiplier ||*requests->requestvb->val.integer > MAX_lldpMessageTxHoldMultiplier)
            {
               netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetMsgTxHoldMul(*requests->requestvb->val.integer) != LLDP_TYPE_RETURN_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
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
do_lldpReinitDelay(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_SysConfigEntry_T config_entry;

            if (LLDP_POM_GetSysConfigEntry(&config_entry)==TRUE)
            {
                long_return = config_entry.reinit_delay;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
            }

            break;
        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer< MIN_lldpReinitDelay ||*requests->requestvb->val.integer > MAX_lldpReinitDelay)
            {
               netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetReinitDelay(*requests->requestvb->val.integer) != LLDP_TYPE_RETURN_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
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
do_lldpTxDelay(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_SysConfigEntry_T config_entry;

            if (LLDP_POM_GetSysConfigEntry(&config_entry)==TRUE)
            {
                long_return = config_entry.tx_delay;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
            }

            break;
        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer< MIN_lldpTxDelay ||*requests->requestvb->val.integer > MAX_lldpTxDelay)
            {
               netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetTxDelay(*requests->requestvb->val.integer) != LLDP_TYPE_RETURN_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
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
do_lldpNotificationInterval(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_SysConfigEntry_T config_entry;

            if (LLDP_POM_GetSysConfigEntry(&config_entry)==TRUE)
            {
                long_return = config_entry.notification_interval;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:

            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
            }

            break;
        case MODE_SET_RESERVE2:

            if (*requests->requestvb->val.integer< MIN_lldpNotificationInterval ||*requests->requestvb->val.integer > MAX_lldpNotificationInterval)
            {
               netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGVALUE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            if (LLDP_PMGR_SetNotificationInterval(*requests->requestvb->val.integer) != LLDP_TYPE_RETURN_OK)
            {
                netsnmp_set_request_error(reqinfo, requests,  SNMP_ERR_COMMITFAILED);
            }
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


/*
 * lldpPortConfigTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpPortConfigTable_variables_oid[] = { 1,0,8802,1,1,2,1,1 };

/*
 * variable3 lldpPortConfigTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 lldpPortConfigTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
/*    (L = length of the oidsuffix) */
    #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    {LLDPPORTCONFIGPORTNUM,  ASN_INTEGER,  RONLY,   var_lldpPortConfigTable, 3,  { 6, 1, 1 }},
    #endif
    {LLDPPORTCONFIGADMINSTATUS,  ASN_INTEGER,  RWRITE,  var_lldpPortConfigTable, 3,  { 6, 1, 2 }},
    {LLDPPORTCONFIGNOTIFICATIONENABLE,  ASN_INTEGER,  RWRITE,  var_lldpPortConfigTable, 3,  { 6, 1, 3 }},
    {LLDPPORTCONFIGTLVSTXENABLE,  ASN_OCTET_STR,  RWRITE,  var_lldpPortConfigTable, 3,  { 6, 1, 4 }},
};

/** Initializes the lldpPortConfigTable module */
void
init_lldpPortConfigTable(void)
{

    DEBUGMSGTL(("lldpPortConfigTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpPortConfigTable", lldpPortConfigTable_variables, variable3,
               lldpPortConfigTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPPORTCONFIGENTRY_INSTANCE_LEN  1

BOOL_T lldpPortConfigTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *port_num)
{

    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPPORTCONFIGENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *port_num=compl[0];
    return TRUE;
}

/*
 * var_lldpPortConfigTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpPortConfigTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPPORTCONFIGENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPPORTCONFIGENTRY_INSTANCE_LEN];
    LLDP_MGR_PortConfigEntry_T  entry;
    switch(vp->magic)
    {
    case LLDPPORTCONFIGADMINSTATUS:
        *write_method = write_lldpPortConfigAdminStatus;
        break;
    case LLDPPORTCONFIGNOTIFICATIONENABLE:
        *write_method = write_lldpPortConfigNotificationEnable;
        break;
    case LLDPPORTCONFIGTLVSTXENABLE:
        *write_method = write_lldpPortConfigTLVsTxEnable;
        break;
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPPORTCONFIGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpPortConfigTable_OidIndexToData(exact,compc,compl, &entry.port_num)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetPortConfigEntry(&entry) != LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpPortConfigTable_OidIndexToData(exact,compc,compl, &entry.port_num);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPPORTCONFIGENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetPortConfigEntry(&entry) != LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextPortConfigEntry(&entry) != LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextPortConfigEntry(&entry) != LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.port_num;
    memcpy(name+vp->namelen,best_inst,LLDPPORTCONFIGENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPPORTCONFIGENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LLDPPORTCONFIGPORTNUM:
        *var_len = sizeof(long_return);
        long_return = entry.port_num;
        return (u_char *) &long_return;
#endif
    case LLDPPORTCONFIGADMINSTATUS:
        *var_len = sizeof(long_return);
        long_return = entry.admin_status;
        return (u_char *) &long_return;
    case LLDPPORTCONFIGNOTIFICATIONENABLE:
        *var_len = sizeof(long_return);
        long_return = entry.notification_enable;
        return (u_char *) &long_return;
    case LLDPPORTCONFIGTLVSTXENABLE:
        *var_len = SIZE_lldpPortConfigTLVsTxEnable;
        SNMP_MGR_BitsFromCoreToSnmp(&entry.basic_tlvs_tx_flag, return_buf, *var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }

    return NULL;
}

int
write_lldpPortConfigAdminStatus(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpPortConfigAdminStatus_txOnly:
                    break;
                case VAL_lldpPortConfigAdminStatus_rxOnly:
                    break;
                case VAL_lldpPortConfigAdminStatus_txAndRx:
                    break;
                case VAL_lldpPortConfigAdminStatus_disabled:
                    break;
                default:
                    return SNMP_ERR_WRONGVALUE;
            }
          break;

        case FREE:
          break;

        case ACTION:
        {
            UI32_T oid_name_length = 11;
            I32_T value=0;
            UI32_T port_num = 0;

            if(lldpPortConfigTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &port_num)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetPortConfigAdminStatus( port_num, value) != LLDP_TYPE_RETURN_OK)
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
write_lldpPortConfigNotificationEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:

          if (var_val_type != ASN_INTEGER)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          break;

        case RESERVE2:
            switch ( *(long *)var_val )
            {
                case VAL_lldpPortConfigNotificationEnable_true:
                    break;
                case VAL_lldpPortConfigNotificationEnable_false:
                    break;
                default:
                    return SNMP_ERR_WRONGVALUE;
            }
          break;

        case FREE:
          break;

        case ACTION:
        {
            UI32_T oid_name_length = 11;
            UI32_T value=0;
            UI32_T port_num = 0;

            if(lldpPortConfigTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &port_num)==FALSE)
                return SNMP_ERR_COMMITFAILED;
            value = *(long *)var_val;
            if(LLDP_PMGR_SetPortConfigNotificationEnable( port_num,(BOOL_T) value) != LLDP_TYPE_RETURN_OK )
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
write_lldpPortConfigTLVsTxEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != SIZE_lldpPortConfigTLVsTxEnable)
            {
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*var_val & 0x0f)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 11;
            UI8_T byte_buffer[SIZE_lldpPortConfigTLVsTxEnable]={0};
            UI32_T port_num = 0;

            if(lldpPortConfigTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &port_num)==FALSE)
                return SNMP_ERR_COMMITFAILED;

            SNMP_MGR_BitsFromSnmpToCore(byte_buffer, var_val, var_val_len);
            if(LLDP_PMGR_SetPortOptionalTlvStatus( port_num, *(UI8_T *)byte_buffer) != LLDP_TYPE_RETURN_OK )
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

/*
 * lldpConfigManAddrTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid lldpConfigManAddrTable_variables_oid[] = {1,0,8802,1,1,2,1,1,7};

/*
 * variable4 lldpPortConfigTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable4 lldpManConfiguration_variables[] =
{
    {LEAF_lldpConfigManAddrPortsTxEnable, ASN_OCTET_STR,   RWRITE, var_lldpConfigManAddrTable, 2, {1,1}},
};

/** Initializes the lldpConfigManAddrTable module */
void
init_lldpConfigManAddrTable(void)
{

    DEBUGMSGTL(("lldpConfigManAddrTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpConfigManAddrTable", lldpManConfiguration_variables, variable4,
               lldpConfigManAddrTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPCONFIGMANADDRENTRY_INSTANCE_LEN  33

BOOL_T lldpConfigManAddrTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI8_T *loc_man_addr_subtype, UI8_T *loc_man_addr,UI32_T *loc_man_addr_len)
{
    UI32_T i;

    /* get or write
     */
    if(exact)
    {
        /* check the index length: variable-length index
         *
         * compc < 3, means the "address content" is empty
         */
        if ((compc < 3) || (compc > LLDPCONFIGMANADDRENTRY_INSTANCE_LEN))
        {
            return FALSE;
        }
    }

    *loc_man_addr_subtype=(UI8_T)compl[0];

    if((compl[1]< MINSIZE_lldpLocManAddr) || (compl[1]> MAXSIZE_lldpLocManAddr ))
    {
        return FALSE;
    }

    for( i=0;i< compl[1] ;i++)
    {
        loc_man_addr[i]=(UI8_T)compl[2+i];
    }

    *loc_man_addr_len = compl[1];

    return TRUE;
}

#define LLDPCONFIGMANADDRPORTSENABLE     10

unsigned char *
var_lldpConfigManAddrTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T i;
    UI32_T compc=0;
    oid compl[LLDPCONFIGMANADDRPORTSENABLE]={0};
    oid best_inst[LLDPCONFIGMANADDRPORTSENABLE];
    LLDP_MGR_LocalManagementAddrEntry_T  entry;
    switch(vp->magic)
    {
    case LEAF_lldpConfigManAddrPortsTxEnable:
        *write_method = write_lldpConfigManAddrTable;
        break;
    default:
        *write_method = 0;
        break;
    }
    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPCONFIGMANADDRPORTSENABLE);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        lldpConfigManAddrTable_OidIndexToData(exact,compc,compl, &entry.loc_man_addr_subtype, entry.loc_man_addr, &entry.loc_man_addr_len);

        /*get data
         */
        if (LLDP_PMGR_GetLocalManagementAddress(&entry) != LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpConfigManAddrTable_OidIndexToData(exact,compc,compl, &entry.loc_man_addr_subtype, entry.loc_man_addr, &entry.loc_man_addr_len);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {
            if ( LLDP_PMGR_GetLocalManagementAddress(&entry) != LLDP_TYPE_RETURN_OK)
            {
                if ( LLDP_PMGR_GetNextLocalManagementAddress(&entry) != LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {
            if ( LLDP_PMGR_GetNextLocalManagementAddress(&entry) != LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));

    /* assign data to the oid index
     */
    best_inst[0] = entry.loc_man_addr_subtype;
    best_inst[1] = entry.loc_man_addr_len;

    for( i=0;i< entry.loc_man_addr_len ;i++)
    {
        best_inst[2+i]=entry.loc_man_addr[i];
    }
    memcpy(name+vp->namelen,best_inst,(2+best_inst[1])*sizeof(oid));
    *length = vp->namelen+2+best_inst[1];

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

    case LEAF_lldpConfigManAddrPortsTxEnable:
        *var_len = SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;
        memcpy(return_buf, entry.ports_tx_enable,*var_len);
        return (u_char*)return_buf;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

int
write_lldpConfigManAddrTable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{

    switch ( action ) {

        case RESERVE1:
          if (var_val_type != ASN_OCTET_STR)
          {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len != SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST)
          {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          break;

        case FREE:
          break;

        case ACTION:
        {
            UI32_T oid_name_length = 11;
            UI8_T byte_buffer[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};
            UI8_T  set_ports_tx_enable[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
            LLDP_MGR_LocalManagementAddrEntry_T  entry;
            int i;

	    memset(&entry,0,sizeof(LLDP_MGR_LocalManagementAddrEntry_T));
            lldpConfigManAddrTable_OidIndexToData(TRUE,name_len-oid_name_length,&(name[oid_name_length]), &entry.loc_man_addr_subtype, entry.loc_man_addr, &entry.loc_man_addr_len);
            if (LLDP_PMGR_GetLocalManagementAddress(&entry) != LLDP_TYPE_RETURN_OK)
            {
                return SNMP_ERR_GENERR;
            }
            memcpy(byte_buffer,var_val,var_val_len);
            for(i=1;i<=SYS_ADPT_TOTAL_NBR_OF_LPORT;i++)
            {

                if(!(lldp_bitstring_testbit((char*)entry.ports_tx_enable,(i-1)))&&lldp_bitstring_testbit((char*)byte_buffer,(i-1)))
                {
                    lldp_bitstring_setbit((char*)set_ports_tx_enable,(i-1));
                }
                else if(lldp_bitstring_testbit((char*)entry.ports_tx_enable,(i-1))&&!(lldp_bitstring_testbit((char*)byte_buffer,(i-1))))
                {
                    lldp_bitstring_clearbit((char*)set_ports_tx_enable,(i-1));
                }
            }
            if (LLDP_PMGR_SetConfigAllPortManAddrTlv(set_ports_tx_enable) != LLDP_TYPE_RETURN_OK)
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;

        case UNDO:
          break;

        case COMMIT:
          break;
    }
    return SNMP_ERR_NOERROR;
}
/** Initializes the lldpStatistics module */
void
init_lldpStatistics(void)
{
    static oid lldpStatsRemTablesLastChangeTime_oid[] = { 1,0,8802,1,1,2,1,2,1,0 };
    static oid lldpStatsRemTablesInserts_oid[] = { 1,0,8802,1,1,2,1,2,2,0 };
    static oid lldpStatsRemTablesDeletes_oid[] = { 1,0,8802,1,1,2,1,2,3,0 };
    static oid lldpStatsRemTablesDrops_oid[] = { 1,0,8802,1,1,2,1,2,4,0 };
    static oid lldpStatsRemTablesAgeouts_oid[] = { 1,0,8802,1,1,2,1,2,5,0 };

  DEBUGMSGTL(("lldpStatistics", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpStatsRemTablesLastChangeTime",
                                         get_lldpStatsRemTablesLastChangeTime,
                                         lldpStatsRemTablesLastChangeTime_oid,
                                         OID_LENGTH(lldpStatsRemTablesLastChangeTime_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpStatsRemTablesInserts",
                                         get_lldpStatsRemTablesInserts,
                                         lldpStatsRemTablesInserts_oid,
                                         OID_LENGTH(lldpStatsRemTablesInserts_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpStatsRemTablesDeletes",
                                         get_lldpStatsRemTablesDeletes,
                                         lldpStatsRemTablesDeletes_oid,
                                         OID_LENGTH(lldpStatsRemTablesDeletes_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpStatsRemTablesDrops",
                                         get_lldpStatsRemTablesDrops,
                                         lldpStatsRemTablesDrops_oid,
                                         OID_LENGTH(lldpStatsRemTablesDrops_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpStatsRemTablesAgeouts",
                                         get_lldpStatsRemTablesAgeouts,
                                         lldpStatsRemTablesAgeouts_oid,
                                         OID_LENGTH(lldpStatsRemTablesAgeouts_oid),
                                         HANDLER_CAN_RONLY));
}


int
get_lldpStatsRemTablesLastChangeTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_Statistics_T entry;

            if (LLDP_PMGR_GetSysStatisticsEntry(&entry) == LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.rem_tables_last_change_time;
                snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpStatsRemTablesInserts(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_Statistics_T entry;

            if (LLDP_PMGR_GetSysStatisticsEntry(&entry) == LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.rem_tables_inserts;
                snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpStatsRemTablesDeletes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_Statistics_T entry;

            if (LLDP_PMGR_GetSysStatisticsEntry(&entry) == LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.rem_tables_deletes;
                snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpStatsRemTablesDrops(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_Statistics_T entry;

            if (LLDP_PMGR_GetSysStatisticsEntry(&entry) == LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.rem_tables_drops;
                snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpStatsRemTablesAgeouts(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_Statistics_T entry;

            if (LLDP_PMGR_GetSysStatisticsEntry(&entry)==LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.rem_tables_ageouts;
                snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

/*
 * lldpStatsTxPortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpStatsTxPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,2 };
/*
 * variable3 lldpStatsTxPortTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpStatsTxPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpStatsTxPortNum,  ASN_INTEGER,  RONLY,   var_lldpStatsTxPortTable, 3,  { 6, 1, 1 }},
#endif

{LEAF_lldpStatsTxPortFramesTotal,  ASN_COUNTER,  RONLY,   var_lldpStatsTxPortTable, 3,  { 6, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpStatsTxPortTable module */
void
init_lldpStatsTxPortTable(void)
{

    DEBUGMSGTL(("lldpStatsTxPortTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpStatsTxPortTable", lldpStatsTxPortTable_variables, variable3,
               lldpStatsTxPortTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPSTATSTXPORTENTRY_INSTANCE_LEN  1

BOOL_T lldpStatsTxPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *port_num)
{

    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPSTATSTXPORTENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *port_num=compl[0];

    return TRUE;
}

/*
 * var_lldpStatsTxPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpStatsTxPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPSTATSTXPORTENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPSTATSTXPORTENTRY_INSTANCE_LEN];
    LLDP_MGR_PortTxStatistics_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPSTATSTXPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpStatsTxPortTable_OidIndexToData(exact,compc,compl, &entry.port_num)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetPortTxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpStatsTxPortTable_OidIndexToData(exact,compc,compl, &entry.port_num);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPSTATSTXPORTENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetPortTxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextPortTxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextPortTxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.port_num;
    memcpy(name+vp->namelen,best_inst,LLDPSTATSTXPORTENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPSTATSTXPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpStatsTxPortNum:
        *var_len = sizeof(long_return);
        long_return = entry.port_num;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpStatsTxPortFramesTotal:
        *var_len = sizeof(long_return);
        long_return = entry.tx_frames_total;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpStatsRxPortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpStatsRxPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,2 };
/*
 * variable3 lldpStatsRxPortTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpStatsRxPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpStatsRxPortNum,  ASN_INTEGER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 1 }},
#endif

{LEAF_lldpStatsRxPortFramesDiscardedTotal,  ASN_COUNTER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 2 }},
{LEAF_lldpStatsRxPortFramesErrors,  ASN_COUNTER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 3 }},
{LEAF_lldpStatsRxPortFramesTotal,  ASN_COUNTER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 4 }},
{LEAF_lldpStatsRxPortTLVsDiscardedTotal,  ASN_COUNTER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 5 }},
{LEAF_lldpStatsRxPortTLVsUnrecognizedTotal,  ASN_COUNTER,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 6 }},
{LEAF_lldpStatsRxPortAgeoutsTotal,  ASN_GAUGE,  RONLY,   var_lldpStatsRxPortTable, 3,  { 7, 1, 7 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpStatsRxPortTable module */
void
init_lldpStatsRxPortTable(void)
{

    DEBUGMSGTL(("lldpStatsRxPortTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpStatsRxPortTable", lldpStatsRxPortTable_variables, variable3,
               lldpStatsRxPortTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPSTATSRXPORTENTRY_INSTANCE_LEN  1

BOOL_T lldpStatsRxPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *port_num)
{

    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPSTATSRXPORTENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *port_num=compl[0];

    return TRUE;
}

/*
 * var_lldpStatsRxPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpStatsRxPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPSTATSRXPORTENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPSTATSRXPORTENTRY_INSTANCE_LEN];
    LLDP_MGR_PortRxStatistics_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPSTATSRXPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpStatsRxPortTable_OidIndexToData(exact,compc,compl, &entry.port_num)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetPortRxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpStatsRxPortTable_OidIndexToData(exact,compc,compl, &entry.port_num);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPSTATSRXPORTENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetPortRxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextPortRxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextPortRxStatisticsEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.port_num;
    memcpy(name+vp->namelen,best_inst,LLDPSTATSRXPORTENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPSTATSRXPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpStatsRxPortNum:
        *var_len = sizeof(long_return);
        long_return = entry.port_num;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpStatsRxPortFramesDiscardedTotal:
        *var_len = sizeof(long_return);
        long_return = entry.rx_frames_discarded_total;
        return (u_char *) &long_return;
    case LEAF_lldpStatsRxPortFramesErrors:
        *var_len = sizeof(long_return);
        long_return = entry.rx_frames_errors;
        return (u_char *) &long_return;
    case LEAF_lldpStatsRxPortFramesTotal:
        *var_len = sizeof(long_return);
        long_return = entry.rx_frames_total;
        return (u_char *) &long_return;
    case LEAF_lldpStatsRxPortTLVsDiscardedTotal:
        *var_len = sizeof(long_return);
        long_return = entry.rx_tlvs_discarded_total;
        return (u_char *) &long_return;
    case LEAF_lldpStatsRxPortTLVsUnrecognizedTotal:
        *var_len = sizeof(long_return);
        long_return = entry.rx_tlvs_unrecognized_total;
        return (u_char *) &long_return;
    case LEAF_lldpStatsRxPortAgeoutsTotal:
        *var_len = sizeof(long_return);
        long_return = entry.rx_ageouts_total;
        return (u_char *) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/** Initializes the lldpLocalSystemData module */
void
init_lldpLocalSystemData(void)
{
    static oid lldpLocChassisIdSubtype_oid[] = { 1,0,8802,1,1,2,1,3,1,0 };
    static oid lldpLocChassisId_oid[] = { 1,0,8802,1,1,2,1,3,2,0 };
    static oid lldpLocSysName_oid[] = { 1,0,8802,1,1,2,1,3,3,0 };
    static oid lldpLocSysDesc_oid[] = { 1,0,8802,1,1,2,1,3,4,0 };
    static oid lldpLocSysCapSupported_oid[] = { 1,0,8802,1,1,2,1,3,5,0 };
    static oid lldpLocSysCapEnabled_oid[] = { 1,0,8802,1,1,2,1,3,6,0 };

  DEBUGMSGTL(("lldpLocalSystemData", "Initializing\n"));

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocChassisIdSubtype",
                                         get_lldpLocChassisIdSubtype,
                                         lldpLocChassisIdSubtype_oid,
                                         OID_LENGTH(lldpLocChassisIdSubtype_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocChassisId",
                                         get_lldpLocChassisId,
                                         lldpLocChassisId_oid,
                                         OID_LENGTH(lldpLocChassisId_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocSysName",
                                         get_lldpLocSysName,
                                         lldpLocSysName_oid,
                                         OID_LENGTH(lldpLocSysName_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocSysDesc",
                                         get_lldpLocSysDesc,
                                         lldpLocSysDesc_oid,
                                         OID_LENGTH(lldpLocSysDesc_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocSysCapSupported",
                                         get_lldpLocSysCapSupported,
                                         lldpLocSysCapSupported_oid,
                                         OID_LENGTH(lldpLocSysCapSupported_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("lldpLocSysCapEnabled",
                                         get_lldpLocSysCapEnabled,
                                         lldpLocSysCapEnabled_oid,
                                         OID_LENGTH(lldpLocSysCapEnabled_oid),
                                         HANDLER_CAN_RONLY));
}

int
get_lldpLocChassisIdSubtype(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_LocalSystemData_T entry;

            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                long_return = entry.loc_chassis_id_subtype;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER , (u_char *)&long_return , sizeof(long_return) );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpLocChassisId(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            UI32_T var_len=0;
            LLDP_MGR_LocalSystemData_T entry;
            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                var_len =entry.loc_chassis_id_len;
                memcpy(return_buf,entry.loc_chassis_id,var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR , (u_char *)return_buf , var_len );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpLocSysName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            UI32_T var_len=0;
            LLDP_MGR_LocalSystemData_T entry;
            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                var_len =entry.loc_sys_name_len;
                memcpy(return_buf,entry.loc_sys_name,var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR , (u_char *)return_buf , var_len );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpLocSysDesc(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            UI32_T var_len=0;
            LLDP_MGR_LocalSystemData_T entry;
            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                var_len =entry.loc_sys_desc_len;
                memcpy(return_buf,entry.loc_sys_desc,var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR , (u_char *)return_buf , var_len );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpLocSysCapSupported(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_LocalSystemData_T entry;
            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                UI32_T var_len=0;
                UI8_T tmp;
#if 0
                ((UI8_T*)(&tmp))[1] = L_CVRT_ByteFlip(((UI8_T*)&entry.loc_sys_cap_supported)[1]);
                ((UI8_T*)(&tmp))[0] = L_CVRT_ByteFlip(((UI8_T*)&entry.loc_sys_cap_supported)[0]);
#endif
                tmp = entry.loc_sys_cap_supported;
                var_len =SIZE_lldpLocSysCapSupported;
                memcpy(return_buf,&tmp,var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR , (u_char *)return_buf , var_len );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
get_lldpLocSysCapEnabled(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{

    switch(reqinfo->mode) {
        case MODE_GET:
        {
            LLDP_MGR_LocalSystemData_T entry;
            if (LLDP_PMGR_GetLocalSystemData(&entry)==LLDP_TYPE_RETURN_OK)
            {
                UI32_T var_len=0;
                UI8_T  tmp;

#if 0
                ((UI8_T*)(&tmp))[1] = L_CVRT_ByteFlip(((UI8_T*)&entry.loc_sys_cap_enabled)[1]);
                ((UI8_T*)(&tmp))[0] = L_CVRT_ByteFlip(((UI8_T*)&entry.loc_sys_cap_enabled)[0]);
#endif
                tmp = entry.loc_sys_cap_enabled;
                var_len =SIZE_lldpLocSysCapEnabled;
                memcpy(return_buf,&tmp,var_len);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR , (u_char *)return_buf , var_len );
            }
            else
            {
                return SNMP_ERR_GENERR;
            }
        }
            break;


        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

/*
 * lldpLocPortTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpLocPortTable_variables_oid[] = { 1,0,8802,1,1,2,1,3 };
/*
 * variable3 lldpLocPortTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpLocPortTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpLocPortNum,  ASN_INTEGER,  RONLY,   var_lldpLocPortTable, 3,  { 7, 1, 1 }},
#endif

{LEAF_lldpLocPortIdSubtype,  ASN_INTEGER,  RONLY,   var_lldpLocPortTable, 3,  { 7, 1, 2 }},
{LEAF_lldpLocPortId,  ASN_OCTET_STR,  RONLY,   var_lldpLocPortTable, 3,  { 7, 1, 3 }},
{LEAF_lldpLocPortDesc,  ASN_OCTET_STR,  RONLY,   var_lldpLocPortTable, 3,  { 7, 1, 4 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpLocPortTable module */
void
init_lldpLocPortTable(void)
{

    DEBUGMSGTL(("lldpLocPortTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpLocPortTable", lldpLocPortTable_variables, variable3,
               lldpLocPortTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPLOCPORTENTRY_INSTANCE_LEN  1

BOOL_T lldpLocPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *port_num)
{

    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPLOCPORTENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *port_num=compl[0];

    return TRUE;
}

/*
 * var_lldpLocPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpLocPortTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPLOCPORTENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPLOCPORTENTRY_INSTANCE_LEN];
    LLDP_MGR_LocalPortData_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPLOCPORTENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpLocPortTable_OidIndexToData(exact,compc,compl, &entry.port_num)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetLocalPortData(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpLocPortTable_OidIndexToData(exact,compc,compl, &entry.port_num);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPLOCPORTENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetLocalPortData(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextLocalPortData(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextLocalPortData(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.port_num;
    memcpy(name+vp->namelen,best_inst,LLDPLOCPORTENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPLOCPORTENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpLocPortNum:
        *var_len = sizeof(long_return);
        long_return = entry.port_num;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpLocPortIdSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.loc_port_id_subtype;
        return (u_char *) &long_return;
    case LEAF_lldpLocPortId:
        memcpy(return_buf, entry.loc_port_id, entry.loc_port_id_len);
        *var_len  =   entry.loc_port_id_len;
        return (u_char*)return_buf;
    case LEAF_lldpLocPortDesc:
        *var_len  =   strlen(entry.loc_port_desc);
        memcpy(return_buf, entry.loc_port_desc,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpLocManAddrTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpLocManAddrTable_variables_oid[] = { 1,0,8802,1,1,2,1,3 };
/*
 * variable3 lldpLocManAddrTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpLocManAddrTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
/*    (L = length of the oidsuffix) */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    {LEAF_lldpLocManAddrSubtype,  ASN_INTEGER,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 1 }},
    {LEAF_lldpLocManAddr,  ASN_OCTET_STR,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 2 }},
#endif
    {LEAF_lldpLocManAddrLen,  ASN_INTEGER,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 3 }},
    {LEAF_lldpLocManAddrIfSubtype,  ASN_INTEGER,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 4 }},
    {LEAF_lldpLocManAddrIfId,  ASN_INTEGER,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 5 }},
    {LEAF_lldpLocManAddrOID,  ASN_OBJECT_ID,  RONLY,   var_lldpLocManAddrTable, 3,  { 8, 1, 6 }},
};

/** Initializes the lldpLocManAddrTable module */
void
init_lldpLocManAddrTable(void)
{

    DEBUGMSGTL(("lldpLocManAddrTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpLocManAddrTable", lldpLocManAddrTable_variables, variable3,
               lldpLocManAddrTable_variables_oid);

    /* place any other initialization junk you need here */
}

#define LLDPLOCMANADDRENTRY_INSTANCE_LEN  33

BOOL_T lldpLocManAddrTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI8_T *loc_man_addr_subtype, UI8_T *loc_man_addr,UI32_T *loc_man_addr_len)
{
    UI32_T i;
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if( compc < 3  || compc > LLDPLOCMANADDRENTRY_INSTANCE_LEN) /* the dynamic size index*/
        {
            return FALSE;
        }
    }
    *loc_man_addr_subtype=(UI8_T)compl[0];
    if((compl[1]< MINSIZE_lldpLocManAddr) || (compl[1]> MAXSIZE_lldpLocManAddr ))
    {
        return FALSE;
    }

    for( i=0;i< compl[1] ;i++)
    {
        loc_man_addr[i]=(UI8_T)compl[2+i];
    }
    *loc_man_addr_len = compl[1];
    return TRUE;
}

/*
 * var_lldpLocManAddrTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpLocManAddrTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T i;
    UI32_T compc=0;
    oid compl[LLDPLOCMANADDRENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPLOCMANADDRENTRY_INSTANCE_LEN];
    LLDP_MGR_LocalManagementAddrEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPLOCMANADDRENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpLocManAddrTable_OidIndexToData(exact,compc,compl, &entry.loc_man_addr_subtype, entry.loc_man_addr, &entry.loc_man_addr_len)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetLocalManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {
        lldpLocManAddrTable_OidIndexToData(exact,compc,compl, &entry.loc_man_addr_subtype, entry.loc_man_addr, &entry.loc_man_addr_len);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {

            if ( LLDP_PMGR_GetLocalManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextLocalManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextLocalManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.loc_man_addr_subtype;
    best_inst[1] = entry.loc_man_addr_len;

    for( i=0;i< best_inst[1] ;i++)
    {
        best_inst[2+i]=entry.loc_man_addr[i];
    }
    memcpy(name+vp->namelen,best_inst,(2+best_inst[1])*sizeof(oid));
    *length = vp->namelen+2+best_inst[1] ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpLocManAddrSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.loc_man_addr_subtype;
        return (u_char *) &long_return;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpLocManAddr:
        *var_len  = entry.loc_man_addr_len;
        memcpy(return_buf, entry.loc_man_addr,*var_len);
        return (u_char*)return_buf;
#endif
    case LEAF_lldpLocManAddrLen:
        *var_len = sizeof(long_return);
        long_return = entry.loc_man_addr_len + 1; /* 802.1AB-2005 9.5.9.2 */
        return (u_char *) &long_return;
    case LEAF_lldpLocManAddrIfSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.loc_man_addr_if_subtype;
        return (u_char *) &long_return;
    case LEAF_lldpLocManAddrIfId:
        *var_len = sizeof(long_return);
        long_return = entry.loc_man_addr_if_id;
        return (u_char *) &long_return;
    case LEAF_lldpLocManAddrOID:
        *var_len = ((sizeof(null_oid))/(sizeof((null_oid)[0])));
        #if 0 /*not support*/
        memcpy(return_buf,entry.loc_man_addr_oid,*var_len);
        #endif
        return (u_char *) null_oid;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpRemTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpRemTable_variables_oid[] = { 1,0,8802,1,1,2,1,4 };
/*
 * variable3 lldpRemTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpRemTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemTimeMark,  ASN_TIMETICKS,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 1 }},
#endif

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemLocalPortNum,  ASN_INTEGER,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 2 }},
#endif

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemIndex,  ASN_INTEGER,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 3 }},
#endif

{LEAF_lldpRemChassisIdSubtype,  ASN_INTEGER,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 4 }},
{LEAF_lldpRemChassisId,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 5 }},
{LEAF_lldpRemPortIdSubtype,  ASN_INTEGER,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 6 }},
{LEAF_lldpRemPortId,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 7 }},
{LEAF_lldpRemPortDesc,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 8 }},
{LEAF_lldpRemSysName,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 9 }},
{LEAF_lldpRemSysDesc,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 10 }},
{LEAF_lldpRemSysCapSupported,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 11 }},
{LEAF_lldpRemSysCapEnabled,  ASN_OCTET_STR,  RONLY,   var_lldpRemTable, 3,  { 1, 1, 12 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpRemTable module */
void
init_lldpRemTable(void)
{

    DEBUGMSGTL(("lldpRemTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpRemTable", lldpRemTable_variables, variable3,
               lldpRemTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPREMENTRY_INSTANCE_LEN  3

BOOL_T lldpRemTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *rem_time_mark, UI32_T *rem_local_port_num, UI32_T *rem_index)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPREMENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *rem_time_mark=compl[0];
    *rem_local_port_num=compl[1];
    *rem_index=compl[2];
    return TRUE;
}

/*
 * var_lldpRemTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpRemTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPREMENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPREMENTRY_INSTANCE_LEN];
    LLDP_MGR_RemoteSystemData_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPREMENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpRemTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_POM_GetRemoteSystemData(&entry)!=TRUE)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpRemTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {

            if ( LLDP_POM_GetRemoteSystemData(&entry)!=TRUE)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextRemoteSystemData(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextRemoteSystemData(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    memcpy(name+vp->namelen,best_inst,LLDPREMENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPREMENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemTimeMark:
        *var_len = sizeof(long_return);
        long_return = entry.rem_time_mark;
        return (u_char *) &long_return;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemLocalPortNum:
        *var_len = sizeof(long_return);
        long_return = entry.rem_local_port_num;
        return (u_char *) &long_return;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemIndex:
        *var_len = sizeof(long_return);
        long_return = entry.rem_index;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpRemChassisIdSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.rem_chassis_id_subtype;
        return (u_char *) &long_return;
    case LEAF_lldpRemChassisId:
        *var_len  =   entry.rem_chassis_id_len;
        memcpy(return_buf, &entry.rem_chassis_id,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpRemPortIdSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.rem_port_id_subtype;
        return (u_char *) &long_return;
    case LEAF_lldpRemPortId:
        *var_len  =   entry.rem_port_id_len;
        memcpy(return_buf, &entry.rem_port_id,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpRemPortDesc:
        *var_len  =   entry.rem_port_desc_len;
        memcpy(return_buf, entry.rem_port_desc,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpRemSysName:
        *var_len  =   entry.rem_sys_name_len;
        memcpy(return_buf, entry.rem_sys_name,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpRemSysDesc:
        *var_len  =   entry.rem_sys_desc_len;
        memcpy(return_buf, entry.rem_sys_desc,*var_len);
        return (u_char*)return_buf;
    case LEAF_lldpRemSysCapSupported:
        {
            UI16_T tmp;

           ((UI8_T*)(&tmp))[1] = L_CVRT_ByteFlip(((UI8_T*)&entry.rem_sys_cap_supported)[1]);
           ((UI8_T*)(&tmp))[0] = L_CVRT_ByteFlip(((UI8_T*)&entry.rem_sys_cap_supported)[0]);
            *var_len = SIZE_lldpRemSysCapSupported;
            memcpy(return_buf, &tmp,*var_len);
        }
        return (u_char*)return_buf;
    case LEAF_lldpRemSysCapEnabled:
        {
            UI16_T tmp;

           ((UI8_T*)(&tmp))[1] = L_CVRT_ByteFlip(((UI8_T*)&entry.rem_sys_cap_enabled)[1]);
           ((UI8_T*)(&tmp))[0] = L_CVRT_ByteFlip(((UI8_T*)&entry.rem_sys_cap_enabled)[0]);
            *var_len = SIZE_lldpRemSysCapEnabled;
            memcpy(return_buf, &tmp,*var_len);
        }
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpRemManAddrTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpRemManAddrTable_variables_oid[] = { 1,0,8802,1,1,2,1,4 };
/*
 * variable3 lldpRemManAddrTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpRemManAddrTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemManAddrSubtype,  ASN_INTEGER,  RONLY,   var_lldpRemManAddrTable, 3,  { 2, 1, 1 }},
#endif

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemManAddr,  ASN_OCTET_STR,  RONLY,   var_lldpRemManAddrTable, 3,  { 2, 1, 2 }},
#endif

{LEAF_lldpRemManAddrIfSubtype,  ASN_INTEGER,  RONLY,   var_lldpRemManAddrTable, 3,  { 2, 1, 3 }},
{LEAF_lldpRemManAddrIfId,  ASN_INTEGER,  RONLY,   var_lldpRemManAddrTable, 3,  { 2, 1, 4 }},
{LEAF_lldpRemManAddrOID,  ASN_OBJECT_ID,  RONLY,   var_lldpRemManAddrTable, 3,  { 2, 1, 5 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpRemManAddrTable module */
void
init_lldpRemManAddrTable(void)
{

    DEBUGMSGTL(("lldpRemManAddrTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpRemManAddrTable", lldpRemManAddrTable_variables, variable3,
               lldpRemManAddrTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPREMMANADDRENTRY_INSTANCE_LEN  36

BOOL_T lldpRemManAddrTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *rem_time_mark, UI32_T *rem_local_port_num, UI32_T *rem_index, UI8_T *rem_man_addr_subtype, UI8_T *rem_man_addr,UI32_T *rem_man_addr_len)
{
    UI32_T i;
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if( compc < 6  || compc > LLDPREMMANADDRENTRY_INSTANCE_LEN) /* the dynamic size index*/
        {
            return FALSE;
        }
    }
    *rem_time_mark=compl[0];
    *rem_local_port_num=compl[1];
    *rem_index=compl[2];
    *rem_man_addr_subtype=(UI8_T)compl[3];
    if((compl[4]< MINSIZE_lldpRemManAddr) || (compl[4]> MAXSIZE_lldpRemManAddr ))
    {
        return FALSE;
    }

    for( i=0;i< compl[4] ;i++)
    {
        rem_man_addr[i]=(UI8_T)compl[5+i];
    }
    *rem_man_addr_len=compl[4];
    return TRUE;
}

/*
 * var_lldpRemManAddrTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpRemManAddrTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T i;
    UI32_T compc=0;
    oid compl[LLDPREMMANADDRENTRY_INSTANCE_LEN]={0};
    oid best_inst[LLDPREMMANADDRENTRY_INSTANCE_LEN];
    LLDP_MGR_RemoteManagementAddrEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPREMMANADDRENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpRemManAddrTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_man_addr_subtype, entry.rem_man_addr, &entry.rem_man_addr_len)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetRemoteManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpRemManAddrTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_man_addr_subtype, entry.rem_man_addr, &entry.rem_man_addr_len);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< 1)
        {

            if ( LLDP_PMGR_GetRemoteManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    best_inst[3]=entry.rem_man_addr_subtype;
    best_inst[4] = entry.rem_man_addr_len;

    for( i=0;i< best_inst[4] ;i++)
    {
        best_inst[5+i]=entry.rem_man_addr[i];
    }
    memcpy(name+vp->namelen,best_inst,(5+best_inst[4])*sizeof(oid));
    *length = vp->namelen+5+best_inst[4] ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemManAddrSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.rem_man_addr_subtype;
        return (u_char *) &long_return;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemManAddr:
        *var_len  =  entry.rem_man_addr_len;
        memcpy(return_buf, entry.rem_man_addr,*var_len);
        return (u_char*)return_buf;
#endif
    case LEAF_lldpRemManAddrIfSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.rem_man_addr_if_subtype;
        return (u_char *) &long_return;
    case LEAF_lldpRemManAddrIfId:
        *var_len = sizeof(long_return);
        long_return = entry.rem_man_addr_if_id;
        return (u_char *) &long_return;
    case LEAF_lldpRemManAddrOID:
        *var_len = 0;
#if 0 /*not support*/
        memcpy(return_buf,entry.rem_man_addr_oid,*var_len);
#endif
        return (u_char *) return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

#if 0 /*not support*/
/*
 * lldpRemUnknownTLVTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpRemUnknownTLVTable_variables_oid[] = { 1,0,8802,1,1,2,1,4 };
/*
 * variable3 lldpRemUnknownTLVTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpRemUnknownTLVTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemUnknownTLVType,  ASN_INTEGER,  RONLY,   var_lldpRemUnknownTLVTable, 3,  { 3, 1, 1 }},
#endif

{LEAF_lldpRemUnknownTLVInfo,  ASN_OCTET_STR,  RONLY,   var_lldpRemUnknownTLVTable, 3,  { 3, 1, 2 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpRemUnknownTLVTable module */
void
init_lldpRemUnknownTLVTable(void)
{

    DEBUGMSGTL(("lldpRemUnknownTLVTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpRemUnknownTLVTable", lldpRemUnknownTLVTable_variables, variable3,
               lldpRemUnknownTLVTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN  4

BOOL_T lldpRemUnknownTLVTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *rem_time_mark, UI32_T *rem_local_port_num, UI32_T *rem_index, UI32_T *rem_unknown_tlv_type)
{
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *rem_time_mark=compl[0];
    *rem_local_port_num=compl[1];
    *rem_index=compl[2];
    *rem_unknown_tlv_type=compl[3];
    return TRUE;
}

/*
 * var_lldpRemUnknownTLVTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpRemUnknownTLVTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T compc=0;
    oid compl[LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN];
    oid best_inst[LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN];
    LLDP_MGR_RemoteUnknownTlvEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    if (exact)/*get,set*/
    {
        if(lldpRemUnknownTLVTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_unknown_tlv_type)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetRemoteUnknownTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpRemUnknownTLVTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, &entry.rem_unknown_tlv_type);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetRemoteUnknownTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextRemoteUnknownTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextRemoteUnknownTlvEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */


    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    best_inst[3]=entry.rem_unknown_tlv_type;
    memcpy(name+vp->namelen,best_inst,LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPREMUNKNOWNTLVENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemUnknownTLVType:
        *var_len = sizeof(long_return);
        long_return = entry.rem_unknown_tlv_type;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpRemUnknownTLVInfo:
        *var_len  =   strlen(entry.rem_unknown_tlv_info);
        memcpy(return_buf, entry.rem_unknown_tlv_info,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

/*
 * lldpRemOrgDefInfoTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid lldpRemOrgDefInfoTable_variables_oid[] = { 1,0,8802,1,1,2,1,4 };
/*
 * variable3 lldpRemOrgDefInfoTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 lldpRemOrgDefInfoTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemOrgDefInfoOUI,  ASN_OCTET_STR,  RONLY,   var_lldpRemOrgDefInfoTable, 3,  { 4, 1, 1 }},
#endif

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemOrgDefInfoSubtype,  ASN_INTEGER,  RONLY,   var_lldpRemOrgDefInfoTable, 3,  { 4, 1, 2 }},
#endif

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
{LEAF_lldpRemOrgDefInfoIndex,  ASN_INTEGER,  RONLY,   var_lldpRemOrgDefInfoTable, 3,  { 4, 1, 3 }},
#endif

{LEAF_lldpRemOrgDefInfo,  ASN_OCTET_STR,  RONLY,   var_lldpRemOrgDefInfoTable, 3,  { 4, 1, 4 }},
};
/*    (L = length of the oidsuffix) */

/** Initializes the lldpRemOrgDefInfoTable module */
void
init_lldpRemOrgDefInfoTable(void)
{

    DEBUGMSGTL(("lldpRemOrgDefInfoTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("lldpRemOrgDefInfoTable", lldpRemOrgDefInfoTable_variables, variable3,
               lldpRemOrgDefInfoTable_variables_oid);

    /* place any other initialization junk you need here */
}
#define LLDPREMORGDEFINFOENTRY_INSTANCE_LEN  8

BOOL_T lldpRemOrgDefInfoTable_OidIndexToData(UI32_T exact, UI32_T compc,
            oid * compl ,  UI32_T *rem_time_mark, UI32_T *rem_local_port_num, UI32_T *rem_index, UI8_T *rem_org_def_info_oui, UI8_T *rem_org_def_info_subtype, UI32_T *rem_org_def_info_index)
{
    UI32_T i;
    /* get or write
     */
    if(exact)
    {

        /* check the index length
         */

        if(compc != LLDPREMORGDEFINFOENTRY_INSTANCE_LEN) /* the constant size index*/
        {
            return FALSE;
        }
    }
    *rem_time_mark=compl[0];
    *rem_local_port_num=compl[1];
    *rem_index=compl[2];
    for( i=0;i< SIZE_lldpRemOrgDefInfoOUI ;i++)
    {
        rem_org_def_info_oui[i]=compl[3+i];
    }

    *rem_org_def_info_subtype=compl[6];
    *rem_org_def_info_index=compl[7];
    return TRUE;
}

/*
 * var_lldpRemOrgDefInfoTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_lldpRemOrgDefInfoTable(struct variable *vp,
            oid     *name,
            size_t  *length,
            int     exact,
            size_t  *var_len,
            WriteMethod **write_method)
{
    /* variables we may use later */
    UI32_T i;
    UI32_T compc=0;
    oid compl[LLDPREMORGDEFINFOENTRY_INSTANCE_LEN];
    oid best_inst[LLDPREMORGDEFINFOENTRY_INSTANCE_LEN];
    LLDP_MGR_RemoteOrgDefInfoEntry_T  entry;
    switch(vp->magic)
    {
    default:
        *write_method = 0;
        break;
    }

    /*check compc, retrive compl*/
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, LLDPREMORGDEFINFOENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));


    if (exact)/*get,set*/
    {
        if(lldpRemOrgDefInfoTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, entry.rem_org_def_info_oui, &entry.rem_org_def_info_subtype, &entry.rem_org_def_info_index)==FALSE)
        {
            return NULL;
        }

        /*get data
         */
        if (LLDP_PMGR_GetRemoteOrgDefInfoEntry(&entry)!=LLDP_TYPE_RETURN_OK)
        {
            return NULL;
        }
    }
    else/*getnext*/
    {

        lldpRemOrgDefInfoTable_OidIndexToData(exact,compc,compl, &entry.rem_time_mark, &entry.rem_local_port_num, &entry.rem_index, entry.rem_org_def_info_oui, &entry.rem_org_def_info_subtype, &entry.rem_org_def_info_index);
        /* check the length of inputing index,if <1 we should try get {0.0.0.0.0...}
         */
        if (compc< LLDPREMORGDEFINFOENTRY_INSTANCE_LEN)
        {

            if ( LLDP_PMGR_GetRemoteOrgDefInfoEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {

                /*get next data*/
                if ( LLDP_PMGR_GetNextRemoteOrgDefInfoEntry(&entry)!=LLDP_TYPE_RETURN_OK)
                {
                    return NULL;
                }
            }
        }
        else
        {

            /*get next data*/
            if ( LLDP_PMGR_GetNextRemoteOrgDefInfoEntry(&entry)!=LLDP_TYPE_RETURN_OK)
            {
                return NULL;
            }
        }
    }

    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    /* assign data to the oid index
     */
    best_inst[0]=entry.rem_time_mark;
    best_inst[1]=entry.rem_local_port_num;
    best_inst[2]=entry.rem_index;
    for( i=0;i< SIZE_lldpRemOrgDefInfoOUI ;i++)
    {
        best_inst[3+i]=entry.rem_org_def_info_oui[i];
    }
    best_inst[6]=entry.rem_org_def_info_subtype;
    best_inst[7]=entry.rem_org_def_info_index;
    memcpy(name+vp->namelen,best_inst,LLDPREMORGDEFINFOENTRY_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen+LLDPREMORGDEFINFOENTRY_INSTANCE_LEN ;

    /* this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {

#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemOrgDefInfoOUI:
        *var_len = SIZE_lldpRemOrgDefInfoOUI;
        memcpy(return_buf, entry.rem_org_def_info_oui,*var_len);
        return (u_char*)return_buf;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemOrgDefInfoSubtype:
        *var_len = sizeof(long_return);
        long_return = entry.rem_org_def_info_subtype;
        return (u_char *) &long_return;
#endif
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    case LEAF_lldpRemOrgDefInfoIndex:
        *var_len = sizeof(long_return);
        long_return = entry.rem_org_def_info_index;
        return (u_char *) &long_return;
#endif
    case LEAF_lldpRemOrgDefInfo:
        *var_len = strlen(entry.rem_org_def_info_len;
        memcpy(return_buf, entry.rem_org_def_info,*var_len);
        return (u_char*)return_buf;
    default:
      ERROR_MSG("");
    }
    return NULL;
}
#endif

#endif /* #if (SYS_CPNT_LLDP == TRUE) */
