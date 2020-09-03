/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_bcm5910x.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    driver API for Poedrv task to access PoE control.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created
 *    07/10/2007 - Daniel Chen, Porting Broadcom series PoE ASIC
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
/* Std lib */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* System default */
#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"

/* Mib */
//#include "leaf_es3626a.h"
#include "leaf_3621.h"

/* System */
#include "sysfun.h"
#include "stktplg_mgr.h"
//#include "backdoor_pom.h"
#include "fs.h"
#include "fs_type.h"
//#include "uartdrv.h"

/* PoE */
#include "poedrv_type.h"
#include "poedrv.h"
#include "poedrv_backdoor.h"
#include "poedrv_control.h"

#include "l_stdlib.h"
//#include "stktplg_board.h"

/* NAME CONSTANT DECLARATIONS
 */
#define POEDRV_NO_OF_BYTES_FOR_CHECKSUM 11 /* offset 0 - offset 10 */
#define POEDRV_SIZE_OF_PACKET sizeof(POEDRV_BCM_TYPE_PktBuf_T)
#define POEDRV_PORT_POWER_LIMIT 31000 /* BCM chip support 31000mw per port in high power mode*/
#define POEDRV_TYPE_DATA_POWERSUPPLY_NUM 8

/* #define POEDRV_DEBUG */

/* MACRO FUNCTION DECLARATIONS
 */

#define INITIALIZE_PACKET_BUFFER(X,Y)                      \
    {                                                          \
        memset((X), 0xFF, POEDRV_SIZE_OF_PACKET); \
        memset((Y), 0, POEDRV_SIZE_OF_PACKET); \
    }

/* Macro function to increment sequence number by 1, range: 0x0 ~ 0xFE.
 */
#define INCREMENT_SEQ_NUMBER()              \
    {                                        \
        if ( poedrv_seq_number == 0xFE )    \
             poedrv_seq_number = 0;         \
        else                                 \
             poedrv_seq_number ++;          \
    }

#define POEDRV_BCM_ENTER_CRITICAL_SECTION        SYSFUN_TakeSem(poedrv_bcm5910x_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_BCM_LEAVE_CRITICAL_SECTION        SYSFUN_GiveSem(poedrv_bcm5910x_sem_id)
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* DATA TYPE DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T poedrv_bcm5910x_sem_id;
static UI8_T  poedrv_seq_number = 0;     /* Seq number(0x0~0xFE) */
static UI32_T uart2_handler = 0;             /* for second uart */

/* Function Prototype
 */
static BOOL_T POEDRV_BCM_SendPacket(POEDRV_BCM_TYPE_PktBuf_T  *tx_buf);
static BOOL_T POEDRV_BCM_ReceivePacket(POEDRV_BCM_TYPE_PktBuf_T *rx_buf);
static UI8_T POEDRV_BCM_Checksum(UI8_T *buffer);
static BOOL_T POEDRV_BCM_SendAndWaitReply(POEDRV_BCM_TYPE_PktBuf_T *tx_buf, POEDRV_BCM_TYPE_PktBuf_T *rx_buf);
static BOOL_T POEDRV_BCM_GetPortAllConfig(UI32_T port, void *cfg);

static BOOL_T POEDRV_BCM_OpenUart(void)
{
    if ((uart2_handler=SYSFUN_OpenUART(SYSFUN_UART_CHANNEL2))==-1)
		return FALSE;
	
    SYSFUN_SetUartCfg(uart2_handler, SYSFUN_UART_BAUDRATE_19200,
		                                SYSFUN_UART_PARITY_NONE,
		                                SYSFUN_UART_DATA_LENGTH_8_BITS,
		                                SYSFUN_UART_STOP_BITS_1_BITS);
	return TRUE;
}


UI32_T sysSerialPolltxBuff(UI8_T *tx_buf,UI32_T size)
{
    UI32_T ret=0;

    ret =  SYSFUN_UARTPutData(uart2_handler, size, tx_buf);

    return ret;
}

UI32_T sysPollRxBuff2(UI32_T size, UI8_T *rx_buf)
{
    UI32_T len = 0;

    len =  SYSFUN_UARTPollRxBuff(uart2_handler, size, rx_buf);

    return len;
}


/* FUNCTION NAME : POEDRV_BCM_Inits
 * PURPOSE: This function is used to intial the resource which the driver needed.
 *
 * INPUT:   sem_id
 * OUTPUT:
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_Inits(void)
{
    /* Create semaphore
     */
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_bcm5910x_sem_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\n\rCreate poedrv_bcm5910x_sem_id failed.");
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : POEDRV_BCM_GetPortAdminStatus
 * PURPOSE: This function is used to the admin status of a port PSE.
 *
 * INPUT:   port -- port ID.
 * OUTPUT:
 *          admin_status -- VAL_pethPsePortAdminEnable_true
 *                          VAL_pethPsePortAdminEnable_false
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortAdminStatus(UI32_T port, UI32_T *admin_status)
{
    POEDRV_PORT_CONFIG_T cfg;
    BOOL_T ret;

DBG_PRINT();

    *admin_status = 0;

    ret = POEDRV_BCM_GetPortAllConfig(port,(void *) &cfg);
    if (TRUE == ret)
    {
        *admin_status = cfg.admin_status;
    }

    return ret;
} /* end POEDRV_BCM_GetPortAdminStatus */


/* FUNCTION NAME : POEDRV_BCM_SetPortAdminStatus
 * PURPOSE: This function is used to enable or disable a port PSE.
 *
 * INPUT:   port -- port ID.
 *          admin_status -- VAL_pethPsePortAdminEnable_true
 *                          VAL_pethPsePortAdminEnable_false
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortAdminStatus(UI32_T port, UI32_T admin_status)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;


DBG_PRINT();
    if ((admin_status != VAL_pethPsePortAdminEnable_false) &&
        (admin_status != VAL_pethPsePortAdminEnable_true))
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.command = POEDRV_TYPE_CMD_PORT_PSE_SWITCH;
    transmit.data1   = port;
    transmit.data2   = admin_status==VAL_pethPsePortAdminEnable_true?admin_status:0;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* End of POEDRV_BCM_SetPortAdminStatus() */


/* FUNCTION NAME : POEDRV_BCM_SetPortAllowPowerState
 * PURPOSE: This function is used to allow power or disallow power up on the
 *          specified power requesting port .
 * INPUT:   port -- port ID.
 *          state -- 0 - not allow power to PD
 *                   1 - allow power to PD
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortAllowPowerState(UI32_T port, UI32_T state)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if ((state != 0) && (state != 1))
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.command = POEDRV_TYPE_CMD_PORT_POWER_UP_ENABLE;
    transmit.data1   = port;
    transmit.data2   = state;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}/* end POEDRV_BCM_SetPortAllowPowerState() */

/* FUNCTION NAME : POEDRV_BCM_EnablePortMapping
 * PURPOSE: This function is used to enable logical port mapping and set port mapping relation.
 *
 * INPUT:   port_map -- mapping relation between physical and logical port
 *                      Array index means logical port index. (counted from zero)
 *                      Data in the array means the physical port.(counted from zero)
 *
 *          total_port_num -- Array length
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES: Here we count logical port from zero. i.e. number of front panel port decrease 1
 */
static BOOL_T POEDRV_BCM_EnablePortMapping(UI32_T port_map[], UI32_T total_port_num)
{
    int i, j;
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

    /* Enable the logical port map function */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_MAPPING_ENABLE;
    transmit.data1 = 1;

    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
DBG_PRINT("ret=%d",ret);
    if (ret)
    {
        if (receive.data1 != POEDRV_TYPE_DATA_ACK)
        {
DBG_PRINT("FAIL");
            return FALSE;
        }
    }

    /* set the port map */
    for (i=0, j=0 ; j<total_port_num/4 ; i+=4, j++)
    {
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        transmit.command = POEDRV_TYPE_CMD_PORT_MAPPING_SET;
        transmit.data1 = i; /* logical port */
        transmit.data2 = port_map[i]; /* physical port */
        transmit.data3 = i+1;
        transmit.data4 = port_map[i+1];
        transmit.data5 = i+2;
        transmit.data6 = port_map[i+2];
        transmit.data7 = i+3;
        transmit.data8 = port_map[i+3];

        ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
        if (ret)
        {
            if (receive.data2 != POEDRV_TYPE_DATA_ACK)
            {
                return FALSE;
            }
        }
    }

    for (;i<total_port_num;i++)
    {
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        transmit.command = POEDRV_TYPE_CMD_PORT_MAPPING_SET;
        transmit.data1 = i; /* logical port */
        transmit.data2 = port_map[i]; /* physical port */

        ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
        if (ret)
        {
            if (receive.data2 != POEDRV_TYPE_DATA_ACK)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}
/* end POEDRV_BCM_EnablePortMapping() */

/* FUNCTION NAME : POEDRV_BCM_DisablePortMapping
 * PURPOSE: This function is used to disable logical port mapping function.
 *
 * INPUT: None
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_DisablePortMapping(void)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    /* Enable the logical port map function */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_MAPPING_ENABLE;
    transmit.data1 = 0;

    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}/* end POEDRV_BCM_DisablePortMapping() */

/* FUNCTION NAME : POEDRV_BCM_ResetPort
 * PURPOSE: This function is used to reset the specified port
 *
 * INPUT:   port -- port ID.
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_ResetPort(UI32_T port)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.command = POEDRV_TYPE_CMD_PORT_RESET;
    transmit.data1 = port;
    transmit.data2 = 1;

    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}
/* end POEDRV_BCM_ResetPort() */


/* FUNCTION NAME : POEDRV_BCM_GetPortDetectionType
 * PURPOSE: This function is used to configure the detection type on the specified port
 *
 * INPUT:   port -- port ID.
 *
 * OUTPUT:  type -- POEDRV_PORT_DETECTION_NONE,
 *                  POEDRV_PORT_DETECTION_LEGACY,
 *                  POEDRV_PORT_DETECTION_DOT3AF_4POINT,
 *                  POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY,
 *                  POEDRV_PORT_DETECTION_DOT3AF_2POINT,
 *                  POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY,
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortDetectionType(UI32_T port, UI32_T *type)
{
    POEDRV_PORT_CONFIG_T cfg;
    BOOL_T ret;

DBG_PRINT();
    *type = 0;

    ret = POEDRV_BCM_GetPortAllConfig(port,(void *) &cfg);
    if (TRUE == ret)
    {
        *type = cfg.detection_type;
    }

    return ret;
} /* end POEDRV_BCM_GetPortDetectionType() */

/* FUNCTION NAME : POEDRV_BCM_SetPortDetectionType
 * PURPOSE: This function is used to configure the detection type on the specified port
 *
 * INPUT:   port -- port ID.
 *          type -- POEDRV_PORT_DETECTION_NONE,
 *                  POEDRV_PORT_DETECTION_LEGACY,
 *                  POEDRV_PORT_DETECTION_DOT3AF_4POINT,
 *                  POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY,
 *                  POEDRV_PORT_DETECTION_DOT3AF_2POINT,
 *                  POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY,
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortDetectionType(UI32_T port, UI32_T type)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (type < POEDRV_PORT_DETECTION_NONE || type > POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_DETECTION_SET;
    transmit.data1   = port;
    transmit.data2   = type;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}/* end POEDRV_BCM_SetPortDetectionType */


/* FUNCTION NAME : POEDRV_BCM_SetPortClassType
 * PURPOSE: This function is used to set the classification type on the specified port
 *
 * INPUT:   port -- port ID.
 *          type -- POEDRV_PORT_CLASSIFICATION_NONE,
 *                  POEDRV_PORT_CLASSIFICATION_DOT3AF
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortClassType(UI32_T port, UI32_T type)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (type < POEDRV_PORT_CLASSIFICATION_NONE || type > POEDRV_PORT_CLASSIFICATION_DOT3AF)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_CLASSIFICATION_TYPE_SET;
    transmit.data1   = port;
    transmit.data2   = type;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}/* end POEDRV_BCM_SetPortClassType() */

/* FUNCTION NAME : POEDRV_BCM_SetPortAutoMode
 * PURPOSE: This function is used to set the classification type on the specified port
 *
 * INPUT:   port -- port ID.
 *          mode -- 1 Enable,
 *                  0 Disable
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortAutoMode(UI32_T port, UI32_T mode)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (mode != 0 && mode != 1)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_AUTO_MODE_ENABLE;
    transmit.data1   = port;
    transmit.data2   = mode;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;

} /* end POEDRV_BCM_SetPortAutoMode() */


/* FUNCTION NAME : POEDRV_BCM_SetPortPowerThresholdType
 * PURPOSE: This function is used to set the power threshold type on the specified port
 *
 * INPUT:   port -- port ID.
 *          type -- POEDRV_PORT_POWER_THRESHOLD_NONE,
 *                  POEDRV_PORT_POWER_THRESHOLD_CLASS_BASE,
 *                  POEDRV_PORT_POWER_THRESHOLD_USER_DEFINE
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortPowerThresholdType(UI32_T port, UI32_T type)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (type < POEDRV_PORT_POWER_THRESHOLD_NONE || type > POEDRV_PORT_POWER_THRESHOLD_USER_DEFINE)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_THRESHOLD_TYPE_SET;
    transmit.data1   = port;
    transmit.data2   = type;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;

}   /* end POEDRV_BCM_SetPortPowerThresholdType() */


/* FUNCTION NAME : POEDRV_BCM_GetPortPowerMaximumAllocation
 * PURPOSE: This function is used to get the maximun power threshold on the specified port
 *          (The real used unit in chip is 0.2 W/LSB)
 * INPUT:   port -- port ID.
 *
 *
 * OUTPUT:  milliwatts -- power limit
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortPowerMaximumAllocation(UI32_T port, UI32_T *milliwatts)
{
    POEDRV_PORT_CONFIG_T cfg;
    BOOL_T ret;

DBG_PRINT();
    *milliwatts = 0;

    ret = POEDRV_BCM_GetPortAllConfig(port,(void *) &cfg);
    if (TRUE == ret)
    {
        *milliwatts = cfg.power_limit;
    }

    return ret;
} /* end POEDRV_BCM_GetPortPowerMaximumAllocation */

/* FUNCTION NAME : POEDRV_BCM_SetPortPowerMaximumAllocation
 * PURPOSE: This function is used to set the maximun power threshold on the specified port
 *          (The real used unit in chip is 0.2 W/LSB)
 * INPUT:   port -- port ID.
 *          milliwatts -- power limit
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortPowerMaximumAllocation(UI32_T port, UI32_T milliwatts)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    UI32_T real_value;
    BOOL_T ret;

DBG_PRINT();
    if (milliwatts < SYS_HWCFG_MIN_POWER_INLINE_ALLOCATION || milliwatts > SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION)
    {
        return FALSE;
    }

    real_value = milliwatts/200; /* transfer unit from 1mW/LSB to 0.2W/LSB */

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_POWER_LIMIT_SET;
    transmit.data1   = port;
    transmit.data2   = real_value;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* end POEDRV_BCM_SetPortPowerMaximumAllocation() */

/* FUNCTION NAME : POEDRV_BCM_SetPowerManagementMode
 * PURPOSE: This function is used to set the maximun power management mode
 *
 * INPUT:   power_mode -- POEDRV_POWER_MANAGE_NONE
 *                        POEDRV_POWER_MANAGE_STATIC
 *                        POEDRV_POWER_MANAGE_DYNAMIC
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPowerManagementMode(UI32_T power_mode)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (power_mode < POEDRV_POWER_MANAGE_NONE || power_mode > POEDRV_POWER_MANAGE_DYNAMIC)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_POWER_MANAGE_MODE_SET;
    transmit.data1   = power_mode;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data1 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* end POEDRV_BCM_SetPowerManagementMode() */


/* FUNCTION NAME : POEDRV_BCM_SetPowerSourceControl
 * PURPOSE: This function is used to set the power source detail for PoE subsystem
 *
 * INPUT:   power_limit (Watt)
 *          guard_band  (Watt)
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPowerSourceControl(UI32_T power_limit, UI32_T guard_band)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    UI8_T powersupply_num=0;
    BOOL_T ret;

DBG_PRINT();
    if (guard_band > power_limit)
    {
        return FALSE;
    }

    /* real used unit is 0.1W/LSB in ASIC */
    power_limit *= 10;
    guard_band *= 10;

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    for (powersupply_num=0; powersupply_num<POEDRV_TYPE_DATA_POWERSUPPLY_NUM; powersupply_num++)
    {
        transmit.command = POEDRV_TYPE_CMD_SYSTEM_POWER_SOURCE_SET;
        transmit.data1 = powersupply_num;
        transmit.data2 = (power_limit>>8) & 0xff;
        transmit.data3 = power_limit & 0xff;
        transmit.data4 = (guard_band>>8) & 0xff;
        transmit.data5 = guard_band & 0xff;
    
        ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
        if ((!ret) || (receive.data2 != POEDRV_TYPE_DATA_ACK))
        {
DBG_PRINT("No.%u, PL=%lu, GB=%lu, ret=%u, ack=%u!",powersupply_num,power_limit,guard_band,ret,receive.data2);
            return FALSE;
        }
    }
    return TRUE;
} /* end POEDRV_BCM_SetPowerSourceControl() */

/* FUNCTION NAME : POEDRV_BCM_GetPortPowerPairs
 * PURPOSE: This function is used to get power pair used to deliver power
 *          on the specified port
 * INPUT:   port -- port id
 *
 * OUTPUT:  power_pairs -- VAL_pethPsePortPowerPairs_signal (Alternative A)
 *                         VAL_pethPsePortPowerPairs_spare (Alternative B)
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortPowerPairs(UI32_T port, UI32_T *power_pairs)
{
    POEDRV_PORT_CONFIG_T cfg;
    BOOL_T ret;

DBG_PRINT();
    *power_pairs = 0;

    ret = POEDRV_BCM_GetPortAllConfig(port,(void *) &cfg);
    if (TRUE == ret)
    {
        *power_pairs = cfg.power_pair;
    }

    return ret;

} /* POEDRV_BCM_GetPortPowerPairs */

/* FUNCTION NAME : POEDRV_BCM_SetPortPowerPairs
 * PURPOSE: This function is used to conigurare the pair used to deliver power
 *          on the specified port
 * INPUT:   port -- port id
 *          power_pairs -- VAL_pethPsePortPowerPairs_signal (Alternative A)
 *                         VAL_pethPsePortPowerPairs_spare (Alternative B)
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortPowerPairs(UI32_T port, UI32_T power_pairs)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (power_pairs != VAL_pethPsePortPowerPairs_signal && power_pairs != VAL_pethPsePortPowerPairs_spare)
    {
        return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_POWER_PAIR_SET;
    transmit.data1   = port;
    transmit.data2   = power_pairs-1; /* 0/1 is used in the PoE system */
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}
/* end POEDRV_BCM_SetPortPowerPairs() */

/* FUNCTION NAME : POEDRV_BCM_GetPortPriority
 * PURPOSE: This function is used to get port priority on the specified port
 *
 * INPUT:   port -- port id
 *
 * OUTPUT:  priority -- VAL_pethPsePortPowerPriority_low
 *                      VAL_pethPsePortPowerPriority_high
 *                      VAL_pethPsePortPowerPriority_critical
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortPriority(UI32_T port, UI32_T *priority)
{
    POEDRV_PORT_CONFIG_T cfg;
    BOOL_T ret;

DBG_PRINT();
    *priority = 0;

    ret = POEDRV_BCM_GetPortAllConfig(port,(void *) &cfg);
    if (TRUE == ret)
    {
        *priority = cfg.power_priority;
    }

    return ret;
} /* end POEDRV_BCM_GetPortPriority */

/* FUNCTION NAME : POEDRV_BCM_SetPortPriority
 * PURPOSE: This function is used to conigurare port priority on the specified port
 *
 * INPUT:   port -- port id
 *          priority -- VAL_pethPsePortPowerPriority_low
 *                      VAL_pethPsePortPowerPriority_high
 *                      VAL_pethPsePortPowerPriority_critical
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortPriority(UI32_T port, UI32_T priority)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    /* Four prioriy types in the ASIC,
     * We only use three types: critical, high, low
     */
    switch (priority)
    {
        case VAL_pethPsePortPowerPriority_critical:
            priority = POEDRV_PORT_PRIORITY_CRITICAL;
            break;
        case VAL_pethPsePortPowerPriority_high:
            priority = POEDRV_PORT_PRIORITY_HIGH;
            break;
        case VAL_pethPsePortPowerPriority_low:
            priority = POEDRV_PORT_PRIORITY_LOW;
            break;
        default:
            return FALSE;
    }

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_PRIORITY_SET;
    transmit.data1   = port;
    transmit.data2   = priority;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* end POEDRV_BCM_SetPortPriority() */

/* FUNCTION NAME : POEDRV_BCM_SetPortForceHighPowerMode
 * PURPOSE: This function is used to conigurare port up mode on the specified port
 *
 * INPUT:   port -- port id
 *          mode -- 1 - high power mode
 *                  0 - normal mode (for .3af)
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortForceHighPowerMode(UI32_T port, UI32_T mode)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (mode !=0 && mode != 1)
    {
        return FALSE;
    }


    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_PORT_POWER_MODE_SET;
    transmit.data1   = port;
    transmit.data2   = mode;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* end POEDRV_BCM_SetPortForceHighPowerMode() */

/* FUNCTION NAME : POEDRV_BCM_SetPortDot3atHighPowerMode
 * PURPOSE: This function is used to conigurare port up mode on the specified port
 *
 * INPUT:   port -- port id
 *          mode -- 1 - high power mode (Switch the Port from 802.3af to High Power Mode)
 *                  0 - normal mode (Ignore the Port switch request)
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetPortDot3atHighPowerMode(UI32_T port, UI32_T mode)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (mode !=0 && mode != 1)
    {
        return FALSE;
    }


    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_HIGH_POWER_FOR_DLL_SET;
    transmit.data1   = port;
    transmit.data2   = mode;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
} /* end POEDRV_BCM_SetPortDot3atHighPowerMode() */


/* FUNCTION NAME : POEDRV_BCM_SetCurrentInHighPower
 * PURPOSE: This function is used to conigurare Sysytem Icut/Ilim in high power mode
 *
 * INPUT:   mode -- 0 - Ilim range : 504mA ¡V 584mA ; Icut 450mA(Min)/465mA(Typ)/480mA(Max)
 *                  1 - Ilim range : 563mA ¡V 650mA ; Icut 530mA(Min)/545mA(Typ)/560mA(Max)
 *                  2 - Ilim range : 850mA ¡V 1.1A ; Icut 660mA(Min)/675mA(Typ)/690mA(Max)
 *                  3 - Ilim range : 850mA ¡V 1.1A ; Icut 730mA(Min)/745mA(Typ)/760mA(Max)
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SetCurrentInHighPower(UI32_T mode)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    if (mode < 0 || mode > 3)
    {
        return FALSE;
    }


    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_ICUT_IN_HIGH_POWER_MODE_SET;
    transmit.data1   = mode;
    ret=POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    if (ret)
    {
        if (receive.data1 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }
    return FALSE;
}/* end POEDRV_BCM_SetCurrentInHighPower() */

/* FUNCTION NAME : POEDRV_BCM_GetPoeSoftwareVersion
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : None
 * OUTPUT  : version -- version number
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_BCM_GetPoeSoftwareVersion(UI8_T *version)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_SYSTEM_STATUS_GET;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        *version = receive.data6;
    }
    return ret;
} /* End of POEDRV_BCM_GetPoeSoftwareVersion() */

/* FUNCTION NAME: POEDRV_BCM_GetPortStatus2
 * PURPOSE: This function is used to get port status. (overload or classification..)
 * INPUT:   port -- port ID
 * OUTPUT:  status1 -- 0=Disable,1=Searching,2=Delivering power,3=Test mode,4=Fault,5=Other fault,
 *	                   6=Requesting power
 *          status2 -- If status1=2 or 6, status2 is class of this port.
 *                     If status1=4 or 5, status2 is error type. 0=None,1=MPS absent,2=Short,
 *                     3=Overload,4=Power denied
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_BCM_GetPortStatus2(UI32_T port, UI8_T *status1, UI8_T *status2)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_STATUS_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        *status1 =  receive.data2;
        *status2 =  receive.data3;
    }
    return ret;
} /* End of POEDRV_BCM_GetPortStatus2() */


/* FUNCTION NAME: POEDRV_BCM_GetTotalAllocPower
 * PURPOSE: This function is used to get total allocated power in the PoE subsystem
 * INPUT:
 * OUTPUT:  total_power -- used power consumption. (unit: 0.1 W)
 *	                   6=Requesting power
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_BCM_GetTotalAllocPower(UI32_T *total_power)
{ 
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_TOTAL_POWER_ALLOCATED_GET;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        *total_power =  (receive.data1<<8)|receive.data2; /* The unit in ASIC is 0.1W/LSB */
    }
    return ret;
} /* end POEDRV_BCM_GetTotalAllocPower */

/* FUNCTION NAME: POEDRV_BCM_GetPortPowerConsumption
 * PURPOSE: This function is used to get power consumption of a port.
 * INPUT:   port -- port ID
 *
 * OUTPUT:  milliwatt -- power consumption of a port in milliwatt
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_BCM_GetPortPowerConsumption(UI32_T port, UI32_T *milliwatt)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_MEASURE_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        *milliwatt =  (receive.data8<<8)|receive.data9; /* The unit in ASIC is 0.1W/LSB */
        *milliwatt *= 100;
    }
    return ret;
}
/* end POEDRV_BCM_GetPortPowerConsumption() */

/* FUNCTION NAME: POEDRV_BCM_GetPortMeasurement
 * PURPOSE: This function is used to get power consumption and temperature of a port.
 * INPUT:   port -- port ID
 *
 * OUTPUT:  used_pwr -- power consumption of a port in milliwatt
 *          tmp -- temperature of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_BCM_GetPortMeasurement(UI32_T port, UI32_T *used_pwr, I32_T *tmp, UI32_T *volt, UI32_T *cur)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_MEASURE_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        *used_pwr =  (receive.data8<<8)|receive.data9; /* The unit in ASIC is 0.1W/LSB */
        *used_pwr *= 100;

        *tmp = (receive.data6<<8)|receive.data7;

        /* temperature = (vaule-127)*(-1.25)+125 */
        *tmp = (*tmp - 127)* (-125)/100;
        *tmp +=125;

        /* voltage, unit: 64.45 mV/LSB */
        *volt = (receive.data2<<8)|receive.data3;
        *volt *= 6445; 
        *volt /= (100*1000);

        /* current, unit: 1 mA/LSB */
        *cur = (receive.data4<<8)|receive.data5;
    }
    return ret;
}
/* end POEDRV_BCM_GetPortMeasurement() */


/* FUNCTION NAME : POEDRV_BCM_ResetPortStatistic
 * PURPOSE : This function is used to reset the port statistics of the specific port
 *
 * INPUT   : port : port ids
 *
 * OUTPUT  : 
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_ResetPortStatistic(UI32_T port)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_STATISTIC_RESET;
    transmit.data1    = port;
    transmit.data2    = 1;  /* reset */

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if ( ret )
    {
        if (receive.data2 == POEDRV_TYPE_DATA_ACK)
        {
            return TRUE;
        }
    }

    return FALSE;

}

/* FUNCTION NAME : POEDRV_BCM_ResetAllPortStatistic
 * PURPOSE : This function is used to reset all port statistics of the specific port
 *
 * INPUT   : port_num : port number of PoE port
 *
 * OUTPUT  : 
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_ResetAllPortStatistic(UI32_T port_num)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;
    UI32_T port;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_STATISTIC_RESET;
    transmit.data2    = 1;  /* reset */
    transmit.data4    = 1;  /* reset */
    transmit.data6    = 1;  /* reset */
    transmit.data8    = 1;  /* reset */

    for (port = 0 ; port < port_num; port+=4)
    {
        transmit.data1 =  port;
        transmit.data3 =  port+1;
        transmit.data5 =  port+2;
        transmit.data7 =  port+3;
        
        ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

        if ( ret )
        {
            if (receive.data2 != POEDRV_TYPE_DATA_ACK)
            {
                return FALSE;
            }
        }
    }

    return TRUE;

}


/* FUNCTION NAME : POEDRV_BCM_GetPortAllCounter
 * PURPOSE : This function is used to get the port statistics of the specific port
 *
 * INPUT   : port : port ids
 *
 * OUTPUT  : counters -- buffer to store all data counters
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortAllCounter(UI32_T port, UI8_T counters[POEDRV_MAX_COUNTER_TYPE])
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;

DBG_PRINT();
    memset(counters, 0 , sizeof(UI8_T)*POEDRV_MAX_COUNTER_TYPE);

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_STATISTIC_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if (ret)
    {
        counters[POEDRV_MPSABSENT_COUNTER] =  receive.data2;
        counters[POEDRV_INVALID_SIGNATURE_COUNTER] = 0 ; /* don't support */
        counters[POEDRV_POWER_DENIED_COUNTER] =  receive.data5;
        counters[POEDRV_OVERLOAD_COUNTER] =  receive.data3;
        counters[POEDRV_SHORT_COUNTER] =  receive.data4;
    }
    return ret;
}

/* FUNCTION NAME : POEDRV_BCM_GetPortAllConfig
 * PURPOSE : This function is used to get the all configuration of a speified port
 *
 * INPUT   : port : port id
 *
 * OUTPUT  : cfg -- buffer to store port configuration info
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_GetPortAllConfig(UI32_T port, void *cfg)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    BOOL_T ret;
    POEDRV_PORT_CONFIG_T *config_ptr = cfg;

DBG_PRINT();
    memset(config_ptr, 0 , sizeof(POEDRV_PORT_CONFIG_T));

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_PORT_CONFIG_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if (TRUE == ret)
    {
        if (receive.data2 == 1)
            config_ptr->admin_status = VAL_pethPsePortAdminEnable_true;
        else
            config_ptr->admin_status = VAL_pethPsePortAdminEnable_false;

        config_ptr->detection_type= receive.data4;

        if (receive.data7 == 0)
            config_ptr->power_pair = VAL_pethPsePortPowerPairs_signal;
        else
            config_ptr->power_pair = VAL_pethPsePortPowerPairs_spare;
    }
    else
        return FALSE;

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);
    transmit.command  = POEDRV_TYPE_CMD_PORT_EXT_CONFIG_GET;
    transmit.data1    = port;

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if (TRUE == ret)
    {
        config_ptr->power_mode = receive.data2;

        switch (receive.data5)
        {
            case POEDRV_PORT_PRIORITY_LOW:
            case POEDRV_PORT_PRIORITY_MEDIUM:  /* this value shouldn't be happen */
                config_ptr->power_priority = VAL_pethPsePortPowerPriority_low;
                break;
            case POEDRV_PORT_PRIORITY_HIGH:
                config_ptr->power_priority = VAL_pethPsePortPowerPriority_high;
                break;
            case POEDRV_PORT_PRIORITY_CRITICAL:
                config_ptr->power_priority = VAL_pethPsePortPowerPriority_critical;
                break;
            default:
                return FALSE;
        }

        /* change unit: 0.2 W/LSB-> 1 mW/LSB */
        config_ptr->power_limit = receive.data4 * 200;

    }
    else
        return FALSE;

    return TRUE;
} /* end POEDRV_BCM_GetPortAllConfig */

/* FUNCTION NAME : POEDRV_BCM_ShowPortInfo
 * PURPOSE : This function is used to dump port status and configuration.
 *           (for backdoor to debug, if needed.)
 *
 * INPUT   : port : port number
 *
 * OUTPUT  :
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_ShowPortInfo(UI32_T port)
{
    UI8_T status1, status2;
    POEDRV_PORT_CONFIG_T cfg;
    UI32_T value, volt, cur;
	I32_T  value1;

DBG_PRINT();
    /* printf("======= [Port %d] Information =========\n", port); */
    if (FALSE == POEDRV_BCM_GetPortStatus2(port, &status1, &status2))
    {
        printf("Get Port Status2 Error\n");
        return FALSE;
    }

    /* ==========Get Port Status======== */
    printf("Status:");
    switch (status1)
    {
        case POEDRV_PORT_STATUS1_DISABLE:
            printf(" Disabled\n");
            break;
        case POEDRV_PORT_STATUS1_SEARCHING:
            printf(" Searching\n");
            break;
        case POEDRV_PORT_STATUS1_REQUEST_POWER:
            printf(" Requesting power\n");
            break;
        case POEDRV_PORT_STATUS1_DELIVER_POWER:
            printf(" Delivering power\n");
            printf(" Class value: %d\n", status2);
            break;
        case POEDRV_PORT_STATUS1_TEST:
            printf(" Test mode\n");
            break;
        case POEDRV_PORT_STATUS1_FAULT:
        case POEDRV_PORT_STATUS1_OTHER_FAULT:
            printf(" Fault or Other Fault\n");
            printf("Reason:  ");
            switch (status2)
            {
                case POEDRV_PORT_STATUS2_ERROR_NONE:
                    printf("  None\n");
                    break;
                case POEDRV_PORT_STATUS2_ERROR_MPS_ABSENT:
                    printf("  MPS absent\n");
                    break;
                case POEDRV_PORT_STATUS2_ERROR_SHORT:
                    printf("  short\n");
                    break;
                case POEDRV_PORT_STATUS2_ERROR_OVERLOAD:
                    printf("  overload\n");
                    break;
                case POEDRV_PORT_STATUS2_ERROR_POWER_DENIED:
                    printf("  power denied\n");
                    break;
            }
            break;
        default:
            printf(" Error ! !\n");
            return FALSE;
    }

    printf("\n");
    /* ==========Get Port power consumption======== */
    printf("Current power consumption : ");
    if (POEDRV_BCM_GetPortMeasurement(port, &value, &value1, &volt, &cur) == FALSE)
    {
        printf("ERROR ! ! !\n");
        return FALSE;
    }
    printf("%lu milliWatts\n", value);
    printf("Current temperature: %ld Centegrade, volt: %lu V, current: %lu mA", value1, volt, cur);

    printf("\n");
    /* ==========Get Port Configuration======== */
    printf("Configuration: ");

    if (POEDRV_BCM_GetPortAllConfig(port, &cfg) == FALSE)
    {
        printf("Get configuration information error\n");
        return FALSE;
    }
    printf("admin status: %s\n", cfg.admin_status==VAL_pethPsePortAdminEnable_true?"Enable":"Disable");
    printf("power mode: %s\n", cfg.power_mode==1?"High Power":"Normal");
    printf("power pairs: %s\n", cfg.power_pair==VAL_pethPsePortPowerPairs_signal?"Signal":"Spare");
    if (cfg.power_priority==VAL_pethPsePortPowerPriority_critical)
        printf("power priority: critical \n");
    else if (cfg.power_priority==VAL_pethPsePortPowerPriority_high)
        printf("power priority: high \n");
    else
        printf("power priority: low \n");
    printf("power limit: %lu mW/LSB\n", cfg.power_limit);
    printf("detection type: ");
    switch (cfg.detection_type)
    {
        case POEDRV_PORT_DETECTION_NONE:
            printf("none\n");
            break;
        case POEDRV_PORT_DETECTION_LEGACY:
            printf("legacy\n");
            break;
        case POEDRV_PORT_DETECTION_DOT3AF_4POINT:
            printf("ieee802.3af 4point\n");
            break;
        case POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY:
            printf("ieee802.3af 4point followed by legacy\n");
            break;
        case POEDRV_PORT_DETECTION_DOT3AF_2POINT:
            printf("ieee802.3af 2point\n");
            break;
        case POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY:
            printf("ieee802.3af 2point followed by legacy\n");
            break;
    }
    return TRUE;
}


/* FUNCTION NAME : POEDRV_BCM_ShowMainpowerInfo
 * PURPOSE : This function is used to dump mainpower information
 *           (for backdoor to debug, if needed.)
 *
 * INPUT   : port : port number
 *
 * OUTPUT  :
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_ShowMainpowerInfo(void)
{
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    UI32_T ret;
    UI32_T value;

DBG_PRINT();
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command  = POEDRV_TYPE_CMD_POWER_MANAGE_MODE_GET;
    transmit.seq_no = 0; /* Power Supply 0, for PoE Image V2.1 */

    ret = POEDRV_BCM_SendAndWaitReply(&transmit, &receive);

    if (ret == TRUE)
    {
        value = (UI32_T)receive.data1;
        printf("Power Management Mode: ");
        if (value == POEDRV_POWER_MANAGE_DYNAMIC)
            printf("Dynamic \n");
        else if (value == POEDRV_POWER_MANAGE_STATIC)
            printf("Static \n");
        else
            printf("None \n");
        value = (receive.data2<<8)|receive.data3;/* unit: 0.1 W/LSB */
        printf("Total Power: %ld Watts\n", value/10);
        value = (receive.data4<<8)|receive.data5;/* unit: 0.1 W/LSB */
        printf("Guard Band: %ld Watts\n", value/10);
    }
    else
        printf("Error !\n");
    return ret;
}


/* FUNCTION NAME : POEDRV_BCM_SendRawPacket
 * PURPOSE: This function is used to send a raw packet from engineering backdoor
 * INPUT:   transmit: data pointer of packet to be transmitted (length depend on ASIC)
 * OUTPUT:  receive : data pointer of receiving packet
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_BCM_SendRawPacket(UI8_T *transmit, UI8_T *receive)
{
    BOOL_T ret;

DBG_PRINT();
    ret = POEDRV_BCM_SendAndWaitReply((POEDRV_BCM_TYPE_PktBuf_T *) transmit,
        (POEDRV_BCM_TYPE_PktBuf_T *) receive);

    return ret;
}
#define POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME 8
static BOOL_T POEDRV_BCM_LocalUpgradeImage(const UI8_T *image, UI32_T image_size)
{
#if 1
    UI8_T tx_image[POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME+4] = {0xFF};
    UI8_T tx[POEDRV_SIZE_OF_PACKET] = {0xFF};
    UI8_T rx[POEDRV_SIZE_OF_PACKET]={0};
    UI32_T ret;
    UI32_T i;

#if 0
    /* waiting for BootRom Mode */
    UI32_T wait=10;

    tx[0]=0x20;
    tx[1]=0x01;
    tx[11]=0x18;
    for(j=0;j<=wait;j++)
    {
        POEDRV_BCM_SendRawPacket(tx,rx);

        if (rx[0]==0xAF)
	        break;
        else if(j==wait)
            DBG_PRINT("waiting BootRom mode for %lusec but still failed",wait);

        SYSFUN_Sleep(100); /* 1 sec */
    }
    DBG_PRINT("waiting BootRom mode for %lusec!!!",j);
#endif

    tx_image[0] = POEDRV_TYPE_CMD_IMAGE_UPGRADE;
    tx_image[1] = POEDRV_TYPE_SUBCOM_DOWNLOAD_IMAGE;

    for (i=0;i<image_size;i+=POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME)
    {
        memset(rx, 0, sizeof(POEDRV_BCM_TYPE_PktBuf_T));

        tx_image[2] = (i>>8)&0xFF;    /* offset */
        tx_image[3] = i&0xFF;         /* offset */
        memcpy(tx_image+4, &image[i], POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME);

        ret = sysSerialPolltxBuff(tx_image,POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME+4);

        SYSFUN_Sleep(2); /* need 20ms delay between 2 frames */
        INITIALIZE_PACKET_BUFFER(tx, rx);
        ret = sysPollRxBuff2(POEDRV_SIZE_OF_PACKET, rx);
    }    

    /* CRC check */
    tx[0] = POEDRV_TYPE_CMD_IMAGE_UPGRADE;
    tx[1] = POEDRV_TYPE_SUBCOM_CHECK_CRC;

    ret = sysSerialPolltxBuff( tx, 2 );
    SYSFUN_Sleep(5);
    ret = sysPollRxBuff2(POEDRV_SIZE_OF_PACKET, rx);
#else
    UI8_T tx_buf[POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME];
    UI8_T rx[12];
    UI16_T read_bytes;
    UI32_T i;
    BOOL_T ret;

    memset(rx, 0, sizeof(POEDRV_BCM_TYPE_PktBuf_T));

    tx_buf[0] = POEDRV_TYPE_CMD_IMAGE_UPGRADE;
    tx_buf[1] = POEDRV_TYPE_SUBCOM_DOWNLOAD_IMAGE;
    tx_buf[2] = 0;    /* offset */
    tx_buf[3] = 0;    /* offset */

    /* transmit first 4 byte */
    read_bytes = sysSerialPolltxBuff( tx_buf, 4 );

    /* transmit real image data  */
    for (i=0;i<image_size;i+=POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME)
    {
        memcpy(tx_buf, &image[i], POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME);
        read_bytes = sysSerialPolltxBuff( tx_buf, POEDRV_TRAMIT_IMAGE_BYTE_ONE_TIME );
        /* delay 10ms */
        SYSFUN_Sleep(1);
    }    

    SYSFUN_Sleep(50);  /* 0.5 second */

    /* CRC check */
    // printf("Do CRC check\n");
    tx_buf[0] = POEDRV_TYPE_CMD_IMAGE_UPGRADE;
    tx_buf[1] = POEDRV_TYPE_SUBCOM_CHECK_CRC;
    read_bytes = sysSerialPolltxBuff( tx_buf, 2 );

#endif

    return TRUE;
} /* end POEDRV_BCM_LocalUpgradeImage */

static BOOL_T POEDRV_BCM_LocalUpgradeImageCommand(UI8_T sub_command)
{
/*
    POEDRV_BCM_TYPE_PktBuf_T transmit;
    POEDRV_BCM_TYPE_PktBuf_T receive;
    UI8_T ret, i;
DBG_PRINT();

    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    transmit.command = POEDRV_TYPE_CMD_IMAGE_UPGRADE;
    transmit.seq_no = sub_command;

    POEDRV_BCM_SendAndWaitReply(&transmit,&receive);
    transmit.checksum = POEDRV_BCM_Checksum((UI8_T *)(&transmit));

    ret = sysSerialPolltxBuff((UI8_T*)&transmit, POEDRV_SIZE_OF_PACKET );
    SYSFUN_Sleep(2);
    ret = sysPollRxBuff2(POEDRV_SIZE_OF_PACKET, (UI8_T*)&receive);

    printf("Tx: ");
    for(i=0;i<POEDRV_SIZE_OF_PACKET;i++)
        printf("%2x ",((UI8_T*)(&transmit))[i]);
    printf("\r\nRx: ");
    for(i=0;i<POEDRV_SIZE_OF_PACKET;i++)
        printf("%2x ",((UI8_T*)(&receive))[i]);
    printf("\r\n");
*/
return TRUE;
}


POEDRV_CONTROL_T poedrv_59101 =
{
    "Broadcom 59101",                                      /* drv_name */
    POEDRV_BCM_Inits,                                /* poedrv_init */
    POEDRV_BCM_OpenUart,                                /* poedrv_open_uart */
    POEDRV_BCM_LocalUpgradeImage,                             /* poedrv_upgrade_image */
    POEDRV_BCM_LocalUpgradeImageCommand,             /* poedrv_upgrade_image_command */
    POEDRV_BCM_GetPortAdminStatus,                    /* poedrv_get_port_admin_status */
    POEDRV_BCM_SetPortAdminStatus,                  /* poedrv_set_port_admin_status */
    POEDRV_BCM_SetPortAllowPowerState,             /* poedrv_set_port_allow_power_state */
    POEDRV_BCM_EnablePortMapping,                   /* poedrv_enable_logical_port_map */
    POEDRV_BCM_DisablePortMapping,                       /* poedrv_disable_logical_port_map */
    POEDRV_BCM_ResetPort,                             /* poedrv_reset_port */
    POEDRV_BCM_GetPortDetectionType,                   /* poedrv_get_port_detection_type */
    POEDRV_BCM_SetPortDetectionType,              /* poedrv_set_port_detection_type */
    POEDRV_BCM_SetPortClassType,                 /* poedrv_set_port_classification_type */
    POEDRV_BCM_SetPortAutoMode,             /* poedrv_set_port_auto_mode */
    POEDRV_BCM_SetPortPowerThresholdType,       /* poedrv_set_port_power_threshold_type */
    POEDRV_BCM_GetPortPowerMaximumAllocation,   /* poedrv_get_port_power_limit */
    POEDRV_BCM_SetPortPowerMaximumAllocation, /* poedrv_set_port_power_limit */
    POEDRV_BCM_GetPortPowerPairs,               /* poedrv_get_port_power_pairs */
    POEDRV_BCM_SetPortPowerPairs,               /* poedrv_set_port_power_pairs */
    POEDRV_BCM_GetPortPriority,                 /* poedrv_get_port_priority */
    POEDRV_BCM_SetPortPriority,                 /* poedrv_set_port_priority */
    POEDRV_BCM_SetPortForceHighPowerMode,      /* poedrv_set_port_force_high_power_mode */
    POEDRV_BCM_SetPortDot3atHighPowerMode,      /* poedrv_set_port_dot3at_high_power_mode */
    POEDRV_BCM_SetCurrentInHighPower,          /* poedrv_set_current_in_high_powe */
    POEDRV_BCM_SetPowerManagementMode,         /* poedrv_set_power_management_mode */
    POEDRV_BCM_SetPowerSourceControl,          /* poedrv_set_power_source_control */
    POEDRV_BCM_GetPoeSoftwareVersion,          /* poedrv_get_soft_ver */
    POEDRV_BCM_GetPortStatus2,                  /* poedrv_get_port_status2 */
    POEDRV_BCM_GetTotalAllocPower,                 /* poedrv_get_total_allocated_power */
    POEDRV_BCM_GetPortPowerConsumption,        /* poedrv_get_port_power_consumption */
    POEDRV_BCM_GetPortMeasurement,                 /* poedrv_get_port_measurement */
    POEDRV_BCM_ResetPortStatistic,                 /* poedrv_reset_port_statistic */
    POEDRV_BCM_ResetAllPortStatistic,              /* poedrv_reset_all_port_statistic */
    NULL,                                          /* poedrv_get_port_mpsabsent_counter */
    NULL,                                          /* poedrv_get_port_invalid_sign_counter */
    NULL,                                          /* poedrv_get_port_power_deny_counter */
    NULL,                                          /* poedrv_get_port_overload_counter */
    NULL,                                          /* poedrv_get_port_short_counter */
    POEDRV_BCM_GetPortAllCounter,             /* poedrv_get_port_all_counter */
    POEDRV_BCM_GetPortAllConfig,              /* poedrv_get_port_all_config */
    POEDRV_BCM_ShowPortInfo,                   /* poedrv_show_port_info */
    POEDRV_BCM_ShowMainpowerInfo,              /* poedrv_show_mainpower_info */
    POEDRV_BCM_SendRawPacket                       /* poedrv_send_raw_packet */
 };

/* ==================Basic API to access PoE controller ============= */
/* FUNCTION NAME : POEDRV_BCM_SendPacket
 * PURPOSE: This function is used to send a packet to PoE controller
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   None
 */
static BOOL_T POEDRV_BCM_SendPacket(POEDRV_BCM_TYPE_PktBuf_T  *tx_buf)
{
    UI32_T out_bytes;

DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
         POEDRV_BACKDOOR_DisplayPacket(TRUE, (UI8_T *)tx_buf);
#endif

    out_bytes = sysSerialPolltxBuff((UI8_T*)tx_buf, POEDRV_SIZE_OF_PACKET);
//Eugene temp,    out_bytes = SYSFUN_UARTPutData( uart_handler, tx_buf, POEDRV_SIZE_OF_PACKET );

    return TRUE;
} /* End of POEDRV_SendPacket() */


/* FUNCTION NAME : POEDRV_BCM_ReceivePacket
 * PURPOSE: This function is used to receive a packet from PoE controller
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   None
 */
static BOOL_T POEDRV_BCM_ReceivePacket(POEDRV_BCM_TYPE_PktBuf_T *rx_buf)
{
    UI32_T             starting_ticks = 0;
    UI32_T             current_ticks = 0;
    UI32_T             length = 0;
    UI32_T             bytes_to_read = POEDRV_SIZE_OF_PACKET;
    UI32_T             read_bytes = 0;
    UI8_T              *buf_ptr = (UI8_T *)rx_buf;
//    UI32_T             int_mask;

    starting_ticks = SYS_TIME_GetSystemTicksBy10ms();
DBG_PRINT();

    while ( TRUE )
    {
          /* Read bytes from UART interface
           */
          // int_mask = SYSFUN_InterruptLock ();             /*  Interrupt Disable   */
          read_bytes = sysPollRxBuff2(bytes_to_read-length, buf_ptr);
//Eugene temp,          read_bytes = SYSFUN_UARTPollRxBuff(uart_handler,bytes_to_read-length, buf_ptr);
          // SYSFUN_InterruptUnlock (int_mask);              /*  Interrupt Enable    */

          /* Check to see if got bytes
           */
          if ( read_bytes > 0)
          {
               length += read_bytes;

               /* Check to see if 15-byte packet is received
                */
               if ( length == POEDRV_SIZE_OF_PACKET )
                    break;

#ifndef INCLUDE_DIAG
               //if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                    //printf(" - length %ld - ", length);
#endif
//DBG_PRINT(" - length %ld - ", length);
                if ( length > POEDRV_SIZE_OF_PACKET )
                {
DBG_PRINT("receive %lu bytes, too more!", length);
                    return FALSE;
                }
               buf_ptr += read_bytes;      /*length;*/
          }

          current_ticks = SYS_TIME_GetSystemTicksBy10ms();

//DBG_PRINT(" =%ld=  =%ld= ", starting_ticks, current_ticks);
          /* Time-out checking
           */
          if ( (current_ticks - starting_ticks) >= 40 )
          {
#ifndef INCLUDE_DIAG
               //if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                    //printf("\n\rTime out: Starting ticks=%ld Current ticks=%ld", starting_ticks, current_ticks);

//               if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
//                    POEDRV_BACKDOOR_DisplayPacket(FALSE, (UI8_T *)rx_buf);
#endif
               /* Return failed due to timeout
                */
#ifdef POEDRV_DEBUG
                    printf("\r\nPOEDRV_ReceivePacket receive timeout");
#endif
DBG_PRINT("timeout, receive %lu bytes", length);
               return FALSE;
          }
//		  else
//DBG_PRINT(" Time-out checking ");

          SYSFUN_Sleep(1);
    } /* End of while ( TRUE ) */

#ifndef INCLUDE_DIAG
//    if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
//         POEDRV_BACKDOOR_DisplayPacket(FALSE, (UI8_T *)rx_buf);
#endif

    return TRUE;
} /* End of POEDRV_ReceivePacket() */


/* FUNCTION NAME : POEDRV_BCM_Checksum
 * PURPOSE: This function is used to calculate the 8-bit arithmetic sum of the
 *          first 11 data bytes from the data buffer.
 * INPUT:   buffer -- starting address for calculating checksum
 * OUTPUT:  None
 * RETURN:  checksum -- the 1-byte result of checksum
 * NOTES:   None
 */
static UI8_T POEDRV_BCM_Checksum(UI8_T *buffer)
{
    UI8_T              index,checksum;
    UI16_T             checksum_temp = 0;

    for (index=0; index<POEDRV_NO_OF_BYTES_FOR_CHECKSUM;index++)
        checksum_temp += buffer[index];

    checksum = checksum_temp&0xFF;
    return checksum;

} /* End of POEDRV_Checksum() */


/* FUNCTION NAME : POEDRV_BCM_SendAndWaitReply
 * PURPOSE: This function is used to send a request to PoE controller, and
 *          then wait the reply from PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   This function also provide the service for the calculation
 *          and verification of checksum on all Tx and Rx packet. It also
 *          automatically generate the echo number, incremented by 1.
 */
static BOOL_T POEDRV_BCM_SendAndWaitReply(POEDRV_BCM_TYPE_PktBuf_T *tx_buf, POEDRV_BCM_TYPE_PktBuf_T *rx_buf)
{
    UI8_T checksum = 0;
    BOOL_T ret = FALSE;
    BOOL_T resend_this_packet = TRUE;
    UI8_T retry_times = 0;

DBG_PRINT();
    POEDRV_BCM_ENTER_CRITICAL_SECTION;

    /* Increment the sequence number for this packet
     * If seq_no != 0xFF, means it's a subcmd, don't change it's value
     */
    if (tx_buf->seq_no == 0xFF)
    {
        INCREMENT_SEQ_NUMBER();
        tx_buf->seq_no = poedrv_seq_number;
    }

    /* Calculate the checksum for this transmitting packet
     */
    tx_buf->checksum = POEDRV_BCM_Checksum((UI8_T *)tx_buf);

    while ( (resend_this_packet == TRUE) && (retry_times < 3) )
    {
        /* clear buffer */
        {
            UI8_T temp;
            do
            {
                ret=sysPollRxBuff2(1, &temp);
            }while(ret);
        }

        //SYSFUN_Sleep(1); /* wait for 10ms*/
        if ( POEDRV_BCM_SendPacket(tx_buf) == TRUE )
        {
            SYSFUN_Sleep(4); /* wait for 10ms*/
            if ( POEDRV_BCM_ReceivePacket(rx_buf) == TRUE )
            {
                /* Calculate the checksum for this receiving packet
                 */
                checksum   =POEDRV_BCM_Checksum((UI8_T *)rx_buf);
                if ( (rx_buf->checksum == checksum) && (rx_buf->seq_no == tx_buf->seq_no))
                {
                    if (rx_buf->command == tx_buf->command)
                    {
#if 0
{
UI8_T index;
printf("SUCCESS!!\r\nTx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)tx_buf)[index]);
}
printf("\r\nRx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)rx_buf)[index]);
}
printf("\r\n");
}
#endif
                        resend_this_packet = FALSE;
                        ret = TRUE;
                    } /* End of if (rx_buf->command == tx_buf->command)*/
                    else
                    {
DBG_PRINT("command diff!!");
                        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                            printf("command Error\n");
                    }
                } /* End of if ( (rx_buf->checksum == checksum) &&.. */
                else
                {
#if 0
{
DBG_PRINT("checksum/sequence error:");
UI8_T index;
printf("Tx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)tx_buf)[index]);
}
printf("\r\nRx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)rx_buf)[index]);
}
printf("\r\n");
}
#endif
                    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                    {
                        printf("\n\rchecksum/Sequence Error");
                        POEDRV_BACKDOOR_DisplayPacket(TRUE, (UI8_T *)tx_buf);
                        POEDRV_BACKDOOR_DisplayPacket(FALSE, (UI8_T *)rx_buf);
                        printf("\n\r");
                    }
                }
            } /* End of if ( POEDRV_ReceivePacket((UI8_T *)rx_buf) == TRUE )*/
            else
            {
#if 0
{
DBG_PRINT("POEDRV_BCM_ReceivePacket Error");
UI8_T index;
printf("Tx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)tx_buf)[index]);
}
printf("\r\nRx:");
for(index=0;index<12;index++)
{
printf(" %2x", ((UI8_T*)rx_buf)[index]);
}
printf("\r\n");
}
#endif
                if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                    printf("POEDRV_BCM_ReceivePacket Error\n");
            }
        }  /* End of if ( POEDRV_SendPacket((UI8_T *)tx_buf) == TRUE )*/
        else
        {
DBG_PRINT("POEDRV_BCM_SendPacket Error");
            if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("POEDRV_BCM_SendPacket Error\n");
        }
        retry_times++;
        SYSFUN_Sleep(2); /* wait for 10ms*/
    }

    POEDRV_BCM_LEAVE_CRITICAL_SECTION;
    return ret;
} /* End of POEDRV_SendAndWaitReply() */


