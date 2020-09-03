/*---------------------------------------------------------------------
 * File_Name : HRDRV.H
 *
 * Purpose   : 
 * 
 * Copyright(C)      Accton Corporation, 2002, 2003
 *
 * Note    : 
 *---------------------------------------------------------------------
 */
 
#ifndef _HRDRV_H
#define _HRDRV_H


#include "sys_type.h"
#include "sys_adpt.h"
#include "hrdrv_type.h"
#include "sys_cpnt.h"
#include "leaf_es3626a.h"
#include "dev_hrdrv.h"

#if (SYS_CPNT_STACKING == TRUE)
#include "l_mm.h"
#include "isc.h"
#endif /* SYS_CPNT_STACKING == TRUE */

typedef enum 
{   
    HRDRV_DO_IP_ACL       =   DEV_HRDRV_DO_IP_ACL,
    HRDRV_DO_MAC_ACL      =   DEV_HRDRV_DO_MAC_ACL,
	HRDRV_DO_IP_DIFFSERV  =   DEV_HRDRV_DO_IP_DIFFSERV,
	HRDRV_DO_MAC_DIFFSERV =   DEV_HRDRV_DO_MAC_DIFFSERV

               
} HRDRV_FunctionType_T ;

#define HRDRV_MIN_INDEX_OF_FUNC_TYPE  DEV_HRDRV_DO_IP_ACL
#define HRDRV_MAX_INDEX_OF_FUNC_TYPE  DEV_HRDRV_DO_MAC_DIFFSERV

typedef enum HRDRV_L4_CosMapType_E
{
    HRDRV_L4_MapTosToCos = 1,
    HRDRV_L4_MapDscpToCos,
    HRDRV_L4_MapTcpPortToCos,
    HRDRV_L4_MapUdpPortToCos
    
} HRDRV_L4_CosMapType_T;
     
#define HRDRV_L4_MAX_COS_MAP_TYPE   HRDRV_L4_MapUdpPortToCos
#define HRDRV_MIN_NBR_OF_DO_FUN     HRDRV_DO_COS_IP_ACL
#define HRDRV_MAX_NBR_OF_DO_FUN     HRDRV_DO_MAC_ACL

typedef enum 
{    
    HRDRV_L2_SMAC,
    HRDRV_L2_DMAC,
    HRDRV_L2_TAG_TYPE,
    HRDRV_L2_Dot1p_TAG_RPI,
    HRDRV_L2_Dot1p_TAG_VID, 
    HRDRV_L2_ETH2_ETHERTYPE,
    HRDRV_L3_IP_VER_LEN,
    HRDRV_L3_IP_PRECEDENCE,
    HRDRV_L3_IP_TOS,
    HRDRV_L3_IP_DSCP,
    HRDRV_L3_IP_PROTOCOL,
    HRDRV_L3_IP_SIP,
    HRDRV_L3_IP_DIP,
    HRDRV_L4_SPORT,
    HRDRV_L4_DPORT,
    HRDRV_L4_TCP_CONTROL
    
} HRDRV_PktField_T ; 

#define HRDRV_MIN_NBR_OF_PKT_FIELD  HRDRV_L2_TAG_TYPE
#define HRDRV_MAX_NBR_OF_PKT_FIELD  HRDRV_L4_TCP_CONTROL

#define HRRV_MASK_CAPABILITY  ((UI32_T)(0x1 << HRDRV_L2_TAG_TYPE) |\
                               (UI32_T)(0x1 << HRDRV_L2_Dot1p_TAG_RPI) |\
                               (UI32_T)(0x1 << HRDRV_L2_Dot1p_TAG_VID) |\
                               (UI32_T)(0x1 << HRDRV_L2_ETH2_ETHERTYPE)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_VER_LEN)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_PRECEDENCE)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_TOS)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_DSCP)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_PROTOCOL)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_SIP)|\
                               (UI32_T)(0x1 << HRDRV_L3_IP_DIP)|\
                               (UI32_T)(0x1 << HRDRV_L4_SPORT)|\
                               (UI32_T)(0x1 << HRDRV_L4_DPORT)|\
                               (UI32_T)(0x1 << HRDRV_L4_TCP_CONTROL)|\
                               (UI32_T)(0x1 << HRDRV_L2_SMAC)|\
                               (UI32_T)(0x1 << HRDRV_L2_DMAC))
    
    
/* out action means out of profile action used by diffserv*/                               
typedef enum 
{
    HRDRV_OUT_ACTION_TRAP_TO_CPU=   0x01,       /* trap to cpu */
    HRDRV_OUT_ACTION_DROP       =   0x02,       /* discard packet but copy packet to CPU */
    HRDRV_OUT_ACTION_SWITCH     =   0x04,       /* without drop packet                   */
    HRDRV_OUT_ACTION_REMARK_DSCP=   0x08,       /* change the dscp vaule                 */
    HRDRV_OUT_ACTION_DROP_COUNT =   0x10        /* drop packet count for out-profile     */
        
} HRDRV_OutProfileAction_T ;    


/* out action means in-profile action used by diffserv and cos marker acl*/         
typedef enum 
{    
    HRDRV_IN_ACTION_SWITCH         =   0x01,       /* without discard packet  */
    HRDRV_IN_ACTION_DROP           =   0x02,       /* discard packet          */
    HRDRV_IN_ACTION_TRAP_TO_CPU    =   0x04,       /* packet trap to CPU      */
    HRDRV_IN_ACTION_SET_PRI_2COS   =   0x08,       /* set 802.1p priority for COSQ assignment */
    HRDRV_IN_ACTION_PACKET_COUNT   =   0x10,       /* set record in-profile packet count      */
    HRDRV_IN_ACTION_REMARK_DSCP    =   0x20,
    HRDRV_IN_ACTION_REDIRCT_TO_PORT  = 0x40,
    HRDRV_IN_ACTION_CLASSIFICATION =   0x80,       
    HRDRV_IN_ACTION_VLAN_REWRITE   =   0x100,
    HRDRV_ACTION_RATE_LIMIT        =   0x200,
    HRDRV_IN_ACTION_REMARK_PRECEDENCE= 0x400,
    HRDRV_IN_ACTION_REMARK_DOT1P     = 0x800,
    HRDRV_IN_ACTION_NONE           =   0x1000
    
} HRDRV_InProfileAction_T ;

#define HRDRV_DEF_ROUTE_DROP_PORT 255

/***********************************************/
typedef enum 
{
    HRDRV_INBOUND, 
    HRDRV_OUTBOUND,
        
} HRDRV_Inout_T ;    

#define HRDRV_MAX_INDEX_OF_INOUT_TYPE  HRDRV_OUTBOUND
#define HRDRV_MIN_INDEX_OF_INOUT_TYPE  HRDRV_INBOUND

/***********************************************/
typedef enum 
{
    HRDRV_PKT_TYPE_DONT_CARE = VAL_aclMacAcePktformat_any,
    HRDRV_UNTAGED_802Eth2 = VAL_aclMacAcePktformat_untagged_Eth2,
    HRDRV_UNTAGED_802DOT3 = VAL_aclMacAcePktformat_untagged802Dot3, 
    HRDRV_TAGED_802Eth2   = VAL_aclMacAcePktformat_tagggedEth2,
    HRDRV_TAGED_802DOT3   =  VAL_aclMacAcePktformat_tagged802Dot3


} HRDRV_PktFormat_T ;    

#define HRDRV_PKT_FORMAT_MIN_VALUE HRDRV_PKT_TYPE_DONT_CARE
#define HRDRV_PKT_FORMAT_MAX_VALUE HRDRV_TAGED_802DOT3

/***********************************************/
typedef enum 
{
    HRDRV_IP_MASK, 
    HRDRV_MAC_MASK,
    HRDRV_MAX_MASK_TYPE
    
} HRDRV_MaskType_T ;    


/* opeation mode is called by storm 
*/
enum { HRDRV_ADMIN_MODE=1, HRDRV_OP_MODE};
    

#define HRDRV_MAX_INDEX_OF_MASK_TYPE  HRDRV_MAC_MASK
#define HRDRV_MIN_INDEX_OF_MASK_TYPE  HRDRV_IP_MASK
/***********************************************/

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - HRDRV_InitiateSystemResources
 * ---------------------------------------------------------------------
 * PURPOSE: Initial Semaphore
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  This is local function and only will be called by
 *          HRDRV_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
BOOL_T HRDRV_Initiate_System_Resources();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - HRDRV_Create_InterCSC_Relation
 * ---------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES :  None
 * ---------------------------------------------------------------------
 */
void HRDRV_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the HRDRV enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None
 * NOTE:     if initial not success, do not enter master mode
 *---------------------------------------------------------------------------
 */
void HRDRV_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the HRDRV enter the slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void HRDRV_EnterSlaveMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the HRDRV enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void HRDRV_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the HRDRV enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void HRDRV_EnterTransitionMode();





/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION:  to set the port spanning tree state to hrdrv 
 * INPUT   : unit, port, state
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1: if port is not in forwarding state, 
 *              set config.FIL_EN to disable the FFP function 
 *              it is called in lan.c to avoid receive any packet trap by FFP
 *           2. if port is in forwarding state, 
 *              set config.FIL_EN to enable the FFP function to trap traffic from this 
 *              port to cpu 
 *           3. this API only used in XGS chipset, because we can turn off FFP by port
 *           
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_SetPortSTAState(UI32_T unit, UI32_T port, UI32_T state);




/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_InitRe
 *---------------------------------------------------------------------------
 * PURPOSE:  before use a reousce entry , must init it first
 * INPUT:    None
 * OUTPUT:   re
 * RETURN:   None
 * NOTE:     
 *---------------------------------------------------------------------------
 */
void HRDRV_InitRe(HRDRV_TYPE_ResouceEntity_T *re);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyPktformat
 *---------------------------------------------------------------------------
 * PURPOSE:  to specified the packet format of a L4 rule
 * INPUT:    format
 * OUTPUT    re          
 * RETURN:   TRUE / FASLE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */   
BOOL_T HRDRV_QualifyPktformat(UI32_T format, HRDRV_TYPE_ResouceEntity_T *re);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Qualify_SMac
 *---------------------------------------------------------------------------
 * PURPOSE:  to specified source mac and source mac mask of a l4 rule
 * INPUT:    data --source mac
 *           mask
 * OUTPUT:   re.
 * RETURN:   TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */   
BOOL_T HRDRV_Qualify_SMac(UI8_T data[6], UI8_T mask[6], HRDRV_TYPE_ResouceEntity_T *re);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Qualify_DMac
 *---------------------------------------------------------------------------
 * PURPOSE:  to qualify destination mac of a l4 rule
 * INPUT:    data --destination mac
 *           mask
 * OUTPUT:   re.
 * RETURN:   
 * NOTE:     
 *---------------------------------------------------------------------------
 */   
BOOL_T HRDRV_Qualify_DMac(UI8_T data[6], UI8_T mask[6], HRDRV_TYPE_ResouceEntity_T *re) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyField
 *---------------------------------------------------------------------------
 * PURPOSE:  to qualify the checking field of a L4 rule 
 * INPUT:    field  -- one of HRDRV_PktField_T]
 *           op     -- HRDRV_OPERATOR_NO_OPERATOR : val and mask is invalid, no checking 
 *                  -- HRDRV_OPERATOR_EQUAL : val means checking filed and mask is the interested bits of val
 *                  -- HRDRV_OPERATOR_RANGE: val means the max value , mask is the min value
 *           val    -- value :
 *           mask   -- depned on op value 
 *           
 * OUTPUT:   re     -- resource entity
 * RETURN:   TRUE /FALSE 
 * NOTE   :  val and mask are in Host Order       
 *---------------------------------------------------------------------------
 */ 
BOOL_T HRDRV_QualifyField(UI32_T  field, HRDRV_Operator_T op,  UI32_T val, UI32_T mask, HRDRV_TYPE_ResouceEntity_T *re);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyDoFuncType
 *---------------------------------------------------------------------------
 * PURPOSE:  to specifiy the L4 rule is used by which component
 * INPUT:    type -- value of HRDRV_FunctionType_T 
 *                -- HRDRV_DO_IP_ACL
 *                -- HRDRV_DO_MAC_ACL
 *                -- HRDRV_DO_IP_DIFFSERV
 *                -- HRDRV_DO_MAC_DIFFSERV
 * OUTPUT    re          
 * RETURN:   TRUE / FASLE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */   
BOOL_T HRDRV_QualifyDoFuncType(UI32_T fun_type, HRDRV_TYPE_ResouceEntity_T *re) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME -HRDRV_QualifyInProfileAction
 *---------------------------------------------------------------------------
 * PURPOSE:  to specify the FFP action of a in-profile L4 rule
 * INPUT:    action    -- is the bitwise or of HRDRV_Action_T
 *           param -- if a action without paramter , for example HR_ACTION_PERMIT, 
 *                        then don't care about parameter or set parameter=0
 * OUTPUT:   re
 * RETURN:   TRUE / FALSE , if the action is supported by chipset , return TRUE,
 *                          otherwise return FALSE
 * NOTE:     1. these function can be called several time for multiple action, 
 *              per action per HRDRV_QualifyInProfileAction call
 *           
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_QualifyInProfileAction(UI32_T action, UI32_T param, HRDRV_TYPE_ResouceEntity_T *re) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyOutProfileAction
 *---------------------------------------------------------------------------
 * PURPOSE:  to specify the FFP out prifile action of L4 rule
 * INPUT:    action    -- HRDRV_OutAction_T  is the bitwise-or of HRDRV_OutAction_T 
 *           param -- if a action without paramter , for example HR_ACTION_PERMIT, 
 *                        then don't care about parameter or set parameter=0
 * OUTPUT:   re
 * RETURN:   TRUE / FALSE , if the action is supported by chipset , return TRUE,
 *                          otherwise return FALSE
 * NOTE:     this API can called multiple time for combinational action
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_QualifyOutProfileAction(UI32_T action, UI32_T param, HRDRV_TYPE_ResouceEntity_T *re) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyClassInstantRateLimitID
 *---------------------------------------------------------------------------
 * PURPOSE:  to associate a L4 rule with a class_instant_id  which is allocate by 
 *           HRDRV_AllocateClassInstantRateLimitID
 * INPUT:    unit , port
 *           class_instant_id : used to indentify a meter resouce 
 * OUTPUT:   re
 * RETURN:   TRUE / FALSE
 * NOTE:     the class instant rate id is record in re 
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_QualifyClassInstantRateLimitID(UI32_T unit, UI32_T port, UI32_T class_instatnt_id,  HRDRV_TYPE_ResouceEntity_T *re) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_QualifyBindingPortAndInout
 *---------------------------------------------------------------------------
 * PURPOSE:  specify binding port and ingress/egress direction of a L4 rule
 * INPUT:    unit 
 *           port  -- 65535 means global configuratoin, bind to the whole system 
 *           inout -- one of HRDRV_Inout_T 
 *                 -- HRDRV_INBOUND
 *                 -- HRDRV_OUTBOUND
 * OUTPUT:   re    -- resource entity
 * RETURN:   TRUE / FALSE
 * NOTE:     if port = 65535, don't care of the unit 
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_QualifyBindingPortAndInout(UI32_T unit, UI32_T port, UI32_T inout, HRDRV_TYPE_ResouceEntity_T *re) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Alloc
 *---------------------------------------------------------------------------
 * PURPOSE:  allocate an rid (transation ID) before an L4 rule write the ASIC
 * INPUT:    None.
 * OUTPUT:   rid
 * RETURN:   TRUE / FALSE
 * NOTE:     rid is the conceptual transition id, 
 *           you must allocate it first and use this rid to allocate resource 
 *         
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_Alloc(UI32_T *rid) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Commit
 *---------------------------------------------------------------------------
 * PURPOSE:  to write the l4 rule with the same rid to ASIC
 * INPUT:    rid;
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *          
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_Commit(UI32_T rid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Abort
 *---------------------------------------------------------------------------
 * PURPOSE:  releas all the resocues allocate for this trasaction id 
 * INPUT:    rid : transaction id 
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTE:     a transaction is a atomic operation, if any of the allocate faield for
 *           this transaction, then user must abort this transaction instead of commit it 
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_Abort(UI32_T rid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Reserve
 *---------------------------------------------------------------------------
 * PURPOSE:  reserve the resouce for a L4 rule 
 * INPUT:    rid, re
 * OUTPUT:   NONE
 * RETURN:   TRUE / FALSE 
 *           TRUE : hardware has resouce for this L4 rule
 *           FALSE: short of resouce for this L4 rule
 * NOTE:     
 *         
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_Reserve(UI32_T rid, HRDRV_TYPE_ResouceEntity_T *re) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Release
 *---------------------------------------------------------------------------
 * PURPOSE:  delete the L4 rule from the ASIC 
 * INPUT:    re
 * OUTPUT:   NONE
 * RETURN:   TRUE / FALSE 
 * NOTE:     
 *           
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_Release(HRDRV_TYPE_ResouceEntity_T *re);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_AllocateClassInstantRateLimitID
 *---------------------------------------------------------------------------
 * PURPOSE:  to allocate the meter resouces for instance of the ACL class binding 
 *           in a interface 
 * INPUT:    fun_type, unit, port
 *           rate : Kbps, 
 *           burst_size : bytes
 *           class_instant_id : is defined by L4 core layer , a unique key to indentify the meter resouce
 * OUTPUT:   support_burst_size : is the burst size supported in chipset
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_AllocateClassInstantRateLimitID(UI32_T fun_type, UI32_T unit, UI32_T port, UI32_T class_instant_id, UI32_T rate, UI32_T burst_size,  UI32_T *support_burst_size);




/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_FreeClassInstantRateLimitID
 *---------------------------------------------------------------------------
 * PURPOSE:  to free meter resouces for instance of the ACL class binding 
 *           in a interface 
 * INPUT:    fun_type, unit, port
 *           class_instant_id : is defined by L4 core layer, a key to identify the
 *                              meter resource
 * OUTPUT:   
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_FreeClassInstantRateLimitID(UI32_T fun_type, UI32_T unit, UI32_T port, UI32_T class_instant_id);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable igmp control packet trap to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : call by IGMP compnent
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T HRDRV_EnableIgmpTrap(UI32_T mode);




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will diable igmp control packet trap to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : call by IGMP compnent
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T HRDRV_DisableIgmpTrap(UI32_T mode) ;


/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable tos mapping
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : it is as the driver of swdrvl4.c 
 *           this API is global configuration compared to HRDRV_EnablePortCosMapping
 *           which to call , depend on upper layer 
 * -------------------------------------------------------------------------
 */
BOOL_T HRDRV_EnableCosMapping(UI32_T maptype);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable tos mapping
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : it is as the driver of swdrvl4.c 
 *           this API is global configuration compared to HRDRV_EnablePortCosMapping
 *           which to call , depend on upper layer 
 *
 * -------------------------------------------------------------------------
 */
BOOL_T HRDRV_DisableCosMapping(UI32_T maptype);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_SetPortCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set TOS/COS mapping of system
 * INPUT   : unit   -- device_id
 *           port   -- port number
 *           maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 *           cos    -- the dot1p priority used to map to hardware queue 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/       
BOOL_T HRDRV_SetPortCosMapping(UI32_T unit, UI32_T port, UI32_T maptype, UI32_T value, UI32_T cos) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_SetCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set TOS/COS mapping of system
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 *           cos    -- the dot1p priority used to map to hardware queue 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is global configuration compared to HRDRV_SetPortCosMapping
 * -------------------------------------------------------------------------*/       
BOOL_T HRDRV_SetCosMapping(UI32_T maptype, UI32_T value, UI32_T cos) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DelTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete TOS/COS mapping of system
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is global conifuration compared to HRDRV_DelPortCosMapping
 * -------------------------------------------------------------------------*/
BOOL_T HRDRV_DelCosMapping(UI32_T maptype, UI32_T value) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DelPortCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete TOS/COS mapping of system
 * INPUT   : unit   -- device_id
 *           port 
 *           maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T HRDRV_DelPortCosMapping(UI32_T unit, UI32_T port, UI32_T maptype, UI32_T value);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : unit, port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T HRDRV_EnablePortIngressRateLimit(UI32_T unit, UI32_T port) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_SetPortIngressRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  to set the ingress rate limit 
 * INPUT:    unit
 *           port
 *           rate   -- ingress rate
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTES :   
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_SetPortIngressRateLimit(UI32_T unit, UI32_T port, UI32_T rate);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisablePortIngressRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  to diable port ingress rate-limit
 * INPUT:    unit, port.
 * OUTPUT:   None.
 * RETURN:   TRUE 
 *           FALSE -- if port ingress rate-limit is not supported
 * NOTES :  
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_DisablePortIngressRateLimit(UI32_T unit, UI32_T port);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableBroadcastIpTrapToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will diable broadcast ip packet trap to cpu
 * INPUT:    mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE 
 * NOTES :   no core component will call this , because DHCP install trap broadcast ip 
 *           to cpu by DEV_HRDRV_SetInterventionEntry
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_DisableBroadcastIpTrapToCpu(UI32_T mode);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableBroadcastIpTrapToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will enable broadcast ip packet trap to cpu
 * INPUT:    mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE 
 * NOTES :   no core component will call this , because DHCP install trap broadcast ip 
 *           to cpu by DEV_HRDRV_SetInterventionEntry
 * 
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_EnableBroadcastIpTrapToCpu(UI32_T mode) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableTrapOspfToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  to enable trap ospf protocl packet  to CPU
 * INPUT:    *cpu_mac : cpu mac address(6 bytes)
 * OUTPUT:   None
 * RETURN:   TRUE  
 *           FALSE -- if trap ospf is not supported
 * NOTES :   This API will add three rule and one mask
 *           1)Eth-Type=0800, Protocol Type=89(0x59), DA=CPU Mac-address
 *           2)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000005
 *           3)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000006
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_EnableTrapOspfToCpu(UI8_T *cpu_mac);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableTrapOspfToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  to disable trap ospf protocl packet  to CPU
 * INPUT:    *cpu_mac : cpu mac address(6 bytes)
 * OUTPUT:   None
 * RETURN:   TRUE  
 *           FALSE -- if trap ospf is not supported
 * NOTES :   This API will remvoe mask and rule from FFP
 *           1)Eth-Type=0800, Protocol Type=89(0x59), DA=CPU Mac-address
 *           2)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000005
 *           3)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000006
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_DisableTrapOspfToCpu(UI8_T *cpu_mac);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DelAclMaskEntry
 * -------------------------------------------------------------------------
 * FUNCTION: use to del a user-defined mask
 * INPUT   : mask
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1. before an ace can write to hrdrv, mask must be added first
 *              sys_defined mask is created by hrdrv automaticaly
 *           2. if a mask is used by any existing irule, it will return FALSE
 * -------------------------------------------------------------------------*/ 
BOOL_T HRDRV_DelAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *mask) ;


/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_AddAclMaskEnry
 * -------------------------------------------------------------------------
 * FUNCTION: use to add a user-defined a mask
 * INPUT   : mask
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1. before an ace can write to hrdrv, mask must be added first
 *              sys_defined mask is created by hrdrv automaticaly
 *           2. the earlier the acl is added,  matched precedence is high
 * -------------------------------------------------------------------------*/   
BOOL_T HRDRV_AddAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *mask) ;



/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_ModifyAclMaskEntry
 * -------------------------------------------------------------------------
 * FUNCTION: modify a existed acl mask entry
 * INPUT   : old -- the mask to be replaced 
 *           input -- new setting
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : if a mask is in-use, the operation is not allowed 
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_ModifyAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *old, HRDRV_TYPE_AclMaskEntry_T *input);




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DelMaskTbl
 * -------------------------------------------------------------------------
 * FUNCTION: use to delete a user-defined mask table
 * INPUT   : type  -- one of HRDRV_MaskType_T
 *                 HRDRV_IP_MASK
 *                 HRDRV_MAC_MASK
 *           inout -- one of HRDRV_Inout_T
 *                HRDRV_INBOUND
 *                HRDRV_OUTBOUND
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1. before an ace can write to hrdrv, mask must be added first
 *              sys_defined mask is created by hrdrv automaticaly
 *           2. if a mask is used by irule, it will return FALSE
 * -------------------------------------------------------------------------*/ 
BOOL_T HRDRV_DelAclMaskTbl(UI32_T type, UI32_T inout) ;




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableMyMacTrap2Cpu
 * -------------------------------------------------------------------------
 * PURPOSE:  to enable packet with DA =master's mac address cpu 
 * INPUT:    mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE -- if trap ospf is not supported
 * NOTES :   this is call when CPU STORM of my mac class is off
 *           no core component will call this, my mac is trap to cpu by l2 address
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_EnableMyMacTrap2Cpu(UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableMyMacTrap2Cpu
 * -------------------------------------------------------------------------
 * FUNCTION: to disable packet with DA =master's mac address cpu 
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of my mac class is on
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_DisableMyMacTrap2Cpu(UI32_T mode);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableTrapArpToCpu
 *---------------------------------------------------------------------------
 * FUNCTION: to enable trap arp request packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp is off
 *           no core layer component will call this, arp entry is trap to CPU 
 *           by HRDRV_SetInterventionEntry
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_EnableTrapArpToCpu(UI32_T mode);





/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableTrapArpToCpu
 *---------------------------------------------------------------------------
 * FUNCTION: to disable trap arp request packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp is on
 *           no core layer component will call this, arp entry is trap to CPU 
 *           by HRDRV_SetInterventionEntry
 *---------------------------------------------------------------------------
 */
BOOL_T HRDRV_DisableTrapArpToCpu(UI32_T mode);




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableTrapArpReply
 * -------------------------------------------------------------------------
 * FUNCTION: to enable trap arp reply packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp reply is off
 *           no core layer component will call this, arp reply entry is trap to 
 *           cpu by l2 mac address 
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_EnableTrapArpReply(UI32_T mode);




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableTrapArpReply
 * -------------------------------------------------------------------------
 * FUNCTION: to disable trap arp reply packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp reply is off
 *           no core layer component will call this, arp reply entry is trap to 
 *           cpu by l2 mac address 
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_DisableTrapArpReply(UI32_T mode);


/*------------------------------------------------------------------------------
 * Function : HRDRV_SetInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : Packets with this MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only 
 *            depend on spec.                                                            
 *------------------------------------------------------------------------------*/
BOOL_T HRDRV_SetInterventionEntry(UI32_T vid, UI8_T *mac);


/*------------------------------------------------------------------------------
 * Function : HRDRV_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : Delete specific MAC address from the list of traping packet to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : None.                                                    
 *------------------------------------------------------------------------------*/
BOOL_T HRDRV_DeleteInterventionEntry(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : HRDRV_AddMyMACIP2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add mgmt MAC address and mgmt ip address.
 * INPUT    : UI32_T ip     - ip address
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : None.                                                    
 *------------------------------------------------------------------------------*/
BOOL_T HRDRV_AddMyMACIP2Cpu(UI8_T *mac, UI32_T ip);

/*------------------------------------------------------------------------------
 * Function : HRDRV_RemoveMyMACIP2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Remove mgmt MAC address and mgmt ip address.
 * INPUT    : UI32_T ip     - ip address
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : None.                                                    
 *------------------------------------------------------------------------------*/
BOOL_T HRDRV_RemoveMyMACIP2Cpu(UI8_T *mac, UI32_T ip);

#if (SYS_CPNT_DOT1X == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_SetDot1xAuthTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will support trap dot1x mac packet
 * INPUT   : unit  --- unit id
 *           port  --- unit port
 *           mode      --  DEV_SWDRV_DOT1X_PACKET_DISCARD = 0,
 *                         DEV_SWDRV_DOT1X_PACKET_FORWARD,
 *                         DEV_SWDRV_DOT1X_PACKET_TRAPTOCPU
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : so this function will set ETHER_TYPE=888E packet to switch/trap/discard
 * -------------------------------------------------------------------------*/  
BOOL_T HRDRV_SetDot1xAuthTrap(UI32_T unit, UI32_T port, UI32_T mode);
#endif

#if (SYS_CPNT_STACKING == TRUE)
/* -------------------------------------------------------------------------
 * Function : HRDRV_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of HRDRV via ISC
 * INPUT    : *key      -- key of ISC
 *            *mem_ref  -- transfer data 
 * OUTPUT   : None                                                           
 * RETURN   : None             
 * NOTE     : call by ISC_AGENT
 * -------------------------------------------------------------------------
 */
BOOL_T HRDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif /* end of SYS_CPNT_STACKING == TRUE */

#endif /*  end of HRDRV_H */
