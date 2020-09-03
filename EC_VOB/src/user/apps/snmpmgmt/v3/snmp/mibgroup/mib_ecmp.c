#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ECMP_BALANCE_MODE== TRUE)
#include "sysfun.h"
#include "mibtreeinit.h"
#include "sys_pmgr.h"
#include "mib_ecmp.h"
#include "netcfg_pom_route.h"
#include "netcfg_pmgr_route.h"

void init_ecmpMgt(void)
{
    oid ecmpBalanceMode_oid[] = { SYS_ADPT_PRIVATEMIB_OID, 1,18,7,1, 0 };
    oid ecmpHashSelectionListIndex_oid[] = { SYS_ADPT_PRIVATEMIB_OID, 1,18,7,2, 0 };

    UI8_T private_mib_root_str[SYS_ADPT_MAX_OID_STRING_LEN + 1] = {0};
    oid board_privmib_root[32];
    UI32_T board_privmib_root_len;

    SYS_PMGR_GetPrivateMibRoot(SYS_VAL_LOCAL_UNIT_ID, private_mib_root_str);

    SNMP_MGR_StringToObejctID(board_privmib_root, (I8_T *)private_mib_root_str, &board_privmib_root_len);

    _REGISTER_RWRITE("ecmpBalanceMode",
                     do_ecmpBalanceMode,
                     ecmpBalanceMode_oid,
                     HANDLER_CAN_RWRITE);

    _REGISTER_RWRITE("ecmpHashSelectionListIndex",
                     do_ecmpHashSelectionListIndex,
                     ecmpHashSelectionListIndex_oid,
                     HANDLER_CAN_RWRITE);
}

int do_ecmpBalanceMode(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T mode, hidx;

            /* get from core layer
             */
            if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&mode, &hidx))
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
                switch (mode)
                {
                case NETCFG_TYPE_ECMP_HASH_SELECTION:
                    mode = VAL_ecmpBalanceMode_hashSelectionList;
                    break;
                case NETCFG_TYPE_ECMP_DIP_L4_PORT:
                    mode = VAL_ecmpBalanceMode_dstIpL4Port;
                    break;
                default:
                    return SNMP_ERR_GENERR;
                }
                long_return = mode;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                    (u_char *) &long_return, sizeof(long_return));
            }

            break;
        }

        /* SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            /* check type and length
             */
            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }

            break;

        case MODE_SET_RESERVE2:
            switch (*requests->requestvb->val.integer)
            {
                case VAL_ecmpBalanceMode_dstIpL4Port:
                    break;

                case VAL_ecmpBalanceMode_hashSelectionList:
                    break;

                default:
                    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
                    return SNMP_ERR_NOERROR;
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            /* set to core layer
             */
            UI32_T n_mode =0, o_mode =0, hidx =0;

            /* get from core layer
             */
            if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&o_mode, &hidx))
            {
                return SNMP_ERR_GENERR;
            }

            switch (*requests->requestvb->val.integer)
            {
            case VAL_ecmpBalanceMode_hashSelectionList:
                n_mode = NETCFG_TYPE_ECMP_HASH_SELECTION;
                hidx = 1;
                break;
            case VAL_ecmpBalanceMode_dstIpL4Port:
                n_mode = NETCFG_TYPE_ECMP_DIP_L4_PORT;
                break;
            default:
                break;
            }

            if (o_mode == n_mode)
                return SNMP_ERR_NOERROR;

            if (NETCFG_TYPE_OK != NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(n_mode, hidx))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
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

int do_ecmpHashSelectionListIndex(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T mode, hidx;

            /* get from core layer
             */
            if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&mode, &hidx))
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
                long_return = hidx;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                    (u_char *) &long_return, sizeof(long_return));
            }

            break;
        }

        /* SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            /* check type and length
             */
            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }

            break;

        case MODE_SET_RESERVE2:
            if ((*requests->requestvb->val.integer < MIN_ecmpHashSelectionListIndex)
                || (*requests->requestvb->val.integer > MAX_ecmpHashSelectionListIndex))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
                return SNMP_ERR_NOERROR;
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            UI32_T mode, hidx;

            /* get from core layer
             */
            if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&mode, &hidx))
            {
                return SNMP_ERR_GENERR;
            }

            if (hidx == *requests->requestvb->val.integer)
                return SNMP_ERR_NOERROR;

            hidx = *requests->requestvb->val.integer;

            switch (mode)
            {
            case NETCFG_TYPE_ECMP_HASH_SELECTION:
                if (hidx == 0)
                {
                    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                    return SNMP_ERR_NOERROR;
                }
                else
                {
                    /* set to core layer
                     */
                    if (NETCFG_TYPE_OK != NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(
                            mode, hidx))
                    {
                        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                        return SNMP_ERR_NOERROR;
                    }
                }
                break;
            case NETCFG_TYPE_ECMP_DIP_L4_PORT:
                if (hidx != 0)
                {
                    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                    return SNMP_ERR_NOERROR;
                }
                break;
            default:
                break;
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

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

