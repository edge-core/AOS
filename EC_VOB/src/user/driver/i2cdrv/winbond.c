/*---------------------------------------------------------------------
 * File_Name : WINBOND.C
 *
 * Purpose   : 
 * 
 * Copyright(C)      Accton Corporation, 2006
 *
 * Note    : 
 * 
 * Porting : River.Xu 2008. 2.28
 * I2C Switch 1, Channel 2 
 *---------------------------------------------------------------------
 */

#include "i2c.h"
#include "i2cdrv.h"
#include "winbond.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "stdio.h"

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_SetBank
 *---------------------------------------------------------------------------
 * PURPOSE:  set W83782D bank
 * INPUT:      num:W83782D bank num.             
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T WINBOND_SetBank(UI8_T num)
{
    UI8_T ch;

    if( num>7 ) 
        return FALSE;
    
    ch = 0x80|num;

    /* Turn on */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetBank: I2CDRV_SetChannel open fail !\n");
        return FALSE;
    }

    /* Transaction */
    if( I2CDRV_SetI2CInfo(W38782D_ADDR, W38782D_BANK, 1, &ch) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetBank: I2CDRV_SetI2CInfo fail !\n");
        return FALSE;
    }

    /* Turn off */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT ) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetBank:I2CDRV_SetChannel close fail !\n");
        return FALSE;
    }

    I2C_DEBUG_OUT("WINBOND_SetBank: I2C_do_transaction success !\n");    

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init chip
 * INPUT:          
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */


static BOOL_T WINBOND_Init()
{
    UI8_T data = 0;
    static BOOL_T init_flag = FALSE;

    if(init_flag != TRUE)
        /* Initial w83782 thermal monitor  */
    {
        if(WINBOND_SetBank(0) != TRUE)
            return FALSE;

        /* Turn on */
        if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
        {
            I2C_DEBUG_OUT("WINBOND_Init:I2CDRV_SetChannel open fail !\n");
            return FALSE;
        }

        /* Transaction */
        if (I2CDRV_GetI2CInfo(W38782D_ADDR, W38782D_FAN_DIVISOR_2, 1, &data) != TRUE)
            return FALSE;

        data |= 0x0E;
        if (I2CDRV_SetI2CInfo(W38782D_ADDR, W38782D_FAN_DIVISOR_2, 1, &data) != TRUE)
            return FALSE;

        data = 0x00;
        if (I2CDRV_SetI2CInfo(W38782D_ADDR, W38782D_DIODE_SELECT, 1, &data) != TRUE)
            return FALSE;    
    
        /* Initial w83782 fan clk input to 48MHz */
        data = 0x48;
        if (I2CDRV_SetI2CInfo(W38782D_ADDR, W38782D_MONITOR_CLKINSEL, 1, &data) != TRUE)
            return FALSE;    

        /* Turn off */
        if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT ) != TRUE )
        {
            I2C_DEBUG_OUT("WINBOND_Init:I2CDRV_SetChannel close fail !\n");
            return FALSE;
        }   
    }
    init_flag = TRUE;
    return TRUE;        
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_GetThermal
 *---------------------------------------------------------------------------
 * PURPOSE:  get cuurent temperature from Thermal
 * INPUT:      index: Thermal index 1,2,3.             
 * OUTPUT:   temperature:Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T WINBOND_GetThermal(UI8_T index, I8_T* temperature)
{
    UI8_T data[2] = {0};
    UI8_T temp;
    UI16_T offset;

    if( temperature == NULL )
    {
        I2C_DEBUG_OUT("WINBOND_GetThermal:temperature NULL !\n");
        return FALSE;
    }
    
    if( index==1 ) offset=W38782D_THERMAL_1;
    else if( index==2) offset=W38782D_THERMAL_2;
    else if(index==3) offset=W38782D_THERMAL_3;
    else return FALSE;

    /* Read temperature from thermal sensor */
    if( WINBOND_SetBank(index - 1)!= TRUE ) 
        return FALSE;

    /* Turn on */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_GetThermal:I2CDRV_SetChannel open fail !\n");
        return FALSE;
    }

    /* Transaction */
    if(index == 1)
    {
        if( I2CDRV_GetI2CInfo(W38782D_ADDR, offset, 1, data) != TRUE )
        {
            I2C_DEBUG_OUT("WINBOND_GetThermal:I2CDRV_GetI2CInfo fail !\n");
            return FALSE;
        }

        if(data[0] & 0x80) 
        {
            *temperature =(-1) * ( (~data[0]) + 1 );
        }
        else 
        {
            *temperature = data[0];
        }        
    }
    else
    {
        if( I2CDRV_GetI2CInfo(W38782D_ADDR, offset, 2, data)!= TRUE )
        {
            I2C_DEBUG_OUT("WINBOND_GetThermal:I2CDRV_GetI2CInfo close fail !\n");
            return FALSE;
        }

        if(data[0] & 0x80) 
        {
            temp = 0xFF00 |data[0];
            temp = (temp << 1) | (data[1] >> 7);
            *temperature = (-1) * (~temp + 1);
        }
        else  
        {
            *temperature = (( data[0] << 1) | (data[1] >> 7)) / 2;
        }
    }

    /* Turn off */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_GetThermal:I2CDRV_SetChannel close fail !\n");
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_SetThreshold
 *---------------------------------------------------------------------------
 * PURPOSE:  set Thermal trap value
 * INPUT:      index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T WINBOND_SetThreshold(UI8_T index, I8_T  temperature) 
{
    I16_T data= 0;
    UI8_T data1, data2;

    /* Only Thermal sensor 2 and 3 have over-temperature threshold */
    if(index != 2 && index != 3) 
        return FALSE;

    /* read temperature from thermal sensor */
    if( WINBOND_SetBank(index - 1) != TRUE ) 
        return FALSE;

    data = temperature << 7;

    data1 = (UI8_T) (data & 0xFF00) >> 8;
    data2 = (UI8_T) (data & 0x00FF) ;

    I2C_DEBUG_OUT("Threshold : %d %d %d \r\n", data, data1, data2);

    /* Turn on */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetThreshold:I2CDRV_SetChannel open fail !\n");
        return FALSE;
    }

    /* Transaction */
    if( I2CDRV_SetI2CInfo(W38782D_ADDR, W38782D_THERMAL_OVER_H, 1, &data1) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetThreshold:I2CDRV_SetI2CInfo fail !\n");
        return FALSE;
    }
    
    /* Turn off */
   if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetThreshold:I2CDRV_SetChannel close fail !\n");
        return FALSE;
    }

    I2C_DEBUG_OUT("WINBOND_SetThreshold success !\n");

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_GetFan
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:      index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T WINBOND_GetFan(UI8_T index, UI32_T* speed)
{
    UI8_T divisor[3], count[3];
    UI8_T reg[3];
    UI8_T  divisor_offset[3] = {W38782D_FAN_DIVISOR_0, W38782D_FAN_DIVISOR_1, W38782D_FAN_DIVISOR_2};
    UI8_T  count_offset[3] = {W38782D_FAN_COUNT_0, W38782D_FAN_COUNT_1, W38782D_FAN_COUNT_2};

    int i;

    if(index < 1 || index > 3) 
        return FALSE;

    /* Select Bank 0*/
    if( WINBOND_SetBank(0) != TRUE ) 
        return FALSE;

    /* Turn on */
   if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_GetFan:I2CDRV_SetChannel open fail !\n");
        return FALSE;
    }

    /* Read Divisor Register */
    for(i=0; i < 3; i++)
    {
        if( I2CDRV_GetI2CInfo(W38782D_ADDR, divisor_offset[i], 1, reg+i) != TRUE )
            return FALSE;
    }
    
    /* Calculate divisor */
    divisor[0] = ((reg[0] & 0x30)>>4) | ( (reg[2] & 0x20)>>3);
    divisor[1] = ((reg[0] & 0xC0)>>6) | ( (reg[2] & 0x40)>>4);
    divisor[2] = ((reg[1] & 0xC0)>>6) | ( (reg[2] & 0x80)>>5);

    for(i = 0; i < 3; i++)
    {
        divisor[i] = ( divisor[i] == 0 ) ? 1 : (2 << (divisor[i]-1));
    }

    /* Read Counter */
    if( I2CDRV_GetI2CInfo(W38782D_ADDR, count_offset[index -1], 1, count + index -1)!= TRUE )
    {        
        I2C_DEBUG_OUT("WINBOND_GetFan:I2CDRV_GetI2CInfo fail !\n");
        return FALSE;
    }

    /* count = 255 mean the lowest speed so we think the speed is 0 */
    if (count[index - 1] == 0xFF)
    {
        *speed = 0;
        return TRUE;
    }
    
    if(count[index - 1] != 0 && divisor[index - 1] !=0)
    {
        *speed = 1350000/(divisor[index - 1] * count[index - 1]);
        I2C_DEBUG_OUT("Speed %d : %ld\r\n", index, *speed);
    }
    else 
        return FALSE;   

    /* Turn off */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_GetFan:I2CDRV_SetChannel close fail !\n");
        return FALSE;
    }

    I2C_DEBUG_OUT("WINBOND_GetFan success !\n");

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WINBOND_SetFan
 *---------------------------------------------------------------------------
 * PURPOSE:  set fan speed value
 * INPUT:      index: fan index.             
 * OUTPUT:   speed:set fan speed.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T WINBOND_SetFan(UI8_T index, UI32_T speed)
{
	UI8_T data = 0;
	UI16_T offset;

    if(speed > SYS_HWCFG_FAN_SPEED_MAX) 
        return FALSE;
        
	if( index==1 ) offset=W38782D_FAN_CTL_0;
	else if( index==2) offset=W38782D_FAN_CTL_1;
	else if(index==3) offset=W38782D_FAN_CTL_2;
	else return FALSE;

    /* Select Bank 0*/
	if( WINBOND_SetBank(0) != TRUE ) 
        return FALSE;

    /*calculate PWM duty data*/
    data = (speed * 255) / SYS_HWCFG_FAN_SPEED_MAX;

    I2C_DEBUG_OUT("speed = %ld PWM = %d\r\n", speed, data);

    /* Turn on */
    if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetFan:I2CDRV_SetChannel open fail !\n");
        return FALSE;
    }

    /* Transaction */
    if( I2CDRV_SetI2CInfo(W38782D_ADDR, offset, 1, &data)!= TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetFan:I2CDRV_SetI2CInfo fail !\n");
        return FALSE;
    }
    
    /* Turn off */
   if( I2CDRV_SetChannel(I2C_CHANNEL_DEVICE_ADDR, I2C_CHANNEL_DEVICE_DATARGADDR, 
                                       I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT) != TRUE )
    {
        I2C_DEBUG_OUT("WINBOND_SetFan:I2CDRV_SetChannel close fail !\n");
        return FALSE;
    }

    I2C_DEBUG_OUT("WINBOND_SetFan success !\n");

	return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init chip
 * INPUT:          
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_CHIP_Init()
{
    if(WINBOND_Init() != TRUE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init chip
 * INPUT:          
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_CHIP_Init()
{
    if(WINBOND_Init() != TRUE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetTemperature
 *---------------------------------------------------------------------------
 * PURPOSE:  get cuurent temperature from Thermal
 * INPUT:      index: Thermal index 1,2,3.             
 * OUTPUT:   temperature:Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_GetTemperature(UI8_T index, I8_T* temperature)
{
    if(WINBOND_GetThermal(index,temperature) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_SetThreshold
 *---------------------------------------------------------------------------
 * PURPOSE:  set Thermal trap value
 * INPUT:      index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_SetThreshold(UI8_T index, I8_T  temperature) 
{
    if(WINBOND_SetThreshold(index,temperature) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInRPM
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInRPM(UI8_T index, UI32_T* speed)
{
    if(WINBOND_GetFan(index,speed) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInDutyCycle(UI8_T index, UI32_T* speed)
{
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_SetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE:  set fan speed value
 * INPUT:      index: fan index.             
 * OUTPUT:   speed:set fan speed.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_SetSpeed(UI8_T index, UI32_T speed)
{
    if(WINBOND_SetFan(index,speed) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}
