/* MODULE NAME:  sys_hwcfg_common.h
 * PURPOSE:
 *  This file contains the common definitions that apply to all projects.
 *
 * NOTES:
 *  None
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    6/2/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
#ifndef SYS_HWCFG_COMMON_H
#define SYS_HWCFG_COMMON_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
/* enumeration for register access method
 */
#define SYS_HWCFG_REG_ACCESS_METHOD_I2C                          0 /* do I2C operation with given i2c dev addr */
#define SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL             1 /* do I2C operation with given i2c dev addr and i2c mux channel info */
#define SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR                      2 /* do physicall address access operation */
#define SYS_HWCFG_REG_ACCESS_METHOD_SYS_HWCFG_API                3 /* for special case, HWCFG shall provide API to read/write reg */
#define SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_BUS_IDX             4 /* do I2C operation with given i2c bus index and i2c dev addr */

#define SYS_HWCFG_FLASH_TYPE_NOR    0x01  /* bit 0 */
#define SYS_HWCFG_FLASH_TYPE_NAND   0x02  /* bit 1 */

/* Definitions for thermal types
 * These value will be used by SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP definition
 * to represent the bit position in the bitmap.
 */
#define SYS_HWCFG_THERMAL_LM75                      (0UL)
#define SYS_HWCFG_THERMAL_LM77                      (1UL)
#define SYS_HWCFG_THERMAL_W83782                    (2UL)
#define SYS_HWCFG_THERMAL_NE1617A                   (3UL)
#define SYS_HWCFG_THERMAL_MARVELL_PHY_88E154X       (4UL)
#define SYS_HWCFG_THERMAL_MAX6581                   (5UL)
#define SYS_HWCFG_THERMAL_ONLP                      (6UL) /* A wrapper thermal driver that will call ONLP APIs. SYS_CPNT_SYSDRV_USE_ONLP must be defined as TRUE when using this driver. */

/* Definitions for fan type:
 * These value will be used by SYS_HWCFG_FAN_SUPPORT_TYPE_BMP definition
 * to represent the bit position in the bitmap.
 *
 * SYS_HWCFG_FAN_MAX6651 ==> BLANC, BLANC_08, Hagrid, Hagrid L3
 * SYS_HWCFG_FAN_ADM1030 ==> EIF8X10G
 */

#define SYS_HWCFG_FAN_NONE          (0UL) /* no fan control chip */
#define SYS_HWCFG_FAN_ADM1030       (1UL) /* ADM1030 chip */
#define SYS_HWCFG_FAN_MAX6651       (2UL) /* MAX6651 chip */
#define SYS_HWCFG_FAN_WIN83782      (3UL) /* WINBOND W83782D chip */
#define SYS_HWCFG_FAN_ADT7470       (4UL) /* ADT7470 */
#define SYS_HWCFG_FAN_ADT7475       (5UL) /* ADT7475 */
#define SYS_HWCFG_FAN_AOS5700_54X   (6UL) /* AOS5700-54X fan contoller(CPLD) */
#define SYS_HWCFG_FAN_AOS6700_32X   (7UL) /* AOS6700-32X fan contoller(CPLD) */
#define SYS_HWCFG_FAN_ONLP          (8UL) /* A wrapper fan driver that will call ONLP APIs. SYS_CPNT_SYSDRV_USE_ONLP must be defined as TRUE when using this driver. */

/* common definition for power module type
 * Need to update the macro function SYS_HWCFG_COMMON_POWER_TYPE_IS_DC
 * if new power module type is added
 */
enum {
    SYS_HWCFG_COMMON_POWER_DC_N48_MODULE_TYPE=0,
    SYS_HWCFG_COMMON_POWER_DC_P24_MODULE_TYPE,
    SYS_HWCFG_COMMON_POWER_AC_MODULE_TYPE,
    SYS_HWCFG_COMMON_POWER_UNKNOWN_MODULE_TYPE
};


/* common definition for hardware watchdog type
 */
#define SYS_HWCFG_WATCHDOG_MPC8248         0
#define SYS_HWCFG_WATCHDOG_MARVELL_PONCAT  1
#define SYS_HWCFG_WATCHDOG_OS6450          2
#define SYS_HWCFG_WATCHDOG_ECS5610_52S     3
#define SYS_HWCFG_WATCHDOG_ECS4910_28F     4

/* common definition for Synchronous Ethernet chip type
 */
#define SYS_HWCFG_SYNCE_CHIP_ZL30132    0
#define SYS_HWCFG_SYNCE_CHIP_IDT82V3399 1

/* common definition for UART TO I2C chip type
 */
#define SYS_HWCFG_UART_TO_I2C_CHIP_NONE        0
#define SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700   1

/* common definition for CPLD TYPE
 */
#define SYS_HWCFG_CPLD_TYPE_NONE        0
#define SYS_HWCFG_CPLD_TYPE_LATTICE     (1UL<<0) /* Bit 0 */

/* DATA TYPE DECLARATIONS
 */

/* I2C register information
 */
typedef struct SYS_HWCFG_i2cRegInfo_S
{
    UI8_T dev_addr;
    UI8_T op_type; /* I2C_7BIT_ACCESS_MODE/I2C_10BIT_ACCESS_MODE */
    UI8_T validOffset;
    UI8_T offset;
    UI8_T moreThen256;
    UI8_T data_len;
    UI8_T bus_idx;
}SYS_HWCFG_i2cRegInfo_T;

/* For I2C register access that need to set I2C channel
 */
typedef struct SYS_HWCFG_i2cRegAndChannelInfo_S
{
    SYS_HWCFG_i2cRegInfo_T i2c_reg_info;
    UI8_T                  i2c_mux_index; /* 0 based */
    UI32_T                 channel_val;
}SYS_HWCFG_i2cRegAndChannelInfo_T;

/* For the view of caller of I2CDRV_SetAndLockMux(), the bits that set as 1 in
 * channel_bmp means the corresponding mux channels to be opened (For example,
 * channel_bmp=3 means the first two channels need to be opened on the mux).
 * And channel_bmp=0 means closing all channels on the mux.
 *
 * However, the real value to be set to the I2C mux to open or close all channels
 * is different among different type of I2C mux device. This enum type depicts
 * the translation method between channel_bmp and the real value to be set to
 * the I2C mux device.
 *
 * Here is the description of each translation type:
 * SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_DIRECT
 *     -  The value to be set to the I2C mux device is exactly the same value
 *        the argument channel_bmp.
 *
 * SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL
 *     -  The value to be set to the I2C mux device needs to be converted to
 *        the corresponding channel index value. Note that this type of translation
 *        does not allow more than one channel to be opened at the same time.
 *        See the translation examples below:
 *       --------------------------------------------- 
 *        channel_bmp val        real value to be set
 *       ---------------------------------------------
 *        0x0000_0001            0
 *        0x0000_0002            1
 *        0x0000_0004            2
 *        0x0000_0008            3
 *        0x0000_0010            4
 *                       ...
 *        0x1000_0000            28
 *        0x2000_0000            29
 *        0x4000_0000            30
 *        0x8000_0000            31
 *        0x0000_0000            256 (Close all channels)
 *
 * SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL_2
 *     -  The value to be set to the I2C mux device needs to be converted to
 *        the corresponding channel index value. Note that this type of translation
 *        does not allow more than one channel to be opened at the same time.
 *        See the translation examples below:
 *       --------------------------------------------- 
 *        channel_bmp val        real value to be set
 *       ---------------------------------------------
 *        0x0000_0001            1
 *        0x0000_0002            2
 *        0x0000_0004            3
 *        0x0000_0008            4
 *        0x0000_0010            5
 *                       ...
 *        0x0000_0100            9
 *        0x0000_0200            16 (Translation method changed here)
 *        0x0000_0400            17
 *        0x0000_0800            18
 *                       ...
 *        0x0001_0000            23
 *        0x0002_0000            24
 *        0x0004_0000            25
 *        0x0008_0000            32 (Translation method changed here)
 *        0x0010_0000            33
 *        0x0020_0000            34
 *                       ...
 *        0x1000_0000            41
 *        0x2000_0000            42
 *        0x4000_0000            43
 *        0x8000_0000            44
 *        0x0000_0000            0  (Close all channels)
 *
 */
typedef enum SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_E
{
    SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_DIRECT,
    SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL,
    SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL_2,
    SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_MAX
} SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T;

/* For I2C register access that support set multi-level Mux channel
 & This structure defines the entry which contains the information of the
 * channel to be set on each intermediate level of mux device in order to
 * access the leaf mux device.
 *
 * The projects that need to support multi-level Mux channel should defines
 * its own SYS_HWCFG_MultiLevelMuxInfoEntry_T in the
 * way shown below:
 * typedef struct SYS_HWCFG_MultiLevelMuxInfoEntry_S
 * {
 *     SYS_HWCFG_COMMON_MultiLevelMuxInfoEntry_T common_info;
 *     UI8_T dev_addr_of_intermediate_mux[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
 *     UI8_T channel_idx_on_mux[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
 *     BOOL_T intermediate_mux_has_channel_reg[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
 *     UI8_T intermediate_mux_channel_reg_addr[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
 *     SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T leaf_mux_channel_bmp_xslt_type[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
 * } SYS_HWCFG_MultiLevelMuxInfoEntry_T;
 */
typedef struct SYS_HWCFG_COMMON_MultiLevelMuxInfoEntry_S
{
    /* i2c bus index */
    UI8_T  i2c_bus_idx;
    /* number of intermediate mux device to be set to access the leaf mux device */
    UI8_T num_of_intermediate_mux;
    /* device address of the leaf mux device */
    UI8_T leaf_mux_dev_addr;
    /* does the leaf mux device has the register to set channel? */
    BOOL_T leaf_mux_dev_has_channel_reg;
    /* the address of the register to set channel */
    UI8_T leaf_mux_dev_channel_reg_addr;  /* valid only when leaf_mux_dev_has_channel_reg is TRUE */
    SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T leaf_mux_channel_bmp_xslt_type; /* type of translation from channel_bmp to the real set value for the I2C mux */
    /* move these two fields into SYS_HWCFG_MultiLevelMuxInfoEntry_T because
     * this header file cannot include sys_hwcfg.h and SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM
     * shall be defined in sys_hwcfg.h
     */
    #if 0
    /* device address of the intermediate mux device */
    UI8_T dev_addr_of_intermediate_mux[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
    /* the channel index(0 based) to be set on each intermediate mux device */
    UI8_T channel_idx_on_mux[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
    /* does the intermediate mux device has the register to set channel? */
    BOOL_T intermediate_mux_has_channel_reg[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
    /* the address of the register to set channel for the intermediate mux device */
    UI8_T intermediate_mux_channel_reg_addr[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM]; /* valid only when intermediate_mux_has_channel_reg[n] is TRUE */
    /* the type of translation from channel_bmp to the real set value for the the intermediate mux device */
    SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T leaf_mux_channel_bmp_xslt_type[SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM];
    #endif
} SYS_HWCFG_COMMON_MultiLevelMuxInfoEntry_T;

typedef struct SYS_HWCFG_I2CTransactionEntry_S
{
    UI8_T  i2c_bus_idx;          /* i2c bus index */
    UI8_T  dev_addr;
    UI8_T  trans_type;           /* I2C_7BIT_ACCESS_MODE/I2C_10BIT_ACCESS_MODE */
    BOOL_T validOffset;          /* TRUE if contains offset in I2C transaction */
    BOOL_T offset_more_than_256; /* TRUE if offset is more than 256 */
    UI32_T offset;               /* used only when validOffset is TRUE */
    UI8_T  data;                 /* data to be written */
    UI8_T  data_mask;            /* The written value = ((original read value) & ~data_mask) | (data | data_mask)*/
}SYS_HWCFG_I2CTransactionEntry_T;

/* PHY_ADDR register information
 */
typedef struct SYS_HWCFG_phyAddrRegInfo_S
{
    SYS_TYPE_PAddr_T physical_address;
    UI8_T  data_type_bit_len;  /* data width, valid settings: 8/16/32 */
    UI8_T  data_len;           /* number of bytes */
}SYS_HWCFG_phyAddrRegInfo_T;

/* Alarm output register information
 */
typedef struct SYS_HWCFG_AlarmOutputRegInfo_S
{
    UI32_T mask;
    /* output_enable_val specify the value for enabling alarm output
     * the value to disable alarm output is evaluated by the expression shown
     * below:
     *     ((~output_enable_val) & mask)
     */
    UI32_T output_enable_val;
    union
    {
        SYS_HWCFG_i2cRegInfo_T     i2c;      /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_phyAddrRegInfo_T phy_addr; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
    
}SYS_HWCFG_AlarmOutputRegInfo_T;

typedef struct SYS_HWCFG_AlarmOutputLedRegInfo_S
{
    UI32_T mask;
    /* led_on_val specify the value for lighting up alarm output led
     * the value to turn off alarm output led is evaluated by the expression
     * shown below:
     *     ((~led_on_val) & mask)
     */
    UI32_T led_on_val;
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_phyAddrRegInfo_T phy_addr; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_AlarmOutputLedRegInfo_T;

typedef struct SYS_HWCFG_ThermalRegInfo_S
{
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_i2cRegAndChannelInfo_T i2c_with_channel; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_ThermalRegInfo_T;

/* each thermal index(system-wised index, 0 based) occupies one
 * SYS_HWCFG_ThermalControlInfo_T entry
 */
typedef struct SYS_HWCFG_ThermalControlInfo_S
{
    /* the index of the thermal controller ASIC which
     * belongs to this thermal index(system-wised index, 0 based)
     */
    UI32_T thermal_ctl_idx;
    /* The thermal type of this thermal controller index
     * The value is one of the constants prefixed with
     * "SYS_HWCFG_THERMAL_" which is defined in
     * sys_hwcfg_common.h
     */
    UI32_T thermal_type;
   /* A thermal controller ASIC might be able to have
    * more than one thermal sensors. This value specifies
    * the internal thermal sensor index in this thermal controller
    * index(0 based).
    */
    UI32_T thermal_ctl_internal_thermal_idx;

    SYS_HWCFG_ThermalRegInfo_T reg_info;
}SYS_HWCFG_ThermalControlInfo_T;

typedef struct SYS_HWCFG_ThermalOps_S
{
    BOOL_T (*thermal_chip_init)(UI32_T thermal_idx);
    BOOL_T (*thermal_chip_get_temperature)(UI8_T thermal_idx, I8_T* temperature);
}SYS_HWCFG_ThermalOps_T;


typedef struct SYS_HWCFG_SFPTxDisableRegInfo_S
{
    UI32_T mask;
    /* tx_disable_val specify the value for sfp tx disable
     * to activate sfp tx disable, the value to be set is evaluated by the
     * expression shown below:
     *     (original read val & ~(mask)) | (tx_disable_val & mask)
     * to deactivate sfp tx disable, the value to be set is envaulated by the
     * expression shown below:
     *     (original read val & ~(mask)) | (~(tx_disable_val) & mask)
     */
    UI32_T tx_disable_val;
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_i2cRegAndChannelInfo_T i2c_with_channel; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
        SYS_HWCFG_phyAddrRegInfo_T phy_addr; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_SFPTxDisableRegInfo_T;

/* Fan related definitions  -  START
 */
#define SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_REG_BIT       (1<<0)  /* evaluate the fan status by the fan fault register */
#define SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_FAN_SPEED_BIT (1<<1)  /* evaluate the fan status by the fan speed */


typedef struct SYS_HWCFG_FanRegInfo_S
{
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_i2cRegAndChannelInfo_T i2c_with_channel; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_FanRegInfo_T;

typedef struct SYS_HWCFG_FanStatusInfo_S
{
    /* Information for the method to access fan fault register
     */
    SYS_HWCFG_FanRegInfo_T fan_fault_reg_info;
    /* the bit mask used to get the fan fault value from the value read from the
     * fan fault register
     */
    UI8_T  fan_fault_reg_mask;
    /* the value that indicates the fan is faulty
     */
    UI8_T  fan_fault_reg_val;
    /* the fan is considered as faulty when the fan speed is less than the value
     * in fan_fault_speed(the unit is RPM).
     */
    UI32_T fan_fault_speed;
    /* The bitmap for evaluation method, the meanings of each bit can
     * be found at SYS_HWCFG_FAN_STATUS_EVAL_METHOD_XXX_BIT. (e.g. SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_REG_BIT)
     */
    UI8_T  fan_status_eval_method_bmp;
}SYS_HWCFG_FanStatusInfo_T;

/* each fan index(system-wised index, 0 based) occupies one 
 * SYS_HWCFG_FanControllerInfo_T entry
 */
typedef struct SYS_HWCFG_FanControllerInfo_S
{
    /* the index of the fan controller ASIC which
     * control this fan_idx(system-wised index, 0 based)
     */
    UI32_T fan_ctl_idx;
    /* The fan controller type of this fan index
     * The value is one of the constants prefixed with
     * "SYS_HWCFG_FAN_" which is defined in
     * sys_hwcfg_common.h
     */
    UI32_T fan_ctl_type;
    
   /* A fan controller might be able to detect
    * more than one fan speed. This value specifies
    * the internal fan speed index in the fan
    * fan controller (0 based).
    */
    UI32_T fan_ctl_internal_fan_speed_detect_idx;

   /* A fan controller might be able to control
    * more than one fan . This value specifies
    * the internal fan speed control index in this fan
    * controller index(0 based).
    */
    UI32_T fan_ctl_internal_fan_speed_control_idx;

    SYS_HWCFG_FanRegInfo_T reg_info;
}SYS_HWCFG_FanControllerInfo_T;

typedef struct SYS_HWCFG_FanControllerOps_S
{
    BOOL_T (*fan_chip_init)(UI32_T fan_idx);
    BOOL_T (*fan_chip_get_speed_in_rpm)(UI8_T fan_idx, UI32_T* speed);
    BOOL_T (*fan_chip_get_speed_in_duty_cycle)(UI8_T fan_idx, UI32_T* duty_cycle_p);
    BOOL_T (*fan_chip_set_speed)(UI8_T fan_idx, UI32_T speed);
}SYS_HWCFG_FanControllerOps_T;
/* Fan related definitions  -  END
 */

/* Power related definitions  -  START
 */
/* SYS_HWCFG_POWER_STATUS_TYPE_T defines the constants for different types of
 * power related status
 */
typedef enum
{
  SYS_HWCFG_POWER_STATUS_TYPE_PRESENT,      /* the present status of the power module */
  SYS_HWCFG_POWER_STATUS_TYPE_POWER_FAILED, /* the powering status of the power module */
  SYS_HWCFG_POWER_STATUS_TYPE_ALERT,        /* the alert status of the power module */
} SYS_HWCFG_POWER_STATUS_TYPE_T;

typedef struct SYS_HWCFG_PowerRegInfo_S
{
    UI8_T power_alert_mask;
    UI8_T power_is_alert_val;
    UI8_T power_present_mask;
    UI8_T power_is_present_val;
    UI8_T power_status_mask;
    UI8_T power_status_ok_val;
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_i2cRegAndChannelInfo_T i2c_with_channel; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_PowerRegInfo_T;

/* Power related definitions  -  END
 */

typedef struct SYS_HWCFG_RTCRegInfo_S
{
    union
    {
        SYS_HWCFG_i2cRegInfo_T i2c; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
        SYS_HWCFG_i2cRegAndChannelInfo_T i2c_with_channel; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
    }info;
    UI8_T access_method; /* valid values are SYS_HWCFG_REG_ACCESS_METHOD_XXX */
}SYS_HWCFG_RTCRegInfo_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define SYS_HWCFG_COMMON_POWER_TYPE_IS_DC(power_type) ({ \
    BOOL_T __ret; \
    if(((power_type)!=SYS_HWCFG_COMMON_POWER_AC_MODULE_TYPE) && \
       ((power_type)!=SYS_HWCFG_COMMON_POWER_UNKNOWN_MODULE_TYPE)) \
       __ret=TRUE; \
    else \
       __ret=FALSE; \
    __ret;})
        
    

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/****************
 * Alarm Output *
 ****************
 */
/* FUNCTION NAME: SYS_HWCFG_GetMajorAlarmOutputRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function provides the register information to control major
 *          alarm output 
 *-----------------------------------------------------------------------------
 * INPUT    : board_id  -  which board id
 * OUTPUT   : info_p    -  the register info for major alarm output control
 * RETURN   : TRUE if get sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: This API is used when SYS_CPNT_ALARM_DETECT == TRUE
 */
BOOL_T SYS_HWCFG_GetMajorAlarmOutputRegInfo(UI32_T board_id, SYS_HWCFG_AlarmOutputRegInfo_T *info_p);

/* FUNCTION NAME: SYS_HWCFG_GetMinorAlarmOutputRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function provides the register information to control minor
 *          alarm output 
 *-----------------------------------------------------------------------------
 * INPUT    : board_id  -  which board id
 * OUTPUT   : info_p    -  the register info for minor alarm output control
 * RETURN   : TRUE if get sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: This API is used when SYS_CPNT_ALARM_DETECT == TRUE
 */
BOOL_T SYS_HWCFG_GetMinorAlarmOutputRegInfo(UI32_T board_id, SYS_HWCFG_AlarmOutputRegInfo_T *info_p);

/* FUNCTION NAME: SYS_HWCFG_GetMajorAlarmOutputLedRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function provides the register information to control major
 *          alarm led
 *-----------------------------------------------------------------------------
 * INPUT    : board_id  -  which board id
 * OUTPUT   : info_p    -  the register info for major alarm led
 * RETURN   : TRUE if get sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: This API is used when SYS_CPNT_ALARM_DETECT == TRUE
 */
BOOL_T SYS_HWCFG_GetMajorAlarmOutputLedRegInfo(UI32_T board_id, SYS_HWCFG_AlarmOutputLedRegInfo_T *info_p);

/* FUNCTION NAME: SYS_HWCFG_GetMinorAlarmOutputLedRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function provides the register information to control minor
 *          alarm led
 *-----------------------------------------------------------------------------
 * INPUT    : board_id  -  which board id
 * OUTPUT   : info_p    -  the register info for minor alarm led
 * RETURN   : TRUE if get sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: This API is used when SYS_CPNT_ALARM_DETECT == TRUE
 */
BOOL_T SYS_HWCFG_GetMinorAlarmOutputLedRegInfo(UI32_T board_id, SYS_HWCFG_AlarmOutputLedRegInfo_T *info_p);

/* FUNCTION NAME: SYS_HWCFG_GetThermalRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function provides the register information to access thermal
 *          register
 *-----------------------------------------------------------------------------
 * INPUT    : board_id    -  which board id
 *            thermal_idx -  which thermal (idx starts from 0)
 * OUTPUT   : info_p      -  the register info for thermal
 * RETURN   : TRUE if get sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: This API is used when
 *        (SYS_CPNT_THERMAL_DETECT == TRUE)
 */
BOOL_T SYS_HWCFG_GetThermalRegInfo(UI32_T board_id, UI32_T thermal_idx, SYS_HWCFG_ThermalRegInfo_T* info_p);

#endif    /* End of SYS_HWCFG_COMMON_H */

