/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_conntrol.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    PoE mirop/driver Communication interface
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    ??/??/???? - ???, Created
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


#ifndef _POEDRV_CONTROL_H
#define _POEDRV_CONTROL_H

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
 
typedef struct POEDRV_CONTROL_S {
    char     *drv_name;
    BOOL_T   (*poedrv_init)(void);
    BOOL_T   (*poedrv_open_uart)(void);
    BOOL_T   (*poedrv_upgrade_image)(const UI8_T *image, UI32_T image_size);
    BOOL_T   (*poedrv_upgrade_image_command)(UI8_T sub_command);
    BOOL_T   (*poedrv_get_port_admin_status)(UI32_T port, UI32_T *admin_status);
    BOOL_T   (*poedrv_set_port_admin_status)(UI32_T port, UI32_T admin_status);
    BOOL_T   (*poedrv_set_port_allow_power_state)(UI32_T port, UI32_T state);
    BOOL_T   (*poedrv_enable_logical_port_map)(UI32_T port_map[], UI32_T total_port_num);
    BOOL_T   (*poedrv_disable_logical_port_map)(void);
    BOOL_T   (*poedrv_reset_port)(UI32_T port);
    BOOL_T   (*poedrv_get_port_detection_type)(UI32_T port, UI32_T *type);
    BOOL_T   (*poedrv_set_port_detection_type)(UI32_T port, UI32_T type);
    BOOL_T   (*poedrv_set_port_classification_type)(UI32_T port, UI32_T type);
    BOOL_T   (*poedrv_set_port_auto_mode)(UI32_T port, UI32_T mode);
    // BOOL_T   (*poedrv_set_port_disconnect_type)(UI32_T port, UI32_T type);
    BOOL_T   (*poedrv_set_port_power_threshold_type)(UI32_T port, UI32_T type);
    BOOL_T   (*poedrv_get_port_power_limit)(UI32_T port, UI32_T *power_limit);
    BOOL_T   (*poedrv_set_port_power_limit)(UI32_T port, UI32_T power_limit);
    BOOL_T   (*poedrv_get_port_power_pairs)(UI32_T port, UI32_T *power_pairs);
    BOOL_T   (*poedrv_set_port_power_pairs)(UI32_T port, UI32_T power_pairs);
    BOOL_T   (*poedrv_get_port_priority)(UI32_T port, UI32_T *priority);
    BOOL_T   (*poedrv_set_port_priority)(UI32_T port, UI32_T priority);
    BOOL_T   (*poedrv_set_port_force_high_power_mode)(UI32_T port, UI32_T mode);
    BOOL_T   (*poedrv_set_port_dot3at_high_power_mode)(UI32_T port, UI32_T mode);
    BOOL_T   (*poedrv_set_current_in_high_power)(UI32_T mode);
    BOOL_T   (*poedrv_set_power_management_mode)(UI32_T power_mode);
    BOOL_T   (*poedrv_set_power_source_control)(UI32_T power_limit, UI32_T guard_band);
    BOOL_T   (*poedrv_get_soft_ver)(UI8_T *version);
    BOOL_T   (*poedrv_get_port_status2)(UI32_T port, UI8_T *status1, UI8_T *status2);
    BOOL_T   (*poedrv_get_total_allocated_power)(UI32_T *total_power);
    BOOL_T   (*poedrv_get_port_power_consumption)(UI32_T port, UI32_T *milliwatt);
    BOOL_T   (*poedrv_get_port_measurement)(UI32_T port, UI32_T *used_pwr, I32_T *tmp, UI32_T *volt, UI32_T *cur);
    BOOL_T   (*poedrv_reset_port_statistic)(UI32_T port);
    BOOL_T   (*poedrv_reset_all_port_statistic)(UI32_T port_num);
    BOOL_T   (*poedrv_get_port_mpsabsent_counter)(UI32_T port, UI32_T *counters);
    BOOL_T   (*poedrv_get_port_invalid_sign_counter)(UI32_T port, UI32_T *counters);
    BOOL_T   (*poedrv_get_port_power_deny_counter)(UI32_T port, UI32_T *counters);
    BOOL_T   (*poedrv_get_port_overload_counter)(UI32_T port, UI32_T *counters);
    BOOL_T   (*poedrv_get_port_short_counter)(UI32_T port, UI32_T *counters);
    BOOL_T   (*poedrv_get_port_all_counter)(UI32_T port, UI8_T counters[]); /* ArrayLen:POEDRV_MAX_COUNTER_TYPE */
    BOOL_T   (*poedrv_get_port_all_config)(UI32_T port, void *cfg);
    BOOL_T   (*poedrv_show_port_info)(UI32_T port); /* for backdoor */
    BOOL_T   (*poedrv_show_mainpower_info)(void);   /* for backdoor */
    BOOL_T   (*poedrv_send_raw_packet)(UI8_T *transmit, UI8_T *receive);  /* for backdoor */
} POEDRV_CONTROL_T;


#define	POEDRV_EXEC(func,ret, arg...)    \
    {                                        \
        if (func!=NULL) ret=func(arg);  \
        else ret = FALSE;                   \
    }

BOOL_T POEDRV_Control_Hook(POEDRV_CONTROL_T **poedrv_pointer);

#endif
