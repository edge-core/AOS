/* Module Name: STKTPLG_TX.C
 *
 * Purpose: 
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */

#define STKTPLG_BACKDOOR_OPEN

/* INCLUDE FILE DECLARATIONS 
 */
#if 0 
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sys_module.h"

#include "stkmgmt_type.h"
#include "stktplg_type.h"
#include "stktplg_task.h"
#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "stktplg_om_private.h"
#include "stktplg_timer.h"
#include "stktplg_tx.h"
#include "stktplg_engine.h"
#include "sysfun.h"
#include "stktplg_shom.h"

//#ifdef Use_BcmDrv
//#include "bcmdrv.h"
//#else
#include "dev_swdrv.h"
//#endif

#include "syslog_type.h"
#include "syslog_om.h"
#include "sys_time.h"
#include "lan_type.h"

#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif

#include "l_mm.h"
#include "l_math.h"
#include "sys_dflt.h"
#include "l_pt.h"

#ifdef STKTPLG_BACKDOOR_OPEN
#include "stktplg_backdoor.h"
#endif /* STKTPLG_BACKDOOR_OPEN */

//#define XGS_STACKING_TX_DEBUG
//#ifdef XGS_STACKING_TX_DEBUG
#define xgs_stacking_tx_debug(fmt, arg...) \
		   if (stktplg_tx_debug_mode)  \
				printf(fmt, ##arg)
//#else
//#define xgs_stacking_tx_debug(fmt, arg...)
//#endif

/* NAMING CONSTANT DECLARATIONS
 */
 
#define LOCAL_UNIT        1 
#define     STKTPLG_OPTIONMODULE_RESET      28
#define STKTPLG_MAINBORAD_ACK               21 

/* DATA TYPE DECLARATIONS
 */
static BOOL_T stktplg_tx_debug_mode= FALSE; 

/* MACRO FUNCTION DECLARATIONS
 */
#define STKTPLG_TX_ALLOCATETXBUFFER(size) \
    L_MM_AllocateTxBuffer(size, L_MM_USER_ID2(SYS_MODULE_STKTPLG, STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHBT))

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void STKTPLG_TX_FillStackingPortLinkStatus(STKTPLG_OM_HBT_0_1_Payload_T *payload_info);
#if (SYS_CPNT_STACKING == TRUE)
 
static void STKTPLG_TX_FillUnitInfo(UI8_T unit, STKTPLG_OM_HBT_0_1_Payload_T *unit_info); 

#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
static void STKTPLG_TX_FillTenGModuleIdInfo(UI8_T teng_module_id[SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT]);
#endif

static UI8_T  inc(UI8_T seq_no);
 
#endif

/* STATIC VARIABLE DECLARATIONS 
 */
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE) /* modified by Jinhua Wei ,to remove warning ,becaued the array is only used when such macro is closed */
static DEV_SWDRV_Device_Port_Mapping_T port_mapping[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
#endif


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : STKTPLG_TX_SendHBTType0
 * PURPOSE: This function sends out HBT type 0
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType0(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI8_T                   unit;
    UI16_T                  size = sizeof(STKTPLG_OM_HBT_0_1_T);
    BOOL_T 					is_originator;

    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
    
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hbt0_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;
    }

    memset(hbt0_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        hbt0_ptr->header.version = 1;
        hbt0_ptr->header.type = (NORMAL_PDU == mode) ? STKTPLG_HBT_TYPE_0 : STKTPLG_HBT_TYPE_0_REBOUND; 
        hbt0_ptr->header.next_unit = 1;
        
        if (LAN_TYPE_TX_UP_LINK == port)
        {
            hbt0_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_UP];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_UP]=inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_UP]);
        }
        else
        {
            hbt0_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_DOWN];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_DOWN] = inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_DOWN]);
        }

        hbt0_ptr->header.length = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
    }
    else
    {
		STKTPLG_OM_HBT_0_1_T    *hbt0_ptr_rx;

    	/* get HBT, it should be payload of IUC
    	 */
        hbt0_ptr_rx = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(hbt0_ptr_rx==NULL)
        {
            return;
        }
        
        if ( (hbt0_ptr_rx->header.type != STKTPLG_HBT_TYPE_0        ) &&
             (hbt0_ptr_rx->header.type != STKTPLG_HBT_TYPE_0_REBOUND) )
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError HBT Type.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hbt0_ptr, (UI8_T *)hbt0_ptr_rx, sizeof(STKTPLG_OM_HBT_0_1_T));
    }    	 	    	 	

    if ((NORMAL_PDU == mode) || (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_0))
    {
        /* Normal mode reply/relay, append unit information
         */
            
        /* array is indexed from zero, but unit id is indexed from one
         * it is necessary to minus one to acess correct entry of unit info of HBT
         */
        unit = (hbt0_ptr->header.next_unit - 1); 
            
        /* let next unit know its unit id
         */
        hbt0_ptr->header.next_unit++;
            
        /* fill in our unit information to HBT
         */
        STKTPLG_TX_FillUnitInfo(unit, &hbt0_ptr->payload[unit]);        
    }

    memcpy((UI8_T *)(&ctrl_info_p->prev_type_0), (UI8_T *)hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));
    

    if (NORMAL_PDU != mode)
    {
        /* Rebounce PDU, change header type ONLY
        */
        hbt0_ptr->header.type = STKTPLG_HBT_TYPE_0_REBOUND; 
    }
                
    /* recaculate checksum for this packet
     */
    hbt0_ptr->header.checksum = 0;
    hbt0_ptr->header.checksum = L_MATH_CheckSum(hbt0_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt0_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to next unit specified by port
     */

    xgs_stacking_tx_debug("\r\n ..... Send HBT Type 0 (%ld - %ld) [%d]\r\n", mode, port, hbt0_ptr->header.next_unit);
    
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	



/* FUNCTION NAME : STKTPLG_TX_SendHBTType1
 * PURPOSE: This function sends out HBT type 1
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType1(BOOL_T flag, L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    STKTPLG_OM_HBT_0_1_T    *hbt1_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    BOOL_T                  is_originator;
    int 					i,unit;
    
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
    
    mref_handle_tx = L_MM_AllocateTxBufferFromDedicatedBufPool(
        L_MM_TX_BUF_POOL_ID_STKTPLG_HBT1,
        L_MM_USER_ID2(SYS_MODULE_STKTPLG, STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHBT1));

    if(mref_handle_tx == NULL)
    {
        perror("\r\nL_MM_AllocateTxBufferFromDedicatedBufPool Fail.");

        return;
    }
        
    hbt1_ptr = (STKTPLG_OM_HBT_0_1_T *) L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    
    
    if (is_originator)
    {
        if (LAN_TYPE_TX_UP_LINK == port)
        {
            memcpy(hbt1_ptr, &ctrl_info_p->stable_hbt_up, sizeof(STKTPLG_OM_HBT_0_1_T));
            i = ctrl_info_p->total_units_up;
           	for (unit = 1; unit < ctrl_info_p->total_units_down && i < ctrl_info_p->total_units; unit++, i++)
           	{
                memcpy(&hbt1_ptr->payload[i], &ctrl_info_p->stable_hbt_down.payload[unit],
            	           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
           	}
            hbt1_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1] = inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1]);                  	
        }
        else /* if (DOWN_PORT == port) */
        {
            memcpy(hbt1_ptr, &ctrl_info_p->stable_hbt_down, sizeof(STKTPLG_OM_HBT_0_1_T));
            i = ctrl_info_p->total_units_down;
            for (unit = 1; unit < ctrl_info_p->total_units_up && i < ctrl_info_p->total_units; unit++, i++)
            {
                memcpy(&hbt1_ptr->payload[i], &ctrl_info_p->stable_hbt_up.payload[unit],
            	           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
            }
            hbt1_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1_DOWN];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1_DOWN] = inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1_DOWN]);    
        }        
        hbt1_ptr->payload[0].is_ring  = ctrl_info_p->is_ring ; 
        hbt1_ptr->payload[0].preempted = ctrl_info_p->preempted_master;
        hbt1_ptr->payload[0].button_pressed =  ctrl_info_p->button_pressed;     
        hbt1_ptr->header.type = (NORMAL_PDU == mode) ? STKTPLG_HBT_TYPE_1 : STKTPLG_HBT_TYPE_1_REBOUND;
        hbt1_ptr->header.next_unit = 1;
        #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetMyRuntimeFirmwareVer(hbt1_ptr->originator_runtime_fw_ver);
        #else
        STKTPLG_OM_GetMyRuntimeFirmwareVer(hbt1_ptr->originator_runtime_fw_ver);
        #endif
        hbt1_ptr->payload[0].expansion_module_exist=ctrl_info_p->expansion_module_exist;
        //printf("0:TX--HBT1:module_id=%d  expansion_module_exist=%d\r\n",hbt1_ptr->payload[0].expansion_module_id,hbt1_ptr->payload[0].expansion_module_exist);            
        hbt1_ptr->payload[0].expansion_module_ready=ctrl_info_p->expansion_module_ready;
        hbt1_ptr->payload[0].provision_completed_state = STKTPLG_OM_IsProvisionCompleted();
        hbt1_ptr->payload[0].board_id = ctrl_info_p->board_id;

        STKTPLG_TX_FillStackingPortLinkStatus(&hbt1_ptr->payload[0]);

        #if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
        STKTPLG_TX_FillTenGModuleIdInfo(hbt1_ptr->payload[0].teng_module_id);
        #endif

    }
    else
    {
    	STKTPLG_OM_HBT_0_1_T    *hbt1_ptr_rx;
    	
    	/* get HBT, it should be payload of IUC
    	 */
        hbt1_ptr_rx = (STKTPLG_OM_HBT_0_1_T*)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if (hbt1_ptr_rx == NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if ( (hbt1_ptr_rx->header.type != STKTPLG_HBT_TYPE_1        ) &&
             (hbt1_ptr_rx->header.type != STKTPLG_HBT_TYPE_1_REBOUND) )
        {
            /* should not happen, print out error message
             */        	 
            perror("\r\nError HBT Type.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;        	            
        }
        
        /* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hbt1_ptr, (UI8_T *)hbt1_ptr_rx, sizeof(STKTPLG_OM_HBT_0_1_T));		
    }         

    if ((NORMAL_PDU == mode) || (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1))
    {   
        /* fill in slave is ready or not
         */
        if (STKTPLG_OM_SlaveIsReady() &&
            (ctrl_info_p->state == STKTPLG_STATE_SLAVE))
        { 
            hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].slave_ready = TRUE;
        }            
        else
        {
            hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].slave_ready = FALSE;        	
        }

        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].button_pressed = ctrl_info_p->button_pressed;
        
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_id    = ctrl_info_p->expansion_module_id;
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_exist = ctrl_info_p->expansion_module_exist;
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_type  = ctrl_info_p->expansion_module_type;
        //printf("*******unit=%d expansion_module_type=%d  \r\n",hbt1_ptr->header.next_unit - 1,hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_type);
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_ready=ctrl_info_p->expansion_module_ready; 
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].board_id = ctrl_info_p->board_id;
        memcpy((UI8_T *)(hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].module_runtime_fw_ver),(UI8_T *)(ctrl_info_p->module_runtime_fw_ver),SYS_ADPT_FW_VER_STR_LEN + 1);
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
        STKTPLG_TX_FillTenGModuleIdInfo(hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].teng_module_id);
#endif
        STKTPLG_TX_FillStackingPortLinkStatus(&hbt1_ptr->payload[hbt1_ptr->header.next_unit-1]);

        hbt1_ptr->header.next_unit++;
    }

    if (NORMAL_PDU != mode)
    {
  /*      if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
            hbt1_ptr->header.next_unit--;
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_exist=ctrl_info_p->expansion_module_exist;
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_ready=ctrl_info_p->expansion_module_ready; 
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].expansion_module_type  = ctrl_info_p->expansion_module_type;
        hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].board_id = ctrl_info_p->board_id;
        memcpy((UI8_T *)(hbt1_ptr->payload[hbt1_ptr->header.next_unit - 1].module_runtime_fw_ver),(UI8_T *)(ctrl_info_p->module_runtime_fw_ver),SYS_ADPT_FW_VER_STR_LEN + 1);
        if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
            hbt1_ptr->header.next_unit++; */
        hbt1_ptr->header.type = STKTPLG_HBT_TYPE_1_REBOUND;
    }
    
    /* recalculate checksum for this packet
     */
    hbt1_ptr->header.checksum = 0;
    hbt1_ptr->header.checksum = L_MATH_CheckSum(hbt1_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt1_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/

    /* send out this packet to next unit specified by port
     */
  /*   if (flag)
        printf("\r\n ....... Send HBT Type 1 [%d] (%ld - %ld) [%d]\r\n", ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_1_DOWN],mode, port, STKTPLG_MGR_SlaveIsReady()); */

    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return; 	    	 	
}

/* FUNCTION NAME : STKTPLG_TX_SendHBTType2
 * PURPOSE: This function sends out HBT type 2
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          query_unit -- if mem_ref is NULL, means we need to send out query packet
 *                        we need this parameter to know which unit that we want
 *                        to query.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType2(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T query_unit, UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                     pdu_len;
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    STKTPLG_OM_HBT_2_T         *hbt2_ptr;
    L_MM_Mref_Handle_T         *mref_handle_tx;
    UI16_T                     size = sizeof(STKTPLG_OM_HBT_2_T);
    STK_UNIT_CFG_T             unit_cfg;
    BOOL_T                     is_originator;
    
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
    
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hbt2_ptr = (STKTPLG_OM_HBT_2_T *)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hbt2_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;
    }

    memset(hbt2_ptr, 0, size);   
    
    if (is_originator)
    {      
        /* fill header information
         */
        hbt2_ptr->header.version   = 1;
        hbt2_ptr->header.type      = STKTPLG_HBT_TYPE_2; 
        hbt2_ptr->header.next_unit = query_unit;

        if (LAN_TYPE_TX_UP_LINK == port)
        {
            hbt2_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2] = inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2]);
        }
        else
        {
            hbt2_ptr->header.seq_no = ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2_DOWN];
            ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2_DOWN] = inc(ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2_DOWN]);
        }

        hbt2_ptr->header.length    = size - sizeof(STKTPLG_OM_HBT_Header_T);    
                    	
    }
    else
    {
    	
    	STKTPLG_OM_HBT_2_T    *hbt2_ptr_rx;
    	    	
    	/* get HBT, it should be payload of IUC
    	 */
        hbt2_ptr_rx = (STKTPLG_OM_HBT_2_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if (hbt2_ptr_rx == NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (hbt2_ptr_rx->header.type != STKTPLG_HBT_TYPE_2)
        {
            /* should not happen, print out error message
             */        	            
            L_MM_Mref_Release(&mref_handle_tx);
            
            SYSFUN_LogMsg("STKTPLG_TX.C>Error HBT Type\n", 0, 0, 0, 0, 0, 0);
            
            return;
        }
        
        /* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hbt2_ptr, (UI8_T *)hbt2_ptr_rx, sizeof(STKTPLG_OM_HBT_2_T));
     
        /* DON'T touch this PDU if it is rebounce from edges
        */
        if (NORMAL_PDU == mode)   
        {
            /* check if this packet is used to query our configuration
             */
            if ( (hbt2_ptr->header.next_unit == ctrl_info_p->my_unit_id) &&
                 (ctrl_info_p->state == STKTPLG_STATE_SLAVE            ) )
            {
                #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                if (STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &unit_cfg))
                #else
                if (STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &unit_cfg))
                #endif
//                if (STKTPLG_MGR_GetDeviceInfo(LOCAL_UNIT, &unit_cfg))
                {
                	/* reply unit configuration to master
                	 */
                    memcpy(&hbt2_ptr->payload.unit_cfg, &unit_cfg, sizeof(STK_UNIT_CFG_T));
                    
                    /* set next unit to be master to indicate this packet
                     * contains valid information
                     */                    
                    hbt2_ptr->header.next_unit           = ctrl_info_p->stable_hbt_down.payload[0].unit_id;
                    hbt2_ptr->payload.unit_id            = ctrl_info_p->my_unit_id;
                    /*hbt2_ptr->payload.unit_cfg.next_unit = ctrl_info_p->next_stacking_unit;*/
                }            	        	
            }
        }
    }
    
    /* recaculate checksum for this packet
     */
    hbt2_ptr->header.checksum = 0;
    hbt2_ptr->header.checksum = L_MATH_CheckSum(hbt2_ptr, size);    	 	    	 	

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt2_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/
    
    /* send out this packet to next unit
     */

    xgs_stacking_tx_debug("\r\n .......... Send HBT Type 2 (%ld - %ld - %d)\r\n", mode, port, hbt2_ptr->header.next_unit);
    
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);

#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;      	    	 	
}



/* FUNCTION NAME : STKTPLG_TX_SendHello
 * PURPOSE: This function sends out Hello
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHello(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                    pdu_len;
    STKTPLG_OM_Ctrl_Info_T    *ctrl_info_p;
    STKTPLG_OM_HELLO_0_T      *hello_ptr;
  //  STKTPLG_OM_HBT_Header_T *hello_ptr;
    L_MM_Mref_Handle_T        *mref_handle_tx;
    UI16_T                    size;
    BOOL_T                    is_originator;
 
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif  
    
    size          = sizeof(STKTPLG_OM_HELLO_0_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
    
    /* allocate memory for Hello PDU, for copy-free, we should reserve
     * location for lower level (IUC).
     */
    /* buf = L_MEM_Allocate(length);
    
    if(buf == NULL)
    {
        perror("\r\nL_MEM_Allocate Fail.");
        assert(0);            
    }*/

    mref_handle_tx = L_MM_AllocateTxBufferFromDedicatedBufPool(
        L_MM_TX_BUF_POOL_ID_STKTPLG_HBT1,
        L_MM_USER_ID2(SYS_MODULE_STKTPLG, STKTPLG_TYPE_TRACE_ID_STKTPLG_TX_SENDHELLO));

    if(mref_handle_tx == NULL)
    {
        printf("\r\n%s %d:L_MM_AllocateTxBufferFromDedicatedBufPool failed.", __FUNCTION__,__LINE__);

        return;
    }
    /* default pdu_len of L_MM_AllocateTxBufferFromDedicatedBufPool is
     * set as sizeof(STKTPLG_TX_TYPE_HBT1_T), need to set as correct pdu_len.
     */
    if(FALSE==L_MM_Mref_SetPduLen(mref_handle_tx, size))
    {
        printf("\r\n%s:L_MM_Mref_SetPduLen failed.", __FUNCTION__);
        
        L_MM_Mref_Release(&mref_handle_tx);
        return;
    }

    hello_ptr = (STKTPLG_OM_HELLO_0_T *) L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);


    if (is_originator)
    {                
        /* fill header information
         */
        hello_ptr->header.version   = 1;
        hello_ptr->header.type      = STKTPLG_HELLO_TYPE_0; 
        hello_ptr->header.next_unit = 1;
        hello_ptr->header.seq_no    = ctrl_info_p->seq_no[mode];
        ctrl_info_p->seq_no[mode] = inc(ctrl_info_p->seq_no[mode]);
        hello_ptr->header.length    = sizeof(STKTPLG_OM_HELLO_0_T) - sizeof(STKTPLG_OM_HBT_Header_T);
        memcpy(hello_ptr->mac_addr,ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN); 
    //    printf(" Send Hello with mac=%x %x %x %x %x %x\n",hello_ptr->mac_addr[0],hello_ptr->mac_addr[1],hello_ptr->mac_addr[2],hello_ptr->mac_addr[3],hello_ptr->mac_addr[4],hello_ptr->mac_addr[5]);                 	
    }
    else
    {
		STKTPLG_OM_HELLO_0_T *hello_ptr_rx;

    	/* get HELLO PDU, it should be payload of IUC
    	 */
        hello_ptr_rx = (STKTPLG_OM_HELLO_0_T *) L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if (hello_ptr_rx == NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if ( hello_ptr_rx->header.type != STKTPLG_HELLO_TYPE_0 )
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError HELLO packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hello_ptr, (UI8_T *)hello_ptr_rx, sizeof(STKTPLG_OM_HELLO_0_T));
    }    	 	    	 	

    /* No matter this hello packet is initiated at this unit or just relayed, 
     * update tx_port.
     */
    hello_ptr->tx_up_dw_port = port;
            
    /* recaculate checksum for this packet
     */
    hello_ptr->header.checksum = 0;
    hello_ptr->header.checksum = L_MATH_CheckSum(hello_ptr, sizeof(STKTPLG_OM_HELLO_0_T));

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hello_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/

    //xgs_stacking_tx_debug("\r\n ..... Send Hello Type 0 (%d - %d)\r\n", mode, port);
    
    /* send out this packet to next unit
     */
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendHelloType1
 * PURPOSE: This function sends out Hello Type 1
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Type 1 Enquire
 *                  -- Type 1 Ready   
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHelloType1(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                 pdu_len;
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    STKTPLG_OM_HBT_0_1_T   *hello_ptr;
    L_MM_Mref_Handle_T     *mref_handle_tx;
    UI16_T                 length, size;
    BOOL_T                 is_originator;
  
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    
    size          = sizeof(STKTPLG_OM_HBT_0_1_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;

//return;
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hello_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hello_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;            
    }

    memset(hello_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        hello_ptr->header.type   = mode; 
        hello_ptr->header.length = size - sizeof(STKTPLG_OM_HBT_Header_T);
        length = hello_ptr->header.length;
        hello_ptr->payload[0].unit_id               = ctrl_info_p->my_unit_id;
        hello_ptr->payload[0].expansion_module_type = ctrl_info_p->expansion_module_type;
        hello_ptr->payload[0].expansion_module_id   = ctrl_info_p->expansion_module_id;
        hello_ptr->payload[0].start_module_id       = ctrl_info_p->start_module_id;
        hello_ptr->payload[0].chip_nums             = ctrl_info_p->chip_nums;
        hello_ptr->payload[0].board_id              = ctrl_info_p->master_unit_id;

        memcpy(hello_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);

    }
    else
    {
        STKTPLG_OM_HBT_0_1_T *hello_ptr_rx;

    	/* get HELLO PDU, it should be payload of IUC
    	 */
        hello_ptr_rx = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(hello_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if ( ((hello_ptr_rx->header.type != STKTPLG_HELLO_TYPE_1_ENQ) && (mode == STKTPLG_HELLO_TYPE_1_ENQ)) ||
             ((hello_ptr_rx->header.type != STKTPLG_HELLO_TYPE_1_RDY) && (mode == STKTPLG_HELLO_TYPE_1_RDY)) )
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError HELLO packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }

        length = hello_ptr_rx->header.length;

		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hello_ptr, (UI8_T *)hello_ptr_rx, sizeof(STKTPLG_OM_HBT_0_1_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    hello_ptr->header.checksum = 0;
    hello_ptr->header.checksum = L_MATH_CheckSum(hello_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hello_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/

    xgs_stacking_tx_debug("\r\n ..... Send Hello Type 1 (%d) [%d : %d]\r\n", mode, size, length);
    
    /* send out this packet to next unit
     */
    ISC_SendPacketToNextHop(mref_handle_tx, LAN_TYPE_TX_MAINBRD_TO_EXPANSION, ISC_STK_TPLG_SID);

#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME : STKTPLG_TX_SendTCN
 * PURPOSE: This function sends out TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (0) / DOWN (1) stacking port
 *          renumber-- TRUE - This TCN packet is due to renumber command
 *                     FALSE- This is a normal TCN packet
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T  port, BOOL_T renumber)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    STKTPLG_OM_HBT_Header_T *tcn_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI16_T                  size;
    BOOL_T                  is_originator;
 
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
   
    size          = sizeof(STKTPLG_OM_HBT_Header_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    tcn_ptr = (STKTPLG_OM_HBT_Header_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    
    if(tcn_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;          
    }
    
    memset(tcn_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        tcn_ptr->version          = 1;
        tcn_ptr->type             = STKTPLG_TCN; 
        tcn_ptr->next_unit        = ctrl_info_p->my_unit_id;
        tcn_ptr->seq_no           = ctrl_info_p->seq_no[STKTPLG_TCN];
        ctrl_info_p->seq_no[STKTPLG_TCN] = inc(ctrl_info_p->seq_no[STKTPLG_TCN]);
        tcn_ptr->length           = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
        tcn_ptr->masters_location = renumber? RENUMBER_PDU:NORMAL_PDU;
    }
    else
    {
		STKTPLG_OM_HBT_Header_T *tcn_ptr_rx;

    	/* get tcn PDU, it should be payload of IUC
    	 */
        tcn_ptr_rx = (STKTPLG_OM_HBT_Header_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(tcn_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (tcn_ptr_rx->type != STKTPLG_TCN)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError tcn packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)tcn_ptr, (UI8_T *)tcn_ptr_rx, sizeof(STKTPLG_OM_HBT_Header_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    tcn_ptr->checksum = 0;
    tcn_ptr->checksum = L_MATH_CheckSum(tcn_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)tcn_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to specific unit
     */
    if (is_originator) 
    	xgs_stacking_tx_debug("\r\n ..... Send TCN (%u )\r\n",  port);
     
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendTCNType1
 * PURPOSE: This function sends out TCN Type 1 packet
 * INPUT:   mref_handle_rx -- packet that we need to handle.
 *                            if NULL, means we should create a new one and send it.
 *          port           -- UP (0) / DOWN (1) stacking port
 *          exist_units    -- The units bmp for which master think there are exist
 * OUTPUT:  None.
 * RETURN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendTCNType1(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T  port, UI16_T exist_units)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    STKTPLG_OM_TCN_TYPE_1_T *tcn_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI16_T                  size;
    BOOL_T                  is_originator;
 
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
   
    size = sizeof(STKTPLG_OM_TCN_TYPE_1_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    tcn_ptr = (STKTPLG_OM_TCN_TYPE_1_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    
    if(tcn_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;          
    }
    
    memset(tcn_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        /* version 2: add ttl field
         */
        tcn_ptr->header.version          = 2;
        tcn_ptr->header.type             = STKTPLG_TCN_TYPE_1; 
        tcn_ptr->header.next_unit        = ctrl_info_p->my_unit_id;
        tcn_ptr->header.seq_no           = ctrl_info_p->seq_no[STKTPLG_TCN_TYPE_1];
        ctrl_info_p->seq_no[STKTPLG_TCN_TYPE_1] = inc(ctrl_info_p->seq_no[STKTPLG_TCN_TYPE_1]);
        tcn_ptr->header.length           = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
        tcn_ptr->header.masters_location = NORMAL_PDU;

        tcn_ptr->exist_units             = exist_units;
        tcn_ptr->ttl          = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK-1;
    }
    else
    {
        STKTPLG_OM_TCN_TYPE_1_T *tcn_ptr_rx;

    	/* get tcn PDU, it should be payload of IUC
    	 */
        tcn_ptr_rx = (STKTPLG_OM_TCN_TYPE_1_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(tcn_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (tcn_ptr_rx->header.type != STKTPLG_TCN_TYPE_1)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError tcn type 1 packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if(tcn_ptr->header.version==1)
        {
            printf("\r\nStop relay tcn type 1 HBT because older version HBT is received");
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }

        if(tcn_ptr_rx->ttl==0)
        {
            /* the ttl field of received tcn type 1 packet is 0,
             * should not relay this packet anymore.
             */
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }

        /* copy the received HBT packet to new created outgoing memory buffer
         */
        memcpy((UI8_T *)tcn_ptr, (UI8_T *)tcn_ptr_rx, sizeof(STKTPLG_OM_TCN_TYPE_1_T));
        /* decrease the ttl field by 1
         */
        tcn_ptr->ttl=tcn_ptr_rx->ttl-1;

    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    tcn_ptr->header.checksum = 0;
    tcn_ptr->header.checksum = L_MATH_CheckSum(tcn_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)tcn_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to specific unit
     */
    if (is_originator) 
    	xgs_stacking_tx_debug("\r\n ..... Send TCN Type 1(%u )\r\n",  port);
     
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
}	

#else
/* FUNCTION NAME : STKTPLG_TX_SendTCN
 * PURPOSE: This function sends out TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T  port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    STKTPLG_OM_HBT_Header_T *tcn_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI16_T                  size;
    BOOL_T                  is_originator;
 
    ctrl_info_p   = STKTPLG_OM_GetCtrlInfo();
    
    size          = sizeof(STKTPLG_OM_HBT_Header_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    tcn_ptr = (STKTPLG_OM_HBT_Header_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    
    if(tcn_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;          
    }
    
    memset(tcn_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        tcn_ptr->version          = 1;
        tcn_ptr->type             = STKTPLG_TCN; 
        tcn_ptr->next_unit        = ctrl_info_p->my_unit_id;
        tcn_ptr->seq_no           = ctrl_info_p->seq_no[STKTPLG_TCN];
        ctrl_info_p->seq_no[STKTPLG_TCN] = inc(ctrl_info_p->seq_no[STKTPLG_TCN]);
        tcn_ptr->length           = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
        tcn_ptr->masters_location = NORMAL_PDU; 
    }
    else
    {
		STKTPLG_OM_HBT_Header_T *tcn_ptr_rx;

    	/* get tcn PDU, it should be payload of IUC
    	 */
        tcn_ptr_rx = (STKTPLG_OM_HBT_Header_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(tcn_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (tcn_ptr_rx->type != STKTPLG_TCN)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError tcn packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)tcn_ptr, (UI8_T *)tcn_ptr_rx, sizeof(STKTPLG_OM_HBT_Header_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    tcn_ptr->checksum = 0;
    tcn_ptr->checksum = L_MATH_CheckSum(tcn_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)tcn_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to specific unit
     */
    if (is_originator) 
    	xgs_stacking_tx_debug("\r\n ..... Send TCN (%u )\r\n",  port);
     
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	
#endif

/* FUNCTION NAME : STKTPLG_TX_SendHBTType0Ack
 * PURPOSE: This function sends out HBT type 0
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType0Ack(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;

#ifdef STKTPLG_BACKDOOR_OPEN
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
#endif

    UI16_T                  size = sizeof(STKTPLG_OM_HBT_0_1_T);
    BOOL_T 					is_originator;

    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
    
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
        
    if(hbt0_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;          
    }
    memset(hbt0_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        hbt0_ptr->header.version   = 1;
        hbt0_ptr->header.type      = STKTPLG_HBT_TYPE_0_ACK; 
        hbt0_ptr->header.next_unit = 1;

        hbt0_ptr->header.length = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_tx);
        return;
    }    	 	    	 	

    /* recaculate checksum for this packet
     */
    hbt0_ptr->header.checksum = 0;
    hbt0_ptr->header.checksum = L_MATH_CheckSum(hbt0_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt0_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to next unit specified by port
     */

    xgs_stacking_tx_debug("\r\n ..... Send HBT T0 Ack (%ld - %ld)\r\n", mode, port);
    
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	


/* FUNCTION NAME : STKTPLG_TX_SendHBT
 * PURPOSE: This function sends out Halt
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBT(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode, UI8_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    STKTPLG_OM_HBT_Header_T *hbt_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI16_T                  size;
    BOOL_T 					is_originator;
 
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    size          = sizeof(STKTPLG_OM_HBT_Header_T);
 
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hbt_ptr = (STKTPLG_OM_HBT_Header_T *)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hbt_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;            
    }
    if (mode==1)
    {
        xgs_stacking_tx_debug("Send HBT1\n");
    }
    
    memset(hbt_ptr, 0, (size));

    if (is_originator)
    {                
        /* fill header information
         */
        hbt_ptr->version   = 1;
        hbt_ptr->type      = mode; 
        hbt_ptr->next_unit = 1;
        hbt_ptr->seq_no    = ctrl_info_p->seq_no[mode];
        ctrl_info_p->seq_no[mode] = inc(ctrl_info_p->seq_no[mode]);
        hbt_ptr->length    = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
    }
    else
    {
		STKTPLG_OM_HBT_Header_T *hbt_ptr_rx;

    	/* get tcn PDU, it should be payload of IUC
    	 */
        hbt_ptr_rx = (STKTPLG_OM_HBT_Header_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if (hbt_ptr_rx == NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (hbt_ptr_rx->type != mode)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError tcn packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hbt_ptr, (UI8_T *)hbt_ptr_rx, sizeof(STKTPLG_OM_HBT_Header_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    hbt_ptr->checksum = 0;
    hbt_ptr->checksum = L_MATH_CheckSum(hbt_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to specific unit
     */
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendResetPacketToExp
 * PURPOSE: 
 * INPUT :  None.
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendResetPacketToExp(void)
{   
    UI32_T                      pdu_len;
    Main2Module_ResetPacket_T   *hello_ptr;
    L_MM_Mref_Handle_T          *mref_handle_tx;
    UI16_T                      size;
       
    size          = sizeof(Main2Module_ResetPacket_T);   
	
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hello_ptr = (Main2Module_ResetPacket_T *)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hello_ptr == NULL)
    {
        printf(" \r\nSTKTPLG_TX_SendResetPacketToExp L_MM_Mref_GetPdu Fail.");
        return;     
    }

    memset(hello_ptr, 0, size);
   
    hello_ptr->pkt_type                 = STKTPLG_OPTIONMODULE_RESET; 
    printf("Send Reset Packet To Exp\r\n");
    ISC_SendPacketToNextHop(mref_handle_tx, LAN_TYPE_TX_MAINBRD_TO_EXPANSION, ISC_STK_TPLG_SID);
    return;
}    

/* FUNCTION NAME : STKTPLG_TX_SendPacketToExp
 * PURPOSE: This function sends out packet to option module
 * INPUT :  reset_cmd
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendPacketToExp(void)
{
    static DEV_SWDRV_Device_Port_Mapping_T main_port_mapping[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI32_T                 pdu_len;
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    Main2Module_Packet_T   *hello_ptr;
    L_MM_Mref_Handle_T     *mref_handle_tx;
    UI16_T                 size;
    Stacking_Info_T        *StackingInfo_p = STKTPLG_OM_GetStackingInfo();
    UI32_T                 port_num;
    
       
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    size          = sizeof(Main2Module_Packet_T);   
	
    /* send first half part of the port mapping
     */

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hello_ptr = (Main2Module_Packet_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hello_ptr == NULL)
    {
        printf(" \r\n@@STKTPLG_TX_SendPacketToExp L_MM_Mref_GetPdu Fail.");
        return;     
    }

    memset(hello_ptr, 0, size);
     
    hello_ptr->pkt_type                 =STKTPLG_MAINBORAD_ACK; 
    hello_ptr->unit_id                  = ctrl_info_p->my_unit_id;
    hello_ptr->module_id                = ctrl_info_p->expansion_module_id;    
    hello_ptr->master_unit_id           = ctrl_info_p->master_unit_id;
    if((ctrl_info_p->state==STKTPLG_STATE_STANDALONE)|| (ctrl_info_p->state==STKTPLG_STATE_MASTER_SYNC)||(ctrl_info_p->state==STKTPLG_STATE_MASTER))
            hello_ptr->is_master_ready          = STKTPLG_OM_IsProvisionCompleted()? TRUE:  FALSE;
    else if((ctrl_info_p->state==STKTPLG_STATE_SLAVE)||(ctrl_info_p->state==STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT))
            hello_ptr->is_master_ready          =ctrl_info_p->provision_completed_state;
    else    
            hello_ptr->is_master_ready          = FALSE;       
    STKTPLG_OM_GetAllUnitsPortMapping(main_port_mapping);    
    STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id,&port_num);

    hello_ptr->mainbrd_port_num=port_num;
    hello_ptr->start_unit_forportmapping=0; 
    memcpy(&hello_ptr->port_mapping, &main_port_mapping[0],sizeof(DEV_SWDRV_Device_Port_Mapping_T)*(UNITS_FOR_PORTMAPPING)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
    memcpy(&(hello_ptr->stack_info),StackingInfo_p,sizeof(Stacking_Info_T));
    
    ISC_SendPacketToNextHop(mref_handle_tx, LAN_TYPE_TX_MAINBRD_TO_EXPANSION, ISC_STK_TPLG_SID);
    
    /* send second half part of the port mapping
     */
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hello_ptr = (Main2Module_Packet_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hello_ptr == NULL)
    {
        printf(" \r\n@@STKTPLG_TX_SendPacketToExp L_MM_Mref_GetPdu Fail.");
        return;
    }

    memset(hello_ptr, 0, size);
                 
        hello_ptr->pkt_type                 = STKTPLG_MAINBORAD_ACK; 
        hello_ptr->unit_id                  = ctrl_info_p->my_unit_id;
        hello_ptr->module_id                = ctrl_info_p->expansion_module_id;    
        hello_ptr->master_unit_id           = ctrl_info_p->master_unit_id;
        if((ctrl_info_p->state==STKTPLG_STATE_STANDALONE)|| (ctrl_info_p->state==STKTPLG_STATE_MASTER_SYNC)||(ctrl_info_p->state==STKTPLG_STATE_MASTER))
            hello_ptr->is_master_ready          = STKTPLG_OM_IsProvisionCompleted()? TRUE:  FALSE;
        else if((ctrl_info_p->state==STKTPLG_STATE_SLAVE)||(ctrl_info_p->state==STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT))
            hello_ptr->is_master_ready          =ctrl_info_p->provision_completed_state;
        else    
            hello_ptr->is_master_ready          = FALSE;    
        hello_ptr->mainbrd_port_num=port_num;
        hello_ptr->start_unit_forportmapping=UNITS_FOR_PORTMAPPING;
        memcpy(&hello_ptr->port_mapping, &main_port_mapping[UNITS_FOR_PORTMAPPING],sizeof(DEV_SWDRV_Device_Port_Mapping_T)*(UNITS_FOR_PORTMAPPING)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
        memcpy(&hello_ptr->stack_info,StackingInfo_p,sizeof(Stacking_Info_T));

        
    ISC_SendPacketToNextHop(mref_handle_tx, LAN_TYPE_TX_MAINBRD_TO_EXPANSION, ISC_STK_TPLG_SID);
  
    return;
    
}	

void STKTPLG_TX_SendPDU(UI32_T mode, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI32_T                  pdu_len, size = sizeof(STKTPLG_OM_HBT_0_1_T);

    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hbt0_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");
        return;            
    }

    memset(hbt0_ptr, 0, size);
    
	/* copy the received HBT packet to new created outgoing memory buffer
	 */
    memcpy((UI8_T *)hbt0_ptr, (UI8_T *)(&ctrl_info_p->prev_type_0), sizeof(STKTPLG_OM_HBT_0_1_T));

    if (NORMAL_PDU == mode)
    {
        /* Rebounce PDU, change header type ONLY
        */
        hbt0_ptr->header.type = STKTPLG_HBT_TYPE_0; 
    }
    else
    {
        /* Rebounce PDU, change header type ONLY
        */
        hbt0_ptr->header.type = STKTPLG_HBT_TYPE_0_REBOUND; 
    }

    /* recaculate checksum for this packet
     */
    hbt0_ptr->header.checksum = 0;
    hbt0_ptr->header.checksum = L_MATH_CheckSum(hbt0_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hbt0_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to next unit specified by port
     */

    xgs_stacking_tx_debug("\r\n ..... Send HBT 0 Prev (%ld - %ld)\r\n", mode, port);
    
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);
    
#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendClosedLoopTCN
 * PURPOSE: This function sends out Closed Loop TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal
 *                  -- Bounce
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendClosedLoopTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode, UI8_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  pdu_len;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    STKTPLG_OM_HBT_3_T      *tcn_ptr;
    L_MM_Mref_Handle_T      *mref_handle_tx;
    UI16_T                  size=sizeof(STKTPLG_OM_HBT_3_T);
    BOOL_T 					is_originator;
 
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
 
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    tcn_ptr = (STKTPLG_OM_HBT_3_T *) L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(tcn_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;            
    }

    memset(tcn_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        tcn_ptr->header.version   = 1;
        tcn_ptr->header.type      = STKTPLG_CLOSED_LOOP_TCN; 
        tcn_ptr->header.next_unit = 1;
        tcn_ptr->header.seq_no    = ctrl_info_p->seq_no[STKTPLG_CLOSED_LOOP_TCN];
        ctrl_info_p->seq_no[STKTPLG_CLOSED_LOOP_TCN] = inc(ctrl_info_p->seq_no[STKTPLG_CLOSED_LOOP_TCN]);
        tcn_ptr->header.length    = size - sizeof(STKTPLG_OM_HBT_Header_T);
        
        memcpy(&tcn_ptr->payload, &ctrl_info_p->stable_hbt_up.payload[0], sizeof(STKTPLG_OM_HBT_0_1_Payload_T) );
        tcn_ptr->payload.total_units_up   = ctrl_info_p->total_units_up;
        tcn_ptr->payload.total_units_down = ctrl_info_p->total_units_down;
        tcn_ptr->payload.is_ring          = ctrl_info_p->is_ring;
		memcpy(tcn_ptr->payload.mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
    }
    else
    {
		STKTPLG_OM_HBT_3_T *tcn_ptr_rx;

    	/* get tcn PDU, it should be payload of IUC
    	 */
        tcn_ptr_rx = (STKTPLG_OM_HBT_3_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(tcn_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (tcn_ptr_rx->header.type != STKTPLG_CLOSED_LOOP_TCN)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError tcn packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)tcn_ptr, (UI8_T *)tcn_ptr_rx, sizeof(STKTPLG_OM_HBT_3_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    tcn_ptr->header.checksum = 0;
    tcn_ptr->header.checksum = L_MATH_CheckSum(tcn_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)tcn_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/


    /* send out this packet to specific port
     */
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);

#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendExpNotify
 * PURPOSE: This function sends out Hello Type 1
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Type 1 Enquire
 *                  -- Type 1 Ready   
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendExpNotify(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                 pdu_len;
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    STKTPLG_OM_HBT_0_1_T   *hello_ptr;
    L_MM_Mref_Handle_T     *mref_handle_tx;
    UI16_T                 size=sizeof(STKTPLG_OM_HBT_0_1_T);
    BOOL_T                 is_originator;
  
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
 
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    hello_ptr = (STKTPLG_OM_HBT_0_1_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(hello_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;            
    }

    memset(hello_ptr, 0, size);

    if (is_originator)
    {                
        /* fill header information
         */
        hello_ptr->header.type   = STKTPLG_EXP_NOTIFY; 
        hello_ptr->header.length = size - sizeof(STKTPLG_OM_HBT_Header_T);                    	
        hello_ptr->payload[0].unit_id               = ctrl_info_p->my_unit_id;
        hello_ptr->payload[0].expansion_module_type = ctrl_info_p->expansion_module_type;
        hello_ptr->payload[0].expansion_module_id   = ctrl_info_p->expansion_module_id;
        hello_ptr->payload[0].start_module_id       = ctrl_info_p->master_unit_id;
    }
    else
    {
        STKTPLG_OM_HBT_0_1_T *hello_ptr_rx;

    	/* get HELLO PDU, it should be payload of IUC
    	 */
        hello_ptr_rx = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(hello_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (hello_ptr_rx->header.type != STKTPLG_EXP_NOTIFY)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError Exp Notify packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)hello_ptr, (UI8_T *)hello_ptr_rx, sizeof(STKTPLG_OM_HBT_0_1_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    hello_ptr->header.checksum = 0;
    hello_ptr->header.checksum = L_MATH_CheckSum(hello_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)hello_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/

    xgs_stacking_tx_debug("\r\n ..... Send Exp Notify (%ld) [%d]\r\n", port, size);

    /* send out this packet to next unit
     */
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);

#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}	

/* FUNCTION NAME : STKTPLG_TX_SendTplgSync
 * PURPOSE: This function sends out topology sync.
 * INPUT:   mem_ref  -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 *          sync_bmp -- Unit bit map need to sync to.
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:   None.
 *          
 */
void STKTPLG_TX_SendTplgSync(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T port, UI8_T sync_bmp)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                 pdu_len;
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    STKTPLG_OM_TPLG_SYNC_T *tplg_sync_ptr;
    L_MM_Mref_Handle_T     *mref_handle_tx;
    UI16_T                 size=sizeof(STKTPLG_OM_TPLG_SYNC_T);
    BOOL_T                 is_originator;
  
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif 
    
    is_originator = (mref_handle_rx == NULL) ? TRUE : FALSE;
 
    mref_handle_tx = STKTPLG_TX_ALLOCATETXBUFFER(size);
    tplg_sync_ptr = (STKTPLG_OM_TPLG_SYNC_T*)L_MM_Mref_GetPdu(mref_handle_tx, &pdu_len);
    if(tplg_sync_ptr == NULL)
    {
        perror("\r\nL_MM_Mref_GetPdu Fail.");

        return;          
    }
    
    memset(tplg_sync_ptr, 0, pdu_len);

    if (is_originator)
    {                
        /* fill header information
         */
        tplg_sync_ptr->header.type   = STKTPLG_TPLG_SYNC; 
        tplg_sync_ptr->header.length = size - sizeof(STKTPLG_OM_HBT_Header_T);        
        
        tplg_sync_ptr->payload.src_unit_id     = ctrl_info_p->my_unit_id;
        tplg_sync_ptr->payload.unit_bmp_to_get = sync_bmp;
        STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &(tplg_sync_ptr->payload.unit_cfg));
    }
    else
    {
        STKTPLG_OM_TPLG_SYNC_T *tplg_sync_ptr_rx;

    	/* get HELLO PDU, it should be payload of IUC
    	 */
        tplg_sync_ptr_rx = (STKTPLG_OM_TPLG_SYNC_T*)L_MM_Mref_GetPdu(mref_handle_rx, &pdu_len);
        if(tplg_sync_ptr_rx==NULL)
        {
            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
        if (tplg_sync_ptr_rx->header.type != STKTPLG_TPLG_SYNC)
        {
            /* should not happen, print out error message
             */        	            
            perror("\r\nError Exp Notify packet.");

            L_MM_Mref_Release(&mref_handle_tx);
            return;
        }
        
		/* copy the received HBT packet to new created outgoing memory buffer
    	 */
		memcpy((UI8_T *)tplg_sync_ptr, (UI8_T *)tplg_sync_ptr_rx, sizeof(STKTPLG_OM_TPLG_SYNC_T));
    }    	 	    	 	
    
    /* recaculate checksum for this packet
     */
    tplg_sync_ptr->header.checksum = 0;
    tplg_sync_ptr->header.checksum = L_MATH_CheckSum(tplg_sync_ptr, size);

#ifdef STKTPLG_BACKDOOR_OPEN
	/* show the HBT packet
	 */	 
	STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_TXHBT, ctrl_info_p->state, (UI8_T *)tplg_sync_ptr);
#endif /*STKTPLG_BACKDOOR_OPEN*/

    xgs_stacking_tx_debug("\r\n ..... Send TPLG_SYNC: sync_bmp (%u)\r\n", sync_bmp);

    /* send out this packet to next unit
     */
    ISC_SendPacketToNextHop(mref_handle_tx, port, ISC_STK_TPLG_SID);

#endif /* (SYS_CPNT_STACKING == TRUE) */
    
    return;
    
}


/* LOCAL SUBPROGRAM BODIES 
 */

/* FUNCTION NAME : STKTPLG_TX_FillStackingPortLinkStatus
 * PURPOSE: Get link state and fill them to stacking_ports_link_status 
 * INPUT:   None.
 * OUTPUT:  payload_info - target payload to be updated.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
static void STKTPLG_TX_FillStackingPortLinkStatus(STKTPLG_OM_HBT_0_1_Payload_T *payload_info)
{
    UI32_T up_state,  down_state;
    UI8_T  stacking_ports_link_status=0;    

    if(FALSE==STKTPLG_SHOM_GetHgPortLinkState(&up_state, &down_state))
    {
        printf("%s:STKTPLG_SHOM_GetHgPortLinkState fail\r\n", __FUNCTION__);
    }

    if(up_state == DEV_SWDRV_PORT_LINK_UP)
        stacking_ports_link_status |= LAN_TYPE_TX_UP_LINK; 
    else
        stacking_ports_link_status &= ~LAN_TYPE_TX_UP_LINK; 
   
    if(down_state == DEV_SWDRV_PORT_LINK_UP)
        stacking_ports_link_status |= LAN_TYPE_TX_DOWN_LINK; 
    else
        stacking_ports_link_status &= ~LAN_TYPE_TX_DOWN_LINK; 

    payload_info->stacking_ports_link_status = stacking_ports_link_status;
    
    return;
}



#if (SYS_CPNT_STACKING == TRUE)

static void STKTPLG_TX_FillUnitInfo(UI8_T unit, STKTPLG_OM_HBT_0_1_Payload_T *unit_info)
{
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif  
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)
    UI32_T i;             /* modified by Jinhua Wei ,to remove warning ,becaued the variable is only used when such macro is closed */
#endif
    
    /* fill in my mac address
     */
    memcpy(unit_info->mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
    	
    /* fill in image type, chip nums using for this unit and board id
     */	
    unit_info->image_type = ctrl_info_p->image_type;
    unit_info->chip_nums  = ctrl_info_p->chip_nums;
    unit_info->board_id   = ctrl_info_p->board_id;
    
    /* this field is assigned by master to slave unit, 
     * it is no meaning when we fill in information, just assign zero
     */
    unit_info->start_module_id = 0;
    
    /* shold not be ready when we fill in our information at first time
     */
    unit_info->slave_ready = FALSE;
    
    unit_info->expansion_module_type  = ctrl_info_p->expansion_module_type;

    
    unit_info->button_pressed = ctrl_info_p->button_pressed;
    unit_info->preempted        = ctrl_info_p->preempted;
    unit_info->preempted_master = ctrl_info_p->preempted_master;

    STKTPLG_TX_FillStackingPortLinkStatus(unit_info);
    
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
    STKTPLG_TX_FillTenGModuleIdInfo(unit_info->teng_module_id);
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
	memcpy(unit_info->past_master_mac, ctrl_info_p->past_master_mac, STKTPLG_MAC_ADDR_LEN);
#else
    STKTPLG_OM_GetPortMapping(&port_mapping[0], ctrl_info_p->my_unit_id);
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; i++)
    {
        unit_info->port_mapping[i].mod_dev_id          = (port_mapping[i].module_id      & 0x1f);
        unit_info->port_mapping[i].mod_dev_id         |= (port_mapping[i].device_id      & 0x07) << 5;
        unit_info->port_mapping[i].device_port_phy_id  = (port_mapping[i].device_port_id & 0xff);
        //unit_info->port_mapping[i].device_port_phy_id |= (port_mapping[i].phy_id         & 0x0f);
        unit_info->port_mapping[i].port_type           = port_mapping[i].port_type;
    }
#endif   
//    STKTPLG_MGR_GetPortMapping((DEV_SWDRV_Device_Port_Mapping_T *)&unit_info->port_mapping, LOCAL_UNIT);

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    unit_info->stacking_port_option = ctrl_info_p->stacking_port_option;
#endif

    return;
}	

#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
static void STKTPLG_TX_FillTenGModuleIdInfo(UI8_T teng_module_id[SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT])
{

    UI8_T module_slot_index, module_id;

    for(module_slot_index=0; module_slot_index<SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT; module_slot_index++)
    {
        module_id=0;

        if(STKTPLG_BOARD_GetModuleIDByModuleIndex(module_slot_index, &module_id)==TRUE)
        {
            xgs_stacking_tx_debug("%s(%d):Module slot %hu module id:%hu\r\n", __FUNCTION__, __LINE__, module_slot_index, module_id);
        }
        teng_module_id[module_slot_index]=module_id;
    }
}
#endif

BOOL_T STKTPLG_TX_GetDebugMode(void)
{
    return stktplg_tx_debug_mode;
}

void STKTPLG_TX_ToggleDebugMode(void)
{
    stktplg_tx_debug_mode = !stktplg_tx_debug_mode;
}

static UI8_T  inc(UI8_T seq_no)
{
    if (seq_no == 255)
        return(0) ;
    else
        return(++seq_no);
}
#endif

