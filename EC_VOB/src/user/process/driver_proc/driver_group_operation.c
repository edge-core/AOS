/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysdrv_init.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "driver_group.h"
#include "driver_proc_comm.h"
#include "dev_amtrdrv.h"
#include "dev_swdrv.h"
#include "sys_time_init.h"

#if (SYS_CPNT_NICDRV == TRUE)
#include "lan.h"
#include "dev_nicdrv_om.h"
#endif
#if (SYS_CPNT_ISCDRV == TRUE)
#include "isc_init.h"
#include "isc_agent_init.h"
#endif
#if (SYS_CPNT_AMTRDRV == TRUE)
#include "amtrdrv_mgr.h"    
#endif

#if (SYS_CPNT_NMTRDRV == TRUE)
#include "nmtrdrv.h"    
#endif

#if (SYS_CPNT_FLASHDRV == TRUE)
#include "fs_init.h"
#endif

#if (SYS_CPNT_SWDRV == TRUE)
#include "swdrv_init.h"
#endif
#if (SYS_CPNT_SWDRVL3 == TRUE)
#include "swdrvl3_init.h"
#include "swdrvl3.h"
#endif
#if (SYS_CPNT_SWDRVL4 == TRUE)
#include "swdrvl4.h"
#endif
#if (SYS_CPNT_COS == TRUE)
#include "cos_om.h"
#endif
#if (SYS_CPNT_QOSV2 == TRUE)
#include "rule_ctrl.h"
#include "rule_om.h"
#endif
#if (SYS_CPNT_LEDDRV == TRUE)
#include "leddrv_init.h"
#endif
#include "rule_ctrl.h"
#if (SYS_CPNT_POE == TRUE)
#include "poedrv_init.h"
#endif
#if (SYS_CPNT_SYNCE == TRUE)
#include "syncedrv.h"
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_NICDRV == TRUE)
    LAN_SetTransitionMode();
    DEV_NICDRV_OM_SetTransitionMode();
#endif
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_SetTransitionMode();
    ISC_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_INIT_SetTransitionMode();	
#endif
#if (SYS_CPNT_SYS_TIME == TRUE)
    SYS_TIME_INIT_SetTransitionMode();	
#endif
#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_SWDRVL4 == TRUE)
    SWDRVL4_SetTransitionMode();
#endif
#if (SYS_CPNT_FLASHDRV == TRUE)
    FLASHDRV_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_MGR_SetTransitionMode();
#endif
#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_SetTransitionMode();
#endif
#if (SYS_CPNT_COS == TRUE)
    COS_OM_SetTransitionMode();
#endif
#if (SYS_CPNT_QOS_V2 == TRUE)
    RULE_CTRL_SetTransitionMode();
#endif
#if (SYS_CPNT_LEDDRV == TRUE)
    LEDDRV_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    SYNCEDRV_SetTransitionMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_NICDRV == TRUE)
    LAN_EnterTransitionMode();
    DEV_NICDRV_OM_EnterTransitionMode();
#endif
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_EnterTransitionMode();
    ISC_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_INIT_EnterTransitionMode();	
#endif

#if (SYS_CPNT_SYS_TIME == TRUE)
     SYS_TIME_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_SWDRVL4 == TRUE)
    SWDRVL4_EnterTransitionMode();
#endif

#if (SYS_CPNT_FLASHDRV == TRUE)
    FLASHDRV_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_MGR_EnterTransitionMode();
#endif
#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_EnterTransitionMode();
#endif
#if (SYS_CPNT_COS == TRUE)
    COS_OM_EnterTransitionMode();
#endif
#if (SYS_CPNT_QOS_V2 == TRUE)
    RULE_CTRL_EnterTransitionMode();
#endif
#if (SYS_CPNT_LEDDRV == TRUE)
    LEDDRV_INIT_EnterTransitionMode();
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_EnterTransitionMode();
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    SYNCEDRV_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_NICDRV == TRUE)
    LAN_EnterMasterMode();
    DEV_NICDRV_OM_EnterMasterMode();
#endif
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_EnterMasterMode();
    ISC_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_INIT_EnterMasterMode();	
#endif
#if (SYS_CPNT_SYS_TIME == TRUE)
    SYS_TIME_INIT_EnterMasterMode();	
#endif
#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_SWDRVL4 == TRUE)
    SWDRVL4_EnterMasterMode();
#endif
#if (SYS_CPNT_FLASHDRV == TRUE)
    FLASHDRV_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_MGR_EnterMasterMode();
#endif
#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_EnterMasterMode();
#endif
#if (SYS_CPNT_COS == TRUE)
    COS_OM_EnterMasterMode();
#endif
#if (SYS_CPNT_QOS_V2 == TRUE)
    RULE_CTRL_EnterMasterMode();
#endif
#if (SYS_CPNT_LEDDRV == TRUE)
    LEDDRV_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    SYNCEDRV_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_NICDRV == TRUE)
    LAN_EnterSlaveMode();
    DEV_NICDRV_OM_EnterSlaveMode();
#endif
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_EnterSlaveMode();
    ISC_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_INIT_EnterSlaveMode();	
#endif
#if (SYS_CPNT_SYS_TIME == TRUE)
    SYS_TIME_INIT_EnterSlaveMode();	
#endif
#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_SWDRVL4 == TRUE)
    SWDRVL4_EnterSlaveMode();
#endif
#if (SYS_CPNT_FLASHDRV == TRUE)
    FLASHDRV_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_MGR_EnterSlaveMode();
#endif
#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_EnterSlaveMode();
#endif
#if (SYS_CPNT_COS == TRUE)
    COS_OM_EnterSlaveMode();
#endif
#if (SYS_CPNT_QOS_V2 == TRUE)
    RULE_CTRL_EnterSlaveMode();
#endif
#if (SYS_CPNT_LEDDRV == TRUE)
    LEDDRV_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    SYNCEDRV_EnterSlaverMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set provision complete mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_ProvisionComplete(UI32_T unit)
{
#if (SYS_CPNT_ISCDRV == TRUE)
	ISC_INIT_ProvisionComplete();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_ProvisionComplete();
#endif
#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_INIT_ProvisionComplete();	
#endif
#if (SYS_CPNT_SYS_TIME == TRUE)
     SYS_TIME_INIT_ProvisionComplete(unit);	
#endif
#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_MGR_ProvisionComplete();
#endif
#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_ProvisionComplete();
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_ProvisionComplete();
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    SYNCEDRV_ProvisionComplete();
#endif
}


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

void DRIVER_GROUP_HandleHotInsertion(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port)
{
#if (SYS_CPNT_ISCDRV == TRUE)
   ISC_INIT_HandleHotInsertion();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_HandleHotInsertion(unit_id, starting_port_ifindex, number_of_port);
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_HandleHotInsertion();
#endif
}

void DRIVER_GROUP_HandleHotRemoval(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port)
{
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_INIT_HandleHotRemoval();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_HandleHotRemoval(unit_id, starting_port_ifindex, number_of_port);
#endif
#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_HandleHotRemoval();
#endif
}
#endif

