#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"

#if (SYS_CPNT_CFM == TRUE)
#include "cli_api_cfm.h"
#include "cfm_pmgr.h"
#include "sys_time.h"
#endif /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
static UI32_T show_eth_cfm_mep_local (CFM_OM_MepInfo_T *mep_result, UI32_T line);
static UI32_T show_eth_cfm_mip_local (CFM_OM_MipInfo_T *mip_result, UI32_T line);
static UI32_T show_eth_cfm_mep_local_detail (CFM_OM_MepInfo_T *result,UI32_T line_num);
static UI32_T show_eth_cfm_error(UI32_T line_num,CFM_OM_Error_T *result);
static UI32_T show_eth_cfm_mp_remote_detail(CFM_OM_RemoteMepCrossCheck_T *result, UI32_T line_num);
static UI32_T show_eth_cfm_port_status(UI32_T lport, CFM_TYPE_CfmStatus_T status, UI32_T line_num);
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Domain
 *-------------------------------------------------------------------------
 * PURPOSE  : Creat the md domain
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Domain(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_TYPE_MhfCreation_T      create_way=SYS_DFLT_CFM_MIP_CREATE;
    char                        *end_p;
    UI32_T                      md_index;

    md_index = strtoul(arg[1], &end_p, 10);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_ETHERNET_CFM_DOMAIN:
            if(arg[2][0] == 'n' || arg[2][0] =='N')
            {
                if(NULL!=arg[6])
                {
                    switch(arg[7][0])
                    {
                        case 'n':
                        case 'N':
                            create_way=CFM_TYPE_MHF_CREATION_NONE;
                            break;

                        case 'e':
                        case 'E':
                            create_way=CFM_TYPE_MHF_CREATION_EXPLICIT;
                            break;

                        case 'd':
                        case 'D':
                            create_way=CFM_TYPE_MHF_CREATION_DEFAULT;
                            break;
                    }
                }

                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetMD(md_index, arg[3], strlen(arg[3]),atoi(arg[5]), create_way ))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure Maintenance Domain.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

            }
            else
            {
                if(NULL!=arg[4])
                {
                    switch(arg[5][0])
                    {
                        case 'n':
                        case 'N':
                            create_way=CFM_TYPE_MHF_CREATION_NONE;
                            break;

                        case 'e':
                        case 'E':
                            create_way=CFM_TYPE_MHF_CREATION_EXPLICIT;
                            break;

                        case 'd':
                        case 'D':
                            create_way=CFM_TYPE_MHF_CREATION_DEFAULT;
                            break;
                    }
                }


                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetMD(md_index, NULL, 0, atoi(arg[3]), create_way ))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure Maintenance Domain.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

            }

            /* if setting is success, change mode */
            ctrl_P->CMenu.domain_index = md_index;
            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_DOMAIN_MODE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_ETHERNET_CFM_DOMAIN:

            if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_DeleteMD(md_index))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure Maintenance Domain.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

    }

#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Domain*/

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Cc_Md
 *-------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Cc_Md(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    /* cmd format: [no] ethernet cfm cc md md_name ma ma_name interval
     */
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_CC_MD:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmInterval(
                arg[0], strlen(arg[0]), arg[2], strlen(arg[2]), atoi(arg[4])))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to configure Continuous Check Message.\r\n");
#endif
            return CLI_NO_ERROR;
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_CC_MD:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmInterval(
                arg[0], strlen(arg[0]), arg[2], strlen(arg[2]), SYS_DFLT_CFM_CCM_INTERVAL))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to configure Continuous Check Message.\r\n");
#endif
            return CLI_NO_ERROR;
        }

        break;
    }
#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Cc_Ma*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Cc_Enable_Md
 *-------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Cc_Enable_Md(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    /* cmd format: [no] ethernet cfm cc enable md md_name ma ma_name
     */
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W5_ETHERNET_CFM_CC_ENABLE_MD:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmStatus(
                                arg[0], strlen(arg[0]), arg[2], strlen(arg[2]),
                                CFM_TYPE_CCM_STATUS_ENABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable transmit the connectivity check message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_ETHERNET_CFM_CC_ENABLE_MD:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmStatus(
                                arg[0], strlen(arg[0]), arg[2], strlen(arg[2]),
                                CFM_TYPE_CCM_STATUS_DISABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable connectivity check message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Cc_Enable_Ma */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Md
 *-------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Md(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_TYPE_AIS_STATUS_T   ais_status = CFM_TYPE_AIS_STATUS_DISABLE;

    /* cmd format: [no] ethernet cfm ais md md_name ma ma_name
     */
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_AIS_MD:
        ais_status = CFM_TYPE_AIS_STATUS_ENABLE;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_AIS_MD:
        break;
    }

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisStatus(
                arg[0], strlen(arg[0]), arg[2], strlen(arg[2]), ais_status))
    {
        CLI_LIB_PrintStr("Failed to set AIS status.\r\n");
        return CLI_NO_ERROR;
    }

#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Suppress_Md
 *-------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Suppress_Alarm_Md(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    char                    *md_name_p  =NULL, *ma_name_p  =NULL;
    UI32_T                  md_name_len =0,    ma_name_len =0, arg_beg_idx =0;
    CFM_TYPE_AIS_STATUS_T   sup_status = CFM_TYPE_AIS_STATUS_DISABLE;

    /* cmd format: [no] ethernet cfm ais suppress alarm md md_name ma ma_name
     */
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W6_ETHERNET_CFM_AIS_SUPPRESS_ALARM_MD:
        sup_status = CFM_TYPE_AIS_STATUS_ENABLE;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_ETHERNET_CFM_AIS_SUPPRESS_ALARM_MD:
        break;
    }

    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisSuppressStatus(
                md_name_p, md_name_len, ma_name_p, ma_name_len, sup_status))
    {
        CLI_LIB_PrintStr("Failed to set suppress AIS.\r\n");
        return CLI_NO_ERROR;
    }

#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}

#else

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Cc_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the CC interval
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Cc_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_CC_MA:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmInterval(
                NULL, 0, arg[0], strlen(arg[0]), atoi(arg[2])))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to configure Continuous Check Message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_CC_MA:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmInterval(
                NULL, 0, arg[0], strlen(arg[0]), SYS_DFLT_CFM_CCM_INTERVAL))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to configure Continuous Check Message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Cc_Ma*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Cc_Enable_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the CC status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Cc_Enable_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W5_ETHERNET_CFM_CC_ENABLE_MA:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_CCM_STATUS_ENABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable transmit the connectivity check message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_ETHERNET_CFM_CC_ENABLE_MA:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetCcmStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_CCM_STATUS_DISABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable connectivity check message.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Cc_Enable_Ma */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     : ethernet cfm ais ma ma_name
 *           no ethernet cfm ais ma ma_name
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_AIS_MA:
        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_AIS_STATUS_ENABLE))
        {
            CLI_LIB_PrintStr("Failed to set AIS status.\r\n");
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_AIS_MA:

        if (CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetAisStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_AIS_STATUS_DISABLE))
        {
            CLI_LIB_PrintStr("Failed to set AIS status.\r\n");
            return CLI_NO_ERROR;
        }
        break;
    }
#endif

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Suppress_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :ethernet cfm ais supress alarm ma ma_name
 *           no ethernet cfm ais supress alarm ma ma_name
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Suppress_Alarm_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W6_ETHERNET_CFM_AIS_SUPPRESS_ALARM_MA:
        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisSuppressStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_AIS_STATUS_ENABLE))
        {
            CLI_LIB_PrintStr("Failed to set suppress AIS.\r\n");
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_ETHERNET_CFM_AIS_SUPPRESS_ALARM_MA:

        if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisSuppressStatus(
                NULL, 0, arg[0], strlen(arg[0]), CFM_TYPE_AIS_STATUS_DISABLE))
        {
            CLI_LIB_PrintStr("Failed to set suppress AIS.\r\n");
            return CLI_NO_ERROR;
        }
        break;
    }
#endif

    return CLI_NO_ERROR;
}

#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Linktrace_Cache
 *-------------------------------------------------------------------------
 * PURPOSE  :Configure the link trace cache size hold time, and status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Linktrace_Cache(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_LINKTRACE_CACHE:

            if(NULL == arg[0])
            {

                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheStatus(CFM_TYPE_LINKTRACE_STATUS_ENABLE))
                {
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr("Failed to configure linktrace cache size.\r\n");
#endif
                 }

                 return CLI_NO_ERROR;
            }

            switch(arg[0][0])
            {
                case 'h':
                case 'H':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheHoldTime(atoi(arg[1])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure linktrace cache hold time.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

                case 's':
                case 'S':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheSize(atoi(arg[1])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure linktrace cache size.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;
            }
        break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_LINKTRACE_CACHE:

            if(NULL == arg[0])
            {
                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheStatus(CFM_TYPE_LINKTRACE_STATUS_DISABLE))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to disable the linktrace cache.\r\n");
#endif
                 }

                 return CLI_NO_ERROR;
            }

            switch(arg[0][0])
            {
                case 'h':
                case 'H':

                     if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheHoldTime(SYS_DFLT_CFM_LINKTRACE_HOLD_TIME))
                     {
#if (SYS_CPNT_EH == TRUE)
                         CLI_API_Show_Exception_Handeler_Msg();
#else
                         CLI_LIB_PrintStr("Failed to configure linktrace cache hold time.\r\n");
#endif
                         return CLI_NO_ERROR;
                     }
                    break;

                case 's':
                case 'S':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetLinkTraceCacheSize(SYS_DFLT_CFM_LINKTRACE_CACHE_SIZE))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure linktrace cache size.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

            }

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Linktrace_Cache*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Enable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the CFM status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_ETHERNET_CFM_ENABLE:
            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMGlobalStatus(CFM_TYPE_CFM_STATUS_ENABLE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to globally enable the Connectivity Fault Management.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_ETHERNET_CFM_ENABLE:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMGlobalStatus(CFM_TYPE_CFM_STATUS_DISABLE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to globally disable the Connectivity Fault Management.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Enable*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Mep_Crosscheck_StartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the cross check start delay
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Mep_Crosscheck_StartDelay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_ETHERNET_CFM_MEP_CROSSCHECK_STARTDELAY:

            if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetCrossCheckStartDelay(atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the cross check start delay.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_ETHERNET_CFM_MEP_CROSSCHECK_STARTDELAY:

            if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetCrossCheckStartDelay(SYS_DFLT_CFM_CROSSCHECK_START_DELAY))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the cross check start delay.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Mep_Crosscheck_StartDelay*/

/* Domain mode:
 * domani index = ctrl_P->CMenu.domain_index;
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Domain_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : create the ma
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Domain_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    char                    *end_p;
    UI32_T                  primary_vid=0;
    UI32_T                  vid_num=0;
    UI32_T                  ma_index;
    CFM_TYPE_MA_Name_T      ma_name_fmt = CFM_TYPE_MA_NAME_CHAR_STRING;
    CFM_TYPE_MhfCreation_T  create_way=CFM_TYPE_MHF_CREATION_DEFAULT;
    UI8_T                   vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1]={0};

    ma_index = strtoul(arg[1], &end_p, 10);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_DOMAIN_CMD_W1_MA:
        {
            /* cmd format:
             *  ma index index name-format {character-string| icc-based}
             *  ma index index name name vlan vid-list [mip-creation type]
             *  ma index index name name
             */
            if((NULL!= arg[2]) &&
               (arg[2][4] == '-'))
            {
                if((arg[3][0] == 'i') || (arg[3][0] == 'I'))
                {
                    ma_name_fmt = CFM_TYPE_MA_NAME_ICC_BASED;
                }

                if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetMANameFormat(
                            ctrl_P->CMenu.domain_index, ma_index, ma_name_fmt))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure MA name format.\r\n");
#endif
                }
            }
            else
            {
                if(NULL!= arg[4])
                {
                    CLI_MGR_ParseCfmVlanList((UI8_T *)arg[5], &primary_vid, &vid_num, vid_list);
                }

                if(NULL!=arg[6])
                {
                    switch(arg[7][0])
                    {
                    case 'n':
                    case 'N':
                        create_way=CFM_TYPE_MHF_CREATION_NONE;
                        break;

                    case 'e':
                    case 'E':
                        create_way=CFM_TYPE_MHF_CREATION_EXPLICIT;
                        break;

                    case 'd':
                    case 'D':
                        create_way=CFM_TYPE_MHF_CREATION_DEFAULT;
                        break;
                    }
                }

                /*set ma*/
                if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetMA(ma_index, arg[3], strlen(arg[3]),ctrl_P->CMenu.domain_index,
                                                            (UI16_T) primary_vid, vid_num, vid_list, create_way))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure the maintenance association.\r\n");
#endif
                }
            }
        }
        break;

        case PRIVILEGE_CFG_DOMAIN_CMD_W2_NO_MA:
            /* cmd format:
             *  no ma index index name-format
             *  no ma index index vlan vid
             *  no ma index index
             */
            if(NULL!= arg[2])
            {
                if((arg[2][0] == 'n') || (arg[2][0] == 'N'))
                {
                    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetMANameFormat(
                                ctrl_P->CMenu.domain_index, ma_index, ma_name_fmt))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure MA name format.\r\n");
#endif
                    }
                }
                else if ((arg[2][0]=='v') || (arg[2][0] == 'V'))
                {
                    if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_DeleteMAVlan(ctrl_P->CMenu.domain_index, ma_index, atoi(arg[3])))
                    {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to delete the vlan.\r\n");
#endif
                    }
                }
            }
            else if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_DeleteMA(ctrl_P->CMenu.domain_index, ma_index))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete the maintenance association.\r\n");
#endif
            }

            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Domain_Ma*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Domain_Mep_AchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the archive hold time in minutes
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Domain_Mep_AchiveHoldTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_DOMAIN_CMD_W2_MEP_ARCHIVEHOLDTIME:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetArchiveHoldTime(ctrl_P->CMenu.domain_index, atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the archive-hold time.\r\n");
#endif
                return CLI_NO_ERROR;
            }

        break;

        case PRIVILEGE_CFG_DOMAIN_CMD_W3_NO_MEP_ARCHIVEHOLDTIME:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetArchiveHoldTime(ctrl_P->CMenu.domain_index, SYS_DFLT_CFM_ARCHIVE_HOLD_TIME ))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the archive-hold time.\r\n");
#endif
                return CLI_NO_ERROR;
            }

        break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Domain_Mep_AchiveHoldTime*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Domain_Mep_Crosscheck_Mpid
 *-------------------------------------------------------------------------
 * PURPOSE  : set the cross check mep
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Domain_Mep_Crosscheck_Mpid(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_DOMAIN_CMD_W3_MEP_CROSSCHECK_MPID:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_AddRemoteMep(ctrl_P->CMenu.domain_index, atoi(arg[0]), arg[2], strlen(arg[2])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to add the crosscheck remote mep.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

        case PRIVILEGE_CFG_DOMAIN_CMD_W4_NO_MEP_CROSSCHECK_MPID:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_DeleteRemoteMep(ctrl_P->CMenu.domain_index, atoi(arg[0]), arg[2], strlen(arg[2])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete the crosscheck remote mep.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Domain_Mep_Crosscheck_Mpid*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Domain_Mep_FaultNotify
 *-------------------------------------------------------------------------
 * PURPOSE  : set the fault alarm attributes
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Domain_Mep_FaultNotify(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_DOMAIN_CMD_W2_MEP_FAULTNOTIFY:

            switch(arg[0][0])
            {
                case 'l':
                case 'L':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyLowestPriority(ctrl_P->CMenu.domain_index, atoi(arg[1])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification lowest priority.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

                case 'a':
                case 'A':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyAlarmTime(ctrl_P->CMenu.domain_index, atoi(arg[1])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification alarm time..\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

                case 'r':
                case 'R':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyRestTime(ctrl_P->CMenu.domain_index, atoi(arg[1])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification reset time.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

            }

            break;

        case PRIVILEGE_CFG_DOMAIN_CMD_W3_NO_MEP_FAULTNOTIFY:
            switch((arg[0][0]))
            {
                case 'l':
                case 'L':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyLowestPriority(ctrl_P->CMenu.domain_index, SYS_DFLT_CFM_FNG_LOWEST_ALARM_PRI))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification lowest priority.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                break;

                case 'a':
                case 'A':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyAlarmTime(ctrl_P->CMenu.domain_index, SYS_DFLT_CFM_FNG_LOWEST_ALARM_TIME))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification alarm time..\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

                case 'r':
                case 'R':

                    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetFaultNotifyRestTime(ctrl_P->CMenu.domain_index, SYS_DFLT_CFM_FNG_LOWEST_RESET_TIME))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the fault notification reset time.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                break;

            }

        break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Domain_Mep_FaultNotify*/

/* interface mode */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_PortEnable_Eth
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port cfm status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_PortEnable_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_ETHERNET_CFM_PORTENABLE:

                    if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMPortStatus(lport, CFM_TYPE_CFM_STATUS_ENABLE))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to enable the Connectivity Fault Management on this interface.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_ETHERNET_CFM_PORTENABLE:

                    if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMPortStatus(lport, CFM_TYPE_CFM_STATUS_DISABLE))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to disable the Connectivity Fault Management on this interface.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    break;
                }
            }
        }
    }
#endif

  return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_PortEnable_Eth*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_PortEnable_Pch
 *-------------------------------------------------------------------------
 * PURPOSE  : set the port channel cfm status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_PortEnable_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T              lport=0;
    UI32_T              verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_EthStatus_T verify_ret;

    /* sync from ASE4512BBS-FLF-P5-01125
     */
    if(CLI_API_TRUNK_OK != (verify_ret = verify_trunk(verify_trunk_id, &lport)))
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_ETHERNET_CFM_PORTENABLE:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMPortStatus(lport, CFM_TYPE_CFM_STATUS_ENABLE))
            {
#if (SYS_CPNT_EH == TRUE)
                 CLI_API_Show_Exception_Handeler_Msg();
#else
                 CLI_LIB_PrintStr("Failed to enable the Connectivity Fault Management on this interface.\r\n");
#endif
                 return CLI_NO_ERROR;
             }

             break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_ETHERNET_CFM_PORTENABLE:

            if(CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCFMPortStatus(lport, CFM_TYPE_CFM_STATUS_DISABLE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable the Connectivity Fault Management on this interface.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            break;

    }

#endif
     return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_PortEnable_Pch*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Mep_Eth
 *-------------------------------------------------------------------------
 * PURPOSE  : set the mep
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Mep_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0, verify_ret=0;
    UI32_T  i = 0, lport = 0;
    UI32_T  md_name_len =0,    ma_name_len =0, arg_beg_idx =3;
    char    *md_name_p  =NULL, *ma_name_p  =NULL;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if ( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);

                continue;
            }
            else
            {
#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
                md_name_p = arg[arg_beg_idx];
                ma_name_p = arg[arg_beg_idx+2];
                arg_beg_idx += 2;
#else
                ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

                if (NULL != md_name_p)
                    md_name_len = strlen(md_name_p);
                if (NULL != ma_name_p)
                    ma_name_len = strlen(ma_name_p);

                /* cmd format: [no] ethernet cfm mep mpid mp_id md md_name ma ma_name [up]
                 */
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_ETHERNET_CFM_MEP:
                    {
                        CFM_TYPE_MP_Direction_T direction = CFM_TYPE_MP_DIRECTION_DOWN;

                        if (  (NULL != arg[arg_beg_idx+1])
                            &&((arg[arg_beg_idx+1][0] == 'U')||(arg[arg_beg_idx+1][0]=='u'))
                           )
                        {
                            direction = CFM_TYPE_MP_DIRECTION_UP;
                        }

                        if (CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_AddnewMEP(
                                lport, atoi(arg[1]), md_name_p, md_name_len,
                                ma_name_p, ma_name_len, direction))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to configure Maintenance association End Point (MEP).\r\n");
#endif
                            return CLI_NO_ERROR;
                         }

                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_ETHERNET_CFM_MEP:

                    if (CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_DeleteMEP(
                                    lport, atoi(arg[1]), md_name_p, md_name_len,
                                    ma_name_p, ma_name_len))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to configure the maintenance association point (MEP).\r\n");
#endif
                        return CLI_NO_ERROR;
                    }
                    break;
                }
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Mep_Eth*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Mep_Pch
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the mep on port channel
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Mep_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T  lport=0;
    UI32_T  md_name_len =0,    ma_name_len =0, arg_beg_idx =3;
    char    *md_name_p  =NULL, *ma_name_p  =NULL;

    if(FALSE == SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &lport))
    {
        #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
        #else
        CLI_LIB_PrintStr("This portchannel does not exist.\r\n");
        #endif
        return CLI_NO_ERROR;
    }

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];
    arg_beg_idx += 2;
#else
    ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_ETHERNET_CFM_MEP:
        {
            CFM_TYPE_MP_Direction_T direction=CFM_TYPE_MP_DIRECTION_DOWN;

            if (  (NULL != arg[arg_beg_idx+1])
                &&((arg[arg_beg_idx+1][0] == 'U')||(arg[arg_beg_idx+1][0]=='u'))
               )
            {
                direction = CFM_TYPE_MP_DIRECTION_UP;
            }

            if (CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_AddnewMEP(
                    lport, atoi(arg[1]), md_name_p, md_name_len,
                    ma_name_p, ma_name_len, direction))
             {
#if (SYS_CPNT_EH == TRUE)
                 CLI_API_Show_Exception_Handeler_Msg();
#else
                 CLI_LIB_PrintStr("Failed to configure the maintenance association point (MEP).\r\n");
#endif
                 return CLI_NO_ERROR;
             }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_ETHERNET_CFM_MEP:

        if (CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_DeleteMEP(
                lport, atoi(arg[1]), md_name_p, md_name_len,
                ma_name_p, ma_name_len))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to configure the maintenance association point (MEP).\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Mep_Pch*/


/* EXEC mode */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Mep_Crosscheck
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the corss check status
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Mep_Crosscheck(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T  md_name_len =0,    ma_name_len =0, arg_beg_idx =2;
    char    *md_name_p  =NULL, *ma_name_p  =NULL;

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];
#else
    ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    /* cmd format: ethernet cfm mep crosscheck {enable | disable}
     *                      md md_name ma ma_name
     */
    switch(arg[0][0])
    {
    case 'e':
    case 'E':

        if (CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCrossCheckStatus(
                        CFM_TYPE_CROSS_CHECK_STATUS_ENABLE,
                        md_name_p, md_name_len, ma_name_p, ma_name_len))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable the cross check.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

    case 'd':
    case 'D':

        if (CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetCrossCheckStatus(
                        CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
                        md_name_p, md_name_len, ma_name_p, ma_name_len))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable the cross check.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Mep_Crosscheck*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Clear_Ethernet_Cfm_MP_Remote
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear the archive databse which store the remtoe mep info
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Clear_Ethernet_Cfm_MP_Remote(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    if(NULL == arg[0])
    {
        if(CFM_TYPE_CONFIG_SUCCESS!=  CFM_PMGR_ClearRemoteMepAll())
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to clear the connectivity check message database.\r\n");
#endif
        }

        return CLI_NO_ERROR;
    }

    switch(arg[0][0])
    {
        case 'd':
        case 'D':

            if(CFM_TYPE_CONFIG_SUCCESS!=  CFM_PMGR_ClearRemoteMepByDomain(arg[1], strlen(arg[1])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to clear the connectivity check message database.\r\n");
#endif
                return CLI_NO_ERROR;
            }

        break;

        case 'l':
        case 'L':

        if(CFM_TYPE_CONFIG_SUCCESS!=  CFM_PMGR_ClearRemoteMepByLevel(atoi(arg[1])))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to clear the connectivity check message database.\r\n");
#endif
            return CLI_NO_ERROR;
        }

        break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Clear_Ethernet_Cfm_MP_Remote */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Clear_Ethernet_Cfm_Errors
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the error database
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Clear_Ethernet_Cfm_Errors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    if(NULL == arg[0])
    {
        if(CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_ClearErrorList())
        {
            #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
            #else
            CLI_LIB_PrintStr("Failed the clear the mep configure errors.\r\n");
            #endif
        }

        return CLI_NO_ERROR;
    }

    switch(arg[0][0])
    {
        case 'd':
        case 'D':

            if(CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_ClearErrorsListByMdNameOrLevel(arg[1], strlen(arg[1]), CFM_TYPE_MD_LEVEL_NONE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed the clear the mep configure errors.\r\n");
                #endif
                return CLI_NO_ERROR;
            }

            break;

        case 'l':
        case 'L':

            if(CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_ClearErrorsListByMdNameOrLevel(NULL, 0, atoi(arg[1])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed the clear the mep configure errors.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Clear_Ethernet_Cfm_Errors*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Clear_Eth_Cfm_LinktraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the linktrace cache
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Clear_Eth_Cfm_LinktraceCache(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_ClearLinktraceCache())
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed the clear linktrace messages.\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Clear_Eth_Cfm_LinktraceCache*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_LinkTrace
 *-------------------------------------------------------------------------
 * PURPOSE  : transmit the link trace message
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :ethernet cfm linktrace [src-mep smpid ] {dest-mep dmpid | mac-address} ma ma-name ttl num
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_LinkTrace(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    char    *md_name_p = NULL, *ma_name_p = NULL;
    UI32_T  src_mep=0, dest_mep=0, ttl=64, md_name_len =0, ma_name_len=0, arg_idx;
    UI16_T  pkt_pri  = CFM_TYPE_DEF_PDU_PRIORITY;
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN]={0};

    /* cmd format: ethernet cfm linktrace
     *               [src-mep smpid] {dest-mep dmpid | dmac}
     *               md md_name ma ma_name [ttl ttl] [priority pkt_pri]
     */
    switch(arg[0][0])
    {
    case 'd':
    case 'D':
        dest_mep = atoi(arg[1]);
#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
        md_name_p = arg[3];
        ma_name_p = arg[5];
        arg_idx   = 6;
#else
        ma_name_p = arg[3];
        arg_idx   = 4;
#endif
        break;
    case 's':
    case 'S':
        src_mep=atoi(arg[1]);
        switch(arg[2][0])
        {
        case 'd':
        case 'D':
            dest_mep = atoi(arg[3]);
#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
            md_name_p = arg[5];
            ma_name_p = arg[7];
            arg_idx   = 8;
#else
            ma_name_p = arg[5];
            arg_idx   = 6;
#endif
            break;
        default:
            CLI_LIB_ValsInMac(arg[2], mac_addr);
#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
            md_name_p = arg[4];
            ma_name_p = arg[6];
            arg_idx   = 7;
#else
            ma_name_p = arg[4];
            arg_idx   = 5;
#endif
            break;
        }
        break;
    default:
        CLI_LIB_ValsInMac(arg[0], mac_addr);
#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
        md_name_p = arg[2];
        ma_name_p = arg[4];
        arg_idx   = 5;
#else
        ma_name_p = arg[2];
        arg_idx   = 3;
#endif
       break;
    }

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);

    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    for ( ; arg[arg_idx] != NULL; )
    {
        switch(arg[arg_idx][0])
        {
        case 'p':
        case 'P':
            pkt_pri = atoi(arg[arg_idx+1]);
            arg_idx+=2;
            break;
        case 't':
        case 'T':
            ttl = atoi(arg[arg_idx+1]);
            arg_idx+=2;
            break;
        default: /* should not occur */
            return CLI_ERR_INTERNAL;
        }
    }

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_TransmitLinktrace(
                        src_mep, dest_mep, mac_addr,
                        md_name_p, md_name_len, ma_name_p, ma_name_len, ttl, pkt_pri))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to transmit linktrace messages.\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_LinkTrace*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Loopback
 *-------------------------------------------------------------------------
 * PURPOSE  : transmit the loopback message
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Loopback(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T  md_name_len =0, ma_name_len =0, arg_idx =0,
            src_mep_id=0, dst_mep_id =0, line_num =0;
    UI32_T  counts   = CFM_TYPE_DEF_LBM_COUNT,
            pkt_size = CFM_TYPE_DEF_LBM_PKTSIZE,
            timeout  = CFM_TYPE_DEF_LBM_TIMEOUT;
    UI16_T  pkt_pri  = CFM_TYPE_DEF_PDU_PRIORITY,
            pattern  = CFM_TYPE_DEF_LBM_PATTERN;
    char    *md_name_p = NULL, *ma_name_p = NULL;
    UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN] ={0};
    char    buff[CLI_DEF_MAX_BUFSIZE]={0};

    /* cmd format: ethernet cfm loopback
     *               [src-mep smpid] {dest-mep dmpid | dmac} md md_name ma ma_name
     *               [count count] [size bytes] [pattern hex_num] [priority pri]
     */
    for (arg_idx =0; arg[arg_idx] != NULL;)
    {
        switch(arg[arg_idx][0])
        {
        case 'p':
        case 'P':
            if ((arg[arg_idx][1] == 'a') || (arg[arg_idx][1] == 'A'))
            {
                pattern = CLI_LIB_AtoUl(arg[arg_idx+1], 16);
            }
            else
            {
                pkt_pri = atoi(arg[arg_idx+1]);
            }
            arg_idx+=2;
            break;
        case 's':
        case 'S':
            if (arg_idx == 0)
            {
                src_mep_id = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            else
            {
                pkt_size = atoi(arg[arg_idx+1]);
                arg_idx+=2;
                break;
            }
            break;
        case 'm':
        case 'M':
            if ((arg[arg_idx][1] == 'a') || (arg[arg_idx][1] == 'A'))
            {
                ma_name_p   = arg[arg_idx+1];
                ma_name_len = strlen(ma_name_p);
                arg_idx+=2;
            }
            else if ((arg[arg_idx][1] == 'd') || (arg[arg_idx][1] == 'D'))
            {
                md_name_p   = arg[arg_idx+1];
                md_name_len = strlen(md_name_p);
                arg_idx+=2;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;
        case 'c': /* may be mac */
        case 'C':
            if (TRUE == CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar))
            {
                arg_idx++;
            }
            else
            {
                counts = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            break;
        case 'd': /* may be mac */
        case 'D':
            if (TRUE == CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar))
            {
                arg_idx++;
            }
            else
            {
                dst_mep_id = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            break;
        default:
            if ((arg_idx == 0) || (arg_idx == 2))
            {
                CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar);
                arg_idx++;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;
        }
    }

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_DoThrptMeasureByLBM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_p, md_name_len, ma_name_p, ma_name_len,
                        counts, pkt_size, pattern, pkt_pri))
    {
        CLI_LIB_PrintStr("Failed to transmit loopback messages.\r\n");
    }
    else
    {
        UI32_T  real_send, rcvd_1sec, rcvd_total;
        UI8_T   res_bmp =0, print_bmp =0;

        sprintf(buff, "Type ESC to abort.\r\n");
        PROCESS_MORE(buff);

        while (TRUE == CFM_OM_GetThrpMeasureResult(
                        &real_send, &rcvd_1sec, &rcvd_total, &res_bmp))
        {
            if (res_bmp & CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS)
            {
                if (!(print_bmp & CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS))
                {
                    sprintf(buff, "Sending %ld Ethernet CFM loopback message, timeout is %ld sec.\r\n", (long)real_send, (long)timeout);
                    PROCESS_MORE(buff);

                    print_bmp |= CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS;
                }
            }

            if (res_bmp & CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC)
            {
                if (!(print_bmp & CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC))
                {
                    sprintf(buff, "Received %ld Ethernet CFM loopback message in 1 sec.\r\n",  (long)rcvd_1sec);
                    PROCESS_MORE(buff);

                    print_bmp |= CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC;
                }
            }

            /* NOTE: may need to improve...
             *  if timeout is able to be configured,
             *  may not display this message if timout is 1 sec.
             */
            if (res_bmp & CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT)
            {
                if (!(print_bmp & CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT))
                {
                    sprintf(buff, "Received %ld Ethernet CFM loopback message in %lu secs.\r\n", (long)rcvd_total, (unsigned long)timeout);
                    PROCESS_MORE(buff);

                    if (real_send > 0)
                    {
                        sprintf(buff, "Success rate is %ld%% (%ld/%ld).\r\n",
                                                    (long)(100 * rcvd_total /real_send), (long)rcvd_total, (long)real_send);
                        PROCESS_MORE(buff);
                    }
                    else
                    {
                        sprintf(buff, "Success rate is 0%%.\r\n");
                        PROCESS_MORE(buff);
                    }
                }
                break;
            }

            if (ctrl_P->sess_type == CLI_TYPE_TELNET)
            {
                if (CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) /*ESC*/
                {
                    CFM_PMGR_AbortThrptMeasureByLBM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_p, md_name_len, ma_name_p, ma_name_len);
                }
            }
            else
            {
                if (CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) /*ESC*/
                {
                    CFM_PMGR_AbortThrptMeasureByLBM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_p, md_name_len, ma_name_p, ma_name_len);
                }
            }

            SYSFUN_Sleep(30);
        }
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Ethernet_Cfm_Loopback*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_Md
 *-------------------------------------------------------------------------
 * PURPOSE  : show the md infomation
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_Md(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T              line_num=0, nxt_md_index=0;
    CFM_OM_MdInfo_T     result;
    char               buff[CLI_DEF_MAX_BUFSIZE]={0};
    char               *mip_creation[]={"","None","Default","Explicit"};
    UI8_T               get_level = 0xff;
    UI8_T               tmp_ch=0;

    sprintf(buff,"MD Index MD Name                    Level MIP Creation Archive Hold Time (min.)\r\n");
    PROCESS_MORE(buff);
               /* 8        26                         5     12           24 */
    sprintf(buff,"-------- -------------------------- ----- ------------ ------------------------\r\n");
    PROCESS_MORE(buff);

    if(NULL != arg[0])
    {
        get_level = atoi(arg[1]);
    }

    while(TRUE == CFM_OM_GetNextMdInfo(&nxt_md_index, &result))
    {
        if((get_level == 0xff) || (get_level == result.level))
        {
            if (result.name_len > 26)
            {
                tmp_ch = result.name_a[26];
                result.name_a[26] = '\0';
            }

            sprintf(buff,"%8lu %-26s %5d %-12s %24ld\r\n",
                    (unsigned long)result.index, result.name_a, result.level,
                    mip_creation[result.mhf_creation], (long)result.mep_archive_hold_time);
            PROCESS_MORE(buff);

            if (result.name_len > 26)
            {
                result.name_a[26] = tmp_ch;
                sprintf(buff,"%8s %-26s\r\n", "", &result.name_a[26]);
                PROCESS_MORE(buff);
            }
        }
    }
#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_Md*/

#if (SYS_CPNT_CFM == TRUE)
static UI8_T *cli_api_cfm_copy_name_by_len(
    UI8_T   *src_p,
    UI8_T   src_len,
    UI8_T   src_beg,
    UI8_T   *dst_p,
    UI8_T   dst_len)
{
    UI8_T   *ret_p = NULL;
    UI8_T   copy_len;

    if ((NULL != src_p) && (NULL != dst_p))
    {
        if (src_len > src_beg)
        {
            if ((src_len - src_beg) > dst_len)
            {
                copy_len = dst_len;
            }
            else
            {
                copy_len = src_len - src_beg;
            }

            memcpy(dst_p, src_p+src_beg, copy_len);
            dst_p[copy_len]= '\0';
        }
        else
        {
            dst_p[0] = '\0';
        }
        ret_p = dst_p;
    }

    return ret_p;
}
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_Ma
 *-------------------------------------------------------------------------
 * PURPOSE  : show the ma information
 * INPUT    : cmd_idx - command index
 *            arg[]   - argument array, two dimension array.
 *                      first dimension means arg order
 *                      second dimension means arg character order
 * OUTPUT   : None
 * RETURN   : help message indication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_Ma(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T          line_num=0, nxt_md_index=0, nxt_ma_index=0;
    CFM_OM_MaInfo_T ma_result;
    CFM_OM_MdInfo_T md_result;
    char            *mip_creation[]={"","None","Default","Explicit"};
    char            buff[CLI_DEF_MAX_BUFSIZE]={0},
                    tmp_p_vid[15]={0};
    char            tmp_md_name[23], tmp_ma_name[23];
    UI8_T           get_level = 0xff, loop_idx, name_beg_id;

    sprintf(buff,"MD Name       MA Index   MA Name       Primary VID    CC Interval MIP Creation\r\n");
    PROCESS_MORE(buff);
               /* 13            10         13            14             11          12 */
    sprintf(buff,"------------- ---------- ------------- -------------- ----------- ------------\r\n");
    PROCESS_MORE(buff);

    if (NULL != arg[0])
    {
        get_level = atoi(arg[1]);
    }

    while (TRUE== CFM_OM_GetNextMdInfo(&nxt_md_index, &md_result))
    {
        while (TRUE == CFM_OM_GetNextMaInfo(nxt_md_index, &nxt_ma_index, &ma_result))
        {
            if ((get_level == 0xff) || (get_level == ma_result.md_level))
            {
                if (0 == ma_result.primary_vid)
                {
                    sprintf(tmp_p_vid, "%-14s", "Not Configured");
                }
                else
                {
                    sprintf(tmp_p_vid, "%14d", ma_result.primary_vid);
                }

                name_beg_id = 0;
                for (loop_idx =0; loop_idx <4; loop_idx++)
                {
                    cli_api_cfm_copy_name_by_len(
                        ma_result.md_name_a, ma_result.md_name_len, name_beg_id,
                        (UI8_T *)tmp_md_name, 13);

                    cli_api_cfm_copy_name_by_len(
                        ma_result.ma_name_a, ma_result.ma_name_len, name_beg_id,
                        (UI8_T *)tmp_ma_name, 13);

                    if (loop_idx == 0)
                    {
                        sprintf(buff,"%-13s %10lu %-13s %-14s %11d %-12s\r\n",
                                tmp_md_name, (unsigned long)ma_result.ma_index,
                                tmp_ma_name, tmp_p_vid, ma_result.interval,
                                mip_creation[ma_result.mhf_creation]);
                    }
                    else
                    {
                        sprintf(buff,"%-13s %10s %-13s\r\n",
                                tmp_md_name, "", tmp_ma_name);
                    }
                    PROCESS_MORE(buff);

                    name_beg_id   += 13;

                    if ((ma_result.ma_name_len <= name_beg_id) &&
                        (ma_result.md_name_len <= name_beg_id))
                        break;
                }
            }
        }

        nxt_ma_index = 0;
    }
#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_Ma*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_LinktraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  :show the content of lin trace cache
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_LinktraceCache(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_OM_LinktraceReply_T linktrace_reply;
    UI32_T                  md_index=0,ma_index=0,mep_id=0,seq_num=0,rcvd_order=0;
    UI32_T                  line_num=0;
    char                    buff[CLI_DEF_MAX_BUFSIZE]={0};
    char                    buff2[CLI_DEF_MAX_BUFSIZE]={0}, tmp_ma_name[15], name_beg_id;
    UI8_T                   null_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

    sprintf(buff,"Hops MA             IP / Alias              Ingress MAC       Ing. Action Relay\r\n");
    PROCESS_MORE(buff);
    sprintf(buff,"                    Forwarded               Egress MAC        Egr. Action\r\n");
    PROCESS_MORE(buff);
               /* 4    14             23                      17                11          5 */
    sprintf(buff,"---- -------------- ----------------------- ----------------- ----------- -----\r\n");
    PROCESS_MORE(buff);

    memset(&linktrace_reply, 0, sizeof(linktrace_reply));

    while(TRUE==CFM_OM_GetNextLinktraceReplyInfo(&md_index, &ma_index, &mep_id, &seq_num, &rcvd_order, &linktrace_reply))
    {
        name_beg_id = 0;

        /*hops ,host*/
        cli_api_cfm_copy_name_by_len(
            linktrace_reply.ma_name_a, linktrace_reply.ma_name_len, name_beg_id, (UI8_T *)tmp_ma_name, 14);
        name_beg_id += 14;
        sprintf(buff,"%4ld %-14s ",(long)linktrace_reply.hops, tmp_ma_name);

        /*mangement address*/
        if(SYS_ADPT_IPV4_ADDR_LEN == linktrace_reply.mgmt_addr_len)
        {
            char tmp[SYS_ADPT_IPV4_ADDR_LEN*3+3+1]={0};/*IP len + '.' no. + '\0\*/
            sprintf(tmp, "%d.%d.%d.%d", linktrace_reply.mgmt_addr_a[0], linktrace_reply.mgmt_addr_a[1], linktrace_reply.mgmt_addr_a[2], linktrace_reply.mgmt_addr_a[3]);
            sprintf(buff2,"%-23s", tmp);
        }
        else if(SYS_ADPT_IPV6_ADDR_LEN == linktrace_reply.mgmt_addr_len)
        {
            char tmp[SYS_ADPT_IPV6_ADDR_LEN+7+1]={0};/*IP len + '.' no. + '\0\*/

            sprintf(tmp, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                       linktrace_reply.mgmt_addr_a[0],
                       linktrace_reply.mgmt_addr_a[1],
                       linktrace_reply.mgmt_addr_a[2],
                       linktrace_reply.mgmt_addr_a[3],
                       linktrace_reply.mgmt_addr_a[4],
                       linktrace_reply.mgmt_addr_a[5],
                       linktrace_reply.mgmt_addr_a[6],
                       linktrace_reply.mgmt_addr_a[7],
                       linktrace_reply.mgmt_addr_a[8],
                       linktrace_reply.mgmt_addr_a[9],
                       linktrace_reply.mgmt_addr_a[10],
                       linktrace_reply.mgmt_addr_a[11],
                       linktrace_reply.mgmt_addr_a[12],
                       linktrace_reply.mgmt_addr_a[13],
                       linktrace_reply.mgmt_addr_a[14],
                       linktrace_reply.mgmt_addr_a[15]
                       );
            sprintf(buff2,"%23s", tmp);
        }
        else if(0!=linktrace_reply.mgmt_addr_len)
        {
            sprintf(buff2,"%-23s", linktrace_reply.mgmt_addr_a);
        }
        else
        {
            sprintf(buff2, "%23s", "");
        }
        strcat(buff,buff2);

        /*ingress port mac*/
        if(memcmp(linktrace_reply.ingress_port_mac_a, null_mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            sprintf(buff2, " %02X-%02X-%02X-%02X-%02X-%02X",
                    linktrace_reply.ingress_port_mac_a[0],
                    linktrace_reply.ingress_port_mac_a[1],
                    linktrace_reply.ingress_port_mac_a[2],
                    linktrace_reply.ingress_port_mac_a[3],
                    linktrace_reply.ingress_port_mac_a[4],
                    linktrace_reply.ingress_port_mac_a[5]);
        }
        else
            sprintf(buff2, " %17s", "");

        strcat(buff,buff2);

        /*ingress port action*/
        switch(linktrace_reply.ingress_action)
        {
            case CFM_TYPE_INGRESS_ACTION_OK:
                sprintf(buff2," %-11s","ingOk");
                break;

            case CFM_TYPE_INGRESS_ACTION_VID:
                sprintf(buff2," %-11s","ingVid");
                break;

            case CFM_TYPE_INGRESS_ACTION_BLOCKED:
                sprintf(buff2," %-11s","ingBlocked");
                break;

            case CFM_TYPE_INGRESS_ACTION_DOWN:
                sprintf(buff2," %-11s","ingDown");
                break;

            default:
                sprintf(buff2," %11s", "");
        }
        strcat(buff,buff2);

        /*relay action*/
        switch(linktrace_reply.relay_action)
        {
            case CFM_TYPE_RELAY_ACTION_FDB:
                sprintf(buff2," %-5s\r\n","FDB");
                break;

            case CFM_TYPE_RELAY_ACTION_MPDB:
                sprintf(buff2," %-5s\r\n","MPDB");
                break;

            case CFM_TYPE_RELAY_ACTION_HIT:
                sprintf(buff2," %-5s\r\n","Hit");
                break;

            default:
                sprintf(buff2," %5s\r\n", "");

        }
        strcat(buff,buff2);
        PROCESS_MORE(buff);

        while (name_beg_id < linktrace_reply.ma_name_len)
        {
            cli_api_cfm_copy_name_by_len(
                linktrace_reply.ma_name_a, linktrace_reply.ma_name_len, name_beg_id, (UI8_T *)tmp_ma_name, 14);
            name_beg_id += 14;
            sprintf(buff, "%-14s\r\n", tmp_ma_name);
            PROCESS_MORE(buff);
        }

        /*forwarded or not forwarded*/
        sprintf(buff, "                    %-23s",
                    (linktrace_reply.forwarded?"Forwarded":"Not Forwarded"));

        /*egress port*/
        if(memcmp(linktrace_reply.egress_port_mac_a, null_mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            sprintf(buff2," %02X-%02X-%02X-%02X-%02X-%02X",
                  linktrace_reply.egress_port_mac_a[0],
                  linktrace_reply.egress_port_mac_a[1],
                  linktrace_reply.egress_port_mac_a[2],
                  linktrace_reply.egress_port_mac_a[3],
                  linktrace_reply.egress_port_mac_a[4],
                  linktrace_reply.egress_port_mac_a[5]);
        }
        else
            sprintf(buff2, " %17s", "");

        strcat(buff,buff2);

        /*egress port action*/
        switch(linktrace_reply.egress_action)
        {
            case CFM_TYPE_EGRESS_ACTION_OK:
                sprintf(buff2," %-11s\r\n","egrOk");
                break;

            case CFM_TYPE_EGRESS_ACTION_VID:
                sprintf(buff2," %-11s\r\n","egrVid");
                break;

            case CFM_TYPE_EGRESS_ACTION_BLOCKED:
                sprintf(buff2," %-11s\r\n","egrBlocked");
                break;

            case CFM_TYPE_EGRESS_ACTION_DOWN:
                sprintf(buff2," %-11s\r\n","egrDown");
                break;

            default:
                sprintf(buff2," %11s\r\n", "");
                break;
        }

        strcat(buff,buff2);
        PROCESS_MORE(buff);
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_LinktraceCache*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_MP_Local
 *-------------------------------------------------------------------------
 * PURPOSE  :show the local mp info
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_MP_Local(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T              line_num =0, nxt_md_index=0, nxt_ma_index=0,
                        nxt_mep_id=0, nxt_lport=0;
    CFM_OM_MepInfo_T    mep_result;
    CFM_OM_MipInfo_T    mip_result;
    char               buff[CLI_DEF_MAX_BUFSIZE]={0};

    switch(arg[0][1])
    {
        case 'i':
        case 'I':
        { /*show ethernet cfm maintenance-points local  mip [domain domain-name | level level-id]*/
            sprintf(buff,"MD Name                 Level MA Name                 VLAN Interface\r\n");
            PROCESS_MORE(buff);
                      /*  23                      5     23                      4    9*/
            sprintf(buff,"----------------------- ----- ----------------------- ---- ---------\r\n");
            PROCESS_MORE(buff);

            if(NULL == arg[1])
            {
                while(TRUE == CFM_OM_GetNextMipInfo(&nxt_md_index, &nxt_ma_index, &nxt_lport, &mip_result))
                {
                    if(JUMP_OUT_MORE == (line_num=show_eth_cfm_mip_local(&mip_result,line_num)))
                    {
                        return CLI_NO_ERROR;
                    }
                }

                return CLI_NO_ERROR;
            }

            switch(arg[1][0])
            {
                case 'd':
                case 'D':

                    while(TRUE == CFM_OM_GetNextMipInfo(&nxt_md_index, &nxt_ma_index, &nxt_lport, &mip_result))
                    {
                        if(!memcmp(mip_result.md_name_a, arg[2], strlen(arg[2])))
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mip_local(&mip_result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    break;

                case 'l':
                case 'L':

                    while(TRUE == CFM_OM_GetNextMipInfo(&nxt_md_index, &nxt_ma_index, &nxt_lport, &mip_result))
                    {
                        if(mip_result.md_level == atoi(arg[2]))
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mip_local(&mip_result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    break;

            }
            break;

        }
        case 'e':
        case 'E':
        {/*show ethernet cfm maintenance-points local  mep [interface interface |domain domain-name | level level-id]*/

            sprintf(buff,"MPID MD Name          Level Direct VLAN Interface CC Status MAC Address\r\n");
            PROCESS_MORE(buff);
                       /* 4    16               5     6      4    9         9         17 */
            sprintf(buff,"---- ---------------- ----- ------ ---- --------- --------- -----------------\r\n");
            PROCESS_MORE(buff);

            if(NULL == arg[1])
            {
                while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, 0, &mep_result))
                {
                    if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local(&mep_result,line_num)))
                    {
                        return CLI_NO_ERROR;
                    }
                }

                return CLI_NO_ERROR;
            }

            switch(arg[1][0])
            {
                case 'i':
                case 'I':
                {
                    UI32_T verify_unit,verify_port,lport;
                    CLI_API_EthStatus_T verify_ret;

                    if(arg[2][0] == 'e' || arg[2][0] == 'E')
                    {
                        if (isdigit(arg[3][0]))
                        {
                            verify_unit = atoi(arg[3]);
                            verify_port = atoi(strchr(arg[3], '/') + 1);
                        }
                        else
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }

                        verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                        if(CLI_API_ETH_OK!=verify_ret)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                    else if(arg[2][0] == 'p'||arg[2][0]=='P')
                    {
                        if (isdigit(arg[3][0]))
                        {
                            verify_port=atoi(arg[3]);
                        }
                        else
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }

                        verify_ret=verify_trunk(verify_port, &lport);

                        if(CLI_API_ETH_OK!=verify_ret)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                    else
                    {
                        /* should not occur */
                        return CLI_ERR_INTERNAL;
                    }

                    while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,lport, &mep_result))
                    {
                        if(mep_result.lport == lport)
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local(&mep_result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    break;

                }
                case 'd':
                case 'D':
                    while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,0, &mep_result))
                    {
                        if(!memcmp(mep_result.md_name_a,arg[2],strlen(arg[2])) )
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local(&mep_result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    break;

                case 'l':
                case 'L':
                    while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,0, &mep_result))
                    {
                        if(mep_result.md_level == atoi(arg[2]))
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local(&mep_result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    break;

            }/*end switch arg[1][0]*/
            break;
        }

    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_MP_Local*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_MP_Local_Detail
 *-------------------------------------------------------------------------
 * PURPOSE  :show the local mp info
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :show ethernet cfm maintenance-points local detail {mep [ interface interface |domain domain-name | level level-id] }
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_MP_Local_Detail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_OM_MepInfo_T mep_result;
    UI32_T nxt_md_index=0,nxt_ma_index=0,nxt_mep_id=0;
    UI32_T line_num=0;
    char buff[CLI_DEF_MAX_BUFSIZE]={0};

    sprintf(buff, "MEP Settings:\r\n");
    PROCESS_MORE(buff);
    sprintf(buff, "-------------\r\n");
    PROCESS_MORE(buff);

     /*show ethernet cfm maintenance-points local detail {mep [ interface interface |domain domain-name | level level-id] }*/
    if(arg[0]!=NULL)
    {
        /*show ethernet cfm maintenance-points local detail mep*/
        if(NULL == arg[1])
        {
            while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,0, &mep_result))
            {
                if(JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local_detail(&mep_result,line_num)))
                {
                    return CLI_NO_ERROR;
                }
            }

            return CLI_NO_ERROR;
        }

        /*show ethernet cfm maintenance-points local detail {mep [ interface interface |domain domain-name | level level-id] }*/
        switch(arg[1][0])
        {
        case 'i':
        case 'I':
            {
                UI32_T              verify_unit,verify_port,lport;
                CLI_API_EthStatus_T verify_ret;

                if(arg[2][0] == 'e' || arg[2][0] == 'E')
                {
                    if (isdigit(arg[3][0]))
                    {
                        verify_unit = atoi(arg[3]);
                        verify_port = atoi(strchr(arg[3], '/') + 1);
                    }
                    else
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                    if(CLI_API_ETH_OK!=verify_ret)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }
                }
                else if(arg[2][0] == 'p'||arg[2][0]=='P')
                {
                    if (isdigit(arg[3][0]))
                    {
                        verify_port=atoi(arg[3]);
                    }
                    else
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }

                    verify_ret=verify_trunk(verify_port,&lport);

                    if(CLI_API_ETH_OK!=verify_ret)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                        return CLI_NO_ERROR;
                    }
                }
                else
                {
                    /* should not occur */
                    return CLI_ERR_INTERNAL;
                }

                while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,lport, &mep_result))
                {
                    if(mep_result.lport == lport)
                    {
                        if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local_detail(&mep_result,line_num)))
                        {
                            return CLI_NO_ERROR;
                        }
                    }
                }
                break;
            }

        case 'd':
        case 'D':
            while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,0, &mep_result))
            {
                if(!memcmp(mep_result.md_name_a,arg[2],strlen(arg[2])) )
                {
                    if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local_detail(&mep_result,line_num)))
                    {
                        return CLI_NO_ERROR;
                    }
                }
            }
            break;

        case 'l':
        case 'L':
            while(TRUE == CFM_OM_GetNextMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id,0, &mep_result))
            {
                if(mep_result.md_level == atoi(arg[2]))
                {
                    if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mep_local_detail(&mep_result,line_num)))
                    {
                        return CLI_NO_ERROR;
                    }
                }
            }
            break;
        }  /* End show ethernet cfm maintenance-points local detail [interface interface |domain domain-name | level level-id] */
    }
#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_CFM == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_mep_local
 *-------------------------------------------------------------------------
 * PURPOSE  : show the local mep info
 * INPUT    : *result  - the struct store the messsage will be shown
 *           line_num  - the shown line number
 * OUTPUT   : the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static UI32_T show_eth_cfm_mep_local (CFM_OM_MepInfo_T *result,UI32_T line_num)
{
    UI32_T  unit=0, port=0, trunk_id=0, name_beg_id =0;
    char   buff[CLI_DEF_MAX_BUFSIZE]={0},
            tmp_mac_str[18], tmp_md_name[17],
            tmp_port_str[9], tmp_ccm_sts_str[9], tmp_dir_str[5];

    /* MPID MD Name          Level Direct VLAN Interface CC Status MAC Address
     * 4    16               5     6      4    9         9         17
     * ---- ---------------- ----- ------ ---- --------- --------- -----------------
     */
    SWCTRL_POM_LogicalPortToUserPort(result->lport, &unit, &port, &trunk_id);
    if (trunk_id == 0)
    {
        sprintf(tmp_port_str, "Eth %1ld/%2ld", (long)unit, (long)port);
    }
    else
    {
        sprintf(tmp_port_str, "Trunk %2ld", (long)trunk_id);
    }

    if (result->direction==CFM_TYPE_MP_DIRECTION_DOWN)
    {
        sprintf(tmp_dir_str, "Down");
    }
    else
    {
        sprintf(tmp_dir_str, "Up");
    }

    if(CFM_TYPE_CCM_STATUS_ENABLE== result->ccm_status)
    {
        sprintf(tmp_ccm_sts_str, "Enabled");
    }
    else
    {
        sprintf(tmp_ccm_sts_str, "Disabled");
    }

    sprintf(tmp_mac_str,"%02X-%02X-%02X-%02X-%02X-%02X",
            result->mac_addr_a[0], result->mac_addr_a[1], result->mac_addr_a[2],
            result->mac_addr_a[3], result->mac_addr_a[4], result->mac_addr_a[5]);

    cli_api_cfm_copy_name_by_len(
        result->md_name_a, result->md_name_len, name_beg_id,
        (UI8_T *)tmp_md_name, 16);

    name_beg_id += 16;

    sprintf(buff,"%4ld %-16s %5d %-6s %4ld %-9s %-9s %17s\r\n",
            (long)result->identifier, tmp_md_name, result->md_level,
            tmp_dir_str, (long)result->primary_vid, tmp_port_str,
            tmp_ccm_sts_str, tmp_mac_str);

    PROCESS_MORE_FUNC(buff);

    while (name_beg_id < result->md_name_len)
    {
        cli_api_cfm_copy_name_by_len(
            result->md_name_a, result->md_name_len, name_beg_id,
            (UI8_T *)tmp_md_name, 16);
        sprintf(buff, "%4s %-16s\r\n", "", tmp_md_name);
        PROCESS_MORE_FUNC(buff);

        name_beg_id += 16;
    }

    return line_num;
}/*End of show_eth_cfm_mep_local*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_mep_local_detail
 *-------------------------------------------------------------------------
 * PURPOSE  : show the local mep detail info
 * INPUT    : *result  - the struct store the messsage will be shown
 *           line_num  - the shown line number
 * OUTPUT   : the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static UI32_T show_eth_cfm_mep_local_detail (CFM_OM_MepInfo_T *result_p, UI32_T line_num)
{
#define CFM_MEP_LOCAL_DETAIL_FMT_HEAD   "%-18s : "

    CFM_OM_MdInfo_T md_info;
    CFM_OM_MaInfo_T ma_info;
    UI32_T          unit=0,port=0,trunk_id=0;
    char           buff[CLI_DEF_MAX_BUFSIZE]={0};
    char           ma_name_fmt_buf[20];
    BOOL_T          is_ma_ok;

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%ld\r\n",
            "MPID", (long)result_p->identifier);
    PROCESS_MORE_FUNC(buff);

    if(TRUE == CFM_OM_GetMdInfo(result_p->md_index, &md_info))
    {
        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n" ,
                "MD Name", md_info.name_a);
        PROCESS_MORE_FUNC(buff);
    }

    is_ma_ok = CFM_OM_GetMaInfo(result_p->md_index, result_p->ma_index, &ma_info);
    if(TRUE == is_ma_ok)
    {
        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n" ,
                "MA Name", ma_info.ma_name_a);
        PROCESS_MORE_FUNC(buff);

        switch(ma_info.name_format)
        {
        case CFM_TYPE_MA_NAME_CHAR_STRING:
            strcpy(ma_name_fmt_buf, "Character String");
            break;
        case CFM_TYPE_MA_NAME_ICC_BASED:
            strcpy(ma_name_fmt_buf, "ICC Based");
            break;
        default:
            strcpy(ma_name_fmt_buf, "Unknown");
            break;
        }
        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n" ,
                "MA Name Format", ma_name_fmt_buf);
        PROCESS_MORE_FUNC(buff);
    }

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%d\r\n",
            "Level", md_info.level);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
            "Direction", (result_p->direction == CFM_TYPE_MP_DIRECTION_UP?"Up":"Down"));
    PROCESS_MORE_FUNC(buff);

    if(TRUE == SWCTRL_POM_LogicalPortToUserPort(result_p->lport, &unit, &port, &trunk_id))
    {
        if(0==trunk_id)
            sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"Eth%2ld/%2ld\r\n",
                    "Interface", (long)unit, (long)port);
        else
            sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"Port Channel %ld\r\n",
                    "Interface", (long)trunk_id );
    }
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
            "CC Status", (result_p->ccm_status == CFM_TYPE_CCM_STATUS_ENABLE?"Enabled":"Disabled"));
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%02X-%02X-%02X-%02X-%02X-%02X\r\n",
            "MAC Address",  result_p->mac_addr_a[0],    result_p->mac_addr_a[1],
                    result_p->mac_addr_a[2],    result_p->mac_addr_a[3],
                    result_p->mac_addr_a[4],    result_p->mac_addr_a[5]);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD, "Defect Condition");
    PROCESS_MORE_FUNC(buff);

    /* Defect Condition shows the current highest defect priority.
     */
    switch(result_p->cur_highest_pri_defect)
    {
    case CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM:
        sprintf(buff,"%s\r\n","defRDICCM");
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS:
        sprintf(buff,"%s\r\n","defMACstatus");
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM:
        sprintf(buff,"%s\r\n","defRemoteCCM");
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM:
        sprintf(buff,"%s\r\n","defErrorCCM");
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM:
        sprintf(buff,"%s\r\n","defXconCCM");
        break;

    default:
        sprintf(buff,"%s\r\n","No Defect");
        break;
    }
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
            "Received RDI", (result_p->defects&0x01)?"True":"False");
    PROCESS_MORE_FUNC(buff);

    if(TRUE == is_ma_ok)
    {
        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
                "AIS Status", ma_info.ais_status == CFM_TYPE_AIS_STATUS_ENABLE?"Enabled":"Disabled");
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%d seconds\r\n",
                "AIS Period", ma_info.ais_period);
        PROCESS_MORE_FUNC(buff);

        if(0 == ma_info.ais_level)
            sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s (%d)\r\n",
                    "AIS Transmit Level", "Default", ma_info.ais_level);
        else
            sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%d\r\n",
                    "AIS Transmit Level", ma_info.ais_level);
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
                "Suppress Alarm", ma_info.ais_suppress_status == CFM_TYPE_AIS_STATUS_ENABLE? "Enabled":"Disabled");
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, CFM_MEP_LOCAL_DETAIL_FMT_HEAD"%s\r\n",
                "Suppressing Alarms", ma_info.ais_suppresing == TRUE?"Enabled":"Disabled");
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, "\r\n\r\n");
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}/*End of show_eth_cfm_mep_local*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_mip_local
 *-------------------------------------------------------------------------
 * PURPOSE  : show the local mip info
 * INPUT    : *result  - the struct store the messsage will be shown
 *           *line    - the shown line number
 * OUTPUT   : *line    -the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static UI32_T show_eth_cfm_mip_local (CFM_OM_MipInfo_T *result,UI32_T line_num)
{
    UI32_T  unit=0, port=0, trunk_id=0, name_beg_id =0;
    char   buff[CLI_DEF_MAX_BUFSIZE]={0},
            tmp_ma_name[24], tmp_md_name[24], tmp_port_str[9];

    /* MD Name                 Level MA Name                 VLAN Interface
     * 23                      5     23                      4    9
     * ----------------------- ----- ----------------------- ---- ---------
     */
    SWCTRL_POM_LogicalPortToUserPort(result->lport, &unit, &port, &trunk_id);

    if(trunk_id == 0)
    {
        sprintf(tmp_port_str, "Eth %1ld/%2ld",(long)unit, (long)port);
    }
    else
    {
        sprintf(tmp_port_str, "Trunk %2ld",(long)trunk_id);
    }

    cli_api_cfm_copy_name_by_len(
        result->md_name_a, result->md_name_len, name_beg_id,
        (UI8_T *)tmp_md_name, 23);

    cli_api_cfm_copy_name_by_len(
        result->ma_name_a, result->ma_name_len, name_beg_id,
        (UI8_T *)tmp_ma_name, 23);

    sprintf(buff, "%-23s %5d %-23s %4ld %-9s\r\n",
            tmp_md_name, result->md_level, tmp_ma_name, (long)result->vid, tmp_port_str);

    name_beg_id += 23;

    PROCESS_MORE_FUNC(buff);

    while ((name_beg_id < result->md_name_len) ||
           (name_beg_id < result->ma_name_len))
    {
        cli_api_cfm_copy_name_by_len(
            result->md_name_a, result->md_name_len, name_beg_id,
            (UI8_T *)tmp_md_name, 23);

        cli_api_cfm_copy_name_by_len(
            result->ma_name_a, result->ma_name_len, name_beg_id,
            (UI8_T *)tmp_ma_name, 23);

        sprintf(buff, "%-23s %5s %-23s\r\n", tmp_md_name, "", tmp_ma_name);
        PROCESS_MORE_FUNC(buff);

        name_beg_id += 23;
    }

    return line_num;
}/*End ofshow_eth_cfm_mip_local*/
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_Errors
 *-------------------------------------------------------------------------
 * PURPOSE  : show the errors info
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_Errors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    UI32_T          line_num=0;
    UI32_T          lport=0;
    UI16_T          vid=0;
    CFM_OM_Error_T  result;
    char           buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    sprintf(buff,"Level VLAN MPID Interface Remote MAC        Reason           MA Name\r\n");
    PROCESS_MORE(buff);
               /* 5     4    4    9         17                16               17 */
    sprintf(buff,"----- ---- ---- --------- ----------------- ---------------- -----------------\r\n");
    PROCESS_MORE(buff);

    if(NULL == arg[0])
    {
        while(TRUE == CFM_OM_GetNextErrorInfo(&vid, &lport, &result))
        {
            if(JUMP_OUT_MORE == (line_num=show_eth_cfm_error(line_num, &result)))
            {
                return CLI_NO_ERROR;
            }
        }

        return CLI_NO_ERROR;
    }

    switch(arg[0][0])
    {
        case 'd':
        case 'D':

            while(TRUE == CFM_OM_GetNextErrorInfo(&vid, &lport, &result))
            {
                if(strcmp(arg[1], (char *)result.md_name_a))
                {
                    continue;
                }
                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_error(line_num, &result)))
                {
                    return CLI_NO_ERROR;
                }
            }

            break;

        case 'l':
        case 'L':

            while(TRUE == CFM_OM_GetNextErrorInfo(&vid, &lport, &result))
            {
                if(atoi(arg[1])!= result.level)
                {
                    continue;
                }

                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_error(line_num, &result)))
                {
                    return CLI_NO_ERROR;
                }
            }

            break;

    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_Errors*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_mip_local
 *-------------------------------------------------------------------------
 * PURPOSE  : show the errors info
 * INPUT    : *result  - the struct store the messsage will be shown
 *           *line    - the shown line number
 * OUTPUT   : *line    -the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
#if (SYS_CPNT_CFM == TRUE)
static UI32_T show_eth_cfm_error(UI32_T line_num, CFM_OM_Error_T *result)
{
    UI32_T  unit, port, trunk_id;
    UI8_T   null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   *reason_str[] =
            {"cfmLeak", "conflictingVids", "excessiveLevels",
             "overlappedLevels", "receiveAIS", ""};
    char   tmp_ma_name [24], mac_str[18] = {0}, port_str[9], mpid_str[5] = {0};
    UI8_T name_beg_id =0, loop_idx,
            fst_eid  =5,    /* point to the "" */
            loop_bit =0x1;

    /* Level VLAN MPID Interface Remote MAC        Reason           MA Name
     * 5     4    4    9         17                16               17
     * ----- ---- ---- --------- ----------------- ---------------- ------------------
     */
    /* prepare mac
     */
    if (0 != memcmp(result->mac_addr_a, null_mac, SYS_ADPT_MAC_ADDR_LEN))
    {
        sprintf(mac_str, "%02X-%02X-%02X-%02X-%02X-%02X",
            result->mac_addr_a[0], result->mac_addr_a[1], result->mac_addr_a[2],
            result->mac_addr_a[3], result->mac_addr_a[4], result->mac_addr_a[5]);
    }

    SWCTRL_POM_LogicalPortToUserPort(result->lport, &unit, &port, &trunk_id);
    if(trunk_id == 0)
    {
        sprintf(port_str, "Eth %1ld/%2ld", (long)unit, (long)port);
    }
    else
    {
        sprintf(port_str, "Trunk %2ld", (long)trunk_id);
    }

    if (result->mep_id != 0)
    {
        sprintf(mpid_str, "%4ld", (long)result->mep_id);
    }

    /* find the first reason
     */
    for (loop_idx =0; loop_idx <5; loop_idx++)
    {
        if (result->reason_bit_map & loop_bit)
        {
            fst_eid = loop_idx;
            /* mask off the current reason
             */
            result->reason_bit_map ^= loop_bit;
            break;
        }
        loop_bit <<= 1;
    }

    /* copy the first 17 chars of ma name
     */
    cli_api_cfm_copy_name_by_len(
        result->ma_name_a, result->ma_name_len, name_beg_id,
        (UI8_T *)tmp_ma_name, 17);
    name_beg_id += 17;

    /* display the first line
     */
    sprintf(buff,"%5d %4d %4s %-9s %-17s %-16s %-17s\r\n",
          result->level, result->vlan_id, mpid_str, port_str,
          mac_str, reason_str[fst_eid], tmp_ma_name);
    PROCESS_MORE_FUNC(buff);

    /* display other reason or the other part of ma name
     */
    while ((result->reason_bit_map != 0) ||
           (name_beg_id < result->ma_name_len))
    {
        /* find the other reason
         */
        fst_eid = 5; /* reset to "" */
        for (; loop_idx <5; loop_idx++)
        {
            if (result->reason_bit_map & loop_bit)
            {
                fst_eid = loop_idx;
                /* mask off the current reason
                 */
                result->reason_bit_map ^= loop_bit;
                break;
            }
            loop_bit <<= 1;
        }

        /* try to quit when reason_bit_map is wrong
         */
        if (loop_idx >=5 )
        {
            result->reason_bit_map = 0;
        }

        cli_api_cfm_copy_name_by_len(
            result->ma_name_a, result->ma_name_len, name_beg_id,
            (UI8_T *)tmp_ma_name, 17);
        name_beg_id += 17;

        /* display the other line
         */
        sprintf(buff,"%5s %4s %4s %9s %17s %-16s %-17s\r\n",
                "", "", "", "", "", reason_str[fst_eid], tmp_ma_name);
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}/*End of show_eth_cfm_error*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_MP_Remote_Detail
 *-------------------------------------------------------------------------
 * PURPOSE  : show the remote mep detail info
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_MP_Remote_Detail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_OM_RemoteMepCrossCheck_T result;
    UI32_T line_num=0;
    UI32_T ori_md_index=0,ori_ma_index=0;
    UI32_T nxt_md_index=0,nxt_ma_index=0,nxt_mep_id=0;

    UI8_T filter_mac_addr[SYS_ADPT_MAC_ADDR_LEN]={0};
    UI32_T filter_mep_id=0;

    memset(&result,0,sizeof(CFM_OM_RemoteMepCrossCheck_T));
    switch(arg[0][1])
    {
        /*show et cf mai re de mac 00-00-00-00-00-01 [domain| level |ma]*/
        case 'a':
        case 'A':
        {
            CLI_LIB_ValsInMac(arg[1],filter_mac_addr);

            if(NULL == arg[2])
            {
                while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                {
                    if(!memcmp(filter_mac_addr, result.mep_mac_a,SYS_ADPT_MAC_ADDR_LEN))
                    {
                        if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                        {
                            break;
                        }
                    }
                }

                return CLI_NO_ERROR;
            }

            switch(arg[2][0])
            {
                case 'd':
                case 'D':
                {
                    if(TRUE!=CFM_OM_GetMdIndexByName((UI8_T *)arg[3],strlen(arg[3]), &ori_md_index))
                    {
                        return CLI_NO_ERROR;
                    }

                    nxt_md_index=ori_md_index;

                    while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                    {
                        if(ori_md_index!=nxt_md_index)
                        {
                            return CLI_NO_ERROR;
                        }

                        if(!memcmp(filter_mac_addr, result.mep_mac_a,SYS_ADPT_MAC_ADDR_LEN))
                        {
                            if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }

                    break;

                }
                case 'l':
                case 'L':
                {
                    /*there may have some same level md index*/
                    while(TRUE == CFM_OM_GetNextMdIndexByIndexAndLevel(atoi(arg[3]), &ori_md_index))
                    {
                        nxt_md_index=ori_md_index;
                        nxt_ma_index=0;
                        nxt_mep_id=0;

                        while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                        {
                            if(ori_md_index!=nxt_md_index)
                            {
                                break;
                            }

                            if(!memcmp(filter_mac_addr, result.mep_mac_a,SYS_ADPT_MAC_ADDR_LEN))
                            {
                                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                                {
                                    return CLI_NO_ERROR;
                                }
                            }
                        }
                    }

                    break;

                }
                case 'm':
                case 'M':
                {
                    /*there may have some same ma name in different md*/
                    if(TRUE==CFM_OM_GetMdMaIndexByMaName((UI8_T *)arg[3],strlen(arg[3]), &ori_md_index, &ori_ma_index))
                    {
                        nxt_md_index=ori_md_index;
                        nxt_ma_index=ori_ma_index;
                        nxt_mep_id=0;
                        while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                        {
                            if((ori_md_index!=nxt_md_index)||(ori_ma_index!=nxt_ma_index))
                            {
                                break;
                            }

                            if(!memcmp(filter_mac_addr, result.mep_mac_a,SYS_ADPT_MAC_ADDR_LEN))
                            {
                                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                                {
                                    return CLI_NO_ERROR;
                                }
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
        /*show et cf mai re de mp 2  [domain| level |ma]*/
        case 'p':
        case 'P':
        {
            filter_mep_id=atoi(arg[1]);

            if(NULL == arg[2])
            {
               while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
               {
                   if(filter_mep_id==result.mep_id)
                   {
                       if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                       {
                           break;
                       }
                   }
               }

               return CLI_NO_ERROR;
            }

            switch(arg[2][0])
            {
               case 'd':
               case 'D':
               {
                   if(TRUE!=CFM_OM_GetMdIndexByName((UI8_T *)arg[3],strlen(arg[3]), &ori_md_index))
                   {
                       return CLI_NO_ERROR;
                   }

                   nxt_md_index=ori_md_index;

                   while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                   {
                       if(ori_md_index!=nxt_md_index)
                       {
                           return CLI_NO_ERROR;
                       }

                       if(filter_mep_id==result.mep_id)
                       {
                           if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                           {
                               return CLI_NO_ERROR;
                           }
                       }
                   }

                   break;

               }
               case 'l':
               case 'L':
               {
                   /*there may have some same level md index*/
                   while(TRUE == CFM_OM_GetNextMdIndexByIndexAndLevel(atoi(arg[3]), &ori_md_index))
                   {
                       nxt_md_index=ori_md_index;
                       nxt_ma_index=0;
                       nxt_mep_id=0;
                       while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                       {
                           if(ori_md_index!=nxt_md_index)
                           {
                               break;
                           }

                           if(filter_mep_id==result.mep_id)
                           {
                                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                                {
                                    return CLI_NO_ERROR;
                                }
                           }
                       }
                   }

                   break;

               }
               case 'm':
               case 'M':
               {
                   /*there may have some same ma name in different md*/
                   if(TRUE==CFM_OM_GetMdMaIndexByMaName((UI8_T *)arg[3], strlen(arg[3]), &ori_md_index, &ori_ma_index))
                   {
                       nxt_md_index=ori_md_index;
                       nxt_ma_index=ori_ma_index;
                       nxt_mep_id=0;
                       while(TRUE==CFM_OM_GetNextRemoteMepInfo(&nxt_md_index, &nxt_ma_index, &nxt_mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result))
                       {
                           if((ori_md_index!=nxt_md_index)||(ori_ma_index!=nxt_ma_index))
                           {
                               break;
                           }

                           if(filter_mep_id==result.mep_id)
                           {
                                if( JUMP_OUT_MORE == (line_num=show_eth_cfm_mp_remote_detail(&result,line_num)))
                                {
                                    return CLI_NO_ERROR;
                                }
                           }
                       }
                   }

                   break;
                }
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_MP_Remote_Detail*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_mip_local
 *-------------------------------------------------------------------------
 * PURPOSE  :show the remote mep detail info
 * INPUT    : *result  - the struct store the messsage will be shown
 *           *line    - the shown line number
 * OUTPUT   : *line    -the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
#if (SYS_CPNT_CFM == TRUE)
static UI32_T show_eth_cfm_mp_remote_detail (CFM_OM_RemoteMepCrossCheck_T *result, UI32_T line_num)
{
#define CFM_MP_REMOTE_DETAIL_FMT_HEAD   "%-22s : "

    UI32_T  unit=0, port=0, trunk_id=0;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char   *port_status[]={"No port state","Blocked","Up"};
    char   *interface_status[]={"No status","Up","Down","Testing","Unknwon","Dormant","Not present", "Low layer down"};

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%02X-%02X-%02X-%02X-%02X-%02X\r\n",
            "MAC Address",
            result->mep_mac_a[0], result->mep_mac_a[1],
            result->mep_mac_a[2], result->mep_mac_a[3],
            result->mep_mac_a[4], result->mep_mac_a[5]);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%s / %d\r\n",
            "Domain/Level", result->md_name_a, result->level);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%s\r\n",
            "MA Name", result->ma_name_a);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu\r\n",
            "Primary VLAN", (unsigned long)result->primary_vid);
    PROCESS_MORE_FUNC(buff) ;
    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu\r\n",
            "MPID", (unsigned long)result->mep_id);
    PROCESS_MORE_FUNC(buff);

    if(SWCTRL_LPORT_UNKNOWN_PORT!=SWCTRL_POM_LogicalPortToUserPort(result->incoming_port, &unit, &port, &trunk_id))
    {
        if(0==trunk_id)
        {
            sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"Eth %1ld/%2ld\r\n",
                    "Incoming Port", (long)unit, (long)port);
        }
        else
        {
            sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"Port Channel %3ld\r\n",
                    "Incoming Port", (long)trunk_id);
        }
    }
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu seconds\r\n",
                    "CC Lifetime", (unsigned long)result->cc_life_time);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu seconds\r\n",
                    "Age of Last CC Message", (unsigned long)result->age_of_last_cc);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu\r\n",
                    "Frame Loss", (unsigned long)result->frame_loss);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%lu/%lu (Received/Error)\r\n",
                    "CC Packet Statistics", (unsigned long)result->packet_rcvd_count, (unsigned long)result->packet_error_count);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%s\r\n",
                    "Port State", port_status[result->port_status]);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%s\r\n",
                    "Interface State", interface_status[result->interface_status]);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, CFM_MP_REMOTE_DETAIL_FMT_HEAD"%s\n\r\n",
                    "Crosscheck Status", (TRUE ==result->cross_check_enabled?"Enabled":"Disabled"));
    PROCESS_MORE_FUNC(buff);
    return line_num;
}/*End of show_eth_cfm_mp_remote_detail*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_MP_Remote_Crosscheck
 *-------------------------------------------------------------------------
 * PURPOSE  : show the remote mep
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T  CLI_API_Show_Eth_Cfm_MP_Remote_Crosscheck(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
#define SHOW_CFM_MP_REMOTE_CROSSCHECK_ALL       0
#define SHOW_CFM_MP_REMOTE_CROSSCHECK_MPID      1
#define SHOW_CFM_MP_REMOTE_CROSSCHECK_MA        2
#define SHOW_CFM_MP_REMOTE_CROSSCHECK_MD        3

    UI32_T                          line_num=0, md_index=0, ma_index=0, mep_id=0,
                                    ori_mep_id=0, ori_ma_id=0, ori_md_id=0;
    CFM_OM_RemoteMepCrossCheck_T    result;
    char                           buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char                           tmp_ma_name[24];
    char                           broadcast_mac[SYS_ADPT_MAC_ADDR_LEN]={0xff,0xff,0xff,0xff,0xff,0xff};
    char                           tmp_mac_str[18];
    UI8_T                          show_type, name_beg_id =0;

    memset(&result,0,sizeof(CFM_OM_RemoteMepCrossCheck_T));

    sprintf(buff,"MPID  MA Name                  Level  VLAN  MEP Up  Remote MAC\r\n");
    PROCESS_MORE(buff);
               /* 4     23                       5      4     6       17 */
    sprintf(buff,"----  -----------------------  -----  ----  ------  -----------------\r\n");
    PROCESS_MORE(buff);

    if(NULL == arg[0])
    {
        show_type = SHOW_CFM_MP_REMOTE_CROSSCHECK_ALL;
    }
    else
    {
        switch(arg[0][1])
        {
        case 'p':
        case 'P':
            show_type = SHOW_CFM_MP_REMOTE_CROSSCHECK_MPID;
            ori_mep_id = atoi(arg[1]);
            break;
        case 'a':
        case 'A':
            show_type = SHOW_CFM_MP_REMOTE_CROSSCHECK_MA;

            if(CFM_TYPE_CONFIG_SUCCESS!=CFM_OM_GetMdMaIndexByMaName((UI8_T *)arg[1],strlen(arg[1]),&md_index,&ma_index))
            {
                /*there doesn't exist this ma, so just show title*/
                return CLI_NO_ERROR;
            }
            ori_md_id = md_index;
            ori_ma_id = ma_index;
            break;
        case 'o':
        case 'O':
            show_type = SHOW_CFM_MP_REMOTE_CROSSCHECK_MD;
            if(CFM_TYPE_CONFIG_SUCCESS!=CFM_OM_GetMdIndexByName((UI8_T *)arg[1], strlen(arg[1]), &md_index))
            {
                return CLI_NO_ERROR;
            }
            ori_md_id = md_index;
            break;
        default:
            return CLI_ERR_INTERNAL;
        }
    }

    while(TRUE==CFM_OM_GetNextRemoteMepInfo(&md_index, &ma_index, &mep_id, SYS_TIME_GetSystemTicksBy10ms(), &result ) )
    {
        switch(show_type)
        {
        case SHOW_CFM_MP_REMOTE_CROSSCHECK_MPID:
            if(ori_mep_id != mep_id)
            {
                continue;
            }
            break;
        case SHOW_CFM_MP_REMOTE_CROSSCHECK_MA:
            if((md_index!=ori_md_id)||(ma_index != ori_ma_id))
            {
                return CLI_NO_ERROR;
            }
            break;
        case SHOW_CFM_MP_REMOTE_CROSSCHECK_MD:
            if(md_index!=ori_md_id)
            {
                return CLI_NO_ERROR;
            }
            break;
        }

        if(!memcmp(broadcast_mac,result.mep_mac_a,SYS_ADPT_MAC_ADDR_LEN))
        {
            sprintf(tmp_mac_str, "Not Configured");
        }
        else
        {
            sprintf(tmp_mac_str,"%02X-%02X-%02X-%02X-%02X-%02X",
                    result.mep_mac_a[0], result.mep_mac_a[1] ,result.mep_mac_a[2],
                    result.mep_mac_a[3], result.mep_mac_a[4], result.mep_mac_a[5]);
        }

        name_beg_id =0;

        cli_api_cfm_copy_name_by_len(
            result.ma_name_a, result.ma_name_len, name_beg_id, (UI8_T *)tmp_ma_name, 23);
        name_beg_id += 23;

        sprintf(buff,"%4ld  %-23s  %5d  %4ld  %-6s  %-17s\r\n",
                (long)result.mep_id, tmp_ma_name, result.level,
                (long)result.primary_vid,
                (CFM_TYPE_REMOTE_MEP_STATE_OK == result.mep_state)?"Yes":"No", tmp_mac_str);
        PROCESS_MORE(buff);

        while (name_beg_id < result.ma_name_len)
        {
            cli_api_cfm_copy_name_by_len(
                result.ma_name_a, result.ma_name_len, name_beg_id, (UI8_T *)tmp_ma_name, 23);
            name_beg_id += 23;
            sprintf(buff,"%4s  %-23s\r\n", "", tmp_ma_name);

            PROCESS_MORE(buff);
        }
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_Eth_Cfm_MP_Remote_Crosscheck*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_FaultNotifyGenerator_Mep
 *-------------------------------------------------------------------------
 * PURPOSE  : show the fng configuration
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Ethernet_Cfm_FaultNotifyGenerator_Mep(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_OM_MepFngConfig_T   result;
    UI32_T                  line_num=0, ma_index=0, md_index=0, mep_id=0;
    char                   buff[CLI_DEF_MAX_BUFSIZE]  = {0};
    char                   buff2[CLI_DEF_MAX_BUFSIZE] = {0};

    memset(&result,0,sizeof(CFM_OM_MepFngConfig_T));

    sprintf(buff,"MD Name      MA Name      Highest Defect Lowest Alarm  Alarm Time Reset Time\r\n");
    PROCESS_MORE(buff);
               /* 12           12           14             13           10          10 */
    sprintf(buff,"------------ ------------ -------------- ------------- ---------- ----------\r\n");
    PROCESS_MORE(buff);

    while(TRUE==CFM_OM_GetNextFngConfig(&md_index, &ma_index, &mep_id, &result))
    {
        UI32_T  name_beg_id = 0;
        char   tmp_md_name[13], tmp_ma_name[13];

        if( (NULL!=arg[0])&&(mep_id!=atoi(arg[0])))
        {
            continue;
        }

        cli_api_cfm_copy_name_by_len(
            result.md_name_a, result.md_name_len, name_beg_id,
            (UI8_T *)tmp_md_name, 12);

        cli_api_cfm_copy_name_by_len(
            result.ma_name_a, result.ma_name_len, name_beg_id,
            (UI8_T *)tmp_ma_name, 12);

        name_beg_id += 12;
        sprintf(buff, "%-12s %-12s", tmp_md_name, tmp_ma_name);

        /* Highest Defect shows the highest defect priority ever occurred.
         */
        switch(result.highest_defect)
        {
            case CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM:
                sprintf(buff2," %-14s","defRDICCM");
                break;

            case CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS:
                sprintf(buff2," %-14s","defMACstatus");
                break;

            case CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM:
                sprintf(buff2," %-14s","defRemoteCCM");
                break;

            case CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM:
                sprintf(buff2," %-14s","defErrorCCM");
                break;

            case CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM:
                sprintf(buff2," %-14s","defXconCCM");
                break;
            case CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE:
                sprintf(buff2," %-14s","none");
                break;

            default:
                sprintf(buff2," %-14s"," ");
                break;

        }
        strcat(buff,buff2);
        switch(result.lowest_alarm_pri)
        {
            case CFM_TYPE_FNG_LOWEST_ALARM_ALL:
                sprintf(buff2," %-13s","allDef");
                break;

            case CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON:
                sprintf(buff2," %-13s","macRemErrXcon");
                break;

            case CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON:
                sprintf(buff2," %-13s","remErrXcon");
                break;

            case CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON:
                sprintf(buff2," %-13s","errXcon");
                break;

            case CFM_TYPE_FNG_LOWEST_ALARM_XCON:
                sprintf(buff2," %-13s","xcon");
                break;

            case CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF:
                sprintf(buff2," %-13s","noXcon");
                break;

            default:
                sprintf(buff2," %-13s"," f");
                break;
        }
        strcat(buff,buff2);
        sprintf(buff2," %5ld sec. %5ld sec.\r\n", (long)result.fng_alarm_time, (long)result.fng_reset_time);
        strcat(buff,buff2);
        PROCESS_MORE(buff);

        while ((name_beg_id < result.md_name_len) ||
               (name_beg_id < result.ma_name_len))
        {
            cli_api_cfm_copy_name_by_len(
                result.md_name_a, result.md_name_len, name_beg_id,
                (UI8_T *)tmp_md_name, 12);

            cli_api_cfm_copy_name_by_len(
                result.ma_name_a, result.ma_name_len, name_beg_id,
                (UI8_T *)tmp_ma_name, 12);

            name_beg_id += 12;

            sprintf(buff, "%-12s %-12s\r\n", tmp_md_name, tmp_ma_name);
            PROCESS_MORE(buff);
        }
    }
#endif

    return CLI_NO_ERROR;
}/*End of CLI_API_Show_FaultNotifyGenerator_Mep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_Cc
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_Cc(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    CFM_TYPE_SnmpTrapsCC_T trap=CFM_TYPE_SNMP_TRAPS_CC_ALL;
    BOOL_T trap_enabled=TRUE;


    if(NULL == arg[0])
    {
        trap=CFM_TYPE_SNMP_TRAPS_CC_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'm':
            case 'M':

                switch(arg[0][4])
                {
                    case 'u':
                    case 'U':
                        trap=CFM_TYPE_SNMP_TRAPS_CC_MEP_UP;
                        break;

                    case 'd':
                    case 'D':
                        trap=CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN;
                        break;

                }

                break;

            case 'c':
            case 'C':

                trap=CFM_TYPE_SNMP_TRAPS_CC_CONFIG;

                break;

            case 'l':
            case 'L':

                trap=CFM_TYPE_SNMP_TRAPS_CC_LOOP;

                break;

        }
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W6_SNMPSERVER_ENABLE_TRAPS_ETHERNET_CFM_CC:

            trap_enabled=TRUE;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_SNMPSERVER_ENABLE_TRAPS_ETHERNET_CFM_CC:

            trap_enabled=FALSE;

            break;
    }

    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetSNMPCcStatus(trap,trap_enabled))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to enable SNMP traps.\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_Cc*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_CrossCheck
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_CrossCheck(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)

    CFM_TYPE_SnmpTrapsCrossCheck_T trap=CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL;
    BOOL_T trap_enabled=TRUE;

    if(NULL == arg[0])
    {
        trap = CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL;
    }
    else
    {
        switch(arg[0][1])
        {
            case 'e':
            case 'E':
                switch(arg[0][4])
                {
                    case 'u':
                    case 'U':
                        trap=CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN;
                        break;

                    case 'm':
                    case 'M':
                        trap=CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING;
                        break;
                }
                break;

            case 'a':
            case 'A':
                trap=CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP;
                break;

        }

    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W6_SNMPSERVER_ENABLE_TRAPS_ETHERNET_CFM_CROSSCHECK:
            trap_enabled=TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_SNMPSERVER_ENABLE_TRAPS_ETHERNET_CFM_CROSSCHECK:
            trap_enabled=FALSE;
            break;
    }

    if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetSNMPCrosscheckStatus(trap,trap_enabled))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to enable SNMP traps.\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}/*End of CLI_API_Snmpserver_Enable_Traps_Ethernet_Cfm_CrossCheck*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Eth_Cfm_Config
 *-------------------------------------------------------------------------
 * PURPOSE  :show the global configuratoin
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Eth_Cfm_Config(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
#define ETH_CFM_CONFIG_GLOBAL_FMT_HEAD   "%-25s : "
#define ETH_CFM_CONFIG_TRAPS_FMT_HEAD    "%-28s : "

    CFM_OM_GlobalConfigInfo_T   global_info;
    UI32_T                      line_num=0;
    char                       buff[CLI_DEF_MAX_BUFSIZE] = {0};

    memset(&global_info, 0, sizeof(CFM_OM_GlobalConfigInfo_T));

    switch(arg[0][0])
    {
        case 'g':
        case 'G':
            if(TRUE == CFM_OM_GetCfmGlobalCofigurationGlobalInfo(&global_info))
            {
                if(CFM_TYPE_CFM_STATUS_ENABLE==global_info.cfm_global_status)
                {
                    sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%s\r\n", "CFM Global Status", "Enabled");
                }
                else
                {
                    sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%s\r\n", "CFM Global Status", "Disabled");
                }

                PROCESS_MORE(buff);

                sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%lu seconds\r\n", "Crosscheck Start Delay", (unsigned long)global_info.start_delay);
                PROCESS_MORE(buff);

                switch(global_info.linktrace_cache_status)
                {
                    case CFM_TYPE_LINKTRACE_STATUS_ENABLE:
                        sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%s\r\n", "Linktrace Cache Status", "Enabled");
                        break;

                    case CFM_TYPE_LINKTRACE_STATUS_DISABLE:
                        sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%s\r\n", "Linktrace Cache Status", "Disabled");
                        break;
                }

                PROCESS_MORE(buff);

                sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%lu minutes\r\n", "Linktrace Cache Hold Time", (unsigned long)global_info.linktrace_cache_holdTime);
                PROCESS_MORE(buff);

                sprintf(buff, ETH_CFM_CONFIG_GLOBAL_FMT_HEAD"%lu entries\r\n", "Linktrace Cache Size", (unsigned long)global_info.linktrace_cache_size);
                PROCESS_MORE(buff);
            }
            break;

        case 't':
        case 'T':
            if(TRUE == CFM_OM_GetCfmGlobalCofigurationTrapInfo(&global_info))
            {
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "CC MEP Up Trap", (global_info.cc_mep_up?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "CC MEP Down Trap", (global_info.cc_mep_down?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "CC Configure Trap", (global_info.cc_config?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "CC Loop Trap", (global_info.cc_loop?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "Crosscheck MEP Unknown Trap", (global_info.cross_mep_unknown?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "Crosscheck MEP Missing Trap", (global_info.cross_mep_missing?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
                sprintf(buff, ETH_CFM_CONFIG_TRAPS_FMT_HEAD"%s\r\n",
                        "Crosscheck MA Up Trap", (global_info.cross_ma_up?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
            }
            break;

        case 'i':
        case 'I':
        {
            CLI_API_EthStatus_T verify_ret;
            CFM_TYPE_CfmStatus_T status;
            UI32_T verify_unit,verify_port,lport;

            if(arg[1] == NULL) /*show all port*/
            {
                UI32_T lport=0;

                while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_GetNextLogicalPort(&lport))
                {
                    if(TRUE == CFM_OM_GetCFMPortStatus(lport, &status))
                    {
                        if(JUMP_OUT_MORE == (line_num=show_eth_cfm_port_status(lport, status, line_num)))
                        {
                           return CLI_NO_ERROR;
                        }
                    }
                }
                return CLI_NO_ERROR;
            }
            else if(arg[1][0] == 'e' || arg[1][0] == 'E')
            {
                if (isdigit(arg[2][0]))
                {
                    verify_unit = atoi(arg[2]);
                    verify_port = atoi(strchr(arg[2], '/') + 1);
                }
                else
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                if(CLI_API_ETH_OK!=verify_ret)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

                if(TRUE == CFM_OM_GetCFMPortStatus(lport, &status))
                {
                    if(JUMP_OUT_MORE == (line_num=show_eth_cfm_port_status(lport, status, line_num)))
                    {
                        return CLI_NO_ERROR;
                    }
                }
            }
            else if(arg[1][0] == 'p'||arg[1][0]=='P')
            {
                if (isdigit(arg[2][0]))
                {
                    verify_port=atoi(arg[2]);
                }
                else
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

                verify_ret=verify_trunk(verify_port, &lport);

                if(CLI_API_ETH_OK!=verify_ret)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("The interface does not exist.\r\n");
#endif
                    return CLI_NO_ERROR;
                }

                if(TRUE == CFM_OM_GetCFMPortStatus(lport, &status))
                    {
                        if(JUMP_OUT_MORE == (line_num=show_eth_cfm_port_status(lport, status, line_num)))
                        {
                            return CLI_NO_ERROR;
                        }
                    }
                }
            }
        break;
    }
#endif

    return CLI_NO_ERROR;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Level
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :ethernet cfm ais level level ma ma_name
 *          no ethernet cfm ais level ma ma_name
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Level(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    CFM_TYPE_MdLevel_T  level = SYS_DFLT_CFM_AIS_LEVEL;
    char                *md_name_p  =NULL, *ma_name_p  =NULL;
    UI32_T              md_name_len =0,    ma_name_len =0, arg_beg_idx =1;

    /* cmd format: ethernet cfm ais level lvl md md_name ma ma_name
     */
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_AIS_LEVEL:
        level       = atoi(arg[0]);
        arg_beg_idx = 2;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_AIS_LEVEL:
        break;
    }

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];
#else
    ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetAisLevel(
                       md_name_p, md_name_len, ma_name_p, ma_name_len, level))
    {
        CLI_LIB_PrintStr("Failed to configure AIS level.\r\n");
        return CLI_NO_ERROR;
    }
#endif

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ethernet_Cfm_Ais_Period
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     : ethernet cfm ais period {1|60} ma ma_name
 *           no ethernet cfm ais period ma ma_name
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Ethernet_Cfm_Ais_Period(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    char    *md_name_p  =NULL, *ma_name_p  =NULL;
    UI32_T  md_name_len =0,    ma_name_len =0, arg_beg_idx =1;
    UI16_T  period = SYS_DFLT_CFM_AIS_PERIOD;

    /* cmd format: [no] ethernet cfm ais period p_val md md_name ma ma_name
     */
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_ETHERNET_CFM_AIS_PERIOD:
        period = atoi(arg[0]);
        arg_beg_idx = 2;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_ETHERNET_CFM_AIS_PERIOD:
        break;
    }

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];
#else
    ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    if (CFM_TYPE_CONFIG_SUCCESS!= CFM_PMGR_SetAisPeriod(
                md_name_p, md_name_len, ma_name_p, ma_name_len, period))
    {
        CLI_LIB_PrintStr("Failed to configure AIS period.\r\n");
        return CLI_NO_ERROR;
    }
#endif

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Clear_Ethernet_Cfm_Ais
 *-------------------------------------------------------------------------
 * PURPOSE  : show the snmp trap enabled item
 * INPUT    :cmd_idx    - command index
 *           *arg[]     - argument array, two dimension array. first dimension means arg order
 *                        second dimension means arg character order
 *
 * OUTPUT   : None
 * RETURN   : help message idication
 * NOTE     :clear ethernet cfm ais mpid local_mep_id ma ma_name
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Clear_Ethernet_Cfm_Ais(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE)
    char    *md_name_p  =NULL, *ma_name_p  =NULL;
    UI32_T  md_name_len =0,    ma_name_len =0, arg_beg_idx =3;

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    md_name_p = arg[arg_beg_idx];
    ma_name_p = arg[arg_beg_idx+2];
#else
    ma_name_p = arg[arg_beg_idx];
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    if (NULL != md_name_p)
        md_name_len = strlen(md_name_p);
    if (NULL != ma_name_p)
        ma_name_len = strlen(ma_name_p);

    /* cmd format: clear ethernet cfm ais mpid mp_id md md_name ma ma_name
     */
    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_ClearAisError(
                atoi(arg[1]), md_name_p, md_name_len, ma_name_p, ma_name_len))
    {
        CLI_LIB_PrintStr("Failed to clear AIS error.\r\n");
        return CLI_NO_ERROR;
    }
#endif

    return CLI_NO_ERROR;
}

/* cmd format: ethernet cfm delay-measure
 *               [src-mep smpid] {dest-mep dmpid | dmac} md md_name ma ma_name
 *               [count count] [interval seconds] [timeout seconds] [size bytes]
 *               [priority pkt_pri]
 */
UI32_T CLI_API_Ethernet_Cfm_DelayMeasure_TwoWay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CFM == TRUE && SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE)
    char    *md_name_p = NULL, *ma_name_p = NULL;
    UI32_T  md_name_len =0, ma_name_len =0, arg_idx =0,
            src_mep_id=0, dst_mep_id =0, line_num =0;
    UI32_T  counts   = CFM_TYPE_DEF_DMM_COUNT,
            interval = CFM_TYPE_DEF_DMM_INTERVAL,
            timeout  = CFM_TYPE_DEF_DMM_TIMEOUT,
            pkt_size = CFM_TYPE_DEF_DMM_PKTSIZE;
    UI16_T  pkt_pri  = CFM_TYPE_DEF_PDU_PRIORITY;
    UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN] ={0};
    char    buff[CLI_DEF_MAX_BUFSIZE]={0};

    for (arg_idx =0; arg[arg_idx] != NULL;)
    {
        switch(arg[arg_idx][0])
        {
        case 'p':
        case 'P':
            pkt_pri = atoi(arg[arg_idx+1]);
            arg_idx+=2;
            break;
        case 's':
        case 'S':
            if (arg_idx == 0)
            {
                src_mep_id = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            else
            {
                pkt_size = atoi(arg[arg_idx+1]);
                arg_idx+=2;
                break;
            }
            break;
        case 'm':
        case 'M':
            if ((arg[arg_idx][1] == 'a') || (arg[arg_idx][1] == 'A'))
            {
                ma_name_p   = arg[arg_idx+1];
                ma_name_len = strlen(ma_name_p);
                arg_idx+=2;
            }
            else if ((arg[arg_idx][1] == 'd') || (arg[arg_idx][1] == 'D'))
            {
                md_name_p   = arg[arg_idx+1];
                md_name_len = strlen(md_name_p);
                arg_idx+=2;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;
        case 'i':
        case 'I':
            interval = atoi(arg[arg_idx+1]);
            arg_idx+=2;
            break;
        case 't':
        case 'T':
            timeout = atoi(arg[arg_idx+1]);
            arg_idx+=2;
            break;
        case 'c': /* may be mac */
        case 'C':
            if (TRUE == CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar))
            {
                arg_idx++;
            }
            else
            {
                counts = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            break;
        case 'd': /* may be mac */
        case 'D':
            if (TRUE == CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar))
            {
                arg_idx++;
            }
            else
            {
                dst_mep_id = atoi(arg[arg_idx+1]);
                arg_idx+=2;
            }
            break;
        default:
            if ((arg_idx == 0) || (arg_idx == 2))
            {
                CLI_LIB_ValsInMac(arg[arg_idx],dst_mac_ar);
                arg_idx++;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;
        }
    }

    if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_DoDelayMeasureByDMM(
                            src_mep_id, dst_mep_id, dst_mac_ar,
                            md_name_p, md_name_len, ma_name_p, ma_name_len,
                            counts, interval, timeout, pkt_size, pkt_pri))
    {
        CLI_LIB_PrintStr("Failed to do delay measurement.\r\n");
    }
    else
    {
        UI32_T  fd_ms, avg_fd_ms, min_fd_ms, max_fd_ms, fdv_ms, avg_fdv_ms,
                succ_cnt, total_cnt, send_idx, recv_idx =0;
        UI8_T   res;
        BOOL_T  is_succ;

        sprintf(buff, "Type ESC to abort.\r\n");
        PROCESS_MORE(buff);
        sprintf(buff, "Sending %ld Ethernet CFM delay measurement message, timeout is %ld sec.\r\n", (long)counts, (long)timeout);
        PROCESS_MORE(buff);
        sprintf(buff, "Sequence  Delay Time (ms.)  Delay Variation (ms.)\r\n");
        PROCESS_MORE(buff);
        sprintf(buff, "--------  ----------------  ---------------------\r\n");
        PROCESS_MORE(buff);

        while (TRUE == CFM_OM_GetNextDmmReplyRec(
                            &recv_idx, &send_idx, &fd_ms, &fdv_ms, &res, &is_succ))
        {
            if (TRUE == is_succ)
            {
                if (res == CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED)
                {
                    if (fd_ms < 10)
                    {
                        sprintf(buff, "%8ld  %16s  %21ld\r\n", (long)send_idx, "< 10", (long)fdv_ms);
                    }
                    else
                    {
                        sprintf(buff, "%8ld  %16lu  %21ld\r\n", (long)send_idx, (unsigned long)fd_ms, (long)fdv_ms);
                    }
                    PROCESS_MORE(buff);
                }
                else
                {
                    sprintf(buff, "%8ld  %16s\r\n", (long)send_idx, "time out");
                    PROCESS_MORE(buff);
                }
            }

            if (ctrl_P->sess_type == CLI_TYPE_TELNET)
            {
                if (CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) /*ESC*/
                {
                    CFM_PMGR_AbortDelayMeasureByDMM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_p, md_name_len, ma_name_p, ma_name_len);
                    break;
                }
            }
            else
            {
                if (CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) /*ESC*/
                {
                    CFM_PMGR_AbortDelayMeasureByDMM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_p, md_name_len, ma_name_p, ma_name_len);
                    break;
                }
            }

            SYSFUN_Sleep(30);
        }

        if (TRUE == CFM_OM_GetDelayMeasureResult(
                        &avg_fd_ms, &min_fd_ms, &max_fd_ms, &avg_fdv_ms, &succ_cnt, &total_cnt))
        {
            sprintf(buff, "Success rate is %ld%% (%ld/%ld), ",
                                    (long)(100 * succ_cnt /total_cnt), (long)succ_cnt, (long)total_cnt);

            PROCESS_MORE(buff);
            sprintf(buff, "delay time min/avg/max=%ld/%ld/%ld ms.\r\n",
                                    (long)min_fd_ms, (long)avg_fd_ms, (long)max_fd_ms);
            PROCESS_MORE(buff);

            sprintf(buff, "Average frame delay variation is %ld ms.\r\n",
                                    (long)avg_fdv_ms);
            PROCESS_MORE(buff);
        }
    }

#endif /* #if (SYS_CPNT_CFM == TRUE) */

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - show_eth_cfm_port_status
 *-------------------------------------------------------------------------
 * PURPOSE  : show the port cfm status
 * INPUT    : lport  - the logical port to show status
 *           status  - the port status
 *           line_num  - the shown line number
 * OUTPUT   : the line already be shown
 * RETURN   : help message idication
 * NOTE     :
 *-------------------------------------------------------------------------
 */
#if (SYS_CPNT_CFM == TRUE)
static UI32_T show_eth_cfm_port_status(UI32_T lport, CFM_TYPE_CfmStatus_T status, UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T unit=0, port=0,trunk_id=0;
    UI32_T port_type;

    port_type=SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if(SWCTRL_LPORT_NORMAL_PORT == port_type)
    {

        if(CFM_TYPE_CFM_STATUS_ENABLE==status)
        {
            sprintf(buff,"Ethernet %lu/%lu CFM Status : %s\r\n", (unsigned long)unit,(unsigned long)port, "Enabled");
        }
        else
        {
            sprintf(buff,"Ethernet %lu/%lu CFM Status : %s\r\n", (unsigned long)unit, (unsigned long)port, "Disabled");
        }
    }
    else if(TRK_PMGR_GetTrunkMemberCounts(trunk_id)>0)
    {
        if(CFM_TYPE_CFM_STATUS_ENABLE==status)
        {
            sprintf(buff,"Port Channel %lu CFM Status : %s\r\n",(unsigned long)trunk_id, "Enabled");
        }
        else
        {
            sprintf(buff,"Port Channel %lu CFM Status : %s\r\n",(unsigned long)trunk_id, "Disabled");
        }
    }

    PROCESS_MORE_FUNC(buff);

    return line_num;
}
#endif
