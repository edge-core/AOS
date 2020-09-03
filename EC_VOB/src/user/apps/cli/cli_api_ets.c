/* MODULE NAME: cli_api_ets.c
 * PURPOSE:
 *   Definitions of CLI APIs for Enhanced Transmission Selection.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"

#include "cli_api_ets.h"
#include "swctrl.h"
#include "nmtr_pmgr.h"
#include "ets_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T Show_one_interface_ets(UI32_T lport, UI8_T display_cont, UI32_T line_num);

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

#define DISPLAY_INF_ALL      0
#define DISPLAY_INF_PORT     1
#define DISPLAY_INF_TRUNK    2

#define DISPLAY_MAPPING_WEIGHT  0
#define DISPLAY_MAPPING         1
#define DISPLAY_WEIGHT          2
UI32_T CLI_API_ETS_TrafficClass_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    BOOL_T in_no_command=FALSE;
    UI32_T to_detect_position=0;
    int  ret;
    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_TRAFFICCLASS:
            in_no_command=TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_TRAFFICCLASS:
            in_no_command=FALSE;
            break;
        default:
            break;
    }
    if(arg[to_detect_position] == NULL)
    {
        return CLI_ERR_CMD_INVALID;
    }
    /* ETS weight
     */
    else if(arg[to_detect_position][0] == 'w' || arg[to_detect_position][0] == 'W')
    {
#if 0

        UI32_T num, arg_num, widx=0;
        UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];

        /*Get all weights and should not exceed SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS*/
        num=to_detect_position+1;
        arg_num=0;
        while(arg[num]!=NULL)
        {
            num++;
            arg_num++;
        }
        if(arg_num>SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS)
        {
            CLI_LIB_PrintStr("Given too many TC weight\r\n");
            return CLI_NO_ERROR;
        }
#endif

        UI32_T  num, widx=0;
        UI32_T  weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];

        if (arg[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS-1] == NULL)
        {
            return CLI_ERR_CMD_INVALID;
        }

        if (FALSE == in_no_command)
        {
            for (num=to_detect_position+1, widx=0; widx<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ;  num++,widx++)
            {
                weight[widx] = atoi(arg[num]);
            }
        }
        else
        {
            memset(weight, 0xff, sizeof(weight));
        }

        if((ret=ETS_PMGR_SetWeightByUser(lport, weight))!=ETS_TYPE_RETURN_OK)
        {
            printf("ETS_PMGR_SetWeightByUser: %d\r\n", ret);
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set TC weight\r\n");
#endif
        }
    }
    /* ETS mapping
     */
    else if((arg[to_detect_position][0] == 'm' || arg[to_detect_position][0] == 'M'))
    {
        UI32_T prio = atoi((char*)arg[1]);
        UI32_T tc;
        if(in_no_command==TRUE)
        {
            tc=SYS_DFLT_ETS_TRAFFIC_CLASS_OF_PRIORITY;
        }
        else
        {
            tc = atoi((char*)arg[2]);
        }
        if((ret=ETS_PMGR_SetPortPrioAssignByUser(lport, prio, tc))!=ETS_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set prority assignment\r\n");
#endif
        }
    }
    /* ETS algo
     */
    else if((arg[to_detect_position][0] == 'a' || arg[to_detect_position][0] == 'A'))
    {
        ETS_TYPE_TSA_T tsa;
        UI32_T tc = atoi((char*)arg[to_detect_position+1]);
        if(in_no_command==TRUE)
        {
             tsa=SYS_DFLT_ETS_TC_SCHEDULE_MODE;
        }
        else
        {
            if(arg[to_detect_position+2][0] == 'e' || arg[to_detect_position+2][0] == 'E')
            {
                tsa=ETS_TYPE_TSA_ETS;
            }
            else if(arg[to_detect_position+2][0] == 's' || arg[to_detect_position+2][0] == 'S')
            {
                tsa=ETS_TYPE_TSA_SP;
            }
            else
            {
                return CLI_ERR_CMD_UNRECOGNIZED;
            }
        }
        if((ret=ETS_PMGR_SetTSAByUser(lport, tc, tsa))!=ETS_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set mode\r\n");
#endif
        }

    }
    else
    {
        return CLI_ERR_CMD_UNRECOGNIZED;
    }
#endif /*#if (SYS_CPNT_ETS==TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_ETS_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    BOOL_T in_no_command=FALSE;
    UI32_T to_detect_position=0;
    int  ret;
    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_ETS:
            in_no_command=TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_ETS:
            in_no_command=FALSE;
            break;
        default:
            break;
    }

    if(arg[to_detect_position] == NULL)
    {
        return CLI_ERR_CMD_INVALID;
    }
    /* ETS mode
     */
    else if((arg[to_detect_position][0] == 'm' || arg[to_detect_position][0] == 'M')
          &&(arg[to_detect_position][1] == 'o' || arg[to_detect_position][1] == 'O'))
    {
        ETS_TYPE_MODE_T mode;
        if(in_no_command==TRUE)
        {
             mode=ETS_TYPE_MODE_OFF;
        }
        else
        {
            if(arg[to_detect_position+1][0] == 'a' || arg[to_detect_position+1][0] == 'a')
            {
                mode=ETS_TYPE_MODE_AUTO;
            }
            else if(arg[to_detect_position+1][0] == 'o' || arg[to_detect_position][0] == 'O')
            {
                mode=ETS_TYPE_MODE_USER;
            }
            else
            {
                CLI_LIB_PrintStr("invalid mode\r\n");
                return CLI_NO_ERROR;
            }
        }
        if((ret=ETS_PMGR_SetMode(lport, mode))!=ETS_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set mode\r\n");
#endif
        }
    }
    else
    {
        return CLI_ERR_CMD_UNRECOGNIZED;
    }
#endif /*#if (SYS_CPNT_ETS==TRUE)*/
    return CLI_NO_ERROR;
}



UI32_T CLI_API_ETS_TrafficClass_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    BOOL_T in_no_command=FALSE;
    UI32_T to_detect_position   = 0;
    int  ret;
    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
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
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_TRAFFICCLASS:
                        in_no_command=TRUE;
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_TRAFFICCLASS:
                        in_no_command=FALSE;
                        break;
                    default:
                        break;
                }
            }

            if(arg[to_detect_position] == NULL)
            {
                return CLI_ERR_CMD_INVALID;
            }
            /* ETS weight
             */
            else if(arg[to_detect_position][0] == 'w' || arg[to_detect_position][0] == 'W')
            {
#if 0
                UI32_T num, arg_num=0, widx=0;
                UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];

                /*Get all weights and should not exceed SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS*/
                num=to_detect_position+1;
                arg_num=0;
                while(arg[num]!=NULL)
                {
                    num++;
                    arg_num++;
                }
                if(arg_num>SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS)
                {
                    CLI_LIB_PrintStr("Given too many TC weight\r\n");
                    return CLI_NO_ERROR;
                }
#endif
                UI32_T  num, widx=0;
                UI32_T  weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];

                if (arg[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS-1] == NULL)
                {
                    return CLI_ERR_CMD_INVALID;
                }

                if (FALSE == in_no_command)
                {
                    for (num=to_detect_position+1, widx=0; widx<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ;  num++,widx++)
                    {
                        weight[widx] = atoi(arg[num]);
                    }
                }
                else
                {
                    memset(weight, 0xff, sizeof(weight));
                }
 
                if((ret=ETS_PMGR_SetWeightByUser(lport, weight))!=ETS_TYPE_RETURN_OK)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set TC weight\r\n");
#endif
                }
            }
            /* ETS mapping
             */
            else if((arg[to_detect_position][0] == 'm' || arg[to_detect_position][0] == 'M'))
            {
                UI32_T prio = atoi((char*)arg[1]);
                UI32_T tc;
                if(in_no_command==TRUE)
                {
                    tc=SYS_DFLT_ETS_TRAFFIC_CLASS_OF_PRIORITY;
                }
                else
                {
                    tc = atoi((char*)arg[2]);
                }

                if((ret=ETS_PMGR_SetPortPrioAssignByUser(lport, prio, tc))!=ETS_TYPE_RETURN_OK)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set prority assignment\r\n");
#endif
                }
            }
           /* ETS algo
             */
            else if((arg[to_detect_position][0] == 'a' || arg[to_detect_position][0] == 'A'))
            {
                ETS_TYPE_TSA_T tsa;
                UI32_T tc = atoi((char*)arg[to_detect_position+1]);
                if(in_no_command==TRUE)
                {
                     tsa=SYS_DFLT_ETS_TC_SCHEDULE_MODE;
                }
                else
                {
                    if(arg[to_detect_position+2][0] == 'e' || arg[to_detect_position+2][0] == 'E')
                    {
                        tsa=ETS_TYPE_TSA_ETS;
                    }
                    else if(arg[to_detect_position+2][0] == 's' || arg[to_detect_position+2][0] == 'S')
                    {
                        tsa=ETS_TYPE_TSA_SP;
                    }
                    else
                    {
                        return CLI_ERR_CMD_UNRECOGNIZED;
                    }
                }
                if((ret=ETS_PMGR_SetTSAByUser(lport, tc, tsa))!=ETS_TYPE_RETURN_OK)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set transmission selection algorithm\r\n");
#endif
                }
            }
            else
            {
                return CLI_ERR_CMD_UNRECOGNIZED;
            }
        }
    }
#endif /*#if (SYS_CPNT_ETS==TRUE)*/
    return CLI_NO_ERROR;
}


UI32_T CLI_API_ETS_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    BOOL_T in_no_command=FALSE;
    UI32_T to_detect_position   = 0;
    int  ret;
    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
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
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_ETS:
                        in_no_command=TRUE;
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_ETS:
                        in_no_command=FALSE;
                        break;
                    default:
                        break;
                }
            }

            if(arg[to_detect_position] == NULL)
            {
                return CLI_ERR_CMD_INVALID;
            }
            /* ETS mode
             */
            else if((arg[to_detect_position][0] == 'm' || arg[to_detect_position][0] == 'M'))
            {
                ETS_TYPE_MODE_T mode;
                if(in_no_command==TRUE)
                {
                     mode=ETS_TYPE_MODE_OFF;
                }
                else
                {
                    if(arg[to_detect_position+1][0] == 'a' || arg[to_detect_position+1][0] == 'a')
                    {
                        mode=ETS_TYPE_MODE_AUTO;
                    }
                    else if(arg[to_detect_position+1][0] == 'o' || arg[to_detect_position+1][0] == 'O')
                    {
                        mode=ETS_TYPE_MODE_USER;
                    }
                    else
                    {
                        continue;
                    }
                }
                if((ret=ETS_PMGR_SetMode(lport, mode))!=ETS_TYPE_RETURN_OK)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set mode\r\n");
#endif
                }
            }
            else
            {
                return CLI_ERR_CMD_UNRECOGNIZED;
            }
        }
    }
#endif /*#if (SYS_CPNT_ETS==TRUE)*/
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Show_ETS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T to_detect_position   = 0, interface_position=0;
    UI32_T unit=0, port=0, trunk_id=0, lport=0;
    CLI_API_TrunkStatus_T trunk_verify_ret;
    CLI_API_EthStatus_T port_verify_ret;
    UI8_T  display_type =  DISPLAY_INF_ALL;
    UI8_T  display_cont =  DISPLAY_MAPPING_WEIGHT;
    SWCTRL_Lport_Type_T lport_type=0;
    UI32_T line_num = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*weight or mapping*/
    if(arg[to_detect_position] == NULL)
    {
        display_cont = DISPLAY_MAPPING_WEIGHT;
    }
    else if(arg[to_detect_position][0] == 'w' || arg[to_detect_position][0] == 'W')
    {
        display_cont = DISPLAY_WEIGHT;
    }
    else if(arg[to_detect_position][0] == 'm' || arg[to_detect_position][0] == 'M')
    {
        display_cont = DISPLAY_MAPPING;
    }else
    {
        return CLI_ERR_CMD_UNRECOGNIZED;
    }

    to_detect_position++;

   /*interface*/
   if(arg[to_detect_position] != NULL)
   {
        if( (arg[to_detect_position+1] != NULL)&&
          (((arg[to_detect_position+1][0] == 'e' || arg[to_detect_position+1][0] == 'E') )||
           ( arg[to_detect_position+1][0] == 'p' || arg[to_detect_position+1][0] == 'P') ))
        {
            to_detect_position++;
            interface_position  = to_detect_position;
            to_detect_position += 2;

            if((arg[interface_position] != NULL)&&(arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E')) /*ethernet*/
            {
                unit = atoi((char*)arg[interface_position+1]);
                port = atoi(strchr((char*)arg[interface_position+1],'/')+1);

#if (CLI_SUPPORT_PORT_NAME == 1)
                if ((arg[interface_position+1] != NULL) && (isdigit(arg[interface_position+1][0])))
                {
                    unit = atoi((char*)arg[interface_position+1]);
                    port = atoi(strchr((char*)arg[interface_position+1],'/')+1);
                }
                else/*port name*/
                {
                    UI32_T trunk_id = 0;

                    if (!IF_PMGR_IfnameToIfindex(arg[interface_position+1], &lport))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        sprintf(buff, "%s does not exist.\r\n",arg[interface_position+1]);
                        PROCESS_MORE(buff);
#endif
                        return CLI_ERR_CMD_INCOMPLETECLI_NO_ERROR;
                    }

                    SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
                }
#endif
                port_verify_ret = verify_ethernet(unit, port, &lport);
                if(  port_verify_ret != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(port_verify_ret, unit, port);
                    return CLI_NO_ERROR;
                }
                display_type =  DISPLAY_INF_PORT;
            }
            else    /* port-channel */
            {
                trunk_id = atoi((char*)arg[interface_position+1]);

                if( (trunk_verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(trunk_verify_ret, trunk_id);
                    return CLI_NO_ERROR;
                }
                display_type =  DISPLAY_INF_TRUNK;
            }
        }

    }

    {
        if (display_type == DISPLAY_INF_PORT || display_type == DISPLAY_INF_TRUNK)
        {
            if((line_num = Show_one_interface_ets(lport, display_cont, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
        else if (display_type == DISPLAY_INF_ALL)
        {
            for(lport=1;lport<SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
                if (lport_type == SWCTRL_LPORT_NORMAL_PORT)
                {
                    sprintf(buff, "Interface Eth %lu/%2lu\r\n", (unsigned long)unit, (unsigned long)port);
                    PROCESS_MORE(buff);
                    if((line_num = Show_one_interface_ets(lport, display_cont, line_num)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                }
                else if(lport_type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    UI32_T lport_tmp;
                    if( (trunk_verify_ret = verify_trunk(trunk_id, &lport_tmp)) != CLI_API_TRUNK_OK)
                    {
                        continue;
                    }
                    sprintf(buff, "Interface Trunk %2lu\r\n", (unsigned long)trunk_id);
                    PROCESS_MORE(buff);
                    if((line_num = Show_one_interface_ets(lport, display_cont, line_num)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }
#endif /*#if (SYS_CPNT_ETS==TRUE)*/
    return CLI_NO_ERROR;
}

static UI32_T Show_one_interface_ets(UI32_T lport, UI8_T display_cont, UI32_T line_num)
{
#if (SYS_CPNT_ETS==TRUE)
    UI32_T          tsa, tc, weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], i;
    ETS_TYPE_MODE_T mode, oper_mode;
    char            buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char            *mode_tag_p[] = {"Off", "On", "Auto"};

    PROCESS_MORE_FUNC(" Configuration:\r\n");
    ETS_PMGR_GetMode(lport,  &mode);
    sprintf(buff, " ETS Mode: %-4s\r\n", mode_tag_p[mode]);
    PROCESS_MORE_FUNC(buff);

    if ((display_cont == DISPLAY_MAPPING_WEIGHT) || (display_cont == DISPLAY_MAPPING))
    {
        PROCESS_MORE_FUNC(" Priority Traffic Class\r\n");
        PROCESS_MORE_FUNC(" -------- -------------\r\n");
        for(i=0; i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
        {
            ETS_PMGR_GetPortPrioAssign(lport,  i, &tc, ETS_TYPE_DB_CONFIG);
            sprintf(buff, " %8lu %13lu\r\n",(unsigned long)i, (unsigned long)tc);
            PROCESS_MORE_FUNC(buff);
        }
    }

    if ((display_cont == DISPLAY_MAPPING_WEIGHT) || (display_cont == DISPLAY_WEIGHT))
    {
        PROCESS_MORE_FUNC(" Traffic Class Tx Selection Mode Weight%\r\n");
        PROCESS_MORE_FUNC(" ------------- ----------------- -------\r\n");
        ETS_PMGR_GetWeight(lport, weight, ETS_TYPE_DB_CONFIG);
        for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
        {
            ETS_PMGR_GetTSA( lport, i, &tsa, ETS_TYPE_DB_CONFIG);
            sprintf(buff, " %13lu %-17s %7lu\r\n",(unsigned long)i, (tsa==ETS_TYPE_TSA_ETS)?"ETS":"Strict",(unsigned long)weight[i]);
            PROCESS_MORE_FUNC(buff);
        }
    }

    PROCESS_MORE_FUNC("\r\n");

    PROCESS_MORE_FUNC(" Operational:\r\n");
    ETS_PMGR_GetOperMode(lport,  &oper_mode);
    sprintf(buff, " ETS Mode: %-4s\r\n", mode_tag_p[oper_mode]);
    PROCESS_MORE_FUNC(buff);

    if (ETS_TYPE_MODE_OFF != oper_mode)
    {
        if ((display_cont == DISPLAY_MAPPING_WEIGHT) || (display_cont == DISPLAY_MAPPING))
        {
            PROCESS_MORE_FUNC(" Priority Traffic Class\r\n");
            PROCESS_MORE_FUNC(" -------- -------------\r\n");
            for(i=0; i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
            {
                ETS_PMGR_GetPortPrioAssign(lport,  i, &tc, ETS_TYPE_DB_OPER);
                sprintf(buff, " %8lu %13lu\r\n",(unsigned long)i, (unsigned long)tc);
                PROCESS_MORE_FUNC(buff);
            }
        }

        if ((display_cont == DISPLAY_MAPPING_WEIGHT) || (display_cont == DISPLAY_WEIGHT))
        {
            PROCESS_MORE_FUNC(" Traffic Class Tx Selection Mode Weight%\r\n");
            PROCESS_MORE_FUNC(" ------------- ----------------- -------\r\n");
            ETS_PMGR_GetWeight(lport, weight, ETS_TYPE_DB_OPER);
            for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
            {
                ETS_PMGR_GetTSA( lport, i, &tsa, ETS_TYPE_DB_OPER);
                sprintf(buff, " %13lu %-17s %7lu\r\n",(unsigned long)i, (tsa==ETS_TYPE_TSA_ETS)?"ETS":"Strict",(unsigned long)weight[i]);
                PROCESS_MORE_FUNC(buff);
            }
        }
    }

    PROCESS_MORE_FUNC("\r\n");

#endif /*#if (SYS_CPNT_ETS==TRUE)*/

   return line_num;
}

#undef DISPLAY_INF_ALL
#undef DISPLAY_INF_PORT
#undef DISPLAY_INF_TRUNK

#undef DISPLAY_MAPPING_WEIGHT
#undef DISPLAY_MAPPING
#undef DISPLAY_WEIGHT

