/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_backdoor.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file contains the debugging information of PoE driver.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created
 *    07/25/2007 - Daniel Chen, Porting Broadcom series PoE ASIC
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "poedrv.h"
#include "poedrv_om.h"
#include "poedrv_type.h"
#include "poedrv_control.h"
//#include "leaf_es3626a.h"
#include "leaf_3621.h"
#include "stktplg_pom.h"
//#include "stktplg_board.h"
#include "poe_image.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE 255
#define PACKET_LENGTH 12

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void POEDRV_BACKDOOR_SetPortAdminStatus(void);
static void POEDRV_BACKDOOR_SetPortAutoMode(void);
static void POEDRV_BACKDOOR_SetPortPriority(void);
static void POEDRV_BACKDOOR_ResetPort(void);
static void POEDRV_BACKDOOR_ShowPortInfo(void);
//static void POEDRV_BACKDOOR_ShowPortPowerConsumption(void);
//static void POEDRV_BACKDOOR_ShowPortPowerClassification(void);
static void POEDRV_BACKDOOR_ShowMainpowerStatus(void);
static void POEDRV_BACKDOOR_SoftwareReset(BOOL_T is_enable);
static void POEDRV_BACKDOOR_SetPortDetectionType(void);
static void POEDRV_BACKDOOR_SendPoePacket(void);
//static void POEDRV_BACKDOOR_SetLegacy(void);
//static void POEDRV_BACKDOOR_ShowPowerMode(void);
static void POEDRV_BACKDOOR_ShowAllConfig(void);
static void POEDRV_BACKDOOR_ShowAllCounter(void);
static void POEDRV_BACKDOOR_ShowPortPowerClassification(void);
static void POEDRV_BACKDOOR_SetPortPowerLimit(void);
static void POEDRV_BACKDOOR_readAddr(void);
static void POEDRV_BACKDOOR_writeAddr(void);
static void POEDRV_BACKDOOR_SetPowerControl(void);
static void POEDRV_BACKDOOR_SetForceHighPowerMode(void);
static void POEDRV_BACKDOOR_SetDot3atHighPowerMode(void);
static void POEDRV_BACKDOOR_SetThresholdType(void);
static void POEDRV_BACKDOOR_SetClassificationType(void);
static void POEDRV_BACKDOOR_SetIcutAndIlimInHighPower(void);
static void POEDRV_BACKDOOR_ResetPortStatistic(void);

/* STATIC VARIABLE DECLARATIONS
 */

static BOOL_T   backdoor_display_packet  = FALSE;
static BOOL_T   backdoor_display_notify  = FALSE;
static BOOL_T   backdoor_display_debug   = FALSE;
static BOOL_T   backdoor_software_reset  = TRUE;
static UI32_T   backdoor_poe_min_port = 1;
static UI32_T   backdoor_poe_max_port = 24;

static POEDRV_CONTROL_T *backdoor_hooked_control = NULL;

//static UI32_T               backdoor_member_id;

/* EXPORTED SUBPROGRAM BODIES
 */




/* show Poedrv backdoor menu */
static void POEDRV_BACKDOOR_ShowMenu(void)
{
    BACKDOOR_MGR_Printf("\n\r");
	BACKDOOR_MGR_Printf("\n\r---------------------------------------------------");
	BACKDOOR_MGR_Printf("\n\r-----   PoE Driver Engineer Menu 2007/07/25   -----");
	BACKDOOR_MGR_Printf("\n\r---------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r [0] Exit");
    BACKDOOR_MGR_Printf("\n\r [1] set port admin status");
    BACKDOOR_MGR_Printf("\n\r [2] Set port priority");
    BACKDOOR_MGR_Printf("\n\r [3] Show port info");
    BACKDOOR_MGR_Printf("\n\r [4] Show mainpower status");
    BACKDOOR_MGR_Printf("\n\r [5] PoE port reset");
    BACKDOOR_MGR_Printf("\n\r [6] set port detection type");
    BACKDOOR_MGR_Printf("\n\r [7] Set port power limit");

    BACKDOOR_MGR_Printf("\n\r [8] Software reset");
    if (backdoor_software_reset)
        BACKDOOR_MGR_Printf("[ports powering enabled]");
    else
        BACKDOOR_MGR_Printf("[ports powering disabled]");

    BACKDOOR_MGR_Printf("\n\r [9] Display PoE packet");
    if (backdoor_display_packet)
        BACKDOOR_MGR_Printf("[enabled]");
    else
        BACKDOOR_MGR_Printf("[disabled]");
    BACKDOOR_MGR_Printf("\n\r [a] Display Notification to callback function");
    if (backdoor_display_notify)
        BACKDOOR_MGR_Printf("[enabled]");
    else
        BACKDOOR_MGR_Printf("[disabled]");
    BACKDOOR_MGR_Printf("\n\r [b] Display Debug information");
    if (backdoor_display_debug)
        BACKDOOR_MGR_Printf("[enabled]");
    else
        BACKDOOR_MGR_Printf("[disabled]");

    BACKDOOR_MGR_Printf("\n\r [c] Send a packet to PoE controller");
    BACKDOOR_MGR_Printf("\n\r [d] polling function state");
    if (POEDRV_OM_IsStopMonitorFlagOn()==FALSE)
        BACKDOOR_MGR_Printf("[enabled]");
    else
        BACKDOOR_MGR_Printf("[disabled]");
    BACKDOOR_MGR_Printf("\n\r [e] set main power infomation.");
    BACKDOOR_MGR_Printf("\n\r [f] set power auto mode.");
    BACKDOOR_MGR_Printf("\n\r [?] Show Menu");
    BACKDOOR_MGR_Printf("\n\r [h] PoE module hardware reset.");
    BACKDOOR_MGR_Printf("\n\r [i] show All port admins_status/priority/power_limit/detection_type");
    BACKDOOR_MGR_Printf("\n\r [j] show All port counter");
    BACKDOOR_MGR_Printf("\n\r [l] read CPU memory space address");
    BACKDOOR_MGR_Printf("\n\r [n] set 'Power Threshold Type'");
    BACKDOOR_MGR_Printf("\n\r [m] write CPU memory space address");
    BACKDOOR_MGR_Printf("\n\r [o] show all port measurement");
    BACKDOOR_MGR_Printf("\n\r [p] Set classification type");
    BACKDOOR_MGR_Printf("\n\r [s] Set force high power mode");
    BACKDOOR_MGR_Printf("\n\r [v] Set dot3at (DLL) high power mode");
    BACKDOOR_MGR_Printf("\n\r [t] Set Icut/Ilim in high power mode");
    BACKDOOR_MGR_Printf("\n\r [k] Show power classification of a port");
    BACKDOOR_MGR_Printf("\n\r [u] upgrade Application image");
    BACKDOOR_MGR_Printf("\n\r [x] reset port statistic");
}




BOOL_T __poedrv_debug_flag = FALSE;

/* FUNCTION NAME: POEDRV_BACKDOOR_Main
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void POEDRV_BACKDOOR_Main (void)
{
    UI32_T unit_id, unit_num;
#if 0 /* Eugene marked for not using universal image */
    UI32_T board_id;
#endif
    UI8_T ch;

#if 0 /* Eugene marked for not using universal image */
    STKTPLG_BOARD_BoardInfo_T  *board_info_p;
#endif

    STKTPLG_POM_GetMyUnitID(&unit_id);
    STKTPLG_POM_GetNumberOfUnit(&unit_num);

#if 0 /* Eugene marked for not using universal image */
    /* Get board_id */
    if ( STKTPLG_MGR_GetUnitBoardID(unit_id, &board_id) )
    {
        /* Get POEDRV board_info and set relative vlaue */
        if ( STKTPLG_BOARD_GetBoardInformation( board_id, &board_info_p))
        {
            backdoor_poe_min_port = board_info_p->min_poe_port_num;
            backdoor_poe_max_port = board_info_p->max_poe_port_num;
        }
        else
        {
            BACKDOOR_MGR_Printf("\n\r*** Can not get related board information.***\n");
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\rGet Board error !!\n");
        return;
    }
#else
    backdoor_poe_min_port = POEDRV_TYPE_PSE_MIN_PORT_NUMBER;
    backdoor_poe_max_port = POEDRV_TYPE_PSE_MAX_PORT_NUMBER;
#endif

    if (POEDRV_Control_Hook(&backdoor_hooked_control)==FALSE)
    {
        BACKDOOR_MGR_Printf("\n\rPoedrv backdoor hooks control function fail !! \n");
        return;
    }

    POEDRV_BACKDOOR_ShowMenu();
    while(1)
    {
        BACKDOOR_MGR_Printf("\n\r\n\r\n\r");
        BACKDOOR_MGR_Printf("Enter Selection (? for help page) : ");

        ch = getchar();
        switch(ch)
        {
            case '0':
                BACKDOOR_MGR_Printf("\n\r Exit from PoE Driver Engineer Menu");
                return;
            case '1':
                POEDRV_BACKDOOR_SetPortAdminStatus();
                break;
            case '2':
                POEDRV_BACKDOOR_SetPortPriority();
                break;
            case '3':
                POEDRV_BACKDOOR_ShowPortInfo();
                break;
            case '4':
                POEDRV_BACKDOOR_ShowMainpowerStatus();
                break;
            case '5':
                POEDRV_BACKDOOR_ResetPort();
                break;
            case '6':
                POEDRV_BACKDOOR_SetPortDetectionType();
                break;
            case '7':
                POEDRV_BACKDOOR_SetPortPowerLimit();
                break;
            case '8':
                if (backdoor_software_reset)
                {
                    backdoor_software_reset = FALSE;
                    POEDRV_BACKDOOR_SoftwareReset(backdoor_software_reset);
                }
                else
                {
                    backdoor_software_reset = TRUE;
                    POEDRV_BACKDOOR_SoftwareReset(backdoor_software_reset);
                }
                break;
            case '9':
                backdoor_display_packet = !backdoor_display_packet;
                break;
            case 'a':
            case 'A':
                backdoor_display_notify = !backdoor_display_notify;
                break;
            case 'b':
            case 'B':
                backdoor_display_debug = !backdoor_display_debug;
                break;
            case 'c':
            case 'C':
                POEDRV_BACKDOOR_SendPoePacket();
                break;
            case 'd':
                POEDRV_OM_SetStopMonitorFlag(!POEDRV_OM_IsStopMonitorFlagOn());
                break;
            case 'e':
                POEDRV_BACKDOOR_SetPowerControl();
                break;
            case 'f':
                POEDRV_BACKDOOR_SetPortAutoMode();
                break;
            case '?':
                POEDRV_BACKDOOR_ShowMenu();
                break;
            case 'h':
                POEDRV_HardwareReset();
                // POEDRV_BACKDOOR_HardwareReset();
                break;
            case 'i':
                POEDRV_BACKDOOR_ShowAllConfig();
                break;
            case 'j':
                POEDRV_BACKDOOR_ShowAllCounter();
                break;
            case 'k':
                POEDRV_BACKDOOR_ShowPortPowerClassification();
                break;
            case 'l':
                POEDRV_BACKDOOR_readAddr();
                break;
            case 'm':
                POEDRV_BACKDOOR_writeAddr();
                break;
            case 'n':
                POEDRV_BACKDOOR_SetThresholdType();
                break;
            case 'o':
            {
                UI32_T i;
                I32_T temp;
                UI32_T volt, cur;
                UI32_T pwr;

                for (i=backdoor_poe_min_port;i<=backdoor_poe_max_port;i++)
                {
                    POEDRV_GetPortPowerConsumption(1, i, &pwr);
                    POEDRV_GetPoePortTemperature(1, i, &temp);
                    POEDRV_GetPoePortVoltage(1, i, &volt);
                    POEDRV_GetPoePortCurrent(1, i, &cur);
                    BACKDOOR_MGR_Printf("port: %2lu, used pwr: %d mW, temp: %d C, volt: %lu V, current: %lu mA\n", 
                        i, pwr, temp, volt, cur);
                }
                break;
            }
            case 'p':
                POEDRV_BACKDOOR_SetClassificationType();
                break;
            case 's':
                POEDRV_BACKDOOR_SetForceHighPowerMode();
                break;
            case 'v':
                POEDRV_BACKDOOR_SetDot3atHighPowerMode();
                break;
            case 't':
                POEDRV_BACKDOOR_SetIcutAndIlimInHighPower();
                break;
            case 'u':
            {
                BOOL_T ret;

                POEDRV_EXEC(backdoor_hooked_control->poedrv_upgrade_image, ret,
                    poe_image, POE_IMAGE_SIZE);
                if (ret == TRUE)
                    BACKDOOR_MGR_Printf("Upgrade done.\n");
                else
                    BACKDOOR_MGR_Printf("Upgrade fail.\n");
                break;
            }
            case 'x':
            {
                POEDRV_BACKDOOR_ResetPortStatistic();
                break;
            }
            case 'z':
            {
                __poedrv_debug_flag = !__poedrv_debug_flag;
                BACKDOOR_MGR_Printf("current debug flag : %d\n", __poedrv_debug_flag);
                break;
            }
            default:
                break;
        }
#if 0
        BACKDOOR_MGR_Printf("\n\rPress any key to continue.");
        BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE);
#endif
    }
} /* End of POEDRV_BACKDOOR_Main() */


/* FUNCTION NAME: POEDRV_BACKDOOR_DisplayRecvPacket
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void POEDRV_BACKDOOR_DisplayPacket(BOOL_T is_transmit, UI8_T *buf)
{
    UI32_T    index;

    if (is_transmit)
        BACKDOOR_MGR_Printf("\n\rTx PoE packet= ");
    else
        BACKDOOR_MGR_Printf("\n\rRx PoE packet= ");

    for (index=0;index<PACKET_LENGTH;index++)
    {
         BACKDOOR_MGR_Printf("%02x-", buf[index]);
    }
}


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayPacketFlagOn
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayPacketFlagOn()
{
     return (backdoor_display_packet);

} /* End of POEDRV_BACKDOOR_IsDisplayPacketFlagOn() */


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayNotifyFlagOn
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayNotifyFlagOn()
{
     return (backdoor_display_notify);

} /* End of POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() */


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayDebugFlagOn
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayDebugFlagOn()
{
     return (backdoor_display_debug);

} /* End of POEDRV_BACKDOOR_IsDisplayDebugFlagOn() */


/* LOCAL SUBPROGRAM BODIES
 */
static void POEDRV_BACKDOOR_SetForceHighPowerMode(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    int select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- POEDRV_BACKDOOR_SetForceHighPowerMode ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port high power (0-normal, 1-high):");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set error !!");
        return;
    }

    if (select_value!=1 && select_value != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set high power value at port %ld !!", port);
        return;
    }

    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_force_high_power_mode, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                return;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_force_high_power_mode, ret,
                phy_port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set high power mode");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
}

static void POEDRV_BACKDOOR_SetDot3atHighPowerMode(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    int select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- POEDRV_BACKDOOR_SetDot3atHighPowerMode ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port high power (0-normal, 1-high):");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set error !!");
        return;
    }

    if (select_value!=1 && select_value != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set high power value at port %ld !!", port);
        return;
    }

    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_dot3at_high_power_mode, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                return;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_dot3at_high_power_mode, ret,
                phy_port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set high power mode");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
}


static void POEDRV_BACKDOOR_ResetPortStatistic(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    port, phy_port;
    BOOL_T    ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- ResetPortStatistic ---------------------");
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n");
        return;
    }


    if (port != 0)
    {

        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            BACKDOOR_MGR_Printf("port error !\n");
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_reset_port_statistic, ret,
            phy_port);
    }
    else
    {
        for (port = backdoor_poe_min_port ; port <= backdoor_poe_max_port ; port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                continue;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_reset_port_statistic, ret,
                phy_port);
        }
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to reset port statistic");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to reset port statistic : %d !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

}


static void POEDRV_BACKDOOR_SetClassificationType(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    int select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetClassificationType ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port classification type (0-bypass, 1-802.3af classification):");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set error !!");
        return;
    }

    if (select_value!=1 && select_value != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);
        return;
    }

    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_classification_type, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                return;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_classification_type, ret,
                phy_port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set port classification type");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
}


static void POEDRV_BACKDOOR_SetThresholdType(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    int select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPortThresholdType ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port threshold type (0-None, 1-Class-base, 2-UserDefine):");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set error !!");
        return;
    }

    if (select_value!=1 && select_value != 0 && select_value != 2)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);
        return;
    }

    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_power_threshold_type, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                return;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_power_threshold_type, ret,
                phy_port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set port threshold type");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
}

/* FUNCTION NAME: POEDRV_BACKDOOR_SetIcutAndIlimInHighPower
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetIcutAndIlimInHighPower(void)
{
    char line_buffer[MAXLINE];
    BOOL_T ret;
    I32_T select_value = 0;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetIcutAndIlimInHighPower ---------------------\n");
    BACKDOOR_MGR_Printf("0 = Ilim range : 504mA ¡V 584mA ; Icut 450mA(Min)/465mA(Typ)/480mA(Max)\n");
    BACKDOOR_MGR_Printf("1 = Ilim range : 563mA ¡V 650mA ; Icut 530mA(Min)/545mA(Typ)/560mA(Max)\n");
    BACKDOOR_MGR_Printf("2 = Ilim range : 850mA ¡V 1.1A ; Icut 660mA(Min)/675mA(Typ)/690mA(Max)\n");
    BACKDOOR_MGR_Printf("3 = Ilim range : 850mA ¡V 1.1A ; Icut 730mA(Min)/745mA(Typ)/760mA(Max)\n");
    BACKDOOR_MGR_Printf("\n\r Set Icut and Ilim in high power (0..3) : ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Icut/Ilim Error !!");
        return;
    }

    if (select_value < 0 || select_value > 3)
    {
        BACKDOOR_MGR_Printf("\n\r Error Icut/Ilim mode : %d !!", select_value);
        return;
    }

    POEDRV_EXEC(backdoor_hooked_control->poedrv_set_current_in_high_power, ret,
            select_value);

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set Icut/Ilim mode");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set Icut/Ilim mode");

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

}

/* FUNCTION NAME: POEDRV_BACKDOOR_SetPortAdminStatus
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetPortAdminStatus(void)
{
    char line_buffer[MAXLINE];
    UI32_T port;
    UI32_T unit = 1;
    int select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPortAdminStatus ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port admin status (1/0) : ");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    if (select_value!=1 && select_value != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);
        return;
    }

    select_value = select_value==0?VAL_pethPsePortAdminEnable_false:VAL_pethPsePortAdminEnable_true;

    if (port != 0)
    {
        ret = POEDRV_SetPortAdminStatus(unit, port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            ret = POEDRV_SetPortAdminStatus(unit, port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set port admin status");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* End of POEDRV_BACKDOOR_SetPortAdminStatus() */

/* FUNCTION NAME: POEDRV_BACKDOOR_SetPortAutoMode
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetPortAutoMode(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    UI32_T select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPortAutoMode ---------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set port Auto Mode (1/0) : ");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    if (select_value!=1 && select_value != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);
        return;
    }

    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            return;
    
        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_auto_mode, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
                continue;

            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_auto_mode, ret,
                phy_port, select_value);
            if (ret == FALSE)
                break;
        }
    }


    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set port admin status");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* End of POEDRV_BACKDOOR_SetPortAutoMode() */


/* FUNCTION NAME: POEDRV_BACKDOOR_SetPortPriority
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetPortPriority(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    port;
    UI32_T    unit = 1;
    UI32_T    priority;
    int       select_value = 0;
    BOOL_T    ret = FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- Set Port Priority ------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }

    port = select_value;

    BACKDOOR_MGR_Printf("\n\r Enter Priority (1-critical, 2-high, 3-low) : ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port Priority Error !!");
    }


    priority = select_value;

    if ( (priority >= VAL_pethPsePortPowerPriority_critical) && (priority <= VAL_pethPsePortPowerPriority_low) )
    {
        if (port != 0 )
        {
            if ( POEDRV_SetPortPowerPriority(unit, port, priority) == TRUE )
                ret = TRUE;
        }
        else
        {
            for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
            {
                ret = POEDRV_SetPortPowerPriority(unit, port, priority);
                if (ret == FALSE)
                    break;
            }
        }
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set priority %ld ", priority);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set priority %ld to port %ld", priority, port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* End of POEDRV_BACKDOOR_SetPortPriority() */


/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPortInfo
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPortInfo(void)
{
    char      line_buffer[MAXLINE];
    UI8_T     ret;
    UI32_T    port, phy_port;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- Show Port Info ------------------");
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);


    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n\r");
        return;
    }


    if (port != 0)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            BACKDOOR_MGR_Printf("port error\n\r");
            return ;
        }
        BACKDOOR_MGR_Printf("\n\r===========> Port %d\n\r", port);
        POEDRV_EXEC(backdoor_hooked_control->poedrv_show_port_info, ret,
            phy_port);
    }
    else
    {
        for (port= backdoor_poe_min_port; port <= backdoor_poe_max_port; port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                continue;
            }
            BACKDOOR_MGR_Printf("\n\r===========> Port %d\n\r", port);
            POEDRV_EXEC(backdoor_hooked_control->poedrv_show_port_info, ret,
                phy_port);
        }
    }

} /* End of POEDRV_BACKDOOR_ShowPortInfo() */

#if 0 /* Eugene mark */
/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPortPowerConsumption
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPortPowerConsumption(void)
{
    UI32_T    port, phy_port;
    UI32_T    unit = 1;
    UI32_T    milliwatts=0;
    UI32_T    power_limit=0;
    BOOL_T    ret;

    BACKDOOR_MGR_Printf("\n\r\n\r---------- Show Port Power Consumption -------------");
    BACKDOOR_MGR_Printf("\n\r          Consumption     power limit");
    BACKDOOR_MGR_Printf("\n\r          -----------     -----------");

    for (port=backdoor_poe_min_port;port<=backdoor_poe_max_port;port++)
    {

        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            continue;
        }
        BACKDOOR_MGR_Printf("\n\rPort %2ld = ", port);
        if ( POEDRV_GetPortPowerConsumption(unit,port, &milliwatts) == TRUE )
            BACKDOOR_MGR_Printf("%4ld", milliwatts);
        else
            BACKDOOR_MGR_Printf("?");

        POEDRV_EXEC(backdoor_hooked_control->poedrv_get_port_power_limit, ret,
            phy_port, &power_limit);
        if ( ret == TRUE )
            BACKDOOR_MGR_Printf("              %4ld", power_limit);
        else
            BACKDOOR_MGR_Printf("               ?");
    }

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* End of POEDRV_BACKDOOR_ShowPortPowerConsumption() */
#endif

static void POEDRV_BACKDOOR_ShowAllConfig(void)
{
    UI32_T port, phy_port;
    BOOL_T ret;
    POEDRV_PORT_CONFIG_T cfg;

    BACKDOOR_MGR_Printf("\nPort   Admin status   Priority   Allocation   power pair power mode\n");
    BACKDOOR_MGR_Printf("----   ------------   --------   ----------   --------- ---------- \n");

    for(port=backdoor_poe_min_port; port<=backdoor_poe_max_port;port++)
    {

        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port)==FALSE)
            continue;

        POEDRV_EXEC(backdoor_hooked_control->poedrv_get_port_all_config, ret,
            phy_port, &cfg);
        if (ret == TRUE)
        {
            BACKDOOR_MGR_Printf("%3d    %8s      %6d     %8d     %s     %s\n", port,
                cfg.admin_status==1?"Enable":"Disable",
                cfg.power_priority,
                cfg.power_limit,
                cfg.power_pair==1?"signal":"spare",
                cfg.power_mode==1?"High Power":"Normal");
        }
        else
        {
            BACKDOOR_MGR_Printf("%-3d\n", port);
        }
    }
}

static void POEDRV_BACKDOOR_ShowAllCounter(void)
{
    UI32_T port, phy_port;
    BOOL_T ret;
    UI8_T counter[POEDRV_MAX_COUNTER_TYPE];
    UI32_T var_ctr[POEDRV_MAX_COUNTER_TYPE];

    BACKDOOR_MGR_Printf("\nASIC Value\n");
    BACKDOOR_MGR_Printf("\nPort   mps absent     inval sig  denied       overload     short\n");
    BACKDOOR_MGR_Printf("----   ------------   --------   ----------   ---------   --------\n");

    for(port=backdoor_poe_min_port; port<=backdoor_poe_max_port;port++)
    {

        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port)==FALSE)
            continue;

        POEDRV_EXEC(backdoor_hooked_control->poedrv_get_port_all_counter, ret,
            phy_port, counter);
        if (ret == TRUE)
        {
            BACKDOOR_MGR_Printf("%3d    %8d      %6d     %8d     %8d       %d\n", port,
                counter[POEDRV_MPSABSENT_COUNTER],
                counter[POEDRV_INVALID_SIGNATURE_COUNTER],
                counter[POEDRV_POWER_DENIED_COUNTER],
                counter[POEDRV_OVERLOAD_COUNTER],
                counter[POEDRV_SHORT_COUNTER]);
        }
        else
        {
            BACKDOOR_MGR_Printf("%3d\n", port);
        }
    }

    BACKDOOR_MGR_Printf("Variable\n");
    BACKDOOR_MGR_Printf("\nPort   mps absent     inval sig  denied       overload     short\n");
    BACKDOOR_MGR_Printf("----   ------------   --------   ----------   ---------   --------\n");

    for(port=backdoor_poe_min_port; port<=backdoor_poe_max_port;port++)
    {
        if (POEDRV_GetPortMPSAbsentCounter(1, port, &var_ctr[POEDRV_MPSABSENT_COUNTER])==FALSE)
        {
            BACKDOOR_MGR_Printf("%3d [1]\n", port);
            continue;
        }
        if (POEDRV_GetPortInvalidSignCounter(1, port, &var_ctr[POEDRV_INVALID_SIGNATURE_COUNTER])==FALSE)
        {
            BACKDOOR_MGR_Printf("%3d [2]\n", port);
            continue;
        }
        if (POEDRV_GetPortPowerDeniedCounter(1, port, &var_ctr[POEDRV_POWER_DENIED_COUNTER])==FALSE)
        {
            BACKDOOR_MGR_Printf("%3d [3]\n", port);
            continue;
        }
        if (POEDRV_GetPortOverloadCounter(1, port, &var_ctr[POEDRV_OVERLOAD_COUNTER])==FALSE)
        {
            BACKDOOR_MGR_Printf("%3d [4]\n", port);
            continue;
        }
        if (POEDRV_GetPortShortCounter(1, port, &var_ctr[POEDRV_SHORT_COUNTER])==FALSE)
        {
            BACKDOOR_MGR_Printf("%3d [5]\n", port);
            continue;
        }

        BACKDOOR_MGR_Printf("%3d    %8d      %6d     %8d     %8d       %d\n", port,
            var_ctr[POEDRV_MPSABSENT_COUNTER],
            var_ctr[POEDRV_INVALID_SIGNATURE_COUNTER],
            var_ctr[POEDRV_POWER_DENIED_COUNTER],
            var_ctr[POEDRV_OVERLOAD_COUNTER],
            var_ctr[POEDRV_SHORT_COUNTER]);
    }

}

/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPortPowerClassification
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPortPowerClassification(void)
{
    UI32_T    unit = 1;
    UI32_T    power_class=0;
     int       i;

    BACKDOOR_MGR_Printf("\n\r\n\rShow Port Power Classification:");

    BACKDOOR_MGR_Printf("\n\rPort  ");
    for (i=backdoor_poe_min_port;i<=backdoor_poe_max_port;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rClass ");
    for (i=backdoor_poe_min_port;i<=backdoor_poe_max_port;i++)
    {
         if ( POEDRV_GetPortPowerClassification(unit, i, &power_class) == TRUE )
              BACKDOOR_MGR_Printf("%02x|", (UI8_T)power_class);
         else
              BACKDOOR_MGR_Printf("??|");
    }

    BACKDOOR_MGR_Printf("\n\r");
}

/* FUNCTION NAME: POEDRV_BACKDOOR_ShowMainpowerStatus
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowMainpowerStatus(void)
{
    BOOL_T ret;
    UI32_T watts;

    BACKDOOR_MGR_Printf("\n\r\n\r------------- Show MainPower Status ----------------\n");

    POEDRV_EXEC(backdoor_hooked_control->poedrv_show_mainpower_info, ret);

    if (POEDRV_GetMainpowerConsumption(1, &watts) == TRUE)
    {
        BACKDOOR_MGR_Printf("Current power consumption: %lu Watts\n", watts);
    }
    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* End of POEDRV_BACKDOOR_ShowMainpowerStatus() */


/* FUNCTION NAME: POEDRV_BACKDOOR_ResetPort
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ResetPort(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    port, phy_port;
    UI32_T    ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- ResetPort ---------------------");
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n");
        return;
    }


    if (port != 0)
    {

        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            BACKDOOR_MGR_Printf("port error !\n");
            return;
        }

        POEDRV_EXEC(backdoor_hooked_control->poedrv_reset_port, ret,
            phy_port);
    }
    else
    {
        for (port = backdoor_poe_min_port ; port <= backdoor_poe_max_port ; port++)
        {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                continue;
            }

            POEDRV_EXEC(backdoor_hooked_control->poedrv_reset_port, ret,
                phy_port);
        }
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to reset port");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to reset port : %d !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
} /* end POEDRV_BACKDOOR_ResetPort() */

/* FUNCTION NAME: POEDRV_BACKDOOR_SetPortDetectionType
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetPortDetectionType(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    I32_T select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPortDetectionType ---------------------");
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n");
        return;
    }

    BACKDOOR_MGR_Printf ("\n\n");
    BACKDOOR_MGR_Printf("0 = POEDRV_PORT_DETECTION_NONE\n");
    BACKDOOR_MGR_Printf("1 = POEDRV_PORT_DETECTION_LEGACY\n");
    BACKDOOR_MGR_Printf("2 = POEDRV_PORT_DETECTION_DOT3AF_4POINT\n");
    BACKDOOR_MGR_Printf("3 = POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY\n");
    BACKDOOR_MGR_Printf("4 = POEDRV_PORT_DETECTION_DOT3AF_2POINT\n");
    BACKDOOR_MGR_Printf("5 = POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY\n");
    BACKDOOR_MGR_Printf("\n\r Set port detection type (0..5) : ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Detection Type Error !!");
    }

    if (select_value < POEDRV_PORT_DETECTION_NONE || select_value > POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY)
    {
        BACKDOOR_MGR_Printf("\n\r Error Detection type : %d !!", select_value);
        return;
    }

    if (port !=0 )
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            BACKDOOR_MGR_Printf("port error !\n");
            return;
        }
        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_detection_type, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port; port <= backdoor_poe_max_port; port ++)
            {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                continue;
            }
            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_detection_type, ret,
                phy_port, select_value);
        }
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set detection type");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set detection type , port :%d!!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* end POEDRV_BACKDOOR_SetPortDetectionType() */


/* FUNCTION NAME: POEDRV_BACKDOOR_SetPortPowerLimit
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetPortPowerLimit(void)
{
    char line_buffer[MAXLINE];
    UI32_T port, phy_port;
    I32_T select_value = 0;
    BOOL_T ret=FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPortPowerLimit ---------------------");
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", backdoor_poe_min_port, backdoor_poe_max_port);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!");
        return;
    }


    BACKDOOR_MGR_Printf("\npower limit(0-31000 milliwatts): ");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Power limit Error !!");
    }

    if (select_value < 0 || select_value > 31000)
    {
        BACKDOOR_MGR_Printf("\n\r Error milliwatts : %d !!", select_value);
        return;
    }

    if (port !=0 )
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            BACKDOOR_MGR_Printf("port error !\n");
            return;
        }
        POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_power_limit, ret,
            phy_port, select_value);
    }
    else
    {
        for (port=backdoor_poe_min_port; port <= backdoor_poe_max_port; port ++)
            {
            if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            {
                continue;
            }
            POEDRV_EXEC(backdoor_hooked_control->poedrv_set_port_power_limit, ret,
                phy_port, select_value);
        }
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set power limit");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set power limit , port :%d!!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");

} /* end POEDRV_BACKDOOR_SetPortDetectionType() */



/* FUNCTION NAME: POEDRV_BACKDOOR_SendPoePacket
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SendPoePacket(void)
{
    UI8_T     transmit[PACKET_LENGTH];       /* Transmit buffer       */
    UI8_T     receive[PACKET_LENGTH];        /* Receive buffer        */
    char      line_buffer[MAXLINE];
    UI32_T    select_value = 0;
    int       i;


    memset(transmit, 0, sizeof(transmit));
    memset(receive, 0, sizeof(receive));

    BACKDOOR_MGR_Printf("\n\r\n\r-------- Send a packet to PoE controller -----------");
    BACKDOOR_MGR_Printf("\n\r Enter your data(in HEX): ");

    for (i=1;i<=PACKET_LENGTH;i++)
    {
        BACKDOOR_MGR_Printf("\n\r       [Byte %2d]: ", i);
        if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
        {
            select_value = (UI32_T)strtoul(line_buffer,0,16);
        }
        else
        {
            BACKDOOR_MGR_Printf("\n\rInput error\n\r");
            return;
        }

        transmit[i-1] = (UI8_T)select_value;
    }

    BACKDOOR_MGR_Printf("\n\r-------");
    for (i=0;i<PACKET_LENGTH;i++)
        BACKDOOR_MGR_Printf("---");

    if ( POEDRV_SendRawPacket(transmit, receive) == TRUE )
    {
        BACKDOOR_MGR_Printf("\n\r Byte: ");
        for (i=0;i<PACKET_LENGTH;i++)
            BACKDOOR_MGR_Printf("%2d ", i+1);

        BACKDOOR_MGR_Printf("\n\r-------");
        for (i=0;i<PACKET_LENGTH;i++)
            BACKDOOR_MGR_Printf("---");

        BACKDOOR_MGR_Printf("\n\r   Tx: ");
        for (i=0;i<PACKET_LENGTH;i++)
            BACKDOOR_MGR_Printf("%02x ", transmit[i]);

        BACKDOOR_MGR_Printf("\n\r   Rx: ");
        for (i=0;i<PACKET_LENGTH;i++)
            BACKDOOR_MGR_Printf("%02x ", receive[i]);
    }
    else
        BACKDOOR_MGR_Printf("\n\r Failed to send this packet to PoE controller");

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------");
    BACKDOOR_MGR_Printf("\n\r");
}


static void POEDRV_BACKDOOR_SetPowerControl(void)
{
    char line_buffer[MAXLINE];
    UI32_T power = 0, srcGuard = 0;
    BOOL_T ret = FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- SetPowerControl ---------------------");

    BACKDOOR_MGR_Printf("\npower limit(0-180 watts): ");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        power = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Power limit Error !!");
    }

    if (power < 0 || power > 180)
    {
        BACKDOOR_MGR_Printf("\n\r Error watts : %d !!", power);
        return;
    }

    BACKDOOR_MGR_Printf("\npower source guard(0-180 watts): ");
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        srcGuard = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Power limit Error !!");
    }

    if (srcGuard < 0 || srcGuard > 180)
    {
        BACKDOOR_MGR_Printf("\n\r Error milliwatts : %d !!", srcGuard);
        return;
    }

    POEDRV_EXEC(backdoor_hooked_control->poedrv_set_power_source_control, ret, 
        power, srcGuard);
}





static void POEDRV_BACKDOOR_SoftwareReset(BOOL_T is_enable)
{
    POEDRV_SetModuleStatus(is_enable);
} /* End of POEDRV_BACKDOOR_SoftwareReset() */


static void POEDRV_BACKDOOR_readAddr(void)
{
    char line_buffer[MAXLINE];
    UI32_T select_value = 0;
    volatile UI8_T *__ptr, data;
    
    BACKDOOR_MGR_Printf("\nInput the address: ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer,MAXLINE)>0)
    {
        select_value = (UI32_T)strtoul(line_buffer,0,16);
    }

    BACKDOOR_MGR_Printf("\n0x%x\n", select_value);

    __ptr = (UI8_T *)select_value;

    data = *__ptr;
    BACKDOOR_MGR_Printf("result: 0x%x\n", data);
    
}

static void POEDRV_BACKDOOR_writeAddr(void)
{
    char line_buffer[MAXLINE], line_buffer_1[MAXLINE];
    UI32_T select_value = 0, set_value = 0;
    volatile UI8_T *__ptr;
    
    BACKDOOR_MGR_Printf("\nInput the address: ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer,MAXLINE)>0)
    {
        select_value = (UI32_T)strtoul(line_buffer,0,16);
    }

    BACKDOOR_MGR_Printf("\nInput the value: ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer_1,MAXLINE)>0)
    {
        set_value = (UI32_T)strtoul(line_buffer_1,0,16);
    }

    BACKDOOR_MGR_Printf("\naddr: 0x%x, value: 0x%x\n", select_value, set_value);

    __ptr = (UI8_T *)select_value;
    *__ptr = (UI8_T)set_value;
}

#if 0 /*Eugene use POEDRV_HardwareReset()*/
static void POEDRV_BACKDOOR_HardwareReset(I32_T centi_sec)
{
    UI32_T t1, t2;
    BOOL_T ret;
    UI8_T version;
    UI32_T cnt = 0;
    
    // BACKDOOR_MGR_Printf("pull high - normal operation\n");
    *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = SYS_HWCFG_POE_SOFTWARE_RESET|SYS_HWCFG_POE_SOFTWARE_RESET_MASK;

    // BACKDOOR_MGR_Printf("pull low - reset PoE module, And wait for 300 ms\n");
    *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = SYS_HWCFG_POE_SOFTWARE_RESET;
    t1 = SYS_TIME_GetSystemTicksBy10ms();
    SYSFUN_Sleep(30);
    // BACKDOOR_MGR_Printf("[1] system tick: %lu\n", t1);

    // BACKDOOR_MGR_Printf("pull high - normal operation AGAIN!!!!\n");
    *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = SYS_HWCFG_POE_SOFTWARE_RESET|SYS_HWCFG_POE_SOFTWARE_RESET_MASK;
    SYSFUN_Sleep(400); /* delay for 4 seconds*/
    BACKDOOR_MGR_Printf("Reset Done.\n");
/*
    while(TRUE && cnt < 100)
    {
        SYSFUN_Sleep(10);
        t2 = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("[2] system tick: %lu\n", t2);
        POEDRV_EXEC(backdoor_hooked_control->poedrv_get_soft_ver, ret, &version);
        if (ret == TRUE)
            break;
    }
    if (cnt == 100)
        BACKDOOR_MGR_Printf("time out .....\n");
*/
} /* End of POEDRV_HardwareReset() */
#endif


