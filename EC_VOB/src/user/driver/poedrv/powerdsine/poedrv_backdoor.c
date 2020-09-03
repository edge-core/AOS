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
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
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
#include "sys_bld.h"
#include "poedrv.h"
#include "poedrv_type.h"
#include "poedrv_om.h"
//#include "Leaf_es3626a.h"
#include "leaf_3621.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "phyaddr_access.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE                                      255

/* DATA TYPE DECLARATIONS
 */


/* MACRO FUNCTIONS DECLARACTION
 */
#define POEDRV_GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(poedrv_gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_gpio_sem_id)


/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void POEDRV_BACKDOOR_EnablePort(void);
static void POEDRV_BACKDOOR_DisablePort(void);
static void POEDRV_BACKDOOR_SetPortPriority(void);
static void POEDRV_BACKDOOR_ShowPortStatus(void);
static void POEDRV_BACKDOOR_ShowPortPowerConsumption(void);
static void POEDRV_BACKDOOR_ShowPortPowerClassification(void);
static void POEDRV_BACKDOOR_ShowMainpowerStatus(void);
static void POEDRV_BACKDOOR_ShowPortPriority(void);
static void POEDRV_BACKDOOR_SoftwareReset(BOOL_T is_enable);
static void POEDRV_BACKDOOR_SendPoePacket(void);
static void POEDRV_BACKDOOR_EnableCapacitorDetection(void);
static void POEDRV_BACKDOOR_DisableCapacitorDetection(void);
//static int POEDRV_BACKDOOR_GetLine(char s[], int lim);
static void POEDRV_BACKDOOR_Getb();
static void POEDRV_BACKDOOR_HighPriorityPortDenied(void);
static void POEDRV_BACKDOOR_LowPriorityPortShutdown(void);
static void POEDRV_BACKDOOR_SetLegacy(void);
static void POEDRV_BACKDOOR_ShowPowerMode(void);
static void POEDRV_BACKDOOR_EnableClassMode(void);
static void POEDRV_BACKDOOR_DiableClassMode(void);



/* STATIC VARIABLE DECLARATIONS 
 */

static BOOL_T   backdoor_display_packet  = FALSE;
static BOOL_T   backdoor_display_notify  = FALSE;
static BOOL_T   backdoor_display_debug   = FALSE;
static BOOL_T   backdoor_software_reset  = TRUE;

static UI32_T   poedrv_gpio_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */


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
//    char     line_buffer[MAXLINE];
//    int      select_value = 0;
    UI8_T    ch;
   
    while(1)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("\n\r---------------------------------------------------");
        BACKDOOR_MGR_Printf("\n\r-----   PoE Driver Engineer Menu 2003/03/31   -----");
        BACKDOOR_MGR_Printf("\n\r---------------------------------------------------");
        BACKDOOR_MGR_Printf("\n\r [0] Exit");
        BACKDOOR_MGR_Printf("\n\r [1] Enable a port");
        BACKDOOR_MGR_Printf("\n\r [2] Disable a port"); 
        BACKDOOR_MGR_Printf("\n\r [3] Set port priority");
        BACKDOOR_MGR_Printf("\n\r [4] Show port status");
        BACKDOOR_MGR_Printf("\n\r [5] Show power consumption of a port");
        BACKDOOR_MGR_Printf("\n\r [6] Show mainpower status");
        BACKDOOR_MGR_Printf("\n\r [7] Show port priority");
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
        BACKDOOR_MGR_Printf("\n\r [d] Enable Capacitor detection");
        BACKDOOR_MGR_Printf("\n\r [e] Diable Capacitor detection");
        BACKDOOR_MGR_Printf("\n\r [f] Power management high priority port is denied when exceeds power budge");
        BACKDOOR_MGR_Printf("\n\r [g] Power management low priority port is shut down when exceeds power budge");
        BACKDOOR_MGR_Printf("\n\r [h] Getb");
        BACKDOOR_MGR_Printf("\n\r [i] Show power classification of a port");
        BACKDOOR_MGR_Printf("\n\r [j] Set Legacy of a unit");

        BACKDOOR_MGR_Printf("\n\r [k] Show Power Mode");
        BACKDOOR_MGR_Printf("\n\r [m] Enable Class Mode");
        BACKDOOR_MGR_Printf("\n\r [n] Diable Class Mode");
        BACKDOOR_MGR_Printf("\n\r [o] polling function state");
        if (POEDRV_OM_IsStopMonitorFlagOn() == FALSE)
            BACKDOOR_MGR_Printf("[enabled]");
        else
            BACKDOOR_MGR_Printf("[disabled]");        

        BACKDOOR_MGR_Printf("\n\r\n\r Enter Selection: ");
#if 0        
        if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }
#endif
        ch = getchar();        
        switch(ch)
        {
            case '0':
                BACKDOOR_MGR_Printf("\n\r Exit from PoE Driver Engineer Menu");
                return;
            case '1':
                POEDRV_BACKDOOR_EnablePort();
                break;
            case '2':
                POEDRV_BACKDOOR_DisablePort();
                break;
            case '3':
                POEDRV_BACKDOOR_SetPortPriority();
                break;
            case '4':
                POEDRV_BACKDOOR_ShowPortStatus();
                break;
            case '5':
                POEDRV_BACKDOOR_ShowPortPowerConsumption();
                break;
            case '6':
                POEDRV_BACKDOOR_ShowMainpowerStatus();
                break;
            case '7':
                POEDRV_BACKDOOR_ShowPortPriority();
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
                if (backdoor_display_packet)
                    backdoor_display_packet = FALSE;
                else
                    backdoor_display_packet = TRUE;
                break;
            case 'a':
            case 'A':
                if (backdoor_display_notify)
                    backdoor_display_notify = FALSE;
                else
                    backdoor_display_notify = TRUE;
                break;
            case 'b':
            case 'B':
                if (backdoor_display_debug)
                    backdoor_display_debug = FALSE;
                else
                    backdoor_display_debug = TRUE;
                break;
            case 'c':
            case 'C':
                POEDRV_BACKDOOR_SendPoePacket();
                break;
            case 'd':    
            case 'D':
                POEDRV_BACKDOOR_EnableCapacitorDetection();
                break;
            case 'e':    
            case 'E':
                POEDRV_BACKDOOR_DisableCapacitorDetection();
                break;
                
            case 'f':    
            case 'F':
                POEDRV_BACKDOOR_HighPriorityPortDenied();
                break;
                
            case 'g':    
            case 'G':
                POEDRV_BACKDOOR_LowPriorityPortShutdown();
                break;

            case 'H':    
            case 'h':
                POEDRV_BACKDOOR_Getb();
                break;
            case 'I':
            case 'i':	
                POEDRV_BACKDOOR_ShowPortPowerClassification();
                break;
            case 'J':
            case 'j':
            	POEDRV_BACKDOOR_SetLegacy();
                break;
            case 'K':
            case 'k':
            	POEDRV_BACKDOOR_ShowPowerMode();            	
            	break;
            case 'M':
            case 'm':
            	POEDRV_BACKDOOR_EnableClassMode();
            	break;
            case 'N':
            case 'n':
            	POEDRV_BACKDOOR_DiableClassMode();		
                break;
            case 'O':
            case 'o':
                POEDRV_OM_SetStopMonitorFlag(!POEDRV_OM_IsStopMonitorFlagOn());
                break;
            default:
                break;
        }
#if 0
        BACKDOOR_MGR_Printf("\n\rPress <enter> to continue.");
        BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
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

    for (index=0;index<15;index++)
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

/* FUNCTION NAME: POEDRV_BACKDOOR_EnablePort
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_EnablePort(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    port;
    UI32_T    unit = 1;
    int       select_value = 0;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- Enable a Port ---------------------");  
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", SYS_ADPT_POE_PSE_MIN_PORT_NUMBER, SYS_ADPT_POE_PSE_MAX_PORT_NUMBER); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    port = select_value;

    if ( POEDRV_SetPortAdminStatus(unit, port, VAL_pethPsePortAdminEnable_true) == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to enable port %ld", port);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to enable port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 
 
} /* End of POEDRV_BACKDOOR_EnablePort() */


/* FUNCTION NAME: POEDRV_BACKDOOR_DisablePort
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_DisablePort(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    port;
    UI32_T    unit = 1;
    int       select_value = 0;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- Disable a Port --------------------");  
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", SYS_ADPT_POE_PSE_MIN_PORT_NUMBER, SYS_ADPT_POE_PSE_MAX_PORT_NUMBER); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    port = select_value;

    if ( POEDRV_SetPortAdminStatus(unit, port, VAL_pethPsePortAdminEnable_false) == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to disable port %ld", port);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to disable port %ld !!", port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_DisablePort() */


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
    BACKDOOR_MGR_Printf("\n\r Enter Port ID (%d-%d, 0 for all ports) : ", SYS_ADPT_POE_PSE_MIN_PORT_NUMBER, SYS_ADPT_POE_PSE_MAX_PORT_NUMBER); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    port = select_value;

    BACKDOOR_MGR_Printf("\n\r Enter Priority (1-critical, 2-high, 3-low) : "); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }

    priority = select_value;

    if ( (priority >= VAL_pethPsePortPowerPriority_critical) && (priority <= VAL_pethPsePortPowerPriority_low) )
    {
         if ( POEDRV_SetPortPowerPriority(unit, port, priority) == TRUE )
              ret = TRUE;
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set priority %ld to port %ld", priority, port);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set priority %ld to port %ld", priority, port);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_SetPortPriority() */


/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPortStatus
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPortStatus(void)
{
    UI8_T     port_status;
    UI32_T    unit = 1;
    int       i;

    BACKDOOR_MGR_Printf("\n\r\n\rShow Port Status:");   

    BACKDOOR_MGR_Printf("\n\rPort  ");
    if (SYS_ADPT_POE_PSE_MAX_PORT_NUMBER <= 24)
    {
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
             BACKDOOR_MGR_Printf("%2d|", i);

        BACKDOOR_MGR_Printf("\n\rStatus");
        for (i=1;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
        {
             if ( POEDRV_GetPortStatus(unit, i, &port_status) == TRUE )
                  BACKDOOR_MGR_Printf("%02x|", port_status);
             else
                  BACKDOOR_MGR_Printf("??|"); 
        }
        BACKDOOR_MGR_Printf("\n\r"); 
    }
    else
    {
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rStatus");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
    {
         if ( POEDRV_GetPortStatus(unit, i, &port_status) == TRUE )
              BACKDOOR_MGR_Printf("%02x|", port_status);
         else
              BACKDOOR_MGR_Printf("??|"); 
    }
	BACKDOOR_MGR_Printf("\n\r"); 

	BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rStatus");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
    {
         if ( POEDRV_GetPortStatus(unit, i, &port_status) == TRUE )
              BACKDOOR_MGR_Printf("%02x|", port_status);
         else
              BACKDOOR_MGR_Printf("??|"); 
    }
    BACKDOOR_MGR_Printf("\n\r"); 
    }
} /* End of POEDRV_BACKDOOR_ShowPortStatus() */


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
    UI32_T    port;
    UI32_T    unit = 1;
    UI32_T    milliwatts=0;
    UI32_T    power_limit=0;

    BACKDOOR_MGR_Printf("\n\r\n\r---------- Show Port Power Consumption -------------");  
    BACKDOOR_MGR_Printf("\n\r          Consumption     power limit");
    BACKDOOR_MGR_Printf("\n\r          -----------     -----------");

    for (port=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;port<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;port++)
    {
         BACKDOOR_MGR_Printf("\n\rPort %2ld = ", port);
         if ( POEDRV_GetPortPowerConsumption(unit,port, &milliwatts) == TRUE )
              BACKDOOR_MGR_Printf("%5ld", milliwatts);
         else
              BACKDOOR_MGR_Printf("?");

         if ( POEDRV_GetPortPowerMaximumAllocation(unit,port, &power_limit) == TRUE )
              BACKDOOR_MGR_Printf("              %5ld", power_limit);
         else
              BACKDOOR_MGR_Printf("               ?");              
    }

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_ShowPortPowerConsumption() */

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

    if (SYS_ADPT_POE_PSE_MAX_PORT_NUMBER <= 24)
    {
    BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rClass ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
        {
            if ( POEDRV_GetPortPowerClassification(unit, i, &power_class) == TRUE )
                 BACKDOOR_MGR_Printf("%02x|", (UI8_T)power_class);
            else
                BACKDOOR_MGR_Printf("??|"); 
        }
        BACKDOOR_MGR_Printf("\n\r");
    }
    else
    {
    BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rClass ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
    {
         if ( POEDRV_GetPortPowerClassification(unit, i, &power_class) == TRUE )
              BACKDOOR_MGR_Printf("%02x|", (UI8_T)power_class);
         else
              BACKDOOR_MGR_Printf("??|"); 
    }
        BACKDOOR_MGR_Printf("\n\r");

        BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
            BACKDOOR_MGR_Printf("%2d|", i);

        BACKDOOR_MGR_Printf("\n\rClass ");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
        {
            if ( POEDRV_GetPortPowerClassification(unit, i, &power_class) == TRUE )
                 BACKDOOR_MGR_Printf("%02x|", (UI8_T)power_class);
            else
                BACKDOOR_MGR_Printf("??|"); 
        }
        BACKDOOR_MGR_Printf("\n\r");

    }
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
    UI32_T    unit = 1;
    UI32_T    watts=0, milliwatts=0;
    UI8_T     system_mask=0;

    BACKDOOR_MGR_Printf("\n\r\n\r------------- Show MainPower Status ----------------");  


    if ( POEDRV_GetMainpowerConsumption(unit, &watts) == TRUE )
         BACKDOOR_MGR_Printf("\n\r MainPower consumption : %ld watts", watts);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to get mainpower consumption");

    if ( POEDRV_GetMainPowerParameters(unit, &milliwatts) == TRUE )
         BACKDOOR_MGR_Printf("\n\r MainPower MAX available : %.1lf watts", (double)milliwatts/1000);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to get mainpower max. available");

    if ( POEDRV_GetSystemMask(&system_mask) == TRUE )
         BACKDOOR_MGR_Printf("\n\r System Mask : 0x%02x", system_mask);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to get system mask");


    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_ShowMainpowerStatus() */


/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPortPriority
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPortPriority(void)
{
    UI8_T     priority;
    UI32_T    unit = 1;
    int       i;

    BACKDOOR_MGR_Printf("\n\r\n\rShow Port Priority:");   

    if (SYS_ADPT_POE_PSE_MAX_PORT_NUMBER <= 24)
    {
    BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rPrior ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
    {
         if ( POEDRV_GetPortPriority(unit, i, &priority) == TRUE )
              BACKDOOR_MGR_Printf("%02x|", priority);
         else
              BACKDOOR_MGR_Printf("??|");
    }
        BACKDOOR_MGR_Printf("\n\r"); 
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
            BACKDOOR_MGR_Printf("%2d|", i);

        BACKDOOR_MGR_Printf("\n\rPrior ");
        for (i=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;i<=24;i++)
        {
             if ( POEDRV_GetPortPriority(unit, i, &priority) == TRUE )
                 BACKDOOR_MGR_Printf("%02x|", priority);
             else
                 BACKDOOR_MGR_Printf("??|");
        }
    BACKDOOR_MGR_Printf("\n\r"); 

	BACKDOOR_MGR_Printf("\n\rPort  ");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
         BACKDOOR_MGR_Printf("%2d|", i);

    BACKDOOR_MGR_Printf("\n\rPrior ");
        for (i=25;i<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;i++)
        {
            if ( POEDRV_GetPortPriority(unit, i, &priority) == TRUE )
                BACKDOOR_MGR_Printf("%02x|", priority);
            else
                BACKDOOR_MGR_Printf("??|");
        }
        BACKDOOR_MGR_Printf("\n\r"); 
    }
} /* End of POEDRV_BACKDOOR_ShowPortPriority() */


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
    UI8_T     transmit[15];       /* Transmit buffer       */
    UI8_T     receive[15];        /* Receive buffer        */
    char      line_buffer[MAXLINE];
    UI32_T    select_value = 0;
    int       i;

    BACKDOOR_MGR_Printf("\n\r\n\r-------- Send a packet to PoE controller -----------");  
    BACKDOOR_MGR_Printf("\n\r Enter your data(in HEX): "); 
   
    for (i=1;i<=13;i++)
    {
         BACKDOOR_MGR_Printf("\n\r       [Byte %2d]: ", i);
         if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
         {
             select_value = (UI32_T)strtoul(line_buffer,0,16);
         }
         else 
             return;

         transmit[i-1] = (UI8_T)select_value;
    }
   
    BACKDOOR_MGR_Printf("\n\r-------");
    for (i=0;i<15;i++)
         BACKDOOR_MGR_Printf("---");

    if ( POEDRV_SendRawPacket(transmit, receive) == TRUE )
    {
         BACKDOOR_MGR_Printf("\n\r Byte: ");
         for (i=0;i<15;i++)
              BACKDOOR_MGR_Printf("%2d ", i+1);

         BACKDOOR_MGR_Printf("\n\r-------");
         for (i=0;i<15;i++)
              BACKDOOR_MGR_Printf("---");

         BACKDOOR_MGR_Printf("\n\r   Tx: ");
         for (i=0;i<15;i++)
              BACKDOOR_MGR_Printf("%02x ", transmit[i]);

         BACKDOOR_MGR_Printf("\n\r   Rx: ");
         for (i=0;i<15;i++)
              BACKDOOR_MGR_Printf("%02x ", receive[i]);
    }
    else
         BACKDOOR_MGR_Printf("\n\r Failed to send this packet to PoE controller");

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

    BACKDOOR_MGR_Printf("\n\rPress <enter> to continue.");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 255);
}

/* FUNCTION NAME: POEDRV_BACKDOOR_EnableCapacitorDetection
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */

static void POEDRV_BACKDOOR_EnableCapacitorDetection(void)
{
    UI8_T     system_mask=0;    
#define system_mask_bit1_enable    0x02

    BACKDOOR_MGR_Printf("\n\r\n\r------------- Enable  Capacitor Detection------------");  	
    
    if ( POEDRV_GetSystemMask(&system_mask) == TRUE )
         system_mask|=system_mask_bit1_enable;
    else 
         BACKDOOR_MGR_Printf("\n\r Failed to get system mask");
    if (backdoor_display_debug)
        BACKDOOR_MGR_Printf("\n\r system mask=0x %2x",system_mask);

    if (POEDRV_SetSystemMask(system_mask)==TRUE)
         BACKDOOR_MGR_Printf("\n\r Enable capacitor detection on Poe Controller successfully");
    else      
         BACKDOOR_MGR_Printf("\n\r Failed to ENABLE capacitor on Poe Controller");
              
}
/* FUNCTION NAME: POEDRV_BACKDOOR_DisableCapacitorDetection
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_DisableCapacitorDetection(void)
{ 
    UI8_T     system_mask=0;    
#define system_mask_bit1_disable    0xfd

    BACKDOOR_MGR_Printf("\n\r\n\r------------- Enable  Capacitor Detection------------");  	
    
    if ( POEDRV_GetSystemMask(&system_mask) == TRUE )
         system_mask&=system_mask_bit1_disable;
    else 
         BACKDOOR_MGR_Printf("\n\r Failed to get system mask");
    if (backdoor_display_debug)
        BACKDOOR_MGR_Printf("\n\r system mask= 0x%2x",system_mask);
    if (POEDRV_SetSystemMask(system_mask)==TRUE)
         BACKDOOR_MGR_Printf("\n\r Disable capacitor detection on Poe Controller successfully");
    else      
         BACKDOOR_MGR_Printf("\n\r Failed to Disable capacitor on Poe Controller");
    
              
}
 
/* FUNCTION NAME: POEDRV_BACKDOOR_HighPriorityPortDenied()
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */

static void POEDRV_BACKDOOR_HighPriorityPortDenied(void)
{
    UI8_T     system_mask=0;    
#define system_mask_bit0_enable    0x01

    BACKDOOR_MGR_Printf("\n\r\n\r------------- Enable  Capacitor Detection------------");  	
    
    if ( POEDRV_GetSystemMask(&system_mask) == TRUE )
         system_mask|=system_mask_bit0_enable;
    else 
         BACKDOOR_MGR_Printf("\n\r Failed to get system mask");
    if (backdoor_display_debug)
        BACKDOOR_MGR_Printf("\n\r system mask=0x %2x",system_mask);

    if (POEDRV_SetSystemMask(system_mask)==TRUE)
         BACKDOOR_MGR_Printf("\n\r Set High Priority Port Denied on Poe Controller successfully");
    else      
         BACKDOOR_MGR_Printf("\n\r Failed to Set High Priority Port Denied on Poe Controller");
              
} /* end of POEDRV_BACKDOOR_HighPriorityPortDenied(void) */

/* FUNCTION NAME: POEDRV_BACKDOOR_LowPriorityPortShutdown
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_LowPriorityPortShutdown(void)
{ 
    UI8_T     system_mask=0;    
#define system_mask_bit0_disable    0xfe

    BACKDOOR_MGR_Printf("\n\r\n\r------------- LowPriority Port Shutdown------------");  	
    
    if ( POEDRV_GetSystemMask(&system_mask) == TRUE )
         system_mask&=system_mask_bit0_disable;
    else 
         BACKDOOR_MGR_Printf("\n\r Failed to get system mask");
    if (backdoor_display_debug)
        BACKDOOR_MGR_Printf("\n\r system mask= 0x%2x",system_mask);
    if (POEDRV_SetSystemMask(system_mask)==TRUE)
         BACKDOOR_MGR_Printf("\n\r set Low Priority Port Shutdown on Poe Controller successfully");
    else      
         BACKDOOR_MGR_Printf("\n\r Failed to set Low Priority Port Shutdown on Poe Controller");
    
              
}/* end of POEDRV_BACKDOOR_LowPriorityPortShutdown(void)*/

static void POEDRV_BACKDOOR_Getb()
{
    	char      line_buffer[MAXLINE];
    	UI32_T    select_value = 0;
   

	BACKDOOR_MGR_Printf("\n\r Enter your Address(in HEX,ex:0xFF0a0001): "); 
	if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
       {
             select_value = (UI32_T)strtoul(line_buffer,0,16);
             BACKDOOR_MGR_Printf("\n\r");
       }
	else
	{
	 	return;
	}

	if (select_value != 0) 
	{
		BACKDOOR_MGR_Printf("0x%02x \n\r", *((UI8_T*)(select_value)));
	}
	else
	{
	 	return;
	}
	
}/* End of POEDRV_BACKDOOR_Getb() */
 
static void POEDRV_BACKDOOR_SoftwareReset(BOOL_T is_enable)
{
    UI8_T data_out, data_in, data_act;
    BOOL_T retval;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &poedrv_gpio_sem_id) != SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK\n",__FUNCTION__);
    }

    /* set GPIO */
    POEDRV_GPIO_ENTER_CRITICAL_SECTION();

    retval = PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_GPIO_IN, 1, 1, (UI8_T *)&data_in);
    data_out = data_in & (~SYS_HWCFG_SYSTEM_RESET_POE_ENABLE);
    if (is_enable)
        data_out |= SYS_HWCFG_SYSTEM_RESET_POE_ENABLE;
    else
        data_out &= (~SYS_HWCFG_SYSTEM_RESET_POE_ENABLE);
    retval = PHYSICAL_ADDR_ACCESS_Write(SYS_HWCFG_GPIO_OUT, 1, 1, &data_out);
    data_act = SYS_HWCFG_SYSTEM_RESET_POE_ENABLE;
    data_act = ~data_act;
    retval = PHYSICAL_ADDR_ACCESS_Write(SYS_HWCFG_GPIO_ACT, 1, 1, &data_act);

    POEDRV_GPIO_LEAVE_CRITICAL_SECTION();

    SYSFUN_Sleep(1);
} /* End of POEDRV_BACKDOOR_SoftwareReset() */


#if 0
/* FUNCTION NAME: POEDRV_BACKDOOR_HardwareReset
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_HardwareReset(void)
{


} /* End of POEDRV_BACKDOOR_HardwareReset() */
#endif



#if 0 /* Eugene mark, do not need it anymore */
/* FUNCTION NAME: POEDRV_BACKDOOR_GetLine
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static int POEDRV_BACKDOOR_GetLine(char s[], int lim)
{
    int c, i;
    
    for (i=0; i<lim-1 && (c=getchar()) != 0 && c!='\n'; ++i)
    {
        s[i] = c;
        BACKDOOR_MGR_Printf("%c", c);
    }
    if (c == '\n')
    {
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return i;

} /* End of POEDRV_BACKDOOR_GetLine */
#endif

/* FUNCTION NAME: POEDRV_BACKDOOR_SetLegacy
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_SetLegacy(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    unit = 0;
    UI32_T    enable = 2;
    int       select_value = 0;
    BOOL_T    ret = FALSE;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- Set Port Priority ------------------");  
    BACKDOOR_MGR_Printf("\n\r Enter Unit ID (1-8) : "); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }
    unit = select_value;

    BACKDOOR_MGR_Printf("\n\r Enter Enable (1-enable, 0-disable) : "); 
   
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        select_value = atoi(line_buffer);
    }

    enable = select_value;

    if ( (enable>= 0) && (enable <= 1) )
    {
         if ( POEDRV_SetCapacitorDetectionControl(unit, enable) == TRUE )
              ret = TRUE;
    }

    if ( ret == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to set Legacy %ld to unit %ld", enable, unit);
    else
         BACKDOOR_MGR_Printf("\n\r Failed to set Legacy %ld to unit %ld", enable, unit);

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_SetPortPriority() */

/* FUNCTION NAME: POEDRV_BACKDOOR_ShowPowerMode
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_ShowPowerMode(void)
{
    BOOL_T  ClassMode;

    BACKDOOR_MGR_Printf("\n\r\n\r---------- Show  Power  Mode -------------");  

    if ( POEDRV_GetClassMode(&ClassMode) == TRUE )
    {
        if ( ClassMode )
            BACKDOOR_MGR_Printf("\n\r\n\r         Class  Mode          ");
        else
            BACKDOOR_MGR_Printf("\n\r\n\r         NonClass  Mode          ");
    }
    else
        BACKDOOR_MGR_Printf("            ??????         ");    

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 

} /* End of POEDRV_BACKDOOR_ShowPowerMode() */

/* FUNCTION NAME: POEDRV_BACKDOOR_EnableClassMode
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_EnableClassMode(void)
{
    BOOL_T ClassMode=TRUE;
    UI32_T    unit = 1;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- Enable Class Mode ---------------------");  

    if ( POEDRV_SetClassMode( unit,ClassMode ) == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to enable Class Mode");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to enable Class Mode !!");

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 
 
} /* End of POEDRV_BACKDOOR_EnableClassMode() */

/* FUNCTION NAME: POEDRV_BACKDOOR_EnableClassMode
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void POEDRV_BACKDOOR_DiableClassMode(void)
{
    BOOL_T ClassMode=FALSE;
    UI32_T    unit = 1;

    BACKDOOR_MGR_Printf("\n\r\n\r---------------- Disable Class Mode ---------------------");  

    if ( POEDRV_SetClassMode( unit,ClassMode ) == TRUE )
         BACKDOOR_MGR_Printf("\n\r Completed to Diable Class Mode");
    else
         BACKDOOR_MGR_Printf("\n\r Failed to disable Class Mode !!");

    BACKDOOR_MGR_Printf("\n\r----------------------------------------------------"); 
    BACKDOOR_MGR_Printf("\n\r"); 
 
} /* End of POEDRV_BACKDOOR_DiableClassMode() */



