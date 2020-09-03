/*-------------------------------------------------------------------------
 * Module Name  :   dhcp_backdoor.c
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for DHCP
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *  0.1 2002.06.10  --  Penny Chang, created
 *  0.2	2002.09.06  --  Penny Chang, added backdoor functions for server
 *  0.3 2003.09.18  --  Jamescyl, Enhanced DHCP Relay Server APIs
 *  0.4 2003.10.16  --  Jamescyl, Remove useless APIs
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */


/************************************
 ***   INCLUDE FILE DECLARATIONS  ***
 ************************************/
#include "l_inet.h"
#include "cli_lib.h"
#include "vlan_lib.h"
#include "backdoor_mgr.h"
#include "memory.h"
#include "l_threadgrp.h"
#include "ip_service_proc_comm.h"
#include "swctrl_pmgr.h"
#include "netcfg_pmgr_ip.h"
#include "sys_type.h"
#include "sys_time.h"
#include "dhcp_mgr.h"
#include "dhcp_wa.h"
#include "dhcp_om.h"
#include "dhcp_backdoor.h"
#include "dhcp_error_print.h"


#define MAXLINE 255
 /* MACRO FUNCTION DECLARATIONS */
#define DHCP_EXPAND_IP(ip)  ((unsigned char *)&(ip))[0],((unsigned char *)&(ip))[1],((unsigned char *)&(ip))[2],((unsigned char *)&(ip))[3]

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void DHCP_BACKDOOR_Main(void);
static void DHCP_BACKDOOR_MainMenu(void);
static void DHCP_BACKDOOR_ConfigMenu(void);
static void DHCP_BACKDOOR_DatabaseMenu(void);
static void DHCP_BACKDOOR_DebugMenu(void);
static void DHCP_BACKDOOR_WA_Interface_Info_Menu(void);
static void DHCP_BACKDOOR_WA_Print_Interface_Info(DHCP_WA_InterfaceDhcpInfo_T *if_config);
static void DHCP_BACKDOOR_OM_Interface_Info_Menu(void);
static void DHCP_BACKDOOR_OM_Print_Interface_Info(struct interface_info *if_info);
static void DHCP_BACKDOOR_GetOmLcbInformation(void);
static void DHCP_BACKDOOR_OM_DisplayUCData(void);
static void DHCP_BACKDOOR_TrapDhcpPacket_Menu(void);
static void DHCP_BACKDOOR_SendInform(void);


/* STATIC VARIABLE DECLARATIONS */
static L_THREADGRP_Handle_T tg_handle;
static UI32_T backdoor_member_id;
static UI32_T dhcp_backdoor_debug_flag;
static DHCP_BACKDOOR_CommandStruct_T dhcp_main_menu_cmd[]=
{
    /* cmd index*/                            /* cmd destription */    /* cmd function */
    {DHCP_BACKDOOR_MAIN_MENU_CONFIG_INDEX,   "DHCP CONFIG",            DHCP_BACKDOOR_ConfigMenu},
    {DHCP_BACKDOOR_MAIN_MENU_DATABASE_INDEX, "DHCP DATABASE",          DHCP_BACKDOOR_DatabaseMenu},
    {DHCP_BACKDOOR_MAIN_MENU_DEBUG_INDEX,    "DHCP DEBUG",             DHCP_BACKDOOR_DebugMenu},
    {DHCP_BACKDOOR_MAIN_MENU_END_INDEX,      "",                       NULL}

};

static DHCP_BACKDOOR_CommandStruct_T dhcp_config_menu_cmd[]=
{
    /* cmd index*/                                   /* cmd destription */    /* cmd function */
    {DHCP_BACKDOOR_CONFIG_MENU_TRAP_PKT_INDEX,        "Trap DHCP packet",      DHCP_BACKDOOR_TrapDhcpPacket_Menu},
    {DHCP_BACKDOOR_CONFIG_MENU_SEND_INFORM_PKT_INDEX, "Send DHCP Infom",       DHCP_BACKDOOR_SendInform},
    {DHCP_BACKDOOR_CONFIG_MENU_END_INDEX,             "",                      NULL}

};

static DHCP_BACKDOOR_CommandStruct_T dhcp_database_menu_cmd[]=
{
    /* cmd index*/                                    /* cmd destription */        /* cmd function */
    {DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_WA_INTF_INFO,"Display WA interface info", DHCP_BACKDOOR_WA_Interface_Info_Menu},
    {DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_LCB_INFO, "Display OM lcb info",       DHCP_BACKDOOR_GetOmLcbInformation},
    {DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_UC_DATA,  "Display OM UC data",        DHCP_BACKDOOR_OM_DisplayUCData},
    {DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_INTF_INFO,"Display OM interface info", DHCP_BACKDOOR_OM_Interface_Info_Menu},
    {DHCP_BACKDOOR_DATABASE_MENU_END_INDEX,           "",                          NULL}

};




/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_BACKDOOR_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DHCP_BACKDOOR_Init(void)
{
    return;
} /* End of DHCP_BACKDOOR_Init */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_GetDebugFlag
 *------------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *------------------------------------------------------------------------------
 */
BOOL_T  DHCP_BACKDOOR_GetDebugFlag(UI32_T flag)
{
    return ( (dhcp_backdoor_debug_flag & flag) != 0);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DHCP_BACKDOOR_Create_InterCSC_Relation(void)
{
#if 1
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("dhcp",
                                                      SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
                                                      DHCP_BACKDOOR_Main);
#else
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("dhcp", DHCP_BACKDOOR_Menu);
#endif
    /* Init debug flag */
    dhcp_backdoor_debug_flag = 0;
    return;
} /* end of DHCP_BACKDOOR_Create_InterCSC_Relation */

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_Main
 *------------------------------------------------------------------------------
 * PURPOSE: This function will enter backdoor main function
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_Main(void)
{
#if 0
    int ch;
    BOOL_T eof = FALSE;
    char buf[16] ={0};
    char *terminal;

    L_THREADGRP_Handle_T tg_handle;
    UI32_T               backdoor_member_id;
#endif
    tg_handle      = IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* Enter DHCP backdoor main menu
     */
    DHCP_BACKDOOR_MainMenu();
#if 0
    while (eof == FALSE)
    {
        memset(buf, 0, sizeof(buf));
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("0.  Exit\n");
        BACKDOOR_MGR_Printf("1.  Set MGR Debug Flag\n");
        BACKDOOR_MGR_Printf("2.  Set DHCP ALGO Debug Flag\n");
        BACKDOOR_MGR_Printf("3.  Set DHCP Server Debug Flag\n");
        BACKDOOR_MGR_Printf("4.  DHCP WA interface info\n");
        BACKDOOR_MGR_Printf("5.  DHCP OM interface info\n");
        BACKDOOR_MGR_Printf("6.  Get OM LCB information\n");
        BACKDOOR_MGR_Printf("7.  Trap DHCP packet\n");
#if (SYS_CPNT_DHCP_INFORM == TRUE)
        BACKDOOR_MGR_Printf("8.  Send DHCPINFORM\n");
#endif
        BACKDOOR_MGR_Printf("    select =");

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:  /* Set MGR Debug Flag */
            {
                BOOL_T mgr_exit = FALSE;

                while(!mgr_exit)
                {
                    BACKDOOR_MGR_Printf("\n (1)Debug");
                    BACKDOOR_MGR_Printf("\n (2)Info");
                    BACKDOOR_MGR_Printf("\n (3)Note");
                    BACKDOOR_MGR_Printf("\n (0)Exit");
                    BACKDOOR_MGR_Printf("\n Select:");
                    BACKDOOR_MGR_RequestKeyIn(buf,15);
                    switch(atoi(buf))
                    {
                        case 0:
                            mgr_exit = TRUE;
                            break;
                        case 1:
                            DEBUG_FLAG=(DEBUG_FLAG^DEBUG_FLAG_BIT_DEBUG);
                        break;

                        case 2:
                            DEBUG_FLAG=(DEBUG_FLAG^DEBUG_FLAG_BIT_INFO);
                        break;

                        case 3:
                            DEBUG_FLAG=(DEBUG_FLAG^DEBUG_FLAG_BIT_NOTE);
                        break;

                        default:
                            BACKDOOR_MGR_Printf("\n invalid input!");
                    }

                    BACKDOOR_MGR_Printf("\n Debug flag: %s",(DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0 ?"OFF":"ON");
                    BACKDOOR_MGR_Printf("\n Info  flag: %s",(DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0 ?"OFF":"ON");
                    BACKDOOR_MGR_Printf("\n Note  flag: %s",(DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0 ?"OFF":"ON");
                    BACKDOOR_MGR_Printf("\n");
                }

            }
                break;

            case 2: /* Set DHCP ALGO Debug Flag */
            {

                UI32_T flag = 0;
                BOOL_T algo_exit = FALSE;

                while(!algo_exit)
                {
                    BACKDOOR_MGR_Printf("\n (1)Option82");
                    BACKDOOR_MGR_Printf("\n (2)Dynamic provision");
                    BACKDOOR_MGR_Printf("\n (3)Socket");
                    BACKDOOR_MGR_Printf("\n (0)Exit:");
                    BACKDOOR_MGR_Printf("\n Select:");
                    BACKDOOR_MGR_RequestKeyIn(buf,15);
                    DHCP_ALGO_GetDebugFlag(&flag);
                    switch(atoi(buf))
                    {
                        case 0:
                            algo_exit = TRUE;
                            break;
                        case 1:
                            /* option 82 debug flag */
                            flag = flag^DHCP_ALGO_DEBUG_FLAG_BIT_OPTION82;
                            DHCP_ALGO_SetDebugFlag(flag);
                        break;

                        case 2:
                            /* dynamic provision debug flag */
                            flag = flag^DHCP_ALGO_DEBUG_FLAG_BIT_DDP;
                            DHCP_ALGO_SetDebugFlag(flag);
                        break;

                        case 3:
                            /* socket debug flag */
                            flag = flag^DHCP_ALGO_DEBUG_FLAG_BIT_SOCKET;
                            DHCP_ALGO_SetDebugFlag(flag);
                        break;
                        default:
                            BACKDOOR_MGR_Printf("\n invalid input!");
                    }

                    DHCP_ALGO_GetDebugFlag(&flag);
                    BACKDOOR_MGR_Printf("\n DHCP ALGO debug flag:");
                    BACKDOOR_MGR_Printf("\n Option82(%s)",(flag&DHCP_ALGO_DEBUG_FLAG_BIT_OPTION82)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("\n Dynamic provision(%s)",(flag&DHCP_ALGO_DEBUG_FLAG_BIT_DDP)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("\n Socket(%s)",(flag&DHCP_ALGO_DEBUG_FLAG_BIT_SOCKET)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("\n");
                }



            }
                break;
            case 3: /* Set DHCP server debug flag */
            {
                BOOL_T server_exit = FALSE;

                while(!server_exit)
                {
                    BACKDOOR_MGR_Printf("\n (1)Server debug flag");
                    BACKDOOR_MGR_Printf("\n (0)Exit:");
                    BACKDOOR_MGR_Printf("\n Select:");
                    BACKDOOR_MGR_RequestKeyIn(buf,15);
                    switch(atoi(buf))
                    {
                        case 0:
                            server_exit = TRUE;
                            break;
                        case 1:
                            /* server debug flag */
                            dhcp_server_debug_flag = ~dhcp_server_debug_flag;
                        break;
                        default:
                            BACKDOOR_MGR_Printf("\n invalid input!");
                    }


                    BACKDOOR_MGR_Printf("\n DHCP server debug flag(%s)",dhcp_server_debug_flag?"ON":"OFF");
                    BACKDOOR_MGR_Printf("\n");
                }
            }
                break;
            case 4: /* DHCP WA interface info */
            {
                DHCP_MGR_Backdoor_WA_Interface_Info();
            }
                break;
            case 5: /* DHCP OM interface info */
            {
                DHCP_MGR_Backdoor_OM_Interface_Info();
            }
                break;
            case 6:
            {
                L_THREADGRP_Execution_Request(tg_handle,backdoor_member_id);
                DHCP_MGR_Backdoor_GetOmLcbInformation();
                L_THREADGRP_Execution_Release(tg_handle,backdoor_member_id);
            }
                break;
            case 7:
            {
                L_THREADGRP_Execution_Request(tg_handle,backdoor_member_id);
                DHCP_MGR_Backdoor_TrapDhcpPacket();
                L_THREADGRP_Execution_Release(tg_handle,backdoor_member_id);
            }
                break;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
            case 8:
            {
                UI32_T vid_ifindex =0;
                L_THREADGRP_Execution_Request(tg_handle,backdoor_member_id);
                BACKDOOR_MGR_Printf("\n vid ifindex:");
                BACKDOOR_MGR_RequestKeyIn(buf,15);
                vid_ifindex = strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf ("\n");
                NETCFG_PMGR_IP_SetDhcpInform(vid_ifindex, TRUE);
                DHCP_MGR_Restart3(DHCP_MGR_RESTART_CLIENT);

                L_THREADGRP_Execution_Release(tg_handle,backdoor_member_id);
            }
                break;
#endif

            default:
                ch = 0;
                break;
        }
    }
#endif
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_MainMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will display DHCP main menu,
 *           and select debug information
 * INPUT   : None
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_MainMenu(void)
{

    char line_buffer[MAXLINE];
    int  select_value = 0;
    UI8_T cmd_idx=0;
    while (1)
    {
        DHCP_BD_MSG("\r\n------------- DHCP Backdoor Main Menu ------------");
        cmd_idx=0;
        while((cmd_idx+1)!= DHCP_BACKDOOR_MAIN_MENU_END_INDEX)
        {
            DHCP_BD_MSG("\r\n\t %u : %s",dhcp_main_menu_cmd[cmd_idx].cmd_idx,
                                                   dhcp_main_menu_cmd[cmd_idx].cmd_description);
            cmd_idx++;
        }
        DHCP_BD_MSG("\r\n\t 0 : Return");
        DHCP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        DHCP_BD_MSG("\r\nEnter Selection:");

        if (DHCP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        if((select_value>=DHCP_BACKDOOR_MAIN_MENU_CONFIG_INDEX)&&
           (select_value<=DHCP_BACKDOOR_MAIN_MENU_DEBUG_INDEX))
        {
            dhcp_main_menu_cmd[select_value-1].cmd_function();
        }

        /* exit */
        if(select_value == 0)
        {
            DHCP_BD_MSG("\r\n");
            break;
        }

    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_ConfigMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter config menu
 *
 * INPUT   : None
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_ConfigMenu(void)
{
    char line_buffer[MAXLINE];
    int  select_value = 0;
    UI8_T cmd_idx=0;
    while (1)
    {
        DHCP_BD_MSG("\r\n---- DHCP Config Menu ----");
        cmd_idx=0;
        while((cmd_idx+1)!=DHCP_BACKDOOR_CONFIG_MENU_END_INDEX)
        {
            DHCP_BD_MSG("\r\n\t %u : %s",dhcp_config_menu_cmd[cmd_idx].cmd_idx,
                                                   dhcp_config_menu_cmd[cmd_idx].cmd_description);
            cmd_idx++;
        }

        DHCP_BD_MSG("\r\n\t 0 : Return");
        DHCP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        DHCP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (DHCP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        if((select_value>=DHCP_BACKDOOR_CONFIG_MENU_TRAP_PKT_INDEX)&&
           (select_value<=DHCP_BACKDOOR_CONFIG_MENU_SEND_INFORM_PKT_INDEX))
        {
            dhcp_config_menu_cmd[select_value-1].cmd_function();
        }

        /* exit */
        if(select_value == 0)
        {
            DHCP_BD_MSG("\r\n");
            break;
        }


    }
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_DatabaseMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter database menu
 *
 * INPUT   : None
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_DatabaseMenu(void)
{
    char line_buffer[MAXLINE];
    int  select_value = 0;
    UI8_T cmd_idx=0;
    while (1)
    {
        DHCP_BD_MSG("\r\n---- DHCP Database Menu ----");
        cmd_idx=0;
        while((cmd_idx+1)!=DHCP_BACKDOOR_DATABASE_MENU_END_INDEX)
        {
            DHCP_BD_MSG("\r\n\t %u : %s",dhcp_database_menu_cmd[cmd_idx].cmd_idx,
                                                   dhcp_database_menu_cmd[cmd_idx].cmd_description);
            cmd_idx++;
        }

        DHCP_BD_MSG("\r\n\t 0 : Return");
        DHCP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        DHCP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (DHCP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        if((select_value>=DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_WA_INTF_INFO)&&
           (select_value<=DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_INTF_INFO))
        {
            dhcp_database_menu_cmd[select_value-1].cmd_function();
        }

        /* exit */
        if(select_value == 0)
        {
            DHCP_BD_MSG("\r\n");
            break;
        }


    }
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_DebugMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter debug menu
 *
 * INPUT   : None
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_DebugMenu(void)
{
    char line_buffer[MAXLINE];
    int  select_value = 0;
    const UI32_T debug_flag[7]={DHCP_BD_FLAG_CONFIG,
                                DHCP_BD_FLAG_EVENT,
                                DHCP_BD_FLAG_PACKET,
                                DHCP_BD_FLAG_CLIENT,
                                DHCP_BD_FLAG_RELAY,
                                DHCP_BD_FLAG_SERVER,
                                DHCP_BD_FLAG_DATABASE};
    while (1)
    {
        DHCP_BD_MSG("\r\n---- DHCP Debug Menu ----");
        DHCP_BD_MSG("\r\n\t 0 : Return");
        DHCP_BD_MSG("\r\n\t 1 : CONFIG(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CONFIG)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 2 : EVENT(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_EVENT)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 3 : PACKET(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_PACKET)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 4 : CLIENT(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 5 : RELAY(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 6 : SERVER(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_SERVER)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 7 : DATABASE(%s)",
                        DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_DATABASE)?"ON":"OFF");
        DHCP_BD_MSG("\r\n\t 8 : Set all");
        DHCP_BD_MSG("\r\n\t 9 : Unset all");
        DHCP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        DHCP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (DHCP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch(select_value)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                dhcp_backdoor_debug_flag ^= debug_flag[select_value-1];
                break;
            case 8:
                dhcp_backdoor_debug_flag = DHCP_BD_FLAG_CONFIG|
                                           DHCP_BD_FLAG_EVENT|
                                           DHCP_BD_FLAG_PACKET|
                                           DHCP_BD_FLAG_CLIENT|
                                           DHCP_BD_FLAG_RELAY|
                                           DHCP_BD_FLAG_SERVER|
                                           DHCP_BD_FLAG_DATABASE;
                break;
            case 9:
                dhcp_backdoor_debug_flag = DHCP_BD_FLAG_NONE;
                break;
            case 0:
                /* exit */
                DHCP_BD_MSG("\r\n");
                return;
        }
    }
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_WA_Interface_Info_Menu
 *------------------------------------------------------------------------------
 * PURPOSE: backdoor menu of DHCP WA interface information
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_WA_Interface_Info_Menu(void)
{
     BOOL_T exit = FALSE;
     char   buf[16] ={0};
     int    ch=0;
     char   *terminal;
     while(exit == FALSE)
     {
        memset(buf, 0, sizeof(buf));
        BACKDOOR_MGR_Printf("\n (1) Dump All WA Interface Information");
        BACKDOOR_MGR_Printf("\n (2) Dump One WA Interface Information");
        BACKDOOR_MGR_Printf("\n (0) exit ");
        BACKDOOR_MGR_Printf("\n     Select:");
        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch)
        {
            case 0:   /* exit */
                exit = TRUE;
                break;
            case 1:   /* Dump All WA Interface Information */
            {

                UI32_T vid_ifindex = 0;
                DHCP_WA_InterfaceDhcpInfo_T  if_config;
                memset(&if_config, 0, sizeof(if_config));
                BACKDOOR_MGR_Printf("\n Dump All Interface Information");
                BACKDOOR_MGR_Printf("\n ==============================");
                while(DHCP_WA_GetNextIfConfig(vid_ifindex, &if_config))
                {
                    DHCP_BACKDOOR_WA_Print_Interface_Info(&if_config);
                    BACKDOOR_MGR_Printf("\n ****************************** ");
                    vid_ifindex = if_config.ifIndex;
                }

            }
                break;
            case 2:   /* Dump One WA Interface Information */
            {
                UI32_T vid_ifindex=0;
                DHCP_WA_InterfaceDhcpInfo_T  if_config;


                BACKDOOR_MGR_Printf("\n Enter Interface Vid:");
                memset(buf, 0, sizeof(buf));
                BACKDOOR_MGR_RequestKeyIn(buf, 15);
                ch = (int) strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf ("\n");

                VLAN_VID_CONVERTTO_IFINDEX(ch, vid_ifindex);
                memset(&if_config, 0, sizeof(if_config));
                if(DHCP_WA_GetIfConfig(vid_ifindex, &if_config))
                    DHCP_BACKDOOR_WA_Print_Interface_Info(&if_config);
                else
                   BACKDOOR_MGR_Printf("\n Can't find interface!");
            }
                break;
            default:
                BACKDOOR_MGR_Printf("\n Not valid input, please enter again!");
                BACKDOOR_MGR_Printf("\n");
                break;
        }
     }

     return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_WA_Print_Interface_Info
 *------------------------------------------------------------------------------
 * PURPOSE: print out DHCP WA interface information
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_WA_Print_Interface_Info(DHCP_WA_InterfaceDhcpInfo_T *if_config)
{
     UI8_T index;
     BACKDOOR_MGR_Printf("\n ifindex       :%lu",(unsigned long)if_config->ifIndex);
     BACKDOOR_MGR_Printf("\n assigned_ip   :%s",if_config->assigned_ip?"TRUE":"FALSE");
     BACKDOOR_MGR_Printf("\n binding role  :%lu",(unsigned long)if_config->if_binding_role);
     BACKDOOR_MGR_Printf("\n ip            :%u.%u.%u.%u",DHCP_EXPAND_IP(if_config->if_ip));
     BACKDOOR_MGR_Printf("\n server ip     :%u.%u.%u.%u",DHCP_EXPAND_IP(if_config->if_server_ip));
     BACKDOOR_MGR_Printf("\n gateway       :%u.%u.%u.%u",DHCP_EXPAND_IP(if_config->if_gateway));
     BACKDOOR_MGR_Printf("\n client port   :%u",if_config->client_port);
     BACKDOOR_MGR_Printf("\n server port   :%u",if_config->server_port);

     for(index=0;index<DHCP_WA_MAX_NBR_OF_RELAY_SERVER;index++)
     {
         if(index==0)
             BACKDOOR_MGR_Printf("\n relay server  :");
         else
             BACKDOOR_MGR_Printf("\n               :");
         BACKDOOR_MGR_Printf("%u.%u.%u.%u",DHCP_EXPAND_IP(if_config->relay_server[index]));
     }
     /* DHCP_MGR_ClientId_T	cid; */   /* not in use */
	 BACKDOOR_MGR_Printf("\n vendor class id information(option 60)");
     BACKDOOR_MGR_Printf("\n ======================================");
     BACKDOOR_MGR_Printf("\n vendor mode   :%lu",(unsigned long)if_config->classid.vendor_mode);
     BACKDOOR_MGR_Printf("\n vendor len    :%lu",(unsigned long)if_config->classid.vendor_len);
     BACKDOOR_MGR_Printf("\n vendor content:%s",if_config->classid.vendor_buf);

     return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_OM_DisplayUCData
 *------------------------------------------------------------------------------
 * PURPOSE: backdoor to display OM UC data
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_OM_DisplayUCData(void)
{
    DHCP_OM_UC_LEASE_DATA_T uc_lease;
    BOOL_T ret;

    memset(&uc_lease, 0, sizeof(uc_lease));

    ret = DHCP_OM_GetUCLeaseData(&uc_lease);
    if(!ret)
        return;
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("UC Lease data:\n");
    BACKDOOR_MGR_Printf("   vid_ifindex = %lu\n",(unsigned long)uc_lease.vid_ifindex);
    BACKDOOR_MGR_Printf("   address\n");
    BACKDOOR_MGR_Printf("       len     = %lu\n",(unsigned long)uc_lease.address.len);
    BACKDOOR_MGR_Printf("       iabuf   = ");
    {
        UI8_T idx;
        for(idx=0;idx<16;idx++)
            BACKDOOR_MGR_Printf("%02X",uc_lease.address.iabuf[idx]);
        BACKDOOR_MGR_Printf("\n");
    }
    BACKDOOR_MGR_Printf("   expiry      = %lu\n",(unsigned long)uc_lease.expiry);
    BACKDOOR_MGR_Printf("   renewal     = %lu\n",(unsigned long)uc_lease.renewal);
    BACKDOOR_MGR_Printf("   rebind      = %lu\n",(unsigned long)uc_lease.rebind);
    BACKDOOR_MGR_Printf("   is_bootp    = %s\n",uc_lease.is_bootp?"TRUE":"FALSE");
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_OM_Interface_Info_Menu
 *------------------------------------------------------------------------------
 * PURPOSE: backdoor menu of DHCP OM interface information
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_OM_Interface_Info_Menu(void)
{
     BOOL_T exit = FALSE;
     char   buf[16] ={0};
     int    ch=0;
     char   *terminal;
     while(exit == FALSE)
     {
        memset(buf, 0, sizeof(buf));
        BACKDOOR_MGR_Printf("\n (1) Dump All OM Interface Information");
        BACKDOOR_MGR_Printf("\n (2) Dump One OM Interface Information");
        BACKDOOR_MGR_Printf("\n (0) exit ");
        BACKDOOR_MGR_Printf("\n     Select:");
        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch)
        {
            case 0:   /* exit */
                exit = TRUE;
                break;
            case 1:   /* Dump All OM Interface Information */
            {
                struct interface_info *if_ptr = NULL;
                struct interface_info *if_ptr_tmp = NULL;
                BACKDOOR_MGR_Printf("\n Dump All Interface Information");
                BACKDOOR_MGR_Printf("\n ==============================");
                while((if_ptr = DHCP_OM_GetNextInterface(if_ptr_tmp)))
                {
                    DHCP_BACKDOOR_OM_Print_Interface_Info(if_ptr);
                    BACKDOOR_MGR_Printf("\n ****************************** ");
                    if_ptr_tmp = if_ptr;
                }
            }
                break;
            case 2:   /* Dump One OM Interface Information */
            {
                UI32_T vid_ifindex=0;
                struct interface_info *if_ptr = NULL;
                struct interface_info *if_ptr_tmp = NULL;
                BACKDOOR_MGR_Printf("\n Enter Interface Vid:");
                memset(buf, 0, sizeof(buf));
                BACKDOOR_MGR_RequestKeyIn(buf, 15);
                ch = (int) strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf ("\n");

                VLAN_VID_CONVERTTO_IFINDEX(ch, vid_ifindex);
                while((if_ptr = DHCP_OM_GetNextInterface(if_ptr_tmp)))
                {
                    if(if_ptr->vid_ifIndex == vid_ifindex)
                        break;
                    else
                        if_ptr_tmp = if_ptr;
                }

                DHCP_BACKDOOR_OM_Print_Interface_Info(if_ptr);
            }
                break;
            default:
                BACKDOOR_MGR_Printf("\n Not valid input, please enter again!");
                BACKDOOR_MGR_Printf("\n");
                break;
        }
    }

     return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_OM_Print_Interface_Info
 *------------------------------------------------------------------------------
 * PURPOSE: print out DHCP OM interface information
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_OM_Print_Interface_Info(struct interface_info *if_info)
{
    UI8_T index;
    if(if_info == NULL)
        return;

    BACKDOOR_MGR_Printf("\n ifindex       :%lu",(unsigned long)if_info->vid_ifIndex);
    BACKDOOR_MGR_Printf("\n mode          :%lu",(unsigned long)if_info->mode);
    BACKDOOR_MGR_Printf("\n role          :%lu",(unsigned long)if_info->role);
    BACKDOOR_MGR_Printf("\n primary ip    :%u.%u.%u.%u",DHCP_EXPAND_IP(if_info->primary_address));
    BACKDOOR_MGR_Printf("\n client port   :%lu",(unsigned long)if_info->client_port);
    BACKDOOR_MGR_Printf("\n server port   :%lu",(unsigned long)if_info->server_port);
    BACKDOOR_MGR_Printf("\n server ip     :%u.%u.%u.%u",DHCP_EXPAND_IP(if_info->server_ip));
    BACKDOOR_MGR_Printf("\n gateway ip    :%u.%u.%u.%u",DHCP_EXPAND_IP(if_info->gateway_ip));
#if (SYS_CPNT_DHCP_INFORM == TRUE)
    BACKDOOR_MGR_Printf("\n dhcp inform   :%s",if_info->dhcp_inform?"TRUE":"FALSE");
#endif
    for(index=0;index<MAX_RELAY_SERVER;index++)
    {
        if(index==0)
            BACKDOOR_MGR_Printf("\n relay server  :");
        else
            BACKDOOR_MGR_Printf("\n               :");
        BACKDOOR_MGR_Printf("%u.%u.%u.%u",DHCP_EXPAND_IP(if_info->relay_server_list[index]));
}
    BACKDOOR_MGR_Printf("\n vendor class id information(option 60)");
    BACKDOOR_MGR_Printf("\n ======================================");
    BACKDOOR_MGR_Printf("\n vendor mode   :%lu",(unsigned long)if_info->classid.vendor_mode);
    BACKDOOR_MGR_Printf("\n vendor len    :%lu",(unsigned long)if_info->classid.vendor_len);
    BACKDOOR_MGR_Printf("\n vendor content:%s",if_info->classid.vendor_buf);
    BACKDOOR_MGR_Printf("\n");
    /* client information */
    if(if_info->client)
    {
        UI32_T idx;
        BACKDOOR_MGR_Printf("\n Client information");
        BACKDOOR_MGR_Printf("\n ======================================");
        BACKDOOR_MGR_Printf("\n active lease  :%s",if_info->client->active?"TRUE":"FALSE");
        if(if_info->client->active)
        {
            for(idx=0;idx<256;idx++)
            {
                if(if_info->client->active->options[idx].len!=0)
                {
                    UI32_T data_index;
                    BACKDOOR_MGR_Printf("\n option %u:",idx);
                    for(data_index=0;data_index<(if_info->client->active->options[idx].len);data_index++)
                        BACKDOOR_MGR_Printf("%02X",if_info->client->active->options[idx].data[data_index]);
                }
            }
        }
    }
    BACKDOOR_MGR_Printf("\n");
#if 0   /* other interface information */

    struct shared_network *shared_network;  /* Networks connected to this interface. */
    struct hardware hw_address;             /* Its physical address. */
    /* Only used by DHCP client code. */
    struct client_state *client
#endif

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_GetOmLcbInformation
 *------------------------------------------------------------------------------
 * PURPOSE: get DHCP OM lcb information
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_GetOmLcbInformation(void)
{
    DHCP_OM_LCB_T om_lcb;

    memset(&om_lcb, 0, sizeof(om_lcb));
    DHCP_OM_GetLcbInformation(&om_lcb);
    BACKDOOR_MGR_Printf("\n DHCP OM LCB Information");
    BACKDOOR_MGR_Printf("\n =======================");
    BACKDOOR_MGR_Printf("\n system role :%lu",(unsigned long)om_lcb.system_role);
    BACKDOOR_MGR_Printf("\n relay count :%lu",(unsigned long)om_lcb.system_relay_count);
    BACKDOOR_MGR_Printf("\n client count:%lu",(unsigned long)om_lcb.system_client_count);
    BACKDOOR_MGR_Printf("\n");
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_TrapDhcpPacket_Menu
 *------------------------------------------------------------------------------
 * PURPOSE: backdoor menu of trap dhcp packet
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_TrapDhcpPacket_Menu(void)
{
    char buf[16] ={0};
    BOOL_T exit = FALSE;
    BOOL_T trap = FALSE;
    SWCTRL_TrapPktOwner_T flag=0;
    while(!exit)
    {
        BACKDOOR_MGR_Printf("\n Trap DHCP packet rule to cpu");
        BACKDOOR_MGR_Printf("\n (1)Enable (2)Disable");
        BACKDOOR_MGR_Printf("\n Select:");
        BACKDOOR_MGR_RequestKeyIn(buf,15);
        switch(atoi(buf))
        {
            case 1:
                trap = TRUE;
            break;
            case 2:
                trap = FALSE;
            break;
            default:
                BACKDOOR_MGR_Printf("\n invalid input!");
                continue;
        }

        BACKDOOR_MGR_Printf("\n By which CSC");
        BACKDOOR_MGR_Printf("\n (1) DHCP client");
        BACKDOOR_MGR_Printf("\n (2) DHCP snooping");
        BACKDOOR_MGR_Printf("\n (3) DHCP L2 relay");
        BACKDOOR_MGR_Printf("\n (4) DHCP L3 relay");
        BACKDOOR_MGR_Printf("\n (5) DHCP server");
        BACKDOOR_MGR_Printf("\n Select:");
        memset(buf, 0, sizeof(buf));
        BACKDOOR_MGR_RequestKeyIn(buf,15);
        flag = 0;
        switch(atoi(buf))
        {
            case 1:
                flag = SWCTRL_DHCP_TRAP_BY_DHCP_CLIENT;
            break;
            case 2:
                flag = SWCTRL_DHCP_TRAP_BY_DHCPSNP;
            break;
            case 3:
                flag = SWCTRL_DHCP_TRAP_BY_L2_RELAY;
            break;
            case 4:
                flag = SWCTRL_DHCP_TRAP_BY_L3_RELAY;
            break;
            case 5:
                flag = SWCTRL_DHCP_TRAP_BY_DHCP_SERVER;
            break;
            default:
                BACKDOOR_MGR_Printf("\n invalid input!");
                continue;
        }

        if(trap)
            SWCTRL_PMGR_EnableDhcpTrap(flag);
        else
            SWCTRL_PMGR_DisableDhcpTrap(flag);

        exit = TRUE;
    }
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME : DHCP_BACKDOOR_SendInform
 *------------------------------------------------------------------------------
 * PURPOSE: send DHCP Inform packet
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
static void DHCP_BACKDOOR_SendInform(void)
{

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    char   line_buffer[MAXLINE];
    UI32_T vid_ifindex =0;

    DHCP_BD_MSG("\n vid ifindex:");
    if (DHCP_BD_GetLine(line_buffer, MAXLINE) > 0)
    {
        vid_ifindex = atoi(line_buffer);
    }

    DHCP_BD_MSG ("\n");
    NETCFG_PMGR_IP_SetDhcpInform(vid_ifindex, TRUE);
    DHCP_MGR_Restart3(DHCP_MGR_RESTART_CLIENT);
#else
    DHCP_BD_MSG("\n Not support.");
#endif
    return;
}

