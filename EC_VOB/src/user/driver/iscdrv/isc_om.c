#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sysfun.h"
#include "isc_om.h"
#include "isc.h"
#include "sysrsc_mgr.h"
#include "sys_bld.h"
#include "l_math.h"
#include "l_stack.h"
#include "l_mm.h"
#include "l_ipcmem.h"
#include "dev_nicdrv_om.h"

#define ISC_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(isc_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define ISC_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(isc_om_sem_id)
#define ATOM_EXPRESSION(exp) { ISC_OM_ENTER_CRITICAL_SECTION();\
                               exp;\
                               ISC_OM_LEAVE_CRITICAL_SECTION();\
                             }

typedef struct
{
    UI32_T  timeout_time;   /* time out time of this fragment packet */
    void   *packet;         /* NULL mean empty entry */
} Fragment_Packet_T;

typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T  is_allocated;
    UI32_T  my_drv_unit_id;
    UI16_T  exist_drv_units;     /* store exist units bitmap */
    UI16_T  non_exist_drv_units; /* ~exist_drv_units */
    UI16_T  valid_drv_units;     /* store valid(i.e. STKTPLG_OM_IsValidDriverUnit()==TRUE) units bitmap */
    UI16_T  non_valid_drv_units; /* ~valid_drv_units */

    /* IscStackStatusNormal = TRUE  : version of "main boards" are the same(normal state)
     * IscStackStatusNormal = FALSE : version of "main boards" are different(abnormal state)
     */
    BOOL_T  IscStackStatusNormal;
    
    WaitReply_Info_T    pvc_info [ISC_MAX_NBR_OF_PVC];
    ISC_ServiceFunc_T   isc_service_func[ISC_SID_UNSUPPORTED];
    
    UI32_T              last_fragment_index;
    Fragment_Packet_T   fragment_packet_list [SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER];

    /* tx_seq_nbr  keep the tx_seq_nbr for send packet.
     * remote_tx_seq_nbr keep the tx_seq_nbr which will be updated by a receive packet.
     * Note: pvc_no=0~ISC_MAX_NBR_OF_PVC-1
     */
    UI16_T                 tx_seq_nbr [ISC_MAX_NBR_OF_PVC];
   	/* anzhen.zheng, 6/25/2008. */
	UI32_T   isc_sid_delay_tick[ISC_SID_UNSUPPORTED]; /* retransmit delay tick for NAK*/
    L_STACK_ShMem_Desc_T   pvc_handle;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T   isc_db_tx_counters[ISC_SID_UNSUPPORTED+1][2];
#endif
    ISC_BD_OM_ControlBlock_T bd_cb; /* for ISC Backdoor */
    UI8_T                  pvc_buffer[0];

} ISC_Shmem_Data_T;

static const UI16_T FragmentTimeOut[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE] = {800,800,600,600,400,400,200,200};

static ISC_Shmem_Data_T   *shmem_data_p;
static UI32_T             isc_om_sem_id;

void ISC_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_ISCDRV_SHMEM_SEGID;
    *seglen_p = sizeof(ISC_Shmem_Data_T)+L_STACK_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(ISC_MAX_NBR_OF_PVC);    
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_OM_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for ISC OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_OM_InitiateSystemResources(void)
{
    shmem_data_p = (ISC_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_ISCDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    shmem_data_p->is_allocated = FALSE;    
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_ISC_OM, &isc_om_sem_id);    
    /* for ISC Backdoor begin */
    shmem_data_p->bd_cb.capture_received=FALSE;
    shmem_data_p->bd_cb.display_rx_payload=FALSE;
    shmem_data_p->bd_cb.capture_received_sid=ISC_SID_UNSUPPORTED;
    shmem_data_p->bd_cb.capture_transmitted=FALSE;
    shmem_data_p->bd_cb.display_tx_payload=FALSE;
    shmem_data_p->bd_cb.capture_transmitted_sid=ISC_SID_UNSUPPORTED;
    /* for ISC Backdoor end */
}
 
/*---------------------------------------------------------------------------------
 * FUNCTION : ISC_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ISC OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void ISC_OM_AttachSystemResources(void)
{
    shmem_data_p = (ISC_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_ISCDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_ISC_OM, &isc_om_sem_id);    
}

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
BOOL_T ISC_OM_InitateProcessResource(void)
{
    /*if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &isc_om_sem_id)!=SYSFUN_OK)
        printf("%s:get om sem id fail.\n", __FUNCTION__);

    memset(a, , 0, sizeof(a));*/
    return TRUE;
}

void ISC_OM_Init(void)
{
    UI32_T pvc_no;
    
    /* create a stack for pvc numbers
     */
    if (FALSE == L_STACK_ShMem_Create(&(shmem_data_p->pvc_handle), shmem_data_p->pvc_buffer, ISC_MAX_NBR_OF_PVC))
    {
        printf("%s: L_STACK_ShMem_Create return FALSE\n",__FUNCTION__);
    }
    for (pvc_no=0; pvc_no<ISC_MAX_NBR_OF_PVC; pvc_no++)
    {
        L_STACK_ShMem_Push(&(shmem_data_p->pvc_handle), pvc_no);
    }
    memset(shmem_data_p->pvc_info, 0, sizeof(shmem_data_p->pvc_info));
    /*for (pvc_no=0; pvc_no<ISC_MAX_NBR_OF_PVC; pvc_no++)
    {
        shmem_data_p->pvc_info[pvc_no].rep_buf_offset = L_IPCMEM_GetOffset(&(shmem_data_p->rep_buf[pvc_no][0]));
    }*/
    memset(shmem_data_p->tx_seq_nbr, 0, sizeof(shmem_data_p->tx_seq_nbr) );
    memset(shmem_data_p->isc_sid_delay_tick, 0, sizeof(shmem_data_p->isc_sid_delay_tick) );
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    memset(shmem_data_p->isc_db_tx_counters,0,sizeof(shmem_data_p->isc_db_tx_counters));
#endif
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);     
}

SYS_TYPE_Stacking_Mode_T ISC_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

void ISC_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

void ISC_OM_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

void ISC_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
}

void ISC_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
}

UI32_T ISC_OM_GetMyDrvUnitId(void)
{
    return shmem_data_p->my_drv_unit_id;
}

void ISC_OM_SetMyDrvUnitId(UI32_T drv_unit_id)
{
    ATOM_EXPRESSION(shmem_data_p->my_drv_unit_id = drv_unit_id);
}

UI16_T ISC_OM_GetExistDrvUnitBmp(void)
{
    return shmem_data_p->exist_drv_units;
}

void ISC_OM_SetExistDrvUnitBmp(UI16_T exist_drv_unit_bmp)
{
    ATOM_EXPRESSION(shmem_data_p->exist_drv_units = exist_drv_unit_bmp);
    return;
}

UI16_T ISC_OM_GetValidDrvUnitBmp(void)
{
    return shmem_data_p->valid_drv_units;
}

void ISC_OM_SetValidDrvUnitBmp(UI16_T valid_drv_unit_bmp)
{
    ATOM_EXPRESSION(shmem_data_p->valid_drv_units = valid_drv_unit_bmp);
    return;
}
/*
EPR:ES3628BT-FLF-ZZ-00742
Problem: stack:the master unit hung when remve from the stack.
Rootcause:when the hot removal happend,and other csc is sending pkt to slaves which have been removed 
          and the csc is waiting for the slave reply.so the csc will never receive the slave reply.and it is still in the suspend state
Solution:when the slave removed update isc_om database ,and wakeup the csc which is waiting for the slave reply which is removed
Files:ISC_OM.C,stktplg_engine.c,stktplg_om.c,stktplg_om.h makefile.am,l_stack.c,l_stack.h,isc_init.c
*/
void ISC_OM_SetDrvUnitBmp(UI16_T valid_drv_unit_bmp,UI16_T exist_drv_unit_bmp)
{
    UI16_T number,index;
    BOOL_T ret;
    UI32_T pvc_no;
    
    ISC_OM_ENTER_CRITICAL_SECTION();

    shmem_data_p->valid_drv_units = shmem_data_p->valid_drv_units & valid_drv_unit_bmp;
    shmem_data_p->exist_drv_units = shmem_data_p->exist_drv_units & exist_drv_unit_bmp;
    ret =L_STACK_ShMem_GetNumberOfElement(&(shmem_data_p->pvc_handle), &number);
    if(!ret)
     ISC_OM_LEAVE_CRITICAL_SECTION();
     
    for(index = ISC_MAX_NBR_OF_PVC;index>number;index--)
    {
       L_STACK_ShMem_GetElementValueByindex(&(shmem_data_p->pvc_handle), index-1,&pvc_no);
       shmem_data_p->pvc_info[pvc_no].wait_dst_bmp &= valid_drv_unit_bmp;
       shmem_data_p->pvc_info[pvc_no].dst_bmp      &= valid_drv_unit_bmp; 
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
                    */

       if(!shmem_data_p->pvc_info[pvc_no].dst_bmp)
       {
        shmem_data_p->pvc_info[pvc_no].dst_bmp =ISC_OM_LOCAL_UNIT_BIT;
        SYSFUN_ResumeTaskInTimeoutSuspense(shmem_data_p->pvc_info[pvc_no].tid);
       }
    }
    ISC_OM_LEAVE_CRITICAL_SECTION();
}



BOOL_T ISC_OM_GetStackStatusNormal(void)
{
    return shmem_data_p->IscStackStatusNormal;
}

void ISC_OM_SetStackStatusNormal(BOOL_T stack_status_normal)
{
    ATOM_EXPRESSION(shmem_data_p->IscStackStatusNormal = stack_status_normal);
}

BOOL_T ISC_OM_PopPvcNo(UI32_T *pvc_no_p)
{
    BOOL_T ret;
    ATOM_EXPRESSION(ret = L_STACK_ShMem_Pop(&(shmem_data_p->pvc_handle), pvc_no_p));
    return ret;
}

BOOL_T ISC_OM_PushPvcNo(UI32_T pvc_no)
{
    BOOL_T ret;
    ATOM_EXPRESSION(ret = L_STACK_ShMem_Push(&(shmem_data_p->pvc_handle), pvc_no));
    return ret;
}

BOOL_T ISC_OM_GetPvcInfo(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;
    if (shmem_data_p->pvc_info[pvc_no].magic != ((UI8_T *)(&(shmem_data_p->pvc_info[pvc_no])) - (UI8_T *)shmem_data_p))
        return FALSE;
    ATOM_EXPRESSION(memcpy(pvc_info_p, &(shmem_data_p->pvc_info[pvc_no]), sizeof(WaitReply_Info_T)));
    return TRUE;
}

BOOL_T ISC_OM_SetPvcInfo(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;
    if (pvc_info_p == NULL)
        return FALSE;    
    ISC_OM_ENTER_CRITICAL_SECTION();
    memcpy(&(shmem_data_p->pvc_info[pvc_no]), pvc_info_p, sizeof(WaitReply_Info_T));
    shmem_data_p->pvc_info[pvc_no].magic = (UI8_T *)(&(shmem_data_p->pvc_info[pvc_no])) - (UI8_T *)shmem_data_p;
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

BOOL_T ISC_OM_SetPvcInfoWithSeqNoCheck(UI32_T pvc_no, WaitReply_Info_T *pvc_info_p, UI32_T seq_no)
{
    I32_T mref_handle_offset;
    L_MM_Mref_Handle_T *mref_handle_p;
    
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;

    if (pvc_info_p == NULL)
        return FALSE;

    ISC_OM_ENTER_CRITICAL_SECTION();
    if (shmem_data_p->pvc_info[pvc_no].seq_no != seq_no)
    {
        ISC_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    mref_handle_offset = shmem_data_p->pvc_info[pvc_no].mref_handle_offset;
    if (mref_handle_offset != 0)
    {
        mref_handle_p = (L_MM_Mref_Handle_T *)L_IPCMEM_GetPtr(mref_handle_offset);
        L_MM_Mref_Release(&mref_handle_p);
    }    
    memcpy(&(shmem_data_p->pvc_info[pvc_no]), pvc_info_p, sizeof(WaitReply_Info_T));
    shmem_data_p->pvc_info[pvc_no].magic = (UI8_T *)(&(shmem_data_p->pvc_info[pvc_no])) - (UI8_T *)shmem_data_p;
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

#if 1/* anzhen.zheng, 6/25/2008 */
BOOL_T ISC_OM_GetSidDelayTick(UI32_T service_id, UI32_T *delay_tick)
{
    if (service_id >= ISC_SID_UNSUPPORTED)
        return FALSE;
	
    ATOM_EXPRESSION(*delay_tick = shmem_data_p->isc_sid_delay_tick[service_id]);
    return TRUE;  
}

BOOL_T ISC_OM_SetSidDelayTick(UI32_T service_id, UI32_T delay_tick)
{
    if (service_id >= ISC_SID_UNSUPPORTED)
        return FALSE;
	
    ISC_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->isc_sid_delay_tick[service_id] = delay_tick;
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE; 
}
#endif

BOOL_T ISC_OM_ClearPvcInfo(UI32_T pvc_no)
{
    I32_T mref_handle_offset;
    L_MM_Mref_Handle_T *mref_handle_p;

    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;
    ISC_OM_ENTER_CRITICAL_SECTION();
    mref_handle_offset = shmem_data_p->pvc_info[pvc_no].mref_handle_offset;
    if (mref_handle_offset != 0)
    {
        mref_handle_p = (L_MM_Mref_Handle_T *)L_IPCMEM_GetPtr(mref_handle_offset);
        L_MM_Mref_Release(&mref_handle_p);
    }
    memset(&(shmem_data_p->pvc_info[pvc_no]), 0, sizeof(WaitReply_Info_T));
    //shmem_data_p->pvc_info[pvc_no].rep_buf_offset = L_IPCMEM_GetOffset(&(shmem_data_p->rep_buf[pvc_no][0]));
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

BOOL_T ISC_OM_GetPvcInfoWaitDstBmp(UI32_T pvc_no, UI16_T *wait_dst_bmp_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(*wait_dst_bmp_p = shmem_data_p->pvc_info[pvc_no].wait_dst_bmp);
    return TRUE;    
}

BOOL_T ISC_OM_SetPvcInfoWaitDstBmp(UI32_T pvc_no, UI16_T wait_dst_bmp)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(shmem_data_p->pvc_info[pvc_no].wait_dst_bmp = wait_dst_bmp);
    return TRUE;    
}

BOOL_T ISC_OM_GetPvcInfoDstBmp(UI32_T pvc_no, UI16_T *dst_bmp_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(*dst_bmp_p = shmem_data_p->pvc_info[pvc_no].dst_bmp);
    return TRUE;    
}

BOOL_T ISC_OM_SetPvcInfoDstBmp(UI32_T pvc_no, UI16_T dst_bmp)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(shmem_data_p->pvc_info[pvc_no].dst_bmp = dst_bmp);
    return TRUE;    
}

BOOL_T ISC_OM_GetPvcInfoTaskId(UI32_T pvc_no, UI32_T *task_id_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(*task_id_p = shmem_data_p->pvc_info[pvc_no].tid);
    return TRUE;
}

BOOL_T ISC_OM_GetPvcInfoMrefHandleOffset(UI32_T pvc_no,I32_T *mref_handle_offset)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(*mref_handle_offset = shmem_data_p->pvc_info[pvc_no].mref_handle_offset);
    return TRUE;
}

BOOL_T ISC_OM_ClearPvcInfoMrefHandleOffset(UI32_T pvc_no)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;    
    ATOM_EXPRESSION(shmem_data_p->pvc_info[pvc_no].mref_handle_offset = 0);
    return TRUE;
}

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
BOOL_T ISC_OM_GetNextSeqNo(UI32_T pvc_no, UI16_T *seq_nbr_p)
{
    if (pvc_no>=ISC_MAX_NBR_OF_PVC)
        return FALSE;
    ISC_OM_ENTER_CRITICAL_SECTION();
    *seq_nbr_p = shmem_data_p->tx_seq_nbr[pvc_no];
    (shmem_data_p->tx_seq_nbr[pvc_no])++;
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;    
}

L_MM_Mref_Handle_T *ISC_OM_Find1stFragment(L_MM_Mref_Handle_T *mem_ref_p)
{
    UI32_T  i;
    UI32_T  pdu_len;
    
    L_MM_Mref_Handle_T  *fragment_packet;
    ISC_Header_T        *first_hdr_p, *second_hdr_p;

    second_hdr_p = (ISC_Header_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);
    ISC_OM_ENTER_CRITICAL_SECTION();
    for(i=0;i<SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER;i++)
    {
        if (shmem_data_p->fragment_packet_list[i].packet == NULL)
        {
            continue;
        }
        fragment_packet = (L_MM_Mref_Handle_T *)shmem_data_p->fragment_packet_list[i].packet;
        first_hdr_p = (ISC_Header_T *)L_MM_Mref_GetPdu((L_MM_Mref_Handle_T *)fragment_packet, &pdu_len);
        if (first_hdr_p->pvc_no == second_hdr_p->pvc_no &&
            first_hdr_p->seq_no == second_hdr_p->seq_no)
        {
            fragment_packet = (L_MM_Mref_Handle_T *)shmem_data_p->fragment_packet_list[i].packet;
            shmem_data_p->fragment_packet_list[i].packet = NULL;
            DEV_NICDRV_ReleaseFragmentBuffer(fragment_packet->pkt_info.cos);
            ISC_OM_LEAVE_CRITICAL_SECTION();
            return fragment_packet;
        }
    }
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return NULL;    
}

BOOL_T ISC_OM_Save1stFragment(L_MM_Mref_Handle_T *mem_ref_p)
{
    UI32_T free_entry = SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER;
    UI32_T i;
    UI32_T current_time = SYSFUN_GetSysTick();
    ISC_Header_T       *isc_header_p;
    UI32_T             pdu_len;

    if ( (isc_header_p = L_MM_Mref_GetPdu(mem_ref_p, &pdu_len)) == NULL)
        return FALSE;

    ISC_OM_ENTER_CRITICAL_SECTION();
    for(i=0;i<SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER;i++)
    {
        shmem_data_p->last_fragment_index++;
        if (shmem_data_p->last_fragment_index >= SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER)
            shmem_data_p->last_fragment_index -= SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER;
        if (shmem_data_p->fragment_packet_list[shmem_data_p->last_fragment_index].packet == NULL)
        {
            free_entry = shmem_data_p->last_fragment_index;
            break;
        }
        else if (L_MATH_TimeOut32(current_time, shmem_data_p->fragment_packet_list[shmem_data_p->last_fragment_index].timeout_time))
        {
            L_MM_Mref_Handle_T *frag_mref_p = shmem_data_p->fragment_packet_list[shmem_data_p->last_fragment_index].packet;
        
            DEV_NICDRV_ReleaseFragmentBuffer(frag_mref_p->pkt_info.cos);
            L_MM_Mref_Release((L_MM_Mref_Handle_T **)(&frag_mref_p));
            free_entry = shmem_data_p->last_fragment_index;
            break;                    
        }
    }
    if (free_entry < SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER)
    {
        DEV_NICDRV_ReserveFragmentBuffer(mem_ref_p->pkt_info.cos);
        shmem_data_p->fragment_packet_list[shmem_data_p->last_fragment_index].packet = mem_ref_p;
        shmem_data_p->fragment_packet_list[shmem_data_p->last_fragment_index].timeout_time = SYSFUN_GetSysTick()+
                                                                                FragmentTimeOut[isc_header_p->priority];
        ISC_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
    else
    {
        L_MM_Mref_Release(&mem_ref_p);
        ISC_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
BOOL_T ISC_OM_DB_ClearTxCounters()
{

    ISC_OM_ENTER_CRITICAL_SECTION();
    
    memset(shmem_data_p->isc_db_tx_counters,0,sizeof(shmem_data_p->isc_db_tx_counters));   

    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;    
}
BOOL_T ISC_OM_DB_IncTxCounters(UI8_T sid,UI8_T type)
{
    if (sid >= ISC_SID_UNSUPPORTED)
        sid = ISC_SID_UNSUPPORTED ;
        
    ISC_OM_ENTER_CRITICAL_SECTION();
    
    shmem_data_p->isc_db_tx_counters[sid][type]++;
    
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;    
}
BOOL_T ISC_OM_DB_GetTxCounters(UI32_T *tx_counters)
{
    if(!tx_counters)
      return FALSE;
        
    ISC_OM_ENTER_CRITICAL_SECTION();
    
    memcpy(tx_counters,shmem_data_p->isc_db_tx_counters,sizeof(shmem_data_p->isc_db_tx_counters));
    
    ISC_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;    
}

#endif

ISC_BD_OM_ControlBlock_T* ISC_OM_DB_GetCBPtr(void)
{
    return &(shmem_data_p->bd_cb);
}

