/* MODULE NAME:  sys_callback_om.h
 * PURPOSE:
 *     OM of SYS_CALLBACK
 *
 * NOTES:
 *
 * HISTORY
 *    7/4/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYS_CALLBACK_OM_H
#define SYS_CALLBACK_OM_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* defines the capacity of SYS_CALLBACK_OM_FailEntry_T.list[]
 */
#define SYS_CALLBACK_OM_CALLBACK_FAIL_MAX_NBR_OF_CSC_LIST 10

/* defines the overflow value of SYS_CALLBACK_OM_FailEntry_T.csc_list_counter
 */
#define SYS_CALLBACK_OM_CSC_LIST_OVERFLOW                 0xffffffff

enum
{
    SYS_CALLBACK_OM_KIND_VLAN,
    SYS_CALLBACK_OM_KIND_VLAN_MEMBER,
    SYS_CALLBACK_OM_KIND_GVRP_VLAN,
    SYS_CALLBACK_OM_KIND_GVRP_VLAN_MEMBER,
    SYS_CALLBACK_OM_KIND_L3_VLAN,
    SYS_CALLBACK_OM_KIND_L3_IF_OPER_STATUS,
    SYS_CALLBACK_OM_KIND_PORT_VLAN,
    SYS_CALLBACK_OM_KIND_PORT_VLAN_MODE,
    SYS_CALLBACK_OM_KIND_PORT_PVLAN_MODE,
    SYS_CALLBACK_OM_KIND_PVID,
    SYS_CALLBACK_OM_KIND_VLAN_NAME,
    SYS_CALLBACK_OM_KIND_PROTOCOL_VLAN,
    SYS_CALLBACK_OM_KIND_VLAN_MEMBER_TAG,
    SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE,
    SYS_CALLBACK_OM_KIND_XSTP_VERSION,
    SYS_CALLBACK_OM_KIND_XSTP_PORT_TOPO,
    SYS_CALLBACK_OM_MAX_KIND
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/******************************
 *  callback fail data types  *
 ******************************
 */

/* a cscgroup that receives callback messages owns a fail entry
 */
typedef struct SYS_CALLBACK_OM_FailEntry_S
{
    /* csc_list contains the list of the originator csc id of the callback event
     */
    UI32_T csc_list[SYS_CALLBACK_OM_CALLBACK_FAIL_MAX_NBR_OF_CSC_LIST]; /* csc id */
    UI32_T csc_list_counter;   /* The number of element in csc_list.
                                * The value will be SYS_CALLBACK_MGR_CSC_LIST_OVERFLOW
                                * if csc_list is overflow.
                                */
} SYS_CALLBACK_OM_FailEntry_T;

typedef struct
{
    BOOL_T  state;
    BOOL_T  change;
    BOOL_T  merge;
} SYS_CALLBACK_OM_Common_T;

typedef struct
{
    BOOL_T  vlan_state;
    BOOL_T  vlan_change;
    BOOL_T  vlan_merge;
    UI8_T   member_state[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   member_change[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   member_merge[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} SYS_CALLBACK_OM_Vlan_T;

typedef struct
{
    BOOL_T  vlan_destroy;
    BOOL_T  oper_status_change;
} SYS_CALLBACK_OM_L3Vlan_T;

typedef struct
{
    UI32_T  old_pvid;
    UI32_T  new_pvid;
} SYS_CALLBACK_OM_Pvid_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get current SYS_CALLBACK operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  current operating mode
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T SYS_CALLBACK_OM_GetOperatingMode(void);

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
void SYS_CALLBACK_OM_SetCmgrThreadId(UI32_T tid);
UI32_T SYS_CALLBACK_OM_GetCmgrThreadId();
BOOL_T SYS_CALLBACK_OM_GetAndResetNextChange(UI32_T kind, UI32_T *key_p,
        void *buffer_p);
#endif

#endif /* End of SYS_CALLBACK_OM_H */
