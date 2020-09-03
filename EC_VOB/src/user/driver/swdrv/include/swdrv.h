/* Module Name: SWDRV.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides switch driver interface.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port configuration, VLAN, port mirroring,
 *         trunking, spanning tree, IGMP, broadcast storm control, and
 *         port mapping.
 *        ( 3.  The domain would not be handled by this module. )
 *         But this module doesn't include MAC address manipulation and
 *         port statistics.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2001/6/1    Jimmy Lin   Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#ifndef _SWDRV_H
#define _SWDRV_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "swdrv_type.h"
#include "l_mm.h"
#include "isc.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define SWDRV_Create_Task_FunNo 0
#define SWDRV_Create_Task_ErrNo 0

/* Event happens when Slave notify Master to update port link status.
 */
#define SWDRV_EVENT_UPDATE_PORT_LINK_STATUS            BIT_0
#define SWDRV_EVENT_ENTER_TRANSITION_MODE              BIT_1
#define SWDRV_EVENT_UPDATE_PORT_SFP_PRESENT_STATUS    BIT_2

/* Constants for load balance modes of port trunking */
enum SWDRV_Trunk_Load_Balance_Mode_E
{
    SWDRV_TRUNK_MAC_SA    = DEV_SWDRV_TRUNK_MAC_SA,
    SWDRV_TRUNK_MAC_DA    = DEV_SWDRV_TRUNK_MAC_DA,
    SWDRV_TRUNK_MAC_SA_DA = DEV_SWDRV_TRUNK_MAC_SA_DA,
    SWDRV_TRUNK_IP_SA     = DEV_SWDRV_TRUNK_IP_SA,
    SWDRV_TRUNK_IP_DA     = DEV_SWDRV_TRUNK_IP_DA,
    SWDRV_TRUNK_IP_SA_DA  = DEV_SWDRV_TRUNK_IP_SA_DA
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/****************************************************************************/
/* Switch Initialization */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_Init(void);

void SWDRV_HotSwapInsert(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

void SWDRV_HotSwapremove(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CreateTask
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create Switch Driver Task. This function
 *           will be called by root.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CreateTask(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SWDRV into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_SetTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will initialize the Switch Driver module and
 *           free all resource to enter transition mode while stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_EnterTransitionMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Switch Driver module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function must be invoked first before
 *           STA_Enter_Master_Mode() is called.
 * -------------------------------------------------------------------------*/
void SWDRV_EnterMasterMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the Switch Driver services and
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_EnterSlaveMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Switch Driver module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_ProvisionComplete(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetSwdrvProvisionCompleteStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will let all the Switch Driver module (about swdrvl4, swdrvl3) to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetSwdrvProvisionCompleteStatus(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_HotSwapInsert_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a hot swap mudule is
 *           inserted the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port)
 *           For loosely coulping, a module hot swapped will not cause
 *           topology change.  So this function can be used at loosely
 *           coupling architecture.
 *           But for tightly coupling, we won't use this function.
 * -------------------------------------------------------------------------*/
void SWDRV_Register_HotSwapInsert_CallBack(void (*fun)(UI32_T unit,
                                                       UI32_T port));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_HotSwapRemove_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a hot swap mudule is
 *           removed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port)
 *           For loosely coulping, a module hot swapped will not cause
 *           topology change.  So this function can be used at loosely
 *           coupling architecture.
 *           But for tightly coupling, we won't use this function.
 * -------------------------------------------------------------------------*/
void SWDRV_Register_HotSwapRemove_CallBack(void (*fun)(UI32_T unit,
                                                       UI32_T port));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_PortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link down to up the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port)
 * -------------------------------------------------------------------------*/
void SWDRV_Register_PortLinkUp_CallBack(void (*fun)(UI32_T unit,
                                                    UI32_T port));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_PortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link up to down the
 *           register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port)
 * -------------------------------------------------------------------------*/
void SWDRV_Register_PortLinkDown_CallBack(void (*fun)(UI32_T unit,
                                                      UI32_T port));



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_PortTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port type changed
 *           register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port, UI32_T port_type)
 * -------------------------------------------------------------------------*/
void SWDRV_Register_PortTypeChanged_CallBack(void (*fun)(UI32_T unit,
                                                         UI32_T port,
                                                         UI32_T port_type));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_PortSpeedDuplex_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the speed or duplex of a
 *           port is changed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port, UI32_T speed_duplex)
 *           Whenever speed or duplex changes, SWDRV needs to notify SWCTRL
 *           and SWCTRL needs to notify
 *           1. LED_MGMT
 *           2. STA
 *           3. IML(RFC2233)
 * -------------------------------------------------------------------------*/
void SWDRV_Register_PortSpeedDuplex_CallBack(void (*fun)(UI32_T unit,
                                                         UI32_T port,
                                                         UI32_T speed_duplex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Register_PortFlowCtrl_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the flow control of a
 *           port is changed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port, UI32_T flow_control)
 * -------------------------------------------------------------------------*/
void SWDRV_Register_PortFlowCtrl_CallBack(void (*fun)(UI32_T unit,
                                                      UI32_T port,
                                                      UI32_T flow_control));

/* -------------------------------------------------------------------------
 * Function : SWDRV_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of SWDRV via ISC
 * INPUT    : *key      -- key of ISC
 *            *mem_ref  -- transfer data
 *            svc_id    -- service id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_Agent
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

/****************************************************************************/
/* Port Configuration                                                       */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable a specified port
 * INPUT   : unit -- in which unit
 *           port -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortAdmin(UI32_T unit,
                             UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable a specified port
 * INPUT   : unit -- in which unit
 *           port -- which port to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortAdmin(UI32_T unit,
                              UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableAllPortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable all ports
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : only for swctrl enter trasition mode
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableAllPortAdmin(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortCfgSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set speed/duplex configuration of a port
 * INPUT   : unit         -- in which unit
 *           port         -- which port to set
 *           speed_duplex -- speed/duplex to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Forced mode only
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortCfgSpeedDuplex(UI32_T unit,
                                   UI32_T port,
                                   UI32_T speed_duplex);

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPort1000BaseTForceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set master/slave mode configuration of a forced speed port
 * INPUT   : unit         -- in which unit
 *           port         -- which port to set
 *           force_mode   -- master/slave mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Forced mode only
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPort1000BaseTForceMode(UI32_T unit,
                                       UI32_T port,
                                       UI32_T force_mode);
#endif

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetCopperEnergyDetect
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function get the copper energy detect status.
 * INPUT   : unit_id                -- in which unit
 *           port                   -- which port to set
 *           copper_energy_detect   -- on/off mode for a given force 1000T port
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If a given (unit, port) is not available, or it can not support
 *                                 the specified speed/duplex mode
 * NOTE    : 1. This function is applied for 1000T "Forced" master/slave mode only.
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_GetCopperEnergyDetect(UI32_T unit,
                                   UI32_T port,
                                   UI32_T *copper_energy_detect);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortAutoNeg
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable auto-negotiation of a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortAutoNeg(UI32_T unit,
                               UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortAutoNeg
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable auto-negotiation of a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortAutoNeg(UI32_T unit,
                                UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortCfgFlowCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the flow control function of a port
 * INPUT   : unit -- in which unit
 *           port -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Flow control mode will be auto determined by duplex.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortCfgFlowCtrl(UI32_T unit,
                                   UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortCfgFlowCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the flow control function of a port
 * INPUT   : unit -- in which unit
 *           port -- which port to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortCfgFlowCtrl(UI32_T unit,
                                    UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortCfgFlowCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the flow control function of a port
 * INPUT   : unit -- in which unit
 *           port -- which port to disable
 *           mode -- VAL_portFlowCtrlCfg_enabled: Turn on both TX and RX pause function
 *                   VAL_portFlowCtrlCfg_disabled: Turn off both TX and RX pause function
 *                   VAL_portFlowCtrlCfg_tx: Turn on TX and turn off RX pause function.
 *                   VAL_portFlowCtrlCfg_rx: Turn on RX and turn off TX pause function.
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortCfgFlowCtrl(UI32_T unit, UI32_T port, UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set auto-negotiation capability of a port
 * INPUT   : unit       -- in which unit
 *           port       -- which port to set
 *           capability -- port auto-negotiation capability
 *           (refer to enum SWDRV_Port_Capability_E)
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Flow control capability bit always depends on flow control mode.
 *           ie. If flow control is enabled, when enabing auto negotiation,
 *               flow control capability bit needs to be set on as well.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortAutoNegCapability(UI32_T unit,
                                      UI32_T port,
                                      UI32_T capability);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetStackInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the stacking feature for the stacking
 *           port
 * INPUT   : stacking_port               -- local user port
 *           total_stacking_device_count -- the chip number in stacking
 *           stack_id                    -- base is 0
 *           simplex                     -- TRUE is simplex
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : After local discovery, STK_TPLG need to call this API to set
 *           stack port.
 * -------------------------------------------------------------------------*/
#if (SYS_CPNT_STACKING == TRUE)
BOOL_T SWDRV_SetStackInfo(UI32_T stacking_port,
                          UI32_T total_stacking_device_count,
                          UI32_T stack_id,
                          BOOL_T simplex);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set spanning tree state of specified port in
 *           certain VLAN
 * INPUT   : vid   -- on which VLAN
 *           unit  -- in which unit
 *           port  -- which port to set
 *           state -- the spanning tree state
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : For single STA, vid is DONT CARE
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortSTAState(UI32_T vid,
                             UI32_T unit,
                             UI32_T port,
                             UI32_T state);

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableMultipleSTA
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable Multiple spanning tree
 * INPUT   : unit -- in which unit
 *           port -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableMultipleSTA(UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableMultipleSTA
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable Multiple spanning tree
 * INPUT   : unit -- in which unit
 *           port -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableMultipleSTA(UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddVlanToMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function adds a VLAN to a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. This function SHALL ONLY be supported/invoked for those ASICs which have
 *              "Multiple Spanning Instance" concept in their design. "mstidx" will be used
 *              to identify a unique "Spanning Instance" in the system.
 *           2. For those ASICs which have "Multiple Spanning Instance" concept, this following
 *              set of service routines shall be invoked for Multiple Spanning Tree configuration:
 *                  SWDRV_SetPortStateWithMstidx();
 *                  SWDRV_AddVlanToMst();
 *                  SWDRV_DeleteVlanFromMst();
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddVlanToMst(UI32_T vid, UI32_T mstidx);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeleteVlanFromMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function deletes a VLAN from a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. This function SHALL ONLY be supported/invoked for those ASICs which have
 *              "Multiple Spanning Instance" concept in their design. "mstidx" will be used
 *              to identify a unique "Spanning Instance" in the system.
 *           2. For those ASICs which have "Multiple Spanning Instance" concept, this following
 *              set of service routines shall be invoked for Multiple Spanning Tree configuration:
 *                  SWDRV_SetPortStateWithMstidx();
 *                  SWDRV_AddVlanToMst();
 *                  SWDRV_DeleteVlanFromMst();
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeleteVlanFromMst(UI32_T vid, UI32_T mstidx);

#endif /* (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP) */

/*
-------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortXstpState
 *
-------------------------------------------------------------------------------------------
 * PURPOSE : This function sets the Port Spanning Tree State of the specified port.
 * INPUT   : mstid              -- Multi-Spanning Tree ID
 *           vlan_count         -- total vlan count
 *           vlan_list          -- the vlan list (UI16_T array)
 *           unit_id            -- which unit
 *           port               -- which port to set
 *           state              -- the port spanning tree state
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If a given port is not available
 * NOTE    : 1. In multiple spanning tree (MSTP) mode,
 *              a) In multiple spanning tree environment, each port will have a specific
 *                 state on a given spanning tree.
 *              b) Most of ASIC uses the per VLAN port state to implement the Port Spanning Tree State.
 *              c) Since a spanning tree is running over a certain VLANs, we need to configure the
 *                 all "per VLAN port states" to the same port state.
 *              d) The vlan_count and vlan_list shows the vlan groups that a given spanning tree
 *                 is running.
 *           2. In single spanning tree (SSTP) mode, the vlan_count and vlan_list shall be ignored.
 *              The calling route shall set vlan_count = 0, and vlan_list = NULL.
 *           3. The enumerated value for port spanning tree state is defined in file, "leaf_1493.h".
 *                  #define VAL_dot1dStpPortState_disabled
 *                  #define VAL_dot1dStpPortState_blocking
 *                  #define VAL_dot1dStpPortState_listening
 *                  #define VAL_dot1dStpPortState_learning
 *                  #define VAL_dot1dStpPortState_forwarding
 *
-------------------------------------------------------------------------------------------
 *//* ArthurWu, 12/10/2002 02:29¤U¤È*/
BOOL_T SWDRV_SetPortXstpState(  UI32_T mstid,
                                UI32_T vlan_count,
                                UI16_T *vlan_list,
                                UI32_T unit_id,
                                UI32_T port,
                                UI32_T state);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortStateWithMstidx
 * -------------------------------------------------------------------------
  * PURPOSE : This function sets the Port Spanning Tree State of the specified port with
 *           "Multiple Spanning Tree Index" associated.
 * INPUT   : mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 *           unit_id            -- which unit
 *           port               -- which port to set
 *           state              -- the port spanning tree state
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. This function SHALL ONLY be supported/invoked for those ASICs which have
 *              "Multiple Spanning Instance" concept in their design. "mstidx" will be used
 *              to identify a unique "Spanning Instance" in the system.
 *           2. For those ASICs which have "Multiple Spanning Instance" concept, this following
 *              set of service routines shall be invoked for Multiple Spanning Tree configuration:
 *                  SWDRV_SetPortStateWithMstidx();
 *                  SWDRV_AddVlanToMst();
 *                  SWDRV_DeleteVlanFromMst();
 *           2. In multiple spanning tree (MSTP) mode, each port will have a specific
 *              state on a given spanning tree.
 *           3. In single spanning tree (SSTP) mode, the "mstidx" SHALL be ignored.
 *           4. The enumerated value for port spanning tree state is defined in file, "leaf_1493.h".
 *                  #define VAL_dot1dStpPortState_disabled
 *                  #define VAL_dot1dStpPortState_blocking
 *                  #define VAL_dot1dStpPortState_listening
 *                  #define VAL_dot1dStpPortState_learning
 *                  #define VAL_dot1dStpPortState_forwarding
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortStateWithMstidx(UI32_T mstidx, UI32_T unit_id, UI32_T port, UI32_T state);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortType
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the port type
 * INPUT   : unit   -- which unit
 *           port   -- which port
 * OUTPUT  : port_type -- port type to return
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : Nonly for Led_drv
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortType(UI32_T unit, UI32_T port, UI32_T *port_type);

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_GetPortLoopbackTestResult
 * ------------------------------------------------------------------------
 * PURPOSE  : SWCTRL use this API to get loopback test result.
 * INPUT    : None.
 * OUTPUT   : test_result --- The loopback test result in the format of port bit map.
 *                            If loopback test fail, the bit is "1", otherwise "0".
 *                            The MSB of byte 0 is port 1,
 *                            the LSB of byte 0 is port 8,
 *                            the MSB of byte 1 is port 9,
 *                            the LSB of byte 1 is port 16,
 *                            ...
 *                            and so on.
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_GetPortLoopbackTestResult(UI8_T test_result[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortTypeList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get whole system port types.
 * INPUT   : None
 * OUTPUT  : port_type_list -- port types of whole system
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : 1. The number of port_type_list is
 *              SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT.
 *           2. used it before provision-complete only.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortTypeList(UI32_T *port_type_list);

/****************************************************************************/
/* VLAN                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortPVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set default VLAN ID of a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 *           pvid -- permanent VID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortPVID(UI32_T unit,
                         UI32_T port,
                         UI32_T pvid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CreateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a specified VLAN
 * INPUT   : vid -- which VLAN to create
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_CreateVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DestroyVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DestroyVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetGlobalDefaultVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function changes the global default VLAN
 * INPUT   : vid                -- the vid of the new default VLAN
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If the specified VLAN is not available.
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetGlobalDefaultVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddPortToVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a port to the member set of a specified
 *           VLAN
 * INPUT   : unit -- which unit to add
 *           port -- which port to add
 *           vid  -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddPortToVlanMemberSet(UI32_T unit,
                                    UI32_T port,
                                    UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeletePortFromVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the member set of a
 *           specified VLAN
 * INPUT   : unit -- which unit to delete
 *           port -- which port to delete
 *           vid  -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeletePortFromVlanMemberSet(UI32_T unit,
                                         UI32_T port,
                                         UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddPortToVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set a port to output untagged frames over
 *           the specified VLAN
 * INPUT   : unit -- which unit to add
 *           port -- which port to add
 *           vid  -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddPortToVlanUntaggedSet(UI32_T unit,
                                      UI32_T port,
                                      UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeletePortFromVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the untagged set of a
 *           specified VLAN
 * INPUT   : unit -- which unit to add
 *           port -- which port to add
 *           vid  -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Delete a port from untagged set means to recover this port to be
 *           a tagged member set of specified vlan.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeletePortFromVlanUntaggedSet(UI32_T unit,
                                           UI32_T port,
                                           UI32_T vid);


BOOL_T SWDRV_SetSystemMTU(UI32_T status,UI32_T mtu);
BOOL_T SWDRV_SetPortMTU(UI32_T unit,UI32_T port,UI32_T MTU);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_GetPortMaxFrameSize
 * -------------------------------------------------------------------------
 * PURPOSE : to get max frame size of port
 * INPUT   : unit
 *           port
 * OUTPUT  : untagged_max_frame_sz_p - max frame size for untagged frames
 *           tagged_max_frame_sz_p   - max frame size for tagged frames
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_GetPortMaxFrameSize(UI32_T unit, UI32_T port, UI32_T *untagged_max_frame_sz_p, UI32_T *tagged_max_frame_sz_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable ingress filter of a port
 * INPUT   : unit -- which unit to enable
 *           port -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableIngressFilter(UI32_T unit,
                                 UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable ingress filter of a port
 * INPUT   : unit -- which unit to disable
 *           port -- which port to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableIngressFilter(UI32_T unit,
                                  UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AdmitVLANTaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow tagged frames entering a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AdmitVLANTaggedFramesOnly(UI32_T unit,
                                       UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AdmitVLANUntaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow untagged frames entering a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AdmitVLANUntaggedFramesOnly(UI32_T unit,
                                         UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AdmitAllFrames

 * -------------------------------------------------------------------------
 * FUNCTION: This function will allow all kinds of frames entering a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AdmitAllFrames(UI32_T unit,
                            UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableTrapUnspecifiedTagFrame

 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable trap unspecified tag frame
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : If chip receives a tagged frame which is not registered in the
 *           system, chip will trap this packet to CPU.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableTrapUnspecifiedTagFrame(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableTrapUnspecifiedTagFrame

 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable trap unspecified tag frame
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableTrapUnspecifiedTagFrame(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddHostToVlan

 * -------------------------------------------------------------------------
 * FUNCTION: This function will add CPU to a specified VLAN
 * INPUT   : vid -- which VLAN to add
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : This is for host to join vlan.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddHostToVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeleteHostFromVlan

 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete CPU from a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeleteHostFromVlan(UI32_T vid);




/****************************************************************************/
/* Port Mirroring                                                           */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortMirroring
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set source port to mirror target port
 * INPUT   : from_port  -- which port to mirror
 *           rx_to_port -- which port mirrors the received packets
 *           tx_to_port -- which port mirrors the transmitted packets
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : If rx_to_port set 0, the rx of from_port will not mirror.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortMirroring(SYS_TYPE_Uport_T from_port,
                              SYS_TYPE_Uport_T rx_to_port,
                              SYS_TYPE_Uport_T tx_to_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeletePortMirroring
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a set of port mirroring
 * INPUT   : from_port  -- as index to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeletePortMirroring(SYS_TYPE_Uport_T from_port , SYS_TYPE_Uport_T to_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortMirroring
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the mirroring function
 * INPUT   : from_port -- as index to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortMirroring(SYS_TYPE_Uport_T from_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortMirroring
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the mirroring function
 * INPUT   : from_port -- as index to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortMirroring(SYS_TYPE_Uport_T from_port);

/****************************************************************************/
/* VLAN Mirroring                                                           */
/****************************************************************************/
#if (SYS_CPNT_VLAN_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will add the vlan mirror and destination port
 * INPUT   : unit -- which unit to set
 *           port -- which destination port to set
 *           vid  -- which vlan-id add to source mirrored table
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T SWDRV_AddVlanMirror(UI32_T unit, UI32_T port, UI32_T vid);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeleteVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will delete the vlan mirror and destination port
 * INPUT   : unit -- which unit to set
 *           port -- which destination port to set
 *           vid  -- which vlan-id add to source mirrored table
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be removed when source vlan mirror has empty
 *------------------------------------------------------------------------*/
BOOL_T SWDRV_DeleteVlanMirror(UI32_T unit, UI32_T port, UI32_T vid);
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets MAC based MIRROR
 * INPUT   : mac_address        -- MAC address
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMacMirrorEntry(UI8_T *mac_address);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeleteMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE : This function deletes MAC based MIRROR
 * INPUT   : mac_address        -- MAC address
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeleteMacMirrorEntry(UI8_T *mac_address);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDestPortForMacMirror
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets destination port for MAC based MIRROR
 * INPUT   : unit -- in which unit
 *           port -- which port to monitor
 *           mode -- TRUE: set, FALSE: remove
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDestPortForMacMirror(UI32_T unit, UI32_T port, BOOL_T mode);

#endif /* #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

#if (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDestPortForAclMirror
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets destination port for ACL based MIRROR
 * INPUT   : unit -- in which unit
 *           port -- which port to monitor
 *           mode -- TRUE: set, FALSE: remove
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDestPortForAclMirror(UI32_T unit, UI32_T port, BOOL_T mode);
#endif /* (SYS_CPNT_ACL_MIRROR == TRUE) */

/****************************************************************************/
/* Trunking                                                                 */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunk id to create
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_CreateTrunk(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DestroyTrunk(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetTrunkPorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add members to a trunking port
 * INPUT   : trunk_id       -- which trunking port to add member
 *           port_count     -- how many members
 *           port_list      -- the member list
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : We will set the lowest port as the broadcast_port.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetTrunkPorts(UI32_T trunk_id,
                           UI32_T port_count,
                           SYS_TYPE_Uport_T *port_member);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetTrunkInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the trunk memeber information
 * INPUT   : ifindex               -- which interface index
 * OUTPUT  : trunk_port_ext_info   -- trunk information
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : only for Led_drv
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetTrunkInfo(UI32_T ifindex,SWDRV_Trunk_Info_T *trunk_port_info);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetTrunkBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the mode of trunking
 * INPUT   : mode -- Egressport selection criterion
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : the egressport is determinded by following criteria :
 *           SWDRV_TRUNK_MAC_SA      Determinded by source mac address
 *           SWDRV_TRUNK_MAC_DA      Determinded by destination mac address
 *           SWDRV_TRUNK_MAC_SA_DA   Determinded by source and destination mac address
 *           SWDRV_TRUNK_IP_SA       Determinded by source IP address
 *           SWDRV_TRUNK_IP_DA       Determinded by destination IP address
 *           SWDRV_TRUNK_IP_SA_DA    Determinded by source and destination IP address
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetTrunkBalanceMode(UI32_T mode);



/****************************************************************************/
/* IGMP Functions                                                           */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable IGMP packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableIgmpTrap(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable IGMP packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableIgmpTrap(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddMulticastAddrToTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will force a multicast address always go specified
 *           trunk member port while doing load balance.
 * INPUT   : mac          -- the multicast address
 *           vid          -- which VLAN
 *           trunk_id     -- join to which trunk
 *           trunk_member -- which port to add
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : (unit, port) is trunk port member
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddMulticastAddrToTrunkMember(UI8_T *mac,
                                           UI32_T vid,
                                           UI32_T trunk_id,
                                           SYS_TYPE_Uport_T trunk_member);

/*******************************************************************************
 *  SWDRV_SetUnknownIPMcastFwdPortList
 *
 * Purpose: Set the unknown multicast packet forwarding-to port list.
 * Inputs:
 *   unit           - which unit to set
 *   port_list      - on which the multicast packets allow to forward-to
 * Outputs:
 * Return:
 *   TRUE/FALSE
 * Note: For those ASIC can not support this function, this service shall be
 *       ignored.
 *       In StrataSwitch, the API will impact all kinds of packets (L2/L3).
 *******************************************************************************
 */
BOOL_T SWDRV_SetUnknownIPMcastFwdPortList(UI32_T unit, UI8_T port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);


/****************************************************************************/
/* Broadcast/Multicast Storm Control                                        */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetBroadcastStormControlThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the threshold of broadcast
 *           storm control of a port
 * INPUT   : unit      -- which unit to set
 *           port      -- which port to set
 *           threshold -- control threshold
 *           mode      -- storm mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : threshold : 0 - 17bit packets/sec
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetBroadcastStormControlThreshold(UI32_T unit,
                                               UI32_T port,
                                               UI32_T threshold,
                                               UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMulticastStormControlThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the threshold of multicast
 *           storm control of a port
 * INPUT   : unit      -- which unit to set
 *           port      -- which port to set
 *           threshold -- control threshold
 *           mode      -- storm mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : threshold : 0 - 17bit packets/sec
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMulticastStormControlThreshold(UI32_T unit,
                                               UI32_T port,
                                               UI32_T threshold,
                                               UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetUnknownUnicastStormControlThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the threshold of multicast
 *           storm control of a port
 * INPUT   : unit      -- which unit to set
 *           port      -- which port to set
 *           threshold -- control threshold
 *           mode      -- storm mode, one of
 *                        DEV_SWDRV_STORM_CTRL_MODE_PACKET_RATE
 *                        DEV_SWDRV_STORM_CTRL_MODE_BYTE_RATE
 *                        DEV_SWDRV_STORM_CTRL_MODE_PERCENTAGE
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : threshold : 0 - 17bit packets/sec
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetUnknownUnicastStormControlThreshold(UI32_T unit,
                                               UI32_T port,
                                               UI32_T threshold,
                                               UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableBroadcastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable broadcast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableBroadcastStormControl(UI32_T unit,
                                         UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableBroadcastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable broadcast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableBroadcastStormControl(UI32_T unit,
                                          UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableMulticastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable multicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableMulticastStormControl(UI32_T unit,
                                         UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableMulticastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable multicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableMulticastStormControl(UI32_T unit,
                                          UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableUnknownUnicastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable unknown unicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableUnknownUnicastStormControl(UI32_T unit,
                                              UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableUnknownUnicastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable unknown unicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableUnknownUnicastStormControl(UI32_T unit,
                                               UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortStormGranularity
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get granularity of a port
 * INPUT   : unit       -- which unit to set
 *           port       -- which port to set
 *           mode       -- storm mode, one of
 *                         VAL_bcastStormSampleType_pkt_rate
 *                         VAL_bcastStormSampleType_octet_rate
 *                         VAL_bcastStormSampleType_percent
 * OUTPUT  : granularit -- granularity of a port
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortStormGranularity(UI32_T unit, UI32_T port,
                                     UI32_T mode, UI32_T *granularity);
/****************************************************************************/
/* ATC Broadcast/Multicast Storm Control                                        */
/****************************************************************************/
#if (SYS_CPNT_ATC_BSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetATCBroadcastStormControlThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the threshold of auto traffic control broadcast
 *           storm control of a port
 * INPUT   : unit      -- which unit to set
 *           port      -- which port to set
 *           threshold -- control threshold
 *           mode      -- storm mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : threshold : 0 - 17bit packets/sec
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetATCBroadcastStormControlThreshold(UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T threshold,
                                                  UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableATCBroadcastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable auto traffic control broadcast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableATCBroadcastStormControl(UI32_T unit,
                                            UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableATCBroadcastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable auto traffic control broadcast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableATCBroadcastStormControl(UI32_T unit,
                                             UI32_T port);

#endif



#if (SYS_CPNT_ATC_MSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetATCMulticastStormControlThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the threshold of auto traffic control multicast
 *           storm control of a port
 * INPUT   : unit      -- which unit to set
 *           port      -- which port to set
 *           threshold -- control threshold
 *           mode      -- storm mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : threshold : 0 - 17bit packets/sec
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetATCMulticastStormControlThreshold(UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T threshold,
                                                  UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableATCMulticastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable auto traffic control multicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableATCMulticastStormControl(UI32_T unit,
                                            UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableATCMulticastStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable auto traffic control multicast storm control function of
 *           a port
 * INPUT   : unit -- which unit to set
 *           port -- which port to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableATCMulticastStormControl(UI32_T unit,
                                             UI32_T port);
#endif






/****************************************************************************/
/* Quality of Service                                                       */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortUserDefaultPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set user default priority
 * INPUT   : unit     -- which unit to set
 *           port     -- which port to set
 *           priority -- user default priority to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortUserDefaultPriority(UI32_T unit,
                                        UI32_T port,
                                        UI32_T priority);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of 10/100 ports
 * INPUT   : mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPriorityMapping(UI32_T unit,
                                UI32_T port,
                                UI8_T mapping[8]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPriorityMappingPerSystem
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping
 * INPUT   : dot1p_to_cos_mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : It's always support per-system setting.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPriorityMappingPerSystem(UI8_T *dot1p_to_cos_mapping);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetStackingPortPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the priority mapping of stacking port
 * INPUT   : mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
#if (SYS_CPNT_STACKING == TRUE)
BOOL_T SWDRV_SetStackingPortPriorityMapping(UI32_T unit,
                                            UI32_T port,
                                            UI8_T mapping[8]);
#endif


/****************************************************************************/
/* Prio Queue                                                          */
/****************************************************************************/
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetEgressSchedulingMethod
 *------------------------------------------------------------------------------
 * FUNCTION: This function will set port egress scheduling mothod
 * INPUT   : method   -- strict / wrr / drr / strict-drr / strict-wrr
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetEgressSchedulingMethod(UI32_T method);
#else
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortEgressSchedulingMethod
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set egress scheduling mothod
 * INPUT   : unit     -- which unit to set
 *           port     -- which port to set
 *           mothod   -- strict / wrr
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortEgressSchedulingMethod(UI32_T unit,
                                       UI32_T port,
                                       UI32_T method);
#endif

#if 0
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortWrrQueueWeight
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the WRR queue weight
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortWrrQueueWeight(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortWrrQueueWeight
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the WRR queue weight function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortWrrQueueWeight(void);
#endif

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == FALSE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetWrrQueueWeight
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the WRR queue weight
 * INPUT   : q_id -- in which queue
 *           weight -- the weight value to be set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetWrrQueueWeight(UI32_T q_id, UI32_T weight);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetStrictPriorityQueue
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the strict priority queue
 * INPUT   : q_id -- in which queue
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetStrictPriorityQueue(UI32_T q_id);
#else
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortWrrQueueWeight
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set WRR queue weight of 10/100/1000 ports
 * INPUT   : unit     -- which unit to set
 *           port     -- which port to set
 *           q_id     -- which queue to set
 *           weight   -- which weight to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortWrrQueueWeight(UI32_T unit,
                                   UI32_T port,
                                   UI32_T q_id,
                                   UI32_T weight);
#endif


#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/****************************
 * Traffic Segmatation APIs *
 ****************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableTrafficSegmatation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the traffic segmatation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableTrafficSegmatation();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableTrafficSegmatation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the traffic segmatation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableTrafficSegmatation();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetTrafficSegmatation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the traffic segmatation
 * INPUT   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetTrafficSegmatation( SYS_TYPE_Uport_T    *uplink_uport_list,
                                    UI32_T              uplink_uport_list_cnt,
                                    SYS_TYPE_Uport_T    *downlink_uport_list,
                                    UI32_T              downlink_uport_list_cnt
                                   );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_ResetTrafficSegmatation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will reset the context of traffic segmatation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_ResetTrafficSegmatation();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetTrafficSegmatationByPortlist
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan
 * INPUT   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetTrafficSegmatationByPortlist(UI8_T *uplink_port_list,
                                             UI8_T *downlink_port_list);
#endif /* End of SYS_CPNT_PORT_TRAFFIC_SEGMENTATION is TRUE */

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPrivateVlanPortlistBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan by session group
 * INPUT   : session_id         -- session id to pvlan group
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPrivateVlanPortlistBySessionId(UI32_T session_id,
                                               UI8_T  *uplink_port_list,
                                               UI8_T  *downlink_port_list);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeletePrivateVlanPortlistBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete the private vlan by session group
 * INPUT   : session_id         -- session id to pvlan group
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeletePrivateVlanPortlistBySessionId(UI32_T session_id,
                                               UI8_T  *uplink_port_list,
                                               UI8_T  *downlink_port_list);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable blocking traffic of uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePrivateVlanUplinkToUplinkBlockingMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable blocking traffic of uplink ports
 *           so every traffic can be forwarding different uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePrivateVlanUplinkToUplinkBlockingMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPrivateVlanTrunkMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set status of private vlan of trunk id
 * INPUT   : session_id
 *           trunk_id
 *           is_uplink
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPrivateVlanTrunkMode(UI32_T session_id, UI32_T trunk_id, BOOL_T is_uplink);
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
/*******************
 * Rate Limit APIs *
 *******************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : unit, port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortIngressRateLimit(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port ingress rate limit
 * INPUT   : unit, port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortIngressRateLimit(UI32_T unit, UI32_T port, UI32_T trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port ingress rate limit
 * INPUT   : unit -- unit id
 *           port -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortIngressRateLimit(UI32_T unit, UI32_T port, UI32_T trunk_id, UI32_T rate);
#endif

#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
/*******************
 * Rate Limit APIs *
 *******************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port egress rate limit
 * INPUT   : unit -- unit id
 *           port -- u_port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortEgressRateLimit(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port egress rate limit
 * INPUT   : unit -- unit id
 *           port -- u_port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortEgressRateLimit(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port gress rate limit
 * INPUT   : unit -- unit id
 *           port -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortEgressRateLimit(UI32_T unit, UI32_T port, UI32_T rate);
#endif

#if (SYS_CPNT_JUMBO_FRAMES == TRUE)
/********************
 * Jumbo Frame APIs *
 ********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableJumboFrame
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the jumbo frame
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableJumboFrame();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableJumboFrame
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the jumbo frame
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableJumboFrame();
#endif

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set active medium type for combo port
 * INPUT   : unit           -- unit id
 *           port           -- which port to set
 *           forcedmode     -- forcedfiber / forcedcopper / autodetecfiber / autodeteccopper
 *           fiber_speed    -- which speed (VAL_portType_hundredBaseFX/VAL_portType_thousandBaseSfp)
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortComboForcedMode(UI32_T unit, UI32_T port, UI32_T forcedmode, UI32_T fiber_speed);
#else
BOOL_T SWDRV_SetPortComboForcedMode(UI32_T unit, UI32_T port, UI32_T forcedmode);
#endif
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_ShutdownSwitch
 *------------------------------------------------------------------------
 * FUNCTION: This function will shutdown the switch before warm start
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void SWDRV_ShutdownSwitch(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortSecurity
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable port security
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortSecurity(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortSecurity
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable port security
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortSecurity(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_PortSecurityActionNone
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable port security active status.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_PortSecurityActionNone(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_PortSecurityActionTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable port security active trap status.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_PortSecurityActionTrap(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_PortSecurityActionShutdown
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable port security active shutdown status.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_PortSecurityActionShutdown(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_PortSecurityActionTrapAndShutdown
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable port security active trap and shutdown status.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_PortSecurityActionTrapAndShutdown(UI32_T unit, UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableIPMC(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableIPMC(void);



/*-------------------------------------------------------------------------
 *                      Protocol Base VLAN API
 *------------------------------------------------------------------------*/

#define SWDRV_MAX_1V_PROTOCOL_VALUE_LENGTH 5

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_EnableUMCASTIpTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DisableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_DisableUMCASTIpTrap(void);

void SWDRV_SfpInserted(UI32_T unit, UI32_T sfp_index);
void SWDRV_SfpRemoved(UI32_T unit, UI32_T sfp_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_EnableUMCASTMacTrap(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DisableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_DisableUMCASTMacTrap(void);


#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortRestartAutoNego
 * -------------------------------------------------------------------------
 * FUNCTION: To triger PHY to re-start auto-nego.
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortRestartAutoNego (UI32_T unit, UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortAutoNegoRemoteFaultAdvertisement
 * -------------------------------------------------------------------------
 * FUNCTION: Set auto-nego remote fault advertisement bits.
 * INPUT   : unit         -- Which unit.
 *           port         -- Which port.
 *           remote_fault -- VAL_ifMauAutoNegRemoteFaultAdvertised_noError
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_offline
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_linkFailure
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_autoNegError
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortAutoNegoRemoteFaultAdvertisement (UI32_T unit, UI32_T port, UI32_T remote_fault);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortAutoNegoRemoteFaultAdvertisement
 * -------------------------------------------------------------------------
 * FUNCTION: Set auto-nego remote fault advertisement bits.
 * INPUT   : unit         -- Which unit.
 *           port         -- Which port.
 * OUTPUT  : remote_fault -- VAL_ifMauAutoNegRemoteFaultAdvertised_noError
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_offline
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_linkFailure
 *                           VAL_ifMauAutoNegRemoteFaultAdvertised_autoNegError
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortAutoNegoRemoteFaultAdvertisement (UI32_T unit, UI32_T port, UI32_T *remote_fault);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortLinkPartnerAutoNegoSignalingState
 * -------------------------------------------------------------------------
 * FUNCTION: Get the status if link partner in auto-nego signaling state.
 * INPUT   : unit  -- Which unit.
 *           port  -- Which port.
 * OUTPUT  : state -- VAL_ifMauAutoNegRemoteSignaling_detected
 *                    VAL_ifMauAutoNegRemoteSignaling_notdetected
 * RETURN  : TRUE/FALSE
 * NOTE    : If the link state is link-down, VAL_ifMauAutoNegRemoteSignaling_notdetected
 *           shall be outputed.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortLinkPartnerAutoNegoSignalingState (UI32_T unit, UI32_T port, UI32_T *state);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortAutoNegoProcessState
 * -------------------------------------------------------------------------
 * FUNCTION: Get the status of auto-nego process.
 * INPUT   : unit  -- Which unit.
 *           port  -- Which port.
 * OUTPUT  : state -- VAL_ifMauAutoNegConfig_other
 *                    VAL_ifMauAutoNegConfig_configuring
 *                    VAL_ifMauAutoNegConfig_complete
 *                    VAL_ifMauAutoNegConfig_disabled
 *                    VAL_ifMauAutoNegConfig_parallelDetectFail
 * RETURN  : TRUE/FALSE
 * NOTE    : In this API, check "auto-nego" enable/disable first, if disabled
 *           return, then check "parallel detect fail", if true, return.
 *           Finally check status is "configuring" or "complete".
 *           If link-down, VAL_ifMauAutoNegConfig_other is outputed.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortAutoNegoProcessState (UI32_T unit, UI32_T port, UI32_T *state);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortLinkPartnerAutoNegoCapa
 * -------------------------------------------------------------------------
 * FUNCTION: Get the auto-nego capability bits of link partner.
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : capabilities -- bitmap:
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_bOther      )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b10baseT    )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b10baseTFD  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b100baseT4  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b100baseTX  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b100baseTXFD)
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b100baseT2  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b100baseT2FD)
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_bFdxPause   )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_bFdxAPause  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_bFdxSPause  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_bFdxBPause  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b1000baseX  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b1000baseXFD)
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b1000baseT  )
 *                           (1 << VAL_ifMauAutoNegCapReceivedBits_b1000baseTFD)
 * RETURN  : TRUE/FALSE
 * NOTE    : If the link state is link-down, only (1 << VAL_ifMauAutoNegCapReceivedBits_bOther)
 *           shall be outputed.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortLinkPartnerAutoNegoCapa (UI32_T unit, UI32_T port, UI32_T *capabilities);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortLinkPartnerAutoNegoRemoteFault
 * -------------------------------------------------------------------------
 * FUNCTION: Get the auto-nego remote fault of link partner.
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : remote fault -- VAL_ifMauAutoNegRemoteFaultReceived_noError
 *                           VAL_ifMauAutoNegRemoteFaultReceived_offline
 *                           VAL_ifMauAutoNegRemoteFaultReceived_linkFailure
 *                           VAL_ifMauAutoNegRemoteFaultReceived_autoNegError
 *
 * RETURN  : TREU/FALSE
 * NOTE    : If the link state is link-down, VAL_ifMauAutoNegRemoteFaultReceived_noError
 *           shall be outputed.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortLinkPartnerAutoNegoRemoteFault (UI32_T unit, UI32_T port, UI32_T *remote_fault);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortJabberState
 * -------------------------------------------------------------------------
 * FUNCTION: Get the status if the port in jabbering state.
 * INPUT   : unit  -- Which unit.
 *           port  -- Which port.
 * OUTPUT  : state -- VAL_ifMauJabberState_other
 *                    VAL_ifMauJabberState_unknown
 *                    VAL_ifMauJabberState_noJabber
 *                    VAL_ifMauJabberState_jabbering
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortJabberState (UI32_T unit, UI32_T port, UI32_T *state);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortFalseCarrierSenseCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the counter of the false carrier sense.
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : cntr -- Just cunter.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortFalseCarrierSenseCounter (UI32_T unit, UI32_T port, UI32_T *cntr);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPDPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get the status of the PD port
 * INPUT   : unit  -- Which unit.
 *           port  -- Which port.
 *           status_p-- SWDRV_POWER_SOURCE_NONE
 *                      SWDRV_POWER_SOURCE_UP
 *                      SWDRV_POWER_SOURCE_DOWN
 *           mode_p  -- SWDRV_POWERED_DEVICE_MODE_NONE
 *                      SWDRV_POWERED_DEVICE_MODE_AF
 *                      SWDRV_POWERED_DEVICE_MODE_AT
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_GetPDPortStatus (UI32_T unit, UI32_T port, UI8_T *status_p, UI8_T *mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisablePortLearning
 * -------------------------------------------------------------------------
 * FUNCTION: Disable port learning function
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : none.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisablePortLearning (UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortLearning
 * -------------------------------------------------------------------------
 * FUNCTION: Enable port learning functione.
 * INPUT   : unit -- Which unit.
 *           port -- Which port.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortLearning (UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetPortLearningStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port learning status and NA handling
 *              behavior
 * INPUT    :   unit
 *              port
 *              learning
 *              to_cpu
 *              drop
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetPortLearningStatus(UI32_T unit, UI32_T port, BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetVlanLearningStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddMyMACIP2Cpu
 * -------------------------------------------------------------------------
 * FUNCTION: Add mgmt MAC address and mgmt ip address.
 * INPUT   : UI32_T ip     - ip address
 *           UI8_T *mac    - MAC address
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddMyMACIP2Cpu (UI8_T *mac, UI32_T ip);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_RemoveMyMACIP2Cpu
 * -------------------------------------------------------------------------
 * FUNCTION: Remove mgmt MAC address and mgmt ip address.
 * INPUT   : UI32_T ip     - ip address
 *           UI8_T *mac    - MAC address
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_RemoveMyMACIP2Cpu (UI8_T *mac, UI32_T ip);


#if (SYS_CPNT_OSPF == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable OSPF trap.
 * INPUT   : cpu_mac - CPU MAC address.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableOSPFTrap(UI8_T *cpu_mac);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to disable OSPF trap.
 * INPUT   : CPU MAC address.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableOSPFTrap(UI8_T *cpu_mac);
#endif

#if (SYS_CPNT_DOT1X == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDot1xAuthTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap EtherType 888E packets to CPU
 * INPUT   : unit, port,
 *           mode      -- DEV_SWDRV_DOT1X_PACKET_DISCARD = 0,
 *                        DEV_SWDRV_DOT1X_PACKET_FORWARD,
 *                        DEV_SWDRV_DOT1X_PACKET_TRAPTOCPU
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDot1xAuthTrap(UI32_T unit, UI32_T port, UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDot1xAuthControlMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set dot1x auth control mode
 * INPUT   : unit, port,
 *               mode
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDot1xAuthControlMode(UI32_T unit, UI32_T port, UI32_T mode);
#endif /* SYS_CPNT_DOT1X == TRUE */

BOOL_T SWDRV_SetPortToVlanMemberSet(UI32_T vid, UI8_T *port_list);
BOOL_T SWDRV_SetPortToVlanUntaggedSet(UI32_T vid, UI8_T *port_list);

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetCableDiag
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port by latest result
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetCableDiag(UI32_T unit, UI32_T port, SWDRV_CableDiagInfo_T *cable_diag_result) ;

#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetCableDiagResult
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port after test
 * INPUT   : lport : Logical port num
 * OUTPUT  : cable_diag_result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetCableDiagResult(UI32_T unit, UI32_T port, SWDRV_CableDiagInfo_T *cable_diag_result);
#endif
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableMldPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: Enable MLD packet trap to CPU
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableMldPacketTrap(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DisableMldPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: Disable MLD packet trap to CPU
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DisableMldPacketTrap(void);
#endif

#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetRateBasedStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set rate based storm control
 * INPUT   : unit
 *           port
 *           rate
 *           mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : Forced mode only
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetRateBasedStormControl(UI32_T unit, UI32_T port, UI32_T rate, UI32_T mode);
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetOamLoopback
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enable EFM OAM Loopback to CPU rules
 * INPUT   : unit  -- which unit.
 *           port  -- which port.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWDRV_SetOamLoopback(
    UI32_T unit, UI32_T port, BOOL_T is_enable, UI32_T type);
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetInternalLoopback
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enable EFM OAM Loopback to CPU rules
 * INPUT   : unit  -- which unit.
 *           port  -- which port.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWDRV_SetInternalLoopback(UI32_T unit, UI32_T port, BOOL_T is_enable);
#endif
void SWDRV_BD_ClearRxCounter();
void SWDRV_BD_ShowRxCounter();
void SWDRV_BD_DumpRxMaxTick();
void SWDRV_BD_ClearRxMaxTick();

void SWDRV_Notify_PortFlowCtrl(UI32_T unit, UI32_T port, UI32_T flow_ctrl);
void SWDRV_Notify_PortLinkStatus2Master();
void SWDRV_Notify_PortLinkStatus2UpperLayer();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Notify_PortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Notify upper layer when the sfp present status of a port
 *           is changed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_Notify_PortSfpPresent(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Notify_PortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Notify upper layer when the sfp eeprom  of a port is changed
 * INPUT   : unit
 *           sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_Notify_PortSfpInfo(UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Notify_PortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Notify upper layer when the sfp DDM eeprom of a port is changed
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_Notify_PortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_Notify_PortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Notify upper layer when the sfp DDM measured eeprom info
 *           of a port is changed
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_measured_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_Notify_PortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

void SWDRV_MonitorModule(UI32_T port);

void SWDRV_Notify_HotSwapInsert(UI32_T unit, UI32_T port);
void SWDRV_Notify_HotSwapRemove(UI32_T unit, UI32_T port);
void SWDRV_Notify_PortSpeedDuplex(UI32_T unit, UI32_T port, UI32_T speed_duplex);
void SWDRV_LocalUpdatePortLinkStatus(UI32_T port, UI8_T new_port_status);
void SWDRV_Notify_PortLinkUp(UI32_T unit, UI32_T port);
void SWDRV_Notify_PortLinkDown(UI32_T unit, UI32_T port);
void SWDRV_Notify_CraftPortLinkUp(UI32_T unit);
void SWDRV_Notify_CraftPortLinkDown(UI32_T unit);



#if (SYS_CPNT_REFINE_ISC_MSG == TRUE)

BOOL_T SWDRV_EnableIngressFilter_PortList(UI8_T *port_list);
BOOL_T SWDRV_DisableIngressFilter_PortList(UI8_T *port_list);
BOOL_T SWDRV_AddPortToVlanUntaggedSet_PortList(UI8_T *port_list,UI32_T vid);
BOOL_T SWDRV_AddPortToVlanMemberSet_PortList(UI8_T *port_list,UI32_T vid);
BOOL_T SWDRV_SetPortPVID_PortList(UI8_T *port_list,UI32_T pvid);
BOOL_T SWDRV_AdmitAllFrames_PortList(UI8_T *port_list);
BOOL_T SWDRV_SetPortListComboForcedMode(UI8_T *port_list,UI32_T forcedmode);
BOOL_T SWDRV_EnablePortListAdmin(UI8_T *port_list);
BOOL_T SWDRV_DisablePortListAdmin(UI8_T *port_list);
BOOL_T SWDRV_SetPortListAutoNegCapability(UI8_T *port_list,UI32_T capability);
BOOL_T SWDRV_DisablePortListSecurity(UI8_T *port_list);
BOOL_T SWDRV_PortListSecurityActionNone(UI8_T *port_list);
BOOL_T SWDRV_SetPortListMulticastStormControlThreshold(UI8_T *port_list,
                                               UI32_T threshold,
                                               UI32_T mode);
BOOL_T SWDRV_DisablePortListMulticastStormControl(UI8_T *port_list);
BOOL_T SWDRV_EnablePortListMulticastStormControl(UI8_T *port_list);
BOOL_T SWDRV_EnablePortListBroadcastStormControl(UI8_T *port_list);
BOOL_T SWDRV_DisablePortListBroadcastStormControl(UI8_T *port_list);
BOOL_T SWDRV_SetPortListBroadcastStormControlThreshold(UI8_T *port_list,UI32_T threshold,
                                               UI32_T mode);
BOOL_T SWDRV_SetPortListIngressRateLimit(UI8_T *port_list, UI32_T rate);
BOOL_T SWDRV_DisablePortListIngressRateLimit(UI8_T *port_list);
BOOL_T SWDRV_EnablePortListIngressRateLimit(UI8_T *port_list);

BOOL_T SWDRV_SetPortListEgressRateLimit(UI8_T *port_list,UI32_T rate);
BOOL_T SWDRV_DisablePortListEgressRateLimit(UI8_T *port_list);
BOOL_T SWDRV_EnablePortListEgressRateLimit(UI8_T *port_list);

BOOL_T SWDRV_SetPortListWrrQueueWeight(UI8_T *port_list, UI32_T q_id, UI32_T weight);

BOOL_T SWDRV_SetPortListEgressSchedulingMethod(UI8_T *port_list,UI32_T method);

#endif


#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMDIXMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set mdix mode
 * INPUT   : unit           -- unit id
 *           port           -- which port to set
 *           mode     -- automode enable or disable(normal/cross_over)
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_straight              2L
 *                       - VAL_portMdixMode_crossover             3L
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMDIXMode(UI32_T unit, UI32_T port, UI32_T mode);
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMacVlanEntry
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets MAC based VLAN
 * INPUT   : mac_address        -- MAC address
 *           vid                -- VLAN id
 *           priority           -- priority
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask, UI16_T vid, UI8_T priority);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_DeleteMacVlanEntry
 * -------------------------------------------------------------------------
 * PURPOSE : This function deletes MAC based VLAN
 * INPUT   : mac_address        -- MAC address
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T *mask);

#endif /* #if (SYS_CPNT_MAC_VLAN == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetEapolFramePassThrough
 * -------------------------------------------------------------------------------------------
 * PURPOSE : To set EAPOL frames pass through (pass through means not trapped to CPU)
 * INPUT   : state (TRUE/FALSE)
 * OUTPUT  : None
 * RETURN  : TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetEapolFramePassThrough(BOOL_T state);
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#if (SYS_CPNT_POWER_SAVE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortPowerSave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port power-save status
 * INPUT   : unit --which unit
 *               port -- which port to set
 *               status--TRUE:enable
 *                           FALSE:disable
 * OUTPUT  :
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortPowerSave(UI32_T unit, UI32_T port,BOOL_T status);
#endif /* end of #if (SYS_CPNT_POWER_SAVE == TRUE) */

#if (SYS_CPNT_RSPAN == TRUE)
/* -------------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetRspanVlanTag
 * -------------------------------------------------------------------------------------------------
 * PURPOSE : Set VLAN for egressing mirrored packets on a port. ( remote and local )
 * INPUT   : target_port   -- mirror-to port to set (-1 for all ports)
 *           tpid          -- tag protocol id (tpid and vid are equal 0, meaning to disable RSPAN)
 *           vlan          -- virtual lan number (tpid and vid are equal 0, meaning to disable RSPAN)
 * OUTPUT  : None
 * RETURN  : TRUE          -- Success
 *           FALSE         -- If the specified specific-port is not available, or the ASIC can
 *                            not support port mirroring function
 * NOTE    : For now RSPAN doesn't support stacking.
 * -------------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetRspanVlanTag (SYS_TYPE_Uport_T target_port, UI16_T tpid, UI16_T vlan);

/* -------------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_ModifyMaxFrameSize
 * -------------------------------------------------------------------------------------------------
 * FUNCTION: This function will modify maximum fram size of RSPAN
 * INPUT   : port -- in which port
 *           is_increase -- TRUE:add RSPAN tag size (4 bytes)
 *                       -- FALSE:delete RSPAN tag size (4 bytes)
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : For now RSPAN doesn't support stacking.
 * ------------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_ModifyMaxFrameSize (UI32_T unit, UI32_T port, BOOL_T is_increase);
#endif /*#if (SYS_CPNT_RSPAN == TRUE)*/

/****************************************************************************/
/* Trap traffic to cpu                                                      */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapUnknownIpMcastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ip multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapUnknownIpMcastToCPU(BOOL_T to_cpu, BOOL_T flood, UI32_T vid);

#if (SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapUnknownIpv6McastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ipv6 multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapUnknownIpv6McastToCPU(BOOL_T to_cpu, BOOL_T flood, UI32_T vid);
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapIpv6PIMToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap ipv6 PIM packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapIpv6PIMToCPU(BOOL_T to_cpu);
#endif

#if (SYS_CPNT_SFLOW == TRUE)
/*******************
 * sFlow APIs *
 *******************/
 /* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnableSystemSflow
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the system sFlow
 * INPUT   : enable-- TRUE:enable
 *                    FALSE:disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnableSystemSflow(BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_EnablePortSflow
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable specific port sFlow
 * INPUT   : port -- which port to enable/disable sFlow source
 *           enable-- TRUE:enable
 *                    FALSE:disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_EnablePortSflow(UI32_T unit, UI32_T port, BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortSflowRate
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set specific port  sFlow rate
 * INPUT   : port -- which port to set sFlow rate
 *           rate--sFlow rate
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortSflowRate(UI32_T unit, UI32_T port,UI32_T rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetSflowSampleCount
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the sFlow sample count
 * INPUT   : port --sflow source
 * OUTPUT  : count--sFlow sample counter
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetSflowSampleCount(UI32_T unit, UI32_T port,UI32_T *count);
#endif/*#if (SYS_CPNT_SFLOW == TRUE)*/

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DropIpv6MulticastData
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enable/disable ip multicast data drop
 *            for specified unit and port
 * INPUT   : unit      -- which unit.
 *           port      -- which port.
 *           enabled -- enable/disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_DropIpv6MulticastData(UI32_T unit, UI32_T port, BOOL_T enabled);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapCDP
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function enables/disables trap CDP PDU.
 * INPUT   : enable
 * OUTPUT  : None
 * RETURN  : TRUE   -- Success
 *           FALSE  -- Failed
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_TrapCDP(BOOL_T enable);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapPVST
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function enables/disables trap PVST PDU.
 * INPUT   : enable
 * OUTPUT  : None
 * RETURN  : TRUE   -- Success
 *           FALSE  -- Failed
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_TrapPVST(BOOL_T enable);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DropCdpPacketByPort
 * -------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           unit    -- unit
 *           port    -- port
 * OUTPUT  : None
 * RETURN  : TRUE   -- Success
 *           FALSE  -- Failed
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWDRV_DropCdpPacketByPort(
    BOOL_T enable,
    UI32_T unit,
    UI32_T port
);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DropPvstPacketByPort
 * -------------------------------------------------------------------------
 * PURPOSE : Drop PVST packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           unit    -- unit
 *           port    -- port
 * OUTPUT  : None
 * RETURN  : TRUE   -- Success
 *           FALSE  -- Failed
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWDRV_DropPvstPacketByPort(
    BOOL_T enable,
    UI32_T unit,
    UI32_T port
);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapDhcpServerPacket
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DHCP server packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapDhcpServerPacket(BOOL_T to_cpu, BOOL_T flood);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapDhcpClientPacket
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DHCP client packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapDhcpClientPacket(BOOL_T to_cpu, BOOL_T flood);

#if (SYS_CPNT_PPPOE_IA == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPPPoEDPktToCpu
 * -------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for specified unit/port.
 * INPUT  : unit      - unit to enable/disable
 *          port      - port to enable/disable
 *          is_enable - enable or disable the feature.
 * OUTPUT : None
 * RETURN : True: Successfully, False: Failed.
 * NOTE    : 1. for projects who can install rule on trunk's member ports.
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetPPPoEDPktToCpu(
    UI32_T  unit,
    UI32_T  port,
    BOOL_T  is_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPPPoEDPktToCpuPerSystem
 * -------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for entire system.
 * INPUT  : is_enable - enable or disable the feature.
 * OUTPUT : None
 * RETURN : True: Successfully, False: Failed.
 * NOTE   : 1. for projects who encounter problems to install rule on
 *             trunk's member ports.
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetPPPoEDPktToCpuPerSystem(
    BOOL_T  is_enable);

#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_DOS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDosProtectionFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           enable - TRUE to enable; FALSE to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDosProtectionFilter(UI32_T type, BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDosProtectionRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           rate   - rate in kbps. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetDosProtectionRateLimit(UI32_T type, UI32_T rate);
#endif /* (SYS_CPNT_DOS == TRUE) */

#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_DHCPV6 == TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapDhcp6ServerPacket
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DHCP6 server packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapDhcp6ServerPacket(BOOL_T to_cpu, BOOL_T flood);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_TrapDhcp6ClientPacket
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DHCP6 client packet to trap to CPU
 * INPUT   : unit -- which unit to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapDhcp6ClientPacket(BOOL_T to_cpu, BOOL_T flood);
#endif /* SYS_CPNT_DHCPV6SNP */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetOrgSpecificTrap
 * -------------------------------------------------------------------------
 * PURPOSE : Set how to trap organization specific packets.
 * INPUT   : to_cpu - whether packet is trapped to CPU
 *           flood  - whether packet is flooded
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetOrgSpecificTrap(BOOL_T to_cpu, BOOL_T flood);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPktTrapStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : pkt_type  - which packet to trap
 *           to_cpu    - trap to cpu or not
 *           drop      - drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPktTrapStatus(UI32_T unit, UI32_T port, UI32_T pkt_type, BOOL_T to_cpu, BOOL_T drop);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 * INPUT   : pse_check_status  --  TRUE :PSE check status enabled
 *                                 FALSE:PSE check status disabled
 * OUTPUT  : None
 * RETURN  : TRUE:Success, FALSE:Failed
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPSECheckStatus(BOOL_T pse_check_status);

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetRaAndRrPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/disable RA/RR packet trap to CPU
 * INPUT   : is_enabled - TRUE to enable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetRaAndRrPacketTrap(
    BOOL_T  is_enabled);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortRaAndRrPacketDrop
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/disable RA/RR packet drop by specified unit/port.
 * INPUT   : unit       - unit to enable/disable
 *           port       - port to enable/disable
 *         : is_enabled - TRUE to enable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortRaAndRrPacketDrop(UI32_T unit, UI32_T port, BOOL_T is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE) */

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortMacAddr
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function configure source MAC address of chip port.
 * INPUT   : unit
 *           port
 *           mac_addr
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetPortMacAddr(UI32_T unit, UI32_T port, UI8_T *mac_addr);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortPfcStatus
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function configure PFC status.
 * INPUT   : unit
 *           port
 *           rx_en      -- enable/disable PFC response
 *           tx_en      -- enable/disable PFC triggering
 *           pri_en_vec -- bitmap of enable status per priority
 *                         set bit to enable PFC; clear to disable.
 * OUTPUT  : None
 * RETURN  : TRUE  --  get next succuess
             FALSE --  don't exist the next chip id
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetPortPfcStatus(UI32_T unit, UI32_T port, BOOL_T rx_en, BOOL_T tx_en, UI16_T pri_en_vec);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_UpdatePfcPriMap
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function update PFC priority to queue mapping.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWDRV_UpdatePfcPriMap(void);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_ETS == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortCosGroupMapping
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets mapping between CoS Queue and CoS group
 * INPUT   : unit
 *           port
 *           cosq2group -- array of cos groups.
 *                         element 0 is cos group of cosq 0,
 *                         element 1 is cos group of cosq 1, ...
 *                         NULL to map all cos to single cos group
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortCosGroupMapping(
    UI32_T unit,
    UI32_T port,
    UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortCosGroupSchedulingMethod
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets scheduling method for CoS groups
 * INPUT   : unit
 *           port
 *           method  -- DEV_SWDRVL4_EgressSchedulingMethod_T
 *           weights -- weights for cos groups.
 *                      NULL if method is STRICT
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortCosGroupSchedulingMethod(
    UI32_T unit,
    UI32_T port,
    UI32_T method,
    UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);
#endif /* (SYS_CPNT_ETS == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetQcnCnmPriority
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets 802.1p priority of egress QCN CNM
 * INPUT   : pri -- 802.1p priority of egress QCN CNM
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_SetQcnCnmPriority(UI32_T pri);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortQcnCpq
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets CP Queue of the CoS Queue
 * INPUT   : unit
 *           port
 *           cosq -- CoS Queue
 *           cpq  -- CP Queue. 0xffffffff means to disable QCN
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortQcnCpq(
    UI32_T unit,
    UI32_T port,
    UI32_T cosq,
    UI32_T cpq);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortQcnEgrCnTagRemoval
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets removal of CN-Tag of egress pkts
 * INPUT   : unit
 *           port
 *           no_cntag_bitmap - bit 0 for pri 0, and so on.
 *                             set the bit to remove CN-tag of packets with the pri.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortQcnEgrCnTagRemoval(
    UI32_T unit,
    UI32_T port,
    UI8_T no_cntag_bitmap);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortQcnCpid
 *------------------------------------------------------------------------------
 * FUNCTION: This function gets CPID
 * INPUT   : unit
 *           port
 *           cosq -- CoS Queue
 *           cpid -- CPID
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortQcnCpid(
    UI32_T unit,
    UI32_T port,
    UI32_T cosq,
    UI8_T cpid[8]);
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMimService
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy a MiM service instance.
 * INPUT   : mim_p            -- MiM service instance info.
 *           is_valid         -- TRUE to create/update; FALSE to destroy.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMimService(SWDRV_MimServiceInfo_T *mim_p, BOOL_T is_valid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetMimServicePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add/delete member port to a MiM service instance.
 * INPUT   : mim_port_p       -- MiM port info.
 *           is_valid         -- TRUE to add; FALSE to delete.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetMimServicePort(SWDRV_MimPortInfo_T *mim_port_p, BOOL_T is_valid);

#if (SYS_CPNT_IAAS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetMimServicePortLearningStatusForStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set MiM port learning status
 *              for station move handling only
 * INPUT    :   learning
 *              to_cpu
 *              drop
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetMimServicePortLearningStatusForStationMove(BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);
#endif /* (SYS_CPNT_IAAS == TRUE) */
#endif /* (SYS_CPNT_MAC_IN_MAC == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To configure CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 *           rate      -- in pkt/s. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetCpuRateLimit(UI32_T pkt_type, UI32_T rate);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_GetPortAbility
 * -------------------------------------------------------------------------
 * PURPOSE : To get port abilities
 * INPUT   : unit
 *           port
 * OUTPUT  : ability_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_GetPortAbility(UI32_T unit, UI32_T port, SWDRV_PortAbility_T *ability_p);
#if (SYS_CPNT_VRRP == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_TrapVrrpTpCpu
 * -------------------------------------------------------------------------
 * PURPOSE : trap Vrrp message to cpu
 * INPUT   : is_trap - trap or not
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_TrapVrrpTpCpu(BOOL_T is_trap);
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_GbicTransceiverInserted
 * -------------------------------------------------------------------------
 * PURPOSE : To set port combo force mode
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_GbicTransceiverInserted(UI32_T unit, UI32_T sfp_index);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_GbicTransceiverRemoved
 * -------------------------------------------------------------------------
 * PURPOSE : To set port combo force mode
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWDRV_GbicTransceiverRemoved(UI32_T unit, UI32_T sfp_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : unit
 *           port
 *           egr_blk_uport_list - uport list to block.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortEgressBlock(
    UI32_T unit,
    UI32_T port,
    UI8_T egr_blk_uport_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnablePortOpenFlowMode
 * -------------------------------------------------------------------------
 * PURPOSE : This function will configure the following settings for
 *           specified port to meet OpenFlow requirements.
 *           1. Turn off flow control on each port.
 *           2. Disable MAC learning on new addresses and copy SLF packet
 *              to CPU.
 *           3. Disable MAC learning on station moves and copy SLF packet
 *              to CPU.
 *           4. Disable IPv4 Multicast
 *           5. Disable IPv6 Multicast
 *           6. Discard untagged traffic
 *           7. Filter traffic for a VLAN which is not a member of a port.
 * INPUT   : unit -- which unit
 *           port -- which port to enable OpenFlow mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_EnablePortOpenFlowMode(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetVlanFloodingForUknUC
 * -------------------------------------------------------------------------
 * PURPOSE : This function will drop and disable unknown UC/MC/BC flooding.
 *           1. Drop unicast packets in the VLAN that miss the L2 lookup and
 *              do not copy to CPU. Multicast packets must not be dropped.
 *           2. Block flooding of unknown unicast, unknown multicast and
 *              broadcast by default.
 * INPUT   : vid   -- which VLAN to disable flooding
 *           flood -- TRUE = flooding, FALSE = not flooding
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetVlanFloodingForUknUC(UI32_T vid, BOOL_T flood);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetVlanFloodingForUknUC
 * -------------------------------------------------------------------------
 * PURPOSE : This function will drop and disable unknown UC/MC/BC flooding.
 *           1. Drop unicast packets in the VLAN that miss the L2 lookup and
 *              do not copy to CPU. Multicast packets must not be dropped.
 *           2. Block flooding of unknown unicast, unknown multicast and
 *              broadcast by default.
 * INPUT   : vid   -- which VLAN to disable flooding
 *           flood -- TRUE = flooding, FALSE = not flooding
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetVlanFloodingForUknMC(UI32_T vid, BOOL_T flood);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_SetVlanFloodingForUknUC
 * -------------------------------------------------------------------------
 * PURPOSE : This function will drop and disable unknown UC/MC/BC flooding.
 *           1. Drop unicast packets in the VLAN that miss the L2 lookup and
 *              do not copy to CPU. Multicast packets must not be dropped.
 *           2. Block flooding of unknown unicast, unknown multicast and
 *              broadcast by default.
 * INPUT   : vid   -- which VLAN to disable flooding
 *           flood -- TRUE = flooding, FALSE = not flooding
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_SetVlanFloodingForBC(UI32_T vid, BOOL_T flood);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DeleteAllPortFromVlan
 * -------------------------------------------------------------------------
 * PURPOSE : Remove all ports from specified VLAN
 * INPUT   : vid -- which VLAN to remove all members
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_DeleteAllPortFromVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_StaticMacMovePktToCpu
 * -------------------------------------------------------------------------
 * PURPOSE : The operation for static MAC address port move event.
 * INPUT   : is_enable -- TRUE will trap packet to CPU. FASLE will not.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_StaticMacMovePktToCpu(UI32_T is_enable);

#if (SYS_CPNT_VXLAN == TRUE)

BOOL_T SWDRV_CreateVxlanAccessPort(
    UI32_T              vfi_id,
    UI32_T              l3_inf_id,
    UI32_T              unit,
    UI32_T              port,
    UI8_T               mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T              match_type,
    UI32_T              *vxlan_port_p);

BOOL_T SWDRV_CreateVxlanNetworkPort(
    UI32_T                      vfi_id,
    UI32_T              udp_port,
    BOOL_T              is_mc,
    L_INET_AddrIp_T     *l_vtep_p,
    L_INET_AddrIp_T     *r_vtep_p,
    BOOL_T                      is_ecmp,
    void                        *nh_hw_info,
    UI32_T              *vxlan_port_p);

BOOL_T SWDRV_DestroyVxlanPort(
    UI32_T  vfi_id,
    UI32_T  vxlan_port_id,
    BOOL_T  is_ecmp);

BOOL_T SWDRV_CreateVxlanNexthop(
    UI32_T              l3_inf_id,
    UI32_T              unit,
    UI32_T              port,
    UI8_T               mac[SYS_ADPT_MAC_ADDR_LEN],
    BOOL_T              is_mc,
    void                **nh_hw_info_pp);

BOOL_T SWDRV_DestroyVxlanNexthop(void *nh_hw_info);

BOOL_T SWDRV_AddVxlanEcmpNexthop(
    UI32_T                      vfi_id,
    UI32_T                      vxlan_port_id,
    void                        *nh_hw_info);

BOOL_T SWDRV_RemoveVxlanEcmpNexthop(
    UI32_T                      vfi_id,
    UI32_T                      vxlan_port_id,
    void                        *nh_hw_info);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetVxlanVpn
 * -------------------------------------------------------------------------
 * FUNCTION: To add/del VPN for VXLAN
 * INPUT   : vpn_info_p - key for add is vnid
 *                        key for del is vfi/bc_group
 *           is_add     - TRUE to add
 * OUTPUT  : vpn_info_p - vfi/bc_group for add
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetVxlanVpn(
    SWDRV_VxlanVpnInfo_T    *vpn_info_p,
    BOOL_T                  is_add);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_AddVtepIntoMcastGroup
 * -------------------------------------------------------------------------
 * FUNCTION: To add/del a vxlan port to mcast group.
 * INPUT   : bcast_group - bcast group to set
 *           vxlan_port  - vxlan port to set
 *           unit        - unit to set
 *           port        - port to set (trunk id if unit is 0)
 *           is_add      - TRUE to add
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_AddVtepIntoMcastGroup(
    UI32_T  bcast_group,
    UI32_T  vxlan_port,
    BOOL_T  is_add);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetVxlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable vxlan globally.
 * INPUT   : is_enable          - TRUE to enable
 *           is_random_src_port - TRUE to configure random src port also
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetVxlanStatus(
    BOOL_T  is_enable,
    BOOL_T  is_random_src_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetVxlanStatusPort
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable vxlan status for a port.
 * INPUT   : unit        - unit to set
 *           port        - port to set
 *           is_acc_port - TRUE if it's access port
 *           is_enable   - TRUE if enable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : not support trunk !!!
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetVxlanStatusPort(
    UI32_T  unit,
    UI32_T  port,
    BOOL_T  is_acc_port,
    BOOL_T  is_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetVxlanUdpPort
 * -------------------------------------------------------------------------
 * FUNCTION: To set udp dst port for vxlan globally.
 * INPUT   : udp_port - dst port to conifgure
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetVxlanUdpPort(UI32_T udp_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetVxlanPortLearning
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable MAC learning for VXLAN port.
 * INPUT   : vxlan_port_id -- Which VXLAN logical port.
 *           is_learning -- TRUE to enable MAC learning.
 *                          FALSE to disable MAC learning.
 * OUTPUT  : none.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetVxlanPortLearning(UI32_T vxlan_port_id, BOOL_T is_learning);

#endif /* #if (SYS_CPNT_VXLAN == TRUE) */

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: To bind the specified hash-selection block index for service
 * INPUT   : service - SWDRV_HASH_SEL_SERVICE_ECMP
 *                     SWDRV_HASH_SEL_SERVICE_TRUNK (Not implement yet)
 *           block_info_p - block info
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_BindHashSelForService(
    SWDRV_HashSelService_T service,
    SWDRV_HashSelBlockInfo_T *block_info_p
);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortStatusForLicense
 * -------------------------------------------------------------------------
 * FUNCTION: To set set port administration status
 * INPUT   : unit
 *           port
 *           status  - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortStatusForLicense(UI32_T unit, UI32_T port, BOOL_T status);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: To bind the specified hash-selection block index for service
 * INPUT   : service - SWDRV_HASH_SEL_SERVICE_ECMP
 *                     SWDRV_HASH_SEL_SERVICE_TRUNK (Not implement yet)
 *           block_info_p - block info
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_BindHashSelForService(
    SWDRV_HashSelService_T service,
    SWDRV_HashSelBlockInfo_T *block_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: To unbind the specified hash-selection block index for service
 * INPUT   : service - SWDRV_HASH_SEL_SERVICE_ECMP
 *                     SWDRV_HASH_SEL_SERVICE_TRUNK (Not implement yet)
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_UnBindHashSelForService(
    SWDRV_HashSelService_T service,
    UI8_T list_index);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetSwitchingMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set switching mode
 * INPUT   : unit
 *           port
 *           mode - VAL_swctrlSwitchModeSF
 *                  VAL_swctrlSwitchModeCT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetSwitchingMode(UI32_T unit, UI32_T port, UI32_T mode);
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortFec
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable FEC
 * INPUT   : unit
 *           port
 *           fec_mode - VAL_portFecMode_XXX
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortFec(UI32_T unit, UI32_T port, UI32_T fec_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_GetPortFecStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To get FEC status
 * INPUT   : unit
 *           port
 * OUTPUT  : fec_mode_p - VAL_portFecMode_XXX
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_GetPortFecStatus(UI32_T unit, UI32_T port, UI32_T *fec_mode_p);
#endif /* (SYS_CPNT_SWCTRL_FEC == TRUE) */

#if(SYS_CPNT_WRED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_RandomDetect
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port ecn marking percentage
 * INPUT    :   unit       - which unit
 *              port       - which port
 *              value_p    - what percantage
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   percentage =0 means disable
 * ------------------------------------------------------------------------
 */
BOOL_T SWDRV_RandomDetect(UI32_T unit, UI32_T port, SWDRV_RandomDetec_T *value_p);
#endif

#endif /* SWDRV_H */

