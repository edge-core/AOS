
#include "sys_cpnt.h"
#if (SYS_CPNT_SYNCE == TRUE)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "swctrl_pom.h"
#include "snmp_mgr.h"
#include "mib_synce.h"
#include "sync_e_pmgr.h"
#include "l_pbmp.h"
#include "l_bitmap.h"
#include "sync_e_type.h"
#include "syncedrv_type.h"
#include "syncedrv_om.h"

int do_syncEStatus(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    UI32_T i=0;
    UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  r_lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0;

            /* get from core layer
             */
            SYNCEDRV_OM_GetSyncEEnabledPbmp(lport_pbmp);

            var_len = sizeof(lport_pbmp);
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                (u_char *) lport_pbmp, var_len);

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
            if (requests->requestvb->type != ASN_OCTET_STR)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }

            if (requests->requestvb->val_len > SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                return SNMP_ERR_NOERROR;
            }

            break;

        case MODE_SET_RESERVE2:
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
            memcpy(lport_pbmp, requests->requestvb->val.string, sizeof(lport_pbmp));

            for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
                i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
            {
                if(!L_PBMP_GET_PORT_IN_PBMP_ARRAY(lport_pbmp, i))
                {
                    L_PBMP_SET_PORT_IN_PBMP_ARRAY(r_lport_pbmp, i);
                }
            }

            if (SYNC_E_PMGR_EnableSyncE(lport_pbmp) != SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            if (SYNC_E_PMGR_DisableSyncE(r_lport_pbmp) != SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_COMMITFAILED;
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

int do_syncESsmStatus(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    SYNC_E_MGR_PortEntry_T port_entry;

    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0, i;
            UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

            /* get from core layer
             */
            for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
                i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
            {
              port_entry.ifindex = i;

              if(SYNC_E_PMGR_GetPortEntry(&port_entry) != SYNC_E_TYPE_RET_SUCCESS)
                  continue;
              if(port_entry.is_eanbled)
                L_BITMAP_port_set(lport_pbmp, port_entry.ifindex);
            }
            {
                var_len = sizeof(lport_pbmp);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                    (u_char *) lport_pbmp, var_len);
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
            if (requests->requestvb->type != ASN_OCTET_STR)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }

            if (requests->requestvb->val_len > SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                return SNMP_ERR_NOERROR;
            }

            break;

        case MODE_SET_RESERVE2:
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            UI32_T i;
            /* set to core layer
             */
            for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
                i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
            {
              port_entry.ifindex = i;

              if(SYNC_E_PMGR_GetPortEntry(&port_entry) != SYNC_E_TYPE_RET_SUCCESS)
                  continue;

              if(L_BITMAP_port_is_set(requests->requestvb->val.string, i))
              {
                SYNC_E_PMGR_SetPortSsm(i, TRUE, port_entry.pri);
              }
              else
              {
                SYNC_E_PMGR_SetPortSsm(i, FALSE, port_entry.pri);
             }
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

int do_syncEClockSourcePort(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI32_T clock_src_num;
    UI32_T clock_src_get_ret_val, i;
    UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  tmp_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};
    BOOL_T locked_status;

    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0;

            clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);

            /* get from core layer
             */

            for(i=0; i<clock_src_num; i++)
            {
               L_BITMAP_port_set(lport_pbmp, clock_src_lst[i].port);
            }

            if (clock_src_get_ret_val != SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
                var_len = sizeof(lport_pbmp);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                    (u_char *) lport_pbmp, var_len);
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
            if (requests->requestvb->type != ASN_OCTET_STR)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }

            if (requests->requestvb->val_len > SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGLENGTH);
                return SNMP_ERR_NOERROR;
            }

            break;

        case MODE_SET_RESERVE2:
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            UI32_T port, clock_src_lst_len=0;


            memcpy(lport_pbmp, requests->requestvb->val.string,
                   (requests->requestvb->val_len < sizeof(lport_pbmp))? requests->requestvb->val_len: sizeof(lport_pbmp));

            /* get from core layer
             */

            clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);
            if (clock_src_get_ret_val != SYNC_E_TYPE_RET_SUCCESS)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
            }

            for(i=0; i<clock_src_num; i++)
            {
               L_BITMAP_port_set(tmp_pbmp, clock_src_lst[i].port);
            }

            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(lport_pbmp, SWCTRL_POM_UIGetUnitPortNumber(1), port)
            {
                if(FALSE == SYNC_E_TYPE_IS_VALID_IFINDEX_RANGE(port))
                {
                    continue;
                }
                /*remove unset port*/
                L_BITMAP_port_unset(tmp_pbmp, port);

                clock_src_lst[clock_src_lst_len].port = port;
                clock_src_lst[clock_src_lst_len].priority = port; /* use user port id as default setting */
                clock_src_lst_len++;

                if(clock_src_lst_len == SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
                {
                    break;
                }
            }

            /* set to core layer
             */

            if(clock_src_lst_len != 0)
            {
              if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_UpdateSyncEClockSource(1, clock_src_lst, clock_src_lst_len))
              {
                  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                  return SNMP_ERR_NOERROR;
              }
            }

            /*unset*/
            clock_src_lst_len=0;
            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(tmp_pbmp, SWCTRL_POM_UIGetUnitPortNumber(1), port)
            {
                clock_src_lst[clock_src_lst_len].port=port;
                clock_src_lst_len++;

                if(clock_src_lst_len == SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
                {
                    break;
                }
            }

            if(clock_src_lst_len == 0)
                return SNMP_ERR_NOERROR;

            if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_RemoveSyncEClockSource(1, clock_src_lst, clock_src_lst_len))
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

int get_syncEGoodClockSource(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI32_T clock_src_num;
    UI32_T clock_src_get_ret_val, i;
    BOOL_T locked_status;

    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0;
            UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
            /* get from core layer
             */
            clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);

            if (clock_src_get_ret_val != SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
              for(i=0; i<clock_src_num; i++)
              {
                 if(clock_src_lst[i].is_good_status == TRUE)
                   L_BITMAP_port_set(lport_pbmp, clock_src_lst[i].port);
              }
              var_len = sizeof(lport_pbmp);
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                  (u_char *) lport_pbmp, var_len);
            }
            break;
        }

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int do_syncEClockSourceSelect(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    BOOL_T is_enabled = FALSE;

    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T mode=0;

            /* get from core layer
             */
            if(SYNC_E_PMGR_GetClockSrcSelectMode(&mode)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
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
                case VAL_syncEClockSourceSelect_manual:
                    break;

                case VAL_syncEClockSourceSelect_auto:
                    break;

                case VAL_syncEClockSourceSelect_ssm:
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
            BOOL_T is_enabled_org, is_revertive_org;
            UI32_T force_clock_src_ifindex_org;

            /* set to core layer
             */
            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
            }

            if(*requests->requestvb->val.integer == VAL_syncEClockSourceSelect_auto)
            {
              if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_GetClkSrcSsm(&is_enabled))
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }
              if(is_enabled)
              {
                if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_SetClkSrcSsm(FALSE))
                {
                  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                  return SNMP_ERR_NOERROR;
                }
              }
              if (SYNC_E_PMGR_SetClockSrcSelectMode(TRUE, is_revertive_org, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }
            }
            else if(*requests->requestvb->val.integer == VAL_syncEClockSourceSelect_manual)
            {
              if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_GetClkSrcSsm(&is_enabled))
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }

              if(is_enabled)
              {
                if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_SetClkSrcSsm(FALSE))
                {
                  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                  return SNMP_ERR_NOERROR;
                }
              }

              if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE, 0)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }
            }
            else
            {
              if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE, 0)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }

              if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_SetClkSrcSsm(TRUE))
              {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
              }
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


int do_syncEAutoClockSourceRevertive(netsnmp_mib_handler *handler,
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
            BOOL_T is_enabled_org, is_revertive_org;
            UI32_T force_clock_src_ifindex_org;

            /* get from core layer
             */
            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
            }
            else
            {
                long_return = is_revertive_org?VAL_syncEAutoClockSourceRevertive_enabled:VAL_syncEAutoClockSourceRevertive_disabled;
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
                case VAL_syncEAutoClockSourceRevertive_enabled:
                    break;

                case VAL_syncEAutoClockSourceRevertive_disabled:
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

            BOOL_T is_enabled_org, is_revertive_org;
            UI32_T force_clock_src_ifindex_org;
            /* set to core layer
             */
            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                return SNMP_ERR_NOERROR;
            }
            is_revertive_org = (*requests->requestvb->val.integer == VAL_syncEAutoClockSourceRevertive_enabled?TRUE:FALSE);
            if (SYNC_E_PMGR_SetClockSrcSelectMode(is_enabled_org, is_revertive_org, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
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

int get_syncEClockSourceLocked(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI32_T clock_src_num;
    UI32_T clock_src_get_ret_val;
    BOOL_T locked_status;
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /* GET REQUEST
         */
        case MODE_GET:
        {
            /* get from core layer
             */
            clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);
            if(clock_src_get_ret_val!=SYNC_E_TYPE_RET_SUCCESS)
            {
                return SNMP_ERR_GENERR;
            }
            else
            {
                long_return = locked_status?VAL_syncEClockSourceLocked_true:VAL_syncEClockSourceLocked_false;
                snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                    (u_char *) &long_return, sizeof(long_return));
            }

            break;
        }

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


#define SYNCEPORTENTRY_INSTANCE_LEN  1

BOOL_T syncEPortTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *syncEPortIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != SYNCEPORTENTRY_INSTANCE_LEN)  /* the constant size index */
        {
            return FALSE;
        }
    }

    *syncEPortIndex = compl[0];

    return TRUE;
}

/*
 * var_syncEPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_syncEPortTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[SYNCEPORTENTRY_INSTANCE_LEN] = {0};
    oid best_inst[SYNCEPORTENTRY_INSTANCE_LEN] = {0};

    /* table-specific variables
     */
    SYNC_E_MGR_PortEntry_T port_entry;

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_syncEPortStatus:
            *write_method = write_syncEPortStatus;
            break;
        case LEAF_syncEPortSSMStatus:
            *write_method = write_syncEPortSSMStatus;
            break;

        case LEAF_syncEPortSSMPriority:
            *write_method = write_syncEPortSSMPriority;
            break;

        case LEAF_syncEPortForceClockSourceSelect:
            *write_method = write_syncEPortForceClockSourceSelect;
            break;
        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        SYNCEPORTENTRY_INSTANCE_LEN);

    memset(&port_entry, 0, sizeof(port_entry));

    /* dispatch get-exact versus get-next
     */
    if (exact)  /* get or set */
    {
        /* extract index
         */
        if (! syncEPortTable_OidIndexToData(exact, compc, compl,
            &port_entry.ifindex))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_GetPortEntry(&port_entry))
        {
            return NULL;
        }
    }
    else  /* get-next */
    {
        /* extract index
         */
        syncEPortTable_OidIndexToData(exact, compc, compl,
            &port_entry.ifindex);

        /* Check the length of inputing index.  If compc is less than the
         * instance length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < SYNCEPORTENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_GetPortEntry(&port_entry))
            {
              if (SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_GetNextPortEntry(&port_entry))
              {
                return NULL;
              }

            }
        }
        else   /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_GetNextPortEntry(&port_entry))
            {
              return NULL;
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = port_entry.ifindex;
    memcpy(name + vp->namelen, best_inst, SYNCEPORTENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + SYNCEPORTENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_syncEPortIndex:
            *var_len = sizeof(long_return);
            long_return = port_entry.ifindex;
            return (u_char *) &long_return;
#endif  /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */
        case LEAF_syncEPortStatus:
        {
            UI8_T  pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]={0};
            *var_len = sizeof(long_return);

            SYNCEDRV_OM_GetSyncEEnabledPbmp(pbmp);
            if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(pbmp, port_entry.ifindex))
                long_return = VAL_syncEPortStatus_enabled;
            else
                long_return = VAL_syncEPortStatus_disabled;
            return (u_char *) &long_return;
        }
        case LEAF_syncEPortSSMStatus:
            *var_len = sizeof(long_return);
            long_return = port_entry.is_eanbled?VAL_syncEPortSSMStatus_enabled:VAL_syncEPortSSMStatus_disabled;
            return (u_char *) &long_return;

        case LEAF_syncEPortSSMPriority:
            *var_len = sizeof(long_return);
            long_return = port_entry.pri;
            return (u_char *) &long_return;

        case LEAF_syncEPortTxQL:
            *var_len = sizeof(long_return);
            long_return = port_entry.tx_ql;
            return (u_char *) &long_return;

        case LEAF_syncEPortRxQL:
            *var_len = sizeof(long_return);
            long_return = port_entry.rx_ql;
            return (u_char *) &long_return;
        case LEAF_syncEPortForceClockSourceSelect:
        {
            UI32_T force_clock_src_ifindex_org;
            BOOL_T is_enabled_org, is_revertive_org;

            *var_len = sizeof(long_return);

            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
            {
              return NULL;
            }

            if(force_clock_src_ifindex_org == port_entry.ifindex)
            {
              long_return = VAL_syncEPortForceClockSourceSelect_enabled;
              return (u_char *) &long_return;
            }
            else
            {
              long_return = VAL_syncEPortForceClockSourceSelect_disabled;
              return (u_char *) &long_return;
            }
        }

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_syncEPortStatus(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_syncEPortStatus_enabled:
                    break;

                case VAL_syncEPortStatus_disabled:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* table-specific variables
             */
            UI32_T syncEPortIndex = 0;

            /* extract index
             */
            if (! syncEPortTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &syncEPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            L_PBMP_SET_PORT_IN_PBMP_ARRAY(lport_pbmp, syncEPortIndex);
            /* set to core layer
             */
            if(value == VAL_syncEPortStatus_enabled)
            {
              if (SYNC_E_PMGR_EnableSyncE(lport_pbmp)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                return SNMP_ERR_COMMITFAILED;
              }
            }
            else if(value == VAL_syncEPortStatus_disabled)
            {
              if (SYNC_E_PMGR_DisableSyncE(lport_pbmp)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                  return SNMP_ERR_COMMITFAILED;
              }
            }

            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

int write_syncEPortSSMStatus(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_syncEPortSSMStatus_enabled:
                    break;

                case VAL_syncEPortSSMStatus_disabled:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* table-specific variables
             */
            UI32_T syncEPortIndex = 0;

            /* extract index
             */
            if (! syncEPortTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &syncEPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            /* set to core layer
             */
            if(value == VAL_syncEPortSSMStatus_enabled)
            {
              if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_SetPortSsm(syncEPortIndex, TRUE, SYS_DFLT_SYNC_E_SSM_PRIORITY))
              {
                return SNMP_ERR_COMMITFAILED;
			  }
			}
			else
			{
              if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_SetPortSsm(syncEPortIndex, FALSE, SYS_DFLT_SYNC_E_SSM_PRIORITY))
              {
                return SNMP_ERR_COMMITFAILED;
			  }
			}
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

int write_syncEPortSSMPriority(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {

                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            if ((*(long *) var_val < SYS_ADPT_SYNC_E_MIN_SSM_PRIORITY)
                || (*(long *) var_val > SYS_ADPT_SYNC_E_MAX_SSM_PRIORITY))
            {

                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;
            BOOL_T is_enabled;

            /* table-specific variables
             */
            UI32_T syncEPortIndex = 0;

            /* extract index
             */
            if (! syncEPortTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &syncEPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_GetPortSsmStatus(syncEPortIndex, &is_enabled))
            {
                return SNMP_ERR_COMMITFAILED;
            }

			if(!is_enabled)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* set to core layer
             */
            if(SYNC_E_TYPE_RET_SUCCESS != SYNC_E_PMGR_SetPortSsm(syncEPortIndex, TRUE, value))
            {
              return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

int write_syncEPortForceClockSourceSelect(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_syncEPortForceClockSourceSelect_enabled:
                    break;

                case VAL_syncEPortForceClockSourceSelect_disabled:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = SNMP_MGR_Get_PrivateMibRootLen() + 5;
            I32_T value = 0;

            /* table-specific variables
             */
            UI32_T syncEPortIndex = 0;

            /* extract index
             */
            if (! syncEPortTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &syncEPortIndex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            value = *(long *)var_val;

            /* set to core layer
             */
            if(value == VAL_syncEPortForceClockSourceSelect_enabled)
            {
              if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE /* dont care */, syncEPortIndex)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                  return SNMP_ERR_COMMITFAILED;
              }
            }
            else
            {
              if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE, 0)!=SYNC_E_TYPE_RET_SUCCESS)
              {
                return SNMP_ERR_NOERROR;
              }
            }
            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}


#endif/*#if (SYS_CPNT_SYNCE == TRUE)*/
