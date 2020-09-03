/* Module Name: ISC.C
 * Purpose: 
 *      ISC is the short form of Inter-Service-Communication. This module provides four APIs for
 *      CSCs to communicate with other units when stacking. These four APIs are of different 
 *      delivery types: unreliable multicast delivery, reliable multicast delivery, reliable 
 *      unicast delivery with reply data, and delivery to next unit.   
 * Notes: 
 *      1. CSCs' Set functions should send ISC packet first, then set CSCs'  
 *         local (for a better performance)(otherwise, need to get SA's persmision)
 *      2. CSCs' Set functions should use ISC_SendMCastReliable instead of 
 *         ISC_RemoteCall/ISC_Send(for a better performance and more correct)
 *         (otherwise, need to get SA's persmision)
 *      3. CSCs should call ISC APIs only when remote destination unit(s)/bitmap 
 *         is(are) all existed (for a better performance)       
 * History:                                                               
 *    
 * Copyright(C)      Accton Corporation, 2005                   
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
//kh_shi #include <assert.h>
#include <stdio.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_stack.h"
#include "l_bitmap.h"
#include "l_math.h"
#include "l_ipcmem.h"
#include "isc.h"
#include "iuc.h"
#include "dev_nicdrv.h"
#include "lan_type.h"
#include "stktplg_pom.h"
#include "sys_module.h"
#include "backdoor_mgr.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "isc_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define ISC_HEADER_LEN  sizeof(ISC_Header_T)

#define ISC_POOL_ID_DEFAULT  0 
#define ISC_TRACE_ID_DEFAULT 0

#define NO_PVC_SLEEP_TIME 2

#define ISC_VERSION  0x02

/* how many entity joined the communication
 */
#define ISC_MAX_NBR_OF_UNIT     SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK

/* define the fragment type of ISC packet
 */
#define ISC_NO_FRAGMENT         0
#define ISC_FIRST_FRAGMENT      1
#define ISC_SECOND_FRAGMENT     2

/* define 802.1p
 */
 
#define ISC_MIN_OF_8021P       0
#define ISC_MAX_OF_8021P       7

/* DATA TYPE DECLARACTIONS
 */



/* MACRO FUNCTION DECLARATIONS
 */
#define ATOM_EXPRESSION(exp)                \
{                                           \
    SYSFUN_OM_ENTER_CRITICAL_SECTION();     \
    (exp);                                  \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION();     \
}


/* LOCAL FUNCTION DECLARATIONS
 */
/* ISC_PacketIncoming and ISC_GetPacketType do not use static because they are
 * called directly from IUC
 */
void          ISC_PacketIncoming(UI32_T rx_port, L_MM_Mref_Handle_T *mem_ref_p);
LAN_TYPE_PacketType_T ISC_GetPacketType(UI8_T *data_p);
static void   ISC_CallbackToUpperCsc(L_MM_Mref_Handle_T *mem_ref, UI32_T rx_port);
static void   ISC_GetStackAndUnitsInfo(void);
static UI16_T ISC_SendAndWaitReply(UI32_T                  op_code,
                                     UI16_T                  dst_bmp,    
                                     UI8_T                   svc_id,
                                     L_MM_Mref_Handle_T      *mem_ref,   
                                     UI32_T                  priority,
                                     UI16_T                  rep_buflen, 
                                     UI8_T                   *rep_buf,
                                     UI8_T                   try_count,
                                     UI32_T                  time_out );
static void   ISC_FillHeader(ISC_Header_T *isc_header_p, ISC_Opcode_T opcode, UI8_T svc_id, UI32_T pdu_len, UI32_T priority); 
static BOOL_T ISC_SendFragmentPacket(L_MM_Mref_Handle_T *mem_ref, UI32_T dst_unit, UI8_T Uplink_Downlink, UI32_T priority);
static L_MM_Mref_Handle_T *ISC_Find_1st_Fragment_Packet_And_Assembly(L_MM_Mref_Handle_T *mem_ref_p);
static void   ISC_ResumeAllSuspendTask(void);
static BOOL_T ISC_ReplyEngine(ISC_Key_T *key, ISC_Opcode_T opcode); 
static BOOL_T ISC_CheckValidDestUnit(UI16_T *dst_bmp_p, UI8_T svc_id);

/* backdoor functions
 */
static UI32_T ISC_BD_InputInteger(void);
static void   ISC_BD_DisplayDataByHex(UI8_T *data, UI32_T length);
static void   ISC_BD_DisplayPacket(L_MM_Mref_Handle_T *mem_ref, BOOL_T is_show_payload);
static void   ISC_BD_Show_ISC_Status(void);
static void   ISC_BD_CapturePacketToggle(BOOL_T *capture_packet, BOOL_T *display_contain, UI8_T *sid);
static BOOL_T ISC_BD_PacketIncoming(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id);
static void   ISC_BD_SendTestingISCsPDU(void);
static void   ISC_BD_SendTestingISCsPDUAndWaitReply(void);
static void   ISC_BD_SendTestingMCastPDUnWaitReply(void);
static void   ISC_BD_SendTestingISCPDUToNextHop(void);
static void   ISC_BD_BackDoorMenu(void);
static BOOL_T ISC_BD_IncTxReplyCounter(UI8_T sid,UI8_T type);
static void ISC_BD_ClearTxReplyCounter();
static void ISC_BD_ShowTxReplyCounter();
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
static void ISC_BD_ShowTxCounters();
#endif


/* STATIC VARIABLE DECLARACTIONS
 */

static const BOOL_T is_opcode_reply_type[ISC_OP_NUM_OF_OPCODE] = { FALSE, /*ISC_OP_RCC_REQUEST         */
                                                                   TRUE,  /*ISC_OP_RCC_REPLY           */
                                                                   FALSE, /*ISC_OP_UNRELIABLE_DELIVERY */
                                                                   FALSE, /*ISC_OP_RMC_REQUEST         */
                                                                   FALSE, /*ISC_OP_RMC_RC_REQUEST      */
                                                                   TRUE,  /*ISC_OP_ACK                 */
                                                                   TRUE   /*ISC_OP_NAK                 */ };
                                                              
static const BOOL_T is_get_PVC[ISC_OP_NUM_OF_OPCODE]           = { TRUE,  /*ISC_OP_RCC_REQUEST         */ 
                                                                   FALSE, /*ISC_OP_RCC_REPLY           */ 
                                                                   FALSE, /*ISC_OP_UNRELIABLE_DELIVERY */ 
                                                                   TRUE,  /*ISC_OP_RMC_REQUEST         */ 
                                                                   TRUE,  /*ISC_OP_RMC_RC_REQUEST      */ 
                                                                   FALSE, /*ISC_OP_ACK                 */ 
                                                                   FALSE  /*ISC_OP_NAK                 */ };



                                                  


#if 0 /*add by fen.wang,use share memory to store it. */
static UI32_T   isc_sid_delay_tick[ISC_SID_UNSUPPORTED]; /* retransmit delay tick for NAK*/
#endif
/* define which svc_id is allowed when ISC is in abnormal mode
 * abnormal mode means there exists slave's runtime code version different to master
 * only INTERNAL, FS, LEDDRV and SYSDRV is allowed in current design
 * FS for file operation, SYSDRV for reload and LEDDRV for download LED indicator
 */
static const BOOL_T isc_abnormal_stack_allowable[ISC_SID_UNSUPPORTED] = { FALSE,  /* ISC_RESERVED_SID            */
                                                                          TRUE,   /* ISC_INTERNAL_SID            */
                                                                          FALSE,  /* ISC_LAN_DIRECTCALL_SID      */
                                                                          FALSE,  /* ISC_LAN_CALLBYAGENT_SID     */
                                                                          FALSE,  /* ISC_AMTRDRV_DIRECTCALL_SID  */
                                                                          FALSE,  /* ISC_AMTRDRV_CALLBYAGENT_SID */
                                                                          FALSE,  /* ISC_NMTRDRV_SID             */
                                                                          FALSE,  /* ISC_SWDRV_SID               */
                                                                          TRUE,   /* ISC_LEDDRV_SID              */
                                                                          TRUE,   /* ISC_FS_SID                  */
                                                                          FALSE,  /* ISC_SWDRVL4_SID             */
                                                                          TRUE,   /* ISC_SYSDRV_SID              */
                                                                          FALSE,  /* ISC_HRDRV_SID               */
                                                                          FALSE,  /* ISC_SWDRVL3_SID             */
                                                                          FALSE,  /* ISC_SWDRV_CACHE_SID         */
                                                                          FALSE,  /* ISC_CFGDB_SID               */
                                                                          FALSE,  /* ISC_POEDRV_SID              */
                                                                          FALSE,  /* ISC_STK_TPLG_SID            */
                                                                          FALSE,   /* ISC_RULE_CTRL_SID           */
                                                                          FALSE,    /*ISC_MSL_SID*/
                                                                          FALSE    /*ISC_SYS_TIME*/
                                                                        };

static const UI16_T ServiceId2ModuleId[ISC_SID_UNSUPPORTED] = { 0,                      /* ISC_RESERVED_SID            */
                                                                SYS_MODULE_ISCDRV,      /* ISC_INTERNAL_SID            */
                                                                SYS_MODULE_LAN,         /* ISC_LAN_DIRECTCALL_SID      */
                                                                SYS_MODULE_LAN,         /* ISC_LAN_CALLBYAGENT_SID     */
                                                                SYS_MODULE_AMTRDRV,     /* ISC_AMTRDRV_DIRECTCALL_SID  */
                                                                SYS_MODULE_AMTRDRV,     /* ISC_AMTRDRV_CALLBYAGENT_SID */
                                                                SYS_MODULE_NMTRDRV,     /* ISC_NMTRDRV_SID             */
                                                                SYS_MODULE_SWDRV,       /* ISC_SWDRV_SID               */
                                                                SYS_MODULE_LEDDRV,      /* ISC_LEDDRV_SID              */
                                                                SYS_MODULE_FS,          /* ISC_FS_SID                  */
                                                                SYS_MODULE_SWDRVL4,     /* ISC_SWDRVL4_SID             */
                                                                SYS_MODULE_SYSDRV,      /* ISC_SYSDRV_SID              */
                                                                SYS_MODULE_HRDRV,       /* ISC_HRDRV_SID               */
                                                                SYS_MODULE_SWDRVL3,     /* ISC_SWDRVL3_SID             */
                                                                SYS_MODULE_SWDRV_CACHE, /* ISC_SWDRV_CACHE_SID         */
                                                                SYS_MODULE_CFGDB_UTIL,  /* ISC_CFGDB_SID               */
                                                                SYS_MODULE_POEDRV,      /* ISC_POEDRV_SID              */
                                                                SYS_MODULE_STKTPLG,     /* ISC_STK_TPLG_SID            */
                                                                SYS_MODULE_RULE_CTRL,    /* ISC_RULE_CTRL_SID           */
                                                                SYS_MODULE_MSL,          /*ISC_MSL_SID*/
                                                                SYS_MODULE_SYS_TIME      /*SYS_MODULE_SYS_TIME*/
                                                              };


static  const char *OpcodeName[ISC_OP_NUM_OF_OPCODE] = { "ISC_OP_RCC_REQUEST",
                                                          "ISC_OP_RCC_REPLY",
                                                          "ISC_OP_UNRELIABLE_DELIVERY",
                                                          "ISC_OP_RMC_REQUEST",
                                                          "ISC_OP_RMC_RC_REQUEST",
                                                          "ISC_OP_ACK",
                                                          "ISC_OP_NAK"};                   
                                                                                             
static  const char *ServName[ISC_SID_UNSUPPORTED+1] = {  "ISC_RESERVED_SID",
                                                        "ISC_INTERNAL_SID",    
                                                        "ISC_LAN_DIRECTCALL_SID",  
                                                        "ISC_LAN_CALLBYAGENT_SID", 
                                                        "ISC_AMTRDRV_DIRECTCALL_SID", 
                                                        "ISC_AMTRDRV_CALLBYAGENT_SID",
                                                        "ISC_NMTRDRV_SID",            
                                                        "ISC_SWDRV_SID",           
                                                        "ISC_LEDDRV_SID",          
                                                        "ISC_FS_SID",              
                                                        "ISC_SWDRVL4_SID",         
                                                        "ISC_SYSDRV_SID",          
                                                        "ISC_HRDRV_SID",           
                                                        "ISC_SWDRVL3_SID",         
                                                        "ISC_SWDRV_CACHE_SID",     
                                                        "ISC_CFGDB_SID",           
                                                        "ISC_POEDRV_SID",          
                                                        "ISC_STK_TPLG_SID",
                                                        "ISC_RULE_CTRL_SID",
                                                        "ISC_MSL_SID",
                                                        "ISC_SYS_TIME",
                                                        "ISC_SID_UNSUPPORTED"
                                                      };

static ISC_ServiceFunc_T   isc_service_func[ISC_SID_UNSUPPORTED];
static ISC_BD_OM_ControlBlock_T* isc_bd_cb_p;
#define ISC_BD_capture_received (isc_bd_cb_p->capture_received)
#define ISC_BD_capture_transmitted (isc_bd_cb_p->capture_transmitted)
#define ISC_BD_capture_received_sid (isc_bd_cb_p->capture_received_sid)
#define ISC_BD_capture_transmitted_sid (isc_bd_cb_p->capture_transmitted_sid)
#define ISC_BD_display_rx_payload (isc_bd_cb_p->display_rx_payload)
#define ISC_BD_display_tx_payload (isc_bd_cb_p->display_tx_payload)

static BOOL_T ISC_BD_IncRxCounter(UI8_T type);
static void ISC_BD_ShowRxCounter();
static void ISC_BD_ClearRxCounter();
static UI32_T  ISC_BD_PKT_COUNTER[ISC_SID_UNSUPPORTED+1];
static UI32_T  ISC_BD_PKT_TxReplyCounter[ISC_SID_UNSUPPORTED+1][2];

/* EXPORTED FUNCTIONS BODY
 */

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for ISC
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_InitiateSystemResources(void)
{
    ISC_OM_InitiateSystemResources();
    ISC_OM_Init();    
}
 

/*---------------------------------------------------------------------------------
 * FUNCTION : ISC_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ISC in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void ISC_AttachSystemResources(void)
{
    ISC_OM_AttachSystemResources();    
    isc_bd_cb_p = ISC_OM_DB_GetCBPtr();
} 
 
/* FUNCTION NAME : ISC_Init
 * PURPOSE: 
 *          ISC_Init is called to init values and allocate resource for ISC
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_Init(void)
{
    UI32_T  SpeedLevelToDelayTick[SYS_ADPT_ISC_MAX_SPEEDLEVEL+1] = { 0,                           
                                                                     SYS_ADPT_ISC_SPEEDLEVEL1_RETRANSMIT_DELAY_TICK, 
                                                                     SYS_ADPT_ISC_SPEEDLEVEL2_RETRANSMIT_DELAY_TICK, 
                                                                     SYS_ADPT_ISC_SPEEDLEVEL3_RETRANSMIT_DELAY_TICK, 
                                                                     SYS_ADPT_ISC_SPEEDLEVEL4_RETRANSMIT_DELAY_TICK };
                                                             
    /* Assign initial value to variables                           
     */                                                      
    memset(isc_service_func, 0, sizeof(isc_service_func));
#if 0  /* anzhen.zheng, 6/25/2008 */
    isc_sid_delay_tick [ ISC_RESERVED_SID            ] =  0; 
    isc_sid_delay_tick [ ISC_INTERNAL_SID            ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_INTERNAL_SPEEDLEVEL           ];
    isc_sid_delay_tick [ ISC_LAN_DIRECTCALL_SID      ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_LAN_DIRECTCALL_SPEEDLEVEL     ];
    isc_sid_delay_tick [ ISC_LAN_CALLBYAGENT_SID     ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_LAN_CALLBYAGENT_SPEEDLEVEL    ];
    isc_sid_delay_tick [ ISC_AMTRDRV_DIRECTCALL_SID  ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_AMTRDRV_DIRECTCALL_SPEEDLEVEL ];
    isc_sid_delay_tick [ ISC_AMTRDRV_CALLBYAGENT_SID ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_AMTRDRV_CALLBYAGENT_SPEEDLEVEL];
    isc_sid_delay_tick [ ISC_NMTRDRV_SID             ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_NMTRDRV_SPEEDLEVEL            ];
    isc_sid_delay_tick [ ISC_SWDRV_SID               ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRV_SPEEDLEVEL              ];
    isc_sid_delay_tick [ ISC_LEDDRV_SID              ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_LEDDRV_SPEEDLEVEL             ];
    isc_sid_delay_tick [ ISC_FS_SID                  ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_FS_SPEEDLEVEL                 ];
    isc_sid_delay_tick [ ISC_SWDRVL4_SID             ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRVL4_SPEEDLEVEL            ];
    isc_sid_delay_tick [ ISC_SYSDRV_SID              ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_SYSDRV_SPEEDLEVEL             ];
    isc_sid_delay_tick [ ISC_HRDRV_SID               ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_HRDRV_SPEEDLEVEL              ];
    isc_sid_delay_tick [ ISC_SWDRVL3_SID             ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRVL3_SPEEDLEVEL            ];
    isc_sid_delay_tick [ ISC_SWDRV_CACHE_SID         ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRV_CACHE_SPEEDLEVEL        ];
    isc_sid_delay_tick [ ISC_CFGDB_SID               ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_CFGDB_SPEEDLEVEL              ];
    isc_sid_delay_tick [ ISC_POEDRV_SID              ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_POEDRV_SPEEDLEVEL             ];
    isc_sid_delay_tick [ ISC_STK_TPLG_SID            ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_STK_TPLG_SPEEDLEVEL           ];
    isc_sid_delay_tick [ ISC_RULE_CTRL_SID           ] = SpeedLevelToDelayTick[ SYS_ADPT_ISC_RULE_CTRL_SPEEDLEVEL          ];
#else
	ISC_OM_SetSidDelayTick(ISC_RESERVED_SID, 0);
	ISC_OM_SetSidDelayTick(ISC_INTERNAL_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_INTERNAL_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_LAN_DIRECTCALL_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_LAN_DIRECTCALL_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_LAN_CALLBYAGENT_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_LAN_CALLBYAGENT_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_AMTRDRV_DIRECTCALL_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_AMTRDRV_DIRECTCALL_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_AMTRDRV_CALLBYAGENT_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_AMTRDRV_CALLBYAGENT_SPEEDLEVEL]);

	ISC_OM_SetSidDelayTick(ISC_NMTRDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_NMTRDRV_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_SWDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRV_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_LEDDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_LEDDRV_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_FS_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_FS_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_SWDRVL4_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRVL4_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_SYSDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_SYSDRV_SPEEDLEVEL]);

	ISC_OM_SetSidDelayTick(ISC_HRDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_HRDRV_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_SWDRVL3_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRVL3_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_SWDRV_CACHE_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_SWDRV_CACHE_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_CFGDB_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_CFGDB_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_POEDRV_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_POEDRV_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_STK_TPLG_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_STK_TPLG_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_RULE_CTRL_SID, SpeedLevelToDelayTick[ SYS_ADPT_ISC_RULE_CTRL_SPEEDLEVEL]);
	ISC_OM_SetSidDelayTick(ISC_MSL_SID, SpeedLevelToDelayTick[SYS_ADPT_ISC_MSL_SPEEDLEVEL]);

#endif 																														   
    ISC_BD_capture_received = FALSE;
    ISC_BD_capture_transmitted = FALSE;
    ISC_BD_display_rx_payload = FALSE;
    ISC_BD_display_tx_payload = FALSE;    
                                                                                                                           
    ISC_BD_ClearRxCounter();
    ISC_BD_ClearTxReplyCounter();

    return;
    
} /* ISC_Init */ 


/* FUNCTION NAME : ISC_Register_Service_CallBack
 * PURPOSE: 
 *          ISC_Register_Service_CallBack is called for any entity who needs ISC's service
 * INPUT:   
 *          svc_id      --  Service ID, an calling entiry must have a Service ID to identify itself
 *          svc_func    --  this is func reference, it refers the function invoked once upon
 *                          the arrival of the message belonging to the service.                                
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE  -- callback function has been registered successfully
 *          FALSE -- Invalid service ID or duplicted ISC register callback function
 * NOTES:
 *          None
 */
BOOL_T  ISC_Register_Service_CallBack(UI8_T svc_id,  ISC_ServiceFunc_T svc_func )
{
    if ((ISC_RESERVED_SID == svc_id) || (ISC_SID_UNSUPPORTED <= svc_id) )
    {
        printf("ISC: Invalid service ID: %u\r\n", svc_id);
        return FALSE;
    }
    
    if (NULL != isc_service_func[svc_id])
    {   
        printf("ISC: error, duplicted ISC register callback function.\r\n");
        return FALSE;
    }   
     
    isc_service_func[svc_id] = svc_func;
    
    return TRUE;
}


/* FUNCTION NAME : ISC_Send
 * PURPOSE: 
 *          ISC_Send is called for an registered entity who wants to send a message unreliablly
 * INPUT:   
 *          dst_bmp     --  the intended receiver in bit map format(Bit0 of dst_bmp is unit1)
 *                          each bit in dst_bmp is corresponding to 16 drv_unit
 *          svc_id      --  Service ID, an calling entiry must have a Service ID to identify itself
 *          mem_ref     --  the pointer to memroy reference of the sending packet
 *          priority    --  priority of the sent packet
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE    :   the packet is transmited 
 *          FALSE   :   the packet is not transmited due to some error or transmitting failed
 * NOTES:
 *          1. This API doesn't process the PDU which needs to be fragmented except from LAN
 */
BOOL_T  ISC_Send(UI16_T              dst_bmp, 
                 UI8_T               svc_id, 
                 L_MM_Mref_Handle_T  *mem_ref, 
                 UI32_T              priority )
{
    ISC_Header_T    *isc_header_p;
    UI32_T          pdu_len;
    UI32_T          nonzero_bit_num;
    UI8_T           bit_pos_list[32];
    UI32_T          dst_unit;
         
    if((ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE) )
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;        
    }
    
    if (priority > ISC_MAX_OF_8021P)
    {
        printf("%s(): priority = %lu is invalid.\n", __FUNCTION__, priority);
        L_MM_Mref_Release(&mem_ref);
        return FALSE; 
    }

    if(svc_id >= ISC_SID_UNSUPPORTED)
    {
        printf("%s(): svc_id = %u is undefined.\n", __FUNCTION__, svc_id);
        L_MM_Mref_Release(&mem_ref);
        return FALSE; 
    }
    if (!ISC_CheckValidDestUnit( &dst_bmp, svc_id))
    {
        L_MM_Mref_Release(&mem_ref);
        return TRUE;    
    }
    
    if (L_MM_Mref_GetAvailSzBeforePdu(mem_ref) < SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN) 
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;
    }
    isc_header_p = L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);

    /* now the fragment feature can only be used by LAN, if other SIDs want
     * to send fragment packet, must be approved by SA
     */
    if (svc_id == ISC_LAN_CALLBYAGENT_SID || svc_id == ISC_LAN_DIRECTCALL_SID)
    {
        if(pdu_len > (2*SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented to more than two packets: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len-ISC_HEADER_LEN);
            L_MM_Mref_Release(&mem_ref);
            return FALSE;
        }
    }
    else
    {
        if(pdu_len > (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len-ISC_HEADER_LEN);
            L_MM_Mref_Release(&mem_ref);
            return FALSE;
        }
    }
    
    ISC_FillHeader(isc_header_p, ISC_OP_UNRELIABLE_DELIVERY, svc_id, pdu_len, priority);
    isc_header_p->dst_bmp = dst_bmp;  
   
    nonzero_bit_num = L_BITMAP_Get_BitPos_List((UI32_T)dst_bmp, bit_pos_list);
   
   /* ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED mean capture all ISC packets
    */
        if(ISC_BD_capture_transmitted && ((ISC_BD_capture_transmitted_sid == isc_header_p->sid )||(ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED)))
        {
            printf("\n");
            printf("========================ISC_Send===========================\n");
            printf("*    Shown before invoking IUC_SendPacket to transmit     *\n");
            printf("===========================================================\n");
            ISC_BD_DisplayPacket(mem_ref, ISC_BD_display_tx_payload);
            printf("\r\n");
        }            

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    ISC_OM_DB_IncTxCounters(svc_id,1);
#endif
    /* if the number of destination units more than one, use IUC_STACK_UNIT_ALL, which
     * will send to all units (the received units will filter it)
     */
    if (nonzero_bit_num == 1)
    {
        dst_unit = bit_pos_list[0] + 1;
    }
    else
    {
        dst_unit = IUC_STACK_UNIT_ALL;
    }
     
    if (pdu_len <= (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
    {
        return IUC_SendPacket(mem_ref, dst_unit, LAN_TYPE_TX_UP_DOWN_LINKS, priority);
    }
    else
    {
        return ISC_SendFragmentPacket(mem_ref, dst_unit, LAN_TYPE_TX_UP_DOWN_LINKS, priority);
    }
    
}/* ISC_Send */


/* FUNCTION NAME : ISC_RemoteCall
 * PURPOSE  : ISC_RemoteCall is called for an registered entity who wants to send a 
 *            message and wait for its reply     
 * INPUT    :
 *          drv_unit    : destination unit id.
 *                          1 ~ n   : Single unit
 *                          where n = SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK is the max allowable number of unit in stacking
 *          svc_id      : the id of requested service on remote unit.
 *          mem_ref     : the pointer to memroy reference of the sending packet
 *          priority    : priority of the sent packet
 *          try_count   : the number of tries.
 *                        0 means try forever.
 *          time_out    : waiting time for reply, unit :second.
 * OUTPUT   :
 *          rep_buflen  : length of rep_buf
 *          rep_buf     : data replied from the remote service function.
 *                    (This is a pointer pointing to the buffer allocated by user)
 * RETURN   :
 *          TRUE  - the return result saved in reply buffer.
 *          FALSE - can't send this request or time out, reply_frame is not valid.
 * Notes    :
 *            1. it's reply function is ISC_RemoteReply()
 *            2. This API doesn't process the PDU which needs to be fragmented except from LAN
 */
BOOL_T  ISC_RemoteCall(UI8_T              drv_unit,   
                       UI8_T              svc_id, 
                       L_MM_Mref_Handle_T *mem_ref,   
                       UI32_T             priority,
                       UI16_T             rep_buflen, 
                       UI8_T              *rep_buf,
                       UI8_T              try_count,    
                       UI32_T             time_out   )
{
    UI16_T  dst_bmp;

    if(ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;        
    }
    
    if (priority > ISC_MAX_OF_8021P)
    {
        printf("%s(): priority = %lu is invalid.\n", __FUNCTION__, priority);
        L_MM_Mref_Release(&mem_ref);
        return FALSE; 
    }
    
    if(svc_id >= ISC_SID_UNSUPPORTED)
    {
        printf("%s(): svc_id = %u is undefined.\n", __FUNCTION__, svc_id);    
        L_MM_Mref_Release(&mem_ref);
        return FALSE; 
    }

    dst_bmp =  (UI16_T) BIT_VALUE(drv_unit-1);

    if (!ISC_CheckValidDestUnit( &dst_bmp, svc_id))
    {
        L_MM_Mref_Release(&mem_ref);
        return TRUE;    
    }

    return (ISC_SendAndWaitReply(ISC_OP_RCC_REQUEST, dst_bmp, svc_id, mem_ref, priority,
                                 rep_buflen, rep_buf, try_count, time_out ) == 0 );
} /* ISC_RemoteCall */


/* FUNCTION NAME : ISC_SendMcastReliable
 * PURPOSE  :   For a registered entity to multicast packets to multiple units   
 * INPUT    :   dst_bmp         : destination unit bitmap
 *              svc_id          : the id of requested service on remote unit.
 *              mem_ref         : the pointer to memroy reference of the sending packet  
 *              priority        :   
 *              try_count       : the number of tries.
 *                                0 means try forever.
 *              time_out        : waiting time for reply, unit :msec.
 *              is_acked_by_app : TRUE  -- CSCs(receiver) will reply ACK by itself and ISC will reply NAK 
 *                                         if enqueue failed(Queue in ISC_AGENT) or callback function return FALSE.
 *                                FALSE -- ISC(receiver) will reply ACK or NAK.
 * OUTPUT   : None
 * RETURN   :    
                dst_bmp : the result of transmission (0:success; not equal to 0:fail)
 * Notes    : 1. IF(is_acked_by_app==TURE), remote CSC has to reply ISC_ACK by itself(call ISC_MCastReply()). 
 *               (similar to ISC_RemoteReply used for ISC_RemoteCall)
 *            2. If (is_acked_by_app==FALSE), ISC will reply ISC_ACK if remote CSC's ISC callback function returns
 *               TRUE, otherwise (return FALSE) reply ISC_NAK. Note that if CSC's ISC callback function is called by
 *               ISC agent task, the callback function return value is not used.
 *            3. This API doesn't process the PDU which needs to be fragmented except from LAN
 *            4. dst_bmp is identified as driver unit. Bit0 of dst_bmp is unit1.
 */
UI16_T ISC_SendMcastReliable (UI16_T                dst_bmp,    
                              UI8_T                 svc_id, 
                              L_MM_Mref_Handle_T    *mem_ref,   
                              UI32_T                priority,
                              UI8_T                 try_count,    
                              UI32_T                time_out,
                              BOOL_T                is_acked_by_app)
{
    if(ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mem_ref);
        return dst_bmp;        
    }

    if (priority > ISC_MAX_OF_8021P)
    {
        printf("%s(): priority = %lu is invalid.\n", __FUNCTION__, priority);
        L_MM_Mref_Release(&mem_ref);
        return dst_bmp; 
    }

    if(svc_id >= ISC_SID_UNSUPPORTED)
    {
        printf("%s(): svc_id = %u is undefined.\n", __FUNCTION__, svc_id);    
        L_MM_Mref_Release(&mem_ref);
        return dst_bmp; 
    }
    
    if (!ISC_CheckValidDestUnit( &dst_bmp, svc_id))
    {
        L_MM_Mref_Release(&mem_ref);
        return dst_bmp;    
    }
                 
    if(is_acked_by_app)
    {
        return ISC_SendAndWaitReply(ISC_OP_RMC_RC_REQUEST, dst_bmp, svc_id, mem_ref,
                                    priority, 0, 0, try_count, time_out);
    }
    else
    {
        return ISC_SendAndWaitReply(ISC_OP_RMC_REQUEST, dst_bmp, svc_id, mem_ref,
                                    priority, 0, 0, try_count, time_out);
    }
} /* ISC_SendMcastReliable */


/* FUNCTION NAME : ISC_SendAndWaitReply
 * PURPOSE  : ISC_SendAndWaitReply is called by ISC_RemoteCall and ISC_SendMcastReliable.
 *            It sends the packet out and waits the reply back.      
 * INPUT    :
 *              op_code     -- operation code(ISC_OP_RCC_REQUEST or ISC_OP_RMC_RC_REQUEST)
 *              dst_bmp     -- destination unit bitmap
 *              svc_id      -- the id of requested service on remote unit.
 *              mem_ref     -- the pointer to memroy reference of the sending packet
 *              priority    -- priority of the sent packet
 *              try_count   -- the number of retries.
 *              time_out    -- waiting time for reply, unit :second.
 * OUTPUT   :
 *              rep_buflen  -- length of rep_buf
 *              rep_buf     -- data replied from the remote service function.
 *                             (This is a pointer pointing to the buffer allocated by user)
 * RETURN   :   
 *              dst_bmp     -- the corresponding bit will be revised to ¡§0¡¨ when the transmitter receives 
 *                             the ACK replied by the receiver. Thus the transmitter base on "dst_bmp" to  
 *                             know whether the receiver receive the packet or not  
 * Notes    :  
 *              1. In this function, the sender task will suspend after send out the packet. There is a
 *                 defect that might cause the task suspend until time out when the system entering 
 *                 transition mode at unsuitable time.
 *                 ISC_SendAndWaitReply(){                  ISC_SetTransitionMode(){
 *                     ...                                      SYSFUN_SetTransitionMode();
 *  caller task IP ~~~>                                         ISC_ResumeAllSuspendTask();
 *                     fill pvc info;                       }
 *                     IUC_SendPacket();
 *                     SYSFUN_SuspendTaskWithTimeout();
 *                 }
 *                 There is a change that the caller task execute until ~~~> location, then context switch 
 *                 to other task. If the system enter transition mode at this moment and call 
 *                 ISC_SetTransitionMode(), only tasks that already suspended will be resumed. The caller
 *                 task of this ISC function will continue it's execution and suspend after send out
 *                 the packet. The result is: it will suspend until time out and it's transition mode
 *                 activities will be delayed.
 *              2. In order to solve the problem that we mentioned in note 1, the following strategy is 
 *                 adopted:
 *                 ISC_SendAndWaitReply(){                  ISC_SetTransitionMode(){
 *                     ...                                      SYSFUN_SetTransitionMode();
 *  caller task IP1 ~~~>                                        ISC_ResumeAllSuspendTask();
 *                     fill pvc info;                       }
 *                     if (in transition mode){
 *                        clear pvc info;
 *                     }else{
 *  caller task IP2 ~~~> 
 *                        IUC_SendPacket();
 *                        SYSFUN_SuspendTaskWithTimeout();
 *                     }
 *                 }
 *                 In the same hypothetic situation described in note 1, there are two possibilities:
 *                     a.Execute until IP1 and then context switch, enter transition mode, ... :
 *                       The caller task will find ISC is in transition mode and clear its own pvc info.
 *                       It wouldn't suspend its task in this case.
 *                     b.Execute until IP2 and then context switch, enter transition mode, ... :
 *                       Since the caller task had already fill the pvc info, corresponding binary semaphore
 *                       that use to realize SYSFUN_SuspendTaskWithTimeout()/SYSFUN_ResumeTaskInTimeoutSuspense()
 *                       will be set to full in function ISC_ResumeAllSuspendTask(). The only thing that will
 *                       happen is: SYSFUN_SuspendTaskWithTimeout() is called and continue to execute right
 *                       away (since it get the semaphone immediately).
 *              3. If SYSFUN_SuspendTaskWithTimeout()/SYSFUN_ResumeTaskInTimeoutSuspense() are implemented
 *                 by other methods instead of semaphore, different tactics must be considered.
 *
 */
static UI16_T ISC_SendAndWaitReply ( UI32_T              op_code,        
                                     UI16_T              dst_bmp,        
                                     UI8_T               svc_id,         
                                     L_MM_Mref_Handle_T  *mem_ref,       
                                     UI32_T              priority,       
                                     UI16_T              rep_buflen,     
                                     UI8_T               *rep_buf,       
                                     UI8_T               try_count,    
                                     UI32_T              time_out )      
{
    UI32_T              pdu_len;
    ISC_Header_T        *isc_header_p;   
    WaitReply_Info_T    pvc_info; 
    UI32_T              tx_count;
    UI32_T              nonzero_bit_num;
    UI8_T               bit_pos_list[32];
    UI8_T               *pdu_location;
    UI32_T              dst_unit;
    UI16_T              wait_dst_bmp;
	UI32_T				delay_tick;

    if (L_MM_Mref_GetAvailSzBeforePdu(mem_ref) < SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN)
    {
        goto QUIT;    
    }
     
    isc_header_p = L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
     
    /* now the fragment feature can only be used by LAN, if other SIDs want
     * to send fragment packet, must be approved by SA
     */     
    if (svc_id == ISC_LAN_CALLBYAGENT_SID || svc_id == ISC_LAN_DIRECTCALL_SID)
    {
        if(pdu_len > (2*SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented to more than two packets: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len-ISC_HEADER_LEN);
            goto QUIT;
        }
    }
    else
    {
        if(pdu_len > (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len-ISC_HEADER_LEN);
            goto QUIT;
        }
    }
         
    ISC_FillHeader(isc_header_p,  op_code, svc_id, pdu_len, priority);
   
    /* fill pvc_info
     */
    memset(&pvc_info, 0, sizeof(pvc_info));
    pvc_info.seq_no         = isc_header_p->seq_no;
    pvc_info.rep_buflen     = rep_buflen;
    pvc_info.dst_bmp        = dst_bmp;
    pvc_info.wait_dst_bmp   = dst_bmp;
    pvc_info.tid            = SYSFUN_TaskIdSelf();
    ISC_OM_SetPvcInfo(isc_header_p->pvc_no,&pvc_info);

    /* send packet
     */
    for(tx_count=0; ; tx_count++)
    {
        if (try_count>0 && tx_count>= try_count)
        {
            goto CLEAR_PVC_AND_QUIT;            
        }
        
        ISC_OM_SetPvcInfoWaitDstBmp(isc_header_p->pvc_no, dst_bmp);  
        isc_header_p->dst_bmp = dst_bmp;
        
        /* The reason we put transition mode checking here please refer to function header notes 1 & 2
         */
        if(ISC_OM_GetOperatingMode()== SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            goto CLEAR_PVC_AND_QUIT;
        }

        L_MM_Mref_AddRefCount(mem_ref, 1);/* for IUC_SendPacket to release*/

        nonzero_bit_num = L_BITMAP_Get_BitPos_List((UI32_T)dst_bmp, bit_pos_list);
   
        pdu_location = L_MM_Mref_GetPdu(mem_ref, &pdu_len);
        L_MM_Mref_MovePdu(mem_ref, (I32_T)((UI8_T *)isc_header_p - (UI8_T *)pdu_location), &pdu_len);
   
        /* ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED mean capture all packets
         */
        if(ISC_BD_capture_transmitted && ((ISC_BD_capture_transmitted_sid == isc_header_p->sid )||(ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED)))
        {
            printf("\n");
            if(op_code == ISC_OP_RCC_REQUEST)
                printf("=====================ISC_RemoteCall========================\n");
            else if(op_code == ISC_OP_RMC_REQUEST)
                printf("=============ISC_SendMcastReliable(ACK by ISC)=============\n");
            else
                printf("=============ISC_SendMcastReliable(ACK by APP)=============\n");
                                
            printf("     try_count : %lu/%u\n",tx_count+1,try_count);
            printf("*    Shown before invoking IUC_SendPacket to transmit     *\n");
            printf("===========================================================\n");
            ISC_BD_DisplayPacket(mem_ref, ISC_BD_display_tx_payload);
            printf("\r\n");
        }            

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
       ISC_OM_DB_IncTxCounters(svc_id,0);
#endif
        /* if the number of destination units more than one, use IUC_STACK_UNIT_ALL, which
         * will send to all units (the received units will filter it)
         */   
        if (nonzero_bit_num == 1)
        {
            dst_unit = bit_pos_list[0] + 1;
        }
        else
        {
            dst_unit = IUC_STACK_UNIT_ALL;
        }

        if (pdu_len <= (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            if(!IUC_SendPacket(mem_ref, dst_unit, LAN_TYPE_TX_UP_DOWN_LINKS, priority))
                goto CLEAR_PVC_AND_QUIT;
        }
        else
        {
            if(!ISC_SendFragmentPacket(mem_ref, dst_unit, LAN_TYPE_TX_UP_DOWN_LINKS, priority))
                goto CLEAR_PVC_AND_QUIT;
        }

        SYSFUN_SuspendTaskWithTimeout(time_out);
        ISC_OM_GetPvcInfoDstBmp(isc_header_p->pvc_no, &dst_bmp);
        ISC_OM_GetPvcInfoWaitDstBmp(isc_header_p->pvc_no, &wait_dst_bmp);
        
        /* if receive all replies but some are NAK 
         *   --> wait_dst_bmp == 0 (means all destination units had replied) and 
         *       dst_bmp != 0     (means not all destination units reply ACK)
         */  
/*EPR: ES3628BT-FLF-ZZ-01085
Problem:stack:hot remove and insert units cause unit not stackable.
Rootcause:(1)somes dut stacking together,and M is master then remove more than 2 dut(such as A,B,C,all them are slaves)
            And one of them A will be the new master,BC will be the slave of A.When A is doing enter master mode
            Then hotinsert to the old topo, and BC will be M slave and the unit Id may changed according  to the M mapping table.
            For A it should changed to slave but it will hang for it is send pkt to slave according to the ISC unit bit map
         (2) at this time ,the ISC unit bit map is still BC,but BC is the slave of M 
            
Solution: update A isc unit bitmap,and clear it.
Files:stktplg_engine.c,ISC.C,ISC_OM.C
*/
            /*dst_bmp &0xf000 ==0xf000 means it is updated by stktplg modue , not by the reply,
             stktplg need the csc not to wait for the reply,and return to next state.
             if not add ISC_OM_LOCAL_UNIT_BIT , when op_code == ISC_OP_RCC_REQUEST it may cause exception
             */

        if (wait_dst_bmp == 0 && (dst_bmp != 0 &&( (dst_bmp &ISC_OM_LOCAL_UNIT_BIT) !=ISC_OM_LOCAL_UNIT_BIT))) 
        {
            /*check if the get tick ok*/
            if (ISC_OM_GetSidDelayTick(svc_id, &delay_tick))
                SYSFUN_Sleep(delay_tick);
        }
        else if (dst_bmp == 0)
        {
            if (op_code == ISC_OP_RCC_REQUEST)
            {
                I32_T  mref_handle_offset;
                L_MM_Mref_Handle_T *mref_handle_p;
                UI8_T  *pdu;
                UI32_T pdu_len;

                ISC_OM_GetPvcInfoMrefHandleOffset(isc_header_p->pvc_no,&mref_handle_offset);
                mref_handle_p = L_IPCMEM_GetPtr(mref_handle_offset);
                pdu = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
                if (rep_buf != NULL && pdu != NULL)
                {
                    memcpy(rep_buf,pdu,rep_buflen);
                }

                L_MM_Mref_Release(&mref_handle_p);
                ISC_OM_ClearPvcInfoMrefHandleOffset(isc_header_p->pvc_no);
            }
            goto CLEAR_PVC_AND_QUIT;
        }
        /*dst_bmp &0xf000 ==0xf000 means it is updated by stktplg modue , not by the reply,
        stktplg need the csc not to wait for the reply,and return to next state.
        */
        else if ( (dst_bmp &ISC_OM_LOCAL_UNIT_BIT) ==ISC_OM_LOCAL_UNIT_BIT)
        {
          goto CLEAR_PVC_AND_QUIT;
        }
        /*else: timeout and retry*/
    } /* for(tx_count = 0; ; tx_count++) */
    
    /* end of retry. transmit fail */
CLEAR_PVC_AND_QUIT:
    ISC_OM_ClearPvcInfo(isc_header_p->pvc_no);
    ISC_OM_PushPvcNo(isc_header_p->pvc_no);
        
QUIT:
    L_MM_Mref_Release(&mem_ref);
    return dst_bmp;
     
}/* ISC_SendAndWaitReply */


/* FUNCTION NAME : ISC_SendPacketToNextHop
 * PURPOSE  :   
 *              Send packet to next hop  
 * INPUT    :
 *              mem_ref         -- the pointer to memroy reference of the sending packet
 *              Uplink_Downlink -- LAN_TYPE_TX_UP_LINK      : Uplink
 *                                 LAN_TYPE_TX_DOWN_LINK    : Downlink
 *                                 LAN_TYPE_TX_UP_DOWN_LINKS: Uplink and Downlink
 *              svc_id          -- Service ID, an calling entiry must have a Service ID to identify itself
 * OUTPUT   :
 *              None
 * RETURN   :   
 *              TRUE    --  the packet is transmited 
 *              FALSE   --  the packet is not transmited due to some error or transmitting failed     
 * Notes    :  
 *              ISC_SendPacketToNextHop doesn't wait reply (unreliable send)
 */
BOOL_T ISC_SendPacketToNextHop(L_MM_Mref_Handle_T *mem_ref, UI8_T Uplink_Downlink, UI8_T svc_id)
{
    ISC_Header_T    *isc_header_p;
    UI32_T          pdu_len;
    
    if(svc_id >= ISC_SID_UNSUPPORTED)
    {
        printf("%s(): svc_id = %u is undefined.\n", __FUNCTION__, svc_id);
        L_MM_Mref_Release(&mem_ref);
        return FALSE; 
    }
    
    if(Uplink_Downlink >= LAN_TYPE_TX_INVALID_DIRECTION)
    {
        printf("%s(): Uplink_Downlink = %u is out of range.\n", __FUNCTION__, Uplink_Downlink);
        L_MM_Mref_Release(&mem_ref);
        return FALSE;             
    }
        
    if (L_MM_Mref_GetAvailSzBeforePdu(mem_ref) < SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN) 
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;
    }
    
    isc_header_p = L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
    if (isc_header_p == NULL)
    {
        printf("%s(): L_MM_Mref_MovePdu return NULL\n",__FUNCTION__);
        L_MM_Mref_Release(&mem_ref);
        return FALSE;
    }    
    
    /* now the fragment feature can only be used by LAN, if other SIDs want
     * to send fragment packet, must be approved by SA
     */        
    if (svc_id == ISC_LAN_CALLBYAGENT_SID || svc_id == ISC_LAN_DIRECTCALL_SID)
    {
        if(pdu_len > (2*SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented to more than two packets: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len - ISC_HEADER_LEN);
            L_MM_Mref_Release(&mem_ref);
            return FALSE;
        }    
    }
    else
    {
        if(pdu_len > (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
        {
            printf("%s():Packet needs to be fragmented: svc_id = %u pdu_len=%lu\n",__FUNCTION__,svc_id,pdu_len-ISC_HEADER_LEN);
            L_MM_Mref_Release(&mem_ref);
            return FALSE;
        }
    }    

    ISC_FillHeader(isc_header_p, ISC_OP_UNRELIABLE_DELIVERY, svc_id, pdu_len, SYS_DFLT_STK_TPLG_PACKET_TO_CPU_PRIORITY);
    isc_header_p->dst_bmp = 0; 

    /* ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED mean capture all packets
     */
    if(ISC_BD_capture_transmitted && ((ISC_BD_capture_transmitted_sid == isc_header_p->sid )||(ISC_BD_capture_transmitted_sid == ISC_SID_UNSUPPORTED)))
    {
        printf("\n");
        printf("=================ISC_SendPacketToNextHop===================\n");
        printf("*    Shown before invoking IUC_SendPacket to transmit     *\n");
        printf("===========================================================\n");
        ISC_BD_DisplayPacket(mem_ref, ISC_BD_display_tx_payload);
        printf("\r\n");
    }            

    if (pdu_len <= (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
    {
        return IUC_SendPacket(mem_ref, IUC_STACK_UNIT_NEXT, Uplink_Downlink, SYS_DFLT_STK_TPLG_PACKET_TO_CPU_PRIORITY);
    }
    else
    {
        return ISC_SendFragmentPacket(mem_ref, IUC_STACK_UNIT_NEXT, Uplink_Downlink, SYS_DFLT_STK_TPLG_PACKET_TO_CPU_PRIORITY);
    }
}


/* FUNCTION NAME : ISC_FillHeader
 * PURPOSE: 
 *          Fill in the header of the ISC's PDU , get PVC and sequence number
 * INPUT:   
 *          isc_header_p --  the ISC's header
 *          opcode       --  represent the transmitting type
 *          svc_id       --  Service ID, an calling entiry must have a Service ID to identify itself
 *          pdu_len      --  the pdu len include isc header and its payload
 *          priority     --  send packet priority
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE       -- complete filling in the ISC's header
 *          FALSE      -- ISC can't get an available PVC number 
 * NOTES:
 *          None
 */
static void ISC_FillHeader(ISC_Header_T *isc_header_p, ISC_Opcode_T opcode, UI8_T svc_id, UI32_T pdu_len, UI32_T priority)
{
    UI32_T  my_drv_unit_id;
    
    my_drv_unit_id = ISC_OM_GetMyDrvUnitId();
    
    isc_header_p->version  = ISC_VERSION;   
    isc_header_p->src_unit = (UI8_T)my_drv_unit_id;
    isc_header_p->sid      = svc_id;        
    isc_header_p->opcode   = (UI8_T) opcode;
    isc_header_p->priority = priority;
    isc_header_p->fragment = ISC_NO_FRAGMENT;
    isc_header_p->data_len = (UI16_T) (pdu_len - ISC_HEADER_LEN) ;    
 
    /* Only ISC_OP_RCC_REQUEST, ISC_OP_RMC_REQUEST and ISC_OP_RMC_RC_REQUEST
     * need to get a PVC number before sending packet
     */   
    if ( is_get_PVC[opcode] )
    {
        UI32_T pvc_no;
        BOOL_T ret_value;

        do
        {
            ret_value = ISC_OM_PopPvcNo(&pvc_no);
            if (!ret_value)
            {
                printf("ISC can't get an available PVC number");
                SYSFUN_Sleep(NO_PVC_SLEEP_TIME);
            }
        }while(!ret_value);

        isc_header_p->pvc_no = (UI8_T) pvc_no; 
        ISC_OM_GetNextSeqNo(pvc_no, &isc_header_p->seq_no);
    }
    return ;
}


/* FUNCTION NAME:   ISC_SendFragmentPacket
 * PURPOSE: 
 *          Send an ISC fragment packet
 * INPUT:   
 *          mem_ref          -- points to the data block that holds the packet content
 *          dest_unit        -- unit_id or IUC_STACK_UNIT_NEXT, IUC_STACK_UNIT_ALL
 *          Uplink_Downlink  -- LAN_TYPE_TX_UP_LINK: Uplink
 *                              LAN_TYPE_TX_DOWN_LINK: Downlink
 *                              LAN_TYPE_TX_UP_DOWN_LINKs: Uplink and Downlink 
 *          priority         -- the send packet priority
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 *          1. support only fragment to two packets
 *          2. do not check pdu len in mem_ref, caller have to make sure that it is
 *             larger than one packet but smaller than two packets
 */
static BOOL_T ISC_SendFragmentPacket(L_MM_Mref_Handle_T *mem_ref, UI32_T dst_unit, UI8_T Uplink_Downlink, UI32_T priority)
{
    L_MM_Mref_Handle_T *mem_ref_ext_p;
    ISC_Header_T       *isc_header_p;
    
    UI32_T  pdu_len_ext;
    UI32_T  pdu_len;
    UI8_T   *pdu_ext_p;
    UI8_T   *pdu_p;
    BOOL_T  ret_value;

    pdu_p         = L_MM_Mref_GetPdu(mem_ref, &pdu_len);
    pdu_len_ext   = pdu_len - (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN);
    mem_ref_ext_p = L_MM_AllocateTxBuffer(pdu_len_ext, /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_ISCDRV, 
                                                       ISC_POOL_ID_DEFAULT, 
                                                       ISC_TRACE_ID_DEFAULT) /* user_id */);
    pdu_ext_p = L_MM_Mref_GetPdu(mem_ref_ext_p, &pdu_len);
    if (pdu_ext_p == NULL)
    {
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return FALSE;
    }
    memcpy(pdu_ext_p, pdu_p + (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN), pdu_len_ext);
    pdu_ext_p = L_MM_Mref_MovePdu(mem_ref_ext_p, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
    memcpy(pdu_ext_p, pdu_p, ISC_HEADER_LEN);
    
    isc_header_p = (ISC_Header_T *)pdu_p;
    isc_header_p->fragment = ISC_FIRST_FRAGMENT;
    isc_header_p = (ISC_Header_T *)pdu_ext_p;
    isc_header_p->fragment = ISC_SECOND_FRAGMENT;

    L_MM_Mref_SetPduLen(mem_ref, (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN));
    ret_value = IUC_SendPacket(mem_ref, dst_unit, Uplink_Downlink, priority);
    ret_value = ret_value & IUC_SendPacket(mem_ref_ext_p, dst_unit, Uplink_Downlink, priority);

    return ret_value;
}


/* FUNCTION NAME:  ISC_Find_1st_Fragment_Packet_And_Assembly
 * PURPOSE: 
 *          Find the first fragment packet and assemble to form original packet
 * INPUT:   
 *          mem_ref     -- points to the data block that holds the packet content
 * OUTPUT:  
 *          None
 * RETURN:  
 *          assembled packet's mref or NULL if fail
 * NOTES:
 */
static L_MM_Mref_Handle_T *ISC_Find_1st_Fragment_Packet_And_Assembly(L_MM_Mref_Handle_T *mem_ref_p)
{
    UI32_T               pdu_len;
    L_MM_Mref_Handle_T   *new_mem_ref_p;
    UI8_T                *new_pdu_p;
    L_MM_Mref_Handle_T   *first_mem_ref_p;
    ISC_Header_T         *first_hdr_p; 
    ISC_Header_T         *second_hdr_p;

    first_mem_ref_p = ISC_OM_Find1stFragment(mem_ref_p);

    if (first_mem_ref_p != NULL)
    {
        first_hdr_p  = (ISC_Header_T *)L_MM_Mref_GetPdu(first_mem_ref_p, &pdu_len);
        second_hdr_p = (ISC_Header_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);

        /* We use L_MM_AllocateTxBuffer (which can allocate up to 1800 bytes) 
         * to allocate memory, it satisfy current usage. If we have to allocate
         * more than 1800 bytes in future, need to implement/use new allocate API
         */
        new_mem_ref_p = L_MM_AllocateTxBuffer(first_hdr_p->data_len, /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_ISCDRV, 
                                                           ISC_POOL_ID_DEFAULT, 
                                                           ISC_TRACE_ID_DEFAULT) /* user_id */);
        new_pdu_p = L_MM_Mref_MovePdu(new_mem_ref_p, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
        if (new_pdu_p == NULL)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&first_mem_ref_p);
            L_MM_Mref_Release(&mem_ref_p);
            return NULL;
        }
        else
        {
            memcpy(new_pdu_p, first_hdr_p, SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN);
            memcpy(new_pdu_p + SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN,
                   ((UI8_T*)second_hdr_p) + ISC_HEADER_LEN, 
                   first_hdr_p->data_len - SYS_ADPT_ISC_MAX_PDU_LEN);
            L_MM_Mref_Release(&first_mem_ref_p);
            L_MM_Mref_Release(&mem_ref_p);
            return new_mem_ref_p;
        }
    }
    else
    {
        L_MM_Mref_Release(&mem_ref_p);
        return NULL;
    }
}


/* Function   : ISC_PacketIncoming
 * Purpose    : it's a call back function and called once upon the ISC's PDU arrival form IUC
 * Parameters : 
 *              rx_port    -- the receive port of the source device
 *              mem_ref_p  -- the pointer to memroy reference of the coming packet
 * OUTPUT:         
 *          none
 * RETURN:
 *          none
 * NOTES:
 */
void ISC_PacketIncoming(UI32_T rx_port, L_MM_Mref_Handle_T *mem_ref_p)
{
    ISC_Header_T        *isc_header_p;
    UI32_T              pdu_len;
    WaitReply_Info_T    pvc_info;
    L_MM_Mref_Handle_T  *mref_handle_p;
    L_MM_Mref_Handle_T  *new_mem_ref_p;
    UI32_T              my_drv_unit_id;
    BOOL_T              release_mref_on_quit = TRUE;
    
    if ( (isc_header_p = L_MM_Mref_GetPdu(mem_ref_p, &pdu_len)) == NULL)
     { 
      ISC_BD_IncRxCounter(-1);
      goto QUIT;
	 }

    if (isc_header_p->sid >= ISC_SID_UNSUPPORTED) /* check whether service id is valid */
     {
      ISC_BD_IncRxCounter(-1);
      goto QUIT;
	 }
	else
	  ISC_BD_IncRxCounter(isc_header_p->sid);

    /* discard the incoming packet and return if mgr is in Transition mode
     * except STKTPLG can use ISC API event in transition mode
     */
    if ((ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE) &&
        (isc_header_p->sid != ISC_STK_TPLG_SID))
    {
        ISC_BD_IncRxCounter(-1);
        goto QUIT;
    }

    my_drv_unit_id = ISC_OM_GetMyDrvUnitId();
        
    /* isc_header_p->dst_bmp == 0 means this packet is sent by ISC_SendPacketToNextHop()
     */
    if ((isc_header_p->dst_bmp !=0) && ((BIT_VALUE(my_drv_unit_id - 1) & isc_header_p->dst_bmp) == 0))
    {
        goto QUIT;
    }

    if (isc_header_p->fragment != ISC_NO_FRAGMENT) /* if it is a fragment packet */
    {
        if (isc_header_p->fragment == ISC_FIRST_FRAGMENT) /* first part of fragment packet */
        {
            ISC_OM_Save1stFragment(mem_ref_p);
            return;
        }
        else if (isc_header_p->fragment == ISC_SECOND_FRAGMENT) /* second part of fragment packet */
        {
            new_mem_ref_p = ISC_Find_1st_Fragment_Packet_And_Assembly(mem_ref_p);
            if (new_mem_ref_p != NULL)
            {
                mem_ref_p = new_mem_ref_p;
                if ( (isc_header_p = L_MM_Mref_GetPdu(mem_ref_p, &pdu_len)) == NULL)
                {
                    goto QUIT;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            printf("%s: receive invalid fragment packet!\n",__FUNCTION__);
            goto QUIT;
        }
    }

    /* L_MM_Mref_SetPduLen cannot be omit since isc payload may be very small. When
     * the packet is sent by NIC, the packet size will be padding to 64 bytes. So 
     * the receive packet size of remote unit may be wrong, we must set pdu size 
     * by length store in isc header
     */
    if (!L_MM_Mref_SetPduLen(mem_ref_p, ISC_HEADER_LEN + isc_header_p->data_len))
    {
        printf("%s: receive invalid ISC packet!\n",__FUNCTION__);
        goto QUIT;
    }
        
    /*  if it's opcode = ISC_OP_RCC_REQUEST or ISC_OP_UNRELIABLE_DELIVERY or ISC_OP_RMC_REQUEST
     *  and it's callback method is directcall. 
     */
    if ( !is_opcode_reply_type [isc_header_p->opcode]  )
    {
        ISC_CallbackToUpperCsc(mem_ref_p, rx_port);
        return;
    }

    /* check reply packet pvc_info 
     */
    if (ISC_OM_GetPvcInfo(isc_header_p->pvc_no, &pvc_info) == FALSE)
        goto CHECK_RESUME_TASK_AND_QUIT;

    if (pvc_info.seq_no != isc_header_p->seq_no)
        goto CHECK_RESUME_TASK_AND_QUIT;

    /* ISC_BD_capture_received_sid == ISC_SID_UNSUPPORTED mean capture all packets
     */
    if(ISC_BD_capture_received && ((ISC_BD_capture_received_sid == isc_header_p->sid )||(ISC_BD_capture_received_sid == ISC_SID_UNSUPPORTED)))
    {
        printf("\n");
        printf("================ISC_PacketIncoming===============\n");
        printf("*               Shown reply packet              *\n");
        printf("=================================================\n");
        ISC_BD_DisplayPacket(mem_ref_p, ISC_BD_display_rx_payload);
        printf("\r\n");
    }  
    
    switch (isc_header_p->opcode)
    {
        case ISC_OP_RCC_REPLY:
            if ( pvc_info.rep_buflen < isc_header_p->data_len)
                break;

            L_MM_Mref_MovePdu(mem_ref_p, ISC_HEADER_LEN, &pdu_len);
            pvc_info.mref_handle_offset = L_IPCMEM_GetOffset(mem_ref_p);
            pvc_info.wait_dst_bmp = pvc_info.dst_bmp = 0;
            if (TRUE == ISC_OM_SetPvcInfoWithSeqNoCheck(isc_header_p->pvc_no, &pvc_info, isc_header_p->seq_no))
            {
                release_mref_on_quit = FALSE;
            }
            break;            
        
        case ISC_OP_ACK:
            pvc_info.wait_dst_bmp &= ~BIT_VALUE(isc_header_p->src_unit-1);
            pvc_info.dst_bmp      &= ~BIT_VALUE(isc_header_p->src_unit-1);     
            ISC_OM_SetPvcInfo(isc_header_p->pvc_no, &pvc_info);
            break;
        
        case ISC_OP_NAK:
            pvc_info.wait_dst_bmp &= ~BIT_VALUE(isc_header_p->src_unit-1);
            ISC_OM_SetPvcInfo(isc_header_p->pvc_no, &pvc_info);
            break;
    }
                  
CHECK_RESUME_TASK_AND_QUIT:
    if(!pvc_info.wait_dst_bmp)
    {
        SYSFUN_ResumeTaskInTimeoutSuspense(pvc_info.tid);
    }

QUIT:
    if (TRUE == release_mref_on_quit)
        L_MM_Mref_Release(&mem_ref_p);
    return;
         
} /* ISC_PacketIncoming */


/* FUNCTION NAME:ISC_GetPacketType
 * PURPOSE: return the packet type according to service id in isc header
 * INPUT:   
 *          data_p     -- the packet content pointer
 * OUTPUT:  
 *          none
 * RETURN:  
 *          LAN_TYPE_PacketType_E
 * NOTES:
 */
LAN_TYPE_PacketType_T ISC_GetPacketType(UI8_T *data_p)
{
    ISC_Header_T   *isc_header_p = (ISC_Header_T*)data_p;

    if (data_p == NULL)
    {
        return LAN_TYPE_INVALID_PACKET_TYPE;
    }
    else if (is_opcode_reply_type[isc_header_p->opcode])
    {
        return LAN_TYPE_ISC_REPLY_PACKET;
    }
    else if (isc_header_p->sid == ISC_STK_TPLG_SID)
    {
        return LAN_TYPE_ISC_INTERNAL_STK_TPLG;
    }
    else if (isc_header_p->sid == ISC_LAN_CALLBYAGENT_SID ||
             isc_header_p->sid == ISC_LAN_DIRECTCALL_SID)
    {
        return LAN_TYPE_REMOTE_NETWORK_PACKET;
    }
    else if (isc_header_p->sid >= ISC_SID_UNSUPPORTED)
    {
        return LAN_TYPE_INVALID_PACKET_TYPE;
    }
    else
    {
        return LAN_TYPE_ISC_INTERNAL_OTHERS;
    }
}


/* FUNCTION NAME:ISC_CallbackToUpperCsc
 * PURPOSE: When ISC receive packet, it callback to upper CSCs by this API
 * INPUT:   
 *          mem_ref     -- the receive packet
 *          rx_port     -- destination port
 * OUTPUT:  
 *          none
 * RETURN:  
 *          none
 * NOTES:   1.When receive Multicast remote control packet, ISC needs reply ACK or NAK
 *            voluntarily.
 */
static void ISC_CallbackToUpperCsc(L_MM_Mref_Handle_T *mem_ref, UI32_T rx_port)
{
    ISC_Header_T    *isc_header_p;
    UI32_T          pdu_len;
    ISC_Key_T       key;
    BOOL_T          ret;
    UI8_T opcode, sid;
    
    /* get ISC's header  */
    isc_header_p = L_MM_Mref_GetPdu(mem_ref, &pdu_len );
    
    /* ISC_BD_capture_received_sid == ISC_SID_UNSUPPORTED mean capture all packets  */
    if(ISC_BD_capture_received && ((ISC_BD_capture_received_sid == isc_header_p->sid )||(ISC_BD_capture_received_sid == ISC_SID_UNSUPPORTED)))
    {
        printf("\n");
        printf("==================ISC_CallbackToUpperCsc=============\n");
        printf("*                   Shown ISC packet                *\n");
        printf("=====================================================\n");
        ISC_BD_DisplayPacket(mem_ref, ISC_BD_display_rx_payload);
        printf("\r\n");
    }            
    
    /* move the pdu position before callback to upper layer  */
    L_MM_Mref_MovePdu(mem_ref, ISC_HEADER_LEN, &pdu_len);
        
    key.unit     = isc_header_p->src_unit;
    key.pvc_no   = isc_header_p->pvc_no;
    key.seq_no   = isc_header_p->seq_no; 
    key.priority = isc_header_p->priority;
    
    mem_ref->current_usr_id = SYS_MODULE_ISCDRV;
    mem_ref->next_usr_id    = ServiceId2ModuleId[isc_header_p->sid];

    opcode = isc_header_p->opcode;
    sid = isc_header_p->sid;

    if (isc_header_p->sid == ISC_STK_TPLG_SID)
    {
        /* third parameter of isc service function of STKTPLG is the rx_port of device  */
        ret = SYS_CALLBACK_MGR_ReceiveStktplgPacketCallback(SYS_MODULE_ISCDRV, &key, mem_ref, rx_port );
    }
    else
    {
        if ((ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
            || (isc_service_func[(UI32_T)isc_header_p->sid] == NULL))
        {
             L_MM_Mref_Release(&mem_ref);
             return;
        }    

        ret = isc_service_func[(UI32_T)isc_header_p->sid] (&key, mem_ref, isc_header_p->sid );
        /*there is a risk that task switch happens here, so mem_ref may be freed, isc_header_p may be garbage. 
                       from here, it should not ref mem_ref and isc_header_p any longer*/
    }
    
    if (opcode != ISC_OP_UNRELIABLE_DELIVERY )
    {
        if (!ret)
        {
            ISC_BD_IncTxReplyCounter(sid,1);
            ISC_ReplyEngine(&key, ISC_OP_NAK);  
        }
        else if (opcode == ISC_OP_RMC_REQUEST )
        {
            ISC_BD_IncTxReplyCounter(sid,0);
            ISC_ReplyEngine(&key, ISC_OP_ACK);
        }    
    }

    return;
} /* ISC_CallbackToUpperCsc */


/* FUNCTION NAME : ISC_GetStackAndUnitsInfo
 * PURPOSE: Get stack status, get exist and valid units bmp 
 * INPUT:   
 *          none
 * OUTPUT:  
 *          none
 * RETURN:  
 *          none
 * NOTES:
 */
static void ISC_GetStackAndUnitsInfo(void)
{
    UI32_T  drv_unit;
    UI32_T  my_drv_unit_id;
    UI16_T  exist_drv_units = 0;
    UI16_T  valid_drv_units = 0;
    BOOL_T  stack_status_normal;

    if (STKTPLG_POM_GetStackStatus(&stack_status_normal) == FALSE)
    {
        SYSFUN_Debug_Printf("Failed to get stack status.\r\n");
    }
    ISC_OM_SetStackStatusNormal(stack_status_normal);
 
    STKTPLG_POM_GetMyDriverUnit(&my_drv_unit_id);
    ISC_OM_SetMyDrvUnitId(my_drv_unit_id);

    /* find all exist driver units and valid driver units (exclude local unit)
     */
    for (drv_unit=1; drv_unit<=SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK; drv_unit++)
    {
        if (drv_unit == my_drv_unit_id)
            continue;
        if (STKTPLG_POM_DriverUnitExist(drv_unit))
            exist_drv_units |= IUC_STACK_UNIT_BMP(drv_unit);
        else
            continue;
        if (STKTPLG_POM_IsValidDriverUnit(drv_unit))
            valid_drv_units |= IUC_STACK_UNIT_BMP(drv_unit);
    }
    ISC_OM_SetExistDrvUnitBmp(exist_drv_units);
    ISC_OM_SetValidDrvUnitBmp(valid_drv_units);
}

/* FUNCTION NAME : ISC_MCastReply
 * PURPOSE: reply ISC_ACK or ISC_NAK
 *
 * INPUT:   key  -- key for ISC service
 *          type -- ISC_ACK/ISC_NAK
 * OUTPUT:  
 *          none
 * RETURN:
 *          TRUE    -- the reply message has been transmited 
 *          FALSE   -- the reply message has not been transmited 
 * Notes:
 *          1. reply function of ISC_SendMcstReliable
 *          2. return FALSE immediately in transition mode 
 *             in an assumption that StkCtrl won't call this API
 */
BOOL_T ISC_MCastReply(ISC_Key_T *key, UI8_T type)
{
    if(ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }    
    
    if ((type!=ISC_OP_ACK) && (type!=ISC_OP_NAK))
    {
        return FALSE;
    }

    return ISC_ReplyEngine(key, type);
}


/* FUNCTION NAME:   ISC_GetISCHeaderLen
 * PURPOSE: 
 *          Get the ISC_HEADER_LEN of ISC packet
 * INPUT:   
 *          None
 * OUTPUT:  
 *          length
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T ISC_GetISCHeaderLen(UI16_T *length)
{
    if (NULL == length)
        return FALSE;

    *length = (UI16_T)ISC_HEADER_LEN;
    return TRUE;
}


/* FUNCTION NAME : ISC_ReplyEngine
 * PURPOSE: reply ISC_ACK or ISC_NAK
 *
 * INPUT:   key        -- key for ISC service
 *          opcode     -- represent the transmitting type
 * OUTPUT:         
 *          none
 * RETURN:
 *          none
 * NOTES:
 */
static BOOL_T ISC_ReplyEngine(ISC_Key_T *key, ISC_Opcode_T opcode)
{
    L_MM_Mref_Handle_T  *mem_ref;
    ISC_Header_T        *isc_header_p;
    UI32_T              pdu_len;
    UI32_T              my_drv_unit_id;
    
    if ((key->unit == 0) || (key->unit > ISC_MAX_NBR_OF_UNIT))
    {
        return FALSE;
    }

    if (NULL == (mem_ref = L_MM_AllocateTxBuffer(0 /* no isc pdu */, 
                                                 L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                              ISC_POOL_ID_DEFAULT, 
                                                              ISC_TRACE_ID_DEFAULT))))  
    {
        return FALSE;
    }

    /* get ISC header for operation
     */
    isc_header_p = L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);

    my_drv_unit_id = ISC_OM_GetMyDrvUnitId();
        
    /* fill in the ISC header 
     */ 
    isc_header_p->version  = ISC_VERSION;
    isc_header_p->sid      = ISC_INTERNAL_SID; /* internal service ID of ISC */
    isc_header_p->opcode   = (UI8_T)opcode;            
    isc_header_p->pvc_no   = key->pvc_no;              
    isc_header_p->seq_no   = key->seq_no;             
    isc_header_p->fragment = ISC_NO_FRAGMENT;
    isc_header_p->data_len = 0;                   
    isc_header_p->src_unit = (UI8_T) my_drv_unit_id;
    isc_header_p->dst_bmp  = (UI16_T) BIT_VALUE(key->unit-1);
    
    return IUC_SendPacket(mem_ref, key->unit, LAN_TYPE_TX_UP_DOWN_LINKS, key->priority); 
}


/* FUNCTION NAME : ISC_RemoteReply
 * PURPOSE: ISC_RemoteReply is called to send a reply to a remote caller 
 * INPUT:
 *          mem_ref -- the pointer to memroy reference of the sending packet
 *          key     -- key for ISC service          
 * OUTPUT:   
 *          none
 * RETURN:
 *          TRUE    -- the reply message has been transmited 
 *          FALSE   -- the reply message has not been transmited 
 * Notes:
 *          reply function of ISC_RemoteCall
 */   
BOOL_T ISC_RemoteReply( L_MM_Mref_Handle_T *mem_ref, ISC_Key_T *key)
{
    ISC_Header_T    *isc_header_p;
    UI32_T          pdu_len;
    UI32_T          my_drv_unit_id;
    
    if(ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;        
    }
    
    if (L_MM_Mref_GetAvailSzBeforePdu(mem_ref) < SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN) 
    {
        L_MM_Mref_Release(&mem_ref);
        return FALSE;
    }        

    /* move mref, point to ISC header
     */
    isc_header_p = L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
    
    if(pdu_len > (2*SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
    {
        printf("%s():Packet needs to be fragmented more than two packet, pdu_len=%lu\n",__FUNCTION__,pdu_len-ISC_HEADER_LEN);
        L_MM_Mref_Release(&mem_ref);
        return FALSE;
    }
    
    my_drv_unit_id = ISC_OM_GetMyDrvUnitId();
        
    isc_header_p->version  = ISC_VERSION;                    
    isc_header_p->sid      = ISC_INTERNAL_SID;        
    isc_header_p->opcode   = ISC_OP_RCC_REPLY;
    isc_header_p->pvc_no   = key->pvc_no;
    isc_header_p->seq_no   = key->seq_no;
    isc_header_p->fragment = ISC_NO_FRAGMENT;
    isc_header_p->data_len = (UI16_T) (pdu_len - ISC_HEADER_LEN);
    isc_header_p->src_unit = (UI8_T) my_drv_unit_id;
    isc_header_p->dst_bmp  = BIT_VALUE( key->unit-1 );

    if (pdu_len <= (SYS_ADPT_ISC_MAX_PDU_LEN + ISC_HEADER_LEN))
    {
        return IUC_SendPacket(mem_ref, key->unit,LAN_TYPE_TX_UP_DOWN_LINKS, key->priority);
    }
    else
    {
        return ISC_SendFragmentPacket(mem_ref, key->unit,LAN_TYPE_TX_UP_DOWN_LINKS, key->priority);
    }
}
    

/* FUNCTION NAME : ISC_EnterMasterMode
 * PURPOSE: 
 *          ISC_EnterMasterMode is called to Enter Master Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_EnterMasterMode(void)
{
    ISC_OM_EnterMasterMode(); 
    ISC_GetStackAndUnitsInfo();

    return;
}  


/* FUNCTION NAME : ISC_EnterSlaveMode
 * PURPOSE: 
 *          ISC_EnterSlaveMode is called to Enter Slave Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_EnterSlaveMode(void)
{
    ISC_OM_EnterSlaveMode();
    ISC_GetStackAndUnitsInfo();
       
    return;
}
  
  void ISC_ProvisionComplete(void)
  {
	  ISC_GetStackAndUnitsInfo();
		 
	  return;
  }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

void ISC_HandleHotInsertion(void)
{
	ISC_GetStackAndUnitsInfo();
			 
	return;

}
void ISC_HandleHotRemoval(void)
{
	ISC_GetStackAndUnitsInfo();
			 
	return;

}
#endif

/* FUNCTION NAME : ISC_EnterTransitionMode
 * PURPOSE: 
 *          ISC_EnterTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_EnterTransitionMode(void)
{   
    ISC_OM_EnterTransitionMode();    
    return;
} 


/* FUNCTION NAME : ISC_SetTransitionMode
 * PURPOSE: 
 *          ISC_SetTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_SetTransitionMode(void)
{
    /* MGR's set transition mode 
     */
    ISC_OM_SetTransitionMode(); 

    ISC_OM_SetStackStatusNormal(FALSE);
    
    /* resume the suspended task
     */
    ISC_ResumeAllSuspendTask();

    return;
}


/* FUNCTION NAME : ISC_ResumeAllSuspendTask
 * PURPOSE: Resume all suspended tasks
 * INPUT:  
 *          None   
 * OUTPUT:  
 *          None  
 * RETURN:  
 *          None  
 * NOTES:
 */ 
static void ISC_ResumeAllSuspendTask(void)
{   
    UI32_T pvc_no;
    UI32_T task_id;
    
    /* Resume the suspended task
     */
    for (pvc_no=0; pvc_no<ISC_MAX_NBR_OF_PVC; pvc_no++)
    {
        if (ISC_OM_GetPvcInfoTaskId(pvc_no, &task_id) == FALSE)
            continue;
        if (task_id != 0)
        {
            if (SYSFUN_ResumeTaskInTimeoutSuspense(task_id) == FALSE)
            {
                //kh_shi assert(0);/*error*/
                printf("%s:SYSFUN_ResumeTaskInTimeoutSuspense return FALSE\n",__FUNCTION__);
            } 
        }
    }
    return;
}
    

/* FUNCTION NAME : ISC_Create_InterCSC_Relation
 * PURPOSE: 
 *          This function initiates all function pointer registration operations.
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ISC", 
                                                      SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
                                                      ISC_BD_BackDoorMenu);
    ISC_Register_Service_CallBack(ISC_INTERNAL_SID, ISC_BD_PacketIncoming);      

    return;
}


/* FUNCTION NAME : ISC_CheckValidDestUnit
 * PURPOSE: 
 *          check whether the version is consistent and the driver unit exists or not
 *          this function will turn off the invalid unit in *dst_bmp_p unit bitmap
 * INPUT:   
 *          dst_bmp_p   -- destination unit bitmap
 *          svc_id      -- Service ID, an calling entiry must have a Service ID to identify itself
 * OUTPUT:  
 *          dst_bmp_p   -- valid destination unit bitmap
 * RETURN:  
 *          TRUE        -- there exist valid unit in dst_bmp
 *          FALSE       -- all unit in dst_bmp are invalid 
 * NOTES:
 *          1. when svc_id = ISC_FS_SID or ISC_SYSDRV_SID or ISC_LEDDRV_SID, it doesn't need to check its version.
 *          2. IscStackStatusNormal = TRUE  : runtime code's version of "main boards" are consistent (normal state)
 *             IscStackStatusNormal = FALSE : runtime code's version of "main boards" are different (abnormal state)
 *             Master: compare all units, Slave: compare only with master
 */
static BOOL_T ISC_CheckValidDestUnit(UI16_T *dst_bmp_p, UI8_T svc_id)
{
    UI16_T  valid_dst_bmp   = 0;
    UI16_T  invalid_dst_bmp = 0;
    
    if(0 == *dst_bmp_p)
    {
        return FALSE;
    }
    
    if (TRUE == isc_abnormal_stack_allowable[svc_id])
    {
        valid_dst_bmp   = *dst_bmp_p & ISC_OM_GetExistDrvUnitBmp();
        invalid_dst_bmp = *dst_bmp_p & ~ISC_OM_GetExistDrvUnitBmp();
    }
    else
    {
        if (ISC_OM_GetStackStatusNormal() == TRUE)
        {
            valid_dst_bmp   = *dst_bmp_p & ISC_OM_GetValidDrvUnitBmp();   
            invalid_dst_bmp = *dst_bmp_p & ~ISC_OM_GetValidDrvUnitBmp();
        }
        else
        {
            valid_dst_bmp   = 0;
            invalid_dst_bmp = *dst_bmp_p;
        }
    }
    
    if (IUC_STACK_UNIT_ALL != *dst_bmp_p &&
        invalid_dst_bmp    != 0)
    {
        SYSFUN_Debug_Printf("%s(): dst_bmp = %04x, invalid_dst_bmp = %04x, svc_id = %u\n",__FUNCTION__, *dst_bmp_p, invalid_dst_bmp,svc_id);
    }

    *dst_bmp_p = valid_dst_bmp;
    
    if (valid_dst_bmp == 0)
    {
        return FALSE;
    }   
     
    return TRUE;   
}


/* FUNCTION NAME: ISC_BD_InputInteger
 * PURPOSE: 
 *          get an integer from console
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          The integer key in by user
 * NOTES:
 */
static UI32_T ISC_BD_InputInteger(void)
{
    #define ASCII_0  0x30
    #define ASCII_9  0x39

    UI32_T ret_int = 0;
    UI8_T  ch;

    while((ch=getchar()) != 0xD)
    {
        if ((ch < ASCII_0) || (ch > ASCII_9))
            continue;
        ch -= ASCII_0;
        printf("%d", ch);
        ret_int = ret_int * 10 + ch;
    }
    printf("\n");

    return ret_int;
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_DisplayDataByHex
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Print out raw data in HEX format
 * INPUT    : data   -- pointer to the data
 *            length -- data length that want to print
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None    
 * ----------------------------------------------------------------------------------*/
static void ISC_BD_DisplayDataByHex(UI8_T *data, UI32_T length)
{
    UI32_T i;

    #define BYTES_IN_GROUP  4
    #define BYTES_IN_LINE   (3*BYTES_IN_GROUP)

    for (i=0; i<length; i++)
    {
        if ((i%BYTES_IN_LINE) == 0)
        {
            printf("%04lx:",i);
        }
        if (((i%BYTES_IN_GROUP) == 0) && ((i%BYTES_IN_LINE) != 0))
        {
            printf("  ");
        }
        if ((i%BYTES_IN_GROUP) == 0)
        {
            printf("0x");
        }
        printf("%02x ", data[i]);
        if (((i+1)%BYTES_IN_LINE) == 0)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_DisplayPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Print out mref packet data
 * INPUT    : mem_ref         -- target mref handler
 *            is_show_payload -- TRUE/FALSE: show ISC payload or not
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None    
 * ----------------------------------------------------------------------------------*/
static void ISC_BD_DisplayPacket(L_MM_Mref_Handle_T *mem_ref, BOOL_T is_show_payload)
{
    UI8_T         *pkt;  
    UI32_T        pdu_len;
    ISC_Header_T  *isc_header_p;   
         
    isc_header_p = L_MM_Mref_GetPdu(mem_ref, (UI32_T*)&pdu_len);
    
    printf("version  :  %u\n", isc_header_p->version);     
    printf("opcode   :  %s\n", OpcodeName[isc_header_p->opcode]);
    printf("SID      :  %s\n", ServName[isc_header_p->sid]);
    printf("PVC No   :  %u\n", isc_header_p->pvc_no);     
    printf("seq No   :  %u\n", isc_header_p->seq_no);     
    printf("dst_bmp  :  %04x\n", isc_header_p->dst_bmp);    
    printf("data Len :  %u\n", isc_header_p->data_len);   
    printf("src unit :  %u\n", isc_header_p->src_unit);   
    printf("priority :  %u\n", isc_header_p->priority);   

    if(is_show_payload == TRUE)
    {
        printf("payload: \n");
        pkt = L_MM_Mref_MovePdu(mem_ref, ISC_HEADER_LEN, &pdu_len);
        ISC_BD_DisplayDataByHex(pkt, pdu_len);
        L_MM_Mref_MovePdu(mem_ref, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
    }
    
    printf("================================================\r\n");    
    printf("\n Report done! \n");        
    return;
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_Show_ISC_Status
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Show ISC component informations, including in what stacking mode,
 *            all registered callback funtions' addresses and all exist driver units
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/
static void ISC_BD_Show_ISC_Status(void)
{
    UI8_T index;
     
    printf("\n================ISC Service's Status Report================\n");
    switch(ISC_OM_GetOperatingMode())
    {
        case SYS_TYPE_STACKING_MASTER_MODE:
            printf("ISC is in MASTER mode\n");
            break;
        case SYS_TYPE_STACKING_SLAVE_MODE:
            printf("ISC is in SLAVE mode\n");
            break;
        case SYS_TYPE_STACKING_TRANSITION_MODE:
            printf("ISC is in TRANSTITION mode\n");
            break;
        default:
            printf("ISC is in invalid/unknown mode\n");
            break;           
    }
    
    printf("\n================Registered Clients=======================\n");    
    for (index = 0; index<ISC_SID_UNSUPPORTED; index++)
    {
        printf("        Service ID : %u\n", index);
        printf("        Service ID : %s\n", ServName[index]);
        if (isc_service_func[index] != NULL)
        {
            printf("        Service function entry point : %p\r\n",isc_service_func[index]);         
        }
        else
        {
            printf("        Service function entry point : NULL\r\n");         
        }
        printf("---------------------------------------------------------------\r\n");
    }           
    printf(" Total : %u registered client listed as above\n", index);
    
    printf("\n================Existed/Valid Driver Unit=================\n\n");    
    printf("\r\nExist driver unit = %x\r\n", ISC_OM_GetExistDrvUnitBmp());
    printf("\r\nValid driver unit = %x\r\n", ISC_OM_GetValidDrvUnitBmp());
    if (ISC_OM_GetStackStatusNormal()==TRUE)
        printf("\r\nIscStackStatusNormal = TRUE\r\n");
    else printf("\r\nIscStackStatusNormal = FALSE\r\n");
    printf("------------------------------------------------------------\r\n");
    
    return;
}
        

/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_CapturePacketToggle
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Toggle capture packet variable, display contain, and ask user to input sid
 * INPUT    : None
 * OUTPUT   : capture_packet  --- capture packet or not
 *            display_contain --- display packet content or not
 *            sid             --- cap which service id
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static void ISC_BD_CapturePacketToggle(BOOL_T *capture_packet, BOOL_T *display_contain, UI8_T *sid)
{
    UI8_T    ch;
    UI8_T    index;
    
    if (!(*capture_packet)) 
    { 
        printf("\r\n");
        printf("\nDo you want to capture which SID packet? \r\n");
        for (index = 0; index<ISC_SID_UNSUPPORTED; index++)
        {
            printf("(%u)%s\r\n",index,ServName[index]);
        }
        printf("(%u)Capture All\r\n",ISC_SID_UNSUPPORTED);
        printf("=================================================\n");
        printf("    select = ");
        
        *sid = ISC_BD_InputInteger();        
        if(*sid > ISC_SID_UNSUPPORTED)
        {
            printf("Input error! Do nothing!");
            return;
        }

        printf("\r\n");
        printf("Do you want to display payload of the packet? (Y/N) ");
    
        ch = getchar();
        printf ("%c\n", ch);
        switch (ch)
        {
            case 'y':                
            case 'Y':                
                *display_contain = TRUE;
                break;
            default:
                *display_contain = FALSE;
                break;
        }         
        printf("\r\n");
    }
    *capture_packet = !(*capture_packet);   
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_PacketIncoming
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Register to ISC to receive ISC packet with sid == ISC_INTERNAL_SID
 * INPUT    : key           --- ISC key of received packet
 *            mref_handle_p --- mref handler of received packet
 *            svc_id        --- service id, here it would always be ISC_INTERNAL_SID
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static BOOL_T ISC_BD_PacketIncoming(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id)
{    
    UI8_T               *rep_frame;
    UI8_T               *frame;  
    UI8_T               *req_frame;
    UI32_T              frame_len;      
    UI32_T              pdu_len;
    UI16_T              index;
    ISC_Header_T        *isc_header_p;
    L_MM_Mref_Handle_T  *mem_ref;
    

    frame = L_MM_Mref_GetPdu(mref_handle_p, &frame_len);        
    isc_header_p = L_MM_Mref_MovePdu(mref_handle_p, (I32_T)(0 - ISC_HEADER_LEN), &pdu_len);
    
    if (NULL == (req_frame = L_MM_Malloc( isc_header_p->data_len, 
                                          L_MM_USER_ID( SYS_MODULE_ISCDRV, 
                                                        ISC_POOL_ID_DEFAULT, 
                                                        ISC_TRACE_ID_DEFAULT) )))
    {
        printf("%s: L_MM_Malloc return NULL!\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);     
        return FALSE;
    }
    
    for(index = 0; index < isc_header_p->data_len; index++)
    {
        req_frame[index] = index;
    }
    
    if (memcmp(frame,req_frame,isc_header_p->data_len) != 0) 
    {
        printf("the received packet isn't correct!\n");
        printf("data_len = %u frame_len = %lu\n",isc_header_p->data_len,frame_len);
        printf("received frame = \n");
        ISC_BD_DisplayDataByHex(frame, isc_header_p->data_len);
        printf("\n");
        printf("expected frame = \n");
        ISC_BD_DisplayDataByHex(req_frame, isc_header_p->data_len);
        printf("\n");
        L_MM_Mref_Release(&mref_handle_p);
        L_MM_Free(req_frame);
        return FALSE;                    
    }
     
    L_MM_Free(req_frame);
    if(isc_header_p->opcode == ISC_OP_RCC_REQUEST) 
    {
        if (NULL == (mem_ref = L_MM_AllocateTxBuffer(5, 
                                                     L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                                 ISC_POOL_ID_DEFAULT, 
                                                                 ISC_TRACE_ID_DEFAULT))))                                                    
        {
            printf("%s: L_MM_AllocateTxBuffer return NULL!\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;    
        }                                       
        rep_frame = L_MM_Mref_GetPdu(mem_ref, &pdu_len);
        memcpy(rep_frame, "Hello", 5);
        ISC_RemoteReply(mem_ref, key);
        printf("\n\nReceive a remote call and send reply packet!(press enter)\n");
    }
    else if(isc_header_p->opcode == ISC_OP_RMC_RC_REQUEST)
    {
        ISC_MCastReply(key, ISC_OP_ACK);  
        printf("\n\nReceive a multicast reliable packet(ack by APP) and reply ACK!(press enter)\n"); 
    }
    else if(isc_header_p->opcode == ISC_OP_RMC_REQUEST)
    {
        printf("\n\nReceive a multicast reliable packet(ack by ISC) and return TRUE!(press enter)\n");
        L_MM_Mref_Release(&mref_handle_p);
        return TRUE;
    }   
    else if(isc_header_p->opcode == ISC_OP_UNRELIABLE_DELIVERY)
    {   
        printf("\n\nReceive a unreliably packet!(press enter)\n");
    }    
    else
    {
        printf("\n\nReceive a ISC packet with invalid opcode value!(press enter)\n");
    }
        
    if(L_MM_Mref_Release(&mref_handle_p)==L_MM_ERROR_RETURN_VAL)
    {
        printf("something goes wrong to release reference!\n");
    }
    
    return TRUE;        
}/* ISC_BD_PacketIncoming */


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_SendTestingISCsPDU
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Test ISC_Send() work fine or not
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static void ISC_BD_SendTestingISCsPDU(void)
{
    UI8_T               dst_unit;
    UI8_T               *req_frame;
    UI8_T	        unit_num = ISC_MAX_NBR_OF_UNIT+1;    
    UI16_T              dst_bmp; 
    UI16_T              isc_sdu_len; 
    UI16_T              index; 
    UI32_T              frame_len;    
    L_MM_Mref_Handle_T  *mem_ref;
      
    printf("\r\n");
    printf("================================================\r\n");  
    printf("||                                            ||\r\n");      
    printf("||               ISC Send Test                ||\r\n");
    printf("||                                            ||\r\n");         
    printf("================================================\r\n");    
    printf("How many unit do you want send? (max number:%d) ", ISC_MAX_NBR_OF_UNIT);

    unit_num = ISC_BD_InputInteger();
    dst_bmp = 0;

    if(unit_num>0 && unit_num < ISC_MAX_NBR_OF_UNIT)
    {
        printf("Which unit do you want to send to (1-%d)? \r\n", ISC_MAX_NBR_OF_UNIT);
        for(index = 1; index<=unit_num; index++)
        {
            printf("<%u> Unit: ", index);
            dst_unit = ISC_BD_InputInteger();
            
            if(0 < dst_unit && dst_unit <= ISC_MAX_NBR_OF_UNIT)
            {
                dst_bmp |= IUC_STACK_UNIT_BMP(dst_unit);
            }
            else
            {
                printf("Unit ID is out of range! Do nothing!\n"); 
                return;
            }           
        }	
    }     
    else if(unit_num == ISC_MAX_NBR_OF_UNIT)
    {
        dst_bmp = 0xFFFF; 
    }
    else
    {
        printf("Unit number is out of range! Do nothing!\n"); 
        return;
    }  
        
    printf(" Packet content size=>");
    isc_sdu_len = ISC_BD_InputInteger();

    if(isc_sdu_len == 0)    
    {
        printf("Lenght is 0, send nothing..\n");
        return;
    }  
    else if(isc_sdu_len > SYS_ADPT_ISC_MAX_PDU_LEN)
    {
        printf("Packet size is too large\n");
        return;
    }
    
    if (NULL == (mem_ref = L_MM_AllocateTxBuffer(isc_sdu_len, 
                                                 L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                              ISC_POOL_ID_DEFAULT, 
                                                              ISC_TRACE_ID_DEFAULT))))                                                    
    {
        printf("%s: L_MM_AllocateTxBuffer return NULL!\n",__FUNCTION__);
        return;    
    } 
    
    req_frame = L_MM_Mref_GetPdu (mem_ref, &frame_len);
    for(index = 0; index<isc_sdu_len; index++)
        req_frame[index] = index;
       
    if (TRUE == ISC_Send(dst_bmp, ISC_INTERNAL_SID, mem_ref, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY))
        printf(" Send successfully\n");
    else
        printf(" Send unsuccessfully\n");
    
    return;    
         
}/* ISC_BD_SendTestingISCsPDU */


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_SendTestingISCsPDUAndWaitReply
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Test ISC_RemoteCall() work fine or not
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static void ISC_BD_SendTestingISCsPDUAndWaitReply(void)
{
#if 0 /* JinhuaWei, 03 August, 2008 1:31:02 */
    UI8_T               ch;
#endif /* #if 0 */
    UI8_T               dst_unit;
    UI8_T               *req_frame;    
    UI8_T               rep_frame[5]="00000";
    UI8_T               expected_reply[5] = "Hello";
    UI16_T              isc_sdu_len;  
    UI16_T              retry_times; 
    UI16_T              timeout; 
    UI16_T              index;       
    UI32_T              strength_test;
    UI32_T              strength_test_index; 
    UI32_T              frame_len;     
    UI32_T              before_ticks;
    UI32_T              after_ticks;      
    L_MM_Mref_Handle_T  *mem_ref;
    BOOL_T              send_result;    
    
    printf("\r\n");
    printf("================================================\r\n");  
    printf("||                                            ||\r\n");      
    printf("||           ISC Remote Call Test             ||\r\n");
    printf("||                                            ||\r\n");         
    printf("================================================\r\n");
    printf("\r\n");
    printf("Which driver unit you want to send to (1-%d): ", ISC_MAX_NBR_OF_UNIT);

    dst_unit = ISC_BD_InputInteger();
   
    if((dst_unit <1) && (dst_unit > ISC_MAX_NBR_OF_UNIT))
    {
        printf("Unit ID is out of range! Do nothing!\n"); 
        return;     
    }
   
    printf(" Packet content size: ");
    isc_sdu_len = ISC_BD_InputInteger();
    
    if(isc_sdu_len == 0)    
    {
        printf("Lenght is 0, send nothing..\r\n");
        return;
    }       
    else if(isc_sdu_len > SYS_ADPT_ISC_MAX_PDU_LEN)
    {
        printf("Packet size is too large\n");
        return;
    }
    
    printf(" Retry times: ");
    retry_times = ISC_BD_InputInteger();
    
    printf(" Time to expire in ms: ");
    timeout = ISC_BD_InputInteger();
    
    printf(" Enter Strength test times (0 for forever, press ENTER to quit): ");
    strength_test = ISC_BD_InputInteger();
    
    for(strength_test_index = 0; (strength_test_index < strength_test)||(strength_test == 0); strength_test_index++)
    {                       
        printf("[%lu]: ISC Remote Call Test\r\n", strength_test_index+1);
      
        if (NULL == (mem_ref = L_MM_AllocateTxBuffer(isc_sdu_len, 
                                                     L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                                 ISC_POOL_ID_DEFAULT, 
                                                                 ISC_TRACE_ID_DEFAULT))))                                                    
        {
            printf("%s: L_MM_AllocateTxBuffer return NULL!\n",__FUNCTION__);
            return;    
        }
        req_frame = L_MM_Mref_GetPdu (mem_ref, &frame_len);
        for(index = 0; index<isc_sdu_len; index++)
            req_frame[index] = index;        
        
        before_ticks = SYS_TIME_GetSystemTicksBy10ms();
        send_result = ISC_RemoteCall(dst_unit, 
                                     ISC_INTERNAL_SID, 
                                     mem_ref,
                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                     5, rep_frame,
                                     retry_times, timeout);
        after_ticks = SYS_TIME_GetSystemTicksBy10ms();    

        if(send_result == TRUE)
        {
            printf("  Send successfully.\r\n");
            if(!memcmp(rep_frame, expected_reply, 5)) /*reply is exactly what we expected*/
            {
                printf("  Reply correctly, and took %lu ticks.\r\n", after_ticks - before_ticks);
            }
            else
            {
                printf("  Reply incorrectly ,payload of reply packet:\r\n"); 
                ISC_BD_DisplayDataByHex(rep_frame,5);
                printf("\r\n");                
            }
        }
        else
        {
            printf("  Send failed.\r\n");
        }
        printf("\r\n"); 
    
        /*to see if quit the for loop (press enter key will break the for-loop)
         */
        /* kh_shi
        ch = sysSerialPollRxBuff();
    
        if(ch == '\n')
        {
            printf("Stop test!\r\n");
            break;
        } */          
    }

    return;                    
}/* ISC_BD_SendTestingISCsPDUAndWaitReply */


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_SendTestingMCastPDUnWaitReply
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Test ISC_SendMcastReliable() work fine or not
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static void ISC_BD_SendTestingMCastPDUnWaitReply(void)
{
    UI8_T               ack;
    UI8_T               unit_num = ISC_MAX_NBR_OF_UNIT+1;
    UI8_T               dst_unit;
    UI8_T               *req_frame;    
    UI16_T              index;   
    UI16_T              dst_bmp;  
    UI16_T              isc_sdu_len;
    UI16_T              retry_times;
    UI16_T              timeout;
    UI16_T              ret_dst_bmp;
    UI32_T              strength_test;
    UI32_T              strength_test_index; 
    UI32_T              frame_len;       
    UI32_T              before_ticks;
    UI32_T              after_ticks;      
    L_MM_Mref_Handle_T  *mem_ref;
    BOOL_T              is_acked_by_app;
  
    printf("\r\n");
    printf("================================================\r\n");  
    printf("||                                            ||\r\n");      
    printf("||     ISC Send Multicast Reliable Test       ||\r\n");
    printf("||                                            ||\r\n");         
    printf("================================================\r\n");
    printf("\r\n");
    printf("How many unit do you want send? (max number:%d) ", ISC_MAX_NBR_OF_UNIT);

    unit_num = ISC_BD_InputInteger();
    dst_bmp = 0;

    if(unit_num>0 && unit_num < ISC_MAX_NBR_OF_UNIT)
    {
        printf("Which unit do you want to send to (1-%d)? \r\n", ISC_MAX_NBR_OF_UNIT);
        for(index = 1; index<=unit_num; index++)
        {
            printf("<%u> Unit: ", index);
            dst_unit = ISC_BD_InputInteger();
            
            if(0 < dst_unit && dst_unit <= ISC_MAX_NBR_OF_UNIT)
            {
                dst_bmp |= 1<<(dst_unit-1);  
            }
            else
            {
                printf("Unit ID is out of range! Do nothing!\n"); 
                return;
            }           
        }   
    }     
    else if(unit_num == ISC_MAX_NBR_OF_UNIT)
    {
        dst_bmp = 0xFFFF; 
    }
    else
    {
        printf("Unit number is out of range! Do nothing!\n"); 
        return;
    }       
    
    printf(" Packet content size: ");
    isc_sdu_len = ISC_BD_InputInteger();
    
    if(isc_sdu_len == 0)    
    {
        printf("Lenght is 0, send nothing..\r\n");
        return;
    }
    else if(isc_sdu_len > SYS_ADPT_ISC_MAX_PDU_LEN)
    {
        printf("Packet size is too large\n");
        return;
    }
    
    printf(" Retry times: ");
    retry_times = ISC_BD_InputInteger();
    
    printf(" Time to expire in ms: ");
    timeout = ISC_BD_InputInteger();
    
    printf(" Do you want to ACK by (0)ISC (1)APP? ");
    ack = ISC_BD_InputInteger();

    if(ack == 0)    
    {
        is_acked_by_app = FALSE;
    }
    else if(ack == 1)
    {
        is_acked_by_app = TRUE;
    }
    else    
    {
        printf("input error!!\n");
        return;
    }
    
    printf(" Enter Strength test times (0 for forever, press ENTER to quit): ");
    strength_test = ISC_BD_InputInteger();

    for(strength_test_index = 0; (strength_test_index < strength_test)||(strength_test == 0); strength_test_index++)
    {
        printf("[%lu]: ISC Send Multicast Reliable Test\r\n", strength_test_index+1); 
        if (NULL == (mem_ref = L_MM_AllocateTxBuffer(isc_sdu_len, 
                                                     L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                                 ISC_POOL_ID_DEFAULT, 
                                                                 ISC_TRACE_ID_DEFAULT))))                                                    
        {
            printf("%s(): L_MM_AllocateTxBuffer return NULL!\n", __FUNCTION__);
            return;    
        }        
        req_frame = L_MM_Mref_GetPdu (mem_ref, &frame_len);
        for(index = 0; index<isc_sdu_len; index++)
            req_frame[index] = index;         
        
        printf(" dst_bmp     : %x\n", dst_bmp);
        before_ticks = SYS_TIME_GetSystemTicksBy10ms();
        ret_dst_bmp = ISC_SendMcastReliable(dst_bmp, 
                                            ISC_INTERNAL_SID, 
                                            mem_ref,
                                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                            retry_times, 
                                            timeout,
                                            is_acked_by_app);
        after_ticks = SYS_TIME_GetSystemTicksBy10ms();
        printf(" failed dst_bmp : %x\n", ret_dst_bmp);
        
        /* to see if quit the for loop, press enter key will break the for-loop
         */
        /*kh_shi 
        ch = sysSerialPollRxBuff();
        if(ch == '\n')
        {
            printf("Stop test!\r\n");
            break;
        } */
    }

    return;  
}/* ISC_BD_SendTestingMCastPDUnWaitReply */


/* ----------------------------------------------------------------------------------
 * FUNCTION : ISC_BD_SendTestingISCPDUToNextHop
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Test ISC_SendPacketToNextHop() work fine or not
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ----------------------------------------------------------------------------------*/ 
static void ISC_BD_SendTestingISCPDUToNextHop(void)
{
    UI8_T               Uplink_Downlink;
    UI8_T               *req_frame;
    UI16_T              isc_sdu_len; 
    UI16_T              index;     
    UI32_T              frame_len;      
    L_MM_Mref_Handle_T  *mem_ref; 
    
    printf("\r\n");
    printf("================================================\r\n");  
    printf("||                                            ||\r\n");      
    printf("||     ISC Send Packet To Next Hop Test       ||\r\n");
    printf("||                                            ||\r\n");         
    printf("================================================\r\n");
    printf("\nDo you want send to 1.Uplink 2.Downlink 3.Uplink and Downlink ?");

    Uplink_Downlink = ISC_BD_InputInteger();
        
    if(Uplink_Downlink < 1 || Uplink_Downlink >3)
    {
        printf("input is out of range! Do nothing!\n"); 
        return;
    }  

    printf(" Packet content size=>");
    isc_sdu_len = ISC_BD_InputInteger();
    
    if(isc_sdu_len == 0)    
    {
        printf("Lenght is 0, send nothing..\n");
        return;
    }
    else if(isc_sdu_len > SYS_ADPT_ISC_MAX_PDU_LEN)
    {
        printf("Packet size is too large\n");
        return;
    }  

    if (NULL == (mem_ref = L_MM_AllocateTxBuffer(isc_sdu_len, 
                                                 L_MM_USER_ID(SYS_MODULE_ISCDRV,
                                                             ISC_POOL_ID_DEFAULT, 
                                                             ISC_TRACE_ID_DEFAULT))))                                                    
    {
        printf("%s(): L_MM_AllocateTxBuffer return NULL!\n", __FUNCTION__);
        return;    
    } 
    
    req_frame = L_MM_Mref_GetPdu (mem_ref, &frame_len);
    for(index = 0; index<isc_sdu_len; index++)
        req_frame[index] = index;

    if (TRUE == ISC_SendPacketToNextHop( mem_ref, Uplink_Downlink, ISC_INTERNAL_SID))
        printf(" Send successfully");
    else
        printf(" Send unsuccessfully");
    
    return; 
}/* ISC_BD_SendTestingISCPDUToNextHop */

    
/* FUNCTION NAME : ISC_BD_BackDoorMenu
 * PURPOSE:
 *      Display ISC back door available function and accept user seletion.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void ISC_BD_BackDoorMenu (void)
{
    int ch;
    BOOL_T eof = FALSE;
    
    while (! eof)
    {
        printf("\n\n");
        printf("===============ISC BackDoor Menu================\n");
        printf(" 0. Exit\n");
        printf(" 1. Show ISC status:\n");        
        printf(" 2. Toggle Show/Hide received packet"); if(ISC_BD_capture_received) printf("(Show):\n"); else printf("(Hide):\n");  
        printf(" 3. Toggle Show/Hide transmitted packet"); if(ISC_BD_capture_transmitted) printf("(Show):\n"); else printf("(Hide):\n");            
        printf(" 4. Send the testing ISC's PDU(unreliable send)\n");        
        printf(" 5. Send the testing ISC's PDU and wait reply(remote call)\n");
        printf(" 6. Send the testing ISC's PDU and wait ACK/NAK(multicast reliable)\n");
		printf(" 7. Send the testing ISC's PDU to next hop\n");
        printf(" 8. Show rx counter\n");
		printf(" 9. Clear rx counter\n");	
		printf(" a. Show tx counter\n");
		printf(" b. Clear tx counter\n");
        printf("=================================================\n");
        printf("    select = ");
        ch = getchar();

        printf ("%c\n", ch);

        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;   
            case '1' :
                ISC_BD_Show_ISC_Status();
                break;
            case '2':
                ISC_BD_CapturePacketToggle(&ISC_BD_capture_received,&ISC_BD_display_rx_payload,&ISC_BD_capture_received_sid);
                break;
            case '3':
                ISC_BD_CapturePacketToggle(&ISC_BD_capture_transmitted,&ISC_BD_display_tx_payload,&ISC_BD_capture_transmitted_sid);
                break;
            case '4':
                ISC_BD_SendTestingISCsPDU();
                break;   
            case '5':
                ISC_BD_SendTestingISCsPDUAndWaitReply();                
                break;   
            case '6':
                ISC_BD_SendTestingMCastPDUnWaitReply();                
                break;                                      
            case '7':
                ISC_BD_SendTestingISCPDUToNextHop();                
                break;  
			case '8':
                ISC_BD_ShowRxCounter();  
                ISC_BD_ShowTxReplyCounter();
                break;                                      
            case '9':
                ISC_BD_ClearRxCounter();  
                ISC_BD_ClearTxReplyCounter();
                break;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            case 'a':
                ISC_BD_ShowTxCounters();
                break;
             case 'b':
                ISC_OM_DB_ClearTxCounters();
                break;
#endif
            default:
                break;
        }               
    }
}

static BOOL_T ISC_BD_IncRxCounter(UI8_T type)
{  
   if( type >=ISC_SID_UNSUPPORTED)
   {
    ISC_BD_PKT_COUNTER[ISC_SID_UNSUPPORTED]++;
   	return FALSE;
   }
   ISC_BD_PKT_COUNTER[type]++;
   return TRUE;
   

}

static void ISC_BD_ShowRxCounter()
{
	UI8_T i;
	UI32_T allcounter = 0;
	printf("type                          counter\n");
	printf("=========================================\n");
	for(i =0 ; i< ISC_SID_UNSUPPORTED+1; i++)
	{
	   if(ISC_BD_PKT_COUNTER[i] != 0)
	   {
	   printf("%20s     %ld\n",ServName[i],ISC_BD_PKT_COUNTER[i]);
	   allcounter = allcounter +ISC_BD_PKT_COUNTER[i];
	   }
	}
	printf("=========================================\n");
	printf("All counter is    %ld\n",allcounter);
}


static void ISC_BD_ClearRxCounter()
{
  memset(ISC_BD_PKT_COUNTER,0,sizeof(ISC_BD_PKT_COUNTER));
}

static void ISC_BD_ShowTxReplyCounter()
{
	UI8_T i,j;
	UI32_T allcounter = 0;
	char * str;
	printf("Type            Opcode              counter\n");
	printf("=========================================\n");
	for(i =0 ; i< ISC_SID_UNSUPPORTED+1; i++)
	{
	  for(j=0;j<2;j++)
	  {
	   if(ISC_BD_PKT_TxReplyCounter[i][j] != 0)
	   {
	    if(j == 0)
	      str = "ISC_OP_ACK";
	    else
	      str = "ISC_OP_NAK";
	      
	   printf("%20s    %20s    %ld\n",ServName[i],str,ISC_BD_PKT_TxReplyCounter[i][j]);
	   allcounter = allcounter +ISC_BD_PKT_TxReplyCounter[i][j];
	   }
	  }
	}
	printf("=========================================\n");
	printf("All counter is    %ld\n",allcounter);
}

static void ISC_BD_ClearTxReplyCounter()
{
  memset(ISC_BD_PKT_TxReplyCounter,0,sizeof(ISC_BD_PKT_TxReplyCounter));
}

static BOOL_T ISC_BD_IncTxReplyCounter(UI8_T sid,UI8_T type)
{  
   if(sid >=ISC_SID_UNSUPPORTED)
   {
    ISC_BD_PKT_TxReplyCounter[ISC_SID_UNSUPPORTED][type]++;
   	return FALSE;
   }
   ISC_BD_PKT_TxReplyCounter[sid][type]++;
   return TRUE;
   

}

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
static void ISC_BD_ShowTxCounters()
{
	UI8_T i,j;
    UI32_T tx_counters[ISC_SID_UNSUPPORTED+1][2];
    char *name_str;
    
	ISC_OM_DB_GetTxCounters((UI32_T *)&tx_counters);
	printf("Type            Opcode              counter\n");
	printf("=========================================\n");
	for(i =0 ; i< ISC_SID_UNSUPPORTED+1; i++)
	{
	  for(j=0;j<2;j++)
	  {
	   if(j == 0)
	    name_str = "ISC_SendAndWaitReply";
	   else
	    name_str = "ISC_Send";
	    
	   if(tx_counters[i][j] != 0)
	     printf("%20s    %30s   %ld\n",ServName[i],name_str,tx_counters[i][j]);
	  }
	}
	printf("=========================================\n");
}
#endif

