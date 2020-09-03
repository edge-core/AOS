/* MODULE NAME - CN_TYPE.H
 * PURPOSE : Provides the declarations for CN constants and data structures.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_TYPE_H
#define CN_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "leaf_2674p.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"

/* NAMING CONSTANT DECLARATIONS
 */

enum CN_TYPE_RETURN_CODE_E
{
    CN_TYPE_RETURN_OK,
    CN_TYPE_RETURN_ERROR,
    CN_TYPE_RETURN_ERROR_MASTER_MODE,
    CN_TYPE_RETURN_ERROR_GLOBAL_STATUS_RANGE,
    CN_TYPE_RETURN_ERROR_PRIORITY_RANGE,
    CN_TYPE_RETURN_ERROR_ACTIVE_RANGE,
    CN_TYPE_RETURN_ERROR_CNPV_EXISTENCE,
    CN_TYPE_RETURN_ERROR_CNPV_CAPACITY,
    CN_TYPE_RETURN_ERROR_MODE_RANGE,
    CN_TYPE_RETURN_ERROR_ALT_PRIORITY_RANGE,
    CN_TYPE_RETURN_ERROR_LPORT_RANGE,
    CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE,
    CN_TYPE_RETURN_ERROR_CP_INDEX_RANGE,
    CN_TYPE_RETURN_ERROR_CP_EXISTENCE,
    CN_TYPE_RETURN_ERROR_CP_CAPACITY,
    CN_TYPE_RETURN_ERROR_NULL_POINTER,
    CN_TYPE_RETURN_ERROR_PFC_CONFLICT,
    CN_TYPE_RETURN_ERROR_ETS_CONFLICT,
};

enum CN_TYPE_DEFENSE_MODE_E
{
    CN_TYPE_DEFENSE_MODE_DISABLED = VAL_cnPortPriAdminDefenseMode_disabled,
    CN_TYPE_DEFENSE_MODE_EDGE = VAL_cnPortPriAdminDefenseMode_edge,
    CN_TYPE_DEFENSE_MODE_INTERIOR = VAL_cnPortPriAdminDefenseMode_interior,
    CN_TYPE_DEFENSE_MODE_INTERIOR_READY = VAL_cnPortPriAdminDefenseMode_interiorReady,
    CN_TYPE_DEFENSE_MODE_AUTO = VAL_cnPortPriAdminDefenseMode_auto,
    CN_TYPE_DEFENSE_MODE_BY_GLOBAL = VAL_cnPortPriAdminDefenseMode_byGlobal,
};

#define CN_TYPE_MIN_PRIORITY            MIN_dot1dUserPriority
#define CN_TYPE_MAX_PRIORITY            MAX_dot1dUserPriority
#define CN_TYPE_MAX_NBR_OF_CNPV         (CN_TYPE_MAX_PRIORITY - CN_TYPE_MIN_PRIORITY)

#define CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL    0xffffffff
#define CN_TYPE_START_PRIORITY_FOR_SEARCHING    0xfffffff0

#define CN_TYPE_CPID_SIZE   8 /* octets */

#define CN_TYPE_GLOBAL_STATUS_ENABLE    VAL_cnGlobalAdminStatus_enabled
#define CN_TYPE_GLOBAL_STATUS_DISABLE   VAL_cnGlobalAdminStatus_disabled

#define CN_TYPE_DEFAULT_SET_POINT           26000   /* unit: octets */
#define CN_TYPE_DEFAULT_FEEDBACK_WEIGHT     3       /* the real weight is 2 */
#define CN_TYPE_DEFAULT_MIN_SAMEPLE_BASE    150000  /* unit: octets */

/* follow chip limitation: 1/4, 1/2, 1, 2, 4, 8, 16, 32 */
#define CN_TYPE_MIN_FEEDBACK_WEIGHT     0   /* the real weight is 1/4 */
#define CN_TYPE_MAX_FEEDBACK_WEIGHT     7   /* the real weight is 32 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T  admin_status;
    UI32_T  oper_status;
    UI32_T  cnm_tx_priority;
/*    UI64_T  total_discarded_frames;   marked until NMTR_POM is provided */
} CN_TYPE_GlobalEntry_T;

typedef struct
{
    UI32_T  cnpv;               /* index */
    UI32_T  defense_mode;
    UI32_T  admin_alt_priority;
    UI32_T  auto_alt_priority;
} CN_TYPE_CnpvEntry_T;

typedef struct
{
    UI32_T  cnpv;               /* index */
    UI32_T  lport;              /* index */
    UI32_T  admin_defense_mode;
    UI32_T  oper_defense_mode;
    UI32_T  admin_alt_priority;
    UI32_T  oper_alt_priority;
} CN_TYPE_PortCnpvEntry_T;

typedef struct
{
    UI32_T  lport;                              /* index */
    UI32_T  cp_index;                           /* index */
/*    UI8_T   cpid[CN_TYPE_CPID_SIZE];  marked until SWCTRL_POM is provided */
    UI8_T   queue;
    UI8_T   managed_cnpvs;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  set_point;
    UI32_T  feedback_weight;
    UI32_T  min_sample_base;
/*    UI64_T  discarded_frames;     marked until NMTR_POM is provided */
/*    UI64_T  transmitted_frames;   marked until NMTR_POM is provided */
/*    UI64_T  transmitted_cnms;     marked until NMTR_POM is provided */
} CN_TYPE_CpEntry_T;

typedef struct
{
    UI32_T  lport;
    UI8_T   old_cnpv_indicators;
    UI8_T   old_ready_indicators;
    UI8_T   new_cnpv_indicators;
    UI8_T   new_ready_indicators;
} CN_TYPE_RemoteChange_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif /* End of CN_TYPE_H */
