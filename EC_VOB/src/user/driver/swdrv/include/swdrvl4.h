/*------------------------------------------------------------------------------
 * File_Name : swdrvl4.h
 *
 * Purpose   : Provide physical port APIs to SWCTRL
 *
 * 2002/10/7    : Jeff Kao Create
 *
 * Copyright(C)      Accton Corporation, 2002, 2003
 *
 * Note    : Designed for Mercury
 *------------------------------------------------------------------------------
 */

#ifndef _SWDRVL4_H
#define _SWDRVL4_H

/*------------------------------------------------------------------------------
 * INCLUDE FILES
 *------------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"

#if(SYS_CPNT_STACKING==TRUE)
#include "l_mm.h"
#include "isc.h"
#endif


#define SWDRVL4_INGRESS_PRE_TO_DSCP_MAPPING_ENTRY_NMB             8
#define SWDRVL4_INGRESS_DSCP_TO_DSCP_MAPPING_ENTRY_NMB           64

#define SWDRVL4_MIN_PRE_VAL                                       0
#define SWDRVL4_MAX_PRE_VAL                                       7

#define SWDRVL4_MIN_DSCP_VAL                                      0
#define SWDRVL4_MAX_DSCP_VAL                                     63

typedef enum {
    SWDRVL4_COS_MAPPING_MODE = 0,
    SWDRVL4_PRECEDENCE_MAPPING_MODE,
    SWDRVL4_DSCP_IF_MAPPING_MODE,
    SWDRVL4_UNVALID_IF_MAPPING_MODE
} swdrvl4_qos_if_trust_mode_t;

typedef enum {
    SWDRVL4_L2_MAPPING_MODE = 0,
    SWDRVL4_DSCP_MAPPING_MODE = 2,
} swdrvl4_qos_if_mapping_mode_t;

/*dscp to dscp */
typedef struct{
	UI32_T phb;
	UI32_T color;

}swdrvl4_internal_dscp_t;


typedef struct{
	swdrvl4_internal_dscp_t CURRENT_DSCP_TO_DSCP_MAPPING[SWDRVL4_INGRESS_DSCP_TO_DSCP_MAPPING_ENTRY_NMB];
}swdrvl4_per_port_dscp_dscp_t;


/*precedence to dscp */
typedef struct{
     swdrvl4_internal_dscp_t CURRENT_PRE_TO_DSCP_MAPPING[SWDRVL4_INGRESS_PRE_TO_DSCP_MAPPING_ENTRY_NMB];
}swdrvl4_per_port_pre_dscp_t;

/*------------------------------------------------------------------------------
 * EXPORTED FUNCTIONS
 *------------------------------------------------------------------------------
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_Init
 *------------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_Init(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterMasterMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterSlaveMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterSlaveMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterTransitionMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_SetTransitionMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           set transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_SetTransitionMode(void);

BOOL_T SWDRVL4_EnableTosCosMap();
BOOL_T SWDRVL4_DisableTosCosMap();
BOOL_T SWDRVL4_EnableDscpCosMap();
BOOL_T SWDRVL4_DisableDscpCosMap();
BOOL_T SWDRVL4_EnableTcpPortCosMap();
BOOL_T SWDRVL4_DisableTcpPortCosMap();

BOOL_T SWDRVL4_SetTosCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);
BOOL_T SWDRVL4_SetDscpCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);
BOOL_T SWDRVL4_SetTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);

BOOL_T SWDRVL4_DelTosCosMap(UI32_T unit, UI32_T port, UI32_T value);
BOOL_T SWDRVL4_DelDscpCosMap(UI32_T unit, UI32_T port, UI32_T value);
BOOL_T SWDRVL4_DelTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value);

#if(SYS_CPNT_STACKING==TRUE)
/*------------------------------------------------------------------------------
 * Function : SWDRVL4_ISC_Handler
 *------------------------------------------------------------------------------
 * Purpose  : This function is called by isc_agent to handle non Global-wised
 *            configurations.
 * INPUT    : *key            -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by isc_agent
 *------------------------------------------------------------------------------
 */
BOOL_T SWDRVL4_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif

/* FUNCTION NAME: SWDRVL4_AttachSystemResources
 *------------------------------------------------------------------------------
 * PURPOSE: init share memory semaphore
 *------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL4_AttachSystemResources(void) ;

/* FUNCTION NAME: SWDRVL4_GetShMemInfo
 *------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL4_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

BOOL_T SWDRVL4_SetCosTrustMode(UI32_T unit, UI32_T port, UI32_T mode);

BOOL_T SWDRVL4_SetPortListCosTrustMode(UI8_T *port_list, UI32_T mode);

BOOL_T SWDRVL4_SetQosIngCos2Dscp(UI32_T unit, UI32_T port,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetPortListQosIngCos2Dscp(UI8_T *port_list,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetQosIngPre2Dscp(UI32_T unit, UI32_T port,UI32_T dscp,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetPortListQosIngPre2Dscp(UI8_T *port_list,UI32_T dscp,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetQosIngDscp2Dscp(UI32_T unit, UI32_T port,UI32_T o_dscp,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetPortListQosIngDscp2Dscp(UI8_T *port_list,UI32_T o_dscp,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetQosIngDscp2Queue(UI32_T unit, UI32_T port,UI32_T phb,UI32_T queue);

BOOL_T SWDRVL4_SetPortListQosIngDscp2Queue(UI8_T *port_list,UI32_T phb,UI32_T queue);

BOOL_T SWDRVL4_SetQosIngDscp2Color(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetPortListQosIngDscp2Color(UI8_T *port_list,UI32_T phb,UI32_T color);

BOOL_T SWDRVL4_SetQosIngDscp2Cos(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi);

BOOL_T SWDRVL4_SetPortListQosIngDscp2Cos(UI8_T *port_list,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi);

#endif
