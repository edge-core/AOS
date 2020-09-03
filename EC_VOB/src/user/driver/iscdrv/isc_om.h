#ifndef _ISC_OM_H
#define _ISC_OM_H

#include "sys_type.h"
#include "l_mm.h"
#include "sysrsc_mgr.h"

/*dst_bmp &0xf000 ==0xf000 means it is updated by stktplg modue , not by the reply,
 stktplg need the csc not to wait for the reply,and return to next state.
 if not add ISC_OM_LOCAL_UNIT_BIT , when op_code == ISC_OP_RCC_REQUEST it may cause exception
 */

#define ISC_OM_LOCAL_UNIT_BIT 0xf000

typedef struct
{
    UI32_T  magic;          /* use for checking legality */
    UI32_T  tid;            /* caller's task id          */
    I32_T   mref_handle_offset; /* only used by remote-call  */
    UI16_T  seq_no;         /* the sequence number       */
    UI16_T  rep_buflen;     /* only used by remote-call  */
    UI16_T  dst_bmp;        /* destination unit that haven't reply ACK */
    UI16_T  wait_dst_bmp;   /* destination unit that haven't reply */
} WaitReply_Info_T;

/* ISC_BD_OM_ControlBlock_T is used by ISC backdoor only
 */
typedef struct ISC_BD_OM_ControlBlock_S
{
    BOOL_T capture_received;
    BOOL_T display_rx_payload;
    UI8_T  capture_received_sid;

    BOOL_T capture_transmitted;
    BOOL_T display_tx_payload;
    UI8_T  capture_transmitted_sid;
} ISC_BD_OM_ControlBlock_T;

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_OM_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for ISC OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_OM_InitiateSystemResources(void);
 
/*---------------------------------------------------------------------------------
 * FUNCTION : ISC_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ISC OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void ISC_OM_AttachSystemResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : ISC_OM_InitateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
//BOOL_T ISC_OM_InitateProcessResource(void);

void ISC_OM_Init(void);

void ISC_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

SYS_TYPE_Stacking_Mode_T ISC_OM_GetOperatingMode(void);

void ISC_OM_SetTransitionMode(void);

void ISC_OM_EnterTransitionMode(void);

void ISC_OM_EnterMasterMode(void);

void ISC_OM_EnterSlaveMode(void);

UI32_T ISC_OM_GetMyDrvUnitId(void);

void ISC_OM_SetMyDrvUnitId(UI32_T drv_unit_id);

UI16_T ISC_OM_GetExistDrvUnitBmp(void);

void ISC_OM_SetExistDrvUnitBmp(UI16_T exist_drv_unit_bmp);

UI16_T ISC_OM_GetValidDrvUnitBmp(void);

void ISC_OM_SetDrvUnitBmp(UI16_T valid_drv_unit_bmp,UI16_T exist_drv_unit_bmp);
void ISC_OM_SetValidDrvUnitBmp(UI16_T valid_drv_unit_bmp);

BOOL_T ISC_OM_GetStackStatusNormal(void);

void ISC_OM_SetStackStatusNormal(BOOL_T stack_status_normal);

BOOL_T ISC_OM_PopPvcNo(UI32_T *pvc_no_p);

BOOL_T ISC_OM_PushPvcNo(UI32_T pvc_no);

BOOL_T ISC_OM_GetPvcInfo(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p);

BOOL_T ISC_OM_SetPvcInfo(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p);

BOOL_T ISC_OM_SetPvcInfoWithSeqNoCheck(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p, UI32_T seq_no);

#if 1/* anzhen.zheng, 6/25/2008 */
BOOL_T ISC_OM_GetSidDelayTick(UI32_T service_id, UI32_T *delay_tick);

BOOL_T ISC_OM_SetSidDelayTick(UI32_T service_id, UI32_T delay_tick);
#endif

BOOL_T ISC_OM_ClearPvcInfo(UI32_T pvc_no);

BOOL_T ISC_OM_GetPvcInfoWaitDstBmp(UI32_T pvc_no, UI16_T *wait_dst_bmp_p);

BOOL_T ISC_OM_SetPvcInfoWaitDstBmp(UI32_T pvc_no, UI16_T wait_dst_bmp);

BOOL_T ISC_OM_GetPvcInfoDstBmp(UI32_T pvc_no, UI16_T *dst_bmp_p);

BOOL_T ISC_OM_SetPvcInfoDstBmp(UI32_T pvc_no, UI16_T dst_bmp);

BOOL_T ISC_OM_GetPvcInfoWaitDstBmp(UI32_T pvc_no, UI16_T *wait_dst_bmp_p);

BOOL_T ISC_OM_GetPvcInfoTaskId(UI32_T pvc_no, UI32_T *task_id_p);

BOOL_T ISC_OM_GetPvcInfoMrefHandleOffset(UI32_T pvc_no,I32_T *mref_handle_offset);

BOOL_T ISC_OM_ClearPvcInfoMrefHandleOffset(UI32_T pvc_no);

/* FUNCTION NAME : ISC_OM_GetNextSeqNo
 * PURPOSE: Uniquely assign a transmit sequece number to a ISC PDU for a PVC
 * INPUT:   
 *          pvc_no  --  unique PVC number
 * OUTPUT:  
 *          none
 * RETURN:  
 *          Seqential no
 * NOTES:
 *          1.it's not necessay to protect tx_seq_nbr [pvc_no] because 
 *          this API is always call with different pvc_no 
 *         
 *          2.pvc_no = 0~SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM-1 correspond to
 *          tx_seq_nbr [0]~tx_seq_nbr [ISC_MAX_NBR_OF_PVC-1]
 */
BOOL_T ISC_OM_GetNextSeqNo(UI32_T pvc_no, UI16_T *seq_nbr_p);

L_MM_Mref_Handle_T *ISC_OM_Find1stFragment(L_MM_Mref_Handle_T *mem_ref_p);

BOOL_T ISC_OM_Save1stFragment(L_MM_Mref_Handle_T *mem_ref_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)

BOOL_T ISC_OM_DB_ClearTxCounters();

BOOL_T ISC_OM_DB_IncTxCounters(UI8_T sid,UI8_T type);

BOOL_T ISC_OM_DB_GetTxCounters(UI32_T *tx_counters);

#endif

ISC_BD_OM_ControlBlock_T* ISC_OM_DB_GetCBPtr(void);

#endif
